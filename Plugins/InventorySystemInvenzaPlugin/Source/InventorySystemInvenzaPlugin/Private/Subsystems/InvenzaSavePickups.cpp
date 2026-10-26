//  Nublin Studio 2026 All Rights Reserved.

#include "Subsystems/InvenzaSaveSubsystem.h"
#include "ActorComponents/Interactable/PickupComponent.h"
#include "Data/Items/itemBase.h"
#include "SaveLoad/InvenzaSaveGame.h"
#include "Factory/ItemFactory.h"
#include "Engine/World.h"
#include "EngineUtils.h"

static bool CaptureInvenzaPickup(UPickupComponent* Pickup, FInventorySavedPickup& Saved, FString& Error)
{
	auto* Item = Cast<UItemBase>(Pickup->PeekItemData());
	if (!Item)
	{
		Error = TEXT("Pickup has no persistent item instance");
		return false;
	}
	Saved.Key = Pickup->GetPersistentSaveKey();
	Saved.ActorClass = FSoftClassPath(Pickup->GetOwner()->GetClass());
	Saved.Transform = Pickup->GetOwner()->GetActorTransform();
	Saved.bRuntimeActor = Pickup->IsRuntimeSaveActor();
	Saved.Item.InstanceID = Item->GetPersistentInstanceID();
	Saved.Item.InstanceClass = FSoftClassPath(Item->GetClass());
	Saved.Item.Definition = IObjectDataProvider::Execute_GetItemRow(Item);
	Saved.Item.DefinitionTable = FSoftObjectPath(Saved.Item.Definition.DataTable);
	Saved.Item.DefinitionRow = Saved.Item.Definition.RowName;
	Saved.Item.Quantity = IObjectDataProvider::Execute_GetQuantity(Item);
	Saved.Item.SetCustomState(Item->CaptureSaveState());
	return true;
}

void UInvenzaSaveSubsystem::RegisterPickup(UPickupComponent* Pickup)
{
	if (!Pickup || !Pickup->GetOwner()->HasAuthority() || !Pickup->PeekItemData())
	{
		return;
	}
	if (!Pickup->HasAssignedSaveKey() && !Pickup->PersistentPickupID.IsValid() &&
		!Pickup->GetOwner()->HasAnyFlags(RF_WasLoaded) && !Pickup->GetOwner()->IsNetStartupActor())
	{
		Pickup->SetPersistentSaveKey(TEXT("RuntimePickup:") + FGuid::NewGuid().ToString(), true);
	}
	Pickups.Add(Pickup);
	MarkSessionChanged();
}

void UInvenzaSaveSubsystem::UnregisterPickup(UPickupComponent* Pickup, EEndPlayReason::Type Reason)
{
	if (!Pickup || Pickup->GetWorld() != SessionWorld.Get())
	{
		return;
	}
	if (!bApplyingSnapshot && (Reason == EEndPlayReason::Destroyed || Reason == EEndPlayReason::RemovedFromWorld))
	{
		FInventorySavedPickup Saved;
		FString Error;
		if (CaptureInvenzaPickup(Pickup, Saved, Error))
		{
			Saved.bDestroyed = Reason == EEndPlayReason::Destroyed;
			if (Saved.bDestroyed)
			{
				Saved.Item = {};
			}
			RetainedPickups.Add(Saved.Key, MoveTemp(Saved));
		}
		MarkSessionChanged();
	}
	Pickups.Remove(Pickup);
}

bool UInvenzaSaveSubsystem::CapturePickups(UInvenzaSaveGame* Save, FString& Error)
{
	// Include a drop created this frame even before its next-tick pickup initialization callback.
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (auto* Pickup = It->FindComponentByClass<UPickupComponent>(); Pickup && !Pickups.Contains(Pickup))
		{
			RegisterPickup(Pickup);
		}
	}
	TMap<FString, FInventorySavedPickup> Records = RetainedPickups;
	TSet<FString> LiveKeys;
	for (const auto& Weak : Pickups)
	{
		auto* Pickup = Weak.Get();
		if (!Pickup || Pickup->GetWorld() != GetWorld() || Pickup->GetOwner()->IsActorBeingDestroyed())
		{
			continue;
		}
		FInventorySavedPickup Saved;
		if (!CaptureInvenzaPickup(Pickup, Saved, Error))
		{
			return false;
		}
		if (LiveKeys.Contains(Saved.Key))
		{
			Error = TEXT("Duplicate pickup identity");
			return false;
		}
		LiveKeys.Add(Saved.Key);
		Records.Add(Saved.Key, MoveTemp(Saved));
	}
	for (const auto& Pair : Records)
	{
		Save->Pickups.Add(Pair.Value);
	}
	return true;
}

bool UInvenzaSaveSubsystem::PreparePickups(const UInvenzaSaveGame* Save, TArray<FInventoryPreparedPickup>& Plans,
										   TArray<AActor*>& Spawned, TArray<AActor*>& DestroyAfterCommit,
										   FString& Error)
{
	TMap<FString, UPickupComponent*> Current;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (auto* Pickup = It->FindComponentByClass<UPickupComponent>())
		{
			if (!Pickups.Contains(Pickup))
			{
				RegisterPickup(Pickup);
			}
			Current.Add(Pickup->GetPersistentSaveKey(), Pickup);
		}
	}
	TSet<FString> Keys;
	TSet<FGuid> IDs;
	for (const auto& Owner : Save->Owners)
	{
		for (const auto* Items : {&Owner.Items, &Owner.RecoveryItems})
		{
			for (const auto& Item : *Items)
			{
				IDs.Add(Item.InstanceID);
			}
		}
	}
	for (const auto& Saved : Save->Pickups)
	{
		if (Saved.Key.IsEmpty() || Keys.Contains(Saved.Key))
		{
			Error = TEXT("Duplicate saved pickup");
			return false;
		}
		Keys.Add(Saved.Key);
		auto* Pickup = Current.FindRef(Saved.Key);
		if (Saved.bDestroyed)
		{
			if (Pickup)
			{
				DestroyAfterCommit.Add(Pickup->GetOwner());
			}
			continue;
		}
		if (!Saved.Item.InstanceID.IsValid() || IDs.Contains(Saved.Item.InstanceID) || Saved.Item.Quantity <= 0)
		{
			Error = TEXT("Invalid pickup item or duplicate instance");
			return false;
		}
		IDs.Add(Saved.Item.InstanceID);
		if (!Pickup)
		{
			UClass* Class = Saved.ActorClass.TryLoadClass<AActor>();
			if (!Class || Class->HasAnyClassFlags(CLASS_Abstract))
			{
				Error = TEXT("Saved pickup class is unavailable");
				return false;
			}
			FActorSpawnParameters Parameters;
			Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			auto* Actor = GetWorld()->SpawnActor<AActor>(Class, Saved.Transform, Parameters);
			if (Actor)
			{
				Spawned.Add(Actor);
			}
			Pickup = Actor ? Actor->FindComponentByClass<UPickupComponent>() : nullptr;
			if (!Pickup)
			{
				Error = TEXT("Saved pickup component is unavailable");
				return false;
			}
			Pickup->SetPersistentSaveKey(Saved.Key, Saved.bRuntimeActor);
			Pickups.Add(Pickup);
		}
		auto* Item = UItemFactory::CreateItemFromSave(Pickup, Saved.Item);
		if (!Item || IObjectDataProvider::Execute_GetQuantity(Item) != Saved.Item.Quantity)
		{
			Error = TEXT("Pickup definition is unavailable; load was not applied");
			return false;
		}
		auto& Plan = Plans.AddDefaulted_GetRef();
		Plan.Component = Pickup;
		Plan.Item.Reset(Item);
	}
	for (const auto& Pair : Current)
	{
		if (Pair.Value->IsRuntimeSaveActor() && !Keys.Contains(Pair.Key))
		{
			DestroyAfterCommit.Add(Pair.Value->GetOwner());
		}
	}
	return true;
}

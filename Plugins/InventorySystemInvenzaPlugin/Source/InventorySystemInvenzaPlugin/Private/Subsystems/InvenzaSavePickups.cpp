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
	if (Pickup->IsCollecting() && !Pickup->GetOwner()->IsActorBeingDestroyed())
	{
		Error = TEXT("Pickup delivery is in progress; retry saving after it completes");
		return false;
	}
	Saved.Key = Pickup->GetPersistentSaveKey();
	Saved.ActorClass = FSoftClassPath(Pickup->GetOwner()->GetClass());
	Saved.Transform = Pickup->GetOwner()->GetActorTransform();
	Saved.bRuntimeActor = Pickup->IsRuntimeSaveActor();
	Saved.bIsBundle = Pickup->IsBundlePickup();
	for (auto* Object : Pickup->GetPickupItems())
	{
		auto* Item = Cast<UItemBase>(Object);
		if (!Item)
		{
			Error = TEXT("Pickup has an unsupported persistent item instance");
			return false;
		}
		FInventorySavedItem Entry;
		Entry.InstanceID = Item->GetPersistentInstanceID();
		Entry.InstanceClass = FSoftClassPath(Item->GetClass());
		Entry.Definition = IObjectDataProvider::Execute_GetItemRow(Item);
		Entry.DefinitionTable = FSoftObjectPath(Entry.Definition.DataTable);
		Entry.DefinitionRow = Entry.Definition.RowName;
		Entry.Quantity = IObjectDataProvider::Execute_GetQuantity(Item);
		Entry.SetCustomState(Item->CaptureSaveState());
		if (Saved.bIsBundle)
		{
			Saved.Items.Add(MoveTemp(Entry));
		}
		else
		{
			Saved.Item = MoveTemp(Entry);
		}
	}
	if (Saved.GetSavedItems().IsEmpty() && !Pickup->GetOwner()->IsActorBeingDestroyed())
	{
		Error = TEXT("Pickup has no persistent item instances");
		return false;
	}
	return true;
}

void UInvenzaSaveSubsystem::RegisterPickup(UPickupComponent* Pickup)
{
	if (!Pickup || !Pickup->GetOwner()->HasAuthority() || Pickup->GetOwner()->IsActorBeingDestroyed() || !Pickup->HasInitializedContents())
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
				Saved.Items.Reset();
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
		if (auto* Pickup = It->FindComponentByClass<UPickupComponent>(); Pickup && !It->IsActorBeingDestroyed())
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
		if (Saved.Transform.ContainsNaN() || !Saved.Transform.IsValid())
		{
			Error = TEXT("Saved pickup transform is invalid");
			return false;
		}
		const auto SavedItems = Saved.GetSavedItems();
		if (SavedItems.IsEmpty() || (!Saved.bIsBundle && SavedItems.Num() != 1))
		{
			Error = TEXT("Saved pickup has no valid contents");
			return false;
		}
		for (const auto& Item : SavedItems)
		{
			if (!Item.InstanceID.IsValid() || IDs.Contains(Item.InstanceID) || Item.Quantity <= 0)
			{
				Error = TEXT("Invalid pickup item or duplicate instance");
				return false;
			}
			IDs.Add(Item.InstanceID);
		}
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
		auto& Plan = Plans.AddDefaulted_GetRef();
		Plan.Component = Pickup;
		Plan.Transform = Saved.Transform;
		Plan.bIsBundle = Saved.bIsBundle;
		for (const auto& Entry : SavedItems)
		{
			auto* Item = UItemFactory::CreateItemFromSave(Pickup, Entry);
			if (!Item || IObjectDataProvider::Execute_GetQuantity(Item) != Entry.Quantity)
			{
				Error = TEXT("Pickup definition is unavailable; load was not applied");
				return false;
			}
			Plan.Items.Emplace(Item);
		}
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

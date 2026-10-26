//  Nublin Studio 2026 All Rights Reserved.

#include "Subsystems/LinkedStorageSubsystem.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/LinkedStorageIdentityComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Items/itemBase.h"
#include "Data/Settings/LinkedStorageDefinition.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Factory/ItemFactory.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Interface/Interaction/ObjectDataProvider.h"
#include "World/LinkedStorageState.h"

FString ULinkedStorageSubsystem::ResolvePersonalPlayerID(AActor* Player) const
{
	const auto* Pawn = Cast<APawn>(Player);
	const auto* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC || !PC->HasAuthority() || PC->GetWorld() != GetWorld())
	{
		return {};
	}
	if (const auto* Identity = PC->FindComponentByClass<ULinkedStorageIdentityComponent>())
	{
		if (!Identity->GetPersistentPlayerID().IsEmpty())
		{
			return TEXT("Account:") + Identity->GetPersistentPlayerID();
		}
	}
	const auto* State = PC->PlayerState.Get();
	if (State && State->GetUniqueId().IsValid() && State->GetUniqueId().GetType() != FName(TEXT("NULL")))
	{
		return State->GetUniqueId().GetType().ToString() + TEXT(":") + State->GetUniqueId().ToString();
	}
	if (PC->IsLocalController() && (GetWorld()->GetNetMode() == NM_Standalone ||
		GetWorld()->GetNetMode() == NM_ListenServer))
	{
		return TEXT("LocalHost");
	}
	// LAN clients require an authenticated server-side profile ID; display names are not identities.
	return {};
}

FString ULinkedStorageSubsystem::MakeStorageKey(const ULinkedStorageDefinition* Definition,
	ELinkedStorageAccessMode Mode, AActor* Player) const
{
	if (!Definition || Definition->StorageGroupID.IsNone() || !IsValid(Player) ||
		!Player->HasAuthority() || Player->GetWorld() != GetWorld() || GetWorld()->GetNetMode() == NM_Client)
	{
		return {};
	}
	const FString Owner = Mode == ELinkedStorageAccessMode::Personal ? ResolvePersonalPlayerID(Player) : FString();
	if (Mode == ELinkedStorageAccessMode::Personal && Owner.IsEmpty())
	{
		return {};
	}
	if (Mode != ELinkedStorageAccessMode::Personal && Mode != ELinkedStorageAccessMode::Shared)
	{
		return {};
	}
	const FString Group = Definition->StorageGroupID.ToString().ToLower();
	return FString::Printf(TEXT("LinkedStorage:%d:%d:%s:%s"), static_cast<int32>(Mode), Group.Len(), *Group, *Owner);
}

ALinkedStorageState* ULinkedStorageSubsystem::FindStorage(const FString& Key) const
{
	if (Key.IsEmpty())
	{
		return nullptr;
	}
	if (const auto* Cached = Storages.Find(Key); Cached && Cached->IsValid() && !Cached->Get()->IsActorBeingDestroyed())
	{
		return Cached->Get();
	}
	// Also discovers owners recreated by the existing transactional save loader.
	for (TActorIterator<ALinkedStorageState> It(GetWorld()); It; ++It)
	{
		if (!It->IsActorBeingDestroyed() && It->Collection->GetPersistentOwnerKey() == Key)
		{
			Storages.Add(Key, *It);
			return *It;
		}
	}
	return nullptr;
}

ALinkedStorageState* ULinkedStorageSubsystem::FindOrCreate(ULinkedStorageDefinition* Definition,
	ELinkedStorageAccessMode Mode, AActor* Player)
{
	const FString Key = MakeStorageKey(Definition, Mode, Player);
	if (Key.IsEmpty())
	{
		return nullptr;
	}
	if (auto* Existing = FindStorage(Key))
	{
		return Existing->Definition == Definition && Existing->Collection->GetActorInventories().Num() == 1
			? Existing : nullptr;
	}
	if (!Definition->Inventory.Settings.InventoryClass ||
		Definition->Inventory.Settings.InventoryClass->HasAnyClassFlags(CLASS_Abstract) ||
		Definition->Inventory.Settings.bIsReferenceContainer)
	{
		return nullptr;
	}
	FActorSpawnParameters Parameters;
	Parameters.OverrideLevel = GetWorld()->PersistentLevel;
	Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Parameters.bDeferConstruction = true;
	auto* Storage = GetWorld()->SpawnActor<ALinkedStorageState>(ALinkedStorageState::StaticClass(),
		FTransform::Identity, Parameters);
	if (!Storage)
	{
		return nullptr;
	}
	Storage->Definition = Definition;
	Storage->Collection->SetRuntimeSaveIdentity(Key);
	Storage->Collection->InitItemCollection();
	FInventoryStartupData Startup = Definition->Inventory;
	Startup.Settings.CustomInventoryID.Reset();
	Startup.Settings.bAllowItemReferencing = false;
	Startup.Settings.bCollectInvDataFromWidget = false;
	auto* Inventory = UInventoryBase::CreateInventoryAdvanced(Storage, Startup, Storage, Storage->Collection);
	if (!Inventory)
	{
		Storage->Destroy();
		return nullptr;
	}
	Storage->Collection->AddPawnInventory_Internal(Inventory);
	for (const auto& Entry : Startup.StartItems)
	{
		auto* Sample = Entry.Amount > 0 ? UItemFactory::CreateItemByHandle(Storage, Entry.Item, 1) : nullptr;
		const int32 MaxStack = Sample ? IObjectDataProvider::Execute_GetItemRef(Sample).ItemNumeraticData.MaxStackSizeInCharacter : 0;
		if (!Sample || MaxStack <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid initial item in linked storage group %s."), *Definition->StorageGroupID.ToString());
			Storage->Destroy();
			return nullptr;
		}
		int32 Remaining = Entry.Amount;
		while (Remaining > 0)
		{
			const int32 Amount = FMath::Min(Remaining, MaxStack);
			auto* Item = UItemFactory::CreateItemByHandle(Storage, Entry.Item, Amount);
			FItemMoveData Move;
			Move.SourceItem = Item;
			Move.TargetInventory = Inventory;
			if (!Item || Inventory->HandleAddItem(Move).ActualAmountAdded != Amount)
			{
				UE_LOG(LogTemp, Error, TEXT("Initial items do not fit linked storage group %s."), *Definition->StorageGroupID.ToString());
				Storage->Destroy();
				return nullptr;
			}
			Remaining -= Amount;
		}
	}
	Storage->FinishSpawning(FTransform::Identity);
	Storages.Add(Key, Storage);
	return Storage;
}

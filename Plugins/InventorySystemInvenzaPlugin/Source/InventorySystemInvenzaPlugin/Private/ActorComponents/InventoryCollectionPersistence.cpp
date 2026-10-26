//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/Crafting/CraftingComponent.h"
#include "Data/Inventory/Equipment/EquipmentComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/Inventory/ListInventory/ListInventory.h"
#include "Factory/ItemFactory.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "TimerManager.h"
#include "World/LinkedStorageState.h"
#include "Data/Settings/LinkedStorageDefinition.h"

FString UInventoryCollectionComponent::GetPersistentOwnerKey() const
{
	if (!ResolvedSaveKey.IsEmpty())
	{
		return ResolvedSaveKey;
	}
	if (PersistentOwnerID.IsValid())
	{
		return TEXT("Owner:") + PersistentOwnerID.ToString();
	}
	if (const auto* Pawn = Cast<APawn>(GetOwner()); Pawn && Pawn->IsPlayerControlled())
	{
		const auto* PC = Cast<APlayerController>(Pawn->GetController());
		const auto* State = Pawn->GetPlayerState();
		if (PC && PC->IsLocalController())
		{
			return TEXT("Player:Host");
		}
		if (State && State->GetUniqueId().IsValid())
		{
			return TEXT("Player:") + State->GetUniqueId().ToString();
		}
		return TEXT("Player:LAN:") + (State ? State->GetPlayerName() : Pawn->GetName());
	}
	// Existing levels need no asset rewrite. An explicit GUID survives actor/package renames.
	return TEXT("Level:") + UWorld::RemovePIEPrefix(GetOwner()->GetPathName()) + TEXT(":") + GetName();
}

void UInventoryCollectionComponent::SetRuntimeSaveIdentity(const FString& Key)
{
	ResolvedSaveKey = Key;
	bRuntimeSaveActor = true;
}

FString UInventoryCollectionComponent::GetPersistentInventoryKey(UInventoryBase* Inventory) const
{
	if (!Inventory || !ActorInventories.Contains(Inventory))
	{
		return {};
	}
	const auto Settings = Inventory->GetInventorySettings();
	if (!Settings.CustomInventoryID.IsEmpty())
	{
		return TEXT("Custom:") + Settings.CustomInventoryID;
	}
	const FString Tag = Settings.InventoryTag.ToString();
	int32 Ordinal = 0;
	for (UInventoryBase* Other : ActorInventories)
	{
		if (Other == Inventory)
		{
			break;
		}
		if (Other && Other->GetInventorySettings().InventoryTag == Settings.InventoryTag)
		{
			++Ordinal;
		}
	}
	return FString::Printf(TEXT("Tag:%s:%d"), *Tag, Ordinal);
}

bool UInventoryCollectionComponent::CapturePersistence(FInventorySavedOwner& Data, FString& Error)
{
	Data = {};
	Data.Key = GetPersistentOwnerKey();
	if (const auto* Storage = Cast<ALinkedStorageState>(GetOwner()))
	{
		Data.LinkedStorageDefinition = FSoftObjectPath(Storage->Definition);
	}
	Data.ActorClass = FSoftClassPath(GetOwner()->GetClass());
	Data.Transform = GetOwner()->GetActorTransform();
	Data.LevelPackage = UWorld::RemovePIEPrefix(GetOwner()->GetLevel()->GetOutermost()->GetName());
	Data.bRuntimeActor = bRuntimeSaveActor;
	Data.RecoveryItems = RecoveryItems;
	TMap<FString, UInventoryBase*> ByRuntimeID;
	TSet<FString> Keys;
	for (UInventoryBase* Inventory : ActorInventories)
	{
		if (!IsValid(Inventory))
		{
			continue;
		}
		const FString Key = GetPersistentInventoryKey(Inventory);
		if (Keys.Contains(Key))
		{
			Error = TEXT("Duplicate persistent inventory key: ") + Key;
			return false;
		}
		Keys.Add(Key);
		ByRuntimeID.Add(Inventory->GetInventoryContainerID(), Inventory);
		auto& Saved = Data.Inventories.AddDefaulted_GetRef();
		Saved.Key = Key;
		Saved.Size = Inventory->GetInventorySize();
		Saved.Title = Inventory->GetInventorySettings().ContainerTitle;
		Saved.StartupSettings = Inventory->GetInventorySettings();
	}
	for (const auto& Entry : InventoryArray.Items)
	{
		auto* Item = Cast<UItemBase>(Entry.Item);
		if (!Item)
		{
			Error = TEXT("Persistent items must derive from UItemBase");
			return false;
		}
		FInventorySavedItem Saved;
		if (!Item->GetPersistentInstanceID().IsValid())
		{
			Item->SetPersistentInstanceID(FGuid::NewGuid());
		}
		Saved.InstanceID = Item->GetPersistentInstanceID();
		Saved.InstanceClass = FSoftClassPath(Item->GetClass());
		Saved.Definition = IObjectDataProvider::Execute_GetItemRow(Item);
		Saved.DefinitionTable = FSoftObjectPath(Saved.Definition.DataTable);
		Saved.DefinitionRow = Saved.Definition.RowName;
		Saved.Quantity = IObjectDataProvider::Execute_GetQuantity(Item);
		Saved.SetCustomState(Item->CaptureSaveState());
		int32 PhysicalLocations = 0;
		for (const auto& Mapping : Entry.Locations.Mappings)
		{
			auto* Inventory = ByRuntimeID.FindRef(Mapping.InventoryID);
			if (!Inventory)
			{
				continue;
			}
			if (Mapping.OccupiedSlots.IsEmpty() || !Mapping.OccupiedSlots[0])
			{
				Error = TEXT("Item has no valid pivot slot");
				return false;
			}
			auto& Location = Saved.Locations.AddDefaulted_GetRef();
			Location.InventoryKey = GetPersistentInventoryKey(Inventory);
			Location.Cell = Mapping.OccupiedSlots[0]->InventorySlotInfo.CellPosition;
			Location.Orientation = Mapping.ItemOrientation;
			Location.EquipmentSlot = Mapping.OccupiedSlots[0]->InventorySlotInfo.LinkedEquipmentSlot;
			if (auto* List = Cast<UListInventory>(Inventory))
			{
				Location.ListOrder = List->InventorySlots.IndexOfByKey(Mapping.OccupiedSlots[0]);
			}
			if (!Inventory->GetInventorySettings().bIsReferenceContainer)
			{
				++PhysicalLocations;
			}
		}
		if (Saved.Locations.IsEmpty())
		{
			continue;
		}
		if (PhysicalLocations != 1 || Saved.Quantity <= 0 || !Saved.Definition.DataTable ||
			Saved.Definition.RowName.IsNone())
		{
			Error = TEXT("Item must have one physical location, a definition and positive quantity");
			return false;
		}
		Data.Items.Add(MoveTemp(Saved));
	}
	TArray<UCraftingComponent*> CraftComponents;
	GetOwner()->GetComponents(CraftComponents);
	for (auto* Craft : CraftComponents)
	{
		if (!Craft->CapturePersistence(Data.Crafting.AddDefaulted_GetRef(), Error))
		{
			return false;
		}
	}
	return true;
}

bool UInventoryCollectionComponent::PreparePersistence(const FInventorySavedOwner& Data, FInventoryPreparedOwner& Plan,
													   FString& Error, bool bIgnoreCapacityLimits)
{
	if (!GetOwner()->HasAuthority() || Data.Key != GetPersistentOwnerKey())
	{
		Error = TEXT("Snapshot owner does not match the collection");
		return false;
	}
	Plan.Collection = this;
	TMap<FString, int32> ContainerIndices;
	for (const auto& Saved : Data.Inventories)
	{
		if (ContainerIndices.Contains(Saved.Key))
		{
			Error = TEXT("Invalid saved inventory dimensions or duplicate key");
			return false;
		}
		UInventoryBase* Inventory = nullptr;
		for (UInventoryBase* Candidate : ActorInventories)
		{
			if (GetPersistentInventoryKey(Candidate) == Saved.Key)
			{
				Inventory = Candidate;
				break;
			}
		}
		if (!Inventory)
		{
			continue; // Removed inventory definitions retain their items in recovery.
		}
		if (Cast<USlotbasedInventory>(Inventory) &&
			(Saved.Size.X < 1 || Saved.Size.Y < 1 || Saved.Size.X > 256 || Saved.Size.Y > 256))
		{
			Error = TEXT("Invalid grid dimensions for ") + Data.Key + TEXT("/") + Saved.Key;
			return false;
		}
		ContainerIndices.Add(Saved.Key, Plan.Containers.Num());
		auto& Prepared = Plan.Containers.AddDefaulted_GetRef();
		Prepared.Inventory = Inventory;
		Prepared.Size = Saved.Size;
		// Older saves without a title should retain the display name configured in the level.
		Prepared.Title = Saved.Title.IsEmpty() ? Inventory->GetInventorySettings().ContainerTitle : Saved.Title;
		if (auto* Grid = Cast<USlotbasedInventory>(Inventory))
		{
			const auto ExistingSlots = Grid->GetInventorySlots();
			if (Grid->GetInventorySettings().bCollectInvDataFromWidget ||
				!Grid->GetSlotsWithLinkedEquipment().IsEmpty())
			{
				// Authored layouts can contain gaps. Restore items into their real slots,
				// rather than replacing the template with a dense saved rectangle.
				Prepared.Size = Grid->GetInventorySize();
				Prepared.Slots = ExistingSlots;
			}
			else
			{
				for (int32 Row = 0; Row < Saved.Size.X; ++Row)
				{
					for (int32 Column = 0; Column < Saved.Size.Y; ++Column)
					{
						const auto* Old = Grid->GetSlotByPosition(FIntPoint(Row, Column));
						FInventorySlotInfo Info = Old ? Old->InventorySlotInfo
													  : (ExistingSlots.IsEmpty() ? FInventorySlotInfo()
																				 : ExistingSlots[0]->InventorySlotInfo);
						Info.CellPosition = FIntPoint(Row, Column);
						Info.SlotGuid = FGuid::NewGuid();
						auto* Slot = UInventorySlotData::CreateWithData(GetOwner(), Info);
						Plan.CreatedObjects.Emplace(Slot);
						Prepared.Slots.Add(Slot);
					}
				}
			}
		}
	}
	TMap<UInventoryBase*, TSet<UInventorySlotData*>> Occupied;
	TMap<UInventoryBase*, float> Weights;
	TMap<UInventoryBase*, int32> StackCounts;
	auto Place = [&](UObject* Item, const FInventorySavedLocation& Location, FItemMapping& Result) -> bool
	{
		const int32* Index = ContainerIndices.Find(Location.InventoryKey);
		if (!Index)
		{
			return false;
		}
		auto& Container = Plan.Containers[*Index];
		auto* Inventory = Container.Inventory;
		const auto Settings = Inventory->GetInventorySettings();
		const auto Meta = IObjectDataProvider::Execute_GetItemRef(Item);
		const float Weight = IObjectDataProvider::Execute_GetItemStackWeight(Item);
		if (!bIgnoreCapacityLimits &&
			((Settings.MaxStackCount >= 0 && StackCounts.FindRef(Inventory) >= Settings.MaxStackCount) ||
			 (!Settings.bIsReferenceContainer && Settings.InventoryMaxWeightCapacity >= 0 &&
			  Weights.FindRef(Inventory) + Weight > Settings.InventoryMaxWeightCapacity + KINDA_SMALL_NUMBER)))
		{
			return false;
		}
		Result.InventoryID = Inventory->GetInventoryContainerID();
		Result.bIsReferenceContainer = Settings.bIsReferenceContainer;
		Result.ItemOrientation = Location.Orientation;
		if (Cast<UListInventory>(Inventory))
		{
			auto* Slot = UInventorySlotData::Create(GetOwner());
			Plan.CreatedObjects.Emplace(Slot);
			Container.Slots.Add(Slot);
			Result.OccupiedSlots.Add(Slot);
		}
		else if (Cast<USlotbasedInventory>(Inventory))
		{
			auto At = [&](FIntPoint Cell) -> UInventorySlotData*
			{
				const auto* Found = Container.Slots.FindByPredicate(
					[&](const auto* Slot)
					{
						return Slot && Slot->InventorySlotInfo.CellPosition == Cell;
					});
				return Found ? *Found : nullptr;
			};
			auto Fit = [&](FIntPoint Pivot, EItemOrientationType Orientation) -> bool
			{
				const FIntPoint Footprint = Settings.InventorySlotBasedSettings.bIgnoreItemSize
												? FIntPoint(1, 1)
												: IObjectDataProvider::Execute_GetItemSize(Item, Orientation);
				if (Footprint.X <= 0 || Footprint.Y <= 0 || Pivot.X < 0 || Pivot.Y < 0 ||
					Pivot.X + Footprint.Y > Container.Size.X || Pivot.Y + Footprint.X > Container.Size.Y)
				{
					return false;
				}
				TArray<UInventorySlotData*> Cells;
				for (int32 Row = 0; Row < Footprint.Y; ++Row)
				{
					for (int32 Column = 0; Column < Footprint.X; ++Column)
					{
						auto* Slot = At(Pivot + FIntPoint(Row, Column));
						if (!Slot || Occupied.FindOrAdd(Inventory).Contains(Slot) ||
							!Meta.ItemCategory.MatchesTag(Slot->InventorySlotInfo.AllowedCategory))
						{
							return false;
						}
						Cells.Add(Slot);
					}
				}
				Result.OccupiedSlots = Cells;
				Result.ItemOrientation = Orientation;
				return true;
			};
			if (Location.EquipmentSlot.IsValid())
			{
				const auto* Slot = Container.Slots.FindByPredicate(
					[&](const auto* Entry)
					{
						return Entry && Entry->InventorySlotInfo.LinkedEquipmentSlot == Location.EquipmentSlot;
					});
				auto* Equipment = GetOwner()->FindComponentByClass<UEquipmentComponent>();
				if (!Slot || !Equipment ||
					!Equipment->IsCategoryCompatibleWithSlot(Location.EquipmentSlot, Meta.ItemCategory) ||
					!Fit((*Slot)->InventorySlotInfo.CellPosition, Location.Orientation))
				{
					return false;
				}
			}
			else if (!Fit(Location.Cell, Location.Orientation))
			{
				bool bPlaced = false;
				for (const auto Orientation :
					 {Location.Orientation, Location.Orientation == EItemOrientationType::Horizontal
												? EItemOrientationType::Vertical
												: EItemOrientationType::Horizontal})
				{
					for (auto* Slot : Container.Slots)
					{
						if (Fit(Slot->InventorySlotInfo.CellPosition, Orientation))
						{
							bPlaced = true;
							break;
						}
					}
					if (bPlaced)
					{
						break;
					}
				}
				if (!bPlaced)
				{
					return false;
				}
			}
		}
		else
		{
			return false;
		}
		for (UInventorySlotData* Slot : Result.OccupiedSlots)
		{
			Occupied.FindOrAdd(Inventory).Add(Slot);
		}
		Weights.FindOrAdd(Inventory) += Settings.bIsReferenceContainer ? 0 : Weight;
		++StackCounts.FindOrAdd(Inventory);
		return true;
	};
	TArray<FInventorySavedItem> Items = Data.Items;
	Items.Append(Data.RecoveryItems);
	Items.StableSort(
		[](const auto& A, const auto& B)
		{
			return (A.Locations.IsEmpty() ? 0 : A.Locations[0].ListOrder) <
				   (B.Locations.IsEmpty() ? 0 : B.Locations[0].ListOrder);
		});
	TSet<FGuid> IDs;
	for (const auto& Saved : Items)
	{
		if (!Saved.InstanceID.IsValid() || IDs.Contains(Saved.InstanceID) || Saved.Quantity <= 0)
		{
			Error = TEXT("Duplicate item instance or invalid quantity");
			return false;
		}
		IDs.Add(Saved.InstanceID);
		auto* Item = UItemFactory::CreateItemFromSave(this, Saved);
		if (!Item || IObjectDataProvider::Execute_GetQuantity(Item) != Saved.Quantity)
		{
			Plan.RecoveryItems.Add(Saved);
			continue;
		}
		Plan.CreatedObjects.Emplace(Item);
		FInventoryEntry Entry;
		Entry.Item = Item;
		UInventoryBase* PhysicalInventory = nullptr;
		int32 PhysicalCount = 0;
		for (const auto& Location : Saved.Locations)
		{
			const int32* Index = ContainerIndices.Find(Location.InventoryKey);
			if (!Index || Plan.Containers[*Index].Inventory->GetInventorySettings().bIsReferenceContainer)
			{
				continue;
			}
			if (++PhysicalCount > 1)
			{
				Error = TEXT("Item has multiple physical owners");
				return false;
			}
			FItemMapping Mapping;
			if (Place(Item, Location, Mapping))
			{
				PhysicalInventory = Plan.Containers[*Index].Inventory;
				Entry.Locations.Mappings.Add(MoveTemp(Mapping));
			}
		}
		if (!PhysicalInventory)
		{
			Plan.RecoveryItems.Add(Saved);
			continue;
		}
		for (const auto& Location : Saved.Locations)
		{
			const int32* Index = ContainerIndices.Find(Location.InventoryKey);
			if (!Index || !Plan.Containers[*Index].Inventory->GetInventorySettings().bIsReferenceContainer)
			{
				continue;
			}
			if (!PhysicalInventory->GetInventorySettings().bAllowItemReferencing)
			{
				continue;
			}
			FItemMapping Mapping;
			if (Place(Item, Location, Mapping))
			{
				Entry.Locations.Mappings.Add(MoveTemp(Mapping));
			}
		}
		Plan.Entries.Add(MoveTemp(Entry));
	}
	for (const auto& SavedCraft : Data.Crafting)
	{
		UCraftingComponent* Component = nullptr;
		TArray<UCraftingComponent*> Components;
		GetOwner()->GetComponents(Components);
		for (auto* Candidate : Components)
		{
			if (Candidate->GetFName() == SavedCraft.ComponentName)
			{
				Component = Candidate;
			}
		}
		if (!Component)
		{
			Error = TEXT("Saved crafting component is unavailable");
			return false;
		}
		if (!Component->PreparePersistence(SavedCraft, Plan.Crafting.AddDefaulted_GetRef(), Error))
		{
			return false;
		}
	}
	return true;
}

void UInventoryCollectionComponent::CommitPersistence(FInventoryPreparedOwner& Plan)
{
	if (auto* Equipment = GetOwner()->FindComponentByClass<UEquipmentComponent>())
	{
		for (const auto& Slot : Equipment->GetEquipmentSlotsArray())
		{
			Equipment->Server_UnequipItemFromSlot(Slot.SlotTag);
		}
	}
	InventoryArray.Items = MoveTemp(Plan.Entries);
	InventoryArray.MarkArrayDirty();
	RecoveryItems = Plan.RecoveryItems;
	for (auto& Craft : Plan.Crafting)
	{
		Craft.Component->CommitPersistence(Craft);
	}
	for (auto& Container : Plan.Containers)
	{
		Container.Inventory->SetInventorySize(Container.Size);
		if (auto* Grid = Cast<USlotbasedInventory>(Container.Inventory))
		{
			Grid->SetInventorySlots(Container.Slots);
		}
		if (auto* List = Cast<UListInventory>(Container.Inventory))
		{
			List->InventorySlots = Container.Slots;
		}
		auto Settings = Container.Inventory->GetInventorySettings();
		Settings.ContainerTitle = Container.Title;
		Container.Inventory->SetInventorySettings(Settings);
	}
}

void UInventoryCollectionComponent::NotifyPersistenceRestored()
{
	for (const auto& Entry : InventoryArray.Items)
	{
		for (const auto& Mapping : Entry.Locations.Mappings)
		{
			for (UInventorySlotData* Slot : Mapping.OccupiedSlots)
			{
				if (Slot && Slot->InventorySlotInfo.LinkedEquipmentSlot.IsValid())
				{
					if (auto* Equipment = GetOwner()->FindComponentByClass<UEquipmentComponent>())
					{
						Equipment->Server_EquipItemToSlot(Slot->InventorySlotInfo.LinkedEquipmentSlot, Entry.Item);
					}
				}
			}
		}
	}
	for (UInventoryBase* Inventory : ActorInventories)
	{
		if (auto* Grid = Cast<USlotbasedInventory>(Inventory))
		{
			Grid->OnInventorySlotDataUpdated.Broadcast();
		}
		if (auto* List = Cast<UListInventory>(Inventory))
		{
			List->RefreshItemEntries();
		}
		Inventory->UpdateWeightInfo();
		Inventory->UpdateMoneyInfo();
		Inventory->OnInventoryRedrawRequested.Broadcast();
		OnInventoryItemsChanged.Broadcast(Inventory->GetInventoryContainerID());
	}
	OnActorInventoriesChanged.Broadcast();
	TArray<UCraftingComponent*> CraftComponents;
	GetOwner()->GetComponents(CraftComponents);
	for (auto* Craft : CraftComponents)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(Craft, &UCraftingComponent::ResumeAfterPersistence);
	}
	GetOwner()->ForceNetUpdate();
}

void UInventoryCollectionComponent::ResolvePlayerSaveIdentity()
{
	if (GetOwner()->HasAuthority() && ResolvedSaveKey.IsEmpty())
	{
		if (const auto* Pawn = Cast<APawn>(GetOwner()); Pawn && Pawn->IsPlayerControlled())
		{
			ResolvedSaveKey = GetPersistentOwnerKey();
		}
	}
}

//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/UIInventoryManager.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/Interactable/CharacterInventoryAccessComponent.h"
#include "Data/Inventory/Equipment/EquipmentComponent.h"
#include "Data/Inventory/Equipment/EquipmentTransferTypes.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/Inventory/ListInventory/ListInventory.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Interface/Interaction/ObjectDataProvider.h"

bool UIInventoryManager::TryResolveEquipmentContext(UInventoryBase* Inventory, FGuid SlotID,
													FEquipmentInventoryContext& OutContext) const
{
	OutContext = {};
	if (!IsValid(Inventory) || !GlobalSettings ||
		Inventory->GetInventorySettings().InventoryTag != GlobalSettings->EquipmentInvTag)
	{
		return false;
	}
	auto* Collection = Inventory->GetItemCollectionLinked();
	auto* Actor = Inventory->GetInventoryOwnerActor();
	if (!IsValid(Collection) || !IsValid(Actor) || Actor != Collection->GetOwner() ||
		!Collection->GetActorInventories().Contains(Inventory))
	{
		return false;
	}
	auto* Equipment = Actor->FindComponentByClass<UEquipmentComponent>();
	if (!IsValid(Equipment))
	{
		return false;
	}
	OutContext.Collection = Collection;
	OutContext.Equipment = Equipment;
	OutContext.Slot = Inventory->GetSlotByGuid(SlotID);
	if (OutContext.Slot)
	{
		OutContext.SlotTag = OutContext.Slot->InventorySlotInfo.LinkedEquipmentSlot;
	}
	return true;
}

static bool IsEquipmentTransferInventory(UInventoryBase* Inventory, const UInvenzaInventorySettingsAsset* Settings)
{
	return IsValid(Inventory) && Settings &&
		   Inventory->GetInventorySettings().InventoryTag == Settings->EquipmentInvTag;
}

static UInventoryBase* FindEquipmentTransferBag(UInventoryCollectionComponent* Collection,
												const UInvenzaInventorySettingsAsset* Settings)
{
	if (!Collection || !Settings)
	{
		return nullptr;
	}
	if (auto* Access = Collection->GetOwner()->FindComponentByClass<UCharacterInventoryAccessComponent>())
	{
		auto* Bag = Access->GetMainLootContainer().Get();
		if (Access->ContainsInventory(Bag) && !Bag->GetInventorySettings().bIsReferenceContainer)
		{
			return Bag;
		}
	}
	for (auto* Inventory : Collection->GetActorInventories())
	{
		if (IsValid(Inventory) && Inventory->GetInventorySettings().InventoryTag == Settings->MainInvTag &&
			!Inventory->GetInventorySettings().bIsReferenceContainer)
		{
			return Inventory;
		}
	}
	return nullptr;
}

// Read-only placement against the final planned occupancy, including slots freed by this move.
static bool PlanEquipmentTransferPlacement(FEquipmentInventoryTransferState& State, UObject* Item, FGuid TargetSlotID,
										   EItemOrientationType Orientation)
{
	auto* Inventory = State.Inventory;
	const auto Settings = Inventory->GetInventorySettings();
	const auto Meta = IObjectDataProvider::Execute_GetItemRef(Item);
	const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(Item);
	if (Quantity <= 0 || State.After.Contains(Item))
	{
		return false;
	}
	if (Settings.MaxStackCount >= 0 && State.After.Num() >= Settings.MaxStackCount)
	{
		return false;
	}
	if (Settings.InventoryMaxWeightCapacity >= 0 && !Settings.bIsReferenceContainer)
	{
		float Weight = IObjectDataProvider::Execute_GetItemSingleWeight(Item) * Quantity;
		for (const auto& Pair : State.After)
		{
			Weight += IObjectDataProvider::Execute_GetItemSingleWeight(Pair.Key) *
					  IObjectDataProvider::Execute_GetQuantity(Pair.Key);
		}
		if (Weight > Settings.InventoryMaxWeightCapacity + KINDA_SMALL_NUMBER)
		{
			return false;
		}
	}
	FItemMapping Mapping;
	Mapping.InventoryID = Inventory->GetInventoryContainerID();
	Mapping.bIsReferenceContainer = Settings.bIsReferenceContainer;
	Mapping.ItemOrientation = Orientation;
	if (auto* Grid = Cast<USlotbasedInventory>(Inventory))
	{
		TArray<UInventorySlotData*> Candidates;
		if (TargetSlotID.IsValid())
		{
			auto* Slot = Grid->GetSlotByGuid(TargetSlotID);
			if (!Slot)
			{
				return false;
			}
			Candidates.Add(Slot);
		}
		else
		{
			Candidates = Grid->GetInventorySlots();
		}
		TSet<UInventorySlotData*> Occupied;
		for (const auto& Pair : State.After)
		{
			for (UInventorySlotData* Slot : Pair.Value.OccupiedSlots)
			{
				Occupied.Add(Slot);
			}
		}
		for (int32 Rotation = 0; Rotation < (TargetSlotID.IsValid() ? 1 : 2); ++Rotation)
		{
			const auto TestOrientation =
				Rotation == 0 ? Orientation
							  : (Orientation == EItemOrientationType::Horizontal ? EItemOrientationType::Vertical
																				 : EItemOrientationType::Horizontal);
			const FIntPoint Footprint = Settings.InventorySlotBasedSettings.bIgnoreItemSize
											? FIntPoint(1, 1)
											: IObjectDataProvider::Execute_GetItemSize(Item, TestOrientation);
			for (auto* Candidate : Candidates)
			{
				if (!Candidate)
				{
					continue;
				}
				auto Slots = Grid->GetSlotsForItemAt(Candidate->InventorySlotInfo.CellPosition, Item, TestOrientation);
				if (Footprint.X <= 0 || Footprint.Y <= 0 || Slots.Num() != Footprint.X * Footprint.Y)
				{
					continue;
				}
				if (Slots.ContainsByPredicate(
						[&](UInventorySlotData* Slot)
						{
							return !Slot || Occupied.Contains(Slot) ||
								   !Meta.ItemCategory.MatchesTag(Slot->InventorySlotInfo.AllowedCategory);
						}))
				{
					continue;
				}
				Mapping.OccupiedSlots = Slots;
				Mapping.ItemOrientation = TestOrientation;
				State.After.Add(Item, Mapping);
				return true;
			}
		}
		return false;
	}
	if (Cast<UListInventory>(Inventory) && !Settings.bIsReferenceContainer)
	{
		Mapping.OccupiedSlots.Add(UInventorySlotData::Create(Inventory->GetInventoryOwnerActor()));
		State.After.Add(Item, Mapping);
		return true;
	}
	return false;
}

bool UIInventoryManager::CheckEquipmentTransfer(FItemMoveData ItemMoveData, bool& OutCanTransfer)
{
	OutCanTransfer = false;
	return TryApplyEquipmentTransfer(ItemMoveData, true, &OutCanTransfer);
}

bool UIInventoryManager::TryApplyEquipmentTransfer(FItemMoveData Move, bool bOnlyCheck, bool* OutCanTransfer)
{
	const bool bSourceGear = IsEquipmentTransferInventory(Move.SourceInventory, GlobalSettings);
	const bool bTargetGear = IsEquipmentTransferInventory(Move.TargetInventory, GlobalSettings);
	auto* SourceCollection = IsValid(Move.SourceInventory) ? Move.SourceInventory->GetItemCollectionLinked() : nullptr;
	auto* TargetCollection = IsValid(Move.TargetInventory) ? Move.TargetInventory->GetItemCollectionLinked() : nullptr;
	auto* SourceEquipment = SourceCollection && SourceCollection->GetOwner()
								? SourceCollection->GetOwner()->FindComponentByClass<UEquipmentComponent>()
								: nullptr;
	const bool bLeavingOwner = SourceCollection && TargetCollection && SourceCollection != TargetCollection;
	FGameplayTag PreviouslyEquippedSlot;
	const bool bTransfersEquippedItem =
		bLeavingOwner && SourceEquipment && SourceEquipment->FindSlotByItem(Move.SourceItem, PreviouslyEquippedSlot);
	if (!bSourceGear && !bTargetGear && !bTransfersEquippedItem)
	{
		return false;
	}
	// Returning true also consumes rejected equipment requests: never fall through to the old move path.
	if (bApplyingEquipmentTransfer || (!bOnlyCheck && !GetOwner()->HasAuthority()) ||
		!IsInventoryAccessible(Move.SourceInventory) || !IsInventoryAccessible(Move.TargetInventory) ||
		!SourceCollection || !TargetCollection || !IsValid(Move.SourceItem) ||
		!Move.SourceItem->Implements<UObjectDataProvider>())
	{
		return true;
	}
	TGuardValue<bool> TransferGuard(bApplyingEquipmentTransfer, true);
	const auto* SourceMapping = SourceCollection->FindItemMappingByContainerName(
		Move.SourceItem, Move.SourceInventory->GetInventoryContainerID());
	if (!SourceMapping || SourceMapping->OccupiedSlots.IsEmpty() || !SourceMapping->OccupiedSlots[0])
	{
		return true;
	}
	if (!Move.SourceSlotID.IsValid())
	{
		Move.SourceSlotID = SourceMapping->OccupiedSlots[0]->InventorySlotInfo.SlotGuid;
	}
	if (!SourceMapping->OccupiedSlots.ContainsByPredicate(
			[&](UInventorySlotData* Slot)
			{
				return Slot && Slot->InventorySlotInfo.SlotGuid == Move.SourceSlotID;
			}))
	{
		return true;
	}
	FEquipmentInventoryContext SourceContext, TargetContext;
	if (bSourceGear && (!TryResolveEquipmentContext(Move.SourceInventory, Move.SourceSlotID, SourceContext) ||
						!SourceContext.Equipment->IsItemEquippedInSlot(Move.SourceItem, SourceContext.SlotTag)))
	{
		return true;
	}
	if (bTargetGear)
	{
		if (!TryResolveEquipmentContext(Move.TargetInventory, Move.TargetSlotID, TargetContext))
		{
			return true;
		}
		if (!TargetContext.Slot)
		{
			// Quick equip selects a compatible free slot first, then an occupied slot.
			for (const FGuid ID : Move.TargetInventory->GetSlotsWithLinkedEquipment())
			{
				auto* Slot = Move.TargetInventory->GetSlotByGuid(ID);
				if (!Slot || !TargetContext.Equipment->IsCategoryCompatibleWithSlot(
								 Slot->InventorySlotInfo.LinkedEquipmentSlot,
								 IObjectDataProvider::Execute_GetItemRef(Move.SourceItem).ItemCategory))
				{
					continue;
				}
				if (!TargetContext.Slot ||
					!TargetContext.Equipment->IsSlotOccupied(Slot->InventorySlotInfo.LinkedEquipmentSlot))
				{
					TargetContext.Slot = Slot;
					TargetContext.SlotTag = Slot->InventorySlotInfo.LinkedEquipmentSlot;
				}
				if (!TargetContext.Equipment->IsSlotOccupied(TargetContext.SlotTag))
				{
					break;
				}
			}
			if (Move.TargetSlotID.IsValid() || !TargetContext.Slot)
			{
				return true;
			}
			Move.TargetSlotID = TargetContext.Slot->InventorySlotInfo.SlotGuid;
		}
		if (!TargetContext.Equipment->DoesSlotExist(TargetContext.SlotTag) ||
			!TargetContext.Equipment->IsCategoryCompatibleWithSlot(
				TargetContext.SlotTag, IObjectDataProvider::Execute_GetItemRef(Move.SourceItem).ItemCategory))
		{
			return true;
		}
		if (SourceContext.Equipment == TargetContext.Equipment && SourceContext.SlotTag == TargetContext.SlotTag)
		{
			return true;
		}
	}

	TArray<FEquipmentInventoryTransferState> States;
	for (auto* Collection : {SourceCollection, TargetCollection})
	{
		for (auto* Inventory : Collection->GetActorInventories())
		{
			if (!IsValid(Inventory) || States.ContainsByPredicate(
										   [&](const auto& State)
										   {
											   return State.Inventory == Inventory;
										   }))
			{
				continue;
			}
			auto& State = States.AddDefaulted_GetRef();
			State.Inventory = Inventory;
			for (const auto& Pair : Collection->GetItemsWithMappingsByContainer(Inventory->GetInventoryContainerID()))
			{
				if (Pair.Value)
				{
					State.Before.Add(Pair.Key, *Pair.Value);
				}
			}
			State.After = State.Before;
		}
	}
	auto StateFor = [&](UInventoryBase* Inventory) -> FEquipmentInventoryTransferState*
	{
		return States.FindByPredicate(
			[&](const auto& State)
			{
				return State.Inventory == Inventory;
			});
	};
	if (!StateFor(Move.SourceInventory) || !StateFor(Move.TargetInventory))
	{
		return true;
	}
	TSet<UInventoryBase*> Changed;
	auto Remove = [&](UInventoryBase* Inventory, UObject* Item)
	{
		if (auto* State = StateFor(Inventory); State && State->After.Remove(Item))
		{
			Changed.Add(Inventory);
		}
	};
	auto Place = [&](UInventoryBase* Inventory, UObject* Item, FGuid SlotID, EItemOrientationType Orientation)
	{
		auto* State = StateFor(Inventory);
		if (!State)
		{
			return false;
		}
		if (State->After.Contains(Item) && !SlotID.IsValid())
		{
			return true;
		}
		State->After.Remove(Item);
		if (!PlanEquipmentTransferPlacement(*State, Item, SlotID, Orientation))
		{
			return false;
		}
		Changed.Add(Inventory);
		return true;
	};
	const bool bReferenceTarget = Move.TargetInventory->GetInventorySettings().bIsReferenceContainer;
	if (bReferenceTarget && !bTargetGear)
	{
		return true;
	}
	if (bLeavingOwner)
	{
		for (auto* Inventory : SourceCollection->GetActorInventories())
		{
			Remove(Inventory, Move.SourceItem);
		}
	}
	else
	{
		if (!bReferenceTarget || Move.SourceInventory->GetInventorySettings().bIsReferenceContainer)
		{
			Remove(Move.SourceInventory, Move.SourceItem);
		}
		// A reference item can occupy only one equipment slot on its owner.
		if (bSourceGear || bTargetGear)
		{
			for (auto* Inventory : SourceCollection->GetActorInventories())
			{
				if (IsEquipmentTransferInventory(Inventory, GlobalSettings))
				{
					Remove(Inventory, Move.SourceItem);
				}
			}
		}
	}
	if (bTargetGear)
	{
		auto* Previous =
			TargetCollection->GetItemFromSlot(Move.TargetSlotID, Move.TargetInventory->GetInventoryContainerID());
		if (Previous && Previous != Move.SourceItem)
		{
			Remove(Move.TargetInventory, Previous);
			if (!bReferenceTarget &&
				!Place(FindEquipmentTransferBag(TargetCollection, GlobalSettings), Previous, {}, Move.SavedOrientation))
			{
				return true;
			}
		}
		const bool bHasPlannedPhysicalLocation = States.ContainsByPredicate(
			[&](const auto& State)
			{
				return State.Inventory->GetItemCollectionLinked() == TargetCollection &&
					   !State.Inventory->GetInventorySettings().bIsReferenceContainer &&
					   State.After.Contains(Move.SourceItem);
			});
		if (bReferenceTarget && !bHasPlannedPhysicalLocation)
		{
			auto* Bag = FindEquipmentTransferBag(TargetCollection, GlobalSettings);
			if (!Bag || !Bag->GetInventorySettings().bAllowItemReferencing ||
				!Place(Bag, Move.SourceItem, {}, Move.TargetOrientation))
			{
				return true;
			}
		}
		if (bReferenceTarget && bHasPlannedPhysicalLocation &&
			!States.ContainsByPredicate(
				[&](const auto& State)
				{
					return State.Inventory->GetItemCollectionLinked() == TargetCollection &&
						   !State.Inventory->GetInventorySettings().bIsReferenceContainer &&
						   State.Inventory->GetInventorySettings().bAllowItemReferencing &&
						   State.After.Contains(Move.SourceItem);
				}))
		{
			return true;
		}
	}
	if (!Place(Move.TargetInventory, Move.SourceItem, Move.TargetSlotID, Move.TargetOrientation))
	{
		return true;
	}
	if (OutCanTransfer)
	{
		*OutCanTransfer = true;
	}
	if (bOnlyCheck)
	{
		return true;
	}
	// Every mapping and capacity check has succeeded. Commit without intermediate UI/equipment events.
	auto WriteStates = [&](bool bRestore)
	{
		for (auto& State : States)
		{
			if (!Changed.Contains(State.Inventory))
			{
				continue;
			}
			auto* Collection = State.Inventory->GetItemCollectionLinked();
			for (auto* Item : Collection->GetAllItemsByContainer(State.Inventory->GetInventoryContainerID()))
			{
				Collection->RemoveItem(Item, State.Inventory->GetInventoryContainerID());
			}
			for (const auto& Pair : bRestore ? State.Before : State.After)
			{
				Collection->AddItem(Pair.Key, Pair.Value);
			}
		}
	};
	WriteStates(false);
	for (const auto& State : States)
	{
		for (const auto& Pair : State.After)
		{
			if (Changed.Contains(State.Inventory) && !State.Inventory->GetItemCollectionLinked()->ItemHasInventory(
														 Pair.Key, State.Inventory->GetInventoryContainerID()))
			{
				WriteStates(true);
				return true;
			}
		}
	}
	// Reconcile only after a successful commit. This handles source bag references too.
	for (auto* Collection : {SourceCollection, TargetCollection})
	{
		auto* Equipment = Collection->GetOwner()->FindComponentByClass<UEquipmentComponent>();
		if (!Equipment)
		{
			continue;
		}
		TMap<FGameplayTag, UObject*> DesiredEquipment;
		for (auto* Inventory : Collection->GetActorInventories())
		{
			if (IsEquipmentTransferInventory(Inventory, GlobalSettings))
			{
				for (const FGuid ID : Inventory->GetSlotsWithLinkedEquipment())
				{
					if (auto* Slot = Inventory->GetSlotByGuid(ID))
					{
						DesiredEquipment.Add(Slot->InventorySlotInfo.LinkedEquipmentSlot,
											 Collection->GetItemFromSlot(ID, Inventory->GetInventoryContainerID()));
					}
				}
			}
		}
		for (const auto& Slot : Equipment->GetEquipmentSlotsArray())
		{
			if (DesiredEquipment.Contains(Slot.SlotTag) && DesiredEquipment[Slot.SlotTag] != Slot.EquippedItem)
			{
				Equipment->Server_UnequipItemFromSlot(Slot.SlotTag);
			}
		}
		for (const auto& Pair : DesiredEquipment)
		{
			if (Pair.Value && !Equipment->IsItemEquippedInSlot(Pair.Value, Pair.Key))
			{
				Equipment->Server_EquipItemToSlot(Pair.Key, Pair.Value);
			}
		}
	}
	for (auto* Inventory : Changed)
	{
		Inventory->UpdateWeightInfo();
		Inventory->UpdateMoneyInfo();
		if (auto* List = Cast<UListInventory>(Inventory))
		{
			List->InventorySlots.Reset();
			for (const auto& Mapping :
				 Inventory->GetItemCollectionLinked()->GetAllMappingsByContainer(Inventory->GetInventoryContainerID()))
			{
				List->InventorySlots.Append(Mapping.OccupiedSlots);
			}
			List->RefreshItemEntries();
		}
		Inventory->OnInventoryRedrawRequested.Broadcast();
		Inventory->GetItemCollectionLinked()->OnInventoryItemsChanged.Broadcast(Inventory->GetInventoryContainerID());
	}
	return true;
}

void UIInventoryManager::Server_EquipInventoryItem_Implementation(UInventoryBase* Inventory, UObject* Item,
																  bool bUnequip)
{
	if (!IsInventoryAccessible(Inventory) || !IsValid(Item) || !Item->Implements<UObjectDataProvider>() ||
		!GlobalSettings)
	{
		return;
	}
	auto* Collection = Inventory->GetItemCollectionLinked();
	if (!Collection || !Collection->GetActorInventories().Contains(Inventory) ||
		!Collection->ItemHasInventory(Item, Inventory->GetInventoryContainerID()))
	{
		return;
	}
	if (bUnequip)
	{
		for (auto* Gear : Collection->GetActorInventories())
		{
			if (!IsEquipmentTransferInventory(Gear, GlobalSettings) || !IsInventoryAccessible(Gear))
			{
				continue;
			}
			const auto* Mapping = Collection->FindItemMappingByContainerName(Item, Gear->GetInventoryContainerID());
			if (!Mapping || Mapping->OccupiedSlots.IsEmpty() || !Mapping->OccupiedSlots[0])
			{
				continue;
			}
			FItemMoveData Move(Item, Gear, Mapping->OccupiedSlots[0]->InventorySlotInfo.SlotGuid,
							   FindEquipmentTransferBag(Collection, GlobalSettings), {});
			Move.SavedOrientation = Mapping->ItemOrientation;
			Handle_ItemTransferRequest(Move);
			return;
		}
		return;
	}
	const auto* Mapping = Collection->FindItemMappingByContainerName(Item, Inventory->GetInventoryContainerID());
	if (!Mapping || Mapping->OccupiedSlots.IsEmpty() || !Mapping->OccupiedSlots[0])
	{
		return;
	}
	for (auto* Gear : Collection->GetActorInventories())
	{
		if (!IsEquipmentTransferInventory(Gear, GlobalSettings) || !IsInventoryAccessible(Gear))
		{
			continue;
		}
		FItemMoveData Move(Item, Inventory, Mapping->OccupiedSlots[0]->InventorySlotInfo.SlotGuid, Gear, {});
		Move.SavedOrientation = Mapping->ItemOrientation;
		Handle_ItemTransferRequest(Move);
		return;
	}
}

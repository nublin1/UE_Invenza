//  Nublin Studio 2026 All Rights Reserved.


#include "Data/Inventory/ListInventory/ListInventory.h"

#include "ActorComponents/ItemCollection.h"
#include "Data/Items/ItemBase.h"
#include "UI/Inventory/ListInventorySlotWidget.h"


UListInventory::UListInventory()
{
}

void UListInventory::InitInventory()
{
	Super::InitInventory();

	if (InventorySettings.EntryClass)
		EntryClass = InventorySettings.EntryClass;
}

void UListInventory::SortItemsInContainerByName()
{
	if (!ItemCollectionLinked)
		return;
	
	MergeStackableItems();
	
	auto SortByName = [](const UItemBase& A, const UItemBase& B)
	{
		return A.GetItemDisplayText().ToString() < B.GetItemDisplayText().ToString();
	};
	
	auto AllItems = ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID);
	if (AllItems.IsEmpty())
		return;

	AllItems.Sort(SortByName);

	InvSlotsArray.Empty();
	FilteredInvSlotsArray.Empty();

	for (auto Item : AllItems)
	{
		UInventoryListEntry* EntryObject = NewObject<UInventoryListEntry>(this, EntryClass);
		EntryObject->Item = Item;
		InvSlotsArray.Add(EntryObject);
	}
	
	NotifyReDrawRequest();
	NotifyUpdateWeight();
	NotifyUpdateMoney();
}

void UListInventory::HandleRemoveItemsByType(UItemBase* ItemSample, int32 RequestedAmount)
{
	if (!ItemSample || RequestedAmount <= 0) return;
	
	int32 RemainingToRemove = RequestedAmount;
	int32 RemovedTotal = 0;
	TArray<UItemBase*> FoundItems =	ItemCollectionLinked->GetAllSameItemsInContainer(InventoryContainerID, ItemSample);
	if (FoundItems.IsEmpty())
		return;

	for (UItemBase* Item : FoundItems)
	{
		if (!Item || RemainingToRemove <= 0)
			break;

		int32 Removed = TryRemoveFromStackItem(Item, RemainingToRemove);

		RemainingToRemove -= Removed;
		RemovedTotal += Removed;
	}
}

void UListInventory::HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity)
{
	if (!Item) return;

	auto RemovedActual = TryRemoveFromStackItem(Item, RemoveQuantity);

	UpdateInvSlotsArray();
}

FItemAddResult UListInventory::HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
	if (!ItemCollectionLinked)
		return FItemAddResult::AddedNone(FText::FromString("ItemCollectionLinked is null"));
	
	if (!ItemMoveData.SourceItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item is null. Nothing to add"));
		return FItemAddResult::AddedNone(FText::FromString("Item is null. Nothing to add"));
	}

	if (ItemMoveData.SourceInventory
		&& ItemMoveData.SourceInventory->GetInventorySettings().bIsReferenceContainer
		&& InventorySettings.bIsReferenceContainer
		&& ItemMoveData.SourceInventory != ItemMoveData.TargetInventory)
	{
		return FItemAddResult::AddedNone(
			FText::FromString("Moving items between reference containers is not allowed.")
		);
	}

	if (ItemMoveData.SourceInventory
		&& ItemMoveData.SourceInventory != ItemMoveData.TargetInventory
		&& !ItemMoveData.SourceInventory->GetInventorySettings().bAllowItemReferencing
		&& InventorySettings.bIsReferenceContainer)
	{
		return FItemAddResult::AddedNone(
		FText::Format(
			FText::FromString("Item {0} cannot be added because the source inventory does not allow referencing."),
			FText::FromName(ItemMoveData.SourceItem->GetItemID())));
	}

	if (ItemMoveData.SourceInventory == this)
	{
		return FItemAddResult::AddedNone(
			FText::Format( FText::FromString("Item {0} is already inside this inventory."),
			FText::FromName(ItemMoveData.SourceItem->GetItemID())));
	}

	if (InventorySettings.bIsReferenceContainer)
		return HandleAddReferenceItem(ItemMoveData, bOnlyCheck);

	if (!ItemMoveData.SourceItem->IsStackable())
		return HandleNonStackableItems(ItemMoveData, bOnlyCheck);
	
	
	if (ItemMoveData.SourceItem->IsStackable())
	{
		return TryAddStackableItem(ItemMoveData, bOnlyCheck);
	}
	

	return FItemAddResult::AddedNone(FText::Format(FText::FromString("Couldn't add {0} to inventory."),
	                                               ItemMoveData.SourceItem->GetItemRef().ItemTextData.DisplayName));
}

FItemAddResult UListInventory::HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	if (ItemMoveData.TargetSlot == nullptr)
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
		                                               1, FText::FromName(ItemMoveData.SourceItem->GetItemID())));

	if (ItemMoveData.SourceInventory == this)
	{
		return FItemAddResult::AddedNone(
			FText::Format(
				FText::FromString("Cannot move {0} of {1} within the same inventory."),
				1,
				ItemMoveData.SourceItem->GetItemRef().ItemTextData.DisplayName
			));
	}

	UListInventorySlotWidget* ListInventorySlot = NewObject<UListInventorySlotWidget>();
	FItemMapping Slots;
	Slots.InventoryID = InventoryContainerID;
	Slots.OccupiedSlots.Add(ListInventorySlot->GetSlotData());

	if (!bOnlyCheck)
		AddNewItem(ItemMoveData, Slots, ItemMoveData.SourceItem->GetQuantity());

	TMap<UInventorySlotData*, FItemPlacementData> AffectedSlots;
	AffectedSlots.Add(ItemMoveData.TargetSlot->GetSlotData(), {1, ItemMoveData.SavedOrientation});

	return FItemAddResult::AddedAll(1, true, FText::Format(
		                                FText::FromString("Successfully added {0} to inventory as a reference"),
		                                FText::FromName(ItemMoveData.SourceItem->GetItemID())), AffectedSlots);
}

FItemAddResult UListInventory::HandleNonStackableItems(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
	int32 ActualAmountToAdd = CalculateActualAmountToAdd(1, ItemMoveData.SourceItem->GetItemSingleWeight());

	if (ActualAmountToAdd <= 0)
	{
		return FItemAddResult::AddedNone(FText::Format(
				FText::FromString("Item {0} would overflow limits"),
				ItemMoveData.SourceItem->GetItemRef().ItemTextData.DisplayName));
	}
	
	if (!bOnlyCheck)
	{
		UListInventorySlotWidget* ListInventorySlot = NewObject<UListInventorySlotWidget>();
		FItemMapping Slots;
		Slots.InventoryID = InventoryContainerID;
		Slots.OccupiedSlots.Add(ListInventorySlot->GetSlotData());
		AddNewItem(ItemMoveData, Slots, 1);
	}

	TMap<UInventorySlotData*, FItemPlacementData> AffectedSlots;
	if (ItemMoveData.TargetSlot)
		AffectedSlots.Add(ItemMoveData.TargetSlot->GetSlotData(), {1, ItemMoveData.SavedOrientation});

	return FItemAddResult::AddedAll(1, false, FText::Format(
										FText::FromString("Successfully added {0} of {1} to inventory"),
										1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.DisplayName), AffectedSlots);
}

FItemAddResult UListInventory::TryAddStackableItem(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	TMap<UInventorySlotData*, FItemPlacementData> AffectedSlots;

	const int32 InitialRequestedAddAmount = ItemMoveData.SourceItem->GetQuantity();
	const int32 StackableAmountAdded = HandleStackableItems(ItemMoveData, InitialRequestedAddAmount, bOnlyCheck, AffectedSlots);
	
	if (StackableAmountAdded == InitialRequestedAddAmount)
	{
		return FItemAddResult::AddedAll(StackableAmountAdded, false, FText::Format(
											FText::FromString("Successfully added {0} of {1} to inventory"),
											InitialRequestedAddAmount,
											ItemMoveData.SourceItem->GetItemRef().ItemTextData.DisplayName), AffectedSlots);
	}
	else if (StackableAmountAdded < InitialRequestedAddAmount && StackableAmountAdded > 0)
	{
		return FItemAddResult::AddedPartial(StackableAmountAdded, false, FText::Format(
												FText::FromString(
													"Partial amount of {0} added to inventory. Number added: {1}"),
												ItemMoveData.SourceItem->GetItemRef().ItemTextData.DisplayName,
												StackableAmountAdded), AffectedSlots);
	}
	
	return FItemAddResult::AddedNone(FText::Format(FText::FromString("Couldn't add {0} to inventory."),
												   ItemMoveData.SourceItem->GetItemRef().ItemTextData.DisplayName));
}

int32 UListInventory::HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount, bool bOnlyCheck,
	TMap<UInventorySlotData*, FItemPlacementData>& AffectedPivotSlots)
{
	int32 AmountToDistribute = RequestedAddAmount;
	int32 TotalAddedAmount = 0;

	auto SameItems = ItemCollectionLinked->GetAllSameItemsInContainer(InventoryContainerID, ItemMoveData.SourceItem);
	if (SameItems.Num()> 0)
	{
		for (auto& SameItem : SameItems)
		{
			if(AmountToDistribute<=0)
				break;
			
			int32 ActualAmountToAdd = TryInsertToStackItem(SameItem, AmountToDistribute, bOnlyCheck);
			AmountToDistribute -= ActualAmountToAdd;
			TotalAddedAmount += ActualAmountToAdd;
		}
	}

	if (AmountToDistribute<=0) return RequestedAddAmount;
	const int32 AmountToAddToStack = FMath::Min(AmountToDistribute, ItemMoveData.SourceItem->GetQuantity());
	int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack, ItemMoveData.SourceItem->GetItemSingleWeight());
		
	if (bOnlyCheck)
		return RequestedAddAmount;

	UListInventorySlotWidget* ListInventorySlot = NewObject<UListInventorySlotWidget>();	
	FItemMapping Slots;
	Slots.InventoryID = InventoryContainerID;
	Slots.OccupiedSlots.Add(ListInventorySlot->GetSlotData());
			
	AddNewItem(ItemMoveData, Slots, AmountToDistribute);
	return ActualAmountToAdd + TotalAddedAmount;
}

UItemBase* UListInventory::AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount)
{
	TObjectPtr<UItemBase> FinalItem;
	if (InventorySettings.bIsReferenceContainer)
	{
		FinalItem = ItemMoveData.SourceItem;
	}
	else
	{
		FinalItem = ItemMoveData.SourceItem->DuplicateItem();
		FinalItem->SetQuantity(AddAmount);
	}

	// Add item
	OccupiedSlots.InventoryID = InventoryContainerID;
	OccupiedSlots.bIsReferenceContainer = InventorySettings.bIsReferenceContainer;
	FItemMapping& StoredMapping = ItemCollectionLinked->AddItem(FinalItem, OccupiedSlots);

	NotifyAddNewItem(StoredMapping, FinalItem, ItemMoveData.SourceItem->GetQuantity());

	UpdateInvSlotsArray();

	UpdateMoneyInfo();
	UpdateWeightInfo();

	return FinalItem;
}

void UListInventory::UpdateInvSlotsArray()
{
	if (!ItemCollectionLinked)
		return;

	InvSlotsArray.Reset();

	auto AllItems = ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID);
	if (AllItems.IsEmpty())
		return;

	InvSlotsArray.Reserve(AllItems.Num());

	for (auto Item : AllItems)
	{
		UInventoryListEntry* EntryObject = NewObject<UInventoryListEntry>(this, EntryClass);
		EntryObject->Item = Item;
		InvSlotsArray.Add(EntryObject);
	}

	NotifyReDrawRequest();
}

//  Nublin Studio 2026 All Rights Reserved.

#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"

#include "ActorComponents/ItemCollection.h"
#include "Factory/ItemFactory.h"
#include "UI/Inventory/InventorySlot.h"

USlotbasedInventory::USlotbasedInventory()
{
}

void USlotbasedInventory::InitInventory(UItemCollection* ItemCollectionRef, FVector2D NewSize)
{
	if (!ItemCollectionRef)
		return;

	if (NewSize.X <= 0 || NewSize.Y <= 0)
		return;

	ItemCollectionLinked = ItemCollectionRef;
	InvSize = NewSize;

	bWasInit = true;
}

void USlotbasedInventory::SortItemsInContainerByName()
{
	if (!ItemCollectionLinked)
		return;
	
	MergeStackableItems();

	auto AllItems = ItemCollectionLinked->GetItemsWithMappingsByContainer(InventoryContainerID);
	
	TArray<TPair<UItemBase*, FItemMapping>> SortedItems;

	for (const auto& Pair : AllItems)
	{
		SortedItems.Add(Pair);
	}

	SortedItems.Sort([](const TPair<UItemBase*, FItemMapping>& A, const TPair<UItemBase*, FItemMapping>& B)
	{
		const FString NameA = A.Key->GetItemDisplayText().ToString();
		const FString NameB = B.Key->GetItemDisplayText().ToString();

		return NameA.Compare(NameB, ESearchCase::IgnoreCase) < 0;
	});

	for (auto& Pair : SortedItems)
	{
		Pair.Value.OccupiedSlots.Empty();
	}
		
	for (auto i =0; i < SortedItems.Num();  i++)
	{
		auto& Pair = SortedItems[i];
		auto AvSlot = GetAvailableSlotForItem(Pair.Key);
		if (!AvSlot.IsEmpty())
		{
			Pair.Value.OccupiedSlots = AvSlot;
		}
	}

	NotifyReDrawRequest();
	NotifyUpdateWeight();
	NotifyUpdateMoney();
}

TArray<UInventorySlotData*> USlotbasedInventory::GetSlotsForItemAt(const FIntPoint& StartPos, UItemBase* ItemBase,
                                                                   EItemOrientationType Orientation)
{
	TArray<TObjectPtr<UInventorySlotData>> Slots;

	if (InventorySettings.bIgnoreItemSize)
	{
		if (auto SlotForAdd = GetSlotByPosition(StartPos))
			Slots.Add(SlotForAdd);

		return Slots;
	}
	
	TArray<FIntPoint> Positions = GetItemGridPositions(
		StartPos, ItemBase->GetItemRef().ItemNumeraticData.InventoryHorizontalSlots,
		ItemBase->GetItemRef().ItemNumeraticData.InventoryVerticalSlots, Orientation);

	for (const FIntPoint& Pos : Positions)
	{
		auto SlotForAdd = GetSlotByPosition(Pos);
		if (SlotForAdd)
			Slots.Add(SlotForAdd);
	}

	return Slots;
}

bool USlotbasedInventory::ReserveSlots(AActor* Requestor, TMap<UInventorySlotData*, FItemPlacementData> Slots,
                                       UItemBase* ItemBase)
{
	if (!Requestor || Slots.IsEmpty()) return false;

	/*for (auto [Key, Value] : Slots)
	{
		FIntPoint Slot = FIntPoint(Key->CellPosition.X, Key->CellPosition.Y);
		if (!CanPlaceItemAt(Slot, Resource, Value.Orientation))
		{
			//return false;
		}
	}*/

	TArray<FSlotReservationData>& ActorReservations = ReservedSlotsToAdd.FindOrAdd(Requestor);

	for (auto& [Key, Value] : Slots)
	{
		FSlotReservationData Reservation;
		Reservation.Slot = Key;
		Reservation.ItemPlacementData = Value;
		Reservation.Resource = ItemBase;

		ActorReservations.Add(Reservation);
	}

	if (OnSlotsReservedDelegate.IsBound())
		OnSlotsReservedDelegate.Broadcast(ActorReservations);

	return true;
}

void USlotbasedInventory::ReleaseReservation(AActor* Requestor)
{
	if (!Requestor || ReservedSlotsToAdd.IsEmpty()) return;

	ReservedSlotsToAdd.Remove(Requestor);
}

bool USlotbasedInventory::ConsumeReserved(AActor* Requestor)
{
	TArray<UInventorySlotData*> TempSlots;
	if (!Requestor || ReservedSlotsToAdd.IsEmpty())
		return false;

	// TODO

	auto ActorReservations = ReservedSlotsToAdd.FindRef(Requestor);

	if (OnConsumeReservedDelegate.IsBound())
		OnConsumeReservedDelegate.Broadcast(ActorReservations);

	return true;
}

bool USlotbasedInventory::CanPlaceItemAt(const FIntPoint& StartPos, UItemBase* ItemBase, EItemOrientationType Orientation, bool bIgnoreOccupiedSlots)
{
	if (InventorySettings.bIgnoreItemSize)
	{
		auto CheckSlot = GetSlotByPosition(StartPos);

		return bIsSlotPositionValid(StartPos)
			&& (bIgnoreOccupiedSlots || bIsSlotEmptyByPos(StartPos))
			&& bIsItemCategoryCompatible(ItemBase->GetItemRef().ItemCategory, CheckSlot->AllowedCategory);
	}

	TArray<FIntPoint> Positions = GetItemGridPositions(
		StartPos,
		ItemBase->GetItemRef().ItemNumeraticData.InventoryHorizontalSlots,
		ItemBase->GetItemRef().ItemNumeraticData.InventoryVerticalSlots,
		Orientation);

	for (auto CheckPos : Positions)
	{
		auto CheckSlot = GetSlotByPosition(CheckPos);

		if (!bIsSlotPositionValid(CheckPos)
			|| (!bIgnoreOccupiedSlots && !bIsSlotEmptyByPos(CheckPos))
			|| !bIsItemCategoryCompatible(ItemBase->GetItemRef().ItemCategory, CheckSlot->AllowedCategory))
		{
			return false;
		}
	}

	return true;
}

TArray<UItemBase*> USlotbasedInventory::GetAllItems()
{
	TArray<UItemBase*> Instances;

	if (!ItemCollectionLinked)
		return Instances;

	return ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID);
}

TArray<FItemIDEntry> USlotbasedInventory::CollectItemsAggregated() const
{
	TArray<FItemIDEntry> Result;

	if (!ItemCollectionLinked)
		return Result;

	TArray<UItemBase*> Instances =
		ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID);

	TMap<UItemBase*, int32> AggregatedMap;

	for (auto Instance : Instances)
	{
		if (!Instance)
			continue;

		FName ItemID = Instance->GetItemID();
		int32 Count = Instance->GetQuantity();

		int32& StoredCount = AggregatedMap.FindOrAdd(Instance);
		StoredCount += Count;
	}

	for (const auto& Pair : AggregatedMap)
	{
		FItemIDEntry Entry;
		Entry.ItemID = Pair.Key->GetItemID();
		Entry.Amount = Pair.Value;

		Result.Add(Entry);
	}

	return Result;
}

TArray<FIntPoint> USlotbasedInventory::GetItemGridPositions(const FIntPoint& StartPos, int32 Width, int32 Height,
                                                            EItemOrientationType Orientation)
{
	TArray<FIntPoint> Positions;
	Positions.Reserve(Width * Height);

	for (int w = 0; w < Width; w++)
	{
		for (int h = 0; h < Height; h++)
		{
			if (Orientation == EItemOrientationType::Horizontal)
			{
				Positions.Add(FIntPoint(StartPos.X + w, StartPos.Y + h));
			}
			else
			{
				Positions.Add(FIntPoint(StartPos.X + h, StartPos.Y + w));
			}
		}
	}

	return Positions;
}

TArray<UInventorySlotData*> USlotbasedInventory::GetAvailableSlotForItem(UItemBase* Item)
{
	for (int32 i = 0; i <= InvSize.X; i++)
	{
		for (int32 j = 0; j <= InvSize.Y; j++)
		{
			FIntPoint StartPos(i, j);
			if (CanPlaceItemAt(StartPos, Item, EItemOrientationType::Horizontal))
				return GetSlotsForItemAt(StartPos, Item, EItemOrientationType::Horizontal);
			if (CanPlaceItemAt(StartPos, Item, EItemOrientationType::Vertical))
			{
				return GetSlotsForItemAt(StartPos, Item, EItemOrientationType::Vertical);
			}
		}
	}

	return {};
}

void USlotbasedInventory::HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity)
{
	if (!Item) return;

	auto removedActual = TryRemoveFromStackItem(Item, RemoveQuantity);
}

FItemAddResult USlotbasedInventory::HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
	if (!ItemMoveData.SourceItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item is null. Nothing to add"));
		return FItemAddResult::AddedNone(FText::FromString("Item is null. Nothing to add"));
	}

	if (ItemMoveData.SourceInventory && ItemMoveData.SourceItem->GetQuantity() <= 0)
		UE_LOG(LogTemp, Warning, TEXT("item Quantity is %i"), ItemMoveData.SourceItem->GetQuantity());

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
				ItemMoveData.SourceItem->GetItemRef().ItemTextData.NameID
			)
		);
	}

	if (ItemMoveData.SourceInventory == this && ItemCollectionLinked->ItemHasInventory(
			ItemMoveData.SourceItem, InventoryContainerID))
	{
		return TryReplaceItems(ItemMoveData, bOnlyCheck);
	}

	if (InventorySettings.bIsReferenceContainer)
		return HandleAddReferenceItem(ItemMoveData, bOnlyCheck);

	if (ItemMoveData.SourceInventory
		&& ItemMoveData.SourceInventory->GetInventorySettings().bIsReferenceContainer
		&& ItemMoveData.SourceInventory != ItemMoveData.TargetInventory
		&& ItemMoveData.TargetSlot)
	{
		if (bIsSlotEmpty(ItemMoveData.TargetSlot->GetSlotData()))
		{
			if (auto Mapping = ItemCollectionLinked->FindItemMappingByContainerName(
				ItemMoveData.SourceItem, InventoryContainerID))
			{
				// TODO
				return FItemAddResult::AddedNone(FText::FromString("Cant add Item by References"));
			}
		}

		return FItemAddResult::AddedNone(FText::FromString("Cant add Item by References"));
	}

	// non-stack
	if (!ItemMoveData.SourceItem->IsStackable())
	{
		// will the item weight overflow the weight capacity?
		if (InventorySettings.InventoryMaxWeightCapacity >= 0)
		{
			if (InventoryTotalWeight + ItemMoveData.SourceItem->GetItemSingleWeight() > InventorySettings.
				InventoryMaxWeightCapacity)
			{
				return FItemAddResult::AddedNone(FText::Format(
					FText::FromString("Item {0} would overflow weight limit"),
					ItemMoveData.SourceItem->GetItemRef().ItemTextData.NameID));
			}
		}

		return HandleNonStackableItems(ItemMoveData, bOnlyCheck);
	}

	if (ItemMoveData.SourceItem->IsStackable())
	{
		return TryAddStackableItem(ItemMoveData, bOnlyCheck);
	}

	return FItemAddResult::AddedNone(FText::FromString("Item is null. Nothing to add"));
}

FItemAddResult USlotbasedInventory::HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	if (ItemMoveData.TargetSlot == nullptr)
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
		                                               1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.NameID));

	auto TargetSlots = GetSlotsForItemAt(ItemMoveData.TargetSlot->GetSlotPosition(), ItemMoveData.SourceItem, ItemMoveData.TargetOrientation);
	
	if (AreSlotsEmpty(TargetSlots))
	{
		bool IsCanPlace = CanPlaceItemAt(ItemMoveData.TargetSlot->GetSlotPosition(), ItemMoveData.SourceItem, ItemMoveData.TargetOrientation);
		if (!IsCanPlace)
		{
			FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
													   1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.NameID));
		}
		
		FItemMapping Slots;
		Slots.InventoryID = InventoryContainerID;
		Slots.OccupiedSlots = TargetSlots;
		if (!bOnlyCheck)
			AddNewItem(ItemMoveData, Slots, ItemMoveData.SourceItem->GetQuantity());

		TMap<UInventorySlotData*, FItemPlacementData> AffectedSlots;
		AffectedSlots.Add(Slots.OccupiedSlots[0], {1, ItemMoveData.TargetOrientation});

		return FItemAddResult::AddedAll(1, true, FText::Format(
			                                FText::FromString("Successfully added {0} to inventory as a reference"),
			                                ItemMoveData.SourceItem->GetItemRef().ItemTextData.NameID), AffectedSlots);
	}

	
	auto ItemInTarSlot = ItemCollectionLinked->GetItemFromSlot(ItemMoveData.TargetSlot->GetSlotData(),
		                                                     InventoryContainerID);
	auto ItemInTarSlotOccSlots = ItemCollectionLinked->FindItemMappingByContainerName(ItemInTarSlot, InventoryContainerID)->OccupiedSlots;
	auto IsSlotsMatch = DoSlotsMatch(TargetSlots, ItemInTarSlotOccSlots);
	auto ItemsHaveSameFootprint = UItemBase::DoItemsHaveSameFootprint(ItemMoveData.SourceItem, ItemInTarSlot);
	bool IsCanPlace = CanPlaceItemAt(ItemMoveData.TargetSlot->GetSlotPosition(), ItemMoveData.SourceItem, ItemMoveData.TargetOrientation, true);
		
	if (ItemsHaveSameFootprint && IsSlotsMatch && IsCanPlace)
	{
		FItemMapping Slots;
		Slots.InventoryID = InventoryContainerID;
		Slots.OccupiedSlots = ItemInTarSlotOccSlots;
		
		if (!bOnlyCheck)
		{
			RemoveItemFromInventory(ItemInTarSlot);
			AddNewItem(ItemMoveData, Slots, ItemMoveData.SourceItem->GetQuantity());

		}
		
		TMap<UInventorySlotData*, FItemPlacementData> AffectedPivotSlots;
		AffectedPivotSlots.Add(ItemInTarSlotOccSlots[0], {1, ItemMoveData.TargetOrientation});	
		return FItemAddResult::AddedAll(1, true, FText::Format(
											FText::FromString("Successfully added {0} to inventory as a reference"),
											ItemMoveData.SourceItem->GetItemRef().ItemTextData.NameID), AffectedPivotSlots);
	}

	return FItemAddResult::AddedNone(FText::FromString(""));
}

FItemAddResult USlotbasedInventory::HandleNonStackableItems(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
	TArray<TObjectPtr<UInventorySlotData>> EmptySlots;
	if (!ItemMoveData.TargetSlot)
	{
		EmptySlots = GetAvailableSlotForItem(ItemMoveData.SourceItem);
	}
	else
	{
		EmptySlots.Add(ItemMoveData.TargetSlot->GetSlotData());
	}

	if (EmptySlots.IsEmpty())
	{
		FTextFormat Pattern = FTextFormat::FromString(
			TEXT("Can't be added {0} of {1} to inventory. No empty slots"));

		FText CountText = FText::AsNumber(1);
		FText NameText = FText::FromString(ItemMoveData.SourceItem->GetName());
		FText Msg = FText::Format(Pattern, CountText, NameText);

		return FItemAddResult::AddedNone(Msg);
	}

	for (auto InventorySlotData : EmptySlots)
	{
		if (!InventorySlotData)
		{
			UE_LOG(LogTemp, Warning, TEXT("InventorySlotData Error"));
		}
	}

	FIntPoint StartPos(EmptySlots[0]->CellPosition.X, EmptySlots[0]->CellPosition.Y);
	bool IsCanPlace = CanPlaceItemAt(StartPos, ItemMoveData.SourceItem, ItemMoveData.TargetOrientation);

	int32 ActualAmountToAdd = CalculateActualAmountToAdd(1, ItemMoveData.SourceItem->GetItemSingleWeight());
	if (ActualAmountToAdd <= 0)
	{
		return FItemAddResult::AddedNone(FText::Format(
				FText::FromString("Item {0} would overflow limits"),
				ItemMoveData.SourceItem->GetItemRef().ItemTextData.DisplayName));
	}

	if (IsCanPlace && !bOnlyCheck)
	{
		FItemMapping Slots;
		Slots.InventoryID = InventoryContainerID;
		Slots.OccupiedSlots = EmptySlots;
		DeductResourceOnAddToInventory(ItemMoveData.SourceItem, 1);
		AddNewItem(ItemMoveData, Slots, 1);
	}

	TMap<UInventorySlotData*, FItemPlacementData> AffectedPivotSlots;
	AffectedPivotSlots.Add(EmptySlots[0], {1, ItemMoveData.TargetOrientation});
	return FItemAddResult::AddedAll(1, false,
	                                FText::FromString("Successfully added to inventory"), AffectedPivotSlots);
}

FItemAddResult USlotbasedInventory::TryAddStackableItem(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	TMap<UInventorySlotData*, FItemPlacementData> AffectedPivotSlots;
	const int32 InitialRequestedAddAmount = ItemMoveData.SourceItem->GetQuantity();
	const int32 StackableAmountAdded = HandleStackableItems(ItemMoveData, InitialRequestedAddAmount, bOnlyCheck,
	                                                        AffectedPivotSlots);

	if (StackableAmountAdded == InitialRequestedAddAmount)
	{
		return FItemAddResult::AddedAll(
			StackableAmountAdded,
			false,
			FText::Format(
				FText::FromString("Successfully added {0} of {1} to inventory"),
				FText::AsNumber(InitialRequestedAddAmount),
				FText::FromString(ItemMoveData.SourceItem->GetItemID().ToString())
			),
			AffectedPivotSlots);
	}
	else if (StackableAmountAdded < InitialRequestedAddAmount && StackableAmountAdded > 0)
	{
		return FItemAddResult::AddedPartial(StackableAmountAdded, false, FText::Format(
			                                    FText::FromString(
				                                    "Partial amount of {0} added to inventory. Number added: {1}"),
			                                    FText::FromString(ItemMoveData.SourceItem->GetItemID().ToString()),
			                                    StackableAmountAdded),
		                                    AffectedPivotSlots);
	}

	return FItemAddResult::AddedNone(FText::Format(FText::FromString("Couldn't add {0} to inventory."),
	                                               FText::FromString(ItemMoveData.SourceItem->GetItemID().ToString())));
}

int32 USlotbasedInventory::HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount, bool bOnlyCheck,
                                                TMap<UInventorySlotData*, FItemPlacementData>& AffectedPivotSlots)
{
	int32 AmountToDistribute = RequestedAddAmount;
	int32 TotalAddedAmount = 0;

	if (!ItemMoveData.TargetSlot)
	{
		while (AmountToDistribute > 0)
		{
			auto Sameitems = GetAllSameItems(ItemMoveData.SourceItem);
			if (Sameitems.Num() > 0)
			{
				TotalAddedAmount += DistributeToExistingStacks(Sameitems, AmountToDistribute, ItemMoveData.SourceItem,
				                                               bOnlyCheck, AffectedPivotSlots);
			}

			if (AmountToDistribute <= 0) return RequestedAddAmount;

			auto EmptySlots = GetAvailableSlotForItem(ItemMoveData.SourceItem);
			if (EmptySlots.IsEmpty())
				break;

			AffectedPivotSlots.Add(EmptySlots[0], {AmountToDistribute,ItemMoveData.TargetOrientation});
			if (!bOnlyCheck)
			{
				FItemMapping Slots;
				Slots.InventoryID = InventoryContainerID;
				Slots.OccupiedSlots = EmptySlots;
				auto NewItem = AddNewItem(ItemMoveData, Slots, AmountToDistribute);
			}

			TotalAddedAmount += AmountToDistribute;
			AmountToDistribute = 0;
		}

		return TotalAddedAmount;
	}

	if (bIsSlotEmpty(ItemMoveData.TargetSlot->GetSlotData()))
	{
		FIntPoint Slot = FIntPoint(ItemMoveData.TargetSlot->GetSlotPosition());
		if (CanPlaceItemAt(Slot, ItemMoveData.SourceItem, ItemMoveData.TargetOrientation))
		{
			auto EmptySlots = GetSlotsForItemAt(Slot, ItemMoveData.SourceItem,
			                                   ItemMoveData.TargetOrientation);

			if (!bOnlyCheck)
			{
				FItemMapping Slots;
				Slots.InventoryID = InventoryContainerID;
				Slots.OccupiedSlots = EmptySlots;
				auto NewItem = AddNewItem(ItemMoveData, Slots, 1);
				AmountToDistribute = AmountToDistribute - 1;
				TotalAddedAmount = TotalAddedAmount + 1;
				DeductResourceOnAddToInventory(ItemMoveData.SourceItem, 1);

				int32 ActualAmountToAdd = TryInsertToStackItem(NewItem, ItemMoveData.SourceItem, AmountToDistribute,
				                                               bOnlyCheck);
				AmountToDistribute -= ActualAmountToAdd;
				TotalAddedAmount += ActualAmountToAdd;

				return TotalAddedAmount;
			}
		}
	}
	else
	{
		auto Item = GetItemFromSlot(ItemMoveData.TargetSlot->GetSlotData());
		if (ItemMoveData.SourceItem->GetItemID().Compare(Item->GetItemID()))
		{
			int32 ActualAmountToAdd = TryInsertToStackItem(Item, ItemMoveData.SourceItem, AmountToDistribute,
			                                               bOnlyCheck);

			AmountToDistribute -= ActualAmountToAdd;
			TotalAddedAmount += ActualAmountToAdd;
			return TotalAddedAmount;
		}
		else
		{
			return TotalAddedAmount;
		}
	}

	return TotalAddedAmount;
}

FItemAddResult USlotbasedInventory::TryReplaceItems(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	auto TargetSlots = GetSlotsForItemAt(ItemMoveData.TargetSlot->GetSlotPosition(), ItemMoveData.SourceItem, ItemMoveData.SavedOrientation);
	bool IsUseRefs = InventorySettings.bIsReferenceContainer;
	
	if (AreSlotsEmpty(TargetSlots))
	{
		bool IsCanPlace = CanPlaceItemAt(ItemMoveData.TargetSlot->GetSlotPosition(), ItemMoveData.SourceItem, ItemMoveData.SavedOrientation);
		if (!IsCanPlace)
		{
			FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.NameID));
		}
			
		if (!bOnlyCheck)
			ReplaceItem(ItemMoveData.SourceItem, TargetSlots);
		return FItemAddResult::Swapped(0, IsUseRefs, FText::FromString("Item successfully moved to an empty slot."));
	}

	auto ItemInTarSlot = ItemCollectionLinked->GetItemFromSlot(ItemMoveData.TargetSlot->GetSlotData(),															 InventoryContainerID);
	auto ItemInTarSlotOccSlots = ItemCollectionLinked->FindItemMappingByContainerName(ItemInTarSlot, InventoryContainerID)->OccupiedSlots;
		
	auto IsSlotsMatch = DoSlotsMatch(TargetSlots, ItemInTarSlotOccSlots);
	auto ItemsHaveSameFootprint = UItemBase::DoItemsHaveSameFootprint(ItemMoveData.SourceItem, ItemInTarSlot);
	bool IsCanPlace = CanPlaceItemAt(ItemMoveData.TargetSlot->GetSlotPosition(), ItemMoveData.SourceItem, ItemMoveData.SavedOrientation);
		
	if (ItemsHaveSameFootprint && IsSlotsMatch && IsCanPlace)
	{
		if (!bOnlyCheck)
		{
			ReplaceItem(ItemMoveData.SourceItem,TargetSlots);
			ReplaceItem(ItemInTarSlot, ItemInTarSlotOccSlots);
		}
		return FItemAddResult::Swapped(0, IsUseRefs, FText::FromString("Items successfully swapped."));
	}
	
	return FItemAddResult::AddedNone(FText::FromString(""));
		
}

int32 USlotbasedInventory::DistributeToExistingStacks(TArray<UItemBase*>& SameItems, int32& AmountToDistribute,
                                                      UItemBase* ResourceToDeductFrom,
                                                      bool bOnlyCheck,
                                                      TMap<UInventorySlotData*, FItemPlacementData>& AffectedSlots)
{
	int32 TotalAdded = 0;

	for (auto* Item : SameItems)
	{
		if (AmountToDistribute <= 0)
			break;

		int32 ActualAmountToAdd = TryInsertToStackItem(Item, ResourceToDeductFrom, AmountToDistribute, bOnlyCheck);
		if (ActualAmountToAdd > 0)
		{
			FItemMappingArrayWrapper Wrapper = ItemCollectionLinked->GetItemLocations().FindRef(
				TObjectPtr<UItemBase>(Item));
			auto Mapping = ItemCollectionLinked->FindItemMappingByContainerName(Item, InventoryContainerID);
			AffectedSlots.Add(Mapping->OccupiedSlots[0], {ActualAmountToAdd, EItemOrientationType::Horizontal});

			AmountToDistribute -= ActualAmountToAdd;
			TotalAdded += ActualAmountToAdd;
		}
	}

	return TotalAdded;
}

UItemBase* USlotbasedInventory::AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots,
                                           int32 AddAmount)
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
	FItemMappingArrayWrapper MapWrapper = ItemCollectionLinked->AddItem(FinalItem, OccupiedSlots);
	OccupiedSlots.InventoryID = InventoryContainerID;
	MapWrapper.Mappings.Add(OccupiedSlots);

	NotifyAddNewItem(OccupiedSlots, FinalItem, ItemMoveData.SourceItem->GetQuantity());

	return FinalItem;
}

void USlotbasedInventory::ReplaceItem(UItemBase* Item, const TArray<UInventorySlotData*>& NewSlotDatas)
{
	if (!ItemCollectionLinked)
	{
		return;
	}

	if (!Item || NewSlotDatas.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("USlotbasedInventory::ReplaceItem: Invalid parameters."));
		return;
	}

	FItemMapping* Mapping =
		ItemCollectionLinked->FindItemMappingByContainerName(Item, InventoryContainerID);
	if (!Mapping)
		return;

	TArray<TObjectPtr<UInventorySlotData>> OldSlots = Mapping->OccupiedSlots;

	Mapping->OccupiedSlots = NewSlotDatas;

	NotifyReplaceItem(OldSlots, *Mapping, Item);
}

int32 USlotbasedInventory::TryInsertToStackItem(UItemBase* ItemToInsertInto,
                                                UItemBase* ItemToDeductFrom, int32 AmountToDistribute,
                                                bool bOnlyCheck)
{
	if (ItemToInsertInto->IsFullItemStack())
		return 0;

	int32 AmountToAddToStack = FMath::Min(AmountToDistribute,
	                                      ItemToInsertInto->GetItemRef().ItemNumeraticData.MaxStackSizeInCharacter -
	                                      ItemToInsertInto
	                                      ->GetQuantity());
	int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack, ItemToInsertInto->GetItemSingleWeight());
	int32 OldAmount = ItemToInsertInto->GetQuantity();

	if (!bOnlyCheck)
	{
		DeductResourceOnAddToInventory(ItemToDeductFrom, ActualAmountToAdd);
		ItemToInsertInto->SetQuantity(OldAmount + ActualAmountToAdd);
		NotifyAddItemToStack(ItemToInsertInto, ActualAmountToAdd);
	}
	//ActualAmountToAdd = OldAmount + ActualAmountToAdd;

	return ActualAmountToAdd;
}

int32 USlotbasedInventory::TryRemoveFromStackItem(UItemBase* Item, int32 RequestedRemoveAmount)
{
	if (!Item || Item->GetQuantity() <= 0)
		return 0;

	int32 AmountToRemove = FMath::Min(RequestedRemoveAmount, Item->GetQuantity());
	Item->SetQuantity(Item->GetQuantity() - AmountToRemove);

	NotifyRemoveItemFromStack(Item, RequestedRemoveAmount);
	if (Item->GetQuantity() <= 0)
	{
		RemoveItemFromInventory(Item);
	}

	return AmountToRemove;
}

bool USlotbasedInventory::bIsSlotPositionValid(FIntPoint GridPosition)
{
	return GridPosition.X >= 0 && GridPosition.Y >= 0 && GridPosition.X < InvSize.X && GridPosition.Y < InvSize.Y;
}

bool USlotbasedInventory::bIsItemCategoryCompatible(EItemCategory ItemCategory, EItemCategory SlotCategory)
{
	if (SlotCategory == EItemCategory::All)
	{
		return true;
	}
	return ItemCategory == SlotCategory;
}

bool USlotbasedInventory::bIsSlotEmptyByPos(FIntPoint SlotPosition)
{
	auto BusySlots = CollectOccupiedSlots();
	for (const auto InvSlotData : BusySlots)
	{
		if (InvSlotData->CellPosition == SlotPosition)
			return false;
	}

	return true;
}

bool USlotbasedInventory::bIsSlotEmpty(UInventorySlotData* Slot)
{
	auto Posit = (Slot->CellPosition);
	return bIsSlotEmptyByPos(Posit);
}

bool USlotbasedInventory::AreSlotsEmpty(const TArray<UInventorySlotData*>& Slots)
{
	bool AllSlotsEmpty = true;
	for (const auto SlotData : Slots)
	{
		if (!bIsSlotEmpty(SlotData))
		{
			AllSlotsEmpty = false;
			break;
		}
	}

	return AllSlotsEmpty;
}

bool USlotbasedInventory::DoSlotsMatch(const TArray<UInventorySlotData*>& FirstSlots,
	const TArray<UInventorySlotData*>& SecondSlots)
{
	if (FirstSlots.IsEmpty() && SecondSlots.IsEmpty())
		return false;

	if (FirstSlots.Num() != SecondSlots.Num())
		return false;

	for (const auto SlotData : FirstSlots)
	{
		if (!SecondSlots.Contains(SlotData))
		{
			return false;
		}
	}
	
	return true;
}

float USlotbasedInventory::GetInventoryOccupancyPercent()
{
	float Percent = 0.0f;

	if (CollectOccupiedSlots().IsEmpty())
		return 0.0f;

	return (InvSize.X * InvSize.Y) / CollectOccupiedSlots().Num() * 100.0f;
}

TArray<UInventorySlotData*> USlotbasedInventory::CollectOccupiedSlots()
{
	TArray<TObjectPtr<UInventorySlotData>> OccupiedSlots;
	if (ItemCollectionLinked->GetItemLocations().IsEmpty())
		return OccupiedSlots;

	for (const auto Pair : ItemCollectionLinked->GetItemLocations())
	{
		for (const FItemMapping& Mapping : Pair.Value.Mappings)
		{
			if (Mapping.InventoryID != InventoryContainerID)
				continue;

			for (auto Slot : Mapping.OccupiedSlots)
				OccupiedSlots.AddUnique(Slot);
		}
	}

	return OccupiedSlots;
}

UInventorySlotData* USlotbasedInventory::GetSlotByPosition(FIntPoint SlotPosition)
{
	for (auto& Elem : InvSlotsDatas)
	{
		if (Elem->CellPosition == SlotPosition)
			return Elem;
	}

	return nullptr;
}

TArray<UItemBase*> USlotbasedInventory::GetAllSameItems(UItemBase* ReferenceItem)
{
	TArray<UItemBase*> SameItems;
	if (!ReferenceItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetAllSameItemsInContainer: %s"), TEXT("ReferenceItem is null."));
		return SameItems;
	}
	if (ItemCollectionLinked->GetItemLocations().IsEmpty())
		return SameItems;

	auto RefName = ReferenceItem->GetItemID();
	for (const auto& Pair : ItemCollectionLinked->GetItemLocations())
	{
		auto Item = Pair.Key;
		//if (Item && Item->GetItemID().Compare(RefName) && !Item->IsFullItemStackInStorage())
		if (Item && Item->GetItemID().Compare(RefName))
		{
			SameItems.AddUnique(Item.Get());
		}
	}

	return SameItems;
}

UItemBase* USlotbasedInventory::GetItemFromSlot(UInventorySlotData* Slot)
{
	if (ItemCollectionLinked->GetItemLocations().IsEmpty())
		return nullptr;

	for (const auto& Pair : ItemCollectionLinked->GetItemLocations())
	{
		for (auto Mapping : Pair.Value.Mappings)
		{
			for (auto MapSlot : Mapping.OccupiedSlots)
			{
				if (MapSlot->CellPosition == Slot->CellPosition)
					return Pair.Key.Get();
			}
		}
	}

	return nullptr;
}

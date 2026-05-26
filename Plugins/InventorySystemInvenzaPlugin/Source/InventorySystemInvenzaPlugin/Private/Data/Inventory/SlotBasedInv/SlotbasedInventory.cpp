//  Nublin Studio 2026 All Rights Reserved.

#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"

#include "ActorComponents/ItemCollection.h"
#include "Data/Settings/InvenzaInventoryUISettingsAsset.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "UI/Inventory/InventorySlot.h"

void USlotbasedInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USlotbasedInventory, InventorySlotData);
	DOREPLIFETIME(USlotbasedInventory, WidgetSlotInitData);
	DOREPLIFETIME(USlotbasedInventory, SlotSpacing);
	DOREPLIFETIME(USlotbasedInventory, InvCellSize);
}

bool USlotbasedInventory::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
	struct FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (UInventorySlotData* Slot : InventorySlotData)
	{
		if (IsValid(Slot))
		{
			bWroteSomething |= Channel->ReplicateSubobject(Slot, *Bunch, *RepFlags);
		}
	}

	return bWroteSomething;
}

void USlotbasedInventory::InitInventory()
{
	Super::InitInventory();

	if (!InventorySettings.bCollectInvDataFromWidget)
	{
		GenerateInventorySlots();
	}

	InvSize = FIntPoint(InventorySettings.InventorySlotBasedSettings.InitNumberRows, InventorySettings.InventorySlotBasedSettings.InitNumColumns);
	InvCellSize = InventorySettings.InventorySlotBasedSettings.InvCellSize;

	if (!InventoryOwnerActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("SlotbasedInventory [%s]: InventoryOwnerActor is NULL!"), *GetName());
	}
	
	bWasInit = true;
}

void USlotbasedInventory::RebuildInventory()
{
	GenerateInventorySlots();
}

void USlotbasedInventory::SortItemsInContainerByName()
{
	if (!ItemCollectionLinked)
		return;

	MergeStackableItems();

	auto AllItems =ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID);
	TArray<UItemBase*> SortedItems;
	SortedItems.Reserve(AllItems.Num());
	for (UItemBase* Item : AllItems)
	{
		UItemBase* NewItem = Item->DuplicateItem();
		SortedItems.Add(NewItem);
	}
	for (UItemBase* Item : AllItems)
	{
		HandleRemoveItem(Item, Item->GetQuantity());
	}

	SortedItems.Sort([](const UItemBase& A, const UItemBase& B)
	{
		const FString NameA = A.GetItemDisplayText().ToString();
		const FString NameB = B.GetItemDisplayText().ToString();

		return NameA.Compare(NameB, ESearchCase::IgnoreCase) < 0;
	});

	for (auto i = 0; i < SortedItems.Num(); i++)
	{
		FItemMoveData ItemMoveData;
		ItemMoveData.TargetInventory = this;
		ItemMoveData.SourceItem = SortedItems[i];
		const EItemOrientationType InitOrientation = ItemMoveData.SourceItem->GetInitialItemOrientation();

		ItemMoveData.SavedOrientation = InitOrientation;
		ItemMoveData.TargetOrientation = InitOrientation;
		HandleAddItem(ItemMoveData);
	}

	
	NotifyReDrawRequest();
	OnRep_InventoryTotalWeight();
	OnRep_InventoryTotalMoney();
}

TArray<UInventorySlotData*> USlotbasedInventory::GetSlotsForItemAt(const FIntPoint& StartPos, UItemBase* ItemBase,
                                                                   EItemOrientationType Orientation)
{
	TArray<TObjectPtr<UInventorySlotData>> Slots;

	if (InventorySettings.InventorySlotBasedSettings.bIgnoreItemSize)
	{
		if (auto SlotForAdd = GetSlotByPosition(StartPos))
			Slots.Add(SlotForAdd);

		return Slots;
	}

	TArray<FIntPoint> Positions = GetItemGridPositions(
		StartPos, ItemBase->GetItemSize(Orientation));

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

float USlotbasedInventory::GetInventoryOccupancyPercent()
{
	if (CollectOccupiedSlots().IsEmpty())
		return 0.0f;

	return (InvSize.X * InvSize.Y) / CollectOccupiedSlots().Num() * 100.0f;
}

bool USlotbasedInventory::CanPlaceItemAt(const FIntPoint& StartPos, UItemBase* ItemBase,
                                         EItemOrientationType Orientation, TArray<UInventorySlotData*> IgnoreSlots)
{
	if (!ItemBase) return false;

	if (InventorySettings.InventorySlotBasedSettings.bIgnoreItemSize)
	{
		auto CheckSlot = GetSlotByPosition(StartPos);
		if (!CheckSlot) return false;

		return bIsSlotPositionValid(StartPos)
			&& bIsSlotEmptyByPos(StartPos, IgnoreSlots)
			&& BIsItemCategoryCompatible(ItemBase->GetItemRef().ItemCategory,
				CheckSlot->InventorySlotInfo.AllowedCategory);
	}

	TArray<FIntPoint> Positions = GetItemGridPositions(StartPos, ItemBase->GetItemSize(Orientation));
	for (const FIntPoint& CheckPos : Positions)
	{
		if (!bIsSlotPositionValid(CheckPos)) return false;

		auto CheckSlot = GetSlotByPosition(CheckPos);
		if (!CheckSlot) return false;

		if (!bIsSlotEmptyByPos(CheckPos, IgnoreSlots)) return false;
		if (!BIsItemCategoryCompatible(ItemBase->GetItemRef().ItemCategory,
			CheckSlot->InventorySlotInfo.AllowedCategory)) return false;
	}

	return true;
}

void USlotbasedInventory::RequestSplitStack(UItemBase* ItemToSplit, int32 SplitAmount)
{
	if (!ItemToSplit || SplitAmount <= 0)
		return;

	if (ItemToSplit->GetQuantity() == 1 || ItemToSplit->GetQuantity() <= SplitAmount)
		return;

	EItemOrientationType FinalOrientation;
	auto EmptySlots = GetAvailableSlotForItem(ItemToSplit, FinalOrientation);
	if (EmptySlots.IsEmpty())
		return;

	if (InventorySettings.MaxStackCount > 0)
	{
		auto ResultMaxStack = ItemCollectionLinked->GetStackCountInContainer(InventoryContainerID);
		if (ResultMaxStack + 1 >InventorySettings.MaxStackCount)
			return;
	}
	
	auto NewItem = ItemToSplit->DuplicateItem();
	if (!NewItem) return;

	NewItem->SetQuantity(SplitAmount);
	
	FItemMoveData ItemMove;
	ItemMove.SourceItem = NewItem;
	ItemMove.TargetInventory = this;
	ItemMove.TargetSlotCoordinate = EmptySlots[0]->InventorySlotInfo.CellPosition;
	ItemMove.SavedOrientation = FinalOrientation;
	ItemMove.TargetOrientation = FinalOrientation;
		
	OnSplitDelegate.Broadcast(this,ItemToSplit, SplitAmount);
	
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

TArray<FIntPoint> USlotbasedInventory::GetItemGridPositions(const FIntPoint& StartPos, FIntPoint Size)
{
	TArray<FIntPoint> Positions;
	Positions.Reserve(Size.X * Size.Y);

	for (int Row = 0; Row < Size.X; Row++)
	{
		for (int Col = 0; Col < Size.Y; Col++)
		{
			Positions.Add(FIntPoint(StartPos.X + Col, StartPos.Y + Row));
		}
	}

	return Positions;
}

TArray<UInventorySlotData*> USlotbasedInventory::GetAvailableSlotForItem(
	UItemBase* Item, EItemOrientationType& OutOrientation)
{
	const bool bPreferVertical = Item->GetInitialItemOrientation() == EItemOrientationType::Vertical;
	const EItemOrientationType First = bPreferVertical
		                                   ? EItemOrientationType::Vertical
		                                   : EItemOrientationType::Horizontal;
	const EItemOrientationType Second = bPreferVertical
		                                    ? EItemOrientationType::Horizontal
		                                    : EItemOrientationType::Vertical;

	for (int32 i = 0; i < InvSize.X; i++)
	{
		for (int32 j = 0; j < InvSize.Y; j++)
		{
			FIntPoint StartPos(i, j);

			if (CanPlaceItemAt(StartPos, Item, First, TArray<UInventorySlotData*>()))
			{
				OutOrientation = First;
				return GetSlotsForItemAt(StartPos, Item, First);
			}
			if (CanPlaceItemAt(StartPos, Item, Second, TArray<UInventorySlotData*>()))
			{
				OutOrientation = Second;
				return GetSlotsForItemAt(StartPos, Item, Second);
			}
		}
	}

	OutOrientation = EItemOrientationType::Horizontal;
	return {};
}

void USlotbasedInventory::SetWidgetInitData(FSlotBasedInventoryWidgetInitData WidgetInitData)
{
	SetInventorySize(WidgetInitData.InventorySize);
	WidgetSlotInitData = WidgetInitData.SlotLayout;
	SlotSpacing = WidgetInitData.SlotSpacing;
	InvCellSize = WidgetInitData.InvCellSize;
}

void USlotbasedInventory::HandleRemoveItemsByType(UItemBase* ItemSample, int32 RequestedAmount)
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

void USlotbasedInventory::HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity)
{
	if (!Item)
		return;

	if (Item->GetQuantity() <= 0)
	{
		RemoveItemFromInventory(Item);
		return;
	}

	TryRemoveFromStackItem(Item, RemoveQuantity);
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
				FText::FromName(ItemMoveData.SourceItem->GetItemID())));
	}

	if (bool bIsItemExist = ItemCollectionLinked->ItemHasInventory(ItemMoveData.SourceItem, InventoryContainerID))
	{
		if (ItemMoveData.SourceInventory == this)
		{
			TArray<UInventorySlotData*> SlotsToIgnore;
		
			if (ItemMoveData.SourceItem->IsStackable() && !bIsSlotEmpty(GetSlotByPosition(ItemMoveData.TargetSlotCoordinate), SlotsToIgnore))
			{
				return TryAddStackableItem(ItemMoveData, bOnlyCheck);
			}
		
			return TryReplaceItems(ItemMoveData, bOnlyCheck);
		}
		else
		{
			return TryReplaceItems(ItemMoveData, bOnlyCheck);
		}
	}

	if (InventorySettings.bIsReferenceContainer)
		return HandleAddReferenceItem(ItemMoveData, bOnlyCheck);

	if (ItemMoveData.SourceInventory
		&& ItemMoveData.SourceInventory->GetInventorySettings().bIsReferenceContainer
		&& ItemMoveData.SourceInventory != ItemMoveData.TargetInventory
		&& ItemMoveData.TargetSlotCoordinate != FIntPoint(-1))
	{
		TArray<UInventorySlotData*> IgnoreSlots = GetIgnoreSlotsForItem(ItemMoveData.SourceItem);
		if (bIsSlotEmpty(GetSlotByPosition(ItemMoveData.TargetSlotCoordinate), IgnoreSlots))
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
					FText::FromName(ItemMoveData.SourceItem->GetItemID())));
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
	if (ItemMoveData.TargetSlotCoordinate == FIntPoint(-1))
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
		                                               1, FText::FromName(ItemMoveData.SourceItem->GetItemID())));

	auto TargetSlots = GetSlotsForItemAt(ItemMoveData.TargetSlotCoordinate, ItemMoveData.SourceItem,
	                                     ItemMoveData.TargetOrientation);
	TArray<UInventorySlotData*> IgnoreSlots = GetIgnoreSlotsForItem(ItemMoveData.SourceItem);
	if (AreSlotsEmpty(TargetSlots, IgnoreSlots))
	{
		bool IsCanPlace = CanPlaceItemAt(ItemMoveData.TargetSlotCoordinate, ItemMoveData.SourceItem,
		                                 ItemMoveData.TargetOrientation,IgnoreSlots);
		if (!IsCanPlace)
		{
			FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
			                                        1, FText::FromName(ItemMoveData.SourceItem->GetItemID())));
		}

		FItemMapping Slots;
		Slots.InventoryID = InventoryContainerID;
		Slots.OccupiedSlots = TargetSlots;
		Slots.ItemOrientation = ItemMoveData.TargetOrientation;
		if (!bOnlyCheck)
			AddNewItem(ItemMoveData, Slots, ItemMoveData.SourceItem->GetQuantity());

		TMap<UInventorySlotData*, FItemPlacementData> AffectedSlots;
		AffectedSlots.Add(Slots.OccupiedSlots[0], {1, ItemMoveData.TargetOrientation});

		return FItemAddResult::AddedAll(1, true, FText::Format(
			                                FText::FromString("Successfully added {0} to inventory as a reference"),
			                                FText::FromName(ItemMoveData.SourceItem->GetItemID())), AffectedSlots);
	}
	
	auto ItemInTarSlot = ItemCollectionLinked->GetItemFromSlot(GetSlotByPosition(ItemMoveData.TargetSlotCoordinate), InventoryContainerID);
	auto ItemInTarMapping = ItemCollectionLinked->FindItemMappingByContainerName(ItemInTarSlot, InventoryContainerID);        
	auto ItemInTarSlotOccSlots = ItemCollectionLinked->FindItemMappingByContainerName(
		ItemInTarSlot, InventoryContainerID)->OccupiedSlots;
	for (auto InTarSlot : ItemInTarSlotOccSlots)
	{
		IgnoreSlots.Add(InTarSlot);
	}
	
	bool ItemsHaveSameFootprint = true;
	if (!InventorySettings.InventorySlotBasedSettings.bIgnoreItemSize)
	{
		ItemsHaveSameFootprint = UItemBase::DoItemsHaveSameFootprint(ItemMoveData.SourceItem, ItemInTarSlot,
		ItemMoveData.SavedOrientation, ItemInTarMapping->ItemOrientation, InventorySettings.InventorySlotBasedSettings.bIgnoreItemSize);
	}
	bool IsCanPlace = CanPlaceItemAt(ItemMoveData.TargetSlotCoordinate, ItemMoveData.SourceItem,
	                                 ItemMoveData.TargetOrientation, IgnoreSlots);

	if (ItemsHaveSameFootprint && IsCanPlace)
	{
		FItemMapping Slots;
		Slots.InventoryID = InventoryContainerID;
		Slots.OccupiedSlots = ItemInTarSlotOccSlots;
		Slots.ItemOrientation = ItemMoveData.TargetOrientation;

		if (!bOnlyCheck)
		{
			RemoveItemFromInventory(ItemInTarSlot);
			AddNewItem(ItemMoveData, Slots, ItemMoveData.SourceItem->GetQuantity());
		}

		TMap<UInventorySlotData*, FItemPlacementData> AffectedPivotSlots;
		AffectedPivotSlots.Add(ItemInTarSlotOccSlots[0], {1, ItemMoveData.TargetOrientation});
		return FItemAddResult::AddedAll(1, true, FText::Format(
			                                FText::FromString("Successfully added {0} to inventory as a reference"),
			                                FText::FromName(ItemMoveData.SourceItem->GetItemID())), AffectedPivotSlots);
	}

	return FItemAddResult::AddedNone(FText::FromString(""));
}

FItemAddResult USlotbasedInventory::HandleNonStackableItems(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
	TArray<TObjectPtr<UInventorySlotData>> EmptySlots;
	EItemOrientationType FinalOrientation = ItemMoveData.TargetOrientation;

	if (!bIsSlotPositionValid(ItemMoveData.TargetSlotCoordinate))
	{
		EmptySlots = GetAvailableSlotForItem(ItemMoveData.SourceItem, FinalOrientation);
	}
	else
	{
		FIntPoint StartPos(ItemMoveData.TargetSlotCoordinate);
		if (!CanPlaceItemAt(StartPos, ItemMoveData.SourceItem, FinalOrientation, TArray<UInventorySlotData*>()))
			return FItemAddResult::AddedNone(FText::FromString("Can't place item at target slot"));

		EmptySlots = GetSlotsForItemAt(StartPos, ItemMoveData.SourceItem, FinalOrientation);
	}

	if (EmptySlots.IsEmpty())
	{
		return FItemAddResult::AddedNone(FText::Format(
			FText::FromString("Can't be added {0} of {1} to inventory. No empty slots"),
			FText::AsNumber(1),
			FText::FromString(ItemMoveData.SourceItem->GetName())));
	}

	if (EmptySlots.ContainsByPredicate([](const TObjectPtr<UInventorySlotData>& S) { return !S; }))
	{
		return FItemAddResult::AddedNone(FText::FromString("Invalid slot data"));
	}

	if (CalculateActualAmountToAdd(1, ItemMoveData.SourceItem->GetItemSingleWeight()) <= 0)
	{
		return FItemAddResult::AddedNone(FText::Format(
			FText::FromString("Item {0} would overflow limits"),
			ItemMoveData.SourceItem->GetItemRef().ItemTextData.DisplayName));
	}

	if (!bOnlyCheck)
	{
		FItemMapping Slots;
		Slots.InventoryID = InventoryContainerID;
		Slots.OccupiedSlots = EmptySlots;
		Slots.ItemOrientation = FinalOrientation;
		//DeductResourceOnAddToInventory(ItemMoveData.SourceItem, 1);
		AddNewItem(ItemMoveData, Slots, 1);
	}

	TMap<UInventorySlotData*, FItemPlacementData> AffectedPivotSlots;
	AffectedPivotSlots.Add(EmptySlots[0], {1, FinalOrientation});
	return FItemAddResult::AddedAll(1, false, FText::FromString("Successfully added to inventory"), AffectedPivotSlots);
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

	if (!bIsSlotPositionValid(ItemMoveData.TargetSlotCoordinate))
	{
		while (AmountToDistribute > 0)
		{
			auto Sameitems = GetAllSameItems(ItemMoveData.SourceItem);
			if (Sameitems.Num() > 0)
			{
				TotalAddedAmount += DistributeToExistingStacks(Sameitems, AmountToDistribute, bOnlyCheck, AffectedPivotSlots);
			}

			if (AmountToDistribute <= 0) return RequestedAddAmount;

			EItemOrientationType FinalOrientation;
			auto EmptySlots = GetAvailableSlotForItem(ItemMoveData.SourceItem, FinalOrientation);
			if (EmptySlots.IsEmpty())
				break;

			AffectedPivotSlots.Add(EmptySlots[0], {AmountToDistribute, FinalOrientation});
			if (!bOnlyCheck)
			{
				FItemMapping Slots;
				Slots.InventoryID = InventoryContainerID;
				Slots.OccupiedSlots = EmptySlots;
				Slots.ItemOrientation = FinalOrientation;
				auto NewItem = AddNewItem(ItemMoveData, Slots, AmountToDistribute);
			}

			TotalAddedAmount += AmountToDistribute;
			AmountToDistribute = 0;
		}

		return TotalAddedAmount;
	}

	TArray<UInventorySlotData*> IgnoreSlots = GetIgnoreSlotsForItem(ItemMoveData.SourceItem);
	if (bIsSlotEmpty(GetSlotByPosition(ItemMoveData.TargetSlotCoordinate), IgnoreSlots))
	{
		FIntPoint Slot = FIntPoint(ItemMoveData.TargetSlotCoordinate);
		if (CanPlaceItemAt(Slot, ItemMoveData.SourceItem, ItemMoveData.TargetOrientation, TArray<UInventorySlotData*>()))
		{
			auto EmptySlots = GetSlotsForItemAt(Slot, ItemMoveData.SourceItem,
			                                    ItemMoveData.TargetOrientation);


			if (bOnlyCheck)
			{
				UItemBase* ItemToTest = ItemMoveData.SourceItem->DuplicateItem();
				ItemToTest->SetQuantity(1);

				int32 StackAmount = TryInsertToStackItem(ItemToTest, AmountToDistribute - 1, true);
				TotalAddedAmount += 1 + StackAmount;
				AmountToDistribute -= 1 + StackAmount;
			}
			else
			{
				FItemMapping Slots;
				Slots.InventoryID = InventoryContainerID;
				Slots.OccupiedSlots = EmptySlots;
				Slots.ItemOrientation = ItemMoveData.TargetOrientation;
				auto NewItem = AddNewItem(ItemMoveData, Slots, 1);
				TotalAddedAmount += 1;
				AmountToDistribute -= 1;

				int32 ActualAmountToAdd = TryInsertToStackItem(NewItem, AmountToDistribute, false);
				TotalAddedAmount += ActualAmountToAdd;
				AmountToDistribute -= ActualAmountToAdd;
			}

			return TotalAddedAmount;
		}
	}
	else
	{
		auto Item = GetItemFromSlot(GetSlotByPosition(ItemMoveData.TargetSlotCoordinate));
		if (ItemMoveData.SourceItem->GetItemID() == Item->GetItemID())
		{
			int32 ActualAmountToAdd = TryInsertToStackItem(Item, AmountToDistribute, bOnlyCheck);

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
	auto TargetSlots = GetSlotsForItemAt(ItemMoveData.TargetSlotCoordinate, ItemMoveData.SourceItem,
	                                     ItemMoveData.TargetOrientation);
	bool IsUseRefs = InventorySettings.bIsReferenceContainer;

	TArray<UInventorySlotData*> IgnoreSlots = GetIgnoreSlotsForItem(ItemMoveData.SourceItem);
	if (AreSlotsEmpty(TargetSlots, IgnoreSlots))
	{
		bool IsCanPlace = CanPlaceItemAt(ItemMoveData.TargetSlotCoordinate, ItemMoveData.SourceItem,
		                                 ItemMoveData.TargetOrientation, IgnoreSlots);
		if (!IsCanPlace)
		{
			return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
			                                               1, FText::FromName(ItemMoveData.SourceItem->GetItemID())));
		}

		if (!bOnlyCheck)
			ReplaceItem(ItemMoveData.SourceItem, TargetSlots, ItemMoveData.TargetOrientation);
		return FItemAddResult::Swapped(0, IsUseRefs, FText::FromString("Item successfully moved to an empty slot."));
	}

	auto ItemInTarSlot = ItemCollectionLinked->GetItemFromSlot(GetSlotByPosition(ItemMoveData.TargetSlotCoordinate), InventoryContainerID);
	if (!ItemInTarSlot)
	{
		return FItemAddResult::AddedNone(FText::FromString(""));
	}
	auto SourceItemMapping = ItemCollectionLinked->FindItemMappingByContainerName(ItemMoveData.SourceItem, InventoryContainerID);
	auto ItemInTarMapping = ItemCollectionLinked->FindItemMappingByContainerName(ItemInTarSlot, InventoryContainerID);
	auto ItemInTarSlotOccSlots = ItemInTarMapping->OccupiedSlots;
	for (auto InTarSlot : ItemInTarSlotOccSlots)
	{
		IgnoreSlots.Add(InTarSlot);
	}
	
	bool ItemsHaveSameFootprint = true;
	if (!InventorySettings.InventorySlotBasedSettings.bIgnoreItemSize)
	{
		ItemsHaveSameFootprint = UItemBase::DoItemsHaveSameFootprint(ItemMoveData.SourceItem, ItemInTarSlot,
		ItemMoveData.SavedOrientation, ItemInTarMapping->ItemOrientation, InventorySettings.InventorySlotBasedSettings.bIgnoreItemSize);
	}
	
	bool IsCanPlace = CanPlaceItemAt(ItemMoveData.TargetSlotCoordinate, ItemMoveData.SourceItem,
	                                 ItemMoveData.TargetOrientation, IgnoreSlots);

	if (ItemsHaveSameFootprint && IsCanPlace)
	{
		if (!bOnlyCheck)
		{
			ReplaceItem(ItemInTarSlot, SourceItemMapping->OccupiedSlots, ItemInTarMapping->ItemOrientation);
			ReplaceItem(ItemMoveData.SourceItem, TargetSlots, ItemMoveData.TargetOrientation);
		}
		return FItemAddResult::Swapped(0, IsUseRefs, FText::FromString("Items successfully swapped."));
	}

	return FItemAddResult::AddedNone(FText::FromString(""));
}

int32 USlotbasedInventory::DistributeToExistingStacks(TArray<UItemBase*>& SameItems, int32& AmountToDistribute,
                                                      bool bOnlyCheck,
                                                      TMap<UInventorySlotData*, FItemPlacementData>& AffectedSlots)
{
	int32 TotalAdded = 0;

	for (auto* Item : SameItems)
	{
		if (AmountToDistribute <= 0)
			break;

		// Пытаемся вставить в существующий стек.
		// ВАЖНО: Внутри TryInsertToStackItem (или там, где меняется Quantity предмета) 
		// должен вызываться InventoryArray.MarkItemDirty(Entry), иначе клиент не увидит новое число.
		int32 ActualAmountToAdd = TryInsertToStackItem(Item, AmountToDistribute, bOnlyCheck);
		if (ActualAmountToAdd > 0)
		{
			FItemMapping* Mapping = ItemCollectionLinked->FindItemMappingByContainerName(Item, InventoryContainerID);
          
			if (Mapping && Mapping->OccupiedSlots.Num() > 0)
			{
				AffectedSlots.Add(Mapping->OccupiedSlots[0], {ActualAmountToAdd, Mapping->ItemOrientation});
			}

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
	OccupiedSlots.InventoryID = InventoryContainerID;
	OccupiedSlots.bIsReferenceContainer = InventorySettings.bIsReferenceContainer;
	FItemMapping StoredMappingCopy = ItemCollectionLinked->AddItem(FinalItem, OccupiedSlots);

	NotifyAddNewItem(StoredMappingCopy, FinalItem, AddAmount);
	UpdateWeightInfo();
	UpdateMoneyInfo();

	return FinalItem.Get();
}

void USlotbasedInventory::ReplaceItem(UItemBase* Item, const TArray<UInventorySlotData*>& NewSlotDatas,
                                      EItemOrientationType NewItemOrientation)
{
	if (!ItemCollectionLinked || !Item || NewSlotDatas.IsEmpty()) return;

	FItemMapping OldMapping;
	if (FItemMapping* FoundPtr = ItemCollectionLinked->FindItemMappingByContainerName(Item, InventoryContainerID))
	{
		OldMapping = *FoundPtr;
	}
	else return;
	
	ItemCollectionLinked->UpdateItemMapping(Item, InventoryContainerID, NewSlotDatas, NewItemOrientation);
	
	FItemMapping* NewMappingPtr = ItemCollectionLinked->FindItemMappingByContainerName(Item, InventoryContainerID);
	if (NewMappingPtr)
	{
		NotifyReplaceItem(OldMapping.OccupiedSlots, *NewMappingPtr, Item);
	}
}

int32 USlotbasedInventory::TryInsertToStackItem(UItemBase* ItemToInsertInto,
                                                int32 AmountToDistribute,
                                                bool bOnlyCheck)
{
	if (ItemToInsertInto->IsFullItemStack())
		return 0;

	int32 AmountToAddToStack = FMath::Min(AmountToDistribute,
	                                      ItemToInsertInto->GetItemRef().ItemNumeraticData.MaxStackSizeInCharacter
	                                      - ItemToInsertInto ->GetQuantity());
	int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack, ItemToInsertInto->GetItemSingleWeight());
	int32 OldAmount = ItemToInsertInto->GetQuantity();

	if (!bOnlyCheck)
	{
		ItemToInsertInto->SetQuantity(OldAmount + ActualAmountToAdd);
		ItemCollectionLinked->MarkItemAsDirty(ItemToInsertInto);
		NotifyAddItemToStack(ItemToInsertInto);
		
	}
	//ActualAmountToAdd = OldAmount + ActualAmountToAdd;

	return ActualAmountToAdd;
}

bool USlotbasedInventory::bIsSlotPositionValid(FIntPoint GridPosition)
{
	//UE_LOG(LogTemp, Log, TEXT("GridPosition: X=%d, Y=%d"), GridPosition.X, GridPosition.Y);
	return GridPosition.X >= 0 && GridPosition.Y >= 0 && GridPosition.X < InvSize.X && GridPosition.Y < InvSize.Y;
}

bool USlotbasedInventory::BIsItemCategoryCompatible(const FGameplayTag ItemCategory, FGameplayTag SlotCategory, bool bExactMatch)
{
	if (bExactMatch)
	{
		return ItemCategory.MatchesTagExact(SlotCategory);
	}
	
	return ItemCategory.MatchesTag(SlotCategory);
}

bool USlotbasedInventory::bIsSlotEmptyByPos(FIntPoint SlotPosition, const TArray<UInventorySlotData*>& SlotsToIgnore)
{
	auto BusySlots = CollectOccupiedSlots();
	for (const auto InvSlotData : BusySlots)
	{
		if (SlotsToIgnore.Contains(InvSlotData))
			continue;
		
		if (InvSlotData->InventorySlotInfo.CellPosition == SlotPosition)
			return false;
	}

	return true;
}

bool USlotbasedInventory::bIsSlotEmpty(UInventorySlotData* SlotToCheck, const TArray<UInventorySlotData*>& SlotsToIgnore)
{
	auto Posit = (SlotToCheck->InventorySlotInfo.CellPosition);
	return bIsSlotEmptyByPos(Posit, SlotsToIgnore);
}

bool USlotbasedInventory::AreSlotsEmpty(const TArray<UInventorySlotData*>& SlotsToCheck, const TArray<UInventorySlotData*>& SlotsToIgnore)
{
	bool AllSlotsEmpty = true;
	for (const auto SlotData : SlotsToCheck)
	{
		if (!bIsSlotEmpty(SlotData, SlotsToIgnore))
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

TArray<UInventorySlotData*> USlotbasedInventory::CollectOccupiedSlots()
{
	TArray<TObjectPtr<UInventorySlotData>> OccupiedSlots;
	if (!ItemCollectionLinked)
		return OccupiedSlots;

	const FInventoryArray& InventoryData = ItemCollectionLinked->GetItemLocations();
	
	if (InventoryData.Items.IsEmpty())
		return OccupiedSlots;

	for (const auto Mapping : ItemCollectionLinked->GetAllMappingsByContainer(InventoryContainerID))
	{
		for (auto Slot : Mapping.OccupiedSlots)
			OccupiedSlots.AddUnique(Slot);
	}

	return OccupiedSlots;
}

TArray<UInventorySlotData*> USlotbasedInventory::GetIgnoreSlotsForItem(UItemBase* Item)
{
	TArray<UInventorySlotData*> IgnoreSlots;

	if (auto SourceItemMapping = ItemCollectionLinked->FindItemMappingByContainerName(Item, InventoryContainerID))
	{
		for (auto Occup : SourceItemMapping->OccupiedSlots)
		{
			IgnoreSlots.Add(Occup);
		}
	}

	return IgnoreSlots;
}

UInventorySlotData* USlotbasedInventory::GetSlotByPosition(FIntPoint SlotPosition)
{
	for (auto& Elem : InventorySlotData)
	{
		if (Elem->InventorySlotInfo.CellPosition == SlotPosition)
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
	
	const FInventoryArray& InventoryData = ItemCollectionLinked->GetItemLocations();
	
	if (InventoryData.Items.IsEmpty())
		return SameItems;

	SameItems = ItemCollectionLinked->GetAllSameItemsInContainer(InventoryContainerID, ReferenceItem);

	return SameItems;
}

UItemBase* USlotbasedInventory::GetItemFromSlot(UInventorySlotData* Slot)
{
	if (!Slot || !ItemCollectionLinked)
		return nullptr;

	const FInventoryArray& InventoryData = ItemCollectionLinked->GetItemLocations();
	
	if (InventoryData.Items.IsEmpty())
		return nullptr;

	for (const FInventoryEntry& Entry : InventoryData.Items)
	{
		for (const FItemMapping& Mapping : Entry.Locations.Mappings)
		{
			if (Mapping.InventoryID != InventoryContainerID)
				continue;

			for (UInventorySlotData* MapSlot : Mapping.OccupiedSlots)
			{
				if (MapSlot && MapSlot->InventorySlotInfo.CellPosition == Slot->InventorySlotInfo.CellPosition)
				{
					return Entry.Item;
				}
			}
		}
	}

	return nullptr;
}

void USlotbasedInventory::OnRep_InventorySlotData()
{
	OnInventorySlotDataUpdated.Broadcast();
}

void USlotbasedInventory::GenerateInventorySlots()
{
	if (!this->InventoryOwnerActor)
		return;

	if (!InventoryOwnerActor->HasAuthority()) return;

	const auto* MySettings = UInvenzaInventorySettingsSubsystem::GetSettingsStatic(this);
	if (!MySettings)
		return;
	
	if (InvSize.X <= 0 || InvSize.Y <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SlotbasedInventory [%s] on Actor [%s]: Invalid InvSize (%d x %d) — check InventorySettings"),
		*GetName(),
		InventoryOwnerActor ? *InventoryOwnerActor->GetName() : TEXT("NULL"),
		InvSize.X, InvSize.Y);

		return;
	}

	TArray<UInventorySlotData*> ResultSlots;
	
	if (!WidgetSlotInitData.IsEmpty())
	{
		ResultSlots.Reserve(WidgetSlotInitData.Num());

		for (const FInventorySlotInfo& SlotInfo : WidgetSlotInitData)
		{
			UInventorySlotData* NewSlot = UInventorySlotData::CreateWithData(
				this->InventoryOwnerActor,
				SlotInfo.SlotName,
				SlotInfo.CellPosition,
				SlotInfo.UseAction.Get(),
				SlotInfo.AllowedCategory);

			if (NewSlot)
				ResultSlots.Add(NewSlot);
		}
	}
	else
	{
		int32 TotalSlots = InvSize.X * InvSize.Y;
		ResultSlots.Reserve(TotalSlots);

		for (int32 X = 0; X < InvSize.X; X++)
		{
			for (int32 Y = 0; Y < InvSize.Y; Y++)
			{
				UInventorySlotData* NewSlot = UInventorySlotData::CreateWithData(
					this->InventoryOwnerActor,
					NAME_None,
					FIntPoint(X, Y),
					nullptr,
					MySettings->AnyCategoryGameplayTag);

				if (NewSlot)
					ResultSlots.Add(NewSlot);
			}
		}
	}

	InventorySlotData = ResultSlots;
	WidgetSlotInitData.Empty();

	OnRep_InventorySlotData();
}


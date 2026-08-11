//  Nublin Studio 2026 All Rights Reserved.

#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"

#include "ActorComponents/ItemCollection.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "UI/Inventory/InventorySlot.h"
#include "Utility/InterfaceUtils.h"
#include "Utility/InvenzayUtility.h"

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

	auto AllItems = ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID);
	TArray<UObject*> SortedItems;
	SortedItems.Reserve(AllItems.Num());

	for (UObject* Item : AllItems)
	{
		if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("SortItemsInContainerByName")))
			continue;

		UObject* NewItem = IObjectDataProvider::Execute_DuplicateItem(Item);
		SortedItems.Add(NewItem);
	}

	for (UObject* Item : AllItems)
	{
		if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("SortItemsInContainerByName")))
			continue;

		HandleRemoveItem(Item, IObjectDataProvider::Execute_GetQuantity(Item));
	}

	SortedItems.Sort([](const UObject& A, const UObject& B)
	{
		const FString NameA = IObjectDataProvider::Execute_GetItemDisplayText(&A).ToString();
		const FString NameB = IObjectDataProvider::Execute_GetItemDisplayText(&B).ToString();
		return NameA.Compare(NameB, ESearchCase::IgnoreCase) < 0;
	});

	for (int32 i = 0; i < SortedItems.Num(); i++)
	{
		UObject* ItemObj = SortedItems[i];
		if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemObj, TEXT("SortItemsInContainerByName")))
			continue;

		FItemMoveData ItemMoveData;
		ItemMoveData.TargetInventory = this;
		ItemMoveData.SourceItem = ItemObj;

		const EItemOrientationType InitOrientation =
			IObjectDataProvider::Execute_GetInitialItemOrientation(ItemObj);

		ItemMoveData.SavedOrientation = InitOrientation;
		ItemMoveData.TargetOrientation = InitOrientation;

		HandleAddItem(ItemMoveData);
	}

	NotifyReDrawRequest();
	OnRep_InventoryTotalWeight();
	OnRep_InventoryTotalMoney();
}

TArray<UInventorySlotData*> USlotbasedInventory::GetSlotsForItemAt(const FIntPoint& StartPos, UObject* ItemBase,
                                                                   EItemOrientationType Orientation)
{
	TArray<TObjectPtr<UInventorySlotData>> Slots;

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemBase, TEXT("GetSlotsForItemAt")))
		return Slots;

	if (InventorySettings.InventorySlotBasedSettings.bIgnoreItemSize)
	{
		if (auto SlotForAdd = GetSlotByPosition(StartPos))
			Slots.Add(SlotForAdd);
		return Slots;
	}

	TArray<FIntPoint> Positions = GetItemGridPositions(
		StartPos,
		IObjectDataProvider::Execute_GetItemSize(ItemBase, Orientation)
	);

	for (const FIntPoint& Pos : Positions)
	{
		auto SlotForAdd = GetSlotByPosition(Pos);
		if (SlotForAdd)
			Slots.Add(SlotForAdd);
	}

	return Slots;
}

bool USlotbasedInventory::ReserveSlots(AActor* Requestor, TMap<UInventorySlotData*, FItemPlacementData> Slots,
                                       UObject* ItemBase)
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

bool USlotbasedInventory::CanPlaceItemAt(const FIntPoint& StartPos, FGameplayTag ItemCategory, FIntPoint ItemSize,
                                         EItemOrientationType Orientation, TArray<UInventorySlotData*> IgnoreSlots)
{
	if (InventorySettings.InventorySlotBasedSettings.bIgnoreItemSize)
	{
		auto CheckSlot = GetSlotByPosition(StartPos);
		if (!CheckSlot)
			return false;

		return bIsSlotPositionValid(StartPos)
			&& bIsSlotEmptyByPos(StartPos, IgnoreSlots)
			&& BIsItemCategoryCompatible(ItemCategory,
				CheckSlot->InventorySlotInfo.AllowedCategory
			);
	}

	TArray<FIntPoint> Positions = GetItemGridPositions(StartPos,ItemSize );

	for (const FIntPoint& CheckPos : Positions)
	{
		if (!bIsSlotPositionValid(CheckPos))
			return false;

		auto CheckSlot = GetSlotByPosition(CheckPos);
		if (!CheckSlot)
			return false;

		if (!bIsSlotEmptyByPos(CheckPos, IgnoreSlots))
			return false;

		if (!BIsItemCategoryCompatible(ItemCategory,CheckSlot->InventorySlotInfo.AllowedCategory))
			return false;
	}

	return true;
}

void USlotbasedInventory::RequestSplitStack(UObject* ItemToSplit, int32 SplitAmount)
{
	if (!ItemToSplit || SplitAmount <= 0)
		return;

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemToSplit, TEXT("RequestSplitStack")))
		return;

	const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(ItemToSplit);
	if (Quantity == 1 || Quantity <= SplitAmount)
		return;

	EItemOrientationType FinalOrientation;
	auto EmptySlots = GetAvailableSlotForItem(ItemToSplit, FinalOrientation);
	if (EmptySlots.IsEmpty())
		return;

	if (InventorySettings.MaxStackCount > 0)
	{
		auto ResultMaxStack = ItemCollectionLinked->GetStackCountInContainer(InventoryContainerID);
		if (ResultMaxStack + 1 > InventorySettings.MaxStackCount)
			return;
	}

	UObject* NewItem = IObjectDataProvider::Execute_DuplicateItem(ItemToSplit);
	if (!NewItem)
		return;

	IObjectDataProvider::Execute_SetQuantity(NewItem, SplitAmount);

	FItemMoveData ItemMove;
	ItemMove.SourceItem = NewItem;
	ItemMove.TargetInventory = this;
	ItemMove.TargetSlotID = EmptySlots[0]->InventorySlotInfo.SlotGuid;
	ItemMove.SavedOrientation = FinalOrientation;
	ItemMove.TargetOrientation = FinalOrientation;

	OnSplitDelegate.Broadcast(this, ItemToSplit, SplitAmount);
}

TArray<FItemIDEntry> USlotbasedInventory::CollectItemsAggregated() const
{
	TArray<FItemIDEntry> Result;

	if (!ItemCollectionLinked)
		return Result;

	TArray<UObject*> Instances =
		ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID);

	TMap<FName, int32> AggregatedMap;

	for (UObject* Instance : Instances)
	{
		if (!Instance)
			continue;

		if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Instance, TEXT("CollectItemsAggregated")))
			continue;

		FName ItemID = IObjectDataProvider::Execute_GetItemID(Instance);
		int32 Count = IObjectDataProvider::Execute_GetQuantity(Instance);

		int32& StoredCount = AggregatedMap.FindOrAdd(ItemID);
		StoredCount += Count;
	}

	for (const auto& Pair : AggregatedMap)
	{
		FItemIDEntry Entry;
		Entry.ItemID = Pair.Key;
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
	UObject* Item, EItemOrientationType& OutOrientation)
{
	if (!Item || !UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("GetAvailableSlotForItem")))
	{
		OutOrientation = EItemOrientationType::Horizontal;
		return {};
	}

	const bool bPreferVertical =
		IObjectDataProvider::Execute_GetInitialItemOrientation(Item) == EItemOrientationType::Vertical;

	const EItemOrientationType First  = bPreferVertical ? EItemOrientationType::Vertical   : EItemOrientationType::Horizontal;
	const EItemOrientationType Second = bPreferVertical ? EItemOrientationType::Horizontal : EItemOrientationType::Vertical;

	const FGameplayTag ItemCategory =
		IObjectDataProvider::Execute_GetItemRef(Item).ItemCategory;

	const FIntPoint SizeFirst =
		IObjectDataProvider::Execute_GetItemSize(Item, First);

	const FIntPoint SizeSecond =
		IObjectDataProvider::Execute_GetItemSize(Item, Second);

	for (int32 i = 0; i < InvSize.X; i++)
	{
		for (int32 j = 0; j < InvSize.Y; j++)
		{
			const FIntPoint StartPos(i, j);

			if (CanPlaceItemAt(StartPos, ItemCategory, SizeFirst, First, {}))
			{
				OutOrientation = First;
				return GetSlotsForItemAt(StartPos, Item, First);
			}

			if (CanPlaceItemAt(StartPos, ItemCategory, SizeSecond, Second, {}))
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

void USlotbasedInventory::HandleRemoveItemsByID(FName ItemID, int32 RequestedAmount)
{
	if (ItemID.IsNone() || RequestedAmount <= 0) return;

	int32 RemainingToRemove = RequestedAmount;
	int32 RemovedTotal = 0;
	TArray<UObject*> FoundItems =	ItemCollectionLinked->GetAllSameItemsInContainerByID(InventoryContainerID, ItemID);
	if (FoundItems.IsEmpty())
		return;

	for (UObject* Item : FoundItems)
	{
		if (!Item || RemainingToRemove <= 0)
			break;

		int32 Removed = TryRemoveFromStackItem(Item, RemainingToRemove);

		RemainingToRemove -= Removed;
		RemovedTotal += Removed;
	}
}

void USlotbasedInventory::HandleRemoveItemsBySample(UObject* ItemSample, int32 RequestedAmount)
{
	if (!ItemSample || RequestedAmount <= 0) return;
	
	int32 RemainingToRemove = RequestedAmount;
	int32 RemovedTotal = 0;
	TArray<UObject*> FoundItems =	ItemCollectionLinked->GetAllSameItemsInContainerByItemSample(InventoryContainerID, ItemSample);
	if (FoundItems.IsEmpty())
		return;

	for (UObject* Item : FoundItems)
	{
		if (!Item || RemainingToRemove <= 0)
			break;

		int32 Removed = TryRemoveFromStackItem(Item, RemainingToRemove);

		RemainingToRemove -= Removed;
		RemovedTotal += Removed;
	}
}

void USlotbasedInventory::HandleRemoveItem(UObject* Item, int32 RemoveQuantity)
{
	if (!Item || !UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("HandleRemoveItem")))
		return;

	const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(Item);

	if (Quantity <= 0)
	{
		RemoveItemFromInventory(Item);
		return;
	}

	TryRemoveFromStackItem(Item, RemoveQuantity);
}

FItemAddResult USlotbasedInventory::HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
	if (!ItemMoveData.SourceItem ||
        !UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemMoveData.SourceItem, TEXT("HandleAddItem")))
    {
        return FItemAddResult::AddedNone(FText::FromString("Item is null. Nothing to add"));
    }

    const int32 ItemQuantity = IObjectDataProvider::Execute_GetQuantity(ItemMoveData.SourceItem);

    if (ItemMoveData.SourceInventory && ItemQuantity <= 0)
        UE_LOG(LogTemp, Warning, TEXT("item Quantity is %i"), ItemQuantity);

    if (ItemMoveData.SourceInventory &&
        ItemMoveData.SourceInventory->GetInventorySettings().bIsReferenceContainer &&
        InventorySettings.bIsReferenceContainer &&
        ItemMoveData.SourceInventory != ItemMoveData.TargetInventory)
    {
        return FItemAddResult::AddedNone(
            FText::FromString("Moving items between reference containers is not allowed."));
    }

    if (ItemMoveData.SourceInventory &&
        ItemMoveData.SourceInventory != ItemMoveData.TargetInventory &&
        !ItemMoveData.SourceInventory->GetInventorySettings().bAllowItemReferencing &&
        InventorySettings.bIsReferenceContainer)
    {
        return FItemAddResult::AddedNone(
            FText::Format(
                FText::FromString("Item {0} cannot be added because the source inventory does not allow referencing."),
                FText::FromName(IObjectDataProvider::Execute_GetItemID(ItemMoveData.SourceItem))));
    }

    if (ItemCollectionLinked->ItemHasInventory(ItemMoveData.SourceItem, InventoryContainerID))
    {
        if (ItemMoveData.SourceInventory == this)
        {
            TArray<UInventorySlotData*> SlotsToIgnore;
            auto TarSlotPos = GetSlotByGuid(ItemMoveData.TargetSlotID)->InventorySlotInfo.CellPosition;

            if (IObjectDataProvider::Execute_IsStackable(ItemMoveData.SourceItem) &&
                !bIsSlotEmpty(GetSlotByPosition(TarSlotPos), SlotsToIgnore))
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

    if (ItemMoveData.SourceInventory &&
        ItemMoveData.SourceInventory->GetInventorySettings().bIsReferenceContainer &&
        ItemMoveData.SourceInventory != ItemMoveData.TargetInventory &&
        ItemMoveData.TargetSlotID.IsValid())
    {
        TArray<UInventorySlotData*> IgnoreSlots = GetIgnoreSlotsForItem(ItemMoveData.SourceItem);
        auto TarSlotPos = GetSlotByGuid(ItemMoveData.TargetSlotID)->InventorySlotInfo.CellPosition;

        if (bIsSlotEmpty(GetSlotByPosition(TarSlotPos), IgnoreSlots))
        {
            if (auto Mapping = ItemCollectionLinked->FindItemMappingByContainerName(
                ItemMoveData.SourceItem, InventoryContainerID))
            {
                return FItemAddResult::AddedNone(FText::FromString("Cant add Item by References"));
            }
        }

        return FItemAddResult::AddedNone(FText::FromString("Cant add Item by References"));
    }

    const bool bIsStackable = IObjectDataProvider::Execute_IsStackable(ItemMoveData.SourceItem);

    if (!bIsStackable)
    {
        if (InventorySettings.InventoryMaxWeightCapacity >= 0)
        {
            const float SingleWeight = IObjectDataProvider::Execute_GetItemSingleWeight(ItemMoveData.SourceItem);

            if (InventoryTotalWeight + SingleWeight > InventorySettings.InventoryMaxWeightCapacity)
            {
                return FItemAddResult::AddedNone(
                    FText::Format(
                        FText::FromString("Item {0} would overflow weight limit"),
                        FText::FromName(IObjectDataProvider::Execute_GetItemID(ItemMoveData.SourceItem))));
            }
        }

        return HandleNonStackableItems(ItemMoveData, bOnlyCheck);
    }

    if (bIsStackable)
        return TryAddStackableItem(ItemMoveData, bOnlyCheck);

    return FItemAddResult::AddedNone(FText::FromString("Item is null. Nothing to add"));
}

FItemAddResult USlotbasedInventory::HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemMoveData.SourceItem, TEXT("HandleAddReferenceItem")))
        return FItemAddResult::AddedNone(FText::FromString("Invalid item"));

    if (ItemMoveData.TargetSlotID.IsValid())
        return FItemAddResult::AddedNone(
            FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
            1, FText::FromName(IObjectDataProvider::Execute_GetItemID(ItemMoveData.SourceItem))));

    FIntPoint TarSlotPos(-1);
    if (ItemMoveData.TargetSlotID.IsValid())
        TarSlotPos = GetSlotByGuid(ItemMoveData.TargetSlotID)->InventorySlotInfo.CellPosition;

    auto TargetSlots = GetSlotsForItemAt(TarSlotPos, ItemMoveData.SourceItem, ItemMoveData.TargetOrientation);
    TArray<UInventorySlotData*> IgnoreSlots = GetIgnoreSlotsForItem(ItemMoveData.SourceItem);

    if (AreSlotsEmpty(TargetSlots, IgnoreSlots))
    {
        const FGameplayTag ItemCategory = IObjectDataProvider::Execute_GetItemRef(ItemMoveData.SourceItem).ItemCategory;
        const FIntPoint ItemSize = IObjectDataProvider::Execute_GetItemSize(ItemMoveData.SourceItem, ItemMoveData.TargetOrientation);

        bool IsCanPlace = CanPlaceItemAt(TarSlotPos, ItemCategory, ItemSize, ItemMoveData.TargetOrientation, IgnoreSlots);
        if (!IsCanPlace)
        {
            return FItemAddResult::AddedNone(
                FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
                1, FText::FromName(IObjectDataProvider::Execute_GetItemID(ItemMoveData.SourceItem))));
        }

        FItemMapping Slots;
        Slots.InventoryID = InventoryContainerID;
        Slots.OccupiedSlots = TargetSlots;
        Slots.ItemOrientation = ItemMoveData.TargetOrientation;

        if (!bOnlyCheck)
            AddNewItem(ItemMoveData, Slots, IObjectDataProvider::Execute_GetQuantity(ItemMoveData.SourceItem));

        TMap<UInventorySlotData*, FItemPlacementData> AffectedSlots;
        AffectedSlots.Add(Slots.OccupiedSlots[0], {1, ItemMoveData.TargetOrientation});

        return FItemAddResult::AddedAll(
            1, true,
            FText::Format(FText::FromString("Successfully added {0} to inventory as a reference"),
            FText::FromName(IObjectDataProvider::Execute_GetItemID(ItemMoveData.SourceItem))),
            AffectedSlots);
    }

    UObject* ItemInTarSlot =
        ItemCollectionLinked->GetItemFromSlot(GetSlotByPosition(TarSlotPos)->InventorySlotInfo.SlotGuid, InventoryContainerID);

    if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemInTarSlot, TEXT("HandleAddReferenceItem")))
        return FItemAddResult::AddedNone(FText::FromString("Invalid target item"));

    auto ItemInTarMapping = ItemCollectionLinked->FindItemMappingByContainerName(ItemInTarSlot, InventoryContainerID);
    auto ItemInTarSlotOccSlots = ItemInTarMapping->OccupiedSlots;

    for (auto InTarSlot : ItemInTarSlotOccSlots)
        IgnoreSlots.Add(InTarSlot);

    bool ItemsHaveSameFootprint = true;

    if (!InventorySettings.InventorySlotBasedSettings.bIgnoreItemSize)
    {
        ItemsHaveSameFootprint =
            UInvenzayUtility::DoItemsHaveSameFootprint(
                ItemMoveData.SourceItem,
                ItemInTarSlot,
                ItemMoveData.SavedOrientation,
                ItemInTarMapping->ItemOrientation,
                InventorySettings.InventorySlotBasedSettings.bIgnoreItemSize);
    }

    const FGameplayTag ItemCategory = IObjectDataProvider::Execute_GetItemRef(ItemMoveData.SourceItem).ItemCategory;
    const FIntPoint ItemSize = IObjectDataProvider::Execute_GetItemSize(ItemMoveData.SourceItem, ItemMoveData.TargetOrientation);

    bool IsCanPlace = CanPlaceItemAt(TarSlotPos, ItemCategory, ItemSize, ItemMoveData.TargetOrientation, IgnoreSlots);

    if (ItemsHaveSameFootprint && IsCanPlace)
    {
        FItemMapping Slots;
        Slots.InventoryID = InventoryContainerID;
        Slots.OccupiedSlots = ItemInTarSlotOccSlots;
        Slots.ItemOrientation = ItemMoveData.TargetOrientation;

        if (!bOnlyCheck)
        {
            RemoveItemFromInventory(ItemInTarSlot);
            AddNewItem(ItemMoveData, Slots, IObjectDataProvider::Execute_GetQuantity(ItemMoveData.SourceItem));
        }

        TMap<UInventorySlotData*, FItemPlacementData> AffectedPivotSlots;
        AffectedPivotSlots.Add(ItemInTarSlotOccSlots[0], {1, ItemMoveData.TargetOrientation});

        return FItemAddResult::AddedAll(
            1, true,
            FText::Format(FText::FromString("Successfully added {0} to inventory as a reference"),
            FText::FromName(IObjectDataProvider::Execute_GetItemID(ItemMoveData.SourceItem))),
            AffectedPivotSlots);
    }

    return FItemAddResult::AddedNone(FText::FromString(""));
}

FItemAddResult USlotbasedInventory::HandleNonStackableItems(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
    if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemMoveData.SourceItem, TEXT("HandleNonStackableItems")))
        return FItemAddResult::AddedNone(FText::FromString("Invalid item"));

    TArray<TObjectPtr<UInventorySlotData>> EmptySlots;
    EItemOrientationType FinalOrientation = ItemMoveData.TargetOrientation;

    FIntPoint TarSlotPos(-1);
    if (ItemMoveData.TargetSlotID.IsValid())
        TarSlotPos = GetSlotByGuid(ItemMoveData.TargetSlotID)->InventorySlotInfo.CellPosition;

    const FGameplayTag ItemCategory = IObjectDataProvider::Execute_GetItemRef(ItemMoveData.SourceItem).ItemCategory;
    const FIntPoint ItemSize = IObjectDataProvider::Execute_GetItemSize(ItemMoveData.SourceItem, FinalOrientation);

    if (!bIsSlotPositionValid(TarSlotPos))
    {
        EmptySlots = GetAvailableSlotForItem(ItemMoveData.SourceItem, FinalOrientation);
    }
    else
    {
        if (!CanPlaceItemAt(TarSlotPos, ItemCategory, ItemSize, FinalOrientation, {}))
            return FItemAddResult::AddedNone(FText::FromString("Can't place item at target slot"));

        EmptySlots = GetSlotsForItemAt(TarSlotPos, ItemMoveData.SourceItem, FinalOrientation);
    }

    if (EmptySlots.IsEmpty())
    {
        return FItemAddResult::AddedNone(
            FText::Format(FText::FromString("Can't be added {0} of {1} to inventory. No empty slots"),
            1, FText::FromString(ItemMoveData.SourceItem->GetName())));
    }

    if (EmptySlots.ContainsByPredicate([](const TObjectPtr<UInventorySlotData>& S) { return !S; }))
        return FItemAddResult::AddedNone(FText::FromString("Invalid slot data"));

    const float SingleWeight = IObjectDataProvider::Execute_GetItemSingleWeight(ItemMoveData.SourceItem);

    if (CalculateActualAmountToAdd(1, SingleWeight) <= 0)
    {
        return FItemAddResult::AddedNone(
            FText::Format(FText::FromString("Item {0} would overflow limits"),
            IObjectDataProvider::Execute_GetItemRef(ItemMoveData.SourceItem).ItemTextData.DisplayName));
    }

    if (!bOnlyCheck)
    {
        FItemMapping Slots;
        Slots.InventoryID = InventoryContainerID;
        Slots.OccupiedSlots = EmptySlots;
        Slots.ItemOrientation = FinalOrientation;

        AddNewItem(ItemMoveData, Slots, 1);
    }

    TMap<UInventorySlotData*, FItemPlacementData> AffectedPivotSlots;
    AffectedPivotSlots.Add(EmptySlots[0], {1, FinalOrientation});

    return FItemAddResult::AddedAll(1, false, FText::FromString("Successfully added to inventory"), AffectedPivotSlots);
}

FItemAddResult USlotbasedInventory::TryAddStackableItem(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	
	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemMoveData.SourceItem, TEXT("TryAddStackableItem")))
	{
		return FItemAddResult::AddedNone(FText::FromString("Invalid item"));
	}

	TMap<UInventorySlotData*, FItemPlacementData> AffectedPivotSlots;

	const int32 InitialRequestedAddAmount =
		IObjectDataProvider::Execute_GetQuantity(ItemMoveData.SourceItem);

	const int32 StackableAmountAdded =
		HandleStackableItems(ItemMoveData, InitialRequestedAddAmount, bOnlyCheck, AffectedPivotSlots);

	if (StackableAmountAdded == InitialRequestedAddAmount)
	{
		return FItemAddResult::AddedAll(
			StackableAmountAdded,
			false,
			FText::Format(
				FText::FromString("Successfully added {0} of {1} to inventory"),
				FText::AsNumber(InitialRequestedAddAmount),
				FText::FromString(IObjectDataProvider::Execute_GetItemID(ItemMoveData.SourceItem).ToString())
			),
			AffectedPivotSlots);
	}
	else if (StackableAmountAdded < InitialRequestedAddAmount && StackableAmountAdded > 0)
	{
		return FItemAddResult::AddedPartial(
			StackableAmountAdded,
			false,
			FText::Format(
				FText::FromString("Partial amount of {0} added to inventory. Number added: {1}"),
				FText::FromString(IObjectDataProvider::Execute_GetItemID(ItemMoveData.SourceItem).ToString()),
				StackableAmountAdded
			),
			AffectedPivotSlots);
	}

	return FItemAddResult::AddedNone(
		FText::Format(
			FText::FromString("Couldn't add {0} to inventory."),
			FText::FromString(IObjectDataProvider::Execute_GetItemID(ItemMoveData.SourceItem).ToString())
		));
}

int32 USlotbasedInventory::HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount, bool bOnlyCheck,
                                                TMap<UInventorySlotData*, FItemPlacementData>& AffectedPivotSlots)
{
	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemMoveData.SourceItem, TEXT("HandleStackableItems")))
        return 0;

    int32 AmountToDistribute = RequestedAddAmount;
    int32 TotalAddedAmount = 0;

    FIntPoint TarSlotPos(-1);
    if (ItemMoveData.TargetSlotID.IsValid())
        TarSlotPos = GetSlotByGuid(ItemMoveData.TargetSlotID)->InventorySlotInfo.CellPosition;

    const FGameplayTag ItemCategory =
        IObjectDataProvider::Execute_GetItemRef(ItemMoveData.SourceItem).ItemCategory;

    const FIntPoint ItemSize =
        IObjectDataProvider::Execute_GetItemSize(ItemMoveData.SourceItem, ItemMoveData.TargetOrientation);

    if (!bIsSlotPositionValid(TarSlotPos))
    {
        while (AmountToDistribute > 0)
        {
            auto SameItems = GetAllSameItems(ItemMoveData.SourceItem);
            if (SameItems.Num() > 0)
            {
                TotalAddedAmount += DistributeToExistingStacks(
                    SameItems,
                    AmountToDistribute,
                    bOnlyCheck,
                    AffectedPivotSlots);
            }

            if (AmountToDistribute <= 0)
                return RequestedAddAmount;

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

                AddNewItem(ItemMoveData, Slots, AmountToDistribute);
            }

            TotalAddedAmount += AmountToDistribute;
            AmountToDistribute = 0;
        }

        return TotalAddedAmount;
    }

    TArray<UInventorySlotData*> IgnoreSlots = GetIgnoreSlotsForItem(ItemMoveData.SourceItem);

    if (bIsSlotEmpty(GetSlotByPosition(TarSlotPos), IgnoreSlots))
    {
        if (CanPlaceItemAt(TarSlotPos, ItemCategory, ItemSize, ItemMoveData.TargetOrientation, {}))
        {
            auto EmptySlots = GetSlotsForItemAt(TarSlotPos, ItemMoveData.SourceItem, ItemMoveData.TargetOrientation);

            if (bOnlyCheck)
            {
                UObject* ItemToTest = IObjectDataProvider::Execute_DuplicateItem(ItemMoveData.SourceItem);
                IObjectDataProvider::Execute_SetQuantity(ItemToTest, 1);

                int32 StackAmount =
                    TryInsertToStackItem(ItemToTest, AmountToDistribute - 1, true);

                TotalAddedAmount += 1 + StackAmount;
                AmountToDistribute -= 1 + StackAmount;
            }
            else
            {
                FItemMapping Slots;
                Slots.InventoryID = InventoryContainerID;
                Slots.OccupiedSlots = EmptySlots;
                Slots.ItemOrientation = ItemMoveData.TargetOrientation;

                UObject* NewItem = AddNewItem(ItemMoveData, Slots, 1);

                TotalAddedAmount += 1;
                AmountToDistribute -= 1;

                int32 ActualAmountToAdd =
                    TryInsertToStackItem(NewItem, AmountToDistribute, false);

                TotalAddedAmount += ActualAmountToAdd;
                AmountToDistribute -= ActualAmountToAdd;
            }

            return TotalAddedAmount;
        }
    }
    else
    {
        UObject* ItemInSlot = GetItemFromSlot(GetSlotByPosition(TarSlotPos));

        if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemInSlot, TEXT("HandleStackableItems")))
            return TotalAddedAmount;

        if (IObjectDataProvider::Execute_GetItemID(ItemMoveData.SourceItem) ==
            IObjectDataProvider::Execute_GetItemID(ItemInSlot))
        {
            int32 ActualAmountToAdd =
                TryInsertToStackItem(ItemInSlot, AmountToDistribute, bOnlyCheck);

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
    if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemMoveData.SourceItem, TEXT("TryReplaceItems")))
        return FItemAddResult::AddedNone(FText::FromString("Invalid item"));

    const FGameplayTag ItemCategory =
        IObjectDataProvider::Execute_GetItemRef(ItemMoveData.SourceItem).ItemCategory;

    const FIntPoint ItemSize =
        IObjectDataProvider::Execute_GetItemSize(ItemMoveData.SourceItem, ItemMoveData.TargetOrientation);

    auto TarSlotPos = GetSlotByGuid(ItemMoveData.TargetSlotID)->InventorySlotInfo.CellPosition;
    auto TargetSlots = GetSlotsForItemAt(TarSlotPos, ItemMoveData.SourceItem, ItemMoveData.TargetOrientation);

    bool IsUseRefs = InventorySettings.bIsReferenceContainer;

    TArray<UInventorySlotData*> IgnoreSlots = GetIgnoreSlotsForItem(ItemMoveData.SourceItem);

    if (AreSlotsEmpty(TargetSlots, IgnoreSlots))
    {
        bool IsCanPlace =
            CanPlaceItemAt(TarSlotPos, ItemCategory, ItemSize, ItemMoveData.TargetOrientation, IgnoreSlots);

        if (!IsCanPlace)
        {
            return FItemAddResult::AddedNone(
                FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
                1, FText::FromName(IObjectDataProvider::Execute_GetItemID(ItemMoveData.SourceItem))));
        }

        if (!bOnlyCheck)
            ReplaceItem(ItemMoveData.SourceItem, TargetSlots, ItemMoveData.TargetOrientation);

        return FItemAddResult::Swapped(0, IsUseRefs, FText::FromString("Item successfully moved to an empty slot."));
    }

    UObject* ItemInTarSlot =
        ItemCollectionLinked->GetItemFromSlot(ItemMoveData.TargetSlotID, InventoryContainerID);

    if (!ItemInTarSlot ||
        !UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemInTarSlot, TEXT("TryReplaceItems")))
        return FItemAddResult::AddedNone(FText::FromString(""));

    auto SourceItemMapping =
        ItemCollectionLinked->FindItemMappingByContainerName(ItemMoveData.SourceItem, InventoryContainerID);

    auto ItemInTarMapping =
        ItemCollectionLinked->FindItemMappingByContainerName(ItemInTarSlot, InventoryContainerID);

    auto ItemInTarSlotOccSlots = ItemInTarMapping->OccupiedSlots;

    for (auto InTarSlot : ItemInTarSlotOccSlots)
        IgnoreSlots.Add(InTarSlot);

    bool ItemsHaveSameFootprint = true;

    if (!InventorySettings.InventorySlotBasedSettings.bIgnoreItemSize)
    {
        ItemsHaveSameFootprint =
            UInvenzayUtility::DoItemsHaveSameFootprint(
                ItemMoveData.SourceItem,
                ItemInTarSlot,
                ItemMoveData.SavedOrientation,
                ItemInTarMapping->ItemOrientation,
                InventorySettings.InventorySlotBasedSettings.bIgnoreItemSize);
    }

    bool IsCanPlace =
        CanPlaceItemAt(TarSlotPos, ItemCategory, ItemSize, ItemMoveData.TargetOrientation, IgnoreSlots);

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

int32 USlotbasedInventory::DistributeToExistingStacks(TArray<UObject*>& SameItems, int32& AmountToDistribute,
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

UObject* USlotbasedInventory::AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots,
                                           int32 AddAmount)
{
	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemMoveData.SourceItem, TEXT("AddNewItem")))
		return nullptr;

	TObjectPtr<UObject> FinalItem;

	if (InventorySettings.bIsReferenceContainer)
	{
		FinalItem = ItemMoveData.SourceItem;
	}
	else
	{
		FinalItem = IObjectDataProvider::Execute_DuplicateItem(ItemMoveData.SourceItem);
		IObjectDataProvider::Execute_SetQuantity(FinalItem, AddAmount);
	}

	OccupiedSlots.InventoryID = InventoryContainerID;
	OccupiedSlots.bIsReferenceContainer = InventorySettings.bIsReferenceContainer;

	FItemMapping StoredMappingCopy = ItemCollectionLinked->AddItem(FinalItem, OccupiedSlots);

	NotifyAddNewItem(StoredMappingCopy, FinalItem, AddAmount);
	UpdateWeightInfo();
	UpdateMoneyInfo();

	return FinalItem.Get();
}

void USlotbasedInventory::ReplaceItem(UObject* Item, const TArray<UInventorySlotData*>& NewSlotDatas,
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

int32 USlotbasedInventory::TryInsertToStackItem(UObject* ItemToInsertInto,
                                                int32 AmountToDistribute,
                                                bool bOnlyCheck)
{
	if (!ItemToInsertInto ||
		!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemToInsertInto, TEXT("TryInsertToStackItem")))
		return 0;

	if (IObjectDataProvider::Execute_IsFullItemStack(ItemToInsertInto))
		return 0;

	const FItemMetaData Meta = IObjectDataProvider::Execute_GetItemRef(ItemToInsertInto);
	const int32 CurrentQuantity = IObjectDataProvider::Execute_GetQuantity(ItemToInsertInto);
	const int32 MaxStack = Meta.ItemNumeraticData.MaxStackSizeInCharacter;

	const int32 AmountToAddToStack = FMath::Min(AmountToDistribute, MaxStack - CurrentQuantity);
	const float SingleWeight = IObjectDataProvider::Execute_GetItemSingleWeight(ItemToInsertInto);

	const int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack, SingleWeight);

	if (!bOnlyCheck)
	{
		const int32 NewQuantity = CurrentQuantity + ActualAmountToAdd;
		IObjectDataProvider::Execute_SetQuantity(ItemToInsertInto, NewQuantity);

		ItemCollectionLinked->MarkItemAsDirty(ItemToInsertInto);
		NotifyAddItemToStack(ItemToInsertInto);
	}

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

bool USlotbasedInventory::bIsSlotEmptyByID(FGuid SlotIDToCheck, const TArray<FGuid>& SlotsIDToIgnore)
{
	auto BusySlots = CollectOccupiedSlots();
	for (const auto InvSlotData : BusySlots)
	{
		if (SlotsIDToIgnore.Contains(InvSlotData->InventorySlotInfo.SlotGuid))
			continue;
		
		if (InvSlotData->InventorySlotInfo.SlotGuid == SlotIDToCheck)
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

TArray<UInventorySlotData*> USlotbasedInventory::GetIgnoreSlotsForItem(UObject* Item)
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

UInventorySlotData* USlotbasedInventory::GetSlotByGuid(FGuid InGuid)
{
	for (auto& Elem : InventorySlotData)
	{
		if (Elem->InventorySlotInfo.SlotGuid == InGuid)
			return Elem;
	}

	return nullptr;
}

TArray<FGuid> USlotbasedInventory::GetSlotsWithLinkedEquipment()
{
	TArray<FGuid> ResultArray;
	if (InventorySlotData.IsEmpty())
		return ResultArray;

	for (auto SlotData : InventorySlotData)
	{
		if (SlotData->IsEquipmentSlot())
			ResultArray.Add(SlotData->InventorySlotInfo.SlotGuid);
	}
	
	return ResultArray;
}

TArray<UObject*> USlotbasedInventory::GetAllSameItems(UObject* ReferenceItem)
{
	TArray<UObject*> SameItems;
	if (!ReferenceItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetAllSameItemsInContainerByItemSample: %s"), TEXT("ReferenceItem is null."));
		return SameItems;
	}
	
	const FInventoryArray& InventoryData = ItemCollectionLinked->GetItemLocations();
	
	if (InventoryData.Items.IsEmpty())
		return SameItems;

	SameItems = ItemCollectionLinked->GetAllSameItemsInContainerByItemSample(InventoryContainerID, ReferenceItem);

	return SameItems;
}

UObject* USlotbasedInventory::GetItemFromSlot(UInventorySlotData* Slot)
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


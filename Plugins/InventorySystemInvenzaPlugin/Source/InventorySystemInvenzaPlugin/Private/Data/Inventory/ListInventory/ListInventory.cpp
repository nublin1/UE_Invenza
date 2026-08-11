//  Nublin Studio 2026 All Rights Reserved.


#include "Data/Inventory/ListInventory/ListInventory.h"

#include "ActorComponents/ItemCollection.h"
#include "Data/Items/ItemBase.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "UI/Inventory/ListInventorySlotWidget.h"
#include "Utility/InterfaceUtils.h"


UListInventory::UListInventory()
{
}

void UListInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UListInventory, InvSlotsArray);
	DOREPLIFETIME(UListInventory, EntryClass);
}

bool UListInventory::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
	struct FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	for (int32 i = 0; i < InvSlotsArray.Num(); ++i)
	{
		UInventoryListEntry* Entry = InvSlotsArray[i].Get();
		if (IsValid(Entry))
		{
			if (Entry->IsSupportedForNetworking())
			{
				bWroteSomething |= Channel->ReplicateSubobject(Entry, *Bunch, *RepFlags);
			}
		}
	}
    
	return bWroteSomething;
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

	auto SortByName = [](const UObject& A, const UObject& B)
	{
		return IObjectDataProvider::Execute_GetItemDisplayText(&A).ToString()
			 < IObjectDataProvider::Execute_GetItemDisplayText(&B).ToString();
	};

	auto AllItems = ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID);
	if (AllItems.IsEmpty())
		return;

	AllItems.Sort(SortByName);

	InvSlotsArray.Empty();
	FilteredInvSlotsArray.Empty();

	for (UObject* Item : AllItems)
	{
		if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("UpdateInvSlotsArray")))
			continue;

		UInventoryListEntry* EntryObject = NewObject<UInventoryListEntry>(this, EntryClass);
		EntryObject->Item = Item;

		auto IDs = ItemCollectionLinked->GetOccupatedSlotsIDByContainerName(InventoryContainerID, Item);
		if (!IDs.IsEmpty())
			EntryObject->SlotGuid = IDs[0];

		InvSlotsArray.Add(EntryObject);
	}

	NotifyReDrawRequest();
	OnRep_InventoryTotalWeight();
	OnRep_InventoryTotalMoney();
}

float UListInventory::GetInventoryOccupancyPercent()
{
	if (InventorySettings.MaxStackCount <= 0)
	{
		if (ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID).IsEmpty())
			return 0.0f;
		else
		{
			return 100.0f;
		}
	}

	int32 CurrentCount = ItemCollectionLinked->GetStackCountInContainer(InventoryContainerID);
	if (CurrentCount <= 0) return 0.0f;

	return static_cast<float>(CurrentCount) / InventorySettings.MaxStackCount * 100.0f;
}

UInventorySlotData* UListInventory::GetSlotByGuid(FGuid InGuid)
{
	if (!InGuid.IsValid())
		return nullptr;

	for (UInventorySlotData* Slot : InventorySlots)
	{
		if (!Slot)
			continue;

		if (Slot->InventorySlotInfo.SlotGuid == InGuid)
			return Slot;
	}

	return nullptr;
}

void UListInventory::RequestSplitStack(UObject* ItemToSplit, int32 SplitAmount)
{
	if (!ItemToSplit || SplitAmount <= 0)
		return;

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemToSplit, TEXT("RequestSplitStack")))
		return;

	const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(ItemToSplit);
	if (Quantity == 1 || Quantity <= SplitAmount)
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
	ItemMove.SavedOrientation = IObjectDataProvider::Execute_GetInitialItemOrientation(NewItem);
	ItemMove.TargetOrientation = ItemMove.SavedOrientation;

	OnSplitDelegate.Broadcast(this, ItemToSplit, SplitAmount);
}

void UListInventory::HandleRemoveItemsByID(FName ItemID, int32 RequestedAmount)
{
	if (ItemID.IsNone() || RequestedAmount <= 0)
		return;

	int32 RemainingToRemove = RequestedAmount;
	int32 RemovedTotal = 0;

	TArray<UObject*> FoundItems =
		ItemCollectionLinked->GetAllSameItemsInContainerByID(InventoryContainerID, ItemID);

	if (FoundItems.IsEmpty())
		return;

	for (UObject* Item : FoundItems)
	{
		if (!Item || RemainingToRemove <= 0)
			break;

		if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("HandleRemoveItemsByID")))
			continue;

		int32 Removed = TryRemoveFromStackItem(Item, RemainingToRemove);
		RemainingToRemove -= Removed;
		RemovedTotal += Removed;

		if (IObjectDataProvider::Execute_GetQuantity(Item) <= 0)
		{
			auto IDs = ItemCollectionLinked->GetOccupatedSlotsIDByContainerName(InventoryContainerID, Item);
			if (!IDs.IsEmpty())
			{
				const FGuid& SlotID = IDs[0];

				InventorySlots.RemoveAll(
					[&SlotID](const UInventorySlotData* Slot)
					{
						return Slot &&
							   Slot->InventorySlotInfo.SlotGuid == SlotID;
					});
			}
		}
	}
}

void UListInventory::HandleRemoveItemsBySample(UObject* ItemSample, int32 RequestedAmount)
{
	if (!ItemSample || RequestedAmount <= 0)
		return;

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemSample, TEXT("HandleRemoveItemsBySample")))
		return;

	int32 RemainingToRemove = RequestedAmount;
	int32 RemovedTotal = 0;

	TArray<UObject*> FoundItems =
		ItemCollectionLinked->GetAllSameItemsInContainerByItemSample(InventoryContainerID, ItemSample);

	if (FoundItems.IsEmpty())
		return;

	for (UObject* Item : FoundItems)
	{
		if (!Item || RemainingToRemove <= 0)
			break;

		if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("HandleRemoveItemsBySample")))
			continue;

		int32 Removed = TryRemoveFromStackItem(Item, RemainingToRemove);
		RemainingToRemove -= Removed;
		RemovedTotal += Removed;

		if (IObjectDataProvider::Execute_GetQuantity(Item) <= 0)
		{
			auto IDs = ItemCollectionLinked->GetOccupatedSlotsIDByContainerName(InventoryContainerID, Item);
			if (!IDs.IsEmpty())
			{
				const FGuid& SlotID = IDs[0];

				InventorySlots.RemoveAll(
					[&SlotID](const UInventorySlotData* Slot)
					{
						return Slot &&
							   Slot->InventorySlotInfo.SlotGuid == SlotID;
					});
			}
		}
	}
}

void UListInventory::HandleRemoveItem(UObject* Item, int32 RemoveQuantity)
{
	if (!Item ||
		!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("HandleRemoveItem")))
		return;

	int32 RemovedActual = TryRemoveFromStackItem(Item, RemoveQuantity);

	if (IObjectDataProvider::Execute_GetQuantity(Item) <= 0)
	{
		auto IDs = ItemCollectionLinked->GetOccupatedSlotsIDByContainerName(InventoryContainerID, Item);
		if (!IDs.IsEmpty())
		{
			const FGuid& SlotID = IDs[0];

			InventorySlots.RemoveAll(
				[&SlotID](const UInventorySlotData* Slot)
				{
					return Slot &&
						   Slot->InventorySlotInfo.SlotGuid == SlotID;
				});
		}
	}

	UpdateInvSlotsArray();
}

FItemAddResult UListInventory::HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
	 if (!ItemCollectionLinked)
        return FItemAddResult::AddedNone(FText::FromString("ItemCollectionLinked is null"));
    
    if (!ItemMoveData.SourceItem ||
        !UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemMoveData.SourceItem, TEXT("HandleAddItem")))
    {
        UE_LOG(LogTemp, Warning, TEXT("Item is null. Nothing to add"));
        return FItemAddResult::AddedNone(FText::FromString("Item is null. Nothing to add"));
    }

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

    if (ItemMoveData.SourceInventory == this)
    {
        return FItemAddResult::AddedNone(
            FText::Format(
                FText::FromString("Item {0} is already inside this inventory."),
                FText::FromName(IObjectDataProvider::Execute_GetItemID(ItemMoveData.SourceItem))));
    }

    if (InventorySettings.bIsReferenceContainer)
        return HandleAddReferenceItem(ItemMoveData, bOnlyCheck);

    if (!IObjectDataProvider::Execute_IsStackable(ItemMoveData.SourceItem))
        return HandleNonStackableItems(ItemMoveData, bOnlyCheck);

    if (IObjectDataProvider::Execute_IsStackable(ItemMoveData.SourceItem))
        return TryAddStackableItem(ItemMoveData, bOnlyCheck);

    return FItemAddResult::AddedNone(
        FText::Format(
            FText::FromString("Couldn't add {0} to inventory."),
            IObjectDataProvider::Execute_GetItemRef(ItemMoveData.SourceItem).ItemTextData.DisplayName));
}

void UListInventory::OnRep_InvSlotsArray()
{
	NotifyReDrawRequest();
}

FItemAddResult UListInventory::HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemMoveData.SourceItem, TEXT("HandleAddReferenceItem")))
		return FItemAddResult::AddedNone(FText::FromString("Invalid item"));

	if (ItemMoveData.SourceInventory == this)
	{
		return FItemAddResult::AddedNone(
			FText::Format(
				FText::FromString("Cannot move {0} of {1} within the same inventory."),
				1,
				IObjectDataProvider::Execute_GetItemRef(ItemMoveData.SourceItem).ItemTextData.DisplayName));
	}

	UInventorySlotData* NewSlot = UInventorySlotData::Create(this);

	FItemMapping Slots;
	Slots.InventoryID = InventoryContainerID;
	Slots.OccupiedSlots.Add(NewSlot);

	if (!bOnlyCheck)
	{
		AddNewItem(ItemMoveData, Slots, IObjectDataProvider::Execute_GetQuantity(ItemMoveData.SourceItem));
		InventorySlots.Add(NewSlot);
	}

	TMap<UInventorySlotData*, FItemPlacementData> AffectedSlots;
	AffectedSlots.Add(NewSlot, FItemPlacementData());

	return FItemAddResult::AddedAll(
		1,
		true,
		FText::Format(
			FText::FromString("Successfully added {0} to inventory as a reference"),
			FText::FromName(IObjectDataProvider::Execute_GetItemID(ItemMoveData.SourceItem))),
		AffectedSlots);
}

FItemAddResult UListInventory::HandleNonStackableItems(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemMoveData.SourceItem, TEXT("HandleNonStackableItems")))
		return FItemAddResult::AddedNone(FText::FromString("Invalid item"));

	int32 ActualAmountToAdd =
		CalculateActualAmountToAdd(1, IObjectDataProvider::Execute_GetItemSingleWeight(ItemMoveData.SourceItem));

	if (ActualAmountToAdd <= 0)
	{
		return FItemAddResult::AddedNone(
			FText::Format(
				FText::FromString("Item {0} would overflow limits"),
				IObjectDataProvider::Execute_GetItemRef(ItemMoveData.SourceItem).ItemTextData.DisplayName));
	}

	UInventorySlotData* NewSlot = UInventorySlotData::Create(this);

	if (!bOnlyCheck)
	{
		FItemMapping Slots;
		Slots.InventoryID = InventoryContainerID;
		Slots.OccupiedSlots.Add(NewSlot);

		AddNewItem(ItemMoveData, Slots, 1);
		InventorySlots.Add(NewSlot);
	}

	TMap<UInventorySlotData*, FItemPlacementData> AffectedSlots;
	AffectedSlots.Add(NewSlot, FItemPlacementData());

	return FItemAddResult::AddedAll(
		1,
		false,
		FText::Format(
			FText::FromString("Successfully added {0} of {1} to inventory"),
			1,
			IObjectDataProvider::Execute_GetItemRef(ItemMoveData.SourceItem).ItemTextData.DisplayName),
		AffectedSlots);
}

FItemAddResult UListInventory::TryAddStackableItem(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemMoveData.SourceItem, TEXT("TryAddStackableItem")))
		return FItemAddResult::AddedNone(FText::FromString("Invalid item"));

	TMap<UInventorySlotData*, FItemPlacementData> AffectedSlots;

	const int32 InitialRequestedAddAmount =
		IObjectDataProvider::Execute_GetQuantity(ItemMoveData.SourceItem);

	const int32 StackableAmountAdded =
		HandleStackableItems(ItemMoveData, InitialRequestedAddAmount, bOnlyCheck, AffectedSlots);

	if (StackableAmountAdded == InitialRequestedAddAmount)
	{
		return FItemAddResult::AddedAll(
			StackableAmountAdded,
			false,
			FText::Format(
				FText::FromString("Successfully added {0} of {1} to inventory"),
				InitialRequestedAddAmount,
				IObjectDataProvider::Execute_GetItemRef(ItemMoveData.SourceItem).ItemTextData.DisplayName),
			AffectedSlots);
	}
	else if (StackableAmountAdded < InitialRequestedAddAmount && StackableAmountAdded > 0)
	{
		return FItemAddResult::AddedPartial(
			StackableAmountAdded,
			false,
			FText::Format(
				FText::FromString("Partial amount of {0} added to inventory. Number added: {1}"),
				IObjectDataProvider::Execute_GetItemRef(ItemMoveData.SourceItem).ItemTextData.DisplayName,
				StackableAmountAdded),
			AffectedSlots);
	}

	return FItemAddResult::AddedNone(
		FText::Format(
			FText::FromString("Couldn't add {0} to inventory."),
			IObjectDataProvider::Execute_GetItemRef(ItemMoveData.SourceItem).ItemTextData.DisplayName));
}

int32 UListInventory::HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount, bool bOnlyCheck,
	TMap<UInventorySlotData*, FItemPlacementData>& AffectedPivotSlots)
{
	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemMoveData.SourceItem, TEXT("HandleStackableItems")))
		return 0;

	int32 AmountToDistribute = RequestedAddAmount;
	int32 TotalAddedAmount = 0;

	auto SameItems =
		ItemCollectionLinked->GetAllSameItemsInContainerByItemSample(InventoryContainerID, ItemMoveData.SourceItem);

	for (UObject* SameItem : SameItems)
	{
		if (!SameItem ||
			!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(SameItem, TEXT("HandleStackableItems")))
			continue;

		if (AmountToDistribute <= 0)
			break;

		int32 ActualAmountToAdd =
			TryInsertToStackItem(SameItem, AmountToDistribute, bOnlyCheck);

		AmountToDistribute -= ActualAmountToAdd;
		TotalAddedAmount += ActualAmountToAdd;
	}

	if (AmountToDistribute <= 0)
		return RequestedAddAmount;

	const int32 AmountToAddToStack =
		FMath::Min(AmountToDistribute,
				   IObjectDataProvider::Execute_GetQuantity(ItemMoveData.SourceItem));

	int32 ActualAmountToAdd =
		CalculateActualAmountToAdd(
			AmountToAddToStack,
			IObjectDataProvider::Execute_GetItemSingleWeight(ItemMoveData.SourceItem));

	if (bOnlyCheck)
		return RequestedAddAmount;

	UInventorySlotData* NewSlot = UInventorySlotData::Create(this);

	FItemMapping Slots;
	Slots.InventoryID = InventoryContainerID;
	Slots.OccupiedSlots.Add(NewSlot);

	AddNewItem(ItemMoveData, Slots, AmountToDistribute);

	return ActualAmountToAdd + TotalAddedAmount;
}

UObject* UListInventory::AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount)
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

	FItemMapping StoredMappingCopy =
		ItemCollectionLinked->AddItem(FinalItem, OccupiedSlots);

	NotifyAddNewItem(StoredMappingCopy, FinalItem, IObjectDataProvider::Execute_GetQuantity(ItemMoveData.SourceItem));

	UpdateInvSlotsArray();
	UpdateMoneyInfo();
	UpdateWeightInfo();

	return FinalItem;
}

void UListInventory::UpdateInvSlotsArray()
{
	if (!InventoryOwnerActor || !InventoryOwnerActor->HasAuthority()) return; 

	if (!ItemCollectionLinked) return;

	InvSlotsArray.Reset();

	auto AllItems = ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID);
	if (AllItems.IsEmpty()) return;

	for (auto Item : AllItems)
	{
		UInventoryListEntry* EntryObject = NewObject<UInventoryListEntry>(this, EntryClass);
		EntryObject->Item = Item;
		
		auto IDs = ItemCollectionLinked->GetOccupatedSlotsIDByContainerName(InventoryContainerID, Item);
		if (!IDs.IsEmpty())
		{
			EntryObject->SlotGuid = IDs[0];
		}		
		InvSlotsArray.Add(EntryObject);
	}
	
	OnRep_InvSlotsArray();
}

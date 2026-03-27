//  Nublin Studio 2026 All Rights Reserved.


#include "Data/Inventory/InventoryBase.h"

#include "ActorComponents/ItemCollection.h"
#include "Data/Items/ItemBase.h"
#include "Factory/ItemFactory.h"

UInventoryBase::UInventoryBase()
{
	if (InventoryContainerID.IsEmpty() || InventoryContainerID == "")
	{
		FString UniqueString = FGuid::NewGuid().ToString(EGuidFormats::Short);
		InventoryContainerID = UniqueString;
	}
}

void UInventoryBase::SetupStartingResources()
{
	for (auto InitResource : InitialItems)
	{
		if (InitResource.Item.RowName.IsNone()) continue;

		FItemMoveData Data;
		Data.TargetInventory = this;
		UItemBase* NewItem = UItemFactory::CreateItemByHandle(
			this,
			InitResource.Item,
			InitResource.Amount
		);

		if (!NewItem) continue;

		Data.SourceItem = NewItem;
		if (Data.SourceItem->GetItemRef().ItemNumeraticData.InventoryVerticalSlots > Data.SourceItem->GetItemRef().ItemNumeraticData.InventoryHorizontalSlots)
		{
			Data.SavedOrientation = EItemOrientationType::Vertical;
			Data.TargetOrientation = EItemOrientationType::Vertical;
		}
		HandleAddItem(Data);
	}
}

void UInventoryBase::MergeStackableItems()
{
	auto Items = ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID);
	if (Items.IsEmpty()) return;

	for (int i = Items.Num() - 1; i > 0; --i)
	{
		if (!Items[i] || !Items[i]->IsStackable())
		{
			continue;
		}

		auto SameItems = ItemCollectionLinked->GetAllSameItemsInContainer(InventoryContainerID, Items[i]);
		if (SameItems.IsEmpty()) continue;

		for (const auto& SameItem : SameItems)
		{
			if (SameItem == Items[i])
				continue;
			auto AddedQuantity = TryInsertToStackItem(SameItem, Items[i], Items[i]->GetQuantity(), false);
		}
	}
}

void UInventoryBase::UseSlot(UInventorySlotData* UsedSlot)
{
	if (!UsedSlot || !ItemCollectionLinked)
		return;

	auto ItemLinked = ItemCollectionLinked->GetItemFromSlot(UsedSlot, InventoryContainerID);

	ItemLinked->UseItem();

	NotifyUseSlot(UsedSlot);
}

void UInventoryBase::UpdateWeightInfo()
{
	if (!ItemCollectionLinked)
		return;

	InventoryTotalWeight = 0;
	auto AllItems = ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID);
	if (AllItems.IsEmpty())
	{
		NotifyUpdateWeight();
	}
	else
	{
		for (auto Item : AllItems)
		{
			InventoryTotalWeight += Item->GetQuantity() * Item->GetItemSingleWeight();
		}

		InventoryTotalWeight = FMath::RoundToFloat(InventoryTotalWeight * 100.0f) / 100.0f;
		NotifyUpdateWeight();
	}
}

void UInventoryBase::UpdateMoneyInfo()
{
	if (!ItemCollectionLinked)
		return;
	
	auto AllItems = ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID);
	if (AllItems.IsEmpty())
	{
		InventoryTotalMoney = 0;
		NotifyUpdateMoney();
	}
	else
	{
		for (auto Item : AllItems)
		{
			if (Item->GetItemRef().ItemCategory == EItemCategory::Money)
				InventoryTotalMoney += Item->GetQuantity();
		}

		NotifyUpdateMoney();
	}
}

int32 UInventoryBase::CalculateActualAmountToAdd(int32 InAmountToAdd, float ItemSingleWeight)
{
	if (InventorySettings.InventoryMaxWeightCapacity >= 0)
	{
		const int32 WeightLimitAddAmount = InventorySettings.InventoryMaxWeightCapacity - InventoryTotalWeight;
		int32 MaxItemsThatFit = WeightLimitAddAmount / ItemSingleWeight;
		return FMath::Min(MaxItemsThatFit, InAmountToAdd);
	}

	if (InventorySettings.MaxUniqueItemCount >= 0)
	{
		const int32 TotalCount = ItemCollectionLinked->GetTotalItemCountInContainer(InventoryContainerID);
		const int32 RemainingSlots = InventorySettings.MaxUniqueItemCount - TotalCount;
		if (RemainingSlots <= 0)
		{
			return 0;
		}
		return FMath::Min(RemainingSlots, InAmountToAdd);
	}
	
	return InAmountToAdd;
}

int32 UInventoryBase::TryInsertToStackItem(UItemBase* ResourceToInsertInto, UItemBase* ResourceToDeductFrom,
	int32 AmountToDistribute, bool bOnlyCheck)
{
	if (ResourceToInsertInto->IsFullItemStack())
		return 0;

	int32 AmountToAddToStack = FMath::Min(AmountToDistribute,
										  ResourceToInsertInto->GetItemRef().ItemNumeraticData.MaxStackSizeInCharacter -
										  ResourceToInsertInto->GetQuantity());
	
	int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack, ResourceToInsertInto->GetItemSingleWeight());
	int32 OldAmount = ResourceToInsertInto->GetQuantity();

	if (!bOnlyCheck)
	{
		DeductResourceOnAddToInventory(ResourceToDeductFrom, ActualAmountToAdd);
		ResourceToInsertInto->SetQuantity(OldAmount + ActualAmountToAdd);
		NotifyAddItemToStack(ResourceToInsertInto, ActualAmountToAdd);
	}
	//ActualAmountToAdd = OldAmount + ActualAmountToAdd;

	return ActualAmountToAdd;
}

int32 UInventoryBase::TryRemoveFromStackItem(UItemBase* Item, int32 RequestedRemoveAmount)
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

void UInventoryBase::RemoveItemFromInventory(UItemBase* Item)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemFromInventory: Item is null"));
		return;
	}

	auto MappingWrapper = ItemCollectionLinked->GetItemLocations().FindRef(TObjectPtr<UItemBase>(Item));
	auto Mapping = ItemCollectionLinked->FindItemMappingByContainerName(Item, InventoryContainerID);

	NotifyFullyRemoveItem(*Mapping, Item);

	ItemCollectionLinked->RemoveItem(TObjectPtr<UItemBase>(Item), InventoryContainerID);
}

void UInventoryBase::DeductResourceOnAddToInventory(UItemBase* Resource, int32 DeductAmount)
{
	Resource->SetQuantity(Resource->GetQuantity() - DeductAmount);
	if (Resource->GetQuantity() <= 0)
	{
		ItemCollectionLinked->RemoveItem(Resource, InventoryContainerID);
	}
}

void UInventoryBase::NotifyAddNewItem(FItemMapping& FromSlots, UItemBase* NewItem, int32 ChangeQuantity)
{
	if (OnAddItemDelegate.IsBound())
		OnAddItemDelegate.Broadcast(FromSlots, NewItem);
}

void UInventoryBase::NotifyAddItemToStack(UItemBase* Item, int32 ChangeQuantity)
{
	if (OnStackedItemDelegate.IsBound())
		OnStackedItemDelegate.Broadcast(Item, ChangeQuantity);
}

void UInventoryBase::NotifyRemoveItemFromStack(UItemBase* Item, int32 ChangeQuantity)
{
	if (OnUnstackedItemDelegate.IsBound())
		OnUnstackedItemDelegate.Broadcast(Item, ChangeQuantity);
}

void UInventoryBase::NotifyFullyRemoveItem(FItemMapping& FromSlots, UItemBase* Item)
{
	if (OnItemRemovedDelegate.IsBound())
		OnItemRemovedDelegate.Broadcast(FromSlots, Item);
}

void UInventoryBase::NotifyReplaceItem(TArray<UInventorySlotData*> OldItemSlots,
	FItemMapping& NewItemSlots, UItemBase* Item)
{
	if (OnItemReplaceDelegate.IsBound())
		OnItemReplaceDelegate.Broadcast(OldItemSlots, NewItemSlots, Item);
}

void UInventoryBase::NotifyUseSlot(UInventorySlotData* UsedSlot)
{
	if (OnUseSlotDelegate.IsBound())
		OnUseSlotDelegate.Broadcast(UsedSlot);
}

void UInventoryBase::NotifyUpdateWeight()
{
	if (OnWeightUpdatedDelegate.IsBound())
		OnWeightUpdatedDelegate.Broadcast(InventoryTotalWeight);
}

void UInventoryBase::NotifyUpdateMoney()
{
	if (OnMoneyUpdatedDelegate.IsBound())
		OnMoneyUpdatedDelegate.Broadcast(InventoryTotalMoney);
}

void UInventoryBase::NotifyReDrawRequest()
{
	if (OnInventoryRedrawRequested.IsBound())
		OnInventoryRedrawRequested.Broadcast();
}

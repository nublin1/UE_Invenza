//  Nublin Studio 2026 All Rights Reserved.


#include "Data/Inventory/InventoryBase.h"

#include "ActorComponents/ItemCollection.h"
#include "Data/Items/ItemBase.h"

UInventoryBase::UInventoryBase()
{
	if (InventoryContainerID.IsEmpty() || InventoryContainerID == "")
	{
		FString UniqueString = FGuid::NewGuid().ToString(EGuidFormats::Short);
		InventoryContainerID = UniqueString;
	}
}

void UInventoryBase::UseSlot(UInventorySlotData* UsedSlot)
{
	if (!UsedSlot)
		return;

	UsedSlot->ItemLinked->UseItem();

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

void UInventoryBase::InitInventory(UItemCollection* ItemCollectionRef, FVector2D NewSize)
{
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
	FItemMapping NewItemSlots, UItemBase* Item)
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

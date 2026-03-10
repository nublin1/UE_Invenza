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

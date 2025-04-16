//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/UInventoryWidgetBase.h"

#include "ActorComponents/ItemCollection.h"

FItemMapping* UUInventoryWidgetBase::GetItemMapping(UItemBase* Item)
{
	if (!Item)
	{
		return nullptr;
	}
	auto Mapping = ItemCollectionLink->FindItemMappingForItemInContainer(Item, this);
	return Mapping;
}

int32 UUInventoryWidgetBase::CalculateActualAmountToAdd(int32 InAmountToAdd, float ItemSingleWeight)
{
	if (InventoryWeightCapacity >= 0)
	{
		const int32 WeightLimitAddAmount = InventoryWeightCapacity - InventoryTotalWeight;
		int32 MaxItemsThatFit = WeightLimitAddAmount / ItemSingleWeight;
		return FMath::Min(MaxItemsThatFit, InAmountToAdd);
	}
	return InAmountToAdd;
}

void UUInventoryWidgetBase::InsertToStackItem(UItemBase* Item, int32 AddQuantity)
{
	Item->SetQuantity(Item->GetQuantity() + AddQuantity);
	auto Slots = GetItemMapping(Item);
	if (Slots)
		NotifyAddItem(*Slots, Item, AddQuantity);
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to find occupied slots for item %s"), *Item->GetName());
	}
}

void UUInventoryWidgetBase::UpdateWeightInfo()
{
	if (OnWightUpdatedDelegate.IsBound() && ItemCollectionLink)
	{
		InventoryTotalWeight = 0;
		auto AllItems = ItemCollectionLink->GetAllItemsByContainer(this);
		if (AllItems.IsEmpty())
		{
			OnWightUpdatedDelegate.Broadcast(0, InventoryWeightCapacity);
		}
		else
		{
			for (auto Item : AllItems)
			{
				InventoryTotalWeight += Item->GetQuantity() * Item->GetItemSingleWeight();
			}

			InventoryTotalWeight = FMath::RoundToFloat(InventoryTotalWeight * 100.0f) / 100.0f;
			OnWightUpdatedDelegate.Broadcast(InventoryTotalWeight, InventoryWeightCapacity);
		}
	}
}

void UUInventoryWidgetBase::UpdateMoneyInfo()
{
	if (OnMoneyUpdatedDelegate.IsBound() && ItemCollectionLink)
	{
		InventoryTotalMoney = 0;
	}
	auto AllItems = ItemCollectionLink->GetAllItemsByContainer(this);
	if (AllItems.IsEmpty())
	{
		OnMoneyUpdatedDelegate.Broadcast(0);
	}
	else
	{
		for (auto Item : AllItems)
		{
			if (EnumHasAnyFlags(static_cast<EItemCategory>(Item->GetItemRef().ItemCategory), EItemCategory::Money))
				InventoryTotalMoney += Item->GetQuantity();
		}

		OnMoneyUpdatedDelegate.Broadcast(InventoryTotalMoney);
	}
}

void UUInventoryWidgetBase::NotifyAddItem(FItemMapping& FromSlots, UItemBase* NewItem, int32 ChangeQuantity)
{
	UpdateMoneyInfo();
	if (OnAddItemDelegate.IsBound())
		OnAddItemDelegate.Broadcast(FromSlots, NewItem);
}

/*void UUInventoryWidgetBase::NotifyUpdateItem(FItemMapping FromSlots, UItemBase* UpdatedItem)
{
	if (OnItemUpdateDelegate.IsBound())
		OnItemUpdateDelegate.Broadcast(FromSlots, UpdatedItem);
}*/

void UUInventoryWidgetBase::NotifyRemoveItem(FItemMapping& FromSlots, UItemBase* RemovedItem, int32 RemoveQuantity)
{
	if (OnRemoveItemDelegate.IsBound())
		OnRemoveItemDelegate.Broadcast(FromSlots, RemovedItem);
	
	
}


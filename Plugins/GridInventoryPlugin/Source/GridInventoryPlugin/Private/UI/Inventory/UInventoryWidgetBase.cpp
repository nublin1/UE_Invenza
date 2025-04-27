//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/UInventoryWidgetBase.h"

#include "ActorComponents/ItemCollection.h"
#include "Factory/ItemFactory.h"


void UUInventoryWidgetBase::ChangeItemCollectionLink(UItemCollection* NewItemCollection)
{
	InventoryData.ItemCollectionLink = NewItemCollection;
	
	if (NewItemCollection->InitItems.Num() > 0)
	{
		for (const auto& Item : NewItemCollection->InitItems)
		{
			FItemMoveData ItemMoveData;
			ItemMoveData.TargetInventory = this;
			ItemMoveData.SourceItem = UItemFactory::CreateItemByID(this, Item.ItemName,
																	 Item.ItemCount);
			if (ItemMoveData.SourceItem)
			{
				HandleAddItem(ItemMoveData);
			}
		}

		NewItemCollection->InitItems.Empty();
	}

	
	ReDrawAllItems();
}

void UUInventoryWidgetBase::UseSlot(UInventorySlot* UsedSlot)
{
	auto Item = InventoryData.ItemCollectionLink->GetItemFromSlot(UsedSlot, this);
	if (!Item)
		return;
	
	Item->UseItem();
}

bool UUInventoryWidgetBase::ExecuteItemChecks(EInventoryCheckType CheckType, UItemBase* Item)
{
	if (InventoryData.Checks.IsEmpty())
		return true;
	
	for (const FInventoryCheck& Check : InventoryData.Checks)
	{
		if (Check.CheckType == CheckType)
		{
			if (!Check.CheckFunction(Item))
			{
				return false;
			}
		}
	}
	return true;
}

FItemMapping* UUInventoryWidgetBase::GetItemMapping(UItemBase* Item)
{
	if (!Item)
	{
		return nullptr;
	}
	auto Mapping = InventoryData.ItemCollectionLink->FindItemMappingForItemInContainer(Item, this);
	return Mapping;
}

int32 UUInventoryWidgetBase::CalculateActualAmountToAdd(int32 InAmountToAdd, float ItemSingleWeight)
{
	if (InventorySettings.InventoryWeightCapacity >= 0)
	{
		const int32 WeightLimitAddAmount = InventorySettings.InventoryWeightCapacity - InventoryData.InventoryTotalWeight;
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
	if (OnWightUpdatedDelegate.IsBound() && InventoryData.ItemCollectionLink)
	{
		InventoryData.InventoryTotalWeight = 0;
		auto AllItems = InventoryData.ItemCollectionLink->GetAllItemsByContainer(this);
		if (AllItems.IsEmpty())
		{
			OnWightUpdatedDelegate.Broadcast(0, InventorySettings.InventoryWeightCapacity);
		}
		else
		{
			for (auto Item : AllItems)
			{
				InventoryData.InventoryTotalWeight += Item->GetQuantity() * Item->GetItemSingleWeight();
			}

			InventoryData.InventoryTotalWeight = FMath::RoundToFloat(InventoryData.InventoryTotalWeight * 100.0f) / 100.0f;
			OnWightUpdatedDelegate.Broadcast(InventoryData.InventoryTotalWeight, InventorySettings.InventoryWeightCapacity);
		}
	}
}

void UUInventoryWidgetBase::UpdateMoneyInfo()
{
	if (!InventoryData.ItemCollectionLink)
	{
		InventoryData.InventoryTotalMoney = 0;
		OnMoneyUpdatedDelegate.Broadcast(InventoryData.InventoryTotalMoney);
		return;
	}
	
	if (OnMoneyUpdatedDelegate.IsBound() && InventoryData.ItemCollectionLink)
	{
		InventoryData.InventoryTotalMoney = 0;
	}
	auto AllItems = InventoryData.ItemCollectionLink->GetAllItemsByContainer(this);
	if (AllItems.IsEmpty())
	{
		OnMoneyUpdatedDelegate.Broadcast(0);
	}
	else
	{
		for (auto Item : AllItems)
		{
			if (EnumHasAnyFlags(static_cast<EItemCategory>(Item->GetItemRef().ItemCategory), EItemCategory::Money))
				InventoryData.InventoryTotalMoney += Item->GetQuantity();
		}

		OnMoneyUpdatedDelegate.Broadcast(InventoryData.InventoryTotalMoney);
	}
}

void UUInventoryWidgetBase::NotifyAddItem(FItemMapping& FromSlots, UItemBase* NewItem, int32 ChangeQuantity)
{
	UpdateWeightInfo();
	UpdateMoneyInfo();
	if (OnAddItemDelegate.IsBound())
		OnAddItemDelegate.Broadcast(FromSlots, NewItem);
}

void UUInventoryWidgetBase::NotifyRemoveItem(FItemMapping& FromSlots, UItemBase* RemovedItem, int32 RemoveQuantity)
{
	UpdateWeightInfo();
	UpdateMoneyInfo();
	if (OnRemoveItemDelegate.IsBound())
		OnRemoveItemDelegate.Broadcast(FromSlots, RemovedItem, RemoveQuantity);
	
}

void UUInventoryWidgetBase::NotifyUseSlot(UInventorySlot* UsedSlot)
{
	if (OnUseSlotDelegate.IsBound())
		OnUseSlotDelegate.Broadcast(UsedSlot);
}


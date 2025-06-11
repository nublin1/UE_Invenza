//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/UInventoryWidgetBase.h"

#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Factory/ItemFactory.h"
#include "UI/Inventory/InventorySlot.h"

void UUInventoryWidgetBase::InitItemsInItemsCollection()
{
	if (!InventoryData.ItemCollectionLink)
		return;
	
	if (InventoryData.ItemCollectionLink->InitItems.Num() > 0)
	{
		for (const auto& Item : InventoryData.ItemCollectionLink->InitItems)
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

		InventoryData.ItemCollectionLink->InitItems.Empty();
	}
	
	ReDrawAllItems();
}

void UUInventoryWidgetBase::UseSlot(UInventorySlot* UsedSlot)
{
	auto Item = InventoryData.ItemCollectionLink->GetItemFromSlot(UsedSlot->GetSlotData(), GetAsContainerWidget());
	if (!Item)
		return;
	
	Item->UseItem();

	NotifyUseSlot(UsedSlot);
}

void UUInventoryWidgetBase::MergeStackableItems()
{
}

FItemMapping* UUInventoryWidgetBase::GetItemMapping(UItemBase* Item)
{
	if (!Item)
	{
		return nullptr;
	}
	auto Mapping = InventoryData.ItemCollectionLink->FindItemMappingForItemInContainer(Item, GetAsContainerWidget());
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
		auto AllItems = InventoryData.ItemCollectionLink->GetAllItemsByContainer(GetAsContainerWidget());
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
	auto AllItems = InventoryData.ItemCollectionLink->GetAllItemsByContainer(GetAsContainerWidget());
	if (AllItems.IsEmpty())
	{
		OnMoneyUpdatedDelegate.Broadcast(0);
	}
	else
	{
		for (auto Item : AllItems)
		{
			if (Item->GetItemRef().ItemCategory == EItemCategory::Money)
				InventoryData.InventoryTotalMoney += Item->GetQuantity();
		}

		OnMoneyUpdatedDelegate.Broadcast(InventoryData.InventoryTotalMoney);
	}
}

bool UUInventoryWidgetBase::HandleTradeModalOpening(UItemBase* Item)
{
	if (!Item) return false;

	if (Item->GetItemRef().ItemCategory == EItemCategory::Money) return false;
	
	UIInventoryManager* InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();
	
	if (InventoryManager->GetCurrentInteractInvWidget()
			&& InventoryManager->GetCurrentInteractInvWidget()->GetInventoryType() == EInventoryType::VendorInventory)
	{
		if (InventoryData.ItemCollectionLink->GetOwner() == Cast<APawn>(GetOwningPlayer()->GetPawn()))
		{
			InventoryManager->OpenTradeModal(false, Item);
			return true;
		}
				
		InventoryManager->OpenTradeModal(true, Item);
		return true;
	}
	return false;
}

void UUInventoryWidgetBase::NotifyAddItem(FItemMapping& FromSlots, UItemBase* NewItem, int32 ChangeQuantity)
{
	UpdateWeightInfo();
	UpdateMoneyInfo();
	if (OnAddItemDelegate.IsBound())
		OnAddItemDelegate.Broadcast(FromSlots, NewItem);
}

void UUInventoryWidgetBase::NotifyPreRemoveItem(FItemMapping& FromSlots, UItemBase* RemovedItem, int32 RemoveQuantity)
{
	if (OnPreRemoveItemDelegate.IsBound())
		OnPreRemoveItemDelegate.Broadcast(FromSlots, RemovedItem, RemoveQuantity);
	
}

void UUInventoryWidgetBase::NotifyPostRemoveItem()
{
	UpdateWeightInfo();
	UpdateMoneyInfo();
	if (OnPostRemoveItemDelegate.IsBound())
		OnPostRemoveItemDelegate.Broadcast();
}

void UUInventoryWidgetBase::NotifyUseSlot(UInventorySlot* UsedSlot)
{
	if (OnUseSlotDelegate.IsBound())
		OnUseSlotDelegate.Broadcast(UsedSlot);
}

void UUInventoryWidgetBase::NotifyReplaceItem(TArray<FInventorySlotData> OldItemSlots, TArray<FInventorySlotData> NewItemSlots, UItemBase* Item)
{
	if (OnItemReplaceDelegate.IsBound())
		OnItemReplaceDelegate.Broadcast(OldItemSlots, NewItemSlots, Item);
}


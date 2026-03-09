//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/UInventoryWidgetBase.h"

#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Factory/ItemFactory.h"
#include "UI/Inventory/InventorySlot.h"

UUInventoryWidgetBase::UUInventoryWidgetBase()
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
	if (InventorySettings.InventoryMaxWeightCapacity >= 0)
	{
		const int32 WeightLimitAddAmount = InventorySettings.InventoryMaxWeightCapacity - InventoryData.InventoryTotalWeight;
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
			OnWightUpdatedDelegate.Broadcast(0, InventorySettings.InventoryMaxWeightCapacity);
		}
		else
		{
			for (auto Item : AllItems)
			{
				InventoryData.InventoryTotalWeight += Item->GetQuantity() * Item->GetItemSingleWeight();
			}

			InventoryData.InventoryTotalWeight = FMath::RoundToFloat(InventoryData.InventoryTotalWeight * 100.0f) / 100.0f;
			OnWightUpdatedDelegate.Broadcast(InventoryData.InventoryTotalWeight, InventorySettings.InventoryMaxWeightCapacity);
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



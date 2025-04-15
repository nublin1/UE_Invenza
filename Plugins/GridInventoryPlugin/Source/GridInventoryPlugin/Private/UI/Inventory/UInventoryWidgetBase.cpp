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
		NotifyUpdateItem(Slots, Item);
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

			OnWightUpdatedDelegate.Broadcast(InventoryTotalWeight, InventoryWeightCapacity);
		}
	}
}
FString ToBinaryString(int32 Value)
{
	FString BinaryStr;
	for (int32 i = 7; i >= 0; --i) // 8 бит (можешь расширить до 32)
	{
		BinaryStr.AppendChar(((Value >> i) & 1) ? '1' : '0');
	}
	return BinaryStr;
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
			auto PrintCategoryBits = [](FString Name, int32 Value)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
					FString::Printf(TEXT("%s = %d (Bits: 0b%s)"), *Name, Value, *ToBinaryString(Value)));
			};

			if (EnumHasAnyFlags(static_cast<EItemCategory>(Item->GetItemRef().ItemCategory), EItemCategory::Money))
				InventoryTotalMoney += Item->GetQuantity();
		}

		OnMoneyUpdatedDelegate.Broadcast(InventoryTotalMoney);
	}
}

void UUInventoryWidgetBase::NotifyAddItem(FItemMapping& FromSlots, UItemBase* NewItem)
{
	UpdateMoneyInfo();
}

void UUInventoryWidgetBase::NotifyUpdateItem(FItemMapping* FromSlots, UItemBase* NewItem)
{
}

void UUInventoryWidgetBase::NotifyRemoveItem(FItemMapping& FromSlots, UItemBase* RemovedItem)
{
}


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

UInventoryBase* UInventoryBase::CreateInventory(UObject* Outer, FInventoryStartupData StartupData)
{
	if (!StartupData.Settings.InventoryClass)
	{
		return nullptr;
	}
	
	UInventoryBase* Inventory =
			NewObject<UInventoryBase>(Outer, StartupData.Settings.InventoryClass);

	if (!Inventory)
		return nullptr;

	Inventory->SetInventorySettings(StartupData.Settings);

	return Inventory;
}

UInventoryBase* UInventoryBase::DuplicateInventory(UObject* Outer)
{
	UInventoryBase* Inventory =
			NewObject<UInventoryBase>(Outer, this->GetClass());

	if (!Inventory)
		return nullptr;

	Inventory->SetInventorySettings(InventorySettings);
	Inventory->SetItemCollectionLink(ItemCollectionLinked);
	Inventory->SetInventorySize(InvSize);

	return Inventory;
}

void UInventoryBase::InitInventory()
{
	InvSize = FVector2D(InventorySettings.InventorySlotBasedSettings.NumberRows, InventorySettings.InventorySlotBasedSettings.NumColumns);
}

void UInventoryBase::InitInventoryWithSettings(FInventorySettings NewInventorySettings)
{
	InventorySettings = NewInventorySettings;
	InitInventory();
}

void UInventoryBase::RequestToResetItemVisual(UItemBase* Item)
{
	if (!Item) return;

	if (ItemCollectionLinked->ItemHasInventory(Item, InventoryContainerID))
		NotifyRequestToResetItemVisual(Item);
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
			auto AddedQuantity = TryInsertToStackItem(SameItem, Items[i]->GetQuantity(), false);
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

	InventoryTotalMoney = 0;
	
	auto AllItems = ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID);
	if (AllItems.IsEmpty())
	{
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

int32 UInventoryBase::TryInsertToStackItem(UItemBase* ResourceToInsertInto,	int32 AmountToDistribute, bool bOnlyCheck)
{
	if (ResourceToInsertInto->IsFullItemStack())
		return 0;

	int32 AmountToAddToStack = FMath::Min(AmountToDistribute,
										  ResourceToInsertInto->GetItemRef().ItemNumeraticData.MaxStackSizeInCharacter -
										  ResourceToInsertInto->GetQuantity());
	
	int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack, ResourceToInsertInto->GetItemSingleWeight());

	if (!bOnlyCheck)
	{
		int32 OldAmount = ResourceToInsertInto->GetQuantity();
		//DeductResourceOnAddToInventory(ResourceToDeductFrom, ActualAmountToAdd);
		ResourceToInsertInto->SetQuantity(OldAmount + ActualAmountToAdd);
		NotifyAddItemToStack(ResourceToInsertInto, ActualAmountToAdd);
	}
	
	return ActualAmountToAdd;
}

int32 UInventoryBase::TryRemoveFromStackItem(UItemBase* Item, int32 RequestedRemoveAmount)
{
	if (!Item || RequestedRemoveAmount <= 0)
		return 0;

	int32 AmountToRemove = FMath::Min(RequestedRemoveAmount, Item->GetQuantity());
	if (AmountToRemove <= 0)
	{
		RemoveItemFromInventory(Item);
		return 0;
	}	

	if (Item->GetQuantity() - AmountToRemove <= 0)
	{
		RemoveItemFromInventory(Item);
	}
	else
	{
		Item->SetQuantity(Item->GetQuantity() - AmountToRemove);
	}
	
	NotifyRemoveItemFromStack(Item, AmountToRemove);
	//NotifyReDrawRequest();

	return AmountToRemove;
}

void UInventoryBase::RemoveItemFromInventory(UItemBase* Item)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemFromInventory: Item is null"));
		return;
	}
	
	auto Mapping = ItemCollectionLinked->FindItemMappingByContainerName(Item, InventoryContainerID);
	if (!Mapping)
		return;

	NotifyFullyRemoveItem(*Mapping, Item);

	ItemCollectionLinked->RemoveItem(Item, InventoryContainerID);

	UpdateWeightInfo();
	UpdateMoneyInfo();
}

/*void UInventoryBase::DeductResourceOnAddToInventory(UItemBase* Resource, int32 DeductAmount)
{
	Resource->SetQuantity(Resource->GetQuantity() - DeductAmount);
	if (Resource->GetQuantity() <= 0)
	{
		ItemCollectionLinked->RemoveItem(Resource, InventoryContainerID);
	}
}*/

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

void UInventoryBase::NotifyFullyRemoveItem(FItemMapping FromSlots, UItemBase* Item)
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

void UInventoryBase::NotifyRequestToResetItemVisual(UItemBase* Item)
{
	if (OnRequestToResetItemVisual.IsBound())
		OnRequestToResetItemVisual.Broadcast(Item);
}

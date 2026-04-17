//  Nublin Studio 2026 All Rights Reserved.


#include "Data/Inventory/InventoryBase.h"

#include "ActorComponents/ItemCollection.h"
#include "Data/Items/ItemBase.h"
#include "Factory/ItemFactory.h"
#include "Net/UnrealNetwork.h"

UInventoryBase::UInventoryBase()
{
	if (InventoryContainerID.IsEmpty() || InventoryContainerID == "")
	{
		FString UniqueString = FGuid::NewGuid().ToString(EGuidFormats::Short);
		InventoryContainerID = UniqueString;
	}
}

void UInventoryBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryBase, InventorySettings);
	DOREPLIFETIME(UInventoryBase, InventoryContainerID);
	DOREPLIFETIME(UInventoryBase, ItemCollectionLinked);
	DOREPLIFETIME(UInventoryBase, InventoryTotalWeight);
	DOREPLIFETIME(UInventoryBase, InventoryTotalMoney);
	DOREPLIFETIME(UInventoryBase, InvSize);
	DOREPLIFETIME(UInventoryBase, InventoryOwnerActor);
	DOREPLIFETIME(UInventoryBase, TradeContext);
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

	if (!StartupData.Settings.InventoryID.IsEmpty())
		Inventory->SetInventoryContainerID(StartupData.Settings.InventoryID);
		
	Inventory->SetInventorySettings(StartupData.Settings);

	return Inventory;
}

UInventoryBase* UInventoryBase::CreateInventoryAdvanced(UObject* Outer, FInventoryStartupData StartupData,
	AActor* OwnerActor, UItemCollection* InItemCollection)
{
	UInventoryBase* Inventory =	CreateInventory(Outer, StartupData);
	if (!Inventory)
		return nullptr;

	Inventory->InitInventory();
	Inventory->SetInventoryOwnerActor(OwnerActor);
	Inventory->SetItemCollectionLink(InItemCollection);

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
	InvSize = FIntPoint(InventorySettings.InventorySlotBasedSettings.NumberRows, InventorySettings.InventorySlotBasedSettings.NumColumns);
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
		OnRep_InventoryTotalWeight();
	}
	else
	{
		for (auto Item : AllItems)
		{
			InventoryTotalWeight += Item->GetQuantity() * Item->GetItemSingleWeight();
		}

		InventoryTotalWeight = FMath::RoundToFloat(InventoryTotalWeight * 100.0f) / 100.0f;
		OnRep_InventoryTotalWeight();
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
		OnRep_InventoryTotalMoney();
	}
	else
	{
		for (auto Item : AllItems)
		{
			if (Item->GetItemRef().ItemCategory == EItemCategory::Money)
				InventoryTotalMoney += Item->GetQuantity();
		}

		OnRep_InventoryTotalMoney();
	}
}

void UInventoryBase::SetTradeContext(FTradeContext InTradeContext)
{
	this->TradeContext = InTradeContext;
	OnRep_TradeContext();
}

int32 UInventoryBase::CalculateActualAmountToAdd(int32 InAmountToAdd, float ItemSingleWeight)
{
	if (InventorySettings.InventoryMaxWeightCapacity >= 0)
	{
		const int32 WeightLimitAddAmount = InventorySettings.InventoryMaxWeightCapacity - InventoryTotalWeight;
		int32 MaxItemsThatFit = WeightLimitAddAmount / ItemSingleWeight;
		return FMath::Min(MaxItemsThatFit, InAmountToAdd);
	}

	if (InventorySettings.MaxStackCount >= 0)
	{
		const int32 TotalCount = ItemCollectionLinked->GetStackCountInContainer(InventoryContainerID);
		const int32 RemainingSlots = InventorySettings.MaxStackCount - TotalCount;
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
		UpdateMoneyInfo();
		UpdateWeightInfo();
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
	UpdateMoneyInfo();
	UpdateWeightInfo();

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
	OnAddItemDelegate.Broadcast(FromSlots, NewItem);
}

void UInventoryBase::NotifyAddItemToStack(UItemBase* Item, int32 ChangeQuantity)
{
	OnStackedItemDelegate.Broadcast(Item, ChangeQuantity);
}

void UInventoryBase::NotifyRemoveItemFromStack(UItemBase* Item, int32 ChangeQuantity)
{
	OnUnstackedItemDelegate.Broadcast(Item, ChangeQuantity);
}

void UInventoryBase::NotifyFullyRemoveItem(FItemMapping FromSlots, UItemBase* Item)
{
	OnItemRemovedDelegate.Broadcast(FromSlots, Item);
}

void UInventoryBase::NotifyReplaceItem(TArray<UInventorySlotData*> OldItemSlots,
	FItemMapping& NewItemSlots, UItemBase* Item)
{
	OnItemReplaceDelegate.Broadcast(OldItemSlots, NewItemSlots, Item);
}

void UInventoryBase::NotifyUseSlot(UInventorySlotData* UsedSlot)
{
	OnUseSlotDelegate.Broadcast(UsedSlot);
}

void UInventoryBase::OnRep_InventoryTotalWeight()
{
	OnWeightUpdatedDelegate.Broadcast(InventoryTotalWeight);
}

void UInventoryBase::OnRep_InventoryTotalMoney()
{
	OnMoneyUpdatedDelegate.Broadcast(InventoryTotalMoney);
}

void UInventoryBase::OnRep_TradeContext()
{
	OnTradeContextUpdated.Broadcast();
}

void UInventoryBase::NotifyReDrawRequest()
{
	OnInventoryRedrawRequested.Broadcast();
}

void UInventoryBase::NotifyRequestToResetItemVisual(UItemBase* Item)
{
	OnRequestToResetItemVisual.Broadcast(Item);
}

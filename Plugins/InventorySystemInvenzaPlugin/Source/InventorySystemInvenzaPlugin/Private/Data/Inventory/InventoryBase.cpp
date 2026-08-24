//  Nublin Studio 2026 All Rights Reserved.


#include "Data/Inventory/InventoryBase.h"

#include "ActorComponents/ItemCollection.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Items/ItemBase.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Factory/ItemFactory.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "Utility/InterfaceUtils.h"

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

	Inventory->SetInventoryOwnerActor(OwnerActor);
	Inventory->SetItemCollectionLink(InItemCollection);
	Inventory->InitInventory();

	return Inventory;
}

UInventoryBase* UInventoryBase::DuplicateInventory(UObject* Outer)
{
	UInventoryBase* Inventory =	NewObject<UInventoryBase>(Outer, this->GetClass());

	if (!Inventory)
		return nullptr;

	Inventory->SetInventorySettings(InventorySettings);
	Inventory->SetItemCollectionLink(ItemCollectionLinked);
	Inventory->SetInventorySize(InvSize);
	Inventory->SetInventoryOwnerActor(InventoryOwnerActor);

	return Inventory;
}

void UInventoryBase::InitInventory()
{
	InvSize = FIntPoint(InventorySettings.InventorySlotBasedSettings.InitNumberRows, InventorySettings.InventorySlotBasedSettings.InitNumColumns);
}

void UInventoryBase::InitInventoryWithSettings(FInventorySettings NewInventorySettings)
{
	InventorySettings = NewInventorySettings;
	InitInventory();
}

void UInventoryBase::RequestToResetItemVisual(UObject* Item)
{
	if (!Item) return;

	if (ItemCollectionLinked->ItemHasInventory(Item, InventoryContainerID))
		NotifyRequestToResetItemVisual(Item);
}

void UInventoryBase::MergeStackableItems()
{
	auto Items = ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID);
	if (Items.IsEmpty()) return;

	for (int32 i = Items.Num() - 1; i > 0; --i)
	{
		UObject* ItemObj = Items[i];
		if (!ItemObj)
			continue;
		
		if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemObj, TEXT("MergeStackableItems")))
			continue;
		
		if (!IObjectDataProvider::Execute_IsStackable(ItemObj))
			continue;

		auto SameItems = ItemCollectionLinked->GetAllSameItemsInContainerByItemSample(InventoryContainerID, ItemObj);
		if (SameItems.IsEmpty())
			continue;

		int32 RemainingQuantity = IObjectDataProvider::Execute_GetQuantity(ItemObj);
		for (UObject* SameItemObj : SameItems)
		{
			if (SameItemObj == ItemObj)
				continue;

			if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(SameItemObj, TEXT("MergeStackableItems")))
				continue;
			
			const int32 AddedQuantity = TryInsertToStackItem(SameItemObj, RemainingQuantity, false);
			RemainingQuantity -= AddedQuantity;
		}
		
		if (RemainingQuantity <= 0)
		{
			HandleRemoveItem(ItemObj, IObjectDataProvider::Execute_GetQuantity(ItemObj));
		}
		else
		{
			IObjectDataProvider::Execute_SetQuantity(ItemObj, RemainingQuantity);
		}
	}
	
	OnRep_InventoryTotalWeight();
	OnRep_InventoryTotalMoney();
}

void UInventoryBase::UseSlot(UInventorySlotData* UsedSlot)
{
	if (!UsedSlot || !ItemCollectionLinked)
		return;

	UObject* ItemObj = ItemCollectionLinked->GetItemFromSlot(
		UsedSlot->InventorySlotInfo.SlotGuid,
		InventoryContainerID
	);

	if (!ItemObj)
		return;

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemObj, TEXT("UseSlot")))
		return;
	
	IObjectDataProvider::Execute_UseItem(ItemObj);
	
	NotifyUseSlot(UsedSlot);
}

void UInventoryBase::UpdateWeightInfo()
{
	if (!ItemCollectionLinked)
		return;

	InventoryTotalWeight = 0.0f;

	const TArray<UObject*> AllItems =
		ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID);

	if (AllItems.IsEmpty())
	{
		OnRep_InventoryTotalWeight();
		return;
	}

	for (UObject* ItemObj : AllItems)
	{
		if (!ItemObj)
			continue;
		
		if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(
				ItemObj,
				TEXT("UpdateWeightInfo")))
		{
			continue;
		}
		
		const int32 Quantity =
			IObjectDataProvider::Execute_GetQuantity(ItemObj);
		
		const float SingleWeight =
			IObjectDataProvider::Execute_GetItemSingleWeight(ItemObj);

		InventoryTotalWeight += Quantity * SingleWeight;
	}
	
	InventoryTotalWeight =
		FMath::RoundToFloat(InventoryTotalWeight * 100.0f) / 100.0f;

	OnRep_InventoryTotalWeight();
}

void UInventoryBase::UpdateMoneyInfo()
{
	if (!ItemCollectionLinked)
		return;

	const auto* MySettings = UInvenzaInventorySettingsSubsystem::GetSettingsStatic(this);
	if (!MySettings)
		return;

	InventoryTotalMoney = 0;

	const auto AllItems = ItemCollectionLinked->GetAllItemsByContainer(InventoryContainerID);
	if (AllItems.IsEmpty())
	{
		OnRep_InventoryTotalMoney();
		return;
	}

	for (UObject* ItemObj : AllItems)
	{
		if (!ItemObj)
			continue;

		if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ItemObj, TEXT("UpdateMoneyInfo")))
			continue;

		const FItemMetaData Meta = IObjectDataProvider::Execute_GetItemRef(ItemObj);
		if (Meta.ItemCategory == MySettings->CurrencyGameplayTag)
			InventoryTotalMoney += IObjectDataProvider::Execute_GetQuantity(ItemObj);
	}

	OnRep_InventoryTotalMoney();
}

TArray<UInventorySlotData*> UInventoryBase::GetAvailableSlotForItem(UObject* Item,
	EItemOrientationType& OutOrientation)
{
	TArray<UInventorySlotData*> ReturnSlots;
	return ReturnSlots;
}

UInventorySlotData* UInventoryBase::GetSlotByPosition(FIntPoint CellPosition)
{
	return nullptr;
}

UInventorySlotData* UInventoryBase::GetSlotByGuid(FGuid InGuid)
{
	return nullptr;
}

TArray<FGuid> UInventoryBase::GetSlotsWithLinkedEquipment()
{
	TArray<FGuid> ResultArray;
	return ResultArray;
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

int32 UInventoryBase::TryInsertToStackItem(UObject* ResourceToInsertInto, int32 AmountToDistribute, bool bOnlyCheck)
{
	if (!ResourceToInsertInto)
		return 0;

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(ResourceToInsertInto, TEXT("TryInsertToStackItem")))
		return 0;

	if (IObjectDataProvider::Execute_IsFullItemStack(ResourceToInsertInto))
		return 0;

	const FItemMetaData Meta = IObjectDataProvider::Execute_GetItemRef(ResourceToInsertInto);
	const int32 CurrentQuantity = IObjectDataProvider::Execute_GetQuantity(ResourceToInsertInto);
	const int32 MaxStack = Meta.ItemNumeraticData.MaxStackSizeInCharacter;

	const int32 AmountToAddToStack = FMath::Min(AmountToDistribute, MaxStack - CurrentQuantity);
	const float SingleWeight = IObjectDataProvider::Execute_GetItemSingleWeight(ResourceToInsertInto);

	const int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack, SingleWeight);

	if (!bOnlyCheck)
	{
		const int32 NewQuantity = CurrentQuantity + ActualAmountToAdd;
		IObjectDataProvider::Execute_SetQuantity(ResourceToInsertInto, NewQuantity);

		NotifyAddItemToStack(ResourceToInsertInto);
		UpdateMoneyInfo();
		UpdateWeightInfo();
	}

	return ActualAmountToAdd;
}

int32 UInventoryBase::TryRemoveFromStackItem(UObject* Item, int32 RequestedRemoveAmount)
{
	if (!Item || RequestedRemoveAmount <= 0)
		return 0;

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("TryRemoveFromStackItem")))
		return 0;

	const int32 CurrentQuantity = IObjectDataProvider::Execute_GetQuantity(Item);
	const int32 AmountToRemove = FMath::Min(RequestedRemoveAmount, CurrentQuantity);

	if (AmountToRemove <= 0)
	{
		RemoveItemFromInventory(Item);
		return 0;
	}

	if (CurrentQuantity - AmountToRemove <= 0)
	{
		RemoveItemFromInventory(Item);
	}
	else
	{
		IObjectDataProvider::Execute_SetQuantity(Item, CurrentQuantity - AmountToRemove);
	}

	NotifyRemoveItemFromStack(Item);
	UpdateMoneyInfo();
	UpdateWeightInfo();

	return AmountToRemove;
}

void UInventoryBase::RemoveItemFromInventory(UObject* Item)
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

void UInventoryBase::NotifyAddNewItem(FItemMapping& FromSlots, UObject* NewItem, int32 ChangeQuantity)
{
	OnAddItemDelegate.Broadcast(FromSlots, NewItem);
}

void UInventoryBase::NotifyAddItemToStack(UObject* Item)
{
	OnStackedItemDelegate.Broadcast(Item);
}

void UInventoryBase::NotifyRemoveItemFromStack(UObject* Item)
{
	OnUnstackedItemDelegate.Broadcast(Item);
}

void UInventoryBase::NotifyFullyRemoveItem(FItemMapping FromSlots, UObject* Item)
{
	OnItemRemovedDelegate.Broadcast(FromSlots, Item);
}

void UInventoryBase::NotifyReplaceItem(TArray<UInventorySlotData*> OldItemSlots,
	FItemMapping& NewItemSlots, UObject* Item)
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

void UInventoryBase::NotifyRequestToResetItemVisual(UObject* Item)
{
	OnRequestToResetItemVisual.Broadcast(Item);
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/BaseInventoryWidget.h"

#include "ActorComponents/Items/itemBase.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "UI/Item/InventoryItemWidget.h"

UBaseInventoryWidget::UBaseInventoryWidget(): SlotsGridPanel(nullptr), InventoryWeightCapacity(0)
{
}

void UBaseInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UBaseInventoryWidget::InitSlots);
	
	UpdateWeightInfo();
}

void UBaseInventoryWidget::InitSlots()
{
	if (!SlotsGridPanel)
		return;

	TArray<TObjectPtr<UBaseInventorySlot>> NewInvSlots;
	const int32 NumChildren = SlotsGridPanel->GetChildrenCount();

	for (int32 i = 0; i < NumChildren; ++i)
	{
		if (UWidget* ChildWidget = SlotsGridPanel->GetChildAt(i))
		{
			auto WClass = ChildWidget->GetClass();
			if (WClass->IsChildOf(UBaseInventorySlot::StaticClass()))
			{
				if (auto HotbarSlot = Cast<UBaseInventorySlot>(ChildWidget))
				{
					NewInvSlots.Add(HotbarSlot);
				}
			}
		}
	}

	int Temp_NumberOfRows = 0;
	int Temp_NumberOfColumns = 0;

	for (int32 i = 0; i < NewInvSlots.Num(); ++i)
	{
		if (const UWidget* ChildWidget = SlotsGridPanel->GetChildAt(i))
		{
			const UUniformGridSlot* UniSlot = Cast<UUniformGridSlot>(ChildWidget->Slot);
			if (UniSlot->GetRow() > Temp_NumberOfRows)
				Temp_NumberOfRows = UniSlot->GetRow();
			if (UniSlot->GetColumn() > Temp_NumberOfColumns)
				Temp_NumberOfColumns = UniSlot->GetColumn();

			NewInvSlots[i]->SetSlotPosition(FIntVector2( UniSlot->GetColumn(), UniSlot->GetRow()));
		}
	}

	if (NewInvSlots.Num() > 0)
	{
		FTimerHandle LayoutTimer;
		GetWorld()->GetTimerManager().SetTimer(LayoutTimer, [this, NewInvSlots]()
		{
			SlotSize = NewInvSlots[0]->GetCachedGeometry().GetLocalSize();
		}, 0.25f, false);
	}
	InventorySlots = NewInvSlots;
}

FArrayItemSlots UBaseInventoryWidget::GetAllSlotsFromInventoryItemsMap()
{
	FArrayItemSlots AllPositions;

	for (auto& Elem : InventoryItemsMap)
	{
		for (const auto& ItemSlot : Elem.Value.ItemSlots)
		{
			AllPositions.ItemSlots.Add(ItemSlot);
		}
	}

	return AllPositions;
}

FArrayItemSlots UBaseInventoryWidget::GetOccupiedSlots(UItemBase* Item)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetItemSlots: Item is null!"));
		return FArrayItemSlots();
	}

	FArrayItemSlots* FoundSlots = InventoryItemsMap.Find(Item);
	if (!FoundSlots)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetItemSlots: No slots found for item %s"), *Item->GetName());
		return FArrayItemSlots();
	}
	
	return FArrayItemSlots();
}

TArray<UItemBase*> UBaseInventoryWidget::GetAllSameItems(UItemBase* TestItem)
{
	TArray<UItemBase*> AllItems;

	for (auto& Elem : InventoryItemsMap)
	{
		if (Elem.Key == TestItem)
			AllItems.Add(Elem.Key);
	}

	return AllItems;
}

UItemBase* UBaseInventoryWidget::GetItemFromSlot(UBaseInventorySlot* TargetSlot)
{
	if (!TargetSlot)
		return nullptr;

	for (auto Pair  : InventoryItemsMap)
	{
		for (auto Element : Pair.Value.ItemSlots)
		{
			if (Element == TargetSlot)
				return Pair.Key;
		}
	}

	return nullptr;
		
}

bool UBaseInventoryWidget::bIsSlotEmpty(const FIntVector2 SlotPosition)
{
	FArrayItemSlots BusySlots = GetAllSlotsFromInventoryItemsMap();
	for (const auto InvSlot : BusySlots.ItemSlots)
	{
		if (InvSlot->GetSlotPosition() == SlotPosition)
			return false;
	}
	
	return true;
}

bool UBaseInventoryWidget::bIsSlotEmpty(const UBaseInventorySlot* SlotCheck)
{
	FArrayItemSlots BusySlots = GetAllSlotsFromInventoryItemsMap();
	for (auto InvSlot : BusySlots.ItemSlots)
	{
		if (InvSlot == SlotCheck)
			return false;
	}
	return true;
}

FItemAddResult UBaseInventoryWidget::HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
	// non-stack
	if (!ItemMoveData.SourceItem->IsStackable())
	{
		// Check if input item has valid weight
		if (FMath::IsNearlyZero(ItemMoveData.SourceItem->GetItemSingleWeight()) || ItemMoveData.SourceItem->
			GetItemSingleWeight() < 0)
		{
			return FItemAddResult::AddedNone(FText::Format(FText::FromString("Item {0} has invalid weight"),
														   ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
		}

		// will the item weight overflow the weight capacity?
		if (InventoryWeightCapacity > 0)
		{
			if (InventoryTotalWeight + ItemMoveData.SourceItem->GetItemSingleWeight() > InventoryWeightCapacity)
			{
				return FItemAddResult::AddedNone(FText::Format(
					FText::FromString("Item {0} would overflow weight limit"),
					ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
			}
		}

		return HandleNonStackableItems(ItemMoveData, bOnlyCheck);
	}

	// stack
	// will the item weight overflow the weight capacity?
	if (InventoryTotalWeight + ItemMoveData.SourceItem->GetItemSingleWeight() * ItemMoveData.SourceItem->GetQuantity() > InventoryWeightCapacity)
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Couldn't add {0} to inventory."),
														   ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}
	
	const int32 InitialRequestedAddAmount = ItemMoveData.SourceItem->GetQuantity();
	const int32 StackableAmountAdded = HandleStackableItems(ItemMoveData, InitialRequestedAddAmount,
																bOnlyCheck);

	if (StackableAmountAdded == InitialRequestedAddAmount)
	{
		return FItemAddResult::AddedAll(StackableAmountAdded, false, FText::Format(
												FText::FromString("Successfully added {0} of {1} to inventory"),
												InitialRequestedAddAmount,ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}
	else if (StackableAmountAdded < InitialRequestedAddAmount && StackableAmountAdded > 0)
	{
		return FItemAddResult::AddedPartial(StackableAmountAdded, false, FText::Format(
													FText::FromString(
														"Partial amount of {0} added to inventory. number added {1}"),
													ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name, StackableAmountAdded));
	}
	else if (StackableAmountAdded <= 0)
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Couldn't add {0} to inventory."),
														   ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}

	return FItemAddResult::AddedNone(FText::Format(
		FText::FromString("Could not add {0} to inventory. No remaining slots or weight."),
		ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
}


FItemAddResult UBaseInventoryWidget::HandleNonStackableItems(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	if (!ItemMoveData.TargetSlot)
	{
		TObjectPtr<UBaseInventorySlot> EmptySlot = nullptr;
		for (const auto InventorySlot : InventorySlots)
		{
			if (bool IsEmptySlot = bIsSlotEmpty(InventorySlot))
			{
				EmptySlot = InventorySlot;
				break;
			}
		}

		if (EmptySlot == nullptr)
			return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory. No Empty slots"),
												   1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));

		if (!bOnlyCheck)
		{
			FArrayItemSlots Slots;
			Slots.ItemSlots.Add(EmptySlot);
			AddNewItem(ItemMoveData, Slots);
		}
		
		return FItemAddResult::AddedAll(1, false, FText::Format(
												FText::FromString("Successfully added {0} of {1} to inventory"),
												1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}

	if (bIsSlotEmpty(ItemMoveData.TargetSlot->GetSlotPosition()))
	{
		if (!bOnlyCheck)
		{
			FArrayItemSlots Slots;
			Slots.ItemSlots.Add(ItemMoveData.TargetSlot);
			AddNewItem(ItemMoveData, Slots);
		}

		return FItemAddResult::AddedAll(1, false, FText::Format(
											FText::FromString("Successfully added {0} of {1} to inventory"),
											1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}

	return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
}

int32 UBaseInventoryWidget::HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount, bool bOnlyCheck)
{
	int32 AmountToDistribute = RequestedAddAmount;
	
	auto CalculateActualAmountToAdd = [&](int32 InAmountToAdd) -> int32
	{
		const int32 WeightLimitAddAmount = InventoryWeightCapacity - InventoryTotalWeight;
		int32 MaxItemsThatFit = WeightLimitAddAmount / ItemMoveData.SourceItem->GetItemSingleWeight();
		UE_LOG(LogTemp, Warning, TEXT("WeightLimitAddAmount: %f, SingleItemWeight: %f, MaxItemsThatFit: %d"), 
			static_cast<float>(WeightLimitAddAmount), 
			static_cast<float>(ItemMoveData.SourceItem->GetItemSingleWeight()), 
			MaxItemsThatFit);
		return FMath::Min(MaxItemsThatFit, InAmountToAdd);
	};

	if (!ItemMoveData.TargetSlot)
	{
		auto Sameitems = GetAllSameItems(ItemMoveData.SourceItem);
		if (Sameitems.Num()> 0)
		{
			for (auto& Item : Sameitems)
			{
				if(AmountToDistribute<=0)
					break;
				
				if (Item->IsFullItemStack())
					continue;

				const int32 AmountToAddToStack = FMath::Min(AmountToDistribute,
					Item->GetItemRef().ItemNumeraticData.MaxStackSize - Item->GetQuantity());
				int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack);

				Item->SetQuantity(Item->GetQuantity() + ActualAmountToAdd);
				AmountToDistribute -= ActualAmountToAdd;

				auto Slots = GetOccupiedSlots(Item);
				NotifyUpdateItem(Slots, Item);
			}
		}

		
		TObjectPtr<UBaseInventorySlot> TargetSlot = nullptr;
		for (const auto InventorySlot : InventorySlots)
		{
			if (bool IsEmptySlot = bIsSlotEmpty(InventorySlot))
			{
				TargetSlot = InventorySlot;
				break;
			}
		}

		if (TargetSlot == nullptr)
			return RequestedAddAmount - AmountToDistribute;

		const int32 AmountToAddToStack = FMath::Min(AmountToDistribute, ItemMoveData.SourceItem->GetQuantity());
		int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack);
		
		if (bOnlyCheck)
			return AmountToDistribute;

		FArrayItemSlots Slots;
		Slots.ItemSlots.Add(TargetSlot);
		FItemMoveData NewItemMoveData;
		NewItemMoveData.SourceItem = ItemMoveData.SourceItem;
		NewItemMoveData.SourceItem->SetQuantity(ActualAmountToAdd);
		
		AddNewItem(NewItemMoveData, Slots);
		ItemMoveData.SourceItem->SetQuantity(ItemMoveData.SourceItem->GetQuantity() - ActualAmountToAdd);
		return RequestedAddAmount - AmountToDistribute;
	}

	if (bIsSlotEmpty(ItemMoveData.TargetSlot))
	{
		const int32 AmountToAddToStack = FMath::Min(AmountToDistribute,
		   ItemMoveData.SourceItem->GetItemRef().ItemNumeraticData.MaxStackSize -
		   ItemMoveData.SourceItem->GetQuantity());

		int32 ActualAmountToAdd = CalculateActualAmountToAdd(AmountToAddToStack);

		FArrayItemSlots Slots;
		Slots.ItemSlots.Add(ItemMoveData.TargetSlot);
		FItemMoveData NewItemMoveData;
		NewItemMoveData.SourceItem = ItemMoveData.SourceItem;
		NewItemMoveData.SourceItem->SetQuantity(ActualAmountToAdd);
		
		AddNewItem(NewItemMoveData, Slots);
		ItemMoveData.SourceItem->SetQuantity(ItemMoveData.SourceItem->GetQuantity() - ActualAmountToAdd);
		return RequestedAddAmount - AmountToDistribute;
	}
	else
	{
		UItemBase* ItemFromSlot= GetItemFromSlot(ItemMoveData.TargetSlot);
		
		if (ItemFromSlot && ItemFromSlot != ItemMoveData.SourceItem)
			return 0;

		const int32 AmountToAddToStack = FMath::Min(AmountToDistribute,
				ItemMoveData.SourceItem->GetItemRef().ItemNumeraticData.MaxStackSize -
				ItemMoveData.SourceItem->GetQuantity());

		int32 WeightLimitAddAmount = InventoryWeightCapacity - InventoryTotalWeight;
		int32 MaxItemsThatFit = WeightLimitAddAmount / ItemMoveData.SourceItem->GetItemSingleWeight();
		int32 ActualAmountToAdd = FMath::Min(MaxItemsThatFit, AmountToDistribute);

		ItemFromSlot->SetQuantity(ItemFromSlot->GetQuantity() + ActualAmountToAdd);
		AmountToDistribute -= ActualAmountToAdd;

		//NotifyUpdateItem(Slots, ItemInPivot);
		return RequestedAddAmount - AmountToDistribute;
	}
}

void UBaseInventoryWidget::AddNewItem(FItemMoveData& ItemMoveData, FArrayItemSlots OccupiedSlots)
{
	TObjectPtr<UItemBase> NewItem = ItemMoveData.SourceItem;
	NewItem->SetQuantity(ItemMoveData.SourceItem->GetQuantity());

	InventoryItemsMap.Add(NewItem, OccupiedSlots);
	if (InventoryWeightCapacity > 0)
		InventoryTotalWeight += NewItem->GetItemStackWeight();

	NotifyAddItem(OccupiedSlots, NewItem);
}

FVector2D UBaseInventoryWidget::CalculateItemVisualPosition(FIntVector2 SlotPosition, FIntVector2 ItemSize)
{
	float X = SlotPosition.X * SlotSize.X;
	float Y = SlotPosition.Y * SlotSize.Y;

	return FVector2D(X, Y);
}

void UBaseInventoryWidget::AddItemToPanel(FArrayItemSlots FromSlots, UItemBase* Item)
{
	UBaseInventorySlot* ItemPivotSlot = Cast<UBaseInventorySlot>(FromSlots.ItemSlots[0]);
	FVector2D VisualPosition = CalculateItemVisualPosition(ItemPivotSlot->GetSlotPosition(), Item->GetOccupiedSlots());

	if (!UISettings.InventoryItemVisualClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryItemVisualClass is not set in UISettings!"));
		return;
	}
	
	TObjectPtr<UInventoryItemWidget> ItemVisual = CreateWidget<UInventoryItemWidget>(this, UISettings.InventoryItemVisualClass);
	auto SlotInCanvas = ItemsVisualsPanel->AddChildToCanvas(ItemVisual);
	if (SlotInCanvas)
		SlotInCanvas->SetSize(FVector2D(SlotSize.X * 1, SlotSize.Y *  1));

	ItemVisual->UpdateVisualSize(SlotSize, FIntVector2(1, 1));
	ItemVisual->UpdateItemName(Item->GetItemRef().ItemTextData.Name);
	ItemVisual->UpdateQuantityText(Item->GetQuantity());
	ItemVisual->UpdateVisual(Item);
	
	//ItemVisual->SetPivotSlot(ItemPivotSlot);			
	SlotInCanvas->SetPosition(VisualPosition);

	UpdateWeightInfo();
}

void UBaseInventoryWidget::UpdateWeightInfo()
{
	if (!WeightInfo)
		return;

	const FString Text_WeightInfo = {" " + FString::SanitizeFloat(InventoryTotalWeight) + "/" + FString::SanitizeFloat(InventoryWeightCapacity)};
	
	WeightInfo->SetText(FText::FromString(Text_WeightInfo));
}

void UBaseInventoryWidget::NotifyAddItem(FArrayItemSlots FromSlots, UItemBase* NewItem)
{
	AddItemToPanel(FromSlots, NewItem);
	OnAddItemDelegate.Broadcast(FromSlots, NewItem);
}

void UBaseInventoryWidget::NotifyUpdateItem(FArrayItemSlots FromSlots, UItemBase* NewItem)
{
	OnItemUpdateDelegate.Broadcast(FromSlots, NewItem);
}

void UBaseInventoryWidget::NotifyRemoveItem(FArrayItemSlots FromSlots, UItemBase* RemovedItem) const
{
	OnRemoveItemDelegate.Broadcast(FromSlots, RemovedItem);
}

/*void UBaseInventoryWidget::NotifyUseSlot(UBaseInventorySlot* FromSlot)
{
	//auto Item = FindItemBySlot(FromSlot);

	//OnUseItemDelegate.Broadcast(FromSlot, Item);
}*/


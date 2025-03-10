// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/BaseInventoryWidget.h"

#include "ActorComponents/Items/itemBase.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

UBaseInventoryWidget::UBaseInventoryWidget(): SlotsGridPanel(nullptr), InventoryWeightCapacity(0)
{
}

void UBaseInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UBaseInventoryWidget::InitSlots);
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
	return  0;
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

void UBaseInventoryWidget::NotifyAddItem(FArrayItemSlots FromSlots, UItemBase* NewItem)
{
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


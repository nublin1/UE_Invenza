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
}

FItemAddResult UBaseInventoryWidget::HandleNonStackableItems(FItemMoveData& ItemMoveData, bool bOnlyCheck)
{
	if (!ItemMoveData.TargetSlot)
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
	}

	return FItemAddResult::AddedNone(FText::Format(FText::FromString("Can't be added {0} of {1} to inventory"),
												   1, ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
}

FItemAddResult UBaseInventoryWidget::HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
	const int32 InitialRequestedAddAmount = ItemMoveData.SourceItem->GetQuantity();

	if (!ItemMoveData.SourceItem->IsStackable())
	{
		// Check if input item has valid weight
		if (FMath::IsNearlyZero(ItemMoveData.SourceItem->GetItemStackWeight()) || ItemMoveData.SourceItem->
			GetItemStackWeight() < 0)
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

	return FItemAddResult::AddedNone(FText::Format(
		FText::FromString("Could not add {0} to inventory. No remaining slots or weight."),
		ItemMoveData.SourceItem->GetItemRef().ItemTextData.Name));
}

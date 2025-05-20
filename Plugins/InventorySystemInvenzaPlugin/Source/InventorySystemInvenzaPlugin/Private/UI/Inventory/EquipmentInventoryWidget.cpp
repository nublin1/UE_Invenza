// Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/EquipmentInventoryWidget.h"

#include "ActorComponents/Items/itemBase.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "UI/Inventory/EquipmentSlotWidget.h"

void UEquipmentInventoryWidget::InitializeInventory()
{
	Super::InitializeInventory();
}

FItemAddResult UEquipmentInventoryWidget::HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck)
{
	if (ItemMoveData.TargetSlot)
	{
		auto EquipmentSlot = Cast<UEquipmentSlotWidget>(ItemMoveData.TargetSlot);
		if (EquipmentSlot && EquipmentSlot->AllowedCategory == ItemMoveData.SourceItem->GetItemRef().ItemCategory)
		{
			return Super::HandleAddItem(ItemMoveData, bOnlyCheck);
		}
		else
		{
			return FItemAddResult::AddedNone(FText::FromString("AllowedCategory is not same"));
		}
	}

	UEquipmentSlotWidget* NewTargetSlot = nullptr;
	for (auto InventorySlot : GetInventoryData().InventorySlots)
	{
		auto EquipmentSlot = Cast<UEquipmentSlotWidget>(InventorySlot);
		if (!EquipmentSlot)
			continue;

		if (EquipmentSlot->AllowedCategory == ItemMoveData.SourceItem->GetItemRef().ItemCategory)
		{
			ItemMoveData.TargetSlot = EquipmentSlot;
			return Super::HandleAddItem(ItemMoveData, bOnlyCheck);
		}
	}
	
	return FItemAddResult::AddedNone(FText::FromString("Not allowed slots"));
}

void UEquipmentInventoryWidget::InitSlots()
{
	if (!SlotsGridPanel)
		return;
	
	TArray<TObjectPtr<UEquipmentSlotWidget>> NewInvSlots;
	const int32 NumChildren = SlotsGridPanel->GetChildrenCount();

	for (int32 i = 0; i < NumChildren; ++i)
	{
		if (UWidget* ChildWidget = SlotsGridPanel->GetChildAt(i))
		{
			auto WClass = ChildWidget->GetClass();
			if (WClass->IsChildOf(UEquipmentSlotWidget::StaticClass()))
			{
				UUniformGridSlot* UniSlot = Cast<UUniformGridSlot>(ChildWidget->Slot);
				if (auto EqSlot = Cast<UEquipmentSlotWidget>(ChildWidget))
				{
					EqSlot->SetSlotPosition(FIntVector2(UniSlot->GetRow(), UniSlot->GetColumn()));
					NewInvSlots.Add(EqSlot);
				}
			}
		}
	}
	
	TArray<UInventorySlot*> ConvertedSlots;
	for (auto InvSlot : NewInvSlots)
	{
		if (InvSlot)
		{
			ConvertedSlots.Add(InvSlot);
		}
	}
	InventoryData.InventorySlots = ConvertedSlots;
}

bool UEquipmentInventoryWidget::bIsGridPositionValid(FIntPoint& GridPosition)
{
	return true;
}

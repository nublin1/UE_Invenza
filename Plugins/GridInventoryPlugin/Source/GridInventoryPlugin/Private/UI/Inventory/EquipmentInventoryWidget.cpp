// Nublin Studio 2025 All Rights Reserved.


#include "UI/Inventory/EquipmentInventoryWidget.h"

#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "UI/Inventory/EquipmentSlotWidget.h"

void UEquipmentInventoryWidget::InitializeInventory()
{
	Super::InitializeInventory();
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
				const UUniformGridSlot* UniSlot = Cast<UUniformGridSlot>(ChildWidget->Slot);
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

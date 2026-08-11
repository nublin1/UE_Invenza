//  Nublin Studio 2026 All Rights Reserved.


#include "UI/Inventory/InventorySlot.h"


UInventorySlot::UInventorySlot()
{
}

void UInventorySlot::UpdateVisualWithItemInfo(UObject* Item)
{
}

void UInventorySlot::UpdateVisualWithTexture(UTexture2D* NewTexture)
{
}

void UInventorySlot::ResetVisual()
{
}

void UInventorySlot::ClearVisual()
{
}

UInputAction* UInventorySlot::GetUseAction() const
{
	if (SlotData->InventorySlotInfo.UseAction.IsNull())
		return nullptr;

	return SlotData->InventorySlotInfo.UseAction.LoadSynchronous();
}

void UInventorySlot::SetSlotNameText(FString InUseKeyText)
{
}

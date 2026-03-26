// Nublin Studio 2026 All Rights Reserved.

#include "Data/Inventory/InventorySlotData.h"

UInventorySlotData::UInventorySlotData()
{
}

UInventorySlotData* UInventorySlotData::Create(UObject* Outer)
{
	return NewObject<UInventorySlotData>(Outer);
}

UInventorySlotData* UInventorySlotData::CreateWithData(UObject* Outer, FName Name, FIntPoint Position,
	UInputAction* Action, EItemCategory Category)
{
	UInventorySlotData* Slot = NewObject<UInventorySlotData>(Outer);
	if (!Slot) return nullptr;

	Slot->AllowedCategory = Category;
	Slot->SlotName = Name;
	Slot->CellPosition = Position;
	Slot->UseAction = Action;

	return Slot;
}

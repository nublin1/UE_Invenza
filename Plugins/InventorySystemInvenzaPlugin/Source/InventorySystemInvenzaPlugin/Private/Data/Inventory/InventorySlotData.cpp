// Nublin Studio 2026 All Rights Reserved.

#include "Data/Inventory/InventorySlotData.h"

#include "InputAction.h"

UInventorySlotData::UInventorySlotData(): InventorySlotInfo()
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

	Slot->InventorySlotInfo.AllowedCategory = Category;
	Slot->InventorySlotInfo.SlotName = Name;
	Slot->InventorySlotInfo.CellPosition = Position;
	Slot->InventorySlotInfo.UseAction = TSoftObjectPtr<UInputAction>(Action);

	return Slot;
}

UInventorySlotData* UInventorySlotData::DuplicateSlotData(UObject* Outer)
{
	UInventorySlotData* NewSlot = NewObject<UInventorySlotData>(Outer);
	if (!NewSlot) return nullptr;
	
	NewSlot->InventorySlotInfo.AllowedCategory	= this->InventorySlotInfo.AllowedCategory;
	NewSlot->InventorySlotInfo.SlotName			= this->InventorySlotInfo.SlotName;
	NewSlot->InventorySlotInfo.CellPosition		= this->InventorySlotInfo.CellPosition;
	NewSlot->InventorySlotInfo.UseAction		= this->InventorySlotInfo.UseAction;

	return NewSlot;
}

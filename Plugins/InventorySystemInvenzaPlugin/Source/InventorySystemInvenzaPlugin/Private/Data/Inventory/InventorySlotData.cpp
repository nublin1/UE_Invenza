// Nublin Studio 2026 All Rights Reserved.

#include "Data/Inventory/InventorySlotData.h"

#include "InputAction.h"
#include "Net/UnrealNetwork.h"

UInventorySlotData::UInventorySlotData(): InventorySlotInfo()
{
}

void UInventorySlotData::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventorySlotData, InventorySlotInfo);
}

UInventorySlotData* UInventorySlotData::Create(UObject* Outer)
{
	return NewObject<UInventorySlotData>(Outer);
}

UInventorySlotData* UInventorySlotData::CreateWithData(UObject* Outer, FInventorySlotInfo SlotData)
{
	UInventorySlotData* Slot = NewObject<UInventorySlotData>(Outer);
	if (!Slot) return nullptr;
	
	Slot->InventorySlotInfo.SlotName = SlotData.SlotName;
	Slot->InventorySlotInfo.CellPosition = SlotData.CellPosition;
	Slot->InventorySlotInfo.UseAction = TSoftObjectPtr<UInputAction>(SlotData.UseAction);
	Slot->InventorySlotInfo.AllowedCategory = SlotData.AllowedCategory;
	Slot->InventorySlotInfo.LinkedEquipmentSlot = SlotData.LinkedEquipmentSlot;

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

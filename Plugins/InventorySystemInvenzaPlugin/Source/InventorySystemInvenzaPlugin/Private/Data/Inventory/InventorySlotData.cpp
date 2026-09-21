// Nublin Studio 2026 All Rights Reserved.

#include "Data/Inventory/InventorySlotData.h"

#include "InputAction.h"
#include "Net/UnrealNetwork.h"
#include "ActorComponents/ItemCollection.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "GameFramework/Actor.h"

void UInventorySlotData::OnRep_InventorySlotInfo()
{
	// Slot references and their coordinates can arrive in separate replication updates.
	const AActor* Owner = GetTypedOuter<AActor>();
	UItemCollection* Collection = Owner ? Owner->FindComponentByClass<UItemCollection>() : nullptr;
	if (!Collection) return;
	for (UInventoryBase* Inventory : Collection->GetActorInventories())
	{
		USlotbasedInventory* Grid = Cast<USlotbasedInventory>(Inventory);
		if (Grid && Grid->GetInventorySlots().Contains(this))
		{
			Collection->OnInventoryItemsChanged.Broadcast(Grid->GetInventoryContainerID());
		}
	}
}

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

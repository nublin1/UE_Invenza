//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Inventory/InventoryTypes.h"

class UEquipmentComponent;
class UInventoryCollectionComponent;

/** Resolved from registered inventories, never from the actor's network owner. */
struct FEquipmentInventoryContext
{
	UInventoryCollectionComponent* Collection = nullptr;
	UEquipmentComponent* Equipment = nullptr;
	UInventorySlotData* Slot = nullptr;
	FGameplayTag SlotTag;
};

/** A transfer is planned using mappings; item instances and their state are retained. */
struct FEquipmentInventoryTransferState
{
	UInventoryBase* Inventory = nullptr;
	TMap<UObject*, FItemMapping> Before;
	TMap<UObject*, FItemMapping> After;
};

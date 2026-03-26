//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryListEntry.h"
#include "Data/Inventory/InventoryBase.h"
#include "ListInventory.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UListInventory : public UInventoryBase
{
	GENERATED_BODY()

public:
	UListInventory();

	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Data")
	TArray<TObjectPtr<UInventoryListEntry>> InvSlotsArray;
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Data")
	TArray<TObjectPtr<UInventoryListEntry>> FilteredInvSlotsArray;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void SortItemsInContainerByName() override;
	
	virtual void HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity) override;
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UInventoryListEntry> EntryClass = UInventoryListEntry::StaticClass();

	//====================================================================
	// FUNCTIONS
	//====================================================================

	virtual FItemAddResult HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck = false) override;
	
	virtual FItemAddResult HandleNonStackableItems(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;
	virtual FItemAddResult TryAddStackableItem(FItemMoveData& ItemMoveData, bool bOnlyCheck) override;
	virtual int32 HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount, bool bOnlyCheck,
									   TMap<UInventorySlotData*, FItemPlacementData>& AffectedPivotSlots) override;
	
	virtual UItemBase* AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount) override;
	
};


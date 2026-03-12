//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable)
	virtual void HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity) override;
	UFUNCTION(BlueprintCallable)
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================

	virtual FItemAddResult HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck = false) override;

	UFUNCTION()
	virtual FItemAddResult HandleNonStackableItems(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;
	UFUNCTION()
	virtual FItemAddResult TryAddStackableItem(FItemMoveData& ItemMoveData, bool bOnlyCheck) override;
	UFUNCTION()
	virtual int32 HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount, bool bOnlyCheck,
									   TMap<UInventorySlotData*, FItemPlacementData>& AffectedPivotSlots) override;

	UFUNCTION()
	virtual UItemBase* AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount) override;
};

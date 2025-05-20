// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SlotbasedInventoryWidget.h"
#include "UI/Inventory/UInventoryWidgetBase.h"
#include "EquipmentInventoryWidget.generated.h"


class UCanvasPanel;
class UUniformGridPanel;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UEquipmentInventoryWidget : public USlotbasedInventoryWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void InitializeInventory() override;

	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck) override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void InitSlots() override;

	virtual bool bIsGridPositionValid(FIntPoint& GridPosition) override;
	
};

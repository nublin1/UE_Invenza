// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SlotbasedInventorySlot.h"
#include "Data/EquipmentStructures.h"
#include "UI/Inventory/InventorySlot.h"
#include "EquipmentSlotWidget.generated.h"

/**
 * Equipment Slot Widget - Represents an equipment slot within the inventory system.
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UEquipmentSlotWidget : public USlotbasedInventorySlot
{
	GENERATED_BODY()
	
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	EItemCategory AllowedCategory;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UEquipmentSlotWidget();
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeConstruct() override;
};

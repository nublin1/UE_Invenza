// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/EquipmentStructures.h"
#include "UI/Inventory/InventorySlot.h"
#include "EquipmentSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UEquipmentSlotWidget : public UInventorySlot
{
	GENERATED_BODY()
	
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	FEquipmentSlot EquipmentSlot;

	//====================================================================
	// FUNCTIONS
	//====================================================================
};

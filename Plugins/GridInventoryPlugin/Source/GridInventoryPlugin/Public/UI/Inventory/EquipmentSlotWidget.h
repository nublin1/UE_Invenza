// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SlotbasedInventorySlot.h"
#include "Data/EquipmentStructures.h"
#include "UI/Inventory/InventorySlot.h"
#include "EquipmentSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UEquipmentSlotWidget : public USlotbasedInventorySlot
{
	GENERATED_BODY()
	
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
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

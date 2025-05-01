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
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemCategory AllowedCategory;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCoreCellWidget> CoreCellWidget;

	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults")
	TObjectPtr<UTexture2D> CellImage;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeConstruct() override;
};

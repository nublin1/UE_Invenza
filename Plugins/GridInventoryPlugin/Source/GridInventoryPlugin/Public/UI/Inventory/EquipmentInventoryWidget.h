// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Inventory/UInventoryWidgetBase.h"
#include "EquipmentInventoryWidget.generated.h"


class UCanvasPanel;
class UUniformGridPanel;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UEquipmentInventoryWidget : public UUInventoryWidgetBase
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

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	TObjectPtr<UUniformGridPanel> SlotsGridPanel;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCanvasPanel> ItemsVisualsPanel;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	virtual void InitSlots();
	
};

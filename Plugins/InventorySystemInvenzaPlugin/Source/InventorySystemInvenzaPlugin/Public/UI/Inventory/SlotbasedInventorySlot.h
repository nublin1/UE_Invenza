//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventorySlot.h"
#include "SlotbasedInventorySlot.generated.h"

class UTextBlock;
class UCoreCellWidget;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API USlotbasedInventorySlot : public UInventorySlot
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCoreCellWidget> CoreCellWidget;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> SlotName;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	USlotbasedInventorySlot();

	virtual void UpdateVisualWithTexture(UTexture2D* NewTexture) override;
	virtual void ResetVisual() override;
	virtual void ClearVisual() override;

	UFUNCTION(BlueprintCallable)
	UTexture2D* GetDefaultCellImage() {return DefaultCellImage;}

	virtual void SetSlotNameText(FString InUseKeyText) override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	/** 
	* Override for slot background image.
	* If set, this texture will be used instead of the default image defined in the inventory widget.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defaults",
		meta = (ToolTip =
			"Override texture for this slot. Takes priority over the default slot texture defined in the inventory widget."
		))
	TObjectPtr<UTexture2D> DefaultCellImage;

	//====================================================================
	// FUNCTIONS
	//====================================================================

	virtual void NativeConstruct() override;
};

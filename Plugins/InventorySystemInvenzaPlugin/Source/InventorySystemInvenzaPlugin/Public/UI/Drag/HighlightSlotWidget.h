//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "Data/Inventory/InventoryTypes.h"
#include "UI/Item/InventoryItemWidget.h"
#include "HighlightSlotWidget.generated.h"

enum class EHighlightState : uint8;
class UCoreCellWidget;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UHighlightSlotWidget : public UInventoryItemWidget
{
	GENERATED_BODY()
public:
	UHighlightSlotWidget();

protected:
	virtual void NativeConstruct() override;

public:
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category = "Highlight")
	void SetHighlightState(EHighlightState NewState);

	UFUNCTION(BlueprintCallable, Category = "Highlight")
	virtual void UpdateVisualWithTexture(UTexture2D* NewTexture);

	UFUNCTION(BlueprintCallable, Category = "Highlight")
	void SetHighlightColors(FLinearColor Allowed, FLinearColor NotAllowed/*, FLinearColor Partial*/);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(blueprintReadOnly, Category = "Highlight")
	EHighlightState CurrentState = EHighlightState::NotAllowed;
	
	UPROPERTY(EditDefaultsOnly, Category = "Highlight")
	FLinearColor AllowedColor = FLinearColor::Green;
	UPROPERTY(EditDefaultsOnly, Category = "Highlight")
	FLinearColor NotAllowedColor = FLinearColor::Red;
	UPROPERTY(EditDefaultsOnly, Category = "Highlight")
	FLinearColor PartialColor = FLinearColor::Yellow;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	virtual void SetBordersColor(const FLinearColor& Color);
};

//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "UI/Inventory/InventoryTypes.h"
#include "HighlightSlotWidget.generated.h"

enum class EHighlightState : uint8;
class UCoreCellWidget;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UHighlightSlotWidget : public UBaseUserWidget
{
	GENERATED_BODY()
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UHighlightSlotWidget();
	
	UFUNCTION(BlueprintCallable, Category = "Highlight")
	void SetHighlightState(EHighlightState NewState);

	UFUNCTION(BlueprintCallable, Category = "Highlight")
	virtual void UpdateVisualWithTexture(UTexture2D* NewTexture);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	TObjectPtr<UCoreCellWidget> CoreCellWidget;

	//
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
	virtual void NativeConstruct() override;
	
	virtual void SetBordersColor(const FLinearColor& Color);
};

//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "OperationPanelWidget.generated.h"

class UUIButton;
class UButton;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UOperationPanelWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta=(BindWidgetOptional))
	TObjectPtr<UUIButton> Button_TakeAll;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta=(BindWidgetOptional))
    TObjectPtr<UUIButton> Button_PlaceAll;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta=(BindWidgetOptional))
	TObjectPtr<UUIButton> Button_Sort;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UOperationPanelWidget();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
};

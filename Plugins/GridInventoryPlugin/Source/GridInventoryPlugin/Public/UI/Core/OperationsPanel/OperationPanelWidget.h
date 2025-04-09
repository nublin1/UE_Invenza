// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "OperationPanelWidget.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UOperationPanelWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta=(BindWidgetOptional))
	TObjectPtr<UButton> Button_TakeAll;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta=(BindWidgetOptional))
    TObjectPtr<UButton> Button_PlaceAll;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta=(BindWidgetOptional))
	TObjectPtr<UButton> Button_Sort;

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

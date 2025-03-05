// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CanvasPanel.h"
#include "UI/BaseUserWidget.h"
#include "CoreHUDWidget.generated.h"

class UInteractionWidget;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UCoreHUDWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	//Getters
	UInteractionWidget* GetInteractionWidget() {return InteractionWidget;} 

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UCanvasPanel> ContentPanel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UInteractionWidget> InteractionWidget;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};

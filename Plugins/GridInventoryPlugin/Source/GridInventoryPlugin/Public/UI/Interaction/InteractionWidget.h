// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BaseUserWidget.h"
#include "InteractionWidget.generated.h"


class UProgressBar;
class UTextBlock;
struct FInteractionData;

/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UInteractionWidget : public UBaseUserWidget
{
	GENERATED_BODY()
	
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> KeyPressText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> ButtonName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> ActionText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> QuantityText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> InteractionProgressBar;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UInteractionWidget();

	UFUNCTION(BlueprintNativeEvent)
	void OnFoundInteractable(FInteractionData& NewInteractableData);
	virtual void OnFoundInteractable_Implementation( FInteractionData& NewInteractableData);

	UFUNCTION(BlueprintNativeEvent)
	void OnLostInteractable(FInteractionData& NewInteractableData);
	virtual void OnLostInteractable_Implementation(FInteractionData& NewInteractableData);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void UpdateText(FInteractionData& NewInteractableData);
};

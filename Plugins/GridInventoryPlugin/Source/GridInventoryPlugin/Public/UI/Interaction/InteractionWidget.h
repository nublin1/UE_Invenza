// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "InteractionWidget.generated.h"


struct FInteractableData;
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
	void OnFoundInteractable(FInteractableData& NewInteractableData);
	virtual void OnFoundInteractable_Implementation( FInteractableData& NewInteractableData);

	UFUNCTION(BlueprintNativeEvent)
	void OnLostInteractable(FInteractableData& NewInteractableData);
	virtual void OnLostInteractable_Implementation(FInteractableData& NewInteractableData);

	UFUNCTION()
	void UpdateProgressBar(float Progress);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeConstruct() override;

	UFUNCTION()
	void UpdateText(FInteractableData& NewInteractableData);
};

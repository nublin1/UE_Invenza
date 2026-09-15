// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InteractionRowWidget.h"
#include "Data/Interaction/InteractionData.h"
#include "UI/InvenzaBaseWidget.h"
#include "InteractionWidget.generated.h"


struct FInteractableData;
class UProgressBar;
class UTextBlock;
struct FInteractionData;

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInteractionWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()
	
public:
	UInteractionWidget();

protected:
	virtual void NativeConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction UI", meta = (BindWidget))
	TObjectPtr<UInteractionRowWidget> FirstRow;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction UI", meta = (BindWidgetOptional))
	TObjectPtr<UInteractionRowWidget> SecondRow;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction UI", meta = (BindWidgetOptional))
	TObjectPtr<UInteractionRowWidget> ThirdRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction UI", meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> InteractionProgressBar;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction Logic")
	void OnFoundInteractable(const TArray<FInteractionDisplayEntry>& Entries);
	virtual void OnFoundInteractable_Implementation(const TArray<FInteractionDisplayEntry>& Entries);

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction Logic")
	void OnLostInteractable(const TArray<FInteractionDisplayEntry>& Entries);
	virtual void OnLostInteractable_Implementation(const TArray<FInteractionDisplayEntry>& Entries);
	

	UFUNCTION(BlueprintCallable, Category = "UI Updates")
	void UpdateProgressBar(float Progress);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	
};

// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "Data/Interactable/InteractableData.h"
#include "UI/InvenzaBaseWidget.h"
#include "InteractionRowWidget.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInteractionRowWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()
	
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction UI", meta = (BindWidget))
	TObjectPtr<UTextBlock> KeyPressText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction UI", meta = (BindWidget))
	TObjectPtr<UTextBlock> ActionText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction UI", meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction UI", meta = (BindWidget))
	TObjectPtr<UTextBlock> QuantityText;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION(BlueprintCallable)
	void SetRowData(const FText& KeyLabel, const FInteractableData& Data, bool bHoldToInteract);
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};

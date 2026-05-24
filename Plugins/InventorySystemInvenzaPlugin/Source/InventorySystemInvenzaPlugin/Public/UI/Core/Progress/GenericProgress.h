// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CraftSystem/ItemRecipe.h"
#include "UI/InvenzaBaseWidget.h"
#include "GenericProgress.generated.h"

class UProgressPercentTimer;
struct FQueuedRecipe;
class UCraftingQuantitySelector;
class ULabelBaseText;
class UImageBaseWidget;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UGenericProgress : public UInvenzaBaseWidget
{
	GENERATED_BODY()
	
public:
	UGenericProgress();

protected:
	//virtual void NativePreConstruct() override;
	//virtual void NativeConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UImageBaseWidget> Icon;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> DisplayName;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UProgressPercentTimer> ProgressPercentTimerWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UCraftingQuantitySelector> CraftingQuantitySelectorMini;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void SetNewCraft(const FQueuedRecipe& NewQueuedRecipe);

	UFUNCTION()
	void UpdateProgress(float NewValue);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY()
	FItemRecipeRow CurrentItemRecipeRow;
};

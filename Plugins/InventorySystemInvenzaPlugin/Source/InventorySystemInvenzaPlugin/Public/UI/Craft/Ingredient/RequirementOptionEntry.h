// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Core/Buttons/UIButton.h"
#include "RequirementOptionEntry.generated.h"

class UUIButton;
struct FRecipeRequirementResult;
class UCurrentMaxDisplay;
class ULabelBaseText;
class UImageBaseWidget;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API URequirementOptionEntry : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	URequirementOptionEntry();

protected:
	virtual void NativeConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	// Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidgetOptional))
	TObjectPtr<UImageBaseWidget> IngredientIcon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> RequiredItemName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidget))
	TObjectPtr<UCurrentMaxDisplay> RemainingCounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> MainButton;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	UFUNCTION(BlueprintCallable)
	void UpdateData(FRecipeRequirementResult NewData);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable)
	void UpdateIngredientImage(const TSoftObjectPtr<UTexture2D>& NewIngredientIcon);

	UFUNCTION(BlueprintCallable)
	void SetToggleStatus(bool bNewStatus);
};

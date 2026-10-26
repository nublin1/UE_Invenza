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
	// Unreal overrides
	virtual void NativeConstruct() override;

public:
	// Properties and variables
	// Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidgetOptional))
	TObjectPtr<UImageBaseWidget> IngredientIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> RequiredItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidget))
	TObjectPtr<UCurrentMaxDisplay> RemainingCounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> MainButton;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UImageBaseWidget> SelectedMarker;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UImageBaseWidget> SelectionBackground;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> MissingResourcesText;

	// Functions
	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable)
	void UpdateData(const FRecipeRequirementResult& NewData);

protected:
	// Runtime

	// Functions
	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable)
	void UpdateIngredientImage(const TSoftObjectPtr<UTexture2D>& NewIngredientIcon);

	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable)
	void SetToggleStatus(bool bNewStatus);
};

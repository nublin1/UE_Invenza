// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "Data/CraftSystem/ItemRecipe.h"
#include "CraftMenuDetail.generated.h"

class UReceptDetailRequiredListSimple;
class UCraftingQuantitySelector;
class UUIButton;
class URecipeRequiredIListEntryObject;
struct FRecipeCheckResult;
struct FItemRecipeRow;
class UImageBaseWidget;
class UMultiLineEditableTextBox;
class UCraftMenuRecipeActions;
class UWidgetSwitcher;
class ULabelBaseText;
/**
 *
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UCraftMenuDetail : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UCraftMenuDetail();

protected:
	// Unreal overrides
	virtual void NativeConstruct() override;

public:
	// Properties and variables
	// Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidget))
	TObjectPtr<UImageBaseWidget> RecipeImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidget))
	TObjectPtr<UReceptDetailRequiredListSimple> RecipeDetailRequiredListSimple;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidget))
	TObjectPtr<UCraftMenuRecipeActions> CraftMenuActionButtons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidget))
	TObjectPtr<UCraftingQuantitySelector> CraftingQuantitySelector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> RecipeTabsSwitcher;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> RecipeName;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> RecipeDescription;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> RecipeWorkAmount;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> RecipeOutput;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> NoRequirementsText;

	// Data for the future description/work widgets; does not require a Blueprint layout change.
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Crafting|Detail")
	FItemRecipeRow CurrentRecipe;

	// Functions
	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable)
	void SetCraftDetail(FItemRecipeRow RecipeRow, FRecipeCheckResult CheckResult);

	void ClearDetail();

	UFUNCTION(BlueprintImplementableEvent, Category = "Crafting|Detail")
	void OnRecipeDataChanged();

protected:
	// Runtime

	// Functions
	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable)
	void OnClickedTabRecipeRequireds(UUIButton* ButtonPressed);

	UFUNCTION()
	void OnClickedTabRecipeDescription(UUIButton* ButtonPressed);
};

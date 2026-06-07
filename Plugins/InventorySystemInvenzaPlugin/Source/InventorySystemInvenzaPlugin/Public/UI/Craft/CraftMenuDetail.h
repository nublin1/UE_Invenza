// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
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
	virtual void NativeConstruct() override;

public:	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
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
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable)
	void SetCraftDetail(FItemRecipeRow RecipeRow, FRecipeCheckResult CheckResult);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
		
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable)
	void OnClickedTabRecipeRequireds(UUIButton* ButtonPressed);
	UFUNCTION()
	void OnClickedTabRecipeDescription(UUIButton* ButtonPressed);
};


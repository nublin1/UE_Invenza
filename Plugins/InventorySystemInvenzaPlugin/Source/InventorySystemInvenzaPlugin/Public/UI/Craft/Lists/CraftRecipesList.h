// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "Data/CraftSystem/Entries/RecipeListEntryObject.h"
#include "UI/InvenzaBaseWidget.h"
#include "CraftRecipesList.generated.h"

class UFiltersPanel;
struct FItemRecipeRow;
class URecipeListEntryObject;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UCraftRecipesList : public UInvenzaBaseWidget
{
	GENERATED_BODY()
	
	
public:
	UCraftRecipesList();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(EditAnywhere, Category = "UI|Components", meta=(BindWidgetOptional))
	TObjectPtr<UFiltersPanel> ItemFiltersPanel;
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UListView> AvailableRecipesList;

	//Data
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Data")
	TSubclassOf<URecipeListEntryObject> RecipeEntryObjectClass;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Inventory|Data")
	TArray<TObjectPtr<URecipeListEntryObject>> ItemsArray;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Inventory|Data")
	TArray<TObjectPtr<URecipeListEntryObject>> FilteredItemsArray;
		
	//====================================================================
	// FUNCTIONS
	//====================================================================
	void SetRecipes(const TArray<FItemRecipeRow>& Recipes);
	void RefreshList();
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Data
	UPROPERTY()
	TArray<FItemRecipeRow> RecipesData;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION()
	virtual void SearchTextChanged(const FText& NewText);
};

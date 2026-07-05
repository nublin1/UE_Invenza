// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "Data/CraftSystem/Entries/RecipeListEntryObject.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Core/List/SimpleUserObjectList.h"
#include "CraftRecipesList.generated.h"

class UFiltersPanel;
struct FItemRecipeRow;
class URecipeListEntryObject;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UCraftRecipesList : public USimpleUserObjectList
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
	
	virtual void SearchTextChanged(const FText& NewText) override;
};

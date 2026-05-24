// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CraftSystem/ItemRecipe.h"
#include "UI/UIStructs.h"
#include "UObject/Object.h"
#include "RecipeListEntryObject.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API URecipeListEntryObject : public UObject
{
	GENERATED_BODY()

public:
	URecipeListEntryObject(){}
	
	UPROPERTY()
	FItemRecipeRow RecipeRow;
	
	UPROPERTY()
	FText Text;

	UPROPERTY()
	FUIBrushStyle BrushStyle;
};

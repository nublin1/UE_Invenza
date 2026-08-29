// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CraftSystem/CraftingStructs.h"
#include "Data/CraftSystem/ItemRecipe.h"
#include "UObject/Object.h"
#include "RecipeRequiredIListEntryObject.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API URecipeRequiredIListEntryObject : public UObject
{
	GENERATED_BODY()
	
	
public:
	UPROPERTY(BlueprintReadOnly)
	FItemRecipeRow RecipeRow;

	UPROPERTY(BlueprintReadOnly)
	FRecipeItemRequirementCheck RecipeCheckResult;

	UPROPERTY(BlueprintReadOnly)
	int32 Index = INDEX_NONE;
	
	UPROPERTY(BlueprintReadOnly)
	int32 SelectedOptionIndex = 0;
};

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
	// Properties and variables
	FSimpleMulticastDelegate OnDataChanged;

	FSimpleMulticastDelegate OnSelectionChanged;

	UPROPERTY(Category = "Invenza|Crafting", BlueprintReadOnly)
	FItemRecipeRow RecipeRow;

	UPROPERTY(Category = "Invenza|Crafting", BlueprintReadOnly)
	FRecipeItemRequirementCheck RecipeCheckResult;

	UPROPERTY(Category = "Invenza|Crafting", BlueprintReadOnly)
	int32 Index = INDEX_NONE;

	UPROPERTY(Category = "Invenza|Crafting", BlueprintReadOnly)
	int32 SelectedOptionIndex = 0;
};

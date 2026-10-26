// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CraftSystem/CraftingStructs.h"
#include "UObject/Object.h"
#include "ProductionQueueListEntryObject.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UProductionQueueListEntryObject : public UObject
{
	GENERATED_BODY()

public:
	UProductionQueueListEntryObject() {};

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDataChanged);
#pragma endregion Delegates

	// Properties and variables
	UPROPERTY(BlueprintAssignable, Category = "Crafting")
	FOnDataChanged OnDataChanged;

	// Functions
	FQueuedRecipe GetQueuedRecipeData() { return QueuedRecipeData; }

	void SetQueuedRecipe(const FQueuedRecipe& NewQueuedRecipe) { QueuedRecipeData = NewQueuedRecipe; }

	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable)
	void UpdateData(int32 NewCount, float NewProgress);

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Category = "Invenza|Crafting", BlueprintReadOnly)
	FQueuedRecipe QueuedRecipeData;
};

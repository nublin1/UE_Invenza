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

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDataChanged);
#pragma endregion
	
public:
	UProductionQueueListEntryObject() {};

	UPROPERTY(BlueprintAssignable, Category = "Crafting")
	FOnDataChanged OnDataChanged;
	
	FQueuedRecipe GetQueuedRecipeData() {return QueuedRecipeData;}

	void SetQueuedRecipe(const FQueuedRecipe& NewQueuedRecipe) {QueuedRecipeData = NewQueuedRecipe;}

	UFUNCTION(BlueprintCallable)
	void UpdateData(int32 NewCount, float NewProgress);

protected:
	UPROPERTY(BlueprintReadOnly)
	FQueuedRecipe QueuedRecipeData;
};

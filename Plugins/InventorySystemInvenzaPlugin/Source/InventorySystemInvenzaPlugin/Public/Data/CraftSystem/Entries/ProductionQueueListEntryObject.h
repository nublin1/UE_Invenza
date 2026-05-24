// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CraftSystem/ItemRecipe.h"
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
	
	UPROPERTY(BlueprintReadOnly)
	FItemRecipeRow RecipeRow;

	UPROPERTY(BlueprintReadOnly)
	int32 AmountInQueue = 0;

	UPROPERTY(BlueprintReadOnly)
	float CurrentProgress = 0.f;
};

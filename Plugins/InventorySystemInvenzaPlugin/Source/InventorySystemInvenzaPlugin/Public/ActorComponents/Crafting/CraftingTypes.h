// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CraftSystem/ItemRecipe.h"
#include "CraftingTypes.generated.h"

UENUM(BlueprintType)
enum class ECraftingResourceConsumePolicy : uint8
{
	OnQueueAdd      UMETA(DisplayName="Consume On Queue Add"),
	OnCraftStart    UMETA(DisplayName="Consume On Craft Start"),
	OnCraftFinish   UMETA(DisplayName="Consume On Craft Finish")
};

USTRUCT(BlueprintType)
struct FQueuedRecipe
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemRecipeRow ItemRecipeRow;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentProgress = 0.f;
	
	FQueuedRecipe() = default;
	FQueuedRecipe(FItemRecipeRow InItemRecipeRow, int32 InCount) : ItemRecipeRow(InItemRecipeRow), Count(InCount) {}
};

USTRUCT(BlueprintType)
struct FAlternativeItemRequirementCheck
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSatisfied = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AmountNeed = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AmountHave = 0;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//int32 MissingQuantity = 0;

};

USTRUCT(BlueprintType)
struct FRecipeItemRequirementCheck
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredItemID;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSatisfied = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AmountNeed = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AmountHave = 0;
	
	/* Not Used */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//int32 MissingQuantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAlternativeItemRequirementCheck> AlternativeRequirementCheck;
};

USTRUCT(BlueprintType)
struct FRecipeCheckResult
{
	GENERATED_BODY()

	// Список результатов по каждой строке рецепта
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRecipeItemRequirementCheck> Requirements;

	// Можно ли запустить крафт
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanCraft = false;
};
	
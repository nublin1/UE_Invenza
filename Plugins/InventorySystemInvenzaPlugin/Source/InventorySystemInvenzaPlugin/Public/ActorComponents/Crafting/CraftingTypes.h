// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CraftSystem/ItemRecipe.h"
#include "Data/ItemDataStructures.h"
#include "CraftingTypes.generated.h"

UENUM(BlueprintType)
enum class ECraftingResourceConsumePolicy : uint8
{
	OnQueueAdd      UMETA(DisplayName="Consume On Queue Add"),
	OnCraftStart    UMETA(DisplayName="Consume On Craft Start"),
	OnCraftFinish   UMETA(DisplayName="Consume On Craft Finish")
};

USTRUCT(BlueprintType)
struct FCraftingComponentConfig
{
	GENERATED_BODY()

	// Inventory tags used for automatic inventory discovery.
	// The Inventory Manager searches for inventories with these tags and,
	// when found, automatically assigns them to InputInventory and OutputInventory.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Crafting")
	FGameplayTag InputInventoryTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Crafting")
	FGameplayTag OutputInventoryTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crafting")
	TArray<FDataTableRowHandle> StartingRecipes;
};

USTRUCT(BlueprintType)
struct FBlockReasonData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag Tag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Message;

	bool operator==(const FBlockReasonData& Other) const
	{
		return Tag == Other.Tag;
	}
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	mutable bool bResourcesWasConsumed = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FInitItemsEntry> ConsumedResources;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<int32> SelectedOptions;
	
	FQueuedRecipe() = default;
	FQueuedRecipe(FItemRecipeRow InItemRecipeRow, int32 InCount, bool bInResourcesWasConsumed = false)
	: ItemRecipeRow(InItemRecipeRow), Count(InCount), bResourcesWasConsumed(bInResourcesWasConsumed) {}
};

USTRUCT(BlueprintType)
struct FRecipeRequirementResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredItemID;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FItemMetaData ItemMetaData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSatisfied = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AmountNeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AmountHave = 0;
};

USTRUCT(BlueprintType)
struct FRecipeItemRequirementCheck
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRecipeRequirementResult Primary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRecipeRequirementResult> Alternatives;
};

USTRUCT(BlueprintType)
struct FRecipeCheckResult
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRecipeItemRequirementCheck> Requirements;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanCraft = false;

	UPROPERTY(BlueprintReadOnly)
	TArray<FInitItemsEntry> ResourcesToConsume;
};

USTRUCT(BlueprintType)
struct FCachedRecipeResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	FName RecipeID;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	FRecipeCheckResult CheckResult;

	FCachedRecipeResult() : RecipeID(NAME_None) {}
	FCachedRecipeResult(FName InRecipeID, const FRecipeCheckResult& InResult) 
		: RecipeID(InRecipeID), CheckResult(InResult) {}
};
	
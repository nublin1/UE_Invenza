// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CraftSystem/ItemRecipe.h"
#include "Data/ItemDataStructures.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "CraftingStructs.generated.h"

USTRUCT(BlueprintType)
struct FCraftingInventoryOverrides
{
	GENERATED_BODY()

	// Optional override for the global default input inventory tag.
	// When specified, the Inventory Manager prioritizes this tag when
	// automatically discovering and assigning the InputInventory.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Crafting")
	FGameplayTag InputInventoryTag;

	// Optional override for the global default output inventory tag.
	// When specified, the Inventory Manager prioritizes this tag when
	// automatically discovering and assigning the OutputInventory.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Crafting")
	FGameplayTag OutputInventoryTag;

	// Optional override for the global default fuel inventory tag.
	// When specified, the Inventory Manager prioritizes this tag when
	// automatically discovering and assigning the FuelInventory.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Crafting")
	FGameplayTag FuelInventoryTag;
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
struct FQueuedRecipe : public FFastArraySerializerItem 
{
	GENERATED_BODY()
	
	UPROPERTY()
	FGuid QueueEntryId = FGuid::NewGuid();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemRecipeRow ItemRecipeRow;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 0;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentProgress = 0.f;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bResourcesWasConsumed = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 SortOrder = 0;
    
	FQueuedRecipe() = default;
	FQueuedRecipe(FItemRecipeRow InItemRecipeRow, int32 InCount, bool bInResourcesWasConsumed = false, int32 NewSortOrder = 0)
	: ItemRecipeRow(InItemRecipeRow), Count(InCount), bResourcesWasConsumed(bInResourcesWasConsumed), SortOrder(NewSortOrder) {}
};

USTRUCT(BlueprintType)
struct FRecipeQueueContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FQueuedRecipe> Items;
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FQueuedRecipe, FRecipeQueueContainer>(Items, DeltaParms, *this);
	}
};

template<>
struct TStructOpsTypeTraits<FRecipeQueueContainer> : public TStructOpsTypeTraitsBase2<FRecipeQueueContainer>
{
	enum { WithNetDeltaSerializer = true };
};

USTRUCT(BlueprintType)
struct FCraftAdditionalData
{
	GENERATED_BODY()
	
	UPROPERTY()
	int32 TargetRepID = -10;

	UPROPERTY(BlueprintReadOnly)
	TArray<FInitItemsEntry> ConsumedResources;

	UPROPERTY(BlueprintReadOnly)
	TArray<int32> SelectedOptions;

	FCraftAdditionalData() = default;
	FCraftAdditionalData(int32 InRepID, const TArray<int32>& InOptions) 
		: TargetRepID(InRepID), SelectedOptions(InOptions) {}
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
	
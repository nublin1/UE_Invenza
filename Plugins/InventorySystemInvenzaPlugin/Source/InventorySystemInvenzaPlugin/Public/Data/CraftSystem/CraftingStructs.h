// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TimerManager.h"
#include "Data/CraftSystem/ItemRecipe.h"
#include "Data/ItemDataStructures.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "CraftingStructs.generated.h"

class UInventoryBase;

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FCraftingInventoryOverrides
{
	GENERATED_BODY()

	// Optional override for the global default input inventory tag.
	// When specified, the Inventory Manager prioritizes this tag when
	// automatically discovering and assigning the InputInventory.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
	FGameplayTag InputInventoryTag;

	// Optional override for the global default output inventory tag.
	// When specified, the Inventory Manager prioritizes this tag when
	// automatically discovering and assigning the OutputInventory.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
	FGameplayTag OutputInventoryTag;

	// Optional override for the global default fuel inventory tag.
	// When specified, the Inventory Manager prioritizes this tag when
	// automatically discovering and assigning the FuelInventory.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
	FGameplayTag FuelInventoryTag;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FBlockReasonData
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Crafting", EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag Tag;

	UPROPERTY(Category = "Invenza|Crafting", EditDefaultsOnly, BlueprintReadOnly)
	FText Message;

	bool operator==(const FBlockReasonData& Other) const { return Tag == Other.Tag; }
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FQueuedRecipe : public FFastArraySerializerItem
{
	GENERATED_BODY()

	// A fresh identity is intentional; independently constructed instances cannot compare equal.
	UPROPERTY(meta = (IgnoreForMemberInitializationTest))
	FGuid QueueEntryId = FGuid::NewGuid();

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	FItemRecipeRow ItemRecipeRow;

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	int32 Count = 0;

	UPROPERTY(Category = "Invenza|Crafting", VisibleAnywhere, BlueprintReadOnly)
	float CurrentProgress = 0.f;

	UPROPERTY(Category = "Invenza|Crafting", VisibleAnywhere, BlueprintReadOnly)
	bool bResourcesWasConsumed = false;

	UPROPERTY(Category = "Invenza|Crafting", VisibleAnywhere, BlueprintReadOnly)
	int32 SortOrder = 0;

	FQueuedRecipe() = default;
	FQueuedRecipe(FItemRecipeRow InItemRecipeRow, int32 InCount, bool bInResourcesWasConsumed = false,
				  int32 NewSortOrder = 0)
		: ItemRecipeRow(InItemRecipeRow), Count(InCount), bResourcesWasConsumed(bInResourcesWasConsumed),
		  SortOrder(NewSortOrder)
	{
	}
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FRecipeQueueContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FQueuedRecipe> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FQueuedRecipe, FRecipeQueueContainer>(Items, DeltaParms,
																								   *this);
	}
};

template <> struct TStructOpsTypeTraits<FRecipeQueueContainer> : public TStructOpsTypeTraitsBase2<FRecipeQueueContainer>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FCraftAdditionalData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 TargetRepID = -10;

	UPROPERTY(Category = "Invenza|Crafting", BlueprintReadOnly)
	TArray<FInitItemsEntry> ConsumedResources;

	UPROPERTY(Category = "Invenza|Crafting", BlueprintReadOnly)
	TArray<int32> SelectedOptions;

	FCraftAdditionalData() = default;
	FCraftAdditionalData(int32 InRepID, const TArray<int32>& InOptions)
		: TargetRepID(InRepID), SelectedOptions(InOptions)
	{
	}
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FRecipeRequirementResult
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	FName RequiredItemID;

	UPROPERTY(Category = "Invenza|Crafting", VisibleAnywhere, BlueprintReadWrite)
	FItemMetaData ItemMetaData;

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	bool bIsSatisfied = false;

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	int32 AmountNeed = 0;

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	int32 AmountHave = 0;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FRecipeItemRequirementCheck
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	FRecipeRequirementResult Primary;

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	TArray<FRecipeRequirementResult> Alternatives;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FRecipeCheckResult
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	TArray<FRecipeItemRequirementCheck> Requirements;

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	bool bCanCraft = false;

	UPROPERTY(Category = "Invenza|Crafting", BlueprintReadOnly)
	TArray<FInitItemsEntry> ResourcesToConsume;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FCachedRecipeResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	FName RecipeID;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	FRecipeCheckResult CheckResult;

	FCachedRecipeResult() : RecipeID(NAME_None) {}
	FCachedRecipeResult(FName InRecipeID, const FRecipeCheckResult& InResult)
		: RecipeID(InRecipeID), CheckResult(InResult)
	{
	}
};

// A snapshot of removed resources, or items waiting for delivery.
USTRUCT(Blueprintable)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FCraftItemBatch
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Crafting", BlueprintReadOnly)
	TObjectPtr<UInventoryBase> Inventory = nullptr;

	UPROPERTY(Category = "Invenza|Crafting", BlueprintReadOnly)
	TObjectPtr<UObject> Sample = nullptr;

	UPROPERTY(Category = "Invenza|Crafting", BlueprintReadOnly)
	FName ItemID = NAME_None;

	UPROPERTY(Category = "Invenza|Crafting", BlueprintReadOnly)
	int32 Amount = 0;
};

USTRUCT(Blueprintable)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FCraftReservation
{
	GENERATED_BODY()

	// Number of fully paid craft iterations.
	UPROPERTY(Category = "Invenza|Crafting", BlueprintReadOnly)
	int32 PaidIterations = 0;

	// Cost of ONE iteration, with alternatives already selected.
	UPROPERTY(Category = "Invenza|Crafting", BlueprintReadOnly)
	TArray<FInitItemsEntry> UnitCost;

	// Resources that have not yet been committed to finished products.
	UPROPERTY(Category = "Invenza|Crafting", BlueprintReadOnly)
	TArray<FCraftItemBatch> Resources;
};

struct FCraftIngredientOption
{
	FDataTableRowHandle Handle;
	FName ItemID = NAME_None;
	int32 Amount = 0;
	bool bValid = false;
};
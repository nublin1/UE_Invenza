// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/ItemDataStructures.h"
#include "ItemRecipe.generated.h"

class UResourcesDT;

#pragma region enums
UENUM(BlueprintType)
enum class ERecipeOperatorType : uint8
{
	SelfProduce,
	PlayerRequired,
	AnyRequired,
};
#pragma endregion

#pragma region Structs
USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FAlternativeItem
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	FDataTableRowHandle Item;
	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FRecipeItemRequirement
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	FDataTableRowHandle Item;
	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	TArray<FAlternativeItem> Alternatives;
};

#pragma endregion

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FItemRecipeRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	FName ID;

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	FText DisplayName;
	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> RecipeIcon;

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	TArray<FRecipeItemRequirement> RequiredItems;

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	ERecipeOperatorType OperatorType = ERecipeOperatorType::SelfProduce;

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	float CraftVolume = 1.f;

	UPROPERTY(Category = "Invenza|Crafting", EditAnywhere, BlueprintReadWrite)
	TArray<FInitItemsEntry> OutputItems;
};
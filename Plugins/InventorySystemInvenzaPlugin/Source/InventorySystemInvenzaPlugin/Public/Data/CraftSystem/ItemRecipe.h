// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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
struct FAlternativeItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDataTableRowHandle Item;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;
};

USTRUCT(BlueprintType)
struct FRecipeItemRequirement
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDataTableRowHandle Item;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAlternativeItem> Alternatives;
};

#pragma endregion

USTRUCT(BlueprintType)
struct FItemRecipeRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> RecipeIcon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRecipeItemRequirement> RequiredItems;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERecipeOperatorType OperatorType = ERecipeOperatorType::SelfProduce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CraftVolume = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FInitItemsEntry> OutputItems;
};
//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectPtr.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "GameplayTagContainer.h"
#include "ItemDataStructures.generated.h"

UENUM(BlueprintType)
enum class EItemOrientationType : uint8
{
	Vertical UMETA(DisplayName = "Vertical"),
	Horizontal UMETA(DisplayName = "Hotizontal"),
};

UENUM(BlueprintType)
enum class EStorageMethod : uint8
{
	SingleMesh	UMETA(DisplayName = "SingleMesh"),
	Logs	UMETA(DisplayName = "Logs"),
};

USTRUCT(BlueprintType)
struct FInitItemsEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDataTableRowHandle Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount;
};

USTRUCT(BlueprintType)
struct FInitItemsList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FInitItemsEntry> Items;
};

USTRUCT(BlueprintType)
struct FItemIDEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	int32 Amount = 0;
};

USTRUCT(BlueprintType)
struct FItemTextData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Text")
	FText DisplayName;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Text")
	//FText InteractionText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Text")
	FText ItemDescription;
};

USTRUCT()
struct FItemAssetData
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Item|Assets")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, Category = "Item|Assets")
	TObjectPtr<UStaticMesh> Mesh;
};

USTRUCT(BlueprintType)
struct FItemNumeraticData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stats")
	float Weight;

	// Size when the item is stored in a character inventory (Player/NPC)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Inventory Size",
		meta = (ToolTip = "Number of horizontal slots the item occupies in a character inventory"))
	int32 InventoryHorizontalSlots = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Inventory Size",
		meta = (ToolTip = "Number of vertical slots the item occupies in a character inventory"))
	int32 InventoryVerticalSlots = 1;

	//==============================
	// STACKING
	//==============================
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stats")
	int32 MaxStackSizeInCharacter;
	
	FItemNumeraticData(): Weight(1), MaxStackSizeInCharacter(1)
	{
	}
};

USTRUCT(BlueprintType)
struct FItemTradeData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Trade")
	bool bCanBeSold = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Trade")
	float BasePrice = 0.0f;
};

USTRUCT(BlueprintType)
struct FItemStorageData
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|StorageData")
	EStorageMethod StorageMethod;
	
	UPROPERTY(EditAnywhere, Category = "Item|StorageData")
	TObjectPtr<UStaticMesh> StorageMesh;
};


USTRUCT(BlueprintType)
struct FItemMetaData
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Item|Metadata")
	FItemAssetData ItemAssetData;

	UPROPERTY(EditAnywhere, Category = "Item|Metadata")
	FItemTextData ItemTextData;

	UPROPERTY(EditAnywhere, Category = "Item|Metadata")
	FItemNumeraticData ItemNumeraticData;

	UPROPERTY(EditAnywhere, Category = "Item|Metadata")
	FItemTradeData ItemTradeData;
	
	UPROPERTY(EditAnywhere, Category = "Item|Metadata")
	FItemStorageData ItemStorageData;

	// CATEGORY
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Metadata")
	FGameplayTag ItemCategory;
	
	// Behavor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Metadata")
	bool bIsDroppable = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Metadata")
	bool bIsDeletable = true;
};

//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
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
	SingleMesh UMETA(DisplayName = "SingleMesh"),
	Logs UMETA(DisplayName = "Logs"),
	SmallBoxes UMETA(DisplayName = "SmallBoxes"),
	BulkBins UMETA(DisplayName = "Bulk Bins"),
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInitItemsEntry
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Items", EditAnywhere, BlueprintReadWrite)
	FDataTableRowHandle Item;

	UPROPERTY(Category = "Invenza|Items", EditAnywhere, BlueprintReadWrite)
	int32 Amount = 0;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInitItemsList
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|Items", EditAnywhere, BlueprintReadWrite)
	TArray<FInitItemsEntry> Items;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FItemIDEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Amount = 0;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FItemTextData
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
struct INVENTORYSYSTEMINVENZAPLUGIN_API FItemAssetData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Item|Assets")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, Category = "Item|Assets")
	TObjectPtr<UStaticMesh> Mesh;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FItemNumeraticData
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

	FItemNumeraticData() : Weight(1), MaxStackSizeInCharacter(1) {}
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FItemTradeData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Trade")
	bool bCanBeSold = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Trade")
	float BasePrice = 0.0f;
};

UENUM(BlueprintType)
enum class EStorageMeshAxis : uint8
{
	X UMETA(DisplayName = "X"),
	Y UMETA(DisplayName = "Y")
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FItemStorageData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|StorageData")
	EStorageMethod StorageMethod = EStorageMethod::SingleMesh;

	UPROPERTY(EditAnywhere, Category = "Item|StorageData")
	TObjectPtr<UStaticMesh> StorageMesh;

	/** Longitudinal axis of the source mesh. Currently used by Logs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|StorageData")
	EStorageMeshAxis MeshLengthAxis = EStorageMeshAxis::X;

	/** Optional material override for the individual chunks used by BulkBins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|StorageData",
			  meta = (EditCondition = "StorageMethod == EStorageMethod::BulkBins", EditConditionHides))
	TObjectPtr<UMaterialInterface> BulkMaterial;

	/** Height of a full pile above its base, in grid-local centimeters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|StorageData",
			  meta = (EditCondition = "StorageMethod == EStorageMethod::BulkBins", EditConditionHides, ClampMin = "1.0",
					  Units = "cm"))
	float BulkFillHeight = 65.0f;

	/** Approximate chunk diameter; visual chunks represent the stack's fill fraction, not individual units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|StorageData",
			  meta = (EditCondition = "StorageMethod == EStorageMethod::BulkBins", EditConditionHides, ClampMin = "1.0",
					  Units = "cm"))
	float BulkChunkSize = 22.0f;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FItemMetaData
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

	/** Independent traits, such as fuel, without changing the item's inventory category. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Metadata")
	FGameplayTagContainer ItemTags;

	// Behavor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Metadata")
	bool bIsDroppable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Metadata")
	bool bIsDeletable = true;
};

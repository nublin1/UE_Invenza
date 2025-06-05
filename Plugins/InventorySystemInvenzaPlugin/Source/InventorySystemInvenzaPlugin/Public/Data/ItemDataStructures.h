#pragma once

#include "CoreMinimal.h"
#include "ItemDataStructures.generated.h"

UENUM(BlueprintType)
enum class EOrientationType : uint8
{
	Vertical UMETA(DisplayName = "Vertical"),
	Hotizontal UMETA(DisplayName = "Hotizontal"),
};

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	None        UMETA(DisplayName = "None"),
	Consumable  UMETA(DisplayName = "Consumable"),
	Money       UMETA(DisplayName = "Money"),
	Weapon      UMETA(DisplayName = "Weapon"),
	Armor       UMETA(DisplayName = "Armor"),
};

USTRUCT(BlueprintType)
struct FItemTextData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Text")
	FText Name;

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

	/*UPROPERTY(EditAnywhere, Category = "Item|Assets", meta = (ToolTip = "Used in ItemSlots"))
	TObjectPtr<UTexture2D> AlternativeIcon;*/

	UPROPERTY(EditAnywhere, Category = "Item|Assets")
	TObjectPtr<UStaticMesh> Mesh;
};

USTRUCT(BlueprintType)
struct FItemNumeraticData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stats")
	float Weight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stats")
	int32 MaxStackSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stats",
		meta = (ToolTip = "Number of horizontal slots occupied by the item"))
	int32 NumHorizontalSlots = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stats",
		meta = (ToolTip = "Number of vertical slots occupied by the item"))
	int32 NumVerticalSlots = 1;
	
	FItemNumeraticData()
		: Weight(1), MaxStackSize(1)
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Metadata")
	EItemCategory ItemCategory = EItemCategory::Armor;

	// Later
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data", meta = (Bitmask, BitmaskEnum = "/Script/GridInventoryPlugin.EItemCategory"))
	//int32 ItemCategory2;
};

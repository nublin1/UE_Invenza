#pragma once

#include "CoreMinimal.h"
#include "ItemDataStructures.generated.h"

UENUM()
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

USTRUCT()
struct FItemTextData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	FText Name;

	UPROPERTY(EditAnywhere)
	FText InteractionText;

	UPROPERTY(EditAnywhere)
	FText ItemDescription;
};

USTRUCT()
struct FItemAssetData
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Used in ItemSlots"))
	TObjectPtr<UTexture2D> AlternativeIcon;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMesh> Mesh;
};

USTRUCT(BlueprintType)
struct FItemNumeraticData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Weight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxStackSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Number of horizontal slots occupied by the item"))
	int32 NumHorizontalSlots = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Number of vertical slots occupied by the item"))
	int32 NumVerticalSlots = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanBeSold = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BasePrice = 0.0f;

	FItemNumeraticData()
		: Weight(1), MaxStackSize(1)
	{
	}
};

USTRUCT(BlueprintType)
struct FItemMetaData
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Item Data")
	FItemAssetData ItemAssetData;

	UPROPERTY(EditAnywhere, Category = "Item Data")
	FItemTextData ItemTextData;

	UPROPERTY(EditAnywhere, Category = "Item Data")
	FItemNumeraticData ItemNumeraticData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	EItemCategory ItemCategory = EItemCategory::Armor;

	// Later
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data", meta = (Bitmask, BitmaskEnum = "/Script/GridInventoryPlugin.EItemCategory"))
	//int32 ItemCategory2;
};
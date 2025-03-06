#pragma once

#include "CoreMinimal.h"
#include "ItemDataStructures.generated.h"

UENUM()
enum class EOrientationType : uint8
{
	Vertical UMETA(DisplayName = "Vertical"),
	Hotizontal UMETA(DisplayName = "Hotizontal"),
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

USTRUCT()
struct FItemNumeraticData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	float Weight;

	UPROPERTY(EditAnywhere)
	int32 MaxStackSize;

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
};
#pragma once

#include "CoreMinimal.h"
#include "ItemDataStructures.h"
#include "ItemData.generated.h"

struct FItemAssetData;

USTRUCT()
struct FItemData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Item Data")
	FName ID;

	UPROPERTY(EditAnywhere, Category = "Item Data")
	FItemMetaData ItemMetaData;
};
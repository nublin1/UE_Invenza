// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Items/itemBase.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"
#include "Data/Inventory/InventoryTypes.h"
#include "SaveLoadStructs.generated.h"

struct FInventorySlotData;

USTRUCT(BlueprintType)
struct FItemMappingSaveEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FString InventoryID;
	
	UPROPERTY()
	TArray<FIntPoint> OccupiedCells;

	UPROPERTY()
	EItemOrientationType ItemOrientation = EItemOrientationType::Horizontal;

	UPROPERTY()
	bool bIsReferenceContainer = false;
};

USTRUCT(BlueprintType)
struct FItemSaveEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FName ItemID;
	UPROPERTY()
	int32 Quantity = 0;
	UPROPERTY()
	FDataTableRowHandle SourceItemRow;

	UPROPERTY()
	TArray<FItemMappingSaveEntry> Mappings;
};

FORCEINLINE uint32 GetTypeHash(const FItemSaveEntry& Data)
{
	return GetTypeHash(Data.ItemID);
}


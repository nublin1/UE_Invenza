#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/Items/itemBase.h"
#include "UI/Container/InvBaseContainerWidget.h"
#include "UI/Inventory/InventoryTypes.h"
#include "SaveLoadStructs.generated.h"

struct FInventorySlotData;

USTRUCT(BlueprintType)
struct FItemSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 0;

	FItemSaveData() {}
	FItemSaveData(UItemBase* Item)
	{
		if (Item)
		{
			ItemID = Item->GetItemID();
			Quantity = Item->GetQuantity();
		}
		else
		{
			ItemID = NAME_None;
			Quantity = 0;
		}
	}

	bool operator==(const FItemSaveData& Other) const
	{
		return ItemID == Other.ItemID;
	}
};
FORCEINLINE uint32 GetTypeHash(const FItemSaveData& Data)
{
	return GetTypeHash(Data.ItemID);
}

USTRUCT(BlueprintType)
struct FItemMappingSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName InventoryContainerName;
	UPROPERTY()
	EInventoryType InventoryType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FIntVector> SlotPositions;

	FItemMappingSaveData(): InventoryType(), SlotPositions()
	{
	}

	explicit FItemMappingSaveData(const FItemMapping& Mapping): InventoryType()
	{
		InventoryContainerName = Mapping.InventoryContainerName;
		InventoryType = Mapping.InventoryType;

		TArray<FIntVector> SlotPositionsTemp;
		for (auto SlotData : Mapping.ItemSlotDatas)
		{
			SlotPositionsTemp.Add(FIntVector(SlotData.SlotPosition.X, SlotData.SlotPosition.Y, 0));
		}
		SlotPositions = SlotPositionsTemp;
	}
};

USTRUCT(BlueprintType)
struct FItemSaveEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemSaveData Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FItemMappingSaveData> Containers;
};

USTRUCT(BlueprintType)
struct FInventorySlotSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FItemSaveEntry> SavedItemLocations;
};


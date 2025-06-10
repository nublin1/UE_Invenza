// Nublin Studio 2025 All Rights Reserved.

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data")
	FName ItemID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data")
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
struct FInventorySlotSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Save Data")
	FName SlotName = " ";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Save Data")
	FIntVector2 SlotPosition{};
};

USTRUCT(BlueprintType)
struct FItemMappingSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mapping Data")
	FName InventoryContainerName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mapping Data")
	EInventoryType InventoryType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mapping Data")
	TArray<FInventorySlotSaveData> SlotSaveDatas;

	FItemMappingSaveData(): InventoryType()
	{
	}
	
	void InitializeFromMapping(const FItemMapping& Mapping)
	{
		InventoryContainerName = Mapping.InventoryContainerName;
		InventoryType          = Mapping.InventoryType;

		SlotSaveDatas.Empty();
		SlotSaveDatas.Reserve(Mapping.ItemSlotDatas.Num());

		for (const FInventorySlotData& SlotData : Mapping.ItemSlotDatas)
		{
			FInventorySlotSaveData InventorySlotSaveData;
			InventorySlotSaveData.SlotName     = SlotData.SlotName;
			InventorySlotSaveData.SlotPosition = SlotData.SlotPosition;
			SlotSaveDatas.Add(MoveTemp(InventorySlotSaveData));
		}
	}
};

USTRUCT(BlueprintType)
struct FItemSaveEntry
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Entry")
	FItemSaveData Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Entry")
	TArray<FItemMappingSaveData> Containers;
};

USTRUCT(BlueprintType)
struct FInvSaveData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data")
	TArray<FItemSaveEntry> SavedItemLocations;
};


// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InventorySlotData.generated.h"

class UItemBase;
class UInputAction;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventorySlotData : public UObject
{
	GENERATED_BODY()

public:
	UInventorySlotData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	FName SlotName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Inventory")
	FIntPoint CellPosition{};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory | Slot")
	TObjectPtr<UItemBase> ItemLinked = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TObjectPtr<UInputAction> UseAction;
	
	/*
	//
	bool operator==(const UInventorySlotData& Other) const
	{
		return CellPosition == Other.CellPosition;
	}*/
};

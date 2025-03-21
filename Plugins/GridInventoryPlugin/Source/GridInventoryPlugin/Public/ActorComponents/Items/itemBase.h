// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataStructures.h"
#include "UObject/Object.h"
#include "itemBase.generated.h"

/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UItemBase : public UObject
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UItemBase();

	UFUNCTION(Category = "Item")
	FORCEINLINE bool IsStackable() const { return ItemRef.ItemNumeraticData.MaxStackSize > 1; }
	UFUNCTION(Category = "Item")
	FORCEINLINE bool IsFullItemStack() const { return Quantity == ItemRef.ItemNumeraticData.MaxStackSize; }
	UFUNCTION(Category = "Item")
	FORCEINLINE float GetItemStackWeight() const { return Quantity * ItemRef.ItemNumeraticData.Weight; }
	UFUNCTION(Category = "Item")
	FORCEINLINE float GetItemSingleWeight() const { return ItemRef.ItemNumeraticData.Weight; }
	UFUNCTION(Category = "Item")
	FORCEINLINE FIntVector2 GetOccupiedSlots() const { return FIntVector2 (ItemRef.ItemNumeraticData.NumHorizontalSlots, ItemRef.ItemNumeraticData.NumVerticalSlots); }

	//
	FName& GetItemName(){return ID;}
	FItemMetaData& GetItemRef()	{return ItemRef;}
	int GetQuantity(){return Quantity;}

	void SetItemName(FName NewName){this->ID = NewName;	}
	void SetItemRef(const FItemMetaData& newItemRef){this->ItemRef = newItemRef;}
	void SetQuantity(int32 newQuantity){this->Quantity = newQuantity;	}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	FName ID;
	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	FItemMetaData ItemRef;
	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	int32 Quantity;

	//====================================================================
	// FUNCTIONS
	//====================================================================
};

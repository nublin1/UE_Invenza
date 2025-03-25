// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataStructures.h"
#include "UObject/Object.h"
#include "itemBase.generated.h"

/**
 * Base class for inventory items
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UItemBase : public UObject
{
	GENERATED_BODY()

public:
	//====================================================================
    // CONSTRUCTOR
    //====================================================================
    UItemBase();

    //====================================================================
    // STATIC METHODS
    //====================================================================
    /** Creates an item from the data table */
    UFUNCTION(BlueprintCallable, Category = "Item|Factory")
    static UItemBase* CreateFromDataTable(UDataTable* ItemDataTable, const FName& DesiredItemID, int32 InitQuantity);
	UFUNCTION(BlueprintCallable, Category = "Item")
	static bool bIsSameitems(UItemBase* FirstItem, UItemBase* SecondItem);

    //====================================================================
    // FUNCTIONS
    //====================================================================
    /** Returns whether the item is stackable */
    UFUNCTION(BlueprintCallable, Category = "Item|Properties")
    FORCEINLINE bool IsStackable() const { return ItemRef.ItemNumeraticData.MaxStackSize > 1; }

    /** Checks if the item stack is full */
    UFUNCTION(BlueprintCallable, Category = "Item|Properties")
    FORCEINLINE bool IsFullItemStack() const { return Quantity == ItemRef.ItemNumeraticData.MaxStackSize; }

    /** Calculates the total weight of the item stack */
    UFUNCTION(BlueprintCallable, Category = "Item|Properties")
    FORCEINLINE float GetItemStackWeight() const { return Quantity * ItemRef.ItemNumeraticData.Weight; }

    /** Returns the weight of a single item */
    UFUNCTION(BlueprintCallable, Category = "Item|Properties")
    FORCEINLINE float GetItemSingleWeight() const { return ItemRef.ItemNumeraticData.Weight; }

    /** Gets the number of slots occupied by the item */
    UFUNCTION(Category = "Item|Properties")
    FORCEINLINE FIntVector2 GetOccupiedSlots() const { return FIntVector2(ItemRef.ItemNumeraticData.NumHorizontalSlots, ItemRef.ItemNumeraticData.NumVerticalSlots); }

    /** Get and set methods */
    FItemMetaData& GetItemRef() { return ItemRef; }
    int32 GetQuantity() { return Quantity; }
    void SetItemRef(const FItemMetaData& NewItemRef) { this->ItemRef = NewItemRef; }
    void SetQuantity(int32 NewQuantity) { this->Quantity = NewQuantity; }

protected:
    //====================================================================
    // PROPERTIES AND VARIABLES
    //====================================================================
    UPROPERTY(VisibleAnywhere, Category = "Item|Data")
    FItemMetaData ItemRef;

    UPROPERTY(VisibleAnywhere, Category = "Item|Data")
    int32 Quantity;
};

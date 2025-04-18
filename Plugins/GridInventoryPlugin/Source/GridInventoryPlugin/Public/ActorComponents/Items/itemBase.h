//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataStructures.h"
#include "UObject/Object.h"
#include "itemBase.generated.h"

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUseItemDelegate, UItemBase*, Item);
#pragma endregion Delegates


USTRUCT(BlueprintType)
struct FItemEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemCount;
};

/**
 * Base class for inventory items
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UItemBase : public UObject
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable)
	FOnUseItemDelegate OnUseItemDelegate;
	
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
	static bool bIsSameItems(UItemBase* FirstItem, UItemBase* SecondItem);

    //====================================================================
    // FUNCTIONS
    //====================================================================
	UFUNCTION()
	virtual void UseItem();
	
	UFUNCTION(BlueprintCallable, Category = "Item|Factory")
	UItemBase* DuplicateItem();
	
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
    int32 GetQuantity() const { return Quantity; }
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

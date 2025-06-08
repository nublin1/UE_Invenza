//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataStructures.h"
#include "UObject/Object.h"
#include "itemBase.generated.h"

#pragma region Delegates
struct FItemData;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUseItemDelegate, UItemBase*, Item);
#pragma endregion Delegates


USTRUCT(BlueprintType)
struct FItemEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 ItemCount = 1;
};

/**
 * Base class for inventory items
 */
UCLASS(Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API UItemBase : public UObject
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable, Category = "Item|Events")
	FOnUseItemDelegate OnUseItemDelegate;
	
	//====================================================================
    // CONSTRUCTOR
    //====================================================================
    UItemBase();

    //====================================================================
    // STATIC METHODS
    //====================================================================
    /** Creates an item from the data table */
	UFUNCTION(BlueprintCallable, Category = "Item")
	static bool bIsSameItems(UItemBase* FirstItem, UItemBase* SecondItem);

    //====================================================================
    // FUNCTIONS
    //====================================================================
	UFUNCTION()
	virtual void InitItem(FName ID, FItemData Data, int32 InQuantity);
	
	UFUNCTION()
	virtual void UseItem();
	
	UFUNCTION(BlueprintCallable, Category = "Item|Factory")
	UItemBase* DuplicateItem();

	UFUNCTION(BlueprintCallable, Category = "Item")
	void DropItem(UWorld* World);
	
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

	UFUNCTION(BlueprintCallable, Category = "Item|Properties")
	FString CategoryToString();

    /** Get and set methods */
	FName GetItemID() const {return ItemID;}
    FItemMetaData& GetItemRef() { return ItemRef; }
    int32 GetQuantity() const { return Quantity; }
    void SetItemRef(const FItemMetaData& NewItemRef) { this->ItemRef = NewItemRef; }
    void SetQuantity(int32 NewQuantity) { this->Quantity = NewQuantity; }

protected:
    //====================================================================
    // PROPERTIES AND VARIABLES
    //====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Data")
	FName ItemID;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Data")
    FItemMetaData ItemRef;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item|Data")
    int32 Quantity;
};

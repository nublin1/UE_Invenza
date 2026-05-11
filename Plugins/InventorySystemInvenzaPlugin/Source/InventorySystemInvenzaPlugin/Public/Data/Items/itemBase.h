//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataStructures.h"
#include "UObject/Object.h"
#include "itemBase.generated.h"

struct FItemData;

/**
 * Base class for inventory items
 */
UCLASS(Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API UItemBase : public UObject
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUseItemDelegate, UItemBase*, Item);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmountChangedDelegate, int32, AmountChanged, UItemBase*, ItemBase);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDataReplicated, UItemBase*, Item);
#pragma endregion Delegates

public:
	UItemBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool IsSupportedForNetworking() const override { return true; }

	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable, Category = "Item|Events")
	FOnUseItemDelegate OnUseItemDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Item|Events")
	FOnAmountChangedDelegate OnAmountChangedDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Item|Events")
	FOnItemDataReplicated OnItemDataReplicated;

	//====================================================================
	// STATIC METHODS
	//====================================================================
	/** Creates an item from the data table */
	UFUNCTION(BlueprintCallable, Category = "Item")
	static bool bIsSameItems(UItemBase* FirstItem, UItemBase* SecondItem);

	UFUNCTION(BlueprintCallable, Category = "Item")
	static bool DoItemsHaveSameFootprint(UItemBase* FirstItem, UItemBase* SecondItem,
		EItemOrientationType OrientationFirstItem, EItemOrientationType OrientationSecondItem,
		bool bIgnoreSize = false);

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	virtual void InitItem(FName ID, FItemData Data, int32 InQuantity);

	UFUNCTION()
	virtual void UseItem();

	UFUNCTION(BlueprintCallable, Category = "Item|Factory")
	UItemBase* DuplicateItem();

	/** Returns whether the item is stackable */
	UFUNCTION(BlueprintCallable, Category = "Item|Properties")
	FORCEINLINE bool IsStackable() const { return ItemRef.ItemNumeraticData.MaxStackSizeInCharacter > 1; }

	/** Checks if the item stack is full */
	UFUNCTION(BlueprintCallable, Category = "Item|Properties")
	FORCEINLINE bool IsFullItemStack() const { return Quantity == ItemRef.ItemNumeraticData.MaxStackSizeInCharacter; }

	/** Calculates the total weight of the item stack */
	UFUNCTION(BlueprintCallable, Category = "Item|Properties")
	FORCEINLINE float GetItemStackWeight() const { return Quantity * ItemRef.ItemNumeraticData.Weight; }

	/** Returns the weight of a single item */
	UFUNCTION(BlueprintCallable, Category = "Item|Properties")
	FORCEINLINE float GetItemSingleWeight() const { return ItemRef.ItemNumeraticData.Weight; }

	UFUNCTION(blueprintCallable, Category = "Item|Properties")
	EItemOrientationType GetInitialItemOrientation();
	

	UFUNCTION(Category = "Item|Properties")
	FIntPoint GetItemSize(EItemOrientationType Orientation);

	UFUNCTION(BlueprintCallable, Category = "Item|Properties")
	FString CategoryToString();

	// Reservation
	UFUNCTION(BlueprintCallable, Category = "Resources|Reservation")
	int32 GetFreeAmount() const;
	UFUNCTION(BlueprintCallable, Category = "Resources|Reservation")
	bool ReserveAmount(AActor* Requestor, int32 AmountToReserve);
	UFUNCTION(BlueprintCallable, Category = "Resources|Reservation")
	void ReleaseReservation(AActor* Requestor, int32 AmountToRelease);
	UFUNCTION(BlueprintCallable, Category = "Resources|Reservation")
	UItemBase* ConsumeReserved(AActor* Requestor, int32 RequestedAmount);


	/** Get and set methods */
	FName GetItemID() const { return ItemID; }
	FDataTableRowHandle GetItemRow() { return ItemRow;}
	FText GetItemDisplayText() const {return ItemRef.ItemTextData.DisplayName;}
	FItemMetaData& GetItemRef() { return ItemRef; }
	int32 GetQuantity() const { return Quantity; }
	void SetItemRow(const FDataTableRowHandle& InRowHandle) {ItemRow = InRowHandle;}
	void SetItemRef(const FItemMetaData& NewItemRef) { this->ItemRef = NewItemRef; }
	void SetQuantity(int32 NewQuantity) { this->Quantity = NewQuantity; }

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Item|Data")
	FName ItemID;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Data")
	FItemMetaData ItemRef;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing=OnRep_ItemRow, Category = "Item|Data")
	FDataTableRowHandle ItemRow;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Item|Data")
	int32 Quantity;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item|Data")
	TMap<TObjectPtr<AActor>, int32> ReservedAmounts;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void OnRep_ItemRow();
};

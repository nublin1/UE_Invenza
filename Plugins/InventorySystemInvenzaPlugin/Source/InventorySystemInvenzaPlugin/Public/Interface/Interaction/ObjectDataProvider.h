// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataStructures.h"
#include "UObject/Interface.h"
#include "ObjectDataProvider.generated.h"

enum class EObjectInteractionType : uint8;
class UInvenzaInventorySettingsAsset;

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUseItemDelegate, UObject*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmountChangedDelegate, int32, AmountChanged, UObject*, ItemBase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDataReplicated, UObject*, Item);
#pragma endregion Delegates

// This class does not need to be modified.
UINTERFACE()
class UObjectDataProvider : public UInterface
{
	GENERATED_BODY()
};

/**
* Provides a common interface for inventory objects/items.
 * The interface contains only the public item API required by
 * inventory, interaction and UI systems.
 */
class INVENTORYSYSTEMINVENZAPLUGIN_API IObjectDataProvider
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
	
public:	
	//====================================================================
	// EVENTS
	//====================================================================
	virtual FOnUseItemDelegate& GetOnUseItemDelegate() = 0;
	virtual FOnAmountChangedDelegate& GetOnAmountChangedDelegate() = 0;
	virtual FOnItemDataReplicated& GetOnItemDataReplicatedDelegate() = 0;
	
	//====================================================================
	// OBJECT OPERATIONS
	//====================================================================

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Operations")
	UObject* DuplicateItem();
	
	//====================================================================
	// IDENTITY & DATA
	//====================================================================

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Identity")
	FName GetItemID() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Identity")
	FItemMetaData GetItemRef();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Identity")
	FDataTableRowHandle GetItemRow();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Identity")
	FText GetItemDisplayText() const;
	
	//====================================================================
	// QUANTITY
	//====================================================================

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Quantity")
	int32 GetQuantity() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Quantity")
	void SetQuantity(int32 NewQuantity);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Quantity")
	bool IsStackable();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Quantity")
	bool IsFullItemStack();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Quantity")
	FVector2D GetMinMaxSplit();
	
	//====================================================================
	// WEIGHT
	//====================================================================

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Weight")
	float GetItemStackWeight();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Weight")
	float GetItemSingleWeight();
	
	//====================================================================
	// FOOTPRINT & ORIENTATION
	//====================================================================

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Footprint")
	EItemOrientationType GetInitialItemOrientation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Footprint")
	FIntPoint GetItemSize(EItemOrientationType Orientation);


	//====================================================================
	// CATEGORY
	//====================================================================

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Category")
	FString CategoryToString();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Category")
	FString CategoryToShortString();


	//====================================================================
	// ACTIONS
	//====================================================================

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Actions")
	void UseItem();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Actions")
	bool CanPerformAction(
		EObjectInteractionType Action,
		const UInvenzaInventorySettingsAsset* SettingsAsset = nullptr
	);
	
	//====================================================================
	// RESERVATION
	//====================================================================

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Reservation")
	int32 GetFreeAmount() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Reservation")
	bool ReserveAmount(
		AActor* Requestor,
		int32 AmountToReserve
	);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Reservation")
	void ReleaseReservation(
		AActor* Requestor,
		int32 AmountToRelease
	);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectData|Reservation")
	UObject* ConsumeReserved(
		AActor* Requestor,
		int32 RequestedAmount
	);
};

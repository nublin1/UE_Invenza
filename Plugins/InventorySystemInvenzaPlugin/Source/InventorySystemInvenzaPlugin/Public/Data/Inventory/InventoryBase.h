//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Inventory/InventoryTypes.h"
#include "Data/Trade/TradeTypes.h"
#include "UObject/Object.h"
#include "InventoryBase.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryBase : public UObject
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAddItemDelegate, FItemMapping&, ItemSlots, UItemBase*, Item);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStackedItemDelegate, UItemBase*, Item, int32, AddedAmount);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnstackedItemDelegate, UItemBase*, Item, int32, RemovedAmount);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemovedDelegate, FItemMapping, ItemSlots, UItemBase*, Item);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemReplaceDelegate, TArray<UInventorySlotData*>, OldItemSlots,
	                                               FItemMapping&, NewItemSlots, UItemBase*, Item);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUseSlot, UInventorySlotData*, UsedSlot);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeightUpdatedDelegate, float, InventoryTotalWeight);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoneyUpdatedDelegate, int32, InventoryTotalMoney);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryRedrawRequested);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRequestToResetItemVisual, UItemBase*, Item);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotsReservedDelegate, TArray<FSlotReservationData>, ReservationData);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConsumeReservedDelegate, TArray<FSlotReservationData>,
	                                            ReservationData);
#pragma endregion Delegates

public:
	UInventoryBase();

	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Delegates
	UPROPERTY(BlueprintAssignable, Category="Inventory|Events")
	FOnAddItemDelegate OnAddItemDelegate;

	UPROPERTY(BlueprintAssignable, Category="Inventory|Events")
	FOnStackedItemDelegate OnStackedItemDelegate;

	UPROPERTY(BlueprintAssignable, Category="Inventory|Events")
	FOnUnstackedItemDelegate OnUnstackedItemDelegate;

	UPROPERTY(BlueprintAssignable, Category="Inventory|Events")
	FOnItemRemovedDelegate OnItemRemovedDelegate;

	UPROPERTY(BlueprintAssignable, Category="Inventory|Events")
	FOnItemReplaceDelegate OnItemReplaceDelegate;

	UPROPERTY(BlueprintAssignable, Category="Inventory|Events")
	FOnUseSlot OnUseSlotDelegate;

	UPROPERTY(BlueprintAssignable, Category="Inventory|Stats")
	FOnWeightUpdatedDelegate OnWeightUpdatedDelegate;

	UPROPERTY(BlueprintAssignable, Category="Inventory|Stats")
	FOnMoneyUpdatedDelegate OnMoneyUpdatedDelegate;

	UPROPERTY(BlueprintAssignable, Category="Inventory|UI")
	FOnInventoryRedrawRequested OnInventoryRedrawRequested;

	UPROPERTY(BlueprintAssignable, Category="Inventory|UI")
	FOnRequestToResetItemVisual OnRequestToResetItemVisual;

	UPROPERTY(BlueprintAssignable, Category="Inventory|Reservation")
	FOnSlotsReservedDelegate OnSlotsReservedDelegate;

	UPROPERTY(BlueprintAssignable, Category="Inventory|Reservation")
	FOnConsumeReservedDelegate OnConsumeReservedDelegate;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static UInventoryBase* CreateInventory(UObject* Outer, FInventoryStartupData StartupData);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UInventoryBase* DuplicateInventory(UObject* Outer);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void InitInventory();
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void InitInventoryWithSettings(FInventorySettings NewInventorySettings);
	
	UFUNCTION(BlueprintCallable)
	virtual void RequestToResetItemVisual(UItemBase* Item);

	UFUNCTION(BlueprintCallable)
	virtual void SortItemsInContainerByName() {};

	virtual bool TrySplitItem(UItemBase* ItemToSplit, int32 SplitAmount)
	PURE_VIRTUAL(UInventoryBase::TrySplitItem, return false;);

	UFUNCTION(BlueprintCallable)
	virtual void HandleRemoveItemsByType(UItemBase* ItemSample, int32 RequestedAmount)
	PURE_VIRTUAL(UInventoryBase::HandleRemoveItemsByType,);
	
	UFUNCTION(BlueprintCallable)
	virtual void HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity)
	PURE_VIRTUAL(UInventoryBase::HandleRemoveItem,);

	UFUNCTION(BlueprintCallable)
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false)
	PURE_VIRTUAL(UInventoryBase::HandleAddItem, return FItemAddResult(););

	UFUNCTION(BlueprintCallable)
	virtual FItemAddResult HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck = false)
	PURE_VIRTUAL(UInventoryBase::HandleAddReferenceItem, return FItemAddResult(););

	UFUNCTION(BlueprintCallable)
	virtual void MergeStackableItems();

	UFUNCTION()
	void UseSlot(UInventorySlotData* UsedSlot);

	UFUNCTION()
	virtual void UpdateWeightInfo();
	UFUNCTION()
	virtual void UpdateMoneyInfo();

	// Getters
	UFUNCTION()
	FString GetInventoryContainerID() { return InventoryContainerID; }

	UFUNCTION()
	virtual FInventorySettings GetInventorySettings() { return InventorySettings; }
	
	UFUNCTION()
	UItemCollection* GetItemCollectionLinked() { return ItemCollectionLinked; }

	UFUNCTION()
	virtual AActor* GetInventoryOwnerActor() {return InventoryOwnerActor;	}

	UFUNCTION()
	virtual FTradeContext GetTradeContext() {return TradeContext;} 

	// Setters
	UFUNCTION()
	virtual void SetInventoryContainerID(FString InID) {InventoryContainerID = InID;}
	
	UFUNCTION(BlueprintCallable)
	virtual void SetInventorySettings(FInventorySettings NewInventorySettings) {InventorySettings = NewInventorySettings;}
	
	UFUNCTION(BlueprintCallable)
	virtual void SetInitialItems(TArray<FInitItemsEntry> NewInitialItems) { InitialItems = NewInitialItems; }

	UFUNCTION(BlueprintCallable)
	virtual void SetInventorySize(FIntPoint NewSize) { InvSize = NewSize; }
	
	UFUNCTION()
	void SetItemCollectionLink(UItemCollection* NewCollection) { ItemCollectionLinked = NewCollection; }

	UFUNCTION()
	virtual void SetInventoryOwnerActor(AActor* InInventoryOwnerActor){this->InventoryOwnerActor = InInventoryOwnerActor;}

	UFUNCTION()
	virtual void SetTradeContext(FTradeContext InTradeContext){this->TradeContext = InTradeContext;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Config")
	FInventorySettings InventorySettings;

	// Initial items with their quantities
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Config")
	TArray<FInitItemsEntry> InitialItems;

	// Data
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString InventoryContainerID; // Uniq ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UItemCollection> ItemCollectionLinked = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasInit = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	float InventoryTotalWeight = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	int32 InventoryTotalMoney = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint InvSize = FIntPoint(1, 1);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TObjectPtr<AActor> InventoryOwnerActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	FTradeContext TradeContext;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	virtual int32 CalculateActualAmountToAdd(int32 InAmountToAdd, float ItemSingleWeight);
	
	UFUNCTION()
	virtual FItemAddResult HandleNonStackableItems(FItemMoveData ItemMoveData, bool bOnlyCheck = false)
	PURE_VIRTUAL(UInventoryBase::HandleNonStackableItems, return FItemAddResult(););

	UFUNCTION()
	virtual FItemAddResult TryAddStackableItem(FItemMoveData& ItemMoveData, bool bOnlyCheck)
	PURE_VIRTUAL(UInventoryBase::TryAddStackableItem, return FItemAddResult(););

	UFUNCTION()
	virtual int32 HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount, bool bOnlyCheck,
	                           TMap<UInventorySlotData*, FItemPlacementData>& AffectedPivotSlots)
	PURE_VIRTUAL(UInventoryBase::HandleStackableItems, return 0; );


	virtual UItemBase* AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount)
	PURE_VIRTUAL(UInventoryBase::AddNewItem, return nullptr;);

	UFUNCTION()
	virtual int32 TryInsertToStackItem(UItemBase* ResourceToInsertInto, int32 AmountToDistribute, bool bOnlyCheck = false);
	
	UFUNCTION()
	virtual int32 TryRemoveFromStackItem(UItemBase* Item, int32 RequestedRemoveAmount);

	UFUNCTION()
	virtual void RemoveItemFromInventory(UItemBase* Item);

	//====================================================================
	// Event Notifiers
	//====================================================================
	void NotifyAddNewItem(FItemMapping& FromSlots, UItemBase* NewItem, int32 ChangeQuantity);
	void NotifyAddItemToStack(UItemBase* Item, int32 ChangeQuantity);
	void NotifyRemoveItemFromStack(UItemBase* Item, int32 ChangeQuantity);
	void NotifyFullyRemoveItem(FItemMapping FromSlots, UItemBase* Item);
	virtual void NotifyReplaceItem(TArray<UInventorySlotData*> OldItemSlots, FItemMapping& NewItemSlots, UItemBase* Item);

	virtual void NotifyUseSlot(UInventorySlotData* UsedSlot);
	virtual void NotifyUpdateWeight();
	virtual void NotifyUpdateMoney();
	virtual void NotifyReDrawRequest();
	virtual void NotifyRequestToResetItemVisual(UItemBase* Item);
};

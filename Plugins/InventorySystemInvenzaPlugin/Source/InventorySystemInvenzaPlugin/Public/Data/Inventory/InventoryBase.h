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

public:
	UInventoryBase();

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSplitDelegate, UInventoryBase*, TargetInventory, UObject*,
												   ItemToSplit, int32, SplitAmount);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAddItemDelegate, FItemMapping&, ItemSlots, UObject*, Item);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStackedItemDelegate, UObject*, Item);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnstackedItemDelegate, UObject*, Item);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemovedDelegate, FItemMapping, ItemSlots, UObject*, Item);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemReplaceDelegate, TArray<UInventorySlotData*>, OldItemSlots,
												   FItemMapping&, NewItemSlots, UObject*, Item);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUseSlot, UInventorySlotData*, UsedSlot);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeightUpdatedDelegate, float, InventoryTotalWeight);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoneyUpdatedDelegate, int32, InventoryTotalMoney);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryRedrawRequested);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTradeContextUpdated);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRequestToResetItemVisual, UObject*, Item);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotsReservedDelegate, TArray<FSlotReservationData>,
												ReservationData);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConsumeReservedDelegate, TArray<FSlotReservationData>,
												ReservationData);
#pragma endregion Delegates

	// Properties and variables
	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnSplitDelegate OnSplitDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnInventoryRedrawRequested OnConfigurationChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnAddItemDelegate OnAddItemDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnStackedItemDelegate OnStackedItemDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnUnstackedItemDelegate OnUnstackedItemDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemRemovedDelegate OnItemRemovedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemReplaceDelegate OnItemReplaceDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnUseSlot OnUseSlotDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Stats")
	FOnWeightUpdatedDelegate OnWeightUpdatedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Stats")
	FOnMoneyUpdatedDelegate OnMoneyUpdatedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|UI")
	FOnInventoryRedrawRequested OnInventoryRedrawRequested;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|UI")
	FOnTradeContextUpdated OnTradeContextUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|UI")
	FOnRequestToResetItemVisual OnRequestToResetItemVisual;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Reservation")
	FOnSlotsReservedDelegate OnSlotsReservedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Reservation")
	FOnConsumeReservedDelegate OnConsumeReservedDelegate;

	// Functions
	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
									 struct FReplicationFlags* RepFlags)
	{
		return false;
	}

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static UInventoryBase* CreateInventory(UObject* Outer, FInventoryStartupData StartupData);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static UInventoryBase* CreateInventoryAdvanced(UObject* Outer, FInventoryStartupData StartupData,
												   AActor* OwnerActor, UInventoryCollectionComponent* InItemCollection);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual UInventoryBase* DuplicateInventory(UObject* Outer);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void InitInventory();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void InitInventoryWithSettings(FInventorySettings NewInventorySettings);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void RebuildInventory() {};

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual void RequestToResetItemVisual(UObject* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Sorting")
	void HandleSortItems(EInventorySortCriteria Criteria);

	virtual void RequestSplitStack(UObject* ItemToSplit, int32 SplitAmount)
		PURE_VIRTUAL(UInventoryBase::TrySplitItem, );

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual void HandleRemoveItemsByID(FName ItemID, int32 RequestedAmount)
		PURE_VIRTUAL(UInventoryBase::HandleRemoveItemsByID, );

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual void HandleRemoveItemsBySample(UObject* ItemSample, int32 RequestedAmount)
		PURE_VIRTUAL(UInventoryBase::HandleRemoveItemsByType, );

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual void HandleRemoveItem(UObject* Item, int32 RemoveQuantity) PURE_VIRTUAL(UInventoryBase::HandleRemoveItem, );

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false)
		PURE_VIRTUAL(UInventoryBase::HandleAddItem, return FItemAddResult(););

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual FItemAddResult HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck = false)
		PURE_VIRTUAL(UInventoryBase::HandleAddReferenceItem, return FItemAddResult(););

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual void MergeStackableItems();

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual void UseSlot(UInventorySlotData* UsedSlot);

	//
	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual bool bIsSlotEmptyByPos(FIntPoint SlotPosition, const TArray<UInventorySlotData*>& SlotsToIgnore)
	{
		return false;
	}

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual bool bIsSlotEmptyByID(FGuid SlotIDToCheck, const TArray<FGuid>& SlotsIDToIgnore) { return false; }

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual bool bIsSlotEmpty(UInventorySlotData* SlotToCheck, const TArray<UInventorySlotData*>& SlotsToIgnore)
	{
		return false;
	}

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual bool AreSlotsEmpty(const TArray<UInventorySlotData*>& SlotsToCheck,
							   const TArray<UInventorySlotData*>& SlotsToIgnore)
	{
		return false;
	}

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
	virtual FIntPoint GetInventorySize() { return InvSize; }

	UFUNCTION()
	UInventoryCollectionComponent* GetItemCollectionLinked() { return ItemCollectionLinked; }

	UFUNCTION()
	virtual AActor* GetInventoryOwnerActor() { return InventoryOwnerActor; }

	UFUNCTION()
	virtual FTradeContext GetTradeContext() { return TradeContext; }

	UFUNCTION()
	virtual float GetInventoryOccupancyPercent() { return 0; }

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual TArray<UInventorySlotData*> GetAvailableSlotForItem(UObject* Item, EItemOrientationType& OutOrientation);

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual UInventorySlotData* GetSlotByPosition(FIntPoint CellPosition);

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual UInventorySlotData* GetSlotByGuid(FGuid InGuid);

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual TArray<FGuid> GetSlotsWithLinkedEquipment();

	// Setters
	UFUNCTION()
	virtual void SetInventoryContainerID(FString InID) { InventoryContainerID = InID; }

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual void SetInventorySettings(FInventorySettings NewInventorySettings)
	{
		InventorySettings = NewInventorySettings;
	}

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual void SetInventorySize(FIntPoint NewSize)
	{
		if (InvSize != NewSize)
		{
			InvSize = NewSize;
			OnRep_InventoryConfiguration();
		}
	}

	UFUNCTION()
	void SetItemCollectionLink(UInventoryCollectionComponent* NewCollection)
	{
		if (ItemCollectionLinked != NewCollection)
		{
			ItemCollectionLinked = NewCollection;
			OnRep_InventoryConfiguration();
		}
	}

	UFUNCTION()
	virtual void SetInventoryOwnerActor(AActor* InInventoryOwnerActor)
	{
		this->InventoryOwnerActor = InInventoryOwnerActor;
	}

	UFUNCTION()
	virtual void SetTradeContext(FTradeContext InTradeContext);

protected:
	// Runtime
	// Properties and variables
	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_InventoryConfiguration,
			  Category = "Inventory|Config")
	FInventorySettings InventorySettings;

	// Data
	UPROPERTY(Category = "Invenza|Inventory", EditAnywhere, BlueprintReadOnly,
			  ReplicatedUsing = OnRep_InventoryConfiguration)
	FString InventoryContainerID;

	// Uniq ID
	UPROPERTY(Category = "Invenza|Inventory", EditAnywhere, BlueprintReadWrite,
			  ReplicatedUsing = OnRep_InventoryConfiguration)
	TObjectPtr<UInventoryCollectionComponent> ItemCollectionLinked = nullptr;

	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite)
	bool bWasInit = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_InventoryTotalWeight, Category = "Inventory")
	float InventoryTotalWeight = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_InventoryTotalMoney, Category = "Inventory")
	int32 InventoryTotalMoney = 0;

	UPROPERTY(Category = "Invenza|Inventory", EditAnywhere, BlueprintReadWrite,
			  ReplicatedUsing = OnRep_InventoryConfiguration)
	FIntPoint InvSize = FIntPoint(1, 1);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Inventory")
	TObjectPtr<AActor> InventoryOwnerActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_TradeContext,
			  Category = "Inventory")
	FTradeContext TradeContext;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Inventory|Sorting")
	virtual void SortItemsInContainerByName() {};

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
		PURE_VIRTUAL(UInventoryBase::HandleStackableItems, return 0;);

	virtual UObject* AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount)
		PURE_VIRTUAL(UInventoryBase::AddNewItem, return nullptr;);

	UFUNCTION()
	virtual int32 TryInsertToStackItem(UObject* ResourceToInsertInto, int32 AmountToDistribute,
									   bool bOnlyCheck = false);

	UFUNCTION()
	virtual int32 TryRemoveFromStackItem(UObject* Item, int32 RequestedRemoveAmount);

	UFUNCTION()
	virtual void RemoveItemFromInventory(UObject* Item);

	// Event Notifiers

	void NotifyAddNewItem(FItemMapping& FromSlots, UObject* NewItem, int32 ChangeQuantity);

	void NotifyAddItemToStack(UObject* Item);

	void NotifyRemoveItemFromStack(UObject* Item);

	void NotifyFullyRemoveItem(FItemMapping FromSlots, UObject* Item);

	virtual void NotifyReplaceItem(TArray<UInventorySlotData*> OldItemSlots, FItemMapping& NewItemSlots, UObject* Item);

	virtual void NotifyUseSlot(UInventorySlotData* UsedSlot);

	UFUNCTION()
	virtual void OnRep_InventoryTotalWeight();

	UFUNCTION()
	void OnRep_InventoryConfiguration();

	UFUNCTION()
	virtual void OnRep_InventoryTotalMoney();

	UFUNCTION()
	virtual void OnRep_TradeContext();

	virtual void NotifyReDrawRequest();

	virtual void NotifyRequestToResetItemVisual(UObject* Item);
};

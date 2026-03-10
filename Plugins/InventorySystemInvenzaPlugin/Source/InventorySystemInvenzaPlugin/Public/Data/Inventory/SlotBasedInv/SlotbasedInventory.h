//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "UI/Inventory/InventoryTypes.h"
#include "SlotbasedInventory.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API USlotbasedInventory : public UInventoryBase
{
	GENERATED_BODY()

public:
	USlotbasedInventory();

protected:

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void InitInventory(UItemCollection* ItemCollectionRef, FVector2D NewSize ) override;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Reservation")
	bool ReserveSlots(AActor* Requestor, TMap<UInventorySlotData*, FItemPlacementData> Slots, UItemBase* ItemBase = nullptr);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Reservation")
	void ReleaseReservation(AActor* Requestor);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Reservation")
	bool ConsumeReserved(AActor* Requestor);

	//
	UFUNCTION(BlueprintCallable)
	void SetupStartingResources();

	UFUNCTION(BlueprintCallable)
	virtual void HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity) override;
	UFUNCTION(BlueprintCallable)
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;

	// Getters
	UFUNCTION(BlueprintCallable)
	TArray<UItemBase*> GetAllItems();
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Returns a list of resources stored in this container, aggregating identical resources and summing their total amount."))
	TArray<FItemIDEntry> CollectItemsAggregated() const;
	UFUNCTION(BlueprintCallable)
	TArray<UInventorySlotData*> GetSlotsForItemAt(FIntPoint& StartPos, UItemBase* ItemBase, EItemOrientationType Orientation);
	UFUNCTION(BlueprintCallable)
	bool CanPlaceItemAt(FIntPoint& StartPos, UItemBase* ItemBase, EItemOrientationType Orientation);
	UFUNCTION()
	TArray<FIntPoint> GetItemGridPositions(FIntPoint& StartPos, int32 Width, int32 Height, EItemOrientationType Orientation = EItemOrientationType::Horizontal);
	UFUNCTION(BlueprintCallable)
	TArray<UInventorySlotData*> GetAvailableSlotForItem(UItemBase* Item);

	// Setters
	UFUNCTION()
	void SetInvSlots (const TArray<UInventorySlotData*>& InSlots) {this->InvSlotsDatas = InSlots;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TMap<TObjectPtr<AActor>, TArray<FSlotReservationData>> ReservedSlotsToAdd;

	//
	virtual FItemAddResult HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck = false) override;

	virtual void MergeStackableItems() override;
	
	UFUNCTION()
	FItemAddResult HandleNonStackableItems(FItemMoveData ItemMoveData, bool bOnlyCheck = false);
	UFUNCTION()
	FItemAddResult TryAddStackableItem(FItemMoveData& ItemMoveData, bool bOnlyCheck);
	UFUNCTION()
	int32 HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount, bool bOnlyCheck,
		TMap<UInventorySlotData*, FItemPlacementData>& AffectedPivotSlots);
	
	UFUNCTION()
	int32 DistributeToExistingStacks(TArray<UItemBase*>& SameItems, int32& AmountToDistribute,
		UItemBase* ResourceToDeductFrom,
		bool bOnlyCheck,TMap<UInventorySlotData*, FItemPlacementData>& AffectedSlots);
	UFUNCTION()
	void DeductResourceOnAddToInventory(UItemBase* Resource, int32 DeductAmount);

	UFUNCTION()
	UItemBase* AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount);
	UFUNCTION()
	void ReplaceItem(UItemBase* Item, UInventorySlotData* NewSlot);
	UFUNCTION()
	int32 TryInsertToStackItem(UItemBase* ResourceToInsertInto, UItemBase* ResourceToDeductFrom, int32 AmountToDistribute, bool bOnlyCheck);
	UFUNCTION()
	void RemoveItemFromInventory(UItemBase* Item);
	UFUNCTION()
	int32 TryRemoveFromStackItem(UItemBase* Item, int32 RequestedRemoveAmount);
	UFUNCTION()
	int32 CalculateActualAmountToAdd(int32 InAmountToAdd, float ItemSingleWeight);

public:
	UFUNCTION()
	bool bIsGridPositionValid(FIntPoint GridPosition);
	UFUNCTION()
	bool bIsSlotEmptyByPos(FIntPoint SlotPosition);
	UFUNCTION()
	bool bIsSlotEmpty(UInventorySlotData* Slot);
	UFUNCTION()
	float GetInventoryOccupancyPercent();
	UFUNCTION()
	TArray<UInventorySlotData*> CollectOccupiedSlots();

protected:
	UFUNCTION()
	UInventorySlotData* GetSlotByPosition(FIntPoint SlotPosition);
	UFUNCTION()
	TArray<UItemBase*> GetAllSameItems(UItemBase* ReferenceItem);
	UFUNCTION()
	UItemBase* GetItemFromSlot(UInventorySlotData* Slot);

	//====================================================================
	// Event Notifiers
	//====================================================================
	UFUNCTION()
	void NotifyAddNewItem(FItemMapping& FromSlots, UItemBase* NewItem, int32 ChangeQuantity);
	UFUNCTION()
	void NotifyAddItemToStack(UItemBase* Item, int32 ChangeQuantity);
	UFUNCTION()
	void NotifyRemoveItemFromStack(UItemBase* Item, int32 ChangeQuantity);
	UFUNCTION()
	void NotifyFullyRemoveItem(FItemMapping& FromSlots, UItemBase* Item);
	UFUNCTION()
	virtual void NotifyReplaceItem(TArray<UInventorySlotData*> OldItemSlots, FItemMapping NewItemSlots, UItemBase* Item);
};

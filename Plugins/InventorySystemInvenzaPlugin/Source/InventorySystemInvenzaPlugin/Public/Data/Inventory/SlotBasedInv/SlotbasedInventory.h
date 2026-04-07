//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Items/ItemBase.h"
#include "Data/Inventory/InventoryTypes.h"
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

	virtual void InitInventory() override;
	
	//
	virtual void SortItemsInContainerByName() override;
	
	//
	UFUNCTION(BlueprintCallable, Category = "Inventory|Reservation")
	bool ReserveSlots(AActor* Requestor, TMap<UInventorySlotData*, FItemPlacementData> Slots, UItemBase* ItemBase = nullptr);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Reservation")
	void ReleaseReservation(AActor* Requestor);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Reservation")
	bool ConsumeReserved(AActor* Requestor);

	//
	UFUNCTION(BlueprintCallable)
	bool CanPlaceItemAt(const FIntPoint& StartPos, UItemBase* ItemBase, EItemOrientationType Orientation, TArray<UInventorySlotData*> IgnoreSlots);

	
	virtual void HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity) override;
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;

	// Getters
	UFUNCTION()
	TArray<UInventorySlotData*> GetInventorySlots() const {return InventorySlotData;}
	UFUNCTION(BlueprintCallable)
	TArray<UItemBase*> GetAllItems();
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Returns a list of resources stored in this container, aggregating identical resources and summing their total amount."))
	TArray<FItemIDEntry> CollectItemsAggregated() const;
	UFUNCTION(BlueprintCallable)
	TArray<UInventorySlotData*> GetSlotsForItemAt(const FIntPoint& StartPos, UItemBase* ItemBase, EItemOrientationType Orientation);
	UFUNCTION()
	TArray<FIntPoint> GetItemGridPositions(const FIntPoint& StartPos, FIntPoint Size);
	UFUNCTION(BlueprintCallable)
	TArray<UInventorySlotData*> GetAvailableSlotForItem(UItemBase* Item, EItemOrientationType& OutOrientation);

	// Setters
	UFUNCTION()
	void SetInventorySlots (const TArray<UInventorySlotData*>& InSlots) {this->InventorySlotData = InSlots;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UInventorySlotData>> InventorySlotData;
	
	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TMap<TObjectPtr<AActor>, TArray<FSlotReservationData>> ReservedSlotsToAdd;

	//
	virtual FItemAddResult HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck = false) override;
	virtual FItemAddResult HandleNonStackableItems(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;
	virtual FItemAddResult TryAddStackableItem(FItemMoveData& ItemMoveData, bool bOnlyCheck) override;
	virtual int32 HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount, bool bOnlyCheck,
	                                   TMap<UInventorySlotData*, FItemPlacementData>& AffectedPivotSlots) override;

	UFUNCTION()
	virtual FItemAddResult TryReplaceItems(FItemMoveData& ItemMoveData, bool bOnlyCheck);
	
	UFUNCTION()
	int32 DistributeToExistingStacks(TArray<UItemBase*>& SameItems, int32& AmountToDistribute,
		UItemBase* ResourceToDeductFrom,
		bool bOnlyCheck,TMap<UInventorySlotData*, FItemPlacementData>& AffectedSlots);
	
	virtual UItemBase* AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount) override;
	void ReplaceItem(UItemBase* Item, const TArray<UInventorySlotData*>& NewSlotDatas, EItemOrientationType NewItemOrientation);
	virtual int32 TryInsertToStackItem(UItemBase* ResourceToInsertInto, int32 AmountToDistribute, bool bOnlyCheck) override;
	

public:
	UFUNCTION()
	bool bIsSlotPositionValid(FIntPoint GridPosition);
	UFUNCTION()
	bool bIsItemCategoryCompatible(EItemCategory ItemCategory, EItemCategory SlotCategory);
	UFUNCTION()
	bool bIsSlotEmptyByPos(FIntPoint SlotPosition, const TArray<UInventorySlotData*>& SlotsToIgnore);
	UFUNCTION()
	bool bIsSlotEmpty(UInventorySlotData* SlotToCheck, const TArray<UInventorySlotData*>& SlotsToIgnore);
	UFUNCTION()
	bool AreSlotsEmpty(const TArray<UInventorySlotData*>& SlotsToCheck, const TArray<UInventorySlotData*>& SlotsToIgnore);
	UFUNCTION()
	static bool DoSlotsMatch(const TArray<UInventorySlotData*>& FirstSlots, const TArray<UInventorySlotData*>& SecondSlots);
	UFUNCTION()
	float GetInventoryOccupancyPercent();
	UFUNCTION()
	TArray<UInventorySlotData*> CollectOccupiedSlots();
	UFUNCTION()
	TArray<UInventorySlotData*> GetIgnoreSlotsForItem(UItemBase* Item);

protected:
	UFUNCTION()
	UInventorySlotData* GetSlotByPosition(FIntPoint SlotPosition);
	UFUNCTION()
	TArray<UItemBase*> GetAllSameItems(UItemBase* ReferenceItem);
	UFUNCTION()
	UItemBase* GetItemFromSlot(UInventorySlotData* Slot);

	UFUNCTION()
	void GenerateInventorySlots();
	
};

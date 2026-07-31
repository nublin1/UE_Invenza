//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Items/ItemBase.h"
#include "Data/Inventory/InventoryTypes.h"
#include "SlotbasedInventory.generated.h"


UCLASS(ClassGroup=(Custom))
class INVENTORYSYSTEMINVENZAPLUGIN_API USlotbasedInventory : public UInventoryBase
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventorySlotDataUpdated);
#pragma endregion Delegates

public:
	USlotbasedInventory(){};

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, struct FReplicationFlags* RepFlags) override;

protected:

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable, Category="Inventory|Events")
	FOnInventorySlotDataUpdated OnInventorySlotDataUpdated;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	virtual void InitInventory() override;

	virtual void RebuildInventory() override;
	
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
	virtual float GetInventoryOccupancyPercent() override;
	
	UFUNCTION(BlueprintCallable)
	bool CanPlaceItemAt(const FIntPoint& StartPos, UItemBase* ItemBase, EItemOrientationType Orientation, TArray<UInventorySlotData*> IgnoreSlots);

	UFUNCTION(BlueprintCallable)
	virtual void RequestSplitStack(UItemBase* ItemToSplit, int32 SplitAmount) override;

	virtual void HandleRemoveItemsByID(FName ItemID, int32 RequestedAmount) override;
	virtual void HandleRemoveItemsBySample(UItemBase* ItemSample, int32 RequestedAmount) override;
	virtual void HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity) override;
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;

	// Getters
	FMargin GetSlotSpacing() const {return SlotSpacing;}
	FVector2D GetInvCellSize() const {return InvCellSize;}

	TArray<UInventorySlotData*> GetInventorySlots() const {return InventorySlotData;}
	
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Returns a list of resources stored in this container, aggregating identical resources and summing their total amount."))
	TArray<FItemIDEntry> CollectItemsAggregated() const;
	UFUNCTION(BlueprintCallable)
	TArray<UInventorySlotData*> GetSlotsForItemAt(const FIntPoint& StartPos, UItemBase* ItemBase, EItemOrientationType Orientation);
	UFUNCTION()
	TArray<FIntPoint> GetItemGridPositions(const FIntPoint& StartPos, FIntPoint Size);
	
	virtual TArray<UInventorySlotData*> GetAvailableSlotForItem(UItemBase* Item, EItemOrientationType& OutOrientation) override;

	// Setters
	UFUNCTION(BlueprintCallable)
	void SetWidgetInitData(FSlotBasedInventoryWidgetInitData WidgetInitData);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_InventorySlotData)
	TArray<TObjectPtr<UInventorySlotData>> InventorySlotData;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated)
	TArray<FInventorySlotInfo> WidgetSlotInitData;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated)
	FMargin SlotSpacing;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated)
	FVector2D InvCellSize = FVector2D(64.0f, 64.0f);
	
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
		bool bOnlyCheck,TMap<UInventorySlotData*, FItemPlacementData>& AffectedSlots);
	
	virtual UItemBase* AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount) override;
	void ReplaceItem(UItemBase* Item, const TArray<UInventorySlotData*>& NewSlotDatas, EItemOrientationType NewItemOrientation);
	
	virtual int32 TryInsertToStackItem(UItemBase* ResourceToInsertInto, int32 AmountToDistribute, bool bOnlyCheck) override;

public:
	UFUNCTION()
	bool bIsSlotPositionValid(FIntPoint GridPosition);
	UFUNCTION()
	bool BIsItemCategoryCompatible(FGameplayTag ItemCategory, FGameplayTag SlotCategory, bool bExactMatch = false);
	UFUNCTION()
	bool bIsSlotEmptyByPos(FIntPoint SlotPosition, const TArray<UInventorySlotData*>& SlotsToIgnore);
	UFUNCTION()
	bool bIsSlotEmpty(UInventorySlotData* SlotToCheck, const TArray<UInventorySlotData*>& SlotsToIgnore);
	UFUNCTION()
	bool AreSlotsEmpty(const TArray<UInventorySlotData*>& SlotsToCheck, const TArray<UInventorySlotData*>& SlotsToIgnore);
	UFUNCTION()
	static bool DoSlotsMatch(const TArray<UInventorySlotData*>& FirstSlots, const TArray<UInventorySlotData*>& SecondSlots);
	UFUNCTION()
	TArray<UInventorySlotData*> CollectOccupiedSlots();
	UFUNCTION()
	TArray<UInventorySlotData*> GetIgnoreSlotsForItem(UItemBase* Item);

	virtual UInventorySlotData* GetSlotByPosition(FIntPoint SlotPosition) override;
	
	virtual UInventorySlotData* GetSlotByGuid(FGuid InGuid) override;
	
protected:
	UFUNCTION()
	TArray<UItemBase*> GetAllSameItems(UItemBase* ReferenceItem);
	UFUNCTION()
	UItemBase* GetItemFromSlot(UInventorySlotData* Slot);

	UFUNCTION()
	void OnRep_InventorySlotData();

	UFUNCTION()
	void GenerateInventorySlots();

};

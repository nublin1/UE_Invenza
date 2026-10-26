//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Margin.h"
#include "Components/ActorComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Items/itemBase.h"
#include "Data/Inventory/InventoryTypes.h"
#include "SlotbasedInventory.generated.h"

UCLASS(ClassGroup = (Custom))
class INVENTORYSYSTEMINVENZAPLUGIN_API USlotbasedInventory : public UInventoryBase
{
	GENERATED_BODY()

public:
	USlotbasedInventory() {};

protected:
	// Unreal overrides
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventorySlotDataUpdated);
#pragma endregion Delegates

	// Properties and variables
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnInventorySlotDataUpdated OnInventorySlotDataUpdated;

	// Functions
	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
									 struct FReplicationFlags* RepFlags) override;

	virtual void InitInventory() override;

	virtual void RebuildInventory() override;

	virtual UInventoryBase* DuplicateInventory(UObject* Outer) override;

	//
	virtual void SortItemsInContainerByName() override;

	//
	UFUNCTION(BlueprintCallable, Category = "Inventory|Reservation")
	bool ReserveSlots(AActor* Requestor, TMap<UInventorySlotData*, FItemPlacementData> Slots,
					  UObject* ItemBase = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Reservation")
	void ReleaseReservation(AActor* Requestor);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Reservation")
	bool ConsumeReserved(AActor* Requestor);

	//
	virtual float GetInventoryOccupancyPercent() override;

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	bool CanPlaceItemAt(const FIntPoint& StartPos, FGameplayTag ItemCategory, FIntPoint ItemSize,
						EItemOrientationType Orientation, TArray<UInventorySlotData*> IgnoreSlots);

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual void RequestSplitStack(UObject* ItemToSplit, int32 SplitAmount) override;

	virtual void HandleRemoveItemsByID(FName ItemID, int32 RequestedAmount) override;

	virtual void HandleRemoveItemsBySample(UObject* ItemSample, int32 RequestedAmount) override;

	virtual void HandleRemoveItem(UObject* Item, int32 RemoveQuantity) override;

	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;

	// Getters
	FMargin GetSlotSpacing() const { return SlotSpacing; }

	FVector2D GetInvCellSize() const { return InvCellSize; }

	TArray<UInventorySlotData*> GetInventorySlots() const { return InventorySlotData; }

	void SetInventorySlots(TArray<UInventorySlotData*> NewSlotData) { InventorySlotData = NewSlotData; }

	UFUNCTION(
		Category = "Invenza|Inventory", BlueprintCallable,
		meta =
			(ToolTip =
				 "Returns a list of resources stored in this container, aggregating identical resources and summing their total amount."))
	TArray<FItemIDEntry> CollectItemsAggregated() const;

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	TArray<UInventorySlotData*> GetSlotsForItemAt(const FIntPoint& StartPos, UObject* ItemBase,
												  EItemOrientationType Orientation);

	UFUNCTION()
	TArray<FIntPoint> GetItemGridPositions(const FIntPoint& StartPos, FIntPoint Size);

	virtual TArray<UInventorySlotData*> GetAvailableSlotForItem(UObject* Item,
																EItemOrientationType& OutOrientation) override;

	// Setters
	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	void SetWidgetInitData(FSlotBasedInventoryWidgetInitData WidgetInitData);

	UFUNCTION()
	bool bIsSlotPositionValid(FIntPoint GridPosition);

	UFUNCTION()
	bool BIsItemCategoryCompatible(FGameplayTag ItemCategory, FGameplayTag SlotCategory, bool bExactMatch = false);

	virtual bool bIsSlotEmptyByPos(FIntPoint SlotPosition, const TArray<UInventorySlotData*>& SlotsToIgnore) override;

	virtual bool bIsSlotEmptyByID(FGuid SlotIDToCheck, const TArray<FGuid>& SlotsIDToIgnore) override;

	virtual bool bIsSlotEmpty(UInventorySlotData* SlotToCheck,
							  const TArray<UInventorySlotData*>& SlotsToIgnore) override;

	virtual bool AreSlotsEmpty(const TArray<UInventorySlotData*>& SlotsToCheck,
							   const TArray<UInventorySlotData*>& SlotsToIgnore) override;

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	static bool DoSlotsMatch(const TArray<UInventorySlotData*>& FirstSlots,
							 const TArray<UInventorySlotData*>& SecondSlots);

	UFUNCTION()
	TArray<UInventorySlotData*> CollectOccupiedSlots();

	UFUNCTION()
	TArray<UInventorySlotData*> GetIgnoreSlotsForItem(UObject* Item);

	virtual UInventorySlotData* GetSlotByPosition(FIntPoint SlotPosition) override;

	virtual UInventorySlotData* GetSlotByGuid(FGuid InGuid) override;

	virtual TArray<FGuid> GetSlotsWithLinkedEquipment() override;

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite, Replicated,
			  ReplicatedUsing = OnRep_InventorySlotData)
	TArray<TObjectPtr<UInventorySlotData>> InventorySlotData;

	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite, Replicated)
	TArray<FInventorySlotInfo> WidgetSlotInitData;

	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite, Replicated)
	FMargin SlotSpacing;

	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite, Replicated)
	FVector2D InvCellSize = FVector2D(64.0f, 64.0f);

	//UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadWrite)
	TMap<TObjectPtr<AActor>, TArray<FSlotReservationData>> ReservedSlotsToAdd;

	// Functions
	//
	virtual FItemAddResult HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck = false) override;

	virtual FItemAddResult HandleNonStackableItems(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;

	virtual FItemAddResult TryAddStackableItem(FItemMoveData& ItemMoveData, bool bOnlyCheck) override;

	virtual int32 HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount, bool bOnlyCheck,
									   TMap<UInventorySlotData*, FItemPlacementData>& AffectedPivotSlots) override;

	UFUNCTION()
	virtual FItemAddResult TryReplaceItems(FItemMoveData& ItemMoveData, bool bOnlyCheck);

	UFUNCTION()
	int32 DistributeToExistingStacks(TArray<UObject*>& SameItems, int32& AmountToDistribute, bool bOnlyCheck,
									 TMap<UInventorySlotData*, FItemPlacementData>& AffectedSlots);

	virtual UObject* AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount) override;

	void ReplaceItem(UObject* Item, const TArray<UInventorySlotData*>& NewSlotDatas,
					 EItemOrientationType NewItemOrientation);

	virtual int32 TryInsertToStackItem(UObject* ResourceToInsertInto, int32 AmountToDistribute,
									   bool bOnlyCheck) override;

	UFUNCTION()
	TArray<UObject*> GetAllSameItems(UObject* ReferenceItem);

	UFUNCTION()
	UObject* GetItemFromSlot(UInventorySlotData* Slot);

	UFUNCTION()
	void OnRep_InventorySlotData();

	UFUNCTION()
	void GenerateInventorySlots();
};

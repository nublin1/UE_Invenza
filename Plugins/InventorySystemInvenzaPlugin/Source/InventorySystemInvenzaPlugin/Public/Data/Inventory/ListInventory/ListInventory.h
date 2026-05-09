//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryListEntry.h"
#include "Data/Inventory/InventoryBase.h"
#include "ListInventory.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UListInventory : public UInventoryBase
{
	GENERATED_BODY()

public:
	UListInventory();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, struct FReplicationFlags* RepFlags) override;

	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_InvSlotsArray, Category = "Inventory|Data")
	TArray<TObjectPtr<UInventoryListEntry>> InvSlotsArray;
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Data")
	TArray<TObjectPtr<UInventoryListEntry>> FilteredInvSlotsArray;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void InitInventory() override;
	
	virtual void SortItemsInContainerByName() override;

	virtual float GetInventoryOccupancyPercent() override;

	UFUNCTION(BlueprintCallable)
	virtual void RequestSplitStack(UItemBase* ItemToSplit, int32 SplitAmount) override;
	virtual void HandleRemoveItemsByType(UItemBase* ItemSample, int32 RequestedAmount) override;
	virtual void HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity) override;
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;

	virtual TSubclassOf<UInventoryListEntry> GetEntryClass() {return EntryClass;}

	virtual void SetEntryClass(TSubclassOf<UInventoryListEntry> NewClass) {EntryClass = NewClass;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "Inventory")
	TSubclassOf<UInventoryListEntry> EntryClass = UInventoryListEntry::StaticClass();

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void OnRep_InvSlotsArray();

	virtual FItemAddResult HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck = false) override;
	
	virtual FItemAddResult HandleNonStackableItems(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;
	virtual FItemAddResult TryAddStackableItem(FItemMoveData& ItemMoveData, bool bOnlyCheck) override;
	virtual int32 HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount, bool bOnlyCheck,
									   TMap<UInventorySlotData*, FItemPlacementData>& AffectedPivotSlots) override;
	
	virtual UItemBase* AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount) override;

	virtual void UpdateInvSlotsArray();
	
};

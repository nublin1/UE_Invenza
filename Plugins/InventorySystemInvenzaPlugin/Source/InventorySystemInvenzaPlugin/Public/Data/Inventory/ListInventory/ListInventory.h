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

protected:
	// Unreal overrides
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Properties and variables
	UPROPERTY(BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_InvSlotsArray, Category = "Inventory|Data")
	TArray<TObjectPtr<UInventoryListEntry>> InvSlotsArray;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Data")
	TArray<TObjectPtr<UInventoryListEntry>> FilteredInvSlotsArray;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Inventory|Data")
	TArray<TObjectPtr<UInventorySlotData>> InventorySlots;

	// Functions
	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
									 struct FReplicationFlags* RepFlags) override;

	virtual void InitInventory() override;

	virtual void SortItemsInContainerByName() override;

	virtual float GetInventoryOccupancyPercent() override;

	virtual UInventorySlotData* GetSlotByGuid(FGuid InGuid) override;

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	virtual void RequestSplitStack(UObject* ItemToSplit, int32 SplitAmount) override;

	virtual void HandleRemoveItemsByID(FName ItemID, int32 RequestedAmount) override;

	virtual void HandleRemoveItemsBySample(UObject* ItemSample, int32 RequestedAmount) override;

	virtual void HandleRemoveItem(UObject* Item, int32 RemoveQuantity) override;

	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;

	virtual TSubclassOf<UInventoryListEntry> GetEntryClass() { return EntryClass; }

	virtual void SetEntryClass(TSubclassOf<UInventoryListEntry> NewClass) { EntryClass = NewClass; }

	/** Rebuild replicated entries after an atomic equipment transfer changes the mappings. */
	void RefreshItemEntries() { UpdateInvSlotsArray(); }

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "Inventory")
	TSubclassOf<UInventoryListEntry> EntryClass = UInventoryListEntry::StaticClass();

	// Functions
	UFUNCTION()
	void OnRep_InvSlotsArray();

	virtual FItemAddResult HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck = false) override;

	virtual FItemAddResult HandleNonStackableItems(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;

	virtual FItemAddResult TryAddStackableItem(FItemMoveData& ItemMoveData, bool bOnlyCheck) override;

	virtual int32 HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount, bool bOnlyCheck,
									   TMap<UInventorySlotData*, FItemPlacementData>& AffectedPivotSlots) override;

	virtual UObject* AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount) override;

	virtual void UpdateInvSlotsArray();
};

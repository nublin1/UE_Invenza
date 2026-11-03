//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/ItemDataStructures.h"
#include "Interface/Interaction/ObjectDataProvider.h"
#include "UObject/Object.h"
#include "itemBase.generated.h"

class UInvenzaInventorySettingsAsset;
class APortableInventoryState;
class UInventoryBase;
struct FItemData;

/**
 * Base class for inventory items
 */
UCLASS(Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API UItemBase : public UObject, public IObjectDataProvider
{
	GENERATED_BODY()

public:
	UItemBase();

protected:
	// Unreal overrides
	// NETWORKING

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Properties and variables
	// EVENTS

	UPROPERTY(BlueprintAssignable, Category = "Item|Events")
	FOnUseItemDelegate OnUseItemDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Item|Events")
	FOnAmountChangedDelegate OnAmountChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Item|Events")
	FOnItemDataReplicated OnItemDataReplicated;

	// Functions
	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintPure, Category = "Item|Portable Inventory")
	bool IsPortableInventory() const;

	APortableInventoryState* GetPortableState() const;
	APortableInventoryState* EnsurePortableState();
	bool HasPortableContents() const;
	void RefreshPortableSummary();
	bool CanStoreInInventory(const UInventoryBase* Inventory) const;
	UItemBase* CreateIdentityTransferCopy(UObject* NewOuter);

	virtual bool IsSupportedForNetworking() const override { return true; }

	// INITIALIZATION

	UFUNCTION(BlueprintCallable, Category = "Item|Initialization")
	virtual void InitItem(FName ID, FItemData Data, int32 InQuantity);

	// ACTIONS

	virtual bool
	CanPerformAction_Implementation(EObjectInteractionType Action,
									const UInvenzaInventorySettingsAsset* SettingsAsset = nullptr) override;

	virtual void UseItem_Implementation() override;

	// ITEM OPERATIONS

	virtual UObject* DuplicateItem_Implementation() override;

	// QUANTITY & STACK

	virtual bool IsStackable_Implementation() override { return !IsPortableInventory() && ItemRef.ItemNumeraticData.MaxStackSizeInCharacter > 1; }

	virtual FVector2D GetMinMaxSplit_Implementation() override { return FVector2D(1.0f, Quantity); }

	virtual bool IsFullItemStack_Implementation() override
	{
		return Quantity == ItemRef.ItemNumeraticData.MaxStackSizeInCharacter;
	}

	virtual int32 GetQuantity_Implementation() const override { return Quantity; }

	virtual void SetQuantity_Implementation(int32 NewQuantity) override { Quantity = NewQuantity; }

	// WEIGHT

	virtual float GetItemStackWeight_Implementation() override;

	virtual float GetItemSingleWeight_Implementation() override;

	// FOOTPRINT & ORIENTATION

	virtual EItemOrientationType GetInitialItemOrientation_Implementation() override;

	virtual FIntPoint GetItemSize_Implementation(EItemOrientationType Orientation) override;

	// CATEGORY

	virtual FString CategoryToString_Implementation() override;

	virtual FString CategoryToShortString_Implementation() override;

	// RESERVATION

	virtual int32 GetFreeAmount_Implementation() const override;

	virtual bool ReserveAmount_Implementation(AActor* Requestor, int32 AmountToReserve) override;

	virtual void ReleaseReservation_Implementation(AActor* Requestor, int32 AmountToRelease) override;

	virtual UObject* ConsumeReserved_Implementation(AActor* Requestor, int32 RequestedAmount) override;

	// DATA ACCESS

	virtual FName GetItemID_Implementation() const override { return ItemID; }

	virtual FItemMetaData GetItemRef_Implementation() override { return ItemRef; }

	virtual FDataTableRowHandle GetItemRow_Implementation() override { return ItemRow; }

	virtual FText GetItemDisplayText_Implementation() const override { return ItemRef.ItemTextData.DisplayName; }

	// EVENT ACCESS

	virtual FOnUseItemDelegate& GetOnUseItemDelegate() override { return OnUseItemDelegate; }

	virtual FOnAmountChangedDelegate& GetOnAmountChangedDelegate() override { return OnAmountChangedDelegate; }

	virtual FOnItemDataReplicated& GetOnItemDataReplicatedDelegate() override { return OnItemDataReplicated; }

	// DATA MUTATORS

	FGuid GetPersistentInstanceID() const { return PersistentInstanceID; }

	void SetPersistentInstanceID(FGuid ID) { PersistentInstanceID = ID; }

	UFUNCTION(BlueprintNativeEvent, Category = "Item|Save")
	TMap<FName, FString> CaptureSaveState() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Item|Save")
	void RestoreSaveState(const TMap<FName, FString>& State);

	void SetItemRow(const FDataTableRowHandle& InRowHandle) { ItemRow = InRowHandle; }

	void SetItemRef(const FItemMetaData& NewItemRef) { ItemRef = NewItemRef; }

	/** Copy authored state to a new owner without copying reservations or runtime listeners. */
	UItemBase* CreateTransferCopy(UObject* NewOuter, int32 Amount);

	/** Commit helper: defer inventory/UI notifications until the complete transaction is applied. */
	void SetQuantityForTransfer(int32 Amount) { Quantity = Amount; }

protected:
	// Runtime
	// Properties and variables
	// ITEM DATA

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Item|Data")
	FName ItemID;

	UPROPERTY(VisibleInstanceOnly, Replicated, Category = "Item|Save")
	FGuid PersistentInstanceID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Data")
	FItemMetaData ItemRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_ItemRow, Category = "Item|Data")
	FDataTableRowHandle ItemRow;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Item|Data")
	int32 Quantity;

	UPROPERTY(ReplicatedUsing = OnRep_PortableSummary)
	float PortableContentsWeight = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_PortableSummary)
	bool bPortableHasContents = false;

	// RESERVATION DATA

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Reservation")
	TMap<TObjectPtr<AActor>, int32> ReservedAmounts;

	UPROPERTY(ReplicatedUsing = OnRep_AvailableAmount)
	int32 ReservedQuantity = 0;

	// Functions
	// REPLICATION

	UFUNCTION()
	void OnRep_ItemRow();

	UFUNCTION()
	void OnRep_AvailableAmount();

	UFUNCTION()
	void OnRep_PortableSummary();

	void UpdateReservedQuantity();
};

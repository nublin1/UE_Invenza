//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/Map.h"
#include "Data/Inventory/InventoryReplicationTypes.h"
#include "UObject/ObjectPtr.h"
#include "Engine/DataTable.h"
#include "Data/Items/itemBase.h"
#include "Data/Inventory/InventoryTypes.h"
#include "GameplayTagContainer.h"
#include "SaveLoad/InventorySaveTypes.h"
#include "InventoryCollectionComponent.generated.h"

struct FItemSaveEntry;
class UIInventoryManager;
struct FItemMappingSaveData;
struct FItemSaveData;
class UUInventoryBaseWidget;
class UInventorySlot;
class UInventoryItemWidget;
class USlotbasedInventorySlot;
class USlotbasedInventoryWidget;
class UObject;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryCollectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryCollectionComponent();

protected:
	// Unreal overrides
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryItemsChanged, const FString&, InventoryID);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActorInventoriesChanged);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollectionItemsIsEmpty, bool, IsEmpty);
#pragma endregion Delegates

	// Properties and variables
	UPROPERTY(BlueprintAssignable, Category = "Inventory Collection")
	FOnInventoryItemsChanged OnInventoryItemsChanged;

	/** Inventory list or inventory selection/size data became available or changed. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory Collection")
	FOnActorInventoriesChanged OnActorInventoriesChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory Collection")
	FOnCollectionItemsIsEmpty OnCollectionItemsIsEmpty;

	/** Optional explicit identity; set on a level instance to retain saves across renames. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Inventory Collection|Save")
	FGuid PersistentOwnerID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Collection|Save")
	bool bPersistInventories = true;

	// Functions
	FString GetPersistentOwnerKey() const;

	FString GetPersistentInventoryKey(UInventoryBase* Inventory) const;

	void SetRuntimeSaveIdentity(const FString& Key);

	void ResolvePlayerSaveIdentity();

	/** Recreate physical items crossing a linked-storage actor boundary under their new owner.
	 * Keeps persistent instance IDs and mappings, without depending on a closed actor channel. */
	void NormalizeLinkedStorageItemOwnership();

	bool IsRuntimeSaveActor() const { return bRuntimeSaveActor; }

	UFUNCTION(BlueprintCallable, Category = "Inventory Collection|Save")
	bool CapturePersistence(FInventorySavedOwner& Data, FString& Error);

	const TArray<FInventorySavedItem>& GetRecoveryItems() const { return RecoveryItems; }

	bool PreparePersistence(const FInventorySavedOwner& Data, FInventoryPreparedOwner& Plan, FString& Error,
							bool bIgnoreCapacityLimits = false);

	void CommitPersistence(FInventoryPreparedOwner& Plan);

	void NotifyPersistenceRestored();

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	void InitItemCollection();

	// Invs
	TArray<UInventoryBase*> GetActorInventories() { return ActorInventories; }

	FLinkedInventories GetLinkedInventories() { return LinkedInventories; }

	FLinkedCraftInventories GetLinkedCraftInventories() const { return LinkedCraftInventories; }

	void SetVendorInventory(UInventoryBase* InVendorInv);

	void SetExternalInventory(UInventoryBase* InExternalInventory);

	void SetCraftInventories(UInventoryBase* InInput, UInventoryBase* InFuel, UInventoryBase* InOutput);

	UFUNCTION()
	void SetInvManager(UIInventoryManager* NewManager) { InventoryArray.OwningManager = NewManager; }

	UFUNCTION(Server, Reliable)
	void Server_SetSlotBasedInventoryWidgetInitData(const FString& ContainerID,
													FSlotBasedInventoryWidgetInitData InitData);

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	UInventoryBase* GetInventoryByTag(const FGameplayTag& Tag);

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	TArray<UInventoryBase*> GetAllInventoriesByTag(const FGameplayTag& Tag);

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	UInventoryBase* GetInventoryByID(FString ContainerID);

	void AddPawnInventory_Internal(UInventoryBase* InInventory);

	// Widgets
	void RegisterContainerWidget(UInventoryBase* Inventory, UInventoryContainerWidget* Widget);

	void UnregisterContainerWidget(UInventoryBase* Inventory);

	UInventoryContainerWidget* GetContainerWidget(UInventoryBase* Inventory) const;

	bool HasContainerWidget(UInventoryBase* Inventory) const { return InventoryContainerWidgetMap.Contains(Inventory); }

	const TMap<TObjectPtr<UInventoryBase>, TObjectPtr<UInventoryContainerWidget>>& GetContainerWidgetMap() const
	{
		return InventoryContainerWidgetMap;
	}

	// Items
	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	void MarkItemAsDirty(UObject* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory Collection")
	float CalculateAvailableMoney();

	UFUNCTION(BlueprintCallable, Category = "Inventory Collection")
	void UpdateItemMapping(UObject* Item, const FString& InventoryID, const TArray<UInventorySlotData*>& NewSlots,
						   EItemOrientationType NewOrientation);

	UFUNCTION(BlueprintCallable, Category = "Inventory Collection")
	void UpdateItemVisualLinks(UObject* Item, const FString& InventoryID, UInventoryItemWidget* InWidget = nullptr,
							   AStorageVisualRepresentation* InActor = nullptr);

	FInventoryArray GetItemLocations() const { return InventoryArray; }

	UFUNCTION(
		Category = "Invenza|Inventory", BlueprintCallable,
		meta =
			(ToolTip =
				 "Returns a list of resources stored in this container, aggregating identical resources and summing their total amount."))
	TArray<FItemIDEntry> CollectItemsAggregated(FString InvID);

	int32 GetStackCountInContainer(FString InvID);

	TArray<UObject*> GetAllItemsByContainer(FString InvID);

	TArray<UObject*> GetAllSameItemsInContainerByItemSample(const FString& InvID, const UObject* ReferenceItem) const;

	TArray<UObject*> GetAllSameItemsInContainerByID(const FString& InvID, FName ReferenceID) const;

	TArray<FItemMapping> GetAllMappingsByContainer(const FString& InvID);

	TMap<UObject*, FItemMapping*> GetItemsWithMappingsByContainer(const FString& InvID);

	TArray<UObject*> GetAllItemsByCategory(FGameplayTag ItemCategory);

	UObject* GetItemFromSlot(FGuid TargetSlotID, const FString& InventoryID);

	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	FItemMapping AddItem(UObject* NewItem, const FItemMapping& ItemMapping);

	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	void RemoveItem(UObject* Item, FString ContainerID);

	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	void RemoveItemFromAllContainers(UObject* Item);

	TArray<FGuid> GetOccupatedSlotsIDByContainerName(FString InventoryID, UObject* Item);

	FItemMapping* FindItemMappingByContainerName(UObject* Item, FString InventoryID);

	TArray<FItemMapping*> FindAllMappingsForItem(UObject* Item);

	FItemMapping* FindMainInventoryMappingForItem(UObject* Item);

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	bool ItemHasInventory(UObject* Item, FString InventoryID);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool IsItemOwnedByActor(UObject* Item);

	//

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	void RequestSortInventory(const FString& ContainerID, EInventorySortCriteria Criteria);

	UFUNCTION(Server, Reliable)
	virtual void Server_SortItems(const FString& ContainerID, EInventorySortCriteria Criteria);

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	void SerializeForSave(TArray<FItemSaveEntry>& OutData, const TArray<FString>& InventoryFilter);

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	void DeserializeFromSave(const TArray<FItemSaveEntry>& InData,
							 UInventoryBase* OverrideInventory, // Used to simulate a specific inventory
							 const TMap<FString, FString>& IDMapping);

	// Key: Old ID, Value: New ID

	UFUNCTION()
	void NotifyUI_ItemChanged(UObject* Item, const FString& ContainerID, EInventoryActionType Action);

	UFUNCTION()
	void NotifyUI_ReDraw(const FString& ContainerID);

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Replicated)
	FString ResolvedSaveKey;

	bool bRuntimeSaveActor = false;

	UPROPERTY(Transient)
	TArray<FInventorySavedItem> RecoveryItems;

	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadOnly,
			  ReplicatedUsing = OnRep_InventoryArray)
	FInventoryArray InventoryArray;

	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite,
			  ReplicatedUsing = OnRep_ActorInventories)
	TArray<TObjectPtr<UInventoryBase>> ActorInventories;

	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite,
			  ReplicatedUsing = OnRep_LinkedInventories)
	FLinkedInventories LinkedInventories;

	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite, Replicated,
			  ReplicatedUsing = OnRep_LinkedCraftInventories)
	FLinkedCraftInventories LinkedCraftInventories;

	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite)
	TMap<TObjectPtr<UInventoryBase>, TObjectPtr<UInventoryContainerWidget>> InventoryContainerWidgetMap;

	//
	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite)
	bool bWasInit = false;

	// Functions
	virtual void ReadyForReplication() override;

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	UFUNCTION()
	void OnRep_InventoryArray();

	UFUNCTION()
	void OnRep_LinkedInventories(const FLinkedInventories& PreviousInventories);

	UFUNCTION()
	void OnRep_LinkedCraftInventories();

	UFUNCTION()
	void OnRep_ActorInventories();

	UFUNCTION()
	void OnItemDataReplicated(UObject* Item);

	virtual FItemMapping* GetMappingMutable(UObject* Item, const FString& InventoryID, FInventoryEntry*& OutEntry);
};

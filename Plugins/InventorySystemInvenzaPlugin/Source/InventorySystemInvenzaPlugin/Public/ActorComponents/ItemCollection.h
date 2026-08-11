//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/Map.h"
#include "Data/Inventory/InventoryReplicationTypes.h"
#include "UObject/ObjectPtr.h"
#include "Engine/DataTable.h"
#include "Data/Items/ItemBase.h"
#include "Data/Inventory/InventoryTypes.h"
#include "GameplayTagContainer.h"
#include "ItemCollection.generated.h"

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


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UItemCollection : public UActorComponent
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryItemsChanged, const FString&, InventoryID);
#pragma endregion Delegates

public:
	UItemCollection();

protected:
	virtual void BeginPlay() override;

	virtual void ReadyForReplication() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable, Category = "Item Collection")
	FOnInventoryItemsChanged OnInventoryItemsChanged;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable)
	void InitItemCollection();

	// Invs
	TArray<UInventoryBase*> GetActorInventories() {return ActorInventories;}
	
	FLinkedInventories GetLinkedInventories() {return LinkedInventories;}
	
	void SetVendorInventory(UInventoryBase* InVendorInv) {LinkedInventories.SetVendor(InVendorInv);}
	void SetExternalInventory(UInventoryBase* InExternalInventory) {LinkedInventories.SetExternal(InExternalInventory);}

	UFUNCTION(Server, Reliable)
	void Server_SetSlotBasedInventoryWidgetInitData(const FString& ContainerID, FSlotBasedInventoryWidgetInitData InitData);

	UFUNCTION(BlueprintCallable)
	UInventoryBase* GetInventoryByTag(const FGameplayTag& Tag);
	UFUNCTION(BlueprintCallable)
	TArray<UInventoryBase*> GetAllInventoriesByTag(const FGameplayTag& Tag);
	UFUNCTION(BlueprintCallable)
	UInventoryBase* GetInventoryByID(FString ContainerID);

	void AddPawnInventory_Internal(UInventoryBase* InInventory);

	// Widgets
	void RegisterContainerWidget(UInventoryBase* Inventory,	UInventoryContainerWidget* Widget);

	void UnregisterContainerWidget(UInventoryBase* Inventory);

	UInventoryContainerWidget* GetContainerWidget(UInventoryBase* Inventory) const;

	bool HasContainerWidget(UInventoryBase* Inventory) const { return InventoryContainerWidgetMap.Contains(Inventory);}

	const TMap<TObjectPtr<UInventoryBase>, TObjectPtr<UInventoryContainerWidget>>& GetContainerWidgetMap() const
	{
		return InventoryContainerWidgetMap;
	}
	
	// Items
	UFUNCTION(BlueprintCallable)
	void MarkItemAsDirty(UObject* Item);
	
	UFUNCTION(BlueprintCallable, Category = "Item Collection")
	float CalculateAvailableMoney();

	UFUNCTION(BlueprintCallable, Category = "Item Collection")
	void UpdateItemMapping(UObject* Item, const FString& InventoryID, const TArray<UInventorySlotData*>& NewSlots, EItemOrientationType NewOrientation);
	UFUNCTION(BlueprintCallable, Category = "Item Collection")
	void UpdateItemVisualLinks(UObject* Item, const FString& InventoryID, 
											UInventoryItemWidget* InWidget = nullptr, 
											AStorageVisualRepresentation* InActor = nullptr);

	FInventoryArray GetItemLocations() const {return InventoryArray;}

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Returns a list of resources stored in this container, aggregating identical resources and summing their total amount."))
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
	UFUNCTION(BlueprintCallable, Category="Item Collection|Item Management")
	void RemoveItem(UObject* Item, FString ContainerID);
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	void RemoveItemFromAllContainers(UObject* Item);
	
	TArray<FGuid> GetOccupatedSlotsIDByContainerName(FString InventoryID, UObject* Item);
	FItemMapping* FindItemMappingByContainerName(UObject* Item, FString InventoryID);
	TArray<FItemMapping> FindAllMappingsForItem(UObject* Item);
	UInventoryBase* FindMainInventoryForItem(UObject* Item);

	UFUNCTION(BlueprintCallable)
	bool ItemHasInventory(UObject* Item, FString InventoryID);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool IsItemOwnedByActor(UObject* Item);

	//
	UFUNCTION()
	void SetInvManager(UIInventoryManager* NewManager) {InventoryArray.OwningManager = NewManager;}

	UFUNCTION(BlueprintCallable)
	void SerializeForSave(TArray<FItemSaveEntry>& OutData, const TArray<FString>& InventoryFilter);
	UFUNCTION(BlueprintCallable)
	void DeserializeFromSave(const TArray<FItemSaveEntry>& InData,
		UInventoryBase* OverrideInventory, // Used to simulate a specific inventory
		const TMap<FString, FString>& IDMapping); // Key: Old ID, Value: New ID

	UFUNCTION()
	void NotifyUI_ItemChanged(UObject* Item, const FString& ContainerID, EInventoryActionType Action);

	UFUNCTION()
	void NotifyUI_ReDraw(const FString& ContainerID);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	FInventoryArray InventoryArray;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_ActorInventories)
	TArray<TObjectPtr<UInventoryBase>> ActorInventories;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated, ReplicatedUsing=OnRep_LinkedInventories)
	FLinkedInventories LinkedInventories;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TMap<TObjectPtr<UInventoryBase>, TObjectPtr<UInventoryContainerWidget>> InventoryContainerWidgetMap;

	//
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	bool bWasInit = false;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void OnRep_LinkedInventories();
		
	UFUNCTION()
	void OnRep_ActorInventories();

	UFUNCTION()
	void OnItemDataReplicated(UObject* Item);

	virtual FItemMapping* GetMappingMutable(UObject* Item, const FString& InventoryID, FInventoryEntry*& OutEntry);
	
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
};

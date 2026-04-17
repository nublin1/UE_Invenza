//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/Map.h"
#include "UObject/ObjectPtr.h"
#include "Engine/DataTable.h"
#include "Data/Items/ItemBase.h"
#include "Data/Inventory/InventoryTypes.h"
#include "Net/Serialization/FastArraySerializer.h"
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
class UItemBase;

USTRUCT(BlueprintType)
struct FInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UItemBase> Item = nullptr;
	
	UPROPERTY()
	FItemMappingArrayWrapper Locations;
	
	void PostReplicatedAdd(const struct FInventoryArray& InArraySerializer);
	void PostReplicatedChange(const struct FInventoryArray& InArraySerializer);
	void PreReplicatedRemove(const struct FInventoryArray& InArraySerializer);
};

USTRUCT(BlueprintType)
struct FInventoryArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FInventoryEntry> Items;
	UPROPERTY()
	TObjectPtr<UIInventoryManager> OwningManager;
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryEntry, FInventoryArray>(Items, DeltaParms, *this);
	}
};

template<>
struct TStructOpsTypeTraits<FInventoryArray> : public TStructOpsTypeTraitsBase2<FInventoryArray>
{
	enum { WithNetDeltaSerializer = true };
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UItemCollection : public UActorComponent
{
	GENERATED_BODY()

public:
	UItemCollection();

protected:
	virtual void BeginPlay() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	// Invs
	TArray<UInventoryBase*> GetActorInventories() {return ActorInventories;}

	void AddPawnInventory_Internal(UInventoryBase* InInventory);
	
	// Items
	UFUNCTION(BlueprintCallable, Category = "Item Collection")
	float CalculateAvailableMoney();

	FInventoryArray GetItemLocations() const {return InventoryArray;}
	
	int32 GetStackCountInContainer(FString InvID);
	TArray<UItemBase*> GetAllItemsByContainer(FString InvID);
	TArray<UItemBase*> GetAllSameItemsInContainer(FString InvID, UItemBase* ReferenceItem) const;
	TArray<FItemMapping> GetAllMappingsByContainer(const FString& InvID);
	TMap<UItemBase*, FItemMapping*> GetItemsWithMappingsByContainer(const FString& InvID);
	TArray<UItemBase*> GetAllItemsByCategory(EItemCategory ItemCategory);
	UItemBase* GetItemFromSlot(UInventorySlotData* TargetSlotData, const FString& InventoryID);
	
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	FItemMapping& AddItem(UItemBase* NewItem, const FItemMapping& ItemMapping);
	UFUNCTION(BlueprintCallable, Category="Item Collection|Item Management")
	void RemoveItem(UItemBase* Item, FString ContainerID);
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	void RemoveItemFromAllContainers(UItemBase* Item);
	
	FItemMapping* FindItemMappingByContainerName(UItemBase* Item, FString InventoryID);

	UFUNCTION(BlueprintCallable)
	bool ItemHasInventory(UItemBase* Item, FString InventoryID);

	//
	UFUNCTION()
	void SetInvManager(UIInventoryManager* NewManager) {InvManager = NewManager;}

	UFUNCTION(BlueprintCallable)
	void SerializeForSave(TArray<FItemSaveEntry>& OutData, const TArray<FString>& InventoryFilter);
	UFUNCTION(BlueprintCallable)
	void DeserializeFromSave(const TArray<FItemSaveEntry>& InData,
		UInventoryBase* OverrideInventory, // Used to simulate a specific inventory
		const TMap<FString, FString>& IDMapping); // Key: Old ID, Value: New ID

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY()
	TObjectPtr<UIInventoryManager> InvManager = nullptr;
	
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	//TMap<TObjectPtr<UItemBase>, FItemMappingArrayWrapper> ItemLocations;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	FInventoryArray InventoryArray;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated )
	TArray<TObjectPtr<UInventoryBase>> ActorInventories;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
};

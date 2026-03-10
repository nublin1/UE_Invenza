//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/Map.h"
#include "UObject/ObjectPtr.h"
#include "Engine/DataTable.h"
#include "Data/Items/ItemBase.h"
#include "UI/Inventory/InventoryTypes.h"
#include "ItemCollection.generated.h"


struct FItemSaveEntry;
class UIInventoryManager;
struct FItemMappingSaveData;
struct FItemSaveData;
class UUInventoryWidgetBase;
class UInventorySlot;
class UInventoryItemWidget;
class USlotbasedInventorySlot;
class USlotbasedInventoryWidget;
class UItemBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UItemCollection : public UActorComponent
{
	GENERATED_BODY()

public:
	UItemCollection();

protected:
	virtual void BeginPlay() override;

public:
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	TMap<TObjectPtr<UItemBase>, FItemMappingArrayWrapper> GetItemLocations() const {return ItemLocations;}
	
	TArray<UItemBase*> GetAllItemsByContainer(FString InvID);
	TArray<UItemBase*> GetAllSameItemsInContainer(FString InvID, UItemBase* ReferenceItem) const;
	TArray<FItemMapping> GetAllMappingsByContainer(const FString& InvID);
	TArray<UItemBase*> GetAllItemsByCategory(EItemCategory ItemCategory);
	UItemBase* GetItemFromSlot(UInventorySlotData* TargetSlotData, const FString& InventoryID);
	
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	FItemMappingArrayWrapper AddItem(UItemBase* NewItem, FItemMapping ItemMapping);
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

	
	
	/*
	
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	void RemoveItemFromAllContainers(UItemBase* Item);
	
	FItemMapping* FindItemMappingForItemInContainer(UItemBase* TargetItem, UInvBaseContainerWidget* InContainer);
	bool HasItemInContainer(UItemBase* Item, UInvBaseContainerWidget* Container) const;
	
	TMap<TObjectPtr<UItemBase>, FItemMappingArrayWrapper> GetItemLocations() const {return ItemLocations;}
	TArray<FInventorySlotData> CollectOccupiedSlotsByContainer(UInvBaseContainerWidget* InContainer);
	UItemBase* GetItemFromSlot(FInventorySlotData TargetSlotData, UInvBaseContainerWidget* TargetContainer) const;
	

	UInventoryItemWidget* GetItemLinkedWidgetForSlot(FInventorySlotData ItemSlotData);

	virtual void SortInContainer(UInvBaseContainerWidget* ContainerToSort);
	
	void SerializeForSave(TArray<FItemSaveEntry>& OutData);
	void DeserializeFromSave(TArray<FItemSaveEntry> InData);*/

	//
	UPROPERTY()
	TObjectPtr<UIInventoryManager> InvManager = nullptr;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY()
	TMap<TObjectPtr<UItemBase>, FItemMappingArrayWrapper> ItemLocations; //ItemLocations
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};

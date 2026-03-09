//  Nublin Studio 2025 All Rights Reserved.

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

	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	FItemMappingArrayWrapper AddItem(UItemBase* NewItem, FItemMapping ItemMapping);
	UFUNCTION(BlueprintCallable, FString = "Item Collection|Item Management")
	void RemoveItem(UItemBase* Item, FName ContainerID);
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	void RemoveItemFromAllContainers(UItemBase* Item);
	
	FItemMapping* FindItemMappingByContainerName(UItemBase* Item, FString InventoryID);

	UFUNCTION(BlueprintCallable)
	bool ItemHasInventory(UItemBase* Item, FString InventoryID);

	UItemBase* GetItemFromSlot(UInventorySlotData* TargetSlotData, const FString& InventoryID);
	
	/*
	
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	void RemoveItemFromAllContainers(UItemBase* Item);
	
	FItemMapping* FindItemMappingForItemInContainer(UItemBase* TargetItem, UInvBaseContainerWidget* InContainer);
	bool HasItemInContainer(UItemBase* Item, UInvBaseContainerWidget* Container) const;
	
	TMap<TObjectPtr<UItemBase>, FItemMappingArrayWrapper> GetItemLocations() const {return ItemLocations;}
	TArray<FInventorySlotData> CollectOccupiedSlotsByContainer(UInvBaseContainerWidget* InContainer);
	UItemBase* GetItemFromSlot(FInventorySlotData TargetSlotData, UInvBaseContainerWidget* TargetContainer) const;
	TArray<UItemBase*> GetAllItemsByContainer(UInvBaseContainerWidget* TargetContainer) const;
	TArray<UItemBase*> GetAllSameItemsInContainer(UInvBaseContainerWidget* TargetContainer, UItemBase* ReferenceItem) const;
	TArray<UItemBase*> GetAllItemsByCategory(EItemCategory ItemCategory);
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

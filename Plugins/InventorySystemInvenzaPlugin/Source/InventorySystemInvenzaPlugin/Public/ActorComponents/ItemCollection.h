//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/Map.h"
#include "UObject/ObjectPtr.h"
#include "Engine/DataTable.h"
#include "Items/itemBase.h"
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
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	/** Data table containing item information */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Collection|Config")
	TObjectPtr<UDataTable> ItemDataTable;

	/** Map of initial items with their quantities */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Collection|Config")
	TArray<FItemEntry> InitItems;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UItemCollection();
	
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	void AddItem(UItemBase* NewItem, FItemMapping ItemMapping);
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	void RemoveItem(UItemBase* Item, UInvBaseContainerWidget* Container);
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
	void DeserializeFromSave(TArray<FItemSaveEntry> InData);

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

	virtual void BeginPlay() override;
};

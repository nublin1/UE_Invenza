//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/Map.h"
#include "Items/itemBase.h"
#include "ItemCollection.generated.h"


class UInventorySlot;
class UInventoryItemWidget;
class USlotbasedInventorySlot;
struct FItemMapping;
class USlotbasedInventoryWidget;
class UItemBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRIDINVENTORYPLUGIN_API UItemCollection : public UActorComponent
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
	void RemoveItem(UItemBase* Item, USlotbasedInventoryWidget* Container);
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	void RemoveItemFromAllContainers(UItemBase* Item);
	
	FItemMapping* FindItemMappingForItemInContainer(UItemBase* TargetItem, USlotbasedInventoryWidget* InContainer);
	bool HasItemInContainer(UItemBase* Item, USlotbasedInventoryWidget* Container) const;

	
	TMap<UItemBase*, TArray<FItemMapping>> GetItemLocations() const {return ItemLocations;}
	TArray<TObjectPtr<UInventorySlot>> CollectOccupiedSlotsByContainer(USlotbasedInventoryWidget* InContainer);
	UItemBase* GetItemFromSlot(UInventorySlot* TargetSlot, USlotbasedInventoryWidget* TargetContainer) const;
	TArray<UItemBase*> GetAllItemsByContainer(USlotbasedInventoryWidget* TargetContainer) const;
	TArray<UItemBase*> GetAllSameItemsInContainer(USlotbasedInventoryWidget* TargetContainer, UItemBase* ReferenceItem) const;
	UInventoryItemWidget* GetItemLinkedWidgetForSlot(USlotbasedInventorySlot* _ItemSlot) const;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	TMap<UItemBase*, TArray<FItemMapping>> ItemLocations; //ItemLocations
	//TMap<UItemBase*, TArray<UBaseInventoryWidget*>> ItemContainers;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================


};

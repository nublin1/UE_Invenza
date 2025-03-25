// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemCollection.generated.h"


class UInventoryItemWidget;
class UBaseInventorySlot;
struct FItemMapping;
class UBaseInventoryWidget;
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
	TMap<FName, int32> InitItems;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UItemCollection();
	
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	void AddItem(UItemBase* NewItem, FItemMapping ItemMapping);
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	void RemoveItem(UItemBase* Item, UBaseInventoryWidget* Container);
	UFUNCTION(BlueprintCallable, Category = "Item Collection|Item Management")
	void RemoveItemFromAllContainers(UItemBase* Item);
	
	FItemMapping* FindItemMappingForItemInContainer(UItemBase* TargetItem, UBaseInventoryWidget* InContainer);
	bool HasItemInContainer(UItemBase* Item, UBaseInventoryWidget* Container) const;

	
	TMap<UItemBase*, TArray<FItemMapping>> GetItemLocations() const {return ItemLocations;}
	TArray<TObjectPtr<UBaseInventorySlot>> CollectOccupiedSlotsByContainer(UBaseInventoryWidget* InContainer);
	UItemBase* GetItemFromSlot(UBaseInventorySlot* TargetSlot, UBaseInventoryWidget* TargetContainer) const;
	TArray<UItemBase*> GetAllItemsByContainer(UBaseInventoryWidget* TargetContainer) const;
	TArray<UItemBase*> GetAllSameItemsInContainer(UBaseInventoryWidget* TargetContainer, UItemBase* ReferenceItem) const;
	UInventoryItemWidget* GetItemLinkedWidgetForSlot(UBaseInventorySlot* _ItemSlot) const;

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

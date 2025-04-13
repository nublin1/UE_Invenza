//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"
#include "Settings/Settings.h"
#include "UI/BaseUserWidget.h"
#include "UInventoryWidgetBase.generated.h"

#pragma region Delegates
class UItemCollection;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUpdateDelegate, const FItemMapping&, ItemSlots, UItemBase*,
                                             Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAddItemDelegate, FItemMapping, ItemSlots, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRemoveItemDelegate, FItemMapping, ItemSlots, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUseItemDelegate, USlotbasedInventorySlot*, ItemSlot, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWightUpdatedDelegate, float, InventoryTotalWeight, float, InventoryWeightCapacity);
#pragma endregion Delegates

/**
 * 
 */
UCLASS(Abstract)
class GRIDINVENTORYPLUGIN_API UUInventoryWidgetBase : public UBaseUserWidget
{
	GENERATED_BODY()
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	FOnItemUpdateDelegate OnItemUpdateDelegate;
	FOnAddItemDelegate OnAddItemDelegate;	
	FOnRemoveItemDelegate OnRemoveItemDelegate;
	FOnUseItemDelegate OnUseItemDelegate;
	FOnWightUpdatedDelegate OnWightUpdatedDelegate;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(Category="Inventory")
	virtual void InitializeInventory() PURE_VIRTUAL(UUInventoryWidgetBase::InitializeInventory,);
	UFUNCTION(Category="Inventory")
	virtual void ReDrawAllItems() PURE_VIRTUAL(UUInventoryWidgetBase::ReDrawAllItems,);
	
	UFUNCTION(Category="Inventory")
	virtual void HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity) PURE_VIRTUAL(UUInventoryWidgetBase::HandleRemoveItem,);
	UFUNCTION(Category="Inventory")
	virtual void HandleRemoveItemFromContainer(UItemBase* Item) PURE_VIRTUAL(UUInventoryWidgetBase::HandleRemoveItemFromContainer,);
	UFUNCTION(BlueprintCallable, Category="Inventory")
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) PURE_VIRTUAL(UUInventoryWidgetBase::HandleAddItem, return FItemAddResult(););

	UFUNCTION(Category="Inventory")
	FORCEINLINE float GetWeightCapacity() const {return InventoryWeightCapacity;}
	UFUNCTION(Category="Inventory")
	FORCEINLINE UItemCollection* GetItemCollection() {return ItemCollectionLink;}
	UFUNCTION(Category="Inventory")
	FORCEINLINE bool GetCanReferenceItems() const {return bCanReferenceItems;}
	UFUNCTION(Category="Inventory")
	FORCEINLINE bool GetIsUseReferences() const {return bUseReferences;}

	//Setters
	FORCEINLINE void SetItemCollection(UItemCollection* _ItemCollection) {ItemCollectionLink = _ItemCollection;}
	FORCEINLINE virtual void SetUISettings(FUISettings NewSettings) {UISettings = NewSettings;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Data
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Inventory")
	TObjectPtr<UItemCollection> ItemCollectionLink;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	float InventoryTotalWeight = 0;

	// Settings
	UPROPERTY(visibleAnywhere, BlueprintReadWrite, Category="Inventory")
	FUISettings UISettings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	float InventoryWeightCapacity = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(tooltip="If true this container will be used as reference."))
	bool bUseReferences = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(tooltip="Can the items be referenced from this container"))
	bool bCanReferenceItems = false;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual FItemMapping* GetItemMapping(UItemBase* Item);
	virtual int32 CalculateActualAmountToAdd(int32 InAmountToAdd, float ItemSingleWeight);
	
	UFUNCTION()
	virtual FItemAddResult HandleNonStackableItems(FItemMoveData& ItemMoveData, bool bOnlyCheck = false) PURE_VIRTUAL(UUInventoryWidgetBase::HandleNonStackableItems, return FItemAddResult(););
	UFUNCTION()
	virtual int32 HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount,
												bool bOnlyCheck) PURE_VIRTUAL(UUInventoryWidgetBase::HandleStackableItems, return 0;);
	UFUNCTION()
	virtual FItemAddResult HandleAddReferenceItem(FItemMoveData& ItemMoveData) PURE_VIRTUAL(UUInventoryWidgetBase::HandleAddReferenceItem, return FItemAddResult(););
	UFUNCTION()
	virtual void AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots) PURE_VIRTUAL(UUInventoryWidgetBase::AddNewItem, );
	UFUNCTION()
	virtual void InsertToStackItem(UItemBase* Item, int32 AddQuantity);

	virtual void AddItemToPanel(UItemBase* Item)  PURE_VIRTUAL(UUInventoryWidgetBase::AddItemToPanel, );

	UFUNCTION()
	virtual void UpdateWeightInfo();

	// Notifications
	virtual void NotifyAddItem(FItemMapping& FromSlots, UItemBase* NewItem);
	virtual void NotifyUpdateItem(FItemMapping* FromSlots, UItemBase* NewItem);
	virtual void NotifyRemoveItem(FItemMapping& FromSlots, UItemBase* RemovedItem);
	//void NotifyUseSlot(UBaseInventorySlot* FromSlot);

	
};

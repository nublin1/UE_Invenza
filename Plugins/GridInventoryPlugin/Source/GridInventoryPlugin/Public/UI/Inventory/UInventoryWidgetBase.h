//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"
#include "Data/LogicChecks.h"
#include "Settings/Settings.h"
#include "UI/BaseUserWidget.h"
#include "UInventoryWidgetBase.generated.h"

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDropped, FItemMoveData, ItemMoveData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUpdateDelegate, FItemMapping, ItemSlots, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAddItemDelegate, FItemMapping, ItemSlots, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRemoveItemDelegate, FItemMapping, ItemSlots, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUseItemDelegate, USlotbasedInventorySlot*, ItemSlot, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWightUpdatedDelegate, float, InventoryTotalWeight, float, InventoryWeightCapacity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoneyUpdatedDelegate, int32, InventoryTotalMoney);
#pragma endregion Delegates

class UItemCollection;

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
	FOnItemDropped OnItemDroppedDelegate;
	FOnItemUpdateDelegate OnItemUpdateDelegate;
	FOnAddItemDelegate OnAddItemDelegate;	
	FOnRemoveItemDelegate OnRemoveItemDelegate;
	FOnUseItemDelegate OnUseItemDelegate;
	FOnWightUpdatedDelegate OnWightUpdatedDelegate;
	FOnMoneyUpdatedDelegate OnMoneyUpdatedDelegate;
	
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

	FORCEINLINE void AddCheck(EInventoryCheckType CheckType, FName CheckID, TFunction<bool(UItemBase*)> CheckFunction) { Checks.Emplace(CheckType, CheckID, CheckFunction);	}
	FORCEINLINE void RemoveAllCheck() { Checks.Empty();	}
	void RemoveCheck(EInventoryCheckType Type, FName CheckID)
	{
		Checks.RemoveAll([=](const FInventoryCheck& Check)
		{
			return Check.CheckType == Type && Check.CheckID == CheckID;
		});
	}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Data
	TArray<FInventoryCheck> Checks;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Inventory")
	TObjectPtr<UItemCollection> ItemCollectionLink;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	float InventoryTotalWeight = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	int32 InventoryTotalMoney = 0;

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
	UFUNCTION()
	virtual bool ExecuteItemChecks(EInventoryCheckType CheckType, UItemBase* Item);
	
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
	virtual void AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount) PURE_VIRTUAL(UUInventoryWidgetBase::AddNewItem, );
	UFUNCTION()
	virtual void InsertToStackItem(UItemBase* Item, int32 AddQuantity);

	virtual void AddItemToPanel(UItemBase* Item)  PURE_VIRTUAL(UUInventoryWidgetBase::AddItemToPanel, );

	UFUNCTION()
	virtual void UpdateWeightInfo();
	UFUNCTION()
	virtual void UpdateMoneyInfo();

	// Notifications
	virtual void NotifyAddItem(FItemMapping& FromSlots, UItemBase* NewItem, int32 ChangeQuantity);
	virtual void NotifyRemoveItem(FItemMapping& FromSlots, UItemBase* RemovedItem, int32 RemoveQuantity);
	//void NotifyUseSlot(UBaseInventorySlot* FromSlot);

	
};

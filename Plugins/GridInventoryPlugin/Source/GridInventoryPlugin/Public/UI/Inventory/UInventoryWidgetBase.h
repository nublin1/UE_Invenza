//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"
#include "Data/LogicChecks.h"
#include "Settings/Settings.h"
#include "UI/BaseUserWidget.h"
#include "UI/Container/InvBaseContainerWidget.h"
#include "UInventoryWidgetBase.generated.h"

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDropped, FItemMoveData, ItemMoveData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUpdateDelegate, FItemMapping, ItemSlots, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAddItemDelegate, FItemMapping, ItemSlots, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPreRemoveItemDelegate, FItemMapping, ItemSlots, UItemBase*, Item, int32, RemoveQuantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPostRemoveItemDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUseSlotDelegate, UInventorySlot*, ItemSlot);
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
	// Delegates
	UPROPERTY(BlueprintAssignable)
	FOnItemDropped OnItemDroppedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnItemUpdateDelegate OnItemUpdateDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnAddItemDelegate OnAddItemDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnPreRemoveItemDelegate OnPreRemoveItemDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnPostRemoveItemDelegate OnPostRemoveItemDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnUseSlotDelegate OnUseSlotDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnWightUpdatedDelegate OnWightUpdatedDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnMoneyUpdatedDelegate OnMoneyUpdatedDelegate;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(Category="Inventory")
	virtual void InitializeInventory() PURE_VIRTUAL(UUInventoryWidgetBase::InitializeInventory,);
	UFUNCTION(Category="Inventory")
	virtual void InitItemsInItemsCollection();
	UFUNCTION(Category="Inventory")
	virtual void ReDrawAllItems() PURE_VIRTUAL(UUInventoryWidgetBase::ReDrawAllItems,);
	UFUNCTION()
	virtual void UseSlot(UInventorySlot* UsedSlot);

	UFUNCTION()
	virtual bool HandleTradeModalOpening(UItemBase* Item);
	UFUNCTION(Category="Inventory")
	virtual void HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity) PURE_VIRTUAL(UUInventoryWidgetBase::HandleRemoveItem,);
	UFUNCTION(Category="Inventory")
	virtual void HandleRemoveItemFromContainer(UItemBase* Item) PURE_VIRTUAL(UUInventoryWidgetBase::HandleRemoveItemFromContainer,);
	UFUNCTION(BlueprintCallable, Category="Inventory")
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) PURE_VIRTUAL(UUInventoryWidgetBase::HandleAddItem, return FItemAddResult(););

	FUISettings GetUISettings() const {return UISettings;}
	FInventorySettings GetInventorySettings() const {return InventorySettings;}
	FInventoryData GetInventoryData() {return InventoryData;}
	
	//Setters
	FORCEINLINE void SetItemCollection(UItemCollection* _ItemCollection) {InventoryData.ItemCollectionLink = _ItemCollection;}
	FORCEINLINE virtual void SetUISettings(FUISettings NewSettings) {UISettings = NewSettings;}
	
	FORCEINLINE void AddCheck(EInventoryCheckType CheckType, FName CheckID, TFunction<bool(UItemBase*)> CheckFunction) { InventoryData.Checks.Emplace(CheckType, CheckID, CheckFunction);	}
	FORCEINLINE void RemoveAllCheck() { InventoryData.Checks.Empty();	}
	void RemoveCheck(EInventoryCheckType Type, FName CheckID)
	{
		InventoryData.Checks.RemoveAll([=](const FInventoryCheck& Check)
		{
			return Check.CheckType == Type && Check.CheckID == CheckID;
		});
	}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
	FInventoryData InventoryData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
	FInventorySettings InventorySettings;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Inventory")
	FUISettings UISettings;

	//
	UPROPERTY()
	TObjectPtr<UInventorySlot> SlotUnderMouse = nullptr;
	
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
	virtual FItemAddResult HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck) PURE_VIRTUAL(UUInventoryWidgetBase::HandleAddReferenceItem, return FItemAddResult(););
	UFUNCTION()
	virtual void AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount) PURE_VIRTUAL(UUInventoryWidgetBase::AddNewItem, );
	UFUNCTION()
	virtual void InsertToStackItem(UItemBase* Item, int32 AddQuantity);

	virtual void AddItemToPanel(UItemBase* Item)  PURE_VIRTUAL(UUInventoryWidgetBase::AddItemToPanel, );

	UFUNCTION()
	virtual void UpdateWeightInfo();
	UFUNCTION()
	virtual void UpdateMoneyInfo();

	//
	UFUNCTION()
	virtual UInvBaseContainerWidget* GetAsContainerWidget() { return Cast<UInvBaseContainerWidget>(ParentWidget);}
	
	
	//====================================================================
	// Event Notifiers
	//====================================================================
	virtual void NotifyAddItem(FItemMapping& FromSlots, UItemBase* NewItem, int32 ChangeQuantity);
	virtual void NotifyPreRemoveItem(FItemMapping& FromSlots, UItemBase* RemovedItem, int32 RemoveQuantity);
	virtual void NotifyPostRemoveItem();
	virtual void NotifyUseSlot(UInventorySlot* UsedSlot);

	
};

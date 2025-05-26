//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"
#include "Settings/Settings.h"
#include "UI/BaseUserWidget.h"
#include "UI/Container/InvBaseContainerWidget.h"
#include "UInventoryWidgetBase.generated.h"

#pragma region Delegates
class UUIButton;
enum class EItemCategory : uint8;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDropped, FItemMoveData, ItemMoveData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemReplaceDelegate, TArray<FInventorySlotData>, OldItemSlots, TArray<FInventorySlotData>, NewItemSlots, UItemBase*, Item);
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
class INVENTORYSYSTEMINVENZAPLUGIN_API UUInventoryWidgetBase : public UBaseUserWidget
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
	FOnItemReplaceDelegate OnItemReplaceDelegate;
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

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Inventory")
	FInventoryData InventoryData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
	FInventorySettings InventorySettings;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category="Inventory")
	FUISettings UISettings;

	//
	UPROPERTY()
	TObjectPtr<UInventorySlot> SlotUnderMouse = nullptr;

	//
	UPROPERTY()
	TSet<EItemCategory> ActiveFilters;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================	
	virtual FItemMapping* GetItemMapping(UItemBase* Item);
	virtual int32 CalculateActualAmountToAdd(int32 InAmountToAdd, float ItemSingleWeight);

	// Filters
	UFUNCTION()
	virtual void ClearFilters() PURE_VIRTUAL(UUInventoryWidgetBase::ClearFilters,);
	UFUNCTION()
	virtual void OnFilterStatusChanged(UUIButton* ItemCategoryButton) PURE_VIRTUAL(UUInventoryWidgetBase::OnFilterStatusChanged,);
	UFUNCTION()
	virtual void RefreshFilteredItemsList() PURE_VIRTUAL(UUInventoryWidgetBase::RefreshFilteredItemsList,);
	UFUNCTION()
	virtual void SearchTextChanged(const FText& NewText) PURE_VIRTUAL(UUInventoryWidgetBase::SearchTextChanged,);

	//
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

public:
	UFUNCTION()
	virtual void UpdateWeightInfo();
	UFUNCTION()
	virtual void UpdateMoneyInfo();
	
protected:
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
	virtual void NotifyReplaceItem(TArray<FInventorySlotData> OldItemSlots, TArray<FInventorySlotData> NewItemSlots, UItemBase* Item);
	
};

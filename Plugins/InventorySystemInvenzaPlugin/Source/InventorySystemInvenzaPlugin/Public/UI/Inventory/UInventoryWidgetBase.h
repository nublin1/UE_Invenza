//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"
#include "Settings/InvnzaSettings.h"
#include "UI/BaseUserWidget.h"
#include "UI/Container/InvBaseContainerWidget.h"
#include "UInventoryWidgetBase.generated.h"

class UUIButton;
enum class EItemCategory : uint8;
class UItemCollection;
class UInventoryBase;

/**
 * 
 */
UCLASS(Abstract)
class INVENTORYSYSTEMINVENZAPLUGIN_API UUInventoryWidgetBase : public UBaseUserWidget
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDropped, FItemMoveData, ItemMoveData);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAddItemDelegate, FItemMapping, ItemSlots, UItemBase*, Item);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPreRemoveItemDelegate, FItemMapping, ItemSlots, UItemBase*, Item, int32, RemoveQuantity);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPostRemoveItemDelegate);
#pragma endregion Delegates
	
public:
	UUInventoryWidgetBase();
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Delegates
	UPROPERTY(BlueprintAssignable)
	FOnItemDropped OnItemDroppedDelegate;

	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag TargetInventoryTag;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(Category="Inventory")
	virtual void InitializeInventoryWidget() PURE_VIRTUAL(UUInventoryWidgetBase::InitializeInventory,);
	UFUNCTION(Category="Inventory")
	virtual void ReDrawAllItems() PURE_VIRTUAL(UUInventoryWidgetBase::ReDrawAllItems,);
	
	UFUNCTION()
	virtual bool HandleTradeModalOpening(UItemBase* Item);
	UFUNCTION(Category="Inventory")
	virtual void HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity) PURE_VIRTUAL(UUInventoryWidgetBase::HandleRemoveItem,);
	UFUNCTION(Category="Inventory")
	virtual void HandleRemoveItemFromContainer(UItemBase* Item) PURE_VIRTUAL(UUInventoryWidgetBase::HandleRemoveItemFromContainer,);
	UFUNCTION(BlueprintCallable, Category="Inventory")
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) PURE_VIRTUAL(UUInventoryWidgetBase::HandleAddItem, return FItemAddResult(););

	FUISettings GetUISettings() const {return UISettings;}
	
	//Setters
	void SetInventoryBaseRef(UInventoryBase* NewInventoryRef) {InventoryRef = NewInventoryRef;}
	FORCEINLINE virtual void SetUISettings(FUISettings NewSettings) {UISettings = NewSettings;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category="Inventory|Settings")
	FUISettings UISettings;

	// Refs
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="InventoryWidget")
	TObjectPtr<UInventoryBase> InventoryRef;

	// Data
	UPROPERTY()
	TSet<EItemCategory> ActiveFilters;
	UPROPERTY()
	TObjectPtr<UInventorySlot> SlotUnderMouse = nullptr;
	
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
	
};

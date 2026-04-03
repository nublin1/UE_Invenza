//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Inventory/InventoryTypes.h"
#include "Settings/InvnzaSettings.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Inventory/Container/InvBaseContainerWidget.h"
#include "UInventoryWidgetBase.generated.h"

class UUIButton;
enum class EItemCategory : uint8;
class UItemCollection;
class UInventoryBase;

/**
 * 
 */
UCLASS(Abstract)
class INVENTORYSYSTEMINVENZAPLUGIN_API UUInventoryWidgetBase : public UInvenzaBaseWidget
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDropped, FItemMoveData, ItemMoveData);
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Settings")
	FGameplayTag TargetInventoryTag;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(Category="Inventory")
	virtual void InitializeInventoryWidget(){} /*PURE_VIRTUAL(UUInventoryWidgetBase::InitializeInventory,)*/;
	UFUNCTION(Category="Inventory")
	virtual void InitializeInventoryWidgetWithSettings(FInventoryStartupData InventoryStartupData){}

	UFUNCTION(Category="Inventory")
	virtual void BindDelegated() PURE_VIRTUAL(UUInventoryWidgetBase::BindDelegated,);
	
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
	UItemTooltipWidget* GetItemTooltipWidget() const {return ItemTooltipWidget;}
	virtual UInventoryBase* GetInventoryRef() const {return InventoryRef;}
	
	//Setters
	virtual void SetInventoryBaseRef(UInventoryBase* NewInventoryRef) {InventoryRef = NewInventoryRef;}
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
	UPROPERTY()
	TObjectPtr<UItemTooltipWidget> ItemTooltipWidget;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	virtual void AddItemToPanel(FItemMapping& ItemSlots, UItemBase* Item) PURE_VIRTUAL(UUInventoryWidgetBase::AddItemToPanel,);
	UFUNCTION()
	virtual void RemoveItemFromPanel(FItemMapping FromSlots, UItemBase* Item)PURE_VIRTUAL(UUInventoryWidgetBase::RemoveItemFromPanel,);
	UFUNCTION()
	virtual void UpdateItem(UItemBase* Item, int32 ChangedAmount) PURE_VIRTUAL(UUInventoryWidgetBase::RemoveItemFromPanel,);

	// Filters
	UFUNCTION()
	virtual void ClearFilters() PURE_VIRTUAL(UUInventoryWidgetBase::ClearFilters,);
	UFUNCTION()
	virtual void OnFilterStatusChanged(UUIButton* ItemCategoryButton) PURE_VIRTUAL(UUInventoryWidgetBase::OnFilterStatusChanged,);
	UFUNCTION()
	virtual void RefreshFilteredItemsList() PURE_VIRTUAL(UUInventoryWidgetBase::RefreshFilteredItemsList,);
	UFUNCTION()
	virtual void SearchTextChanged(const FText& NewText) PURE_VIRTUAL(UUInventoryWidgetBase::SearchTextChanged,);

public:
	UFUNCTION()
	virtual void UpdateWeightInfo(float InventoryTotalWeight) PURE_VIRTUAL(UUInventoryWidgetBase::UpdateWeightInfo,);
	UFUNCTION()
	virtual void UpdateMoneyInfo(int32 InventoryTotalMoney) PURE_VIRTUAL(UUInventoryWidgetBase::UpdateMoneyInfo,);
	
protected:
	//
	UFUNCTION()
	virtual UInvBaseContainerWidget* GetAsContainerWidget() { return Cast<UInvBaseContainerWidget>(ParentWidget);}
};

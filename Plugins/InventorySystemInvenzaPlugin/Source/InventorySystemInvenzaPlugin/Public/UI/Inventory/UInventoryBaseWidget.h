//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Inventory/InventoryTypes.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Data/Trade/TradeTypes.h"
#include "Settings/InvenzaSettings.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"
#include "UInventoryBaseWidget.generated.h"

struct FTradeData;
class UUIButton;
class UItemCollection;
class UInventoryBase;

/**
 * 
 */
UCLASS(Abstract)
class INVENTORYSYSTEMINVENZAPLUGIN_API UUInventoryBaseWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDropped, FItemMoveData, ItemMoveData);
#pragma endregion Delegates
	
public:
	UUInventoryBaseWidget();
	
protected:
	virtual void NativeOnInitialized() override;
	
public:
	
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
	virtual void InitializeInventoryWidget(){}
	UFUNCTION(Category="Inventory")
	virtual void InitializeInventoryWidgetWithSettings(){};

	UFUNCTION(Category="Inventory")
	virtual void BindDelegated() PURE_VIRTUAL(UUInventoryWidgetBase::BindDelegated,);

	UFUNCTION(Category="Inventory")
	virtual void CreateTooltipWidget();
	
	UFUNCTION(Category="Inventory")
	virtual void ReDrawAllItems() PURE_VIRTUAL(UUInventoryWidgetBase::ReDrawAllItems,);
	
	UFUNCTION(Category="Inventory")
	virtual void HandleRemoveItem(UObject* Item, int32 RemoveQuantity) PURE_VIRTUAL(UUInventoryWidgetBase::HandleRemoveItem,);
	UFUNCTION(Category="Inventory")
	virtual void HandleRemoveItemFromContainer(UObject* Item) PURE_VIRTUAL(UUInventoryWidgetBase::HandleRemoveItemFromContainer,);
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInvenzaInventorySettingsAsset> GlobalSettings;

	// Refs
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="InventoryWidget")
	TObjectPtr<UInventoryBase> InventoryRef;

	// Data
	UPROPERTY(BlueprintReadWrite)
	TSet<FGameplayTag> ActiveFilters;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UInventorySlot> SlotUnderMouse = nullptr;
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UItemTooltipWidget> ItemTooltipWidget;
	UPROPERTY(BlueprintReadWrite)
	FTradeContext TradeContext;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
public:
	UFUNCTION()
	virtual void AddItemToPanel(FItemMapping& ItemSlots, UObject* Item) PURE_VIRTUAL(UUInventoryWidgetBase::AddItemToPanel,);
	UFUNCTION()
	virtual void RemoveItemFromPanel(FItemMapping FromSlots, UObject* Item) PURE_VIRTUAL(UUInventoryWidgetBase::RemoveItemFromPanel,);
	UFUNCTION()
	virtual void ReplaceItemInPanel(TArray<UInventorySlotData*> OldItemSlots, FItemMapping& NewItemSlots, UObject* Item) {};
	UFUNCTION()
	virtual void UpdateItem(UObject* Item) PURE_VIRTUAL(UUInventoryWidgetBase::RemoveItemFromPanel,);

protected:
	UFUNCTION(BlueprintCallable)
	virtual void ApplyInventorySettings(){};
	
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
	virtual UInventoryContainerWidget* GetAsContainerWidget() { return Cast<UInventoryContainerWidget>(ParentWidget);}
};

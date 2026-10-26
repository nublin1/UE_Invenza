//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Inventory/InventoryTypes.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "Data/Trade/TradeTypes.h"
#include "Settings/InvenzaSettings.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Core/ItemFiltersPanel/FiltersPanel.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"
#include "UInventoryBaseWidget.generated.h"

struct FTradeData;
class UUIButton;
class UInventoryCollectionComponent;
class UInventoryBase;
class UIInventoryManager;

/**
 *
 */
UCLASS(Abstract)
class INVENTORYSYSTEMINVENZAPLUGIN_API UUInventoryBaseWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UUInventoryBaseWidget();

protected:
	// Unreal overrides
	virtual void NativePreConstruct() override;

	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDropped, FItemMoveData, ItemMoveData);
#pragma endregion Delegates

	// Properties and variables
	// Delegates
	UPROPERTY(Category = "Invenza|UI|Inventory", BlueprintAssignable)
	FOnItemDropped OnItemDroppedDelegate;

	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
	FGameplayTag TargetInventoryTag;

	// Functions
	UFUNCTION(Category = "Inventory")
	virtual void InitializeInventoryWidget() {}

	UFUNCTION(Category = "Inventory")
	virtual void InitializeInventoryWidgetWithSettings() {};

	UFUNCTION(Category = "Inventory")
	virtual void BindDelegated() PURE_VIRTUAL(UUInventoryWidgetBase::BindDelegated, );

	UFUNCTION(Category = "Inventory")
	virtual void CreateTooltipWidget();

	UFUNCTION(Category = "Inventory")
	virtual void ReDrawAllItems() PURE_VIRTUAL(UUInventoryWidgetBase::ReDrawAllItems, );

	UFUNCTION(Category = "Inventory")
	virtual void HandleRemoveItem(UObject* Item, int32 RemoveQuantity)
		PURE_VIRTUAL(UUInventoryWidgetBase::HandleRemoveItem, );

	UFUNCTION(Category = "Inventory")
	virtual void HandleRemoveItemFromContainer(UObject* Item)
		PURE_VIRTUAL(UUInventoryWidgetBase::HandleRemoveItemFromContainer, );

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false)
		PURE_VIRTUAL(UUInventoryWidgetBase::HandleAddItem, return FItemAddResult(););

	FUISettings GetUISettings() const { return UISettings; }

	UItemTooltipWidget* GetItemTooltipWidget() const { return ItemTooltipWidget; }

	virtual UInventoryBase* GetInventoryRef() const { return InventoryRef; }

	//Setters
	virtual void SetInventoryBaseRef(UInventoryBase* NewInventoryRef) { InventoryRef = NewInventoryRef; }

	FORCEINLINE virtual void SetUISettings(FUISettings NewSettings) { UISettings = NewSettings; }

	UFUNCTION()
	virtual void AddItemToPanel(FItemMapping& ItemSlots, UObject* Item)
		PURE_VIRTUAL(UUInventoryWidgetBase::AddItemToPanel, );

	UFUNCTION()
	virtual void RemoveItemFromPanel(FItemMapping FromSlots, UObject* Item)
		PURE_VIRTUAL(UUInventoryWidgetBase::RemoveItemFromPanel, );

	UFUNCTION()
	virtual void ReplaceItemInPanel(TArray<UInventorySlotData*> OldItemSlots, FItemMapping& NewItemSlots,
									UObject* Item) {};

	UFUNCTION()
	virtual void UpdateItem(UObject* Item) PURE_VIRTUAL(UUInventoryWidgetBase::RemoveItemFromPanel, );

	UFUNCTION()
	virtual void UpdateWeightInfo(float InventoryTotalWeight) PURE_VIRTUAL(UUInventoryWidgetBase::UpdateWeightInfo, );

	UFUNCTION()
	virtual void UpdateMoneyInfo(int32 InventoryTotalMoney) PURE_VIRTUAL(UUInventoryWidgetBase::UpdateMoneyInfo, );

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Transient)
	TObjectPtr<UIInventoryManager> TradeFilterManager;

	uint32 ObservedTradeSessionId = 0;

	// Widgets
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFiltersPanel> ItemFiltersPanel;

	UPROPERTY(Category = "Invenza|UI|Inventory", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInvenzaInventorySettingsAsset> GlobalSettings;

	// Config
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Config")
	bool bShowFilterPanel = true;

	// Refs
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "InventoryWidget")
	TObjectPtr<UInventoryBase> InventoryRef;

	// Data
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Inventory|Settings")
	FUISettings UISettings;

	UPROPERTY(Category = "Invenza|UI|Inventory", BlueprintReadWrite)
	TSet<FGameplayTag> ActiveFilters;

	UPROPERTY(Category = "Invenza|UI|Inventory", BlueprintReadOnly)
	TObjectPtr<UInventorySlot> SlotUnderMouse = nullptr;

	UPROPERTY(Category = "Invenza|UI|Inventory", BlueprintReadWrite)
	TObjectPtr<UItemTooltipWidget> ItemTooltipWidget;

	UPROPERTY(Category = "Invenza|UI|Inventory", BlueprintReadWrite)
	FTradeContext TradeContext;

	// Functions
	void ConfigureTradeFilter();

	bool PassesRecentSalesFilter(UObject* Item) const;

	bool PassesCategoryAndSearch(UObject* Item) const;

	bool HasActiveItemFilters() const;

	UFUNCTION()
	void HandleTradeSessionItemsChanged();

	UFUNCTION()
	void HandleRecentSalesClicked(UUIButton* Button);

	UFUNCTION(Category = "Invenza|UI|Inventory", BlueprintCallable)
	virtual void ApplyInventorySettings() {};

	// Filters
	UFUNCTION()
	virtual void ClearFilters() PURE_VIRTUAL(UUInventoryWidgetBase::ClearFilters, );

	UFUNCTION()
	virtual void OnFilterStatusChanged(UUIButton* ItemCategoryButton)
		PURE_VIRTUAL(UUInventoryWidgetBase::OnFilterStatusChanged, );

	UFUNCTION()
	virtual void RefreshFilteredItemsList() PURE_VIRTUAL(UUInventoryWidgetBase::RefreshFilteredItemsList, );

	UFUNCTION()
	virtual void SearchTextChanged(const FText& NewText) PURE_VIRTUAL(UUInventoryWidgetBase::SearchTextChanged, );

	//
	UFUNCTION()
	virtual UInventoryContainerWidget* GetAsContainerWidget() { return Cast<UInventoryContainerWidget>(ParentWidget); }
};

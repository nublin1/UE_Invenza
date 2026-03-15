//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UInventoryWidgetBase.h"
#include "UI/BaseUserWidget.h"
#include "ListInventoryWidget.generated.h"

enum class EItemCategory : uint8;
class UItemCategoryButton;
class UUIButton;
class UItemFiltersPanel;
class UEditableText;
class UInventoryListEntry;
class UListView;
class UScrollBox;

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UListInventoryWidget : public UUInventoryWidgetBase
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UListInventoryWidget();

	virtual void InitializeInventoryWidget() override;
	virtual void SortInventory();
	virtual void BindDelegated() override;
	virtual void ReDrawAllItems() override;


protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|UI", meta=(BindWidgetOptional))
	TObjectPtr<UItemFiltersPanel> ItemFiltersPanel;
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|UI", meta=(BindWidgetOptional))
	TObjectPtr<UScrollBox> ScrollBox;
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|UI", meta=(BindWidget))
	TObjectPtr<UListView> ItemsList;

	//
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Data")
	TArray<TObjectPtr<UInventoryListEntry>> InvSlotsArray;
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Data")
	TArray<TObjectPtr<UInventoryListEntry>> FiltredInvSlotsArray;
	

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeConstruct() override;

	//
	UFUNCTION()
	virtual void ClearFilters() override;
	UFUNCTION()
	virtual void OnFilterStatusChanged(UUIButton* ItemCategoryButton) override;
	UFUNCTION()
	virtual void RefreshFilteredItemsList() override;
	UFUNCTION()
	virtual void SearchTextChanged(const FText& NewText) override;

	//
	UFUNCTION()
	virtual void AddItemToPanel(FItemMapping ItemSlots, UItemBase* Item) override;
	UFUNCTION()
	virtual void RemoveItemFromPanel(FItemMapping FromSlots, UItemBase* Item) override;
	UFUNCTION()
	virtual void UpdateItem(UItemBase* Item, int32 ChangedAmount) override;

	UFUNCTION()
	virtual void UpdateWeightInfo(float InventoryTotalWeight) override;
	UFUNCTION()
	virtual void UpdateMoneyInfo(int32 InventoryTotalMoney) override;

	//
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};

//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UInventoryBaseWidget.h"
#include "UI/InvenzaBaseWidget.h"
#include "ListInventoryWidget.generated.h"

class UListInventory;
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
class INVENTORYSYSTEMINVENZAPLUGIN_API UListInventoryWidget : public UUInventoryBaseWidget
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
	virtual void InitializeInventoryWidgetWithSettings(FInventorySettings InventoryStartupData) override;
	virtual void BindDelegated() override;
	virtual void ReDrawAllItems() override;

	virtual void SetInventoryBaseRef(UInventoryBase* NewInventoryRef) override;

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

	// Refs
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UListInventory> ListInventoryRef;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeConstruct() override;

	//
	virtual void ClearFilters() override;
	virtual void OnFilterStatusChanged(UUIButton* ItemCategoryButton) override;
	UFUNCTION()
	void RebuildFilteredSlots();
	
	virtual void RefreshFilteredItemsList() override;
	virtual void SearchTextChanged(const FText& NewText) override;

	//
	virtual void AddItemToPanel(FItemMapping& ItemSlots, UItemBase* Item) override;
	virtual void RemoveItemFromPanel(FItemMapping FromSlots, UItemBase* Item) override;
	virtual void UpdateItem(UItemBase* Item, int32 ChangedAmount) override;
	
	virtual void UpdateWeightInfo(float InventoryTotalWeight) override;
	virtual void UpdateMoneyInfo(int32 InventoryTotalMoney) override;

	//
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};

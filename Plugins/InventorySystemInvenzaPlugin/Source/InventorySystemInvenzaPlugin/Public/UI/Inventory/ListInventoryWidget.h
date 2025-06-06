//  Nublin Studio 2025 All Rights Reserved.

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

	virtual void InitializeInventory() override;
	virtual void SortInventory();
	virtual void ReDrawAllItems() override;

	virtual void HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity) override;	
	virtual void HandleRemoveItemFromContainer(UItemBase* Item) override;	
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|UI", meta=(BindWidgetOptional))
	TObjectPtr<UItemFiltersPanel> ItemFiltersPanel;
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|UI", meta=(BindWidgetOptional))
	TObjectPtr<UScrollBox> ScrollBox;
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|UI", meta=(BindWidgetOptional))
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
	virtual void ClearFilters() override;
	virtual void OnFilterStatusChanged(UUIButton* ItemCategoryButton) override;
	virtual void RefreshFilteredItemsList() override;
	virtual void SearchTextChanged(const FText& NewText) override;

	//
	virtual FItemAddResult HandleNonStackableItems(FItemMoveData& ItemMoveData, bool bOnlyCheck = false) override;
	virtual int32 HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount,
												bool bOnlyCheck) override;
	virtual FItemAddResult HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck) override;
	virtual void AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount) override;
	virtual void InsertToStackItem(UItemBase* Item, int32 AddQuantity) override;

	virtual void AddItemToPanel(UItemBase* Item) override;

	//
	virtual void NotifyAddItem(FItemMapping& FromSlots, UItemBase* NewItem, int32 ChangeQuantity) override;
	virtual void NotifyPreRemoveItem(FItemMapping& FromSlots, UItemBase* RemovedItem, int32 RemoveQuantity) override;
	
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};

//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"
#include "UInventoryWidgetBase.h"
#include "Components/ActorComponent.h"
#include "Settings/Settings.h"
#include "UI/BaseUserWidget.h"
#include "SlotbasedInventoryWidget.generated.h"

class UItemFiltersPanel;
enum class EItemCategory : uint8;
class UUIButton;
class UButton;
class UItemCollection;
class UScrollBox;
class UTextBlock;
class UCanvasPanel;
class UUniformGridPanel;
class USlotbasedInventorySlot;
class UHighlightSlotWidget;

UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API USlotbasedInventoryWidget : public UUInventoryWidgetBase
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	USlotbasedInventoryWidget();

	virtual void InitializeInventory() override;	
	virtual void ReDrawAllItems() override;
	virtual void RebuildSlots(int32 InRows, int32 InColumns);
	
	virtual void HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity) override;	
	virtual void HandleRemoveItemFromContainer(UItemBase* Item) override;	
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;

	virtual TObjectPtr<UInventorySlot> GetAvailableSlotForItem(UItemBase* Item);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UItemFiltersPanel> ItemFiltersPanel;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UScrollBox> ScrollBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UUniformGridPanel> SlotsGridPanel;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCanvasPanel> ItemsVisualsPanel;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCanvasPanel> HighlightVisualsPanel;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> Button_TakeAll;

	// Data
	int NumberRows = 0;
	int NumColumns = 0;
	FMargin SlotSpacing;
	
	// Highlight
	TObjectPtr<UHighlightSlotWidget> HighlightWidgetPreview = nullptr;

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	bool bHasSlotSpacing = false;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	virtual void InitSlots();

	//
	virtual void ClearFilters() override;
	virtual void OnFilterStatusChanged(UUIButton* ItemCategoryButton) override;
	virtual void RefreshFilteredItemsList() override;
	virtual void SearchTextChanged(const FText& NewText) override;
	
	//
	virtual UInventorySlot* GetSlotByPosition(FIntVector2 SlotPosition);
	virtual bool bIsSlotEmpty(const FIntVector2 SlotPosition);
	virtual bool bIsSlotEmpty(const UInventorySlot* SlotCheck);
	virtual bool bIsGridPositionValid(FIntPoint& GridPosition);
	
	virtual FItemAddResult HandleNonStackableItems(FItemMoveData& ItemMoveData, bool bOnlyCheck = false) override;
	UFUNCTION()
	virtual FItemAddResult TryAddStackableItem(FItemMoveData& ItemMoveData, bool bOnlyCheck);
	
	virtual int32 HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount,
												bool bOnlyCheck) override;
	virtual FItemAddResult HandleAddReferenceItem(FItemMoveData& ItemMoveData, bool bOnlyCheck) override;
	UFUNCTION()
	virtual FItemAddResult HandleSwapOrAddItems(FItemMoveData& ItemMoveData,bool bOnlyCheck );
	
	virtual void AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots, int32 AddAmount) override;
	UFUNCTION()
	virtual void ReplaceItem(UItemBase* Item, UInventorySlot* NewSlot);

	UFUNCTION()
	FVector2D CalculateItemVisualPosition(FIntVector2 SlotPosition) const;
	
	virtual void AddItemToPanel(UItemBase* Item) override;
	virtual void ReplaceItemInPanel(FItemMapping& FromSlots, UItemBase* Item);	
	virtual void UpdateSlotInPanel(FItemMapping FromSlots, UItemBase* Item);
	virtual void RemoveItemFromPanel(FItemMapping* FromSlots, UItemBase* Item);
	
	virtual void NotifyAddItem(FItemMapping& FromSlots, UItemBase* NewItem, int32 ChangeQuantity) override;
	virtual void NotifyPreRemoveItem(FItemMapping& FromSlots, UItemBase* RemovedItem, int32 RemoveQuantity) override;
	//void NotifyUseSlot(UBaseInventorySlot* FromSlot);

	UFUNCTION()
	virtual void CreateHighlightWidget();
	UFUNCTION()
	virtual void CreateTooltipWidget();
	UFUNCTION()
	virtual FIntPoint CalculateGridPosition(const FGeometry& Geometry, const FVector2D& ScreenCursorPos) const;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
};

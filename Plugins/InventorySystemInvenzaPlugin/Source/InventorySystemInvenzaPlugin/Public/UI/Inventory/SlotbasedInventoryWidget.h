//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Inventory/InventoryTypes.h"
#include "UInventoryBaseWidget.h"
#include "Components/ActorComponent.h"
#include "Settings/InvenzaSettings.h"
#include "UI/InvenzaBaseWidget.h"
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
class INVENTORYSYSTEMINVENZAPLUGIN_API USlotbasedInventoryWidget : public UUInventoryBaseWidget
{
	GENERATED_BODY()

public:
	USlotbasedInventoryWidget();

protected:
	virtual void NativeConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void InitializeInventoryWidget() override;
	virtual void InitializeInventoryWidgetWithSettings(FInventorySettings InventoryStartupData) override;
	virtual void BindDelegated() override;
	virtual void ReDrawAllItems() override;

	virtual FIntPoint GetNumberRowsAndColumns() {return FIntPoint(NumberRows, NumColumns);}
	virtual TArray<UInventorySlotData*> GetSlotData();

	virtual void SetInventoryBaseRef(UInventoryBase* NewInventoryRef) override;

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

	UPROPERTY()
	TObjectPtr<UHighlightSlotWidget> HighlightWidgetPreview = nullptr;
	
	// Refs
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USlotbasedInventory> SlotBasedInventoryRef;

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	bool bHasSlotSpacing = false;
	
	/** Default image used for slot background when not overridden by individual slots. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TObjectPtr<UTexture2D> DefaultCellImage = nullptr;
	/**
	 *  If true, the background image of the slot will be hidden when it contains an item.
	 *  * When enabled, the OccupiedCellImage will not be used.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory", meta = (
		ToolTip = "If true, hides the slot background image when an item is present. Disables OccupiedCellImage."))
	bool bHideBackgroundWhenOccupied = false;
	/**
	 *  Image displayed when this slot contains an item.
	 *  Overrides the default background image when the slot is not empty.
	 */
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory", meta = (
		EditCondition = "!bHideBackgroundWhenOccupied",
		ToolTip = "Image to display when this slot contains an item. Used instead of the default cell image."))
	TObjectPtr<UTexture2D> OccupiedCellImage;

	// Data
	UPROPERTY()
	int NumberRows = 0;
	UPROPERTY()
	int NumColumns = 0;
	UPROPERTY()
	FMargin SlotSpacing;
	UPROPERTY()
	FVector2D InvCellSize = FVector2D(64.0f, 64.0f);

	UPROPERTY()
	TArray<TObjectPtr<UInventorySlot>> InventorySlots;

	UPROPERTY()
	bool bIsNeedToReDrawItems = false;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================	
	UFUNCTION()
	virtual void InitSlots();

	//
	virtual void ClearFilters() override;
	virtual void OnFilterStatusChanged(UUIButton* ItemCategoryButton) override;
	virtual void RefreshFilteredItemsList() override;
	virtual void SearchTextChanged(const FText& NewText) override;

	//
	UFUNCTION(BlueprintCallable)
	virtual void ResetItemVisual(UItemBase* ItemToReset);

	//
	virtual UInventorySlot* GetSlotByPosition(FIntPoint SlotPosition);
	virtual bool bIsGridPositionValid(FIntPoint& GridPosition);

	UFUNCTION()
	FVector2D CalculateItemVisualPosition(FIntPoint SlotPosition) const;

public:
	virtual void AddItemToPanel(FItemMapping& ItemSlots, UItemBase* Item) override;
	virtual void ReplaceItemInPanel(TArray<UInventorySlotData*> OldItemSlots, FItemMapping& NewItemSlots, UItemBase* Item) override;
	virtual void UpdateItem(UItemBase* Item) override;
	virtual void UpdateSlotInPanel(FItemMapping FromSlots, UItemBase* Item);
	virtual void RemoveItemFromPanel(FItemMapping FromSlots, UItemBase* Item) override;
	
protected:
	UFUNCTION()
	virtual void UsedItemInPanel(UInventorySlotData* UsedSlot);
	
	virtual void UpdateWeightInfo(float InventoryTotalWeight) override;
	virtual void UpdateMoneyInfo(int32 InventoryTotalMoney) override;

	UFUNCTION(BlueprintCallable)
	virtual void UpdateTradeContext() ;

	UFUNCTION()
	virtual void CreateHighlightWidget();
	
	UFUNCTION()
	virtual FIntPoint CalculateGridPosition(const FGeometry& Geometry, const FVector2D& ScreenCursorPos) const;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent,UDragDropOperation* InOperation) override;
};

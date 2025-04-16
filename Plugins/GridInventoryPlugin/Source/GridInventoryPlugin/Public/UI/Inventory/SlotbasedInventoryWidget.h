//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"
#include "UInventoryWidgetBase.h"
#include "Components/ActorComponent.h"
#include "Settings/Settings.h"
#include "UI/BaseUserWidget.h"
#include "SlotbasedInventoryWidget.generated.h"

class UButton;
class UItemCollection;
class UScrollBox;
class UTextBlock;
class UCanvasPanel;
class UUniformGridPanel;
class USlotbasedInventorySlot;
class UHighlightSlotWidget;

UCLASS()
class GRIDINVENTORYPLUGIN_API USlotbasedInventoryWidget : public UUInventoryWidgetBase
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
	
	virtual void HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity) override;	
	virtual void HandleRemoveItemFromContainer(UItemBase* Item) override;	
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;

	//
	UFUNCTION(Category="Inventory")
	FORCEINLINE float GetInventoryTotalWeight() const {return InventoryTotalWeight;}
	UFUNCTION(Category="Inventory")
	FORCEINLINE bool GetIsUseReference() const {return bUseReferences;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UScrollBox> ScrollBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UUniformGridPanel> SlotsGridPanel;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> ItemsVisualsPanel;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCanvasPanel> HighlightVisualsPanel;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> Button_TakeAll;

	// Data
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<USlotbasedInventorySlot>> InventorySlots;
	int NumberOfRows = 0;
	int NumberOfColumns = 0;
	
	// DragDrop
	TObjectPtr<USlotbasedInventorySlot> SelectedSlot = nullptr;
	TObjectPtr<UHighlightSlotWidget> HighlightWidgetPreview = nullptr;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	virtual void InitSlots();
	UFUNCTION()
	virtual void HandleVisibilityChanged(ESlateVisibility NewVisibility);
	
	virtual USlotbasedInventorySlot* GetSlotByPosition(FIntVector2 SlotPosition);
	virtual bool bIsSlotEmpty(const FIntVector2 SlotPosition);
	virtual bool bIsSlotEmpty(const UInventorySlot* SlotCheck);
	
	virtual FItemAddResult HandleNonStackableItems(FItemMoveData& ItemMoveData, bool bOnlyCheck = false) override;
	UFUNCTION()
	virtual FItemAddResult TryAddStackableItem(FItemMoveData& ItemMoveData, bool bOnlyCheck);
	
	virtual int32 HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount,
												bool bOnlyCheck) override;
	virtual FItemAddResult HandleAddReferenceItem(FItemMoveData& ItemMoveData) override;
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
	virtual void NotifyRemoveItem(FItemMapping& FromSlots, UItemBase* RemovedItem, int32 RemoveQuantity) override;
	//void NotifyUseSlot(UBaseInventorySlot* FromSlot);

	UFUNCTION()
	virtual void CreateHighlightWidget();
	UFUNCTION()
	virtual FIntPoint CalculateGridPosition(const FGeometry& Geometry, const FVector2D& ScreenCursorPos) const;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
};

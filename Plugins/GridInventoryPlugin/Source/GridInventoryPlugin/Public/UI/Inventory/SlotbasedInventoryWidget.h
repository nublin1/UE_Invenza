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
	
	UFUNCTION(Category="Inventory")
	void InitializeInventory();
	UFUNCTION(Category="Inventory")
	virtual void ReDrawAllItems();

	
	virtual void HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity) override;	
	virtual void HandleRemoveItemFromContainer(UItemBase* Item) override;	
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;

	//
	UFUNCTION(Category="Inventory")
	FORCEINLINE float GetWeightCapacity() const {return InventoryWeightCapacity;}
	UFUNCTION(Category="Inventory")
	FORCEINLINE float GetInventoryTotalWeight() const {return InventoryTotalWeight;}
	UFUNCTION(Category="Inventory")
	FORCEINLINE bool GetIsUseReference() const {return bUseReferences;}

	//Setters
	FORCEINLINE void SetItemCollection(UItemCollection* _ItemCollection) {ItemCollectionLink = _ItemCollection;}
	virtual void SetUISettings(FUISettings NewSettings) {UISettings = NewSettings;}

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	float InventoryTotalWeight = 0;
	int NumberOfRows = 0;
	int NumberOfColumns = 0;

	//Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	float InventoryWeightCapacity;
	UPROPERTY()
	FUISettings UISettings;
	

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
	
	virtual FItemMapping* GetItemMapping(UItemBase* Item);
	virtual USlotbasedInventorySlot* GetSlotByPosition(FIntVector2 SlotPosition);
	virtual bool bIsSlotEmpty(const FIntVector2 SlotPosition);
	virtual bool bIsSlotEmpty(const USlotbasedInventorySlot* SlotCheck);

	UFUNCTION()
	virtual FItemAddResult HandleNonStackableItems(FItemMoveData& ItemMoveData, bool bOnlyCheck = false);
	UFUNCTION()
	virtual FItemAddResult TryAddStackableItem(FItemMoveData& ItemMoveData, bool bOnlyCheck);
	UFUNCTION()
	virtual int32 HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount,
												bool bOnlyCheck);
	UFUNCTION()
	virtual FItemAddResult HandleAddReferenceItem(FItemMoveData& ItemMoveData);
	UFUNCTION()
	virtual FItemAddResult HandleSwapOrAddItems(FItemMoveData& ItemMoveData,bool bOnlyCheck );
	UFUNCTION()
	virtual void AddNewItem(FItemMoveData& ItemMoveData, FItemMapping OccupiedSlots);
	UFUNCTION()
	virtual void ReplaceItem(UItemBase* Item, USlotbasedInventorySlot* NewSlot);
	UFUNCTION()
	virtual void InsertToStackItem(UItemBase* Item, int32 AddQuantity);

	UFUNCTION()
	FVector2D CalculateItemVisualPosition(FIntVector2 SlotPosition) const;
	
	virtual void AddItemToPanel(UItemBase* Item);
	virtual void ReplaceItemInPanel(FItemMapping& FromSlots, UItemBase* Item);	
	virtual void UpdateSlotInPanel(FItemMapping* FromSlots, UItemBase* Item);
	virtual void RemoveItemFromPanel(FItemMapping* FromSlots, UItemBase* Item);

	UFUNCTION()
	virtual void UpdateWeightInfo();
	
	void NotifyAddItem(FItemMapping& FromSlots, UItemBase* NewItem);
	void NotifyUpdateItem(FItemMapping* FromSlots, UItemBase* NewItem);
	void NotifyRemoveItem(FItemMapping& FromSlots, UItemBase* RemovedItem);
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

//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"
#include "Components/ActorComponent.h"
#include "Settings/Settings.h"
#include "UI/BaseUserWidget.h"
#include "BaseInventoryWidget.generated.h"

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUpdateDelegate, const FItemMapping&, ItemSlots, UItemBase*,
                                             Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAddItemDelegate, FItemMapping, ItemSlots, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRemoveItemDelegate, FItemMapping, ItemSlots, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUseItemDelegate, UBaseInventorySlot*, ItemSlot, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWightUpdatedDelegate, float, InventoryTotalWeight, float, InventoryWeightCapacity);
#pragma endregion Delegates

class UButton;
class UItemCollection;
class UScrollBox;
class UTextBlock;
class UCanvasPanel;
class UUniformGridPanel;
class UBaseInventorySlot;

UCLASS()
class GRIDINVENTORYPLUGIN_API UBaseInventoryWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	FOnItemUpdateDelegate OnItemUpdateDelegate;
	FOnAddItemDelegate OnAddItemDelegate;	
	FOnRemoveItemDelegate OnRemoveItemDelegate;
	FOnUseItemDelegate OnUseItemDelegate;
	FOnWightUpdatedDelegate OnWightUpdatedDelegate;

	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UBaseInventoryWidget();
	
	UFUNCTION(Category="Inventory")
	void InitializeInventory();
	UFUNCTION(Category="Inventory")
	virtual void ReDrawAllItems();

	UFUNCTION(Category="Inventory")
	virtual void HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity);
	UFUNCTION(Category="Inventory")
	virtual void HandleRemoveItemFromContainer(UItemBase* Item);
	UFUNCTION(Category="Inventory")
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false);

	//
	UFUNCTION(Category="Inventory")
	FORCEINLINE UItemCollection* GetItemCollection() {return ItemCollectionLink;}
	UFUNCTION(Category="Inventory")
	FORCEINLINE float GetWeightCapacity() const {return InventoryWeightCapacity;}
	UFUNCTION(Category="Inventory")
	FORCEINLINE float GetInventoryTotalWeight() const {return InventoryTotalWeight;}
	UFUNCTION(Category="Inventory")
	FORCEINLINE bool GetIsUseReference() const {return bUseReferences;}
	UFUNCTION(Category="Inventory")
	FORCEINLINE bool GetCanReferenceItems() const {return bCanReferenceItems;}

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
	UPROPERTY()
	TObjectPtr<UItemCollection> ItemCollectionLink;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UBaseInventorySlot>> InventorySlots;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	float InventoryTotalWeight = 0;
	int NumberOfRows = 0;
	int NumberOfColumns = 0;

	//Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	float InventoryWeightCapacity;
	UPROPERTY()
	FUISettings UISettings;
	UPROPERTY(EditAnywhere, meta=(tooltip="If true this container will be used as reference."))
	bool bUseReferences = false;
	UPROPERTY(EditAnywhere, meta=(tooltip="Can the items be referenced from this container"))
	bool bCanReferenceItems = false;

	// DragDrop
	TObjectPtr<UBaseInventorySlot> SelectedSlot = nullptr;
	TObjectPtr<UInventoryItemWidget> HighlightWidgetPreview = nullptr;
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	virtual void InitSlots();
	UFUNCTION()
	virtual void HandleVisibilityChanged(ESlateVisibility NewVisibility);
	
	virtual FItemMapping* GetItemMapping(UItemBase* Item);
	virtual UBaseInventorySlot* GetSlotByPosition(FIntVector2 SlotPosition);
	virtual bool bIsSlotEmpty(const FIntVector2 SlotPosition);
	virtual bool bIsSlotEmpty(const UBaseInventorySlot* SlotCheck);

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
	virtual void ReplaceItem(UItemBase* Item, UBaseInventorySlot* NewSlot);
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

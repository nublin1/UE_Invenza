// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"
#include "Components/ActorComponent.h"
#include "Settings/Settings.h"
#include "UI/BaseUserWidget.h"
#include "BaseInventoryWidget.generated.h"

#pragma region Delegates
class UScrollBox;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUpdateDelegate, const FItemSlotMapping&, ItemSlots, UItemBase*,
                                             Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAddItemDelegate, FItemSlotMapping, ItemSlots, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRemoveItemDelegate, FItemSlotMapping, ItemSlots, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUseItemDelegate, UBaseInventorySlot*, ItemSlot, UItemBase*, Item);
#pragma endregion Delegates

class UTextBlock;
class UCanvasPanel;
class UInventoryItemWidget;
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

	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UBaseInventoryWidget();

	UFUNCTION(Category="Inventory")
	virtual void HandleRemoveItem(UItemBase* Item);
	UFUNCTION(Category="Inventory")
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false);

	//
	UFUNCTION(Category="Inventory")
	FORCEINLINE float GetWeightCapacity() const {return InventoryWeightCapacity;}
	UFUNCTION(Category="Inventory")
	FORCEINLINE float GetInventoryTotalWeight() const {return InventoryTotalWeight;}

	//Setters
	virtual void SetUISettings(FUISettings NewSettings) {UISettings = NewSettings;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UScrollBox> ScrollBox;
	UPROPERTY(meta=(BindWidget))
	UUniformGridPanel* SlotsGridPanel;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> ItemsVisualsPanel;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> WeightInfo;

	//
	UPROPERTY()
	FUISettings UISettings;	

	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UBaseInventorySlot>> InventorySlots;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TMap<UItemBase*, FItemSlotMapping> InventoryItemsMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	float InventoryWeightCapacity;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	float InventoryTotalWeight = 0;

	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
	FVector2D SlotSize = FVector2D(0.f);

	// DragDrop
	TObjectPtr<UBaseInventorySlot> SelectedSlot = nullptr; 
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	virtual void NativeConstruct() override;

	UFUNCTION()
	virtual void InitSlots();

	
	virtual UUserWidget* GetItemLinkedWidget(UBaseInventorySlot* _ItemSlot);
	FItemSlotMapping GetAllSlotsFromInventoryItemsMap();
	virtual FItemSlotMapping* GetOccupiedSlots(UItemBase* Item);
	virtual UBaseInventorySlot* GetSlotByPosition(FIntVector2 SlotPosition);
	virtual TArray<UItemBase*> GetAllItems();
	virtual TArray<UItemBase*> GetAllSameItems(UItemBase* TestItem);
	virtual UItemBase* GetItemFromSlot(UBaseInventorySlot* TargetSlot );
	virtual bool bIsSlotEmpty(const FIntVector2 SlotPosition);
	virtual bool bIsSlotEmpty(const UBaseInventorySlot* SlotCheck);

	UFUNCTION()
	virtual FItemAddResult HandleNonStackableItems(FItemMoveData& ItemMoveData, bool bOnlyCheck = false);
	UFUNCTION()
	virtual int32 HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount,
												bool bOnlyCheck);
	UFUNCTION(Category="Inventory")
	virtual void HandleSwapItems(FItemMoveData& ItemMoveData);
	UFUNCTION()
	virtual void AddNewItem(FItemMoveData& ItemMoveData, FItemSlotMapping OccupiedSlots);
	UFUNCTION()
	virtual void ReplaceItem(UItemBase* Item, UBaseInventorySlot* NewSlot);

	UFUNCTION()
	FVector2D CalculateItemVisualPosition(FIntVector2 SlotPosition, FIntVector2 ItemSize) const;
	
	virtual void AddItemToPanel(FItemSlotMapping& FromSlots, UItemBase* Item);
	virtual void ReplaceItemInPanel(FItemSlotMapping& FromSlots, UItemBase* Item);	
	virtual void UpdateSlotInPanel(FItemSlotMapping* FromSlots, UItemBase* Item);

	UFUNCTION()
	virtual void UpdateWeightInfo();
	
	void NotifyAddItem(FItemSlotMapping& FromSlots, UItemBase* NewItem);
	void NotifyUpdateItem(FItemSlotMapping* FromSlots, UItemBase* NewItem);
	void NotifyRemoveItem(FItemSlotMapping FromSlots, UItemBase* RemovedItem) const;
	//void NotifyUseSlot(UBaseInventorySlot* FromSlot);

	UFUNCTION()
	virtual FIntPoint CalculateGridPosition(const FGeometry& Geometry, const FVector2D& ScreenCursorPos) const;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
};

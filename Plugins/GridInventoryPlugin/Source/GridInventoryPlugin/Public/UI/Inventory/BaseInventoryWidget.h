// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"
#include "Components/ActorComponent.h"
#include "UI/BaseUserWidget.h"
#include "BaseInventoryWidget.generated.h"

#pragma region Delegates

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUpdateDelegate, FArrayItemSlots, ItemSlots, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAddItemDelegate, FArrayItemSlots, ItemSlots, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRemoveItemDelegate, FArrayItemSlots, ItemSlots, UItemBase*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUseItemDelegate, UBaseInventorySlot*, ItemSlot, UItemBase*, Item);

#pragma endregion Delegates


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
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(meta=(BindWidget))
	UUniformGridPanel* SlotsGridPanel;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> ItemsVisualsPanel;

	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UBaseInventorySlot>> InventorySlots;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TMap<UItemBase*, FArrayItemSlots> InventoryItemsMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory || Settigs")
	float InventoryWeightCapacity;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	float InventoryTotalWeight = 0;

	//
	FVector2D SlotSize = FVector2D(0.f);
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	virtual void NativeConstruct() override;

	UFUNCTION()
	virtual void InitSlots();

	UFUNCTION()
	FArrayItemSlots GetAllSlotsFromInventoryItemsMap();
	
	virtual bool bIsSlotEmpty(const FIntVector2 SlotPosition);
	bool bIsSlotEmpty(const UBaseInventorySlot* SlotCheck);

	UFUNCTION()
	virtual FItemAddResult HandleNonStackableItems(FItemMoveData& ItemMoveData, bool bOnlyCheck = false);
	UFUNCTION()
	virtual int32 HandleStackableItems(FItemMoveData& ItemMoveData, int32 RequestedAddAmount,
												bool bOnlyCheck);
	UFUNCTION()
	virtual void AddNewItem(FItemMoveData& ItemMoveData, FArrayItemSlots OccupiedSlots);

	UFUNCTION()
	FVector2D CalculateItemVisualPosition(FIntVector2 SlotPosition, FIntVector2 ItemSize);
	UFUNCTION()
	virtual void AddItemToPanel(FArrayItemSlots FromSlots, UItemBase* Item);
	
	void NotifyAddItem(FArrayItemSlots FromSlots, UItemBase* NewItem);
	void NotifyUpdateItem(FArrayItemSlots FromSlots, UItemBase* NewItem);
	void NotifyRemoveItem(FArrayItemSlots FromSlots, UItemBase* RemovedItem) const;
	//void NotifyUseSlot(UBaseInventorySlot* FromSlot);
};

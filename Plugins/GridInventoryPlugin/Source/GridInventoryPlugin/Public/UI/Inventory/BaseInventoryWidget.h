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
	
	UPROPERTY()
	TArray<TObjectPtr<UBaseInventorySlot>> InventorySlots;
	UPROPERTY()
	float InventoryWeightCapacity;
	UPROPERTY(VisibleAnywhere, Category="Inventory")
	float InventoryTotalWeight = 0;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	virtual void NativeConstruct() override;

	UFUNCTION()
	virtual void InitSlots();
	
	UFUNCTION()
	virtual FItemAddResult HandleNonStackableItems(FItemMoveData& ItemMoveData, bool bOnlyCheck = false);
};

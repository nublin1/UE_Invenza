// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "InventoryItemWidget.generated.h"

class UItemBase;
class UTextBlock;
class UHorizontalBox;
class UCoreCellWidget;
class USizeBox;
class UBaseInventorySlot;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UInventoryItemWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UInventoryItemWidget();

	UFUNCTION()
	void UpdateVisual(UItemBase* Item);
	UFUNCTION()
	void UpdateVisualSize(FVector2D SlotSize, FIntVector2 ItemSize);
	UFUNCTION()
	void UpdateItemName(FText Name);
	UFUNCTION()
	void UpdateQuantityText(int Quantity);
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	TObjectPtr<UCoreCellWidget> CoreCellWidget;
	UPROPERTY(VisibleAnywhere, meta=(BindWidgetOptional))
	TObjectPtr<USizeBox> SizeBoxText;
	UPROPERTY(VisibleAnywhere, Category="Inventory Slot", meta=(BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HBoxName;
	UPROPERTY(VisibleAnywhere, Category="Inventory Slot", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemName;
	UPROPERTY(VisibleAnywhere, Category="Inventory Slot", meta=(BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HBoxQuantity;
	UPROPERTY(VisibleAnywhere, Category="Inventory Slot", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemQuantity;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};

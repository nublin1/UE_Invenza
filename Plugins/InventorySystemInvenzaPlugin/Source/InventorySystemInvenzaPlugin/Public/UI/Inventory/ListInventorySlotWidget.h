//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventorySlot.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Data/Inventory/ListInventory/InventoryListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "ListInventorySlotWidget.generated.h"

enum class EInventoryCheckType : uint8;
class UListInventoryWidget;
class UTextBlock;
class UScrollBox;

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UListInventorySlotWidget : public UInventorySlot, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
	UListInventorySlotWidget();

protected:
	virtual void NativeConstruct() override;

public:
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void UpdateVisualWithItemInfo(UObject* Item) override;

	UFUNCTION()
	virtual void UpdatePriceText();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> ItemIcon;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemName;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> MoneyIcon;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> PriceText;
	
	//
	UPROPERTY()
	TObjectPtr<UInventoryListEntry> CachedEntry;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	
};

//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventorySlot.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "ListInventorySlotWidget.generated.h"

enum class EInventoryCheckType : uint8;
class UListInventoryWidget;
class UTextBlock;
class UScrollBox;

UCLASS()
class GRIDINVENTORYPLUGIN_API UInventoryListEntry : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UItemBase> Item = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UListInventoryWidget> ParentInventoryWidget = nullptr;
};

/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UListInventorySlotWidget : public UInventorySlot, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void UpdateVisualWithItem(UItemBase* Item) override;

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
	TObjectPtr<UInventoryListEntry> CachedEntry = nullptr;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	
};

//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventorySlot.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "ListInventorySlotWidget.generated.h"

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
	virtual void UpdateVisual(UItemBase* Item) override;

	//
	void SetParentInventoryWidget(UListInventoryWidget* InventoryWidget){ParentInventoryWidget = InventoryWidget;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> ItemIcon;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemName;

	//
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UListInventoryWidget> ParentInventoryWidget;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UItemBase> LinkedItem;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
};

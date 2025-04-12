//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UInventoryWidgetBase.h"
#include "UI/BaseUserWidget.h"
#include "ListInventoryWidget.generated.h"

class UListView;
class UScrollBox;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UListInventoryWidget : public UUInventoryWidgetBase
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UListInventoryWidget();

	virtual void HandleRemoveItem(UItemBase* Item, int32 RemoveQuantity) override;	
	virtual void HandleRemoveItemFromContainer(UItemBase* Item) override;	
	virtual FItemAddResult HandleAddItem(FItemMoveData ItemMoveData, bool bOnlyCheck = false) override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UScrollBox> ScrollBox;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UListView> ItemsList;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeConstruct() override;

	void AddNewItem(FItemMoveData& ItemMoveData);

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};

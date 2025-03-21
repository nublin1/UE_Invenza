// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/InventoryItemWidget.h"

#include "ActorComponents/UIManagerComponent.h"
#include "ActorComponents/Items/itemBase.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "UI/Core/CoreCellWidget.h"

UInventoryItemWidget::UInventoryItemWidget()
{
}

void UInventoryItemWidget::UpdateVisual(UItemBase* Item)
{
	if (Item->GetItemRef().ItemAssetData.Icon)
	{
		CoreCellWidget->Content_Image->SetBrushFromTexture(Item->GetItemRef().ItemAssetData.Icon);
	}
}

void UInventoryItemWidget::UpdateVisualSize(FVector2D SlotSize, FIntVector2 ItemSize)
{
	CoreCellWidget->SizeBox->SetWidthOverride(SlotSize.X * ItemSize.X);
	CoreCellWidget->SizeBox->SetHeightOverride(SlotSize.Y * ItemSize.Y);
	
	if (!SizeBoxText)
		return;
	
	SizeBoxText->SetWidthOverride(SlotSize.X * ItemSize.X);
	SizeBoxText->SetHeightOverride(SlotSize.Y * ItemSize.Y);
}

void UInventoryItemWidget::UpdateItemName(FText Name)
{
	if (!ItemName)
		return;

	ItemName->SetText(Name);
}

void UInventoryItemWidget::UpdateQuantityText(int Quantity)
{
	if (!ItemQuantity)
		return;
	
	if (Quantity > 1)
		ItemQuantity->SetText(FText::AsNumber(Quantity));
	else
		ItemQuantity->SetVisibility(ESlateVisibility::Collapsed);
}

FReply UInventoryItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		DragOffset = ParentWidget->GetCachedGeometry().AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
		return Reply.Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return FReply::Unhandled();
}

void UInventoryItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	/*auto Manager = GetOwningPlayerPawn()->FindComponentByClass<UUIManagerComponent>();
	if (!Manager)
		return;

	UInventoryItemWidget* DraggedWidget = CreateWidget<UInventoryItemWidget>(GetOwningPlayer(),Manager->GetUISettings().DraggedWidgetClass);
	if (!DraggedWidget) return;

	
	//CreateWidget<UItemDragDropOperation>(this, UItemDragDropOperationClass);	
	UItemDragDropOperation* DragItemDragDropOperation = NewObject<UItemDragDropOperation>();
	DragItemDragDropOperation->DefaultDragVisual = DraggedWidget;
	DragItemDragDropOperation->Pivot = EDragPivot::MouseDown;

	//DragItemDragDropOperation->ItemMoveData.SourceItem =

	OutOperation = DragItemDragDropOperation;*/
}

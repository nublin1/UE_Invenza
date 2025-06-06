// Nublin Studio 2025 All Rights Reserved.


#include "UI/Item/InventoryItemWidget.h"


#include "ActorComponents/Items/itemBase.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "UI/Core/CoreCellWidget.h"

UInventoryItemWidget::UInventoryItemWidget()
{
}

void UInventoryItemWidget::UpdateVisual(UItemBase* Item)
{
	if (Item->GetItemRef().ItemAssetData.Icon)
	{
		CoreCellWidget->SetContentImage(Item->GetItemRef().ItemAssetData.Icon);
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
	{
		ItemQuantity->SetText(FText::AsNumber(Quantity));
		ItemQuantity->SetVisibility(ESlateVisibility::Visible);
	}
	else
		ItemQuantity->SetVisibility(ESlateVisibility::Collapsed);
}

void UInventoryItemWidget::ChangeBorderColor(FLinearColor NewColor) const
{
	CoreCellWidget->Left_Border->SetBrushColor(NewColor);
	CoreCellWidget->Right_Border->SetBrushColor(NewColor);
	CoreCellWidget->Top_Border->SetBrushColor(NewColor);
	CoreCellWidget->BottomBorder->SetBrushColor(NewColor);
}

void UInventoryItemWidget::ChangeOpacity(float NewValue)
{
	CoreCellWidget->Content_Image->SetOpacity(NewValue);
}

FReply UInventoryItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return Reply.Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return FReply::Unhandled();
}

void UInventoryItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
}

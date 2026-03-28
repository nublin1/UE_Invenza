//  Nublin Studio 2026 All Rights Reserved.

#include "DragDrop/ItemDragDropOperation.h"

#include "Data/Items/ItemBase.h"
#include "UI/Item/InventoryItemWidget.h"

UItemDragDropOperation::UItemDragDropOperation(): DragOffset()
{
}

void UItemDragDropOperation::RotateDraggedWidget()
{
	if (ItemMoveData.TargetOrientation == EItemOrientationType::Horizontal)
		ItemMoveData.TargetOrientation = EItemOrientationType::Vertical;
	else
		ItemMoveData.TargetOrientation = EItemOrientationType::Horizontal;

	OnRotationChanged.Broadcast(ItemMoveData.TargetOrientation);

	UInventoryItemWidget* DraggedWidget = Cast<UInventoryItemWidget>(DefaultDragVisual);
	if (!DraggedWidget) return;

	const float NewRotation = (ItemMoveData.TargetOrientation == EItemOrientationType::Vertical) ? 90.f : 0.f;

	DraggedWidget->SetRenderTransformAngle(NewRotation);

	const FIntPoint ItemSize = ItemMoveData.SourceItem->GetItemSize(ItemMoveData.TargetOrientation);

	/*DraggedWidget->SetDesiredSizeInViewport(FVector2D(
		UISettings.SlotSize.X * ItemSize.X,
		UISettings.SlotSize.Y * ItemSize.Y
	));*/

	DraggedWidget->SetDesiredSizeInViewport(FVector2D(
		64.0f * ItemSize.X,
		64.0f * ItemSize.Y
	));
}

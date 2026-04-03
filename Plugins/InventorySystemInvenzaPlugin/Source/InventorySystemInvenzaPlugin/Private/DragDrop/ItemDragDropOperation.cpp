//  Nublin Studio 2026 All Rights Reserved.

#include "DragDrop/ItemDragDropOperation.h"

#include "Components/CanvasPanelSlot.h"
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

	const FIntPoint ItemSize = ItemMoveData.SourceItem->GetItemSize(ItemMoveData.TargetOrientation);
	const FVector2D TotalSize = FVector2D(
	   UISettings.DragWidgetSlotSize.X * ItemSize.X ,
	   UISettings.DragWidgetSlotSize.Y * ItemSize.Y);

	float RotationAngle = 0.f;
	if (ItemSize.X != ItemSize.Y)
	{
		if ( ItemMoveData.SourceItem->GetInitialItemOrientation() == ItemMoveData.TargetOrientation)
			RotationAngle = 0;
		else
			RotationAngle = -90.0f;
	}

	DraggedWidget->SetDesiredSizeInViewport(TotalSize);
	DraggedWidget->UpdateVisualSize(TotalSize);
	DraggedWidget->UpdateVisual(ItemMoveData.SourceItem, RotationAngle);

	/*UE_LOG(LogTemp, Log, TEXT("Rotate: Size=(%d,%d) TotalSize=(%.1f,%.1f) Angle=%.1f Orientation=%s"),
		ItemSize.X, ItemSize.Y, TotalSize.X, TotalSize.Y, RotationAngle,
		ItemMoveData.TargetOrientation == EItemOrientationType::Horizontal ? TEXT("Horizontal") : TEXT("Vertical"));*/

}

//  Nublin Studio 2026 All Rights Reserved.

#include "DragDrop/ItemDragDropOperation.h"

#include "Components/CanvasPanelSlot.h"
#include "Data/Items/ItemBase.h"
#include "UI/Item/InventoryItemWidget.h"
#include "Utility/InterfaceUtils.h"
#include "Utility/InvenzayUtility.h"

UItemDragDropOperation::UItemDragDropOperation(): DragOffset()
{
}

void UItemDragDropOperation::RotateDraggedWidget()
{
	// Переключаем ориентацию
	if (ItemMoveData.TargetOrientation == EItemOrientationType::Horizontal)
		ItemMoveData.TargetOrientation = EItemOrientationType::Vertical;
	else
		ItemMoveData.TargetOrientation = EItemOrientationType::Horizontal;

	OnRotationChanged.Broadcast(ItemMoveData.TargetOrientation);

	UInventoryItemWidget* DraggedWidget = Cast<UInventoryItemWidget>(DefaultDragVisual);
	if (!DraggedWidget)
		return;

	UObject* Item = ItemMoveData.SourceItem.Get();
	if (!Item || !UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("RotateDraggedWidget")))
		return;
	
	const FIntPoint ItemSize =
		IObjectDataProvider::Execute_GetItemSize(Item, ItemMoveData.TargetOrientation);
	
	const FVector2D TotalSize =
		UInvenzayUtility::CalculateItemVisualSize(
			Item,
			ItemMoveData.TargetOrientation,
			UISettings.DragWidgetSlotSize,
			FMargin(0),
			false
		);

	float RotationAngle = 0.f;

	if (ItemSize.X != ItemSize.Y)
	{
		const EItemOrientationType InitialOrientation =
			IObjectDataProvider::Execute_GetInitialItemOrientation(Item);

		RotationAngle = (InitialOrientation == ItemMoveData.TargetOrientation)
			? 0.f
			: -90.f;
	}

	DraggedWidget->SetDesiredSizeInViewport(TotalSize);
	DraggedWidget->UpdateVisualSize(TotalSize);
	DraggedWidget->UpdateVisual(Item, RotationAngle);

	/*UE_LOG(LogTemp, Log, TEXT("Rotate: Size=(%d,%d) TotalSize=(%.1f,%.1f) Angle=%.1f Orientation=%s"),
		ItemSize.X, ItemSize.Y, TotalSize.X, TotalSize.Y, RotationAngle,
		ItemMoveData.TargetOrientation == EItemOrientationType::Horizontal ? TEXT("Horizontal") : TEXT("Vertical"));*/

}

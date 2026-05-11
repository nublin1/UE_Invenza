// Nublin Studio 2025 All Rights Reserve


#include "UI/Core/Zones/WorldDropZoneWidget.h"

#include "DragDrop/ItemDragDropOperation.h"

class UItemDragDropOperation;

UWorldDropZoneWidget::UWorldDropZoneWidget()
{
}

bool UWorldDropZoneWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UItemDragDropOperation* ItemOperation =	Cast<UItemDragDropOperation>(InOperation);
	if (!ItemOperation)
		return false;

	UItemBase* Item = ItemOperation->ItemMoveData.SourceItem.Get();
	if (!Item)
	{
		return false;
	}

	OnItemDroppedToWorld.Broadcast(ItemOperation->ItemMoveData);

	return true;
}

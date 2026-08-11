// Nublin Studio 2025 All Rights Reserve


#include "UI/Core/Zones/WorldDropZoneWidget.h"

#include "Data/Items/itemBase.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "Utility/InterfaceUtils.h"

class UItemDragDropOperation;

UWorldDropZoneWidget::UWorldDropZoneWidget()
{
}

bool UWorldDropZoneWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UItemDragDropOperation* ItemOperation = Cast<UItemDragDropOperation>(InOperation);
	if (!ItemOperation)
		return false;

	UObject* Item = ItemOperation->ItemMoveData.SourceItem.Get();
	if (!Item)
		return false;

	if (!UInterfaceUtils::ValidateImplementsInterface<IObjectDataProvider>(Item, TEXT("NativeOnDrop")))
		return false;

	const int32 Quantity = IObjectDataProvider::Execute_GetQuantity(Item);

	FItemDropData DropData(
		ItemOperation->ItemMoveData.SourceItem,
		ItemOperation->ItemMoveData.SourceInventory,
		Quantity
	);

	OnItemDroppedToWorld.Broadcast(DropData);

	return true;
}

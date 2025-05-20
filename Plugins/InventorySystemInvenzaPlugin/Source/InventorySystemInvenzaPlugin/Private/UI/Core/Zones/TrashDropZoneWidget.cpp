// Nublin Studio 2025 All Rights Reserved.


#include "UI/Core/Zones/TrashDropZoneWidget.h"

#include "ActorComponents/ItemCollection.h"
#include "Components/Border.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "UI/Inventory/UInventoryWidgetBase.h"

UTrashDropZoneWidget::UTrashDropZoneWidget()
{
}

void UTrashDropZoneWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (TrashBorder)
	{
		ColorByDefault = TrashBorder->GetBrushColor();
	}
	
}

bool UTrashDropZoneWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                        UDragDropOperation* InOperation)
{
	bool Result = Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	auto DragOp = Cast<UItemDragDropOperation>(InOperation);
	if (!DragOp->ItemMoveData.SourceItem)
		return Result;

	DragOp->ItemMoveData.SourceInventory->GetInventoryData().ItemCollectionLink->RemoveItemFromAllContainers(
		DragOp->ItemMoveData.SourceItem);

	if (TrashBorder)
	{
		TrashBorder->SetBrushColor(ColorByDefault);
	}
	return true;
}

void UTrashDropZoneWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (TrashBorder)
	{
		TrashBorder->SetBrushColor(ColorOnDrag);
	}
}

void UTrashDropZoneWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (TrashBorder)
	{
		TrashBorder->SetBrushColor(ColorByDefault);
	}
}

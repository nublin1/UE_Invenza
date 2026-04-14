// Nublin Studio 2025 All Rights Reserved.


#include "DragDrop/InvContainerDragDropOperation.h"

#include "UI/Core/MovableTitleBar/MovableTitleBar.h"

UInvContainerDragDropOperation::UInvContainerDragDropOperation(): DragOffset()
{
}

void UInvContainerDragDropOperation::Drop_Implementation(const FPointerEvent& PointerEvent)
{
	Super::Drop_Implementation(PointerEvent);
	
	if (Payload && Payload->GetClass()->ImplementsInterface(UUDraggableWidgetInterface::StaticClass()))
	{
		IUDraggableWidgetInterface::Execute_OnDragFinished(Payload, true, this);
	}
}

void UInvContainerDragDropOperation::DragCancelled_Implementation(const FPointerEvent& PointerEvent)
{
	Super::DragCancelled_Implementation(PointerEvent);

	if (Payload && Payload->GetClass()->ImplementsInterface(UUDraggableWidgetInterface::StaticClass()))
	{
		IUDraggableWidgetInterface::Execute_OnDragFinished(Payload, false, this);
	}
}

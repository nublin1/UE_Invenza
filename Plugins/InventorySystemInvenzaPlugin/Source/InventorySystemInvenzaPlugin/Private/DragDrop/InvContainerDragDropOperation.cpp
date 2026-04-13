// Nublin Studio 2025 All Rights Reserved.


#include "DragDrop/InvContainerDragDropOperation.h"

#include "UI/Core/MovableTitleBar/MovableTitleBar.h"

UInvContainerDragDropOperation::UInvContainerDragDropOperation(): DragOffset()
{
}

void UInvContainerDragDropOperation::Drop_Implementation(const FPointerEvent& PointerEvent)
{
	Super::Drop_Implementation(PointerEvent);
	
	if (auto* TitleBar = Cast<UMovableTitleBar>(Payload))
	{
		TitleBar->OnDragFinished(true, this);
	}
}

void UInvContainerDragDropOperation::DragCancelled_Implementation(const FPointerEvent& PointerEvent)
{
	Super::DragCancelled_Implementation(PointerEvent);

	if (auto* TitleBar = Cast<UMovableTitleBar>(Payload))
	{
		TitleBar->OnDragFinished(false, this);
	}
}

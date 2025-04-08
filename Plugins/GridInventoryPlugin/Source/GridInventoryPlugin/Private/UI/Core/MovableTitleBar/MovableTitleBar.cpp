//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Core/MovableTitleBar/MovableTitleBar.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "DragDrop/ItemDragDropOperation.h"

UMovableTitleBar::UMovableTitleBar()
{
}

void UMovableTitleBar::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (TitleName)
		TitleName->SetText(Title);
}

void UMovableTitleBar::NativeConstruct()
{
	Super::NativeConstruct();
}

bool UMovableTitleBar::OnDropped_Implementation(const FGeometry& DropGeometry, FVector2D DragOffset)
{
	FVector2D DropPosition = DropGeometry.AbsoluteToLocal(FSlateApplication::Get().GetCursorPos());
	auto CaSlot = Cast<UCanvasPanelSlot>(ParentWidget->Slot);
	
	if (!CaSlot)
		return false;
	
	CaSlot->SetPosition(DropPosition - DragOffset);
	ParentWidget->SetVisibility(ESlateVisibility::Visible);
	return true;
}

FReply UMovableTitleBar::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		if (ParentWidget)
		{
			return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
		}		
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UMovableTitleBar::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	UItemDragDropOperation* DragOperation = NewObject<UItemDragDropOperation>();
	DragOperation->DefaultDragVisual = ParentWidget;
	DragOperation->Pivot = EDragPivot::MouseDown;
	DragOperation->Payload = this;

	FVector2D ScreenCursorPos = InMouseEvent.GetScreenSpacePosition();
	auto DragOffset = ParentWidget->GetCachedGeometry().AbsoluteToLocal(ScreenCursorPos);
	DragOperation->DragOffset = DragOffset;
	
	OutOperation = DragOperation;

	ParentWidget->SetVisibility(ESlateVisibility::Collapsed);
	
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
}

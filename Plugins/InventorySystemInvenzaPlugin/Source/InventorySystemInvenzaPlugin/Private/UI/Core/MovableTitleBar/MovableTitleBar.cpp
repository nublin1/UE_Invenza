//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Core/MovableTitleBar/MovableTitleBar.h"

#include "ActorComponents/UIInventoryManager.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "DragDrop/InvContainerDragDropOperation.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "UI/Core/CoreCellWidget.h"

UMovableTitleBar::UMovableTitleBar(): DragContainer_Temp(nullptr)
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

	if (DragContainer_Temp)
		DragContainer_Temp->RemoveFromParent();
	return true;
}

FReply UMovableTitleBar::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		if (ParentWidget && bAllowDragging)
		{
			return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
		}		
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UMovableTitleBar::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	auto InventoryManager = GetOwningPlayerPawn()->FindComponentByClass<UIInventoryManager>();
	if (!InventoryManager) return;
	
	DragContainer_Temp = CreateWidget<UCoreCellWidget>(GetWorld(), InventoryManager->GetUISettings().DragContainerWidgetClass);
	if (DragContainer_Temp)
	{
		DragContainer_Temp->AddToPlayerScreen(1);
		DragContainer_Temp->SetPositionInViewport(FVector2D(-10000, -10000));

		FVector2D OriginalSize = ParentWidget->GetCachedGeometry().GetLocalSize();
		DragContainer_Temp->SizeBox->SetWidthOverride(OriginalSize.X);
		DragContainer_Temp->SizeBox->SetHeightOverride (OriginalSize.Y);
		DragContainer_Temp->SetDesiredSizeInViewport(OriginalSize);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Drag Widget not found"));
		return;
	}
	
	UInvContainerDragDropOperation* DragOperation = NewObject<UInvContainerDragDropOperation>();
	DragOperation->DefaultDragVisual = DragContainer_Temp;
	DragOperation->Pivot = EDragPivot::MouseDown;
	DragOperation->Payload = this;
	
	FVector2D ScreenCursorPos = InMouseEvent.GetScreenSpacePosition();
	auto _DragOffset = ParentWidget->GetCachedGeometry().AbsoluteToLocal(ScreenCursorPos);
	DragOperation->DragOffset = _DragOffset;
	
	OutOperation = DragOperation;

	ParentWidget->SetVisibility(ESlateVisibility::Collapsed);
	
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
}

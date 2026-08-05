//  Nublin Studio 2025 All Rights Reserved.


#include "UI/Core/MovableTitleBar/MovableTitleBar.h"

#include "ActorComponents/UIInventoryManager.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "DragDrop/InvContainerDragDropOperation.h"
#include "DragDrop/ItemDragDropOperation.h"
#include "Framework/Application/SlateApplication.h"
#include "UI/Core/CoreCellWidget.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "UI/Drag/DragContainerWidget.h"
#include "Utility/InvenzayUtility.h"

UMovableTitleBar::UMovableTitleBar()
{
}

void UMovableTitleBar::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UMovableTitleBar::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMovableTitleBar::OnDragFinished_Implementation(bool bSuccess, UDragDropOperation* InOperation)
{
	if (!ParentWidget)
		return;
	
	if (UInvContainerDragDropOperation* DragOp = Cast<UInvContainerDragDropOperation>(InOperation))
	{
		ParentWidget->SetVisibility(ESlateVisibility::Visible);

		if (bSuccess)
		{
			FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();
			FVector2D ViewportPos;
			FVector2D PixelPos;
			USlateBlueprintLibrary::AbsoluteToViewport(GetWorld(), CursorPos, PixelPos, ViewportPos);

			if (auto* CanvasSlot = Cast<UCanvasPanelSlot>(ParentWidget->Slot))
			{
		
				if (DragOp)
				{
					CanvasSlot->SetPosition(ViewportPos - DragOp->DragOffset);
				}
				else
				{
					CanvasSlot->SetPosition(ViewportPos);
				}
			}
		}

		if (DragContainer_Temp)
		{
			DragContainer_Temp->RemoveFromParent();
			DragContainer_Temp = nullptr;
		}
	}
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
	auto* Settings = UInvenzayUtility::GetInvenzaGlobalSettings(GetWorld());
	auto DragContainerclass = Settings->DragContainerWidgetClass;
	
	DragContainer_Temp = CreateWidget<UDragContainerWidget>(GetWorld(), DragContainerclass);
	if (DragContainer_Temp)
	{
		DragContainer_Temp->AddToPlayerScreen(1);
		DragContainer_Temp->SetPositionInViewport(FVector2D(-10000, -10000));

		FVector2D OriginalSize = ParentWidget->GetCachedGeometry().GetLocalSize();
		DragContainer_Temp->CoreCellWidget->SizeBox->SetWidthOverride(OriginalSize.X);
		DragContainer_Temp->CoreCellWidget->SizeBox->SetHeightOverride (OriginalSize.Y);
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

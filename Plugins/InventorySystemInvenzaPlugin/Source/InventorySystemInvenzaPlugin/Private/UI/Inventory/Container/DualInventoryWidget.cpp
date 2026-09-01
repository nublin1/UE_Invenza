// Nublin Studio 2026 All Rights Reserved.


#include "UI/Inventory/Container/DualInventoryWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "DragDrop/InvContainerDragDropOperation.h"

void UDualInventoryWidget::SetInventoriesContainers(UInventoryContainerWidget* FirstInventoryContainerWidget,
                                                    UInventoryContainerWidget* SecondInventoryContainerWidget)
{
}

void UDualInventoryWidget::SetLeftInventoryContainer(UInventoryContainerWidget* InContainer)
{
	if (!RootCanvas || !InContainer) return;

	if (InContainer->GetParent())
	{
		InContainer->RemoveFromParent();
	}

	RootCanvas->AddChildToCanvas(InContainer);
	LeftContainer = InContainer;
	bLeftPinned = true;
	
	LeftContainer->ReDrawRequest();
}

void UDualInventoryWidget::SetRightInventoryContainer(UInventoryContainerWidget* InContainer)
{
	if (!RootCanvas || !InContainer) return;

	if (InContainer->GetParent())
	{
		InContainer->RemoveFromParent();
	}

	RootCanvas->AddChildToCanvas(InContainer);
	RightContainer = InContainer;
	bRightPinned = true;
	
	RightContainer->ReDrawRequest();
}

bool UDualInventoryWidget::AddFloatingInventory(UObject* Key, UInventoryContainerWidget* ExistingWidget,
	FVector2D ScreenPosition, FVector2D Alignment)
{
	if (!Key || !ExistingWidget || !FloatingLayer)
	{
		return false;
	}
	
	if (UInventoryContainerWidget* Existing = FindFloatingInventory(Key))
	{
		if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(Existing->Slot))
		{
			PanelSlot->SetAlignment(Alignment);
			PanelSlot->SetPosition(ScreenPosition);
		}
		return Existing == ExistingWidget;
	}
	
	if (ExistingWidget->GetParent())
	{
		ExistingWidget->RemoveFromParent();
	}

	if (UCanvasPanelSlot* NewSlot = FloatingLayer->AddChildToCanvas(ExistingWidget))
	{
		NewSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
		NewSlot->SetAlignment(Alignment);
		NewSlot->SetPosition(ScreenPosition);
		NewSlot->SetAutoSize(true);
	}
	else
	{
		return false;
	}

	FloatingInventories.Add(Key, ExistingWidget);
	return true;
}

bool UDualInventoryWidget::RemoveFloatingInventory(UObject* Key)
{
	if (!Key) return false;

	if (TObjectPtr<UInventoryContainerWidget>* Found = FloatingInventories.Find(Key))
	{
		if (Found->Get())
		{
			Found->Get()->RemoveFromParent();
		}
		FloatingInventories.Remove(Key);
		return true;
	}
	return false;
}

void UDualInventoryWidget::ClearAll()
{
	LeftContainer = nullptr;
	RightContainer = nullptr;
	
	ClearFloatingInventories();
}

void UDualInventoryWidget::ClearFloatingInventories()
{
	for (auto& Pair : FloatingInventories)
	{
		if (Pair.Value) Pair.Value->RemoveFromParent();
	}
	FloatingInventories.Empty();
}

void UDualInventoryWidget::ApplyCornerAlignment()
{
	if (bLeftPinned && LeftContainer)
	{
		if (UCanvasPanelSlot* LeftSlot = Cast<UCanvasPanelSlot>(LeftContainer->Slot))
		{
			LeftSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
			LeftSlot->SetAlignment(FVector2D(0.f, 0.f));
			LeftSlot->SetPosition(FVector2D(CornerMargin, CornerMargin));
			LeftSlot->SetAutoSize(true);
		}
	}
	
	if (bRightPinned && RightContainer)
	{
		if (UCanvasPanelSlot* RightSlot = Cast<UCanvasPanelSlot>(RightContainer->Slot))
		{
			RightSlot->SetAnchors(FAnchors(1.f, 0.f, 1.f, 0.f));
			RightSlot->SetAlignment(FVector2D(1.f, 0.f));
			RightSlot->SetPosition(FVector2D(-CornerMargin, CornerMargin));
			RightSlot->SetAutoSize(true);
		}
	}
}

UInventoryContainerWidget* UDualInventoryWidget::FindFloatingInventory(UObject* Key) const
{
	if (const TObjectPtr<UInventoryContainerWidget>* Found = FloatingInventories.Find(Key))
	{
		return Found->Get();
	}
	return nullptr;
}

bool UDualInventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
									 UDragDropOperation* InOperation)
{
	if (UInvContainerDragDropOperation* DragOp = Cast<UInvContainerDragDropOperation>(InOperation))
	{
		if (UInventoryContainerWidget* Container = Cast<UInventoryContainerWidget>(DragOp->Payload))
		{
			if (Container == LeftContainer)
			{
				bLeftPinned = false;
			}
			else if (Container == RightContainer)
			{
				bRightPinned = false;
			}
		}
		return true;
	}

	return false;
}

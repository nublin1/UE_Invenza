// Nublin Studio 2026 All Rights Reserved.


#include "UI/GameMenu/GameMenuLayerInv.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/NamedSlot.h"
#include "Components/PanelWidget.h"
#include "Data/Inventory/InventoryBase.h"
#include "DragDrop/InvContainerDragDropOperation.h"

UGameMenuLayerInv::UGameMenuLayerInv()
{
}

void UGameMenuLayerInv::NativePreConstruct()
{
}

void UGameMenuLayerInv::NativeConstruct()
{
}

TArray<UUInventoryBaseWidget*> UGameMenuLayerInv::GetAllPawnInventories() const
{
	TArray<UUInventoryBaseWidget*> Result;
	if (!PawnInventories) return Result;

	for (int32 i = 0; i < PawnInventories->GetChildrenCount(); i++)
	{
		if (auto* InvContainer = Cast<UInventoryContainerWidget>(PawnInventories->GetChildAt(i)))
		{
			if (auto InventoryWidget = Cast<UUInventoryBaseWidget>(InvContainer->ContainerSlot->GetContent()))
				Result.Add(InventoryWidget);
		}
	}
	return Result;
}

TArray<UInventoryContainerWidget*> UGameMenuLayerInv::GetAllPawnInvContainers() const
{
	TArray<UInventoryContainerWidget*> Result;
	if (!PawnInventories) return Result;
	for (int32 i = 0; i < PawnInventories->GetChildrenCount(); i++)
	{
		if (auto* InvContainer = Cast<UInventoryContainerWidget>(PawnInventories->GetChildAt(i)))
			Result.Add(InvContainer);
	}

	return Result;
}

UPanelSlot* UGameMenuLayerInv::AddPawnInvContainerWidget(UInventoryContainerWidget* InvContainerWidgetToAdd) const
{
	if (!PawnInventories) return nullptr;

	UPanelSlot* InvContainerSlot = PawnInventories->AddChild(InvContainerWidgetToAdd);

	auto InvWidget = InvContainerWidgetToAdd->GetInventoryWidgetFromContainerSlot();
	InvWidget->ReDrawAllItems();
	EInventoryType Type = InvWidget->GetInventoryRef()->GetInventorySettings().InventoryType;

	if (InventoryDefaultPositions.Contains(Type))
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(InvContainerSlot))
		{
			CanvasSlot->SetAutoSize(true);
			CanvasSlot->SetSize(FVector2D(0,0));
			CanvasSlot->SetPosition(InventoryDefaultPositions[Type]);
		}
	}

	InvContainerWidgetToAdd->SetVisibility(ESlateVisibility::Collapsed);
	
	return InvContainerSlot;
}

void UGameMenuLayerInv::RemovePawnInvContainer(UInventoryContainerWidget* InvContainerToRemove) const
{
	if (!PawnInventories) return;

	for (int32 i = 0; i < PawnInventories->GetChildrenCount(); i++)
	{
		if (PawnInventories->GetChildAt(i) == InvContainerToRemove)
		{
			PawnInventories->RemoveChildAt(i);
			break;
		}
	}
}

void UGameMenuLayerInv::ToggleInventoryLayout()
{
	bInventoryOpen = !bInventoryOpen;
	
	const auto Containers = GetAllPawnInvContainers();
	if (Containers.IsEmpty())
		return;
	
	const ESlateVisibility NewVisibility =
		bInventoryOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	
	for (auto* Container : Containers)
	{
		if (Container)
		{
			Container->SetVisibility(NewVisibility);
		}
	}

	if (!GetWorld())
		return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
		return;

	if (bInventoryOpen)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);

		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
	else
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}

bool UGameMenuLayerInv::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (UInvContainerDragDropOperation* DragOp = Cast<UInvContainerDragDropOperation>(InOperation))
	{
		return true;
	}

	return false;
}

/*
FVector2D UGameMenuLayerInv::CalculateNextInventoryPosition(UCanvasPanelSlot* ParentSlot, FVector2D WindowSize) const
{
	const float OffsetX = 40.f;
	const float OffsetY = -20.f;

	FVector2D Position;
	if (ParentSlot)
	{
		FVector2D ParentPos = ParentSlot->GetPosition();
		FVector2D ParentSize = ParentSlot->GetSize();
		
		Position = ParentPos + FVector2D(ParentSize.X + OffsetX, OffsetY);
	}
	else
	{
		const int32 Index = PawnInventories ? PawnInventories->GetChildrenCount() : 0;
		Position.X = 500.f + Index * OffsetX;
		Position.Y = 200.f + Index * OffsetY;
	}

	if (UWorld* World = GetWorld())
	{
		// Получаем размер вьюпорта (уже в Slate Units, адаптированный под DPI!)
		FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(World);
		float DPIScale = UWidgetLayoutLibrary::GetViewportScale(World);
        
		// Переводим пиксели в единицы интерфейса (если GetViewportSize вернул сырые пиксели)
		// В зависимости от версии UE, GetViewportSize может отдавать нескалированные значения.
		// Чтобы железно работало везде, делим размер вьюпорта на DPI Scale:
		FVector2D ScaledViewportSize = ViewportSize / DPIScale;

		// Теперь Clamp работает в одной системе координат!
		Position.X = FMath::Clamp(Position.X, 0.f, ScaledViewportSize.X - WindowSize.X);
		Position.Y = FMath::Clamp(Position.Y, 0.f, ScaledViewportSize.Y - WindowSize.Y);
	}

	return Position;
}
*/

// Nublin Studio 2026 All Rights Reserved.


#include "UI/GameMenu/GameMenuLayerInv.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/NamedSlot.h"
#include "Components/PanelWidget.h"

UGameMenuLayerInv::UGameMenuLayerInv()
{
}

void UGameMenuLayerInv::NativePreConstruct()
{
}

void UGameMenuLayerInv::NativeConstruct()
{
}

TArray<UUInventoryWidgetBase*> UGameMenuLayerInv::GetAllPawnInventories() const
{
	TArray<UUInventoryWidgetBase*> Result;
	if (!PawnInventories) return Result;

	for (int32 i = 0; i < PawnInventories->GetChildrenCount(); i++)
	{
		if (auto* InvContainer = Cast<UInvBaseContainerWidget>(PawnInventories->GetChildAt(i)))
		{
			if (auto InventoryWidget = Cast<UUInventoryWidgetBase>(InvContainer->ContainerSlot->GetContent()))
				Result.Add(InventoryWidget);
		}
	}
	return Result;
}

TArray<UInvBaseContainerWidget*> UGameMenuLayerInv::GetAllPawnInvContainers() const
{
	TArray<UInvBaseContainerWidget*> Result;
	if (!PawnInventories) return Result;
	for (int32 i = 0; i < PawnInventories->GetChildrenCount(); i++)
	{
		if (auto* InvContainer = Cast<UInvBaseContainerWidget>(PawnInventories->GetChildAt(i)))
			Result.Add(InvContainer);
	}

	return Result;
}

UPanelSlot* UGameMenuLayerInv::AddPawnInvContainers(UInvBaseContainerWidget* InvContainerToAdd) const
{
	if (!PawnInventories) return nullptr;

	UPanelSlot* InvContainerSlot = PawnInventories->AddChild(InvContainerToAdd);
	return InvContainerSlot;
}

/*FVector2D UGameMenuLayerInv::CalculateNextInventoryPosition(UCanvasPanelSlot* ParentSlot, FVector2D WindowSize) const
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
		const int32 Index = PawnInventories->GetChildrenCount();

		Position.X = 500.f + Index * OffsetX;
		Position.Y = 200.f + Index * OffsetY;
	}

	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);

	Position.X = FMath::Clamp(Position.X, 0.f, ViewportSize.X - WindowSize.X);
	Position.Y = FMath::Clamp(Position.Y, 0.f, ViewportSize.Y - WindowSize.Y);

	return Position;
}*/

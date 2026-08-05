//  Nublin Studio 2026 All Rights Reserved.

#include "UI/Layout/UILayer.h"

#include "Components/Border.h"

UUILayer::UUILayer()
{
}

FReply UUILayer::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (MainBorder && MainBorder->IsHovered())
	{
		OnBackgroundClicked.Broadcast();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

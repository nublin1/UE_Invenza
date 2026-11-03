// Nublin Studio 2026 All Rights Reserved.

#include "UI/Core/BorderFrameWidget.h"

#include "Components/Border.h"

UBorderFrameWidget::UBorderFrameWidget()
{
}

void UBorderFrameWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	auto ApplyBorderStyle = [](UBorder* Border, const FBorderFrameStyle& Style)
	{
		if (!Border)
		{
			return;
		}

		Border->SetBrushColor(Style.BrushColor);
		Border->SetBrush(Style.Brush);
		Border->SetPadding(Style.Padding);
		Border->SetDesiredSizeScale(Style.DesiredSize);
	};

	ApplyBorderStyle(TopBorder, ResolveThemeStyle(TEXT("TopBorderStyle"), TopBorderStyle));
	ApplyBorderStyle(BottomBorder, ResolveThemeStyle(TEXT("BottomBorderStyle"), BottomBorderStyle));
	ApplyBorderStyle(LeftBorder, ResolveThemeStyle(TEXT("LeftBorderStyle"), LeftBorderStyle));
	ApplyBorderStyle(RightBorder, ResolveThemeStyle(TEXT("RightBorderStyle"), RightBorderStyle));
}

void UBorderFrameWidget::RefreshThemeAppearance()
{
	auto ApplyFrameAppearance = [](UBorder* Border, const FBorderFrameStyle& Style)
	{
		if (Border)
		{
			Border->SetBrush(Style.Brush);
			Border->SetBrushColor(Style.BrushColor);
		}
	};
	ApplyFrameAppearance(TopBorder, ResolveThemeStyle(TEXT("TopBorderStyle"), TopBorderStyle));
	ApplyFrameAppearance(BottomBorder, ResolveThemeStyle(TEXT("BottomBorderStyle"), BottomBorderStyle));
	ApplyFrameAppearance(LeftBorder, ResolveThemeStyle(TEXT("LeftBorderStyle"), LeftBorderStyle));
	ApplyFrameAppearance(RightBorder, ResolveThemeStyle(TEXT("RightBorderStyle"), RightBorderStyle));
}

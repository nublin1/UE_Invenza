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
		if (!Border) return;

		Border->SetBrushColor(Style.BrushColor);
		Border->SetBrush(Style.Brush);
		Border->SetPadding(Style.Padding);
		Border->SetDesiredSizeScale(Style.DesiredSize);
	};

	ApplyBorderStyle(TopBorder,    TopBorderStyle);
	ApplyBorderStyle(BottomBorder, BottomBorderStyle);
	ApplyBorderStyle(LeftBorder,   LeftBorderStyle);
	ApplyBorderStyle(RightBorder,  RightBorderStyle);
}

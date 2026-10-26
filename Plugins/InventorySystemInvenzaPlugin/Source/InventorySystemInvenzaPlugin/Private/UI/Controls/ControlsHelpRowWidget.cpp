// Nublin Studio 2026 All Rights Reserved.

#include "UI/Controls/ControlsHelpRowWidget.h"
#include "UI/Core/LabelBaseText.h"

void UControlsHelpRowWidget::SetRow(const FText& Action, const FText& Keys, const FLabelStyle& Style)
{
	if (ActionLabel && KeysLabel)
	{
		FLabelStyle LabelStyle = Style;
		LabelStyle.Text = Action;
		ActionLabel->UpdateTextAndFont(LabelStyle);
		LabelStyle.Text = Keys;
		KeysLabel->UpdateTextAndFont(LabelStyle);
	}
}

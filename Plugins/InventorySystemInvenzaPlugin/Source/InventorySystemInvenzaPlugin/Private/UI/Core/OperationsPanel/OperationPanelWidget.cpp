//  Nublin Studio 2025 All Rights Reserved.

#include "UI/Core/OperationsPanel/OperationPanelWidget.h"
#include "Engine/Font.h"
#include "UI/Core/LabelBaseText.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"

UOperationPanelWidget::UOperationPanelWidget()
{
	const auto* Settings = GetDefault<UInvenzaInventoryDeveloperSettings>();
	RestockLabelStyle.FontInfo =
		FSlateFontInfo(Settings->DefaultUIFont.LoadSynchronous(), Settings->RestockFontSize, TEXT("Bold"));
	RestockLabelStyle.Color = FSlateColor(Settings->DefaultTextColor);
}

void UOperationPanelWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (auto* Label = Cast<ULabelBaseText>(GetWidgetFromName(TEXT("PlaceMatchingLabel"))))
	{
		FLabelStyle Style = RestockLabelStyle;
		Style.Text = GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TEXT("OperationPanelWidget.Literal0"));
		Label->UpdateTextAndFont(Style);
	}
}

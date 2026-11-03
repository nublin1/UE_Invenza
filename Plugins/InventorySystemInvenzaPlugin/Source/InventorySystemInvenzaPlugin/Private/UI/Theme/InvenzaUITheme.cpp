// Nublin Studio 2026 All Rights Reserved.

#include "UI/Theme/InvenzaUITheme.h"

#include "Engine/Engine.h"
#include "Subsystems/InvenzaUIThemeSubsystem.h"

bool UInvenzaUITheme::ResolveFont(FName Role, FSlateFontInfo& OutFont) const
{
	const auto* Style = Typography.Find(Role);
	if (!Style)
	{
		return false;
	}
	OutFont = Style->Font;
	if (Style->bUseDefaultFont && DefaultFont)
	{
		OutFont.FontObject = DefaultFont;
	}
	return true;
}

bool UInvenzaUITheme::ResolveBrush(FName Role, FSlateBrush& OutBrush) const
{
	const auto* Style = Brushes.Find(Role);
	if (!Style)
	{
		return false;
	}
	OutBrush = Style->Brush;
	if (const auto* Color = Palette.Find(Style->ColorRole))
	{
		FLinearColor Tint = *Color;
		Tint.A = OutBrush.TintColor.GetSpecifiedColor().A;
		OutBrush.TintColor = FSlateColor(Tint);
	}
	return true;
}

#if WITH_EDITOR
void UInvenzaUITheme::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (GEngine)
	{
		if (auto* Themes = GEngine->GetEngineSubsystem<UInvenzaUIThemeSubsystem>())
		{
			Themes->RefreshDesignerWidgets();
		}
	}
}
#endif

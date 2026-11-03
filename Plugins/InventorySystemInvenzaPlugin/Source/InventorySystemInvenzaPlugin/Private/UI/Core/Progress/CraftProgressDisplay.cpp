//  Nublin Studio 2026 All Rights Reserved.

#include "UI/Core/Progress/CraftProgressDisplay.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Engine/Font.h"
#include "UI/Core/LabelBaseText.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "UI/Theme/InvenzaUIThemeLibrary.h"
#include "UI/Theme/InvenzaUITheme.h"

UCraftProgressDisplay::UCraftProgressDisplay()
{
	const auto* Settings = GetDefault<UInvenzaInventoryDeveloperSettings>();
	TextStyle.FontInfo = FSlateFontInfo(Settings->DefaultUIFont.LoadSynchronous(), Settings->ProgressFontSize, TEXT("Regular"));
	TextStyle.Color = FSlateColor(Settings->DefaultTextColor);
}

void UCraftProgressDisplay::NativePreConstruct()
{
	Super::NativePreConstruct();
	RefreshDisplay();
}

void UCraftProgressDisplay::ConfigureDisplay(const FCraftProgressDisplaySettings& Settings)
{
	DisplaySettings = Settings;
	RefreshDisplay();
}

void UCraftProgressDisplay::SetProgress(float CompletedWork, float RequiredWork)
{
	TotalWork = FMath::IsFinite(RequiredWork) ? FMath::Max(RequiredWork, 0.f) : 0.f;
	CurrentWork = FMath::IsFinite(CompletedWork) ? FMath::Clamp(CompletedWork, 0.f, TotalWork) : 0.f;
	RefreshDisplay();
}

float UCraftProgressDisplay::GetNormalizedProgress() const
{
	return TotalWork > 0.f ? FMath::Clamp(CurrentWork / TotalWork, 0.f, 1.f) : 0.f;
}

void UCraftProgressDisplay::RefreshThemeAppearance()
{
	if (ProgressBar)
	{
		ProgressBar->SetFillColorAndOpacity(ResolveThemeColorForProperty(TEXT("FillColor"), FillColor));
		FProgressBarStyle Style = ProgressBar->GetWidgetStyle();
		Style.BackgroundImage.TintColor = FSlateColor(ResolveThemeColorForProperty(TEXT("TrackColor"), TrackColor));
		ProgressBar->SetWidgetStyle(Style);
	}
	for (auto* Label : {CurrentValue.Get(), SeparatorSymbol.Get(), MaxValue.Get(), PercentageLabel.Get()})
	{
		if (Label)
		{
			const auto Style = ResolveThemeStyle(TEXT("TextStyle"), TextStyle);
			Label->UpdateAppearance(Style.FontInfo, Style.Color);
		}
	}
}

void UCraftProgressDisplay::RefreshDisplay()
{
	const bool bShowWorkAmount = !DisplaySettings.bUseProgressBar && !DisplaySettings.bShowPercentage;
	if (WorkAmount)
	{
		WorkAmount->SetVisibility(bShowWorkAmount ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (BarContainer)
	{
		BarContainer->SetVisibility(DisplaySettings.bUseProgressBar ? ESlateVisibility::HitTestInvisible
																	: ESlateVisibility::Collapsed);
		BarContainer->SetHeightOverride(FMath::Max(BarHeight, 1.f));
	}
	if (ProgressBar)
	{
		ProgressBar->SetIsMarquee(false);
		ProgressBar->SetFillColorAndOpacity(ResolveThemeColorForProperty(TEXT("FillColor"), FillColor));
		FProgressBarStyle Style = ProgressBar->GetWidgetStyle();
		Style.BackgroundImage.TintColor = FSlateColor(ResolveThemeColorForProperty(TEXT("TrackColor"), TrackColor));
		if (!UsesUITheme())
		{
			Style.FillImage.TintColor = FSlateColor(FLinearColor::White);
		}
		ProgressBar->SetWidgetStyle(Style);
		ProgressBar->SetPercent(GetNormalizedProgress());
	}
	auto UpdateProgressLabel = [this](ULabelBaseText* Label, const FText& Text)
	{
		if (Label)
		{
			FLabelStyle Style = ResolveThemeStyle(TEXT("TextStyle"), TextStyle);
			Style.Text = Text;
			Label->UpdateTextAndFont(Style);
		}
	};
	UpdateProgressLabel(CurrentValue, FText::AsNumber(CurrentWork));
	UpdateProgressLabel(SeparatorSymbol, GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(
											 TEXT("CraftProgressDisplay.Literal0")));
	UpdateProgressLabel(MaxValue, FText::AsNumber(TotalWork));
	if (PercentageLabel)
	{
		if (UHorizontalBoxSlot* LabelSlot = Cast<UHorizontalBoxSlot>(PercentageLabel->Slot))
		{
			LabelSlot->SetPadding(FMargin(DisplaySettings.bUseProgressBar ? 8.f : 0.f, 0.f, 0.f, 0.f));
		}
		FNumberFormattingOptions Format;
		Format.SetMaximumFractionalDigits(0);
		Format.SetMinimumFractionalDigits(0);
		UpdateProgressLabel(PercentageLabel, FText::AsPercent(GetNormalizedProgress(), &Format));
		PercentageLabel->SetVisibility(DisplaySettings.bShowPercentage ? ESlateVisibility::HitTestInvisible
																	   : ESlateVisibility::Collapsed);
	}
}

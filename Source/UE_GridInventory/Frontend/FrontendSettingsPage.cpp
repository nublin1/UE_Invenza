//  Nublin Studio 2026 All Rights Reserved.

#include "Frontend/FrontendSettingsPage.h"
#include "Frontend/InvenzaGameUserSettings.h"
#include "Frontend/FrontendMenuSettings.h"
#include "UI/Core/Buttons/UIButton.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

void UFrontendSettingsPage::NativeConstruct()
{
	Super::NativeConstruct();
	for (auto* Button :
	    {ApplySettings.Get(), ResetSettings.Get(), SettingsBack.Get(), KeepDisplay.Get(), RevertDisplay.Get()})
	{
		Button->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::Clicked);
	}
	Resolution->ClearOptions();
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
	Resolutions.AddUnique(GEngine->GetGameUserSettings()->GetScreenResolution());
	for (auto R : ResolutionPresets)
	{
		Resolutions.AddUnique(R);
	}
	Resolutions.Sort(
	    [](const FIntPoint& A, const FIntPoint& B)
	    {
		    return A.X == B.X ? A.Y < B.Y : A.X < B.X;
	    });
	for (auto R : Resolutions)
	{
		Resolution->AddOption(FString::Printf(TEXT("%d × %d"), R.X, R.Y));
	}
	WindowMode->ClearOptions();
	for (const auto& Label : WindowModeLabels)
	{
		WindowMode->AddOption(Label.ToString());
	}
	Quality->ClearOptions();
	for (const auto& Label : QualityLabels)
	{
		Quality->AddOption(Label.ToString());
	}
	FrameLimits = FrameLimitPresets;
	FrameLimits.AddUnique(GEngine->GetGameUserSettings()->GetFrameRateLimit());
	FrameLimit->ClearOptions();
	const auto* Texts = GetDefault<UFrontendMenuSettings>();
	for (float Limit : FrameLimits)
	{
		FrameLimit->AddOption(Limit == 0.f
		        ? Texts->GetText(TEXT("UnlimitedFPS")).ToString()
		        : FText::Format(Texts->GetText(TEXT("FPSValue")), FText::AsNumber(Limit)).ToString());
	}
	Activate();
}

void UFrontendSettingsPage::NativeDestruct()
{
	if (bConfirming)
	{
		FinishConfirmation(false);
	}
	for (auto* Button :
	    {ApplySettings.Get(), ResetSettings.Get(), SettingsBack.Get(), KeepDisplay.Get(), RevertDisplay.Get()})
	{
		if (Button)
		{
			Button->OnButtonClicked.RemoveDynamic(this, &ThisClass::Clicked);
		}
	}
	Super::NativeDestruct();
}

void UFrontendSettingsPage::Activate()
{
	if (bConfirming)
	{
		return;
	}
	SettingsPages->SetActiveWidgetIndex(0);
	LoadControls();
	SettingsStatus->SetText(FText::GetEmpty());
}

void UFrontendSettingsPage::LoadControls()
{
	auto* S = Cast<UInvenzaGameUserSettings>(GEngine->GetGameUserSettings());
	if (!S)
	{
		return;
	}
	Resolution->SetSelectedIndex(Resolutions.IndexOfByKey(S->GetScreenResolution()));
	WindowMode->SetSelectedIndex(static_cast<int32>(S->GetFullscreenMode()));
	Quality->SetSelectedIndex(
	    S->GetOverallScalabilityLevel() < 0 ? QualityLabels.Num() - 1 : S->GetOverallScalabilityLevel());
	FrameLimit->SetSelectedIndex(FrameLimits.IndexOfByKey(S->GetFrameRateLimit()));
	VSync->SetIsChecked(S->IsVSyncEnabled());
	MasterVolume->SetValue(S->MasterVolume);
	MusicVolume->SetValue(S->MusicVolume);
	EffectsVolume->SetValue(S->EffectsVolume);
}

void UFrontendSettingsPage::ApplyControls()
{
	auto* S = Cast<UInvenzaGameUserSettings>(GEngine->GetGameUserSettings());
	if (!S || bConfirming)
	{
		return;
	}
	const int32 R = Resolution->GetSelectedIndex();
	if (!Resolutions.IsValidIndex(R))
	{
		return;
	}
	const auto Mode = static_cast<EWindowMode::Type>(WindowMode->GetSelectedIndex());
	const bool Changed = Resolutions[R] != S->GetScreenResolution() || Mode != S->GetFullscreenMode();
	S->SetScreenResolution(Resolutions[R]);
	S->SetFullscreenMode(Mode);
	const int32 Q = Quality->GetSelectedIndex();
	if (Q >= 0 && Q < 5)
	{
		S->SetOverallScalabilityLevel(Q);
	}
	if (FrameLimits.IsValidIndex(FrameLimit->GetSelectedIndex()))
	{
		S->SetFrameRateLimit(FrameLimits[FrameLimit->GetSelectedIndex()]);
	}
	S->SetVSyncEnabled(VSync->IsChecked());
	S->MasterVolume = MasterVolume->GetValue();
	S->MusicVolume = MusicVolume->GetValue();
	S->EffectsVolume = EffectsVolume->GetValue();
	// Apply without saving until the display confirmation succeeds.
	S->ApplyResolutionSettings(false);
	static_cast<UGameUserSettings*>(S)->ApplyNonResolutionSettings();
	S->ApplyAudio(this);
	if (Changed)
	{
		bConfirming = true;
		ConfirmSeconds = DisplayConfirmationSeconds + 1;
		SettingsPages->SetActiveWidgetIndex(1);
		ConfirmTick();
		GetWorld()->GetTimerManager().SetTimer(ConfirmTimer, this, &ThisClass::ConfirmTick, 1.f, true);
	}
	else
	{
		S->ConfirmVideoMode();
		S->SaveSettings();
		SettingsStatus->SetText(GetDefault<UFrontendMenuSettings>()->GetText(TEXT("SettingsSaved")));
	}
}

void UFrontendSettingsPage::ConfirmTick()
{
	if (--ConfirmSeconds <= 0)
	{
		FinishConfirmation(false);
		return;
	}
	ConfirmLabel->SetText(
	    FText::Format(GetDefault<UFrontendMenuSettings>()->GetText(TEXT("DisplayCountdown")), ConfirmSeconds));
}

void UFrontendSettingsPage::FinishConfirmation(bool Keep)
{
	GetWorld()->GetTimerManager().ClearTimer(ConfirmTimer);
	bConfirming = false;
	auto* S = Cast<UInvenzaGameUserSettings>(GEngine->GetGameUserSettings());
	if (Keep)
	{
		S->ConfirmVideoMode();
		S->SaveSettings();
	}
	else
	{
		S->LoadSettings(true);
		S->ApplyResolutionSettings(false);
		static_cast<UGameUserSettings*>(S)->ApplyNonResolutionSettings();
		S->ApplyAudio(this);
	}
	SettingsPages->SetActiveWidgetIndex(0);
	LoadControls();
	SettingsStatus->SetText(
	    GetDefault<UFrontendMenuSettings>()->GetText(Keep ? TEXT("SettingsSaved") : TEXT("SettingsRestored")));
}

void UFrontendSettingsPage::Clicked(UUIButton* Button)
{
	if (Button == KeepDisplay)
	{
		FinishConfirmation(true);
		return;
	}
	if (Button == RevertDisplay)
	{
		FinishConfirmation(false);
		return;
	}
	if (bConfirming)
	{
		return;
	}
	if (Button == ApplySettings)
	{
		ApplyControls();
	}
	else if (Button == SettingsBack)
	{
		OnBackRequested.Broadcast();
	}
	else if (Button == ResetSettings)
	{
		auto* Settings = Cast<UInvenzaGameUserSettings>(GEngine->GetGameUserSettings());
		if (!Settings)
		{
			return;
		}
		// Stage defaults in controls without replacing the currently applied preferences.
		const auto* Defaults = GetDefault<UInvenzaGameUserSettings>();
		Resolution->SetSelectedIndex(Resolutions.IndexOfByKey(Defaults->GetScreenResolution()));
		WindowMode->SetSelectedIndex(static_cast<int32>(Defaults->GetFullscreenMode()));
		Quality->SetSelectedIndex(FMath::Max(0, Defaults->GetOverallScalabilityLevel()));
		FrameLimit->SetSelectedIndex(FrameLimits.IndexOfByKey(Defaults->GetFrameRateLimit()));
		VSync->SetIsChecked(Defaults->IsVSyncEnabled());
		MasterVolume->SetValue(Defaults->MasterVolume);
		MusicVolume->SetValue(Defaults->MusicVolume);
		EffectsVolume->SetValue(Defaults->EffectsVolume);
		SettingsStatus->SetText(GetDefault<UFrontendMenuSettings>()->GetText(TEXT("DefaultsStaged")));
	}
}

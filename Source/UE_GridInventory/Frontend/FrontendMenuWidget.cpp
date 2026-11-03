//  Nublin Studio 2026 All Rights Reserved.

#include "Frontend/FrontendMenuWidget.h"
#include "Frontend/FrontendSettingsPage.h"
#include "Frontend/FrontendNetworkPage.h"
#include "Frontend/FrontendMenuSubsystem.h"
#include "Frontend/FrontendMenuSettings.h"
#include "Frontend/InvenzaSessionsSubsystem.h"
#include "Subsystems/InvenzaSaveSubsystem.h"
#include "Subsystems/UIInputModeSubsystem.h"
#include "UI/SaveLoad/InvenzaSaveWidget.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/LabelBaseText.h"
#include "Components/WidgetSwitcher.h"
#include "Components/PanelWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

void UFrontendSessionBar::NativeConstruct()
{
	Super::NativeConstruct();
	LeaveGame->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::ClickMenu);
}

void UFrontendSessionBar::NativeDestruct()
{
	if (LeaveGame)
	{
		LeaveGame->OnButtonClicked.RemoveDynamic(this, &ThisClass::ClickMenu);
	}
	Super::NativeDestruct();
}

void UFrontendSessionBar::ClickMenu(UUIButton*)
{
	if (auto* Local = GetOwningLocalPlayer())
	{
		Local->GetSubsystem<UFrontendMenuSubsystem>()->ShowMenu();
	}
}

void UFrontendMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (auto* PC = GetOwningPlayer(); PC && !bOwnsGameplayInputBlock)
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
		bOwnsGameplayInputBlock = true;
	}
	bPlaying = GetOwningPlayer() && GetOwningPlayer()->GetPawn();
	if (auto* Local = GetOwningLocalPlayer())
	{
		if (auto* Input = Local->GetSubsystem<UUIInputModeSubsystem>())
		{
			Input->RequestUIInput(this);
		}
	}
	for (auto* Button : {ResumeButton.Get(), NewGame.Get(), ConfirmNewGame.Get(), CancelNewGame.Get(), OpenSave.Get(), OpenLoad.Get(), OpenSettings.Get(), OpenNetwork.Get(),
	         LeaveSession.Get(), QuitGame.Get()})
	{
		Button->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::Clicked);
	}
	SettingsPage->OnBackRequested.AddUniqueDynamic(this, &ThisClass::BackHome);
	NetworkPage->OnBackRequested.AddUniqueDynamic(this, &ThisClass::BackHome);
	SavePage->OnCloseRequested.AddUniqueDynamic(this, &ThisClass::BackHome);
	LoadPage->OnCloseRequested.AddUniqueDynamic(this, &ThisClass::BackHome);
	SavePage->LoadWorld = LoadPage->LoadWorld = GetDefault<UFrontendMenuSettings>()->PlayWorld;
	ResumeButton->SetVisibility(bPlaying ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	LeaveSession->SetVisibility(bPlaying ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	const auto* Instance = GetGameInstance();
	OpenSave->SetIsEnabled(Instance && Instance->GetSubsystem<UInvenzaSaveSubsystem>()->CanManageSession());
	NewGame->SetIsEnabled(GetWorld()->GetNetMode() != NM_Client);
	BackHome();
	SetKeyboardFocus();
}

void UFrontendMenuWidget::NativeDestruct()
{
	ReleaseMenuInput();
	for (auto* Button : {ResumeButton.Get(), NewGame.Get(), ConfirmNewGame.Get(), CancelNewGame.Get(), OpenSave.Get(), OpenLoad.Get(), OpenSettings.Get(), OpenNetwork.Get(),
	         LeaveSession.Get(), QuitGame.Get()})
	{
		if (Button)
		{
			Button->OnButtonClicked.RemoveDynamic(this, &ThisClass::Clicked);
		}
	}
	if (SettingsPage)
	{
		SettingsPage->OnBackRequested.RemoveDynamic(this, &ThisClass::BackHome);
	}
	if (NetworkPage)
	{
		NetworkPage->OnBackRequested.RemoveDynamic(this, &ThisClass::BackHome);
	}
	if (SavePage)
	{
		SavePage->OnCloseRequested.RemoveDynamic(this, &ThisClass::BackHome);
	}
	if (LoadPage)
	{
		LoadPage->OnCloseRequested.RemoveDynamic(this, &ThisClass::BackHome);
	}
	Super::NativeDestruct();
}

void UFrontendMenuWidget::OnRemovedFromLayer_Implementation()
{
	ReleaseMenuInput();
	Super::OnRemovedFromLayer_Implementation();
}

void UFrontendMenuWidget::ReleaseMenuInput()
{
	if (bOwnsGameplayInputBlock)
	{
		if (auto* PC = GetOwningPlayer())
		{
			PC->SetIgnoreMoveInput(false);
			PC->SetIgnoreLookInput(false);
		}
		bOwnsGameplayInputBlock = false;
	}
	if (auto* Local = GetOwningLocalPlayer())
	{
		if (auto* Input = Local->GetSubsystem<UUIInputModeSubsystem>())
		{
			Input->ReleaseUIInput(this);
		}
	}
}

void UFrontendMenuWidget::ShowPage(EFrontendPage Page)
{
	if (SettingsPage->IsConfirming())
	{
		return;
	}
	if (auto* Instance = GetGameInstance())
	{
		if (Instance->GetSubsystem<UInvenzaSessionsSubsystem>()->bBusy && Page != EFrontendPage::Network)
		{
			return;
		}
	}
	switch (Page)
	{
	case EFrontendPage::Settings:
		Pages->SetActiveWidget(SettingsPage);
		SettingsPage->Activate();
		break;
	case EFrontendPage::Network:
		Pages->SetActiveWidget(NetworkPage);
		NetworkPage->Activate();
		break;
	case EFrontendPage::Save:
		Pages->SetActiveWidget(SavePage);
		SavePage->Refresh();
		break;
	case EFrontendPage::Load:
		Pages->SetActiveWidget(LoadPage);
		LoadPage->Refresh();
		break;
	default:
		Pages->SetActiveWidget(HomePage);
		break;
	}
}

void UFrontendMenuWidget::BackHome()
{
	bConfirmingNewGame = false;
	ShowPage(EFrontendPage::Home);
}

void UFrontendMenuWidget::Clicked(UUIButton* Button)
{
	if (Button == NewGame)
	{
		if (!SettingsPage->IsConfirming())
		{
			bConfirmingNewGame = true;
			Pages->SetActiveWidget(NewGameConfirmation);
		}
		return;
	}
	if (Button == CancelNewGame)
	{
		BackHome();
		return;
	}
	if (Button == ConfirmNewGame)
	{
		if (!bConfirmingNewGame)
		{
			return;
		}
		auto* Save = GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>();
		if (Save->StartNewGame(GetDefault<UFrontendMenuSettings>()->PlayWorld))
		{
			RemoveFromParent();
		}
		else
		{
			MenuStatus->UpdateText(GetDefault<UFrontendMenuSettings>()->GetText(TEXT("WaitForSave")));
			BackHome();
		}
		return;
	}
	if (Button == ResumeButton)
	{
		RemoveFromParent();
	}
	else if (Button == OpenSettings)
	{
		ShowPage(EFrontendPage::Settings);
	}
	else if (Button == OpenNetwork)
	{
		ShowPage(EFrontendPage::Network);
	}
	else if (Button == OpenSave)
	{
		ShowPage(EFrontendPage::Save);
	}
	else if (Button == OpenLoad)
	{
		ShowPage(EFrontendPage::Load);
	}
	else if (Button == LeaveSession)
	{
		GetGameInstance()->GetSubsystem<UInvenzaSessionsSubsystem>()->Leave();
	}
	else if (Button == QuitGame)
	{
		if (!GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->SaveBeforeExit())
		{
			MenuStatus->UpdateText(GetDefault<UFrontendMenuSettings>()->GetText(TEXT("WaitForSave")));
			return;
		}
		UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
	}
}

FReply UFrontendMenuWidget::NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
	if (bConfirmingNewGame && Event.GetKey() == EKeys::Escape)
	{
		BackHome();
		return FReply::Handled();
	}
	if (bConfirmingNewGame)
	{
		return FReply::Handled();
	}
	if (bPlaying && Event.GetKey() == GetDefault<UFrontendMenuSettings>()->MenuKey)
	{
		if (!SettingsPage->IsConfirming())
		{
			RemoveFromParent();
		}
		return FReply::Handled();
	}
	if (Event.GetKey() == EKeys::Escape)
	{
		if (SettingsPage->IsConfirming())
		{
			return FReply::Handled();
		}
		if (Pages->GetActiveWidget() != HomePage)
		{
			BackHome();
		}
		else if (bPlaying)
		{
			RemoveFromParent();
		}
		return FReply::Handled();
	}
	return FReply::Handled();
}

FReply UFrontendMenuWidget::NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event)
{
	return FReply::Handled();
}

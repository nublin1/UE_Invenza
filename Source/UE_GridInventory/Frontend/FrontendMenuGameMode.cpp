//  Nublin Studio 2026 All Rights Reserved.

#include "Frontend/FrontendMenuGameMode.h"
#include "Frontend/FrontendMenuWidget.h"
#include "Frontend/FrontendMenuSettings.h"
#include "Engine/LocalPlayer.h"
#include "UI/Layout/PrimaryLayout.h"
#include "UI/Layout/UILayer.h"
#include "GameFramework/PlayerController.h"

AFrontendMenuGameMode::AFrontendMenuGameMode()
{
	HUDClass = AFrontendMenuHUD::StaticClass();
	DefaultPawnClass = nullptr;
}

void AFrontendMenuHUD::BeginPlay()
{
	Super::BeginPlay();
	auto* PC = GetOwningPlayerController();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}
	const auto* Settings = GetDefault<UFrontendMenuSettings>();
	auto LayoutClass = Settings->LayoutClass.LoadSynchronous();
	auto MenuClass = Settings->MenuClass.LoadSynchronous();
	if (!LayoutClass || !MenuClass)
	{
		return;
	}
	PrimaryLayout = CreateWidget<UPrimaryLayout>(PC, LayoutClass);
	PrimaryLayout->AddToViewport();
	PrimaryLayout->GameLayer->SetVisibility(ESlateVisibility::Collapsed);
	PrimaryLayout->GameMenuLayer->SetVisibility(ESlateVisibility::Collapsed);
	PrimaryLayout->ModalLayer->SetVisibility(ESlateVisibility::Collapsed);
	PrimaryLayout->MenuLayer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	UUserWidget* Screen = PrimaryLayout->PushContentToLayer(
	    FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.Menu")), TSoftClassPtr<UUserWidget>(MenuClass));
	PC->bShowMouseCursor = true;
	FInputModeUIOnly Mode;
	if (Screen)
	{
		Mode.SetWidgetToFocus(Screen->TakeWidget());
	}
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(Mode);
}

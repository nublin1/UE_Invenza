//  Nublin Studio 2026 All Rights Reserved.

#include "Frontend/FrontendMenuSubsystem.h"
#include "Frontend/FrontendMenuWidget.h"
#include "Frontend/FrontendMenuSettings.h"
#include "Components/InputComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "UObject/Package.h"
#include "HUD/GameHUD_Inz.h"
#include "Interface/HUD/HUDProvider.h"
#include "UI/Layout/PrimaryLayout.h"
#include "UI/Layout/UILayer.h"

static UPrimaryLayout* FindFrontendPrimaryLayout(APlayerController* Controller)
{
	auto* HUD = Controller ? Controller->GetHUD() : nullptr;
	if (IsValid(HUD) && HUD->Implements<UHUDProvider>())
	{
		return IHUDProvider::Execute_GetPrimaryLayout(HUD);
	}
	if (const auto* InvenzaHUD = Cast<AGameHUD_Inz>(HUD))
	{
		return InvenzaHUD->GetPrimaryLayoutInstance();
	}
	return nullptr;
}

void UFrontendMenuSubsystem::PlayerControllerChanged(APlayerController* NewController)
{
	Super::PlayerControllerChanged(NewController);
	CloseMenu();
	RemoveMenuInput();
	Controller = NewController;
	if (NewController && NewController->GetWorld())
	{
		NewController->GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::InitializeMenuAccess);
	}
}

void UFrontendMenuSubsystem::InitializeMenuAccess()
{
	auto* PC = Controller.Get();
	if (!PC || !PC->IsLocalController() || !PC->GetWorld() || PC->GetWorld()->bIsTearingDown)
	{
		return;
	}
	if (MenuInputComponent)
	{
		return;
	}
	const auto* Settings = GetDefault<UFrontendMenuSettings>();
	if (UWorld::RemovePIEPrefix(PC->GetWorld()->GetOutermost()->GetName()) !=
	    Settings->PlayWorld.ToSoftObjectPath().GetLongPackageName())
	{
		return;
	}
	if (!Settings->MenuKey.IsValid())
	{
		return;
	}
	MenuInputComponent = NewObject<UInputComponent>(PC);
	MenuInputComponent->RegisterComponent();
	MenuInputComponent->bBlockInput = false;
	MenuInputComponent->BindKey(Settings->MenuKey, IE_Pressed, this, &ThisClass::ToggleMenu);
	PC->PushInputComponent(MenuInputComponent);
}

void UFrontendMenuSubsystem::RemoveMenuInput()
{
	if (MenuInputComponent)
	{
		if (auto* PC = Controller.Get())
		{
			PC->PopInputComponent(MenuInputComponent);
		}
		MenuInputComponent->DestroyComponent();
		MenuInputComponent = nullptr;
	}
}

void UFrontendMenuSubsystem::ToggleMenu()
{
	if (Menu && MenuLayer.IsValid() && MenuLayer->ContainsContent(Menu))
	{
		CloseMenu();
	}
	else
	{
		ShowMenu();
	}
}

void UFrontendMenuSubsystem::ShowMenu()
{
	auto* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC)
	{
		return;
	}
	if (Menu && MenuLayer.IsValid() && MenuLayer->ContainsContent(Menu))
	{
		MenuLayer->PushWidget(Menu);
		Menu->SetKeyboardFocus();
		return;
	}
	auto* Layout = FindFrontendPrimaryLayout(PC);
	const FGameplayTag LayerTag = Layout ? Layout->GetLayerTag(Layout->MenuLayer) : FGameplayTag();
	if (!Layout || !LayerTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Frontend menu cannot open: this player's Primary Layout has no registered menu layer."));
		return;
	}
	Menu = Cast<UFrontendMenuWidget>(Layout->PushContentToLayer(LayerTag,
		TSoftClassPtr<UUserWidget>(GetDefault<UFrontendMenuSettings>()->MenuClass.ToSoftObjectPath())));
	if (Menu)
	{
		MenuLayer = Layout->MenuLayer;
		MenuLayer->OnContentRemoved.AddUniqueDynamic(this, &ThisClass::MenuRemovedFromLayer);
	}
}

void UFrontendMenuSubsystem::CloseMenu()
{
	auto* Layer = MenuLayer.Get();
	auto* ClosingMenu = Menu.Get();
	Menu = nullptr;
	MenuLayer.Reset();
	if (Layer)
	{
		Layer->OnContentRemoved.RemoveDynamic(this, &ThisClass::MenuRemovedFromLayer);
		Layer->RemoveContent(ClosingMenu);
	}
}

void UFrontendMenuSubsystem::MenuRemovedFromLayer(UInvenzaBaseWidget* RemovedWidget)
{
	if (RemovedWidget == Menu)
	{
		if (MenuLayer.IsValid())
		{
			MenuLayer->OnContentRemoved.RemoveDynamic(this, &ThisClass::MenuRemovedFromLayer);
		}
		Menu = nullptr;
		MenuLayer.Reset();
	}
}

void UFrontendMenuSubsystem::Deinitialize()
{
	CloseMenu();
	RemoveMenuInput();
	Super::Deinitialize();
}

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
	if (Menu && Menu->IsInViewport())
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
	if (Menu && Menu->IsInViewport())
	{
		Menu->SetKeyboardFocus();
		return;
	}
	if (auto Class = GetDefault<UFrontendMenuSettings>()->MenuClass.LoadSynchronous())
	{
		Menu = CreateWidget<UFrontendMenuWidget>(PC, Class);
		if (Menu)
		{
			Menu->AddToViewport(100);
		}
	}
}

void UFrontendMenuSubsystem::CloseMenu()
{
	if (Menu)
	{
		Menu->RemoveFromParent();
	}
	Menu = nullptr;
}

void UFrontendMenuSubsystem::Deinitialize()
{
	CloseMenu();
	RemoveMenuInput();
	Super::Deinitialize();
}

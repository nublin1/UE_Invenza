//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/SaveLoad/InvenzaSaveManager.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Subsystems/InvenzaSaveSubsystem.h"
#include "SaveLoad/InvenzaSaveSettings.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UInvenzaSaveManager::UInvenzaSaveManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInvenzaSaveManager::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->OnSessionLoaded.AddUniqueDynamic(
		this, &ThisClass::SessionLoaded);
	if (auto* Manager = GetOwner()->FindComponentByClass<UIInventoryManager>())
	{
		Manager->OnInitializationCompleteDelegate.AddUniqueDynamic(this, &ThisClass::OnInitializationComplete);
	}
}

void UInvenzaSaveManager::EndPlay(const EEndPlayReason::Type Reason)
{
	if (auto* Manager = GetOwner()->FindComponentByClass<UIInventoryManager>())
	{
		Manager->OnInitializationCompleteDelegate.RemoveDynamic(this, &ThisClass::OnInitializationComplete);
	}
	if (GetWorld() && GetWorld()->GetGameInstance())
	{
		GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->OnSessionLoaded.RemoveDynamic(
			this, &ThisClass::SessionLoaded);
	}
	Super::EndPlay(Reason);
}

void UInvenzaSaveManager::OnInitializationComplete()
{
	if (GetOwner()->HasAuthority())
	{
		GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->NotifyPlayerReady();
	}
}

void UInvenzaSaveManager::ShowSaveMenu()
{
	if (auto* Pawn = Cast<APawn>(GetOwner()))
	{
		GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->ShowSaveMenu(
			Cast<APlayerController>(Pawn->GetController()));
	}
}

void UInvenzaSaveManager::SaveGame_Implementation(bool Async)
{
	GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->SaveSession(TEXT("Quick"), Async);
}

void UInvenzaSaveManager::LoadGame_Implementation(bool Async)
{
	GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->LoadSession();
}

void UInvenzaSaveManager::SessionLoaded()
{
	LoadedSaveData = GetWorld()->GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->GetLoadedSnapshot();
	OnGameLoaded.Broadcast();
}

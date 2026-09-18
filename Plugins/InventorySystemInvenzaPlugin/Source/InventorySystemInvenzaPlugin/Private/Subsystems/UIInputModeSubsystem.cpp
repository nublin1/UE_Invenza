#include "Subsystems/UIInputModeSubsystem.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

void UUIInputModeSubsystem::RequestUIInput(UObject* RequestOwner)
{
	if (!IsValid(RequestOwner)) return;
	RequestOwners.Add(TWeakObjectPtr<UObject>(RequestOwner));
	UpdateInputMode();
}

void UUIInputModeSubsystem::ReleaseUIInput(UObject* RequestOwner)
{
	RequestOwners.Remove(TWeakObjectPtr<UObject>(RequestOwner));
	UpdateInputMode();
}

void UUIInputModeSubsystem::UpdateInputMode()
{
	for (auto It = RequestOwners.CreateIterator(); It; ++It)
	{
		if (!It->IsValid()) It.RemoveCurrent();
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	APlayerController* PC = LocalPlayer
		? LocalPlayer->GetPlayerController(GetWorld()) : nullptr;
	if (!IsValid(PC)) return;

	const bool bNeedsUIInput = !RequestOwners.IsEmpty();
	if (AppliedController.Get() == PC && bUIInputApplied == bNeedsUIInput) return;

	if (bNeedsUIInput)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
	}
	else
	{
		PC->SetInputMode(FInputModeGameOnly());
	}
	PC->bShowMouseCursor = bNeedsUIInput;
	AppliedController = PC;
	bUIInputApplied = bNeedsUIInput;
}

void UUIInputModeSubsystem::Deinitialize()
{
	RequestOwners.Empty();
	UpdateInputMode();
	AppliedController.Reset();
	Super::Deinitialize();
}

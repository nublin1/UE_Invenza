// Nublin Studio 2026 All Rights Reserved.

#include "Examples/InvenzaExamplePlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedPlayerInput.h"

AInvenzaExamplePlayerController::AInvenzaExamplePlayerController()
{
	OverridePlayerInputClass = UEnhancedPlayerInput::StaticClass();
}

void AInvenzaExamplePlayerController::SetupInputComponent()
{
	if (!InputComponent)
	{
		InputComponent = NewObject<UEnhancedInputComponent>(this, TEXT("ExampleInputComponent"));
		InputComponent->RegisterComponent();
	}
	Super::SetupInputComponent();
}

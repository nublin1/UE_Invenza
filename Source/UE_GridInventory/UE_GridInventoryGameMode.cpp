// Copyright Epic Games, Inc. All Rights Reserved.

#include "UE_GridInventoryGameMode.h"
#include "UE_GridInventoryCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUE_GridInventoryGameMode::AUE_GridInventoryGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

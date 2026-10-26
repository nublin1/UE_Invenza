// Copyright Epic Games, Inc. All Rights Reserved.

#include "UE_GridInventoryGameMode.h"
#include "UE_GridInventoryCharacter.h"

AUE_GridInventoryGameMode::AUE_GridInventoryGameMode()
{
	// Blueprint game modes can select their own pawn without a required content path.
	DefaultPawnClass = AUE_GridInventoryCharacter::StaticClass();
}

// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/SaveLoad/SaveLoadStructs.h"
#include "GameFramework/SaveGame.h"
#include "SaveLoad/InventorySaveTypes.h"
#include "InvenzaSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Properties and variables
	UPROPERTY()
	int32 FormatVersion = 1;

	UPROPERTY()
	bool bFreshStart = false;

	UPROPERTY()
	FString WorldPackage;

	UPROPERTY()
	FGuid PlaythroughID;

	UPROPERTY()
	int64 Revision = 0;

	UPROPERTY()
	FDateTime SavedAt;

	UPROPERTY()
	TArray<FInventorySavedOwner> Owners;

	UPROPERTY()
	TArray<FInventorySavedPickup> Pickups;
};

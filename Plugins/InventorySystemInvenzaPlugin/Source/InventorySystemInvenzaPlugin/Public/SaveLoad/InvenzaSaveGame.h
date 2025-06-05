// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/SaveLoad/SaveLoadStructs.h"
#include "GameFramework/SaveGame.h"
#include "InvenzaSaveGame.generated.h"


/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SaveLoad")
	FInvSaveData PlayerSavedInventories;
};

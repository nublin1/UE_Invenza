// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ISaveLoadGame.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UISaveLoadGame : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYSYSTEMINVENZAPLUGIN_API IISaveLoadGame
{
	GENERATED_BODY()

public:
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveGame")
	void SaveGame(bool Async);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveGame")
	void LoadGame(bool Async);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveGame")
	void GetGameData();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveGame")
	void SavePlayer();
};

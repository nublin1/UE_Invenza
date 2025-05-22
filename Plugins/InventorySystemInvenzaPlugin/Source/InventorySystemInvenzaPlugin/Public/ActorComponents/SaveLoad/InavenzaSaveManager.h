// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SaveLoad/ISaveLoadGame.h"
#include "InavenzaSaveManager.generated.h"


class UInputAction;
class UInvenzaSaveGame;

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameLoaded);
#pragma endregion Delegates

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INVENTORYSYSTEMINVENZAPLUGIN_API UInavenzaSaveManager : public UActorComponent, public IISaveLoadGame
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnGameLoaded OnGameLoaded;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UInavenzaSaveManager();
	
	virtual void SaveGame_Implementation(bool Async) override;
	virtual void LoadGame_Implementation(bool Async) override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> SaveAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> LoadAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SaveSlotName = "InventorySave";
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SaveUserIndex = 0;

	UPROPERTY()
	TObjectPtr<UInvenzaSaveGame> LoadedSaveData;
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnSaveGameLoaded(const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGame);

	UFUNCTION()
	void HandleSaveInput();
	UFUNCTION()
	void HandleLoadInput();
};

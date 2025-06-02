// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SaveLoad/ISaveLoadGame.h"
#include "InvenzaSaveManager.generated.h"


class USaveGame;
class UInputAction;
class UInvenzaSaveGame;

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameLoaded);
#pragma endregion Delegates

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaSaveManager : public UActorComponent, public IISaveLoadGame
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
	UInvenzaSaveManager();
	
	virtual void SaveGame_Implementation(bool Async = false) override;
	virtual void LoadGame_Implementation(bool Async = false) override;

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
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	int32 SaveUserIndex = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TObjectPtr<UInvenzaSaveGame> LoadedSaveData;
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnInitializationComplete();

	UFUNCTION()
	void OnSaveGameLoaded(const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGame);

	UFUNCTION()
	void HandleSaveInput();
	UFUNCTION()
	void HandleLoadInput();
};

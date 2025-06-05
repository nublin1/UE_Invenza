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
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Save System|Events")
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save System|Inputs")
	TObjectPtr<UInputAction> SaveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save System|Inputs")
	TObjectPtr<UInputAction> LoadAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save System|Config")
	FName SaveSlotName = "InventorySave";

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Save System|Config")
	int32 SaveUserIndex = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Save System|Data")
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

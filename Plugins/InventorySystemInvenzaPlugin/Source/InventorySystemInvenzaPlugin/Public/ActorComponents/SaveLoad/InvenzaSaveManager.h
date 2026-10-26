//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "SaveLoad/ISaveLoadGame.h"
#include "SaveLoad/InventorySaveTypes.h"
#include "InvenzaSaveManager.generated.h"

class UInputAction;
class UInvenzaSaveGame;

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameLoaded);
#pragma endregion Delegates

UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaSaveManager : public UActorComponent, public IISaveLoadGame
{
	GENERATED_BODY()

public:
	UInvenzaSaveManager();

protected:
	// Unreal overrides
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

public:
	// Properties and variables
	UPROPERTY(BlueprintAssignable, Category = "Save System|Events")
	FOnGameLoaded OnGameLoaded;

	// Functions
	virtual void SaveGame_Implementation(bool Async = false) override;

	virtual void LoadGame_Implementation(bool Async = false) override;

	UFUNCTION(BlueprintCallable, Category = "Save System")
	void ShowSaveMenu();

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save System|Inputs")
	TObjectPtr<UInputAction> SaveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save System|Inputs")
	TObjectPtr<UInputAction> LoadAction;

	/** Legacy fields remain readable by existing Blueprints; profiles are now configured globally. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save System|Legacy",
			  meta = (DeprecatedProperty, DeprecationMessage = "Use Invenza Save System project settings"))
	FName SaveSlotName = TEXT("InventorySave");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Save System|Legacy")
	int32 SaveUserIndex = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Save System|Legacy")
	TObjectPtr<UInvenzaSaveGame> LoadedSaveData;

	// Functions
	UFUNCTION()
	void OnInitializationComplete();

	UFUNCTION()
	void SessionLoaded();
};

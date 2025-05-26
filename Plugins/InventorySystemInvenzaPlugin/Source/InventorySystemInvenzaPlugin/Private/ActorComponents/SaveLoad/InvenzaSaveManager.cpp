// Nublin Studio 2025 All Rights Reserved.


#include "ActorComponents/SaveLoad/InvenzaSaveManager.h"

#include "EnhancedInputComponent.h"
#include "JsonObjectConverter.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "SaveLoad/InvenzaSaveGame.h"

UInvenzaSaveManager::UInvenzaSaveManager()
{
}

void UInvenzaSaveManager::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;
	
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PC->InputComponent))
	{
		if (SaveAction)
		{
			EnhancedInput->BindAction(SaveAction, ETriggerEvent::Started, this, &UInvenzaSaveManager::HandleSaveInput);
		}
		if (LoadAction)
		{
			EnhancedInput->BindAction(LoadAction, ETriggerEvent::Started, this, &UInvenzaSaveManager::HandleLoadInput);
		}
	}

	auto InvManager = GetOwner()->FindComponentByClass<UIInventoryManager>();
	if (!InvManager) return;

	InvManager->OnInitializationCompleteDelegate.AddDynamic(this, &UInvenzaSaveManager::OnInitializationComplete);

	//LoadGame_Implementation(false);
}

void UInvenzaSaveManager::OnInitializationComplete()
{
	LoadGame_Implementation(false);
}

void UInvenzaSaveManager::SaveGame_Implementation(bool Async)
{
	if (Async)
	{
		UGameplayStatics::AsyncSaveGameToSlot(LoadedSaveData,SaveSlotName.ToString(), SaveUserIndex );
	}
	else
	{
		UGameplayStatics::SaveGameToSlot(LoadedSaveData,SaveSlotName.ToString(), SaveUserIndex);
		UE_LOG(LogTemp, Warning, TEXT("Saved to slot: %s"), *SaveSlotName.ToString());

		/*FString DebugJson;
		FJsonObjectConverter::UStructToJsonObjectString(LoadedSaveData->PlayerSavedInventories, DebugJson);
		FFileHelper::SaveStringToFile(DebugJson, *(FPaths::ProjectSavedDir() + TEXT("InventoryDebug.json")));*/
	}
}

void UInvenzaSaveManager::LoadGame_Implementation(bool Async)
{
	bool bExists = UGameplayStatics::DoesSaveGameExist(SaveSlotName.ToString(), SaveUserIndex);

	if (!bExists)
	{
		TObjectPtr<UInvenzaSaveGame> InvenzaSaveGame = Cast<UInvenzaSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UInvenzaSaveGame::StaticClass()));

		LoadedSaveData = InvenzaSaveGame;
		UGameplayStatics::SaveGameToSlot(InvenzaSaveGame.Get(), SaveSlotName.ToString(), SaveUserIndex);
		
	}

	if (Async)
	{
		UGameplayStatics::AsyncLoadGameFromSlot(
		SaveSlotName.ToString(),
		SaveUserIndex,
		FAsyncLoadGameFromSlotDelegate::CreateUObject(this, &UInvenzaSaveManager::OnSaveGameLoaded));
	}
	else
	{
		auto Savegame = UGameplayStatics::LoadGameFromSlot(SaveSlotName.ToString(), SaveUserIndex);
		OnSaveGameLoaded(SaveSlotName.ToString(), SaveUserIndex, Savegame);
	}
	
}

void UInvenzaSaveManager::OnSaveGameLoaded(const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGame)
{
	UInvenzaSaveGame* Loaded = Cast<UInvenzaSaveGame>(LoadedGame);
	if (!Loaded){
		UE_LOG(LogTemp, Error, TEXT("Failed to load save or incorrect class."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Save loaded from slot: %s"), *SlotName);
	LoadedSaveData = Loaded;

	auto Collection = GetOwner()->FindComponentByClass<UItemCollection>();
	if (!Collection)
	{
		return;
	}

	if (!LoadedSaveData->PlayerSavedInventories.SavedItemLocations.IsEmpty())
	{
		Collection->DeserializeFromSave(LoadedSaveData->PlayerSavedInventories.SavedItemLocations);
	}

	if (OnGameLoaded.IsBound())
		OnGameLoaded.Broadcast();

}

void UInvenzaSaveManager::HandleSaveInput()
{
	auto Collection = GetOwner()->FindComponentByClass<UItemCollection>();
	if (!Collection)
	{
		return;
	}

	Collection->SerializeForSave(LoadedSaveData->PlayerSavedInventories.SavedItemLocations);
	SaveGame_Implementation(false);
}

void UInvenzaSaveManager::HandleLoadInput()
{
	LoadGame_Implementation(false);
}

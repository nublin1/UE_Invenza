//  Nublin Studio 2026 All Rights Reserved.


#include "Subsystems/InvenzaInventorySettingsSubsystem.h"
#include "Data/Settings/InvenzaInventoryUISettingsAsset.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"

void UInvenzaInventorySettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	 auto DevSettings = GetDefault<UInvenzaInventoryDeveloperSettings>();

	if (DevSettings && !DevSettings->InventorySettingsAsset.IsNull())
	{
		Settings = DevSettings->InventorySettingsAsset.LoadSynchronous();
	}
}

const UInvenzaInventoryUISettingsAsset* UInvenzaInventorySettingsSubsystem::GetSettingsStatic(
	const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetSettingsStatic: Failed to get World from ContextObject."));
		return nullptr;
	}
	
	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetSettingsStatic: GameInstance is null."));
		return nullptr;
	}
	
	auto* Subsystem = GI->GetSubsystem<UInvenzaInventorySettingsSubsystem>();
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetSettingsStatic: Could not find UInvenzaInventorySettingsSubsystem."));
		return nullptr;
	}
	
	const UInvenzaInventoryUISettingsAsset* Settings = Subsystem->GetSettings();
	if (!Settings)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetSettingsStatic: Settings asset is null inside the subsystem."));
		return nullptr;
	}

	return Settings;
}

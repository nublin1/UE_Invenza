//  Nublin Studio 2026 All Rights Reserved.


#include "Subsystems/InvenzaInventorySettingsSubsystem.h"

#include "Settings/InvenzaInventoryDeveloperSettings.h"

void UInvenzaInventorySettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UInvenzaInventoryDeveloperSettings* DevSettings = GetDefault<UInvenzaInventoryDeveloperSettings>();

	if (DevSettings && DevSettings->InventorySettingsAsset.IsValid())
	{
		Settings = DevSettings->InventorySettingsAsset.LoadSynchronous();
	}
}

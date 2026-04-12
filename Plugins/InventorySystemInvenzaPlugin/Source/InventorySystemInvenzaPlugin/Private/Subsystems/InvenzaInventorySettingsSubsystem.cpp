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

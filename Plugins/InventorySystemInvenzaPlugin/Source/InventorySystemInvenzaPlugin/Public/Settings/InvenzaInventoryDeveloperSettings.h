//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InvenzaInventoryDeveloperSettings.generated.h"

class UInvenzaInventorySettingsAsset;
/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Invenza Inventory"))
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaInventoryDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UInvenzaInventorySettingsAsset> InventorySettingsAsset;
};

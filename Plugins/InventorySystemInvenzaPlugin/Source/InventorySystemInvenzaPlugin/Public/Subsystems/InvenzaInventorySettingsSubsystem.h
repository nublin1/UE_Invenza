//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InvenzaInventorySettingsSubsystem.generated.h"

class UInvenzaInventoryUISettingsAsset;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaInventorySettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	const UInvenzaInventoryUISettingsAsset* GetSettings() const { return Settings; }

	static const UInvenzaInventoryUISettingsAsset* GetSettingsStatic(const UObject* WorldContextObject);

private:

	UPROPERTY()
	TObjectPtr<UInvenzaInventoryUISettingsAsset> Settings;
};

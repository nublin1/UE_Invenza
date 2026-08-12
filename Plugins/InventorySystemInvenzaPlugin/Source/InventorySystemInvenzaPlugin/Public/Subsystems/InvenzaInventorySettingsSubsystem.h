//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InvenzaInventorySettingsSubsystem.generated.h"

class UInvenzaInventorySettingsAsset;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaInventorySettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UInvenzaInventorySettingsAsset* GetSettings() const { return Settings; }

	static const UInvenzaInventorySettingsAsset* GetSettingsStatic(const UObject* WorldContextObject);

private:

	UPROPERTY()
	TObjectPtr<UInvenzaInventorySettingsAsset> Settings;
	
	UFUNCTION()
	void ValidateSettings() const;
};

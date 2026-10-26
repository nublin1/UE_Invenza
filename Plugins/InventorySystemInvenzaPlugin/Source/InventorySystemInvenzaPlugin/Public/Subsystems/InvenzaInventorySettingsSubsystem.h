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

protected:
	// Unreal overrides
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:
	// Functions
	UInvenzaInventorySettingsAsset* GetSettings() const { return Settings; }

	static const UInvenzaInventorySettingsAsset* GetSettingsStatic(const UObject* WorldContextObject);

protected:
	// Runtime
	// Properties and variables
	UPROPERTY()
	TObjectPtr<UInvenzaInventorySettingsAsset> Settings;

	// Functions
	UFUNCTION()
	void ValidateSettings() const;
};

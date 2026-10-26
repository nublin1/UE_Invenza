//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "SaveLoad/InventorySaveTypes.h"
#include "InvenzaSaveSettings.generated.h"

class UInvenzaSaveWidget;
class UStringTable;

UCLASS(Config = InventorySystemInvenzaPlugin, DefaultConfig, meta = (DisplayName = "Invenza Save System"))
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaSaveSettings : public UDeveloperSettings
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual FName GetCategoryName() const override;

public:
	// Properties and variables
	UPROPERTY(Config, EditAnywhere, Category = "Profile")
	FString Profile = TEXT("Default");

	UPROPERTY(Config, EditAnywhere, Category = "Automatic")
	bool bAutoLoad = true;

	UPROPERTY(Config, EditAnywhere, Category = "Automatic")
	bool bAutoSave = true;

	UPROPERTY(Config, EditAnywhere, Category = "Automatic", meta = (ClampMin = "10", Units = "s"))
	float AutoSaveSeconds = 120.f;

	UPROPERTY(Config, EditAnywhere, Category = "Automatic", meta = (ClampMin = "1", ClampMax = "10"))
	int32 AutoSaveCount = 3;

	UPROPERTY(Config, EditAnywhere, Category = "Automatic")
	bool bSaveOnExit = true;

	/** PIE uses a separate profile prefix, so editor testing cannot overwrite packaged-game saves. */
	UPROPERTY(Config, EditAnywhere, Category = "Editor")
	bool bEnableInPIE = true;

	/** Controlled by the host. Both modes retain ownership, access and structural validation. */
	UPROPERTY(Config, EditAnywhere, Category = "Network")
	EInventoryClientTrust ClientTrust = EInventoryClientTrust::ServerValidated;

	UPROPERTY(Config, EditAnywhere, Category = "Interface")
	TSoftClassPtr<UInvenzaSaveWidget> SaveMenuClass;

	UPROPERTY(Config, EditAnywhere, Category = "Localization")
	TSoftObjectPtr<UStringTable> TextTable;

	// Functions
	UFUNCTION(BlueprintPure, Category = "Invenza|Save")
	FText GetSaveText(FName Key) const;
};

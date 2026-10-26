//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "InvenzaGameUserSettings.generated.h"

/** Player preferences, separate from the inventory plugin's developer settings. */
UCLASS(Config = GameUserSettings)
class UE_GRIDINVENTORY_API UInvenzaGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

protected:
	virtual void SetToDefaults() override;
	virtual void ApplyNonResolutionSettings() override;

public:
	// Properties and variables
	UPROPERTY(Config, BlueprintReadWrite, Category = "Audio")
	float MasterVolume = 1.f;
	UPROPERTY(Config, BlueprintReadWrite, Category = "Audio")
	float MusicVolume = 1.f;
	UPROPERTY(Config, BlueprintReadWrite, Category = "Audio")
	float EffectsVolume = 1.f;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void ApplyAudio(UObject* WorldContext);
};

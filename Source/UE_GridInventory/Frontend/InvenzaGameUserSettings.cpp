//  Nublin Studio 2026 All Rights Reserved.

#include "Frontend/InvenzaGameUserSettings.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

void UInvenzaGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();
	MasterVolume = MusicVolume = EffectsVolume = 1.f;
}

void UInvenzaGameUserSettings::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();
	if (GEngine && GEngine->GameViewport)
	{
		ApplyAudio(GEngine->GameViewport);
	}
}

void UInvenzaGameUserSettings::ApplyAudio(UObject* WorldContext)
{
	MasterVolume = FMath::Clamp(MasterVolume, 0.f, 1.f);
	MusicVolume = FMath::Clamp(MusicVolume, 0.f, 1.f);
	EffectsVolume = FMath::Clamp(EffectsVolume, 0.f, 1.f);
	auto* Mix = LoadObject<USoundMix>(nullptr, TEXT("/Game/Frontend/Audio/SM_UserSettings.SM_UserSettings"));
	auto* Master = LoadObject<USoundClass>(nullptr, TEXT("/Game/Frontend/Audio/SC_Master.SC_Master"));
	auto* Music = LoadObject<USoundClass>(nullptr, TEXT("/Game/Frontend/Audio/SC_Music.SC_Music"));
	auto* Effects = LoadObject<USoundClass>(nullptr, TEXT("/Game/Frontend/Audio/SC_Effects.SC_Effects"));
	if (!Mix || !Master || !Music || !Effects || !WorldContext)
	{
		return;
	}
	UGameplayStatics::SetSoundMixClassOverride(WorldContext, Mix, Master, MasterVolume, 1.f, 0.1f, true);
	UGameplayStatics::SetSoundMixClassOverride(WorldContext, Mix, Music, MusicVolume, 1.f, 0.1f, true);
	UGameplayStatics::SetSoundMixClassOverride(WorldContext, Mix, Effects, EffectsVolume, 1.f, 0.1f, true);
	UGameplayStatics::SetBaseSoundMix(WorldContext, Mix);
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventorySystemInvenzaPlugin.h"
#include "CoreGlobals.h"
#include "UObject/UObjectBase.h"
#include "GameplayTagsManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#if WITH_EDITOR
#include "GameDelegates.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#endif

#define LOCTEXT_NAMESPACE "FInventorySystemInvenzaPluginModule"

#if WITH_EDITOR
static void AddInvenzaSelectedThemeToCook(TConstArrayView<const ITargetPlatform*> TargetPlatforms,
	TArray<FName>& PackagesToCook, TArray<FName>& PackagesToNeverCook)
{
	const auto Theme = GetDefault<UInvenzaInventoryDeveloperSettings>()->UITheme.ToSoftObjectPath();
	if (!Theme.IsNull())
	{
		PackagesToCook.AddUnique(Theme.GetLongPackageFName());
	}
}
#endif

void FInventorySystemInvenzaPluginModule::StartupModule()
{
#if WITH_EDITOR
	ThemeCookDelegate = FGameDelegates::Get().GetModifyCookDelegate().AddStatic(&AddInvenzaSelectedThemeToCook);
#endif
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("InventorySystemInvenzaPlugin"));
	if (Plugin.IsValid())
	{
		GameplayTagConfigDirectory = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Config"), TEXT("Tags"));
		UGameplayTagsManager::Get().AddTagIniSearchPath(GameplayTagConfigDirectory);
	}
}

void FInventorySystemInvenzaPluginModule::ShutdownModule()
{
#if WITH_EDITOR
	FGameDelegates::Get().GetModifyCookDelegate().Remove(ThemeCookDelegate);
#endif
	// UObject subsystems may already be unavailable during engine shutdown.
	if (!GameplayTagConfigDirectory.IsEmpty() && UObjectInitialized() && !IsEngineExitRequested())
	{
		UGameplayTagsManager::Get().RemoveTagIniSearchPath(GameplayTagConfigDirectory);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FInventorySystemInvenzaPluginModule, InventorySystemInvenzaPlugin)

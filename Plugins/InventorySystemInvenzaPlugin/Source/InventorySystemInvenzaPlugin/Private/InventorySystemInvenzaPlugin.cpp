// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventorySystemInvenzaPlugin.h"
#include "CoreGlobals.h"
#include "UObject/UObjectBase.h"
#include "GameplayTagsManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "FInventorySystemInvenzaPluginModule"

void FInventorySystemInvenzaPluginModule::StartupModule()
{
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("InventorySystemInvenzaPlugin"));
	if (Plugin.IsValid())
	{
		GameplayTagConfigDirectory = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Config"), TEXT("Tags"));
		UGameplayTagsManager::Get().AddTagIniSearchPath(GameplayTagConfigDirectory);
	}
}

void FInventorySystemInvenzaPluginModule::ShutdownModule()
{
	// UObject subsystems may already be unavailable during engine shutdown.
	if (!GameplayTagConfigDirectory.IsEmpty() && UObjectInitialized() && !IsEngineExitRequested())
	{
		UGameplayTagsManager::Get().RemoveTagIniSearchPath(GameplayTagConfigDirectory);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FInventorySystemInvenzaPluginModule, InventorySystemInvenzaPlugin)

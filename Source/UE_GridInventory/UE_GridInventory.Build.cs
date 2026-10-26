// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UE_GridInventory : ModuleRules
{
	public UE_GridInventory(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "InventorySystemInvenzaPlugin", "UMG", "OnlineSubsystem", "OnlineSubsystemUtils" });
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "GameplayTags", "DeveloperSettings" });
	}
}

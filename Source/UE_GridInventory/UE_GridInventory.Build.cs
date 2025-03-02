// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UE_GridInventory : ModuleRules
{
	public UE_GridInventory(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}

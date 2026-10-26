// Nublin Studio 2026 All Rights Reserved.

using UnrealBuildTool;

public class InventorySystemInvenzaPlugin : ModuleRules
{
	public InventorySystemInvenzaPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Public headers expose widgets, input helpers, settings, tags and fast-array replication.
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UMG",
			"SlateCore",
			"EnhancedInput",
			"GameplayTags",
			"DeveloperSettings",
			"NetCore"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Slate",
			"CoreOnline",
			"Projects"
		});
	}
}

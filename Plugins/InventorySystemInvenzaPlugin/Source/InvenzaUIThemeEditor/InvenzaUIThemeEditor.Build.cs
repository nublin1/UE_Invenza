// Nublin Studio 2026 All Rights Reserved.
using UnrealBuildTool;
public class InvenzaUIThemeEditor : ModuleRules
{
	public InvenzaUIThemeEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "DataValidation" });
		PrivateDependencyModuleNames.AddRange(new[] { "InventorySystemInvenzaPlugin", "UnrealEd", "UMG", "UMGEditor", "AssetRegistry", "SlateCore" });
	}
}

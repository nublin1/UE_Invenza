// Nublin Studio 2026 All Rights Reserved.

using UnrealBuildTool;

public class InvenzaUIEditor : ModuleRules
{
    public InvenzaUIEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "UMG" });
        PrivateDependencyModuleNames.AddRange(new[] { "UnrealEd", "UMGEditor", "Slate", "SlateCore", "InputCore", "GameplayTags", "NetCore", "RenderCore", "RHI", "ImageWrapper", "InventorySystemInvenzaPlugin", "UE_GridInventory", "OnlineSubsystem" });
    }
}

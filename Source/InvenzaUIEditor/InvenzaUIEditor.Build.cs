using UnrealBuildTool;

public class InvenzaUIEditor : ModuleRules
{
    public InvenzaUIEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "UMG" });
        PrivateDependencyModuleNames.AddRange(new[] { "UnrealEd", "UMGEditor", "Slate", "SlateCore", "RenderCore", "ImageWrapper" });
    }
}

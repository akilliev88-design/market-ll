using UnrealBuildTool;

// Editor-only module: the Product Studio (Tools > Urun Studyosu). Never shipped with the game.
public class MarketSimStudio : ModuleRules
{
    public MarketSimStudio(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Core", "CoreUObject", "Engine", "InputCore", "RenderCore", "ApplicationCore",
            "Slate", "SlateCore", "ToolMenus", "UnrealEd", "EditorFramework",
            "AssetTools", "AssetRegistry", "DesktopPlatform", "ImageWrapper", "MaterialEditor",
            "MeshDescription", "StaticMeshDescription", "Json", "Projects",
            "MarketSim"
        });
    }
}

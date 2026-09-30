using UnrealBuildTool;
public class MirasMarket : ModuleRules
{
    public MirasMarket(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        // Headers live in the module root; expose them to MirasMarketStudio (editor module).
        PublicIncludePaths.Add(ModuleDirectory);
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore", "Json", "JsonUtilities", "AssetRegistry" });
        // In-game HUD (MarketHudWidget) is a Slate overlay.
        PrivateDependencyModuleNames.AddRange(new[] { "Slate", "SlateCore" });
        RuntimeDependencies.Add("$(ProjectDir)/Config/products.json", StagedFileType.NonUFS);
        RuntimeDependencies.Add("$(ProjectDir)/Config/planograms.json", StagedFileType.NonUFS);
        RuntimeDependencies.Add("$(ProjectDir)/Config/iller.json", StagedFileType.NonUFS); // province map of the Subeler page
    }
}

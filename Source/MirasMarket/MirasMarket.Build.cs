using UnrealBuildTool;
public class MirasMarket : ModuleRules
{
    public MirasMarket(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        // Headers live in the module root; expose them to MirasMarketStudio (editor module).
        PublicIncludePaths.Add(ModuleDirectory);
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore", "Json", "JsonUtilities" });
        RuntimeDependencies.Add("$(ProjectDir)/Config/products.json", StagedFileType.NonUFS);
    }
}

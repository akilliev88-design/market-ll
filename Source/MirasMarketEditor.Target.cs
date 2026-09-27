using UnrealBuildTool;
public class MirasMarketEditorTarget : TargetRules
{
    public MirasMarketEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.AddRange(new[] { "MirasMarket", "MirasMarketStudio" });
    }
}

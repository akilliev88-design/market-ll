using UnrealBuildTool;
public class MarketSimEditorTarget : TargetRules
{
    public MarketSimEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.AddRange(new[] { "MarketSim", "MarketSimStudio" });
    }
}

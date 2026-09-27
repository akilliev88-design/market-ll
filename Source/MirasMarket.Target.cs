using UnrealBuildTool;
public class MirasMarketTarget : TargetRules
{
    public MirasMarketTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("MirasMarket");
    }
}

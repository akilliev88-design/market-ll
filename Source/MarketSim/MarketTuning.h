#pragma once

#include "CoreMinimal.h"

// C10 (Codex C9: "change one constant at a time"): balance knobs the automated player can override for controlled
// experiments without a code change (commandlet: -Tune=Key=Value,Key=Value). The game itself never sets them; an
// unset knob is the constant written in the code. Tested (MarketSim.Tuning.*).
//   BranchCompetition   x the province's competition in a branch's pull (MarketBranches; 3.0)
//   RealWageGrowth      yearly real growth of the minimum wage (MarketPrices; 0.005 since C11)
//   RealSpend           elasticity of a branch shopper's basket to the real wage (MarketBranches; 0 = none)
namespace MarketTuning
{
    float Get(const TCHAR* Key, float Default);
    void Set(const FString& Key, float Value);
    void Reset();
    // "BranchCompetition=2.5,RealWageGrowth=0.005": the knobs set (unknown keys are kept and reported).
    int32 Apply(const FString& List);
    // "BranchCompetition=2.5, RealWageGrowth=0.005" for reports ("" = none).
    FString Describe();
}

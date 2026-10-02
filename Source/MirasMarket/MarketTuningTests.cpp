#include "MarketTuning.h"
#include "MarketPrices.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketTuningKnobsTest, "MirasMarket.Tuning.Knobs", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketTuningKnobsTest::RunTest(const FString& Parameters)
{
    // C10: an unset knob is the code's constant; the automated player can set one for a controlled experiment.
    MarketTuning::Reset();
    TestEqual(TEXT("Default"), MarketTuning::Get(TEXT("BranchCompetition"), 3.f), 3.f);
    const double Wage = MarketPrices::WageIndex(3000);
    TestEqual(TEXT("Two knobs"), MarketTuning::Apply(TEXT("BranchCompetition=2.5, RealWageGrowth=0.0")), 2);
    TestEqual(TEXT("Set"), MarketTuning::Get(TEXT("BranchCompetition"), 3.f), 2.5f);
    TestTrue(TEXT("Slower real wages"), MarketPrices::WageIndex(3000) < Wage);
    TestTrue(TEXT("Reported"), MarketTuning::Describe().Contains(TEXT("BranchCompetition=2.5")));
    MarketTuning::Reset();
    TestTrue(TEXT("Back to the code"), FMath::IsNearlyEqual(MarketPrices::WageIndex(3000), Wage));
    return true;
}

#endif

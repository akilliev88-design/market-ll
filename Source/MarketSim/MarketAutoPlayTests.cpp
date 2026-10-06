#include "MarketAutoPlay.h"
#include "MarketCountry.h"
#include "Misc/AutomationTest.h"
#include "HAL/PlatformTime.h"
#include <limits>

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketAutoPlayShort, "MarketSim.AutoPlay.Short", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketAutoPlayShort::RunTest(const FString& Parameters)
{
    TArray<FMarketProduct> Base; TArray<int32> Capacities; TArray<FString> Errors;
    if (!TestTrue(TEXT("Real catalog and shelf plan loaded"), MarketAutoPlay::LoadInputs(Base, Capacities, Errors))) return false;
    MarketAutoPlay::FOptions Options; Options.Days = 120; Options.Seeds = 1;
    const MarketAutoPlay::FReport Report = MarketAutoPlay::Run(Options, Base, Capacities);
    TestEqual(TEXT("Three styles"), Report.Runs.Num(), 3);
    TestEqual(TEXT("No setup error"), Report.Errors.Num(), 0);
    for (const MarketAutoPlay::FRun& Trial : Report.Runs)
    {
        TestEqual(*Trial.Profile, Trial.Daily.Num(), 120);
        TestEqual(TEXT("Money and stock boundaries hold"), Trial.AuditFailures, 0);
        TestEqual(TEXT("Weekly rows including final partial week"), Trial.Weekly.Num(), 18);
        TestTrue(TEXT("Bot places affordable replenishment orders"), Trial.Expenses.FindRef(TEXT("Mal alimi (stok yatirimi)")) > 0);
    }
    TestTrue(TEXT("Under 60 seconds"), Report.Seconds < 60);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketAutoPlayIntegrity, "MarketSim.AutoPlay.IntegrityAndDeterminism", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketAutoPlayIntegrity::RunTest(const FString& Parameters)
{
    FMarketProduct Product; Product.Id = TEXT("tea"); Product.Category = TEXT("cay-kahve"); Product.Cost = 100; Product.BasePrice = 200;
    const TArray<FMarketProduct> Base = {Product};
    MarketAutoPlay::FOptions Options; Options.Days = 8; Options.Seeds = 1; Options.bKeepFinalStates = true;
    const auto First = MarketAutoPlay::Run(Options, Base, {24});
    const auto Second = MarketAutoPlay::Run(Options, Base, {24});
    TestEqual(TEXT("Campaigns created"), First.Runs.Num(), 3);
    TestEqual(TEXT("Review snapshots returned only when requested"), First.FinalStates.Num(), 3);
    TestEqual(TEXT("Review snapshot is the real final day"), First.FinalStates[0].Day, 9);
    for (int32 Index = 0; Index < First.Runs.Num(); ++Index)
    {
        TestEqual(TEXT("Repeat seed cash"), First.Runs[Index].Daily.Last().Cash, Second.Runs[Index].Daily.Last().Cash);
        TestEqual(TEXT("Repeat seed profit"), First.Runs[Index].Daily.Last().Profit, Second.Runs[Index].Daily.Last().Profit);
    }
    FMarketState State; State.Initialize(Base);
    State.Stock[0].Shelf = -1;
    TestTrue(TEXT("Injected negative stock is detected"), MarketAutoPlay::Validate(State, Base).Num() > 0);
    State.Stock[0].Shelf = 0; State.MarketShare = std::numeric_limits<float>::quiet_NaN();
    TestTrue(TEXT("Injected NaN is detected"), MarketAutoPlay::Validate(State, Base).Num() > 0);
    return true;
}
#endif

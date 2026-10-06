#include "MarketAutoPlay.h"
#include "MarketCompany.h"
#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBotDiagnosis, "MarketSim.AutoPlay.SingleShopDiagnosis", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketBotDiagnosis::RunTest(const FString& Parameters)
{
    TArray<FMarketProduct> Base; TArray<int32> Capacities; TArray<FString> Errors;
    if (!TestTrue(TEXT("Real inputs loaded"), MarketAutoPlay::LoadInputs(Base, Capacities, Errors))) return false;
    MarketAutoPlay::FOptions Options; Options.Days=60; Options.Seeds=1; Options.bNoGrowth=true; Options.bKeepFinalStates=true;
    const auto Report=MarketAutoPlay::Run(Options, Base, Capacities);
    if (!TestEqual(TEXT("Only careful counterfactual"), Report.Runs.Num(), 1)) return false;
    const auto& Trial=Report.Runs[0]; const auto& State=Report.FinalStates[0];
    TestEqual(TEXT("One row per played day"), Trial.Diagnosis.Days.Num(),60);
    TestEqual(TEXT("No money or stock gap"), Trial.AuditFailures,0);
    TestEqual(TEXT("Single shop retained"), MarketCompany::TotalStores(State),1);
    TestTrue(TEXT("Monthly product snapshots exist"), !Trial.Diagnosis.Products.IsEmpty());
    TestTrue(TEXT("Roster decisions observable"), !Trial.Diagnosis.Events.IsEmpty());
    TestEqual(TEXT("No voluntary company loan"), Trial.Finance.Commands.FindRef(TEXT("CorpLoan")),0);
    TestEqual(TEXT("No voluntary personal loan"), Trial.Finance.Commands.FindRef(TEXT("TakeLoan")),0);
    const auto Repeat=MarketAutoPlay::Run(Options, Base, Capacities);
    TestEqual(TEXT("Observer and isolated policy deterministic"),Trial.Daily.Last().Cash,Repeat.Runs[0].Daily.Last().Cash);
    return true;
}
#endif

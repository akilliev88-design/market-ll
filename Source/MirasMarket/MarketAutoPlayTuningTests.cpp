#include "MarketAutoPlay.h"
#include "MarketTuning.h"
#include "MarketBranches.h"
#include "MarketFinance.h"
#include "MarketLedger.h"
#include "MarketManagers.h"
#include "MarketOwner.h"
#include "MarketPrices.h"
#include "Misc/AutomationTest.h"
#include "Misc/Parse.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBotExperimentKnobs,"MirasMarket.AutoPlay.ExperimentKnobs",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketBotExperimentKnobs::RunTest(const FString& Parameters)
{
    FString Error;
    TestTrue(TEXT("Command-line comma list"),MarketAutoPlay::ConfigureTuningParams(TEXT("-Tune=BranchCompetition=2.5,RealWageGrowth=0.005,RealSpend=0.7 -Years=10"),Error));
    TestEqual(TEXT("First knob"),MarketTuning::Get(TEXT("BranchCompetition"),3.f),2.5f);
    TestEqual(TEXT("Second knob"),MarketTuning::Get(TEXT("RealWageGrowth"),.015f),.005f);
    TestEqual(TEXT("Third knob"),MarketTuning::Get(TEXT("RealSpend"),0.f),.7f);
    // The parser correction is identical for every completed single-knob experiment.
    for(const TCHAR* List:{TEXT("BranchCompetition=2.5"),TEXT("RealWageGrowth=0.005"),TEXT("RealSpend=0.7"),TEXT("OpenBuffer.Balanced=1.0"),TEXT("OpenBuffer.Careful=2.0"),TEXT("LossMonthsToClose=4")})
    {
        const FString Params=FString(TEXT("-Tune="))+List+TEXT(" -Years=10");FString Before,After;
        FParse::Value(*Params,TEXT("Tune="),Before);FParse::Value(*Params,TEXT("Tune="),After,false);
        TestEqual(TEXT("Completed single-knob input unchanged"),After,Before);
    }
    TestTrue(TEXT("Default run"),MarketAutoPlay::ConfigureTuning(TEXT(""),Error));
    const auto Defaults=MarketAutoPlay::TunedProfiles();
    TestTrue(TEXT("One buffer override"),MarketAutoPlay::ConfigureTuning(TEXT("OpenBuffer.Balanced=1.0"),Error));
    const auto Tuned=MarketAutoPlay::TunedProfiles();
    TestEqual(TEXT("Balanced only"),Tuned[1].ExpansionBuffer,1.0);
    TestEqual(TEXT("Careful unchanged"),Tuned[0].ExpansionBuffer,Defaults[0].ExpansionBuffer);
    TestEqual(TEXT("Bold unchanged"),Tuned[2].ExpansionBuffer,Defaults[2].ExpansionBuffer);
    FMarketBranch Branch;Branch.Stage=static_cast<uint8>(MarketBranches::EStage::Open);Branch.OpenedDay=1;Branch.Last30Profit=-1;
    int32 Red=0;
    TestTrue(TEXT("Four red months"),MarketAutoPlay::ConfigureTuning(TEXT("LossMonthsToClose=4"),Error));
    for(int32 I=0;I<3;++I)TestFalse(TEXT("No early closure"),MarketAutoPlayFinance::LosingMonth(Branch,200,Red));
    TestTrue(TEXT("Fourth month closes"),MarketAutoPlayFinance::LosingMonth(Branch,200,Red));
    TestFalse(TEXT("Typo cannot silently become default"),MarketAutoPlay::ConfigureTuning(TEXT("RealSpend=0.7,Unknown=2"),Error));
    TestTrue(TEXT("Rejected list leaves no partial override"),MarketTuning::Describe().IsEmpty());
    TestFalse(TEXT("Duplicate knob rejected"),MarketAutoPlay::ConfigureTuning(TEXT("RealSpend=0.7,RealSpend=0.5"),Error));
    TestFalse(TEXT("Fractional month rejected"),MarketAutoPlay::ConfigureTuning(TEXT("LossMonthsToClose=2.5"),Error));
    TestTrue(TEXT("Reset for next campaign"),MarketAutoPlay::ConfigureTuning(TEXT(""),Error));
    TestEqual(TEXT("No buffer leaks"),MarketAutoPlay::TunedProfiles()[1].ExpansionBuffer,Defaults[1].ExpansionBuffer);
    Red=1;TestTrue(TEXT("Default two months restored"),MarketAutoPlayFinance::LosingMonth(Branch,200,Red));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketC10CostAllocation,"MirasMarket.AutoPlay.C10CostAllocation",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketC10CostAllocation::RunTest(const FString& Parameters)
{
    using namespace MarketLedger;
    TArray<FMarketProduct> Products;
    FMarketState Baseline;Baseline.Initialize(Products);Baseline.Day=10;Baseline.Cash=1000000;
    FMarketState Costs=Baseline;
    AddStoreCost(Costs,1700,0);AddStoreCost(Costs,2300,HeadOfficeStore);
    Baseline.CloseDay();BeginClose(Baseline,Products);EndClose(Baseline);
    Costs.CloseDay();BeginClose(Costs,Products);EndClose(Costs);
    TestEqual(TEXT("Same family result despite branch fitout"),Statement(Costs,10,10,FirstStore).NetProfit,Statement(Baseline,10,10,FirstStore).NetProfit);
    TestEqual(TEXT("Branch owns cost"),Statement(Costs,10,10,0).At(EAccount::Marketing),int64(-1700));
    TestEqual(TEXT("Head office owns cost"),Statement(Costs,10,10,HeadOfficeStore).At(EAccount::Marketing),int64(-2300));
    TestEqual(TEXT("Allocation preserves actual cash expense"),Costs.Cash-Baseline.Cash,int64(-4000));
    TestEqual(TEXT("Pending consumed"),Costs.Ledger.PendingStoreCosts.Num(),0);
    Baseline.CloseDay();BeginClose(Baseline,Products);EndClose(Baseline);
    Costs.CloseDay();BeginClose(Costs,Products);EndClose(Costs);
    TestEqual(TEXT("No second cash payment"),Costs.Cash-Baseline.Cash,int64(-4000));
    TestEqual(TEXT("No next-day duplicate"),Statement(Costs,11,11,0).At(EAccount::Marketing),int64(0));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketC10RescueAssets,"MirasMarket.AutoPlay.C10RescueAssets",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketC10RescueAssets::RunTest(const FString& Parameters)
{
    FMarketState S;TArray<FMarketProduct> Products;S.Initialize(Products);S.Day=400;S.Cash=-1000000;
    S.CountryId=TEXT("tr");S.CityId=TEXT("kirklareli");S.Company.Trucks=2;
    FMarketDepot Depot;S.Company.DepotSites.Add(Depot);
    FMarketManager Manager;Manager.Level=static_cast<uint8>(MarketManagers::ELevel::Depot);Manager.Name=TEXT("Depot manager");S.Management.Managers.Add(Manager);
    MarketFinance::Rescue(S,Products);
    TestEqual(TEXT("No idle depot"),S.Company.DepotSites.Num(),0);
    TestEqual(TEXT("No idle truck"),S.Company.Trucks,0);
    TestEqual(TEXT("No idle manager"),S.Management.Managers.Num(),0);
    const int64 Sale=MarketLedger::Statement(S,400,400,MarketLedger::HeadOfficeStore).At(MarketLedger::EAccount::Divestment);
    TestEqual(TEXT("Two trucks sold at forty percent"),Sale,FMath::RoundToInt64(MarketFinance::RescueTruckPrice*.4*MarketPrices::ListLevel(S.Day))*2);
    TestTrue(TEXT("Three months of remaining operating costs"),S.Cash>=3*(MarketFinance::CompanyMonthCost(S)+MarketOwner::CompanyCost(S)));
    S.Cash=-10000;MarketFinance::Rescue(S,Products);
    TestEqual(TEXT("Already sold assets never sell again"),MarketLedger::Statement(S,400,400,MarketLedger::HeadOfficeStore).At(MarketLedger::EAccount::Divestment),Sale);
    return true;
}
#endif

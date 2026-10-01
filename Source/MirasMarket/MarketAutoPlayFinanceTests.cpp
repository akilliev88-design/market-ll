#include "MarketAutoPlayFinance.h"
#include "MarketBranches.h"
#include "MarketLedger.h"
#include "MarketStaff.h"
#include "MarketPrices.h"
#include "MarketOnline.h"
#include "MarketAutoPlayRescue.h"
#include "MarketAutoPlayCommand.h"
#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBotNetworkReserve,"MirasMarket.AutoPlay.NetworkReserve",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketBotNetworkReserve::RunTest(const FString& Parameters)
{
    FMarketState State; State.Day=120; State.CountryId=TEXT("tr"); State.CityId=TEXT("kirklareli");
    const int64 Family=MarketAutoPlayFinance::NetworkReserve(State);
    FMarketBranch Branch; Branch.Country=State.CountryId; Branch.Province=State.CityId; Branch.Format=TEXT("mahalle"); Branch.Stage=static_cast<uint8>(MarketBranches::EStage::Open);
    State.Branches.Add(Branch);
    const int64 Monthly=MarketBranches::MonthlyFixedCost(State,Branch.Country,Branch.Province,Branch.Format);
    TestEqual(TEXT("Reserve includes every branch"),MarketAutoPlayFinance::NetworkReserve(State),Family+Monthly);
    State.Cash=Family+Monthly*2+100000-1;
    TestFalse(TEXT("One cent below new network reserve cannot expand"),MarketAutoPlayFinance::CanExpand(State,100000,Monthly,1));
    ++State.Cash;
    TestTrue(TEXT("Exact post-opening network reserve can expand"),MarketAutoPlayFinance::CanExpand(State,100000,Monthly,1));
    FMarketEmployee Employee; Employee.DailyWage=10000; State.Staff.Add(Employee);
    TestEqual(TEXT("Family payroll reserves employer insurance too"),MarketAutoPlayFinance::NetworkReserve(State),Family+Monthly+30*(int64(10000)+MarketStaff::EmployerShare(10000)));
    const int64 BeforeTruck=MarketAutoPlayFinance::NetworkReserve(State); State.Company.Trucks=1;
    TestEqual(TEXT("Reserve also pays idle fleet overhead"),MarketAutoPlayFinance::NetworkReserve(State),BeforeTruck+30*MarketPrices::Scaled(6000,State.Day));
    const int64 BeforeWeb=MarketAutoPlayFinance::NetworkReserve(State);State.Online.bWeb=true;
    TestEqual(TEXT("Reserve includes web monthly bill"),MarketAutoPlayFinance::NetworkReserve(State),BeforeWeb+MarketPrices::Scaled(MarketOnline::WebMonthly,State.Day));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBotCloseLoss,"MirasMarket.AutoPlay.TwoLosingMonths",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketBotCloseLoss::RunTest(const FString& Parameters)
{
    FMarketBranch Branch; Branch.Stage=static_cast<uint8>(MarketBranches::EStage::Open); Branch.OpenedDay=1; Branch.Last30Profit=-100;
    int32 Red=0;
    TestFalse(TEXT("Young shop gets time to mature"),MarketAutoPlayFinance::LosingMonth(Branch,80,Red));
    TestFalse(TEXT("One red mature month stays open"),MarketAutoPlayFinance::LosingMonth(Branch,100,Red));
    Branch.Last30Profit=1;
    TestFalse(TEXT("Recovery resets consecutive losses"),MarketAutoPlayFinance::LosingMonth(Branch,130,Red));
    Branch.Last30Profit=-100;
    TestFalse(TEXT("First new red month"),MarketAutoPlayFinance::LosingMonth(Branch,160,Red));
    TestTrue(TEXT("Second consecutive red month closes"),MarketAutoPlayFinance::LosingMonth(Branch,190,Red));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBotFirst180,"MirasMarket.AutoPlay.First180Books",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketBotFirst180::RunTest(const FString& Parameters)
{
    FMarketState State; State.Day=11; FMarketBranch Branch; Branch.OpenedDay=10; Branch.Format=TEXT("mahalle"); Branch.Stage=static_cast<uint8>(MarketBranches::EStage::Open); State.Branches.Add(Branch);
    State.Ledger.bClosing=true; State.Ledger.ClosingDay=10;
    MarketLedger::Post(State,MarketLedger::EAccount::Sales,10000,true,0);
    MarketLedger::Post(State,MarketLedger::EAccount::CostOfGoods,-6000,false,0);
    MarketLedger::Post(State,MarketLedger::EAccount::Rent,-500,true,0);
    MarketLedger::Post(State,MarketLedger::EAccount::Wages,-1000,true,0);
    MarketLedger::Post(State,MarketLedger::EAccount::SocialSecurity,-200,true,0);
    MarketLedger::Post(State,MarketLedger::EAccount::Investment,-99999,true,0);
    State.Rescues=1;
    MarketAutoPlayFinance::FStats Stats; MarketAutoPlayFinance::Observe(State,0,Stats); MarketAutoPlayFinance::Observe(State,0,Stats);
    TestEqual(TEXT("Net excludes opening investment and counts once"),Stats.Branches[0].Net,int64(2300));
    TestEqual(TEXT("Gross profit"),Stats.Branches[0].Gross,int64(4000));
    TestEqual(TEXT("Rescue counted once"),Stats.RescueDays.Num(),1);
    State.Day=191; State.Ledger.ClosingDay=190; MarketLedger::Post(State,MarketLedger::EAccount::Sales,10000,true,0);
    MarketAutoPlayFinance::Observe(State,0,Stats);
    TestEqual(TEXT("Day 181 outside first 180"),Stats.Branches[0].Revenue,int64(10000));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBotRescuePause,"MirasMarket.AutoPlay.RescuePause",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketBotRescuePause::RunTest(const FString& Parameters)
{
    FMarketState State;State.Day=121;State.RescueUntil=122;State.Cash=1000000000;
    TestTrue(TEXT("Rescue blocks before its exact end"),MarketAutoPlayRescue::Blocked(State));
    TestFalse(TEXT("Rich cash does not bypass plan"),MarketAutoPlayFinance::CanExpand(State,1,1,1));
    MarketAutoPlayFinance::FStats Stats;TArray<FMarketProduct> Products;
    MarketAutoPlayFinance::Decide(State,Products,true,false,Stats);
    TestEqual(TEXT("No banking attempt under plan"),Stats.Attempts.Num(),0);
    FMarketDecision Card;Card.Id=TEXT("command.open:tr|kirklareli");
    TestEqual(TEXT("Open proposal rejected without probing site"),MarketAutoPlayCommand::Choice(State,Products,Card,1),1);
    ++State.Day;TestFalse(TEXT("Exact end releases bot"),MarketAutoPlayRescue::Blocked(State));
    TestTrue(TEXT("Expansion resumes with reserve"),MarketAutoPlayFinance::CanExpand(State,1,1,1));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBotRescueBooks,"MirasMarket.AutoPlay.RescueLifecycle",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketBotRescueBooks::RunTest(const FString& Parameters)
{
    FMarketState State;State.Day=10;State.RescueUntil=20;
    FMarketLoan Old;Old.Principal=10000;Old.Remaining=10000;State.Loans.Add(Old);
    State.Ledger.bClosing=true;State.Ledger.ClosingDay=10;
    MarketLedger::Post(State,MarketLedger::EAccount::LoanIn,4000);
    MarketAutoPlayRescue::FStats Stats;MarketAutoPlayRescue::BeginDay(State,Stats);MarketAutoPlayRescue::BeginDay(State,Stats);
    TestEqual(TEXT("Blocked day once"),Stats.BlockedDays,1);
    MarketLedger::Post(State,MarketLedger::EAccount::LoanRepayment,-1000);
    MarketLedger::Post(State,MarketLedger::EAccount::Penalties,-200,false);
    MarketLedger::Post(State,MarketLedger::EAccount::LoanIn,3000);
    State.Loans[0].Principal=5000;State.Loans[0].Remaining=5000;State.Rescues=1;State.LastRevenue=100;++State.Day;
    MarketAutoPlayRescue::Observe(State,0,Stats);MarketAutoPlayRescue::Observe(State,0,Stats);
    TestEqual(TEXT("Write-off from loan balance conservation"),Stats.Plans[0].WrittenOff,int64(7200));
    TestEqual(TEXT("One rescue record"),Stats.Plans.Num(),1);
    MarketAutoPlayRescue::BeginDay(State,Stats);State.Loans.Reset();State.LastRevenue=300;++State.Day;
    FMarketBranch Branch;Branch.OpenedDay=11;State.Branches.Add(Branch);
    MarketAutoPlayRescue::Observe(State,1,Stats);
    TestEqual(TEXT("Repaid is explicit"),Stats.Plans[0].Paid,11);
    TestEqual(TEXT("Family year cash revenue counted once"),Stats.Years[0].FamilyRevenue,int64(400));
    // A replaced outstanding plan is never reported as paid.
    State.Loans.Add(Old);State.Rescues=2;MarketAutoPlayRescue::BeginDay(State,Stats);State.Rescues=3;++State.Day;
    MarketAutoPlayRescue::Observe(State,1,Stats);MarketAutoPlayRescue::BeginDay(State,Stats);State.Rescues=4;++State.Day;
    MarketAutoPlayRescue::Observe(State,1,Stats);
    TestEqual(TEXT("Replaced outstanding plan"),Stats.Plans[1].Replaced,13);
    TestEqual(TEXT("Replacement not repayment"),Stats.Plans[1].Paid,0);
    return true;
}
#endif

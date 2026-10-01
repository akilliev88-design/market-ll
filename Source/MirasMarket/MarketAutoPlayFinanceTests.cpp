#include "MarketAutoPlayFinance.h"
#include "MarketBranches.h"
#include "MarketLedger.h"
#include "MarketStaff.h"
#include "MarketPrices.h"
#include "MarketOnline.h"
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
#endif

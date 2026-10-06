#include "MarketAutoPlayOnline.h"
#include "MarketAutoPlay.h"
#include "MarketOnline.h"
#include "MarketCalendar.h"
#include "MarketCountry.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBotOnlineChoices,"MarketSim.AutoPlay.OnlineChoices",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketBotOnlineChoices::RunTest(const FString& Parameters)
{
    FMarketState State;State.CountryId=TEXT("tr");State.CityId=TEXT("kirklareli");State.Cash=MAX_int64/100;
    FMarketDecision Card;Card.Id=TEXT("online.app");
    for(int32 Style=0;Style<3;++Style)TestEqual(TEXT("Visible app tier follows style"),MarketAutoPlayOnline::Choice(State,Card,Style,false),Style);
    State.Cash=0;TestEqual(TEXT("Unaffordable app is postponed"),MarketAutoPlayOnline::Choice(State,Card,2,false),3);
    Card.Id=TEXT("online.commission");State.Online.Commission=.21f;
    TestEqual(TEXT("24 percent is accepted"),MarketAutoPlayOnline::Choice(State,Card,1,false),0);
    State.Online.Commission=.24f;TestEqual(TEXT("Above 24 percent leaves"),MarketAutoPlayOnline::Choice(State,Card,1,false),2);
    Card.Id=TEXT("online.pandemic");TestEqual(TEXT("Careful waits"),MarketAutoPlayOnline::Choice(State,Card,0,false),2);TestEqual(TEXT("Balanced joins"),MarketAutoPlayOnline::Choice(State,Card,1,false),0);
    FMarketOnlineArea Area;Area.Country=State.CountryId;Area.Province=State.CityId;State.Online.Areas.Add(Area);
    Card.Id=TEXT("online.area:tr|kirklareli");Card.Arg=8|7;
    const int64 Cost=MarketOnline::DarkStorePrice(State,0);State.Cash=Cost*5-1;
    TestEqual(TEXT("Depot proposal needs five times cost"),MarketAutoPlayOnline::Choice(State,Card,2,false),1);
    State.Cash=Cost*5;TestEqual(TEXT("Exact boundary accepts depot"),MarketAutoPlayOnline::Choice(State,Card,2,false),0);
    Card.Arg=0;TestEqual(TEXT("Losing area closure accepted"),MarketAutoPlayOnline::Choice(State,Card,1,false),0);
    TestEqual(TEXT("Offline area cannot reopen"),MarketAutoPlayOnline::Choice(State,Card,0,true),1);
    FMarketProduct Product;Product.Id=TEXT("tea");Product.Category=TEXT("cay-kahve");Product.Cost=100;Product.BasePrice=200;
    MarketAutoPlay::FOptions Options;Options.Days=8;Options.Seeds=1;Options.bOfflineCareful=true;Options.bKeepFinalStates=true;
    const auto Run=MarketAutoPlay::Run(Options,{Product},{24});
    TestEqual(TEXT("Counterfactual is only careful"),Run.Runs.Num(),1);
    TestTrue(TEXT("Offline marked explicitly"),Run.Runs[0].Online.Offline);
    TestFalse(TEXT("No hidden online opening"),Run.FinalStates[0].Online.bPlatform||Run.FinalStates[0].Online.bWeb);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBotOnlineBooks,"MarketSim.AutoPlay.OnlineMonthAndYearBooks",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketBotOnlineBooks::RunTest(const FString& Parameters)
{
    FMarketState State;State.CountryId=TEXT("tr");State.CityId=TEXT("kirklareli");
    State.Day=MarketCalendar::GameDayOf(MarketCalendar::StartYear+1,1,31);
    State.Online.MonthOrders={10,0,0,0};State.Online.MonthRevenue={10000,0,0,0};State.Online.MonthProfit={2000,0,0,0};
    MarketAutoPlayOnline::FStats Stats;MarketAutoPlayOnline::BeginDay(State,Stats);
    ++State.Day;State.Online.PrevOrders={12,0,0,0};State.Online.PrevRevenue={14000,0,0,0};State.Online.PrevProfit={3000,0,0,0};
    State.Online.MonthOrders.Init(0,4);State.Online.MonthRevenue.Init(0,4);State.Online.MonthProfit.Init(0,4);
    State.Online.LastProfit=700;State.Online.LastOrders=2;State.LastRevenue=6000;
    MarketAutoPlayOnline::Observe(State,1,20,Stats);
    TestEqual(TEXT("Month rollover keeps last day's orders"),Stats.Years[0].Orders[0],int64(2));
    TestEqual(TEXT("Channel revenue delta"),Stats.Years[0].Revenue[0],int64(4000));
    TestEqual(TEXT("Common overhead separately conserved"),Stats.Years[0].CommonCosts,int64(300));
    TestEqual(TEXT("Net includes overhead"),Stats.Years[0].TotalProfit,int64(700));
    MarketAutoPlayOnline::Observe(State,1,20,Stats);TestEqual(TEXT("Same day not counted twice"),Stats.Years.Num(),1);
    MarketAutoPlayOnline::BeginDay(State,Stats);++State.Day;State.Online.MonthOrders={0,0,1,0};State.Online.MonthRevenue={0,0,500,0};State.Online.MonthProfit={0,0,100,0};State.Online.LastProfit=50;
    MarketAutoPlayOnline::Observe(State,1,5,Stats);
    TestEqual(TEXT("Next month enters next campaign year"),Stats.Current.Orders[2],int64(1));
    TestEqual(TEXT("Year total unchanged"),Stats.Years[0].Orders[2],int64(0));
    return true;
}
#endif

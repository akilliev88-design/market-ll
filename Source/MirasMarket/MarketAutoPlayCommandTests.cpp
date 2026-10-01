#include "MarketAutoPlayCommand.h"
#include "MarketAutoPlayFinance.h"
#include "MarketAdvertising.h"
#include "MarketCalendar.h"
#include "MarketCountry.h"
#include "MarketBranches.h"
#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBotCommandChoices,"MirasMarket.AutoPlay.CommandAndAdvertisingChoices",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketBotCommandChoices::RunTest(const FString& Parameters)
{
    using namespace MarketAutoPlayCommand;
    for(int32 Channel=0;Channel<6;++Channel)TestEqual(TEXT("Careful never advertises"),Desired(0,50,Channel,true),0);
    TestEqual(TEXT("Balanced waits for ten shops"),Desired(1,9,3,true),0);
    TestEqual(TEXT("Balanced print at ten"),Desired(1,10,3,false),1);
    TestEqual(TEXT("Balanced social at ten"),Desired(1,10,4,false),1);
    TestEqual(TEXT("Search only with online"),Desired(1,10,5,false),0);
    TestEqual(TEXT("Balanced search"),Desired(1,10,5,true),1);
    TestEqual(TEXT("Bold TV waits for thirty"),Desired(2,29,0,true),0);
    TestEqual(TEXT("Bold TV"),Desired(2,30,0,true),1);
    TestEqual(TEXT("Bold radio"),Desired(2,30,1,true),1);
    TestEqual(TEXT("Bold social"),Desired(2,30,4,true),2);
    FMarketState State;State.CountryId=TEXT("tr");State.CityId=TEXT("kirklareli");State.Day=5006;State.Cash=MAX_int64/100;
    FMarketBranch Branch;Branch.Country=State.CountryId;Branch.Province=State.CityId;Branch.Format=TEXT("mahalle");Branch.Stage=static_cast<uint8>(MarketBranches::EStage::Open);Branch.Last30Profit=-1;
    State.Branches.Add(Branch);FMarketDecision Card;Card.Id=TEXT("command.close:0");Card.Arg=0;
    State.Branches[0].LossMonths=3;TestEqual(TEXT("Three months is not longer than three"),Choice(State,{},Card,1.5),1);
    State.Branches[0].LossMonths=4;TestEqual(TEXT("Longer loss is accepted"),Choice(State,{},Card,1.5),0);
    State.Branches[0].Last30Profit=1;TestEqual(TEXT("Recovered branch stays"),Choice(State,{},Card,1.5),1);
    Card.Id=TEXT("command.open:tr|kirklareli");Card.Arg=99;TestEqual(TEXT("Invalid opening rejected"),Choice(State,{},Card,1.5),1);
    for(int32 BranchIndex=1;BranchIndex<20;++BranchIndex)State.Branches.Add(Branch);
    FStats Stats;Decide(State,{},2,Stats);
    TestFalse(TEXT("Twenty-shop advertising manager hired"),State.Advertising.ManagerName.IsEmpty());
    TestTrue(TEXT("Manager owns the mix"),State.Advertising.bAuto);
    TestEqual(TEXT("Bold manager budget thirty permille"),State.Advertising.BudgetPermille,30);
    const int32 EventsBefore=Stats.Events.Num();Decide(State,{},2,Stats);TestEqual(TEXT("Same-day decision idempotent"),Stats.Events.Num(),EventsBefore);
    FMarketAdCountry Ad;Ad.Country=State.CountryId;Ad.Levels.Init(0,6);Ad.Levels[0]=1;State.Advertising.Countries.Add(Ad);
    const int64 Before=MarketAutoPlayFinance::NetworkReserve(State);State.Advertising.Countries[0].Levels[0]=0;
    TestEqual(TEXT("Network reserve includes TV commitment"),Before-MarketAutoPlayFinance::NetworkReserve(State),MarketAdvertising::MonthCost(State,State.CountryId,MarketAdvertising::EChannel::TV,1));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBotCommandBooks,"MirasMarket.AutoPlay.AdvertisingBooksAndClearance",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketBotCommandBooks::RunTest(const FString& Parameters)
{
    using namespace MarketAutoPlayCommand;
    FMarketState State;State.CountryId=TEXT("tr");State.CityId=TEXT("kirklareli");State.Day=MarketCalendar::GameDayOf(2012,1,31);
    FMarketAdCountry Ad;Ad.Country=State.CountryId;Ad.MonthChannelSpend={100,200,0,0,0,10};Ad.MonthUplift=200;Ad.Stock={20,10,0,0,0,7};State.Advertising.Countries.Add(Ad);
    FMarketBranch Branch;FMarketBranchItem Item;Item.ProductId=TEXT("tea");Branch.Items.Add(Item);State.Branches.Add(Branch);
    FMarketCompetitor Rival;Rival.Company=1;State.Competitors.Add(Rival);
    FStats Stats;BeginDay(State,Stats);++State.Day;State.Competitors[0].ChainId=TEXT("#gone");
    auto& Country=State.Advertising.Countries[0];Country.PrevChannelSpend={150,230,0,0,0,20};Country.PrevUplift=1100;Country.MonthChannelSpend.Init(0,6);Country.MonthUplift=0;
    State.Branches[0].Items[0].Markdown=20;State.Branches[0].Items[0].MarkdownUntil=State.Day-1+7;
    Observe(State,0,Stats);
    TestEqual(TEXT("Month rollover retains TV spend"),Stats.Years[0].Spend[0],int64(50));
    TestEqual(TEXT("Search expense independent"),Stats.Years[0].Spend[5],int64(10));
    TestEqual(TEXT("Estimated physical uplift is conserved"),Stats.Years[0].Estimate[0]+Stats.Years[0].Estimate[1],int64(900));
    TestEqual(TEXT("Search is not attributed walk-in revenue"),Stats.Years[0].Estimate[5],int64(0));
    TestEqual(TEXT("New clearance counted"),Stats.ClearanceItems,1);
    TestEqual(TEXT("Initial state is not a closure"),Stats.Events.FilterByPredicate([](const FString& Line){return Line.Contains(TEXT(",sokak_kapandi,"));}).Num(),1);
    Observe(State,0,Stats);TestEqual(TEXT("No double counting"),Stats.ClearanceItems,1);
    BeginDay(State,Stats);++State.Day;Country.MonthChannelSpend[0]=7;Country.MonthUplift=30;Observe(State,0,Stats);
    TestEqual(TEXT("Next day expense added once"),Stats.Years[0].Spend[0],int64(57));
    TestEqual(TEXT("Ongoing markdown not counted again"),Stats.ClearanceItems,1);
    return true;
}
#endif

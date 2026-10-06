#include "MarketAutoPlayC.h"
#include "MarketAutoPlay.h"
#include "MarketBranches.h"
#include "MarketDepartments.h"
#include "MarketBrands.h"
#include "MarketSourcing.h"
#include "MarketEras.h"
#include "MarketLedger.h"
#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketAutoPlaySupplyPolicy,"MirasMarket.AutoPlay.SupplyAndBrands",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketAutoPlaySupplyPolicy::RunTest(const FString& Parameters)
{
    FMarketState State; State.Day=16; State.Cash=100000000;
    FMarketBranch Branch;Branch.Stage=static_cast<uint8>(MarketBranches::EStage::Open);Branch.Format=TEXT("mahalle");State.Branches={Branch,Branch};
    State.Sourcing.LastMonthDay=1;State.Sourcing.MonthBuy.Init(0,4);
    MarketAutoPlayC::FPolicy Policy;Policy.StartDay=16;Policy.Interval=1;Policy.OpensPerTurn=0;
    MarketAutoPlayC::FStats Stats;FString Reason;
    TestTrue(TEXT("Network qualifies for regional supplier"),MarketSourcing::CanSet(State,MarketSourcing::ELine::Drinks,MarketSourcing::ETier::Regional,Reason));
    MarketAutoPlayC::Decide(State,{},Policy,0,Stats);
    TestEqual(TEXT("No commitment without buying volume"),static_cast<int32>(MarketSourcing::TierOf(State,MarketSourcing::ELine::Drinks)),0);
    State.Sourcing.MonthBuy[0]=10000000;
    MarketAutoPlayC::Decide(State,{},Policy,0,Stats);
    TestEqual(TEXT("Covered minimum permits player command"),static_cast<int32>(MarketSourcing::TierOf(State,MarketSourcing::ELine::Drinks)),1);
    State.Sourcing.MonthBuy[0]=0;
    MarketAutoPlayC::Decide(State,{},Policy,0,Stats);
    TestEqual(TEXT("Insufficient volume steps back"),static_cast<int32>(MarketSourcing::TierOf(State,MarketSourcing::ELine::Drinks)),0);
    TestEqual(TEXT("Supplier change does not invent money"),State.Cash,static_cast<int64>(100000000));
    FMarketBrandOffer Offer;Offer.Kind=static_cast<uint8>(MarketBrands::EKind::Rebate);Offer.Brand=TEXT("test");Offer.Amount=1000;
    State.Brands.MonthSales.Add(Offer.Brand,500);
    TestFalse(TEXT("Uncovered sales promise rejected"),MarketAutoPlayC::BrandWorth(State,{},Offer,Policy));
    State.Brands.MonthSales[Offer.Brand]=2000;
    TestTrue(TEXT("Covered sales promise accepted"),MarketAutoPlayC::BrandWorth(State,{},Offer,Policy));
    FMarketProduct Product;Product.Id=TEXT("listing");Offer.ProductId=Product.Id;Offer.Kind=static_cast<uint8>(MarketBrands::EKind::Listing);
    State.Stock.SetNum(1);State.Stock[0].Capacity=0;
    TestFalse(TEXT("Listing without shelf rejected"),MarketAutoPlayC::BrandWorth(State,{Product},Offer,Policy));
    State.Stock[0].Capacity=12;
    TestTrue(TEXT("Existing listing shelf qualifies"),MarketAutoPlayC::BrandWorth(State,{Product},Offer,Policy));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketAutoPlayDeptPolicy,"MirasMarket.AutoPlay.DepartmentLossAndSpace",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketAutoPlayDeptPolicy::RunTest(const FString& Parameters)
{
    FMarketState State;State.Day=60;State.Cash=100000000;
    FMarketBranch Branch;Branch.Stage=static_cast<uint8>(MarketBranches::EStage::Open);Branch.Format=TEXT("buyuk");Branch.LastShoppers=100;State.Branches.Add(Branch);
    MarketAutoPlayC::FPolicy Policy;Policy.StartDay=60;Policy.Interval=1;Policy.SpaceFraction=1;Policy.OpensPerTurn=14;
    MarketAutoPlayC::FStats Stats;
    MarketAutoPlayC::Decide(State,{},Policy,0,Stats);
    TestTrue(TEXT("Actual departments opened with player commands"),State.Branches[0].Depts.Num()>0);
    TestTrue(TEXT("Supermarket stays within 22 percent"),MarketDepartments::SpaceUsed(State,2)<=22);
    TestTrue(TEXT("Opening pays fit-out and stock"),State.Cash<100000000);
    if(State.Branches[0].Depts.IsEmpty())return false;
    const uint8 Dept=State.Branches[0].Depts[0].Dept;
    for(auto& Row:State.Branches[0].Depts){Row.OpenedDay=1;Row.Last30Profit=-1000;}
    Policy.OpensPerTurn=0;
    MarketAutoPlayC::Decide(State,{},Policy,0,Stats);
    TestFalse(TEXT("Mature loss-making department closed"),MarketDepartments::IsOn(State,static_cast<MarketDepartments::EDept>(Dept),2));
    const int64 AfterClose=State.Cash;
    MarketAutoPlayC::Decide(State,{},Policy,0,Stats);
    TestEqual(TEXT("Repeated decision cannot refund stock twice"),State.Cash,AfterClose);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketAutoPlayCObserver,"MirasMarket.AutoPlay.CObservation",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketAutoPlayCObserver::RunTest(const FString& Parameters)
{
    FMarketState State;MarketAutoPlayC::FStats Stats;
    for(int32 Day=1;Day<=32;++Day){State.Day=Day+1;MarketAutoPlayC::Observe(State,Stats);}
    TestEqual(TEXT("A 31-day quiet period is counted once"),Stats.BoringBase,1);
    State.Sourcing.Tiers={1,0,0,0};State.Day=34;
    MarketAutoPlayC::Observe(State,Stats);MarketAutoPlayC::Observe(State,Stats);
    TestEqual(TEXT("Supplier transition is not double counted"),Stats.Sourcing.Num(),1);
    State.EventLog.Reset();
    for(int32 Day=34;Day<=37;++Day){State.Day=Day+1;State.EventLog.Add(FString::Printf(TEXT("event.power@%d"),Day));MarketAutoPlayC::Observe(State,Stats);}
    TestEqual(TEXT("Four disasters within a week make one cluster"),Stats.PilesBase,1);
    TestEqual(TEXT("Repeated log entries do not duplicate disasters"),Stats.BaseBadDays.Num(),4);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketAutoPlaySitePolicy,"MirasMarket.AutoPlay.ExpansionSite",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketAutoPlaySitePolicy::RunTest(const FString& Parameters)
{
    // M69: big store types open with the company's size (8 shops in 2 provinces), not a chapter.
    FMarketState State;State.CountryId=TEXT("tr");
    for(int32 Index=0;Index<7;++Index){FMarketBranch B;B.Country=TEXT("tr");B.Province=Index<4?TEXT("tekirdag"):TEXT("edirne");B.Format=TEXT("mahalle");B.Stage=static_cast<uint8>(MarketBranches::EStage::Open);State.Branches.Add(B);}
    TestTrue(TEXT("Ordinary shop can use small province"),MarketAutoPlayC::SiteSuitable(State,TEXT("tr"),TEXT("kirklareli"),TEXT("mahalle")));
    TestFalse(TEXT("Hyper cannot use small province"),MarketAutoPlayC::SiteSuitable(State,TEXT("tr"),TEXT("kirklareli"),TEXT("hiper")));
    TestFalse(TEXT("Big province alone is not enough for hyper"),MarketAutoPlayC::SiteSuitable(State,TEXT("tr"),TEXT("istanbul"),TEXT("hiper")));
    FMarketDepot Depot;Depot.Country=TEXT("tr");Depot.Province=TEXT("istanbul");State.Company.DepotSites.Add(Depot);
    TestTrue(TEXT("Suitable big province with depot can be selected"),MarketAutoPlayC::SiteSuitable(State,TEXT("tr"),TEXT("istanbul"),TEXT("hiper")));
    State.Branches.SetNum(2);
    TestFalse(TEXT("Size lock still applies"),MarketAutoPlayC::SiteSuitable(State,TEXT("tr"),TEXT("istanbul"),TEXT("hiper")));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketAutoPlayGrowingStyles,"MirasMarket.AutoPlay.LateCarefulGrowth",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketAutoPlayGrowingStyles::RunTest(const FString& Parameters)
{
    TArray<FMarketProduct> Products;TArray<int32> Capacities;TArray<FString> Errors;
    if(!MarketAutoPlay::LoadInputs(Products,Capacities,Errors))return false;
    MarketAutoPlay::FOptions Options;Options.Days=426;Options.Seeds=1;
    const auto Report=MarketAutoPlay::Run(Options,Products,Capacities);
    if(!TestEqual(TEXT("Three real styles"),Report.Runs.Num(),3))return false;
    for(const auto& Run:Report.Runs)
    {
        TestEqual(TEXT("Longer strategy conserves stock and money"),Run.AuditFailures,0);
        TestEqual(Run.Profile+TEXT(" every cash movement is posted"),Run.C.GapDays,0);
    }
    const int32* Careful=Report.Runs[0].Milestones.Find(TEXT("Ilk sube"));
    const int32* Balanced=Report.Runs[1].Milestones.Find(TEXT("Ilk sube"));
    TestNotNull(TEXT("Careful eventually opens a real branch"),Careful);
    TestNotNull(TEXT("Balanced opens a real branch"),Balanced);
    if(Careful && Balanced){TestTrue(TEXT("M39: Careful opens within months eight to fourteen"),*Careful>=240 && *Careful<=426);TestTrue(TEXT("Careful expands after Balanced"),*Careful>*Balanced);}
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketAutoPlayC3Metrics,"MirasMarket.AutoPlay.C3Metrics",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMarketAutoPlayC3Metrics::RunTest(const FString& Parameters)
{
    FMarketState State;
    const auto Plan=MarketEras::PlanOf(State);
    if(!TestTrue(TEXT("An era can be measured"),Plan.Num()>0))return false;
    const int32 Day=Plan[0].StartDay;
    State.Day=Day+1;State.Cash=12345;
    State.Ledger.GapDays=1;State.Ledger.TotalGap=100;State.Ledger.LastGap=100;
    State.Rivals.Closures=2;State.Rivals.Takeovers=3;State.Rivals.OurBuys=4;
    State.Goals.bStarted=true;State.Goals.LastLivelyDay=Day;
    State.Goals.QuietEvents=5;State.Goals.HeldBadEvents=6;
    FMarketCelebration Celebration;Celebration.Day=Day;Celebration.Title=TEXT("Hedef tamam!");State.Goals.Celebrations.Add(Celebration);
    FMarketLedgerEntry Entry;Entry.Day=Day;Entry.Account=static_cast<uint8>(MarketLedger::EAccount::Sales);Entry.Amount=700;State.Ledger.Entries.Add(Entry);
    MarketAutoPlayC::FStats Metrics;
    MarketAutoPlayC::Observe(State,Metrics);MarketAutoPlayC::Observe(State,Metrics);
    TestEqual(TEXT("Same close cannot double-count celebrations"),Metrics.Celebrations,1);
    TestEqual(TEXT("Completed goals counted"),Metrics.GoalsCompleted,1);
    TestEqual(TEXT("Real closure reasons copied"),Metrics.Closures,2);
    TestEqual(TEXT("Rival purchases copied"),Metrics.Takeovers,3);
    TestEqual(TEXT("Our purchases copied"),Metrics.OurBuys,4);
    TestEqual(TEXT("Era profit uses the ledger"),Metrics.Eras[0].Profit,static_cast<int64>(700));
    TestEqual(TEXT("Read-only observation keeps cash"),State.Cash,static_cast<int64>(12345));
    ++State.Day;State.Ledger.GapDays=2;State.Ledger.TotalGap=0;State.Ledger.LastGap=-100;
    MarketAutoPlayC::Observe(State,Metrics);
    TestEqual(TEXT("Signed gaps can cancel"),Metrics.GapTotal,static_cast<int64>(0));
    TestEqual(TEXT("Absolute gaps still reveal both errors"),Metrics.GapAbsolute,static_cast<int64>(200));
    TestEqual(TEXT("Both gap days retained"),Metrics.GapDays,2);
    return true;
}
#endif

#include "MarketSimulation.h"
#include "MarketDirector.h"
#include "MarketEvents.h"
#include "MarketFinance.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketSimulationTest
{
    FMarketProduct Make(const TCHAR* Id, const TCHAR* Category, int64 Cost, int64 Price)
    {
        FMarketProduct P;
        P.Id = Id; P.RealName = Id; P.Category = Category; P.Cost = Cost; P.BasePrice = Price; P.CaseUnits = 12;
        return P;
    }

    TArray<FMarketProduct> Catalog()
    {
        return {
            Make(TEXT("sut"), TEXT("s\u00fct"), 170, 250),
            Make(TEXT("ayran"), TEXT("s\u00fct"), 60, 100),
            Make(TEXT("makarna"), TEXT("makarna-bakliyat"), 210, 325),
            Make(TEXT("cay"), TEXT("\u00e7ay-kahve"), 480, 675),
            Make(TEXT("cola"), TEXT("i\u00e7ecek"), 180, 275),
            Make(TEXT("biskuvi"), TEXT("bisk\u00fcvi-\u00e7ikolata"), 100, 175),
            Make(TEXT("deterjan"), TEXT("temizlik"), 1400, 1990),
        };
    }

    int32 Units(const FMarketState& S)
    {
        int32 Sum = 0;
        for (const FMarketStock& Row : S.Stock) Sum += Row.Shelf + Row.Warehouse + Row.Dock;
        return Sum;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketSimulationTest, "MarketSim.Simulation.AdvanceAndDifficulty", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketSimulationTest::RunTest(const FString& Parameters)
{
    using namespace MarketSimulationTest;
    const TArray<FMarketProduct> Base = Catalog();
    TArray<FMarketProduct> Products = Base;
    FMarketState S; S.Initialize(Base); S.RivalSeed = 21; S.Cash = 300000;
    MarketDirector::ApplyPrices(S, Base, Products);
    FString Message;

    // Difficulty moves the shoppers and their patience with prices, nothing else.
    FMarketState Easy = S, Hard = S;
    TestTrue(TEXT("Easy"), MarketDirector::Command(Easy, Products, TEXT("Difficulty"), 0, Message));
    TestTrue(TEXT("Hard"), MarketSimulation::SetDifficulty(Hard, 2, Message));
    TestTrue(TEXT("More shoppers when easy"), MarketDirector::TrafficFactor(Easy, Products) > MarketDirector::TrafficFactor(Hard, Products));
    TestTrue(TEXT("Easier on prices"), MarketDirector::ToleranceBonus(Easy, Products[0]) > MarketDirector::ToleranceBonus(Hard, Products[0]));
    TestFalse(TEXT("Same again"), MarketSimulation::SetDifficulty(Hard, 2, Message));

    // One day without walking people.
    const int32 Day = S.Day;
    const int32 UnitsBefore = Units(S);
    FMarketState Copy = S;
    TArray<FMarketProduct> CopyProducts = Products;
    const MarketSimulation::FDay Result = MarketSimulation::PlayDay(S, Base, Products);
    TestEqual(TEXT("The day closed"), S.Day, Day + 1);
    TestTrue(TEXT("Shoppers came"), Result.Shoppers > 20);
    TestTrue(TEXT("Baskets were sold"), Result.Served > 0 && Result.Revenue > 0);
    int32 Sold = 0;
    for (const FMarketStock& Row : S.Stock) Sold += Row.Yesterday.Sold;
    // Stock after = before - sold + what the evening order brought to the rear door (nothing else moves).
    const int32 Arrived = Units(S) - (UnitsBefore - Sold);
    TestTrue(TEXT("Sold units left the stock"), Sold > 0 && Arrived >= 0);
    if (Result.Ordered == 0) TestEqual(TEXT("No order, no new units"), Arrived, 0);
    TestTrue(TEXT("Revenue recorded"), S.LastRevenue == Result.Revenue);
    MarketSimulation::PlayDay(Copy, Base, CopyProducts);
    TestEqual(TEXT("Deterministic"), Copy.Cash, S.Cash);

    // Advance: plays several days and orders what the advice says.
    int32 Played = MarketSimulation::Advance(S, Base, Products, 5, Message);
    TestTrue(TEXT("Days played"), Played >= 1 && Played <= 5);
    TestFalse(TEXT("Summary"), Message.IsEmpty());
    int32 Ordered = 0;
    for (const FMarketStock& Row : S.Stock) Ordered += Row.Dock + Row.Incoming;
    TestTrue(TEXT("The family keeps ordering"), Ordered > 0 || S.LastPurchases > 0 || S.Purchases > 0);

    // M69: the first store's building is ours: worth something, whatever the till holds; no rent is paid.
    FMarketState Home = S;
    Home.Cash = 1000000;
    const int64 Building = MarketFinance::BuildingValue(Home);
    TestTrue(TEXT("The building is worth something"), Building > 0);
    Home.Cash = 0;
    TestEqual(TEXT("Its value does not depend on the till"), MarketFinance::BuildingValue(Home), Building);

    // A waiting decision stops the advance before any day is played.
    FMarketDecision Decision; Decision.Id = TEXT("event.test"); Decision.Title = TEXT("Test"); Decision.Options = { TEXT("Evet"), TEXT("Hay\u0131r") };
    Decision.Deadline = S.Day + 3;
    MarketEvents::Offer(S, Decision);
    const int32 Before = S.Day;
    TestEqual(TEXT("Stops for a decision"), MarketSimulation::Advance(S, Base, Products, 7, Message), 0);
    TestEqual(TEXT("No day played"), S.Day, Before);
    return true;
}

#endif

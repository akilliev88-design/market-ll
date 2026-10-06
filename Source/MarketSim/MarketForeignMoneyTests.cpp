#include "MarketBranches.h"
#include "MarketCountry.h"
#include "MarketLedger.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// E4 (11_TEK_EKONOMI): a store abroad works in its own country's money and reaches the till at the day's rate.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketForeignMoneyTest, "MarketSim.Branches.ForeignMoney", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketForeignMoneyTest::RunTest(const FString& Parameters)
{
    using namespace MarketBranches;
    FMarketState S; S.RivalSeed = 9; S.Day = 400; S.CountryId = MarketCountry::DefaultId();

    // At home nothing changes, not even by rounding.
    const FMoney Home = MoneyOf(S, S.CountryId, S.Day);
    TestTrue(TEXT("Home money"), !Home.bForeign && Home.Goods == 1.0 && Home.Wages == 1.0 && Home.Fx == 1.0);
    TestEqual(TEXT("Home amount unchanged"), InHome(Home, 123457), int64(123457));

    // When the rate only follows the price gap, a store abroad earns what its twin at home earns.
    FMoney Steady; Steady.bForeign = true; Steady.Goods = 1.25; Steady.Wages = 1.25; Steady.Fx = 0.8;
    TestEqual(TEXT("A steady currency: the same profit"), InHome(Steady, 100000), int64(100000));
    TestEqual(TEXT("Wages too"), InHome(Steady, 50000, true), int64(50000));
    // The currency loses 20 % more than the prices: the result in the campaign's money falls 20 %.
    FMoney Fallen = Steady; Fallen.Fx = 0.8 * 0.8;
    TestEqual(TEXT("A 20 % fall: 20 % less"), InHome(Fallen, 100000), int64(80000));
    // The assets held abroad lose the same on the books.
    TestEqual(TEXT("Exchange loss on local assets"), FxDifference(125000, Steady.Fx, Fallen.Fx), int64(-20000));
    TestEqual(TEXT("And gain when it recovers"), FxDifference(125000, Fallen.Fx, Steady.Fx), int64(20000));

    // A real foreign country (when the packs have one): day 1 is the reference, the rate stays sensible.
    FString Abroad;
    for (const MarketCountry::FProfile& P : MarketCountry::All()) if (P.Id != S.CountryId && P.Cities.Num() > 0) { Abroad = P.Id; break; }
    if (!Abroad.IsEmpty())
    {
        const FMoney First = MoneyOf(S, Abroad, 1);
        TestTrue(TEXT("Day 1: one to one"), First.bForeign && FMath::IsNearlyEqual(First.Fx, 1.0, 1e-6));
        const FMoney Later = MoneyOf(S, Abroad, 1500);
        TestTrue(TEXT("Rate and levels sensible later"), Later.Fx > 0.05 && Later.Fx < 20.0 && Later.Goods > 0.05 && Later.Goods < 20.0);
        FMarketBranch B; B.Country = Abroad; B.Stage = static_cast<uint8>(EStage::Open); B.OpenedDay = 1; B.Rent = 60000;
        TestEqual(TEXT("Deposit at the opening rate on day 1"), DepositInHome(S, B, 1), int64(120000));
        const TArray<FMarketProduct> None;
        TestEqual(TEXT("Local assets: the deposit"), LocalAssets(S, B, None, 1), int64(120000));
        FMarketBranch AtHome = B; AtHome.Country = S.CountryId;
        TestEqual(TEXT("No local assets at home"), LocalAssets(S, AtHome, None, 1), int64(0));
    }
    TestTrue(TEXT("The exchange difference is in the income statement"), MarketLedger::IsIncomeStatement(MarketLedger::EAccount::FxDifference));
    return true;
}

#endif

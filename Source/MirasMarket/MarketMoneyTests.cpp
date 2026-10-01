#include "MarketCast.h"
#include "MarketFreshness.h"
#include "MarketFinance.h"
#include "MarketEvents.h"
#include "MarketSuppliers.h"
#include "MarketCalendar.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketMoneyTest
{
    TArray<FMarketProduct> Catalog()
    {
        FMarketProduct Milk; Milk.Id = TEXT("milk"); Milk.Category = TEXT("s\u00fct"); Milk.Cost = 170; Milk.BasePrice = 250;
        FMarketProduct Cola; Cola.Id = TEXT("cola"); Cola.Category = TEXT("i\u00e7ecek"); Cola.Cost = 180; Cola.BasePrice = 275;
        FMarketProduct Tea; Tea.Id = TEXT("tea"); Tea.Category = TEXT("\u00e7ay-kahve"); Tea.Cost = 480; Tea.BasePrice = 675;
        return { Milk, Cola, Tea };
    }

    bool NewsStarts(const FMarketState& S, const TCHAR* Start)
    {
        for (const FString& Line : S.DayNews) if (Line.StartsWith(Start)) return true;
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketFreshnessTest, "MirasMarket.Freshness.BatchesAndWaste", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketFreshnessTest::RunTest(const FString& Parameters)
{
    using namespace MarketFreshness;
    using namespace MarketMoneyTest;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S; S.Initialize(Products); S.ApplyShelfCapacities({ 24, 24, 24 });
    auto Close = [&S, &Products] { S.DayNews.Reset(); S.CloseDay(); MarketFreshness::CloseDay(S, Products); };
    Close();
    TestEqual(TEXT("Milk: seven days"), DaysLeft(S, TEXT("milk")), 6);
    TestTrue(TEXT("Cola: months"), DaysLeft(S, TEXT("cola")) > 100);
    TestEqual(TEXT("Tea does not spoil in the game"), DaysLeft(S, TEXT("tea")), static_cast<int32>(INDEX_NONE));

    // A second delivery makes a younger batch; sold units leave the oldest batch first.
    S.Stock[0].Dock = 12;
    Close();
    S.Stock[0].Warehouse -= 20; // sold / moved out
    Close();
    int32 Old = 0, Young = 0;
    for (const FMarketBatch& B : S.Batches) if (B.ProductId == TEXT("milk")) { if (B.ExpiresDay == 8) Old += B.Units; else Young += B.Units; }
    TestTrue(TEXT("FEFO: the old batch was used first"), Old == 12 && Young == 12);

    // Last day: 30 % off; after it: waste.
    while (S.Day < 8) Close();
    TestTrue(TEXT("Last-day markdown"), FMath::IsNearlyEqual(PriceFactor(S, 0), 0.7f) && FMath::IsNearlyEqual(PriceFactor(S, 1), 1.f));
    const int32 Before = S.Stock[0].Shelf + S.Stock[0].Warehouse + S.Stock[0].Dock;
    Close();
    TestEqual(TEXT("Twelve old units spoiled"), S.LastWasteUnits, 12);
    TestEqual(TEXT("Waste cost"), S.LastWasteCost, int64(12 * 170));
    TestEqual(TEXT("They left the shop"), S.Stock[0].Shelf + S.Stock[0].Warehouse + S.Stock[0].Dock, Before - 12);
    TestTrue(TEXT("Reported"), NewsStarts(S, TEXT("Fire:")));

    // Donation: given away on the eve of the last day, the neighbourhood likes it.
    FMarketState D; D.Initialize(Products); D.ApplyShelfCapacities({ 24, 24, 24 });
    FMarketLoyalty Neighbour; Neighbour.CustomerId = 1; Neighbour.Satisfaction = 50.f; D.Loyalty.Add(Neighbour);
    FString Message;
    TestTrue(TEXT("Donate policy"), SetPolicy(D, EPolicy::Donate, Message));
    for (int32 Day = 0; Day < 7; ++Day) { D.DayNews.Reset(); D.CloseDay(); MarketFreshness::CloseDay(D, Products); }
    TestTrue(TEXT("Milk donated"), D.Stock[0].Warehouse == 0 && D.Loyalty[0].Satisfaction > 50.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketFinanceTest, "MirasMarket.Finance.LoansAndTrouble", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketFinanceTest::RunTest(const FString& Parameters)
{
    using namespace MarketFinance;
    using namespace MarketMoneyTest;
    const TArray<FMarketProduct> Products = Catalog();
    TestTrue(TEXT("Annuity"), FMath::Abs(Installment(120000, 0.01, 12) - 10662) <= 1);

    FMarketState S; S.Initialize(Products); S.Cash = 10000;
    FString Message;
    TestEqual(TEXT("A new shop can borrow on the family name"), LoanLimit(S), int64(50000));
    TestFalse(TEXT("Not 1.000 TL"), TakeLoan(S, 1, Message));
    TestTrue(TEXT("500 TL"), TakeLoan(S, 0, Message));
    TestTrue(TEXT("Cash in"), S.Cash == 60000 && Debt(S) == 50000);
    const int64 Due = S.Loans[0].Installment;
    S.Cash = 500000; // a month of running costs
    for (int32 Day = 0; Day < 31; ++Day) { S.DayNews.Reset(); S.Revenue = 3000; S.CloseDay(); MarketFinance::CloseDay(S, Products); }
    TestTrue(TEXT("First installment paid"), NewsStarts(S, *(MarketCast::Bank(0) + TEXT(" taksiti"))) && Debt(S) < 50000 && Debt(S) > 50000 - Due);

    // The trouble ladder: warning, terms closed, a choice, forced sale; positive cash ends it.
    FMarketState T; T.Initialize(Products);
    MarketSuppliers::Account(T, MarketSuppliers::ESupplier::Family).Trust = 80;
    T.Cash = -50000;
    auto Close = [&T, &Products] { T.DayNews.Reset(); T.CloseDay(); MarketFinance::CloseDay(T, Products); T.Cash = FMath::Min<int64>(T.Cash, -50000); };
    Close();
    TestEqual(TEXT("Stage 1"), T.TroubleStage, 1);
    Close(); Close();
    TestEqual(TEXT("Stage 2"), T.TroubleStage, 2);
    TestTrue(TEXT("Terms closed"), MarketSuppliers::TermsDays(T, MarketSuppliers::ESupplier::Family) == 0);
    TestEqual(TEXT("No new loans in trouble"), LoanLimit(T), int64(0));
    for (int32 Day = 0; Day < 4; ++Day) Close();
    TestTrue(TEXT("A choice waits"), MarketEvents::Pending(T) && MarketEvents::Pending(T)->Id == TEXT("finance.rescue"));
    const int32 Depot = T.Stock[0].Warehouse;
    TestTrue(TEXT("Sell the depot at half price"), Depot > 0 && MarketEvents::Decide(T, Products, 1, Message) && T.Stock[0].Warehouse == 0);
    T.Cash = 100000;
    T.DayNews.Reset(); T.CloseDay(); MarketFinance::CloseDay(T, Products);
    TestEqual(TEXT("Trouble over"), T.TroubleStage, 0);

    // Month-end report.
    FMarketState M; M.Initialize(Products); M.Cash = 10000;
    for (int32 Day = 1; Day <= 25; ++Day) { FMarketDayRecord R; R.Day = Day; R.Revenue = 10000; R.Profit = 1000; M.History.Add(R); }
    M.Day = MarketCalendar::GameDayOf(2011, 3, 31);
    M.DayNews.Reset(); M.CloseDay(); MarketFinance::CloseDay(M, Products);
    TestTrue(TEXT("Month report"), NewsStarts(M, TEXT("Ay sonu raporu (Mart, 1. y\u0131l)")));
    return true;
}

#endif

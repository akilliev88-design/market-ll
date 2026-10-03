#include "MarketFreshness.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// E3c2 (M63): the family shop and the branches share one batch rule (MarketFreshness::MatchBatches).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketFreshnessStoresTest, "MirasMarket.Freshness.OneRuleEveryStore", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketFreshnessStoresTest::RunTest(const FString& Parameters)
{
    using namespace MarketFreshness;
    TArray<FMarketBatch> Batches;
    const FString Milk = TEXT("milk");

    // Day 10: 12 arrive, sellable 3 days (10, 11, 12).
    FBatchDay Day = MatchBatches(Batches, Milk, 12, 12, 3, 10, EPolicy::Markdown);
    TestTrue(TEXT("A new batch"), Batches.Num() == 1 && Batches[0].Units == 12 && Batches[0].ExpiresDay == 12 && Day.Wasted == 0);
    // Day 11: 7 sold, 6 new arrive: the sold ones left the old batch first (FEFO).
    Day = MatchBatches(Batches, Milk, 11, 6, 3, 11, EPolicy::Markdown);
    int32 Old = 0, New = 0;
    for (const FMarketBatch& B : Batches) { if (B.ExpiresDay == 12) Old += B.Units; if (B.ExpiresDay == 13) New += B.Units; }
    TestTrue(TEXT("Sold from the oldest"), Old == 5 && New == 6 && Day.Wasted == 0);
    TestEqual(TEXT("Last-day units on day 12"), LastDayUnitsIn(Batches, Milk, 12), 5);
    // Day 13: nothing sold; the day-12 batch expired.
    Day = MatchBatches(Batches, Milk, 11, 0, 3, 13, EPolicy::Markdown);
    TestEqual(TEXT("Expired units are waste"), Day.Wasted, 5);
    TestEqual(TEXT("No donation under markdown"), Day.Donated, 0);

    // Donation: a batch on its last day is given away instead of waiting.
    TArray<FMarketBatch> Given;
    MatchBatches(Given, Milk, 10, 10, 2, 20, EPolicy::Donate);
    const FBatchDay Last = MatchBatches(Given, Milk, 10, 0, 2, 21, EPolicy::Donate);
    TestTrue(TEXT("Donated on the last day"), Last.Donated == 10 && Last.Wasted == 0);

    // An opening stock without a delivery record counts as new goods.
    TArray<FMarketBatch> Opening;
    MatchBatches(Opening, Milk, 24, 0, 5, 30, EPolicy::Nothing);
    TestTrue(TEXT("Opening stock becomes a batch"), Opening.Num() == 1 && Opening[0].Units == 24 && Opening[0].ExpiresDay == 34);
    // Never more removed than held.
    TArray<FMarketBatch> Short;
    MatchBatches(Short, Milk, 8, 8, 1, 40, EPolicy::Nothing);
    const FBatchDay Gone = MatchBatches(Short, Milk, 3, 0, 1, 41, EPolicy::Nothing);
    TestEqual(TEXT("Waste is what is held"), Gone.Wasted, 3);
    return true;
}

#endif

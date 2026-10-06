#include "MarketOrderAdvice.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketOrderAdviceTest, "MarketSim.Economy.OrderAdvice", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketOrderAdviceTest::RunTest(const FString& Parameters)
{
    FMarketProduct Milk; Milk.Id = TEXT("milk"); Milk.Cost = 170; Milk.BasePrice = 250; Milk.CaseUnits = 12;
    FMarketProduct Cola; Cola.Id = TEXT("cola"); Cola.Cost = 180; Cola.BasePrice = 275; Cola.CaseUnits = 12;
    FMarketProduct Soap; Soap.Id = TEXT("soap"); Soap.Cost = 1400; Soap.BasePrice = 1990; Soap.CaseUnits = 4;
    const TArray<FMarketProduct> Catalog = { Milk, Cola, Soap };
    FMarketState S; S.Initialize(Catalog);
    S.ApplyShelfCapacities({ 12, 12, 0 }); // soap is on no shelf; every product starts with 32 in the depot

    TestEqual(TEXT("Depot already fills the shelf"), MarketOrderAdvice::SuggestCases(S, Catalog, 0), 0);
    S.Stock[0].Warehouse = 0;
    TestEqual(TEXT("Empty shop: one case fills the shelf"), MarketOrderAdvice::SuggestCases(S, Catalog, 0), 1);
    S.Stock[0].Yesterday.Sold = 30; S.Stock[0].Yesterday.Empty = 5;
    TestEqual(TEXT("Yesterday's demand + 25 % = 50 units"), MarketOrderAdvice::SuggestCases(S, Catalog, 0), 5);
    S.Stock[0].Dock = 12; S.Stock[0].Incoming = 12;
    TestEqual(TEXT("Rear door and goods on the way count"), MarketOrderAdvice::SuggestCases(S, Catalog, 0), 3);
    TestEqual(TEXT("Nothing for a product on no shelf"), MarketOrderAdvice::SuggestCases(S, Catalog, 2), 0);
    S.Stock[1].Warehouse = 0; S.Stock[1].Yesterday.Sold = 400;
    TestEqual(TEXT("Never more than one order line allows"), MarketOrderAdvice::SuggestCases(S, Catalog, 1), MarketOrderAdvice::MaxCases);
    S.Stock[1].Warehouse = 100;
    TestEqual(TEXT("Depot room limits the suggestion"), MarketOrderAdvice::SuggestCases(S, Catalog, 1), 1);

    TArray<int32> Draft;
    Draft.Init(0, Catalog.Num());
    Draft[0] = 4; Draft[1] = 3;
    TestEqual(TEXT("Only lower lines are raised"), MarketOrderAdvice::FillSuggested(S, Catalog, Draft), 0);
    Draft[0] = 1;
    TestEqual(TEXT("One line raised"), MarketOrderAdvice::FillSuggested(S, Catalog, Draft), 1);
    TestEqual(TEXT("Milk line at the suggestion"), Draft[0], 3);
    TestEqual(TEXT("Cola line kept"), Draft[1], 3);

    // The suggested order is a valid order.
    Draft[1] = 0;
    TestTrue(TEXT("Suggested order can be submitted"), S.SubmitOrder(Draft, Catalog));
    TestEqual(TEXT("Milk on the way"), S.Stock[0].Incoming, 12 + 36);
    return true;
}

#endif

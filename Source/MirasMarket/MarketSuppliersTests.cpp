#include "MarketSuppliers.h"
#include "MarketPrices.h"
#include "MarketCalendar.h"
#include "MarketStaff.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketPricesTest, "MirasMarket.Suppliers.InflationAndWages", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketPricesTest::RunTest(const FString& Parameters)
{
    using namespace MarketPrices;
    TestTrue(TEXT("Day 1 is the base"), FMath::IsNearlyEqual(PriceLevel(1), 1.0) && FMath::IsNearlyEqual(ListLevel(20), 1.0));
    const int32 April = MarketCalendar::GameDayOf(2011, 4, 1);
    TestTrue(TEXT("The April list is a little higher"), ListLevel(April) > 1.004 && ListLevel(April) < 1.01);
    TestTrue(TEXT("The list does not move inside a month"), FMath::IsNearlyEqual(ListLevel(April), ListLevel(April + 20)));
    const double OneYear = PriceLevel(MarketCalendar::GameDayOf(2012, 3, 7));
    TestTrue(TEXT("About 9-11 % in the first year"), OneYear > 1.08 && OneYear < 1.12);
    const double By2024 = PriceLevel(MarketCalendar::GameDayOf(2024, 3, 7));
    TestTrue(TEXT("Prices of 2024 are many times those of 2011"), By2024 > 5.0 && By2024 < 12.0);
    TestTrue(TEXT("Minimum wage 2011"), FMath::IsNearlyEqual(WageIndex(1), 1.0));
    TestTrue(TEXT("July 2011 raise"), WageIndex(MarketCalendar::GameDayOf(2011, 7, 1)) > 1.06);
    TestTrue(TEXT("2024 minimum wage"), WageIndex(MarketCalendar::GameDayOf(2024, 3, 1)) > 20.0);
    TestTrue(TEXT("Beyond the table the wage keeps rising"), WageIndex(MarketCalendar::GameDayOf(2030, 3, 1)) > WageIndex(MarketCalendar::GameDayOf(2026, 3, 1)));
    TestTrue(TEXT("Loan rates of 2018"), FMath::IsNearlyEqual(LoanRate(MarketCalendar::GameDayOf(2018, 5, 1)), 0.28));
    TestTrue(TEXT("Fair wages follow the minimum wage"),
        MarketStaff::FairWage(MarketStaff::ERole::Cashier, 50, MarketCalendar::GameDayOf(2011, 7, 1)) > MarketStaff::FairWage(MarketStaff::ERole::Cashier, 50, 1));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketSuppliersTest, "MirasMarket.Suppliers.TermsAndPriceLists", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketSuppliersTest::RunTest(const FString& Parameters)
{
    using namespace MarketSuppliers;
    FMarketProduct Milk; Milk.Id = TEXT("milk"); Milk.Category = TEXT("s\u00fct"); Milk.Cost = 170; Milk.BasePrice = 250; Milk.CaseUnits = 12;
    const TArray<FMarketProduct> Base = { Milk };
    FMarketState S; S.Initialize(Base); S.Cash = 100000;
    TArray<FMarketProduct> Today = Base;

    // Day 1: the catalog's own prices.
    ApplyPrices(S, Base, Today);
    TestEqual(TEXT("Cost on day 1"), Today[0].Cost, int64(170));
    TestEqual(TEXT("List on day 1"), Today[0].BasePrice, int64(250));
    TestTrue(TEXT("The father's wholesaler"), Current(S) == ESupplier::TrakyaGida);
    Account(S, ESupplier::TrakyaGida);
    TestEqual(TEXT("Cash at first"), TermsDays(S, ESupplier::TrakyaGida), 0);
    TestEqual(TEXT("Nothing extra for a cash order"), OnOrder(S, 6000), FString());

    // Trust brings terms: the bill waits, the cash stays.
    Account(S, ESupplier::TrakyaGida).Trust = 60;
    TestEqual(TEXT("Seven days"), TermsDays(S, ESupplier::TrakyaGida), 7);
    const int64 Cash = S.Cash;
    TestFalse(TEXT("Terms line"), OnOrder(S, 10000).IsEmpty());
    TestEqual(TEXT("Cash given back"), S.Cash, Cash + 10000);
    TestTrue(TEXT("Bill written"), S.Payables.Num() == 1 && S.Payables[0].DueDay == S.Day + 7);
    // Paid at the close of the due day from the till.
    for (int32 D = 0; D < 8; ++D) { S.CloseDay(); CloseDay(S); }
    TestEqual(TEXT("Paid on time"), S.Payables.Num(), 0);
    TestTrue(TEXT("Trust grew"), Account(S, ESupplier::TrakyaGida).Trust >= 65);

    // A late bill: fee, lost trust, no more terms.
    OnOrder(S, 10000);
    S.Cash = 0;
    const int32 TrustBefore = Account(S, ESupplier::TrakyaGida).Trust;
    for (int32 D = 0; D < 8; ++D) { S.Revenue = 0; S.CloseDay(); CloseDay(S); S.Cash = FMath::Max<int64>(S.Cash, 0); }
    TestTrue(TEXT("Still owed, with a late fee"), S.Payables.Num() == 1 && S.Payables[0].Amount > 10000);
    TestTrue(TEXT("Trust fell"), Account(S, ESupplier::TrakyaGida).Trust < TrustBefore - 20);
    TestEqual(TEXT("Terms closed"), TermsDays(S, ESupplier::TrakyaGida), 0);
    S.Cash = 100000;
    TestTrue(TEXT("Paying the bills"), PayBills(S) > 10000 && OpenBills(S) == 0);

    // Volume discount.
    FMarketState V; V.Initialize(Base);
    Account(V, ESupplier::TrakyaGida).Volume30 = 200000;
    TestTrue(TEXT("3 % for volume"), FMath::IsNearlyEqual(Discount(V, ESupplier::TrakyaGida), 0.03f));
    TestTrue(TEXT("Cheaper unit"), UnitCost(V, Milk) < 170);

    // The cheaper wholesaler from day 14.
    FString Message;
    TestFalse(TEXT("Not yet"), Switch(V, ESupplier::Ozdemir, Message));
    V.Day = 14;
    const int32 SelimTrust = Account(V, ESupplier::TrakyaGida).Trust;
    TestTrue(TEXT("Switch"), Switch(V, ESupplier::Ozdemir, Message));
    TestTrue(TEXT("4 % cheaper"), UnitCost(V, Milk) < 170 && FMath::IsNearlyEqual(Discount(V, ESupplier::Ozdemir), 0.04f));
    TestTrue(TEXT("Selim noticed"), Account(V, ESupplier::TrakyaGida).Trust == SelimTrust - 10);
    TestEqual(TEXT("Riskier deliveries"), Info(ESupplier::Ozdemir).DeliveryRisk, 3u);
    TestEqual(TEXT("No terms"), TermsDays(V, ESupplier::Ozdemir), 0);

    // The monthly price list and passing it on.
    FMarketState M; M.Initialize(Base); M.Cash = 100000;
    M.Day = MarketCalendar::GameDayOf(2011, 3, 31);
    M.CloseDay(); CloseDay(M); // day 31 March closes, 1 April is tomorrow
    bool bNewList = false;
    for (const FString& Line : M.DayNews) if (Line.Contains(TEXT("yeni ay"))) bNewList = true;
    TestTrue(TEXT("New list announced"), bNewList);
    ApplyPrices(M, Base, Today);
    TestTrue(TEXT("April costs and rival prices are higher"), Today[0].Cost > 170 && Today[0].BasePrice >= 250);
    TestTrue(TEXT("The shelf is behind the list"), PriceGap(M) > 0.004);
    M.Stock[0].Price = 300;
    TestEqual(TEXT("Prices passed on"), PassOnPriceRise(M, Today), 1);
    TestTrue(TEXT("Shelf price rose"), M.Stock[0].Price > 300);
    TestTrue(TEXT("No gap left"), PriceGap(M) < 0.0001);
    TestFalse(TEXT("Summary"), Summary(M).IsEmpty());
    return true;
}

#endif

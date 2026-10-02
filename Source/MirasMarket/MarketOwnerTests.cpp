#include "MarketOwner.h"
#include "MarketPromotions.h"
#include "MarketLedger.h"
#include "MarketFinance.h"
#include "MarketSuppliers.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketOwnerTest
{
    TArray<FMarketProduct> OwnerCatalog()
    {
        FMarketProduct MilkA; MilkA.Id = TEXT("milk_a"); MilkA.Category = TEXT("s\u00fct"); MilkA.Cost = 170; MilkA.BasePrice = 250;
        FMarketProduct Cola; Cola.Id = TEXT("cola"); Cola.Category = TEXT("i\u00e7ecek"); Cola.Cost = 180; Cola.BasePrice = 275;
        return { MilkA, Cola };
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketOwnerSalaryTest, "MirasMarket.Owner.SalaryAndWealth", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketOwnerSalaryTest::RunTest(const FString& Parameters)
{
    // M37: the company pays us a salary (its cost), we keep the net; the family lives from our own money.
    using namespace MarketOwner;
    const TArray<FMarketProduct> Products = MarketOwnerTest::OwnerCatalog();
    FMarketState S; S.Initialize(Products); S.Day = 40; S.CountryId = TEXT("tr");
    S.Cash = 10000000;
    const int64 Gross = GrossSalary(S);
    TestTrue(TEXT("At least the minimum wage"), Gross >= MinimumMonthly(S));
    TestTrue(TEXT("Net below gross, company pays more"), NetSalary(S) < Gross && CompanyCost(S) > Gross);
    const int64 CashBefore = S.Cash;
    MonthStart(S);
    TestEqual(TEXT("The company paid the salary"), S.Cash, CashBefore - CompanyCost(S));
    TestEqual(TEXT("Net salary minus the living"), S.Owner.Wealth, NetSalary(S) - Living(S));
    TestEqual(TEXT("Counted"), S.Owner.TotalSalary, NetSalary(S));

    // An empty till: no salary that month, the living comes from what we saved.
    S.Cash = 0;
    const int64 Saved = S.Owner.Wealth;
    MonthStart(S);
    TestEqual(TEXT("Missed"), S.Owner.MissedSalaries, 1);
    TestTrue(TEXT("Living from savings, never below zero"), S.Owner.Wealth == FMath::Max<int64>(0, Saved - Living(S)) && S.Cash == 0);

    // A raise, the rescue plan's cut.
    FString Message;
    TestTrue(TEXT("Raise to 3x"), SetSalary(S, 3, Message) && S.Owner.SalaryX10 == SalarySteps[3]);
    TestTrue(TEXT("Dearer"), GrossSalary(S) > Gross);
    TArray<FString> Lines;
    CutToMinimum(S, Lines);
    TestTrue(TEXT("The bank cuts it"), S.Owner.SalaryX10 == 10 && Lines.Num() == 1);
    S.RescueUntil = S.Day + 100;
    TestFalse(TEXT("No raise under the plan"), SetSalary(S, 2, Message));
    S.RescueUntil = 0;

    // No dividend in the first year; personal money into the company.
    TestEqual(TEXT("Nothing to distribute in year one"), Distributable(S), int64(0));
    TestFalse(TEXT("No dividend"), PayDividend(S, 2, Message));
    S.Owner.Wealth = 500000; S.Cash = 1000;
    TestTrue(TEXT("Capital in"), PutCapital(S, 1, Message));
    TestTrue(TEXT("Half moved"), S.Owner.Wealth == 250000 && S.Cash == 251000 && S.Owner.CapitalIn == 250000);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCampaignStoresTest, "MirasMarket.Promotions.EveryStore", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCampaignStoresTest::RunTest(const FString& Parameters)
{
    // M38: every store has the same campaigns: the player starts one in the family shop, a branch or every store;
    // a branch's campaign does not touch the family shop's prices.
    using namespace MarketPromotions;
    const TArray<FMarketProduct> Products = MarketOwnerTest::OwnerCatalog();
    FMarketState S; S.Initialize(Products); S.Cash = 100000;
    S.ApplyShelfCapacities({ 12, 12 });
    FMarketBranch Branch; Branch.Name = TEXT("Deneme 01"); S.Branches.Add(Branch);
    FString Message;
    TestFalse(TEXT("No such store"), SetStore(S, 7, Message));
    TestTrue(TEXT("Branch 0"), SetStore(S, 0, Message));
    TestTrue(TEXT("20 % on the milk in the branch"), StartScoped(S, Products, PackArg(0, EScope::Product, EMechanic::Percent, 20, 5), Message));
    TestEqual(TEXT("Branch campaign"), S.Promotions.Last().Store, 0);
    TestEqual(TEXT("The family shop's price stays"), UnitPrice(S, Products, 0, 1), S.Stock[0].Price);
    float Cut = 0.f, Pull = 1.f;
    StoreEffect(S, Products, 0, 0, S.Day, Cut, Pull);
    TestTrue(TEXT("The branch sells it cheaper"), FMath::IsNearlyEqual(Cut, 0.2f));

    TestTrue(TEXT("Every store"), SetStore(S, MarketLedger::AllStores, Message));
    TestTrue(TEXT("3 al 2 on the cola everywhere"), StartScoped(S, Products, PackArg(1, EScope::Product, EMechanic::ThreeForTwo, 5, 5), Message));
    TestTrue(TEXT("3 for 2 in the family shop"), UnitPrice(S, Products, 1, 3) < S.Stock[1].Price);
    StoreEffect(S, Products, 0, 1, S.Day, Cut, Pull);
    TestTrue(TEXT("and in the branch"), Cut > 0.f && Pull > 1.f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketSupplierLifelineTest, "MirasMarket.Suppliers.Lifeline", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketSupplierLifelineTest::RunTest(const FString& Parameters)
{
    // C9: in a cash crisis the father's wholesaler still gives about three days of goods on short terms, once a week.
    const TArray<FMarketProduct> Products = MarketOwnerTest::OwnerCatalog();
    FMarketState S; S.Initialize(Products); S.Day = 200;
    FMarketSupplierAccount& A = MarketSuppliers::Account(S, MarketSuppliers::ESupplier::Family);
    A.Trust = 20; A.Volume30 = 300000;
    S.Cash = 50000;
    TestEqual(TEXT("No lifeline while the till is fine"), MarketSuppliers::LifelineAllowance(S), int64(0));
    S.Cash = -5000; S.TroubleStage = 2;
    const int64 Room = MarketSuppliers::LifelineAllowance(S);
    TestTrue(TEXT("About three days of goods"), Room > 0 && Room <= 50000 * 2);
    TestEqual(TEXT("It is the order allowance"), MarketSuppliers::OrderAllowance(S), Room);
    const int32 BillsBefore = S.Payables.Num();
    MarketSuppliers::OnOrder(S, FMath::Min<int64>(Room, 20000));
    TestTrue(TEXT("Written on short terms"), S.Payables.Num() == BillsBefore + 1 && S.Payables.Last().DueDay == S.Day + MarketSuppliers::LifelineTerms);
    TestEqual(TEXT("Once a week"), MarketSuppliers::LifelineAllowance(S), int64(0));
    return true;
}

#endif

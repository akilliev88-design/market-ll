#include "MarketSuppliers.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// E3c2b (M63): a branch orders from the first store's wholesaler on the same account (volume, terms, bills).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBranchSupplyTest, "MirasMarket.Suppliers.BranchOrdersOnTheShopAccount", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketBranchSupplyTest::RunTest(const FString& Parameters)
{
    using namespace MarketSuppliers;
    FMarketState S; S.Day = 30; S.Cash = 500000;

    // A new relationship: cash, the volume still counts.
    Account(S, ESupplier::Regular).Trust = 40;
    S.Cash -= 20000; // the branch's order took the cash
    TestEqual(TEXT("Paid in cash without terms"), OnBranchOrder(S, 20000), int64(0));
    TestEqual(TEXT("Cash stays paid"), S.Cash, int64(480000));
    TestEqual(TEXT("Volume counted"), FindAccount(S, ESupplier::Regular)->Volume30, int64(20000));
    TestEqual(TEXT("No bill"), OpenBills(S), int64(0));

    // A trusted shop: the branch gets the same seven days.
    Account(S, ESupplier::Regular).Trust = TermsTrust;
    S.Cash -= 30000;
    TestEqual(TEXT("Bought on terms"), OnBranchOrder(S, 30000), int64(30000));
    TestEqual(TEXT("The cash came back"), S.Cash, int64(480000));
    TestTrue(TEXT("A bill due in seven days"), S.Payables.Num() == 1 && S.Payables[0].Amount == 30000 && S.Payables[0].DueDay == 37);
    TestEqual(TEXT("Open bills"), OpenBills(S), int64(30000));
    TestEqual(TEXT("Volume of both orders"), FindAccount(S, ESupplier::Regular)->Volume30, int64(50000));

    // An overdue bill closes the terms for the branches too.
    S.Day = 40;
    S.Cash -= 10000;
    TestEqual(TEXT("Overdue: cash again"), OnBranchOrder(S, 10000), int64(0));
    return true;
}

#endif

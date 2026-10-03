#include "MarketCampaign.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCampaignDebtTest, "MirasMarket.Campaign.DebtAndWeek", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCampaignDebtTest::RunTest(const FString& Parameters)
{
    FMarketProduct Milk; Milk.Id = TEXT("milk"); Milk.Cost = 170; Milk.BasePrice = 250;
    const TArray<FMarketProduct> Catalog = { Milk };
    FMarketState S; S.Initialize(Catalog);
    TestEqual(TEXT("New campaign starts with the inherited debt"), S.InheritedDebt, MarketCampaign::StartingDebt);
    TestTrue(TEXT("The debt is open"), MarketCampaign::DebtOpen(S));

    const int64 Cash = S.Cash;
    TestEqual(TEXT("One installment"), MarketCampaign::PayDebt(S), MarketCampaign::Installment);
    TestEqual(TEXT("Cash goes down"), S.Cash, Cash - MarketCampaign::Installment);
    TestEqual(TEXT("Debt goes down"), S.InheritedDebt, MarketCampaign::StartingDebt - MarketCampaign::Installment);
    TestTrue(TEXT("Progress bar moves"), MarketCampaign::DebtProgress(S) > 0.1f && MarketCampaign::DebtProgress(S) < 0.2f);
    S.Cash = 1000;
    TestEqual(TEXT("Cannot pay more than the cash"), MarketCampaign::PayDebt(S), int64(1000));
    TestEqual(TEXT("Cash can reach zero"), S.Cash, int64(0));
    TestEqual(TEXT("Nothing to pay with"), MarketCampaign::PayDebt(S), int64(0));
    S.Cash = 100000;
    while (MarketCampaign::PayDebt(S) > 0) {}
    TestFalse(TEXT("Debt closed"), MarketCampaign::DebtOpen(S));
    TestEqual(TEXT("Closed on day 1"), S.DebtClearedDay, 1);
    TestEqual(TEXT("Never below zero"), MarketCampaign::PayDebt(S), int64(0));
    TestEqual(TEXT("Paid this week"), S.WeekDebtPaid, MarketCampaign::StartingDebt);

    // Weekly report: days 1..7 make week 1.
    TestEqual(TEXT("Day 7 is week 1"), MarketCampaign::WeekOf(7), 1);
    TestEqual(TEXT("Day 8 is week 2"), MarketCampaign::WeekOf(8), 2);
    for (int32 Day = 1; Day <= 7; ++Day)
    {
        S.Revenue = 10000; S.Served = 10; S.Lost = 2;
        S.CloseDay();
        const bool bWeekDone = MarketCampaign::CloseDay(S);
        TestEqual(TEXT("Week ends only after day 7"), bWeekDone, Day == 7);
    }
    TestEqual(TEXT("Week number"), S.LastWeekNumber, 1);
    TestEqual(TEXT("Every closed day is in the history"), S.History.Num(), 7);
    TestEqual(TEXT("History keeps the day number"), S.History.Num() == 7 ? S.History[6].Day : 0, 7);
    TestEqual(TEXT("History keeps the revenue"), S.History.Num() == 7 ? S.History[0].Revenue : int64(0), int64(10000));
    TestEqual(TEXT("Week revenue"), S.LastWeekRevenue, int64(70000));
    TestEqual(TEXT("Week shoppers"), S.LastWeekServed, 70);
    TestEqual(TEXT("Week lost shoppers"), S.LastWeekLost, 14);
    TestEqual(TEXT("Week debt payments"), S.LastWeekDebtPaid, MarketCampaign::StartingDebt);
    TestEqual(TEXT("New week starts empty"), S.WeekRevenue + S.WeekDebtPaid, int64(0));
    TestTrue(TEXT("Valid save"), S.IsStructurallyValid());
    return true;
}

#endif

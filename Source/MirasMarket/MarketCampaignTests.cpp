#include "MarketCampaign.h"
#include "MarketRivals.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCampaignDebtTest, "MirasMarket.Campaign.DebtAndWeek", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCampaignDebtTest::RunTest(const FString& Parameters)
{
    FMarketProduct Milk; Milk.Id = TEXT("milk"); Milk.Cost = 170; Milk.BasePrice = 250;
    const TArray<FMarketProduct> Catalog = { Milk };
    FMarketState S; S.Initialize(Catalog);
    TestEqual(TEXT("New campaign starts with the inherited debt"), S.InheritedDebt, MarketCampaign::StartingDebt);
    TestTrue(TEXT("Second branch waits for the debt"), MarketCampaign::ExpandBlock(S) == MarketCampaign::EExpandBlock::Debt);

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
    S.Cash = MarketCampaign::ExpandCash;
    TestTrue(TEXT("Then the usual conditions"), MarketCampaign::ExpandBlock(S) == MarketCampaign::EExpandBlock::ProfitableDays);

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketRivalsTest, "MirasMarket.Rivals.News", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketRivalsTest::RunTest(const FString& Parameters)
{
    FMarketProduct A; A.Id = TEXT("a"); A.Category = TEXT("sut");
    FMarketProduct B; B.Id = TEXT("b"); B.Category = TEXT("icecek");
    FMarketProduct C; C.Id = TEXT("c"); C.Category = TEXT("bakliyat");
    FMarketProduct D; D.Id = TEXT("d"); D.Category = TEXT("sut");
    const TArray<FString> Aisles = MarketRivals::Aisles({ A, B, C, D });
    TestEqual(TEXT("Three distinct aisles"), Aisles.Num(), 3);
    TestEqual(TEXT("Sorted"), Aisles.Num() == 3 ? Aisles[0] : FString(), FString(TEXT("bakliyat")));
    TestEqual(TEXT("Logo folder of the first rival"), MarketRivals::RivalLogoKey(0), FString(TEXT("bim")));
    TestFalse(TEXT("Every rival has a format"), MarketRivals::RivalFormat(1).IsEmpty());

    constexpr int32 Seed = 7;
    TestEqual(TEXT("Quiet first day"), MarketRivals::NewsOn(1, Seed, Aisles).Num(), 0);
    TestEqual(TEXT("Quiet second day"), MarketRivals::NewsOn(2, Seed, Aisles).Num(), 0);
    int32 NewsCount = 0;
    TSet<uint8> Kinds;
    bool bFoundSale = false;
    for (int32 Day = 3; Day <= 60; ++Day)
    {
        const TArray<MarketRivals::FEvent> News = MarketRivals::NewsOn(Day, Seed, Aisles);
        TestEqual(TEXT("Same day, same news"), MarketRivals::NewsOn(Day, Seed, Aisles).Num(), News.Num());
        for (const MarketRivals::FEvent& Event : News)
        {
            ++NewsCount;
            Kinds.Add(static_cast<uint8>(Event.Kind));
            TestFalse(TEXT("Every news has a text"), MarketRivals::Describe(Event).IsEmpty());
            if (!bFoundSale && Event.Kind == MarketRivals::EKind::AisleSale)
            {
                bFoundSale = true;
                // Seed 7: the first news is an aisle sale on day 6 (drinks, -15 %) and nothing else is active then.
                TestEqual(TEXT("First sale day"), Event.FirstDay, 6);
                TestTrue(TEXT("Rival price of that aisle drops"), FMath::IsNearlyEqual(MarketRivals::PriceFactor(Day, Seed, Aisles, Event.Category), Event.PriceFactor));
                TestTrue(TEXT("Only that rival discounts"), FMath::IsNearlyEqual(MarketRivals::RivalFactor(Day, Seed, Aisles, Event.Category, Event.Rival), Event.PriceFactor)
                    && FMath::IsNearlyEqual(MarketRivals::RivalFactor(Day, Seed, Aisles, Event.Category, 1 - Event.Rival), 1.f));
                TestTrue(TEXT("Other aisles keep the list price"), FMath::IsNearlyEqual(MarketRivals::PriceFactor(Day, Seed, Aisles, Event.Category == TEXT("sut") ? TEXT("icecek") : TEXT("sut")), 1.f));
            }
        }
        const float Price = MarketRivals::PriceFactor(Day, Seed, Aisles, TEXT("sut"));
        const float Traffic = MarketRivals::TrafficFactor(Day, Seed, Aisles);
        TestTrue(TEXT("Price factor in range"), Price >= 0.7f && Price <= 1.3f);
        TestTrue(TEXT("Traffic factor in range"), Traffic >= 0.8f && Traffic <= 1.f);
    }
    TestTrue(TEXT("News on many days"), NewsCount >= 15);
    TestTrue(TEXT("Different kinds of news"), Kinds.Num() >= 4);
    TestTrue(TEXT("An aisle sale happened"), bFoundSale);
    TestEqual(TEXT("Two rivals at first"), MarketRivals::RivalCount(14), 2);
    TestEqual(TEXT("The new store opens on day 15"), MarketRivals::RivalCount(15), 3);
    bool bChain = false;
    for (const MarketRivals::FEvent& Event : MarketRivals::NewsOn(MarketRivals::ChainOpensDay, Seed, Aisles)) bChain |= Event.Kind == MarketRivals::EKind::NewRival;
    TestTrue(TEXT("Opening is in the news"), bChain);
    TestTrue(TEXT("The new store takes some shoppers for good"), MarketRivals::TrafficFactor(40, Seed, Aisles) <= 0.95f);
    TestTrue(TEXT("A different campaign brings different news"),
        MarketRivals::NewsOn(6, Seed, Aisles).Num() != MarketRivals::NewsOn(6, Seed + 1, Aisles).Num() ||
        MarketRivals::NewsOn(9, Seed, Aisles).Num() != MarketRivals::NewsOn(9, Seed + 1, Aisles).Num() ||
        MarketRivals::NewsOn(12, Seed, Aisles).Num() != MarketRivals::NewsOn(12, Seed + 1, Aisles).Num() ||
        MarketRivals::NewsOn(17, Seed, Aisles).Num() != MarketRivals::NewsOn(17, Seed + 1, Aisles).Num());
    return true;
}

#endif

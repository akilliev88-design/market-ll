#include "MarketCustomers.h"
#include "MarketCalendar.h"
#include "MarketDemand.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCustomerSegmentsTest, "MirasMarket.Customers.Segments", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCustomerSegmentsTest::RunTest(const FString& Parameters)
{
    using namespace MarketCustomers;
    const int32 Seed = 77;
    TestTrue(TEXT("Nermin teyze is retired"), SegmentOf(NerminTeyzeId, Seed) == ESegment::Retired);
    TestTrue(TEXT("A customer keeps the segment"), SegmentOf(5, Seed) == SegmentOf(5, Seed));
    int32 Counts[static_cast<int32>(ESegment::Count)] = {};
    for (int32 Id = 1; Id <= 2000; ++Id) ++Counts[static_cast<int32>(SegmentOf(Id, Seed))];
    TestTrue(TEXT("District mix: retired about 22 %"), Counts[0] > 340 && Counts[0] < 540);
    TestTrue(TEXT("District mix: traders about 6 %"), Counts[4] > 60 && Counts[4] < 200);

    // People differ.
    TestTrue(TEXT("Retirees walk slower than workers"), Profile(ESegment::Retired).WalkSpeed < Profile(ESegment::Worker).WalkSpeed);
    TestTrue(TEXT("Retirees wait longer"), Profile(ESegment::Retired).Patience > Profile(ESegment::Worker).Patience);
    TestTrue(TEXT("Students watch prices"), Profile(ESegment::Student).PriceTolerance < Profile(ESegment::Worker).PriceTolerance);
    const double Ratio = 1.2;
    TestTrue(TEXT("At 20 % above the rival a worker buys more often than a student"),
        MarketDemand::BuyChance(Ratio, 25.f, Profile(ESegment::Worker).PriceTolerance) > MarketDemand::BuyChance(Ratio, 25.f, Profile(ESegment::Student).PriceTolerance));

    // When they come.
    const int32 Tuesday = 2;
    TestTrue(TEXT("Retirees in the morning"), TimeWeight(ESegment::Retired, 0.1f, Tuesday) > TimeWeight(ESegment::Retired, 0.9f, Tuesday));
    TestTrue(TEXT("Workers in the evening"), TimeWeight(ESegment::Worker, 0.9f, Tuesday) > TimeWeight(ESegment::Worker, 0.1f, Tuesday));
    const int32 July = MarketCalendar::GameDayOf(2011, 7, 12);
    const int32 April = MarketCalendar::GameDayOf(2011, 4, 12);
    TestTrue(TEXT("Students are away in summer"), TimeWeight(ESegment::Student, 0.5f, July) < TimeWeight(ESegment::Student, 0.5f, April));
    TestTrue(TEXT("Families on Saturday"), TimeWeight(ESegment::Family, 0.5f, 6) > TimeWeight(ESegment::Family, 0.5f, 3));
    TestTrue(TEXT("Comes now: sure at high weight"), ComesNow(ESegment::Worker, 0.9f, Tuesday, 0.f) && !ComesNow(ESegment::Worker, 0.9f, Tuesday, 1.f));

    // What they want.
    FMarketProduct Tea; Tea.Id = TEXT("tea"); Tea.Category = TEXT("\u00e7ay-kahve"); Tea.BasePrice = 675;
    FMarketProduct Chips; Chips.Id = TEXT("chips"); Chips.Category = TEXT("bisk\u00fcvi-\u00e7ikolata"); Chips.BasePrice = 175;
    FMarketProduct Soap; Soap.Id = TEXT("soap"); Soap.Category = TEXT("temizlik"); Soap.BasePrice = 1990;
    FMarketProduct Milk; Milk.Id = TEXT("milk"); Milk.Category = TEXT("s\u00fct"); Milk.BasePrice = 250;
    const TArray<FMarketProduct> Products = { Tea, Chips, Soap, Milk };
    FMarketState S; S.Initialize(Products); S.RivalSeed = Seed; S.Day = April;
    S.ApplyShelfCapacities({ 12, 12, 12, 12 });
    FRandomStream Random(99);
    int32 ChildSweets = 0, ChildSoap = 0, RetiredTea = 0, RetiredSweets = 0;
    for (int32 I = 0; I < 400; ++I)
    {
        const TArray<int32> ChildList = BuildList(S, Products, ESegment::Child, Random);
        TestEqual(TEXT("A child wants one thing"), ChildList.Num(), 1);
        if (ChildList.Contains(1)) ++ChildSweets;
        if (ChildList.Contains(2)) ++ChildSoap;
        const TArray<int32> RetiredList = BuildList(S, Products, ESegment::Retired, Random);
        TestTrue(TEXT("Retired list size"), RetiredList.Num() >= 1 && RetiredList.Num() <= 3);
        if (RetiredList.Contains(0)) ++RetiredTea;
        if (RetiredList.Contains(1)) ++RetiredSweets;
    }
    TestTrue(TEXT("Children want sweets, not detergent"), ChildSweets > ChildSoap * 5);
    TestTrue(TEXT("Retirees want tea more than sweets"), RetiredTea > RetiredSweets);

    // Wallets.
    TestTrue(TEXT("Payday wallets are fuller"), VisitBudget(ESegment::Family, MarketCalendar::GameDayOf(2011, 4, 1)) > VisitBudget(ESegment::Family, MarketCalendar::GameDayOf(2011, 4, 29)));
    TestEqual(TEXT("Buys what the money allows"), Affordable(500, 200, 3), 2);
    TestEqual(TEXT("Nothing when broke"), Affordable(100, 200, 3), 0);
    TestEqual(TEXT("Never more than wanted"), Affordable(100000, 200, 3), 3);
    TestTrue(TEXT("A trader buys by the half case"), Profile(ESegment::Trader).MinQuantity >= 3);
    return true;
}

#endif

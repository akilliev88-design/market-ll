#include "MarketCompany.h"
#include "MarketOnline.h"
#include "MarketStaff.h"
#include "MarketStory.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketCompanyTest
{
    void AddPerson(FMarketState& S, int32 Id, MarketStaff::ERole Role)
    {
        FMarketEmployee E; E.Id = Id; E.Name = TEXT("Test"); E.Role = static_cast<uint8>(Role); E.DailyWage = 3000; E.HiredDay = 1;
        S.Staff.Add(E);
    }

    void Close(FMarketState& S)
    {
        S.DayNews.Reset();
        S.CloseDay();
        MarketCompany::CloseDay(S);
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCompanyTest, "MirasMarket.Company.GrowthAndLeadership", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCompanyTest::RunTest(const FString& Parameters)
{
    using namespace MarketCompany;
    using namespace MarketCompanyTest;
    FMarketState S; S.RivalSeed = 4; S.Cash = 50000000; S.InheritedDebt = 0;
    FString Message;

    // A company needs the chapter and people who run it.
    TestFalse(TEXT("Not before the Trakya chapter"), CanOpenStore(S, ECity::Corlu, Message));
    S.Story.Chapter = 4;
    TestFalse(TEXT("Needs HR and an accountant"), CanOpenStore(S, ECity::Corlu, Message));
    AddPerson(S, 1, MarketStaff::ERole::HrManager);
    AddPerson(S, 2, MarketStaff::ERole::Accountant);
    TestFalse(TEXT("\u0130stanbul waits for the next chapter"), CanOpenStore(S, ECity::IstanbulAvrupa, Message));
    const int64 Before = S.Cash;
    TestTrue(TEXT("\u00c7orlu"), OpenStore(S, ECity::Corlu, Message));
    TestEqual(TEXT("Paid the store"), Before - S.Cash, StoreCost(S, ECity::Corlu));
    TestEqual(TEXT("Two stores with the family shop"), TotalStores(S), 2);
    TestEqual(TEXT("Two provinces"), Provinces(S), 2);

    // Logistics: far stores lose margin without a depot; the depot needs four stores.
    const float LossWithout = LogisticsLoss(S, ECity::Corlu);
    TestTrue(TEXT("Far store without a depot"), LossWithout > 0.f);
    TestEqual(TEXT("Home province has no logistics loss"), LogisticsLoss(S, ECity::Kirklareli), 0.f);
    TestFalse(TEXT("Depot needs four stores"), Build(S, 0, Message));
    OpenStore(S, ECity::Kirklareli, Message);
    OpenStore(S, ECity::Babaeski, Message);
    TestTrue(TEXT("Depot"), Build(S, 0, Message));
    TestTrue(TEXT("Truck"), Build(S, 1, Message));
    TestTrue(TEXT("Depot lowers the loss"), LogisticsLoss(S, ECity::Corlu) < LossWithout);
    TestTrue(TEXT("Depot raises the margin"), Margin(S, ECity::Corlu) > 0.2f);

    // Stores learn their customers and earn.
    for (int32 D = 0; D < 70; ++D) Close(S);
    const FMarketCityStores* Home = Find(S, ECity::Kirklareli);
    TestTrue(TEXT("Habit built"), Home && Home->Maturity >= 1.f);
    TestTrue(TEXT("A mature store earns"), Home && Home->LastProfit > 0);
    TestTrue(TEXT("City results reach the day's net"), S.LastBranchProfit == S.Company.LastProfit);

    // Chapter 4 goals: 8 stores in 2 provinces, depot, truck.
    TestEqual(TEXT("Three goals"), MarketStory::Objectives(S).Num(), 3);
    while (TotalStores(S) < 8 && OpenStore(S, ECity::Edirne, Message)) {}
    while (TotalStores(S) < 8 && OpenStore(S, ECity::Tekirdag, Message)) {}
    TestTrue(TEXT("Eight stores"), TotalStores(S) >= 8);
    MarketStory::CloseDay(S, {});
    TestEqual(TEXT("On to T\u00fcrkiye"), S.Story.Chapter, 5);
    TestTrue(TEXT("Central buying"), Build(S, 2, Message));
    TestFalse(TEXT("Own brand needs 20 stores"), Build(S, 3, Message));

    // Abroad: the first months cost margin.
    S.Story.Chapter = 6;
    TestTrue(TEXT("K\u0131rcaali pilot"), OpenStore(S, ECity::Kircaali, Message));
    TestTrue(TEXT("Learning a new country"), Margin(S, ECity::Kircaali) < Margin(S, ECity::Edirne));
    TestTrue(TEXT("National share counts Turkish stores"), NationalShare(S) > 0.f && NationalShare(S) < TotalStores(S) * 0.04f);

    // Dark store helps the web shop.
    FMarketState Online = S;
    Online.Story.Chapter = 5;
    Online.Cash = 2000000000;   // twenty city stores
    Online.Online.bWeb = true;
    while (TotalStores(Online) < 20 && OpenStore(Online, ECity::IstanbulAvrupa, Message)) {}
    const int32 Capacity = MarketOnline::DeliveryCapacity(Online);
    TestTrue(TEXT("Dark store"), Build(Online, 4, Message));
    TestTrue(TEXT("More deliveries"), MarketOnline::DeliveryCapacity(Online) > Capacity);

    // Chapter 7: a year in front on every measure ends in "Miras".
    FMarketState Lead = Online;
    Lead.Story.Chapter = 7;
    Lead.MarketShare = 45.f;
    while (TotalStores(Lead) < 60 && (OpenStore(Lead, ECity::IstanbulAnadolu, Message) || OpenStore(Lead, ECity::Izmir, Message) || OpenStore(Lead, ECity::Ankara, Message))) {}
    FMarketLoyalty L; L.CustomerId = 1; L.Satisfaction = 80.f; Lead.Loyalty.Add(L);
    Lead.Company.LeadershipDays = LeadershipGoalDays - 1;
    Lead.LastProfit = 100000000;   // a good day for the whole company
    TestTrue(TEXT("Leads today"), LeadsToday(Lead));
    Lead.DayNews.Reset();
    MarketCompany::CloseDay(Lead);
    TestTrue(TEXT("Miras ending"), Lead.Story.Ending == static_cast<uint8>(MarketStory::EEnding::Legacy));
    TestFalse(TEXT("Summary"), Summary(Lead).IsEmpty());
    return true;
}

#endif

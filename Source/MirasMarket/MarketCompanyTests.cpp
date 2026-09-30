#include "MarketCompany.h"
#include "MarketBranches.h"
#include "MarketOnline.h"
#include "MarketStaff.h"
#include "MarketStory.h"
#include "MarketEvents.h"
#include "MarketPrices.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketCompanyTest
{
    void AddPerson(FMarketState& S, int32 Id, MarketStaff::ERole Role)
    {
        FMarketEmployee E; E.Id = Id; E.Name = TEXT("Test"); E.Role = static_cast<uint8>(Role); E.DailyWage = 3000; E.HiredDay = 1;
        S.Staff.Add(E);
    }

    // An open shop without the opening steps (the company rules only look at where and what it is).
    void AddShop(FMarketState& S, const TCHAR* Country, const TCHAR* Province, const TCHAR* Format = TEXT("mahalle"), int32 OpenedDay = 1)
    {
        FMarketBranch B;
        B.Country = Country; B.Province = Province; B.Format = Format; B.Name = Province;
        B.Stage = static_cast<uint8>(MarketBranches::EStage::Open);
        B.OpenedDay = OpenedDay;
        B.Rent = 60000;
        S.Branches.Add(B);
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
    // G-086: the company around province branches; the whole country is open from the start.
    using namespace MarketCompany;
    using namespace MarketCompanyTest;
    FMarketState S; S.RivalSeed = 4; S.Cash = 50000000; S.InheritedDebt = 0; S.Day = 200;
    S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
    FString Message;

    TestEqual(TEXT("The family shop"), TotalStores(S), 1);
    TestEqual(TEXT("One province"), Provinces(S), 1);
    AddShop(S, TEXT("tr"), TEXT("tekirdag"));
    TestEqual(TEXT("Two stores"), TotalStores(S), 2);
    TestEqual(TEXT("Two provinces"), Provinces(S), 2);
    TestEqual(TEXT("Far store"), FarStores(S), 1);

    // Logistics: a far shop without a depot pays the van; a home-province shop does not.
    const float Without = CostFactor(S, S.Branches[0]);
    TestTrue(TEXT("Far shop without a depot"), Without > 1.f);
    AddShop(S, TEXT("tr"), TEXT("kirklareli"));
    TestTrue(TEXT("Home shop has no freight"), FMath::IsNearlyEqual(CostFactor(S, S.Branches[1]), 1.f));
    TestFalse(TEXT("A depot needs four stores"), Build(S, 0, Message));
    AddShop(S, TEXT("tr"), TEXT("edirne"));
    TestFalse(TEXT("No depot where we have no shop"), BuildDepot(S, TEXT("tr"), TEXT("dicle"), Message));
    TestTrue(TEXT("Trakya depot"), BuildDepot(S, TEXT("tr"), TEXT("trakya"), Message));
    TestTrue(TEXT("Depot recorded"), HasDepot(S, TEXT("tr"), TEXT("trakya")) && DepotCount(S) == 1);
    // G-089: the sub-region button builds in the province nearest to the branches (Tekirdag between them).
    TestTrue(TEXT("Depot in a province"), S.Company.DepotSites.Num() == 1 && S.Company.DepotSites[0].Province == TEXT("tekirdag"));
    TestFalse(TEXT("Only one per sub-region"), BuildDepot(S, TEXT("tr"), TEXT("trakya"), Message));
    TestTrue(TEXT("The depot lowers the cost"), CostFactor(S, S.Branches[0]) < Without);
    TestTrue(TEXT("Truck"), Build(S, 1, Message));
    TestTrue(TEXT("Home shop gains from the depot too"), CostFactor(S, S.Branches[1]) < 1.f);

    // Head office and the story's chapter 4 goals (8 stores in 2 provinces, depot, truck).
    S.Story.Chapter = 4;
    TestEqual(TEXT("Three goals"), MarketStory::Objectives(S).Num(), 3);
    while (TotalStores(S) < 8) AddShop(S, TEXT("tr"), TEXT("tekirdag"));
    MarketStory::CloseDay(S, {});
    TestEqual(TEXT("On to the whole country"), S.Story.Chapter, 5);
    TestTrue(TEXT("Central buying"), Build(S, 2, Message));
    TestFalse(TEXT("Own brand needs 20 stores"), Build(S, 3, Message));
    TestTrue(TEXT("National share"), NationalShare(S) > 0.f);
    const int64 CashBefore = S.Cash;
    Close(S);
    TestTrue(TEXT("Head office costs money"), S.Cash < CashBefore && S.Company.LastProfit < 0);

    // Abroad: customs and a learning period, counted per country.
    S.Story.Chapter = 6;
    AddShop(S, TEXT("de"), TEXT("by"), TEXT("mahalle"), S.Day - 10);
    TestEqual(TEXT("One foreign country"), ForeignCountries(S), 1);
    const float Abroad = CostFactor(S, S.Branches.Last());
    TestTrue(TEXT("Learning a new country"), Abroad > CostFactor(S, S.Branches[0]));
    TestEqual(TEXT("Stores in Germany"), CountryStores(S, TEXT("de")), 1);
    TestEqual(TEXT("Two chapter 6 goals"), MarketStory::Objectives(S).Num(), 2);

    // Dark store helps the web shop.
    FMarketState Online = S;
    Online.Story.Chapter = 5;
    Online.Online.bWeb = true;
    while (TotalStores(Online) < 20) AddShop(Online, TEXT("tr"), TEXT("istanbul"));
    const int32 Capacity = MarketOnline::DeliveryCapacity(Online);
    TestTrue(TEXT("Dark store"), Build(Online, 4, Message));
    TestTrue(TEXT("More deliveries"), MarketOnline::DeliveryCapacity(Online) > Capacity);

    // Chapter 7: a year in front on every measure ends in "Miras".
    FMarketState Lead = Online;
    Lead.Story.Chapter = 7;
    Lead.MarketShare = 45.f;
    while (TotalStores(Lead) < 60) AddShop(Lead, TEXT("tr"), TEXT("ankara"));
    FMarketLoyalty L; L.CustomerId = 1; L.Satisfaction = 80.f; Lead.Loyalty.Add(L);
    Lead.Company.LeadershipDays = LeadershipGoalDays - 1;
    Lead.LastProfit = 100000000;   // a good day for the whole company
    TestTrue(TEXT("Leads today"), LeadsToday(Lead));
    Lead.DayNews.Reset();
    MarketCompany::CloseDay(Lead);
    TestTrue(TEXT("Miras ending"), Lead.Story.Ending == static_cast<uint8>(MarketStory::EEnding::Legacy));
    // Karar J02: the finale is shown once, then free play without new story content.
    TestTrue(TEXT("Finale shown"), Lead.Decisions.ContainsByPredicate([](const FMarketDecision& D) { return D.Id == TEXT("story.finale"); }));
    TestTrue(TEXT("Story closed"), MarketStory::StoryClosed(Lead));
    TestEqual(TEXT("No goals after the finale"), MarketStory::Objectives(Lead).Num(), 0);
    TestFalse(TEXT("Finale only once"), MarketStory::ReachFinale(Lead, MarketStory::EEnding::TimeUp));
    TestFalse(TEXT("Summary"), Summary(Lead).IsEmpty());
    return true;
}

#endif

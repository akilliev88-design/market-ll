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
        B.LastRevenue = 90000; // B1 (#45): the national share follows revenue
        S.Branches.Add(B);
    }

    void Close(FMarketState& S)
    {
        S.DayNews.Reset();
        S.CloseDay();
        MarketCompany::CloseDay(S);
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCompanyTest, "MarketSim.Company.GrowthAndLeadership", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCompanyTest::RunTest(const FString& Parameters)
{
    // G-086: the company around province branches; the whole country is open from the start.
    using namespace MarketCompany;
    using namespace MarketCompanyTest;
    FMarketState S; S.RivalSeed = 4; S.Cash = 50000000; S.InheritedDebt = 0; S.Day = 200;
    S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
    FString Message;

    TestEqual(TEXT("The first store"), TotalStores(S), 1);
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

    // Head office; M69: the big store types open with the company's size (8 shops in 2 provinces), not a chapter.
    FString Why;
    TestFalse(TEXT("No hypermarket for a small company"), MarketBranches::FormatOpen(S, TEXT("hiper"), Why) || Why.IsEmpty());
    TestTrue(TEXT("A neighbourhood market from the start"), MarketBranches::FormatOpen(S, TEXT("mahalle"), Why));
    while (TotalStores(S) < 8) AddShop(S, TEXT("tr"), TEXT("tekirdag"));
    TestTrue(TEXT("Hypermarket with 8 shops in 2 provinces"), MarketBranches::FormatOpen(S, TEXT("hiper"), Why));
    TestTrue(TEXT("Central buying"), Build(S, 2, Message));
    TestFalse(TEXT("Own brand needs 20 stores"), Build(S, 3, Message));
    TestTrue(TEXT("National share"), NationalShare(S) > 0.f);
    const int64 CashBefore = S.Cash;
    Close(S);
    TestTrue(TEXT("Head office costs money"), S.Cash < CashBefore && S.Company.LastProfit < 0);

    // Abroad: customs and a learning period, counted per country.
    AddShop(S, TEXT("de"), TEXT("by"), TEXT("mahalle"), S.Day - 10);
    TestEqual(TEXT("One foreign country"), ForeignCountries(S), 1);
    const float Abroad = CostFactor(S, S.Branches.Last());
    TestTrue(TEXT("Learning a new country"), Abroad > CostFactor(S, S.Branches[0]));
    TestEqual(TEXT("Stores in Germany"), CountryStores(S, TEXT("de")), 1);

    // A big company (dark stores are per province now: MarketOnline, M32).
    FMarketState Online = S;
    while (TotalStores(Online) < 20) AddShop(Online, TEXT("tr"), TEXT("istanbul"));
    TestFalse(TEXT("No dark store among the company builds"), Build(Online, 4, Message));

    // M69: a big company goes on; there is no finale and the summary has no "Miras".
    FMarketState Lead = Online;
    while (TotalStores(Lead) < 60) AddShop(Lead, TEXT("tr"), TEXT("ankara"));
    Lead.DayNews.Reset();
    MarketCompany::CloseDay(Lead);
    TestFalse(TEXT("Never closed by size"), MarketStory::StoryClosed(Lead));
    TestFalse(TEXT("No finale card"), Lead.Decisions.ContainsByPredicate([](const FMarketDecision& D) { return D.Id == TEXT("story.finale"); }));
    TestFalse(TEXT("Summary"), Summary(Lead).IsEmpty());
    TestFalse(TEXT("No Miras in the summary"), Summary(Lead).Contains(TEXT("Miras")));
    return true;
}

#endif

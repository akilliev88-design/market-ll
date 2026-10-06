#include "MarketOwner.h"
#include "MarketStart.h"
#include "MarketCountry.h"
#include "MarketStaff.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketStartTest
{
    TArray<FMarketProduct> Catalog()
    {
        TArray<FMarketProduct> Products;
        const TCHAR* Ids[] = { TEXT("sut"), TEXT("makarna"), TEXT("cay"), TEXT("cola") };
        for (const TCHAR* Id : Ids)
        {
            FMarketProduct P;
            P.Id = Id; P.RealName = Id; P.Category = TEXT("genel"); P.Cost = 100; P.BasePrice = 150; P.CaseUnits = 12;
            Products.Add(P);
        }
        return Products;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStartTest, "MirasMarket.Start.HandedOverShop", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStartTest::RunTest(const FString& Parameters)
{
    // G-084, M69: a new campaign in a country and a city: a family handed its small market over to the player.
    using namespace MarketStart;
    const TArray<FMarketProduct> Products = MarketStartTest::Catalog();

    // A new shop: one cashier and two stockers, a month of costs in the till.
    FMarketState S; S.Initialize(Products);
    S.Company.BrandName = TEXT("Y\u0131ld\u0131z Market"); S.PlayerName = TEXT("  Deniz  ");
    const int64 CashBefore = S.Cash;
    Setup(S, TEXT("tr"), FString(), 4242);
    TestEqual(TEXT("Three people"), S.Staff.Num(), 3);
    TestEqual(TEXT("One cashier"), MarketStaff::Count(S, MarketStaff::ERole::Cashier), 1);
    TestEqual(TEXT("Two stockers"), MarketStaff::Count(S, MarketStaff::ERole::Stocker), 2);
    TestTrue(TEXT("Cashier on duty"), MarketStaff::CashierOnDuty(S));
    TestEqual(TEXT("Stockers on duty"), MarketStaff::StockersOnDuty(S), 2);
    int64 Payroll = 0;
    for (const FMarketEmployee& E : S.Staff) Payroll += E.DailyWage;
    TestTrue(TEXT("Wages are paid"), Payroll > 0);
    // M37: a month of the shop's fixed costs in the till, the inherited debt a month and a half.
    TestTrue(TEXT("A month of costs in the till"), S.Cash > 30 * Payroll && S.Cash != CashBefore);
    TestTrue(TEXT("The inherited debt"), S.InheritedDebt == S.StartDebt && S.InheritedDebt > S.Cash);
    TestEqual(TEXT("Our salary"), S.Owner.SalaryX10, MarketOwner::StartSalaryX10);
    TestEqual(TEXT("Seed kept"), S.RivalSeed, 4242);
    TestEqual(TEXT("Country"), S.CountryId, FString(TEXT("tr")));
    // No default province for players; automated runs fall back to the country's median province (M61b).
    if (MarketCountry::FindCity(TEXT("tr"), TEXT("kirklareli")))
    {
        const MarketCountry::FProfile* Tr = MarketCountry::Find(TEXT("tr"));
        TestEqual(TEXT("Fallback province: the median one"), S.CityId, Tr->MedianProvince);
        int32 Smaller = 0, Bigger = 0;
        for (const MarketCountry::FCity& City : Tr->Cities) { Smaller += City.PopulationK < Tr->MedianPopK ? 1 : 0; Bigger += City.PopulationK > Tr->MedianPopK ? 1 : 0; }
        TestTrue(TEXT("Median: as many smaller as bigger provinces"), FMath::Abs(Smaller - Bigger) <= 1);
        FMarketState Chosen; Chosen.Initialize(Products);
        Setup(Chosen, TEXT("tr"), TEXT("van"), 7);
        TestEqual(TEXT("The chosen province"), Chosen.CityId, FString(TEXT("van")));
        TestEqual(TEXT("81 provinces"), MarketCountry::Find(TEXT("tr"))->Cities.Num(), 81);
        const MarketCountry::FRegion* Sub = MarketCountry::SubRegionOf(TEXT("tr"), TEXT("kirklareli"));
        TestTrue(TEXT("Kirklareli is in Trakya"), Sub && Sub->Id == TEXT("trakya"));
        const MarketCountry::FRegion* Main = MarketCountry::RegionOf(TEXT("tr"), TEXT("van"));
        TestTrue(TEXT("Van is in Eastern Anatolia"), Main && Main->Id == TEXT("doguanadolu"));
        // M61b: no province is the 1.0 point; the biggest city is more crowded than the median one.
        const MarketCountry::FCity* Middle = MarketCountry::FindCity(TEXT("tr"), Tr->MedianProvince);
        const MarketCountry::FCity* Istanbul = MarketCountry::FindCity(TEXT("tr"), TEXT("istanbul"));
        TestTrue(TEXT("Crowded biggest city"), Middle && Istanbul && Istanbul->Competition > Middle->Competition);
        const MarketCountry::FCity* Van = MarketCountry::FindCity(TEXT("tr"), TEXT("van"));
        TestTrue(TEXT("Van: cheaper rent, poorer shoppers"), Van && Van->Rent < 1.f && Van->Income < 1.f);
        for (const MarketCountry::FProfile& Pack : MarketCountry::All())
            for (const MarketCountry::FCity& City : Pack.Cities)
                TestFalse(*FString::Printf(TEXT("%s/%s has a sub-region"), *Pack.Id, *City.Id), City.SubRegion.IsEmpty());
    }

    // Same seed, same shop (a reload or a replay never rerolls it).
    FMarketState Again; Again.Initialize(Products);
    Setup(Again, TEXT("tr"), FString(), 4242);
    TestEqual(TEXT("Same cashier"), Again.Staff[0].Name, S.Staff[0].Name);

    // Unknown country: Turkey.
    FMarketState Lost; Lost.Initialize(Products);
    Setup(Lost, TEXT("atlantis"), TEXT("x"), 1);
    TestEqual(TEXT("Unknown country falls back"), Lost.CountryId, FString(TEXT("tr")));

    // Texts (M69): the family handed the market over; no father, no year, the player's and the market's names.
    const FString Intro = IntroText(S);
    TestTrue(TEXT("Intro: the family handed it over"), Intro.Contains(TEXT("aile")) && Intro.Contains(TEXT("devretti")));
    TestFalse(TEXT("Intro: no father"), Intro.Contains(TEXT("baba")));
    TestTrue(TEXT("Intro: the market's name"), Intro.Contains(TEXT("Y\u0131ld\u0131z Market")));
    TestTrue(TEXT("Intro: the player's name"), Intro.StartsWith(TEXT("Deniz")));
    TestEqual(TEXT("Player name trimmed"), PlayerName(S), FString(TEXT("Deniz")));
    TestTrue(TEXT("First store's name: market and city"), FirstStoreName(S).StartsWith(TEXT("Y\u0131ld\u0131z Market ")));
    TestTrue(TEXT("Place text"), PlaceText(S).Contains(TEXT("T\u00fcrkiye")));

    // Shelves in order but not full: 40-75 % of each shelf, units only move from the warehouse.
    for (FMarketStock& Row : S.Stock) { Row.Capacity = 20; Row.Warehouse = 30; Row.Shelf = 0; }
    int32 TotalBefore = 0;
    for (const FMarketStock& Row : S.Stock) TotalBefore += Row.Shelf + Row.Warehouse;
    const int32 Moved = StockShelvesPartly(S, 4242);
    int32 TotalAfter = 0;
    for (const FMarketStock& Row : S.Stock)
    {
        TotalAfter += Row.Shelf + Row.Warehouse;
        TestTrue(TEXT("Partly full"), Row.Shelf >= 8 && Row.Shelf <= 15);
    }
    TestEqual(TEXT("Nothing appears or vanishes"), TotalAfter, TotalBefore);
    TestTrue(TEXT("Units moved"), Moved > 0);
    FMarketState Empty = S;
    for (FMarketStock& Row : Empty.Stock) { Row.Shelf = 0; Row.Warehouse = 0; }
    TestEqual(TEXT("An empty warehouse moves nothing"), StockShelvesPartly(Empty, 1), 0);

    // City competition: the prototype's town is neutral, an unknown city too.
    TestEqual(TEXT("Unknown city"), MarketCountry::CityCompetition(TEXT("tr"), TEXT("nowhere")), 1.f);
    TestEqual(TEXT("No city"), MarketCountry::CityCompetition(TEXT("tr"), FString()), 1.f);
    if (MarketCountry::FindCity(TEXT("tr"), TEXT("istanbul")))
        TestTrue(TEXT("Istanbul is crowded"), MarketCountry::CityCompetition(TEXT("tr"), TEXT("istanbul")) > 1.f);
    return true;
}

#endif

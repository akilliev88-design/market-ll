#include "MarketStoreDemand.h"
#include "MarketBranches.h"
#include "MarketCountry.h"
#include "MarketDirector.h"
#include "MarketEconomy.h"
#include "MarketEras.h"
#include "MarketSimulation.h"
#include "MarketStart.h"
#include "MarketChains.h"
#include "MarketCampaign.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// E2 (Docs/Kurgu/11_TEK_EKONOMI.md): one shopper formula for the first store and the branches.
namespace MarketStoreDemandTest
{
    FMarketProduct Make(const TCHAR* Id, const TCHAR* Category, int64 Cost, int64 Price)
    {
        FMarketProduct P;
        P.Id = Id; P.RealName = Id; P.Category = Category; P.Cost = Cost; P.BasePrice = Price; P.CaseUnits = 12; P.bActive = true;
        return P;
    }

    TArray<FMarketProduct> Catalog()
    {
        return {
            Make(TEXT("sut"), TEXT("s\u00fct"), 170, 250),
            Make(TEXT("ayran"), TEXT("s\u00fct"), 60, 100),
            Make(TEXT("makarna"), TEXT("makarna-bakliyat"), 210, 325),
            Make(TEXT("cay"), TEXT("\u00e7ay-kahve"), 480, 675),
            Make(TEXT("cola"), TEXT("i\u00e7ecek"), 180, 275),
            Make(TEXT("deterjan"), TEXT("temizlik"), 1400, 1990),
        };
    }

    // A first store in the default country, founded and stocked the way the game does.
    FMarketState NewShop(const TArray<FMarketProduct>& Base, TArray<FMarketProduct>& Products, int32 Seed)
    {
        Products = Base;
        FMarketState S; S.Initialize(Base);
        MarketStart::Setup(S, MarketCountry::DefaultId(), FString(), Seed);
        MarketCountry::SetActive(S.CountryId, S.RivalSeed);
        MarketEras::Activate(S);
        S.ApplyShelfCapacities({ 24, 24, 24, 24, 24, 24 });
        MarketStart::StockShelvesPartly(S, Seed);
        MarketDirector::ApplyPrices(S, Base, Products);
        return S;
    }

    struct FRun { int64 Revenue = 0; int32 Shoppers = 0; int32 Audit = 0; };

    FRun Play(int32 Days, int32 Seed)
    {
        const TArray<FMarketProduct> Base = Catalog();
        TArray<FMarketProduct> Products;
        FMarketState S = NewShop(Base, Products, Seed);
        FRun Run;
        for (int32 D = 0; D < Days; ++D)
        {
            const MarketSimulation::FDay Day = MarketSimulation::PlayDay(S, Base, Products);
            Run.Revenue += Day.Revenue;
            Run.Shoppers += Day.Shoppers;
            Run.Audit += Day.AuditFailures;
        }
        return Run;
    }

    // E2a measurement (03.10.2026, the old street model with MarketRivals and MarketCompetitors, seed 23): the family
    // shop's first 90 days in this catalog.
    constexpr int64 OldRevenue90 = 7341275;
    constexpr int32 OldShoppers90 = 4323;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStoreDemandSameTest, "MirasMarket.StoreDemand.SameFormulaFirstStoreAndBranch", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStoreDemandSameTest::RunTest(const FString& Parameters)
{
    using namespace MarketStoreDemandTest;
    const TArray<FMarketProduct> Base = Catalog();
    TArray<FMarketProduct> Products;
    FMarketState S = NewShop(Base, Products, 11);
    const FString Home = MarketStart::HomeProvince(S);
    // One neighbourhood branch in the home province: it and the first store share the room the same way.
    FMarketBranch B;
    B.Name = TEXT("test"); B.Province = Home; B.Format = TEXT("mahalle"); B.Stage = static_cast<uint8>(MarketBranches::EStage::Open);
    B.OpenedDay = 1; B.Maturity = 1.f; B.Satisfaction = 50.f; B.PriceIndex = 1.f;
    S.Branches.Add(B);
    S.Day = 20;

    MarketStoreDemand::FStoreDay Family;
    Family.Country = S.CountryId; Family.Province = Home; Family.Format = TEXT("mahalle"); Family.Self = MarketStoreDemand::FirstStore;
    MarketStoreDemand::FStoreDay Branch = Family;
    Branch.Self = 0;
    const int32 Day = 20;
    const float A = MarketStoreDemand::ShoppersExact(S, Family, Day);
    const float C = MarketStoreDemand::ShoppersExact(S, Branch, Day);
    TestTrue(TEXT("People come"), A > 1.f);
    TestTrue(TEXT("Same conditions, same shoppers (5 %)"), FMath::Abs(A - C) <= 0.05f * FMath::Max(A, C));
    TestTrue(TEXT("The branch counts the first store as a neighbour"), FMath::IsNearlyEqual(MarketStoreDemand::Cannibalization(S, Family), MarketStoreDemand::Cannibalization(S, Branch)));

    // The formula responds the way a store should.
    MarketStoreDemand::FStoreDay Dear = Family; Dear.PriceIndex = 1.1f;
    TestTrue(TEXT("Dearer shelves, fewer shoppers"), MarketStoreDemand::ShoppersExact(S, Dear, Day) < A);
    MarketStoreDemand::FStoreDay Empty = Family; Empty.Availability = 0.4f;
    TestTrue(TEXT("Empty shelves, fewer shoppers"), MarketStoreDemand::ShoppersExact(S, Empty, Day) < A);
    MarketStoreDemand::FStoreDay Young = Family; Young.Maturity = 0.f;
    TestTrue(TEXT("A new store has not grown into its share"), MarketStoreDemand::ShoppersExact(S, Young, Day) < A);
    S.Branches.Reset();
    TestTrue(TEXT("Alone in the province: more for the first store"), MarketStoreDemand::ShoppersExact(S, Family, Day) >= A);

    // The first store's own day: one walking figure per real shopper.
    const int32 Real = MarketStoreDemand::FirstStoreShoppers(S, Products, Day);
    AddInfo(FString::Printf(TEXT("OLCUM: ilk magaza gercek musteri (gun 20) %d, gorsel olcek %d"), Real, MarketSimulation::ShoppersPerDay));
    TestTrue(TEXT("First store has shoppers"), Real > 0);
    MarketCountry::SetActive(MarketCountry::DefaultId(), 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStoreDemandBeforeAfterTest, "MirasMarket.StoreDemand.FirstStoreBeforeAfter", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStoreDemandBeforeAfterTest::RunTest(const FString& Parameters)
{
    // E2 calibration (11_TEK_EKONOMI \u00a72 E2.8): the first store's first 90 days with the one store formula against
    // the old street model's measured numbers. Target +-15 % revenue; the run reports the numbers (OLCUM) and fails
    // only on a gross gap.
    using namespace MarketStoreDemandTest;
    const FRun New = Play(90, 23);
    const double Ratio = static_cast<double>(New.Revenue) / static_cast<double>(OldRevenue90);
    AddInfo(FString::Printf(TEXT("OLCUM: 90 gun ciro eski %lld yeni %lld oran %.3f; musteri eski %d yeni %d"),
        static_cast<long long>(OldRevenue90), static_cast<long long>(New.Revenue), Ratio, OldShoppers90, New.Shoppers));
    TestTrue(TEXT("The shop sells"), New.Revenue > 0);
    TestEqual(TEXT("No audit gap"), New.Audit, 0);
    TestTrue(TEXT("Not a gross gap (0.6 .. 1.6)"), Ratio > 0.6 && Ratio < 1.6);
    MarketCountry::SetActive(MarketCountry::DefaultId(), 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStoreDemandOneRivalTest, "MirasMarket.StoreDemand.OneRivalModel", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStoreDemandOneRivalTest::RunTest(const FString& Parameters)
{
    // E2 (11_TEK_EKONOMI \u00a72 E2.9): the first store's traffic is the formula and nothing else (no rival news), the
    // rivals' price is the province's chains, a chain's price war cuts our share, and the local share follows the
    // formula (the story's share goal, MarketCampaign::ShareGoal, looks at it).
    using namespace MarketStoreDemandTest;
    const TArray<FMarketProduct> Base = Catalog();
    TArray<FMarketProduct> Products;
    FMarketState S = NewShop(Base, Products, 31);
    const FString Home = MarketStart::HomeProvince(S);
    TestTrue(TEXT("The province's chains are there from day 1"), MarketChains::RivalsIn(S, S.CountryId, Home, 3).Num() > 0);

    // Traffic: exactly the formula's shoppers over the visual scale, the same every time it is asked.
    for (int32 Day = 2; Day <= 40; Day += 6)
    {
        S.Day = Day;
        const float Traffic = MarketDirector::TrafficFactor(S, Products);
        const float Expected = static_cast<float>(MarketStoreDemand::FirstStoreShoppers(S, Products, Day)) / static_cast<float>(MarketSimulation::ShoppersPerDay);
        TestTrue(*FString::Printf(TEXT("Day %d: traffic = formula"), Day), FMath::IsNearlyEqual(Traffic, Expected));
        TestTrue(*FString::Printf(TEXT("Day %d: asked twice, same"), Day), FMath::IsNearlyEqual(Traffic, MarketDirector::TrafficFactor(S, Products)));
    }
    S.Day = 20;

    // The rivals' price is the chains' weighted price level.
    const float Rival = MarketDirector::RivalPriceFactor(S, FString());
    TestTrue(TEXT("Rival price from the chains"), FMath::IsNearlyEqual(Rival, MarketChains::RivalPriceFactor(S, S.CountryId, Home, S.Day)) && Rival >= 0.7f && Rival <= 1.3f);

    // A price war of the biggest chain against us: dearer competition, lower share, cheaper rival shelves.
    const MarketStoreDemand::FStoreDay Shop = MarketStoreDemand::FirstStoreDay(S, Products, S.Day);
    const float Before = MarketStoreDemand::Share(S, Shop, S.Day);
    FMarketState War = S;
    const int32 Biggest = MarketChains::RivalsIn(War, War.CountryId, Home, 1).Num() > 0 ? MarketChains::RivalsIn(War, War.CountryId, Home, 1)[0] : INDEX_NONE;
    if (TestTrue(TEXT("A chain to fight"), Biggest != INDEX_NONE))
    {
        War.Rivals.Chains[Biggest].WarProvince = Home;
        War.Rivals.Chains[Biggest].WarUntil = War.Day + 10;
        TestTrue(TEXT("War cuts our share"), MarketStoreDemand::Share(War, Shop, War.Day) < Before);
        TestTrue(TEXT("War cuts the rivals' price"), MarketDirector::RivalPriceFactor(War, FString()) < Rival);
    }

    // The local share follows the formula at the close.
    FMarketState Close = S;
    Close.Day = 21; Close.LastServed = 40; Close.ShareBeforeClose = 10.f; Close.MarketShare = 10.f;
    MarketStoreDemand::CloseDay(Close, Products);
    const float Target = MarketStoreDemand::Share(Close, MarketStoreDemand::FirstStoreDay(Close, Products, 20), 20) * 100.f;
    TestTrue(TEXT("Share moves towards the formula"), FMath::IsNearlyEqual(Close.MarketShare, FMath::Clamp(10.f * 0.85f + Target * 0.15f, 5.f, 65.f), 0.01f));
    // M61: the share goal follows the city (a corner shop's share is smaller in the biggest city, larger in the
    // smallest one); the goal sits between 15 and 55 and the leadership share above it.
    const float Goal = MarketCampaign::ShareGoal(S);
    TestTrue(TEXT("Share goal in range"), Goal >= 15.f && Goal <= 55.f && MarketCampaign::LeadShare(S) > Goal);
    const MarketCountry::FProfile* Pack = MarketCountry::Find(S.CountryId);
    if (TestTrue(TEXT("The pack has cities"), Pack && Pack->Cities.Num() > 1))
    {
        const MarketCountry::FCity* Big = &Pack->Cities[0];
        const MarketCountry::FCity* Small = &Pack->Cities[0];
        for (const MarketCountry::FCity& City : Pack->Cities)
        {
            if (City.PopulationK > Big->PopulationK) Big = &City;
            if (City.PopulationK < Small->PopulationK) Small = &City;
        }
        const float BigShare = MarketStoreDemand::NeutralShare(S, S.CountryId, Big->Id);
        const float SmallShare = MarketStoreDemand::NeutralShare(S, S.CountryId, Small->Id);
        TestTrue(TEXT("Biggest city: smaller share than the smallest"), BigShare < SmallShare);
        AddInfo(FString::Printf(TEXT("OLCUM: sube pay hedefi %%%.0f; notr pay en buyuk sehir %%%.1f, en kucuk %%%.1f"), Goal, BigShare * 100.f, SmallShare * 100.f));
    }
    AddInfo(FString::Printf(TEXT("OLCUM: ilk magaza pay hedefi %%%.1f, rakip fiyat duzeyi %.3f"), Target, Rival));
    MarketCountry::SetActive(MarketCountry::DefaultId(), 1);
    return true;
}

#endif

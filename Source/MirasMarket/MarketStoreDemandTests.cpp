#include "MarketStoreDemand.h"
#include "MarketBranches.h"
#include "MarketCountry.h"
#include "MarketDirector.h"
#include "MarketEconomy.h"
#include "MarketEras.h"
#include "MarketSimulation.h"
#include "MarketStart.h"
#include "MarketTuning.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// E2 (Docs/Kurgu/11_TEK_EKONOMI.md): one shopper formula for the family shop and the branches.
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

    // A family shop in the default country, founded and stocked the way the game does.
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

    FRun Play(int32 Days, bool bUnified, int32 Seed)
    {
        MarketTuning::Reset();
        if (bUnified) MarketTuning::Set(TEXT("UnifiedDemand"), 1.f);
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
        MarketTuning::Reset();
        return Run;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStoreDemandSameTest, "MirasMarket.StoreDemand.SameFormulaFamilyAndBranch", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStoreDemandSameTest::RunTest(const FString& Parameters)
{
    using namespace MarketStoreDemandTest;
    const TArray<FMarketProduct> Base = Catalog();
    TArray<FMarketProduct> Products;
    FMarketState S = NewShop(Base, Products, 11);
    const FString Home = MarketStart::HomeProvince(S);
    // One neighbourhood branch in the home province: it and the family shop share the room the same way.
    FMarketBranch B;
    B.Name = TEXT("test"); B.Province = Home; B.Format = TEXT("mahalle"); B.Stage = static_cast<uint8>(MarketBranches::EStage::Open);
    B.OpenedDay = 1; B.Maturity = 1.f; B.Satisfaction = 50.f; B.PriceIndex = 1.f;
    S.Branches.Add(B);
    S.Day = 20;

    MarketStoreDemand::FStoreDay Family;
    Family.Country = S.CountryId; Family.Province = Home; Family.Format = TEXT("mahalle"); Family.Self = MarketStoreDemand::FamilyShop;
    MarketStoreDemand::FStoreDay Branch = Family;
    Branch.Self = 0;
    const int32 Day = 20;
    const float A = MarketStoreDemand::ShoppersExact(S, Family, Day);
    const float C = MarketStoreDemand::ShoppersExact(S, Branch, Day);
    TestTrue(TEXT("People come"), A > 1.f);
    TestTrue(TEXT("Same conditions, same shoppers (5 %)"), FMath::Abs(A - C) <= 0.05f * FMath::Max(A, C));
    TestTrue(TEXT("The branch counts the family shop as a neighbour"), FMath::IsNearlyEqual(MarketStoreDemand::Cannibalization(S, Family), MarketStoreDemand::Cannibalization(S, Branch)));

    // The formula responds the way a store should.
    MarketStoreDemand::FStoreDay Dear = Family; Dear.PriceIndex = 1.1f;
    TestTrue(TEXT("Dearer shelves, fewer shoppers"), MarketStoreDemand::ShoppersExact(S, Dear, Day) < A);
    MarketStoreDemand::FStoreDay Empty = Family; Empty.Availability = 0.4f;
    TestTrue(TEXT("Empty shelves, fewer shoppers"), MarketStoreDemand::ShoppersExact(S, Empty, Day) < A);
    MarketStoreDemand::FStoreDay Young = Family; Young.Maturity = 0.f;
    TestTrue(TEXT("A new store has not grown into its share"), MarketStoreDemand::ShoppersExact(S, Young, Day) < A);
    S.Branches.Reset();
    TestTrue(TEXT("Alone in the province: more for the family shop"), MarketStoreDemand::ShoppersExact(S, Family, Day) >= A);

    // The family shop's own day: about one walking figure per real shopper on an ordinary day.
    const int32 Real = MarketStoreDemand::FamilyShoppers(S, Products, Day);
    AddInfo(FString::Printf(TEXT("OLCUM: aile dukkani gercek musteri (gun 20) %d, gorsel olcek %d"), Real, MarketSimulation::ShoppersPerDay));
    TestTrue(TEXT("Family shop has shoppers"), Real > 0);
    MarketCountry::SetActive(MarketCountry::DefaultId(), 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStoreDemandBeforeAfterTest, "MirasMarket.StoreDemand.FamilyBeforeAfter", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStoreDemandBeforeAfterTest::RunTest(const FString& Parameters)
{
    // E2 calibration (11_TEK_EKONOMI \u00a72 E2.8): the family shop's first 90 days, the old street model against the one
    // store formula. Target +-15 % revenue; this run reports the numbers (OLCUM lines) and only fails on a gross gap.
    using namespace MarketStoreDemandTest;
    const FRun Old = Play(90, false, 23);
    const FRun New = Play(90, true, 23);
    const double Ratio = Old.Revenue > 0 ? static_cast<double>(New.Revenue) / static_cast<double>(Old.Revenue) : 0.0;
    AddInfo(FString::Printf(TEXT("OLCUM: 90 gun ciro eski %lld yeni %lld oran %.3f; musteri eski %d yeni %d"),
        static_cast<long long>(Old.Revenue), static_cast<long long>(New.Revenue), Ratio, Old.Shoppers, New.Shoppers));
    TestTrue(TEXT("Old model sells"), Old.Revenue > 0);
    TestTrue(TEXT("New model sells"), New.Revenue > 0);
    TestEqual(TEXT("No audit gap (old)"), Old.Audit, 0);
    TestEqual(TEXT("No audit gap (new)"), New.Audit, 0);
    TestTrue(TEXT("Not a gross gap (0.5 .. 2)"), Ratio > 0.5 && Ratio < 2.0);
    MarketCountry::SetActive(MarketCountry::DefaultId(), 1);
    return true;
}

#endif

#include "MarketSourcing.h"
#include "MarketBranches.h"
#include "MarketEconomy.h"
#include "MarketPrices.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketSourcingTest
{
    FMarketProduct Make(const TCHAR* Id, const TCHAR* Category, int64 Cost, int64 Price)
    {
        FMarketProduct P;
        P.Id = Id; P.RealName = Id; P.FictionalName = Id; P.Category = Category; P.Brand = Id; P.Cost = Cost; P.BasePrice = Price;
        P.WidthMm = 70; P.DepthMm = 70; P.HeightMm = 200; P.CaseUnits = 12; P.PackageType = TEXT("kutu");
        return P;
    }

    FMarketState Start(const TArray<FMarketProduct>& Products)
    {
        FMarketState S; S.Initialize(Products);
        S.RivalSeed = 3; S.Day = 1; S.Cash = 10000000; S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
        return S;
    }

    void AddShops(FMarketState& S, int32 Count)
    {
        for (int32 I = 0; I < Count; ++I)
        {
            FMarketBranch& B = S.Branches.AddDefaulted_GetRef();
            B.Stage = static_cast<uint8>(MarketBranches::EStage::Open);
        }
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketSourcingTiersTest, "MirasMarket.Sourcing.TiersAndMinimums", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketSourcingTiersTest::RunTest(const FString& Parameters)
{
    using namespace MarketSourcingTest;
    using MarketSourcing::ELine;
    using MarketSourcing::ETier;
    const TArray<FMarketProduct> Products = { Make(TEXT("su"), TEXT("icecek"), 100, 150), Make(TEXT("sut"), TEXT("sut"), 170, 250),
        Make(TEXT("makarna"), TEXT("makarna"), 200, 300), Make(TEXT("deterjan"), TEXT("temizlik"), 900, 1300) };
    FMarketState S = Start(Products);

    // Lines and the argument code.
    TestTrue(TEXT("Drinks line"), MarketSourcing::LineOf(TEXT("icecek")) == ELine::Drinks);
    TestTrue(TEXT("Dairy line"), MarketSourcing::LineOf(TEXT("sut")) == ELine::Dairy);
    TestTrue(TEXT("Household line"), MarketSourcing::LineOf(TEXT("temizlik")) == ELine::Household);
    TestTrue(TEXT("Dry food line"), MarketSourcing::LineOf(TEXT("makarna")) == ELine::DryFood);
    ELine Line = ELine::Drinks; ETier Tier = ETier::Local;
    TestTrue(TEXT("Decode"), MarketSourcing::Decode(MarketSourcing::Encode(ELine::Household, ETier::National), Line, Tier) && Line == ELine::Household && Tier == ETier::National);
    TestFalse(TEXT("Bad argument"), MarketSourcing::Decode(47, Line, Tier));

    // A small shop starts local, at full price.
    TestTrue(TEXT("Local at start"), MarketSourcing::TierOf(S, ELine::Dairy) == ETier::Local);
    TestTrue(TEXT("No discount at start"), FMath::IsNearlyEqual(MarketSourcing::CostFactor(S, TEXT("sut")), 1.f, 0.001f));

    // Requirements: shops, a depot, central buying.
    FString Message;
    TestFalse(TEXT("Regional needs 3 shops"), MarketSourcing::Set(S, ELine::Dairy, ETier::Regional, Message));
    AddShops(S, 2);
    TestTrue(TEXT("Regional with 3 shops"), MarketSourcing::Set(S, ELine::Dairy, ETier::Regional, Message));
    TestTrue(TEXT("2.5 % cheaper"), FMath::IsNearlyEqual(MarketSourcing::CostFactor(S, TEXT("sut")), 0.975f, 0.001f));
    TestTrue(TEXT("Other lines untouched"), FMath::IsNearlyEqual(MarketSourcing::CostFactor(S, TEXT("icecek")), 1.f, 0.001f));
    TestTrue(TEXT("Cold chain"), MarketSourcing::DairySpoilFactor(S) < 1.f);
    AddShops(S, 10);
    FString Why;
    TestFalse(TEXT("National needs a depot"), MarketSourcing::CanSet(S, ELine::Drinks, ETier::National, Why));
    S.Company.DepotSites.AddDefaulted();
    TestTrue(TEXT("National with a depot"), MarketSourcing::CanSet(S, ELine::Drinks, ETier::National, Why));
    AddShops(S, 20);
    TestFalse(TEXT("Producer needs central buying"), MarketSourcing::CanSet(S, ELine::Drinks, ETier::Producer, Why));
    S.Company.bCentralBuying = true;
    TestTrue(TEXT("Producer"), MarketSourcing::Set(S, ELine::Drinks, ETier::Producer, Message));
    TestTrue(TEXT("Stepping down is always possible"), MarketSourcing::CanSet(S, ELine::Drinks, ETier::Local, Why));
    TestFalse(TEXT("Same tier refused"), MarketSourcing::CanSet(S, ELine::Drinks, ETier::Producer, Why));

    // The minimum: a line far under it for two months is dropped one tier; a line above it keeps its tier.
    for (int32 Month = 0; Month < 2; ++Month)
    {
        for (int32 D = 0; D < 30; ++D)
        {
            ++S.Day; S.DayNews.Reset();
            MarketSourcing::RecordPurchase(S, TEXT("sut"), FMath::RoundToInt64(MarketSourcing::TierMinimum(ETier::Regional) * MarketPrices::ListLevel(S.Day) / 25.0));
            MarketSourcing::CloseDay(S, Products);
        }
    }
    TestTrue(TEXT("Dairy kept its distributor"), MarketSourcing::TierOf(S, ELine::Dairy) == ETier::Regional);
    TestTrue(TEXT("Drinks dropped one tier"), MarketSourcing::TierOf(S, ELine::Drinks) == ETier::National);
    TestTrue(TEXT("Describe names the line"), MarketSourcing::Describe(S, ELine::Drinks).Contains(MarketSourcing::LineName(ELine::Drinks)));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketSourcingVolumeTest, "MirasMarket.Sourcing.BuyingPower", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketSourcingVolumeTest::RunTest(const FString& Parameters)
{
    using namespace MarketSourcingTest;
    const TArray<FMarketProduct> Products = { Make(TEXT("su"), TEXT("icecek"), 100, 150) };
    FMarketState S = Start(Products);
    const double Base = MarketSourcing::BaseVolume2011 * MarketPrices::ListLevel(S.Day);
    FMarketSupplierAccount& A = S.SupplierAccounts.AddDefaulted_GetRef();
    A.Volume30 = FMath::RoundToInt64(Base * 0.5);
    TestEqual(TEXT("A small shop has no buying power"), MarketSourcing::VolumeDiscount(S), 0.f);
    A.Volume30 = FMath::RoundToInt64(Base * 4.0);
    TestTrue(TEXT("Two doublings: 6 %"), FMath::IsNearlyEqual(MarketSourcing::VolumeDiscount(S), 0.06f, 0.002f));
    A.Volume30 = FMath::RoundToInt64(Base * 100000.0);
    TestTrue(TEXT("Capped"), FMath::IsNearlyEqual(MarketSourcing::VolumeDiscount(S), MarketSourcing::MaxVolumeDiscount, 0.0001f));
    TestTrue(TEXT("Costs follow"), FMath::IsNearlyEqual(MarketSourcing::CostFactor(S, TEXT("icecek")), 1.f - MarketSourcing::MaxVolumeDiscount, 0.001f));
    return true;
}

#endif

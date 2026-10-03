#include "MarketProductDemand.h"
#include "MarketDemand.h"
#include "MarketCustomers.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// E3b (Mustafa 03.10.2026, "ortak urun istegi"): the family shop's shoppers and the branches use one product demand.
namespace MarketProductDemandTest
{
    FMarketProduct Make(const TCHAR* Id, const TCHAR* Category, int64 Cost, int64 Price, float Kvi)
    {
        FMarketProduct P;
        P.Id = Id; P.RealName = Id; P.Category = Category; P.Cost = Cost; P.BasePrice = Price; P.CaseUnits = 12; P.bActive = true; P.Kvi = Kvi;
        return P;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketProductDemandTest, "MirasMarket.ProductDemand.OneWishForAll", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketProductDemandTest::RunTest(const FString& Parameters)
{
    using namespace MarketProductDemandTest;
    const TArray<FMarketProduct> Products = { Make(TEXT("milk"), TEXT("s\u00fct"), 170, 250, 1.f), Make(TEXT("cola"), TEXT("i\u00e7ecek"), 180, 275, 0.f), Make(TEXT("soap"), TEXT("temizlik"), 300, 450, 0.f) };
    FMarketState S; S.Initialize(Products); S.RivalSeed = 12; S.Day = 40;
    S.ApplyShelfCapacities({ 24, 24, 24 });

    // The wishes of a segment mix are the segments' wishes weighted by the mix, and sum to 1.
    const int32 Mix[6] = { 15, 32, 25, 10, 10, 8 };
    const TArray<float> Wishes = MarketProductDemand::MixWishes(S, Products, Mix, S.Day);
    float Sum = 0.f, Raw = 0.f;
    for (const float W : Wishes) Sum += W;
    TestTrue(TEXT("Wishes sum to 1"), FMath::IsNearlyEqual(Sum, 1.f, 0.001f));
    for (int32 Seg = 0; Seg < 6; ++Seg) for (const FMarketProduct& P : Products) Raw += Mix[Seg] / 100.f * MarketProductDemand::SegmentWish(S, P, static_cast<MarketCustomers::ESegment>(Seg), S.Day);
    float MilkRaw = 0.f;
    for (int32 Seg = 0; Seg < 6; ++Seg) MilkRaw += Mix[Seg] / 100.f * MarketProductDemand::SegmentWish(S, Products[0], static_cast<MarketCustomers::ESegment>(Seg), S.Day);
    TestTrue(TEXT("Milk's wish is its part of the mix"), Raw > 0.f && FMath::IsNearlyEqual(Wishes[0], MilkRaw / Raw, 0.001f));

    // Acceptance: 1 at parity, more for a cheaper shelf, less for a dearer one; a known price (KVI) reacts more.
    TestTrue(TEXT("Parity is 1"), FMath::IsNearlyEqual(MarketProductDemand::AcceptanceFactor(1.0, MarketDemand::NeutralShare, 0.0, Products[1]), 1.0, 0.0001));
    TestTrue(TEXT("Cheaper sells more"), MarketProductDemand::AcceptanceFactor(0.9, MarketDemand::NeutralShare, 0.0, Products[1]) > 1.0);
    TestTrue(TEXT("Dearer sells less"), MarketProductDemand::AcceptanceFactor(1.1, MarketDemand::NeutralShare, 0.0, Products[1]) < 1.0);
    TestTrue(TEXT("A known price reacts more"), MarketProductDemand::AcceptanceFactor(1.1, MarketDemand::NeutralShare, 0.0, Products[0]) < MarketProductDemand::AcceptanceFactor(1.1, MarketDemand::NeutralShare, 0.0, Products[1]));
    TestTrue(TEXT("A tolerant mix accepts more"), MarketProductDemand::Acceptance(1.1, 25.f, 0.05, Products[1]) > MarketProductDemand::Acceptance(1.1, 25.f, 0.0, Products[1]));

    // The family shop's shoppers decide one by one with the same acceptance: over many shoppers the share who buy
    // is the branches' expected value.
    const float Rival = 0.97f;
    for (const int64 Price : { int64(250), int64(275), int64(300) })
    {
        S.Stock[0].Price = Price;
        const int32 N = 2000;
        int32 Bought = 0;
        for (int32 K = 0; K < N; ++K)
        {
            const MarketDemand::FVisit Visit = MarketDemand::Decide(S, Products, 0, 24, Rival, 1, (K + 0.5f) / N, 25.f, 0.0, Price);
            Bought += Visit.Result == MarketDemand::EVisit::Buy ? 1 : 0;
        }
        const double Expected = MarketProductDemand::Acceptance(MarketDemand::PriceRatio(Price, MarketDemand::RivalPrice(Products[0], Rival)), 25.f, 0.0, Products[0]);
        TestTrue(*FString::Printf(TEXT("Price %lld: shoppers one by one = the expected share"), Price), FMath::Abs(static_cast<double>(Bought) / N - Expected) < 0.01);
        AddInfo(FString::Printf(TEXT("OLCUM: fiyat %lld, rakip oran %.3f, tek tek alan %.3f, beklenen %.3f"), Price,
            MarketDemand::PriceRatio(Price, MarketDemand::RivalPrice(Products[0], Rival)), static_cast<double>(Bought) / N, Expected));
    }

    // The family shop's lists follow the shared wishes: the most wished product of a segment lands on more lists.
    const MarketCustomers::ESegment Family = static_cast<MarketCustomers::ESegment>(1);
    int32 Best = 0, Worst = 0;
    for (int32 I = 1; I < Products.Num(); ++I)
    {
        if (MarketProductDemand::SegmentWish(S, Products[I], Family, S.Day) > MarketProductDemand::SegmentWish(S, Products[Best], Family, S.Day)) Best = I;
        if (MarketProductDemand::SegmentWish(S, Products[I], Family, S.Day) < MarketProductDemand::SegmentWish(S, Products[Worst], Family, S.Day)) Worst = I;
    }
    if (Best != Worst && MarketProductDemand::SegmentWish(S, Products[Best], Family, S.Day) > 1.2f * MarketProductDemand::SegmentWish(S, Products[Worst], Family, S.Day))
    {
        FRandomStream Random(77);
        int32 BestCount = 0, WorstCount = 0;
        for (int32 K = 0; K < 3000; ++K)
        {
            const TArray<int32> List = MarketCustomers::BuildList(S, Products, Family, Random);
            BestCount += List.Contains(Best) ? 1 : 0;
            WorstCount += List.Contains(Worst) ? 1 : 0;
        }
        TestTrue(TEXT("Lists follow the shared wish"), BestCount > WorstCount);
    }
    return true;
}

#endif

#include "MarketPromotions.h"
#include "MarketSuppliers.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketPromotionsTest, "MirasMarket.Promotions.Effects", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketPromotionsTest::RunTest(const FString& Parameters)
{
    using namespace MarketPromotions;
    FMarketProduct MilkA; MilkA.Id = TEXT("milk_a"); MilkA.Category = TEXT("s\u00fct"); MilkA.Cost = 170; MilkA.BasePrice = 250;
    FMarketProduct MilkB; MilkB.Id = TEXT("milk_b"); MilkB.Category = TEXT("S\u00fct"); MilkB.Cost = 160; MilkB.BasePrice = 240;
    FMarketProduct Cola; Cola.Id = TEXT("cola"); Cola.Category = TEXT("i\u00e7ecek"); Cola.Cost = 180; Cola.BasePrice = 275;
    const TArray<FMarketProduct> Products = { MilkA, MilkB, Cola };
    FMarketState S; S.Initialize(Products); S.Cash = 100000;
    S.ApplyShelfCapacities({ 12, 12, 12 });
    FString Message;

    // Aisle discount: both milks, not the cola.
    TestTrue(TEXT("Start 20 % on the milk aisle"), Start(S, Products, EKind::AisleDiscount, 0, 20, Message));
    TestEqual(TEXT("Milk A pays 20 % less"), UnitPrice(S, Products, 0, 1), int64(200));
    TestEqual(TEXT("Milk B too (same aisle, any case)"), UnitPrice(S, Products, 1, 1), int64(192));
    TestEqual(TEXT("Cola unchanged"), UnitPrice(S, Products, 2, 1), int64(275));
    TestTrue(TEXT("Discounted aisle is more visible"), Interest(S, Products, 0) > 1.3f && FMath::IsNearlyEqual(Interest(S, Products, 2), 1.f));
    TestFalse(TEXT("Badge"), Badge(S, Products, 0).IsEmpty());
    TestFalse(TEXT("No double discount"), Start(S, Products, EKind::AisleDiscount, 1, 10, Message));

    // Three for two.
    TestTrue(TEXT("3 al 2 \u00f6de on cola"), Start(S, Products, EKind::MultiBuy, 2, 0, Message));
    TestEqual(TEXT("Two become three"), AdjustQuantity(S, 2, 2), 3);
    TestEqual(TEXT("One stays one"), AdjustQuantity(S, 2, 1), 1);
    TestEqual(TEXT("Three for the price of two"), UnitPrice(S, Products, 2, 3), int64(183));

    // Flyer: costs money at the day close, brings shoppers.
    TestTrue(TEXT("Flyer"), Start(S, Products, EKind::Flyer, INDEX_NONE, 0, Message));
    TestEqual(TEXT("Marketing to pay"), S.Marketing, FlyerCost);
    TestTrue(TEXT("More shoppers"), TrafficFactor(S) > 1.1f);
    TestTrue(TEXT("Flyer shows promoted products"), Interest(S, Products, 0) > 1.4f * 1.2f);
    TestFalse(TEXT("At most three at once"), Start(S, Products, EKind::MultiBuy, 0, 0, Message));
    // Endcap is outside the limit, one at a time.
    TestTrue(TEXT("Endcap"), Start(S, Products, EKind::Endcap, 0, 0, Message));
    TestTrue(TEXT("Second endcap replaces the first"), Start(S, Products, EKind::Endcap, 1, 0, Message));
    int32 Endcaps = 0;
    for (const FMarketPromotion* P : Active(S)) if (P->Kind == static_cast<uint8>(EKind::Endcap)) ++Endcaps;
    TestEqual(TEXT("One gondola head"), Endcaps, 1);

    // Day closes: the flyer is paid with the operating costs; finished promotions report.
    S.Stock[0].Today.Sold = 10;
    S.CloseDay();
    TestTrue(TEXT("Flyer paid at the close"), S.LastOperatingCost >= 2200 + FlyerCost && S.Marketing == 0);
    S.DayNews.Reset();
    CloseDay(S, Products);
    for (int32 D = 0; D < 3; ++D) { S.DayNews.Reset(); S.CloseDay(); CloseDay(S, Products); }
    bool bReported = false;
    for (const FString& Line : S.DayNews) if (Line.StartsWith(TEXT("Kampanya bitti"))) bReported = true;
    TestTrue(TEXT("Finished promotions report"), bReported);
    TestEqual(TEXT("Discount over"), UnitPrice(S, Products, 0, 1), int64(250));
    TestFalse(TEXT("Stop needs a running one"), Stop(S, 99, Message));

    // The wholesaler's funded offer comes when Selim trusts the shop.
    FMarketState O; O.Initialize(Products); O.Cash = 100000; O.RivalSeed = 3;
    O.ApplyShelfCapacities({ 12, 12, 12 });
    MarketSuppliers::Account(O, MarketSuppliers::ESupplier::TrakyaGida).Trust = 90;
    for (int32 D = 0; D < 80 && O.Offer.Product == INDEX_NONE; ++D) { O.DayNews.Reset(); O.CloseDay(); CloseDay(O, Products); }
    TestTrue(TEXT("An offer came"), O.Offer.Product != INDEX_NONE);
    const int32 Deal = O.Offer.Product;
    TestTrue(TEXT("Accept"), AcceptOffer(O, Products, Message));
    TestTrue(TEXT("Cheaper to buy"), MarketSuppliers::UnitCost(O, Products[Deal], Deal) < MarketSuppliers::UnitCost(O, Products[Deal]));
    TestTrue(TEXT("10 % off on the shelf"), UnitPrice(O, Products, Deal, 1) < O.Stock[Deal].Price);
    TestFalse(TEXT("Offer used"), AcceptOffer(O, Products, Message));
    return true;
}

#endif

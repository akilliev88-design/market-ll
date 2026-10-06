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

    // M36: no flyer of the shop's own (the company's ads); a third promotion fills the slots.
    TestFalse(TEXT("No shop flyer"), Start(S, Products, EKind::Flyer, INDEX_NONE, 0, Message));
    TestTrue(TEXT("3 al 2 \u00f6de on milk B"), Start(S, Products, EKind::MultiBuy, 1, 0, Message));
    TestFalse(TEXT("At most three at once"), Start(S, Products, EKind::MultiBuy, 0, 0, Message));
    // Endcap is outside the limit, one at a time.
    TestTrue(TEXT("Endcap"), Start(S, Products, EKind::Endcap, 0, 0, Message));
    TestTrue(TEXT("Second endcap replaces the first"), Start(S, Products, EKind::Endcap, 1, 0, Message));
    int32 Endcaps = 0;
    for (const FMarketPromotion* P : Active(S)) if (P->Kind == static_cast<uint8>(EKind::Endcap)) ++Endcaps;
    TestEqual(TEXT("One gondola head"), Endcaps, 1);

    // Day closes: finished promotions report.
    S.Stock[0].Today.Sold = 10;
    S.CloseDay();
    S.DayNews.Reset();
    CloseDay(S, Products);
    bool bReported = false;
    for (int32 D = 0; D < 3; ++D)
    {
        S.DayNews.Reset(); S.CloseDay(); CloseDay(S, Products);
        for (const FString& Line : S.DayNews) bReported |= Line.StartsWith(TEXT("Kampanya bitti"));
    }
    TestTrue(TEXT("Finished promotions report"), bReported);
    TestEqual(TEXT("Discount over"), UnitPrice(S, Products, 0, 1), int64(250));
    TestFalse(TEXT("Stop needs a running one"), Stop(S, 99, Message));

    // The wholesaler's funded offer comes when the wholesaler trusts the shop.
    FMarketState O; O.Initialize(Products); O.Cash = 100000; O.RivalSeed = 3;
    O.ApplyShelfCapacities({ 12, 12, 12 });
    MarketSuppliers::Account(O, MarketSuppliers::ESupplier::Regular).Trust = 90;
    for (int32 D = 0; D < 80 && O.Offer.Product == INDEX_NONE; ++D) { O.DayNews.Reset(); O.CloseDay(); CloseDay(O, Products); }
    TestTrue(TEXT("An offer came"), O.Offer.Product != INDEX_NONE);
    const int32 Deal = O.Offer.Product;
    TestTrue(TEXT("Accept"), AcceptOffer(O, Products, Message));
    TestTrue(TEXT("Cheaper to buy"), MarketSuppliers::UnitCost(O, Products[Deal], Deal) < MarketSuppliers::UnitCost(O, Products[Deal]));
    TestTrue(TEXT("10 % off on the shelf"), UnitPrice(O, Products, Deal, 1) < O.Stock[Deal].Price);
    TestFalse(TEXT("Offer used"), AcceptOffer(O, Products, Message));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketPromotionsScopedTest, "MirasMarket.Promotions.Scoped", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketPromotionsScopedTest::RunTest(const FString& Parameters)
{
    // G-078 (karar J07): one campaign on a single product, a brand, a subcategory, an aisle or the whole store.
    using namespace MarketPromotions;
    FMarketProduct MilkA; MilkA.Id = TEXT("milk_a"); MilkA.Category = TEXT("sut"); MilkA.Subcategory = TEXT("sut"); MilkA.Brand = TEXT("Pinar"); MilkA.Cost = 170; MilkA.BasePrice = 250; MilkA.Elasticity = 1.5f;
    FMarketProduct Labne; Labne.Id = TEXT("labne"); Labne.Category = TEXT("sut"); Labne.Subcategory = TEXT("peynir"); Labne.Brand = TEXT("Pinar"); Labne.Cost = 210; Labne.BasePrice = 310;
    FMarketProduct MilkB; MilkB.Id = TEXT("milk_b"); MilkB.Category = TEXT("sut"); MilkB.Subcategory = TEXT("sut"); MilkB.Brand = TEXT("Sutas"); MilkB.Cost = 160; MilkB.BasePrice = 240;
    FMarketProduct Cola; Cola.Id = TEXT("cola"); Cola.Category = TEXT("icecek"); Cola.Subcategory = TEXT("gazli"); Cola.Brand = TEXT("Coca-Cola"); Cola.Cost = 180; Cola.BasePrice = 275; Cola.Elasticity = 4.f;
    const TArray<FMarketProduct> Products = { MilkA, Labne, MilkB, Cola };
    FMarketState S; S.Initialize(Products); S.Cash = 100000;
    S.ApplyShelfCapacities({ 12, 12, 12, 12 });
    FString Message;

    TestEqual(TEXT("Single product reaches one"), ScopeSize(Products, 0, EScope::Product), 1);
    TestEqual(TEXT("Brand reaches both Pinar"), ScopeSize(Products, 0, EScope::Brand), 2);
    TestEqual(TEXT("Subcategory reaches both milks"), ScopeSize(Products, 0, EScope::Subcategory), 2);
    TestEqual(TEXT("Aisle reaches three"), ScopeSize(Products, 0, EScope::Category), 3);
    TestEqual(TEXT("Store reaches all"), ScopeSize(Products, 0, EScope::Store), 4);

    // 15 % on one product only.
    TestTrue(TEXT("Single product 15 %"), StartScoped(S, Products, PackArg(0, EScope::Product, EMechanic::Percent, 15, 7), Message));
    TestEqual(TEXT("Only that product is cheaper"), UnitPrice(S, Products, 0, 1), int64(213));
    TestEqual(TEXT("Same brand, other product unchanged"), UnitPrice(S, Products, 1, 1), int64(310));
    TestEqual(TEXT("Seven days"), S.Promotions.Last().EndDay, S.Day + 6);
    TestFalse(TEXT("Not twice on the same product"), StartScoped(S, Products, PackArg(0, EScope::Product, EMechanic::Percent, 10, 3), Message));

    // 2 al 1 \u00f6de on the cola: one becomes two, two for the price of one.
    TestTrue(TEXT("2 for 1 on cola"), StartScoped(S, Products, PackArg(3, EScope::Product, EMechanic::TwoForOne, 0, 3), Message));
    TestEqual(TEXT("One becomes two"), AdjustQuantity(S, 3, 1), 2);
    TestEqual(TEXT("Half price each"), UnitPrice(S, Products, 3, 2), int64(138));
    TestTrue(TEXT("A price-sensitive product reacts more"), Interest(S, Products, 3) > Interest(S, Products, 0));

    // A brand-wide second-half deal.
    TestTrue(TEXT("Brand: second unit half"), StartScoped(S, Products, PackArg(1, EScope::Brand, EMechanic::SecondHalf, 0, 7), Message));
    TestEqual(TEXT("Labne pair: 25 % off each"), UnitPrice(S, Products, 1, 2), int64(233));
    TestEqual(TEXT("Other brand unchanged"), UnitPrice(S, Products, 2, 2), int64(240));
    TestFalse(TEXT("Three running at most"), StartScoped(S, Products, PackArg(2, EScope::Store, EMechanic::Percent, 10, 3), Message));
    TestTrue(TEXT("Describe"), Describe(S.Promotions.Last(), Products).Contains(TEXT("Pinar")));
    // Shopper memory: a product on a deal all month excites less; a pantry full after a deal keeps shoppers away.
    const float Fresh = Interest(S, Products, 3);
    S.Stock[3].PromoHeat = 25.f;
    TestTrue(TEXT("Deal fatigue"), Interest(S, Products, 3) < Fresh);
    S.Stock[3].PromoHeat = 0.f;
    S.Stock[3].Pantry = 10.f;
    TestTrue(TEXT("After-deal dip"), Interest(S, Products, 3) < Fresh);
    return true;
}

#endif

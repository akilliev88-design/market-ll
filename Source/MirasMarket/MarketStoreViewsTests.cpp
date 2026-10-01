#include "MarketStoreViews.h"
#include "MarketBranches.h"
#include "MarketEconomy.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketStoreViewsTest
{
    FMarketProduct Make(const TCHAR* Id, const TCHAR* Category, int64 Cost, int64 Price, int32 W, int32 D, int32 H, int32 Case = 12)
    {
        FMarketProduct P;
        P.Id = Id; P.RealName = Id; P.Category = Category; P.Brand = Id; P.Cost = Cost; P.BasePrice = Price;
        P.WidthMm = W; P.DepthMm = D; P.HeightMm = H; P.CaseUnits = Case; P.PackageType = TEXT("kutu");
        return P;
    }

    TArray<FMarketProduct> Catalog()
    {
        return {
            Make(TEXT("sut"), TEXT("s\u00fct"), 170, 250, 70, 70, 200),
            Make(TEXT("yogurt"), TEXT("yo\u011furt"), 300, 450, 120, 120, 90),
            Make(TEXT("cola"), TEXT("i\u00e7ecek"), 180, 275, 80, 80, 260),
            Make(TEXT("cay"), TEXT("\u00e7ay-kahve"), 480, 675, 120, 60, 180),
            Make(TEXT("biskuvi"), TEXT("bisk\u00fcvi-\u00e7ikolata"), 100, 175, 150, 50, 40),
            Make(TEXT("makarna"), TEXT("makarna-bakliyat"), 210, 325, 190, 40, 70),
            Make(TEXT("deterjan"), TEXT("temizlik"), 1400, 1990, 250, 180, 300, 4),
        };
    }

    MarketStoreViews::FView View(const TCHAR* Id, const TCHAR* Format, float Scale, int32 Checkouts = -1)
    {
        MarketStoreViews::FView V;
        V.Id = Id;
        V.Format = Format;
        V.Name = Id;
        V.Measures = MarketStoreAssign::Nominal(Format);
        V.Measures.SalesAreaM2 *= Scale;
        V.Measures.ShelfFrontM *= Scale;
        if (Checkouts >= 0) V.Measures.Checkouts = Checkouts;
        return V;
    }

    FMarketState Start(const TArray<FMarketProduct>& Products)
    {
        FMarketState S; S.Initialize(Products); S.RivalSeed = 12; S.Cash = 50000000;
        S.InheritedDebt = 0; S.ProfitableDays = 5; S.MarketShare = 40.f;
        S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
        return S;
    }

    void Close(FMarketState& S, const TArray<FMarketProduct>& Products)
    {
        S.DayNews.Reset();
        S.CloseDay();
        MarketBranches::CloseDay(S, Products);
    }

    // Opens a mahalle branch in the home province with the given catalog; INDEX_NONE when it could not open.
    int32 OpenWith(FMarketState& S, const TArray<FMarketProduct>& Products, const TArray<MarketStoreViews::FView>& Views, const TCHAR* Format = TEXT("mahalle"))
    {
        MarketStoreViews::SetCatalog(Views);
        FString Message;
        if (!MarketBranches::Open(S, Products, TEXT("tr"), TEXT("kirklareli"), Format, Message)) return INDEX_NONE;
        return S.Branches.Num() - 1;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStoreViewsAssignTest, "MirasMarket.StoreViews.AssignAndCosts", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStoreViewsAssignTest::RunTest(const FString& Parameters)
{
    using namespace MarketStoreViewsTest;
    const TArray<FMarketProduct> Products = Catalog();

    // No catalog and a nominal view give the same rent and the same shelves (every factor 1.0).
    FMarketState Plain = Start(Products);
    const int32 PlainIndex = OpenWith(Plain, Products, {});
    FMarketState Nominal = Start(Products);
    const int32 NominalIndex = OpenWith(Nominal, Products, { View(TEXT("mahalle_90"), TEXT("mahalle"), 1.f) });
    TestTrue(TEXT("Both opened"), PlainIndex != INDEX_NONE && NominalIndex != INDEX_NONE);
    if (PlainIndex == INDEX_NONE || NominalIndex == INDEX_NONE) { MarketStoreViews::ResetCatalog(); return false; }
    TestTrue(TEXT("No catalog: no view"), Plain.Branches[PlainIndex].StoreView.IsEmpty());
    TestEqual(TEXT("View signed"), Nominal.Branches[NominalIndex].StoreView, FString(TEXT("mahalle_90")));
    TestEqual(TEXT("Nominal store: same rent"), Nominal.Branches[NominalIndex].Rent, Plain.Branches[PlainIndex].Rent);
    TestEqual(TEXT("Nominal store: same money spent"), Nominal.Cash, Plain.Cash);
    TestEqual(TEXT("Saved for the site"), Nominal.StoreViews.FindRef(MarketStoreAssign::SiteKey(TEXT("tr"), TEXT("kirklareli"), TEXT("mahalle"))), FString(TEXT("mahalle_90")));

    // A smaller store: cheaper rent and fit-out, fewer units on the shelves.
    FMarketState Small = Start(Products);
    const int32 SmallIndex = OpenWith(Small, Products, { View(TEXT("mahalle_91"), TEXT("mahalle"), 0.6f) });
    TestTrue(TEXT("Small opened"), SmallIndex != INDEX_NONE);
    if (SmallIndex == INDEX_NONE) { MarketStoreViews::ResetCatalog(); return false; }
    TestTrue(TEXT("Smaller store, lower rent"), Small.Branches[SmallIndex].Rent < Nominal.Branches[NominalIndex].Rent);
    TestTrue(TEXT("Smaller store, cheaper to open"), Small.OtherCosts < Nominal.OtherCosts);
    int32 SmallUnits = 0, NominalUnits = 0;
    for (const FMarketBranchItem& Item : Small.Branches[SmallIndex].Items) SmallUnits += Item.Capacity;
    for (const FMarketBranchItem& Item : Nominal.Branches[NominalIndex].Items) NominalUnits += Item.Capacity;
    TestTrue(TEXT("Shorter shelves hold less"), SmallUnits < NominalUnits);

    // The cost preview matches what opening takes (deposit + fit-out).
    FMarketState Preview = Start(Products);
    MarketStoreViews::SetCatalog({ View(TEXT("mahalle_91"), TEXT("mahalle"), 0.6f) });
    const int64 Cost = MarketBranches::OpeningCost(Preview, Products, TEXT("tr"), TEXT("kirklareli"), TEXT("mahalle"));
    TestTrue(TEXT("Preview does not write the save"), Preview.StoreViews.Num() == 0);
    TestTrue(TEXT("Preview covers deposit and fit-out"), Cost >= 2 * Small.Branches[SmallIndex].Rent + Small.OtherCosts);

    // The saved choice stays even when the catalog grows (a new view does not move an existing site).
    MarketStoreViews::SetCatalog({ View(TEXT("mahalle_91"), TEXT("mahalle"), 0.6f), View(TEXT("mahalle_92"), TEXT("mahalle"), 1.2f), View(TEXT("mahalle_93"), TEXT("mahalle"), 1.1f) });
    TestEqual(TEXT("Saved view kept"), MarketStoreViews::ViewFor(Small, TEXT("tr"), TEXT("kirklareli"), TEXT("mahalle")), FString(TEXT("mahalle_91")));
    TestFalse(TEXT("Describe"), MarketStoreViews::Describe(Small.Branches[SmallIndex]).IsEmpty());

    MarketStoreViews::ResetCatalog();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStoreViewsDayTest, "MirasMarket.StoreViews.TillsAndMigrate", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStoreViewsDayTest::RunTest(const FString& Parameters)
{
    using namespace MarketStoreViewsTest;
    const TArray<FMarketProduct> Products = Catalog();

    // The same supermarket with one till instead of the nominal six: shoppers are lost in the queue.
    FMarketState Roomy = Start(Products);
    const int32 RoomyIndex = OpenWith(Roomy, Products, { View(TEXT("buyuk_90"), TEXT("buyuk"), 1.f) }, TEXT("buyuk"));
    FMarketState Tight = Start(Products);
    const int32 TightIndex = OpenWith(Tight, Products, { View(TEXT("buyuk_91"), TEXT("buyuk"), 1.f, 1) }, TEXT("buyuk"));
    TestTrue(TEXT("Both opened"), RoomyIndex != INDEX_NONE && TightIndex != INDEX_NONE);
    if (RoomyIndex == INDEX_NONE || TightIndex == INDEX_NONE) { MarketStoreViews::ResetCatalog(); return false; }
    for (int32 D = 0; D < 45; ++D)
    {
        MarketStoreViews::SetCatalog({ View(TEXT("buyuk_90"), TEXT("buyuk"), 1.f) });
        Close(Roomy, Products);
        MarketStoreViews::SetCatalog({ View(TEXT("buyuk_91"), TEXT("buyuk"), 1.f, 1) });
        Close(Tight, Products);
    }
    const FMarketBranch& R = Roomy.Branches[RoomyIndex];
    const FMarketBranch& T = Tight.Branches[TightIndex];
    TestEqual(TEXT("Both open"), T.Stage, static_cast<uint8>(MarketBranches::EStage::Open));
    TestTrue(TEXT("Fewer tills lose shoppers"), T.LastQueueLost > R.LastQueueLost);
    TestTrue(TEXT("Fewer tills, fewer shoppers served"), T.LastShoppers < R.LastShoppers);
    TestTrue(TEXT("Fewer tills, fewer people"), T.Workers < R.Workers);

    MarketStoreViews::ResetCatalog();
    return true;
}

#endif

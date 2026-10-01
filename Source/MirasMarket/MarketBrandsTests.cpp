#include "MarketBrands.h"
#include "MarketEconomy.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketBrandsTest
{
    FMarketProduct Make(const TCHAR* Id, const TCHAR* Brand, const TCHAR* Category, int64 Cost, int64 Price)
    {
        FMarketProduct P;
        P.Id = Id; P.RealName = Id; P.FictionalName = Id; P.Category = Category; P.Brand = Brand; P.Cost = Cost; P.BasePrice = Price;
        P.WidthMm = 70; P.DepthMm = 70; P.HeightMm = 200; P.CaseUnits = 12; P.PackageType = TEXT("kutu");
        return P;
    }

    TArray<FMarketProduct> Catalog()
    {
        return {
            Make(TEXT("lider_sut"), TEXT("Lider"), TEXT("s\u00fct"), 170, 250),
            Make(TEXT("lider_yogurt"), TEXT("Lider"), TEXT("s\u00fct"), 300, 450),
            Make(TEXT("orta_sut"), TEXT("Orta"), TEXT("s\u00fct"), 160, 240),
        };
    }

    void Table()
    {
        MarketBrands::FBrandInfo A; A.Real = TEXT("Lider"); A.Fictional = TEXT("Lidur"); A.Category = TEXT("s\u00fct"); A.Power = 1.f;
        MarketBrands::FBrandInfo B; B.Real = TEXT("Orta"); B.Fictional = TEXT("Ortu"); B.Category = TEXT("s\u00fct"); B.Power = 0.45f;
        MarketBrands::SetTable({ A, B });
    }

    // C7: only a stocked shelf counts (full from half full).
    void Fill(FMarketState& S) { for (FMarketStock& Item : S.Stock) Item.Shelf = Item.Capacity; }

    FMarketState Start(const TArray<FMarketProduct>& Products)
    {
        FMarketState S; S.Initialize(Products);
        S.RivalSeed = 5; S.Day = 1; S.Cash = 10000000; S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
        S.Stock[0].Capacity = 10; S.Stock[1].Capacity = 0; S.Stock[2].Capacity = 30;
        Fill(S);
        return S;
    }

    void Month(FMarketState& S, const TArray<FMarketProduct>& Products)
    {
        for (int32 D = 0; D < MarketBrands::MonthDays; ++D) { ++S.Day; S.DayNews.Reset(); MarketBrands::CloseDay(S, Products); }
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBrandsSharesTest, "MirasMarket.Brands.SharesAndOffers", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketBrandsSharesTest::RunTest(const FString& Parameters)
{
    using namespace MarketBrandsTest;
    Table();
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S = Start(Products);
    MarketBrands::Ensure(S, Products);
    const FString Milk = TEXT("s\u00fct");
    TestTrue(TEXT("The leader holds more of the country"), MarketBrands::NationalShare(S, TEXT("Lider"), Milk) > MarketBrands::NationalShare(S, TEXT("Orta"), Milk));
    TestTrue(TEXT("Shares add to one"), FMath::IsNearlyEqual(MarketBrands::NationalShare(S, TEXT("Lider"), Milk) + MarketBrands::NationalShare(S, TEXT("Orta"), Milk), 1.f, 0.01f));
    TestTrue(TEXT("Shelf share from capacity"), FMath::IsNearlyEqual(MarketBrands::ShelfShare(S, Products, TEXT("Lider"), Milk), 0.25f, 0.001f));
    TestEqual(TEXT("Fictional name"), MarketBrands::NameOf(S, TEXT("Lider")), FString(TEXT("Lidur")));

    // The short brand comes with an offer within a few months.
    for (int32 M = 0; M < 6 && S.Brands.Offers.Num() == 0; ++M) Month(S, Products);
    const int32 Index = S.Brands.Offers.IndexOfByPredicate([](const FMarketBrandOffer& O) { return O.Brand == TEXT("Lider"); });
    TestTrue(TEXT("The leader asks for room"), Index != INDEX_NONE);
    if (Index == INDEX_NONE) { MarketBrands::ResetTable(); return false; }
    const FMarketBrandOffer Offer = S.Brands.Offers[Index];
    FString Message;
    TestTrue(TEXT("Accepted"), MarketBrands::Accept(S, Offer.Id, Message));
    TestEqual(TEXT("A deal runs"), S.Brands.Deals.Num(), 1);

    // Keep the promise: the yogurt on the shelf, or the leader's share above the target.
    const int64 Before = S.Cash;
    S.Stock[1].Capacity = 20;
    S.Stock[0].Capacity = 40; S.Stock[2].Capacity = 5;
    Fill(S);
    Month(S, Products);
    TestTrue(TEXT("The brand paid"), S.Cash > Before);
    TestTrue(TEXT("Money counted"), S.Brands.TotalReceived > 0);

    // Starve it: under half its national share on our shelves, it cools and charges more.
    S.Brands.Deals.Reset(); S.Brands.Offers.Reset();
    S.Stock[0].Capacity = 1; S.Stock[1].Capacity = 0; S.Stock[2].Capacity = 60;
    Fill(S);
    for (int32 M = 0; M < 6; ++M) Month(S, Products);
    TestTrue(TEXT("A starved brand cools"), MarketBrands::Trust(S, TEXT("Lider")) <= -30.f);
    TestTrue(TEXT("and charges more"), MarketBrands::CostFactor(S, TEXT("Lider")) > 1.f);
    TestTrue(TEXT("Aisle line"), MarketBrands::AisleLine(S, Products, Milk).Contains(TEXT("Lidur")));

    MarketBrands::ResetTable();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBrandsEmptyShelfTest, "MirasMarket.Brands.EmptyShelfEarnsNothing", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketBrandsEmptyShelfTest::RunTest(const FString& Parameters)
{
    // C3 (A istek): an allocated but empty shelf is no shelf for a brand.
    using namespace MarketBrandsTest;
    Table();
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S = Start(Products);
    const FString Milk = TEXT("s\u00fct");
    const float Stocked = MarketBrands::ShelfShare(S, Products, TEXT("Lider"), Milk);
    S.Stock[0].Shelf = 0; S.Stock[0].Warehouse = 0;
    TestTrue(TEXT("The empty shelf does not count"), MarketBrands::ShelfShare(S, Products, TEXT("Lider"), Milk) < Stocked);
    // C7 (Codex C4): goods in the depot are not on the shelf; one unit on a big shelf is not the whole shelf.
    S.Stock[0].Warehouse = 1;
    TestTrue(TEXT("The depot does not count"), MarketBrands::ShelfShare(S, Products, TEXT("Lider"), Milk) < Stocked);
    S.Stock[0].Warehouse = 0; S.Stock[0].Capacity = 100; S.Stock[0].Shelf = 1;
    const float One = MarketBrands::ShelfShare(S, Products, TEXT("Lider"), Milk);
    S.Stock[0].Shelf = 50;
    TestTrue(TEXT("One unit is not a full shelf"), One < MarketBrands::ShelfShare(S, Products, TEXT("Lider"), Milk));
    MarketBrands::ResetTable();
    return true;
}

#endif

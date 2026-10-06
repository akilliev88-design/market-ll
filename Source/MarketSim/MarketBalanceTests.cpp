#include "MarketDemand.h"
#include "MarketPromotions.h"
#include "MarketFreshness.h"
#include "MarketFinance.h"
#include "MarketOnline.h"
#include "MarketStaff.h"
#include "MarketCompany.h"
#include "MarketBranches.h"
#include "MarketPrices.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// Akis B1: balance bugs of Docs/Kurgu/02_DERIN_INCELEME.md (#24, #27, #30, #43, #45).
namespace MarketBalanceTest
{
    FMarketProduct Make(const TCHAR* Id, const TCHAR* Category, int64 Cost, int64 Price, float Elasticity = 0.f)
    {
        FMarketProduct P;
        P.Id = Id; P.RealName = Id; P.Category = Category; P.Cost = Cost; P.BasePrice = Price; P.CaseUnits = 12; P.Elasticity = Elasticity;
        return P;
    }

    TArray<FMarketProduct> Catalog()
    {
        return {
            Make(TEXT("sut"), TEXT("s\u00fct"), 170, 250, 1.5f),
            Make(TEXT("ayran"), TEXT("s\u00fct"), 160, 240, 2.f),
            Make(TEXT("cola"), TEXT("i\u00e7ecek"), 180, 275, 4.f),
            Make(TEXT("deterjan"), TEXT("temizlik"), 1400, 1990, 3.f),
        };
    }

    bool AnyNews(const FMarketState& S, const TCHAR* Start)
    {
        for (const FString& Line : S.DayNews) if (Line.StartsWith(Start)) return true;
        return false;
    }

    bool NewsHas(const FMarketState& S, const TCHAR* Part)
    {
        for (const FString& Line : S.DayNews) if (Line.Contains(Part)) return true;
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBalanceElasticTest, "MarketSim.Balance.ElasticShelf", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketBalanceElasticTest::RunTest(const FString& Parameters)
{
    // #24: the shelf decision follows the product's elasticity.
    using namespace MarketDemand;
    const float Staple = 1.5f, Snack = 4.f;
    TestTrue(TEXT("At the rival's price 90 % buy, any product"), FMath::IsNearlyEqual(BuyChanceFor(1.0, 25.f, 0.0, Staple), 0.9, 1e-9) && FMath::IsNearlyEqual(BuyChanceFor(1.0, 25.f, 0.0, Snack), 0.9, 1e-9));
    const double StapleGain = BuyChanceFor(0.8, 25.f, 0.0, Staple) - BuyChanceFor(1.0, 25.f, 0.0, Staple);
    const double SnackGain = BuyChanceFor(0.8, 25.f, 0.0, Snack) - BuyChanceFor(1.0, 25.f, 0.0, Snack);
    TestTrue(TEXT("20 % cheaper: a snack wins more than a staple"), SnackGain > StapleGain && StapleGain > 0.03);
    TestTrue(TEXT("Cheaper really sells more (the v0.1 curve gave +2.5 points)"), SnackGain > 0.07);
    TestTrue(TEXT("15 % dearer: a snack loses more than a staple"), BuyChanceFor(1.15, 25.f, 0.0, Snack) < BuyChanceFor(1.15, 25.f, 0.0, Staple) - 0.2);
    const double Up = BuyChanceFor(0.9, 25.f, 0.0, 2.5f) - 0.9, Down = 0.9 - BuyChanceFor(1.1, 25.f, 0.0, 2.5f);
    TestTrue(TEXT("Loss aversion: dearer loses more than cheaper wins"), Down > Up);
    TestTrue(TEXT("A staple half-sells about 35 % over the rival"), FMath::Abs(BuyChanceFor(1.35, 25.f, 0.0, Staple) - 0.5) < 0.06);
    TestTrue(TEXT("Loyal shop is forgiven more"), BuyChanceFor(1.2, 60.f, 0.0, 2.5f) > BuyChanceFor(1.2, 10.f, 0.0, 2.5f));
    TestTrue(TEXT("Everybody knows the milk price: dearer milk loses more"), BuyChanceFor(1.1, 25.f, 0.0, Staple, 1.f) < BuyChanceFor(1.1, 25.f, 0.0, Staple, 0.f) - 0.1);
    TestTrue(TEXT("... and cheaper milk is noticed"), BuyChanceFor(0.9, 25.f, 0.0, Staple, 1.f) > BuyChanceFor(0.9, 25.f, 0.0, Staple, 0.f));
    TestTrue(TEXT("Tolerant segment accepts more"), BuyChanceFor(1.2, 25.f, 0.1, 2.5f) > BuyChanceFor(1.2, 25.f, 0.0, 2.5f));
    TestTrue(TEXT("Chance stays within 0..1"), BuyChanceFor(5.0, 0.f, -1.0, 6.f) >= 0.0 && BuyChanceFor(0.01, 100.f, 1.0, 6.f) <= 1.0);
    TestEqual(TEXT("Catalog value"), ElasticityOf(MarketBalanceTest::Make(TEXT("x"), TEXT("s\u00fct"), 1, 1, 3.f)), 3.f);
    TestEqual(TEXT("Default without one"), ElasticityOf(MarketBalanceTest::Make(TEXT("x"), TEXT("s\u00fct"), 1, 1)), DefaultElasticity);

    // Decide uses the product's own curve: 15 % over the rival, the same roll buys milk and leaves cola.
    const TArray<FMarketProduct> Products = MarketBalanceTest::Catalog();
    FMarketState S; S.Initialize(Products); S.ApplyShelfCapacities({ 12, 12, 12, 12 });
    for (FMarketStock& Row : S.Stock) Row.Shelf = 10;
    S.Stock[0].Price = 288; S.Stock[2].Price = 316; // both about +15 %
    const float Roll = 0.6f;
    TestTrue(TEXT("Milk still bought"), Decide(S, Products, 0, 10, 1.f, 1, Roll, 25.f).Result == EVisit::Buy);
    TestTrue(TEXT("Cola left on the shelf"), Decide(S, Products, 2, 10, 1.f, 1, Roll, 25.f).Result == EVisit::Expensive);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBalanceBelowCostTest, "MarketSim.Balance.BelowCostAndFundedDeal", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketBalanceBelowCostTest::RunTest(const FString& Parameters)
{
    // #27: selling below cost is told; the funded deal's report shows the support and the real profit.
    using namespace MarketBalanceTest;
    using namespace MarketPromotions;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S; S.Initialize(Products); S.Cash = 1000000; S.RivalSeed = 5;
    S.ApplyShelfCapacities({ 12, 12, 12, 12 });
    FString Message;

    TestTrue(TEXT("No warning at the list price"), MarketDemand::PriceWarning(S, Products, 1).IsEmpty());
    S.Stock[1].Price = 150; // ayran costs 160
    TestTrue(TEXT("Warning below the cost"), MarketDemand::PriceWarning(S, Products, 1).Contains(TEXT("maliyetin alt\u0131nda")));
    S.Stock[1].Today.Sold = 4;
    S.DayNews.Reset(); S.CloseDay(); MarketPromotions::CloseDay(S, Products);
    TestEqual(TEXT("Units below cost counted"), S.Ledger.LastBelowCostUnits, 4);
    TestEqual(TEXT("Loss on them"), S.Ledger.LastBelowCostLoss, int64(4 * 10));
    TestTrue(TEXT("Day report line"), AnyNews(S, TEXT("Maliyetin alt\u0131nda sat\u0131\u015f: 4 adet")));
    S.Stock[1].Price = 240;
    S.DayNews.Reset(); S.CloseDay(); MarketPromotions::CloseDay(S, Products);
    TestEqual(TEXT("Nothing below cost at the list price"), S.Ledger.LastBelowCostUnits, 0);
    // A deep discount that goes under the cost is found too (20 % off 250 = 200 > 170: fine; 3-for-2 on ayran 240 -> 160: not below).
    TestTrue(TEXT("Aisle discount"), Start(S, Products, EKind::AisleDiscount, 0, 20, Message));
    S.Stock[0].Today.Sold = 5;
    S.DayNews.Reset(); S.CloseDay(); MarketPromotions::CloseDay(S, Products);
    TestEqual(TEXT("20 % off milk stays above cost"), S.Ledger.LastBelowCostUnits, 0);

    // The funded offer: bought 15 % cheaper while it runs, sold 10 % off.
    FMarketState O; O.Initialize(Products); O.Cash = 1000000; O.RivalSeed = 5;
    O.ApplyShelfCapacities({ 12, 12, 12, 12 });
    O.Offer.Kind = static_cast<uint8>(EKind::SupplierDeal); O.Offer.Product = 2; O.Offer.Category = Products[2].Category;
    O.Offer.Percent = DealCostCut; O.Offer.EndDay = O.Day + 2;
    TestTrue(TEXT("Accept"), AcceptOffer(O, Products, Message));
    TArray<FMarketProduct> DealCosts = Products;
    DealCosts[2].Cost = FMath::RoundToInt64(Products[2].Cost * (1.0 - DealCostCut / 100.0)); // what ApplyPrices gives during the deal
    TArray<int32> Cases; Cases.Init(0, Products.Num()); Cases[2] = 2;
    TestTrue(TEXT("Order on the deal"), O.SubmitOrder(Cases, DealCosts));
    O.Stock[2].Shelf = 12; O.Stock[2].Today.Sold = 6;
    O.DayNews.Reset(); O.CloseDay(); MarketPromotions::CloseDay(O, DealCosts);
    const int32 Received = O.Stock[2].Dock; // delivered at this close (minus anything missing or broken)
    TestEqual(TEXT("A tally runs"), O.Ledger.PromoTallies.Num(), 1);
    if (O.Ledger.PromoTallies.Num() == 1)
    {
        const FMarketPromoTally& T = O.Ledger.PromoTallies[0];
        TestEqual(TEXT("Support = units received x the cost cut"), T.Support, FMath::RoundToInt64(Received * DealCosts[2].Cost * DealCostCut / (100.0 - DealCostCut)));
        TestEqual(TEXT("Revenue at the deal price"), T.Revenue, int64(6) * O.Stock[2].Price - int64(6) * O.Stock[2].Price * DealShelfCut / 100);
        TestTrue(TEXT("Cost of the units sold"), T.CostOfGoods > 0);
    }
    for (int32 D = 0; D < DealDays; ++D) { O.DayNews.Reset(); O.CloseDay(); MarketPromotions::CloseDay(O, Products); if (NewsHas(O, TEXT("Kampanya bitti"))) break; }
    TestTrue(TEXT("Report: the support"), NewsHas(O, TEXT("toptanc\u0131 deste\u011fi")));
    TestTrue(TEXT("Report: the real profit"), NewsHas(O, TEXT("br\u00fct k\u00e2r\u0131")));
    TestEqual(TEXT("Tally closed with the report"), O.Ledger.PromoTallies.Num(), 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBalanceOnlineTest, "MarketSim.Balance.OnlineDeals", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketBalanceOnlineTest::RunTest(const FString& Parameters)
{
    // #30: online orders pay the shelf deal, only carried products are ordered, and the next promotion's "before"
    // counts the shop only.
    using namespace MarketBalanceTest;
    using namespace MarketPromotions;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S; S.Initialize(Products); S.RivalSeed = 11; S.Cash = 5000000;
    S.ApplyShelfCapacities({ 30, 30, 30, 0 }); // detergent is in the depot but on no shelf plan
    for (FMarketStock& Row : S.Stock) { Row.Warehouse = 80; Row.Shelf = 20; }
    S.Stock[3].Shelf = 0;
    FString Message;
    // M32: the first store sells through the platform (the epidemic's months: many orders).
    S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
    S.Day = MarketOnline::PandemicStart(S) + 10;
    S.Online.Reputation = 90.f;
    TestTrue(TEXT("Platform"), MarketOnline::Open(S, MarketOnline::EChannel::Platform, Message));
    TestTrue(TEXT("20 % off the milk aisle"), Start(S, Products, EKind::AisleDiscount, 0, 20, Message));
    int32 Detergent = 0, Milk = 0;
    bool bPricesRight = true;
    for (int32 D = 0; D < 3; ++D)
    {
        S.DayNews.Reset(); S.Served = 400; S.CloseDay();
        const int32 Closed = S.Day - 1;
        MarketOnline::CloseDay(S, Products);
        int64 Expected = 0;
        for (int32 I = 0; I < Products.Num(); ++I) Expected += S.Ledger.OnlineSold[I] * DealPrice(S, Products, I, 1, Closed);
        bPricesRight &= Expected == S.Online.LastRevenue;
        Detergent += S.Ledger.OnlineSold[3];
        Milk += S.Ledger.OnlineSold[0] + S.Ledger.OnlineSold[1];
    }
    TestTrue(TEXT("Orders came"), Milk > 0);
    TestTrue(TEXT("Orders pay the deal price"), bPricesRight);
    TestTrue(TEXT("The deal price is lower"), DealPrice(S, Products, 0, 1, S.Day - 1) < S.Stock[0].Price);
    TestEqual(TEXT("Nothing that is on no shelf"), Detergent, 0);

    // The "before" of a new promotion: yesterday's shop sales without the online units.
    S.Stock[2].Yesterday.Sold = FMath::Max(S.Stock[2].Yesterday.Sold, S.Ledger.OnlineSold[2]) + 3;
    const int32 Shop = S.Stock[2].Yesterday.Sold - S.Ledger.OnlineSold[2];
    TestTrue(TEXT("3 al 2 \u00f6de on cola"), Start(S, Products, EKind::MultiBuy, 2, 0, Message));
    TestEqual(TEXT("Baseline without online units"), S.Promotions.Last().Baseline, Shop * MultiBuyDays);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBalanceTaxTest, "MarketSim.Balance.TaxLossCarry", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketBalanceTaxTest::RunTest(const FString& Parameters)
{
    // #43: a losing week's loss lowers the next weeks' income tax.
    using namespace MarketBalanceTest;
    const TArray<FMarketProduct> Products = Catalog();
    auto Play = [](FMarketState& S, int64 Revenue, int64 Cogs, int64 Purchases)
    {
        S.Revenue = Revenue; S.CostOfGoods = Cogs; S.Purchases = Purchases; S.Served = 20;
        S.DayNews.Reset(); S.CloseDay(); MarketStaff::CloseDay(S);
    };
    FString Message;
    FMarketState Loss; Loss.Initialize(Products); Loss.Cash = 5000000; Loss.RivalSeed = 3;
    FMarketState Good = Loss;
    TestTrue(TEXT("Accountant (no audits)"), MarketStaff::HireAccountant(Loss, Message) && MarketStaff::HireAccountant(Good, Message));
    for (int32 D = 1; D <= 7; ++D) { Play(Loss, 3000, 2500, 2500); Play(Good, 50000, 30000, 30000); }
    TestTrue(TEXT("The loss is carried"), Loss.Ledger.TaxLossCarry > 0);
    TestEqual(TEXT("A profitable week carries nothing"), Good.Ledger.TaxLossCarry, int64(0));
    const int64 Carry = Loss.Ledger.TaxLossCarry;
    MarketStaff::PayTax(Good);
    Good.Books.TaxDue = 0;
    for (int32 D = 8; D <= 14; ++D) { Play(Loss, 50000, 30000, 30000); Play(Good, 50000, 30000, 30000); }
    TestTrue(TEXT("Less tax after a losing week"), Loss.Books.TaxDue < Good.Books.TaxDue);
    TestTrue(TEXT("The carry is used up (partly)"), Loss.Ledger.TaxLossCarry < Carry);
    const double Rate = static_cast<double>(MarketStaff::IncomeTaxRate) * static_cast<double>(MarketStaff::AccountantDeduction);
    TestTrue(TEXT("By about the rate x what was used"), FMath::Abs(static_cast<double>(Good.Books.TaxDue - Loss.Books.TaxDue) - (Carry - Loss.Ledger.TaxLossCarry) * Rate) <= 2.0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBalanceMarkdownTest, "MarketSim.Balance.LastDayMarkdown", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketBalanceMarkdownTest::RunTest(const FString& Parameters)
{
    // #43: the last-day markdown is only for the units of the old batch.
    using namespace MarketBalanceTest;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S; S.Initialize(Products); S.ApplyShelfCapacities({ 12, 12, 12, 12 });
    S.Stock[0].Shelf = 10; S.Stock[0].Warehouse = 0;
    FMarketBatch Old; Old.ProductId = TEXT("sut"); Old.Units = 3; Old.ExpiresDay = S.Day;
    FMarketBatch Fresh; Fresh.ProductId = TEXT("sut"); Fresh.Units = 7; Fresh.ExpiresDay = S.Day + 6;
    S.Batches = { Old, Fresh };
    TestEqual(TEXT("Three old units left"), MarketFreshness::MarkdownUnitsLeft(S, 0), 3);
    TestEqual(TEXT("One unit: 30 % off"), MarketPromotions::UnitPrice(S, Products, 0, 1), int64(175));
    TestEqual(TEXT("Four units: three marked, one full"), MarketPromotions::UnitPrice(S, Products, 0, 4), FMath::RoundToInt64(250 * (3 * 0.7 + 1) / 4.0));
    S.Stock[0].Today.Sold = 3; S.Stock[0].Shelf = 7;
    TestEqual(TEXT("After the old ones are sold: full price"), MarketPromotions::UnitPrice(S, Products, 0, 1), int64(250));
    TestTrue(TEXT("No badge any more"), MarketPromotions::Badge(S, Products, 0).IsEmpty());
    S.FreshPolicy = static_cast<uint8>(MarketFreshness::EPolicy::Donate);
    S.Stock[0].Today.Sold = 0;
    TestEqual(TEXT("Donation policy: no markdown"), MarketPromotions::UnitPrice(S, Products, 0, 1), int64(250));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBalanceNationalShareTest, "MarketSim.Balance.NationalShareByRevenue", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketBalanceNationalShareTest::RunTest(const FString& Parameters)
{
    // #45: the national share follows revenue, not the store count.
    using namespace MarketCompany;
    FMarketState S; S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli"); S.Day = 2;
    TestTrue(TEXT("The country's market"), CountryMarketDay(S) > 1000000000);
    TestEqual(TEXT("No revenue, no share"), NationalShare(S), 0.f);
    S.LastRevenue = 60000;
    const float One = NationalShare(S);
    S.LastRevenue = 120000;
    TestTrue(TEXT("Twice the revenue, twice the share"), FMath::IsNearlyEqual(NationalShare(S), 2.f * One, 1e-6f));
    FMarketBranch Empty; Empty.Country = TEXT("tr"); Empty.Province = TEXT("edirne"); Empty.Stage = static_cast<uint8>(MarketBranches::EStage::Open); Empty.LastRevenue = 0;
    S.Branches.Add(Empty);
    TestTrue(TEXT("A store without revenue adds nothing"), FMath::IsNearlyEqual(NationalShare(S), 2.f * One, 1e-6f));
    // Fifty discounter-size stores (about 900 a day) hold about 0.1 %, like fifty stores of the biggest chain.
    S.Branches.Reset(); S.LastRevenue = 0;
    for (int32 I = 0; I < 50; ++I) { FMarketBranch B; B.Country = TEXT("tr"); B.Province = TEXT("edirne"); B.Stage = static_cast<uint8>(MarketBranches::EStage::Open); B.LastRevenue = 90000; S.Branches.Add(B); }
    const float Fifty = NationalShare(S);
    TestTrue(TEXT("Fifty discounters: about 0.1 %"), Fifty > 0.06f && Fifty < 0.15f);
    FMarketBranch Abroad; Abroad.Country = TEXT("de"); Abroad.Province = TEXT("by"); Abroad.Stage = static_cast<uint8>(MarketBranches::EStage::Open); Abroad.LastRevenue = 9000000;
    S.Branches.Add(Abroad);
    TestTrue(TEXT("Stores abroad do not count"), FMath::IsNearlyEqual(NationalShare(S), Fifty, 1e-6f));
    // Smoothed over about a month.
    TrackNationalRevenue(S);
    TestEqual(TEXT("First measured day"), S.Ledger.CountryRevenueDay, int64(50 * 90000));
    for (FMarketBranch& B : S.Branches) B.LastRevenue = 0;
    TrackNationalRevenue(S);
    TestEqual(TEXT("A bad day moves it a thirtieth"), S.Ledger.CountryRevenueDay, int64(50 * 90000) - int64(50 * 90000) / 30);
    TestTrue(TEXT("The share keeps the month's level"), NationalShare(S) > 0.9f * Fifty);
    return true;
}

#endif

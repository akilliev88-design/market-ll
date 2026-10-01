#include "MarketCompetitors.h"
#include "MarketStaff.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketCompetitorsTest
{
    TArray<FMarketProduct> Catalog()
    {
        FMarketProduct Milk; Milk.Id = TEXT("milk"); Milk.Category = TEXT("s\u00fct"); Milk.Cost = 170; Milk.BasePrice = 250;
        FMarketProduct Cola; Cola.Id = TEXT("cola"); Cola.Category = TEXT("i\u00e7ecek"); Cola.Cost = 180; Cola.BasePrice = 275;
        return { Milk, Cola };
    }

    void Play(FMarketState& S, const TArray<FMarketProduct>& Products, const TArray<FString>& Aisles, int32 MilkSold, int32 ColaSold)
    {
        S.Stock[0].Today.Sold = MilkSold; S.Stock[1].Today.Sold = ColaSold;
        S.Served = MilkSold + ColaSold;
        S.CloseDay();
        S.DayNews.Reset();
        MarketCompetitors::CloseDay(S, Products, Aisles);
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCompetitorsTest, "MirasMarket.Competitors.SharesAndWars", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCompetitorsTest::RunTest(const FString& Parameters)
{
    using namespace MarketCompetitors;
    using namespace MarketCompetitorsTest;
    const TArray<FMarketProduct> Products = Catalog();
    const TArray<FString> Aisles = { TEXT("i\u00e7ecek"), TEXT("s\u00fct") };
    FMarketState S; S.Initialize(Products); S.RivalSeed = 11; S.Cash = 100000;
    S.ApplyShelfCapacities({ 24, 24 });
    Ensure(S);
    TestEqual(TEXT("Five companies"), S.Competitors.Num(), static_cast<int32>(ECompany::Count));
    TestFalse(TEXT("A101 not yet"), IsOpen(S, ECompany::A101));
    S.Day = 15;
    TestTrue(TEXT("A101 on day 15"), IsOpen(S, ECompany::A101));
    TestFalse(TEXT("\u015eok arrives in summer"), IsOpen(S, ECompany::Sok));
    S.Day = 1;
    const float Rival = RivalPriceFactor(S, TEXT("s\u00fct"), Aisles);
    TestTrue(TEXT("Rivals price around the list, a little below"), Rival > 0.9f && Rival < 1.03f);

    // Our attractiveness follows price and full shelves.
    for (FMarketStock& Item : S.Stock) { Item.Shelf = 24; Item.Yesterday.Sold = 20; }
    const float Fair = TargetShare(S, Products, Aisles);
    TestTrue(TEXT("A fair family shop gets a real share"), Fair > 0.15f && Fair < 0.45f);
    for (FMarketStock& Item : S.Stock) Item.Price = Item.Price * 85 / 100;
    TestTrue(TEXT("Cheaper shop, more share"), TargetShare(S, Products, Aisles) > Fair);
    for (FMarketStock& Item : S.Stock) { Item.Price = Item.Price * 100 / 85; Item.Yesterday.Empty = 20; }
    TestTrue(TEXT("Empty shelves lose share"), TargetShare(S, Products, Aisles) < Fair);
    for (FMarketStock& Item : S.Stock) Item.Yesterday.Empty = 0;
    TestTrue(TEXT("Share brings shoppers"), [&] { S.MarketShare = 45.f; const float High = TrafficFactor(S); S.MarketShare = 10.f; return High > 1.2f && TrafficFactor(S) < 0.8f; }());
    S.MarketShare = 25.f;

    // Bereket gets angry when we take his customers and starts a price war on our best aisle.
    FMarketCompetitor& Bereket = S.Competitors[static_cast<int32>(ECompany::Bereket)];
    Bereket.Anger = 60.f;
    S.MarketShare = 45.f;
    Play(S, Products, Aisles, 40, 5);
    TestTrue(TEXT("War on milk"), Bereket.WarCategory == TEXT("s\u00fct") && Bereket.WarUntil >= S.Day);
    {
        // G-077 (#15): our share and the rivals' shares add up to 100 %.
        float Sum = S.MarketShare / 100.f;
        for (const FMarketCompetitor& C : S.Competitors) Sum += C.Share;
        TestTrue(TEXT("Shares add up"), FMath::Abs(Sum - 1.f) < 0.01f);
        // G-077 (#14): the war on milk makes Bereket more attractive in the share model, not only on the shelf.
        const float WarShare = TargetShare(S, Products, Aisles);
        const int32 Until = Bereket.WarUntil;
        Bereket.WarUntil = 0;
        TestTrue(TEXT("A price war takes share"), WarShare < TargetShare(S, Products, Aisles));
        Bereket.WarUntil = Until;
    }
    TestTrue(TEXT("Bereket's milk is 15 % cheaper"), PriceIndex(S, ECompany::Bereket, TEXT("s\u00fct"), Aisles) < 0.9f);
    TestTrue(TEXT("Shoppers see cheaper milk elsewhere"), RivalPriceFactor(S, TEXT("s\u00fct"), Aisles) < RivalPriceFactor(S, TEXT("i\u00e7ecek"), Aisles));
    bool bTold = false;
    for (const FString& Line : S.DayNews) if (Line.StartsWith(DisplayName(ECompany::Bereket) + TEXT(" sana cevap"))) bTold = true;
    TestTrue(TEXT("War announced"), bTold);
    for (int32 D = 0; D < MarketCompetitors::WarDays; ++D) Play(S, Products, Aisles, 40, 5);
    TestTrue(TEXT("The war ends"), PriceIndex(S, ECompany::Bereket, TEXT("s\u00fct"), Aisles) > 0.95f);
    // Fights and a small share empty his pockets: he gives up and raises prices.
    Bereket.Cash = -20000;
    Play(S, Products, Aisles, 40, 5);
    TestTrue(TEXT("Broke Bereket raises prices"), Bereket.BaseIndex > 1.f);

    // Poaching: an unhappy good cashier leaves, a happy one stays.
    FMarketState P; P.Initialize(Products); P.RivalSeed = 5; P.ApplyShelfCapacities({ 24, 24 });
    FMarketEmployee Good; Good.Id = 1; Good.Name = TEXT("Selin"); Good.Role = static_cast<uint8>(MarketStaff::ERole::Cashier); Good.Skill = 80; Good.Morale = 30.f; Good.HiredDay = 1;
    P.Staff.Add(Good); P.NextEmployeeId = 2;
    for (int32 D = 0; D < 200 && P.Staff[0].LeaveDay == 0; ++D) { P.Staff[0].Morale = 30.f; Play(P, Products, Aisles, 10, 10); }
    TestTrue(TEXT("An unhappy good worker is poached"), P.Staff[0].LeaveDay > 0);
    FMarketState H; H.Initialize(Products); H.RivalSeed = 5; H.ApplyShelfCapacities({ 24, 24 });
    Good.Morale = 90.f; H.Staff.Add(Good); H.NextEmployeeId = 2;
    for (int32 D = 0; D < 200; ++D) { H.Staff[0].Morale = 90.f; Play(H, Products, Aisles, 10, 10); }
    TestEqual(TEXT("A happy one stays"), H.Staff[0].LeaveDay, 0);
    TestFalse(TEXT("Describe"), Describe(S, ECompany::Bereket).IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketNeighbourhoodTest, "MirasMarket.Competitors.Neighbourhood", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketNeighbourhoodTest::RunTest(const FString& Parameters)
{
    // G-079: corner grocers, the Tuesday street market, Bereket for sale, chains answering a rising shop.
    using namespace MarketCompetitors;
    using namespace MarketCompetitorsTest;
    const TArray<FMarketProduct> Products = Catalog();
    const TArray<FString> Aisles = { TEXT("i\u00e7ecek"), TEXT("s\u00fct") };
    FMarketState S; S.Initialize(Products); S.RivalSeed = 11; S.Cash = 100000;
    S.ApplyShelfCapacities({ 24, 24 });
    Ensure(S);
    TestTrue(TEXT("Grocers every day"), IsOpenOn(S, ECompany::Bakkal, 1) && IsOpenOn(S, ECompany::Bakkal, 3));
    TestEqual(TEXT("Several corner grocers"), Find(S, ECompany::Bakkal)->Stores, 4);
    TestFalse(TEXT("No street market on Monday"), IsOpenOn(S, ECompany::Pazar, 1));
    TestTrue(TEXT("Street market on Tuesday"), IsOpenOn(S, ECompany::Pazar, 2));
    TestTrue(TEXT("Market sells dairy"), Sells(ECompany::Pazar, TEXT("s\u00fct")));
    TestFalse(TEXT("Market sells no drinks"), Sells(ECompany::Pazar, TEXT("i\u00e7ecek")));
    S.Day = 2;
    TestTrue(TEXT("Tuesday: milk looks cheaper elsewhere"), RivalPriceFactor(S, TEXT("s\u00fct"), Aisles) < [&] { FMarketState M = S; M.Day = 1; return RivalPriceFactor(M, TEXT("s\u00fct"), Aisles); }());
    S.Day = 1;
    TestFalse(TEXT("Real names by default"), DisplayName(ECompany::Bim).IsEmpty());

    // Bereket: a month without money and he sells; buying closes his shop.
    FMarketState B = S;
    FMarketCompetitor& Kadir = B.Competitors[static_cast<int32>(ECompany::Bereket)];
    for (int32 D = 0; D < SaleAfterRedDays + 1; ++D) { Kadir.Cash = -50000; Play(B, Products, Aisles, 10, 10); }
    const FMarketDecision* Sale = nullptr;
    for (const FMarketDecision& D : B.Decisions) if (D.Id == TEXT("rival.bereket")) Sale = &D;
    TestTrue(TEXT("Bereket for sale"), Sale != nullptr);
    if (Sale)
    {
        FString Message;
        B.Cash = Sale->Arg + 1000;
        TestTrue(TEXT("Bought"), Resolve(B, Products, *Sale, 0, Message));
        TestFalse(TEXT("His shop is closed"), IsOpen(B, ECompany::Bereket));
        TestEqual(TEXT("Paid"), B.Cash, int64(1000));
    }

    // Chains notice a rising shop and cut their prices a little.
    FMarketState R = S;
    const float Before = Find(R, ECompany::Bim)->BaseIndex;
    for (int32 D = 0; D < 21; ++D) { R.MarketShare = 20.f + D * 2.f; R.ShareBeforeClose = R.MarketShare; R.Served = 0; R.Stock[0].Today.Sold = 0; R.CloseDay(); R.DayNews.Reset(); CloseDay(R, Products, Aisles); }
    TestTrue(TEXT("BIM cut prices"), Find(R, ECompany::Bim)->BaseIndex < Before);
    return true;
}

#endif

#include "MarketChains.h"
#include "MarketBranches.h"
#include "MarketEconomy.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketChainsTest
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
            Make(TEXT("cola"), TEXT("i\u00e7ecek"), 180, 275, 80, 80, 260),
            Make(TEXT("makarna"), TEXT("makarna-bakliyat"), 210, 325, 190, 40, 70),
        };
    }

    FMarketState Start(int32 Seed = 7)
    {
        FMarketState S; S.Initialize(Catalog());
        S.RivalSeed = Seed; S.Day = 1; S.Cash = 100000000;
        S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
        return S;
    }

    int32 Index(const FMarketState& S, const TCHAR* Id)
    {
        return S.Rivals.Chains.IndexOfByPredicate([Id](const FMarketChain& C) { return C.Id == Id; });
    }

    void Days(FMarketState& S, int32 N)
    {
        for (int32 D = 0; D < N; ++D) { ++S.Day; S.DayNews.Reset(); MarketChains::CloseDay(S); }
    }

    // An open branch of ours in a province, without the opening steps.
    void OurBranch(FMarketState& S, const TCHAR* Province)
    {
        FMarketBranch& B = S.Branches.AddDefaulted_GetRef();
        B.Country = TEXT("tr"); B.Province = Province; B.Format = TEXT("mahalle"); B.Name = Province;
        B.Stage = static_cast<uint8>(MarketBranches::EStage::Open); B.OpenedDay = 1;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketChainsSeedTest, "MirasMarket.Chains.SeedAndGrow", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketChainsSeedTest::RunTest(const FString& Parameters)
{
    using namespace MarketChainsTest;
    FMarketState S = Start();
    MarketChains::Ensure(S);
    const int32 Bin = Index(S, TEXT("bim"));
    TestTrue(TEXT("National chains seeded"), Bin != INDEX_NONE && Index(S, TEXT("migros")) != INDEX_NONE);
    if (Bin == INDEX_NONE) return false;
    TestEqual(TEXT("Every start store placed"), MarketChains::TotalStores(S.Rivals.Chains[Bin]), 3500);
    TestTrue(TEXT("Regional chains of the home country"), S.Rivals.Chains.ContainsByPredicate([](const FMarketChain& C) { return C.Id.StartsWith(TEXT("bolge.")); }));
    TestTrue(TEXT("Local chains of the home province"), S.Rivals.Chains.ContainsByPredicate([](const FMarketChain& C) { return C.Id.StartsWith(TEXT("yerel.kirklareli")); }));
    TestEqual(TEXT("Twelve giants"), S.Rivals.Giants.Num(), 12);
    TestTrue(TEXT("Start pressure is the baseline"), FMath::IsNearlyEqual(MarketChains::PressureFactor(S, TEXT("tr"), TEXT("tekirdag"), S.Day), 1.f, 0.01f));
    const int32 Count = S.Rivals.Chains.Num();
    MarketChains::Ensure(S);
    TestEqual(TEXT("Seeding twice changes nothing"), S.Rivals.Chains.Num(), Count);

    // A year: the country has room at the start, so the chains grow; the money stays finite.
    Days(S, 365);
    int32 Grown = 0;
    for (const FMarketChain& C : S.Rivals.Chains)
    {
        TestTrue(TEXT("Cash is finite"), FMath::Abs(static_cast<double>(C.Cash)) < 1.0e15);
        if (C.Id == TEXT("bim") && MarketChains::TotalStores(C) > 3500) ++Grown;
    }
    TestEqual(TEXT("The discounter grew"), Grown, 1);
    TestTrue(TEXT("Its first months were booked"), S.Rivals.Chains[Bin].MonthRevenue > 0);
    TestTrue(TEXT("More chain stores in the country, more pressure somewhere"), MarketChains::PressureFactor(S, TEXT("tr"), TEXT("istanbul"), S.Day) >= 1.f);

    // Tables.
    const TArray<MarketChains::FStanding> National = MarketChains::NationalTable(S, TEXT("tr"));
    TestTrue(TEXT("National table sorted"), National.Num() > 3 && National[0].Revenue >= National[1].Revenue);
    TestTrue(TEXT("We are in it"), MarketChains::OurRank(National) > 0);
    const TArray<MarketChains::FStanding> World = MarketChains::WorldTable(S);
    TestTrue(TEXT("World league: giants and us"), World.Num() >= 13 && MarketChains::OurRank(World) > 0);

    // Same seed, same world.
    FMarketState T = Start();
    MarketChains::Ensure(T);
    Days(T, 365);
    TestEqual(TEXT("Deterministic"), MarketChains::TotalStores(T.Rivals.Chains[Index(T, TEXT("bim"))]), MarketChains::TotalStores(S.Rivals.Chains[Bin]));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketChainsWarTest, "MirasMarket.Chains.WarNemesisAndSale", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketChainsWarTest::RunTest(const FString& Parameters)
{
    using namespace MarketChainsTest;
    FMarketState S = Start(11);
    OurBranch(S, TEXT("tekirdag"));
    MarketChains::Ensure(S);
    const int32 A = Index(S, TEXT("a101"));
    TestTrue(TEXT("Fast discounter"), A != INDEX_NONE);
    if (A == INDEX_NONE) return false;
    {
        FMarketChain& C = S.Rivals.Chains[A];
        C.Aggression = 1.f; C.Rivalry = 80.f; C.Cash = 1000000000000; C.TurnDay = S.Day - 30;
        FMarketChainSpot* Spot = C.Spots.FindByPredicate([](const FMarketChainSpot& P) { return P.Province == TEXT("tekirdag"); });
        if (!Spot) { C.Spots.AddDefaulted_GetRef().Province = TEXT("tekirdag"); Spot = &C.Spots.Last(); }
        Spot->Stores = 500; // its strongest province, where we are
    }
    const float Before = MarketChains::PressureFactor(S, TEXT("tr"), TEXT("tekirdag"), S.Day);
    Days(S, 1);
    TestEqual(TEXT("A price war where we are"), S.Rivals.Chains[A].WarProvince, FString(TEXT("tekirdag")));
    TestTrue(TEXT("The war bites our branch"), MarketChains::PressureFactor(S, TEXT("tr"), TEXT("tekirdag"), S.Day) > Before * 1.2f);
    TestEqual(TEXT("The chain that minds us most is the nemesis"), S.Rivals.Nemesis, FString(TEXT("a101")));
    TestFalse(TEXT("Nemesis line"), MarketChains::NemesisLine(S).IsEmpty());
    Days(S, MarketChains::WarDays + 1);
    TestTrue(TEXT("The war ends"), S.Rivals.Chains[A].WarProvince.IsEmpty() || S.Rivals.Chains[A].WarUntil > S.Day);

    // A local chain deep in the red goes for sale; we buy it and its stores become our branches.
    const int32 Local = S.Rivals.Chains.IndexOfByPredicate([](const FMarketChain& C) { return C.Id.StartsWith(TEXT("yerel.kirklareli")); });
    TestTrue(TEXT("A local chain"), Local != INDEX_NONE);
    if (Local == INDEX_NONE) return false;
    {
        FMarketChain& C = S.Rivals.Chains[Local];
        C.Cash = -1000000000; C.RedTurns = 2; C.TurnDay = S.Day - 30;
    }
    Days(S, 1);
    TestTrue(TEXT("For sale"), S.Rivals.Chains[Local].bForSale);
    FString Why;
    TestTrue(TEXT("We can buy it"), MarketChains::CanBuy(S, Local, Why));
    const int32 Branches = S.Branches.Num();
    const int64 Cash = S.Cash;
    TestTrue(TEXT("Bought"), MarketChains::Buy(S, Catalog(), Local, Why));
    TestTrue(TEXT("Its stores are our branches now"), S.Branches.Num() > Branches);
    TestTrue(TEXT("The new branch is open and stocked"), S.Branches.Last().Stage == static_cast<uint8>(MarketBranches::EStage::Open)
        && S.Branches.Last().Items.ContainsByPredicate([](const FMarketBranchItem& I) { return I.Units > 0; }));
    TestTrue(TEXT("It cost money"), S.Cash < Cash);
    TestTrue(TEXT("The chain is gone"), S.Rivals.Chains[Local].bGone);
    TestFalse(TEXT("Not twice"), MarketChains::CanBuy(S, Local, Why));
    return true;
}

#endif

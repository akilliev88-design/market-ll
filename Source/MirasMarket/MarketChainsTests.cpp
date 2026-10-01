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
    TestTrue(TEXT("The war bites our branch"), MarketChains::PressureFactor(S, TEXT("tr"), TEXT("tekirdag"), S.Day) > Before * 1.05f); // C3: WarPressure 1.1
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
    TestTrue(TEXT("The chain is ours (merged, or what did not fit runs as our subsidiary)"), S.Rivals.Chains[Local].bGone || S.Rivals.Chains[Local].bOurs);
    TestFalse(TEXT("Not twice"), MarketChains::CanBuy(S, Local, Why));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketChainsBidTest, "MirasMarket.Chains.TakeoverBid", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketChainsBidTest::RunTest(const FString& Parameters)
{
    using namespace MarketChainsTest;
    FMarketState S = Start(13);
    OurBranch(S, TEXT("kirklareli"));
    MarketChains::Ensure(S);
    FString Message;
    // A struggling local chain that does not mind us says yes on most days; a bid that is accepted buys it.
    const int32 Local = S.Rivals.Chains.IndexOfByPredicate([](const FMarketChain& C) { return C.Id.StartsWith(TEXT("yerel.kirklareli")); });
    TestTrue(TEXT("A local chain"), Local != INDEX_NONE);
    if (Local == INDEX_NONE) return false;
    S.Rivals.Chains[Local].Cash = -1000; S.Rivals.Chains[Local].RedTurns = 2; S.Rivals.Chains[Local].Rivalry = 0.f;
    TestTrue(TEXT("Not for sale, can bid"), MarketChains::CanBid(S, Local, Message, false));
    TestTrue(TEXT("Likely yes"), MarketChains::AcceptChance(S, Local) >= 0.8f);
    int32 Tries = 0;
    while (!MarketChains::WouldAccept(S, Local) && Tries < 40) { ++S.Day; ++Tries; }
    TestTrue(TEXT("A day it says yes"), MarketChains::WouldAccept(S, Local));
    S.Cash = MarketChains::BidPrice(S, Local) + 100000000;
    const int32 Branches = S.Branches.Num();
    TestTrue(TEXT("Bought"), MarketChains::Bid(S, Catalog(), Local, Message));
    TestTrue(TEXT("Its stores are ours"), (S.Rivals.Chains[Local].bGone || S.Rivals.Chains[Local].bOurs) && S.Branches.Num() > Branches);

    // A national chain that minds us says no: the advisers are paid, it is angrier and waits.
    const int32 Bin = Index(S, TEXT("bim"));
    TestTrue(TEXT("BIN"), Bin != INDEX_NONE);
    if (Bin == INDEX_NONE) return false;
    S.Rivals.Chains[Bin].Rivalry = 90.f;
    Tries = 0;
    while (MarketChains::WouldAccept(S, Bin) && Tries < 40) { ++S.Day; ++Tries; }
    const int64 Cash = S.Cash;
    const float Rivalry = S.Rivals.Chains[Bin].Rivalry;
    S.Cash = MarketChains::BidPrice(S, Bin) + 1000000;
    const int64 Before = S.Cash;
    TestTrue(TEXT("Bid made"), MarketChains::Bid(S, Catalog(), Bin, Message));
    TestFalse(TEXT("Refused"), S.Rivals.Chains[Bin].bGone);
    TestTrue(TEXT("Advisers paid"), S.Cash < Before && Before - S.Cash <= MarketChains::BidPrice(S, Bin) / 100);
    TestTrue(TEXT("Angrier"), S.Rivals.Chains[Bin].Rivalry > Rivalry);
    TestFalse(TEXT("Waits"), MarketChains::CanBid(S, Bin, Message));
    S.Cash = Cash;
    // The nemesis never sells.
    S.Rivals.Nemesis = S.Rivals.Chains[Bin].Id;
    TestEqual(TEXT("Nemesis: no"), MarketChains::AcceptChance(S, Bin), 0.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketChainsSubsidiaryTest, "MirasMarket.Chains.SubsidiaryAndExit", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketChainsSubsidiaryTest::RunTest(const FString& Parameters)
{
    using namespace MarketChainsTest;
    FMarketState S = Start(17);
    OurBranch(S, TEXT("kirklareli"));
    MarketChains::Ensure(S);
    FString Message;
    // A big chain for sale: all of it is ours, as a subsidiary under its own name.
    const int32 Sok = Index(S, TEXT("sok"));
    TestTrue(TEXT("A national chain"), Sok != INDEX_NONE);
    if (Sok == INDEX_NONE) return false;
    FMarketChain& C = S.Rivals.Chains[Sok];
    C.bForSale = true;
    const int32 Stores = MarketChains::TotalStores(C);
    TestTrue(TEXT("Big"), Stores > MarketChains::SmallChain);
    S.Cash = MarketChains::Price(S, Sok) + 100000000;
    const int32 Branches = S.Branches.Num();
    TestTrue(TEXT("Bought"), MarketChains::Buy(S, Catalog(), Sok, Message));
    TestTrue(TEXT("A subsidiary"), S.Rivals.Chains[Sok].bOurs && !S.Rivals.Chains[Sok].bGone);
    TestEqual(TEXT("Every store kept"), MarketChains::SubsidiaryStores(S), Stores);
    TestEqual(TEXT("No branch yet"), S.Branches.Num(), Branches);
    TestTrue(TEXT("Counts as ours in the country"), MarketChains::NationalTable(S, TEXT("tr")).ContainsByPredicate([](const MarketChains::FStanding& R) { return R.bUs && R.Stores > MarketChains::SmallChain; }));
    TestFalse(TEXT("Not in the table as a rival"), MarketChains::NationalTable(S, TEXT("tr")).ContainsByPredicate([Sok](const MarketChains::FStanding& R) { return R.Chain == Sok; }));

    // Its month comes to our till.
    const int64 Before = S.Cash;
    Days(S, MarketChains::TurnDays + 1);
    TestTrue(TEXT("Its result moved our cash"), S.Cash != Before);

    // Some of its stores take our name.
    const int32 Room = MarketChains::ConvertRoom(S, Sok);
    TestTrue(TEXT("Room to convert"), Room > 0 && Room <= MarketChains::ConvertPerMonth);
    S.Cash += MarketChains::ConvertCost(S, Sok, Room);
    TestTrue(TEXT("Converted"), MarketChains::Convert(S, Catalog(), Sok, Room, Message));
    TestTrue(TEXT("Branches now"), S.Branches.Num() > Branches);
    TestTrue(TEXT("Monthly cap"), MarketChains::ConvertRoom(S, Sok) <= MarketChains::ConvertPerMonth - Room);

    // Sold: a rival again.
    TestTrue(TEXT("Sold"), MarketChains::SellSubsidiary(S, Sok, Message));
    TestFalse(TEXT("A rival again"), S.Rivals.Chains[Sok].bOurs);

    // A giant leaving sells for six months of revenue instead of eight.
    const int32 Bin = Index(S, TEXT("bim"));
    S.Rivals.Chains[Bin].bForSale = true;
    const int64 Eight = MarketChains::Price(S, Bin);
    S.Rivals.Chains[Bin].bExitSale = true;
    TestTrue(TEXT("Exit sale is cheaper"), FMath::Abs(MarketChains::Price(S, Bin) - Eight * 6 / 8) <= 1);
    return true;
}

#endif

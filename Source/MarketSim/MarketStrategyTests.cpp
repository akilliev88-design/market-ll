#include "MarketStrategy.h"
#include "MarketBranches.h"
#include "MarketCountry.h"
#include "MarketEconomy.h"
#include "MarketEvents.h"
#include "MarketManagers.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketStrategyTest
{
    FMarketState Start()
    {
        FMarketState S; S.Initialize(TArray<FMarketProduct>());
        S.RivalSeed = 7; S.Day = 900; S.Cash = 500000000;
        S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
        S.Decisions.Reset();
        return S;
    }
    void AddShops(FMarketState& S, const FString& Province, int32 Count, float Satisfaction = 55.f)
    {
        for (int32 I = 0; I < Count; ++I)
        {
            FMarketBranch& B = S.Branches.AddDefaulted_GetRef();
            B.Country = TEXT("tr"); B.Province = Province; B.Format = TEXT("mahalle");
            B.Name = FString::Printf(TEXT("Test %d"), S.Branches.Num());
            B.Stage = static_cast<uint8>(MarketBranches::EStage::Open);
            B.OpenedDay = 100; B.Satisfaction = Satisfaction;
        }
    }
    void AddRival(FMarketState& S, const FString& Province, int32 Stores)
    {
        FMarketChain& C = S.Rivals.Chains.AddDefaulted_GetRef();
        C.Id = FString::Printf(TEXT("test.%d"), S.Rivals.Chains.Num());
        C.Name = TEXT("Rakip");
        C.Country = TEXT("tr");
        C.Cash = 100000000;
        FMarketChainSpot& Spot = C.Spots.AddDefaulted_GetRef();
        Spot.Province = Province; Spot.Stores = Stores;
    }
    FString CityWith(bool bBig)
    {
        if (const MarketCountry::FProfile* Pack = MarketCountry::Find(TEXT("tr")))
            for (const MarketCountry::FCity& City : Pack->Cities)
                if (City.Id != TEXT("kirklareli") && (bBig ? City.PopulationK >= MarketStrategy::BigCityK : City.PopulationK < MarketStrategy::SmallTownK)) return City.Id;
        return FString();
    }
    void Days(FMarketState& S, int32 N)
    {
        for (int32 D = 0; D < N; ++D) { ++S.Day; S.DayNews.Reset(); MarketStrategy::CloseDay(S, TArray<FMarketProduct>()); }
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStrategyProvinceTest, "MarketSim.Strategy.ProvincePush", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStrategyProvinceTest::RunTest(const FString& Parameters)
{
    using namespace MarketStrategyTest;
    FMarketState S = Start();
    const FString Town = CityWith(false);
    TestFalse(TEXT("A small town in the pack"), Town.IsEmpty());
    AddShops(S, Town, 3);
    AddRival(S, Town, 2);
    TestTrue(TEXT("More shops than every rival: champion"), MarketStrategy::IsChampion(S, TEXT("tr"), Town));
    AddRival(S, Town, 4);
    TestFalse(TEXT("A bigger rival: not champion"), MarketStrategy::IsChampion(S, TEXT("tr"), Town));
    TestEqual(TEXT("The leader's shops"), MarketStrategy::RivalStoresIn(S, TEXT("tr"), Town), 4);

    FString Why;
    TestTrue(TEXT("A push can start"), MarketStrategy::CanPush(S, TEXT("tr"), Town, Why));
    const float Before = MarketStrategy::PullFactor(S, TEXT("tr"), Town, S.Day);
    TestTrue(TEXT("Started"), MarketStrategy::StartPush(S, TEXT("tr"), Town, Why));
    TestTrue(TEXT("More shoppers during the push"), MarketStrategy::PullFactor(S, TEXT("tr"), Town, S.Day) > Before);
    TestFalse(TEXT("One push at a time there"), MarketStrategy::CanPush(S, TEXT("tr"), Town, Why));
    const int64 Cash = S.Cash;
    Days(S, MarketStrategy::PushDays + 1);
    TestTrue(TEXT("Ended"), S.Strategy.Pushes.Num() == 1 && S.Strategy.Pushes[0].bEnded);
    TestTrue(TEXT("Paid every day"), S.Strategy.PushPaid > 0 && S.Cash < Cash);
    int32 Left = 0;
    for (const FMarketChain& C : S.Rivals.Chains) for (const FMarketChainSpot& Spot : C.Spots) Left += Spot.Stores;
    TestEqual(TEXT("Rival shops that gave up are gone"), Left + S.Strategy.Pushes[0].Withdrawn, 6);
    TestFalse(TEXT("The province rests"), MarketStrategy::CanPush(S, TEXT("tr"), Town, Why));
    TestEqual(TEXT("No more push effect"), MarketStrategy::PullFactor(S, TEXT("tr"), Town, S.Day), Before);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStrategyForksTest, "MarketSim.Strategy.ForksAndPaths", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStrategyForksTest::RunTest(const FString& Parameters)
{
    using namespace MarketStrategyTest;
    FString Message;
    FMarketState S = Start();
    const FString Big = CityWith(true), Small = CityWith(false);
    TestFalse(TEXT("A big city in the pack"), Big.IsEmpty());
    AddShops(S, Small, 5, 75.f);
    Days(S, 1);
    TestTrue(TEXT("The focus fork at five branches"), S.Decisions.Num() == 1 && S.Decisions[0].Id == TEXT("strategy.focus"));
    TestTrue(TEXT("Decided"), MarketEvents::Decide(S, TArray<FMarketProduct>(), 1, Message));
    TestEqual(TEXT("Big cities"), static_cast<int32>(MarketStrategy::Focus(S)), static_cast<int32>(MarketStrategy::EFocus::BigCities));
    TestTrue(TEXT("Big cities draw more"), MarketStrategy::PullFactor(S, TEXT("tr"), Big, S.Day) > MarketStrategy::PullFactor(S, TEXT("tr"), Small, S.Day) - 0.0001f
        && MarketStrategy::PullFactor(S, TEXT("tr"), Big, S.Day) >= 1.05f);

    // Twenty-five branches: the growth fork; "not now" asks again later.
    AddShops(S, Small, 20, 75.f);
    Days(S, 1);
    TestTrue(TEXT("The growth fork"), S.Decisions.Num() == 1 && S.Decisions[0].Id == TEXT("strategy.growth"));
    TestTrue(TEXT("Not now"), MarketEvents::Decide(S, TArray<FMarketProduct>(), 3, Message));
    TestTrue(TEXT("Asked again in half a year"), S.Strategy.Growth == 0 && S.Strategy.AskGrowthDay > S.Day + 100);
    Days(S, 1);
    TestEqual(TEXT("Not asked again at once"), S.Decisions.Num(), 0);
    S.Strategy.AskGrowthDay = S.Day;
    Days(S, 1);
    TestTrue(TEXT("Own buildings"), S.Decisions.Num() == 1 && MarketEvents::Decide(S, TArray<FMarketProduct>(), 1, Message));
    TestTrue(TEXT("Cheaper rent, dearer fit-out"), MarketStrategy::RentFactor(S, TEXT("tr"), Big) < 0.6f && MarketStrategy::FitOutFactor(S) > 1.5f);

    // A hundred branches: the big investment needs the money.
    AddShops(S, Small, 75, 75.f);
    S.Cash = 0;
    Days(S, 1);
    TestTrue(TEXT("The vertical fork"), S.Decisions.Num() == 1 && S.Decisions[0].Id == TEXT("strategy.vertical"));
    TestTrue(TEXT("Answered"), MarketEvents::Decide(S, TArray<FMarketProduct>(), 0, Message));
    TestEqual(TEXT("No money, no plant"), S.Strategy.Vertical, static_cast<uint8>(0));
    S.Cash = 2000000000;
    S.Strategy.AskVerticalDay = S.Day;
    Days(S, 1);
    TestTrue(TEXT("Built"), S.Decisions.Num() == 1 && MarketEvents::Decide(S, TArray<FMarketProduct>(), 0, Message) && S.Strategy.Vertical == static_cast<uint8>(MarketStrategy::EVertical::Production));
    TestTrue(TEXT("Paid"), S.Cash < 2000000000);
    TestFalse(TEXT("Not ready yet"), MarketStrategy::ProductionReady(S));
    S.Day += MarketStrategy::ProductionDays;
    TestTrue(TEXT("Ready after a year"), MarketStrategy::ProductionReady(S) && MarketStrategy::ShelfPriceFactor(S) > 1.f);

    // Paths: satisfied branches, good managers.
    for (int32 I = 0; I < 5; ++I) { S.Branches[I].ManagerName = FString::Printf(TEXT("Mudur %d"), I); S.Branches[I].ManagerSkill = 70; } // D9: store managers live on the branches
    TestEqual(TEXT("People: second tier"), MarketStrategy::MeasurePathTier(S, MarketStrategy::EPath::People), 2);
    TestEqual(TEXT("Favourite: second tier"), MarketStrategy::MeasurePathTier(S, MarketStrategy::EPath::Favourite), 2);
    S.Strategy.LastLookDay = 0;
    Days(S, 1);
    TestEqual(TEXT("Managers widen management (M45)"), MarketStrategy::CapacityBonus(S), 4);
    TestTrue(TEXT("Celebrated"), S.Strategy.BestTiers.IsValidIndex(2) && S.Strategy.BestTiers[2] == 2);
    TestFalse(TEXT("Path line"), MarketStrategy::PathLine(S, MarketStrategy::EPath::People).IsEmpty());
    return true;
}

#endif

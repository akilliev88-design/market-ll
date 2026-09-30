#include "MarketDepots.h"
#include "MarketBranches.h"
#include "MarketCompany.h"
#include "MarketCountry.h"
#include "MarketDirector.h"
#include "MarketManagers.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketDepotsTest
{
    FMarketState MakeState()
    {
        FMarketState S;
        S.CountryId = TEXT("tr");
        S.CityId = TEXT("kirklareli");
        S.Day = 197;              // the day close of day 196 is a week's end
        S.RivalSeed = 11;
        S.Cash = 100000000;
        S.InheritedDebt = 0;
        return S;
    }

    // An open branch without a store manager (the depot rules only look at where it is).
    int32 AddShop(FMarketState& S, const TCHAR* Province, const TCHAR* Country = TEXT("tr"))
    {
        FMarketBranch B;
        B.Country = Country;
        B.Province = Province;
        B.Format = TEXT("mahalle");
        B.Name = FString::Printf(TEXT("%s %d"), Province, S.Branches.Num() + 1);
        B.Stage = static_cast<uint8>(MarketBranches::EStage::Open);
        B.OpenedDay = 1;
        B.Rent = 60000;
        return S.Branches.Add(B);
    }

    int32 AddDepot(FMarketState& S, const TCHAR* Province, const TCHAR* Country = TEXT("tr"))
    {
        FMarketDepot D;
        D.Country = Country;
        D.Province = Province;
        D.OpenedDay = 1;
        D.Rent = 600000;
        return S.Company.DepotSites.Add(D);
    }

    // A settled depot manager (no settling-in week, morale without effect).
    int32 AddDepotManager(FMarketState& S, const TCHAR* Province, int32 Skill, int32 Honesty = 80)
    {
        FMarketManager M;
        M.Level = static_cast<uint8>(MarketManagers::ELevel::Depot);
        M.Country = TEXT("tr");
        M.Area = Province;
        M.Name = FString::Printf(TEXT("Depocu %d"), S.Management.Managers.Num() + 1);
        M.Skill = Skill;
        M.Potential = 95;
        M.Honesty = Honesty;
        M.Morale = 60.f;
        M.AppointedDay = 1;
        M.BaseWage = 10000;
        return S.Management.Managers.Add(M);
    }

    bool NewsHas(const FMarketState& S, const TCHAR* Part)
    {
        return S.DayNews.ContainsByPredicate([Part](const FString& Line) { return Line.Contains(Part); });
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketDepotsDistanceTest, "MirasMarket.Depots.Distance", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketDepotsDistanceTest::RunTest(const FString& Parameters)
{
    using namespace MarketDepots;
    // Calibration of the map (iller.json centres x 1.61 km): Istanbul-Ankara ~350 km, Edirne-Kars ~1 330-1 390 km.
    const float IstanbulAnkara = DistanceKm(TEXT("tr"), TEXT("istanbul"), TEXT("ankara"));
    TestTrue(TEXT("Istanbul-Ankara about 350 km"), IstanbulAnkara >= 290.f && IstanbulAnkara <= 410.f);
    const float EdirneKars = DistanceKm(TEXT("tr"), TEXT("edirne"), TEXT("kars"));
    TestTrue(TEXT("Edirne-Kars about 1 330 km"), EdirneKars >= 1250.f && EdirneKars <= 1450.f);
    TestTrue(TEXT("Both ways the same"), FMath::IsNearlyEqual(DistanceKm(TEXT("tr"), TEXT("kars"), TEXT("edirne")), EdirneKars));
    TestTrue(TEXT("Same province"), FMath::IsNearlyEqual(DistanceKm(TEXT("tr"), TEXT("tekirdag"), TEXT("tekirdag")), 0.f));
    TestTrue(TEXT("Neighbours are near"), DistanceKm(TEXT("tr"), TEXT("tekirdag"), TEXT("kirklareli")) < 150.f);

    // A pack without a map: the region fallback (Germany: Schleswig-Holstein and Hamburg are both "Nord").
    TestTrue(TEXT("Same sub-region without a map"), FMath::IsNearlyEqual(DistanceKm(TEXT("de"), TEXT("sh"), TEXT("hh")), static_cast<float>(SameSubRegionKm)));
    TestTrue(TEXT("Far without a map"), FMath::IsNearlyEqual(DistanceKm(TEXT("de"), TEXT("sh"), TEXT("be")), static_cast<float>(OtherRegionKm)));

    // The road: the first 100 km free, then 0.6 % per 100 km.
    TestTrue(TEXT("Near is free"), FMath::IsNearlyEqual(DistanceCost(80.f), 0.f));
    TestTrue(TEXT("400 km cost 1.8 %"), FMath::IsNearlyEqual(DistanceCost(400.f), 0.018f, 0.0001f));
    TestTrue(TEXT("Farther costs more"), DistanceCost(500.f) > DistanceCost(300.f));

    // Short deliveries round by the roll.
    TestEqual(TEXT("2 % of 1000"), LostUnits(1000, 20, 0u), 20);
    TestEqual(TEXT("Nothing lost at 0"), LostUnits(1000, 0, 0u), 0);
    TestEqual(TEXT("Never more than the delivery"), LostUnits(3, 1000, 999u), 3);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketDepotsLinkTest, "MirasMarket.Depots.Links", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketDepotsLinkTest::RunTest(const FString& Parameters)
{
    using namespace MarketDepots;
    using namespace MarketDepotsTest;
    FMarketState S = MakeState();
    const int32 Istanbul = AddShop(S, TEXT("istanbul"));
    const int32 Eskisehir = AddShop(S, TEXT("eskisehir"));
    const int32 Van = AddShop(S, TEXT("van"));
    const int32 Home = AddShop(S, TEXT("kirklareli"));
    const float VanBefore = MarketCompany::CostFactor(S, S.Branches[Van]);
    const int32 Tekirdag = AddDepot(S, TEXT("tekirdag"));
    const int32 Ankara = AddDepot(S, TEXT("ankara"));

    // Every branch takes the nearest depot of its country.
    TestEqual(TEXT("Istanbul from Tekirdag"), LinkOf(S, S.Branches[Istanbul]).Depot, Tekirdag);
    TestEqual(TEXT("Eskisehir from Ankara"), LinkOf(S, S.Branches[Eskisehir]).Depot, Ankara);
    TestEqual(TEXT("The home shop from the depot next door"), LinkOf(S, S.Branches[Home]).Depot, Tekirdag);
    TestEqual(TEXT("Tekirdag serves two"), Served(S, Tekirdag), 2);
    TestTrue(TEXT("Distance on the link"), LinkOf(S, S.Branches[Istanbul]).Km > 100.f && LinkOf(S, S.Branches[Istanbul]).Km < 250.f);
    const TArray<FLink> Links = AllLinks(S);
    TestTrue(TEXT("All links at once agree"), Links.Num() == S.Branches.Num() && Links[Istanbul].Depot == Tekirdag && Links[Eskisehir].Depot == Ankara);

    // Beyond 600 km there is no depot: the wholesaler's van as before.
    TestEqual(TEXT("Van is out of range"), LinkOf(S, S.Branches[Van]).Depot, static_cast<int32>(INDEX_NONE));
    TestTrue(TEXT("Van buys from the wholesaler"), FMath::IsNearlyEqual(MarketCompany::CostFactor(S, S.Branches[Van]), VanBefore) && FMath::IsNearlyEqual(VanBefore, 1.f + WholesalerVan));

    // Farther from the depot, dearer goods.
    FMarketState One = MakeState();
    const int32 Near = AddShop(One, TEXT("kocaeli"));
    const int32 Far = AddShop(One, TEXT("ankara"));
    AddDepot(One, TEXT("tekirdag"));
    const FLink NearLink = LinkOf(One, One.Branches[Near]);
    const FLink FarLink = LinkOf(One, One.Branches[Far]);
    TestTrue(TEXT("Both served"), NearLink.Depot == 0 && FarLink.Depot == 0);
    TestTrue(TEXT("The road costs more far away"), FarLink.CostAdd > NearLink.CostAdd);
    TestTrue(TEXT("Cost factor follows"), MarketCompany::CostFactor(One, One.Branches[Far]) > MarketCompany::CostFactor(One, One.Branches[Near]));

    // Trucks: missing ones cost and lose goods on the way.
    TestTrue(TEXT("Trucks needed"), TrucksNeeded(One) >= 1 && FMath::IsNearlyEqual(TruckShortage(One), 1.f));
    const FLink Short = LinkOf(One, One.Branches[Far]);
    One.Company.Trucks = TrucksNeeded(One);
    const FLink Enough = LinkOf(One, One.Branches[Far]);
    TestTrue(TEXT("Enough trucks"), FMath::IsNearlyEqual(TruckShortage(One), 0.f) && Enough.CostAdd < Short.CostAdd && Enough.ShortPermille < Short.ShortPermille);
    TestTrue(TEXT("With trucks still cheaper than the van"), MarketCompany::CostFactor(One, One.Branches[Far]) < 1.f + WholesalerVan);
    TestFalse(TEXT("A menu line"), DescribeLink(One, Far).IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketDepotsManagerTest, "MirasMarket.Depots.Manager", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketDepotsManagerTest::RunTest(const FString& Parameters)
{
    using namespace MarketDepots;
    using namespace MarketDepotsTest;
    using MarketManagers::ELevel;
    FMarketState S = MakeState();
    const int32 Istanbul = AddShop(S, TEXT("istanbul"));
    AddShop(S, TEXT("tekirdag"));
    AddShop(S, TEXT("edirne"));
    FString Message;
    TestFalse(TEXT("No depot manager without a depot"), MarketManagers::CanAppoint(S, ELevel::Depot, TEXT("tr"), TEXT("tekirdag"), INDEX_NONE, Message));
    const int32 Depot = AddDepot(S, TEXT("tekirdag"));

    // Without a manager the depot works at half and says so every week.
    TestTrue(TEXT("Half efficiency"), FMath::IsNearlyEqual(Efficiency(S, Depot), UnmanagedEfficiency));
    const FLink Alone = LinkOf(S, S.Branches[Istanbul]);
    TestTrue(TEXT("Half the rebate"), FMath::IsNearlyEqual(Alone.Efficiency, 0.5f, 0.0001f) && Alone.ShortPermille > 0 && Alone.ExtraWaste > 0.f);
    S.DayNews.Reset();
    CloseDay(S);
    TestTrue(TEXT("Weekly warning"), NewsHas(S, TEXT("m\u00fcd\u00fcr yok")));
    TestTrue(TEXT("In the suggestions"), MarketManagers::Suggestions(S).ContainsByPredicate([](const FString& Line) { return Line.Contains(TEXT("depo m\u00fcd\u00fcr\u00fc")); }));

    // The wage band: 80-120 TL a day in 2011.
    TestEqual(TEXT("Lowest wage"), MarketManagers::BaseWageFor(ELevel::Depot, 30, TEXT("tr")), static_cast<int64>(8000));
    TestEqual(TEXT("Highest wage"), MarketManagers::BaseWageFor(ELevel::Depot, 90, TEXT("tr")), static_cast<int64>(12000));

    // Three candidates; the chosen one counts as one of the player's people.
    const int32 Before = MarketManagers::DirectCount(S);
    TestEqual(TEXT("Three candidates"), MarketManagers::Candidates(S, ELevel::Depot, TEXT("tr"), TEXT("tekirdag")).Num(), MarketManagers::CandidateCount);
    TestTrue(TEXT("Appoint a depot manager"), MarketManagers::AppointCandidate(S, ELevel::Depot, TEXT("tr"), TEXT("tekirdag"), 1, Message));
    TestTrue(TEXT("His depot"), ManagerOf(S, Depot) != INDEX_NONE);
    TestEqual(TEXT("One more of the player's people"), MarketManagers::DirectCount(S), Before + 1);
    TestFalse(TEXT("Only one per depot"), MarketManagers::CanAppoint(S, ELevel::Depot, TEXT("tr"), TEXT("tekirdag"), INDEX_NONE, Message));

    // His boss: not the province manager, the country manager.
    const int32 Mine = ManagerOf(S, Depot);
    FMarketManager Province;
    Province.Level = static_cast<uint8>(ELevel::Province); Province.Country = TEXT("tr"); Province.Area = TEXT("tekirdag");
    Province.Name = TEXT("\u0130l M\u00fcd\u00fcr\u00fc"); Province.Skill = 70; Province.Potential = 80; Province.AppointedDay = 1;
    S.Management.Managers.Add(Province);
    MarketManagers::FPerson Person;
    Person.Level = ELevel::Depot;
    Person.Manager = Mine;
    TestEqual(TEXT("A province manager does not run the depot"), MarketManagers::BossOf(S, Person), static_cast<int32>(INDEX_NONE));
    FMarketManager Head;
    Head.Level = static_cast<uint8>(ELevel::Country); Head.Country = TEXT("tr"); Head.Area = TEXT("tr");
    Head.Name = TEXT("\u00dclke M\u00fcd\u00fcr\u00fc"); Head.Skill = 70; Head.Potential = 80; Head.AppointedDay = 1;
    const int32 HeadIndex = S.Management.Managers.Add(Head);
    TestEqual(TEXT("The country manager does"), MarketManagers::BossOf(S, Person), HeadIndex);

    // Skill: less waste, fewer short deliveries, fuller shelves.
    FMarketState Weak = MakeState();
    const int32 WeakShop = AddShop(Weak, TEXT("istanbul"));
    AddDepot(Weak, TEXT("tekirdag"));
    AddDepotManager(Weak, TEXT("tekirdag"), 30);
    FMarketState Strong = Weak;
    Strong.Management.Managers[0].Skill = 90;
    const FLink WeakLink = LinkOf(Weak, Weak.Branches[WeakShop]);
    const FLink StrongLink = LinkOf(Strong, Strong.Branches[WeakShop]);
    TestTrue(TEXT("A manager beats none"), WeakLink.Efficiency > UnmanagedEfficiency);
    TestTrue(TEXT("Skill lowers waste"), StrongLink.ExtraWaste < WeakLink.ExtraWaste);
    TestTrue(TEXT("Skill lowers short deliveries"), StrongLink.ShortPermille < WeakLink.ShortPermille);
    TestTrue(TEXT("Skill brings the rebate"), StrongLink.CostAdd < WeakLink.CostAdd);

    // A dishonest one takes a little; a warning stops him for a while.
    FMarketState Thief = Strong;
    Thief.Management.Managers[0].Honesty = 20;
    TestEqual(TEXT("Takes goods"), LinkOf(Thief, Thief.Branches[WeakShop]).SkimPermille, SkimPermille);
    Thief.Management.Managers[0].WarnedDay = Thief.Day;
    TestEqual(TEXT("Warned"), LinkOf(Thief, Thief.Branches[WeakShop]).SkimPermille, 0);
    // Once caught, the suggestions ask for a new depot manager.
    Thief.Company.DepotSites[0].CaughtName = Thief.Management.Managers[0].Name;
    TestTrue(TEXT("Caught depot manager in the suggestions"), MarketManagers::Suggestions(Thief).ContainsByPredicate([](const FString& Line) { return Line.Contains(TEXT("maldan ka\u00e7\u0131r\u0131yor")); }));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketDepotsBuildTest, "MirasMarket.Depots.BuildAndAdvice", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketDepotsBuildTest::RunTest(const FString& Parameters)
{
    using namespace MarketDepots;
    using namespace MarketDepotsTest;
    FMarketState S = MakeState();
    FString Message;
    AddShop(S, TEXT("edirne"));
    AddShop(S, TEXT("tekirdag"));
    TestFalse(TEXT("A depot needs four stores"), CanBuild(S, TEXT("tr"), TEXT("istanbul"), Message));
    for (int32 N = 0; N < 3; ++N) AddShop(S, TEXT("istanbul"));
    AddShop(S, TEXT("kocaeli"));

    // The advice: the province nearest to the branches, weighted by revenue.
    const FAdvice Advice = SuggestDepotProvince(S, TEXT("tr"));
    TestFalse(TEXT("A province"), Advice.Province.IsEmpty());
    TestTrue(TEXT("Near the weight of the branches"), DistanceKm(TEXT("tr"), Advice.Province, TEXT("istanbul")) < 150.f);
    TestTrue(TEXT("Serves them all"), Advice.Branches == 6 && Advice.AverageKm < 200.f);
    TestFalse(TEXT("A line for the menu"), Advice.Text.IsEmpty());
    FMarketState Heavy = S;
    Heavy.Branches[0].LastRevenue = 500000000; // Edirne sells a hundred times more
    TestTrue(TEXT("Revenue pulls the advice"), DistanceKm(TEXT("tr"), SuggestDepotProvince(Heavy, TEXT("tr")).Province, TEXT("edirne")) < DistanceKm(TEXT("tr"), Advice.Province, TEXT("edirne")));
    TestTrue(TEXT("No branch, no advice"), SuggestDepotProvince(S, TEXT("de")).Province.IsEmpty());

    // Building: in range of a branch, one per province, paid now.
    TestFalse(TEXT("Nothing near Van"), CanBuild(S, TEXT("tr"), TEXT("van"), Message));
    const int64 Cost = BuildCost(S, TEXT("tr"), Advice.Province);
    const int64 CashBefore = S.Cash;
    TestTrue(TEXT("Build the suggested depot"), Build(S, TEXT("tr"), Advice.Province, Message));
    TestEqual(TEXT("Paid"), CashBefore - S.Cash, Cost);
    TestTrue(TEXT("Recorded"), HasDepotIn(S, TEXT("tr"), Advice.Province) && Count(S) == 1 && MarketCompany::DepotCount(S) == 1);
    TestFalse(TEXT("One per province"), Build(S, TEXT("tr"), Advice.Province, Message));
    TestTrue(TEXT("Rent in the head office"), DailyRent(S, S.Day) > 0 && MonthlyRent(S, TEXT("tr"), Advice.Province) > 0);
    TestTrue(TEXT("The sub-region has a depot"), MarketCompany::HasDepot(S, TEXT("tr"), MarketCountry::FindCity(TEXT("tr"), Advice.Province)->SubRegion));
    TestFalse(TEXT("The advice skips it now"), SuggestDepotProvince(S, TEXT("tr")).Province == Advice.Province);
    TestFalse(TEXT("A depot line"), Describe(S, 0).IsEmpty());

    // The menu command: a second depot in Bursa.
    TestTrue(TEXT("BuildDepotIn"), MarketDirector::Command(S, TArray<FMarketProduct>(), FName(TEXT("BuildDepotIn")),
        MarketManagers::EncodeArea(MarketManagers::ELevel::Depot, TEXT("tr"), TEXT("bursa")), Message));
    TestEqual(TEXT("Two depots"), Count(S), 2);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketDepotsOldSaveTest, "MirasMarket.Depots.OldSaves", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketDepotsOldSaveTest::RunTest(const FString& Parameters)
{
    using namespace MarketDepots;
    using namespace MarketDepotsTest;
    // A G-086 save: a Trakya depot; our shops: the family shop in Kirklareli, two branches in Tekirdag.
    FMarketState S = MakeState();
    AddShop(S, TEXT("tekirdag"));
    AddShop(S, TEXT("tekirdag"));
    S.Company.Depots.Add(TEXT("tr:trakya"));
    TestTrue(TEXT("Counts before moving"), Count(S) == 1 && MarketCompany::HasDepot(S, TEXT("tr"), TEXT("trakya")));
    S.DayNews.Reset();
    Migrate(S);
    TestEqual(TEXT("One depot"), S.Company.DepotSites.Num(), 1);
    TestEqual(TEXT("Where most of our shops are"), S.Company.DepotSites[0].Province, FString(TEXT("tekirdag")));
    TestEqual(TEXT("Old keys gone"), S.Company.Depots.Num(), 0);
    TestEqual(TEXT("Without a manager"), ManagerOf(S, 0), static_cast<int32>(INDEX_NONE));
    TestTrue(TEXT("Told once"), NewsHas(S, TEXT("m\u00fcd\u00fcr ata")));
    const int32 Lines = S.DayNews.Num();
    Migrate(S);
    TestTrue(TEXT("Only once"), S.Company.DepotSites.Num() == 1 && S.DayNews.Num() == Lines);

    // A sub-region without our shops: its most populous province.
    FMarketState Empty = MakeState();
    Empty.Company.Depots.Add(TEXT("tr:dicle"));
    Migrate(Empty);
    TestTrue(TEXT("Moved inside the sub-region"), Empty.Company.DepotSites.Num() == 1
        && MarketCountry::FindCity(TEXT("tr"), Empty.Company.DepotSites[0].Province) != nullptr
        && MarketCountry::FindCity(TEXT("tr"), Empty.Company.DepotSites[0].Province)->SubRegion == TEXT("dicle"));
    return true;
}

#endif

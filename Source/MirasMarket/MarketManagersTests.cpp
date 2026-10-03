#include "MarketStaff.h"
#include "MarketManagers.h"
#include "MarketBranches.h"
#include "MarketDirector.h"
#include "MarketPrices.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketManagersTest
{
    FMarketState MakeState()
    {
        FMarketState S;
        S.CountryId = TEXT("tr");
        S.CityId = TEXT("kirklareli");
        S.Day = 100;
        S.RivalSeed = 7;
        S.Cash = 10000000;
        S.InheritedDebt = 0;
        return S;
    }

    // An open branch with a settled manager (morale 60 = no morale effect, no settling-in week).
    int32 AddShop(FMarketState& S, const TCHAR* Province, int32 Skill = 60, int32 Honesty = 80, const TCHAR* Country = TEXT("tr"))
    {
        FMarketBranch B;
        B.Country = Country;
        B.Province = Province;
        B.Format = TEXT("mahalle");
        B.Name = FString::Printf(TEXT("%s %d"), Province, S.Branches.Num() + 1);
        B.Stage = static_cast<uint8>(MarketBranches::EStage::Open);
        B.OpenedDay = 1;
        B.Rent = 60000;
        B.ManagerName = TEXT("Test M\u00fcd\u00fcr");
        B.ManagerSkill = Skill;
        B.ManagerHonesty = Honesty;
        B.ManagerWage = 3000;
        B.ManagerMorale = 60.f;
        B.ManagerStyle = static_cast<uint8>(MarketManagers::EStyle::Careful);
        B.ManagerSince = 0;
        return S.Branches.Add(B);
    }

    // A small seeded roll for the growth checks (the game's own rolls are internal).
    uint32 NextRoll(uint32& Seed)
    {
        Seed = Seed * 1664525u + 1013904223u;
        return Seed >> 8;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketManagersDirectTest, "MirasMarket.Managers.DirectReports", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketManagersDirectTest::RunTest(const FString& Parameters)
{
    using namespace MarketManagers;
    using namespace MarketManagersTest;
    FMarketState S = MakeState();
    FString Message;

    TestEqual(TEXT("The family shop does not count without a manager"), DirectCount(S), 0);
    AddShop(S, TEXT("tekirdag"));
    AddShop(S, TEXT("tekirdag"));
    TestFalse(TEXT("Two shops are not enough for a province manager"), CanAppoint(S, ELevel::Province, TEXT("tr"), TEXT("tekirdag"), INDEX_NONE, Message));
    AddShop(S, TEXT("tekirdag"));
    TestEqual(TEXT("Three store managers report to the player"), DirectCount(S), 3);
    TestEqual(TEXT("Three shops in Tekirdag"), ProvinceBranches(S, TEXT("tr"), TEXT("tekirdag")), 3);
    TestTrue(TEXT("Province manager possible"), CanAppoint(S, ELevel::Province, TEXT("tr"), TEXT("tekirdag"), INDEX_NONE, Message));
    TestTrue(TEXT("Suggested"), NextStep(S).Contains(TEXT("il m\u00fcd\u00fcr\u00fc")));
    const int32 E1 = AddShop(S, TEXT("edirne"));
    AddShop(S, TEXT("edirne"));
    TestEqual(TEXT("Five people: at the limit"), DirectCount(S), 5);
    TestEqual(TEXT("No penalty at the limit"), SpanPenalty(S), 0);

    TestTrue(TEXT("Appoint from outside"), Appoint(S, ELevel::Province, TEXT("tr"), TEXT("tekirdag"), INDEX_NONE, Message));
    TestEqual(TEXT("One manager"), S.Management.Managers.Num(), 1);
    TestEqual(TEXT("The province manager replaces three people"), DirectCount(S), 3);
    TestFalse(TEXT("Tekirdag's shops answer to him"), ReportsToPlayer(S, 0));
    TestTrue(TEXT("Edirne's shops still answer to the player"), ReportsToPlayer(S, E1));
    TestFalse(TEXT("One per province"), CanAppoint(S, ELevel::Province, TEXT("tr"), TEXT("tekirdag"), INDEX_NONE, Message));
    TestEqual(TEXT("Needed skill 40 + shops / 3"), RequiredSkill(S, 0), 41);
    TestTrue(TEXT("Wage from the province band"), DailyWage(S, S.Management.Managers[0]) >= MarketPrices::WageScaled(6000, S.Day));

    // Oversight: a good honest province manager at full strength, a dishonest one at half.
    S.Management.Managers[0].Skill = 90;
    S.Management.Managers[0].Morale = 60.f;
    S.Management.Managers[0].Honesty = 80; // the outside candidate's honesty is seeded; fix it for the check
    TestTrue(TEXT("Full oversight"), FMath::IsNearlyEqual(Oversight(S, 0), 1.f));
    TestEqual(TEXT("Opening 2 days shorter"), OpeningDaysSavedIn(S, TEXT("tr"), TEXT("tekirdag")), 2);
    const FBranchRule Watched = RuleFor(S, 0);
    TestEqual(TEXT("Oversight lifts the effective skill"), Watched.Skill, 70);
    TestTrue(TEXT("Fewer order errors"), Watched.ErrorFactor < 0.7f);
    TestTrue(TEXT("Store skill visible with a province manager"), SkillVisible(S, 0));
    S.Management.Managers[0].Honesty = 20;
    TestEqual(TEXT("A dishonest one saves only a day"), OpeningDaysSavedIn(S, TEXT("tr"), TEXT("tekirdag")), 1);
    S.Management.Managers[0].Honesty = 80;

    // Promotion: Edirne needs three shops; the branch then gets a new manager with a settling-in week.
    TestFalse(TEXT("Edirne has two shops"), PromoteToProvince(S, E1, Message));
    AddShop(S, TEXT("edirne"));
    TestFalse(TEXT("Sub-region needs two province managers"), CanAppoint(S, ELevel::SubRegion, TEXT("tr"), TEXT("trakya"), INDEX_NONE, Message));
    TestTrue(TEXT("Promote to province manager"), PromoteToProvince(S, E1, Message));
    TestEqual(TEXT("Two province managers"), S.Management.Managers.Num(), 2);
    TestEqual(TEXT("The promoted one"), S.Management.Managers[1].Name, FString(TEXT("Test M\u00fcd\u00fcr")));
    TestTrue(TEXT("A new manager in the branch"), S.Branches[E1].ManagerName != TEXT("Test M\u00fcd\u00fcr") && S.Branches[E1].ManagerSince == S.Day);
    TestTrue(TEXT("The newcomer is settling in"), RuleFor(S, E1).Skill < S.Branches[E1].ManagerSkill + 10);
    TestEqual(TEXT("Two people report to the player"), DirectCount(S), 2);

    // Sub-region manager over the two province managers; a director needs two sub-region managers.
    TestTrue(TEXT("Trakya sub-region manager possible"), CanAppoint(S, ELevel::SubRegion, TEXT("tr"), TEXT("trakya"), INDEX_NONE, Message));
    TestTrue(TEXT("Appoint the sub-region manager"), Appoint(S, ELevel::SubRegion, TEXT("tr"), TEXT("trakya"), INDEX_NONE, Message));
    TestEqual(TEXT("Only the sub-region manager reports to the player"), DirectCount(S), 1);
    TestFalse(TEXT("Director needs two sub-region managers"), CanAppoint(S, ELevel::Region, TEXT("tr"), TEXT("marmara"), INDEX_NONE, Message));
    // The family shop gets a manager: in Kirklareli (Trakya) he answers to the sub-region manager.
    TestTrue(TEXT("Family shop manager"), Appoint(S, ELevel::FamilyShop, TEXT("tr"), FString(), INDEX_NONE, Message));
    TestEqual(TEXT("Still one direct report"), DirectCount(S), 1);

    // Dismissing the sub-region manager: the province managers answer to the player again.
    const int32 Sub = FindManager(S, ELevel::SubRegion, TEXT("tr"), TEXT("trakya"));
    const int64 Costs = S.OtherCosts;
    TestTrue(TEXT("Dismiss"), Dismiss(S, Sub, Message));
    TestTrue(TEXT("Severance"), S.OtherCosts > Costs);
    TestEqual(TEXT("Back to the player"), DirectCount(S), 3);

    // Menu arguments.
    ELevel Level = ELevel::Store;
    FString Country, Area;
    TestTrue(TEXT("Area round trip"), DecodeArea(EncodeArea(ELevel::SubRegion, TEXT("tr"), TEXT("trakya")), Level, Country, Area) && Level == ELevel::SubRegion && Country == TEXT("tr") && Area == TEXT("trakya"));
    TestTrue(TEXT("Province round trip"), DecodeArea(EncodeArea(ELevel::Province, TEXT("tr"), TEXT("tekirdag")), Level, Country, Area) && Level == ELevel::Province && Area == TEXT("tekirdag"));
    TestFalse(TEXT("Describe"), DescribeManager(S, 0).IsEmpty() || DescribeBranchManager(S, 0).IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketManagersSpanTest, "MirasMarket.Managers.SpanLimit", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketManagersSpanTest::RunTest(const FString& Parameters)
{
    using namespace MarketManagers;
    using namespace MarketManagersTest;
    FMarketState S = MakeState();
    FString Message;
    const TCHAR* Provinces[] = { TEXT("tekirdag"), TEXT("edirne"), TEXT("istanbul"), TEXT("bursa"), TEXT("izmir"), TEXT("ankara"), TEXT("kocaeli"),
        TEXT("antalya"), TEXT("adana"), TEXT("konya"), TEXT("kayseri"), TEXT("samsun"), TEXT("trabzon"), TEXT("erzurum"), TEXT("van"),
        TEXT("gaziantep"), TEXT("diyarbakir"), TEXT("mersin"), TEXT("eskisehir"), TEXT("denizli") };
    AddShop(S, Provinces[0], 60, 20); // a dishonest one
    for (int32 I = 1; I < 5; ++I) AddShop(S, Provinces[I]);
    TestEqual(TEXT("At the limit"), DirectCount(S), 5);
    TestFalse(TEXT("The till is watched at the limit"), RuleFor(S, 0).bSkimHidden);
    TestEqual(TEXT("Full skill at the limit"), RuleFor(S, 1).Skill, 60);
    AddShop(S, Provinces[5]);
    AddShop(S, Provinces[6]);
    TestEqual(TEXT("Seven direct reports"), DirectCount(S), 7);
    TestEqual(TEXT("Two over"), OverLimit(S), 2);
    TestEqual(TEXT("-4 a person"), SpanPenalty(S), 8);
    TestEqual(TEXT("Effective skill falls"), RuleFor(S, 1).Skill, 52);
    const FBranchRule Thief = RuleFor(S, 0);
    TestTrue(TEXT("Skimming"), Thief.SkimPermille > 0);
    TestTrue(TEXT("Nobody sees the till"), Thief.bSkimHidden);
    TestTrue(TEXT("Span suggestion"), NextStep(S).Contains(TEXT("s\u0131n\u0131r")));

    const float Before = S.Branches[1].Satisfaction;
    S.DayNews.Reset();
    MarketManagers::CloseDay(S);
    TestTrue(TEXT("Satisfaction falls slowly"), S.Branches[1].Satisfaction < Before && S.Branches[1].Satisfaction > Before - 1.f);

    // A warning keeps a dishonest manager's hands off the till for a while.
    TestTrue(TEXT("Warn"), Warn(S, 0, Message));
    TestEqual(TEXT("Deterred"), RuleFor(S, 0).SkimPermille, 0);

    for (int32 I = 7; I < 20; ++I) AddShop(S, Provinces[I]);
    TestEqual(TEXT("Twenty direct reports"), DirectCount(S), 20);
    TestEqual(TEXT("Capped at 25"), SpanPenalty(S), SpanPenaltyMax);
    // A country manager takes the whole network.
    TestTrue(TEXT("Country manager optional in one country"), Appoint(S, ELevel::Country, TEXT("tr"), FString(), INDEX_NONE, Message));
    TestEqual(TEXT("One person"), DirectCount(S), 1);
    TestEqual(TEXT("No penalty"), SpanPenalty(S), 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketManagersCountryTest, "MirasMarket.Managers.CountryManager", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketManagersCountryTest::RunTest(const FString& Parameters)
{
    using namespace MarketManagers;
    using namespace MarketManagersTest;
    FMarketState S = MakeState();
    FString Message;
    AddShop(S, TEXT("tekirdag"));
    TestFalse(TEXT("One country: not required"), CountryManagerRequired(S));
    const int32 Alone = RuleFor(S, 0).Skill;
    TestFalse(TEXT("No shop in Germany"), CanAppoint(S, ELevel::Country, TEXT("de"), FString(), INDEX_NONE, Message));
    AddShop(S, TEXT("by"), 60, 80, TEXT("de"));
    TestTrue(TEXT("Two countries: required"), CountryManagerRequired(S));
    TestEqual(TEXT("Both countries miss one"), CountriesMissingManager(S).Num(), 2);
    TestEqual(TEXT("Penalty while missing"), RuleFor(S, 0).Skill, Alone - MissingCountryPenalty);
    TestTrue(TEXT("Urgent suggestion"), NextStep(S).Contains(TEXT("\u00fclke m\u00fcd\u00fcr\u00fc")));
    S.DayNews.Reset();
    MarketManagers::CloseDay(S);
    TestTrue(TEXT("Warning in the news"), S.DayNews.ContainsByPredicate([](const FString& Line) { return Line.Contains(TEXT("\u00fclke m\u00fcd\u00fcr\u00fc")); }));
    TestEqual(TEXT("Counted"), S.Management.MissingCountryDays, 1);

    TestTrue(TEXT("Turkey's country manager"), Appoint(S, ELevel::Country, TEXT("tr"), FString(), INDEX_NONE, Message));
    TestTrue(TEXT("Germany's country manager"), Appoint(S, ELevel::Country, TEXT("de"), FString(), INDEX_NONE, Message));
    TestEqual(TEXT("None missing"), CountriesMissingManager(S).Num(), 0);
    TestEqual(TEXT("Two country managers report to the player"), DirectCount(S), 2);
    TestTrue(TEXT("Abroad the country manager lowers the cost"), CostAdjust(S, S.Branches[1]) <= 0.f);

    // Wages are paid at the day close from the till and count in the day's result.
    const int64 Wages = DailyWages(S);
    TestTrue(TEXT("Country managers are expensive"), Wages >= 2 * MarketPrices::WageScaled(FMath::RoundToInt64(12500 * CountryMinScale), S.Day)); // C11 (M40): a few shops, a part of the band
    const int64 Cash = S.Cash;
    const int64 Result = S.LastBranchProfit;
    ++S.Day;
    MarketManagers::CloseDay(S);
    const int64 Paid = DailyWages(S) + MarketStaff::EmployerShare(DailyWages(S)); // C3 (B3): with the employer's social security
    TestEqual(TEXT("Paid from the till"), Cash - S.Cash, Paid);
    TestEqual(TEXT("In the day's result"), Result - S.LastBranchProfit, Paid);
    TestEqual(TEXT("Recorded"), S.Management.LastWages, DailyWages(S));
    TestEqual(TEXT("No longer missing"), S.Management.MissingCountryDays, 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketManagersDecisionsTest, "MirasMarket.Managers.Decisions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketManagersDecisionsTest::RunTest(const FString& Parameters)
{
    using namespace MarketManagers;
    using namespace MarketManagersTest;
    FMarketState S = MakeState();
    FString Message;
    AddShop(S, TEXT("tekirdag"));
    FMarketBranch& B = S.Branches[0];

    // Bonus: a week's wage (paid at the close with the other costs), morale and a little skill.
    TestTrue(TEXT("Bonus"), Bonus(S, 0, Message));
    TestEqual(TEXT("A week's wage"), S.OtherCosts, static_cast<int64>(3000 * BonusDays));
    TestTrue(TEXT("Morale up"), FMath::IsNearlyEqual(B.ManagerMorale, 75.f));
    TestEqual(TEXT("Skill up"), B.ManagerSkill, 61);
    TestFalse(TEXT("Not twice in two weeks"), Bonus(S, 0, Message));

    // Warn a manager with a good mark: he finds it unfair.
    TestTrue(TEXT("Warn"), Warn(S, 0, Message));
    TestEqual(TEXT("One warning"), B.ManagerWarnings, 1);
    TestTrue(TEXT("Morale down"), B.ManagerMorale < 75.f);
    TestFalse(TEXT("Not twice in a week"), Warn(S, 0, Message));

    // Replace: severance to the old one, a seeded outside manager with a settling-in week.
    const int64 Costs = S.OtherCosts;
    TestTrue(TEXT("Replace"), Replace(S, 0, Message));
    TestEqual(TEXT("Severance"), S.OtherCosts - Costs, static_cast<int64>(3000 * SeveranceDays));
    TestTrue(TEXT("Someone new"), B.ManagerName != TEXT("Test M\u00fcd\u00fcr") && !B.ManagerName.IsEmpty());
    TestEqual(TEXT("Starts today"), B.ManagerSince, S.Day);
    TestEqual(TEXT("Clean record"), B.ManagerWarnings, 0);
    TestTrue(TEXT("Fresh morale"), FMath::IsNearlyEqual(B.ManagerMorale, 70.f));
    TestTrue(TEXT("A style"), B.ManagerStyle >= 1 && B.ManagerStyle <= 3);
    TestTrue(TEXT("Settling in"), RuleFor(S, 0).Skill <= FMath::Max(0, B.ManagerSkill + 3 - SettlePenalty));

    // No money, no bonus.
    FMarketState Poor = S;
    Poor.Cash = 0;
    Poor.Branches[0].ManagerBonusDay = 0;
    TestFalse(TEXT("No money"), Bonus(Poor, 0, Message));

    // Styles: generous orders more and wastes more; price-minded follows the rivals under the target.
    B.ManagerStyle = static_cast<uint8>(EStyle::Generous);
    const FBranchRule Generous = RuleFor(S, 0);
    B.ManagerStyle = static_cast<uint8>(EStyle::Careful);
    const FBranchRule Careful = RuleFor(S, 0);
    TestTrue(TEXT("Generous orders more"), Generous.OrderFactor > Careful.OrderFactor && Generous.WasteRate > Careful.WasteRate);
    B.ManagerStyle = static_cast<uint8>(EStyle::PriceMinded);
    TestTrue(TEXT("Price-minded follows the rivals"), RuleFor(S, 0).bFollowsRivals && RuleFor(S, 0).PriceBias < 0.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketManagersFamilyShopTest, "MirasMarket.Managers.FamilyShopManager", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketManagersFamilyShopTest::RunTest(const FString& Parameters)
{
    using namespace MarketManagers;
    using namespace MarketManagersTest;
    FMarketState S = MakeState();
    FString Message;
    // M19: not before a second shop is open.
    TestFalse(TEXT("No branch yet"), CanAppoint(S, ELevel::FamilyShop, TEXT("tr"), FString(), INDEX_NONE, Message));
    TestTrue(TEXT("Reason"), Message.Contains(TEXT("ikinci")));
    TestFalse(TEXT("Hidden"), IsTierVisible(S, ELevel::FamilyShop, TEXT("tr"), FString()));
    const int32 Shop = AddShop(S, TEXT("tekirdag"));
    S.Branches[Shop].Stage = static_cast<uint8>(MarketBranches::EStage::Renovation);
    TestFalse(TEXT("A branch still being built does not count"), CanAppoint(S, ELevel::FamilyShop, TEXT("tr"), FString(), INDEX_NONE, Message));
    S.Branches[Shop].Stage = static_cast<uint8>(MarketBranches::EStage::Open);
    TestTrue(TEXT("Open branch: possible"), CanAppoint(S, ELevel::FamilyShop, TEXT("tr"), FString(), INDEX_NONE, Message));
    TestTrue(TEXT("Visible"), IsTierVisible(S, ELevel::FamilyShop, TEXT("tr"), FString()));
    TestFalse(TEXT("Without a manager the family's routine"), FamilyRule(S).bManaged);
    TArray<int32> Plain = { 10, 4, 0, 7 };
    const TArray<int32> Before = Plain;
    ShapeFamilyOrder(S, FamilyRule(S), Plain);
    TestTrue(TEXT("Order unchanged without a manager"), Plain == Before);

    const TArray<FCandidate> Pool = Candidates(S, ELevel::FamilyShop, TEXT("tr"), FString());
    TestEqual(TEXT("Three candidates"), Pool.Num(), CandidateCount);
    TestEqual(TEXT("Direct: one store manager"), DirectCount(S), 1);
    TestTrue(TEXT("Appoint candidate 2"), AppointCandidate(S, ELevel::FamilyShop, TEXT("tr"), FString(), 2, Message));
    const int32 Index = FindManager(S, ELevel::FamilyShop, TEXT("tr"), FString());
    TestTrue(TEXT("Appointed"), Index != INDEX_NONE && S.Management.Managers[Index].Name == Pool[2].Name);
    TestEqual(TEXT("Counts as one of the player's people"), DirectCount(S), 2);
    TestTrue(TEXT("Paid"), DailyWages(S) > 0);

    // His style and skill run the simulated day.
    FMarketManager& M = S.Management.Managers[Index];
    M.Skill = 80;
    M.Morale = 60.f;
    const int32 Settling = FamilyRule(S).Skill;
    S.Day += SettleDays;
    TestTrue(TEXT("The first week is a settling-in week"), FamilyRule(S).Skill >= Settling + SettlePenalty);
    M.Style = static_cast<uint8>(EStyle::Careful);
    const FFamilyRule Careful = FamilyRule(S);
    M.Style = static_cast<uint8>(EStyle::Generous);
    const FFamilyRule Generous = FamilyRule(S);
    M.Style = static_cast<uint8>(EStyle::PriceMinded);
    const FFamilyRule PriceMinded = FamilyRule(S);
    TestTrue(TEXT("Managed"), Careful.bManaged);
    TestTrue(TEXT("Careful orders less, generous more"), Careful.OrderFactor < 1.f && Generous.OrderFactor > 1.f);
    TestTrue(TEXT("Price-minded holds the old prices longer"), PriceMinded.PriceRiseGap > Careful.PriceRiseGap);
    TestTrue(TEXT("A skilled one refills often"), Careful.RefillEvery <= 8 && Careful.ForgetPermille == 0);
    TArray<int32> Small = { 20, 20, 20, 20 };
    TArray<int32> Big = Small;
    ShapeFamilyOrder(S, Careful, Small);
    ShapeFamilyOrder(S, Generous, Big);
    int32 SmallSum = 0, BigSum = 0;
    for (int32 I = 0; I < Small.Num(); ++I) { SmallSum += Small[I]; BigSum += Big[I]; }
    TestTrue(TEXT("Styles shape the order"), SmallSum < 80 && BigSum > 80);
    M.Skill = 20;
    const FFamilyRule Weak = FamilyRule(S);
    TestTrue(TEXT("A weak one refills late and forgets lines"), Weak.RefillEvery > 8 && Weak.ForgetPermille > 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketManagersCountryVisibleTest, "MirasMarket.Managers.CountryAfterFiveProvinces", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketManagersCountryVisibleTest::RunTest(const FString& Parameters)
{
    using namespace MarketManagers;
    using namespace MarketManagersTest;
    FMarketState S = MakeState();
    FString Message;
    AddShop(S, TEXT("kirklareli")); // the family shop's province: no new province
    AddShop(S, TEXT("tekirdag"));
    AddShop(S, TEXT("edirne"));
    AddShop(S, TEXT("istanbul"));
    const int32 Building = AddShop(S, TEXT("bursa"));
    S.Branches[Building].Stage = static_cast<uint8>(MarketBranches::EStage::Permits);
    TestEqual(TEXT("Four provinces with the family shop's"), ProvincesWithShops(S, TEXT("tr")), 4);
    TestFalse(TEXT("Four provinces: not yet"), CanAppoint(S, ELevel::Country, TEXT("tr"), FString(), INDEX_NONE, Message));
    TestTrue(TEXT("Reason names the five provinces"), Message.Contains(TEXT("5")));
    TestFalse(TEXT("Hidden in the tree"), IsTierVisible(S, ELevel::Country, TEXT("tr"), TEXT("tr")));
    TestFalse(TEXT("Not among the tiers"), VisibleTiers(S, TEXT("tr")).Contains(ELevel::Country));
    TestFalse(TEXT("No suggestion"), Suggestions(S).ContainsByPredicate([](const FString& Line) { return Line.Contains(TEXT("\u00fclke m\u00fcd\u00fcr\u00fc")); }));
    // Five store managers (one still in permits): at the limit the hint names only a province manager.
    TestEqual(TEXT("At the limit"), DirectCount(S), SpanLimit);
    TestTrue(TEXT("Limit hint"), Suggestions(S).ContainsByPredicate([](const FString& Line) { return Line.Contains(TEXT("S\u0131n\u0131rdas\u0131n")); }));
    S.Branches[Building].Stage = static_cast<uint8>(MarketBranches::EStage::Open);
    TestEqual(TEXT("Five provinces"), ProvincesWithShops(S, TEXT("tr")), 5);
    TestTrue(TEXT("Five provinces: possible"), CanAppoint(S, ELevel::Country, TEXT("tr"), FString(), INDEX_NONE, Message));
    TestTrue(TEXT("Visible"), IsTierVisible(S, ELevel::Country, TEXT("tr"), TEXT("tr")) && VisibleTiers(S, TEXT("tr")).Contains(ELevel::Country));
    TestTrue(TEXT("Suggested"), Suggestions(S).ContainsByPredicate([](const FString& Line) { return Line.Contains(TEXT("\u00fclke m\u00fcd\u00fcr\u00fc")); }));
    // C11 (M40): a country manager of a few shops asks for a part of his band; it grows with the network.
    TestEqual(TEXT("Five branches and the family shop"), ShopsInCountry(S, TEXT("tr")), 6);
    TestTrue(TEXT("Small chain, smaller pay"), FMath::IsNearlyEqual(CountryWageScale(S, TEXT("tr")), CountryMinScale));
    const TArray<FCandidate> Small = Candidates(S, ELevel::Country, TEXT("tr"), TEXT("tr"));
    TestTrue(TEXT("A third of the band"), Small.Num() > 0 && Small[0].BaseWage <= BaseWageFor(ELevel::Country, Small[0].Skill, TEXT("tr")) * 0.35);
    TestTrue(TEXT("Appointed"), Appoint(S, ELevel::Country, TEXT("tr"), FString(), INDEX_NONE, Message));
    const int32 Head = FindManager(S, ELevel::Country, TEXT("tr"), TEXT("tr"));
    const int64 Before = S.Management.Managers[Head].BaseWage;
    const TCHAR* More[] = { TEXT("izmir"), TEXT("ankara"), TEXT("antalya"), TEXT("adana"), TEXT("konya"), TEXT("kayseri"), TEXT("samsun"), TEXT("trabzon"), TEXT("mersin"), TEXT("denizli") };
    for (const TCHAR* P : More) AddShop(S, P);
    for (const TCHAR* P : More) AddShop(S, P);
    S.Day = 7 * 30 + 1; // a week's close
    MarketManagers::CloseDay(S);
    TestTrue(TEXT("Pay grew with the network"), S.Management.Managers[Head].BaseWage > Before);

    // In a second country the country manager is required at once, without five provinces.
    FMarketState Two = MakeState();
    AddShop(Two, TEXT("tekirdag"));
    AddShop(Two, TEXT("by"), 60, 80, TEXT("de"));
    TestTrue(TEXT("Required"), CountryManagerRequired(Two));
    TestTrue(TEXT("Turkey with two provinces"), CanAppoint(Two, ELevel::Country, TEXT("tr"), FString(), INDEX_NONE, Message));
    TestTrue(TEXT("Germany with one province"), CanAppoint(Two, ELevel::Country, TEXT("de"), FString(), INDEX_NONE, Message));
    TestTrue(TEXT("Visible when required"), VisibleTiers(Two, TEXT("de")).Contains(ELevel::Country));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketManagersCeilingTest, "MirasMarket.Managers.SkillCeiling", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketManagersCeilingTest::RunTest(const FString& Parameters)
{
    using namespace MarketManagers;
    using namespace MarketManagersTest;
    // The chance shrinks near the ceiling, is 0 there, and 95 is the top for everyone.
    TestTrue(TEXT("Far from the ceiling: faster"), GrowthChance(50, 80, 0) > GrowthChance(75, 80, 0));
    TestTrue(TEXT("Near the ceiling: still possible"), GrowthChance(79, 80, 0) > 0.f);
    TestEqual(TEXT("At the ceiling: none"), GrowthChance(80, 80, 52), 0.f);
    TestEqual(TEXT("Top 95"), GrowthChance(95, 99, 52), 0.f);
    TestTrue(TEXT("Seniority helps a little"), GrowthChance(60, 80, 52) > GrowthChance(60, 80, 0) && GrowthChance(60, 80, 52) <= 1.5f * GrowthChance(60, 80, 0) + 0.001f);

    // Growth week by week: never past the ceiling; the first 10 points (50 -> 60) come faster than the last 10.
    uint32 Seed = 12345u;
    int32 Skill = 50;
    int32 Reached60 = INDEX_NONE, WeeksLast = INDEX_NONE;
    for (int32 Week = 1; Week <= 1500; ++Week)
    {
        Skill = GrowSkill(Skill, 70, Week, NextRoll(Seed));
        TestTrue(TEXT("Never past the ceiling"), Skill <= 70);
        if (Skill == 60 && Reached60 == INDEX_NONE) Reached60 = Week;
        if (Skill == 70 && WeeksLast == INDEX_NONE) { WeeksLast = Week - Reached60; break; }
    }
    TestTrue(TEXT("Reached the ceiling in the end"), WeeksLast != INDEX_NONE);
    TestTrue(TEXT("Slower near the ceiling"), Reached60 != INDEX_NONE && WeeksLast > Reached60);

    // A bonus never passes the ceiling either.
    FMarketState S = MakeState();
    FString Message;
    AddShop(S, TEXT("tekirdag"), 70);
    S.Branches[0].ManagerPotential = 70;
    TestTrue(TEXT("Bonus paid"), Bonus(S, 0, Message));
    TestEqual(TEXT("Store manager at the ceiling stays"), S.Branches[0].ManagerSkill, 70);
    FMarketManager Head;
    Head.Level = static_cast<uint8>(ELevel::Country);
    Head.Country = TEXT("tr");
    Head.Area = TEXT("tr");
    Head.Name = TEXT("Tavan Deneme");
    Head.Skill = 88;
    Head.Potential = 88;
    Head.Style = 1;
    Head.Morale = 90.f;
    Head.BaseWage = 25000;
    Head.AppointedDay = 1;
    S.Management.Managers.Add(Head);
    TestTrue(TEXT("Manager bonus paid"), BonusManager(S, 0, Message));
    TestEqual(TEXT("Manager at the ceiling stays"), S.Management.Managers[0].Skill, 88);
    // A year of good weeks at the day close: the country manager stays at his ceiling.
    S.Day = 106; // day 105 closes a week
    for (int32 Week = 0; Week < 52; ++Week)
    {
        S.Management.Managers[0].Morale = 90.f;
        MarketManagers::CloseDay(S);
        S.Day += 7;
    }
    TestEqual(TEXT("A year later still at the ceiling"), S.Management.Managers[0].Skill, 88);
    S.Management.Managers[0].Potential = 95;
    S.Management.Managers[0].Skill = 70;
    for (int32 Week = 0; Week < 104; ++Week)
    {
        S.Management.Managers[0].Morale = 90.f;
        MarketManagers::CloseDay(S);
        S.Day += 7;
    }
    TestTrue(TEXT("Room to grow: grows"), S.Management.Managers[0].Skill > 70 && S.Management.Managers[0].Skill <= 95);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketManagersCandidatesTest, "MirasMarket.Managers.Candidates", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketManagersCandidatesTest::RunTest(const FString& Parameters)
{
    using namespace MarketManagers;
    using namespace MarketManagersTest;
    FMarketState S = MakeState();
    S.Cash = 1000000000000ll;
    FString Message;
    AddShop(S, TEXT("tekirdag"));

    // Three candidates for every kind of appointment, each with a ceiling 55..95 and a wage.
    const ELevel Levels[7] = { ELevel::Store, ELevel::Province, ELevel::SubRegion, ELevel::Region, ELevel::Country, ELevel::Depot, ELevel::FamilyShop };
    const TCHAR* Areas[7] = { TEXT("tekirdag"), TEXT("tekirdag"), TEXT("trakya"), TEXT("marmara"), TEXT("tr"), TEXT("tekirdag"), TEXT("") };
    for (int32 L = 0; L < 7; ++L)
    {
        const TArray<FCandidate> Pool = Candidates(S, Levels[L], TEXT("tr"), Areas[L]);
        TestEqual(TEXT("Three candidates"), Pool.Num(), CandidateCount);
        TSet<FString> Names;
        for (const FCandidate& Who : Pool)
        {
            Names.Add(Who.Name);
            TestTrue(TEXT("A name"), !Who.Name.IsEmpty());
            TestTrue(TEXT("Ceiling 55..95, not under the skill"), Who.Potential >= PotentialMin && Who.Potential <= SkillTop && Who.Potential >= Who.Skill);
            TestTrue(TEXT("A style"), Who.Style >= 1 && Who.Style <= 3);
            TestTrue(TEXT("A wage"), Who.Wage > 0);
            int32 Low = 0, High = 0;
            SkillRange(S, Who, Low, High);
            TestTrue(TEXT("Without HR a range around the skill"), Low <= Who.Skill && Who.Skill <= High && High - Low <= 2 * CandidateRangeHalf && High > Low);
            TestFalse(TEXT("Line"), DescribeCandidate(S, Who).IsEmpty());
        }
        TestEqual(TEXT("Names unique in the pool"), Names.Num(), CandidateCount);
    }
    TestFalse(TEXT("Honesty hidden without HR"), HonestyVisible(S));
    TestTrue(TEXT("Wage asks rise with skill"), BaseWageFor(ELevel::Province, 80, TEXT("tr")) > BaseWageFor(ELevel::Province, 45, TEXT("tr"))
        && BaseWageFor(ELevel::Country, 80, TEXT("tr")) > BaseWageFor(ELevel::Country, 45, TEXT("tr")));
    TestTrue(TEXT("Depot in the area arguments"), [&]
    {
        ELevel Level = ELevel::Store;
        FString Country, Area;
        return DecodeArea(EncodeArea(ELevel::Depot, TEXT("tr"), TEXT("tekirdag")), Level, Country, Area) && Level == ELevel::Depot && Area == TEXT("tekirdag");
    }());

    // The same week shows the same people (saving and loading changes nothing); the next week others.
    const TArray<FCandidate> Monday = BranchCandidates(S, 0);
    const TArray<FCandidate> Again = BranchCandidates(S, 0);
    const int32 Week = CandidateWeek(S);
    FMarketState Later = S;
    while (CandidateWeek(Later) == Week && Later.Day < S.Day + 6) ++Later.Day;
    if (CandidateWeek(Later) == Week) TestTrue(TEXT("Same pool the whole week"), BranchCandidates(Later, 0)[1].Name == Monday[1].Name);
    bool bSame = true;
    for (int32 I = 0; I < CandidateCount; ++I) bSame = bSame && Again[I].Name == Monday[I].Name && Again[I].Skill == Monday[I].Skill;
    TestTrue(TEXT("Same week, same candidates"), bSame);
    FMarketState NextWeek = S;
    NextWeek.Day += 7;
    const TArray<FCandidate> Next = BranchCandidates(NextWeek, 0);
    bool bDifferent = false;
    for (int32 I = 0; I < CandidateCount; ++I) bDifferent = bDifferent || Next[I].Name != Monday[I].Name;
    TestTrue(TEXT("Next week, new candidates"), bDifferent);
    TestEqual(TEXT("The old single candidate is the first of the pool"), Candidate(S, ELevel::Store, TEXT("tr"), TEXT("tekirdag")).Name, Monday[0].Name);

    // Choosing: the chosen one comes with his skill, style and ceiling; nobody seen comes back.
    TestTrue(TEXT("Replace with candidate 1"), ReplaceWithCandidate(S, 0, 1, Message));
    TestEqual(TEXT("The chosen one"), S.Branches[0].ManagerName, Monday[1].Name);
    TestEqual(TEXT("His skill"), S.Branches[0].ManagerSkill, Monday[1].Skill);
    TestEqual(TEXT("His style"), S.Branches[0].ManagerStyle, Monday[1].Style);
    TestEqual(TEXT("His ceiling"), S.Branches[0].ManagerPotential, Monday[1].Potential);
    const TArray<FCandidate> After = BranchCandidates(S, 0);
    for (const FCandidate& Who : After)
        for (const FCandidate& Seen : Monday) TestTrue(TEXT("Seen candidates do not come back"), Who.Name != Seen.Name);
    TestFalse(TEXT("A branch with a manager is not a hire"), HireCandidateForBranch(S, 0, 0, Message));
    TestFalse(TEXT("No fourth candidate"), ReplaceWithCandidate(S, 0, CandidateCount, Message));

    // Many appointments: no name ever twice.
    TSet<FString> Hired;
    Hired.Add(S.Branches[0].ManagerName);
    for (int32 N = 0; N < 300; ++N)
    {
        if (!ReplaceWithCandidate(S, 0, N % CandidateCount, Message)) { AddError(Message); break; }
        const FString Name = S.Branches[0].ManagerName;
        TestFalse(TEXT("A new name every time"), Hired.Contains(Name));
        Hired.Add(Name);
    }
    TestEqual(TEXT("301 different managers"), Hired.Num(), 301);

    // Upper levels: the chosen candidate becomes the manager; the old commands take the first of the pool.
    AddShop(S, TEXT("tekirdag"));
    AddShop(S, TEXT("tekirdag"));
    const TArray<FCandidate> Province = Candidates(S, ELevel::Province, TEXT("tr"), TEXT("tekirdag"));
    const int32 AreaArg = EncodeArea(ELevel::Province, TEXT("tr"), TEXT("tekirdag"));
    const TArray<FMarketProduct> NoProducts;
    TestTrue(TEXT("Director command"), MarketDirector::Command(S, NoProducts, FName(TEXT("AppointCandidate")), AreaArg * 10 + 2, Message));
    const int32 Index = FindManager(S, ELevel::Province, TEXT("tr"), TEXT("tekirdag"));
    TestTrue(TEXT("Candidate 2 appointed"), Index != INDEX_NONE && S.Management.Managers[Index].Name == Province[2].Name
        && S.Management.Managers[Index].Potential == Province[2].Potential && S.Management.Managers[Index].Style == Province[2].Style);
    TestFalse(TEXT("A store manager is not appointed this way"), AppointCandidate(S, ELevel::Store, TEXT("tr"), TEXT("tekirdag"), 0, Message));
    const TArray<FCandidate> Edirne = Candidates(S, ELevel::Province, TEXT("tr"), TEXT("edirne"));
    AddShop(S, TEXT("edirne"));
    AddShop(S, TEXT("edirne"));
    const int32 Third = AddShop(S, TEXT("edirne"));
    S.Branches[Third].ManagerName.Reset();
    TestTrue(TEXT("Hire for a branch without a manager"), MarketDirector::Command(S, NoProducts, FName(TEXT("ManagerHireFor")), Third * 10 + 0, Message));
    TestFalse(TEXT("Hired"), S.Branches[Third].ManagerName.IsEmpty());
    const TArray<FCandidate> EdirneNow = Candidates(S, ELevel::Province, TEXT("tr"), TEXT("edirne"));
    TestTrue(TEXT("Old command: the first candidate"), Appoint(S, ELevel::Province, TEXT("tr"), TEXT("edirne"), INDEX_NONE, Message)
        && S.Management.Managers.Last().Name == EdirneNow[0].Name);
    TestTrue(TEXT("Hires move the pool on"), Edirne[0].Name != EdirneNow[0].Name || Edirne[1].Name != EdirneNow[1].Name);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketManagersNamesTest, "MirasMarket.Managers.NamesNeverReturn", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketManagersNamesTest::RunTest(const FString& Parameters)
{
    using namespace MarketManagers;
    using namespace MarketManagersTest;
    // M22: a name used once in the campaign (hired, resigned, dismissed) is never offered again.
    FMarketState S = MakeState();
    S.Day = 106; // day 105 closes a week
    FString Message;
    const FString Leaver = TEXT("Ayr\u0131lan M\u00fcd\u00fcr");
    const int32 Shop = AddShop(S, TEXT("tekirdag"));
    S.Branches[Shop].ManagerName = Leaver;
    S.Branches[Shop].ManagerPotential = 70;
    S.Management.UsedNames.AddUnique(Leaver); // E3a: a hire records the name (HireFrom, Promote); no daily migration

    // He resigns (no morale left): the branch has no manager, his name stays used.
    S.Branches[Shop].ManagerMorale = 5.f;
    MarketManagers::CloseDay(S);
    TestTrue(TEXT("He left"), S.Branches[Shop].ManagerName.IsEmpty() && S.Branches[Shop].ManagerPotential == 0);
    TestTrue(TEXT("Still a used name"), S.Management.UsedNames.Contains(Leaver));
    for (const FCandidate& Who : BranchCandidates(S, Shop)) TestTrue(TEXT("Not offered again"), Who.Name != Leaver);

    // A dismissed manager's name too.
    FMarketManager Old;
    Old.Level = static_cast<uint8>(ELevel::Province);
    Old.Country = TEXT("tr");
    Old.Area = TEXT("tekirdag");
    Old.Name = TEXT("G\u00f6revden Al\u0131nan");
    Old.Skill = 60;
    Old.Potential = 70;
    Old.Style = 1;
    Old.Morale = 60.f;
    Old.BaseWage = 6000;
    Old.AppointedDay = 1;
    const int32 OldIndex = S.Management.Managers.Add(Old);
    TestTrue(TEXT("Dismiss"), Dismiss(S, OldIndex, Message));
    TestTrue(TEXT("A dismissed name is used"), S.Management.UsedNames.Contains(Old.Name));
    return true;
}

#endif

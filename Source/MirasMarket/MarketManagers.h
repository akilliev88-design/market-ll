#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Management hierarchy (G-086b, Docs/Kurgu/03_MAGAZA_AGI.md \u00a74). Independent of the world, tested
// (MirasMarket.Managers.*). Every branch has a store manager (FMarketBranch: hidden skill, honesty and style,
// morale, warnings, a settling-in week). Above them come, when the network needs them:
//  - a province manager (a province with 3+ of our branches; the family shop does not count; one per province),
//  - a sub-region manager (2+ province managers in the sub-region), a main-region director (2+ sub-region managers),
//  - a country manager (any country with a shop; required in every country once the company is in two).
// Nobody is appointed automatically. The people of a level that is not appointed answer to the next one above;
// whoever has nobody above answers to the player, who can follow at most 5 people. Every extra person costs the
// directly reporting people 4 skill points (25 at most), hides skimming from the player and slowly costs
// satisfaction. A province manager's oversight needs skill 40 + shops / 3: it lowers order errors, lifts the
// weekly marks, catches a skimming manager and shortens openings in the province by 2 days; a weak or dishonest
// one does less and wears good store managers down. A sub-region manager lowers logistics losses, a director
// brings +0.5 % margin. Managers' wages are paid at the day close like the branches' costs.
// G-086b ek (Docs/Kurgu/01_KARARLAR.md M19-M22):
//  - M19: the family shop can get a manager once a branch outside it is open; he counts as one of the player's
//    people and runs the family's routine in the simulated day (FamilyRule: orders, passing on price rises, shelves).
//  - M20: a country manager can be appointed once the company has shops in 5 provinces of that country (the family
//    shop's province counts); before that the level is hidden. In two or more countries he is required everywhere.
//  - M21: every manager has a hidden ceiling (Potential, 55..95); skill grows in good weeks with a chance that
//    shrinks near the ceiling (seniority helps a little); bonuses never pass it; 95 is the top for everyone.
//  - M22: every appointment offers 3 seeded outside candidates (name, skill - a range without an HR manager -,
//    honesty with an HR manager, style, potential hint, wage). The pool changes once a week (and after each hire);
//    names never come back in a campaign.
namespace MarketManagers
{
    // Depot: the manager of a big depot in a province (M23, G-089: MarketDepots; Area = the depot's province). He
    // answers to the country manager, else to the player (never to a province or sub-region manager).
    enum class ELevel : uint8 { Store = 0, Province = 1, SubRegion = 2, Region = 3, Country = 4, FamilyShop = 5, Depot = 6 };
    // Hidden style of a store manager: careful (small stock, little waste, now and then an empty shelf),
    // generous (full shelves, more waste), price-minded (follows the rivals, may squeeze the margin).
    enum class EStyle : uint8 { Unknown = 0, Careful = 1, Generous = 2, PriceMinded = 3 };

    constexpr int32 SpanLimit = 5;
    constexpr int32 SpanPenaltyPerPerson = 4;
    constexpr int32 SpanPenaltyMax = 25;
    constexpr int32 ProvinceShops = 3;              // branches in a province for a province manager
    constexpr int32 SubRegionManagers = 2;          // province managers in a sub-region for a sub-region manager
    constexpr int32 RegionManagers = 2;             // sub-region managers in a main region for a director
    constexpr int32 SettleDays = 7;                 // a new store manager's first week
    constexpr int32 SettlePenalty = 20;
    constexpr int32 SeveranceDays = 10;             // a replaced or dismissed manager's compensation
    constexpr int32 BonusDays = 7;                  // a bonus is a week's wage
    constexpr int32 BonusCooldown = 14;
    constexpr int32 WarnDeterDays = 21;             // a warned dishonest manager keeps his hands off the till
    constexpr int32 OpeningDaysSaved = 2;
    constexpr int32 MissingCountryPenalty = 10;     // skill lost in a country without its required country manager
    constexpr int32 CountryProvinces = 5;           // provinces with our shops in a country for a country manager (M20)
    constexpr int32 CountryFullShops = 30;          // C11 (M40): a country manager's full band from 30 shops in the country
    constexpr float CountryMinScale = 0.3f;         // C11 (M40): a 5-province chain of a few shops pays 30 % of the band
    constexpr int32 SkillTop = 95;                  // nobody's skill grows past this (M21)
    constexpr int32 PotentialMin = 55;              // a new candidate's ceiling: 55..95
    constexpr int32 CandidateCount = 3;             // outside candidates offered for every appointment (M22)
    constexpr int32 CandidateRangeHalf = 12;        // without an HR manager a candidate's skill shows as a +-12 range

    // Someone in the hierarchy: a branch's store manager (Branch) or an appointed manager (Manager).
    struct FPerson
    {
        ELevel Level = ELevel::Store;
        int32 Branch = INDEX_NONE;       // State.Branches index (Store)
        int32 Manager = INDEX_NONE;      // State.Management.Managers index (every other level)
        FString Name;
        FString Title;                   // "Tekirda\u011f il m\u00fcd\u00fcr\u00fc", "Tekirda\u011f \u00b7 Mahalle 1 m\u00fcd\u00fcr\u00fc"
    };

    // What the day of a branch takes from its manager (MarketBranches::CloseDay).
    struct FBranchRule
    {
        int32 Skill = 0;                 // effective skill: morale, settling in, oversight, the player's span
        float OrderFactor = 1.f;         // x the order target (style)
        float ErrorFactor = 1.f;         // x the order error (province manager's oversight)
        float WasteRate = 0.f;           // share of the units on hand spoiled a day (style)
        float PriceBias = 0.f;           // added to the market type's price target (style)
        bool bFollowsRivals = false;     // keeps prices against the rivals (skilled or price-minded)
        int32 SkimPermille = 0;          // of the revenue kept by a dishonest manager
        bool bSkimHidden = false;        // the player follows too many people: nobody notices the till
    };

    FString LevelName(ELevel Level);                  // "il m\u00fcd\u00fcr\u00fc"
    FString StyleName(uint8 Style);                   // "Temkinli", "C\u00f6mert", "Fiyat\u00e7\u0131"
    // The levels above the shops in the menu's order: country, main region, sub-region, province.
    const TArray<ELevel>& Tiers();
    // M20: can this level of an area show in the menu (it has a manager or can get one now)? A country's country
    // manager stays hidden until 5 provinces of it have our shops (or the company is in two countries).
    bool IsTierVisible(const FMarketState& State, ELevel Level, const FString& Country, const FString& Area);
    // Tiers() without the country level while it is hidden in that country.
    TArray<ELevel> VisibleTiers(const FMarketState& State, const FString& Country);
    // Provinces of a country with an open branch of ours (the family shop's province counts in its country).
    int32 ProvincesWithShops(const FMarketState& State, const FString& Country);
    // C11 (M40): open shops in a country (the family shop counts at home) and the country manager's share of his band.
    int32 ShopsInCountry(const FMarketState& State, const FString& Country);
    float CountryWageScale(const FMarketState& State, const FString& Country);
    // Name of an area: "Tekirda\u011f", "Trakya", "Marmara", "T\u00fcrkiye".
    FString AreaName(ELevel Level, const FString& Country, const FString& Area);

    // Appointed manager of an area (INDEX_NONE / nullptr: none). Country empty = the campaign's.
    int32 FindManager(const FMarketState& State, ELevel Level, const FString& Country, const FString& Area);
    const FMarketManager* ManagerOf(const FMarketState& State, ELevel Level, const FString& Country, const FString& Area);
    const FMarketManager* ProvinceManager(const FMarketState& State, const FString& Country, const FString& Province);
    const FMarketManager* SubRegionManager(const FMarketState& State, const FString& Country, const FString& SubRegion);
    const FMarketManager* RegionManager(const FMarketState& State, const FString& Country, const FString& Region);
    const FMarketManager* CountryManager(const FMarketState& State, const FString& Country);
    // Everyone of a level (Store: every branch with a manager; FamilyShop: the family shop's manager if any).
    TArray<FPerson> People(const FMarketState& State, ELevel Level);
    // Manager index this person answers to (INDEX_NONE = the player).
    int32 BossOf(const FMarketState& State, const FPerson& Person);

    // The people who answer directly to the player (the family shop only once it has a manager).
    TArray<FPerson> DirectReports(const FMarketState& State);
    int32 DirectCount(const FMarketState& State);
    // People beyond the limit and the skill they cost everyone who reports directly (0..25).
    int32 OverLimit(const FMarketState& State);
    int32 SpanPenalty(const FMarketState& State);
    bool ReportsToPlayer(const FMarketState& State, int32 BranchIndex);

    // Our open branches in a province (the family shop never counts) and the skill a province manager needs there.
    int32 ProvinceBranches(const FMarketState& State, const FString& Country, const FString& Province);
    int32 RequiredSkill(const FMarketState& State, int32 ManagerIndex);
    int32 ProvinceRequiredSkill(int32 Shops);        // 40 + shops / 3
    int32 EffectiveManagerSkill(const FMarketState& State, int32 ManagerIndex);
    // 0..1: how much of their work the manager really does (skill against the need, honesty).
    float Strength(const FMarketState& State, int32 ManagerIndex);
    // 0..1: oversight over a branch (its province manager, half of a higher boss when there is none).
    float Oversight(const FMarketState& State, int32 BranchIndex);
    // Everything a branch's day needs from its manager. Span: SpanPenalty(State) when already known.
    FBranchRule RuleFor(const FMarketState& State, int32 BranchIndex, int32 Span = INDEX_NONE);
    int32 EffectiveSkill(const FMarketState& State, int32 BranchIndex);
    // Added to a branch's weekly mark score (0..0.05).
    float GradeBonus(const FMarketState& State, int32 BranchIndex);
    // Days the province manager saves on an opening there (0..2).
    int32 OpeningDaysSavedIn(const FMarketState& State, const FString& Country, const FString& Province);
    // Added to MarketCompany::CostFactor (negative: sub-region manager's logistics, director's margin, country
    // manager abroad).
    float CostAdjust(const FMarketState& State, const FMarketBranch& Branch);

    // Company in two or more countries: every country needs a country manager. Countries still missing one.
    bool CountryManagerRequired(const FMarketState& State);
    TArray<FString> CountriesMissingManager(const FMarketState& State);

    // Wage a day of an appointed manager at today's wage level; the band's start-level wage for a level and skill.
    int64 DailyWage(const FMarketState& State, const FMarketManager& Manager);
    int64 BaseWageFor(ELevel Level, int32 Skill, const FString& Country);
    // Today's daily wages of all appointed managers (store managers are paid with their branch).
    int64 DailyWages(const FMarketState& State);
    // Skill, honesty and style of store managers are hidden until an HR manager or the province manager sees them.
    bool SkillVisible(const FMarketState& State, int32 BranchIndex);

    // M22: an outside candidate for an appointment (not saved: the pool is derived from the campaign seed, the week
    // and the hires so far, so saving and loading shows the same people).
    struct FCandidate
    {
        ELevel Level = ELevel::Store;
        FString Country;
        FString Area;                    // Store: the branch's province
        FString Name;
        int32 Skill = 0;
        int32 Honesty = 70;
        uint8 Style = 0;                 // EStyle
        int32 Potential = 0;             // hidden ceiling 55..95
        int64 BaseWage = 0;              // per day, start-level kurus (appointed managers are paid MarketPrices::WageScaled of it)
        int64 Wage = 0;                  // per day at today's wage level (what a store manager is paid)
    };
    // The 3 outside candidates of a level and area this week (Country empty = the campaign's; Store: Area = the
    // province; Country / FamilyShop: Area may be empty). Names are unique in the pool and never one of
    // State.Management.UsedNames or anyone working for us now.
    TArray<FCandidate> Candidates(const FMarketState& State, ELevel Level, const FString& Country, const FString& Area);
    // The candidates for a branch's store manager (its province's pool).
    TArray<FCandidate> BranchCandidates(const FMarketState& State, int32 BranchIndex);
    // Week of the candidate pool: (Day - 1) / 7.
    int32 CandidateWeek(const FMarketState& State);
    // What the player sees: the skill range (exact with an HR manager, else a +-12 range around a seeded guess that
    // always contains the skill), honesty only with an HR manager, a hint of the potential, the whole line.
    void SkillRange(const FMarketState& State, const FCandidate& Who, int32& OutLow, int32& OutHigh);
    bool HonestyVisible(const FMarketState& State);
    FString PotentialHint(int32 Skill, int32 Potential);   // "geli\u015fmeye \u00e7ok a\u00e7\u0131k" / "biraz geli\u015fir" / "olgun"
    FString DescribeCandidate(const FMarketState& State, const FCandidate& Who);
    // The first candidate of the pool as a manager (the old single-candidate menu line).
    FMarketManager Candidate(const FMarketState& State, ELevel Level, const FString& Country, const FString& Area);
    // Appoints candidate 0..2 of the pool to an area (not Store). False + reason in Turkish.
    bool AppointCandidate(FMarketState& State, ELevel Level, const FString& Country, const FString& Area, int32 CandidateIndex, FString& OutMessage);
    // A branch's store manager from its pool: Replace pays the old one's severance (a branch without a manager just
    // hires); HireCandidateForBranch only fills a branch that has no manager.
    bool ReplaceWithCandidate(FMarketState& State, int32 BranchIndex, int32 CandidateIndex, FString& OutMessage);
    bool HireCandidateForBranch(FMarketState& State, int32 BranchIndex, int32 CandidateIndex, FString& OutMessage);
    // The outside store manager who comes without a choice (a new branch, a promotion): the pool's first candidate.
    void HireStoreManager(FMarketState& State, int32 BranchIndex);
    // FromBranch = INDEX_NONE: an outside candidate; else that branch's manager is promoted and the branch gets a
    // new outside manager (a settling-in week). False + reason in Turkish.
    bool CanAppoint(const FMarketState& State, ELevel Level, const FString& Country, const FString& Area, int32 FromBranch, FString& OutReason);
    bool Appoint(FMarketState& State, ELevel Level, const FString& Country, const FString& Area, int32 FromBranch, FString& OutMessage);
    // Removes an appointed manager (severance); the people below answer to the next one above.
    bool Dismiss(FMarketState& State, int32 ManagerIndex, FString& OutMessage);
    bool BonusManager(FMarketState& State, int32 ManagerIndex, FString& OutMessage);
    bool WarnManager(FMarketState& State, int32 ManagerIndex, FString& OutMessage);

    // Store manager decisions. Bonus: a week's wage, morale and a little skill. Warn: a dishonest one stops
    // skimming for a while, an honest one is hurt when he did not deserve it. Replace: severance to the old one,
    // the pool's first outside candidate comes (a settling-in week). PromoteToProvince: the branch's manager becomes the
    // province manager (the branch gets a new one).
    bool Bonus(FMarketState& State, int32 BranchIndex, FString& OutMessage);
    bool Warn(FMarketState& State, int32 BranchIndex, FString& OutMessage);
    bool Replace(FMarketState& State, int32 BranchIndex, FString& OutMessage);
    bool PromoteToProvince(FMarketState& State, int32 BranchIndex, FString& OutMessage);

    // A branch's new store manager (hired or promoted): seeded style, morale 70, a settling-in week.
    void InitStoreManager(const FMarketState& State, FMarketBranch& Branch, int32 BranchIndex);

    // Menu lines.
    FString Describe(const FMarketState& State, const FPerson& Person);
    FString DescribeBranchManager(const FMarketState& State, int32 BranchIndex);
    FString DescribeManager(const FMarketState& State, int32 ManagerIndex);
    // "Sana do\u011frudan 6 ki\u015fi ba\u011fl\u0131 (s\u0131n\u0131r 5)..."
    FString SpanText(const FMarketState& State);
    // What to do next, most urgent first ("Tekirda\u011f'da 3 ma\u011faza var: il m\u00fcd\u00fcr\u00fc atayabilirsin.").
    TArray<FString> Suggestions(const FMarketState& State);
    FString NextStep(const FMarketState& State);

    // Menu command arguments: an area (level, country, area) and a promotion (branch, level: the area is the branch's own).
    // (Level x 100 + country index) x 1000 + area index into MarketCountry::All(), its Cities / SubRegions /
    // Regions (Depot: Cities; Country and FamilyShop: 0). INDEX_NONE when unknown. A candidate choice adds one digit:
    // area argument x 10 + candidate (0..2); a branch's candidate: branch index x 10 + candidate.
    int32 EncodeArea(ELevel Level, const FString& Country, const FString& Area);
    bool DecodeArea(int32 Arg, ELevel& OutLevel, FString& OutCountry, FString& OutArea);
    // The area of a level that contains a branch ("" when the pack has none).
    FString AreaOfBranch(const FMarketState& State, int32 BranchIndex, ELevel Level);

    // Older saves: store managers without a style / morale / potential and managers without a style / potential get
    // them from the seed (once; idempotent). A potential is the skill + 5..20, at most 95. Everyone working for us
    // is added to State.Management.UsedNames (M22: a name never comes back after he leaves).
    void Migrate(FMarketState& State);

    // M21: a person's ceiling (Potential; an older save's value derived as Migrate does), the chance of +1 skill
    // in a good week (0 at the ceiling, proportional to the distance, seniority adds up to half) and one week's
    // growth with a seeded roll (its value modulo 100000; Pace scales the chance: managers above the shops 0.5).
    int32 PotentialOf(const FMarketBranch& Branch);
    int32 PotentialOf(const FMarketManager& Manager);
    float GrowthChance(int32 Skill, int32 Potential, int32 TenureWeeks);
    int32 GrowSkill(int32 Skill, int32 Potential, int32 TenureWeeks, uint32 Roll, float Pace = 1.f);

    // M19: how the family shop's manager runs its simulated day (MarketSimulation::PlayDay). Without a manager the
    // family's own routine (bManaged false: every value as before).
    struct FFamilyRule
    {
        bool bManaged = false;
        int32 Skill = 0;                 // effective: morale, settling-in week, the player's span, the boss above
        float OrderFactor = 1.f;         // x the suggested order (style)
        double PriceRiseGap = 0.02;      // the month's price rise goes on the shelf above this gap (style)
        int32 RefillEvery = 8;           // shelves are filled after every N shoppers (skill)
        int32 ForgetPermille = 0;        // lines of the order a weak manager forgets (skill)
    };
    FFamilyRule FamilyRule(const FMarketState& State);
    // The suggested order shaped by the family shop's manager (seeded by the campaign and the day).
    void ShapeFamilyOrder(const FMarketState& State, const FFamilyRule& Rule, TArray<int32>& Draft);
    // Day close, right after MarketBranches::CloseDay: wages, morale, weekly marks (growth, leaving), the province
    // manager catching a skimmer, the missing country manager, the player's span, a weekly line.
    void CloseDay(FMarketState& State);
}

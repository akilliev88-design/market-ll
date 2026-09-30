#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Depots in provinces and their managers (G-089, karar M23; Docs/Kurgu/03_MAGAZA_AGI.md \u00a76). Independent of the
// world, tested (MirasMarket.Depots.*). A big depot stands in a province the player chooses (the game suggests
// the one nearest to the branches, weighted by their revenue) and serves several provinces around it:
//  - every open branch takes its goods from the NEAREST depot of its own country within 600 km; farther away it
//    buys from the wholesaler as before (+3 % outside the home province). A branch in the home province uses a
//    depot only within 200 km (the home wholesaler is next door);
//  - a depot gives the chain's rebate (1.5 % of the goods at full efficiency); the road costs: the first 100 km are
//    free, then +0.6 % of the goods per 100 km;
//  - trucks: every served branch is one load plus one per 300 km; a truck carries 8 loads a day. Missing trucks cost
//    up to 1.5 % and late, short deliveries;
//  - the depot manager (MarketManagers::ELevel::Depot, 80-120 TL a day at the start price level) sets the efficiency: a
//    skilled one keeps waste, short and broken deliveries low (the shelves stay full); a depot without a manager
//    works at 50 % and warns every week; a dishonest one takes a little of the goods (the country manager or the
//    player may catch him, never a province or sub-region manager). He answers to the country manager, else to the
//    player (one of the player's 5 people);
//  - more branches than its capacity (60) slow a depot down.
// Distances: iller.json map centres x FProfile::MapKm; packs without a map use 150 km in a sub-region, 350 km in a
// main region, 700 km otherwise.
namespace MarketDepots
{
    constexpr int32 RangeKm = 600;              // beyond this a depot does not serve a branch
    constexpr int32 HomeRangeKm = 200;          // a home-province branch uses a depot only this close
    constexpr int32 FreeKm = 100;               // the first 100 km cost nothing
    constexpr float CostPer100Km = 0.006f;      // +0.6 % of the goods for every 100 km beyond FreeKm
    constexpr float Rebate = 0.015f;            // the depot's rebate at full efficiency
    constexpr float WholesalerVan = 0.03f;      // a branch outside the home province without a depot
    constexpr float TruckShortCost = 0.015f;    // all trucks missing
    constexpr int32 TruckShortPermille = 20;    // late trucks: goods lost on the way when all are missing
    constexpr int32 LoadKm = 300;               // one more truck load per 300 km
    constexpr int32 LoadsPerTruck = 8;
    constexpr int32 DefaultCapacity = 60;
    constexpr float UnmanagedEfficiency = 0.5f;
    constexpr int32 MaxShortPermille = 40;      // short / broken deliveries at efficiency 0
    constexpr float MaxExtraWaste = 0.004f;     // extra share of the goods on hand spoiled a day at efficiency 0
    constexpr int32 SkimPermille = 8;           // goods a dishonest depot manager takes
    constexpr int32 SameSubRegionKm = 150;      // fallback distances without a map
    constexpr int32 SameRegionKm = 350;
    constexpr int32 OtherRegionKm = 700;
    constexpr int32 MinStores = 4;              // shops of the company before the first depot
    constexpr int64 BuildCost2011 = 3000000;    // 30 000 TL x the province's rent (x the list level)
    constexpr int64 MonthlyRent2011 = 600000;   // 6 000 TL a month x the province's rent (x the list level)

    // Kilometres between two provinces' centres of a country (0 for the same province; the fallback without a map).
    float DistanceKm(const FString& Country, const FString& FromProvince, const FString& ToProvince);
    // Share of the goods the road adds (0 up to FreeKm).
    float DistanceCost(float Km);

    // Depots (State.Company.DepotSites; older saves' sub-region depots count until Migrate moves them).
    int32 Count(const FMarketState& State);
    // Index into State.Company.DepotSites (INDEX_NONE: none). Country empty = the campaign's.
    int32 Find(const FMarketState& State, const FString& Country, const FString& Province);
    bool HasDepotIn(const FMarketState& State, const FString& Country, const FString& Province);
    bool HasDepotInSubRegion(const FMarketState& State, const FString& Country, const FString& SubRegion);
    // "Tekirda\u011f deposu".
    FString DepotName(const FMarketState& State, int32 DepotIndex);
    // Its manager (State.Management.Managers index, INDEX_NONE: none).
    int32 ManagerOf(const FMarketState& State, int32 DepotIndex);

    // How a branch gets its goods.
    struct FLink
    {
        int32 Depot = INDEX_NONE;       // INDEX_NONE: the wholesaler
        float Km = 0.f;                 // to the depot
        float Efficiency = 0.f;         // of the depot (0.5 without a manager, 1 at best)
        float CostAdd = 0.f;            // added to MarketCompany::CostFactor (rebate, road, trucks)
        int32 ShortPermille = 0;        // of the morning delivery short or broken
        float ExtraWaste = 0.f;         // added to the branch's daily waste rate
        int32 SkimPermille = 0;         // of the delivery a dishonest depot manager takes
    };
    // The nearest depot of the country within range (INDEX_NONE: none); OutKm its distance.
    int32 Nearest(const FMarketState& State, const FString& Country, const FString& Province, bool bHome, float& OutKm);
    FLink LinkFor(const FMarketState& State, const FString& Country, const FString& Province);
    FLink LinkOf(const FMarketState& State, const FMarketBranch& Branch);
    // Every branch's link at once (indexed like State.Branches; closed / not open ones get none).
    TArray<FLink> AllLinks(const FMarketState& State);
    // Open branches a depot serves now.
    TArray<int32> ServedBranches(const FMarketState& State, int32 DepotIndex);
    int32 Served(const FMarketState& State, int32 DepotIndex);
    // 0..1: the manager's effective skill (UnmanagedEfficiency without one), slower beyond the capacity.
    float Efficiency(const FMarketState& State, int32 DepotIndex);
    // Trucks: loads a day (one per served branch + one per 300 km), trucks needed, the missing share (0..1).
    float TruckLoads(const FMarketState& State);
    int32 TrucksNeeded(const FMarketState& State);
    float TruckShortage(const FMarketState& State);

    // Building a depot in a province. Cost and rent follow the province's rent and the list level.
    int64 BuildCost(const FMarketState& State, const FString& Country, const FString& Province);
    int64 MonthlyRent(const FMarketState& State, const FString& Country, const FString& Province);
    bool CanBuild(const FMarketState& State, const FString& Country, const FString& Province, FString& OutReason);
    bool Build(FMarketState& State, const FString& Country, const FString& Province, FString& OutMessage);

    // The province that brings the depot nearest to the branches of the country (revenue-weighted distance to the
    // nearest depot, the existing ones included). WithinSubRegion limits the choice (the old sub-region button).
    struct FAdvice
    {
        FString Country;
        FString Province;               // empty: no branch in the country
        int32 Branches = 0;             // branches it would serve
        float AverageKm = 0.f;          // revenue-weighted to those branches
        int64 MonthlyGain = 0;          // estimated: cheaper goods minus rent and a manager's wage
        FString Text;                   // "\u00d6nerilen il: ..." for the menu
    };
    FAdvice SuggestDepotProvince(const FMarketState& State, const FString& Country, const FString& WithinSubRegion = FString());

    // Menu lines: a depot (manager, branches served, efficiency, losses) and a branch's supply ("Tekirda\u011f deposu,
    // 156 km" / "toptanc\u0131").
    FString Describe(const FMarketState& State, int32 DepotIndex);
    FString DescribeLink(const FMarketState& State, int32 BranchIndex);

    // Units of a delivery lost at a rate (seeded rounding of the fraction).
    int32 LostUnits(int32 Units, int32 Permille, uint32 Roll);
    // A branch's delivery lost at a depot (goods already paid, at cost): short / broken and skimmed.
    void RecordLoss(FMarketState& State, int32 DepotIndex, int64 ShortCost, int64 SkimCost);

    // Older saves: every sub-region depot ("country:subregion") moves to the province of that sub-region with most
    // of our shops (else its most populous one), without a manager (one line of news). Idempotent.
    void Migrate(FMarketState& State);
    // Today's rent of all depots (paid in MarketCompany::CloseDay with the head office).
    int64 DailyRent(const FMarketState& State, int32 Day);
    // Day close after the branches and managers: a caught skimmer, weekly warnings for depots without a manager,
    // the week's losses.
    void CloseDay(FMarketState& State);
}

#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Akis C2b (Docs/Kurgu/07_AKIL_ISBOLUMU.md, Mustafa 30.09.2026: "iyi bir rakip listesi, akillari iyi olsun; dunya
// ligine gitmek ne cok zor ne cok kolay"): the rival chains of every country we play in and the world's giants.
// Independent of the world, tested (MirasMarket.Chains.*). The street next to the family shop keeps its own rivals
// (MarketCompetitors); this module is the country and the world:
//  - Three layers, every name fictional (L12): local family chains of a province and a regional chain of each
//    sub-region; 6-8 national chains a country, each with a character (discounter, fast discounter, supermarket,
//    hypermarket, premium, regional, cash & carry); 12 world giants whose arms enter countries.
//  - A chain has stores per province, cash, prices, service, aggression, ambition and a memory of us (rivalry).
//    Its month: the stores earn by the province's saturation (all stores, ours included, against what the
//    province can feed), it opens where the gap is, closes where it bleeds, starts a price war where we grow
//    fast, goes for sale when its cash runs out, and strong chains buy weak ones. It decides with what the
//    player can also see; the difficulty changes the quality and speed of its decisions, never gives it money.
//  - Our branches feel it: a province's competition follows the chains' stores against the start
//    (PressureFactor) and a price war against us bites harder for its days.
//  - Tables: the country's retailers by revenue (national rank) and the world league (giants, the national
//    chains of our countries and us, in world units at the start price level). The giants' size against the
//    game's economy is compressed (LeagueCompression) so that a well played company can reach the top in about
//    30 years (tuned with the automatic player, 06 A1).
//  - Fun (06 section 2b): named bosses, a nemesis (the chain that minds us most), wars with a start and an end,
//    chains for sale we can buy, rank changes as news.
namespace MarketChains
{
    enum class EArchetype : uint8 { Discount = 0, FastDiscount, Super, Hyper, Premium, Regional, Family, Wholesale, Club, Count };
    enum class EScope : uint8 { Local = 0, Regional, National, Foreign };

    constexpr int32 TurnDays = 30;           // every chain decides once a month
    constexpr int32 WarDays = 45;
    constexpr float WarPrice = 0.92f;        // the war's shelf prices
    constexpr float WarPressure = 1.3f;      // x competition of our branches in that province during a war
    constexpr float HealthyPer100k = 32.f;   // weighted chain stores a province feeds comfortably
    constexpr float LeagueCompression = 0.04f; // giants' size against the game's economy (tuned with the automatic player)
    constexpr int32 MaxNewsPerDay = 3;

    // Tables of the roster (national chains a country, the giants). Fictional names come from Config/zincirler.json
    // when it is readable; the table's own names otherwise.
    struct FRosterChain
    {
        const TCHAR* Id;
        const TCHAR* Country;
        const TCHAR* Name;
        const TCHAR* Boss;
        EArchetype Archetype;
        int32 StartStores;
        float PriceIndex;
        float Service;
        float Aggression;
        float Ambition;
        const TCHAR* HomeRegion;             // regional chains: the main region they start in ("" = everywhere)
    };
    struct FRosterGiant
    {
        const TCHAR* Id;
        const TCHAR* Name;
        const TCHAR* Home;                   // home country name for the menu
        const TCHAR* HomePack;               // its pack id when we have one ("" = none)
        EArchetype Archetype;
        float RevenueB;                      // billions of world units a year at the start
        float Growth;
    };
    const TArray<FRosterChain>& NationalRoster();
    const TArray<FRosterGiant>& GiantRoster();
    FString ArchetypeName(EArchetype Archetype);  // "indirim marketi"
    // The market type (MarketBranches format) a chain's stores are closest to.
    FString FormatOf(EArchetype Archetype);

    // Seeds a country (its national and regional chains) the first time; the giants once. Idempotent.
    void EnsureCountry(FMarketState& State, const FString& Country);
    // Local family chains of a province (made the first time we are there). Idempotent.
    void EnsureLocal(FMarketState& State, const FString& Country, const FString& Province);
    // Every country we have a shop in plus the campaign's (older saves: called from CloseDay).
    void Ensure(FMarketState& State);

    // Stores of every chain in a province and their weight (a hypermarket counts more than a discounter).
    int32 StoresIn(const FMarketState& State, const FString& Country, const FString& Province);
    float WeightedIn(const FMarketState& State, const FString& Country, const FString& Province);
    // How full a province is: 1 = comfortable, below 1 crowded (every store sells less), above 1 room to grow.
    float Saturation(const FMarketState& State, const FString& Country, const FString& Province);
    // x competition for our branches there: the chains' stores against the start, a war against us on top.
    float PressureFactor(const FMarketState& State, const FString& Country, const FString& Province, int32 Day);
    // The chain at war with us in that province today (INDEX_NONE: none).
    int32 WarIn(const FMarketState& State, const FString& Country, const FString& Province, int32 Day);

    // Revenue a year (kurus at today's level): a chain, and ours in a country (branches + the family shop at home).
    int64 YearRevenue(const FMarketState& State, int32 ChainIndex);
    int64 OurYearRevenue(const FMarketState& State, const FString& Country);
    int32 TotalStores(const FMarketChain& Chain);
    // Kurus of a country -> world units at the start price level.
    double ToWorld(const FMarketState& State, const FString& Country, int64 Kurus);

    struct FStanding
    {
        FString Name;
        FString Detail;                  // "indirim marketi \u00b7 3.512 ma\u011faza" / "ABD"
        double Revenue = 0.0;            // a year: kurus (national table) or world units (league)
        int32 Stores = 0;
        int32 Chain = INDEX_NONE;        // State.Rivals.Chains index, or INDEX_NONE
        bool bUs = false;
        bool bNemesis = false;
    };
    // The country's retailers by revenue, us included (national chains, regional chains, foreign arms).
    TArray<FStanding> NationalTable(const FMarketState& State, const FString& Country);
    // The world league: giants + national chains of our countries + us, by revenue in world units.
    TArray<FStanding> WorldTable(const FMarketState& State);
    int32 OurRank(const TArray<FStanding>& Table);

    // Buying a chain that is for sale: its stores become our branches (as many as the provinces have room for;
    // the rest are sold on). Price: 8 months of its revenue.
    int64 Price(const FMarketState& State, int32 ChainIndex);
    bool CanBuy(const FMarketState& State, int32 ChainIndex, FString& OutReason);
    bool Buy(FMarketState& State, const TArray<FMarketProduct>& Products, int32 ChainIndex, FString& OutMessage);

    // Menu lines.
    FString Describe(const FMarketState& State, int32 ChainIndex);
    FString NemesisLine(const FMarketState& State);   // "" when there is no nemesis

    // Day close (after the branches): the chains whose month is up take their turn, wars end, the giants' year,
    // the tables and their news.
    void CloseDay(FMarketState& State);
}

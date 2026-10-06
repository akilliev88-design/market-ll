#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Akis C2b (Docs/Kurgu/07_AKIL_ISBOLUMU.md, Mustafa 30.09.2026: "iyi bir rakip listesi, akillari iyi olsun; dunya
// ligine gitmek ne cok zor ne cok kolay"): the rival chains of every country we play in and the world's giants.
// Independent of the world, tested (MirasMarket.Chains.*). E2: this is the one rival model; the first store's rivals
// are the chains of its province like every store's (the street model and the daily rival news were removed):
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
    // Convenience (D5/M52: kombini, yakin market; Japan, Mexico, Poland) and CashCarry (toptan perakende, atacarejo;
    // Brazil) were added at the end: the saved values of the others stay.
    enum class EArchetype : uint8 { Discount = 0, FastDiscount, Super, Hyper, Premium, Regional, Family, Wholesale, Club, Convenience, CashCarry, Count };
    enum class EScope : uint8 { Local = 0, Regional, National, Foreign };

    constexpr int32 TurnDays = 30;           // every chain decides once a month
    constexpr int32 WarDays = 21;            // C3 (A6): shorter wars
    constexpr int32 WarRestDays = 60;        // C3 (A6): a province rests after a war
    constexpr float WarPrice = 0.92f;        // the war's shelf prices
    constexpr float WarPressure = 1.1f;      // x competition of our branches in that province during a war (C3, A6: 1.3 -> 1.1)
    constexpr float HealthyPer100k = 32.f;   // weighted chain stores a province feeds comfortably
    constexpr float LeagueCompression = 0.04f; // giants' size against the game's economy (tuned with the automatic player)
    constexpr int32 MaxNewsPerDay = 3;

    // Tables of the roster (national chains a country, the giants). Fictional names come from Config/zincirler.json
    // when it is readable; the table's own names otherwise.
    // D3: a national chain of a country pack (ulkeler.json "roster", MarketCountry::FRosterRow).
    struct FRosterChain
    {
        FString Id;
        FString Country;
        FString Name;
        FString Boss;
        EArchetype Archetype = EArchetype::Super;
        int32 StartStores = 0;
        float PriceIndex = 1.f;
        float Service = 1.f;
        float Aggression = 0.5f;
        float Ambition = 0.5f;
        FString HomeRegion;                  // regional chains: the main region they start in ("" = everywhere)
    };
    // D3: a world giant (Config/ulkeler.json root "giants", MarketCountry::FGiantRow).
    struct FRosterGiant
    {
        FString Id;
        FString Name;
        FString Home;                        // home country name for the menu
        FString HomePack;                    // its pack id when we have one ("" = none)
        EArchetype Archetype = EArchetype::Super;
        float RevenueB = 1.f;                // billions of world units a year at the start
        float Growth = 0.f;
    };
    // Every pack's national chains in pack order (loaded once from MarketCountry::All()).
    const TArray<FRosterChain>& NationalRoster();
    // D3: "discount", "fastDiscount", ... -> archetype (false: unknown text).
    bool ArchetypeOf(const FString& Text, EArchetype& OutArchetype);
    const TArray<FRosterGiant>& GiantRoster();
    FString ArchetypeName(EArchetype Archetype);  // "indirim marketi"
    // The market type (MarketBranches format) a chain's stores are closest to.
    FString FormatOf(EArchetype Archetype);

    // Seeds a country (its national and regional chains) the first time; the giants once. Idempotent.
    void EnsureCountry(FMarketState& State, const FString& Country);
    // Local family chains of a province (made the first time we are there). Idempotent.
    void EnsureLocal(FMarketState& State, const FString& Country, const FString& Province);
    // Every country we have a shop in plus the campaign's (also called from CloseDay).
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
    // E2 (the one rival model): the chains with stores in a province, most present first (weighted stores; not ours,
    // not gone), at most Max of them.
    TArray<int32> RivalsIn(const FMarketState& State, const FString& Country, const FString& Province, int32 Max);
    // A chain's shelf prices in a province on a day against the list (its everyday level, its war prices there).
    float PriceIn(const FMarketState& State, int32 ChainIndex, const FString& Province, int32 Day);
    // What shoppers of the province think "the rivals" charge against the list: the chains' prices weighted by
    // their weighted stores there (1 when nobody is there yet).
    float RivalPriceFactor(const FMarketState& State, const FString& Country, const FString& Province, int32 Day);
    // E2 (was MarketCompetitors): now and then a chain of the home province offers one of our good people a job;
    // a content one says no, an unhappy one (morale < 45) gives notice. Called at the day close.
    void Poach(FMarketState& State, int32 Closed);

    // Revenue a year (kurus at today's level): a chain, and ours in a country (branches + the first store at home).
    int64 YearRevenue(const FMarketState& State, int32 ChainIndex);
    int64 OurYearRevenue(const FMarketState& State, const FString& Country);
    int32 TotalStores(const FMarketChain& Chain);
    // Kurus of a country -> world units at the start price level.
    double ToWorld(const FMarketState& State, const FString& Country, int64 Kurus);

    struct FStanding
    {
        FString Name;                    // M65: the name everybody uses (our brand, the chain's name)
        FString LegalName;               // M65: the registered name, shown in a country's own table (empty in the world league)
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
    // M53 (Mustafa 03.10.2026): the lists show only their first rows (the world league its 50, a country 10-30 by
    // its people); we are on a list only once our revenue passed its last row (who then drops out). Listed: the rows
    // shown (fewer when the table has fewer rivals); ListedRank: our place on it (0 = not on the list); OutsideText:
    // "Listede de\u011filsin: 50. s\u0131radaki X ile aran\u0131zda ... var" ("" when on it).
    constexpr int32 WorldListSize = 50;
    int32 NationalListSize(const FString& Country);
    TArray<FStanding> Listed(const TArray<FStanding>& Full, int32 Cap);
    int32 ListedRank(const TArray<FStanding>& Full, int32 Cap);
    FString OutsideText(const TArray<FStanding>& Full, int32 Cap, bool bWorld);

    // Buying a chain that is for sale (M30: whole, see below). Price: 8 months of its revenue; a giant leaving the
    // country (bExitSale) sells for 6 months, even where we have no store yet: a way into a new country.
    int64 Price(const FMarketState& State, int32 ChainIndex);
    bool CanBuy(const FMarketState& State, int32 ChainIndex, FString& OutReason, bool bCheckCash = true);
    bool Buy(FMarketState& State, const TArray<FMarketProduct>& Products, int32 ChainIndex, FString& OutMessage);

    // Karar M29: a takeover bid for a chain that is not for sale. Price: a year of its revenue, a fifth more for a
    // healthy one. Its owner says yes or no from its health, its pride (scope) and how much it minds us; a nemesis
    // never sells. A refused bid costs the advisers' fee (0.5 %), angers it and waits 180 days.
    constexpr int32 BidWaitDays = 180;
    constexpr float BidFee = 0.005f;
    int64 BidPrice(const FMarketState& State, int32 ChainIndex);
    float AcceptChance(const FMarketState& State, int32 ChainIndex);
    bool WouldAccept(const FMarketState& State, int32 ChainIndex);
    bool CanBid(const FMarketState& State, int32 ChainIndex, FString& OutReason, bool bCheckCash = true);
    bool Bid(FMarketState& State, const TArray<FMarketProduct>& Products, int32 ChainIndex, FString& OutMessage);
    // The chain's yearly operating result (what an acquisition loan can lean on).
    int64 YearProfit(const FMarketState& State, int32 ChainIndex);

    // C13 (M43): the player's bid goes to the owner, who answers in 2-4 days; a yes holds for BidHoldDays at the
    // agreed price (the money can come from a bank application), then the deal falls through. A no costs the
    // advisers' fee and waits BidWaitDays as before. Bid() stays the instant version (automatic player, tests).
    constexpr int32 BidHoldDays = 14;
    bool OfferBid(FMarketState& State, int32 ChainIndex, FString& OutMessage);
    // The price of a deal we can close now: an accepted bid's agreed price, a chain for sale's price (0: none).
    int64 DealPrice(const FMarketState& State, int32 ChainIndex);
    // Pay for an accepted bid or a chain for sale from the till.
    bool CompleteDeal(FMarketState& State, const TArray<FMarketProduct>& Products, int32 ChainIndex, FString& OutMessage);
    // Owners' answers due today, accepted bids that ran out (MarketChains::CloseDay).
    void CloseBids(FMarketState& State);

    // C13 (M43): what a true rumour makes happen on its day (MarketRumors). False when it cannot happen any more.
    bool ForceForSale(FMarketState& State, int32 ChainIndex, FString& OutNews);
    bool ForceEnter(FMarketState& State, int32 ChainIndex, const FString& Province, FString& OutNews);
    bool ForceAcquire(FMarketState& State, int32 BuyerIndex, int32 TargetIndex, FString& OutNews);
    bool ForceWar(FMarketState& State, int32 ChainIndex, const FString& Province, FString& OutNews);
    // D9 (M48): a chain gives up one of its shops in a province (our province push).
    bool Withdraw(FMarketState& State, int32 ChainIndex, const FString& Province, FString& OutNews);
    int32 FindChainIndex(const FMarketState& State, const FString& Id);
    FString ProvinceName(const FString& Country, const FString& Province);

    // Karar M30: what we buy is ours whole. A small chain (up to SmallChain stores) becomes our branches as far as
    // the provinces have room; anything bigger (and what does not fit) runs as our subsidiary under its own name:
    // its month is reckoned like a rival's (no war, no openings, it closes stores where the province starves),
    // its result comes to our till, its stores and revenue count as ours in the tables. Every month up to
    // ConvertPerMonth of its stores can take our name (a refit each) and become full branches; or the whole
    // subsidiary is sold (8 months of its revenue) and goes back to being a rival.
    constexpr int32 SmallChain = 20;
    constexpr int32 ConvertPerMonth = 20;
    constexpr float ConvertCostShare = 0.5f;   // x the store type's fit-out
    int32 Subsidiaries(const FMarketState& State);
    int32 SubsidiaryStores(const FMarketState& State, const FString& Country = FString());
    int64 ConvertCost(const FMarketState& State, int32 ChainIndex, int32 Count);
    int32 ConvertRoom(const FMarketState& State, int32 ChainIndex);   // stores convertible now (room, monthly cap)
    bool Convert(FMarketState& State, const TArray<FMarketProduct>& Products, int32 ChainIndex, int32 Count, FString& OutMessage);
    int64 SalePrice(const FMarketState& State, int32 ChainIndex);
    bool SellSubsidiary(FMarketState& State, int32 ChainIndex, FString& OutMessage);
    // Menu argument for Convert: chain index x 100 + count.
    int32 EncodeConvert(int32 ChainIndex, int32 Count);

    // Menu lines.
    FString Describe(const FMarketState& State, int32 ChainIndex);
    FString NemesisLine(const FMarketState& State);   // "" when there is no nemesis

    // Day close (after the branches): the chains whose month is up take their turn, wars end, the giants' year,
    // the tables and their news.
    void CloseDay(FMarketState& State);
}

#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Branches (G-068, rebuilt on provinces in G-086; Docs/Kurgu/03_MAGAZA_AGI.md). Independent of the world, tested
// (MarketSim.Branches.*). The only place level of the game is the province (il / Land / state): a branch opens
// in a province of any country pack, as one of four market types, and goes lease and fit-out -> permits ->
// hiring -> opening stock -> open. Its shelves are planned automatically (MarketLayout) and its day is simulated
// from the same rules as the first store, without walking customers: the province's shoppers split between us
// and the rivals by attractiveness (price, full shelves, service, habit); what they want follows the calendar;
// what is not on the shelf is a lost sale. A manager orders for tomorrow (a skilled one closer to the real
// demand), keeps prices near the market type's target, and a dishonest one skims a little. A new branch builds
// its habit over 30 days; our own shops in the same province take customers from each other once the province
// is full (about one shop per 80 000 people). Logistics, depots and the company's buying power come from
// MarketCompany.
namespace MarketBranches
{
    enum class EStage : uint8 { Renovation = 0, Permits, Hiring, Open, Closed };

    // A market type (karar: ucuzcu, mahalle, supermarket, hipermarket). Ids are kept from the prototype so older
    // saves load: "kucuk" is the discounter.
    struct FFormat
    {
        const TCHAR* Id = TEXT("mahalle");
        const TCHAR* Name = TEXT("mahalle marketi");
        const TCHAR* Short = TEXT("Mahalle");
        int64 FitOut = 400000;           // start-level kurus
        int32 Workers = 3;
        float Service = 1.f;
        float PriceTarget = 1.f;         // shelf prices against the rivals' (a skilled manager keeps them there)
        int32 Trips = 240;               // shopping trips a day in its catchment, a province of the reference size
        int64 Rent = 60000;              // per month, start-level kurus, x the province's rent
        float Weight = 1.f;              // how much of the province's room it takes (cannibalization)
        float Running = 1.f;             // electricity, cleaning, small costs
        int32 MinPopulationK = 0;        // hypermarket: only in big provinces
        bool bNeedsDepot = false;        // hypermarket: a depot of the country within 600 km (G-089)
        int32 MinStores = 0;             // M69: the company's size it needs (hypermarket, cash-and-carry: 8 shops in
        int32 MinProvinces = 0;          //      2 provinces; 0 = from the start)
        // M54: what a shopper puts in the basket (x UnitsPerShopper; a convenience store sells a few items, a
        // cash-and-carry by the case) and how much dearer shelves its shoppers accept (added to the tolerance;
        // convenience is paid for).
        float Basket = 1.f;
        float Tolerance = 0.f;
        const TCHAR* Base = nullptr;     // M54: the ready-made store type it uses (views, measures, departments); null = itself
    };

    // Where a branch is. Country: the pack id (empty = the campaign's country).
    struct FSite
    {
        FString Country;
        FString Province;
        FString Name;                    // "Tekirdag" with Turkish letters
        FString SubRegion;
        int32 PopulationK = 0;
        float Income = 1.f;
        float Rent = 1.f;
        float Competition = 1.f;
        bool bValid = false;
        bool bHome = false;              // the first store's province
        bool bAbroad = false;
    };

    constexpr int32 RenovationDays = 5;
    constexpr int32 PermitDays = 3;
    constexpr int32 MaturityDays = 30;

    // C15 (M45): growth faster than management can follow. Leases signed in the last year against what the company
    // can follow (6 + 40 % of its shops + 2 for every province/region/country manager, the managers' part at most
    // 30 % of the shops; C15b: the first rule, 8 + half + 3 each, never bit). Above it the strain (0..1)
    // makes a new site a hasty pick with chance HastyChance x strain (a quarter fewer shoppers, for good), and young
    // branches (under YoungDays) serve worse. Fast growth stays possible; it is a gamble the player can see.
    constexpr int32 StrainWindowDays = 365;
    constexpr int32 StrainBase = 6;
    constexpr int32 StrainPerManager = 2;
    constexpr float StrainShopShare = 0.4f;     // of the open shops
    constexpr float StrainManagerShare = 0.3f;  // the managers' part, at most this share of the open shops
    // C15b (M44): the difficulty's shopper factor reaches a branch at half strength (full strength made Zor kill
    // the careful and balanced networks and Rahat double them; branch margins are thin).
    constexpr float BranchDifficultyShare = 0.5f;
    constexpr float HastyChance = 0.6f;
    constexpr float HastyTrips = 0.75f;
    constexpr int32 YoungDays = 180;
    constexpr float YoungServiceLoss = 0.12f;
    int32 SignedLastYear(const FMarketState& State);
    int32 GrowthCapacity(const FMarketState& State);
    float GrowthStrain(const FMarketState& State, int32 ExtraSigned = 0);
    FString GrowthStrainText(const FMarketState& State); // empty when growth is within reach
    constexpr float UnitsPerShopper = 2.2f;
    constexpr int32 PeoplePerStoreK = 40;    // room for one of our shops per 40 000 people (at least 2)
    constexpr int32 PeoplePerSlotK = 80;     // above one shop per 80 000 people ours start to share customers
    // M70: the boss walks through a store (MarketBranches::Visit): its people's morale and the day's service rise a little.
    constexpr float BossMorale = 3.f;
    constexpr float BossService = 1.05f;

    const TArray<FString>& FormatIds();      // kucuk, mahalle, buyuk, hiper (every country; the store kits)
    // M54 (Mustafa 03.10.2026): + yakin (convenience store, kombini) and toptan (cash-and-carry, atacarejo). Menu and
    // command arguments index this list.
    const TArray<FString>& AllFormatIds();
    // The types a player may open in Country: the four everywhere, plus what its market knows (pack "playerFormats").
    TArray<FString> FormatsIn(const FString& Country);
    // The ready-made store type a format uses (yakin -> kucuk, toptan -> hiper; the four: themselves).
    FString BaseFormat(const FString& Format);
    const FFormat& FormatInfo(const FString& Id);
    // M69: a store type the company is big enough for (MinStores / MinProvinces); OutReason when not yet.
    bool FormatOpen(const FMarketState& State, const FString& Format, FString& OutReason);
    constexpr double FirstBranchFitOut = 0.4; // C12 (M42): the first neighbourhood branch at home costs 40 % of its fit-out

    FSite SiteOf(const FMarketState& State, const FString& Country, const FString& Province);
    FSite SiteOf(const FMarketState& State, const FMarketBranch& Branch);

    // E4 (M51, Docs/Kurgu/11_TEK_EKONOMI.md): the money of a store in Country on Day. A store abroad works in its
    // own country's money: its prices, goods, rent and small costs follow that country's list level (Goods, against
    // the campaign's own), its wages that country's wage index (Wages), and every amount reaches the till in the
    // campaign's money at today's exchange rate (Fx, MarketPrices::ToHome, 1 on day 1). At home all three are 1.
    struct FMoney
    {
        double Goods = 1.0;
        double Wages = 1.0;
        double Fx = 1.0;
        bool bForeign = false;
    };
    FMoney MoneyOf(const FMarketState& State, const FString& Country, int32 Day);
    // An amount computed with the campaign's own levels (a home price, cost or wage), earned or paid by such a store,
    // in the campaign's money: x Goods (bWages: x Wages) x Fx. Unchanged at home.
    int64 InHome(const FMoney& Money, int64 HomeLevelAmount, bool bWages = false);
    // The exchange difference of LocalAssets (local units) when the rate moves from FxBefore to FxAfter.
    int64 FxDifference(int64 LocalAssets, double FxBefore, double FxAfter);
    // A branch's assets abroad in its local money on Day (its goods at today's local cost and its deposit, which was
    // paid at the opening's rate); 0 at home.
    int64 LocalAssets(const FMarketState& State, const FMarketBranch& Branch, const TArray<FMarketProduct>& Products, int32 Day);
    // A branch's deposit in the campaign's money on Day (abroad it moves with the rate).
    int64 DepositInHome(const FMarketState& State, const FMarketBranch& Branch, int32 Day);
    // C12 (M42): the company's first branch (a neighbourhood shop in the home province) and a fit-out's price today
    // (x the store's size factor, x the difficulty, the first branch's discount).
    bool IsFirstBranch(const FMarketState& State, const FSite& Site, const FFormat& Kind);
    int64 FitOutCost(const FMarketState& State, const FSite& Site, const FFormat& Kind, float MeasureFactor);
    // Most shops of ours a province takes (the first store counts in the home province).
    int32 Room(const FSite& Site);
    // Our shops in a province that are not closed (+1 for the first store in the home province).
    int32 ShopsIn(const FMarketState& State, const FString& Country, const FString& Province);
    // Provinces of a country with a shop of ours (home first). Country empty = the campaign's.
    TArray<FString> ProvincesWithShops(const FMarketState& State, const FString& Country = FString());
    // The branch's country id (empty: the campaign's).
    FString CountryOf(const FMarketState& State, const FMarketBranch& Branch);

    // Menu command argument for a place and a type: (country index x 1000 + province index) x 10 + type index,
    // indexes into MarketCountry::All(), its Cities and FormatIds(). INDEX_NONE when unknown.
    int32 EncodeSite(const FString& Country, const FString& Province, const FString& Format);
    bool DecodeSite(int32 Arg, FString& OutCountry, FString& OutProvince, FString& OutFormat);

    // Money needed now to open: deposit (2 months' rent), fit-out and the opening stock at cost.
    int64 OpeningCost(const FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Country, const FString& Province, const FString& Format);
    // C3 (A istek): what the branch costs a month before it sells anything (rent, wages with the manager, the
    // employer's share, running costs); the opening question asks to keep at least this much in the till.
    int64 MonthlyFixedCost(const FMarketState& State, const FString& Country, const FString& Province, const FString& Format);
    // M33: the store manager's weekly clearance of slow items (the province manager approves deep cuts and takes
    // back mistakes). Returns the part of the weekly news line ("" nothing).
    FString Clearance(FMarketState& State, int32 BranchIndex, const TArray<FMarketProduct>& Products);
    // D9b (M46): what the portfolio's works (MarketPortfolio) share with an opening: the monthly rent the store would
    // sign today (its province, type and store view), its shelves planned again for its type (the goods on them
    // stay) and the goods that fill them at cost.
    int64 SignedRent(const FMarketState& State, const FMarketBranch& Branch);
    void ReplanShelves(const FMarketState& State, FMarketBranch& Branch, const TArray<FMarketProduct>& Products);
    int64 RestockCost(const FMarketBranch& Branch, const TArray<FMarketProduct>& Products);
    bool CanOpen(const FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Country, const FString& Province, const FString& Format, FString& OutReason);
    bool Open(FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Country, const FString& Province, const FString& Format, FString& OutMessage);
    bool Close(FMarketState& State, const TArray<FMarketProduct>& Products, int32 BranchIndex, FString& OutMessage);
    // Akis C2b: a store taken over with a chain we bought opens at once as our branch (its shelves stocked, a
    // manager hired, the district already half used to it). INDEX_NONE when the province has no room.
    int32 AddAcquired(FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Country, const FString& Province, const FString& Format);
    // Moves a person from the first store to run a branch (a cashier or stocker promoted).
    bool Promote(FMarketState& State, int32 EmployeeId, int32 BranchIndex, FString& OutMessage);
    int32 OpenCount(const FMarketState& State);
    // x shoppers of the first store: our branches in the home province take some of its customers.
    float MainShopFactor(const FMarketState& State);
    FString Summary(const FMarketState& State, int32 BranchIndex, const TArray<FMarketProduct>& Products);
    // Weekly mark of a branch (A best .. D worst; "-" while it is not open for a week).
    FString Grade(const FMarketState& State, int32 BranchIndex);
    // C3 (A4 visit): the player walks through an open branch. The manager's skill shows for 60 days, a first visit
    // in a month lifts the manager a little, and the walk tells what the numbers hide (empty shelves, queue,
    // the till). No money moves.
    constexpr int32 VisitSeenDays = 60;
    bool Visit(FMarketState& State, const TArray<FMarketProduct>& Products, int32 BranchIndex, FString& OutMessage);
    bool RecentlyVisited(const FMarketState& State, int32 BranchIndex);

    // Day close: stages, the simulated day of every open branch, the managers' orders, weekly lines.
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);
}

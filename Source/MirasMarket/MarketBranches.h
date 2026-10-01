#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Branches (G-068, rebuilt on provinces in G-086; Docs/Kurgu/03_MAGAZA_AGI.md). Independent of the world, tested
// (MirasMarket.Branches.*). The only place level of the game is the province (il / Land / state): a branch opens
// in a province of any country pack, as one of four market types, and goes lease and fit-out -> permits ->
// hiring -> opening stock -> open. Its shelves are planned automatically (MarketLayout) and its day is simulated
// from the same rules as the family shop, without walking customers: the province's shoppers split between us
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
        int32 Chapter = 0;               // hypermarket: from "Ulke Capinda" (5)
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
        bool bHome = false;              // the family shop's province
        bool bAbroad = false;
    };

    constexpr int32 RenovationDays = 5;
    constexpr int32 PermitDays = 3;
    constexpr int32 MaturityDays = 30;
    constexpr float UnitsPerShopper = 2.2f;
    constexpr int32 PeoplePerStoreK = 40;    // room for one of our shops per 40 000 people (at least 2)
    constexpr int32 PeoplePerSlotK = 80;     // above one shop per 80 000 people ours start to share customers

    const TArray<FString>& FormatIds();      // kucuk, mahalle, buyuk, hiper
    const FFormat& FormatInfo(const FString& Id);

    FSite SiteOf(const FMarketState& State, const FString& Country, const FString& Province);
    FSite SiteOf(const FMarketState& State, const FMarketBranch& Branch);
    // Most shops of ours a province takes (the family shop counts in the home province).
    int32 Room(const FSite& Site);
    // Our shops in a province that are not closed (+1 for the family shop in the home province).
    int32 ShopsIn(const FMarketState& State, const FString& Country, const FString& Province);
    // Provinces of a country with a shop of ours (home first). Country empty = the campaign's.
    TArray<FString> ProvincesWithShops(const FMarketState& State, const FString& Country = FString());
    // The branch's country id (older saves: the campaign's).
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
    bool CanOpen(const FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Country, const FString& Province, const FString& Format, FString& OutReason);
    bool Open(FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Country, const FString& Province, const FString& Format, FString& OutMessage);
    bool Close(FMarketState& State, const TArray<FMarketProduct>& Products, int32 BranchIndex, FString& OutMessage);
    // Akis C2b: a store taken over with a chain we bought opens at once as our branch (its shelves stocked, a
    // manager hired, the district already half used to it). INDEX_NONE when the province has no room.
    int32 AddAcquired(FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Country, const FString& Province, const FString& Format);
    // Moves a person from the family shop to run a branch (Cem's road in the story).
    bool Promote(FMarketState& State, int32 EmployeeId, int32 BranchIndex, FString& OutMessage);
    int32 OpenCount(const FMarketState& State);
    // x shoppers of the family shop: our branches in the home province take some of its customers.
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

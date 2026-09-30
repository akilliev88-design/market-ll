#pragma once

#include "CoreMinimal.h"

// Country packs (G-084, karar L02-L11, Config/ulkeler.json). Independent of the world, tested
// (MirasMarket.Country.*). The game keeps one internal money unit (the catalog's scale); a country shows it in its
// own currency (DisplayScale x the internal amount, its symbol and decimal mark) and brings its own economy
// character (inflation level and swings), traditional trade (corner shops, street market), fictional chain names,
// holidays, name pools and start cities. The active country is set when a campaign starts or loads; without a pack
// (or an unknown id) the game behaves exactly as the Turkish prototype.
namespace MarketCountry
{
    enum class ECharacter : uint8 { Stable = 0, Volatile, HighInflation };

    // A province (il, Land, state, UK region; G-086, Docs/Kurgu/03_MAGAZA_AGI.md \u00a71). The only place level
    // of the game: nothing below a province has a name. Income / rent / competition are 1 for the balancing
    // reference province (Turkey: Kirklareli) and derived from the population when the pack gives none.
    struct FCity
    {
        FString Id;
        FString Name;
        int32 PopulationK = 0;
        float Income = 1.f;
        float Rent = 1.f;
        float Competition = 1.f;
        FString SubRegion;       // alt bolge (bolge muduru): "trakya"
        FString Region;          // ana bolge (bolge direktoru): "marmara"
        float ChainDensity = -1.f; // chain stores per 100k people (Turkey: iller.json), -1 = unknown
        // G-089: the province's centre on the pack's map (Turkey: iller.json "cx"/"cy"; a pack may give "cx"/"cy"
        // per province). Distances are MapKm x the map units; without a map the depots use a region fallback.
        float MapX = 0.f;
        float MapY = 0.f;
        bool bOnMap = false;
    };

    // A main region (bolge direktoru) or a sub-region (bolge muduru). Sub-regions list their provinces.
    struct FRegion
    {
        FString Id;
        FString Name;
        FString Parent;          // sub-region: its main region
        float Income = 1.f;      // sub-region: purchasing power of its provinces
        TArray<FString> Provinces;
    };

    // How a holiday finds its date (G-084 3. parca). Turkey keeps the calendar's built-in list.
    enum class EHolidayRule : uint8 { Fixed = 0, Easter, Nth, Lunar };
    // National: a day off with a small bump. Feast: the eve is the big shopping day, the feast days are quiet
    // (Christmas, Easter, Thanksgiving; in Turkey the bayrams).
    enum class EHolidayKind : uint8 { National = 0, Feast };

    struct FHoliday
    {
        FString Name;
        int32 Month = 0;
        int32 Day = 0;
        bool bLunar = false;     // moves every year (e.g. Ramazan Bayram\u0131); the calendar computes it
        EHolidayRule Rule = EHolidayRule::Fixed;
        EHolidayKind Kind = EHolidayKind::National;
        int32 Days = 1;          // feast length
        int32 Offset = 0;        // days from the rule's date (Easter: 0 = Sunday)
        int32 Weekday = 0;       // Nth: 0 = Monday
        int32 Nth = 1;           // Nth: 1..4, 5 = the last one in the month
    };

    struct FProfile
    {
        FString Id = TEXT("tr");
        FString Name = TEXT("T\u00fcrkiye");
        FString NameEn = TEXT("Turkey");
        FString CurrencyCode = TEXT("TRY");
        FString CurrencySymbol = TEXT("TL");
        FString CurrencyName = TEXT("T\u00fcrk liras\u0131");
        bool bSymbolBefore = false;
        TCHAR DecimalMark = TEXT(',');
        double DisplayScale = 1.0;      // local money per internal unit
        double FxPerWorld = 1.5;        // local units per world unit at the start (league table)
        ECharacter Character = ECharacter::HighInflation;
        double InflationMean = 0.14;
        double InflationVol = 0.06;
        double LoanSpread = 0.05;
        float WageFactor = 1.f;
        float RentFactor = 1.f;
        float WeeklyShopShare = 0.25f;
        bool bSundayClosed = false;
        float CardShare = 0.35f;
        FString GrocerName;
        FString MarketName;
        int32 MarketWeekday = 1;        // 0 = Monday
        TMap<FString, FString> Chains;  // archetype id (bim, a101, sok, migros) -> fictional chain name
        TArray<FHoliday> Holidays;
        TArray<FString> FirstNames;
        TArray<FString> LastNames;
        TArray<FString> Relatives;      // who left the shop ("teyzen", "aunt"...)
        TArray<FCity> Cities;           // G-086: the provinces (every one a possible start and store place)
        FString CitiesFile;             // province/map file, e.g. "iller.json" ("provinces"/"cities" given as a file name)
        TArray<FRegion> Regions;        // main regions
        TArray<FRegion> SubRegions;     // sub-regions, each with its provinces
        int32 ReferencePopK = 500;      // population of the balancing reference province
        FString ReferenceProvince;      // e.g. "kirklareli": its population and chain density are the 1.0 point
        // G-089: kilometres per map unit ("kmPerMapUnit"). iller.json: 1.61, calibrated on Istanbul-Ankara (~350 km)
        // and Edirne-Kars (~1 380 km as the crow flies between the provinces' centres).
        float MapKm = 1.61f;
    };

    // Parses a country pack file (Config/ulkeler.json). Unknown fields are ignored; bad rows are reported.
    bool Parse(const FString& Json, TArray<FProfile>& OutProfiles, TArray<FString>& OutErrors);
    // All packs (loaded once); always contains "tr".
    const TArray<FProfile>& All();
    const FProfile* Find(const FString& Id);
    // Fills file-based provinces (iller.json), their regions and the derived income / rent / competition. Parse
    // calls it for every pack; public for tests.
    void Resolve(FProfile& Profile);
    // A province of a country (nullptr: unknown country or province).
    const FCity* FindCity(const FString& CountryId, const FString& CityId);
    // Sub-region / main region of a province (nullptr when the pack has none).
    const FRegion* SubRegionOf(const FString& CountryId, const FString& ProvinceId);
    const FRegion* RegionOf(const FString& CountryId, const FString& ProvinceId);
    // Sum of the provinces' population (thousands).
    int32 PopulationK(const FString& CountryId);
    // How crowded the city's grocery trade is: multiplies the rivals' pull in the share model (1 = the prototype).
    float CityCompetition(const FString& CountryId, const FString& CityId);
    // Purchasing power of the city (1 = the prototype): richer shoppers accept a little more on the price tag.
    float CityIncome(const FString& CountryId, const FString& CityId);
    // The country of the running campaign. SetActive also sets the economy of MarketPrices.
    const FProfile& Active();
    void SetActive(const FString& Id, int32 Seed);
    // Test hook: make a parsed profile active without the file.
    void SetActiveProfile(const FProfile& Profile, int32 Seed);

    // "12,50 TL", "4,50 \u20ac", "\u00a34.00", "$5.00": an internal amount in the active currency.
    FString Money(int64 Internal);
    // Decorates an already formatted Turkish-style number ("1.234,50" or "-12,50") with the active currency.
    FString Decorate(const FString& Number);
    // Chain name for an archetype in the active country ("" = keep the default).
    FString ChainName(const FString& Archetype);
}

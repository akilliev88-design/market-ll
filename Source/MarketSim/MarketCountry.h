#pragma once

#include "CoreMinimal.h"

struct FMarketState;

// Country packs (G-084, karar L02-L11, Config/ulkeler.json). Independent of the world, tested
// (MarketSim.Country.*). The game keeps one internal money unit (the catalog's scale); a country shows it in its
// own currency (DisplayScale x the internal amount, its symbol and decimal mark) and brings its own economy
// character (inflation level and swings), traditional trade (corner shops, street market), fictional chain names,
// holidays, name pools and start cities. The active country is set when a campaign starts or loads; an unknown id
// falls back to the default country (D3: Turkey is a pack like the others, every Turkish rule lives in its data).
namespace MarketCountry
{
    enum class ECharacter : uint8 { Stable = 0, Volatile, HighInflation };

    // A province (il, Land, state, UK region; G-086, Docs/Kurgu/03_MAGAZA_AGI.md \u00a71). The only place level
    // of the game: nothing below a province has a name. Income / rent / competition are derived from the
    // population (and chain density) against the country's median province when the pack gives none (M61b).
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
        // D3: a lunar feast names its date table ("table": "ramazan" | "kurban", MarketCalendar); without one a lunar
        // holiday is skipped.
        FString Table;
        int32 FastDays = 0;      // D3: the fasting month before the feast ("fastDays": 29 = Ramadan tag)
        bool bButcherPeak = false; // D3: the butcher's big week is the week before this feast ("butcherPeak")
    };

    // D3: a school day of the calendar (first day / report day), "calendar.school.start|end": the N-th weekday of a
    // month. Month 0 = the country has none.
    struct FSchoolDay
    {
        int32 Month = 0;
        int32 Weekday = 0;       // 0 = Monday
        int32 Nth = 1;
    };

    // D3: a national chain of the country (ulkeler.json "roster"; MarketChains seeds it). Archetype as text:
    // discount, fastDiscount, super, hyper, premium, regional, family, wholesale, club.
    struct FRosterRow
    {
        FString Id;
        FString Name;
        FString Boss;
        FString Archetype;
        int32 Stores = 0;
        float PriceIndex = 1.f;
        float Service = 1.f;
        float Aggression = 0.5f;
        float Ambition = 0.5f;
        FString HomeRegion;      // regional chains: the main region they start in ("" = everywhere)
    };

    // D3: a world giant of the league (ulkeler.json root "giants"; MarketChains seeds them once). Pack = its home
    // country's pack id ("" = a country without a pack); its home arm is the pack's national chain with the same
    // id or name.
    struct FGiantRow
    {
        FString Id;
        FString Name;
        FString Home;            // home country name for the menu
        FString Pack;
        FString Archetype;
        float RevenueB = 1.f;    // billions of world units a year at the start
        float Growth = 0.f;
    };

    struct FProfile
    {
        FString Id = TEXT("tr");
        FString Name = TEXT("T\u00fcrkiye");
        FString NameEn = TEXT("Turkey");
        FString Continent;              // D3/M51: "avrupa", "amerika", "asya"... (continent director, D4)
        // D7: the country's place on the schematic world map ("world": x 0..100, y 0..50, roughly geographic but
        // spread so names do not touch) and the side its name sits on ("below", "above", "left", "right"). x < 0: not drawn.
        float WorldX = -1.f;
        float WorldY = -1.f;
        FString WorldLabel = TEXT("below");
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
        // D3/Y1 (M60): "economy.curve": a hand-made price curve, [inflation, loan rate] per campaign year (karar A06;
        // MarketPrices), and "economy.curveAfter" for the years past it; a pack without one gets a curve generated
        // from its inflation mean and swings. bBuiltinCurve: the pack has a hand-made curve.
        bool bBuiltinCurve = false;
        TArray<double> CurveInflation;
        TArray<double> CurveLoan;
        double CurveAfterInflation = 0.08;
        double CurveAfterLoan = 0.14;
        float WageFactor = 1.f;
        // B1 (#45): what a person spends on groceries a day, internal kurus at the start price level ("economy":
        // "groceryPerPersonDay" in money units, e.g. 0.65). 0 = 65 x wageFactor. Calibrated so that a discounter
        // of the game (about 900 a day) holds what one store of the country's biggest chain holds in reality.
        double GroceryPerPersonDay = 0.0;
        // M65 (Mustafa 03.10.2026): a company of this country ("company": {...}). LegalForms: what follows the brand in
        // a company's registered name ("Gida Ticaret A.S.", "Handels GmbH"), the first is the default; SetupCost: what
        // founding one costs, internal kurus at the start level ("setupCost" in money units); DividendWithholding:
        // the tax on a month's profit sent to a parent company abroad ("dividendWithholding", 0..0.5).
        TArray<FString> LegalForms;
        // M54: store types the player may open here on top of the four everywhere ("playerFormats": "yakin" where
        // convenience stores are a market of their own, "toptan" where cash-and-carry is).
        TArray<FString> PlayerFormats;
        double SetupCost = 0.0;
        float DividendWithholding = 0.f;
        // B3 (#39): the employer's social security share on wages ("economy": "employerSocialRate"; Turkey 0.225 as
        // a game value) and the seniority pay per full year of service when a worker is let go ("severanceDaysPerYear";
        // Turkey 30 days' wage, 0 = none).
        float EmployerSocialRate = 0.225f;
        int32 SeveranceDaysPerYear = 30;
        float RentFactor = 1.f;
        float WeeklyShopShare = 0.25f;
        bool bSundayClosed = false;
        float CardShare = 0.35f;
        // D3: the country's own card use by year from the start year ("habits.cardShareByYear"); without it the card
        // share starts at CardShare and follows the world trend (MarketPayments).
        TArray<float> CardShareByYear;
        // D3: the calendar ("calendar"): show the holiday's own name ("Weihnachten") instead of "bayram"/"resmi
        // tatil" (showHolidayNames, default true), school start and report day.
        bool bShowHolidayNames = true;
        FSchoolDay SchoolStart;
        FSchoolDay SchoolEnd;
        // D3: the menu shows the real chain names in this country unless the brand switch asks for fictional ones
        // ("realChainNames"; Turkey, karar A12). Otherwise the pack's "chains" names are always used.
        bool bRealChainNames = false;
        TArray<FRosterRow> Roster;      // D3: national chains ("roster")
        TArray<FString> RegionalSuffixes; // D3: regional chain names = sub-region + one of these ("regionalSuffixes")
        TArray<FString> LocalSuffixes;  // E2: local family chains = a family name + one of these ("localSuffixes")
        FString WholesaleWord;          // D3: "firmWords.wholesale": the wholesaler's firm word ("G\u0131da Da\u011f\u0131t\u0131m")
        FString CashCarryWord;          // D3: "firmWords.cashCarry" ("Toptan", "Gro\u00dfmarkt")
        FString GrocerName;
        FString MarketName;
        int32 MarketWeekday = 1;        // 0 = Monday
        TMap<FString, FString> Chains;  // archetype id (bim, a101, sok, migros) -> fictional chain name
        TArray<FHoliday> Holidays;
        TArray<FString> FirstNames;
        TArray<FString> LastNames;
        // D3: own pools for shop staff ("names.staff") and managers ("names.managers"); empty = FirstNames/LastNames.
        TArray<FString> StaffFirst;
        TArray<FString> StaffLast;
        TArray<FString> ManagerFirst;
        TArray<FString> ManagerLast;
        TArray<FString> Banks;          // M30: local, commercial, investment, development (ulkeler.json "banks")
        FString PlatformName;           // M32: the country's fast-delivery platform (ulkeler.json "online.platform")
        float OnlinePlateau = 0.08f;    // M32: share of grocery bought online once it settles ("online.plateau")
        TArray<int32> ClimateTemperature; // M35: monthly mean temperature, January .. December ("climate.temperature")
        TArray<int32> ClimateRain;        // M35: chance of rain a day, percent ("climate.rain")
        TArray<FCity> Cities;           // G-086: the provinces (every one a possible start and store place)
        FString CitiesFile;             // province/map file, e.g. "iller.json" ("provinces"/"cities" given as a file name)
        TArray<FRegion> Regions;        // main regions
        TArray<FRegion> SubRegions;     // sub-regions, each with its provinces
        // M61b (Mustafa 03.10.2026: no province is special): the 1.0 point of the derived province values is the
        // country's median province, computed from the pack (Resolve): the median population and the median chain
        // density of its provinces. MedianProvince: the province of median population (ties: by id), the fallback
        // start of automated runs.
        int32 MedianPopK = 500;
        float MedianDensity = -1.f;
        FString MedianProvince;
        // G-089: kilometres per map unit ("kmPerMapUnit"). iller.json: 1.61, calibrated on Istanbul-Ankara (~350 km)
        // and Edirne-Kars (~1 380 km as the crow flies between the provinces' centres).
        float MapKm = 1.61f;
    };

    // Parses a country pack file (Config/ulkeler.json). Unknown fields are ignored; bad rows are reported.
    bool Parse(const FString& Json, TArray<FProfile>& OutProfiles, TArray<FString>& OutErrors);
    // D3: the world giants of a pack file (root "giants").
    bool ParseGiants(const FString& Json, TArray<FGiantRow>& OutGiants, TArray<FString>& OutErrors);
    // The world giants of Config/ulkeler.json (loaded with All()).
    const TArray<FGiantRow>& Giants();
    // D4: the continents of the packs in file order, a country's continent ("" unknown) and a continent's name
    // (ulkeler.json root "continents", else the id with a capital letter).
    TArray<FString> Continents();
    FString ContinentOf(const FString& Country);
    FString ContinentName(const FString& Id);
    // All packs (loaded once); always contains the default country.
    const TArray<FProfile>& All();
    // D3: the default country (ulkeler.json "defaultCountry", "tr"): automated runs, an unknown id.
    const FString& DefaultId();
    // D3: the default country's pack (never null: the bare prototype when the file has none).
    const FProfile& Default();
    // D3: the pack of a country or the default one (never null).
    const FProfile& FindOrDefault(const FString& Id);
    // D3: the country's provinces have the drawn map of the main screen (MarketMapData, iller.json).
    bool HasMap(const FString& Id);
    // D3: the country whose provinces are drawn on the map ("" = none).
    const FString& MapCountry();
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
    // The country's median province population (thousands; 500 for an unknown country): the 1.0 point of a
    // province's size (more trips per shop in denser places).
    int32 MedianPopK(const FString& CountryId);
    // How crowded the city's grocery trade is: multiplies the rivals' pull in the share model (1 = the country's median province).
    float CityCompetition(const FString& CountryId, const FString& CityId);
    // Purchasing power of the city (1 = average): richer shoppers accept a little more on the price tag.
    float CityIncome(const FString& CountryId, const FString& CityId);
    // The country of the running campaign (the default country until a campaign sets one). SetActive also sets the
    // economy of MarketPrices.
    const FProfile& Active();
    void SetActive(const FString& Id, int32 Seed);
    // Test hook: make a parsed profile active without the file.
    void SetActiveProfile(const FProfile& Profile, int32 Seed);
    // M30: the running campaign's seed (MarketCast names its people and firms from it).
    int32 ActiveSeed();

    // "12,50 TL", "4,50 \u20ac", "\u00a34.00", "$5.00": an internal amount in the active currency.
    FString Money(int64 Internal);
    // Decorates an already formatted Turkish-style number ("1.234,50" or "-12,50") with the active currency.
    FString Decorate(const FString& Number);
    // Chain name for an archetype in the active country ("" = keep the default).
    FString ChainName(const FString& Archetype);

    // B5 (\u00f6neri L08): real currency names, fictional rates. Local money per "world unit" (d\u00fcnya birimi, the common
    // unit of the world league, MarketChains) in a country on a game day: the pack's fxPerWorld at the start, then
    // the country's inflation against the world's (2.5 % a year; the campaign's own country uses its price curve
    // with the eras), a seeded random walk sized by the economy character (stable 0.03, volatile 0.08, high
    // inflation 0.06 a year) and, in the campaign's own country, about +25 % x strength over ten days in a currency
    // shock era (MarketEras).
    double FxRate(const FMarketState& State, const FString& CountryId, int32 GameDay);
    // An internal amount earned in a country -> world units x 100 (like kurus).
    int64 ToWorld(const FMarketState& State, const FString& CountryId, int64 Internal, int32 GameDay);
}

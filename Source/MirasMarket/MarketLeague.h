#pragma once

#include "CoreMinimal.h"
#include "MarketLeague.generated.h"

struct FMarketState;

// Ak\u0131\u015f B5 (A7, karar J02, \u00f6neri L08): the world retail league and the currencies. Independent of the world,
// tested (MirasMarket.League.*).
//
// Currencies (L08): real currency names, fictional rates. Every country's money moves against one common "world
// unit" (d\u00fcnya birimi): by its inflation against the world's (2.5 % a year), a seeded random walk whose size
// follows the economy character (stable / volatile / high inflation) and a jump when a currency shock era hits
// the campaign's own country (MarketEras). The league compares everybody in world units.
//
// The league: eight fictional retail giants (names close to the real ones, karar L12) with their own seeded growth,
// and the player's company (all shops, every country, converted day by day). A league year is a game year
// ("N. y\u0131l", 365 days from the campaign's first day). Karar J02: the "Miras" finale comes when the company is the
// league's first in two league years in a row with a positive operating result (EBITDA > 0) and debt below three
// years of it (loans + wholesalers' bills < 3 x EBITDA), in chapter 7. The giants are shrunk by WorldScale so the
// top is reachable in a long campaign ("d\u00fcnya \u00f6l\u00e7e\u011fi", Mustafa decides; the bot reports the time it takes).
USTRUCT()
struct FMarketLeague
{
    GENERATED_BODY()
    UPROPERTY() int32 LeagueYear = 0;        // running league year (0 = not started: an older save starts now)
    UPROPERTY() int32 YearDays = 0;          // days counted in the running year
    UPROPERTY() int64 YearRevenue = 0;       // world units x 100 (like kuru\u015f), running year
    UPROPERTY() int64 YearEbitda = 0;        // internal kuru\u015f, running year (from the books)
    UPROPERTY() int64 LastYearRevenue = 0;   // the last finished league year
    UPROPERTY() int32 LastYear = 0;
    UPROPERTY() int32 LastRank = 0;          // 0 = no finished year yet
    UPROPERTY() int32 BestRank = 0;
    UPROPERTY() int32 FirstPlaceYears = 0;   // finished years in a row as the first (with the money conditions)
};

namespace MarketLeague
{
    constexpr double WorldInflation = 0.025;
    // Real giants' revenue x WorldScale = league revenue (karar bekliyor: "d\u00fcnya \u00f6l\u00e7e\u011fi").
    constexpr double WorldScale = 1.0 / 2500.0;
    constexpr int32 YearDays = 365;
    constexpr int32 FinaleYears = 2;
    constexpr float DebtToEbitda = 3.f;

    // Local money per world unit in a country on a game day (the campaign's seed moves the walk).
    double FxRate(const FMarketState& State, const FString& CountryId, int32 GameDay);
    // An internal amount earned in a country -> world units x 100.
    int64 ToWorld(const FMarketState& State, const FString& CountryId, int64 Internal, int32 GameDay);

    struct FEntry
    {
        FString Name;
        FString Home;          // "ABD \u00b7 hipermarket"
        int64 Revenue = 0;     // world units x 100 a year
        bool bPlayer = false;
    };
    // Number of giants and one giant's revenue in a league year (world units x 100).
    int32 GiantCount();
    int64 GiantRevenue(const FMarketState& State, int32 Giant, int32 LeagueYear);
    // League year of a game day (1 = the first 365 days).
    int32 LeagueYearOf(int32 GameDay);
    // The table of a league year with our revenue, first place first.
    TArray<FEntry> Table(const FMarketState& State, int32 LeagueYear, int64 OurRevenue);
    // Today's view: the last finished year, or before it this year so far scaled to a full year.
    TArray<FEntry> CurrentTable(const FMarketState& State);
    // Our place in CurrentTable (1 = first).
    int32 CurrentRank(const FMarketState& State);
    // The money conditions of the finale (EBITDA > 0, debt < 3 x EBITDA) for a year's EBITDA.
    bool MoneyOk(const FMarketState& State, int64 YearEbitda);
    // One sentence for the menu: "D\u00fcnya ligi: 7. s\u0131ra; birincinin %4'\u00fc kadar ciro."
    FString Summary(const FMarketState& State);

    // Day close (Ak\u0131\u015f B block, after the books): counts the day, closes the league year (rank, news, finale).
    void CloseDay(FMarketState& State);
}

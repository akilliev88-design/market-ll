#pragma once

#include "CoreMinimal.h"

// Long-term economy of the country (Docs/Kurgu/00_KURGU_KITABI.md \u00a72): consumer price inflation, the minimum wage
// and the loan interest rate by calendar year. The curve is the game's own (karar A06): it has the
// shape of 2011-2029 Turkey but softer peaks; the minimum wage follows prices with a little real growth. Independent of the world.
// M24 (Mustafa, 30.09.2026): the game has its own economy. "2011" in names here (Kurus2011, CatalogBase) only means
// the price level at the start of a campaign (year 1 of MarketCalendar); no calendar year is shown to the player.
namespace MarketPrices
{
    // Yearly consumer price inflation of a calendar year, e.g. 0.104 for 2011.
    double YearlyInflation(int32 Year);
    // Daily growth factor on a game day (1 + daily rate), so that 365 days compound to the year's inflation.
    double DailyGrowth(int32 GameDay);
    // Minimum wage (net, monthly TL) in force on a game day, and relative to the start (first half of 2011 = 1).
    double MinimumWage(int32 GameDay);
    double WageIndex(int32 GameDay);
    // Price level on a game day relative to day 1 (compounded daily through the years).
    double PriceLevel(int32 GameDay);
    // The level of the price lists in force: wholesalers, utilities and rival shelf prices change on the 1st of a
    // month (the "zam listesi"). March 2011 = 1.
    double ListLevel(int32 GameDay);
    // Yearly commercial loan interest of the year the game day is in (for MarketFinance).
    double LoanRate(int32 GameDay);
    // G-077 (#36): a fixed start-level amount at the price level of the lists in force / at the wage level of the day.
    int64 Scaled(int64 Kurus2011, int32 GameDay);
    int64 WageScaled(int64 Kurus2011, int32 GameDay);

    // G-084 (karar L04): a country pack brings its own economy instead of the Turkish curve. Inflation of a year =
    // mean + volatility x a seeded wobble, now and then a shock year for swingy economies; loan rate = inflation +
    // spread. ClearEconomy returns to the built-in curve (Turkey, tests).
    void SetEconomy(double InflationMean, double InflationVol, double LoanSpread, bool bShocks, int32 Seed);
    void ClearEconomy();
    bool HasCustomEconomy();
}

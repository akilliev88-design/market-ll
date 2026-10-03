#pragma once

#include "CoreMinimal.h"

struct FMarketState;

// Long-term economy of the country (Docs/Kurgu/00_KURGU_KITABI.md \u00a72): consumer price inflation, the minimum wage
// and the loan interest rate by year. A hand-made curve is pack data (karar A06; Y1: "economy.curve" by campaign
// year), any other pack's is generated; the minimum wage follows prices with a little real growth. Independent of
// the world. M24/Y1 (M60): the game has its own economy and no real year. "Start" amounts (KurusStart, CatalogBase)
// are at the price level of the campaign's first day. The Year arguments below are years of the internal calendar
// (MarketCalendar::DateOf(...).Year); rules are written in campaign years (MarketCalendar::CampaignYear).
namespace MarketPrices
{
    // Yearly consumer price inflation of a year of the internal calendar (campaign year 1 = StartYear).
    double YearlyInflation(int32 Year);
    // Daily growth factor on a game day (1 + daily rate), so that 365 days compound to the year's inflation.
    double DailyGrowth(int32 GameDay);
    // Minimum wage (net, monthly TL) in force on a game day, and relative to the start (the first half of the first year = 1).
    double MinimumWage(int32 GameDay);
    double WageIndex(int32 GameDay);
    // Price level on a game day relative to day 1 (compounded daily through the years).
    double PriceLevel(int32 GameDay);
    // The level of the price lists in force: wholesalers, utilities and rival shelf prices change on the 1st of a
    // month (the "zam listesi"). The first day = 1.
    double ListLevel(int32 GameDay);
    // Yearly commercial loan interest of the year the game day is in (for MarketFinance).
    double LoanRate(int32 GameDay);
    // G-077 (#36): a fixed start-level amount at the price level of the lists in force / at the wage level of the day.
    int64 Scaled(int64 KurusStart, int32 GameDay);
    int64 WageScaled(int64 KurusStart, int32 GameDay);

    // G-084 (karar L04): a country pack brings its own economy instead of the Turkish curve. Inflation of a year =
    // mean + volatility x a seeded wobble, now and then a shock year for swingy economies; loan rate = inflation +
    // spread. ClearEconomy returns to the built-in curve (Turkey, tests).
    void SetEconomy(double InflationMean, double InflationVol, double LoanSpread, bool bShocks, int32 Seed);
    void ClearEconomy();
    bool HasCustomEconomy();

    // E1 (M51, Docs/Kurgu/11_TEK_EKONOMI.md): every country has its own economy. Country "" or the campaign's own
    // country (MarketCountry::Active) = the functions above (its curve, its eras). Any other country: its pack's
    // economy (mean, volatility, shocks for swingy economies, loan spread), seeded by the campaign and the country.
    // The single-argument functions above mean "the campaign's country" and are kept for the code not yet moved.
    bool IsHome(const FString& Country);
    double YearlyInflation(const FString& Country, int32 Year);
    double PriceLevel(const FString& Country, int32 GameDay);
    double ListLevel(const FString& Country, int32 GameDay);
    double WageIndex(const FString& Country, int32 GameDay);
    double LoanRate(const FString& Country, int32 GameDay);
    int64 Scaled(const FString& Country, int64 KurusStart, int32 GameDay);
    int64 WageScaled(const FString& Country, int64 KurusStart, int32 GameDay);
    // E1: a local amount of a country in the campaign's own money on a game day, through the exchange rates
    // (MarketCountry::FxRate). 1 at the start and for the own country; drifts with the inflation gap and the
    // currency's wobble. E4 converts a foreign store's day with it.
    double ToHome(const FMarketState& State, const FString& Country, int32 GameDay);
    // E1: the same in real terms (ToHome x the country's price level / the own one): 1 when the exchange rate only
    // followed the inflation gap; above or below 1 is the currency risk of a foreign store.
    double RealToHome(const FMarketState& State, const FString& Country, int32 GameDay);
}

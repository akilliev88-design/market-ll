#pragma once

#include "CoreMinimal.h"

// Long-term economy of the country (Docs/Kurgu/00_KURGU_KITABI.md \u00a72): consumer price inflation, the minimum wage
// and the loan interest rate by calendar year. 2011-2024 follow the published yearly figures approximately (T\u00dc\u0130K
// CPI, net minimum wage, commercial loan rates); 2025 on is a declared fictional scenario. Independent of the world.
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
}

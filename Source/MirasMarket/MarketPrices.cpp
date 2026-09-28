#include "MarketPrices.h"
#include "MarketCalendar.h"

namespace MarketPrices
{
    struct FYearRates { int32 Year; double Inflation; double LoanRate; };
    // CPI (December to December, rounded) and a typical small-business loan rate. 2025+ is a fictional scenario.
    const FYearRates Rates[] =
    {
        { 2011, 0.104, 0.15 }, { 2012, 0.062, 0.15 }, { 2013, 0.074, 0.13 }, { 2014, 0.082, 0.14 }, { 2015, 0.088, 0.16 },
        { 2016, 0.085, 0.15 }, { 2017, 0.119, 0.17 }, { 2018, 0.203, 0.28 }, { 2019, 0.118, 0.22 }, { 2020, 0.146, 0.16 },
        { 2021, 0.361, 0.24 }, { 2022, 0.643, 0.30 }, { 2023, 0.648, 0.45 }, { 2024, 0.444, 0.55 },
        { 2025, 0.310, 0.45 }, { 2026, 0.250, 0.38 }, { 2027, 0.200, 0.30 }, { 2028, 0.160, 0.25 }, { 2029, 0.130, 0.22 },
    };
    constexpr double LaterInflation = 0.10;
    constexpr double LaterLoanRate = 0.18;

    // Net monthly minimum wage by half year: (year * 10 + half), TL.
    struct FWage { int32 Key; double Net; };
    const FWage Wages[] =
    {
        { 20111, 658.95 }, { 20112, 701.93 }, { 20121, 739.79 }, { 20122, 773.01 }, { 20131, 803.68 }, { 20132, 846.00 },
        { 20141, 846.00 }, { 20142, 891.03 }, { 20151, 949.07 }, { 20152, 1000.54 }, { 20161, 1300.99 }, { 20162, 1300.99 },
        { 20171, 1404.06 }, { 20172, 1404.06 }, { 20181, 1603.12 }, { 20182, 1603.12 }, { 20191, 2020.90 }, { 20192, 2020.90 },
        { 20201, 2324.71 }, { 20202, 2324.71 }, { 20211, 2825.90 }, { 20212, 2825.90 }, { 20221, 4253.40 }, { 20222, 5500.35 },
        { 20231, 8506.80 }, { 20232, 11402.32 }, { 20241, 17002.12 }, { 20242, 17002.12 }, { 20251, 22104.67 }, { 20252, 22104.67 },
    };

    const FYearRates* FindYear(int32 Year)
    {
        for (const FYearRates& R : Rates) if (R.Year == Year) return &R;
        return nullptr;
    }
}

double MarketPrices::YearlyInflation(int32 Year)
{
    if (const FYearRates* R = FindYear(Year)) return R->Inflation;
    return Year < 2011 ? 0.08 : LaterInflation;
}

double MarketPrices::DailyGrowth(int32 GameDay)
{
    return FMath::Pow(1.0 + YearlyInflation(MarketCalendar::DateOf(GameDay).Year), 1.0 / 365.0);
}

double MarketPrices::PriceLevel(int32 GameDay)
{
    // Whole years at their own rate, then the part of the current year; before day 1 the level is 1.
    if (GameDay <= 1) return 1.0;
    double Level = 1.0;
    int32 From = 1;
    while (From < GameDay)
    {
        const int32 Year = MarketCalendar::DateOf(From).Year;
        const int32 NextYear = MarketCalendar::GameDayOf(Year + 1, 1, 1);
        const int32 To = FMath::Min(GameDay, NextYear);
        Level *= FMath::Pow(1.0 + YearlyInflation(Year), (To - From) / 365.0);
        From = To;
    }
    return Level;
}

double MarketPrices::ListLevel(int32 GameDay)
{
    const MarketCalendar::FDate Date = MarketCalendar::DateOf(GameDay);
    return PriceLevel(FMath::Max(1, MarketCalendar::GameDayOf(Date.Year, Date.Month, 1)));
}

double MarketPrices::MinimumWage(int32 GameDay)
{
    const MarketCalendar::FDate Date = MarketCalendar::DateOf(GameDay);
    const int32 Key = Date.Year * 10 + (Date.Month <= 6 ? 1 : 2);
    double Last = Wages[0].Net;
    for (const FWage& W : Wages)
    {
        if (W.Key > Key) break;
        Last = W.Net;
    }
    // After the table the wage follows inflation half year by half year.
    const int32 LastKey = Wages[UE_ARRAY_COUNT(Wages) - 1].Key;
    if (Key > LastKey)
    {
        const int32 Halves = (Key / 10 - LastKey / 10) * 2 + (Key % 10) - (LastKey % 10);
        for (int32 H = 0; H < Halves; ++H) Last *= FMath::Sqrt(1.0 + LaterInflation);
    }
    return Last;
}

double MarketPrices::WageIndex(int32 GameDay)
{
    return MinimumWage(GameDay) / Wages[0].Net;
}

double MarketPrices::LoanRate(int32 GameDay)
{
    const int32 Year = MarketCalendar::DateOf(GameDay).Year;
    if (const FYearRates* R = FindYear(Year)) return R->LoanRate;
    return LaterLoanRate;
}

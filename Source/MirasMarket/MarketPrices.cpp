#include "MarketPrices.h"
#include "MarketCalendar.h"

namespace MarketPrices
{
    struct FYearRates { int32 Year; double Inflation; double LoanRate; };
    // The game's economy curve (karar A06, Mustafa 29.09.2026: fun over a one-to-one copy of history). It keeps the
    // shape a Turkish player recognises - calm early 2010s, a jolt in 2018, a hard stretch in 2021-2023, a slow
    // cool-down - but softer than the published figures (2022 is 30 %, not 64 %), so a player who knows the real
    // numbers cannot read the future off them. The loan rate stays a few points above inflation.
    const FYearRates Rates[] =
    {
        { 2011, 0.090, 0.15 }, { 2012, 0.070, 0.14 }, { 2013, 0.075, 0.13 }, { 2014, 0.080, 0.14 }, { 2015, 0.085, 0.15 },
        { 2016, 0.090, 0.15 }, { 2017, 0.110, 0.17 }, { 2018, 0.160, 0.24 }, { 2019, 0.120, 0.19 }, { 2020, 0.130, 0.17 },
        { 2021, 0.190, 0.24 }, { 2022, 0.300, 0.33 }, { 2023, 0.280, 0.32 }, { 2024, 0.220, 0.28 },
        { 2025, 0.170, 0.24 }, { 2026, 0.140, 0.21 }, { 2027, 0.120, 0.18 }, { 2028, 0.100, 0.16 }, { 2029, 0.090, 0.15 },
    };
    constexpr double LaterInflation = 0.08;
    constexpr double LaterLoanRate = 0.14;

    // Net monthly minimum wage of the first half of 2011 (TL). Afterwards it is raised every January and July to
    // the price level expected at the end of that half year, plus 1.5 % real growth a year: wages never fall behind
    // for long, and never run away from prices either.
    constexpr double StartWage = 658.95;
    constexpr double RealWageGrowth = 0.015;

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
    if (Date.Year == 2011 && Date.Month <= 6) return StartWage;
    // End of the running half year: 1 July or 1 January of the next year.
    const int32 HalfEnd = Date.Month <= 6 ? MarketCalendar::GameDayOf(Date.Year, 7, 1) : MarketCalendar::GameDayOf(Date.Year + 1, 1, 1);
    const int32 HalfStart = Date.Month <= 6 ? MarketCalendar::GameDayOf(Date.Year, 1, 1) : MarketCalendar::GameDayOf(Date.Year, 7, 1);
    const double Years = (HalfStart - 1) / 365.0;
    return StartWage * PriceLevel(HalfEnd) * FMath::Pow(1.0 + RealWageGrowth, Years);
}

double MarketPrices::WageIndex(int32 GameDay)
{
    return MinimumWage(GameDay) / StartWage;
}

double MarketPrices::LoanRate(int32 GameDay)
{
    const int32 Year = MarketCalendar::DateOf(GameDay).Year;
    if (const FYearRates* R = FindYear(Year)) return R->LoanRate;
    return LaterLoanRate;
}

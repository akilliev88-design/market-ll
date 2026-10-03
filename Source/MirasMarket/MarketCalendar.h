#pragma once

#include "CoreMinimal.h"
#include "MarketGoods.h"

namespace MarketCountry { struct FHoliday; }

// The campaign calendar (G-061, Docs/Kurgu/00_KURGU_KITABI.md \u00a72). Independent of the world, tested
// (MirasMarket.Calendar.*). Y1 (karar M60): the game has no real year. Inside, game day 1 sits on a fixed
// calendar date (StartYear/StartMonth/StartDayOfMonth, a Monday) only so weekdays, month lengths, Easter and the
// lunar feasts can be computed; that date is a tool and is never shown nor used as a rule. Rules and texts use the
// campaign's own year (CampaignYear: 1, 2, 3...; "3. y\u0131l"). A played day is a calendar day; weekends are days
// 6 and 7 of every week.
// Everything that makes a day different lives here: season, weather (deterministic per campaign seed), the pack's
// holidays and feasts, Ramadan, paydays and month end. The game asks three questions:
//  TrafficFactor  - how many shoppers come (x rival traffic in MarketDirector)
//  GroupFactor    - how much a demand group is wanted today (shopping list weights)
//  BudgetFactor   - how full the wallets are (basket size)
namespace MarketCalendar
{
    constexpr int32 StartYear = 2011;   // internal anchor of the calendar arithmetic only (Y1, M60)
    constexpr int32 StartMonth = 3;
    constexpr int32 StartDayOfMonth = 7;

    enum class ESeason : uint8 { Spring, Summer, Autumn, Winter };
    enum class EWeather : uint8 { Sunny, Cloudy, Rain, Snow, Hot };

    // Special days. A day can carry several.
    enum class ETag : uint8
    {
        Payday,          // 1st and 15th: pensions and salaries
        MonthEnd,        // last three days of a month: tight wallets
        NationalHoliday, // the pack's national days (Turkey: 1 Jan, 23 Apr, 1 May, 19 May, 30 Aug, 29 Oct)
        BayramEve,       // arife: the big shopping day
        Bayram,          // Ramazan / Kurban bayram\u0131 days: visits, quiet shop
        Ramadan,         // fasting month: sahur and iftar shopping
        MothersDay, FathersDay, Valentines, NewYearsEve, SchoolStart, SchoolEnd,
        Count
    };

    struct FDate
    {
        int32 Year = StartYear;
        int32 Month = StartMonth;   // 1..12
        int32 Day = StartDayOfMonth;
        int32 Weekday = 0;          // 0 = Monday .. 6 = Sunday
    };

    struct FDayInfo
    {
        FDate Date;
        ESeason Season = ESeason::Spring;
        EWeather Weather = EWeather::Sunny;
        int32 TemperatureC = 10;
        TArray<ETag> Tags;
        // G-084, D3: the holidays come from the country pack; their name ("Weihnachten") is shown instead of
        // "bayram" when the pack asks for it (calendar.showHolidayNames). Empty in Turkey.
        FString HolidayName;
        // Shops must stay closed today (e.g. Germany: Sundays and public holidays).
        bool bClosedByLaw = false;
        bool Has(ETag Tag) const { return Tags.Contains(Tag); }
    };

    FDate DateOf(int32 GameDay);
    // Y1 (M60): the campaign's own year of a game day (1 = the first) and the game day of a date in a campaign year
    // (month, day). Every yearly rule (economy curve, eras, the epidemic, the end of the story) is written in these.
    int32 CampaignYear(int32 GameDay);
    int32 GameDayOfCampaign(int32 CampaignYear, int32 Month, int32 Day);
    // Game day of a calendar date (may be < 1 for dates before the start).
    int32 GameDayOf(int32 Year, int32 Month, int32 Day);
    bool IsWeekend(int32 GameDay);
    ESeason SeasonOf(int32 Month);
    int32 DaysInMonth(int32 Year, int32 Month);
    // First day of Ramazan Bayram\u0131 / Kurban Bayram\u0131 in a year (month, day); real dates 2011-2025, astronomical
    // estimates after that.
    FDate RamazanBayrami(int32 Year);
    FDate KurbanBayrami(int32 Year);
    // Easter Sunday (Gregorian computus).
    FDate EasterSunday(int32 Year);
    // Day of month of the N-th weekday (0 = Monday) of a month; N = 5 means the last one.
    int32 NthWeekdayOf(int32 Year, int32 Month, int32 Weekday, int32 N);
    // D3: first day of a pack holiday in a year (offset included), MIN_int32 when it has no date that year (bad
    // row, lunar feast without a table).
    int32 HolidayStart(const MarketCountry::FHoliday& Holiday, int32 Year);
    // The active country's law keeps shops closed on this day (MarketCountry: sundayClosed).
    bool ClosedByLaw(int32 GameDay);

    FDayInfo Info(int32 GameDay, int32 Seed);

    // 0.5..1.6 x shoppers.
    float TrafficFactor(int32 GameDay, int32 Seed);
    // 0.2..3.0 x how often a group is on a shopping list.
    float GroupFactor(int32 GameDay, int32 Seed, MarketGoods::EGroup Group);
    float CategoryFactor(int32 GameDay, int32 Seed, const FString& Category);
    // 0.8..1.2 x basket size.
    float BudgetFactor(int32 GameDay);

    // "7 Mart, 1. y\u0131l Pazartesi"
    FString DateText(int32 GameDay);
    // "Mart, 1. y\u0131l"
    FString MonthText(int32 GameDay);
    // "7 Mart, 1. y\u0131l Pazartesi \u00b7 g\u00fcne\u015fli 11\u00b0C \u00b7 maa\u015f g\u00fcn\u00fc"
    FString Describe(int32 GameDay, int32 Seed);
    // Evening report: what tomorrow brings and what to stock ("" when nothing special).
    FString Forecast(int32 GameDay, int32 Seed);
    FString TagName(ETag Tag);
    FString WeatherName(EWeather Weather);
}

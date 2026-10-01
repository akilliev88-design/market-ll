#include "MarketCalendar.h"
#include "MarketCountry.h"

namespace MarketCalendar
{
    using MarketGoods::EGroup;

    uint32 CalendarMix(int32 Seed, int32 Day, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), static_cast<uint32>(Day), Salt };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }

    // Days since 1970-01-01 of a civil date (proleptic Gregorian; H. Hinnant's algorithm).
    int64 DaysFromCivil(int32 Year, int32 Month, int32 Day)
    {
        const int64 Y = Year - (Month <= 2 ? 1 : 0);
        const int64 Era = (Y >= 0 ? Y : Y - 399) / 400;
        const int64 YearOfEra = Y - Era * 400;
        const int64 DayOfYear = (153 * (Month + (Month > 2 ? -3 : 9)) + 2) / 5 + Day - 1;
        const int64 DayOfEra = YearOfEra * 365 + YearOfEra / 4 - YearOfEra / 100 + DayOfYear;
        return Era * 146097 + DayOfEra - 719468;
    }

    void CivilFromDays(int64 Days, int32& OutYear, int32& OutMonth, int32& OutDay)
    {
        Days += 719468;
        const int64 Era = (Days >= 0 ? Days : Days - 146096) / 146097;
        const int64 DayOfEra = Days - Era * 146097;
        const int64 YearOfEra = (DayOfEra - DayOfEra / 1460 + DayOfEra / 36524 - DayOfEra / 146096) / 365;
        const int64 DayOfYear = DayOfEra - (365 * YearOfEra + YearOfEra / 4 - YearOfEra / 100);
        const int64 MonthPart = (5 * DayOfYear + 2) / 153;
        OutDay = static_cast<int32>(DayOfYear - (153 * MonthPart + 2) / 5 + 1);
        OutMonth = static_cast<int32>(MonthPart < 10 ? MonthPart + 3 : MonthPart - 9);
        OutYear = static_cast<int32>(YearOfEra + Era * 400 + (OutMonth <= 2 ? 1 : 0));
    }

    int64 StartDays() { return DaysFromCivil(StartYear, StartMonth, StartDayOfMonth); }

    // First bayram day (month * 100 + day) for 2011..2033: Diyanet dates to 2025, astronomical estimates after.
    const int32 RamazanTable[] = { 830, 819, 808, 728, 717, 705, 625, 615, 604, 524, 513, 502, 421, 410, 330, 320, 309, 226, 214, 204, 124, 114, 102 };
    const int32 KurbanTable[] = { 1106, 1025, 1015, 1004, 924, 912, 901, 821, 811, 731, 720, 709, 628, 616, 606, 527, 516, 505, 424, 413, 402, 322, 311 };
    constexpr int32 TableFirstYear = 2011;
    constexpr double LunarYearDays = 354.367;

    FDate FromTable(const int32* Table, int32 Count, int32 Year)
    {
        FDate Date;
        const int32 Index = Year - TableFirstYear;
        if (Index >= 0 && Index < Count)
        {
            Date.Year = Year; Date.Month = Table[Index] / 100; Date.Day = Table[Index] % 100;
        }
        else
        {
            // Outside the table: step whole lunar years from the nearest table entry and take the one in Year.
            const int32 RefIndex = Index < 0 ? 0 : Count - 1;
            const int64 Ref = DaysFromCivil(TableFirstYear + RefIndex, Table[RefIndex] / 100, Table[RefIndex] % 100);
            const int64 YearStart = DaysFromCivil(Year, 1, 1);
            int64 Steps = static_cast<int64>(FMath::FloorToDouble((YearStart - Ref) / LunarYearDays));
            int64 Candidate = Ref + static_cast<int64>(FMath::RoundToDouble(Steps * LunarYearDays));
            while (Candidate < YearStart) Candidate = Ref + static_cast<int64>(FMath::RoundToDouble((++Steps) * LunarYearDays));
            CivilFromDays(Candidate, Date.Year, Date.Month, Date.Day);
        }
        Date.Weekday = static_cast<int32>(((DaysFromCivil(Date.Year, Date.Month, Date.Day) - StartDays()) % 7 + 7) % 7);
        return Date;
    }

    // Nth weekday (0 = Monday) of a month, 1-based N.
    int32 NthWeekday(int32 Year, int32 Month, int32 Weekday, int32 N)
    {
        const int32 FirstWeekday = static_cast<int32>(((DaysFromCivil(Year, Month, 1) - StartDays()) % 7 + 7) % 7);
        return 1 + (Weekday - FirstWeekday + 7) % 7 + (N - 1) * 7;
    }

    // M35: the fallback climate (Thrace) when a country pack gives none (ulkeler.json "climate").
    const int32 MeanTemperature[12] = { 4, 5, 8, 13, 18, 23, 25, 25, 20, 15, 10, 6 };
    const int32 RainChance[12] = { 35, 32, 30, 28, 25, 18, 10, 10, 15, 25, 32, 35 };

    float Noise(int32 Seed, int32 Day, uint32 Salt)
    {
        return static_cast<float>(CalendarMix(Seed, Day, Salt) % 2001u) / 1000.f - 1.f;
    }

    // Season x group demand (spring, summer, autumn, winter).
    float SeasonFactor(EGroup Group, ESeason Season)
    {
        static const float Table[static_cast<int32>(EGroup::Count)][4] =
        {
            { 1.00f, 1.05f, 1.00f, 0.98f }, // Dairy
            { 1.00f, 1.35f, 0.95f, 0.80f }, // Drinks
            { 1.00f, 0.85f, 1.05f, 1.25f }, // TeaCoffee
            { 1.00f, 0.85f, 1.05f, 1.15f }, // Sweets
            { 1.00f, 1.10f, 1.00f, 0.95f }, // Snacks
            { 1.00f, 0.90f, 1.05f, 1.12f }, // Staples
            { 1.00f, 0.95f, 1.10f, 1.05f }, // OilSauce (autumn: sal\u00e7a and winter stock)
            { 1.10f, 1.00f, 1.00f, 0.95f }, // Household (spring cleaning)
            { 1.00f, 1.10f, 1.00f, 0.95f }, // PersonalCare
            { 1.00f, 1.00f, 1.00f, 1.00f }, // Paper
            { 0.80f, 2.20f, 0.60f, 0.25f }, // IceCream
            { 1.00f, 1.00f, 1.00f, 1.00f }, // Other
        };
        return Table[FMath::Clamp(static_cast<int32>(Group), 0, static_cast<int32>(EGroup::Count) - 1)][static_cast<int32>(Season)];
    }

    const TCHAR* MonthNames[12] = { TEXT("Ocak"), TEXT("\u015eubat"), TEXT("Mart"), TEXT("Nisan"), TEXT("May\u0131s"), TEXT("Haziran"), TEXT("Temmuz"),
        TEXT("A\u011fustos"), TEXT("Eyl\u00fcl"), TEXT("Ekim"), TEXT("Kas\u0131m"), TEXT("Aral\u0131k") };
    const TCHAR* WeekdayNames[7] = { TEXT("Pazartesi"), TEXT("Sal\u0131"), TEXT("\u00c7ar\u015famba"), TEXT("Per\u015fembe"), TEXT("Cuma"), TEXT("Cumartesi"), TEXT("Pazar") };
}

MarketCalendar::FDate MarketCalendar::DateOf(int32 GameDay)
{
    FDate Date;
    CivilFromDays(StartDays() + GameDay - 1, Date.Year, Date.Month, Date.Day);
    Date.Weekday = ((GameDay - 1) % 7 + 7) % 7;
    return Date;
}

int32 MarketCalendar::GameDayOf(int32 Year, int32 Month, int32 Day)
{
    return static_cast<int32>(DaysFromCivil(Year, Month, Day) - StartDays()) + 1;
}

bool MarketCalendar::IsWeekend(int32 GameDay)
{
    return DateOf(GameDay).Weekday >= 5;
}

MarketCalendar::ESeason MarketCalendar::SeasonOf(int32 Month)
{
    if (Month >= 3 && Month <= 5) return ESeason::Spring;
    if (Month >= 6 && Month <= 8) return ESeason::Summer;
    if (Month >= 9 && Month <= 11) return ESeason::Autumn;
    return ESeason::Winter;
}

int32 MarketCalendar::DaysInMonth(int32 Year, int32 Month)
{
    const int32 NextYear = Month == 12 ? Year + 1 : Year;
    const int32 NextMonth = Month == 12 ? 1 : Month + 1;
    return static_cast<int32>(DaysFromCivil(NextYear, NextMonth, 1) - DaysFromCivil(Year, Month, 1));
}

MarketCalendar::FDate MarketCalendar::RamazanBayrami(int32 Year)
{
    return FromTable(RamazanTable, UE_ARRAY_COUNT(RamazanTable), Year);
}

MarketCalendar::FDate MarketCalendar::KurbanBayrami(int32 Year)
{
    return FromTable(KurbanTable, UE_ARRAY_COUNT(KurbanTable), Year);
}

MarketCalendar::FDate MarketCalendar::EasterSunday(int32 Year)
{
    // Anonymous Gregorian algorithm (Meeus/Jones/Butcher).
    const int32 A = Year % 19, B = Year / 100, C = Year % 100, D = B / 4, E = B % 4;
    const int32 F = (B + 8) / 25, G = (B - F + 1) / 3, H = (19 * A + B - D - G + 15) % 30;
    const int32 I = C / 4, K = C % 4, L = (32 + 2 * E + 2 * I - H - K) % 7, M = (A + 11 * H + 22 * L) / 451;
    FDate Out;
    Out.Year = Year;
    Out.Month = (H + L - 7 * M + 114) / 31;
    Out.Day = (H + L - 7 * M + 114) % 31 + 1;
    Out.Weekday = 6;
    return Out;
}

int32 MarketCalendar::NthWeekdayOf(int32 Year, int32 Month, int32 Weekday, int32 N)
{
    const int32 First = NthWeekday(Year, Month, Weekday, 1);
    if (N < 5) return First + (FMath::Max(N, 1) - 1) * 7;
    int32 Day = First;
    while (Day + 7 <= DaysInMonth(Year, Month)) Day += 7;
    return Day;
}

bool MarketCalendar::ClosedByLaw(int32 GameDay)
{
    return Info(GameDay, 0).bClosedByLaw;
}

namespace MarketCalendar
{
    // G-084 3. parca: a foreign country's holidays from its pack (Turkey keeps the built-in list below).
    void AddPackHolidays(FDayInfo& Out, int32 GameDay, const MarketCountry::FProfile& Country)
    {
        using MarketCountry::EHolidayRule;
        for (const MarketCountry::FHoliday& H : Country.Holidays)
        {
            if (H.Rule == EHolidayRule::Lunar) continue; // only the Turkish calendar knows the lunar feasts
            for (int32 Year = Out.Date.Year - 1; Year <= Out.Date.Year + 1; ++Year)
            {
                int32 Start = 0;
                if (H.Rule == EHolidayRule::Easter)
                {
                    const FDate E = EasterSunday(Year);
                    Start = GameDayOf(E.Year, E.Month, E.Day);
                }
                else if (H.Rule == EHolidayRule::Nth)
                {
                    if (H.Month < 1 || H.Month > 12) continue;
                    Start = GameDayOf(Year, H.Month, NthWeekdayOf(Year, H.Month, H.Weekday, H.Nth));
                }
                else
                {
                    if (H.Month < 1 || H.Month > 12 || H.Day < 1 || H.Day > DaysInMonth(Year, H.Month)) continue;
                    Start = GameDayOf(Year, H.Month, H.Day);
                }
                Start += H.Offset;
                if (H.Kind == MarketCountry::EHolidayKind::Feast)
                {
                    if (GameDay == Start - 1) { Out.Tags.AddUnique(ETag::BayramEve); Out.HolidayName = H.Name; }
                    if (GameDay >= Start && GameDay < Start + H.Days) { Out.Tags.AddUnique(ETag::Bayram); Out.HolidayName = H.Name; }
                }
                else if (GameDay == Start) { Out.Tags.AddUnique(ETag::NationalHoliday); Out.HolidayName = H.Name; }
            }
        }
    }
}

MarketCalendar::FDayInfo MarketCalendar::Info(int32 GameDay, int32 Seed)
{
    FDayInfo Out;
    Out.Date = DateOf(GameDay);
    const FDate& D = Out.Date;
    Out.Season = SeasonOf(D.Month);

    // Weather: monthly mean + smoothed daily noise; rain by month; snow when it is cold and wet.
    const float Smooth = (Noise(Seed, GameDay - 1, 11u) + 2.f * Noise(Seed, GameDay, 11u) + Noise(Seed, GameDay + 1, 11u)) / 4.f;
    const MarketCountry::FProfile& Pack = MarketCountry::Active(); // M35: the country's own climate
    const int32 Mean = Pack.ClimateTemperature.Num() == 12 ? Pack.ClimateTemperature[D.Month - 1] : MeanTemperature[D.Month - 1];
    const int32 Rain = Pack.ClimateRain.Num() == 12 ? Pack.ClimateRain[D.Month - 1] : RainChance[D.Month - 1];
    Out.TemperatureC = Mean + FMath::RoundToInt32(Smooth * 12.f);
    const bool bWet = static_cast<int32>(CalendarMix(Seed, GameDay, 12u) % 100u) < Rain;
    if (bWet) Out.Weather = Out.TemperatureC <= 1 ? EWeather::Snow : EWeather::Rain;
    else if (Out.TemperatureC >= 29) Out.Weather = EWeather::Hot;
    else Out.Weather = CalendarMix(Seed, GameDay, 13u) % 3u == 0u ? EWeather::Cloudy : EWeather::Sunny;

    const int32 MonthDays = DaysInMonth(D.Year, D.Month);
    if (D.Day == 1 || D.Day == 15) Out.Tags.Add(ETag::Payday);
    if (D.Day > MonthDays - 3) Out.Tags.Add(ETag::MonthEnd);
    const int32 Key = D.Month * 100 + D.Day;
    const MarketCountry::FProfile& Country = MarketCountry::Active();
    const bool bTurkey = Country.Id == TEXT("tr");
    if (bTurkey && (Key == 101 || Key == 423 || Key == 501 || Key == 519 || Key == 830 || Key == 1029)) Out.Tags.Add(ETag::NationalHoliday);
    if (Key == 214) Out.Tags.Add(ETag::Valentines);
    if (Key == 1231) Out.Tags.Add(ETag::NewYearsEve);
    if (D.Month == 5 && D.Day == NthWeekday(D.Year, 5, 6, 2)) Out.Tags.Add(ETag::MothersDay);
    if (D.Month == 6 && D.Day == NthWeekday(D.Year, 6, 6, 3)) Out.Tags.Add(ETag::FathersDay);
    if (bTurkey && D.Month == 9 && D.Day == NthWeekday(D.Year, 9, 0, 3)) Out.Tags.Add(ETag::SchoolStart);
    if (bTurkey && D.Month == 6 && D.Day == NthWeekday(D.Year, 6, 4, 2)) Out.Tags.Add(ETag::SchoolEnd);
    if (!bTurkey)
    {
        AddPackHolidays(Out, GameDay, Country);
        Out.bClosedByLaw = Country.bSundayClosed && (D.Weekday == 6 || Out.Has(ETag::NationalHoliday) || Out.Has(ETag::Bayram));
        return Out;
    }

    // Bayrams: arife the day before; Ramazan Bayram\u0131 3 days, Kurban Bayram\u0131 4 days; Ramadan = the 29 days before.
    // A bayram early in January can belong to last year's table entry, so look at this year and the next.
    const int32 Today = GameDay;
    for (int32 Year = D.Year - 1; Year <= D.Year + 1; ++Year)
    {
        const FDate Ramazan = RamazanBayrami(Year);
        const FDate Kurban = KurbanBayrami(Year);
        const int32 R = GameDayOf(Ramazan.Year, Ramazan.Month, Ramazan.Day);
        const int32 K = GameDayOf(Kurban.Year, Kurban.Month, Kurban.Day);
        if (Today >= R - 29 && Today <= R - 1) Out.Tags.AddUnique(ETag::Ramadan);
        if (Today == R - 1 || Today == K - 1) Out.Tags.AddUnique(ETag::BayramEve);
        if ((Today >= R && Today < R + 3) || (Today >= K && Today < K + 4)) Out.Tags.AddUnique(ETag::Bayram);
    }
    return Out;
}

float MarketCalendar::TrafficFactor(int32 GameDay, int32 Seed)
{
    const FDayInfo Day = Info(GameDay, Seed);
    static const float Weekday[7] = { 0.95f, 0.95f, 1.00f, 1.00f, 1.08f, 1.18f, 0.95f };
    float Factor = Weekday[Day.Date.Weekday];
    if (Day.Has(ETag::Payday)) Factor *= 1.10f;
    if (Day.Has(ETag::MonthEnd)) Factor *= 0.93f;
    if (Day.Weather == EWeather::Rain) Factor *= 0.88f;
    if (Day.Weather == EWeather::Snow) Factor *= 0.72f;
    if (Day.Weather == EWeather::Hot) Factor *= 0.97f;
    if (Day.Has(ETag::NationalHoliday)) Factor *= 1.05f;
    if (Day.Has(ETag::BayramEve)) Factor *= 1.35f;
    if (Day.Has(ETag::Bayram))
    {
        // The first bayram day is for visits; the shop is quiet. Later days pick up.
        const bool bFirst = !Info(GameDay - 1, Seed).Has(ETag::Bayram);
        Factor *= bFirst ? 0.55f : 0.80f;
    }
    if (Day.Has(ETag::Ramadan)) Factor *= 1.04f;
    if (Day.Has(ETag::NewYearsEve)) Factor *= 1.30f;
    if (Day.Has(ETag::SchoolStart)) Factor *= 1.08f;
    return FMath::Clamp(Factor, 0.5f, 1.6f);
}

float MarketCalendar::GroupFactor(int32 GameDay, int32 Seed, MarketGoods::EGroup Group)
{
    const FDayInfo Day = Info(GameDay, Seed);
    float Factor = SeasonFactor(Group, Day.Season);
    auto Boost = [&Factor, Group](EGroup Target, float Value) { if (Group == Target) Factor *= Value; };
    if (Day.Weather == EWeather::Hot)
    {
        Boost(EGroup::Drinks, 1.25f); Boost(EGroup::IceCream, 1.5f); Boost(EGroup::Dairy, 1.05f); Boost(EGroup::TeaCoffee, 0.9f);
    }
    if (Day.TemperatureC <= 4)
    {
        Boost(EGroup::TeaCoffee, 1.2f); Boost(EGroup::Staples, 1.1f); Boost(EGroup::IceCream, 0.5f); Boost(EGroup::Drinks, 0.9f);
    }
    if (Day.Weather == EWeather::Rain || Day.Weather == EWeather::Snow) Boost(EGroup::Snacks, 1.05f);
    if (Day.Has(ETag::BayramEve))
    {
        Boost(EGroup::Sweets, 1.8f); Boost(EGroup::Drinks, 1.4f); Boost(EGroup::TeaCoffee, 1.2f); Boost(EGroup::Household, 1.2f); Boost(EGroup::OilSauce, 1.15f);
    }
    if (Day.Has(ETag::Bayram)) { Boost(EGroup::Sweets, 1.3f); Boost(EGroup::Drinks, 1.15f); }
    if (Day.Has(ETag::Ramadan))
    {
        Boost(EGroup::Dairy, 1.15f); Boost(EGroup::Staples, 1.15f); Boost(EGroup::TeaCoffee, 1.1f); Boost(EGroup::OilSauce, 1.1f); Boost(EGroup::Sweets, 1.1f);
    }
    if (Day.Has(ETag::MothersDay)) { Boost(EGroup::Sweets, 1.4f); Boost(EGroup::PersonalCare, 1.3f); }
    if (Day.Has(ETag::FathersDay)) Boost(EGroup::PersonalCare, 1.2f);
    if (Day.Has(ETag::Valentines)) Boost(EGroup::Sweets, 1.5f);
    if (Day.Has(ETag::NewYearsEve)) { Boost(EGroup::Drinks, 1.6f); Boost(EGroup::Snacks, 1.6f); Boost(EGroup::Sweets, 1.3f); }
    if (Day.Has(ETag::SchoolStart)) { Boost(EGroup::Snacks, 1.3f); Boost(EGroup::Paper, 1.4f); Boost(EGroup::Sweets, 1.15f); }
    if (Day.Has(ETag::SchoolEnd)) { Boost(EGroup::IceCream, 1.2f); Boost(EGroup::Snacks, 1.2f); }
    if (Day.Has(ETag::NationalHoliday)) Boost(EGroup::Snacks, 1.15f);
    if (Day.Has(ETag::Payday)) { Boost(EGroup::Household, 1.15f); Boost(EGroup::PersonalCare, 1.15f); }
    if (Day.Has(ETag::MonthEnd)) { Boost(EGroup::Household, 0.85f); Boost(EGroup::PersonalCare, 0.85f); Boost(EGroup::Staples, 1.05f); }
    return FMath::Clamp(Factor, 0.2f, 3.f);
}

float MarketCalendar::CategoryFactor(int32 GameDay, int32 Seed, const FString& Category)
{
    return GroupFactor(GameDay, Seed, MarketGoods::Classify(Category));
}

float MarketCalendar::BudgetFactor(int32 GameDay)
{
    const FDayInfo Day = Info(GameDay, 0); // wallets do not depend on the weather
    float Factor = 1.f;
    if (Day.Has(ETag::Payday)) Factor *= 1.15f;
    if (Day.Has(ETag::MonthEnd)) Factor *= 0.85f;
    if (Day.Has(ETag::BayramEve)) Factor *= 1.2f;
    return FMath::Clamp(Factor, 0.8f, 1.2f);
}

FString MarketCalendar::DateText(int32 GameDay)
{
    const FDate D = DateOf(GameDay);
    // G-084 (karar L06): the year is the campaign's own ("3. y\u0131l"), not a calendar year.
    return FString::Printf(TEXT("%d %s, %d. y\u0131l %s"), D.Day, MonthNames[D.Month - 1], D.Year - StartYear + 1, WeekdayNames[D.Weekday]);
}

FString MarketCalendar::MonthText(int32 GameDay)
{
    const FDate D = DateOf(GameDay);
    return FString::Printf(TEXT("%s, %d. y\u0131l"), MonthNames[D.Month - 1], D.Year - StartYear + 1);
}

FString MarketCalendar::TagName(ETag Tag)
{
    switch (Tag)
    {
    case ETag::Payday: return TEXT("maa\u015f g\u00fcn\u00fc");
    case ETag::MonthEnd: return TEXT("ay sonu");
    case ETag::NationalHoliday: return TEXT("resmi tatil");
    case ETag::BayramEve: return TEXT("bayram arifesi");
    case ETag::Bayram: return TEXT("bayram");
    case ETag::Ramadan: return TEXT("Ramazan");
    case ETag::MothersDay: return TEXT("Anneler G\u00fcn\u00fc");
    case ETag::FathersDay: return TEXT("Babalar G\u00fcn\u00fc");
    case ETag::Valentines: return TEXT("Sevgililer G\u00fcn\u00fc");
    case ETag::NewYearsEve: return TEXT("y\u0131lba\u015f\u0131 gecesi");
    case ETag::SchoolStart: return TEXT("okullar\u0131n ilk g\u00fcn\u00fc");
    case ETag::SchoolEnd: return TEXT("karne g\u00fcn\u00fc");
    default: return FString();
    }
}

FString MarketCalendar::WeatherName(EWeather Weather)
{
    switch (Weather)
    {
    case EWeather::Sunny: return TEXT("g\u00fcne\u015fli");
    case EWeather::Cloudy: return TEXT("bulutlu");
    case EWeather::Rain: return TEXT("ya\u011fmurlu");
    case EWeather::Snow: return TEXT("karl\u0131");
    default: return TEXT("\u00e7ok s\u0131cak");
    }
}

FString MarketCalendar::Describe(int32 GameDay, int32 Seed)
{
    const FDayInfo Day = Info(GameDay, Seed);
    FString Text = FString::Printf(TEXT("%s \u00b7 %s %d\u00b0C"), *DateText(GameDay), *WeatherName(Day.Weather), Day.TemperatureC);
    for (const ETag Tag : Day.Tags)
    {
        // G-084: a foreign holiday shows its own name ("Weihnachten", "Thanksgiving arifesi").
        if (!Day.HolidayName.IsEmpty() && (Tag == ETag::Bayram || Tag == ETag::NationalHoliday)) Text += TEXT(" \u00b7 ") + Day.HolidayName;
        else if (!Day.HolidayName.IsEmpty() && Tag == ETag::BayramEve) Text += TEXT(" \u00b7 ") + Day.HolidayName + TEXT(" arifesi");
        else Text += TEXT(" \u00b7 ") + TagName(Tag);
    }
    if (Day.bClosedByLaw) Text += TEXT(" \u00b7 yasal tatil, d\u00fckk\u00e2nlar kapal\u0131");
    return Text;
}

FString MarketCalendar::Forecast(int32 GameDay, int32 Seed)
{
    FString Text = TEXT("Yar\u0131n ") + Describe(GameDay, Seed) + TEXT(".");
    const float Traffic = TrafficFactor(GameDay, Seed);
    if (Traffic >= 1.15f) Text += TEXT(" Kalabal\u0131k bekleniyor: kasa ve raflar haz\u0131r olsun.");
    else if (Traffic <= 0.8f) Text += TEXT(" Sakin bir g\u00fcn: az m\u00fc\u015fteri.");
    TArray<FString> Up, Down;
    for (int32 G = 0; G < static_cast<int32>(EGroup::Other); ++G)
    {
        const float Factor = GroupFactor(GameDay, Seed, static_cast<EGroup>(G));
        if (Factor >= 1.25f) Up.Add(FString::Printf(TEXT("%s x%.1f"), *MarketGoods::GroupName(static_cast<EGroup>(G)), Factor));
        else if (Factor <= 0.7f) Down.Add(MarketGoods::GroupName(static_cast<EGroup>(G)));
    }
    if (Up.Num() > 0) Text += TEXT(" \u00c7ok aranacak: ") + FString::Join(Up, TEXT(", ")) + TEXT(".");
    if (Down.Num() > 0) Text += TEXT(" Az aranacak: ") + FString::Join(Down, TEXT(", ")) + TEXT(".");
    return Text;
}

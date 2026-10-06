#include "MarketCalendar.h"
#include "MarketGoods.h"
#include "MarketDirector.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCalendarDatesTest, "MarketSim.Calendar.DatesAndHolidays", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCalendarDatesTest::RunTest(const FString& Parameters)
{
    using namespace MarketCalendar;
    const FDate First = DateOf(1);
    TestTrue(TEXT("Day 1 is Monday 7 March 2011"), First.Year == 2011 && First.Month == 3 && First.Day == 7 && First.Weekday == 0);
    TestTrue(TEXT("Days 6 and 7 are the weekend"), IsWeekend(6) && IsWeekend(7) && !IsWeekend(5) && !IsWeekend(8));
    TestEqual(TEXT("Round trip"), GameDayOf(2011, 3, 7), 1);
    // Y1 (M60): rules use the campaign's own year.
    TestEqual(TEXT("Day 1 is in year 1"), CampaignYear(1), 1);
    TestEqual(TEXT("Campaign date round trip"), GameDayOfCampaign(1, 3, 7), 1);
    TestEqual(TEXT("Year 10"), CampaignYear(GameDayOfCampaign(10, 3, 1)), 10);
    const FDate Leap = DateOf(GameDayOf(2012, 2, 29));
    TestTrue(TEXT("Leap day exists and is a Wednesday"), Leap.Month == 2 && Leap.Day == 29 && Leap.Weekday == 2);
    TestTrue(TEXT("New year"), DateOf(GameDayOf(2012, 1, 1)).Year == 2012);
    TestEqual(TEXT("February 2011"), DaysInMonth(2011, 2), 28);
    TestEqual(TEXT("February 2012"), DaysInMonth(2012, 2), 29);
    TestTrue(TEXT("Seasons"), SeasonOf(3) == ESeason::Spring && SeasonOf(7) == ESeason::Summer && SeasonOf(10) == ESeason::Autumn && SeasonOf(1) == ESeason::Winter);

    // 2011's real bayrams.
    const FDate Ramazan = RamazanBayrami(2011);
    const FDate Kurban = KurbanBayrami(2011);
    TestTrue(TEXT("Ramazan Bayrami 2011 = 30 August"), Ramazan.Month == 8 && Ramazan.Day == 30);
    TestTrue(TEXT("Kurban Bayrami 2011 = 6 November"), Kurban.Month == 11 && Kurban.Day == 6);
    const int32 Arife = GameDayOf(2011, 8, 29);
    TestTrue(TEXT("Arife"), Info(Arife, 1).Has(ETag::BayramEve) && Info(Arife, 1).Has(ETag::Ramadan));
    TestTrue(TEXT("Bayram three days"), Info(Arife + 1, 1).Has(ETag::Bayram) && Info(Arife + 3, 1).Has(ETag::Bayram) && !Info(Arife + 4, 1).Has(ETag::Bayram));
    TestTrue(TEXT("Ramadan"), Info(GameDayOf(2011, 8, 10), 1).Has(ETag::Ramadan) && !Info(GameDayOf(2011, 7, 20), 1).Has(ETag::Ramadan));
    TestTrue(TEXT("Kurban four days"), Info(GameDayOf(2011, 11, 9), 1).Has(ETag::Bayram) && !Info(GameDayOf(2011, 11, 10), 1).Has(ETag::Bayram));
    TestTrue(TEXT("Bayrams move about eleven days a year"), RamazanBayrami(2012).Month == 8 && RamazanBayrami(2012).Day == 19);
    const FDate Far = RamazanBayrami(2040);
    TestTrue(TEXT("Estimated beyond the table"), Far.Year == 2040);
    TestTrue(TEXT("Payday"), Info(GameDayOf(2011, 4, 1), 1).Has(ETag::Payday) && Info(GameDayOf(2011, 4, 15), 1).Has(ETag::Payday));
    TestTrue(TEXT("Month end"), Info(GameDayOf(2011, 3, 30), 1).Has(ETag::MonthEnd) && !Info(GameDayOf(2011, 3, 20), 1).Has(ETag::MonthEnd));
    TestTrue(TEXT("Mother's Day 2011 = 8 May"), Info(GameDayOf(2011, 5, 8), 1).Has(ETag::MothersDay));
    TestTrue(TEXT("Schools open 19 September 2011"), Info(GameDayOf(2011, 9, 19), 1).Has(ETag::SchoolStart));
    TestTrue(TEXT("23 Nisan"), Info(GameDayOf(2011, 4, 23), 1).Has(ETag::NationalHoliday));
    TestTrue(TEXT("Date text: the campaign's own year"), DateText(1).StartsWith(TEXT("7 Mart, 1. y\u0131l Pazartesi")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCalendarDemandTest, "MarketSim.Calendar.WeatherAndDemand", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCalendarDemandTest::RunTest(const FString& Parameters)
{
    using namespace MarketCalendar;
    using MarketGoods::EGroup;
    // Categories of the catalog map to their demand groups.
    TestTrue(TEXT("s\u00fct"), MarketGoods::Classify(TEXT("s\u00fct")) == EGroup::Dairy);
    TestTrue(TEXT("i\u00e7ecek"), MarketGoods::Classify(TEXT("i\u00e7ecek")) == EGroup::Drinks);
    TestTrue(TEXT("\u00e7ay-kahve"), MarketGoods::Classify(TEXT("\u00e7ay-kahve")) == EGroup::TeaCoffee);
    TestTrue(TEXT("bisk\u00fcvi-\u00e7ikolata"), MarketGoods::Classify(TEXT("bisk\u00fcvi-\u00e7ikolata")) == EGroup::Sweets);
    TestTrue(TEXT("makarna-bakliyat"), MarketGoods::Classify(TEXT("makarna-bakliyat")) == EGroup::Staples);
    TestTrue(TEXT("ya\u011f-sal\u00e7a"), MarketGoods::Classify(TEXT("ya\u011f-sal\u00e7a")) == EGroup::OilSauce);
    TestTrue(TEXT("temizlik"), MarketGoods::Classify(TEXT("temizlik")) == EGroup::Household);
    TestTrue(TEXT("ki\u015fisel bak\u0131m"), MarketGoods::Classify(TEXT("ki\u015fisel bak\u0131m")) == EGroup::PersonalCare);
    TestTrue(TEXT("ka\u011f\u0131t"), MarketGoods::Classify(TEXT("ka\u011f\u0131t")) == EGroup::Paper);
    TestTrue(TEXT("dondurma"), MarketGoods::Classify(TEXT("dondurma")) == EGroup::IceCream);
    TestTrue(TEXT("at\u0131\u015ft\u0131rmal\u0131k"), MarketGoods::Classify(TEXT("at\u0131\u015ft\u0131rmal\u0131k")) == EGroup::Snacks);
    TestTrue(TEXT("unknown"), MarketGoods::Classify(TEXT("oyuncak")) == EGroup::Other);

    // Deterministic weather; summer is warmer than winter.
    const int32 Seed = 2011;
    TestEqual(TEXT("Same day, same weather"), Info(40, Seed).TemperatureC, Info(40, Seed).TemperatureC);
    int32 July = 0, January = 0, Snow = 0, Hot = 0;
    for (int32 D = 1; D <= 31; ++D)
    {
        July += Info(GameDayOf(2011, 7, D), Seed).TemperatureC;
        January += Info(GameDayOf(2012, 1, D), Seed).TemperatureC;
    }
    for (int32 Day = 1; Day <= 3 * 365; ++Day)
    {
        const FDayInfo I = Info(Day, Seed);
        if (I.Weather == EWeather::Snow) { ++Snow; TestTrue(TEXT("Snow only in the cold"), I.TemperatureC <= 1); }
        if (I.Weather == EWeather::Hot) { ++Hot; TestTrue(TEXT("Heat only in the warm months"), I.Date.Month >= 5 && I.Date.Month <= 9); }
        TestTrue(TEXT("Traffic in range"), TrafficFactor(Day, Seed) >= 0.5f && TrafficFactor(Day, Seed) <= 1.6f);
    }
    TestTrue(TEXT("July is warmer than January"), July > January + 31 * 10);
    TestTrue(TEXT("Some snow and some heat in three years"), Snow > 0 && Hot > 0);

    // Demand follows the season and the special days.
    const int32 Summer = GameDayOf(2011, 7, 12), Winter = GameDayOf(2012, 1, 12);
    TestTrue(TEXT("Ice cream in summer"), GroupFactor(Summer, Seed, EGroup::IceCream) > 3.f * GroupFactor(Winter, Seed, EGroup::IceCream));
    TestTrue(TEXT("Tea in winter"), GroupFactor(Winter, Seed, EGroup::TeaCoffee) > GroupFactor(Summer, Seed, EGroup::TeaCoffee));
    const int32 Arife = GameDayOf(2011, 8, 29);
    TestTrue(TEXT("Sweets on the bayram eve"), GroupFactor(Arife, Seed, EGroup::Sweets) >= 1.5f);
    TestTrue(TEXT("Busy bayram eve"), TrafficFactor(Arife, Seed) > 1.15f);
    TestTrue(TEXT("Quiet first bayram day"), TrafficFactor(Arife + 1, Seed) < TrafficFactor(Arife + 2, Seed));
    TestTrue(TEXT("Payday wallets"), BudgetFactor(GameDayOf(2011, 4, 1)) > 1.f && BudgetFactor(GameDayOf(2011, 4, 29)) < 1.f);
    TestTrue(TEXT("Forecast names the bayram"), Forecast(Arife, Seed).Contains(TEXT("bayram arifesi")));
    TestTrue(TEXT("Category string works"), CategoryFactor(Summer, Seed, TEXT("dondurma")) == GroupFactor(Summer, Seed, EGroup::IceCream));

    // The order suggestion looks ahead: an order placed two days before the arife is sold on the arife.
    FMarketState S; S.Day = Arife - 1; S.RivalSeed = Seed;
    FMarketProduct Chocolate; Chocolate.Id = TEXT("choc"); Chocolate.Category = TEXT("bisk\u00fcvi-\u00e7ikolata");
    TestTrue(TEXT("Stock up before the bayram"), MarketDirector::OrderScale(S, Chocolate) > 1.3f);
    S.Day = Arife + 1;
    TestTrue(TEXT("Less after the bayram"), MarketDirector::OrderScale(S, Chocolate) < 1.f);
    return true;
}

#endif

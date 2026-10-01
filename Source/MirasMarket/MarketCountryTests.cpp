#include "MarketCountry.h"
#include "MarketEras.h"
#include "MarketEconomy.h"
#include "MarketPrices.h"
#include "MarketCalendar.h"
#include "MarketPayments.h"
#include "MarketSimulation.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCountryTest, "MirasMarket.Country.PacksCurrencyEconomy", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCountryTest::RunTest(const FString& Parameters)
{
    // G-084 (karar L02-L06): country packs, the active currency and a country's own economy.
    using namespace MarketCountry;
    const FString Json = TEXT("{\"countries\":[")
        TEXT("{\"id\":\"tr\",\"name\":\"Turkiye\",\"currency\":{\"code\":\"TRY\",\"symbol\":\"TL\",\"symbolBefore\":false,\"decimal\":\",\"},\"displayScale\":1,\"cities\":\"iller.json\"},")
        TEXT("{\"id\":\"gb\",\"name\":\"UK\",\"currency\":{\"code\":\"GBP\",\"symbol\":\"GBP\",\"symbolBefore\":true,\"decimal\":\".\"},\"displayScale\":0.5,")
        TEXT("\"economy\":{\"character\":\"istikrarli\",\"inflationMean\":0.02,\"inflationVol\":0.01,\"loanSpread\":0.03},")
        TEXT("\"traditional\":{\"grocer\":\"Corner shops\",\"market\":\"Farmers market\",\"marketWeekday\":5},\"chains\":{\"bim\":\"Alda\"},")
        TEXT("\"cities\":[{\"id\":\"york\",\"name\":\"York\",\"populationK\":210,\"income\":1.0}]}]}");
    TArray<FProfile> Profiles; TArray<FString> Errors;
    TestTrue(TEXT("Packs parse"), Parse(Json, Profiles, Errors));
    TestEqual(TEXT("Two countries"), Profiles.Num(), 2);
    if (Profiles.Num() != 2) return false;
    TestEqual(TEXT("Turkey uses the provinces file"), Profiles[0].CitiesFile, FString(TEXT("iller.json")));
    TestEqual(TEXT("UK has its own cities"), Profiles[1].Cities.Num(), 1);
    TestEqual(TEXT("Market day"), Profiles[1].MarketWeekday, 5);

    SetActiveProfile(Profiles[0], 1);
    TestEqual(TEXT("Lira with thousands"), Money(123450), FString(TEXT("1.234,50 TL")));
    TestEqual(TEXT("Negative lira"), Money(-250), FString(TEXT("-2,50 TL")));
    TestFalse(TEXT("Turkey keeps the built-in curve"), MarketPrices::HasCustomEconomy());
    const double TurkishLevel = MarketPrices::PriceLevel(MarketCalendar::GameDayOf(2020, 3, 7));

    SetActiveProfile(Profiles[1], 7);
    TestEqual(TEXT("Pound before, dot decimals, scaled"), Money(123450), FString(TEXT("GBP617.25")));
    TestEqual(TEXT("Negative pound"), Money(-250), FString(TEXT("-GBP1.25")));
    TestEqual(TEXT("Local chain name"), ChainName(TEXT("bim")), FString(TEXT("Alda")));
    TestTrue(TEXT("A stable economy of its own"), MarketPrices::HasCustomEconomy());
    const double StableLevel = MarketPrices::PriceLevel(MarketCalendar::GameDayOf(2020, 3, 7));
    TestTrue(TEXT("Stable prices rise slowly"), StableLevel > 1.0 && StableLevel < TurkishLevel && StableLevel < 1.6);
    TestTrue(TEXT("Loans above inflation"), MarketPrices::LoanRate(MarketCalendar::GameDayOf(2015, 5, 1)) > MarketPrices::YearlyInflation(2015));

    // Back to the default so the other tests see the Turkish prototype.
    SetActiveProfile(Profiles[0], 1);
    TestFalse(TEXT("Restored"), MarketPrices::HasCustomEconomy());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCountryHolidayTest, "MirasMarket.Country.HolidaysAndLaw", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCountryHolidayTest::RunTest(const FString& Parameters)
{
    // G-084 3. parca: a foreign country's holidays come from its pack; Germany keeps shops shut on Sundays.
    using namespace MarketCountry;
    using MarketCalendar::ETag;
    const FString Json = TEXT("{\"countries\":[")
        TEXT("{\"id\":\"de\",\"name\":\"Deutschland\",\"habits\":{\"sundayClosed\":true,\"cardShare\":0.45},\"holidays\":[")
        TEXT("{\"name\":\"Ostern\",\"rule\":\"easter\",\"kind\":\"feast\",\"days\":2},")
        TEXT("{\"name\":\"Tag der Deutschen Einheit\",\"month\":10,\"day\":3},")
        TEXT("{\"name\":\"Weihnachten\",\"month\":12,\"day\":25,\"kind\":\"feast\",\"days\":2}]},")
        TEXT("{\"id\":\"us\",\"name\":\"USA\",\"habits\":{\"cardShare\":0.75},\"holidays\":[")
        TEXT("{\"name\":\"Thanksgiving\",\"rule\":\"nth\",\"month\":11,\"weekday\":3,\"n\":4,\"kind\":\"feast\"},")
        TEXT("{\"name\":\"Memorial Day\",\"rule\":\"nth\",\"month\":5,\"weekday\":0,\"n\":5}]}]}");
    TArray<FProfile> Profiles; TArray<FString> Errors;
    TestTrue(TEXT("Packs parse"), Parse(Json, Profiles, Errors));
    if (Profiles.Num() < 2 || Profiles[0].Holidays.Num() < 3) return false;
    TestEqual(TEXT("Easter rule"), static_cast<uint8>(Profiles[0].Holidays[0].Rule), static_cast<uint8>(EHolidayRule::Easter));
    TestEqual(TEXT("Feast length"), Profiles[0].Holidays[2].Days, 2);

    const MarketCalendar::FDate Easter = MarketCalendar::EasterSunday(2024);
    TestTrue(TEXT("Easter 2024 = 31 March"), Easter.Month == 3 && Easter.Day == 31);
    TestEqual(TEXT("Last Monday of May 2012"), MarketCalendar::NthWeekdayOf(2012, 5, 0, 5), 28);

    const int32 Sunday = 7; // day 1 is a Monday
    const int32 TrCards = FMath::RoundToInt(MarketPayments::CardShare(Sunday) * 100.f);
    TestFalse(TEXT("Turkey: open on Sunday"), MarketCalendar::ClosedByLaw(Sunday));
    const float TrEveTraffic = MarketCalendar::TrafficFactor(MarketCalendar::GameDayOf(2012, 12, 24), 3); // same weather, no feast

    SetActiveProfile(Profiles[0], 3);
    const int32 Eve = MarketCalendar::GameDayOf(2012, 12, 24);
    const MarketCalendar::FDayInfo EveInfo = MarketCalendar::Info(Eve, 3);
    TestTrue(TEXT("Christmas Eve is the big day"), EveInfo.Has(ETag::BayramEve) && EveInfo.HolidayName == TEXT("Weihnachten"));
    TestFalse(TEXT("Christmas Eve is open"), EveInfo.bClosedByLaw);
    TestTrue(TEXT("Christmas is a feast"), MarketCalendar::Info(Eve + 1, 3).Has(ETag::Bayram) && MarketCalendar::Info(Eve + 2, 3).Has(ETag::Bayram));
    TestTrue(TEXT("Shops shut on Christmas"), MarketCalendar::ClosedByLaw(Eve + 1));
    TestTrue(TEXT("Shops shut on Sunday"), MarketCalendar::ClosedByLaw(Sunday));
    TestFalse(TEXT("Open on Monday"), MarketCalendar::ClosedByLaw(Sunday + 1));
    TestTrue(TEXT("Unity day"), MarketCalendar::Info(MarketCalendar::GameDayOf(2011, 10, 3), 3).Has(ETag::NationalHoliday));
    TestFalse(TEXT("No Turkish republic day"), MarketCalendar::Info(MarketCalendar::GameDayOf(2011, 10, 29), 3).Has(ETag::NationalHoliday));
    TestFalse(TEXT("No Ramadan"), MarketCalendar::Info(MarketCalendar::GameDayOf(2012, 8, 10), 3).Has(ETag::Ramadan));
    TestTrue(TEXT("Easter Monday"), MarketCalendar::Info(MarketCalendar::GameDayOf(2012, 4, 9), 3).Has(ETag::Bayram));
    TestTrue(TEXT("The name is shown"), MarketCalendar::Describe(Eve, 3).Contains(TEXT("Weihnachten arifesi")));
    TestTrue(TEXT("Crowded eve"), MarketCalendar::TrafficFactor(Eve, 3) > TrEveTraffic);

    // A shut Sunday in the simulation: no shoppers, the day still closes.
    TArray<FMarketProduct> Base;
    {
        FMarketProduct P; P.Id = TEXT("sut"); P.RealName = P.Id; P.Category = TEXT("s\u00fct"); P.Cost = 170; P.BasePrice = 250; P.CaseUnits = 12;
        Base.Add(P);
    }
    TArray<FMarketProduct> Products = Base;
    FMarketState S; S.Initialize(Base); S.RivalSeed = 3; S.Cash = 100000; S.Day = Sunday;
    const MarketSimulation::FDay Closed = MarketSimulation::PlayDay(S, Base, Products);
    TestEqual(TEXT("Nobody shops on a shut day"), Closed.Shoppers, 0);
    TestEqual(TEXT("The day passed"), S.Day, Sunday + 1);

    SetActiveProfile(Profiles[1], 3);
    const int32 Thanks = MarketCalendar::GameDayOf(2011, 11, 24);
    TestTrue(TEXT("Thanksgiving"), MarketCalendar::Info(Thanks, 3).Has(ETag::Bayram));
    TestTrue(TEXT("Thanksgiving eve"), MarketCalendar::Info(Thanks - 1, 3).Has(ETag::BayramEve));
    TestFalse(TEXT("US shops open on Sunday"), MarketCalendar::ClosedByLaw(Sunday));
    TestTrue(TEXT("Americans pay by card more"), FMath::RoundToInt(MarketPayments::CardShare(Sunday) * 100.f) > TrCards);

    // Back to the Turkish prototype for the other tests.
    SetActiveProfile(FProfile(), 1);
    TestTrue(TEXT("Republic day again"), MarketCalendar::Info(MarketCalendar::GameDayOf(2011, 10, 29), 3).Has(ETag::NationalHoliday));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCountryFxTest, "MirasMarket.Country.Currencies", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCountryFxTest::RunTest(const FString& Parameters)
{
    using MarketCountry::FxRate;
    using MarketCountry::ToWorld;
    MarketCountry::SetActiveProfile(MarketCountry::FProfile(), 1);
    FMarketState S; S.CountryId = TEXT("tr"); S.RivalSeed = 21;
    const MarketCountry::FProfile* Tr = MarketCountry::Find(TEXT("tr"));
    const MarketCountry::FProfile* De = MarketCountry::Find(TEXT("de"));
    if (!TestTrue(TEXT("Packs"), Tr && De)) return false;
    TestTrue(TEXT("Start rate from the pack"), FMath::IsNearlyEqual(FxRate(S, TEXT("tr"), 1), Tr->FxPerWorld, 1e-9));
    TestEqual(TEXT("1,50 TL = 1 world unit at the start"), ToWorld(S, TEXT("tr"), 150, 1), int64(100));
    const int32 TenYears = MarketCalendar::GameDayOf(2021, 3, 7);
    const double TrMove = FxRate(S, TEXT("tr"), TenYears) / Tr->FxPerWorld;
    const double DeMove = FxRate(S, TEXT("de"), TenYears) / De->FxPerWorld;
    TestTrue(TEXT("High inflation: the lira loses value over the years"), TrMove > 1.6);
    TestTrue(TEXT("Stable economy: the euro stays about where it was"), DeMove > 0.6 && DeMove < 1.5);
    FMarketState Same = S;
    TestEqual(TEXT("Same seed, same rate"), FxRate(Same, TEXT("tr"), TenYears), FxRate(S, TEXT("tr"), TenYears));
    FMarketState Other = S; Other.RivalSeed = 22;
    TestNotEqual(TEXT("Another campaign, another walk"), FxRate(Other, TEXT("de"), TenYears), FxRate(S, TEXT("de"), TenYears));

    // A currency shock of the campaign's own country makes its money jump.
    MarketEras::FEra Shock = MarketEras::PlanOf(S)[0];
    const double Before = FxRate(S, TEXT("tr"), Shock.StartDay);
    const double After = FxRate(S, TEXT("tr"), Shock.StartDay + 10);
    TestTrue(TEXT("Currency shock: about a quarter in ten days"), After / Before > 1.2);
    TestTrue(TEXT("Only the own country's shock"), FxRate(S, TEXT("de"), Shock.StartDay + 10) / FxRate(S, TEXT("de"), Shock.StartDay) < 1.05);
    return true;
}

#endif

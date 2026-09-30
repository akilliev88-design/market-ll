#include "MarketEras.h"
#include "MarketEconomy.h"
#include "MarketCalendar.h"
#include "MarketCountry.h"
#include "MarketEvents.h"
#include "MarketGoods.h"
#include "MarketOnline.h"
#include "MarketPrices.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// Ak\u0131\u015f B4: eras of the economy.
namespace MarketErasTest
{
    // Leaves the global plan as every other test expects it (the Turkish prototype, no shift).
    void Restore()
    {
        MarketCountry::SetActiveProfile(MarketCountry::FProfile(), 1);
        MarketEras::ActivateNominal(MarketEras::ECharacter::HighInflation);
    }

    bool HasNews(const FMarketState& S, const TCHAR* Start)
    {
        for (const FString& Line : S.DayNews) if (Line.StartsWith(Start)) return true;
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketErasCurveTest, "MirasMarket.Eras.UnshiftedIsTheBuiltInCurve", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketErasCurveTest::RunTest(const FString& Parameters)
{
    using namespace MarketEras;
    MarketErasTest::Restore();
    TestEqual(TEXT("2018 peak"), MarketPrices::YearlyInflation(2018), 0.16);
    TestEqual(TEXT("2022 peak"), MarketPrices::YearlyInflation(2022), 0.30);
    TestEqual(TEXT("A calm year"), MarketPrices::YearlyInflation(2013), 0.075);
    TestEqual(TEXT("Loan rate of a peak year"), MarketPrices::LoanRate(MarketCalendar::GameDayOf(2022, 6, 1)), 0.33);
    bool bSame = true;
    for (int32 Year = 2011; Year <= 2029; ++Year) bSame &= InflationBump(Year) == BuiltInBump(Year);
    TestTrue(TEXT("The unshifted plan puts its peaks where the built-in curve has them"), bSame);
    TestTrue(TEXT("A second, milder wave later in a high-inflation country"), InflationBump(2031) > 0.0 && InflationBump(2031) < InflationBump(2018));

    // An older save: no Setup, no shift.
    FMarketState Old; Old.CountryId = TEXT("tr"); Old.RivalSeed = 99; Old.Day = 2000;
    Activate(Old);
    TestEqual(TEXT("Older save keeps the curve"), MarketPrices::YearlyInflation(2021), 0.19);
    TestEqual(TEXT("Older save: epidemic where it was"), PandemicShiftDays(Old), 0);
    MarketErasTest::Restore();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketErasPlanTest, "MirasMarket.Eras.OrderShiftAndCharacter", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketErasPlanTest::RunTest(const FString& Parameters)
{
    using namespace MarketEras;
    const EKind Order[] = { EKind::CurrencyShock, EKind::Recession, EKind::Pandemic, EKind::HighInflation, EKind::Recovery };
    TSet<int32> Shifts;
    bool bOrdered = true, bInRange = true, bSameAgain = true;
    for (int32 Seed = 1; Seed <= 60; ++Seed)
    {
        FMarketState S; S.CountryId = TEXT("tr"); S.RivalSeed = Seed * 7919;
        Setup(S);
        Shifts.Add(S.Eras.ShiftYears);
        bInRange &= S.Eras.bPlanned && S.Eras.ShiftYears >= -2 && S.Eras.ShiftYears <= 2 && FMath::Abs(S.Eras.ShiftDays) <= 45;
        const TArray<FEra> Plan = PlanOf(S);
        for (int32 I = 0; I < Plan.Num(); ++I)
        {
            if (I < 5) bOrdered &= Plan[I].Kind == Order[I];
            if (I > 0) bOrdered &= Plan[I].StartDay > Plan[I - 1].StartDay;
            bOrdered &= Plan[I].EndDay >= Plan[I].StartDay;
        }
        FMarketState Again; Again.CountryId = TEXT("tr"); Again.RivalSeed = Seed * 7919;
        Setup(Again);
        bSameAgain &= Again.Eras.ShiftYears == S.Eras.ShiftYears && Again.Eras.ShiftDays == S.Eras.ShiftDays;
    }
    TestTrue(TEXT("The order never changes"), bOrdered);
    TestTrue(TEXT("Shift within two years"), bInRange);
    TestTrue(TEXT("Campaigns differ"), Shifts.Num() >= 4);
    TestTrue(TEXT("Same seed, same eras"), bSameAgain);

    // Frequency and strength follow the economy character.
    const TArray<FEra> Stable = Plan(ECharacter::Stable, 5, 0, 0);
    const TArray<FEra> Shaky = Plan(ECharacter::Volatile, 5, 0, 0);
    const TArray<FEra> High = Plan(ECharacter::HighInflation, 5, 0, 0);
    TestFalse(TEXT("A stable economy has no high-inflation era"), Stable.ContainsByPredicate([](const FEra& E) { return E.Kind == EKind::HighInflation; }));
    TestTrue(TEXT("... and a mild currency shock"), Stable.Num() > 0 && Stable[0].Kind == EKind::CurrencyShock && Stable[0].Strength < 0.5f);
    TestTrue(TEXT("Fewer eras when stable"), Stable.Num() < High.Num() && Shaky.Num() <= High.Num());
    TestTrue(TEXT("High inflation comes back once"), High.FilterByPredicate([](const FEra& E) { return E.Wave == 2; }).Num() == 2);
    TestTrue(TEXT("Everybody lives the epidemic"), Stable.ContainsByPredicate([](const FEra& E) { return E.Kind == EKind::Pandemic; }));

    // Shifted peaks move the price curve.
    FMarketState Late; Late.CountryId = TEXT("tr"); Late.RivalSeed = 3;
    Late.Eras.bPlanned = true; Late.Eras.ShiftYears = 2; Late.Eras.ShiftDays = 0;
    Activate(Late);
    TestTrue(TEXT("Two years later: 2018 is calm"), MarketPrices::YearlyInflation(2018) < 0.13);
    TestTrue(TEXT("... and the shock comes in 2020"), MarketPrices::YearlyInflation(2020) > 0.16);
    TestTrue(TEXT("... high inflation peaks in 2024"), MarketPrices::YearlyInflation(2024) > 0.28);
    TestTrue(TEXT("Banks follow"), MarketPrices::LoanRate(MarketCalendar::GameDayOf(2024, 6, 1)) > MarketPrices::LoanRate(MarketCalendar::GameDayOf(2022, 6, 1)));
    FMarketState Early = Late; Early.Eras.ShiftYears = -2;
    Activate(Early);
    TestTrue(TEXT("Two years earlier: the shock in 2016"), MarketPrices::YearlyInflation(2016) > 0.12);
    MarketErasTest::Restore();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketErasEffectsTest, "MirasMarket.Eras.EffectsAndNews", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketErasEffectsTest::RunTest(const FString& Parameters)
{
    using namespace MarketEras;
    using MarketEvents::EModifier;
    using MarketGoods::EGroup;
    FMarketState S; S.CountryId = TEXT("tr"); S.RivalSeed = 12;
    S.Eras.bPlanned = true; S.Eras.ShiftYears = -1; S.Eras.ShiftDays = 10;
    const TArray<FEra> Plan = PlanOf(S);
    const FEra Shock = Plan[0];
    const FEra Recession = Plan[1];
    auto Close = [&S](int32 Day) { S.Day = Day; S.DayNews.Reset(); MarketEras::CloseDay(S); };

    Close(Shock.StartDay - 1);
    TestTrue(TEXT("Calm before"), S.DayNews.Num() == 0 && Summary(S).IsEmpty());
    Close(Shock.StartDay);
    TestTrue(TEXT("The shock is news, no year in it"), MarketErasTest::HasNews(S, TEXT("Kur \u015foku:")) && !S.DayNews[0].Contains(TEXT("20")));
    TestTrue(TEXT("Imported goods cost more"), MarketEvents::Factor(S, EModifier::CostFactor, EGroup::TeaCoffee) > 1.05f);
    TestTrue(TEXT("Milk does not"), FMath::IsNearlyEqual(MarketEvents::Factor(S, EModifier::CostFactor, EGroup::Dairy), 1.f));
    TestTrue(TEXT("Shoppers compare more"), MarketEvents::Tolerance(S, EGroup::Staples) < 0.0);
    TestFalse(TEXT("Menu line"), Summary(S).IsEmpty());
    const int32 Modifiers = S.Modifiers.Num();
    Close(Shock.StartDay + 1);
    TestEqual(TEXT("Started once"), S.Modifiers.Num(), Modifiers);

    Close(Recession.StartDay);
    TestTrue(TEXT("Shock over, recession on"), MarketErasTest::HasNews(S, TEXT("Kur \u015foku yat\u0131\u015ft\u0131")) && MarketErasTest::HasNews(S, TEXT("Durgunluk ba\u015flad\u0131")));
    TestTrue(TEXT("Smaller baskets"), BudgetFactor(S) < 0.95f);
    TestTrue(TEXT("Fewer sweets"), MarketEvents::Factor(S, EModifier::Interest, EGroup::Sweets) < 0.9f);
    FEra Now;
    TestTrue(TEXT("Current era"), Current(S, S.Day, Now) && Now.Kind == EKind::Recession);

    // The epidemic moves with the plan (MarketOnline's profile), and the player can still switch it off.
    FMarketState Legacy; Legacy.RivalSeed = 12;
    const int32 Shift = PandemicShiftDays(S);
    TestEqual(TEXT("A year and ten days earlier"), Shift, MarketCalendar::GameDayOf(2019, 3, 1) - MarketCalendar::GameDayOf(2020, 3, 1) + 10);
    TestEqual(TEXT("Epidemic start follows"), MarketOnline::PandemicStart(S), MarketOnline::PandemicStart(Legacy) + Shift);
    TestEqual(TEXT("... and its end"), MarketOnline::PandemicEnd(S), MarketOnline::PandemicEnd(Legacy) + Shift);
    S.Online.bPandemic = true;
    TestTrue(TEXT("Epidemic is an era"), Current(S, MarketOnline::PandemicStart(S) + 5, Now) && Now.Kind == EKind::Pandemic);
    S.Online.bPandemic = false;
    TestFalse(TEXT("Switched off"), MarketOnline::IsPandemic(S, MarketOnline::PandemicStart(S) + 5));

    // An older save in the middle of an era: nothing is replayed.
    FMarketState Old; Old.CountryId = TEXT("tr"); Old.RivalSeed = 12; Old.Day = PlanOf(Old)[3].StartDay + 40; // high inflation
    MarketEras::CloseDay(Old);
    TestEqual(TEXT("No news"), Old.DayNews.Num(), 0);
    TestEqual(TEXT("No effects replayed"), Old.Modifiers.Num(), 0);
    TestTrue(TEXT("Marked"), Old.Eras.bChecked && (Old.Eras.Started & (1 << 3)) != 0);
    MarketErasTest::Restore();
    return true;
}

#endif

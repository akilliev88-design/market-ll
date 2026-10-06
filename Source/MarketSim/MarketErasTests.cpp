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
        MarketCountry::SetActive(MarketCountry::DefaultId(), 1); // D3: the default pack (Turkey)
        MarketEras::ActivateNominal(MarketEras::ECharacter::HighInflation);
    }

    bool HasNews(const FMarketState& S, const TCHAR* Start)
    {
        for (const FString& Line : S.DayNews) if (Line.StartsWith(Start)) return true;
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketErasCurveTest, "MarketSim.Eras.UnshiftedIsTheBuiltInCurve", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
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

    // A campaign without Setup (tests, tools): no shift.
    FMarketState Plain; Plain.CountryId = TEXT("tr"); Plain.RivalSeed = 99; Plain.Day = 2000;
    Activate(Plain);
    TestEqual(TEXT("Without Setup: the built-in curve"), MarketPrices::YearlyInflation(2021), 0.19);
    TestEqual(TEXT("Without Setup: the epidemic where it was"), PandemicShiftDays(Plain), 0);
    MarketErasTest::Restore();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketErasPlanTest, "MarketSim.Eras.OrderShiftAndCharacter", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
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
        const TArray<FEra> Timeline = PlanOf(S);
        for (int32 I = 0; I < Timeline.Num(); ++I)
        {
            if (I < 5) bOrdered &= Timeline[I].Kind == Order[I];
            if (I > 0) bOrdered &= Timeline[I].StartDay > Timeline[I - 1].StartDay;
            bOrdered &= Timeline[I].EndDay >= Timeline[I].StartDay;
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketErasEffectsTest, "MarketSim.Eras.EffectsAndNews", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketErasEffectsTest::RunTest(const FString& Parameters)
{
    using namespace MarketEras;
    using MarketEvents::EModifier;
    using MarketGoods::EGroup;
    FMarketState S; S.CountryId = TEXT("tr"); S.RivalSeed = 12;
    S.Eras.bPlanned = true; S.Eras.ShiftYears = -1; S.Eras.ShiftDays = 10;
    const TArray<FEra> Timeline = PlanOf(S);
    const FEra Shock = Timeline[0];
    const FEra Recession = Timeline[1];
    auto Close = [&S](int32 Day) { S.Day = Day; S.DayNews.Reset(); MarketEras::CloseDay(S); };

    Close(Shock.StartDay - 1);
    TestTrue(TEXT("Calm before"), S.DayNews.Num() == 0 && Summary(S).IsEmpty());
    Close(Shock.StartDay);
    TestTrue(TEXT("The shock is news, no year in it"), MarketErasTest::HasNews(S, TEXT("Kur \u015foku:")) && !S.DayNews[0].Contains(TEXT("20")));
    TestTrue(TEXT("Imported goods start to cost more"), MarketEvents::Factor(S, EModifier::CostFactor, EGroup::TeaCoffee) > 1.f);
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

    // The epidemic moves with the plan (MarketOnline's profile) and always comes (M32).
    FMarketState Legacy; Legacy.RivalSeed = 12;
    const int32 Shift = PandemicShiftDays(S);
    TestEqual(TEXT("A year and ten days earlier"), Shift, MarketCalendar::GameDayOf(2019, 3, 1) - MarketCalendar::GameDayOf(2020, 3, 1) + 10);
    TestEqual(TEXT("Epidemic start follows"), MarketOnline::PandemicStart(S), MarketOnline::PandemicStart(Legacy) + Shift);
    TestEqual(TEXT("... and its end"), MarketOnline::PandemicEnd(S), MarketOnline::PandemicEnd(Legacy) + Shift);
    TestTrue(TEXT("Epidemic is an era"), Current(S, MarketOnline::PandemicStart(S) + 5, Now) && Now.Kind == EKind::Pandemic);

    MarketErasTest::Restore();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketErasFactorsTest, "MarketSim.Eras.FactorsForDepartmentsAndChains", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketErasFactorsTest::RunTest(const FString& Parameters)
{
    using namespace MarketEras;
    using MarketEvents::EModifier;
    using MarketGoods::EGroup;
    FMarketState S; S.CountryId = TEXT("tr"); S.RivalSeed = 7;
    S.Eras.bPlanned = true; S.Eras.ShiftYears = 0; S.Eras.ShiftDays = 0;
    const TArray<FEra> Timeline = PlanOf(S);
    FEra Shock, Recession, Recovery;
    for (const FEra& E : Timeline)
    {
        if (E.Wave != 1) continue;
        if (E.Kind == EKind::CurrencyShock) Shock = E;
        if (E.Kind == EKind::Recession) Recession = E;
        if (E.Kind == EKind::Recovery) Recovery = E;
    }
    auto AllOne = [&S](int32 Day)
    {
        for (int32 G = 0; G < static_cast<int32>(EGoods::Count); ++G)
            if (DemandFactor(S, static_cast<EGoods>(G), Day) != 1.f || ImportCostFactor(S, static_cast<EGoods>(G), Day) != 1.f) return false;
        return ChainRevenueFactor(S, TEXT("tr"), 0.9f, Day) == 1.f && ChainRevenueFactor(S, TEXT("tr"), 1.15f, Day) == 1.f
            && ChainOpeningFactor(S, TEXT("tr"), Day) == 1.f && ChainRedTurnsToSell(S, TEXT("tr"), Day) == 3 && NonFoodDemand(S, Day) == 1.f;
    };

    // No era: every factor is exactly 1.
    bool bCalm = true;
    for (int32 Day = 1; Day < Shock.StartDay; Day += 5) bCalm &= AllOne(Day);
    TestTrue(TEXT("Calm years: all 1"), bCalm);
    S.Day = 400;
    S.Modifiers.Reset();
    TestTrue(TEXT("Calm: grocery purchase prices untouched"), MarketEvents::Factor(S, EModifier::CostFactor, EGroup::TeaCoffee) == 1.f);

    // Currency shock: imported costs rise over two weeks, hold, then come down slowly.
    const float Day0 = ImportCostFactor(S, EGoods::Electronics, Shock.StartDay);
    const float Peak = ImportCostFactor(S, EGoods::Electronics, Shock.StartDay + 20);
    const float Hold = ImportCostFactor(S, EGoods::Electronics, Shock.EndDay);
    const float After100 = ImportCostFactor(S, EGoods::Electronics, Shock.EndDay + 100);
    const float After200 = ImportCostFactor(S, EGoods::Electronics, Shock.EndDay + 200);
    TestTrue(TEXT("First day: a little"), Day0 > 1.f && Day0 < 1.03f);
    TestTrue(TEXT("Electronics +25 % at the peak"), FMath::IsNearlyEqual(Peak, 1.25f, 0.001f));
    TestTrue(TEXT("Holds while the shock lasts"), FMath::IsNearlyEqual(Hold, Peak, 0.001f));
    TestTrue(TEXT("Comes down slowly"), Hold > After100 && After100 > After200 && After200 > 1.f);
    TestTrue(TEXT("Back to 1 in the end"), ImportCostFactor(S, EGoods::Electronics, Shock.EndDay + 301) == 1.f);
    TestTrue(TEXT("Clothing less than electronics"), ImportCostFactor(S, EGoods::Clothing, Shock.StartDay + 20) < Peak);
    TestTrue(TEXT("Fresh goods untouched"), ImportCostFactor(S, EGoods::Fresh, Shock.StartDay + 20) == 1.f);
    S.Day = Shock.StartDay + 20;
    TestTrue(TEXT("The shop's coffee follows (+8.75 %)"), FMath::IsNearlyEqual(MarketEvents::Factor(S, EModifier::CostFactor, EGroup::TeaCoffee), 1.0875f, 0.001f));
    TestTrue(TEXT("The shop's milk does not"), MarketEvents::Factor(S, EModifier::CostFactor, EGroup::Dairy) == 1.f);
    S.Day = Shock.EndDay + 100;
    TestTrue(TEXT("Coffee still a bit dear after the shock"), MarketEvents::Factor(S, EModifier::CostFactor, EGroup::TeaCoffee) > 1.f);
    TestTrue(TEXT("A stable country feels less"), ImportCostFactor(S, EGoods::Electronics, Shock.StartDay + 20, TEXT("de")) < Peak);

    // Recession: non-food falls, fresh hardly; dear chains lose most, fewer openings, sold sooner.
    const int32 Mid = Recession.StartDay + 60;
    const float NonFood = NonFoodDemand(S, Mid);
    const float Fresh = FreshDemand(S, Mid);
    TestTrue(TEXT("Recession: non-food down"), NonFood < 0.85f);
    TestTrue(TEXT("Recession: electronics -25 %"), FMath::IsNearlyEqual(DemandFactor(S, EGoods::Electronics, Mid), 0.75f, 0.001f));
    TestTrue(TEXT("Recession: fresh barely moves"), Fresh > 0.95f && Fresh - NonFood > 0.1f);
    TestTrue(TEXT("Recession: grocery 1 (the shop's modifiers do it)"), DemandFactor(S, EGoods::Grocery, Mid) == 1.f);
    const float Cheap = ChainRevenueFactor(S, TEXT("tr"), 0.9f, Mid);
    const float Dear = ChainRevenueFactor(S, TEXT("tr"), 1.15f, Mid);
    TestTrue(TEXT("Recession: everyone sells less, the dear most"), Dear < Cheap && Cheap < 1.f && Dear < 0.85f);
    TestTrue(TEXT("Recession: fewer openings"), ChainOpeningFactor(S, TEXT("tr"), Mid) < 0.5f);
    TestEqual(TEXT("Recession: two red months put a chain up for sale"), ChainRedTurnsToSell(S, TEXT("tr"), Mid), 2);
    TestTrue(TEXT("Recession fades over a month"), NonFoodDemand(S, Recession.EndDay + 15) > NonFood && NonFoodDemand(S, Recession.EndDay + 31) >= NonFood);

    // Recovery: more of everything.
    const int32 Up = Recovery.StartDay + 60;
    TestTrue(TEXT("Recovery: non-food up"), NonFoodDemand(S, Up) > 1.05f);
    TestTrue(TEXT("Recovery: more openings"), ChainOpeningFactor(S, TEXT("tr"), Up) > 1.3f);
    TestTrue(TEXT("Recovery: chains sell more"), ChainRevenueFactor(S, TEXT("tr"), 1.f, Up) > 1.f);

    // The epidemic: clothing down, electronics and fresh up.
    const int32 Sick = MarketOnline::PandemicStart(S) + 60;
    TestTrue(TEXT("Epidemic: clothing down, fresh up"), DemandFactor(S, EGoods::Clothing, Sick) < 0.8f && FreshDemand(S, Sick) > 1.05f);

    // Department names.
    TestTrue(TEXT("Goods of departments"), GoodsOf(TEXT("elektronik")) == EGoods::Electronics && GoodsOf(TEXT("Giyim")) == EGoods::Clothing
        && GoodsOf(TEXT("oyuncak")) == EGoods::Toys && GoodsOf(TEXT("manav")) == EGoods::Fresh && GoodsOf(TEXT("kasap")) == EGoods::Fresh
        && GoodsOf(TEXT("z\u00fccaciye")) == EGoods::Home && GoodsOf(TEXT("ev")) == EGoods::Home && GoodsOf(TEXT("bakliyat")) == EGoods::Grocery
        && GoodsOf(TEXT("dept.electronics")) == EGoods::Electronics);
    MarketErasTest::Restore();
    return true;
}

#endif

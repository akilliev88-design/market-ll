#include "MarketLeague.h"
#include "MarketEconomy.h"
#include "MarketCalendar.h"
#include "MarketCountry.h"
#include "MarketEras.h"
#include "MarketLedger.h"
#include "MarketStory.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// Ak\u0131\u015f B5: currencies and the world retail league.
namespace MarketLeagueTest
{
    // One closed league day: the day's revenue and a profit in the books, then the league's close.
    void Close(FMarketState& S, int64 Revenue, int64 Profit)
    {
        S.Day += 1;
        S.LastRevenue = Revenue;
        S.DayNews.Reset();
        S.Ledger.bClosing = true;
        S.Ledger.ClosingDay = S.Day - 1;
        MarketLedger::Post(S, MarketLedger::EAccount::Sales, Profit);
        S.Ledger.bClosing = false;
        MarketLeague::CloseDay(S);
    }

    bool NewsStarts(const FMarketState& S, const TCHAR* Start)
    {
        for (const FString& Line : S.DayNews) if (Line.StartsWith(Start)) return true;
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketLeagueFxTest, "MirasMarket.League.Currencies", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketLeagueFxTest::RunTest(const FString& Parameters)
{
    using namespace MarketLeague;
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketLeagueTableTest, "MirasMarket.League.TableAndFinale", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketLeagueTableTest::RunTest(const FString& Parameters)
{
    using namespace MarketLeague;
    MarketCountry::SetActiveProfile(MarketCountry::FProfile(), 1);
    FMarketState S; S.CountryId = TEXT("tr"); S.RivalSeed = 5; S.Day = 1;
    const TArray<FEntry> Small = Table(S, 1, 1000000);
    TestEqual(TEXT("Eight giants and us"), Small.Num(), GiantCount() + 1);
    bool bSorted = true;
    for (int32 I = 1; I < Small.Num(); ++I) bSorted &= Small[I - 1].Revenue >= Small[I].Revenue;
    TestTrue(TEXT("First place first"), bSorted);
    TestTrue(TEXT("A small shop is last"), Small.Last().bPlayer);
    TestTrue(TEXT("Giants grow at their own pace"), GiantRevenue(S, 5, 20) > GiantRevenue(S, 5, 1) && GiantRevenue(S, 3, 20) < GiantRevenue(S, 3, 1) * 1.2);
    const int64 Top = GiantRevenue(S, 0, 1);
    TestTrue(TEXT("The top is reachable in the game's scale (hundreds of shops, not a hundred thousand)"), Top / 21900000 > 300 && Top / 21900000 < 3000);

    // Two league years as the first, with a healthy balance, in the last chapter: the "Miras" finale.
    S.Story.Chapter = 7;
    const int64 Huge = Top * 3 / 2 / YearDays * 3 / 2; // internal kurus a day: comfortably above the top giant
    for (int32 I = 0; I < YearDays; ++I) MarketLeagueTest::Close(S, Huge, 100000);
    TestEqual(TEXT("First year closed"), S.League.LastYear, 1);
    TestEqual(TEXT("First place"), S.League.LastRank, 1);
    TestTrue(TEXT("Year news without a calendar year"), MarketLeagueTest::NewsStarts(S, TEXT("1. y\u0131l d\u00fcnya ligi: 1.")));
    TestEqual(TEXT("One year counted"), S.League.FirstPlaceYears, 1);
    TestFalse(TEXT("Not yet"), MarketStory::StoryClosed(S));
    TestTrue(TEXT("Menu line"), Summary(S).Contains(TEXT("1. s\u0131ra")));
    for (int32 I = 0; I < YearDays; ++I) MarketLeagueTest::Close(S, Huge, 100000);
    TestTrue(TEXT("Miras"), MarketStory::StoryClosed(S) && S.Story.Ending == static_cast<uint8>(MarketStory::EEnding::Legacy));

    // First, but drowning in debt: not counted.
    FMarketState Debt; Debt.CountryId = TEXT("tr"); Debt.RivalSeed = 5; Debt.Day = 1; Debt.Story.Chapter = 7;
    FMarketLoan Loan; Loan.Principal = Loan.Remaining = 100000 * 365 * 4; Debt.Loans.Add(Loan);
    for (int32 I = 0; I < YearDays; ++I) MarketLeagueTest::Close(Debt, Huge, 100000);
    TestEqual(TEXT("First place"), Debt.League.LastRank, 1);
    TestEqual(TEXT("Debt over three years of profit: not counted"), Debt.League.FirstPlaceYears, 0);
    TestFalse(TEXT("Money conditions"), MoneyOk(Debt, 100000 * 365));

    // An older save joins in the middle of a year: that partial year does not count.
    FMarketState Old; Old.CountryId = TEXT("tr"); Old.RivalSeed = 5; Old.Day = 200; Old.Story.Chapter = 7;
    while (Old.Day <= YearDays) MarketLeagueTest::Close(Old, Huge, 100000);
    TestEqual(TEXT("Ranked"), Old.League.LastRank, 1);
    TestEqual(TEXT("Partial year not counted"), Old.League.FirstPlaceYears, 0);
    TestEqual(TEXT("Next year runs"), Old.League.LeagueYear, 2);
    return true;
}

#endif

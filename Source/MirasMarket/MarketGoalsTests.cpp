#include "MarketGoals.h"
#include "MarketEconomy.h"
#include "MarketBranches.h"
#include "MarketCompany.h"
#include "MarketEvents.h"
#include "MarketLedger.h"
#include "MarketStory.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// Ak\u0131\u015f B6: goals, firsts, records, celebrations and the rhythm guard (06 \u00a72b).
namespace MarketGoalsTest
{
    FMarketProduct Make(const TCHAR* Id, const TCHAR* Category, int64 Cost, int64 Price)
    {
        FMarketProduct P;
        P.Id = Id; P.RealName = Id; P.Category = Category; P.Cost = Cost; P.BasePrice = Price; P.CaseUnits = 12;
        return P;
    }

    TArray<FMarketProduct> Catalog()
    {
        return { Make(TEXT("sut"), TEXT("s\u00fct"), 170, 250), Make(TEXT("cola"), TEXT("i\u00e7ecek"), 180, 275), Make(TEXT("cay"), TEXT("\u00e7ay-kahve"), 480, 675) };
    }

    FMarketState NewShop()
    {
        FMarketState S; S.Initialize(Catalog()); S.RivalSeed = 77; S.Cash = 500000; S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
        S.ApplyShelfCapacities({ 20, 20, 20 });
        for (FMarketStock& Row : S.Stock) Row.Shelf = 18;
        return S;
    }

    // One closed day with the given company revenue, profit and served shoppers (only the goals' close).
    void Close(FMarketState& S, int64 Revenue, int64 Profit, int32 Served = 40)
    {
        S.Day += 1;
        S.LastRevenue = Revenue;
        S.LastProfit = Profit;
        S.LastServed = Served;
        FMarketDayRecord R; R.Day = S.Day - 1; R.Revenue = Revenue; R.Profit = Profit; R.Served = Served; S.History.Add(R);
        S.DayNews.Reset();
        MarketGoals::CloseDay(S, Catalog());
    }

    int32 Count(const FMarketState& S, const TCHAR* Start)
    {
        int32 N = 0;
        for (const FString& Line : S.DayNews) if (Line.StartsWith(Start)) ++N;
        return N;
    }

    void AddBranch(FMarketState& S, const TCHAR* Province)
    {
        FMarketBranch B; B.Country = TEXT("tr"); B.Province = Province; B.Stage = static_cast<uint8>(MarketBranches::EStage::Open); B.Format = TEXT("mahalle");
        S.Branches.Add(B);
        S.bSecondStore = true;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketGoalsAlwaysTest, "MirasMarket.Goals.AlwaysAGoalWithinReach", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketGoalsAlwaysTest::RunTest(const FString& Parameters)
{
    using namespace MarketGoalsTest;
    using namespace MarketGoals;
    FMarketState S = NewShop();
    TestTrue(TEXT("No goals before the first close"), Goals(S).Num() == 0 && StripText(S).IsEmpty());
    bool bAlways = true, bShort = true, bReachable = true, bTexts = true, bNoRepeat = true;
    TArray<uint8> Finished;
    int32 Completed = 0;
    for (int32 D = 0; D < 200; ++D)
    {
        // A shop that slowly gets better, with good and bad days.
        const int64 Revenue = 50000 + D * 100 + (D % 5) * 4000;
        const int64 Profit = 6000 + D * 20 + (D % 7) * 800;
        const int32 Before = S.Goals.RecentKinds.Num();
        Close(S, Revenue, Profit, 40 + D % 9);
        Completed += Count(S, TEXT("Kutlama: Hedef tamam!"));
        const TArray<FGoalView> Views = Goals(S);
        bAlways &= Views.Num() > 0;
        bShort &= Views.ContainsByPredicate([](const FGoalView& V) { return V.Scale == EScale::Short; });
        for (const FGoalView& V : Views)
        {
            bTexts &= !V.Title.IsEmpty() && !V.Why.IsEmpty() && !V.Reward.IsEmpty() && V.DaysLeft >= 0;
            bReachable &= V.Progress < 1.f;
        }
        // The same short goal never comes right after itself.
        if (S.Goals.RecentKinds.Num() > Before)
        {
            const uint8 Last = S.Goals.RecentKinds.Last();
            for (const FMarketGoal& G : S.Goals.Goals)
                if (G.Scale == static_cast<uint8>(EScale::Short) && G.StartDay == S.Day) bNoRepeat &= G.Kind != Last;
        }
    }
    TestTrue(TEXT("Always a goal on the screen"), bAlways);
    TestTrue(TEXT("Always a short one"), bShort);
    TestTrue(TEXT("Every goal says what, why, what for and how long; no calendar year"), bTexts);
    TestTrue(TEXT("Nothing already done stays on the list"), bReachable);
    TestTrue(TEXT("No repeat right away"), bNoRepeat);
    TestTrue(FString::Printf(TEXT("Goals are reached now and then (%d)"), Completed), Completed >= 4);
    TestFalse(TEXT("Strip line"), StripText(S).IsEmpty());
    FGoalView Next;
    TestTrue(TEXT("Next goal"), NextGoal(S, Next) && Next.Progress >= 0.f);

    // A goal whose door is shut is never given: no branch goal for a new shop with the father's debt.
    FMarketState Young = NewShop(); Young.InheritedDebt = 30000;
    for (int32 D = 0; D < 40; ++D)
    {
        Close(Young, 50000, 5000);
        for (const FMarketGoal& G : Young.Goals.Goals)
        {
            TestFalse(TEXT("No shop-count goal yet"), G.Kind == static_cast<uint8>(EGoal::MoreStores));
            TestFalse(TEXT("No abroad goal"), G.Kind == static_cast<uint8>(EGoal::Abroad));
        }
    }
    TestTrue(TEXT("The debt is a goal"), Young.Goals.Goals.ContainsByPredicate([](const FMarketGoal& G)
        { return G.Kind == static_cast<uint8>(EGoal::PayDebt) || G.Kind == static_cast<uint8>(EGoal::DebtFree); }) || Young.Goals.RecentKinds.Contains(static_cast<uint8>(EGoal::PayDebt)));

    // Targets follow the player's own numbers: a ten times bigger shop gets a ten times bigger revenue goal.
    FMarketState Small = NewShop(), Big = NewShop();
    for (int32 D = 0; D < 20; ++D) { Close(Small, 40000, 4000); Close(Big, 400000, 40000); }
    int64 SmallTarget = 0, BigTarget = 0;
    for (const FMarketGoal& G : Small.Goals.Goals) if (G.Kind == static_cast<uint8>(EGoal::MonthProfit)) SmallTarget = G.Target;
    for (const FMarketGoal& G : Big.Goals.Goals) if (G.Kind == static_cast<uint8>(EGoal::MonthProfit)) BigTarget = G.Target;
    if (SmallTarget > 0 && BigTarget > 0) TestTrue(TEXT("Month profit goal scales"), BigTarget > SmallTarget * 8 && BigTarget < SmallTarget * 12);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketGoalsCelebrationTest, "MirasMarket.Goals.FirstsRecordsAndCelebrations", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketGoalsCelebrationTest::RunTest(const FString& Parameters)
{
    using namespace MarketGoalsTest;
    using namespace MarketGoals;
    FMarketState S = NewShop(); S.InheritedDebt = 10000;
    Close(S, 50000, 5000);
    TestEqual(TEXT("First close: nothing to celebrate yet"), Count(S, TEXT("Kutlama")), 0);

    // A first: the first branch.
    AddBranch(S, TEXT("kirklareli"));
    for (FMarketEmployee& E : S.Staff) E.Morale = 50.f;
    FMarketEmployee Worker; Worker.Id = 1; Worker.Name = TEXT("Test"); Worker.Role = 0; Worker.Morale = 50.f; S.Staff.Add(Worker);
    Close(S, 50000, 5000);
    TestEqual(TEXT("First branch celebrated"), Count(S, TEXT("Kutlama: \u0130lk \u015fube!")), 1);
    TestTrue(TEXT("Celebration card"), CelebrationsOn(S, S.Day - 1).Num() >= 1 && CelebrationsOn(S, S.Day - 1)[0].Importance == 2);
    TestTrue(TEXT("The team is glad"), S.Staff.Last().Morale > 50.f);
    const int32 Cards = S.Goals.Celebrations.Num();

    // The same close twice: no second celebration.
    S.Day -= 1;
    S.DayNews.Reset();
    MarketGoals::CloseDay(S, Catalog());
    S.Day += 1;
    TestEqual(TEXT("Same close again: nothing twice"), S.Goals.Celebrations.Num(), Cards);
    Close(S, 50000, 5000);
    TestEqual(TEXT("Next day: the first branch is not told again"), Count(S, TEXT("Kutlama: \u0130lk \u015fube!")), 0);

    // Debt closed.
    S.InheritedDebt = 0; S.DebtClearedDay = S.Day;
    Close(S, 50000, 5000);
    TestEqual(TEXT("Debt closed celebrated"), Count(S, TEXT("Kutlama: Bor\u00e7 bitti!")), 1);

    // Records: not in the first two weeks, then at most one a week.
    for (int32 D = 0; D < 20; ++D) Close(S, 50000, 5000);
    Close(S, 70000, 9000);
    TestEqual(TEXT("A record day"), Count(S, TEXT("Kutlama: Ciro rekoru!")), 1);
    TestEqual(TEXT("Kept"), S.Goals.BestDayRevenue, int64(70000));
    Close(S, 80000, 9000);
    TestEqual(TEXT("Not every day"), Count(S, TEXT("Kutlama: Ciro rekoru!")), 0);
    TestEqual(TEXT("But the record is kept"), S.Goals.BestDayRevenue, int64(80000));
    TestTrue(TEXT("Records page"), Records(S).Num() >= 4 && Records(S)[0].Value.Contains(TEXT("800")));
    TestTrue(TEXT("Newest first"), RecentCelebrations(S, 3).Num() > 0 && RecentCelebrations(S, 3)[0].Day >= RecentCelebrations(S, 3).Last().Day);

    // An older save far into the game: its firsts and records start silently (no shower of cards).
    FMarketState Old = NewShop(); Old.Day = 1500; Old.ProfitableDays = 900; Old.InheritedDebt = 0; Old.DebtClearedDay = 80;
    for (int32 I = 0; I < 12; ++I) AddBranch(Old, I < 6 ? TEXT("kirklareli") : TEXT("edirne"));
    for (int32 I = 0; I < 30; ++I) { FMarketDayRecord R; R.Day = 1400 + I; R.Revenue = 900000; Old.History.Add(R); }
    Close(Old, 500000, 50000);
    TestEqual(TEXT("Older save: no celebrations for the past"), Old.Goals.Celebrations.Num(), 0);
    TestTrue(TEXT("... but the firsts are marked"), (Old.Goals.Firsts & (static_cast<int64>(1) << static_cast<int32>(EFirst::Stores10))) != 0);
    TestEqual(TEXT("... and the best day is known"), Old.Goals.BestDayRevenue, int64(900000));
    while (MarketCompany::TotalStores(Old) < 25) AddBranch(Old, TEXT("tekirdag"));
    Close(Old, 500000, 50000);
    TestEqual(TEXT("A new first after that is told"), Count(Old, TEXT("Kutlama: 25 ma\u011faza!")), 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketGoalsRhythmTest, "MirasMarket.Goals.RhythmGuard", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketGoalsRhythmTest::RunTest(const FString& Parameters)
{
    using namespace MarketGoalsTest;
    using namespace MarketGoals;
    // A quiet stretch brings a pleasant event (normal difficulty: 20 days).
    FMarketState S = NewShop(); S.Difficulty = 1;
    int32 QuietDay = 0;
    for (int32 D = 0; D < 40 && QuietDay == 0; ++D)
    {
        Close(S, 50000, 5000);
        if (S.Goals.QuietEvents > 0) QuietDay = S.Day - 1;
    }
    TestTrue(TEXT("Something pleasant after about twenty quiet days"), QuietDay >= 20 && QuietDay <= 23);
    TestTrue(TEXT("Easy: sooner, hard: later"), QuietDays(FMarketState()) == 20 && [] { FMarketState E; E.Difficulty = 0; return QuietDays(E); }() < [] { FMarketState H; H.Difficulty = 2; return QuietDays(H); }());

    // A pile of bad luck holds the next bad event back.
    FMarketState Bad = NewShop(); Bad.Day = 30;
    TestFalse(TEXT("Nothing bad yet"), HoldBadEvent(Bad));
    TestTrue(TEXT("Bad event 1"), MarketEvents::Trigger(Bad, Catalog(), TEXT("event.power")));
    TestTrue(TEXT("Bad event 2"), MarketEvents::Trigger(Bad, Catalog(), TEXT("event.roadworks")));
    TestFalse(TEXT("Two are fine"), HoldBadEvent(Bad));
    TestTrue(TEXT("Bad event 3"), MarketEvents::Trigger(Bad, Catalog(), TEXT("event.truck")));
    TestTrue(TEXT("Three in a week: the next one waits"), HoldBadEvent(Bad));
    TestFalse(TEXT("A wedding is not bad"), IsBadEvent(TEXT("event.wedding")));
    FMarketState Hard = Bad; Hard.Difficulty = 2;
    TestFalse(TEXT("Hard: one more is allowed"), HoldBadEvent(Hard));
    Bad.Day += 8;
    TestFalse(TEXT("A week later it is over"), HoldBadEvent(Bad));

    // The random event of the day skips bad ones while they are held back.
    FMarketState Busy = NewShop(); Busy.Day = 60; Busy.RivalSeed = 5;
    for (int32 I = 0; I < 3; ++I) Busy.Goals.BadEventDays.Add(Busy.Day - 1);
    int32 BadAfter = 0;
    for (int32 D = 0; D < 6; ++D)
    {
        Busy.Day += 1; Busy.DayNews.Reset(); Busy.Decisions.Reset();
        const int32 Before = Busy.Goals.BadEventDays.Num();
        MarketEvents::CloseDay(Busy, Catalog());
        BadAfter += Busy.Goals.BadEventDays.Num() - Before;
    }
    TestEqual(TEXT("No new bad event in the held week"), BadAfter, 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketGoalsLeagueTest, "MirasMarket.Goals.LeagueFinale", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketGoalsLeagueTest::RunTest(const FString& Parameters)
{
    // J02: two league years in a row as the first (C's world league calls OnLeagueYear), with a healthy balance.
    using namespace MarketGoalsTest;
    using namespace MarketGoals;
    FMarketState S = NewShop(); S.Story.Chapter = 7; S.Day = 400;
    Close(S, 50000, 5000);
    S.Goals.LeagueYearEbitda = 10000000;
    OnLeagueYear(S, 1);
    TestEqual(TEXT("One year first"), S.Goals.LeagueFirstYears, 1);
    TestTrue(TEXT("Celebrated"), S.Goals.Celebrations.ContainsByPredicate([](const FMarketCelebration& C) { return C.Title.Contains(TEXT("birincisi")); }));
    TestFalse(TEXT("Not yet the end"), MarketStory::StoryClosed(S));
    TestTrue(TEXT("Chapter goal line"), MarketStory::Objectives(S).ContainsByPredicate([](const MarketStory::FObjective& O) { return O.Text.Contains(TEXT("D\u00fcnya liginde")); }));
    S.Goals.LeagueYearEbitda = 10000000;
    OnLeagueYear(S, 1);
    TestTrue(TEXT("Miras"), MarketStory::StoryClosed(S) && S.Story.Ending == static_cast<uint8>(MarketStory::EEnding::Legacy));

    FMarketState Debt = NewShop(); Debt.Story.Chapter = 7;
    FMarketLoan Loan; Loan.Principal = Loan.Remaining = 50000000; Debt.Loans.Add(Loan);
    Debt.Goals.LeagueYearEbitda = 10000000;
    OnLeagueYear(Debt, 1);
    TestEqual(TEXT("Deep in debt: not counted"), Debt.Goals.LeagueFirstYears, 0);
    FMarketState Second = NewShop();
    Second.Goals.LeagueFirstYears = 1; Second.Goals.LeagueYearEbitda = 10000000;
    OnLeagueYear(Second, 2);
    TestEqual(TEXT("Second place breaks the run"), Second.Goals.LeagueFirstYears, 0);
    return true;
}

#endif

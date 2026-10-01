#pragma once

#include "CoreMinimal.h"
#include "MarketGoals.generated.h"

struct FMarketState;
struct FMarketProduct;

// Ak\u0131\u015f B6 (Docs/Kurgu/07_AKIL_ISBOLUMU.md \u00a74, 06_GIDIS_YOLU.md \u00a72b "bir tur daha"): goals, milestones, records,
// celebrations and the rhythm guard. Independent of the world, seeded, tested (MirasMarket.Goals.*).
//
//  - Goals in three scales run at the same time: short (this week), medium (this month), long (this year / the
//    chapter). They follow the player's stage (family shop -> first branches -> provinces -> the country ->
//    abroad) and the player's own recent numbers: never the same kind twice in a row, never one whose door is still
//    shut, never one already as good as done. A short goal is always on the screen (a shelf / service goal is the
//    fallback), so something is always about to finish.
//  - Firsts (first branch, 10th shop, debt closed...) and records (best day / week / month, most shops) write a
//    celebration (title, one sentence, importance 0..2). Rewards are small and meaningful: a memory, the team's
//    morale, the wholesaler's trust; never a big sum of money.
//  - Rhythm guard: a long quiet stretch (no event, no decision, no milestone; 15 / 20 / 25 days by difficulty)
//    brings a pleasant or interesting event; too many bad events in 7 days (2 / 3 / 4 by difficulty) hold the next
//    bad one back (MarketEvents asks HoldBadEvent).
//  - The J02 finale hook for the world league (MarketChains, Ak\u0131\u015f C): OnLeagueYear.

// One running goal (MarketGoals::EGoal / EScale as uint8).
USTRUCT()
struct FMarketGoal
{
    GENERATED_BODY()
    UPROPERTY() uint8 Kind = 0;
    UPROPERTY() uint8 Scale = 0;
    UPROPERTY() int32 StartDay = 0;
    UPROPERTY() int32 DueDay = 0;
    UPROPERTY() int64 Base = 0;          // the measure when the goal came
    UPROPERTY() int64 Target = 0;        // the measure that completes it
    UPROPERTY() int64 Value = 0;         // the measure at the last close
};

// One celebration card (the menu shows the newest ones; MarketGoals::Celebrations).
USTRUCT()
struct FMarketCelebration
{
    GENERATED_BODY()
    UPROPERTY() int32 Day = 0;           // the closed day it happened on
    UPROPERTY() FString Title;           // "\u0130lk \u015fube!"
    UPROPERTY() FString Text;            // one sentence
    UPROPERTY() uint8 Importance = 0;    // 0 small, 1 notable, 2 big
};

USTRUCT()
struct FMarketGoals
{
    GENERATED_BODY()
    UPROPERTY() bool bStarted = false;       // false: a new campaign before its first close
    UPROPERTY() int32 LastClosedDay = 0;     // the same close twice does nothing
    UPROPERTY() TArray<FMarketGoal> Goals;
    UPROPERTY() TArray<uint8> RecentKinds;   // the last goals finished or dropped (no repeat)
    UPROPERTY() TArray<FMarketCelebration> Celebrations;
    UPROPERTY() int64 Firsts = 0;            // bit per MarketGoals::EFirst
    // Records (company-wide, internal kuru\u015f).
    UPROPERTY() int64 BestDayRevenue = 0;
    UPROPERTY() int64 BestWeekRevenue = 0;
    UPROPERTY() int64 BestMonthProfit = 0;
    UPROPERTY() int32 MostStores = 0;
    UPROPERTY() int32 RecordDay = 0;         // last record celebration (records are told at most once a week)
    // The last 30 closed days (company revenue, company profit, family-shop shelf fill in per mille).
    UPROPERTY() TArray<int64> RecentRevenue;
    UPROPERTY() TArray<int64> RecentProfit;
    UPROPERTY() TArray<int32> RecentFill;
    UPROPERTY() int32 DaysCounted = 0;
    // Rhythm.
    UPROPERTY() int32 LastLivelyDay = 0;     // last closed day with an event, a decision or a milestone
    UPROPERTY() TArray<int32> BadEventDays;  // closed days of bad events (last 7 days)
    UPROPERTY() int32 QuietEvents = 0;       // pleasant events the guard brought
    UPROPERTY() int32 HeldBadEvents = 0;     // bad events it held back
    // J02 (the world league, MarketChains): operating result of the running league year and the years in a row
    // as the first.
    UPROPERTY() int64 LeagueYearEbitda = 0;
    UPROPERTY() int32 LeagueFirstYears = 0;
    UPROPERTY() int32 LastLeagueRank = 0;
};

namespace MarketGoals
{
    enum class EScale : uint8 { Short = 0, Medium, Long, Count };
    enum class EGoal : uint8
    {
        // short
        DayRevenue = 0,   // a day's revenue
        WeekProfit,       // net profit this week
        ShelvesFull,      // shelves 90 % full three closes in a row
        PayDebt,          // pay part of the inherited debt
        Service,          // shoppers served this week (always possible)
        // medium
        MoreStores,
        MonthProfit,
        LocalShare,
        FirstDepot,
        DebtFree,
        // long
        Chapter,          // the chapter's goals (MarketStory::Objectives)
        Provinces,
        NationalShare,    // per mille of a percent: Target 100 = 0.1 %
        Abroad,
        YearProfit,
        Count
    };
    enum class EFirst : uint8
    {
        ProfitDay = 0, DebtCleared, FirstBranch, Stores5, Stores10, Stores25, Stores50, Stores100, Stores250, Stores500, Stores1000,
        Provinces2, Provinces5, Provinces10, Provinces20, FirstDepot, Abroad, OnlineOrder, Share01, Share1, League10, League3, League1,
        Count
    };

    // The player's stage: 0 family shop, 1 first branches, 2 more provinces, 3 the country, 4 abroad.
    int32 Stage(const FMarketState& State);

    struct FGoalView
    {
        EGoal Kind = EGoal::Service;
        EScale Scale = EScale::Short;
        FString Title;      // "Bu hafta bir g\u00fcnde 850,00 TL ciro"
        FString Why;        // one sentence: why it matters
        FString Reward;     // what happens when it is done
        float Progress = 0.f; // 0..1
        int32 DaysLeft = 0;
    };
    TArray<FGoalView> Goals(const FMarketState& State);
    // The goal closest to done (the strip and "\u015eimdi ne yapmal\u0131"); false when there is none (before the first close).
    bool NextGoal(const FMarketState& State, FGoalView& Out);
    // "Bu hafta: 3 g\u00fcn \u00fcst \u00fcste raflar\u0131 %90 dolu tut (1/3)"; "" when there is no goal.
    FString StripText(const FMarketState& State);

    // Celebrations of a closed day (the day report) and the newest ones.
    TArray<FMarketCelebration> CelebrationsOn(const FMarketState& State, int32 Day);
    TArray<FMarketCelebration> RecentCelebrations(const FMarketState& State, int32 MaxCount = 10);

    struct FRecordView { FString Name; FString Value; };
    // Records for the reports page ("En iyi g\u00fcn", "12.450,00 TL").
    TArray<FRecordView> Records(const FMarketState& State);

    // Rhythm guard: a new bad event should wait (MarketEvents asks before starting one).
    bool HoldBadEvent(const FMarketState& State);
    bool IsBadEvent(const FString& EventId);
    int32 QuietDays(const FMarketState& State);   // difficulty threshold of a quiet stretch
    int32 BadLimit(const FMarketState& State);    // bad events allowed in 7 days

    // J02: C's world league calls this when a league year closes (Rank 1 = first). Two league years in a row as the
    // first, with a positive operating result and debt below three times it, in chapter 7, bring "Miras".
    constexpr int32 FinaleYears = 2;
    void OnLeagueYear(FMarketState& State, int32 Rank, bool bFullYear = true);

    // Day close (MarketDirector, Ak\u0131\u015f B block at the end, after MarketEvents and the books).
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);
}

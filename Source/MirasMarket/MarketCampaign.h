#pragma once
#include "CoreMinimal.h"
#include "MarketEconomy.h"

// The first chapter's goal (G-054), independent of the world (test: MirasMarket.Campaign.DebtAndWeek).
// The family shop comes with a debt to the wholesaler written in the father's old account book. There is no
// deadline and no game over: the player pays it at the office desk when the cash allows (P, 50 TL at a time).
// Until it is closed the second branch cannot be opened, so a shop that never pays it simply stays small.
// Every seven days a weekly report sums up the week.
namespace MarketCampaign
{
    constexpr int64 StartingDebt = 30000;   // 300 TL
    constexpr int64 Installment = 5000;     // one press of P: 50 TL (or what is left)
    constexpr int32 DaysPerWeek = 7;
    constexpr int32 MaxHistoryDays = 3650;
    // Second branch conditions (v0.1 values) plus the closed debt.
    constexpr int64 ExpandCash = 95000;
    int64 ExpandCashOn(int32 GameDay);   // G-077 (#36): at today's prices
    constexpr int32 ExpandProfitableDays = 3;
    constexpr float ExpandShare = 35.f;

    // Pays min(Amount, debt, cash). Returns the amount paid (0 = nothing). Sets DebtClearedDay when it closes.
    int64 PayDebt(FMarketState& State, int64 Amount = Installment);
    bool DebtOpen(const FMarketState& State);
    // 0..1 of the starting debt paid (for the goal bar).
    float DebtProgress(const FMarketState& State);

    enum class EExpandBlock : uint8 { None, Debt, Cash, ProfitableDays, Share, AlreadyOpen };
    // First reason the second branch cannot be opened yet (None = it can).
    EExpandBlock ExpandBlock(const FMarketState& State);

    // Week of a day: days 1-7 = week 1.
    int32 WeekOf(int32 Day);
    // Call right after FMarketState::CloseDay (State.Day is already the next day): adds the closed day to the
    // running week and appends the day to State.History. When that day finished a week, the week moves to LastWeek* and true is returned.
    bool CloseDay(FMarketState& State);
}

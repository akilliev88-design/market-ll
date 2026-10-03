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
    // The first branch waits for the closed debt, a few profitable days and a share of the province (E3a/K5: the v0.1
    // cash condition is gone; MarketBranches::CanOpen checks the real opening cost).
    constexpr int32 ExpandProfitableDays = 3;
    // The surroundings share goal of the first branch and chapter 2 (percent of the shoppers around the family shop,
    // not of the city): 1.25 x what an ordinary shop gets in the home province (MarketStoreDemand::NeutralShare;
    // the same rule for every province, M61/M61b: Istanbul ~26 %, a median province ~33 %, a small one ~42 %),
    // 15..55. LeadShare: the leadership year's share, 1.43 x, 18..60.
    float ShareGoal(const FMarketState& State);
    float LeadShare(const FMarketState& State);

    // M37: one payment is a tenth of the campaign's starting debt (at least Installment).
    int64 InstallmentOf(const FMarketState& State);
    // Pays min(Amount, debt, cash); Amount < 0 = one installment. Returns the amount paid (0 = nothing). Sets
    // DebtClearedDay when it closes.
    int64 PayDebt(FMarketState& State, int64 Amount = -1);
    bool DebtOpen(const FMarketState& State);
    // 0..1 of the starting debt paid (for the goal bar).
    float DebtProgress(const FMarketState& State);

    // Week of a day: days 1-7 = week 1.
    int32 WeekOf(int32 Day);
    // Call right after FMarketState::CloseDay (State.Day is already the next day): adds the closed day to the
    // running week and appends the day to State.History. When that day finished a week, the week moves to LastWeek* and true is returned.
    bool CloseDay(FMarketState& State);
}

#pragma once
#include "CoreMinimal.h"
#include "MarketEconomy.h"

// The inherited debt and the week (G-054, M69), independent of the world (test: MarketSim.Campaign.DebtAndWeek).
// The market came with a light debt to the wholesaler. No deadline, no interest, no goal: it locks nothing, and the
// player pays an installment (P) or all of it (Finance) whenever they like. Every seven days a weekly report sums
// up the week.
namespace MarketCampaign
{
    constexpr int64 StartingDebt = 30000;   // 300 TL
    constexpr int64 Installment = 5000;     // one press of P: 50 TL (or what is left)
    constexpr int32 DaysPerWeek = 7;
    constexpr int32 MaxHistoryDays = 3650;
    // The first branch waits for a few profitable days and a share of the first store's surroundings (M69: never for
    // the debt; MarketBranches::CanOpen checks the real opening cost).
    constexpr int32 ExpandProfitableDays = 3;
    // The surroundings share the first branch waits for (percent of the shoppers around the first store,
    // not of the city): 1.25 x what an ordinary shop gets in the home province (MarketStoreDemand::NeutralShare;
    // the same rule for every province, M61/M61b: Istanbul ~26 %, a median province ~33 %, a small one ~42 %),
    // 15..55. LeadShare: a strong local share (1.43 x, 18..60; the bot's yardstick).
    float ShareGoal(const FMarketState& State);
    float LeadShare(const FMarketState& State);

    // M37: one payment is a tenth of the campaign's starting debt (at least Installment).
    int64 InstallmentOf(const FMarketState& State);
    // Pays min(Amount, debt, cash); Amount < 0 = one installment. Returns the amount paid (0 = nothing). Sets
    // DebtClearedDay when it closes.
    int64 PayDebt(FMarketState& State, int64 Amount = -1);
    bool DebtOpen(const FMarketState& State);
    // 0..1 of the starting debt paid (for the progress bar).
    float DebtProgress(const FMarketState& State);

    // Week of a day: days 1-7 = week 1.
    int32 WeekOf(int32 Day);
    // Call right after FMarketState::CloseDay (State.Day is already the next day): adds the closed day to the
    // running week and appends the day to State.History. When that day finished a week, the week moves to LastWeek* and true is returned.
    bool CloseDay(FMarketState& State);
}

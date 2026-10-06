#include "MarketCampaign.h"
#include "MarketLedger.h"
#include "MarketPrices.h"
#include "MarketStart.h"
#include "MarketStoreDemand.h"

float MarketCampaign::ShareGoal(const FMarketState& State)
{
    const float Neutral = MarketStoreDemand::NeutralShare(State, State.CountryId, MarketStart::HomeProvince(State));
    return FMath::Clamp(static_cast<float>(FMath::RoundToInt32(Neutral * 125.f)), 15.f, 55.f);
}

float MarketCampaign::LeadShare(const FMarketState& State)
{
    const float Neutral = MarketStoreDemand::NeutralShare(State, State.CountryId, MarketStart::HomeProvince(State));
    return FMath::Clamp(static_cast<float>(FMath::RoundToInt32(Neutral * 143.f)), 18.f, 60.f);
}

int64 MarketCampaign::InstallmentOf(const FMarketState& State)
{
    return FMath::Max<int64>(Installment, State.StartDebt / 10 / 100 * 100);
}

int64 MarketCampaign::PayDebt(FMarketState& State, int64 Amount)
{
    if (Amount < 0) Amount = InstallmentOf(State);
    const int64 Paid = FMath::Max<int64>(0, FMath::Min3(Amount, State.InheritedDebt, State.Cash));
    if (Paid <= 0) return 0;
    State.Cash -= Paid;
    MarketLedger::Post(State, MarketLedger::EAccount::InheritedDebt, -Paid); // B2
    State.InheritedDebt -= Paid;
    State.WeekDebtPaid += Paid;
    if (State.InheritedDebt == 0 && State.DebtClearedDay == 0) State.DebtClearedDay = State.Day;
    return Paid;
}

bool MarketCampaign::DebtOpen(const FMarketState& State)
{
    return State.InheritedDebt > 0;
}

float MarketCampaign::DebtProgress(const FMarketState& State)
{
    return FMath::Clamp(1.f - static_cast<float>(State.InheritedDebt) / static_cast<float>(FMath::Max<int64>(1, State.StartDebt)), 0.f, 1.f);
}

int32 MarketCampaign::WeekOf(int32 Day)
{
    return (FMath::Max(1, Day) - 1) / DaysPerWeek + 1;
}

bool MarketCampaign::CloseDay(FMarketState& State)
{
    const int32 ClosedDay = State.Day - 1;
    if (ClosedDay < 1) return false;
    State.WeekRevenue += State.LastRevenue;
    State.WeekProfit += State.LastProfit;
    State.WeekServed += State.LastServed;
    State.WeekLost += State.LastLost;
    FMarketDayRecord Record;
    Record.Day = ClosedDay; Record.Revenue = State.LastRevenue; Record.Profit = State.LastProfit;
    Record.Served = State.LastServed; Record.Lost = State.LastLost; Record.MarketShare = State.MarketShare; Record.Cash = State.Cash;
    State.History.Add(Record);
    if (State.History.Num() > MaxHistoryDays) State.History.RemoveAt(0, State.History.Num() - MaxHistoryDays);
    if (ClosedDay % DaysPerWeek != 0) return false;
    State.LastWeekNumber = WeekOf(ClosedDay);
    State.LastWeekRevenue = State.WeekRevenue;
    State.LastWeekProfit = State.WeekProfit;
    State.LastWeekDebtPaid = State.WeekDebtPaid;
    State.LastWeekServed = State.WeekServed;
    State.LastWeekLost = State.WeekLost;
    State.WeekRevenue = State.WeekProfit = State.WeekDebtPaid = 0;
    State.WeekServed = State.WeekLost = 0;
    return true;
}

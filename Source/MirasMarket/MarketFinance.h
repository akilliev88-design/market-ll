#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Money beyond the till (G-067, Docs/Kurgu/00_KURGU_KITABI.md \u00a711). Independent of the world, tested
// (MirasMarket.Finance.*).
//  - the country's local bank (MarketCast::Bank(0), fictional): a 12-month loan, limit from the last 30 days' profit plus the family name; the
//    interest follows the year (MarketPrices::LoanRate). Installments every 30 days are paid from the till; the
//    interest is a finance cost of that day; early repayment costs 1 %.
//  - The money trouble ladder. There is no game over: cash below zero at a day close starts it, and every step is
//    told before it happens. 1 day: warning. 3 days: the wholesaler closes the payment terms. 7 days: a choice
//    (an emergency loan at a high rate, or sell the depot at half price). 14 days: the depot is sold at half price.
//    30 days: the bank offers a mortgage on the family shop's deed. Positive cash ends the trouble.
//  - Karar M31 (Mustafa 01.10.2026): no zombie company. 45 days: the bank announces a rescue plan. 60 days: the plan
//    runs (Rescue): subsidiaries are sold, losing branches close (the worst first; stock to the depot or sold off,
//    deposits back), managers with nothing left to run leave unpaid, and when no branch is left the family shop
//    keeps at most two workers; the ads, the app, the fast delivery and the dark stores stop. C7 (Codex C4/C5:
//    321 plans and 500 million of debt in one campaign): every loan (family, company, the line) and the money for a
//    month and full shelves become ONE plan loan of 48 months the shop can carry (4 % of its monthly revenue), the
//    rest is written off, the first installment comes half a year later; two years without new loans or branches.
//    A missed installment costs its late fee once a month (it was every day). The game goes on: back to the family
//    shop, a lesson learned.
//  - The family lives from the shop: every evening 30 TL (x the minimum wage index) goes home; half of it when
//    the till is tight and nothing when it is empty. It is not a business cost (the day's net stays), only cash.
//  - At every month end a short report: revenue, net result, money taken home, cash, what customers owe, what
//    the shop owes.
namespace MarketFinance
{
    constexpr int32 LoanMonths = 12;
    constexpr int32 MonthDays = 30;
    constexpr float EarlyRepayFee = 0.01f;
    constexpr float LateFee = 0.03f;
    constexpr double EmergencyRateBonus = 0.12;   // yearly, on top of the year's rate
    // B1 (#43): the mortgage is no reward for a month in the red: a risk premium on the rate, a valuation and deed
    // fee paid from the loan, and only what the shop needs (1.5 x the hole, at least 500 at the start level, at
    // most 3 000).
    constexpr double MortgageRateBonus = 0.06;
    constexpr float MortgageFee = 0.02f;
    constexpr int64 MortgageMin = 50000;
    constexpr int64 MortgageMax = 300000;
    // The mortgage's size tonight (start-level limits x the price list).
    int64 MortgageAmount(const FMarketState& State);
    constexpr int64 HouseholdDraw = 3000;         // start-level kurus a day taken home, follows the minimum wage
    const int64 LoanSteps[3] = { 50000, 100000, 250000 };   // 500 / 1.000 / 2.500 TL at the start price level

    int64 Installment(int64 Principal, double MonthlyRate, int32 Months);
    int64 LoanLimit(const FMarketState& State);
    int64 Debt(const FMarketState& State);
    bool TakeLoan(FMarketState& State, int32 Step, FString& OutMessage);
    bool RepayAll(FMarketState& State, FString& OutMessage);
    FString Summary(const FMarketState& State);

    // Day close (after the other systems): installments, the trouble ladder, the month-end report.
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);
    // Money the family takes home tonight at this cash level.
    int64 HouseholdToday(const FMarketState& State);
    // M31: the bank's rescue plan.
    constexpr int32 RescueWarnDays = 45;
    constexpr int32 RescueDays = 60;
    constexpr int32 RescueMonths = 48;            // C7: one loan for everything (was 36 and a new loan every time)
    constexpr double RescueRateBonus = 0.06;      // yearly, on top of the year's rate
    constexpr double RescueRepeatBonus = 0.02;    // C7: + 2 points for every earlier rescue, at most RescueRepeatCap
    constexpr double RescueRepeatCap = 0.06;
    constexpr float RescueCarryShare = 0.04f;     // C7: the installment the shop can carry: 4 % of its monthly revenue
    constexpr int32 RescueGraceDays = 180;        // C7: the first installment half a year later
    constexpr int32 RescueBlockDays = 730;        // C7: two years without new loans or branches
    constexpr int32 RescueKeepStaff = 2;
    constexpr int64 RescueWorkingCapital = 100000; // 1 000 TL at the start level: enough to fill the shelves again
    // Runs the plan now (the ladder calls it at RescueDays); returns the lines told to the player.
    TArray<FString> Rescue(FMarketState& State, const TArray<FMarketProduct>& Products);
    // A "finance.*" decision (called by MarketEvents::Decide).
    bool Resolve(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& Decision, int32 Option, FString& OutMessage);
}

#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Money beyond the till (G-067, Docs/Kurgu/00_KURGU_KITABI.md \u00a711). Independent of the world, tested
// (MirasMarket.Finance.*).
//  - Trakya Bankas\u0131 (fictional): a 12-month loan, limit from the last 30 days' profit plus the family name; the
//    interest follows the year (MarketPrices::LoanRate). Installments every 30 days are paid from the till; the
//    interest is a finance cost of that day; early repayment costs 1 %.
//  - The money trouble ladder. There is no game over: cash below zero at a day close starts it, and every step is
//    told before it happens. 1 day: warning. 3 days: the wholesaler closes the payment terms. 7 days: a choice
//    (an emergency loan at a high rate, or sell the depot at half price). 14 days: the depot is sold at half price.
//    30 days: the bank offers a mortgage on the family shop's deed. Positive cash ends the trouble.
//  - At every month end a short report: revenue, net result, cash, what customers owe, what the shop owes.
namespace MarketFinance
{
    constexpr int32 LoanMonths = 12;
    constexpr int32 MonthDays = 30;
    constexpr float EarlyRepayFee = 0.01f;
    constexpr float LateFee = 0.03f;
    constexpr double EmergencyRateBonus = 0.12;   // yearly, on top of the year's rate
    const int64 LoanSteps[3] = { 50000, 100000, 250000 };   // 500 / 1.000 / 2.500 TL at 2011 prices

    int64 Installment(int64 Principal, double MonthlyRate, int32 Months);
    int64 LoanLimit(const FMarketState& State);
    int64 Debt(const FMarketState& State);
    bool TakeLoan(FMarketState& State, int32 Step, FString& OutMessage);
    bool RepayAll(FMarketState& State, FString& OutMessage);
    FString Summary(const FMarketState& State);

    // Day close (after the other systems): installments, the trouble ladder, the month-end report.
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);
    // A "finance.*" decision (called by MarketEvents::Decide).
    bool Resolve(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& Decision, int32 Option, FString& OutMessage);
}

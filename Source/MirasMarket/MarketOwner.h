#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Karar M37 (Mustafa 02.10.2026: "benim maa\u015f\u0131m da olmal\u0131, biz de insan\u0131z; oyun i\u00e7inde ki\u015fisel servetimiz olsun").
// The owner's money is not the company's money. Independent of the world, tested (MirasMarket.Owner.*).
//  - Salary: the company pays us a monthly salary like any manager (a multiple of the minimum wage we choose, at
//    least the minimum wage). It is a cost of the company (wages + the employer's social security, booked at the
//    head office); income tax and our social security are cut from it, the net goes to our personal wealth. An
//    empty till does not pay it: that month we go without.
//  - Dividend: once the year's books are closed we may take part of last year's net profit out of the company;
//    a withholding tax is cut. Not under the bank's rescue plan, never more than the till keeps above a month of
//    the company's fixed costs.
//  - Capital: we may put personal money into the company (a hard month, a big opening).
//  - Living: the family lives from our personal wealth (a monthly cost that follows the minimum wage). Later:
//    things to buy in our personal life.
//  - The bank's rescue plan cuts our salary to the minimum wage.
namespace MarketOwner
{
    // Salary steps: tenths of the minimum wage (1x .. 40x).
    constexpr int32 StepCount = 8;
    const int32 SalarySteps[StepCount] = { 10, 15, 20, 30, 50, 100, 200, 400 };
    constexpr int32 StartSalaryX10 = 15;          // a new owner pays himself 1.5 x the minimum wage
    constexpr float LivingX = 0.9f;               // the family's monthly living: 0.9 x the minimum wage
    constexpr float EmployeeSocialRate = 0.15f;   // our own social security share (capped)
    constexpr int32 SocialCapX = 75;              // social security counts up to 7.5 x the minimum wage
    constexpr float DividendWithholding = 0.15f;
    constexpr int32 DividendSteps = 3;
    const int32 DividendPercents[DividendSteps] = { 25, 50, 100 };
    const int32 CapitalPercents[DividendSteps] = { 25, 50, 100 };

    // The month's minimum wage (gross, kurus) today.
    int64 MinimumMonthly(const FMarketState& State);
    int64 GrossSalary(const FMarketState& State);
    // Income tax (progressive) and our social security of a gross salary; the net.
    int64 SalaryTax(const FMarketState& State, int64 Gross);
    int64 SalarySocial(const FMarketState& State, int64 Gross);
    int64 NetSalary(const FMarketState& State);
    // What the company pays for it (gross + the employer's share).
    int64 CompanyCost(const FMarketState& State);
    int64 Living(const FMarketState& State);

    bool SetSalary(FMarketState& State, int32 Step, FString& OutMessage);
    // Last closed year's net profit not yet paid out, and what the till allows now.
    int64 Distributable(const FMarketState& State);
    int64 DividendRoom(const FMarketState& State);
    bool PayDividend(FMarketState& State, int32 Step, FString& OutMessage);
    bool PutCapital(FMarketState& State, int32 Step, FString& OutMessage);
    // The rescue plan: back to the minimum wage.
    void CutToMinimum(FMarketState& State, TArray<FString>& Lines);

    // Month start (called by MarketFinance's month end): the salary, the family's living.
    void MonthStart(FMarketState& State);
    // Menu lines.
    FString Summary(const FMarketState& State);
}

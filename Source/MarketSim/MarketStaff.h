#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

namespace MarketCountry { struct FProfile; }

// The people of the shop and the books (G-060). Pure rules without the world, so they are tested
// (MarketSim.Staff.*). The game calls CloseDay once per day close and asks the rules below for speeds.
// Design and numbers: Docs/PERSONEL_VE_MUHASEBE.md.
//
//  Kasiyer        takes payments. Speed/skill -> seconds per basket; skill/fatigue -> small till differences;
//                 a rare dishonest one makes small shortages (never proven by one day, see the accountant).
//  Reyon gorevlisi refills shelves (StaffPlanner). Speed -> walking/hand speed; skill -> carried units and
//                 whether he may put new blocks on the shelves; many units a day -> fatigue.
//  IK muduru      unlocks with 3 shop employees. Talks to the unhappiest person every day, gives days off to
//                 tired people when someone can cover, replaces leavers from the pool, negotiates wages,
//                 shows exact candidate values and a reference note.
//  Mali musavir   outside service (the shop's old accountant). Keeps the weekly books, declares and pays the
//                 tax on time, finds documented expenses (lower tax), prevents audit fines and points at
//                 suspicious till patterns. Without him the player pays the tax (PayTax) and risks an audit.
namespace MarketStaff
{
    enum class ERole : uint8 { Cashier = 0, Stocker = 1, HrManager = 2, Accountant = 3 };

    constexpr int32 MaxCashiers = 2;
    constexpr int32 MaxHrManagers = 1;
    constexpr int32 HrUnlockStaff = 8;            // C9 (Codex C8: an HR manager for a three-person shop ate its profit): cashiers + stockers before an HR manager is offered
    constexpr int64 HireCost = 12000;             // cashier / stocker: 120 TL (as in v0.1)
    constexpr int64 HrHireCost = 20000;
    constexpr int64 AccountantDailyFee = 400;     // 4 TL per day (28 TL a week)
    constexpr int32 SeveranceDays = 3;            // firing pays three days' wage
    constexpr int32 NoticeDays = 2;               // someone who resigns works two more days
    constexpr int32 PoolSize = 3;                 // without HR: 3 candidates, refreshed weekly
    constexpr int32 HrPoolSize = 6;               // with HR: 6 candidates, refreshed every 3 days
    // Taxes (simplified game model, not real law): one period = one week, declared at its last day's close.
    constexpr float VatRate = 0.08f;              // on sales minus purchases (effective mix of 1/10/20 %)
    constexpr float IncomeTaxRate = 0.15f;        // on a positive weekly result
    constexpr float AccountantDeduction = 0.90f;  // documented expenses: the accountant's declaration is 10 % lower
    constexpr int32 TaxPayDays = 3;               // closed on day 7 -> must be paid by the close of day 10
    constexpr float LatePenaltyFirst = 0.05f;
    constexpr float LatePenaltyPerDay = 0.01f;
    constexpr float LatePenaltyCap = 0.50f;
    constexpr uint32 AuditOneIn = 6;              // without an accountant: one week in six is inspected
    constexpr float AuditPenaltyRate = 0.25f;
    constexpr int64 AuditPenaltyMin = 2000;

    FString RoleName(ERole Role);
    ERole RoleOf(const FMarketEmployee& Employee);

    // B3 (#39): wages, social security and seniority pay (Docs/Surec/akislar/B.md). The same rules are meant for
    // the branches' workers and the managers (MarketBranches, MarketManagers: Ak\u0131\u015f C wires them).
    // The lowest daily wage of the day: the net monthly minimum wage (MarketPrices) / 30 x the country's wage
    // factor, rounded up to 50 kuru\u015f. Nobody on the payroll earns less; the accountant is a fee, not a wage.
    int64 MinimumDailyWage(int32 GameDay);
    // D3: a country's pool of staff names: its names.staff, else its names, else the default country's.
    void StaffNames(const MarketCountry::FProfile& Country, TArray<FString>& OutFirst, TArray<FString>& OutLast);
    // The employer's social security share of the active country (MarketCountry: employerSocialRate).
    float EmployerSocialRate();
    // Social security the employer pays on a wage, and wage + that share.
    int64 EmployerShare(int64 Wage);
    int64 EmployerCost(int64 Wage);
    // Seniority pay (k\u0131dem tazminat\u0131) when the employer lets someone go: severanceDaysPerYear days' wage per year of
    // service, from the first full year on (pro rata beyond it). Resignations get none.
    constexpr int32 SeniorityAfterDays = 365;
    int64 SeniorityPay(int64 DailyWage, int32 HiredDay, int32 GameDay);
    // Notice pay (SeveranceDays' wage) + seniority pay of an employee fired today; 0 for the accountant.
    int64 SeverancePay(const FMarketState& State, const FMarketEmployee& Employee);
    // Today's social security on the payroll (the accountant's fee excluded).
    int64 DailySocialSecurity(const FMarketState& State);

    // Wage the market pays for this role and skill on a game day (follows the minimum wage, MarketPrices).
    // Morale compares the actual wage with it, so a wage that is never raised slowly becomes a low wage.
    int64 FairWage(ERole Role, int32 Skill, int32 GameDay = 1);
    // G-077 (#36): hiring cost at today's wage level (HireCost / HrHireCost are start-level values (M24)).
    int64 HireCostOn(ERole Role, int32 GameDay);
    // True when the employee works today (hired, not on the day off). Off = rests, still paid.
    bool OnDuty(const FMarketState& State, const FMarketEmployee& Employee);
    int32 Count(const FMarketState& State, ERole Role);
    // Index-th employee of a role who works today (hiring order), or nullptr. The world maps workers/till to it.
    const FMarketEmployee* OnDutyAt(const FMarketState& State, ERole Role, int32 Index);
    FMarketEmployee* FindEmployee(FMarketState& State, int32 Id);
    bool HasHr(const FMarketState& State);
    bool HasAccountant(const FMarketState& State);
    bool HrUnlocked(const FMarketState& State);

    // G-084 (karar L03): the people who come with an inherited shop. Ordinary candidates (random skill, fair wage),
    // hired on the current day without a hiring cost.
    void AddStartingStaff(FMarketState& State, int32 Cashiers, int32 Stockers);
    // E3c2c (M63): the people of a branch (FMarketBranch::Staff), cashiers and stockers like the first store's,
    // hired, replaced and looked after by the store manager (better candidates with the company's HR manager).
    // StaffBranch fills the store's positions (B.Workers: half of them, at least one, at the tills) and books the
    // hiring cost to the branch; returns the number hired.
    // bWithCost false: the people came with a bought store (no hiring cost).
    int32 StaffBranch(FMarketState& State, int32 BranchIndex, bool bWithCost = true);
    // The branch's wages today (every person on its payroll).
    int64 BranchWages(const FMarketBranch& Branch);
    // x the store's service from its people: skill, morale and fatigue of those on duty (1 for an average team; an
    // empty or exhausted team serves worse, a skilled happy one better), 0.75..1.15.
    float BranchService(const FMarketBranch& Branch, int32 Day);
    // A branch's closed day for its people: the minimum wage, the tills (honest mistakes, a dishonest cashier), fatigue
    // and learning (a rota gives each person one day off a week), morale, notices and leaving; the manager replaces
    // who left. Returns the till difference (the caller books it) and fills the counts for the weekly line.
    struct FBranchStaffDay
    {
        int64 TillDifference = 0;
        int32 Left = 0;
        int32 Hired = 0;
    };
    FBranchStaffDay BranchDay(FMarketState& State, int32 BranchIndex, int32 Served, int64 Revenue, int32 UnitsSold, int32 Closed);
    // Who works today (E3c2: from the roster, no stored flags): a cashier at the till, the stockers the world
    // spawns (at most FMarketState::MaxStockers).
    bool CashierOnDuty(const FMarketState& State);
    int32 StockersOnDuty(const FMarketState& State);
    // Fills the hiring pool when it is empty (always at least one cashier and one stocker candidate).
    void EnsureCandidates(FMarketState& State);
    // One line for the menu; the HR manager shows exact values and a reference note.
    FString DescribeCandidate(const FMarketState& State, const FMarketEmployee& Candidate);
    FString DescribeEmployee(const FMarketState& State, const FMarketEmployee& Employee);

    // Player / HR decisions. All return false with a Turkish OutMessage when nothing changed.
    bool Hire(FMarketState& State, int32 CandidateIndex, FString& OutMessage);
    // Best candidate of the role by skill per wage (keys H and J).
    bool HireBest(FMarketState& State, ERole Role, FString& OutMessage);
    bool HireAccountant(FMarketState& State, FString& OutMessage);
    bool Fire(FMarketState& State, int32 EmployeeId, FString& OutMessage);
    // Newest employee of the role (key "FireStocker").
    bool FireLast(FMarketState& State, ERole Role, FString& OutMessage);
    bool Raise(FMarketState& State, int32 EmployeeId, FString& OutMessage);
    // bShopOpen: the day off is tomorrow; otherwise the next day to be played.
    bool GiveDayOff(FMarketState& State, int32 EmployeeId, bool bShopOpen, FString& OutMessage);
    // "The till is short, be careful." Deters a dishonest cashier; an honest one is hurt more.
    bool Warn(FMarketState& State, int32 EmployeeId, FString& OutMessage);
    // Pays the declared tax (and penalties) from the cash; returns the amount paid.
    int64 PayTax(FMarketState& State);

    // Seconds a cashier needs for a basket of Units (the till in the world waits this long).
    float CheckoutSeconds(const FMarketEmployee& Cashier, int32 Units);
    // 0.5..1.3: multiplies walking speed, divides load/hand times.
    float WorkSpeed(const FMarketEmployee& Employee);
    // Units a stocker carries in one trip (12..24).
    int32 CarryUnits(const FMarketEmployee& Employee);
    // Experienced stockers (skill >= 45) may put new blocks and widen blocks; the others only refill.
    bool MayEditPlan(const FMarketEmployee& Employee);
    // The world reports shelved units (fatigue).
    void RecordWork(FMarketState& State, int32 EmployeeId, int32 Units);

    // Call right after FMarketState::CloseDay (State.Day is already the next day). Settles the till, fatigue,
    // morale, notices, skill growth, HR and accountant work, the weekly tax and the hiring pool.
    // Writes State.StaffNews (day report) and the Last* fields. Deterministic per campaign seed and day.
    void CloseDay(FMarketState& State);
    // Tax + staff lines for the day report (StaffNews plus the open tax).
    FString ReportText(const FMarketState& State);
}

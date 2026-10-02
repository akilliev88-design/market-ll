#pragma once

#include "CoreMinimal.h"
#include "MarketLedger.generated.h"

struct FMarketState;
struct FMarketProduct;

// Ak\u0131\u015f B (Docs/Kurgu/07_AKIL_ISBOLUMU.md \u00a74, #37, #42): the company's books. One field of FMarketState (Ledger);
// a new campaign opens it at its first day close. Independent of the world, tested
// (MirasMarket.Ledger.*, MirasMarket.Balance.*).
//
// Every money movement is one entry: day, store (family shop -1, head office -2, branch index 0..), account, amount
// (kuru\u015f; + money in / income, - money out / cost) and whether the till moved (cash) or only the books (the cost of
// the goods sold, waste, an accrued tax). Income statements (day / week / month / year) come from the entries,
// the balance sheet from the state (cash, stock, receivables, debts). The audit at every day close checks
// "change of the till = sum of the cash entries"; what no system posted lands on Unexplained and the day's gap.
//
// Who posts: the Ak\u0131\u015f B systems post where the money moves (MarketLedger::Post). The family shop's till and
// wholesaler orders (FMarketState::SellBasket / SubmitOrder / CloseDay) are read from their day counters at the
// start of the day close (BeginClose). Branches, depots, managers, wholesalers' terms and the game mode post
// through the calls listed in Docs/Surec/akislar/B.md ("C'ye istekler") once they are wired.

// B1 (#27): what a running promotion earned, counted at every day close (MarketPromotions). Key: the promotion's
// kind, product and first day. Removed with the promotion's final report.
USTRUCT()
struct FMarketPromoTally
{
    GENERATED_BODY()
    UPROPERTY() uint8 Kind = 0;
    UPROPERTY() int32 Product = INDEX_NONE;
    UPROPERTY() int32 StartDay = 0;
    UPROPERTY() int64 Revenue = 0;       // estimated from the deal price of the covered units sold
    UPROPERTY() int64 CostOfGoods = 0;   // the units' book cost
    UPROPERTY() int64 Support = 0;       // the wholesaler's money: units received on the deal x the cost cut
};

// One posting (MarketLedger::EAccount as uint8).
USTRUCT()
struct FMarketLedgerEntry
{
    GENERATED_BODY()
    UPROPERTY() int32 Day = 0;
    UPROPERTY() int32 Store = -1;
    UPROPERTY() uint8 Account = 0;
    UPROPERTY() int64 Amount = 0;
    UPROPERTY() bool bCash = true;
};

// A calendar month's totals by account (index = MarketLedger::EAccount), kept for the whole campaign.
USTRUCT()
struct FMarketLedgerMonth
{
    GENERATED_BODY()
    UPROPERTY() int32 Year = 0;
    UPROPERTY() int32 Month = 0;
    UPROPERTY() TArray<int64> Totals;
};

USTRUCT()
struct FMarketLedger
{
    GENERATED_BODY()
    // B1 (#43): a losing tax week's loss, taken off the next weeks' taxable profit (MarketStaff).
    UPROPERTY() int64 TaxLossCarry = 0;
    // B1 (#45): our revenue per day in the campaign country, smoothed over about a month (MarketCompany::NationalShare).
    UPROPERTY() int64 CountryRevenueDay = 0;
    // B1 (#27, #30): running promotions' results; online units of the last closed day by catalog row (a promotion's
    // "before" counts the shop only, like its "during").
    UPROPERTY() TArray<FMarketPromoTally> PromoTallies;
    UPROPERTY() TArray<int32> OnlineSold;
    // B1 (#27): units sold below their cost on the last closed day and the loss on them (day report line).
    UPROPERTY() int32 LastBelowCostUnits = 0;
    UPROPERTY() int64 LastBelowCostLoss = 0;

    // B2: entries of the last MarketLedger::KeepDays days (older ones live on in Months).
    UPROPERTY() TArray<FMarketLedgerEntry> Entries;
    UPROPERTY() TArray<FMarketLedgerMonth> Months;
    // The audit: false until the campaign's first day close.
    UPROPERTY() bool bOpen = false;
    UPROPERTY() int64 CashAtClose = 0;       // the till after the last audited close
    UPROPERTY() int64 CashPosted = 0;        // cash entries since then
    UPROPERTY() bool bClosing = false;       // between BeginClose and EndClose: entries belong to the closed day
    UPROPERTY() int32 ClosingDay = 0;
    UPROPERTY() int64 LastGap = 0;           // the last close's unexplained cash (0 = the books agree)
    UPROPERTY() int32 GapDays = 0;           // closes with a gap so far
    UPROPERTY() int64 TotalGap = 0;
    // Goods other systems bought during the last close (branch orders add to FMarketState::Purchases): they post
    // their own lines, so the next BeginClose leaves them out of the family shop's purchases.
    UPROPERTY() int64 PurchasesInClose = 0;
    // C10 (Codex C9 R1: a branch's fit-out and hiring, managers' bonuses and severance landed in the family shop's
    // books): costs paid with the family shop's day close (FMarketState::OtherCosts) that belong to another store or
    // the head office. The next BeginClose books them there (Marketing) and leaves them out of the family shop.
    UPROPERTY() TArray<FMarketLedgerEntry> PendingStoreCosts;
};

namespace MarketLedger
{
    enum class EAccount : uint8
    {
        // Income statement: income
        Sales = 0,        // the family shop's till (and a branch's sales, Store = branch)
        OnlineSales,
        OtherIncome,
        // Income statement: costs
        CostOfGoods,      // book cost of the goods sold (no cash: the goods were paid when bought)
        Waste,            // expired goods (no cash)
        Shrinkage,        // missing / broken deliveries, till differences, goods sold off below their book value
        Wages,
        SocialSecurity,   // B3: the employer's share of the social security
        Severance,        // notice and seniority pay
        Hiring,           // hiring and engagement fees
        Rent,
        Utilities,        // electricity, water, bags, upkeep
        Marketing,        // flyers, campaigns, and the day's other shop costs (repairs, fines, set-ups)
        BankFees,         // card commission, POS rent, early repayment and mortgage fees
        OnlineCosts,      // couriers, packaging, platform commission, the site
        Logistics,        // freight, depots, trucks
        HeadOffice,
        Interest,
        Penalties,        // late fees and tax penalties
        Tax,              // VAT and income tax as declared (no cash: TaxPayment pays it)
        BadDebt,          // credit book money that is gone (M36: no credit book any more; kept for the order)
        BranchResult,     // a branch's net day when it is not booked line by line (the v0.1 second store)
        // B7: C's new systems (departments, brands, sourcing). Income:
        DepartmentSales,  // a department's till (Store = the store it is in)
        DepartmentClearance, // a closing department's stock sold off (cash in; its book value goes to DepartmentCostOfGoods)
        BrandShelfShare,  // a brand's monthly shelf-share payment
        BrandListing,     // a brand's listing money (paid once the product is on a shelf)
        BrandRebate,      // a brand's turnover rebate
        // Costs:
        DepartmentCostOfGoods, // book cost of department goods sold or cleared (no cash)
        DepartmentWaste,  // department goods spoiled, broken or written off (no cash)
        DepartmentMaster, // changing a department's master (notice, transfer, the new one's fee)
        SourcingFees,     // supply line fees and the shortfall of a minimum purchase
        // Balance sheet movements (cash, no profit)
        Purchases,        // goods bought for cash (they become stock)
        SupplierCredit,   // bought on terms (+) and paid later (-)
        LoanIn,
        LoanRepayment,
        Investment,       // deposits, fit-outs, depots, trucks, brands (-)
        Divestment,       // deposits back, goods or assets sold (+)
        CardTransfer,     // card receipts leave the till (-) and come from the bank the next day (+)
        CreditBook,       // sold on the credit book (-), collected (+) (M36: unused)
        TaxPayment,
        OwnerDraw,        // M37: dividends paid to us (the owner)
        InheritedDebt,    // paying the father's debt
        Capital,          // money put in (start help, a buyer's payment)
        DepartmentPurchases, // goods bought for a department (they become its stock)
        DepartmentFitOut, // opening or refitting a department (-), a closing one's fittings sold (+)
        ChainPurchase,    // buying a rival chain (-)
        StoreSale,        // its stores we cannot take, sold on (+)
        Unexplained,      // the audit's gap: money no system posted
        Count
    };

    constexpr int32 FamilyShop = -1;
    constexpr int32 HeadOfficeStore = -2;
    constexpr int32 AllStores = -1000;
    constexpr int32 KeepDays = 120;       // day and week statements; months and years come from the month totals

    // Turkish name for reports ("Sat\u0131lan mal\u0131n maliyeti").
    FString AccountName(EAccount Account);
    // Part of the income statement (true) or a balance sheet movement (false).
    bool IsIncomeStatement(EAccount Account);
    // Income statement groups: revenue (sales, online, other income, departments, brands) and the cost of the
    // goods (cost of goods, waste, shrinkage, department goods and waste); everything else is an expense.
    bool IsRevenue(EAccount Account);
    bool IsGoodsCost(EAccount Account);

    // Books an entry. Amount: + money in / income, - money out / cost; bCash: the till moved by exactly Amount.
    // Between BeginClose and EndClose the entry belongs to the closed day. Zero amounts are not booked; entries of
    // the same day, store, account and kind are added up into one line.
    void Post(FMarketState& State, EAccount Account, int64 Amount, bool bCash = true, int32 Store = FamilyShop);
    // C10: a cost paid with today's costs at the family shop's close (FMarketState::OtherCosts) that belongs to Store
    // (a branch index or HeadOfficeStore). Amount > 0.
    void AddStoreCost(FMarketState& State, int64 Amount, int32 Store);

    // Day close, right after FMarketState::CloseDay (MarketDirector, Ak\u0131\u015f B block at the start): books the family
    // shop's day from its counters (till sales, cost of goods, purchases, wages, the shop's costs, delivery losses).
    void BeginClose(FMarketState& State, const TArray<FMarketProduct>& Products);
    // Day close, after every system (Ak\u0131\u015f B block at the end): the audit; keeps KeepDays of entries.
    void EndClose(FMarketState& State);

    struct FStatement
    {
        int32 FromDay = 0;
        int32 ToDay = 0;
        TArray<int64> ByAccount;         // index = EAccount
        int64 Revenue = 0;               // sales + online + other income + departments + brands
        int64 GrossProfit = 0;           // revenue - cost of goods - waste - shrinkage (shop and departments)
        int64 Expenses = 0;              // every other cost (negative)
        int64 NetProfit = 0;
        int64 CashChange = 0;            // sum of the cash entries (profit and balance movements)
        int64 At(EAccount Account) const { return ByAccount.IsValidIndex(static_cast<int32>(Account)) ? ByAccount[static_cast<int32>(Account)] : 0; }
    };
    // Days FromDay..ToDay from the kept entries (Store = AllStores: the whole company).
    FStatement Statement(const FMarketState& State, int32 FromDay, int32 ToDay, int32 Store = AllStores);
    FStatement DayStatement(const FMarketState& State, int32 Day);
    // Game week N (days 7N-6 .. 7N).
    FStatement WeekStatement(const FMarketState& State, int32 Week);
    // A calendar month / year from the month totals (complete for the whole campaign, all stores).
    FStatement MonthStatement(const FMarketState& State, int32 Year, int32 Month);
    FStatement YearStatement(const FMarketState& State, int32 Year);

    struct FBalance
    {
        // Assets
        int64 Cash = 0;
        int64 Stock = 0;                 // family shop: shelf, depot, rear door, on the way (book cost)
        int64 BranchStock = 0;           // branches' goods (today's cost)
        int64 CardReceivable = 0;        // card money the bank still owes
        int64 Deposits = 0;              // rent deposits of open branches
        int64 DepartmentStock = 0;       // B7: departments' goods (book cost; C3 fills it, see B.md)
        // Liabilities
        int64 Payables = 0;              // wholesalers' bills
        int64 Loans = 0;
        int64 TaxDue = 0;
        int64 InheritedDebt = 0;
        int64 Assets() const { return Cash + Stock + BranchStock + DepartmentStock + CardReceivable + Deposits; }
        int64 Liabilities() const { return Payables + Loans + TaxDue + InheritedDebt; }
        int64 Equity() const { return Assets() - Liabilities(); }
    };
    FBalance Balance(const FMarketState& State, const TArray<FMarketProduct>& Products);

    // True when the last close's till matched the books.
    bool AuditOk(const FMarketState& State);
    // One line for the menu / day report: "Defter: nakit hareketleri kasayla tutuyor." or the gap.
    FString AuditText(const FMarketState& State);
    // Short income statement text of a period (menu tooltip, month report).
    FString StatementText(const FStatement& Statement);
}

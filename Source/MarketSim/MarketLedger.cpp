#include "MarketLedger.h"
#include "MarketEconomy.h"
#include "MarketBranches.h"
#include "MarketCalendar.h"
#include "MarketCountry.h"
#include "MarketDepartments.h"
#include "MarketBanking.h"
#include "MarketFinance.h"
#include "MarketPrices.h"
#include "MarketSuppliers.h"
#include "MarketStaff.h"

namespace MarketLedger
{
    constexpr int32 AccountCount = static_cast<int32>(EAccount::Count);

    FMarketLedgerMonth& MonthOf(FMarketLedger& L, int32 Day)
    {
        const MarketCalendar::FDate Date = MarketCalendar::DateOf(FMath::Max(1, Day));
        for (int32 I = L.Months.Num() - 1; I >= 0; --I)
            if (L.Months[I].Year == Date.Year && L.Months[I].Month == Date.Month) return L.Months[I];
        FMarketLedgerMonth& New = L.Months.AddDefaulted_GetRef();
        New.Year = Date.Year;
        New.Month = Date.Month;
        New.Totals.Init(0, AccountCount);
        return New;
    }

    void Summarize(FStatement& S)
    {
        const auto Get = [&S](EAccount A) { return S.At(A); };
        S.Revenue = 0;
        int64 GoodsCost = 0;
        S.Expenses = 0;
        for (int32 A = 0; A < AccountCount; ++A)
        {
            const EAccount Account = static_cast<EAccount>(A);
            if (!IsIncomeStatement(Account)) continue;
            if (IsRevenue(Account)) S.Revenue += Get(Account);
            else if (IsGoodsCost(Account)) GoodsCost += Get(Account);
            else S.Expenses += Get(Account);
        }
        S.GrossProfit = S.Revenue + GoodsCost;
        S.NetProfit = S.GrossProfit + S.Expenses;
    }

    FString LedgerMoney(int64 Kurus) { return MarketCountry::Money(Kurus); }
}

FString MarketLedger::AccountName(EAccount Account)
{
    switch (Account)
    {
    case EAccount::Sales: return TEXT("Sat\u0131\u015flar");
    case EAccount::OnlineSales: return TEXT("Online sat\u0131\u015flar");
    case EAccount::OtherIncome: return TEXT("Di\u011fer gelirler");
    case EAccount::CostOfGoods: return TEXT("Sat\u0131lan mal\u0131n maliyeti");
    case EAccount::Waste: return TEXT("Fire");
    case EAccount::Shrinkage: return TEXT("Eksik, k\u0131r\u0131k ve kasa fark\u0131");
    case EAccount::Wages: return TEXT("Maa\u015flar");
    case EAccount::SocialSecurity: return TEXT("Sigorta primi");
    case EAccount::Severance: return TEXT("Tazminat");
    case EAccount::Hiring: return TEXT("\u0130\u015fe al\u0131m");
    case EAccount::Rent: return TEXT("Kira");
    case EAccount::Utilities: return TEXT("Enerji ve d\u00fckk\u00e2n gideri");
    case EAccount::Marketing: return TEXT("Reklam ve di\u011fer d\u00fckk\u00e2n gideri");
    case EAccount::BankFees: return TEXT("Banka ve kart \u00fccretleri");
    case EAccount::OnlineCosts: return TEXT("Online giderleri");
    case EAccount::Logistics: return TEXT("Nakliye ve depo");
    case EAccount::HeadOffice: return TEXT("Merkez gideri");
    case EAccount::Interest: return TEXT("Faiz");
    case EAccount::Penalties: return TEXT("Gecikme ve cezalar");
    case EAccount::Tax: return TEXT("Vergi (KDV ve gelir)");
    case EAccount::BadDebt: return TEXT("Batan veresiye");
    case EAccount::BranchResult: return TEXT("Ba\u011fl\u0131 \u015firket sonucu");
    case EAccount::Purchases: return TEXT("Mal al\u0131m\u0131");
    case EAccount::SupplierCredit: return TEXT("Vadeli al\u0131m ve \u00f6demesi");
    case EAccount::LoanIn: return TEXT("Kredi giri\u015fi");
    case EAccount::LoanRepayment: return TEXT("Kredi anapara \u00f6demesi");
    case EAccount::Investment: return TEXT("Yat\u0131r\u0131m");
    case EAccount::Divestment: return TEXT("Yat\u0131r\u0131m d\u00f6n\u00fc\u015f\u00fc ve mal sat\u0131\u015f\u0131");
    case EAccount::CardTransfer: return TEXT("Kart tahsilat\u0131");
    case EAccount::CreditBook: return TEXT("Veresiye");
    case EAccount::TaxPayment: return TEXT("Vergi \u00f6demesi");
    case EAccount::OwnerDraw: return TEXT("K\u00e2r pay\u0131 (ortak)");
    case EAccount::InheritedDebt: return TEXT("Devral\u0131nan bor\u00e7");
    case EAccount::Capital: return TEXT("Sermaye");
    case EAccount::Unexplained: return TEXT("A\u00e7\u0131klanamayan fark");
    case EAccount::DepartmentSales: return TEXT("Reyon sat\u0131\u015flar\u0131");
    case EAccount::DepartmentClearance: return TEXT("Kapanan reyonun mal sat\u0131\u015f\u0131");
    case EAccount::BrandShelfShare: return TEXT("Marka raf pay\u0131 \u00f6demesi");
    case EAccount::BrandListing: return TEXT("Marka raf paras\u0131");
    case EAccount::BrandRebate: return TEXT("Marka ciro primi");
    case EAccount::DepartmentCostOfGoods: return TEXT("Reyon mal\u0131n\u0131n maliyeti");
    case EAccount::DepartmentWaste: return TEXT("Reyon firesi");
    case EAccount::DepartmentMaster: return TEXT("Reyon ustas\u0131 de\u011fi\u015fimi");
    case EAccount::SourcingFees: return TEXT("Tedarik \u00fccretleri");
    case EAccount::FxDifference: return TEXT("Kur fark\u0131");
    case EAccount::Withholding: return TEXT("K\u00e2r transferi stopaj\u0131");
    case EAccount::DepartmentPurchases: return TEXT("Reyon mal al\u0131m\u0131");
    case EAccount::DepartmentFitOut: return TEXT("Reyon tadilat\u0131");
    case EAccount::ChainPurchase: return TEXT("Zincir sat\u0131n alma");
    case EAccount::StoreSale: return TEXT("Ma\u011faza sat\u0131\u015f\u0131");
    default: return TEXT("?");
    }
}

bool MarketLedger::IsIncomeStatement(EAccount Account)
{
    return static_cast<uint8>(Account) < static_cast<uint8>(EAccount::Purchases);
}

bool MarketLedger::IsRevenue(EAccount Account)
{
    switch (Account)
    {
    case EAccount::Sales: case EAccount::OnlineSales: case EAccount::OtherIncome:
    case EAccount::DepartmentSales: case EAccount::DepartmentClearance:
    case EAccount::BrandShelfShare: case EAccount::BrandListing: case EAccount::BrandRebate:
        return true;
    default: return false;
    }
}

bool MarketLedger::IsGoodsCost(EAccount Account)
{
    switch (Account)
    {
    case EAccount::CostOfGoods: case EAccount::Waste: case EAccount::Shrinkage:
    case EAccount::DepartmentCostOfGoods: case EAccount::DepartmentWaste:
        return true;
    default: return false;
    }
}

void MarketLedger::Post(FMarketState& State, EAccount Account, int64 Amount, bool bCash, int32 Store)
{
    if (Amount == 0 || static_cast<int32>(Account) >= AccountCount) return;
    FMarketLedger& L = State.Ledger;
    const int32 Day = L.bClosing ? L.ClosingDay : State.Day;
    // One line per day, store, account and kind: card payments and credit sales come basket by basket.
    bool bMerged = false;
    for (int32 I = L.Entries.Num() - 1; I >= 0 && L.Entries[I].Day == Day; --I)
    {
        FMarketLedgerEntry& Same = L.Entries[I];
        if (Same.Store != Store || Same.Account != static_cast<uint8>(Account) || Same.bCash != bCash) continue;
        Same.Amount += Amount;
        bMerged = true;
        break;
    }
    if (!bMerged)
    {
        FMarketLedgerEntry Entry;
        Entry.Day = Day;
        Entry.Store = Store;
        Entry.Account = static_cast<uint8>(Account);
        Entry.Amount = Amount;
        Entry.bCash = bCash;
        L.Entries.Add(Entry);
    }
    FMarketLedgerMonth& Month = MonthOf(L, Day);
    if (Month.Totals.Num() < AccountCount) Month.Totals.SetNum(AccountCount);
    Month.Totals[static_cast<int32>(Account)] += Amount;
    if (bCash) L.CashPosted += Amount;
}

void MarketLedger::AddStoreCost(FMarketState& State, int64 Amount, int32 Store)
{
    if (Amount <= 0) return;
    State.OtherCosts += Amount;
    FMarketLedgerEntry Cost;
    Cost.Store = Store;
    Cost.Account = static_cast<uint8>(EAccount::Marketing);
    Cost.Amount = Amount;
    State.Ledger.PendingStoreCosts.Add(Cost);
}

void MarketLedger::BeginClose(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    FMarketLedger& L = State.Ledger;
    const int32 Closed = State.Day - 1;
    L.bClosing = true;
    L.ClosingDay = FMath::Max(1, Closed);
    if (Closed < 1) return;
    // FMarketState::CloseDay has just run: its counters hold the first store's closed day, before any other system
    // adds its own lines to them.
    Post(State, EAccount::Sales, State.LastRevenue);
    Post(State, EAccount::CostOfGoods, -State.LastCostOfGoods, false);
    Post(State, EAccount::Purchases, -FMath::Max<int64>(0, State.LastPurchases - L.PurchasesInClose));
    L.PurchasesInClose = 0;
    // FMarketState::CloseDay paid: the shop's running costs (list level, season), the payroll, the day's marketing
    // and other costs; the v0.1 second store's aggregate came in.
    const MarketCalendar::ESeason Season = MarketCalendar::SeasonOf(MarketCalendar::DateOf(Closed).Month);
    const double Seasonal = Season == MarketCalendar::ESeason::Summer ? 1.15 : Season == MarketCalendar::ESeason::Winter ? 1.10 : 1.0;
    const int64 Utilities = FMath::RoundToInt64(2200.0 * MarketPrices::ListLevel(Closed) * Seasonal * State.FirstStoreRunning()); // G-110: a closed first store
    const int64 Payroll = State.DailyPayroll();
    // C11: the HR manager and the accountant serve the whole company: their pay is the head office's, so the family
    // shop's books show only the shop's own people. The till paid the whole payroll; only the books split it.
    int64 HeadOfficePay = 0;
    for (const FMarketEmployee& E : State.Staff)
    {
        const MarketStaff::ERole Role = MarketStaff::RoleOf(E);
        if (Role == MarketStaff::ERole::HrManager || Role == MarketStaff::ERole::Accountant) HeadOfficePay += FMath::Max<int64>(0, E.DailyWage);
    }
    HeadOfficePay = FMath::Clamp<int64>(HeadOfficePay, 0, FMath::Max<int64>(0, Payroll));
    Post(State, EAccount::Utilities, -Utilities);
    Post(State, EAccount::Wages, -(Payroll - HeadOfficePay));
    Post(State, EAccount::Wages, -HeadOfficePay, true, HeadOfficeStore);
    // C10: what other stores and the head office left in today's costs goes to them.
    int64 Elsewhere = 0;
    for (const FMarketLedgerEntry& Cost : L.PendingStoreCosts) { Post(State, EAccount::Marketing, -Cost.Amount, true, Cost.Store); Elsewhere += Cost.Amount; }
    L.PendingStoreCosts.Reset();
    Post(State, EAccount::Marketing, -(State.LastOperatingCost - Utilities - Payroll - Elsewhere));
    // Paid-for units that never arrived whole (#37): FMarketState::CloseDay puts them into PendingLoss for the next
    // report; the books take the loss today.
    Post(State, EAccount::Shrinkage, -State.PendingLoss, false);
}

void MarketLedger::EndClose(FMarketState& State)
{
    FMarketLedger& L = State.Ledger;
    L.LastGap = 0;
    if (L.bOpen)
    {
        const int64 Gap = State.Cash - (L.CashAtClose + L.CashPosted);
        if (Gap != 0)
        {
            L.bClosing = true;
            if (L.ClosingDay < 1) L.ClosingDay = FMath::Max(1, State.Day - 1);
            Post(State, EAccount::Unexplained, Gap);
            L.LastGap = Gap;
            ++L.GapDays;
            L.TotalGap += Gap;
        }
    }
    L.bOpen = true;
    L.PurchasesInClose = State.Purchases;
    L.CashAtClose = State.Cash;
    L.CashPosted = 0;
    L.bClosing = false;
    // Keep KeepDays of entries (the months keep the totals).
    const int32 Oldest = State.Day - KeepDays;
    int32 Drop = 0;
    while (Drop < L.Entries.Num() && L.Entries[Drop].Day < Oldest) ++Drop;
    if (Drop > 0) L.Entries.RemoveAt(0, Drop);
}

MarketLedger::FStatement MarketLedger::Statement(const FMarketState& State, int32 FromDay, int32 ToDay, int32 Store)
{
    FStatement S;
    S.FromDay = FromDay;
    S.ToDay = ToDay;
    S.ByAccount.Init(0, AccountCount);
    for (const FMarketLedgerEntry& E : State.Ledger.Entries)
    {
        if (E.Day < FromDay || E.Day > ToDay || E.Account >= AccountCount) continue;
        if (Store != AllStores && E.Store != Store) continue;
        S.ByAccount[E.Account] += E.Amount;
        if (E.bCash) S.CashChange += E.Amount;
    }
    Summarize(S);
    return S;
}

MarketLedger::FStatement MarketLedger::DayStatement(const FMarketState& State, int32 Day)
{
    return Statement(State, Day, Day);
}

MarketLedger::FStatement MarketLedger::WeekStatement(const FMarketState& State, int32 Week)
{
    return Statement(State, Week * 7 - 6, Week * 7);
}

MarketLedger::FStatement MarketLedger::MonthStatement(const FMarketState& State, int32 Year, int32 Month)
{
    FStatement S;
    S.FromDay = MarketCalendar::GameDayOf(Year, Month, 1);
    S.ToDay = MarketCalendar::GameDayOf(Year, Month, MarketCalendar::DaysInMonth(Year, Month));
    S.ByAccount.Init(0, AccountCount);
    for (const FMarketLedgerMonth& M : State.Ledger.Months)
    {
        if (M.Year != Year || M.Month != Month) continue;
        for (int32 A = 0; A < AccountCount && A < M.Totals.Num(); ++A) S.ByAccount[A] += M.Totals[A];
    }
    Summarize(S);
    // Cash of the month: from the kept entries when they reach back that far.
    S.CashChange = Statement(State, S.FromDay, S.ToDay).CashChange;
    return S;
}

MarketLedger::FStatement MarketLedger::YearStatement(const FMarketState& State, int32 Year)
{
    FStatement S;
    S.FromDay = MarketCalendar::GameDayOf(Year, 1, 1);
    S.ToDay = MarketCalendar::GameDayOf(Year, 12, 31);
    S.ByAccount.Init(0, AccountCount);
    for (const FMarketLedgerMonth& M : State.Ledger.Months)
        if (M.Year == Year)
            for (int32 A = 0; A < AccountCount && A < M.Totals.Num(); ++A) S.ByAccount[A] += M.Totals[A];
    Summarize(S);
    S.CashChange = Statement(State, S.FromDay, S.ToDay).CashChange;
    return S;
}

MarketLedger::FBalance MarketLedger::Balance(const FMarketState& State, const TArray<FMarketProduct>& Products)
{
    FBalance B;
    B.Cash = State.Cash;
    for (int32 I = 0; I < State.Stock.Num() && I < Products.Num(); ++I)
    {
        const FMarketStock& Row = State.Stock[I];
        B.Stock += static_cast<int64>(FMath::Max(0, Row.Shelf + Row.Warehouse + Row.Dock + Row.Incoming)) * State.UnitCost(I, Products);
    }
    for (const FMarketBranch& Branch : State.Branches)
    {
        if (Branch.Stage == static_cast<uint8>(MarketBranches::EStage::Closed)) continue;
        // E4: a store abroad holds its deposit and goods in its own money, worth today's rate.
        B.Deposits += MarketBranches::DepositInHome(State, Branch, State.Day);
        const MarketBranches::FMoney Money = MarketBranches::MoneyOf(State, MarketBranches::CountryOf(State, Branch), State.Day);
        for (const FMarketStock& Item : Branch.Items)
        {
            const FMarketProduct* P = Products.FindByPredicate([&Item](const FMarketProduct& X) { return X.Id == Item.Id; });
            if (P) B.BranchStock += MarketBranches::InHome(Money, static_cast<int64>(FMath::Max(0, Item.Shelf + Item.Incoming)) * P->Cost);
        }
    }
    B.DepartmentStock = MarketDepartments::StockValue(State); // C3 (M26)
    B.Building = MarketFinance::BuildingValue(State); // M69
    B.CardReceivable = State.Payments.CardToday + State.Payments.CardTomorrow;
    B.Payables = MarketSuppliers::OpenBills(State);
    B.Loans = MarketFinance::Debt(State) + MarketBanking::Debt(State); // M28: company loans and the credit line
    B.TaxDue = State.Books.TaxDue;
    B.InheritedDebt = State.InheritedDebt;
    return B;
}

bool MarketLedger::AuditOk(const FMarketState& State)
{
    return State.Ledger.LastGap == 0;
}

FString MarketLedger::AuditText(const FMarketState& State)
{
    if (!State.Ledger.bOpen) return TEXT("Defter ilk g\u00fcn kapan\u0131\u015f\u0131nda a\u00e7\u0131l\u0131r.");
    if (State.Ledger.LastGap == 0) return TEXT("Defter kasayla tutuyor.");
    return FString::Printf(TEXT("Defterde a\u00e7\u0131klanamayan kasa fark\u0131: %s."), *LedgerMoney(State.Ledger.LastGap));
}

FString MarketLedger::StatementText(const FStatement& S)
{
    return FString::Printf(TEXT("Ciro %s \u00b7 br\u00fct k\u00e2r %s \u00b7 giderler %s \u00b7 net %s"),
        *LedgerMoney(S.Revenue), *LedgerMoney(S.GrossProfit), *LedgerMoney(-S.Expenses), *LedgerMoney(S.NetProfit));
}

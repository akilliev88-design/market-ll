#include "MarketLedger.h"
#include "MarketEconomy.h"
#include "MarketDirector.h"
#include "MarketSimulation.h"
#include "MarketSuppliers.h"
#include "MarketFinance.h"
#include "MarketStaff.h"
#include "MarketCredit.h"
#include "MarketCalendar.h"
#include "MarketBranches.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// Ak\u0131\u015f B2 (#37, #42): the company's books.
namespace MarketLedgerTest
{
    FMarketProduct Make(const TCHAR* Id, const TCHAR* Category, int64 Cost, int64 Price)
    {
        FMarketProduct P;
        P.Id = Id; P.RealName = Id; P.Category = Category; P.Cost = Cost; P.BasePrice = Price; P.CaseUnits = 12; P.bActive = true;
        return P;
    }

    TArray<FMarketProduct> Catalog()
    {
        return {
            Make(TEXT("sut"), TEXT("s\u00fct"), 170, 250),
            Make(TEXT("ayran"), TEXT("s\u00fct"), 60, 100),
            Make(TEXT("makarna"), TEXT("makarna-bakliyat"), 210, 325),
            Make(TEXT("cay"), TEXT("\u00e7ay-kahve"), 480, 675),
            Make(TEXT("cola"), TEXT("i\u00e7ecek"), 180, 275),
            Make(TEXT("deterjan"), TEXT("temizlik"), 1400, 1990),
        };
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketLedgerPostTest, "MirasMarket.Ledger.PostAndStatements", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketLedgerPostTest::RunTest(const FString& Parameters)
{
    using namespace MarketLedger;
    FMarketState S; S.Day = 3;
    Post(S, EAccount::Sales, 10000);
    Post(S, EAccount::Sales, 5000);               // same day, account and kind: one line
    Post(S, EAccount::CostOfGoods, -9000, false);
    Post(S, EAccount::Wages, -2000);
    Post(S, EAccount::Purchases, -7000);
    Post(S, EAccount::Sales, 0);                  // nothing booked
    Post(S, EAccount::Sales, 700, true, 0);       // a branch
    TestEqual(TEXT("Lines merged"), S.Ledger.Entries.Num(), 5);
    TestEqual(TEXT("Cash entries counted for the audit"), S.Ledger.CashPosted, int64(10000 + 5000 - 2000 - 7000 + 700));

    const FStatement Day = DayStatement(S, 3);
    TestEqual(TEXT("Revenue"), Day.Revenue, int64(15700));
    TestEqual(TEXT("Gross profit"), Day.GrossProfit, int64(15700 - 9000));
    TestEqual(TEXT("Expenses"), Day.Expenses, int64(-2000));
    TestEqual(TEXT("Net profit"), Day.NetProfit, int64(15700 - 9000 - 2000));
    TestEqual(TEXT("Purchases are no cost"), Day.NetProfit, Day.GrossProfit + Day.Expenses);
    TestEqual(TEXT("Cash change counts purchases, not the cost of goods"), Day.CashChange, int64(15700 - 2000 - 7000));
    TestEqual(TEXT("One store"), Statement(S, 3, 3, FamilyShop).Revenue, int64(15000));
    TestEqual(TEXT("A branch"), Statement(S, 3, 3, 0).Revenue, int64(700));
    TestEqual(TEXT("Another day is empty"), DayStatement(S, 4).Revenue, int64(0));

    // During the close, entries belong to the closed day.
    S.Day = 8; S.Ledger.bClosing = true; S.Ledger.ClosingDay = 7;
    Post(S, EAccount::Interest, -300);
    S.Ledger.bClosing = false;
    TestEqual(TEXT("Closed day"), DayStatement(S, 7).At(EAccount::Interest), int64(-300));
    TestEqual(TEXT("Week 1 (days 1-7)"), WeekStatement(S, 1).NetProfit, int64(15700 - 9000 - 2000 - 300));

    // Months and years keep totals after the entries are gone.
    const MarketCalendar::FDate Date = MarketCalendar::DateOf(3);
    TestEqual(TEXT("Month"), MonthStatement(S, Date.Year, Date.Month).NetProfit, int64(15700 - 9000 - 2000 - 300));
    S.Ledger.Entries.Reset();
    TestEqual(TEXT("Month totals survive"), MonthStatement(S, Date.Year, Date.Month).Revenue, int64(15700));
    TestEqual(TEXT("Year"), YearStatement(S, Date.Year).Revenue, int64(15700));
    TestEqual(TEXT("Other year"), YearStatement(S, Date.Year + 1).Revenue, int64(0));
    TestFalse(TEXT("Every account has a name"), AccountName(EAccount::Unexplained).IsEmpty() || AccountName(EAccount::Sales) == AccountName(EAccount::OnlineSales));
    TestTrue(TEXT("Balance movements are not in the income statement"), IsIncomeStatement(EAccount::Tax) && !IsIncomeStatement(EAccount::Purchases) && !IsIncomeStatement(EAccount::LoanIn));
    TestFalse(TEXT("Statement text"), StatementText(Day).IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketLedgerAuditTest, "MirasMarket.Ledger.CashAudit", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketLedgerAuditTest::RunTest(const FString& Parameters)
{
    // The family shop played without walking people (MarketSimulation::PlayDay: till, card, credit book, online,
    // orders on terms, staff, tax, bank, family money): every cash movement is in the books (C3 wired the
    // wholesaler's terms), so the gap is zero.
    using namespace MarketLedgerTest;
    const TArray<FMarketProduct> Base = Catalog();
    TArray<FMarketProduct> Products = Base;
    FMarketState S; S.Initialize(Base); S.RivalSeed = 17; S.Cash = 400000;
    S.ApplyShelfCapacities({ 24, 24, 24, 24, 24, 24 });
    MarketDirector::ApplyPrices(S, Base, Products);
    FString Message;
    TestTrue(TEXT("POS"), MarketDirector::Command(S, Products, TEXT("Card"), 1, Message));
    TestTrue(TEXT("Credit book"), MarketDirector::Command(S, Products, TEXT("CreditLimit"), 2, Message));
    TestTrue(TEXT("Phone orders"), MarketDirector::Command(S, Products, TEXT("OnlineChannel"), 1, Message));
    for (int32 I = 0; I < 40; ++I) { FMarketLoyalty L; L.CustomerId = I; L.Visits = 6; L.Satisfaction = 80.f; S.Loyalty.Add(L); }
    bool bBooksAgree = true;
    int32 Days = 0;
    for (int32 D = 1; D <= 90; ++D)
    {
        if (D == 10) TestTrue(TEXT("A loan"), MarketFinance::TakeLoan(S, 0, Message));
        if (D == 12) TestTrue(TEXT("An accountant"), MarketStaff::HireAccountant(S, Message));
        if (D == 30) MarketStaff::PayTax(S);
        MarketSimulation::PlayDay(S, Base, Products);
        if (D > 1) { ++Days; bBooksAgree &= S.Ledger.LastGap == 0; }
    }
    TestTrue(TEXT("Books opened"), S.Ledger.bOpen && Days == 89);
    TestTrue(TEXT("Till change = cash entries"), bBooksAgree);
    const MarketLedger::FStatement All = MarketLedger::Statement(S, 1, 90);
    TestTrue(TEXT("Sales booked"), All.At(MarketLedger::EAccount::Sales) > 0 && All.At(MarketLedger::EAccount::Purchases) < 0);
    TestTrue(TEXT("Card money moved"), All.At(MarketLedger::EAccount::CardTransfer) != 0 && All.At(MarketLedger::EAccount::BankFees) < 0);
    TestTrue(TEXT("Bank"), All.At(MarketLedger::EAccount::LoanIn) > 0);
    TestTrue(TEXT("Family money"), All.At(MarketLedger::EAccount::OwnerDraw) < 0);
    TestTrue(TEXT("Tax declared"), All.At(MarketLedger::EAccount::Tax) < 0);
    TestTrue(TEXT("Wages"), All.At(MarketLedger::EAccount::Wages) < 0 && All.At(MarketLedger::EAccount::Hiring) < 0);

    // A movement no system posted shows up as a gap, is booked as unexplained and the next day is clean again.
    FMarketState Leak = S;
    Leak.Cash += 12345;
    Leak.DayNews.Reset(); Leak.CloseDay(); MarketDirector::CloseDay(Leak, Products);
    TestEqual(TEXT("Gap found"), Leak.Ledger.LastGap, int64(12345));
    TestFalse(TEXT("Audit reports it"), MarketLedger::AuditOk(Leak) || MarketLedger::AuditText(Leak).Contains(TEXT("tutuyor")));
    TestTrue(TEXT("Booked as unexplained"), MarketLedger::DayStatement(Leak, Leak.Day - 1).At(MarketLedger::EAccount::Unexplained) == Leak.Ledger.LastGap);
    Leak.DayNews.Reset(); Leak.CloseDay(); MarketDirector::CloseDay(Leak, Products);
    TestEqual(TEXT("Next day clean again"), Leak.Ledger.LastGap, int64(0));

    // Kept days: entries older than KeepDays drop, month totals stay.
    TestTrue(TEXT("Entries kept"), S.Ledger.Entries.Num() > 0 && S.Ledger.Entries[0].Day >= S.Day - MarketLedger::KeepDays);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketLedgerBalanceTest, "MirasMarket.Ledger.BalanceSheet", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketLedgerBalanceTest::RunTest(const FString& Parameters)
{
    using namespace MarketLedgerTest;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S; S.Initialize(Products); S.Cash = 100000; S.InheritedDebt = 30000;
    for (FMarketStock& Row : S.Stock) { Row.Shelf = 0; Row.Warehouse = 0; }
    S.Stock[0].Shelf = 10; S.Stock[0].Warehouse = 5; S.Stock[0].Incoming = 12;
    S.Payments.CardToday = 700; S.Payments.CardTomorrow = 300;
    FMarketCreditAccount Credit; Credit.CustomerId = 1; Credit.Balance = 900; S.Credit.Add(Credit);
    FMarketPayable Bill; Bill.Amount = 4000; Bill.DueDay = 9; S.Payables.Add(Bill);
    FMarketLoan Loan; Loan.Principal = 50000; Loan.Remaining = 45000; S.Loans.Add(Loan);
    S.Books.TaxDue = 1200;
    FMarketBranch Branch; Branch.Stage = static_cast<uint8>(MarketBranches::EStage::Open); Branch.Rent = 60000;
    FMarketBranchItem Item; Item.ProductId = TEXT("cola"); Item.Units = 20; Branch.Items.Add(Item);
    S.Branches.Add(Branch);
    FMarketBranch Gone = Branch; Gone.Stage = static_cast<uint8>(MarketBranches::EStage::Closed); S.Branches.Add(Gone);

    const MarketLedger::FBalance B = MarketLedger::Balance(S, Products);
    TestEqual(TEXT("Cash"), B.Cash, int64(100000));
    TestEqual(TEXT("Stock at cost (on the way too)"), B.Stock, int64(27 * 170));
    TestEqual(TEXT("Branch goods"), B.BranchStock, int64(20 * 180));
    TestEqual(TEXT("Deposit of the open branch only"), B.Deposits, int64(120000));
    TestEqual(TEXT("Card money owed by the bank"), B.CardReceivable, int64(1000));
    TestEqual(TEXT("Credit book"), B.CreditReceivable, int64(900));
    TestEqual(TEXT("Wholesaler"), B.Payables, int64(4000));
    TestEqual(TEXT("Bank"), B.Loans, int64(45000));
    TestEqual(TEXT("Tax"), B.TaxDue, int64(1200));
    TestEqual(TEXT("Father's debt"), B.InheritedDebt, int64(30000));
    TestEqual(TEXT("Equity = assets - liabilities"), B.Equity(), B.Assets() - B.Liabilities());
    TestEqual(TEXT("Assets"), B.Assets(), int64(100000 + 27 * 170 + 20 * 180 + 120000 + 1000 + 900));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketLedgerNewSystemsTest, "MirasMarket.Ledger.DepartmentsBrandsChains", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketLedgerNewSystemsTest::RunTest(const FString& Parameters)
{
    using namespace MarketLedger;
    // B7: the accounts C's systems post to (B.md lists the calls).
    FMarketState S; S.Day = 12;
    int64 Till = 0;
    auto Move = [&S, &Till](EAccount Account, int64 Amount, int32 Store) { Till += Amount; Post(S, Account, Amount, true, Store); };
    Move(EAccount::DepartmentFitOut, -500000, 0);       // a department opens in branch 0
    Move(EAccount::DepartmentPurchases, -120000, 0);    // its goods
    Move(EAccount::DepartmentSales, 30000, 0);          // its till
    Post(S, EAccount::DepartmentCostOfGoods, -18000, false, 0);
    Post(S, EAccount::DepartmentWaste, -1500, false, 0);
    Move(EAccount::DepartmentMaster, -9000, 0);         // a new master
    Move(EAccount::BrandShelfShare, 40000, HeadOfficeStore);
    Move(EAccount::BrandListing, 15000, HeadOfficeStore);
    Move(EAccount::BrandRebate, 6000, HeadOfficeStore);
    Move(EAccount::SourcingFees, -2500, HeadOfficeStore);
    Move(EAccount::ChainPurchase, -9000000, HeadOfficeStore);
    Move(EAccount::StoreSale, 1200000, HeadOfficeStore);
    Move(EAccount::DepartmentClearance, 50000, 0);      // the department closes: its stock sold off
    Post(S, EAccount::DepartmentCostOfGoods, -80000, false, 0);
    Move(EAccount::DepartmentFitOut, 20000, 0);         // its fittings

    TestEqual(TEXT("The audit sees every cash move"), S.Ledger.CashPosted, Till);
    const FStatement Day = DayStatement(S, 12);
    TestEqual(TEXT("Revenue: department till, clearance and brand money"), Day.Revenue, int64(30000 + 50000 + 40000 + 15000 + 6000));
    TestEqual(TEXT("Gross: department goods and waste"), Day.GrossProfit, Day.Revenue - 18000 - 1500 - 80000);
    TestEqual(TEXT("Expenses: master and sourcing"), Day.Expenses, int64(-9000 - 2500));
    TestEqual(TEXT("Investments and purchases are no profit"), Day.NetProfit, Day.Revenue - 18000 - 1500 - 80000 - 9000 - 2500);
    TestEqual(TEXT("Cash change"), Day.CashChange, Till);
    TestEqual(TEXT("A branch's department"), Statement(S, 12, 12, 0).Revenue, int64(80000));
    TestTrue(TEXT("Balance movements"), !IsIncomeStatement(EAccount::DepartmentPurchases) && !IsIncomeStatement(EAccount::DepartmentFitOut)
        && !IsIncomeStatement(EAccount::ChainPurchase) && !IsIncomeStatement(EAccount::StoreSale));
    TestTrue(TEXT("Income statement"), IsIncomeStatement(EAccount::DepartmentSales) && IsIncomeStatement(EAccount::BrandRebate) && IsIncomeStatement(EAccount::SourcingFees));
    TSet<FString> Names;
    bool bNamed = true;
    for (int32 A = 0; A < static_cast<int32>(EAccount::Count); ++A)
    {
        const FString Name = AccountName(static_cast<EAccount>(A));
        bNamed &= !Name.IsEmpty() && Name != TEXT("?") && !Names.Contains(Name);
        Names.Add(Name);
    }
    TestTrue(TEXT("Every account has its own name"), bNamed);
    return true;
}

#endif

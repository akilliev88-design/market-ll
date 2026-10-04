#include "MarketAutoPlayRescue.h"
#include "MarketFinance.h"
#include "MarketBanking.h"
#include "MarketBranches.h"
#include "MarketLedger.h"
#include "MarketSuppliers.h"
namespace MarketAutoPlayRescue
{
    bool Blocked(const FMarketState& State) {return State.Day<State.RescueUntil;}
    int64 LoanMovement(const FMarketState& State,int32 Day)
    {
        int64 Total=0;for(const auto& Entry:State.Ledger.Entries)if(Entry.Day==Day)
        {
            const auto Account=static_cast<MarketLedger::EAccount>(Entry.Account);
            if(Account==MarketLedger::EAccount::LoanIn || Account==MarketLedger::EAccount::LoanRepayment)Total+=Entry.Amount;
            else if(Account==MarketLedger::EAccount::Penalties && !Entry.bCash)Total-=Entry.Amount;
        }return Total;
    }
    int64 SupplierMovement(const FMarketState& State,int32 Day)
    {int64 Total=0;for(const auto& Entry:State.Ledger.Entries)if(Entry.Day==Day && Entry.Account==static_cast<uint8>(MarketLedger::EAccount::SupplierCredit))Total+=Entry.Amount;return Total;}
    void BeginDay(const FMarketState& State,FStats& Stats)
    {
        if(Stats.LastBeginDay==State.Day)return;Stats.LastBeginDay=State.Day;
        Stats.BeforeDebt=MarketFinance::Debt(State)+MarketBanking::Debt(State);Stats.BeforeRescues=State.Rescues;
        Stats.BeforeTaxPenalties=State.Books.TotalPenalties;Stats.BeforeBills=MarketSuppliers::OpenBills(State);Stats.BeforeSupplierMovement=SupplierMovement(State,State.Day);
        Stats.BeforeMovement=LoanMovement(State,State.Day);if(Blocked(State))++Stats.BlockedDays;
    }
    void Observe(const FMarketState& State,int32 Year,FStats& Stats)
    {
        const int32 Day=State.Day-1;if(Day<=Stats.LastDay)return;Stats.LastDay=Day;Stats.Revenue+=State.LastRevenue;
        if(State.Rescues>Stats.BeforeRescues)
        {
            for(auto& Previous:Stats.Plans)if(!Previous.Paid && !Previous.Replaced)Previous.Replaced=Day;
            FPlan Row;Row.Day=Day;Row.Until=State.RescueUntil;Row.Stores=MarketBranches::OpenCount(State)+1;
            if(!State.Loans.IsEmpty()){Row.Principal=State.Loans[0].Principal;Row.Remaining=State.Loans[0].Remaining;Row.Rate=State.Loans[0].MonthlyRate;}
            const int64 SupplierPenalties=MarketSuppliers::OpenBills(State)-Stats.BeforeBills-(SupplierMovement(State,Day)-Stats.BeforeSupplierMovement);
            const int64 TaxPenalties=State.Books.TotalPenalties-Stats.BeforeTaxPenalties;
            const int64 Movement=LoanMovement(State,Day)-Stats.BeforeMovement-SupplierPenalties-TaxPenalties;
            // Loan balance conservation. Write-off is not cash income: all cash borrowing/repayment and
            // accrued loan penalties are included; tax/supplier penalties are excluded from loan movement.
            Row.WrittenOff=Stats.BeforeDebt+Movement-MarketFinance::Debt(State)-MarketBanking::Debt(State);
            Stats.Plans.Add(Row);
        }
        for(auto& Plan:Stats.Plans)
        {
            if(Plan.Replaced)continue;
            if(!Plan.FirstBranch)for(const auto& Branch:State.Branches)
                if(Branch.OpenedDay>Plan.Day && Branch.OpenedDay<=Day && (!Plan.FirstBranch || Branch.OpenedDay<Plan.FirstBranch))Plan.FirstBranch=Branch.OpenedDay;
            if(Plan.Paid)continue;
            const auto* Loan=State.Loans.FindByPredicate([&](const FMarketLoan& Item){return Item.Principal==Plan.Principal && Item.MonthlyRate==Plan.Rate;});
            Plan.Remaining=Loan?Loan->Remaining:0;if(!Loan)Plan.Paid=Day;
        }
        if(Year>Stats.Years.Num())
        {FYear Row;Row.Year=Year;Row.Day=Day;Row.Debt=State.InheritedDebt+MarketFinance::Debt(State)+MarketBanking::Debt(State)+MarketSuppliers::OpenBills(State)+State.Books.TaxDue;Row.FirstStoreRevenue=Stats.Revenue;Stats.Years.Add(Row);Stats.Revenue=0;}
    }
    FString Report(const FStats& Stats)
    {
        int64 Written=0;int32 Paid=0,Replaced=0;for(const auto& Plan:Stats.Plans){Written+=Plan.WrittenOff;Paid+=Plan.Paid>0;Replaced+=Plan.Replaced>0;}
        return FString::Printf(TEXT("\n#### C7: kurtarmadan sonra\n\nPlan %d, tamamen odenen %d, yeni planla degisen %d; silinen kredi borcu %lld kurus. Yeni kredi/sube denenmeyen plan gunu %d. kurtarma.csv gun, ilk yeni sube, kalan plan ve borc silme; kurtarma_yillar.csv toplam borc ve ilk magazanin tam oyun yili cirosu. Silinen borc kasaya gelir degildir; plan degismesi odeme sayilmaz.\n"),Stats.Plans.Num(),Paid,Replaced,Written,Stats.BlockedDays);
    }
    FString PlansCsv(const FStats& Stats,const FString& Style,int32 Seed)
    {
        FString Text;for(const auto& Row:Stats.Plans)Text+=FString::Printf(TEXT("%s,%d,%d,%d,%d,%d,%d,%d,%lld,%lld,%lld\n"),*Style,Seed,Row.Day,Row.Until,Row.Stores,Row.FirstBranch,Row.Paid,Row.Replaced,Row.Principal,Row.Remaining,Row.WrittenOff);return Text;
    }
    FString YearsCsv(const FStats& Stats,const FString& Style,int32 Seed)
    {FString Text;for(const auto& Row:Stats.Years)Text+=FString::Printf(TEXT("%s,%d,%d,%d,%lld,%lld\n"),*Style,Seed,Row.Year,Row.Day,Row.Debt,Row.FirstStoreRevenue);return Text;}
}

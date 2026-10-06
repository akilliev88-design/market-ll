#include "MarketAutoPlayDiagnosis.h"
#include "MarketAutoPlay.h"
#include "MarketOwner.h"
#include "MarketCompany.h"
#include "MarketAutoPlayFinance.h"
#include "MarketDirector.h"
#include "MarketCalendar.h"
#include "MarketPrices.h"
#include "MarketManagers.h"
#include "MarketStaff.h"
#include "MarketSuppliers.h"
#include "MarketLedger.h"
#include "MarketEras.h"
namespace MarketAutoPlayDiagnosis
{
    FString Safe(FString Value) { Value.ReplaceInline(TEXT(","), TEXT(";")); Value.ReplaceInline(TEXT("\n"), TEXT(" ")); return Value; }
    void Policy(FMarketState& State, const TArray<FMarketProduct>& Products, int32 Style, FStats& Stats)
    {
        const auto Date = MarketCalendar::DateOf(State.Day);
        const int32 Stores = MarketCompany::TotalStores(State);
        const int32 Step = Style == 2 && Stores >= 30 ? 4 : Style == 1 && Stores >= 10 ? 3 : 1;
        FString Message;
        if (State.Day >= State.RescueUntil && State.Owner.SalaryX10 != MarketOwner::SalarySteps[Step] &&
            (State.Owner.SalaryX10 > MarketOwner::SalarySteps[Step] || State.Cash >= 3*MarketAutoPlayFinance::NetworkReserve(State)))
        {
            if (MarketDirector::Command(State, Products, TEXT("OwnerSalary"), Step, Message))
                Stats.Events.Add(FString::Printf(TEXT("%d,salary,%d,%s\n"), State.Day, MarketOwner::SalarySteps[Step], *Safe(Message)));
        }
        if (Date.Year > MarketCalendar::StartYear && Date.Month == 1 && Stats.LastDividendYear != Date.Year)
        {
            Stats.LastDividendYear = Date.Year;
            const int64 Before = State.Owner.TotalDividends;
            // A comfortable till keeps three months of the whole network after the distribution.
            if (State.Cash - MarketOwner::DividendRoom(State) / 4 >= 3 * MarketAutoPlayFinance::NetworkReserve(State) &&
                MarketDirector::Command(State, Products, TEXT("Dividend"), 0, Message))
                Stats.Events.Add(FString::Printf(TEXT("%d,dividend,25,%s\n"), State.Day, *Safe(Message)));
            Stats.PendingDividend += State.Owner.TotalDividends - Before;
        }
    }
    void BeginDay(const FMarketState& State, const TArray<FMarketProduct>& Products, double PriceFactor, FStats& Stats)
    {
        Stats.WarningBefore=State.LowCashWarnDay;Stats.LifelineBefore=0;
        for(const auto& Account:State.SupplierAccounts)Stats.LifelineBefore=FMath::Max(Stats.LifelineBefore,Account.LifelineDay);
        Stats.SalaryBefore = State.Owner.TotalSalary; Stats.DividendBefore = State.Owner.TotalDividends;
        Stats.Shelf = Stats.List = Stats.Purchase = Stats.Book = Stats.Target = 0;
        int32 Count = 0;
        const auto Date = MarketCalendar::DateOf(State.Day);
        for (int32 I = 0; I < Products.Num(); ++I)
        {
            const auto& Item = State.Stock[I]; const auto& Product = Products[I];
            if (Item.Capacity <= 0) continue;
            ++Count; Stats.Shelf += Item.Price; Stats.List += Product.BasePrice; Stats.Purchase += Product.Cost;
            Stats.Book += Item.AvgCost > 0 ? Item.AvgCost : Product.Cost;
            const auto& Profiles=MarketAutoPlay::Profiles();
            int32 Style=1;for(int32 P=0;P<Profiles.Num();++P)if(FMath::IsNearlyEqual(Profiles[P].PriceFactor,PriceFactor))Style=P;
            const int64 Target = MarketAutoPlay::PriceTarget(State,Products,I,Profiles[Style]);
            Stats.Target += Target;
            if (Date.Day == 1 || State.Day == 1)
                Stats.Products.Add(FString::Printf(TEXT("%d,%s,%d,%lld,%lld,%lld,%lld,%lld,%d,%d\n"), State.Day, *Safe(Product.Id), Date.Year, Item.Price, Product.BasePrice, Product.Cost, Item.AvgCost, Target, Item.Shelf, Item.Warehouse));
        }
        if (Count > 0) { Stats.Shelf /= Count; Stats.List /= Count; Stats.Purchase /= Count; Stats.Book /= Count; Stats.Target /= Count; }
        Stats.StaffCount = State.Staff.Num(); Stats.Cashiers = Stats.Stockers = Stats.Hr = Stats.Accountant = 0;
        FString Roster;
        for (const auto& Person : State.Staff)
        {
            const auto Role = MarketStaff::RoleOf(Person);
            Stats.Cashiers += Role == MarketStaff::ERole::Cashier; Stats.Stockers += Role == MarketStaff::ERole::Stocker;
            Stats.Hr += Role == MarketStaff::ERole::HrManager; Stats.Accountant += Role == MarketStaff::ERole::Accountant;
            Roster += FString::Printf(TEXT("%d:%s:%d:%lld|"), Person.Id, *Safe(Person.Name), static_cast<int32>(Role), Person.DailyWage);
        }
        Stats.ManagerCost = Stats.FirstStoreManagerCost = 0; Stats.Managers.Empty();
        Stats.ManagersBefore = State.Management.Managers;
        for (const auto& Manager : State.Management.Managers)
        {
            const int64 Cost = MarketStaff::EmployerCost(MarketManagers::DailyWage(State, Manager));
            Stats.ManagerCost += Cost;
            if (Manager.Level == static_cast<uint8>(MarketManagers::ELevel::FirstStore)) Stats.FirstStoreManagerCost += Cost;
            Stats.Managers += FString::Printf(TEXT("%s:%d:%lld|"), *Safe(Manager.Name), Manager.Level, Cost);
        }
        Roster += Stats.Managers;
        if (Roster != Stats.Roster) { Stats.Roster = Roster; Stats.Events.Add(FString::Printf(TEXT("%d,roster,0,%s\n"), State.Day, *Roster)); }
    }
    FString Header()
    {
        return TEXT("gun,takvim_yili,ay,musteri,alan,kaybolan,satilan_adet,pahali_istek,bos_raf_istek,rafta_yok_istek,bekleme_kaybi,kart_kaybi,ciro_kurus,mal_maliyeti_kurus,brut_kar_kurus,ucret_kurus,sgk_kurus,kira_kurus,isletme_kurus,fire_kurus,aile_diger_gider_kurus,aile_favok_kurus,sirket_favok_kurus,sirket_net_kurus,faiz_kurus,vergi_kurus,vergi_odeme_kurus,toptanci_vade_kurus,kasa_kurus,patron_net_maas_kurus,patron_net_kar_payi_kurus,servet_kurus,mudur_gunluk_maliyet_kurus,aile_mudur_gunluk_maliyet_kurus,calisan,kasiyer,gorevli,ik,musavir,raf_ortalama_kurus,liste_ortalama_kurus,alis_ortalama_kurus,stok_maliyeti_ortalama_kurus,bot_hedef_ortalama_kurus,liste_duzeyi,yerel_pay,butce_carpani,donem_butce,donem_ithal_carpani,donem_talep_carpani,donem,rakip_fiyatlari,mudurler,siparis_kurus,magaza,patron_sirket_maliyeti_kurus\n");
    }
    FString ProductHeader() { return TEXT("gun,urun,takvim_yili,raf_kurus,liste_kurus,alis_kurus,stok_ortalama_kurus,bot_hedef_kurus,raf_adet,depo_adet\n"); }
    void Observe(const FMarketState& State, const MarketSimulation::FDay& Day, int64 Ordered, FStats& Stats)
    {
        const int32 Closed = State.Day - 1;
        for(const auto& Account:State.SupplierAccounts)if(Account.LifelineDay>Stats.LifelineBefore)
            Stats.Events.Add(FString::Printf(TEXT("%d,lifeline,0,wholesaler\n"),Closed));
        if(State.LowCashWarnDay>Stats.WarningBefore)Stats.Events.Add(FString::Printf(TEXT("%d,goods_warning,0,cash\n"),Closed)); const auto Date = MarketCalendar::DateOf(Closed);
        using A = MarketLedger::EAccount;
        const auto Family = MarketLedger::Statement(State, Closed, Closed, MarketLedger::FirstStore);
        const auto Company = MarketLedger::Statement(State, Closed, Closed);
        // Management is paid after the day has advanced. Read the booked gross total, including managers
        // removed by a rescue later in the same close, rather than using yesterday's wage index.
        Stats.ManagerCost = MarketStaff::EmployerCost(State.Management.LastWages);
        Stats.FirstStoreManagerCost = 0;
        for (const auto& Manager : Stats.ManagersBefore)
            if (Manager.Level == static_cast<uint8>(MarketManagers::ELevel::FirstStore))
                Stats.FirstStoreManagerCost += MarketStaff::EmployerCost(MarketManagers::DailyWage(State, Manager));
        int32 Sold = 0, Expensive = 0, Empty = 0, Missing = 0;
        for (const auto& Item : State.Stock) { Sold += Item.Yesterday.Sold; Expensive += Item.Yesterday.Expensive; Empty += Item.Yesterday.Empty; Missing += Item.Yesterday.NotCarried; }
        FString Row;
        auto N = [&](int64 V) { Row += FString::Printf(TEXT("%lld,"), V); };
        auto F = [&](double V) { Row += FString::Printf(TEXT("%.6f,"), V); };
        auto S = [&](const FString& V) { Row += Safe(V) + TEXT(","); };
        N(Closed); N(Date.Year); N(Date.Month); N(Day.Shoppers); N(Day.Served); N(Day.Lost); N(Sold); N(Expensive); N(Empty); N(Missing); N(State.LastLostWaiting); N(State.Payments.LastNoCardLost);
        N(Family.Revenue); N(-Family.At(A::CostOfGoods)); N(Family.GrossProfit); N(-Family.At(A::Wages)); N(-Family.At(A::SocialSecurity)); N(-Family.At(A::Rent)); N(-Family.At(A::Utilities)); N(-Family.At(A::Waste));
        N(-Family.Expenses + Family.At(A::Wages) + Family.At(A::SocialSecurity) + Family.At(A::Rent) + Family.At(A::Utilities) + Family.At(A::Interest) + Family.At(A::Tax));
        N(Family.NetProfit - Family.At(A::Interest) - Family.At(A::Tax)); N(Company.NetProfit - Company.At(A::Interest) - Company.At(A::Tax)); N(Company.NetProfit); N(-Company.At(A::Interest)); N(-Company.At(A::Tax)); N(-Company.At(A::TaxPayment));
        N(MarketSuppliers::OpenBills(State)); N(State.Cash); N(State.Owner.TotalSalary - Stats.SalaryBefore); N(State.Owner.TotalDividends - Stats.DividendBefore + Stats.PendingDividend); N(State.Owner.Wealth); N(Stats.ManagerCost); N(Stats.FirstStoreManagerCost);
        N(Stats.StaffCount); N(Stats.Cashiers); N(Stats.Stockers); N(Stats.Hr); N(Stats.Accountant); F(Stats.Shelf); F(Stats.List); F(Stats.Purchase); F(Stats.Book); F(Stats.Target); F(MarketPrices::ListLevel(Closed)); F(State.MarketShare); F(MarketDirector::BudgetFactor(State, 0)); F(MarketEras::BudgetFactor(State)); F(MarketEras::ImportCostFactor(State, 1.f, Closed)); F(MarketEras::NonFoodDemand(State, Closed));
        MarketEras::FEra Era; S(MarketEras::Current(State, Closed, Era) ? FString::FromInt(static_cast<int32>(Era.Kind)) : TEXT("none"));
        S(FString::Printf(TEXT("%.4f"), MarketDirector::RivalPriceFactor(State, FString()))); // E2: the home province's rival price level
        S(Stats.Managers); N(Ordered); N(MarketCompany::TotalStores(State));
        N(State.Owner.TotalSalary > Stats.SalaryBefore ? MarketOwner::CompanyCost(State) : 0);
        Stats.PendingDividend = 0;
        Row.LeftChopInline(1); Row += TEXT("\n"); Stats.Days.Add(Row);
    }
}

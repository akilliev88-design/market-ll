#include "MarketFinance.h"
#include "MarketCast.h"
#include "MarketLedger.h"
#include "MarketCountry.h"
#include "MarketCalendar.h"
#include "MarketCredit.h"
#include "MarketEvents.h"
#include "MarketPrices.h"
#include "MarketSuppliers.h"
#include "MarketBranches.h"
#include "MarketChains.h"
#include "MarketManagers.h"
#include "MarketStaff.h"

namespace MarketFinance
{
    FString FinanceTl(int64 Kurus)
    {
        return MarketCountry::Money(Kurus); // G-084: the active country\'s currency
    }

    double Level(const FMarketState& State) { return MarketPrices::ListLevel(State.Day); }

    void AddLoan(FMarketState& State, int64 Principal, double YearlyRate, bool bMortgage, int32 Months = LoanMonths)
    {
        FMarketLoan Loan;
        Loan.Principal = Principal;
        Loan.Remaining = Principal;
        Loan.MonthlyRate = static_cast<float>(YearlyRate / 12.0);
        Loan.Installment = Installment(Principal, Loan.MonthlyRate, Months);
        Loan.NextDueDay = State.Day + MonthDays;
        Loan.bMortgage = bMortgage;
        State.Loans.Add(Loan);
        State.Cash += Principal;
        MarketLedger::Post(State, MarketLedger::EAccount::LoanIn, Principal); // B2
    }

    int64 DepotValue(const FMarketState& State, const TArray<FMarketProduct>& Products)
    {
        int64 Value = 0;
        for (int32 I = 0; I < State.Stock.Num() && I < Products.Num(); ++I) Value += static_cast<int64>(State.Stock[I].Warehouse) * State.UnitCost(I, Products);
        return Value;
    }

    int64 SellDepotAtHalf(FMarketState& State, const TArray<FMarketProduct>& Products)
    {
        int64 Got = 0;
        for (int32 I = 0; I < State.Stock.Num() && I < Products.Num(); ++I)
        {
            const int64 Book = static_cast<int64>(State.Stock[I].Warehouse) * State.UnitCost(I, Products);
            Got += Book / 2;
            State.PendingLoss += Book - Book / 2; // sold below cost (next report)
            MarketLedger::Post(State, MarketLedger::EAccount::Shrinkage, -(Book - Book / 2), false); // B2
            State.Stock[I].Warehouse = 0;
        }
        State.Cash += Got;
        MarketLedger::Post(State, MarketLedger::EAccount::Divestment, Got); // B2
        return Got;
    }

    FMarketDecision FinanceDecision(const FMarketState& State, const TCHAR* Id, const FString& Title, const FString& Text, std::initializer_list<FString> Options, int32 Default, int32 Days)
    {
        FMarketDecision D;
        D.Id = Id; D.Title = Title; D.Text = Text;
        for (const FString& O : Options) D.Options.Add(O);
        D.DefaultOption = Default;
        D.Deadline = State.Day + Days - 1;
        return D;
    }
}

int64 MarketFinance::Installment(int64 Principal, double MonthlyRate, int32 Months)
{
    if (Months <= 0) return Principal;
    if (MonthlyRate <= 0.0) return (Principal + Months - 1) / Months;
    const double Factor = FMath::Pow(1.0 + MonthlyRate, static_cast<double>(Months));
    return FMath::RoundToInt64(Principal * MonthlyRate * Factor / (Factor - 1.0));
}

int64 MarketFinance::Debt(const FMarketState& State)
{
    int64 Total = 0;
    for (const FMarketLoan& L : State.Loans) Total += L.Remaining;
    return Total;
}

int64 MarketFinance::LoanLimit(const FMarketState& State)
{
    int64 Profit = 0;
    int32 Days = 0;
    for (int32 I = State.History.Num() - 1; I >= 0 && Days < 30; --I, ++Days) Profit += State.History[I].Profit;
    const int64 Monthly = Days > 0 ? Profit * 30 / Days : 0;
    // Six months of profit plus the family name, minus what is already owed. Trouble closes the door.
    const int64 Limit = FMath::Max<int64>(0, Monthly) * 6 + FMath::RoundToInt64(50000 * Level(State)) - Debt(State);
    return State.TroubleStage >= 2 ? 0 : FMath::Max<int64>(0, Limit);
}

bool MarketFinance::TakeLoan(FMarketState& State, int32 Step, FString& OutMessage)
{
    const int64 Amount = FMath::RoundToInt64(LoanSteps[FMath::Clamp(Step, 0, 2)] * Level(State) / 100.0) * 100;
    const int64 Limit = LoanLimit(State);
    if (Amount > Limit)
    {
        OutMessage = FString::Printf(TEXT("%s: \"\u015eu an en \u00e7ok %s verebiliriz.\" K\u00e2rl\u0131 g\u00fcnler limiti art\u0131r\u0131r."), *MarketCast::Bank(0), *FinanceTl(Limit)); // M30: the country's local bank
        return false;
    }
    const double Rate = MarketPrices::LoanRate(State.Day);
    AddLoan(State, Amount, Rate, false);
    OutMessage = FString::Printf(TEXT("Kredi kasaya ge\u00e7ti: %s, y\u0131ll\u0131k %%%.0f faiz, 12 ay \u00d7 %s."), *FinanceTl(Amount), Rate * 100.0, *FinanceTl(State.Loans.Last().Installment));
    return true;
}

bool MarketFinance::RepayAll(FMarketState& State, FString& OutMessage)
{
    const int64 Owed = Debt(State);
    if (Owed <= 0) { OutMessage = TEXT("Banka borcu yok."); return false; }
    const int64 Fee = FMath::RoundToInt64(Owed * static_cast<double>(EarlyRepayFee));
    if (State.Cash < Owed + Fee) { OutMessage = FString::Printf(TEXT("Erken kapatmak i\u00e7in %s gerekiyor."), *FinanceTl(Owed + Fee)); return false; }
    State.Cash -= Owed + Fee;
    State.PendingLoss += Fee; // shows in the next day report and in the tax books
    MarketLedger::Post(State, MarketLedger::EAccount::LoanRepayment, -Owed); // B2
    MarketLedger::Post(State, MarketLedger::EAccount::BankFees, -Fee);
    State.Loans.Reset();
    OutMessage = FString::Printf(TEXT("Kredi erken kapand\u0131: %s (%s erken \u00f6deme \u00fccreti)."), *FinanceTl(Owed + Fee), *FinanceTl(Fee));
    return true;
}

FString MarketFinance::Summary(const FMarketState& State)
{
    FString Text = FString::Printf(TEXT("Banka borcu %s \u00b7 kredi limiti %s"), *FinanceTl(Debt(State)), *FinanceTl(LoanLimit(State)));
    if (State.Loans.Num() > 0)
    {
        int32 Next = MAX_int32;
        int64 Due = 0;
        for (const FMarketLoan& L : State.Loans) { Next = FMath::Min(Next, L.NextDueDay); Due += L.Installment; }
        Text += FString::Printf(TEXT(" \u00b7 taksit %s, %d. g\u00fcn"), *FinanceTl(Due), Next);
    }
    if (MarketCredit::Outstanding(State) > 0) Text += TEXT(" \u00b7 veresiye alaca\u011f\u0131 ") + FinanceTl(MarketCredit::Outstanding(State));
    if (State.TroubleStage > 0) Text += FString::Printf(TEXT(" \u00b7 nakit s\u0131k\u0131nt\u0131s\u0131 (%d. ad\u0131m)"), State.TroubleStage);
    return Text;
}

void MarketFinance::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    const int32 Closed = State.Day - 1;
    if (Closed < 1) return;
    TArray<FString>& News = State.DayNews;

    // Installments.
    for (FMarketLoan& L : State.Loans)
    {
        if (L.NextDueDay > Closed || L.Remaining <= 0) continue;
        const int64 Interest = FMath::RoundToInt64(L.Remaining * static_cast<double>(L.MonthlyRate));
        const int64 Pay = FMath::Min(L.Installment, L.Remaining + Interest);
        if (State.Cash >= Pay)
        {
            State.Cash -= Pay;
            MarketLedger::Post(State, MarketLedger::EAccount::Interest, -FMath::Min(Interest, Pay)); // B2
            MarketLedger::Post(State, MarketLedger::EAccount::LoanRepayment, -(Pay - FMath::Min(Interest, Pay)));
            L.Remaining = FMath::Max<int64>(0, L.Remaining - (Pay - Interest));
            State.LastProfit -= Interest;
            State.Books.PeriodProfit -= Interest; // the books closed before the bank: interest lowers taxable profit
            L.NextDueDay += MonthDays;
            News.Add(FString::Printf(TEXT("%s taksiti \u00f6dendi: %s (faiz %s, kalan %s)."), *MarketCast::Bank(0), *FinanceTl(Pay), *FinanceTl(Interest), *FinanceTl(L.Remaining)));
        }
        else
        {
            const int64 Fee = FMath::Max<int64>(100, FMath::RoundToInt64(Pay * static_cast<double>(LateFee)));
            L.Remaining += Fee;
            State.LastProfit -= Fee;
            MarketLedger::Post(State, MarketLedger::EAccount::Penalties, -Fee, false); // B2: the loan grows
            State.Books.PeriodProfit -= Fee;
            L.NextDueDay = State.Day;
            News.Add(FString::Printf(TEXT("Banka taksiti \u00f6denemedi (%s): %s gecikme faizi eklendi."), *FinanceTl(Pay), *FinanceTl(Fee)));
        }
    }
    State.Loans.RemoveAll([](const FMarketLoan& L) { return L.Remaining <= 0; });

    // The family's living money (never pushes the till below zero).
    const int64 Home = HouseholdToday(State);
    State.Cash -= Home;
    MarketLedger::Post(State, MarketLedger::EAccount::OwnerDraw, -Home); // B2
    State.MonthHousehold += Home;

    // The trouble ladder.
    if (State.Cash < 0) ++State.NegativeCashDays;
    else
    {
        if (State.TroubleStage > 0) News.Add(TEXT("Kasa yeniden art\u0131da. Toptanc\u0131 ve banka rahatlad\u0131."));
        State.NegativeCashDays = 0;
        State.TroubleStage = 0;
    }
    const int32 Neg = State.NegativeCashDays;
    const int32 Stage = Neg >= 30 ? 5 : Neg >= 14 ? 4 : Neg >= 7 ? 3 : Neg >= 3 ? 2 : Neg >= 1 ? 1 : 0;
    if (Stage > State.TroubleStage)
    {
        State.TroubleStage = Stage;
        switch (Stage)
        {
        case 1:
            News.Add(FString::Printf(TEXT("Kasa eksiye d\u00fc\u015ft\u00fc (%s). \u00dc\u00e7 g\u00fcn s\u00fcrerse toptanc\u0131 vadeyi keser."), *FinanceTl(State.Cash)));
            break;
        case 2:
            if (FMarketSupplierAccount* A = State.SupplierAccounts.FindByPredicate([](const FMarketSupplierAccount& X) { return X.Supplier == 0; }))
                A->Trust = FMath::Min(A->Trust, MarketSuppliers::TermsTrust - 1);
            News.Add(FString::Printf(TEXT("%s (%s): \"Hesab\u0131n eksi g\u00f6r\u00fcn\u00fcyor. Bir s\u00fcre pe\u015fin \u00e7al\u0131\u015fal\u0131m.\" Vade kapand\u0131; banka da yeni kredi vermiyor."), *MarketCast::Salesman(), *MarketCast::Wholesaler()));
            break;
        case 3:
        {
            const int64 Emergency = FMath::RoundToInt64(FMath::Max<int64>(-State.Cash, 20000) * 1.5 / 100.0) * 100;
            MarketEvents::Offer(State, FinanceDecision(State, TEXT("finance.rescue"), TEXT("Nakit s\u0131k\u0131nt\u0131s\u0131"),
                FString::Printf(TEXT("Bir haftad\u0131r kasa eksi. Banka y\u00fcksek faizle acil kredi verebilir (%s), ya da depodaki mal\u0131 yar\u0131 fiyat\u0131na bir toptanc\u0131ya verebilirsin (%s)."),
                    *FinanceTl(Emergency), *FinanceTl(DepotValue(State, Products) / 2)),
                { FString(TEXT("Acil kredi al")), FString(TEXT("Depoyu yar\u0131 fiyat\u0131na sat")), FString(TEXT("Biraz daha bekle")) }, 2, 3));
            State.Decisions.Last().Arg = static_cast<int32>(FMath::Min<int64>(Emergency, MAX_int32));
            break;
        }
        case 4:
        {
            const int64 Got = SellDepotAtHalf(State, Products);
            News.Add(FString::Printf(TEXT("\u0130ki haftad\u0131r kasa eksi: toptanc\u0131lar depodaki mal\u0131 yar\u0131 fiyat\u0131na ald\u0131 (%s). Raflardaki mal sat\u0131lmaya devam ediyor."), *FinanceTl(Got)));
            break;
        }
        default:
        {
            const int64 Mortgage = MortgageAmount(State);
            MarketEvents::Offer(State, FinanceDecision(State, TEXT("finance.mortgage"), TEXT("Tapu"),
                FString::Printf(TEXT("Bir ayd\u0131r kasa eksi. %s d\u00fckk\u00e2n\u0131n tapusu kar\u015f\u0131l\u0131\u011f\u0131nda %s kredi \u00f6neriyor (24 ay, y\u0131ll\u0131k %%%.0f faiz, %%%.0f masraf). \u00d6denmezse d\u00fckk\u00e2n bankan\u0131n olur."),
                    *MarketCast::Bank(0), *FinanceTl(Mortgage), (MarketPrices::LoanRate(State.Day) + MortgageRateBonus) * 100.0, MortgageFee * 100.0),
                { FString(TEXT("Tapuyu ipotek ver")), FString(TEXT("Hay\u0131r, ba\u015fka yol bulurum")) }, 1, 5));
            State.Decisions.Last().Arg = static_cast<int32>(FMath::Min<int64>(Mortgage, MAX_int32));
            break;
        }
        }
    }

    // M31: the bank's rescue plan instead of a zombie company.
    if (Neg == RescueWarnDays)
        News.Add(FString::Printf(TEXT("%s: \"K\u0131rk be\u015f g\u00fcnd\u00fcr kasa eksi. On be\u015f g\u00fcn i\u00e7inde art\u0131ya d\u00f6nmezse kurtarma plan\u0131 uygulan\u0131r: ba\u011fl\u0131 \u015firketler sat\u0131l\u0131r, zarar eden \u015fubeler kapan\u0131r, kalan a\u00e7\u0131k uzun vadeli krediye \u00e7evrilir.\""), *MarketCast::Bank(0)));
    if (Neg >= RescueDays) News.Append(Rescue(State, Products));

    // Month end: the new month starts tomorrow.
    if (MarketCalendar::DateOf(State.Day).Day == 1 && State.History.Num() > 0)
    {
        const MarketCalendar::FDate Month = MarketCalendar::DateOf(Closed);
        int64 Revenue = 0, Profit = 0;
        int32 Days = 0;
        for (const FMarketDayRecord& R : State.History)
        {
            const MarketCalendar::FDate D = MarketCalendar::DateOf(R.Day);
            if (D.Year == Month.Year && D.Month == Month.Month) { Revenue += R.Revenue; Profit += R.Profit; ++Days; }
        }
        News.Add(FString::Printf(TEXT("Ay sonu raporu (%s): %d g\u00fcn, ciro %s, net %s, eve %s. Kasa %s, veresiye alaca\u011f\u0131 %s, toptanc\u0131ya bor\u00e7 %s, banka borcu %s."),
            *MarketCalendar::MonthText(Closed),
            Days, *FinanceTl(Revenue), *FinanceTl(Profit), *FinanceTl(State.MonthHousehold), *FinanceTl(State.Cash), *FinanceTl(MarketCredit::Outstanding(State)),
            *FinanceTl(MarketSuppliers::OpenBills(State)), *FinanceTl(Debt(State))));
        State.MonthHousehold = 0;
    }
}

int64 MarketFinance::MortgageAmount(const FMarketState& State)
{
    const int64 Need = FMath::RoundToInt64(FMath::Max<int64>(0, -State.Cash) * 1.5);
    const int64 Amount = FMath::Clamp<int64>(Need, FMath::RoundToInt64(MortgageMin * Level(State)), FMath::RoundToInt64(MortgageMax * Level(State)));
    return FMath::Max<int64>(100, Amount / 100 * 100);
}

int64 MarketFinance::HouseholdToday(const FMarketState& State)
{
    const int64 Full = FMath::RoundToInt64(HouseholdDraw * MarketPrices::WageIndex(FMath::Max(1, State.Day - 1)));
    if (State.Cash >= 3 * Full) return Full;
    if (State.Cash >= Full) return Full / 2;   // a tight month at home
    return 0;
}

bool MarketFinance::Resolve(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& D, int32 Option, FString& OutMessage)
{
    if (D.Id == TEXT("finance.rescue"))
    {
        if (Option == 0)
        {
            AddLoan(State, D.Arg, MarketPrices::LoanRate(State.Day) + EmergencyRateBonus, false);
            OutMessage = FString::Printf(TEXT("Acil kredi kasaya ge\u00e7ti: %s, y\u00fcksek faizle."), *FinanceTl(D.Arg));
        }
        else if (Option == 1) OutMessage = FString::Printf(TEXT("Depo yar\u0131 fiyat\u0131na sat\u0131ld\u0131: %s kasaya girdi."), *FinanceTl(SellDepotAtHalf(State, Products)));
        else OutMessage = TEXT("Beklemeye karar verdin. Bir hafta daha eksi kal\u0131rsa depo zorla sat\u0131l\u0131r.");
        return true;
    }
    if (D.Id == TEXT("finance.mortgage"))
    {
        if (Option == 0)
        {
            // B1 (#43): risk premium and the valuation / deed fee (a cost of the day, paid out of the loan).
            AddLoan(State, D.Arg, MarketPrices::LoanRate(State.Day) + MortgageRateBonus, true, 24);
            const int64 Fee = FMath::RoundToInt64(D.Arg * static_cast<double>(MortgageFee));
            State.Cash -= Fee;
            State.PendingLoss += Fee;
            MarketLedger::Post(State, MarketLedger::EAccount::BankFees, -Fee); // B2
            OutMessage = FString::Printf(TEXT("Tapu ipotek edildi; masraflar d\u00fc\u015f\u00fcld\u00fckten sonra %s kasaya ge\u00e7ti. Aileden kalan d\u00fckk\u00e2n art\u0131k bankaya ba\u011fl\u0131."), *FinanceTl(D.Arg - Fee));
        }
        else OutMessage = TEXT("Tapuya dokunmad\u0131n.");
        return true;
    }
    OutMessage = TEXT("Bu karar art\u0131k ge\u00e7erli de\u011fil.");
    return true;
}

TArray<FString> MarketFinance::Rescue(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    TArray<FString> Lines;
    ++State.Rescues;
    Lines.Add(FString::Printf(TEXT("%s kurtarma plan\u0131n\u0131 uyguluyor (%d. kez)."), *MarketCast::Bank(0), State.Rescues));

    // 1. Subsidiaries bring money.
    for (int32 I = 0; I < State.Rivals.Chains.Num(); ++I)
    {
        if (!State.Rivals.Chains[I].bOurs || State.Rivals.Chains[I].bGone) continue;
        FString Message;
        if (MarketChains::SellSubsidiary(State, I, Message)) Lines.Add(Message);
    }

    // 2. Losing branches close, the worst first; the ones still being fitted out stop too.
    TArray<int32> Closing;
    for (int32 I = 0; I < State.Branches.Num(); ++I)
    {
        const FMarketBranch& B = State.Branches[I];
        const MarketBranches::EStage Stage = static_cast<MarketBranches::EStage>(B.Stage);
        if (Stage == MarketBranches::EStage::Closed) continue;
        if (Stage == MarketBranches::EStage::Open && B.Last30Profit >= 0) continue;
        Closing.Add(I);
    }
    Closing.Sort([&State](int32 A, int32 B) { return State.Branches[A].Last30Profit < State.Branches[B].Last30Profit; });
    TArray<FString> Names;
    for (const int32 I : Closing)
    {
        FString Message;
        if (MarketBranches::Close(State, Products, I, Message)) Names.Add(State.Branches[I].Name);
    }
    if (Names.Num() > 0) Lines.Add(FString::Printf(TEXT("Kapanan \u015fubeler (%d): %s. Depozitolar geri al\u0131nd\u0131, mallar ana depoya ta\u015f\u0131nd\u0131."), Names.Num(), *FString::Join(Names, TEXT(", "))));

    // 3. With no branch left, the managers have nothing to run: their wages were not paid, they leave.
    if (MarketBranches::OpenCount(State) == 0)
    {
        int32 Left = 0;
        for (int32 I = State.Management.Managers.Num() - 1; I >= 0; --I)
        {
            const MarketManagers::ELevel Tier = static_cast<MarketManagers::ELevel>(State.Management.Managers[I].Level);
            if (Tier == MarketManagers::ELevel::FamilyShop || (Tier == MarketManagers::ELevel::Depot && State.Company.DepotSites.Num() > 0)) continue;
            State.Management.UsedNames.AddUnique(State.Management.Managers[I].Name);
            State.Management.Managers.RemoveAt(I);
            ++Left;
        }
        if (Left > 0) Lines.Add(FString::Printf(TEXT("Maa\u015flar\u0131 \u00f6denemeyen %d y\u00f6netici ayr\u0131ld\u0131."), Left));

        // The family shop keeps its best few.
        TArray<int32> Workers;
        for (int32 I = 0; I < State.Staff.Num(); ++I)
            if (MarketStaff::RoleOf(State.Staff[I]) == MarketStaff::ERole::Cashier || MarketStaff::RoleOf(State.Staff[I]) == MarketStaff::ERole::Stocker) Workers.Add(I);
        if (Workers.Num() > RescueKeepStaff)
        {
            Workers.Sort([&State](int32 A, int32 B) { return State.Staff[A].Skill > State.Staff[B].Skill; });
            TArray<int32> Going(Workers.GetData() + RescueKeepStaff, Workers.Num() - RescueKeepStaff);
            Going.Sort([](int32 A, int32 B) { return A > B; });
            for (const int32 I : Going) State.Staff.RemoveAt(I);
            MarketStaff::SyncCounts(State);
            Lines.Add(FString::Printf(TEXT("Aile d\u00fckk\u00e2n\u0131nda kasa ve rafta %d ki\u015fi kald\u0131; %d ki\u015fi \u00fccreti \u00f6denemedi\u011fi i\u00e7in ayr\u0131ld\u0131."), RescueKeepStaff, Going.Num()));
        }
    }

    // 4. What is still missing (and money to fill the shelves) becomes a long loan.
    const int64 Working = FMath::RoundToInt64(RescueWorkingCapital * Level(State));
    if (State.Cash < Working)
    {
        const int64 Amount = FMath::Max<int64>(100, (Working - State.Cash + 99) / 100 * 100);
        const double Rate = MarketPrices::LoanRate(State.Day) + RescueRateBonus + 0.04 * FMath::Max(0, State.Rescues - 1);
        AddLoan(State, Amount, Rate, false, RescueMonths);
        Lines.Add(FString::Printf(TEXT("Kalan a\u00e7\u0131k ve raflar\u0131 doldurmaya yetecek para kurtarma kredisine \u00e7evrildi: %s, %d ay, y\u0131ll\u0131k %%%.0f faiz, ayda %s."),
            *FinanceTl(Amount), RescueMonths, Rate * 100.0, *FinanceTl(State.Loans.Last().Installment)));
    }
    State.NegativeCashDays = 0;
    State.TroubleStage = 0;
    State.Decisions.RemoveAll([](const FMarketDecision& D) { return D.Id.StartsWith(TEXT("finance.")); }); // the ladder's open offers are void now
    Lines.Add(TEXT("\u015eirket aile d\u00fckk\u00e2n\u0131ndan yeniden ba\u015fl\u0131yor. Ders: her \u015fubenin ayl\u0131k gideri kadar yedek tut."));
    return Lines;
}

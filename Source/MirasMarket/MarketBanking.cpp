#include "MarketBanking.h"
#include "MarketStrategy.h"
#include "MarketBranches.h"
#include "MarketCast.h"
#include "MarketCalendar.h"
#include "MarketCountry.h"
#include "MarketFinance.h"
#include "MarketPrices.h"
#include "MarketChains.h"

namespace MarketBankingLocal
{
    using MarketBanking::ERating;
    using MarketBanking::EKind;
    using MarketLedger::EAccount;

    const int32 HQ = MarketLedger::HeadOfficeStore;

    double Level(const FMarketState& State) { return MarketPrices::ListLevel(FMath::Max(1, State.Day)); }

    // A bank's ceiling at the start price level (kurus): the local bank stays small.
    int64 BankCap(int32 Index)
    {
        return Index == 0 ? 2000000 : Index == 1 ? 20000000 : Index == 2 ? 100000000 : Index == 3 ? 30000000 : 300000000;
    }

    double MonthlyRate(double PerYear) { return PerYear / 12.0; }

    FString Percent(double Value) { return FString::Printf(TEXT("%%%.1f"), Value * 100.0); }

    int32 MonthsOf(const FMarketState& State)
    {
        // Calendar months closed since the campaign began.
        const MarketCalendar::FDate Start = MarketCalendar::DateOf(1);
        const MarketCalendar::FDate Now = MarketCalendar::DateOf(FMath::Max(1, State.Day - 1));
        return FMath::Max(0, (Now.Year - Start.Year) * 12 + Now.Month - Start.Month);
    }

    void Book(FMarketState& State, EAccount Account, int64 Amount, bool bCash = true)
    {
        if (Amount != 0) MarketLedger::Post(State, Account, Amount, bCash, HQ);
    }

    int32 RecentLates(const FMarketState& State)
    {
        int32 Count = 0;
        for (const int32 Day : State.Banking.LateDays) if (State.Day - Day <= 365) ++Count;
        return Count;
    }

    // The rating's score and its parts.
    int32 Score(const FMarketState& State, TArray<FString>* Lines)
    {
        const MarketBanking::FPicture P = MarketBanking::Picture(State);
        int32 Total = 0;
        auto Part = [&Total, Lines](int32 Points, const FString& Text)
        {
            Total += Points;
            if (Lines) Lines->Add(FString::Printf(TEXT("%s: %s%d"), *Text, Points >= 0 ? TEXT("+") : TEXT(""), Points));
        };
        if (P.Debt <= 0) Part(3, TEXT("Banka borcu yok"));
        else if (P.Ebitda <= 0) Part(-3, TEXT("Faaliyet sonucu eksi, borcu kar\u015f\u0131lam\u0131yor"));
        else
        {
            const float L = P.Leverage;
            Part(L < 1.f ? 3 : L < 2.f ? 2 : L < 3.f ? 1 : L < 4.f ? 0 : L < 5.f ? -1 : -2, FString::Printf(TEXT("Bor\u00e7 / FAV\u00d6K %.1f"), L));
            Part(P.Cover > 8.f ? 2 : P.Cover > 4.f ? 1 : P.Cover > 2.f ? 0 : -1, FString::Printf(TEXT("Faizi kar\u015f\u0131lama %.1f kat"), FMath::Min(P.Cover, 99.f)));
        }
        const double Lira = P.Revenue / 100.0 / FMath::Max(0.01, Level(State));
        Part(Lira >= 5.0e6 ? 2 : Lira >= 1.0e6 ? 1 : 0, TEXT("\u015eirketin b\u00fcy\u00fckl\u00fc\u011f\u00fc"));
        Part(P.Months >= 24 ? 1 : 0, FString::Printf(TEXT("%d ayl\u0131k defter"), P.Months));
        const int32 Lates = RecentLates(State);
        if (Lates > 0) Part(-FMath::Min(3, Lates), FString::Printf(TEXT("Son bir y\u0131lda %d geciken taksit"), Lates));
        if (State.TroubleStage > 0) Part(-2, TEXT("Nakit s\u0131k\u0131nt\u0131s\u0131"));
        if (State.Banking.BreachMonths > 0) Part(-1, TEXT("Bor\u00e7 s\u0131n\u0131r\u0131 a\u015f\u0131ld\u0131"));
        if (State.Day < State.Banking.RestructuredUntil) Part(-2, TEXT("Yap\u0131land\u0131r\u0131lm\u0131\u015f bor\u00e7"));
        return Total;
    }

    ERating FromScore(int32 S)
    {
        return S >= 6 ? ERating::APlus : S >= 4 ? ERating::A : S >= 2 ? ERating::BPlus : S >= 0 ? ERating::B : S >= -2 ? ERating::C : ERating::D;
    }

    // One month of a loan: interest, and the principal part (none in the grace or for a bond until its last month).
    // E4b: one local unit of a loan's country in our money today (1 at home).
    double FxOf(const FMarketState& State, const FMarketCorpLoan& L)
    {
        return L.Country.IsEmpty() || L.Country == State.CountryId ? 1.0 : MarketPrices::ToHome(State, L.Country, FMath::Max(1, State.Day));
    }

    int64 HomeOf(const FMarketState& State, const FMarketCorpLoan& L, int64 Local)
    {
        const double Fx = FxOf(State, L);
        return Fx == 1.0 ? Local : FMath::RoundToInt64(static_cast<double>(Local) * Fx);
    }

    void MonthDue(const FMarketCorpLoan& L, int64& OutInterest, int64& OutPrincipal)
    {
        OutInterest = FMath::RoundToInt64(L.Balance * MonthlyRate(L.YearRate));
        if (L.PaidMonths < L.Grace) { OutPrincipal = 0; return; }
        if (L.Kind == static_cast<uint8>(EKind::Bond)) { OutPrincipal = L.PaidMonths + 1 >= L.Months ? L.Balance : 0; return; }
        const bool bLast = L.PaidMonths + 1 >= L.Months;
        OutPrincipal = bLast ? L.Balance : FMath::Clamp<int64>(L.Installment - OutInterest, 0, L.Balance);
    }

    int64 LineRoom(const FMarketState& State)
    {
        const FMarketBankingState& B = State.Banking;
        return B.bLine ? FMath::Max<int64>(0, B.LineLimit - B.LineDrawn) : 0;
    }

    void Draw(FMarketState& State, int64 Amount)
    {
        if (Amount <= 0) return;
        State.Banking.LineDrawn += Amount;
        State.Cash += Amount;
        Book(State, EAccount::LoanIn, Amount);
    }

    int32 AddLoan(FMarketState& State, int32 BankIndex, EKind Kind, int64 Amount, double Rate, int32 Months, int32 Grace)
    {
        FMarketBankingState& B = State.Banking;
        FMarketCorpLoan L;
        L.Id = B.NextLoanId++;
        L.Bank = static_cast<uint8>(BankIndex);
        L.Kind = static_cast<uint8>(Kind);
        L.Principal = Amount;
        L.Balance = Amount;
        L.YearRate = static_cast<float>(Rate);
        L.Months = Months;
        L.Grace = Grace;
        L.StartDay = State.Day;
        L.NextDueDay = State.Day + MarketBanking::MonthDays;
        L.Installment = MarketBanking::InstallmentOf(Amount, Rate, FMath::Max(1, Months - Grace), Kind == EKind::Bond);
        return B.Loans.Add(L);
    }
}

MarketBanking::FBank MarketBanking::Bank(const FMarketState& State, int32 Index)
{
    // M30: every country has its own banks (Config/ulkeler.json "banks"); the characters stay the same.
    switch (Index)
    {
    case 0: return { MarketCast::Bank(0), 0.03f, 0.4f, ERating::C, 36, false };
    case 1: return { MarketCast::Bank(1), 0.015f, 1.f, ERating::B, 48, false };
    case 2: return { MarketCast::Bank(2), 0.005f, 1.6f, ERating::BPlus, 60, false };
    case 3: return { MarketCast::Bank(3), -0.02f, 0.8f, ERating::B, 60, true };
    default: return { FString(TEXT("Tahvil (piyasadan)")), -0.01f, 2.5f, ERating::A, 60, false };
    }
}

FString MarketBanking::RatingName(ERating Rating)
{
    switch (Rating)
    {
    case ERating::APlus: return TEXT("A+");
    case ERating::A: return TEXT("A");
    case ERating::BPlus: return TEXT("B+");
    case ERating::B: return TEXT("B");
    case ERating::C: return TEXT("C");
    default: return TEXT("D");
    }
}

float MarketBanking::RatingSpread(ERating Rating)
{
    switch (Rating)
    {
    case ERating::APlus: return 0.01f;
    case ERating::A: return 0.02f;
    case ERating::BPlus: return 0.035f;
    case ERating::B: return 0.05f;
    case ERating::C: return 0.08f;
    default: return 0.12f;
    }
}

MarketBanking::FPicture MarketBanking::Picture(const FMarketState& State)
{
    FPicture P;
    P.Debt = Debt(State) + MarketFinance::Debt(State);
    const int32 Closed = FMath::Max(1, State.Day - 1);
    const MarketCalendar::FDate Now = MarketCalendar::DateOf(Closed);
    const int32 Available = MarketBankingLocal::MonthsOf(State);
    P.Months = Available;
    int64 Revenue = 0, Net = 0, Interest = 0, Tax = 0;
    if (Available > 0)
    {
        int32 Year = Now.Year, Month = Now.Month;
        const int32 Count = FMath::Min(12, Available);
        for (int32 K = 0; K < Count; ++K)
        {
            if (--Month < 1) { Month = 12; --Year; }
            const MarketLedger::FStatement S = MarketLedger::MonthStatement(State, Year, Month);
            Revenue += S.Revenue; Net += S.NetProfit;
            Interest -= S.At(MarketLedger::EAccount::Interest);
            Tax -= S.At(MarketLedger::EAccount::Tax);
        }
        const double Scale = 12.0 / Count;
        P.Revenue = FMath::RoundToInt64(Revenue * Scale);
        P.Ebitda = FMath::RoundToInt64((Net + Interest + Tax) * Scale);
        P.Interest = FMath::RoundToInt64(Interest * Scale);
    }
    else
    {
        // The first month: the days kept in the books, scaled to a year.
        const MarketLedger::FStatement S = MarketLedger::Statement(State, 1, Closed);
        const double Scale = 365.0 / FMath::Max(1, Closed);
        P.Revenue = FMath::RoundToInt64(S.Revenue * Scale);
        P.Interest = FMath::RoundToInt64(-S.At(MarketLedger::EAccount::Interest) * Scale);
        P.Ebitda = FMath::RoundToInt64((S.NetProfit - S.At(MarketLedger::EAccount::Interest) - S.At(MarketLedger::EAccount::Tax)) * Scale);
    }
    P.Leverage = P.Debt <= 0 ? 0.f : P.Ebitda > 0 ? static_cast<float>(static_cast<double>(P.Debt) / P.Ebitda) : 99.f;
    P.Cover = P.Interest > 0 ? static_cast<float>(static_cast<double>(FMath::Max<int64>(0, P.Ebitda)) / P.Interest) : 99.f;
    return P;
}

MarketBanking::ERating MarketBanking::Rating(const FMarketState& State)
{
    if (State.Day < State.RescueUntil) return ERating::D; // C7: under the bank's rescue plan
    if (State.Banking.LastRatingDay == 0) return MarketBankingLocal::FromScore(MarketBankingLocal::Score(State, nullptr));
    return static_cast<ERating>(FMath::Clamp<int32>(State.Banking.Rating, 0, static_cast<int32>(ERating::Count) - 1));
}

TArray<FString> MarketBanking::RatingLines(const FMarketState& State)
{
    TArray<FString> Lines;
    const int32 Total = MarketBankingLocal::Score(State, &Lines);
    Lines.Add(FString::Printf(TEXT("Toplam %d \u2192 %s (A+ 6, A 4, B+ 2, B 0, C -2)"), Total, *RatingName(MarketBankingLocal::FromScore(Total))));
    return Lines;
}

bool MarketBanking::IsOpen(const FMarketState& State)
{
    return MarketBranches::OpenCount(State) >= MinCompanyStores || State.Banking.Loans.Num() > 0;
}

int64 MarketBanking::Debt(const FMarketState& State)
{
    int64 Total = State.Banking.LineDrawn;
    for (const FMarketCorpLoan& L : State.Banking.Loans) Total += BalanceHome(State, L);
    return Total;
}

bool MarketBanking::CovenantBroken(const FMarketState& State)
{
    return State.Banking.BreachMonths > 0;
}

double MarketBanking::YearRate(const FMarketState& State, int32 BankIndex)
{
    return FMath::Max(0.01, MarketPrices::LoanRate(FMath::Max(1, State.Day)) + Bank(State, BankIndex).Spread + RatingSpread(Rating(State))
        + (CovenantBroken(State) ? BreachPenalty : 0.f) - MarketStrategy::RateDiscount(State)); // D9 (M50): the efficient path
}

int64 MarketBanking::InstallmentOf(int64 Principal, double Rate, int32 Months, bool bBond)
{
    if (bBond) return FMath::RoundToInt64(Principal * MarketBankingLocal::MonthlyRate(Rate));
    return MarketFinance::Installment(Principal, MarketBankingLocal::MonthlyRate(Rate), FMath::Max(1, Months));
}

int64 MarketBanking::Offer(const FMarketState& State, int32 BankIndex)
{
    FString Why;
    if (!CanBorrow(State, BankIndex, Why)) return 0;
    const FPicture P = Picture(State);
    // Room: three years of operating result less every bank debt; a young company with little result borrows
    // against its revenue (a sixth of a year's).
    const int64 Room = FMath::Max<int64>(FMath::Max<int64>(0, P.Ebitda) * 3, P.Revenue / 6) - P.Debt;
    const int64 Cap = FMath::RoundToInt64(MarketBankingLocal::BankCap(BankIndex) * MarketBankingLocal::Level(State));
    const int64 Amount = FMath::Min<int64>(FMath::RoundToInt64(FMath::Max<int64>(0, Room) * Bank(State, BankIndex).SizeFactor), Cap);
    return Amount / 10000 * 10000; // round to 100 lira
}

bool MarketBanking::CanBorrow(const FMarketState& State, int32 BankIndex, FString& OutReason)
{
    if (BankIndex < 0 || BankIndex > BondBank) { OutReason = TEXT("B\u00f6yle bir banka yok."); return false; }
    if (!IsOpen(State)) { OutReason = TEXT("\u015eirket kredisi ilk \u015fubeden sonra a\u00e7\u0131l\u0131r."); return false; }
    if (State.Day < State.RescueUntil) { OutReason = FString::Printf(TEXT("Kurtarma plan\u0131 s\u00fcr\u00fcyor: %d g\u00fcn daha kredi yok."), State.RescueUntil - State.Day); return false; } // C7
    const FBank B = Bank(State, BankIndex);
    const ERating R = Rating(State);
    if (R == ERating::D) { OutReason = TEXT("Kredi notun D: bankalar kap\u0131y\u0131 kapatt\u0131."); return false; }
    if (static_cast<int32>(R) < static_cast<int32>(B.MinRating))
    {
        OutReason = FString::Printf(TEXT("%s en az %s notu ister (senin notun %s)."), *B.Name, *RatingName(B.MinRating), *RatingName(R));
        return false;
    }
    if (CovenantBroken(State)) { OutReason = TEXT("Bor\u00e7 s\u0131n\u0131r\u0131 a\u015f\u0131ld\u0131: borcu azaltmadan yeni kredi yok."); return false; }
    if (B.bInvestmentOnly && State.Banking.DevelopmentYear == MarketCalendar::DateOf(FMath::Max(1, State.Day)).Year)
    {
        OutReason = FString::Printf(TEXT("%s y\u0131lda bir yat\u0131r\u0131m kredisi verir; bu y\u0131l\u0131n kotas\u0131 doldu."), *B.Name);
        return false;
    }
    if (BankIndex == BondBank && Picture(State).Revenue / 100.0 / FMath::Max(0.01, MarketBankingLocal::Level(State)) < 5.0e6)
    {
        OutReason = TEXT("Tahvil i\u00e7in \u015firket daha b\u00fcy\u00fck olmal\u0131 (y\u0131ll\u0131k ciro 5 milyonu ge\u00e7meli, ba\u015flang\u0131\u00e7 fiyatlar\u0131yla).");
        return false;
    }
    return true;
}

int32 MarketBanking::EncodeLoan(int32 BankIndex, int32 Step, int32 Tenor, bool bGrace)
{
    return BankIndex * 1000 + Step * 100 + Tenor * 10 + (bGrace ? 1 : 0);
}

bool MarketBanking::DecodeLoan(int32 Arg, int32& OutBank, int32& OutStep, int32& OutTenor, bool& bOutGrace)
{
    if (Arg < 0) return false;
    OutBank = Arg / 1000;
    OutStep = (Arg / 100) % 10;
    OutTenor = (Arg / 10) % 10;
    bOutGrace = Arg % 10 == 1;
    return OutBank <= BondBank && OutStep <= 3 && OutTenor < TenorCount && Arg % 10 <= 1;
}

bool MarketBanking::Borrow(FMarketState& State, int32 BankIndex, int32 Step, int32 Tenor, bool bGrace, FString& OutMessage)
{
    if (!CanBorrow(State, BankIndex, OutMessage)) return false;
    const int64 Max = Offer(State, BankIndex);
    const int64 Amount = Max * FMath::Clamp(Step + 1, 1, 4) / 4 / 10000 * 10000;
    if (Amount <= 0) { OutMessage = TEXT("Banka \u015fu an kredi vermiyor: bor\u00e7 \u015firketin kazanc\u0131na g\u00f6re zaten y\u00fcksek."); return false; }
    const FBank B = Bank(State, BankIndex);
    const bool bBond = BankIndex == BondBank;
    const int32 Months = bBond ? 60 : FMath::Min(Tenors[FMath::Clamp(Tenor, 0, TenorCount - 1)], B.MaxTenor);
    const int32 Grace = bGrace && !bBond ? GraceMonths : 0;
    const double Rate = YearRate(State, BankIndex);
    const int32 Index = MarketBankingLocal::AddLoan(State, BankIndex, bBond ? EKind::Bond : EKind::Investment, Amount, Rate, Months, Grace);
    if (B.bInvestmentOnly) State.Banking.DevelopmentYear = MarketCalendar::DateOf(FMath::Max(1, State.Day)).Year;
    State.Cash += Amount;
    MarketBankingLocal::Book(State, MarketLedger::EAccount::LoanIn, Amount);
    const FMarketCorpLoan& L = State.Banking.Loans[Index];
    OutMessage = bBond
        ? FString::Printf(TEXT("Tahvil sat\u0131ld\u0131: %s kasada. Y\u0131ll\u0131k faiz %s, her ay %s faiz; ana para 60 ay sonra bir kerede."), *MarketCountry::Money(Amount), *MarketBankingLocal::Percent(Rate), *MarketCountry::Money(L.Installment))
        : FString::Printf(TEXT("%s: %s yat\u0131r\u0131m kredisi kasada. Y\u0131ll\u0131k faiz %s, %d ay%s, ayl\u0131k taksit %s."), *B.Name, *MarketCountry::Money(Amount), *MarketBankingLocal::Percent(Rate), Months,
            Grace > 0 ? *FString::Printf(TEXT(" (ilk %d ay yaln\u0131z faiz)"), Grace) : TEXT(""), *MarketCountry::Money(L.Installment));
    return true;
}

bool MarketBanking::Repay(FMarketState& State, int32 LoanIndex, FString& OutMessage)
{
    if (!State.Banking.Loans.IsValidIndex(LoanIndex)) { OutMessage = TEXT("B\u00f6yle bir kredi yok."); return false; }
    const FMarketCorpLoan L = State.Banking.Loans[LoanIndex];
    const int64 Owed = BalanceHome(State, L); // E4b: abroad at the day's rate
    const int64 Fee = FMath::RoundToInt64(Owed * EarlyRepayFee);
    if (State.Cash < Owed + Fee) { OutMessage = FString::Printf(TEXT("Kapatmak i\u00e7in kasada %s gerekir."), *MarketCountry::Money(Owed + Fee)); return false; }
    State.Cash -= Owed + Fee;
    MarketBankingLocal::Book(State, MarketLedger::EAccount::LoanRepayment, -Owed);
    MarketBankingLocal::Book(State, MarketLedger::EAccount::BankFees, -Fee);
    State.Banking.Loans.RemoveAt(LoanIndex);
    OutMessage = FString::Printf(TEXT("%s kredisi kapand\u0131: %s (erken kapama %s)."), *BankNameIn(State, L.Country, L.Bank), *MarketCountry::Money(Owed), *MarketCountry::Money(Fee));
    return true;
}

int64 MarketBanking::AcquisitionRoom(const FMarketState& State, int32 BankIndex, int64 TargetEbitda)
{
    FString Why;
    if (BankIndex == 3 || BankIndex >= BankCount || !CanBorrow(State, BankIndex, Why)) return 0; // the development bank funds only new stores
    const FPicture P = Picture(State);
    const int64 Room = (FMath::Max<int64>(0, P.Ebitda) + FMath::Max<int64>(0, TargetEbitda)) * 3 - P.Debt;
    const int64 Cap = FMath::RoundToInt64(MarketBankingLocal::BankCap(BankIndex) * 2 * MarketBankingLocal::Level(State));
    return FMath::Min<int64>(FMath::RoundToInt64(FMath::Max<int64>(0, Room) * Bank(State, BankIndex).SizeFactor), Cap) / 10000 * 10000;
}

bool MarketBanking::FinanceAcquisition(FMarketState& State, int64 Amount, int64 TargetEbitda, FString& OutMessage)
{
    if (Amount <= 0) { OutMessage.Reset(); return true; }
    const int64 Need = (Amount + 9999) / 10000 * 10000;
    int32 Best = INDEX_NONE;
    double BestRate = 0.0;
    for (int32 I = 0; I < BankCount; ++I)
    {
        if (AcquisitionRoom(State, I, TargetEbitda) < Need) continue;
        const double Rate = YearRate(State, I);
        if (Best == INDEX_NONE || Rate < BestRate) { Best = I; BestRate = Rate; }
    }
    if (Best == INDEX_NONE)
    {
        OutMessage = FString::Printf(TEXT("Hi\u00e7bir banka %s sat\u0131n alma kredisi vermiyor: notun %s, bor\u00e7 \u015firketin ve hedefin kazanc\u0131na g\u00f6re fazla."), *MarketCountry::Money(Need), *RatingName(Rating(State)));
        return false;
    }
    const int32 Months = Bank(State, Best).MaxTenor;
    const int32 Index = MarketBankingLocal::AddLoan(State, Best, EKind::Acquisition, Need, BestRate, Months, 0);
    State.Cash += Need;
    MarketBankingLocal::Book(State, MarketLedger::EAccount::LoanIn, Need);
    OutMessage = FString::Printf(TEXT("%s sat\u0131n alma kredisi: %s, y\u0131ll\u0131k %s, %d ay, ayl\u0131k %s."), *Bank(State, Best).Name, *MarketCountry::Money(Need),
        *MarketBankingLocal::Percent(BestRate), Months, *MarketCountry::Money(State.Banking.Loans[Index].Installment));
    return true;
}

bool MarketBanking::Restructure(FMarketState& State, int32 BankIndex, FString& OutMessage)
{
    FMarketBankingState& B = State.Banking;
    if (BankIndex < 0 || BankIndex >= BankCount || Bank(State, BankIndex).bInvestmentOnly) { OutMessage = TEXT("Bu banka yap\u0131land\u0131rma yapmaz."); return false; }
    int64 Total = 0;
    for (const FMarketCorpLoan& L : B.Loans) Total += BalanceHome(State, L); // E4b: loans abroad join at the day's rate
    if (B.Loans.Num() < 1 || Total <= 0) { OutMessage = TEXT("Yap\u0131land\u0131r\u0131lacak kredi yok."); return false; }
    if (Rating(State) == ERating::D && BankIndex != 0) { OutMessage = TEXT("Notun D iken yaln\u0131z yerel banka konu\u015fur."); return false; }
    const int64 Fee = FMath::RoundToInt64(Total * RestructureFee);
    // Every loan into one: the longest tenor the bank gives, its rate today; the fee joins the debt.
    const double Rate = YearRate(State, BankIndex); // a broken covenant's penalty stays
    B.Loans.Reset();
    MarketBankingLocal::AddLoan(State, BankIndex, EKind::Restructured, Total + Fee, Rate, Bank(State, BankIndex).MaxTenor, 0);
    MarketBankingLocal::Book(State, MarketLedger::EAccount::BankFees, -Fee, false);
    B.RestructuredUntil = State.Day + 180;
    OutMessage = FString::Printf(TEXT("Bor\u00e7lar tek krediye topland\u0131 (%s, %d ay, y\u0131ll\u0131k %s). Masraf %s. Not alt\u0131 ay bir basamak a\u015fa\u011f\u0131da kal\u0131r."),
        *Bank(State, BankIndex).Name, Bank(State, BankIndex).MaxTenor, *MarketBankingLocal::Percent(Rate), *MarketCountry::Money(Fee));
    return true;
}

int64 MarketBanking::LineLimitFor(const FMarketState& State)
{
    if (!IsOpen(State) || State.Day < State.RescueUntil || static_cast<int32>(Rating(State)) < static_cast<int32>(ERating::B)) return 0;
    const int64 Limit = Picture(State).Revenue * 15 / 100;
    return FMath::Max<int64>(0, Limit) / 10000 * 10000;
}

bool MarketBanking::OpenLine(FMarketState& State, FString& OutMessage)
{
    FMarketBankingState& B = State.Banking;
    if (B.bLine) { OutMessage = TEXT("Kredi limiti zaten a\u00e7\u0131k."); return false; }
    const int64 Limit = LineLimitFor(State);
    if (Limit <= 0) { OutMessage = TEXT("Kredi limiti i\u00e7in en az B notu ve bir \u015fube gerekir."); return false; }
    B.bLine = true;
    B.LineLimit = Limit;
    B.LineDueDay = State.Day + MonthDays;
    OutMessage = FString::Printf(TEXT("%s kredi limiti a\u00e7t\u0131: %s. Kasa eksiye d\u00fc\u015ferse kendili\u011finden kapat\u0131r; kullan\u0131lan k\u0131sma ayl\u0131k faiz, kalan\u0131na k\u00fc\u00e7\u00fck bir \u00fccret."),
        *Bank(State, 1).Name, *MarketCountry::Money(Limit));
    return true;
}

bool MarketBanking::SetLineAuto(FMarketState& State, bool bAuto, FString& OutMessage)
{
    if (!State.Banking.bLine) { OutMessage = TEXT("Kredi limiti a\u00e7\u0131k de\u011fil."); return false; }
    State.Banking.bLineAuto = bAuto;
    OutMessage = bAuto ? TEXT("Kredi limiti eksi kasay\u0131 kendili\u011finden kapatacak.") : TEXT("Kredi limiti kendili\u011finden kullan\u0131lmayacak.");
    return true;
}

bool MarketBanking::RepayLine(FMarketState& State, FString& OutMessage)
{
    FMarketBankingState& B = State.Banking;
    const int64 Pay = FMath::Min<int64>(B.LineDrawn, FMath::Max<int64>(0, State.Cash));
    if (Pay <= 0) { OutMessage = B.LineDrawn > 0 ? TEXT("Kasada para yok.") : TEXT("Limitten kullan\u0131lan para yok."); return false; }
    B.LineDrawn -= Pay;
    State.Cash -= Pay;
    MarketBankingLocal::Book(State, MarketLedger::EAccount::LoanRepayment, -Pay);
    OutMessage = FString::Printf(TEXT("Kredi limitine %s \u00f6dendi; kalan %s."), *MarketCountry::Money(Pay), *MarketCountry::Money(B.LineDrawn));
    return true;
}

FString MarketBanking::Describe(const FMarketState& State, int32 LoanIndex)
{
    if (!State.Banking.Loans.IsValidIndex(LoanIndex)) return FString();
    const FMarketCorpLoan& L = State.Banking.Loans[LoanIndex];
    const TCHAR* What = L.Kind == static_cast<uint8>(EKind::Bond) ? TEXT("tahvil") : L.Kind == static_cast<uint8>(EKind::Restructured) ? TEXT("yap\u0131land\u0131r\u0131lm\u0131\u015f")
        : L.Kind == static_cast<uint8>(EKind::Called) ? TEXT("geri \u00e7a\u011fr\u0131lan") : L.Kind == static_cast<uint8>(EKind::Acquisition) ? TEXT("sat\u0131n alma") : TEXT("yat\u0131r\u0131m");
    FString Line = FString::Printf(TEXT("%s \u00b7 %s \u00b7 kalan %s \u00b7 y\u0131ll\u0131k %s \u00b7 %d/%d ay"), *BankNameIn(State, L.Country, L.Bank), What, *MarketCountry::Money(BalanceHome(State, L)),
        *MarketBankingLocal::Percent(L.YearRate), L.PaidMonths, L.Months);
    if (!L.Country.IsEmpty() && L.Country != State.CountryId) Line += FString::Printf(TEXT(" \u00b7 %s paras\u0131yla, g\u00fcn\u00fcn kuruyla"), *MarketCountry::FindOrDefault(L.Country).Name);
    if (L.PaidMonths < L.Grace) Line += FString::Printf(TEXT(" \u00b7 %d ay daha yaln\u0131z faiz"), L.Grace - L.PaidMonths);
    if (L.LateSince > 0) Line += TEXT(" \u00b7 GEC\u0130KMEDE");
    return Line;
}

int64 MarketBanking::DueSoon(const FMarketState& State)
{
    int64 Due = 0;
    for (const FMarketCorpLoan& L : State.Banking.Loans)
    {
        if (L.NextDueDay > State.Day + MonthDays) continue;
        int64 Interest = 0, Principal = 0;
        MarketBankingLocal::MonthDue(L, Interest, Principal);
        Due += Interest + Principal;
    }
    if (State.Banking.bLine) Due += FMath::RoundToInt64(State.Banking.LineDrawn * MarketBankingLocal::MonthlyRate(YearRate(State, 1) + 0.01));
    return Due;
}

FString MarketBanking::Summary(const FMarketState& State)
{
    if (!IsOpen(State)) return TEXT("\u015eirket kredisi ilk \u015fubeden sonra a\u00e7\u0131l\u0131r.");
    const FPicture P = Picture(State);
    FString Text = FString::Printf(TEXT("Kredi notu %s \u00b7 banka borcu %s \u00b7 y\u0131ll\u0131k FAV\u00d6K %s"), *RatingName(Rating(State)), *MarketCountry::Money(Debt(State)), *MarketCountry::Money(P.Ebitda));
    if (P.Debt > 0 && P.Ebitda > 0) Text += FString::Printf(TEXT(" \u00b7 bor\u00e7/FAV\u00d6K %.1f (s\u0131n\u0131r %.1f)"), P.Leverage, CovenantLeverage);
    if (State.Banking.bLine) Text += FString::Printf(TEXT(" \u00b7 limit %s, kullan\u0131lan %s"), *MarketCountry::Money(State.Banking.LineLimit), *MarketCountry::Money(State.Banking.LineDrawn));
    if (CovenantBroken(State)) Text += TEXT(" \u00b7 BOR\u00c7 SINIRI A\u015eILDI");
    return Text;
}

int64 MarketBanking::BalanceHome(const FMarketState& State, const FMarketCorpLoan& Loan)
{
    return MarketBankingLocal::HomeOf(State, Loan, Loan.Balance);
}

FString MarketBanking::BankNameIn(const FMarketState& State, const FString& Country, int32 BankIndex)
{
    if (Country.IsEmpty() || Country == State.CountryId) return Bank(State, BankIndex).Name;
    const MarketCountry::FProfile& Pack = MarketCountry::FindOrDefault(Country);
    return Pack.Banks.IsValidIndex(BankIndex) ? Pack.Banks[BankIndex] : Bank(State, BankIndex).Name;
}

bool MarketBanking::CanBorrowIn(const FMarketState& State, const FString& Country, int32 BankIndex, FString& OutReason)
{
    if (Country.IsEmpty() || Country == State.CountryId) return CanBorrow(State, BankIndex, OutReason);
    if (BankIndex < 0 || BankIndex > 2) { OutReason = TEXT("Yurt d\u0131\u015f\u0131nda yerel, ticari ve yat\u0131r\u0131m bankas\u0131 kredi verir."); return false; }
    const bool bCompany = State.Company.Subsidiaries.ContainsByPredicate([&Country](const FMarketSubsidiary& S) { return S.Country == Country; });
    if (!bCompany) { OutReason = TEXT("O \u00fclkede \u015firketimiz yok: \u00f6nce bir ma\u011faza a\u00e7."); return false; }
    return CanBorrow(State, BankIndex, OutReason);
}

int64 MarketBanking::OfferIn(const FMarketState& State, const FString& Country, int32 BankIndex)
{
    FString Why;
    if (!CanBorrowIn(State, Country, BankIndex, Why)) return 0;
    if (Country.IsEmpty() || Country == State.CountryId) return Offer(State, BankIndex);
    int32 There = 0, All = 1; // the first store counts at home
    for (const FMarketBranch& B : State.Branches)
    {
        if (B.Stage != static_cast<uint8>(MarketBranches::EStage::Open)) continue;
        ++All;
        if (MarketBranches::CountryOf(State, B) == Country) ++There;
    }
    const double Share = FMath::Clamp(static_cast<double>(There) / All, 0.25, 1.0);
    return FMath::RoundToInt64(Offer(State, BankIndex) * Share) / 10000 * 10000;
}

double MarketBanking::YearRateIn(const FMarketState& State, const FString& Country, int32 BankIndex)
{
    if (Country.IsEmpty() || Country == State.CountryId) return YearRate(State, BankIndex);
    return FMath::Max(0.01, MarketPrices::LoanRate(Country, FMath::Max(1, State.Day)) + Bank(State, BankIndex).Spread + RatingSpread(Rating(State))
        + (CovenantBroken(State) ? BreachPenalty : 0.f) - MarketStrategy::RateDiscount(State)); // D9 (M50): the efficient path
}

bool MarketBanking::BorrowIn(FMarketState& State, const FString& Country, int32 BankIndex, int32 Step, int32 Tenor, bool bGrace, FString& OutMessage)
{
    if (Country.IsEmpty() || Country == State.CountryId) return Borrow(State, BankIndex, Step, Tenor, bGrace, OutMessage);
    if (!CanBorrowIn(State, Country, BankIndex, OutMessage)) return false;
    const int64 Amount = OfferIn(State, Country, BankIndex) * FMath::Clamp(Step + 1, 1, 4) / 4 / 10000 * 10000;
    if (Amount <= 0) { OutMessage = TEXT("Banka \u015fu an kredi vermiyor: bor\u00e7 \u015firketin kazanc\u0131na g\u00f6re zaten y\u00fcksek."); return false; }
    const double Fx = MarketPrices::ToHome(State, Country, FMath::Max(1, State.Day));
    const int64 Local = FMath::RoundToInt64(static_cast<double>(Amount) / FMath::Max(1e-9, Fx));
    const int32 Months = FMath::Min(Tenors[FMath::Clamp(Tenor, 0, TenorCount - 1)], Bank(State, BankIndex).MaxTenor);
    const int32 Grace = bGrace ? GraceMonths : 0;
    const double Rate = YearRateIn(State, Country, BankIndex);
    const int32 Index = MarketBankingLocal::AddLoan(State, BankIndex, EKind::Investment, Local, Rate, Months, Grace);
    State.Banking.Loans[Index].Country = Country;
    State.Cash += Amount;
    MarketBankingLocal::Book(State, MarketLedger::EAccount::LoanIn, Amount);
    const FMarketCorpLoan& L = State.Banking.Loans[Index];
    OutMessage = FString::Printf(TEXT("%s: %s kredi kasada (%s paras\u0131yla). Y\u0131ll\u0131k faiz %s, %d ay%s, ayl\u0131k taksit bug\u00fcn\u00fcn kuruyla %s; kur de\u011fi\u015ftik\u00e7e taksit de de\u011fi\u015fir."),
        *BankNameIn(State, Country, BankIndex), *MarketCountry::Money(Amount), *MarketCountry::FindOrDefault(Country).Name, *MarketBankingLocal::Percent(Rate), Months,
        Grace > 0 ? *FString::Printf(TEXT(" (ilk %d ay yaln\u0131z faiz)"), Grace) : TEXT(""), *MarketCountry::Money(MarketBankingLocal::HomeOf(State, L, L.Installment)));
    return true;
}

void MarketBanking::CloseDay(FMarketState& State)
{
    using namespace MarketBankingLocal;
    FMarketBankingState& B = State.Banking;
    const int32 Closed = State.Day - 1;
    if (Closed < 1) return;
    TArray<FString>& News = State.DayNews;

    // Installments due.
    for (int32 I = 0; I < B.Loans.Num(); ++I)
    {
        FMarketCorpLoan& L = B.Loans[I];
        if (Closed < L.NextDueDay) continue;
        int64 Interest = 0, Principal = 0;
        MonthDue(L, Interest, Principal); // local units abroad (E4b)
        const int64 DueLocal = Interest + Principal;
        const int64 Due = HomeOf(State, L, DueLocal);
        if (State.Cash + (B.bLineAuto ? LineRoom(State) : 0) < Due)
        {
            // Missed: a late fee joins the debt (C7: once, then once a month while it stays unpaid), the next try in
            // a week, the rating remembers.
            const bool bFee = L.LateSince == 0 || (Closed - L.LateSince) % MonthDays < 7;
            if (L.LateSince == 0) { L.LateSince = Closed; B.LateDays.Add(Closed); }
            L.NextDueDay = Closed + 7;
            if (!bFee) continue;
            const int64 Fee = FMath::RoundToInt64(DueLocal * LateFee);
            L.Balance += Fee;
            Book(State, EAccount::Penalties, -HomeOf(State, L, Fee), false);
            News.Add(FString::Printf(TEXT("%s: %s taksit \u00f6denemedi, %s gecikme fark\u0131 borca eklendi. Kredi notu bunu hat\u0131rlar."), *BankNameIn(State, L.Country, L.Bank), *MarketCountry::Money(Due), *MarketCountry::Money(HomeOf(State, L, Fee))));
            continue;
        }
        const int64 InterestHome = HomeOf(State, L, Interest);
        State.Cash -= Due;
        L.Balance -= Principal;
        B.InterestPaid += InterestHome;
        Book(State, EAccount::Interest, -InterestHome);
        Book(State, EAccount::LoanRepayment, -(Due - InterestHome));
        ++L.PaidMonths;
        L.LateSince = 0;
        L.NextDueDay = L.StartDay + (L.PaidMonths + 1) * MonthDays;
        if (L.NextDueDay <= Closed) L.NextDueDay = Closed + MonthDays;
    }
    for (int32 I = B.Loans.Num() - 1; I >= 0; --I)
    {
        if (B.Loans[I].Balance > 0) continue;
        News.Add(FString::Printf(TEXT("%s kredisi bitti: son taksit \u00f6dendi."), *BankNameIn(State, B.Loans[I].Country, B.Loans[I].Bank)));
        B.Loans.RemoveAt(I);
    }

    // The credit line: its monthly interest and fee; it covers a negative till, and pays itself back.
    if (B.bLine)
    {
        if (Closed >= B.LineDueDay)
        {
            const int64 Interest = FMath::RoundToInt64(B.LineDrawn * MonthlyRate(YearRate(State, 1) + 0.01));
            const int64 Fee = FMath::RoundToInt64(FMath::Max<int64>(0, B.LineLimit - B.LineDrawn) * MonthlyRate(0.005));
            State.Cash -= Interest + Fee;
            B.InterestPaid += Interest;
            Book(State, EAccount::Interest, -Interest);
            Book(State, EAccount::BankFees, -Fee);
            B.LineDueDay = Closed + MonthDays;
        }
        if (State.Cash < 0 && B.bLineAuto)
        {
            const int64 Cover = FMath::Min<int64>(-State.Cash, LineRoom(State));
            Draw(State, Cover);
            if (Cover > 0 && B.LineDrawn > B.LineLimit * 3 / 4) News.Add(FString::Printf(TEXT("Kredi limitinin %s kadar\u0131 kullan\u0131l\u0131yor (limit %s)."), *MarketCountry::Money(B.LineDrawn), *MarketCountry::Money(B.LineLimit)));
        }
        else if (B.LineDrawn > 0 && B.bLineAuto)
        {
            // Pay back what a comfortable till allows (a tenth of the limit stays as a cushion).
            const int64 Pay = FMath::Min<int64>(B.LineDrawn, State.Cash - B.LineLimit / 10);
            if (Pay > 0) { B.LineDrawn -= Pay; State.Cash -= Pay; Book(State, EAccount::LoanRepayment, -Pay); }
        }
    }

    // Monthly: the rating, the line's limit, covenants.
    if (Closed - B.LastRatingDay < MonthDays) return;
    const bool bFirst = B.LastRatingDay == 0;
    B.LastRatingDay = Closed;
    if (!IsOpen(State)) return;
    const ERating Before = static_cast<ERating>(FMath::Clamp<int32>(B.Rating, 0, static_cast<int32>(ERating::Count) - 1));
    const ERating Now = FromScore(Score(State, nullptr));
    B.Rating = static_cast<uint8>(Now);
    if (!bFirst && Now != Before)
        News.Add(FString::Printf(TEXT("Kredi notun %s oldu (\u00f6nce %s). %s"), *RatingName(Now), *RatingName(Before),
            static_cast<int32>(Now) > static_cast<int32>(Before) ? TEXT("Bankalar daha ucuz ve daha \u00e7ok kredi verir.") : TEXT("Krediler pahalan\u0131r; Finans sayfas\u0131nda nedenleri var.")));
    if (B.bLine)
    {
        B.LineLimit = LineLimitFor(State);
        if (B.LineLimit < B.LineDrawn) News.Add(TEXT("Banka kredi limitini d\u00fc\u015f\u00fcrd\u00fc: kullan\u0131lan k\u0131s\u0131m limitin \u00fcst\u00fcnde, yeni \u00e7ekim yok."));
    }
    const FPicture P = Picture(State);
    const bool bBreach = Debt(State) > 0 && P.Debt > 0 && (P.Ebitda <= 0 || P.Leverage > CovenantLeverage);
    if (!bBreach) { if (B.BreachMonths > 0) News.Add(TEXT("Bor\u00e7 s\u0131n\u0131r\u0131 yeniden tutuyor: bankalar kap\u0131y\u0131 a\u00e7t\u0131.")); B.BreachMonths = 0; return; }
    ++B.BreachMonths;
    if (B.BreachMonths == 1)
    {
        for (FMarketCorpLoan& L : B.Loans) L.YearRate += BreachPenalty;
        News.Add(FString::Printf(TEXT("Bor\u00e7 s\u0131n\u0131r\u0131 a\u015f\u0131ld\u0131 (bor\u00e7 / FAV\u00d6K %.1f, s\u0131n\u0131r %.1f): bankalar faizi 2 puan art\u0131rd\u0131, yeni kredi yok. \u00dc\u00e7 ay s\u00fcrerse en b\u00fcy\u00fck banka borcun bir k\u0131sm\u0131n\u0131 geri ister."),
            FMath::Min(P.Leverage, 99.f), CovenantLeverage));
    }
    if (B.BreachMonths == CallAfterMonths)
    {
        int32 Biggest = INDEX_NONE;
        for (int32 I = 0; I < B.Loans.Num(); ++I)
            if (B.Loans[I].Kind != static_cast<uint8>(EKind::Called) && (Biggest == INDEX_NONE || BalanceHome(State, B.Loans[I]) > BalanceHome(State, B.Loans[Biggest]))) Biggest = I;
        if (Biggest != INDEX_NONE)
        {
            const int64 Part = B.Loans[Biggest].Balance / 4;
            B.Loans[Biggest].Balance -= Part;
            const int32 BankIndex = B.Loans[Biggest].Bank;
            const double Rate = B.Loans[Biggest].YearRate;
            const FString LoanCountry = B.Loans[Biggest].Country;
            const int32 Called = AddLoan(State, BankIndex, EKind::Called, Part, Rate, 1, 0);
            B.Loans[Called].Country = LoanCountry; // E4b: still owed in that money
            News.Add(FString::Printf(TEXT("%s borcunun d\u00f6rtte birini (%s) 30 g\u00fcn i\u00e7inde geri istiyor."), *BankNameIn(State, LoanCountry, BankIndex), *MarketCountry::Money(BalanceHome(State, B.Loans[Called]))));
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------
// C13 (M43): applications

namespace MarketBankingAppsLocal
{
    uint32 Mix(uint32 A, uint32 B, uint32 C)
    {
        uint32 H = A * 0x9E3779B1u ^ (B + 0x7F4A7C15u) * 0x85EBCA77u ^ (C + 0x165667B1u) * 0xC2B2AE3Du;
        H ^= H >> 15; H *= 0x2C1B3C6Du; H ^= H >> 12; H *= 0x297A2D39u; H ^= H >> 15;
        return H;
    }
    FString Money(int64 Kurus) { return MarketCountry::Money(Kurus); }
    bool IsOpenStatus(uint8 Status)
    {
        return Status == static_cast<uint8>(MarketBanking::EAppStatus::Pending) || Status == static_cast<uint8>(MarketBanking::EAppStatus::Offered);
    }
    int32 MonthsFor(const FMarketState& State, int32 BankIndex, int32 Tenor)
    {
        if (BankIndex == MarketBanking::BondBank) return 60;
        return FMath::Min(MarketBanking::Tenors[FMath::Clamp(Tenor, 0, MarketBanking::TenorCount - 1)], MarketBanking::Bank(State, BankIndex).MaxTenor);
    }
    bool HasOpen(const FMarketState& State, int32 BankIndex)
    {
        return State.Banking.Apps.ContainsByPredicate([BankIndex](const FMarketLoanApp& A) { return A.Bank == BankIndex && IsOpenStatus(A.Status); });
    }
    int32 AddApp(FMarketState& State, int32 BankIndex, MarketBanking::EPurpose Purpose, int32 Chain, int64 Asked, int32 Tenor, bool bGrace)
    {
        FMarketBankingState& B = State.Banking;
        FMarketLoanApp App;
        App.Id = B.NextAppId++;
        App.Bank = static_cast<uint8>(BankIndex);
        App.Purpose = static_cast<uint8>(Purpose);
        App.Chain = Chain;
        App.Asked = Asked;
        App.Tenor = Tenor;
        App.bGrace = bGrace && BankIndex != MarketBanking::BondBank;
        App.AppliedDay = State.Day;
        // 2-4 days (the bond: a week of paperwork).
        App.AnswerDay = State.Day + (BankIndex == MarketBanking::BondBank ? 5 : 2 + static_cast<int32>(Mix(static_cast<uint32>(State.RivalSeed), static_cast<uint32>(App.Id), 0xA11Cu) % 3u));
        // Keep the list short: drop finished applications older than a month.
        B.Apps.RemoveAll([&State](const FMarketLoanApp& A) { return !IsOpenStatus(A.Status) && State.Day - A.AnswerDay > 30; });
        return B.Apps.Add(App);
    }
}

bool MarketBanking::DecodeApp(int32 Arg, int32& OutBank, int32& OutStep, int32& OutTenor, bool& bOutGrace)
{
    if (Arg < 0) return false;
    OutBank = Arg / 1000;
    OutStep = (Arg / 100) % 10;
    OutTenor = (Arg / 10) % 10;
    bOutGrace = Arg % 10 == 1;
    return OutBank <= BondBank && OutStep <= MaxAsk && OutTenor < TenorCount && Arg % 10 <= 1;
}

int64 MarketBanking::AskedFor(const FMarketState& State, int32 BankIndex, int32 Step)
{
    const int64 Max = Offer(State, BankIndex);
    const int64 Amount = Step >= MaxAsk ? Max * 3 / 2 : Max * FMath::Clamp(Step + 1, 1, 4) / 4;
    return Amount / 10000 * 10000;
}

bool MarketBanking::CanApply(const FMarketState& State, int32 BankIndex, FString& OutReason)
{
    if (!CanBorrow(State, BankIndex, OutReason)) return false;
    if (MarketBankingAppsLocal::HasOpen(State, BankIndex)) { OutReason = FString::Printf(TEXT("%s'da a\u00e7\u0131k bir ba\u015fvurun var: cevab\u0131n\u0131 bekle ya da teklifi kapat."), *Bank(State, BankIndex).Name); return false; }
    if (Offer(State, BankIndex) <= 0) { OutReason = TEXT("Banka \u015fu an kredi vermiyor: bor\u00e7 \u015firketin kazanc\u0131na g\u00f6re zaten y\u00fcksek."); return false; }
    return true;
}

bool MarketBanking::Apply(FMarketState& State, int32 BankIndex, int32 Step, int32 Tenor, bool bGrace, FString& OutMessage)
{
    if (!CanApply(State, BankIndex, OutMessage)) return false;
    const int64 Asked = AskedFor(State, BankIndex, Step);
    if (Asked <= 0) { OutMessage = TEXT("Ba\u015fvuru tutar\u0131 \u00e7ok k\u00fc\u00e7\u00fck."); return false; }
    const int32 Index = MarketBankingAppsLocal::AddApp(State, BankIndex, EPurpose::Investment, INDEX_NONE, Asked, Tenor, bGrace);
    const FMarketLoanApp& App = State.Banking.Apps[Index];
    OutMessage = FString::Printf(TEXT("%s'a %s i\u00e7in ba\u015fvuruldu (%d ay%s). Kredi komitesi %d g\u00fcn i\u00e7inde cevap verir."),
        *Bank(State, BankIndex).Name, *MarketBankingAppsLocal::Money(Asked), MarketBankingAppsLocal::MonthsFor(State, BankIndex, Tenor),
        App.bGrace ? TEXT(", ilk 6 ay yaln\u0131z faiz") : TEXT(""), App.AnswerDay - State.Day);
    return true;
}

int64 MarketBanking::AcquisitionNeed(const FMarketState& State, int64 Price)
{
    const int64 Need = Price + Price / 20 - State.Cash;
    return Need <= 0 ? 0 : (Need + 9999) / 10000 * 10000;
}

bool MarketBanking::ApplyAcquisition(FMarketState& State, int32 ChainIndex, int64 Price, FString& OutMessage)
{
    const int64 Need = AcquisitionNeed(State, Price);
    if (Need <= 0) { OutMessage = TEXT("Kasa anla\u015fmaya yetiyor: krediye gerek yok."); return false; }
    const int64 Target = MarketChains::YearProfit(State, ChainIndex);
    TArray<FString> Asked;
    for (int32 I = 0; I < BankCount; ++I)
    {
        FString Why;
        if (I == 3 || !CanBorrow(State, I, Why) || MarketBankingAppsLocal::HasOpen(State, I)) continue; // the development bank funds only new stores
        if (AcquisitionRoom(State, I, Target) <= 0) continue;
        MarketBankingAppsLocal::AddApp(State, I, EPurpose::Acquisition, ChainIndex, Need, TenorCount - 1, false);
        Asked.Add(Bank(State, I).Name);
    }
    if (Asked.Num() == 0)
    {
        OutMessage = FString::Printf(TEXT("Hi\u00e7bir banka %s sat\u0131n alma kredisini de\u011ferlendirmiyor: notun %s, bor\u00e7 \u015firketin ve hedefin kazanc\u0131na g\u00f6re fazla (ya da bankalarda a\u00e7\u0131k ba\u015fvurun var)."),
            *MarketBankingAppsLocal::Money(Need), *RatingName(Rating(State)));
        return false;
    }
    OutMessage = FString::Printf(TEXT("Kasada yetmeyen %s i\u00e7in %s'a sat\u0131n alma kredisi ba\u015fvurusu yap\u0131ld\u0131. Cevaplar birka\u00e7 g\u00fcn i\u00e7inde gelir; en iyi teklifi se\u00e7ersin."),
        *MarketBankingAppsLocal::Money(Need), *FString::Join(Asked, TEXT(", ")));
    return true;
}

int32 MarketBanking::FindApp(const FMarketState& State, int32 AppId)
{
    return State.Banking.Apps.IndexOfByPredicate([AppId](const FMarketLoanApp& A) { return A.Id == AppId; });
}

bool MarketBanking::AcceptApp(FMarketState& State, const TArray<FMarketProduct>& Products, int32 AppId, FString& OutMessage)
{
    const int32 Index = FindApp(State, AppId);
    if (Index == INDEX_NONE) { OutMessage = TEXT("B\u00f6yle bir teklif yok."); return false; }
    const FMarketLoanApp App = State.Banking.Apps[Index];
    if (App.Status != static_cast<uint8>(EAppStatus::Offered)) { OutMessage = TEXT("Bu ba\u015fvurunun a\u00e7\u0131k bir teklifi yok."); return false; }
    if (State.Day > App.ValidUntil) { OutMessage = TEXT("Teklifin s\u00fcresi doldu."); return false; }
    if (!CanBorrow(State, App.Bank, OutMessage)) return false;
    const bool bAcq = App.Purpose == static_cast<uint8>(EPurpose::Acquisition);
    int64 Price = 0;
    if (bAcq)
    {
        Price = MarketChains::DealPrice(State, App.Chain);
        if (Price <= 0) { OutMessage = TEXT("Bu sat\u0131n alma art\u0131k ge\u00e7erli de\u011fil: kredi kullan\u0131lmad\u0131."); State.Banking.Apps[Index].Status = static_cast<uint8>(EAppStatus::Declined); return false; }
    }
    const EKind Kind = bAcq ? EKind::Acquisition : App.Bank == BondBank ? EKind::Bond : EKind::Investment;
    const int32 Loan = MarketBankingLocal::AddLoan(State, App.Bank, Kind, App.Offered, App.YearRate, App.Months, App.bGrace ? GraceMonths : 0);
    if (Bank(State, App.Bank).bInvestmentOnly) State.Banking.DevelopmentYear = MarketCalendar::DateOf(FMath::Max(1, State.Day)).Year;
    State.Cash += App.Offered;
    MarketBankingLocal::Book(State, MarketLedger::EAccount::LoanIn, App.Offered);
    State.Banking.Apps[Index].Status = static_cast<uint8>(EAppStatus::Accepted);
    OutMessage = FString::Printf(TEXT("%s: %s kredi kasada. Y\u0131ll\u0131k %s, %d ay, ayl\u0131k taksit %s."), *Bank(State, App.Bank).Name, *MarketBankingAppsLocal::Money(App.Offered),
        *MarketBankingLocal::Percent(App.YearRate), App.Months, *MarketBankingAppsLocal::Money(State.Banking.Loans[Loan].Installment));
    if (bAcq)
    {
        FString Deal;
        if (State.Cash >= Price && MarketChains::CompleteDeal(State, Products, App.Chain, Deal))
        {
            OutMessage += TEXT(" ") + Deal;
            for (FMarketLoanApp& Other : State.Banking.Apps)
                if (Other.Chain == App.Chain && Other.Purpose == App.Purpose && MarketBankingAppsLocal::IsOpenStatus(Other.Status)) Other.Status = static_cast<uint8>(EAppStatus::Declined);
        }
        else OutMessage += FString::Printf(TEXT(" Anla\u015fma i\u00e7in kasada h\u00e2l\u00e2 %s eksik: ba\u015fka bir teklifi de kabul edebilirsin."), *MarketBankingAppsLocal::Money(FMath::Max<int64>(0, Price - State.Cash)));
    }
    return true;
}

bool MarketBanking::DeclineApp(FMarketState& State, int32 AppId, FString& OutMessage)
{
    const int32 Index = FindApp(State, AppId);
    if (Index == INDEX_NONE || !MarketBankingAppsLocal::IsOpenStatus(State.Banking.Apps[Index].Status)) { OutMessage = TEXT("B\u00f6yle bir a\u00e7\u0131k ba\u015fvuru yok."); return false; }
    State.Banking.Apps[Index].Status = static_cast<uint8>(EAppStatus::Declined);
    OutMessage = FString::Printf(TEXT("%s ba\u015fvurusu kapat\u0131ld\u0131."), *Bank(State, State.Banking.Apps[Index].Bank).Name);
    return true;
}

TArray<int32> MarketBanking::OpenApps(const FMarketState& State)
{
    TArray<int32> List;
    for (int32 I = State.Banking.Apps.Num() - 1; I >= 0; --I)
    {
        const FMarketLoanApp& A = State.Banking.Apps[I];
        // Open ones, and refusals of the last week (so the player reads why).
        if (MarketBankingAppsLocal::IsOpenStatus(A.Status) || (A.Status == static_cast<uint8>(EAppStatus::Refused) && State.Day - A.AnswerDay <= 7)) List.Add(I);
    }
    return List;
}

FString MarketBanking::DescribeApp(const FMarketState& State, int32 AppIndex)
{
    if (!State.Banking.Apps.IsValidIndex(AppIndex)) return FString();
    const FMarketLoanApp& A = State.Banking.Apps[AppIndex];
    const FString Name = Bank(State, A.Bank).Name;
    const FString What = A.Purpose == static_cast<uint8>(EPurpose::Acquisition) && State.Rivals.Chains.IsValidIndex(A.Chain)
        ? FString::Printf(TEXT("%s sat\u0131n almas\u0131"), *State.Rivals.Chains[A.Chain].Name) : FString(TEXT("yat\u0131r\u0131m kredisi"));
    switch (static_cast<EAppStatus>(A.Status))
    {
    case EAppStatus::Pending:
        return FString::Printf(TEXT("%s \u00b7 %s \u00b7 %s istendi \u00b7 cevap %d g\u00fcn i\u00e7inde"), *Name, *What, *MarketBankingAppsLocal::Money(A.Asked), FMath::Max(0, A.AnswerDay - State.Day + 1));
    case EAppStatus::Offered:
        return FString::Printf(TEXT("%s \u00b7 %s \u00b7 TEKL\u0130F %s, y\u0131ll\u0131k %s, %d ay%s \u00b7 %d g\u00fcn ge\u00e7erli%s"), *Name, *What, *MarketBankingAppsLocal::Money(A.Offered),
            *MarketBankingLocal::Percent(A.YearRate), A.Months, A.bGrace ? TEXT(" (6 ay yaln\u0131z faiz)") : TEXT(""), FMath::Max(0, A.ValidUntil - State.Day + 1),
            A.Reason.IsEmpty() ? TEXT("") : *(TEXT(" \u00b7 ") + A.Reason));
    case EAppStatus::Refused:
        return FString::Printf(TEXT("%s \u00b7 %s \u00b7 RET: %s"), *Name, *What, *A.Reason);
    default:
        return FString::Printf(TEXT("%s \u00b7 %s"), *Name, *What);
    }
}

void MarketBanking::CloseApps(FMarketState& State)
{
    using namespace MarketBankingAppsLocal;
    const int32 Closed = State.Day - 1;
    if (Closed < 1) return;
    TArray<FString>& News = State.DayNews;
    for (FMarketLoanApp& A : State.Banking.Apps)
    {
        if (A.Status == static_cast<uint8>(EAppStatus::Offered) && Closed > A.ValidUntil)
        {
            A.Status = static_cast<uint8>(EAppStatus::Expired);
            News.Add(FString::Printf(TEXT("%s teklifinin s\u00fcresi doldu."), *Bank(State, A.Bank).Name));
            continue;
        }
        if (A.Status != static_cast<uint8>(EAppStatus::Pending) || Closed < A.AnswerDay) continue;
        const FString Name = Bank(State, A.Bank).Name;
        const bool bAcq = A.Purpose == static_cast<uint8>(EPurpose::Acquisition);
        FString Why;
        int64 Room = 0;
        if (!CanBorrow(State, A.Bank, Why)) Room = 0;
        else if (bAcq) { Room = MarketChains::DealPrice(State, A.Chain) > 0 ? AcquisitionRoom(State, A.Bank, MarketChains::YearProfit(State, A.Chain)) : 0; if (Room <= 0) Why = TEXT("bor\u00e7, \u015firketin ve hedefin kazanc\u0131na g\u00f6re fazla"); }
        else { Room = Offer(State, A.Bank); if (Room <= 0) Why = TEXT("bor\u00e7 \u015firketin kazanc\u0131na g\u00f6re zaten y\u00fcksek"); }
        A.AnswerDay = Closed;
        if (Room < FMath::Max<int64>(10000, A.Asked * 3 / 10))
        {
            A.Status = static_cast<uint8>(EAppStatus::Refused);
            A.Reason = Why.IsEmpty() ? FString::Printf(TEXT("komite yeterli g\u00f6rmedi (verebilece\u011fi %s)"), *Money(Room)) : Why;
            News.Add(FString::Printf(TEXT("Asistan: %s ba\u015fvurumuza cevap verdi: ret. Gerek\u00e7e: %s. Notumuz %s."), *Name, *A.Reason, *RatingName(Rating(State))));
            continue;
        }
        // The bank's mood of the day: -0.5 .. +1 point on the day's rate.
        static const float Mood[4] = { -0.005f, 0.f, 0.005f, 0.01f };
        A.YearRate = static_cast<float>(FMath::Max(0.01, YearRate(State, A.Bank) + Mood[Mix(static_cast<uint32>(State.RivalSeed), static_cast<uint32>(A.Id), A.Bank + 0xB4u) % 4u]));
        A.Offered = FMath::Min(A.Asked, Room) / 10000 * 10000;
        A.Months = bAcq ? Bank(State, A.Bank).MaxTenor : MonthsFor(State, A.Bank, A.Tenor);
        A.ValidUntil = Closed + AppValidDays;
        A.Status = static_cast<uint8>(EAppStatus::Offered);
        A.Reason = A.Offered < A.Asked ? FString::Printf(TEXT("istenenin bir k\u0131sm\u0131: notumuz %s, bor\u00e7 / FAV\u00d6K %.1f"), *RatingName(Rating(State)), FMath::Min(99.f, Picture(State).Leverage)) : FString();
        News.Add(FString::Printf(TEXT("Asistan: %s ba\u015fvurumuza cevap verdi: %s, y\u0131ll\u0131k %s, %d ay%s. Teklif %d g\u00fcn ge\u00e7erli (Finans)."),
            *Name, *Money(A.Offered), *MarketBankingLocal::Percent(A.YearRate), A.Months, A.Offered < A.Asked ? *FString::Printf(TEXT(" (istedi\u011fimiz %s'\u0131n bir k\u0131sm\u0131)"), *Money(A.Asked)) : TEXT(""), AppValidDays));
    }
}

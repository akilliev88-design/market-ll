#include "MarketBanking.h"
#include "MarketBranches.h"
#include "MarketCast.h"
#include "MarketCalendar.h"
#include "MarketCountry.h"
#include "MarketFinance.h"
#include "MarketPrices.h"

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
    for (const FMarketCorpLoan& L : State.Banking.Loans) Total += L.Balance;
    return Total;
}

bool MarketBanking::CovenantBroken(const FMarketState& State)
{
    return State.Banking.BreachMonths > 0;
}

double MarketBanking::YearRate(const FMarketState& State, int32 BankIndex)
{
    return FMath::Max(0.01, MarketPrices::LoanRate(FMath::Max(1, State.Day)) + Bank(State, BankIndex).Spread + RatingSpread(Rating(State))
        + (CovenantBroken(State) ? BreachPenalty : 0.f));
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
    const int64 Fee = FMath::RoundToInt64(L.Balance * EarlyRepayFee);
    if (State.Cash < L.Balance + Fee) { OutMessage = FString::Printf(TEXT("Kapatmak i\u00e7in kasada %s gerekir."), *MarketCountry::Money(L.Balance + Fee)); return false; }
    State.Cash -= L.Balance + Fee;
    MarketBankingLocal::Book(State, MarketLedger::EAccount::LoanRepayment, -L.Balance);
    MarketBankingLocal::Book(State, MarketLedger::EAccount::BankFees, -Fee);
    State.Banking.Loans.RemoveAt(LoanIndex);
    OutMessage = FString::Printf(TEXT("%s kredisi kapand\u0131: %s (erken kapama %s)."), *Bank(State, L.Bank).Name, *MarketCountry::Money(L.Balance), *MarketCountry::Money(Fee));
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
    for (const FMarketCorpLoan& L : B.Loans) Total += L.Balance;
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
    FString Line = FString::Printf(TEXT("%s \u00b7 %s \u00b7 kalan %s \u00b7 y\u0131ll\u0131k %s \u00b7 %d/%d ay"), *Bank(State, L.Bank).Name, What, *MarketCountry::Money(L.Balance),
        *MarketBankingLocal::Percent(L.YearRate), L.PaidMonths, L.Months);
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
        MonthDue(L, Interest, Principal);
        const int64 Due = Interest + Principal;
        if (State.Cash + (B.bLineAuto ? LineRoom(State) : 0) < Due)
        {
            // Missed: a late fee joins the debt (C7: once, then once a month while it stays unpaid), the next try in
            // a week, the rating remembers.
            const bool bFee = L.LateSince == 0 || (Closed - L.LateSince) % MonthDays < 7;
            if (L.LateSince == 0) { L.LateSince = Closed; B.LateDays.Add(Closed); }
            L.NextDueDay = Closed + 7;
            if (!bFee) continue;
            const int64 Fee = FMath::RoundToInt64(Due * LateFee);
            L.Balance += Fee;
            Book(State, EAccount::Penalties, -Fee, false);
            News.Add(FString::Printf(TEXT("%s: %s taksit \u00f6denemedi, %s gecikme fark\u0131 borca eklendi. Kredi notu bunu hat\u0131rlar."), *Bank(State, L.Bank).Name, *MarketCountry::Money(Due), *MarketCountry::Money(Fee)));
            continue;
        }
        State.Cash -= Due;
        L.Balance -= Principal;
        B.InterestPaid += Interest;
        Book(State, EAccount::Interest, -Interest);
        Book(State, EAccount::LoanRepayment, -Principal);
        ++L.PaidMonths;
        L.LateSince = 0;
        L.NextDueDay = L.StartDay + (L.PaidMonths + 1) * MonthDays;
        if (L.NextDueDay <= Closed) L.NextDueDay = Closed + MonthDays;
    }
    for (int32 I = B.Loans.Num() - 1; I >= 0; --I)
    {
        if (B.Loans[I].Balance > 0) continue;
        News.Add(FString::Printf(TEXT("%s kredisi bitti: son taksit \u00f6dendi."), *Bank(State, B.Loans[I].Bank).Name));
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
            if (B.Loans[I].Kind != static_cast<uint8>(EKind::Called) && (Biggest == INDEX_NONE || B.Loans[I].Balance > B.Loans[Biggest].Balance)) Biggest = I;
        if (Biggest != INDEX_NONE)
        {
            const int64 Part = B.Loans[Biggest].Balance / 4;
            B.Loans[Biggest].Balance -= Part;
            const int32 BankIndex = B.Loans[Biggest].Bank;
            const double Rate = B.Loans[Biggest].YearRate;
            AddLoan(State, BankIndex, EKind::Called, Part, Rate, 1, 0);
            News.Add(FString::Printf(TEXT("%s borcunun d\u00f6rtte birini (%s) 30 g\u00fcn i\u00e7inde geri istiyor."), *Bank(State, BankIndex).Name, *MarketCountry::Money(Part)));
        }
    }
}

#include "MarketPortfolio.h"
#include "MarketEconomy.h"
#include "MarketBranches.h"
#include "MarketCalendar.h"
#include "MarketCompany.h"
#include "MarketCountry.h"
#include "MarketDepots.h"
#include "MarketLedger.h"
#include "MarketPrices.h"
#include "MarketStaff.h"
#include "MarketStoreAssign.h"
#include "MarketStoreViews.h"
#include "MarketStory.h"

namespace MarketPortfolioLocal
{
    using MarketBranches::EStage;

    bool IsOpen(const FMarketBranch& B) { return B.Stage == static_cast<uint8>(EStage::Open); }
    FString Tl(int64 Kurus) { return MarketCountry::Money(Kurus); }

    // Rent and wages of the days the store stays shut (paid with the works).
    int64 ClosedDaysCost(const FMarketState& State, const FMarketBranch& B, int32 Days)
    {
        const FString Country = MarketBranches::CountryOf(State, B);
        const double RentDay = static_cast<double>(B.Rent) * MarketPrices::ListLevel(State.Day) / FMath::Max(1e-9, MarketPrices::ListLevel(FMath::Max(1, B.OpenedDay))) / 30.0;
        const int64 Wages = MarketBranches::InHome(MarketBranches::MoneyOf(State, Country, State.Day), MarketStaff::BranchWages(B) + B.ManagerWage, true);
        return FMath::RoundToInt64(Days * (RentDay + Wages + MarketStaff::EmployerShare(Wages)));
    }

    int64 FitOut(const FMarketState& State, const FMarketBranch& B, float Share)
    {
        const MarketBranches::FFormat& Kind = MarketBranches::FormatInfo(B.Format);
        return FMath::RoundToInt64(Share * MarketBranches::FitOutCost(State, MarketBranches::SiteOf(State, B), Kind, MarketStoreAssign::FitOutFactor(MarketStoreViews::MeasuresOf(B), Kind.Id)));
    }

    // The store as it would be after a change of type (its new store view and shelves; nothing is saved).
    FMarketBranch Probe(const FMarketState& State, const FMarketBranch& B, const FString& Format, const TArray<FMarketProduct>& Products)
    {
        FMarketBranch P = B;
        P.Format = Format;
        MarketStoreViews::PreviewTo(State, P);
        MarketBranches::ReplanShelves(State, P, Products);
        return P;
    }

    // Size order of the types (toptan sits beside the hypermarket).
    int32 Rank(const FString& Format)
    {
        if (Format == TEXT("kucuk") || Format == TEXT("yakin")) return 0;
        if (Format == TEXT("mahalle")) return 1;
        if (Format == TEXT("buyuk")) return 2;
        return 3;
    }

    FString WorksName(uint8 Works)
    {
        switch (static_cast<MarketPortfolio::EWorks>(Works))
        {
        case MarketPortfolio::EWorks::Renovate: return TEXT("yenileme");
        case MarketPortfolio::EWorks::Reformat: return TEXT("t\u00fcr de\u011fi\u015fikli\u011fi");
        case MarketPortfolio::EWorks::Relocate: return TEXT("ta\u015f\u0131nma");
        default: return FString();
        }
    }
}

int32 MarketPortfolio::AgeDays(const FMarketState& State, const FMarketBranch& Branch)
{
    return FMath::Max(0, State.Day - FMath::Max(Branch.OpenedDay, Branch.RenewedDay));
}

float MarketPortfolio::AgeOnly(const FMarketState& State, const FMarketBranch& Branch)
{
    const float Years = static_cast<float>(AgeDays(State, Branch)) / YearDays;
    const float Worn = FMath::Clamp((Years - AgeStartYears) / static_cast<float>(AgeFullYears - AgeStartYears), 0.f, 1.f);
    return 1.f - AgeLoss * Worn;
}

bool MarketPortfolio::IsAgeing(const FMarketState& State, const FMarketBranch& Branch)
{
    return AgeDays(State, Branch) >= AgeStartYears * YearDays;
}

float MarketPortfolio::AgeFactor(const FMarketState& State, int32 BranchIndex)
{
    if (!State.Branches.IsValidIndex(BranchIndex)) return 1.f;
    const FMarketBranch& B = State.Branches[BranchIndex];
    const bool bFresh = B.RenewedDay > 0 && State.Day - B.RenewedDay < FreshDays;
    return AgeOnly(State, B) * (bFresh ? FreshPull : 1.f);
}

FString MarketPortfolio::Larger(const FMarketState& State, int32 BranchIndex)
{
    if (!State.Branches.IsValidIndex(BranchIndex)) return FString();
    const FMarketBranch& B = State.Branches[BranchIndex];
    const TArray<FString> Formats = MarketBranches::FormatsIn(MarketBranches::CountryOf(State, B));
    const int32 Now = MarketPortfolioLocal::Rank(B.Format);
    if (Now >= 3) return FString();
    // The plain next size (the four types every country knows).
    static const TCHAR* Order[4] = { TEXT("kucuk"), TEXT("mahalle"), TEXT("buyuk"), TEXT("hiper") };
    return Formats.Contains(Order[Now + 1]) ? FString(Order[Now + 1]) : FString();
}

FString MarketPortfolio::Smaller(const FMarketState& State, int32 BranchIndex)
{
    if (!State.Branches.IsValidIndex(BranchIndex)) return FString();
    const FMarketBranch& B = State.Branches[BranchIndex];
    const TArray<FString> Formats = MarketBranches::FormatsIn(MarketBranches::CountryOf(State, B));
    const int32 Now = MarketPortfolioLocal::Rank(B.Format);
    if (Now <= 0) return FString();
    static const TCHAR* Order[4] = { TEXT("kucuk"), TEXT("mahalle"), TEXT("buyuk"), TEXT("hiper") };
    return Formats.Contains(Order[Now - 1]) ? FString(Order[Now - 1]) : FString();
}

int32 MarketPortfolio::WorksDays(const FString& Format)
{
    return MarketPortfolioLocal::Rank(Format) >= 2 ? BigWorksDays : SmallWorksDays;
}

int64 MarketPortfolio::WorksCost(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 BranchIndex, EWorks Works, const FString& Format)
{
    using namespace MarketPortfolioLocal;
    if (!State.Branches.IsValidIndex(BranchIndex)) return 0;
    const FMarketBranch& B = State.Branches[BranchIndex];
    switch (Works)
    {
    case EWorks::Renovate:
        return FitOut(State, B, RenovateShare) + ClosedDaysCost(State, B, WorksDays(B.Format));
    case EWorks::Relocate:
        return FitOut(State, B, RelocateShare) + FMath::Max<int64>(0, 2 * (MarketBranches::SignedRent(State, B) - B.Rent)) + ClosedDaysCost(State, B, WorksDays(B.Format));
    case EWorks::Reformat:
    {
        if (Format.IsEmpty()) return 0;
        const FMarketBranch P = Probe(State, B, Format, Products);
        const int64 Stock = MarketBranches::InHome(MarketBranches::MoneyOf(State, MarketBranches::CountryOf(State, B), State.Day), MarketBranches::RestockCost(P, Products));
        return FitOut(State, P, ReformatShare) + FMath::Max<int64>(0, 2 * (MarketBranches::SignedRent(State, P) - B.Rent)) + Stock + ClosedDaysCost(State, B, WorksDays(Format));
    }
    default:
        return 0;
    }
}

bool MarketPortfolio::CanStart(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 BranchIndex, EWorks Works, const FString& Format, FString& OutReason)
{
    using namespace MarketPortfolioLocal;
    if (!State.Branches.IsValidIndex(BranchIndex)) { OutReason = TEXT("B\u00f6yle bir ma\u011faza yok."); return false; }
    const FMarketBranch& B = State.Branches[BranchIndex];
    if (B.Works != 0) { OutReason = FString::Printf(TEXT("%s: %s s\u00fcr\u00fcyor."), *B.Name, *WorksName(B.Works)); return false; }
    if (!IsOpen(B)) { OutReason = FString::Printf(TEXT("%s \u015fu an a\u00e7\u0131k de\u011fil."), *B.Name); return false; }
    if (State.Day < State.RescueUntil) { OutReason = TEXT("Kurtarma plan\u0131 s\u00fcr\u00fcyor: \u015fimdi masraf yap\u0131lmaz."); return false; }
    const int32 Age = AgeDays(State, B);
    if (Works == EWorks::Renovate && Age < RenovateMinYears * YearDays)
    {
        OutReason = FString::Printf(TEXT("%s yeni say\u0131l\u0131r: %d y\u0131l\u0131 doldurunca yenilenir."), *B.Name, RenovateMinYears);
        return false;
    }
    if (Works == EWorks::Relocate && Age < YearDays / 2) { OutReason = FString::Printf(TEXT("%s yeni a\u00e7\u0131ld\u0131: alt\u0131 ay dolmadan ta\u015f\u0131nmaz."), *B.Name); return false; }
    if (Works == EWorks::Reformat)
    {
        const FString Country = MarketBranches::CountryOf(State, B);
        const MarketBranches::FFormat& Kind = MarketBranches::FormatInfo(Format);
        const MarketBranches::FSite Site = MarketBranches::SiteOf(State, B);
        if (Format.IsEmpty() || Format == B.Format || !MarketBranches::FormatsIn(Country).Contains(Format)) { OutReason = TEXT("Bu t\u00fcre \u00e7evrilemez."); return false; }
        if (Kind.Chapter > 0 && !MarketCompany::ChapterOpen(State, Kind.Chapter)) { OutReason = FString::Printf(TEXT("%s i\u00e7in \"%s\" b\u00f6l\u00fcm\u00fc a\u00e7\u0131lmal\u0131."), Kind.Short, *MarketStory::ChapterTitle(Kind.Chapter)); return false; }
        if (Site.PopulationK < Kind.MinPopulationK) { OutReason = FString::Printf(TEXT("%s yaln\u0131z n\u00fcfusu %d binin \u00fcst\u00fcndeki illerde olur."), Kind.Short, Kind.MinPopulationK); return false; }
        float Km = 0.f;
        if (Kind.bNeedsDepot && MarketDepots::Nearest(State, Site.Country, Site.Province, false, Km) == INDEX_NONE) { OutReason = FString::Printf(TEXT("%s i\u00e7in %d km i\u00e7inde bir depo gerekir."), Kind.Short, MarketDepots::RangeKm); return false; }
    }
    const int64 Cost = WorksCost(State, Products, BranchIndex, Works, Format);
    if (State.Cash < Cost) { OutReason = FString::Printf(TEXT("Kasada %s gerekiyor."), *Tl(Cost)); return false; }
    return true;
}

namespace MarketPortfolioLocal
{
    bool Start(FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index, MarketPortfolio::EWorks Works, const FString& Format, FString& OutMessage)
    {
        if (!MarketPortfolio::CanStart(State, Products, Index, Works, Format, OutMessage)) return false;
        FMarketBranch& B = State.Branches[Index];
        const FString Target = Works == MarketPortfolio::EWorks::Reformat ? Format : B.Format;
        const int32 Days = MarketPortfolio::WorksDays(Target);
        // Paid now: the works and the shut days' rent and wages. A new lease's deposit and new shelves' goods are
        // paid at the reopening.
        int64 Now = ClosedDaysCost(State, B, Days);
        if (Works == MarketPortfolio::EWorks::Renovate) Now += FitOut(State, B, MarketPortfolio::RenovateShare);
        else if (Works == MarketPortfolio::EWorks::Relocate) Now += FitOut(State, B, MarketPortfolio::RelocateShare);
        else Now += FitOut(State, Probe(State, B, Format, Products), MarketPortfolio::ReformatShare);
        MarketLedger::AddStoreCost(State, Now, Index);
        B.Stage = static_cast<uint8>(EStage::Renovation);
        B.StageUntil = State.Day + Days - 1;
        B.Works = static_cast<uint8>(Works);
        B.WorksFormat = Works == MarketPortfolio::EWorks::Reformat ? Format : FString();
        const MarketBranches::FFormat& Kind = MarketBranches::FormatInfo(Target);
        switch (Works)
        {
        case MarketPortfolio::EWorks::Renovate:
            OutMessage = FString::Printf(TEXT("%s yenileniyor: %d g\u00fcn kapal\u0131 (%s, kapal\u0131 g\u00fcnlerin kiras\u0131 ve maa\u015flar\u0131 dahil). Yeniden a\u00e7\u0131l\u0131nca ya\u015f\u0131 s\u0131f\u0131rlan\u0131r, ilk y\u0131l biraz daha \u00e7ok m\u00fc\u015fteri \u00e7eker."), *B.Name, Days, *Tl(Now));
            break;
        case MarketPortfolio::EWorks::Relocate:
            OutMessage = FString::Printf(TEXT("%s ayn\u0131 ilde daha iyi bir yere ta\u015f\u0131n\u0131yor: %d g\u00fcn kapal\u0131 (%s). Yeni kira s\u00f6zle\u015fmesi a\u00e7\u0131l\u0131\u015fta imzalan\u0131r; m\u00fc\u015fterilerin bir k\u0131sm\u0131 yeni yeri sonradan \u00f6\u011frenir."), *B.Name, Days, *Tl(Now));
            break;
        default:
            OutMessage = FString::Printf(TEXT("%s, %s olacak: %d g\u00fcn kapal\u0131 (%s). Yeni raflar\u0131n mal\u0131 ve yeni kiran\u0131n depozitosu a\u00e7\u0131l\u0131\u015fta \u00f6denir."), *B.Name, Kind.Name, Days, *Tl(Now));
            break;
        }
        return true;
    }
}

bool MarketPortfolio::Renovate(FMarketState& State, const TArray<FMarketProduct>& Products, int32 BranchIndex, FString& OutMessage)
{
    return MarketPortfolioLocal::Start(State, Products, BranchIndex, EWorks::Renovate, FString(), OutMessage);
}

bool MarketPortfolio::Reformat(FMarketState& State, const TArray<FMarketProduct>& Products, int32 BranchIndex, const FString& Format, FString& OutMessage)
{
    return MarketPortfolioLocal::Start(State, Products, BranchIndex, EWorks::Reformat, Format, OutMessage);
}

bool MarketPortfolio::Relocate(FMarketState& State, const TArray<FMarketProduct>& Products, int32 BranchIndex, FString& OutMessage)
{
    return MarketPortfolioLocal::Start(State, Products, BranchIndex, EWorks::Relocate, FString(), OutMessage);
}

FString MarketPortfolio::Finish(FMarketState& State, const TArray<FMarketProduct>& Products, int32 BranchIndex)
{
    using namespace MarketPortfolioLocal;
    if (!State.Branches.IsValidIndex(BranchIndex)) return FString();
    FMarketBranch& B = State.Branches[BranchIndex];
    const EWorks Works = static_cast<EWorks>(B.Works);
    B.Works = 0;
    B.Stage = static_cast<uint8>(EStage::Open);
    B.RenewedDay = State.Day;
    // A new lease (change of type, relocation): today's rent, the deposit's difference.
    auto NewLease = [&State, BranchIndex](FMarketBranch& Store)
    {
        const int64 Rent = MarketBranches::SignedRent(State, Store);
        const int64 Deposit = 2 * (Rent - Store.Rent);
        State.Cash -= Deposit;
        if (Deposit > 0) MarketLedger::Post(State, MarketLedger::EAccount::Investment, -Deposit, true, BranchIndex);
        else if (Deposit < 0) MarketLedger::Post(State, MarketLedger::EAccount::Divestment, -Deposit, true, BranchIndex);
        Store.Rent = Rent;
        Store.OpenedDay = State.Day;
    };
    if (Works == EWorks::Renovate)
    {
        B.Satisfaction = FMath::Min(100.f, B.Satisfaction + 5.f);
        return FString::Printf(TEXT("%s yenilendi ve yar\u0131n a\u00e7\u0131l\u0131yor: raflar, \u0131\u015f\u0131k, tabela yeni. M\u00fc\u015fteri fark\u0131 g\u00f6recek."), *B.Name);
    }
    if (Works == EWorks::Relocate)
    {
        NewLease(B);
        B.bHasty = 0;
        B.Maturity *= RelocateHabit;
        return FString::Printf(TEXT("%s yeni yerinde yar\u0131n a\u00e7\u0131l\u0131yor (kira %s). Eski m\u00fc\u015fterilerin bir k\u0131sm\u0131 yeni adresi birka\u00e7 haftada \u00f6\u011frenir."), *B.Name, *Tl(B.Rent));
    }
    // Change of type: the new store view, its people, its lease, its shelves and their goods.
    const MarketBranches::FFormat& Old = MarketBranches::FormatInfo(B.Format);
    const FString Format = B.WorksFormat.IsEmpty() ? B.Format : B.WorksFormat;
    const MarketBranches::FFormat& Kind = MarketBranches::FormatInfo(Format);
    FMarketBranch Store = B;
    Store.Format = Kind.Id;
    Store.WorksFormat.Reset();
    Store.Name = Store.Name.Replace(Old.Short, Kind.Short);
    MarketStoreViews::AssignTo(State, Store);
    Store.Workers = MarketStoreAssign::WorkersFor(MarketStoreViews::MeasuresOf(Store), Store.Format);
    if (Store.Staff.Num() > Store.Workers) Store.Staff.SetNum(Store.Workers); // the extra people leave with the old type
    NewLease(Store);
    MarketBranches::ReplanShelves(State, Store, Products);
    const int64 Bill = MarketBranches::InHome(MarketBranches::MoneyOf(State, MarketBranches::CountryOf(State, Store), State.Day), MarketBranches::RestockCost(Store, Products));
    for (FMarketStock& Item : Store.Items) Item.Shelf = FMath::Max(Item.Shelf, Item.Capacity);
    Store.Maturity *= ReformatHabit;
    State.Branches[BranchIndex] = Store;
    State.Cash -= Bill;
    State.Purchases += Bill;
    MarketLedger::Post(State, MarketLedger::EAccount::Purchases, -Bill, true, BranchIndex);
    MarketStaff::StaffBranch(State, BranchIndex); // a bigger type hires the people it lacks
    return FString::Printf(TEXT("%s art\u0131k %s; yar\u0131n a\u00e7\u0131l\u0131yor. Yeni raflar\u0131n mal\u0131 %s, yeni kira %s."), *State.Branches[BranchIndex].Name, Kind.Name, *Tl(Bill), *Tl(State.Branches[BranchIndex].Rent));
}

int32 MarketPortfolio::EncodeFormat(int32 BranchIndex, const FString& Format)
{
    const int32 F = MarketBranches::AllFormatIds().IndexOfByKey(Format);
    return BranchIndex < 0 || F == INDEX_NONE ? INDEX_NONE : BranchIndex * 10 + F;
}

bool MarketPortfolio::DecodeFormat(int32 Arg, int32& OutBranch, FString& OutFormat)
{
    if (Arg < 0 || !MarketBranches::AllFormatIds().IsValidIndex(Arg % 10)) return false;
    OutBranch = Arg / 10;
    OutFormat = MarketBranches::AllFormatIds()[Arg % 10];
    return true;
}

uint8 MarketPortfolio::CardOf(float Margin, float Satisfaction, float AgeFactorNow)
{
    const float Money = FMath::Clamp((Margin + 0.02f) / 0.10f, 0.f, 1.f);               // -2 % .. +8 % net
    const float Liked = FMath::Clamp(Satisfaction / 100.f, 0.f, 1.f);
    const float Fresh = FMath::Clamp((AgeFactorNow - (1.f - AgeLoss)) / AgeLoss, 0.f, 1.f);
    const float Score = 0.55f * Money + 0.25f * Liked + 0.2f * Fresh;
    return Score >= 0.75f ? 1 : Score >= 0.6f ? 2 : Score >= 0.45f ? 3 : Score >= 0.3f ? 4 : 5;
}

FString MarketPortfolio::CardLetter(uint8 Card)
{
    static const TCHAR* Letters[6] = { TEXT("-"), TEXT("A"), TEXT("B"), TEXT("C"), TEXT("D"), TEXT("E") };
    return Letters[Card <= 5 ? Card : 0];
}

FString MarketPortfolio::Advice(const FMarketState& State, int32 BranchIndex)
{
    if (!State.Branches.IsValidIndex(BranchIndex)) return FString();
    const FMarketBranch& B = State.Branches[BranchIndex];
    if (B.Works != 0 || !MarketPortfolioLocal::IsOpen(B)) return FString();
    if (B.Card >= 4 && B.CardProfit < 0) return TEXT("zararda: k\u00fc\u00e7\u00fcltmeyi ya da kapatmay\u0131 d\u00fc\u015f\u00fcn");
    if (IsAgeing(State, B)) return FString::Printf(TEXT("eskidi (m\u00fc\u015fteri %%%.0f az): yenileme zaman\u0131"), (1.f - AgeOnly(State, B)) * 100.f);
    if (B.bHasty && AgeDays(State, B) >= YearDays / 2) return TEXT("yeri zay\u0131f: ayn\u0131 ilde ta\u015f\u0131nabilir");
    if (B.Card == 1 && !Larger(State, BranchIndex).IsEmpty())
    {
        const MarketBranches::FFormat& Next = MarketBranches::FormatInfo(Larger(State, BranchIndex));
        if (MarketBranches::SiteOf(State, B).PopulationK >= Next.MinPopulationK) return FString::Printf(TEXT("\u00e7ok iyi gidiyor: %s olabilir"), Next.Name);
    }
    return FString();
}

FString MarketPortfolio::Line(const FMarketState& State, int32 BranchIndex)
{
    if (!State.Branches.IsValidIndex(BranchIndex)) return FString();
    const FMarketBranch& B = State.Branches[BranchIndex];
    if (B.Works != 0)
        return FString::Printf(TEXT("%s s\u00fcr\u00fcyor: %d. g\u00fcne kadar kapal\u0131"), *MarketPortfolioLocal::WorksName(B.Works), B.StageUntil);
    if (!MarketPortfolioLocal::IsOpen(B)) return FString();
    const int32 Years = AgeDays(State, B) / YearDays;
    FString Line = Years > 0 ? FString::Printf(TEXT("%d y\u0131ll\u0131k"), Years) : FString(TEXT("bir y\u0131ldan yeni"));
    if (B.RenewedDay > 0 && State.Day - B.RenewedDay < FreshDays) Line += TEXT(" \u00b7 yeni yenilendi");
    if (B.Card > 0)
        Line += FString::Printf(TEXT(" \u00b7 y\u0131ll\u0131k karne %s (%d. y\u0131l, net %%%.1f)"), *CardLetter(B.Card), B.CardYear,
            B.CardRevenue > 0 ? 100.0 * static_cast<double>(B.CardProfit) / static_cast<double>(B.CardRevenue) : 0.0);
    const FString Hint = Advice(State, BranchIndex);
    if (!Hint.IsEmpty()) Line += TEXT(" \u00b7 ") + Hint;
    return Line;
}

void MarketPortfolio::CloseDay(FMarketState& State)
{
    if (State.Day <= 1) return;
    const int32 Year = MarketCalendar::CampaignYear(State.Day - 1);
    if (MarketCalendar::CampaignYear(State.Day) == Year) return;
    int32 Counts[6] = { 0, 0, 0, 0, 0, 0 };
    int32 Best = INDEX_NONE, Worst = INDEX_NONE, Old = 0;
    double BestMargin = 0.0, WorstMargin = 0.0;
    for (int32 I = 0; I < State.Branches.Num(); ++I)
    {
        FMarketBranch& B = State.Branches[I];
        if (MarketPortfolioLocal::IsOpen(B) && B.YearDays >= CardMinDays && B.YearRevenue > 0)
        {
            const double Margin = static_cast<double>(B.YearProfit) / static_cast<double>(B.YearRevenue);
            B.Card = CardOf(static_cast<float>(Margin), B.Satisfaction, AgeOnly(State, B));
            B.CardYear = Year;
            B.CardRevenue = B.YearRevenue;
            B.CardProfit = B.YearProfit;
            ++Counts[B.Card];
            if (Best == INDEX_NONE || Margin > BestMargin) { Best = I; BestMargin = Margin; }
            if (Worst == INDEX_NONE || Margin < WorstMargin) { Worst = I; WorstMargin = Margin; }
            if (IsAgeing(State, B)) ++Old;
        }
        B.YearRevenue = 0;
        B.YearProfit = 0;
        B.YearDays = 0;
    }
    const int32 Total = Counts[1] + Counts[2] + Counts[3] + Counts[4] + Counts[5];
    if (Total == 0) return;
    FString Line = FString::Printf(TEXT("%d. y\u0131l\u0131n ma\u011faza karneleri: %d A, %d B, %d C, %d D, %d E."), Year, Counts[1], Counts[2], Counts[3], Counts[4], Counts[5]);
    if (Total >= 2) Line += FString::Printf(TEXT(" En iyisi %s (net %%%.1f), en zay\u0131f\u0131 %s (net %%%.1f)."), *State.Branches[Best].Name, BestMargin * 100.0, *State.Branches[Worst].Name, WorstMargin * 100.0);
    if (Old > 0) Line += FString::Printf(TEXT(" %d ma\u011fazan eskidi: yenilenmezse m\u00fc\u015fteri kaybeder."), Old);
    State.DayNews.Add(Line + TEXT(" Ayr\u0131nt\u0131 Ma\u011fazalar sayfas\u0131nda."));
}

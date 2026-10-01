#include "MarketOwner.h"
#include "MarketCalendar.h"
#include "MarketCountry.h"
#include "MarketFinance.h"
#include "MarketLedger.h"
#include "MarketStaff.h"

namespace MarketOwnerLocal
{
    FString OwnerTl(int64 Kurus) { return MarketCountry::Money(Kurus); }
}

int64 MarketOwner::MinimumMonthly(const FMarketState& State)
{
    return 30 * MarketStaff::MinimumDailyWage(FMath::Max(1, State.Day));
}

int64 MarketOwner::GrossSalary(const FMarketState& State)
{
    return MinimumMonthly(State) * FMath::Max(10, State.Owner.SalaryX10) / 10;
}

int64 MarketOwner::SalaryTax(const FMarketState& State, int64 Gross)
{
    // Progressive, in minimum wages: 15 % up to 3, 27 % up to 10, 40 % above (after our social security).
    const double Min = static_cast<double>(FMath::Max<int64>(1, MinimumMonthly(State)));
    const double Base = static_cast<double>(Gross - SalarySocial(State, Gross));
    const double A = FMath::Min(Base, 3.0 * Min);
    const double B = FMath::Clamp(Base - 3.0 * Min, 0.0, 7.0 * Min);
    const double C = FMath::Max(0.0, Base - 10.0 * Min);
    return FMath::RoundToInt64(A * 0.15 + B * 0.27 + C * 0.40);
}

int64 MarketOwner::SalarySocial(const FMarketState& State, int64 Gross)
{
    const int64 Cap = MinimumMonthly(State) * SocialCapX / 10;
    return FMath::RoundToInt64(static_cast<double>(FMath::Min(Gross, Cap)) * EmployeeSocialRate);
}

int64 MarketOwner::NetSalary(const FMarketState& State)
{
    const int64 Gross = GrossSalary(State);
    return Gross - SalarySocial(State, Gross) - SalaryTax(State, Gross);
}

int64 MarketOwner::CompanyCost(const FMarketState& State)
{
    const int64 Gross = GrossSalary(State);
    const int64 Cap = MinimumMonthly(State) * SocialCapX / 10;
    return Gross + MarketStaff::EmployerShare(FMath::Min(Gross, Cap));
}

int64 MarketOwner::Living(const FMarketState& State)
{
    return FMath::RoundToInt64(MinimumMonthly(State) * static_cast<double>(LivingX));
}

bool MarketOwner::SetSalary(FMarketState& State, int32 Step, FString& OutMessage)
{
    if (Step < 0 || Step >= StepCount) { OutMessage = TEXT("B\u00f6yle bir maa\u015f yok."); return false; }
    if (State.Day < State.RescueUntil && SalarySteps[Step] > 10) { OutMessage = TEXT("Kurtarma plan\u0131 s\u00fcrerken banka asgari \u00fccretten fazlas\u0131na izin vermiyor."); return false; }
    if (State.Owner.SalaryX10 == SalarySteps[Step]) { OutMessage = TEXT("Maa\u015f\u0131n zaten bu."); return false; }
    State.Owner.SalaryX10 = SalarySteps[Step];
    OutMessage = FString::Printf(TEXT("Maa\u015f\u0131n ayda %s br\u00fct (eline %s ge\u00e7er; \u015firkete maliyeti %s). Ay ba\u015f\u0131nda \u00f6denir."),
        *MarketOwnerLocal::OwnerTl(GrossSalary(State)), *MarketOwnerLocal::OwnerTl(NetSalary(State)), *MarketOwnerLocal::OwnerTl(CompanyCost(State)));
    return true;
}

int64 MarketOwner::Distributable(const FMarketState& State)
{
    const int32 LastYear = MarketCalendar::DateOf(FMath::Max(1, State.Day)).Year - 1;
    if (LastYear < MarketCalendar::StartYear) return 0;
    const int64 Profit = MarketLedger::YearStatement(State, LastYear).NetProfit;
    const int64 Paid = State.Owner.DividendYear == LastYear ? State.Owner.DividendPaid : 0;
    return FMath::Max<int64>(0, Profit - Paid);
}

int64 MarketOwner::DividendRoom(const FMarketState& State)
{
    if (State.Day < State.RescueUntil || State.TroubleStage > 0) return 0;
    const int64 Keep = MarketFinance::CompanyMonthCost(State);
    return FMath::Clamp<int64>(State.Cash - Keep, 0, Distributable(State));
}

bool MarketOwner::PayDividend(FMarketState& State, int32 Step, FString& OutMessage)
{
    if (Step < 0 || Step >= DividendSteps) { OutMessage = TEXT("B\u00f6yle bir oran yok."); return false; }
    if (State.Day < State.RescueUntil) { OutMessage = TEXT("Kurtarma plan\u0131 s\u00fcrerken k\u00e2r pay\u0131 da\u011f\u0131t\u0131lamaz."); return false; }
    const int64 Room = DividendRoom(State);
    if (Room <= 0)
    {
        OutMessage = Distributable(State) <= 0 ? FString(TEXT("Da\u011f\u0131t\u0131lacak k\u00e2r yok: ge\u00e7en y\u0131l\u0131n net k\u00e2r\u0131 ya yok ya da da\u011f\u0131t\u0131ld\u0131."))
            : FString::Printf(TEXT("Kasada \u015firketin bir ayl\u0131k sabit gideri (%s) kalmal\u0131."), *MarketOwnerLocal::OwnerTl(MarketFinance::CompanyMonthCost(State)));
        return false;
    }
    const int64 Gross = FMath::Max<int64>(100, Room * DividendPercents[Step] / 100 / 100 * 100);
    const int64 Tax = FMath::RoundToInt64(Gross * static_cast<double>(DividendWithholding));
    const int32 LastYear = MarketCalendar::DateOf(FMath::Max(1, State.Day)).Year - 1;
    if (State.Owner.DividendYear != LastYear) { State.Owner.DividendYear = LastYear; State.Owner.DividendPaid = 0; }
    State.Owner.DividendPaid += Gross;
    State.Cash -= Gross;
    MarketLedger::Post(State, MarketLedger::EAccount::OwnerDraw, -Gross, true, MarketLedger::HeadOfficeStore);
    State.Owner.Wealth += Gross - Tax;
    State.Owner.YearDividends += Gross - Tax;
    State.Owner.TotalDividends += Gross - Tax;
    OutMessage = FString::Printf(TEXT("K\u00e2r pay\u0131: \u015firketten %s \u00e7\u0131kt\u0131, %s stopaj kesildi, servetine %s eklendi."),
        *MarketOwnerLocal::OwnerTl(Gross), *MarketOwnerLocal::OwnerTl(Tax), *MarketOwnerLocal::OwnerTl(Gross - Tax));
    return true;
}

bool MarketOwner::PutCapital(FMarketState& State, int32 Step, FString& OutMessage)
{
    if (Step < 0 || Step >= DividendSteps) { OutMessage = TEXT("B\u00f6yle bir oran yok."); return false; }
    const int64 Amount = State.Owner.Wealth * CapitalPercents[Step] / 100 / 100 * 100;
    if (Amount <= 0) { OutMessage = TEXT("Ki\u015fisel servetin yok."); return false; }
    State.Owner.Wealth -= Amount;
    State.Owner.CapitalIn += Amount;
    State.Cash += Amount;
    MarketLedger::Post(State, MarketLedger::EAccount::Capital, Amount, true, MarketLedger::HeadOfficeStore);
    OutMessage = FString::Printf(TEXT("Kendi paran\u0131n %s kadar\u0131n\u0131 \u015firkete sermaye olarak koydun."), *MarketOwnerLocal::OwnerTl(Amount));
    return true;
}

void MarketOwner::CutToMinimum(FMarketState& State, TArray<FString>& Lines)
{
    if (State.Owner.SalaryX10 <= 10) return;
    State.Owner.SalaryX10 = 10;
    Lines.Add(TEXT("Banka senin maa\u015f\u0131n\u0131 da asgari \u00fccrete indirdi; plan bitene kadar k\u00e2r pay\u0131 yok."));
}

void MarketOwner::MonthStart(FMarketState& State)
{
    FMarketOwner& O = State.Owner;
    const int32 Year = MarketCalendar::DateOf(FMath::Max(1, State.Day)).Year;
    if (MarketCalendar::DateOf(FMath::Max(1, State.Day - 1)).Year != Year) { O.YearSalary = 0; O.YearDividends = 0; }
    // The salary: a cost of the company when the till can pay it.
    const int64 Gross = GrossSalary(State);
    const int64 Cost = CompanyCost(State);
    if (State.Cash >= Cost)
    {
        const int64 Employer = Cost - Gross;
        State.Cash -= Cost;
        MarketLedger::Post(State, MarketLedger::EAccount::Wages, -Gross, true, MarketLedger::HeadOfficeStore);
        MarketLedger::Post(State, MarketLedger::EAccount::SocialSecurity, -Employer, true, MarketLedger::HeadOfficeStore);
        State.LastProfit -= Cost;
        State.Books.PeriodProfit -= Cost;
        O.LastNet = NetSalary(State);
        O.Wealth += O.LastNet;
        O.YearSalary += O.LastNet;
        O.TotalSalary += O.LastNet;
        State.DayNews.Add(FString::Printf(TEXT("Maa\u015f\u0131n yatt\u0131: %s (br\u00fct %s; vergi ve sigorta kesildi). \u015eirkete maliyeti %s."),
            *MarketOwnerLocal::OwnerTl(O.LastNet), *MarketOwnerLocal::OwnerTl(Gross), *MarketOwnerLocal::OwnerTl(Cost)));
    }
    else
    {
        O.LastNet = 0;
        ++O.MissedSalaries;
        State.DayNews.Add(TEXT("Kasa bu ay sana maa\u015f \u00f6deyemedi; evin ihtiyac\u0131 birikimden kar\u015f\u0131lanacak."));
    }
    // The family lives from our own money.
    const int64 Need = Living(State);
    O.LastLiving = FMath::Min(Need, FMath::Max<int64>(0, O.Wealth));
    O.Wealth -= O.LastLiving;
    if (O.LastLiving < Need) State.DayNews.Add(TEXT("Evde zor bir ay: birikim ya\u015fam giderine yetmedi."));
}

FString MarketOwner::Summary(const FMarketState& State)
{
    const FMarketOwner& O = State.Owner;
    return FString::Printf(TEXT("Maa\u015f\u0131n ayda %s (eline %s) \u00b7 ki\u015fisel servet %s \u00b7 ev gideri ayda %s \u00b7 bu y\u0131l maa\u015f %s, k\u00e2r pay\u0131 %s"),
        *MarketOwnerLocal::OwnerTl(GrossSalary(State)), *MarketOwnerLocal::OwnerTl(NetSalary(State)), *MarketOwnerLocal::OwnerTl(O.Wealth),
        *MarketOwnerLocal::OwnerTl(Living(State)), *MarketOwnerLocal::OwnerTl(O.YearSalary), *MarketOwnerLocal::OwnerTl(O.YearDividends));
}

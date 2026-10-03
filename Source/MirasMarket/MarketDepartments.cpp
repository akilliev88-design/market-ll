#include "MarketDepartments.h"
#include "MarketBranches.h"
#include "MarketCalendar.h"
#include "MarketCountry.h"
#include "MarketPrices.h"
#include "MarketSourcing.h"
#include "MarketStaff.h"

namespace MarketDepartmentsLocal
{
    using MarketDepartments::EDept;
    using MarketDepartments::FInfo;

    // C7 (Codex C5: hyper fish / home / stationery / baby lost money in every mature sample): fish 0.34 -> 0.36
    // margin and 0.10 -> 0.08 waste, home 0.33 -> 0.36, stationery 0.30 -> 0.33, baby 0.04 -> 0.05 of the sales and
    // 0.20 -> 0.25 margin. To be checked again with the bot's department rows.
    //                 id            name                   fresh  master fmt space ratio  margin waste  shrink  pull   fit   stock inc   season (Jan..Dec)                                                     clearance
    const FInfo Table[MarketDepartments::DeptCount] = {
        { TEXT("manav"),      TEXT("Manav"),                 true,  false, 1, 8,  0.16f, 0.30f, 0.09f, 0.005f, 0.06f, 0.04f, 2,  0.3f, { 0.9f, 0.9f, 0.95f, 1.f, 1.05f, 1.15f, 1.2f, 1.2f, 1.1f, 1.f, 0.95f, 0.9f }, false },
        { TEXT("kasap"),      TEXT("Kasap"),                 true,  true,  2, 6,  0.20f, 0.26f, 0.03f, 0.005f, 0.07f, 0.10f, 3,  0.6f, { 1.05f, 1.f, 1.f, 1.f, 1.f, 0.95f, 0.95f, 1.f, 1.f, 1.f, 1.05f, 1.1f }, false },
        { TEXT("sarkuteri"),  TEXT("\u015eark\u00fcteri"),   true,  false, 2, 5,  0.12f, 0.28f, 0.04f, 0.01f,  0.03f, 0.06f, 5,  0.6f, { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.05f, 1.15f }, false },
        { TEXT("firin"),      TEXT("F\u0131r\u0131n ve pastane"), true, true, 2, 5, 0.08f, 0.50f, 0.12f, 0.f, 0.07f, 0.12f, 1, 0.1f, { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.05f }, false },
        { TEXT("balik"),      TEXT("Bal\u0131k"),            true,  true,  2, 3,  0.08f, 0.36f, 0.08f, 0.f,    0.02f, 0.08f, 1,  0.8f, { 1.3f, 1.3f, 1.1f, 0.9f, 0.8f, 0.6f, 0.6f, 0.7f, 0.9f, 1.2f, 1.4f, 1.4f }, false },
        { TEXT("elektronik"), TEXT("Elektronik ve beyaz e\u015fya"), false, false, 3, 10, 0.24f, 0.20f, 0.f, 0.015f, 0.04f, 0.05f, 60, 1.5f, { 0.9f, 0.8f, 0.9f, 0.9f, 1.f, 1.f, 0.9f, 0.9f, 0.9f, 1.f, 1.8f, 1.4f }, false },
        { TEXT("giyim"),      TEXT("Giyim ve ev tekstili"),  false, false, 3, 10, 0.11f, 0.45f, 0.f,   0.02f,  0.02f, 0.05f, 90, 1.2f, { 0.9f, 0.7f, 1.1f, 1.2f, 1.1f, 0.9f, 0.9f, 0.8f, 1.3f, 1.3f, 1.1f, 1.1f }, true },
        { TEXT("ev"),         TEXT("Ev ve mutfak"),          false, false, 3, 8,  0.07f, 0.36f, 0.f,   0.01f,  0.01f, 0.03f, 75, 1.0f, { 0.9f, 0.9f, 1.f, 1.f, 1.1f, 1.1f, 1.f, 1.f, 1.1f, 1.f, 1.f, 1.2f }, false },
        { TEXT("oyuncak"),    TEXT("Oyuncak"),               false, false, 3, 5,  0.035f, 0.36f, 0.f,  0.015f, 0.01f, 0.03f, 90, 1.0f, { 0.6f, 0.6f, 0.7f, 0.8f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 1.f, 1.3f, 2.6f }, false },
        { TEXT("kirtasiye"),  TEXT("K\u0131rtasiye ve kitap"), false, false, 3, 4, 0.03f, 0.33f, 0.f,  0.01f,  0.01f, 0.02f, 60, 0.6f, { 0.8f, 0.7f, 0.6f, 0.6f, 0.5f, 0.5f, 0.6f, 2.2f, 2.8f, 0.9f, 0.7f, 0.7f }, false },
        { TEXT("bebek"),      TEXT("Bebek"),                 false, false, 2, 4,  0.05f, 0.25f, 0.f,   0.005f, 0.02f, 0.02f, 30, 0.5f, { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f }, false },
        { TEXT("evcil"),      TEXT("Evcil hayvan"),          false, false, 2, 3,  0.02f, 0.28f, 0.f,   0.005f, 0.01f, 0.02f, 45, 1.0f, { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f }, false },
        { TEXT("bahce"),      TEXT("Bah\u00e7e ve oto"),     false, false, 3, 6,  0.035f, 0.30f, 0.f,  0.01f,  0.01f, 0.03f, 90, 1.1f, { 0.6f, 0.6f, 1.f, 1.5f, 1.7f, 1.5f, 1.2f, 1.f, 0.9f, 0.8f, 0.8f, 0.8f }, false },
        { TEXT("mevsimlik"),  TEXT("Mevsimlik"),             false, false, 3, 5,  0.035f, 0.35f, 0.f,  0.01f,  0.02f, 0.03f, 45, 0.8f, { 0.8f, 0.6f, 0.8f, 1.f, 1.2f, 1.5f, 1.5f, 1.2f, 0.9f, 0.8f, 1.f, 1.6f }, true },
    };

    const TCHAR* StanceNames[3] = { TEXT("ucuz"), TEXT("normal"), TEXT("pahal\u0131") };
    const float StanceDemand[3] = { 1.15f, 1.f, 0.88f };
    const float StanceMargin[3] = { -0.06f, 0.f, 0.05f };

    uint32 Mix(int32 Seed, int32 Day, uint32 Salt)
    {
        uint32 H = static_cast<uint32>(Seed) * 0x9E3779B1u ^ static_cast<uint32>(Day) * 0x85EBCA77u ^ Salt * 0xC2B2AE3Du;
        H ^= H >> 15; H *= 0x2C1B3C6Du; H ^= H >> 12; H *= 0x297A2D39u; H ^= H >> 15;
        return H;
    }

    void Fit(FMarketState& State)
    {
        FMarketDepartmentsState& S = State.Departments;
        if (S.Policy.Num() != MarketDepartments::FormatCount * MarketDepartments::DeptCount) S.Policy.SetNumZeroed(MarketDepartments::FormatCount * MarketDepartments::DeptCount);
        if (S.Stance.Num() != MarketDepartments::DeptCount) { S.Stance.Init(1, MarketDepartments::DeptCount); }
        if (S.LastMonthDay == 0) S.LastMonthDay = FMath::Max(1, State.Day);
    }

    int32 PolicyIndex(EDept Dept, int32 Format) { return Format * MarketDepartments::DeptCount + static_cast<int32>(Dept); }

    const TCHAR* FormatShort(int32 Format)
    {
        return Format == 0 ? TEXT("Ucuzcu") : Format == 1 ? TEXT("Mahalle") : Format == 2 ? TEXT("S\u00fcpermarket") : TEXT("Hipermarket");
    }

    int32 Staff(EDept Dept, int32 Format)
    {
        // Small non-food aisles share the floor staff in a supermarket and need one hand in a hypermarket.
        if (Dept == EDept::Pets) return 0; // C3 (A6): the pet aisle shares the floor staff everywhere
        if (!MarketDepartments::Info(Dept).bFresh && MarketDepartments::Info(Dept).Ratio < 0.05f) return Format >= 3 ? 1 : 0;
        if (Format >= 3) return Dept == EDept::Electronics || Dept == EDept::Clothing ? 3 : 2;
        return 1;
    }

    int64 MasterWage(int32 Skill, int32 Day) { return FMath::RoundToInt64(MarketStaff::FairWage(MarketStaff::ERole::Cashier, Skill, Day) * 1.5); }

    int32 RollMaster(const FMarketState& State, int32 Branch, EDept Dept, int32 Low, int32 High)
    {
        return Low + static_cast<int32>(Mix(State.RivalSeed, State.Day, 0xA570u + Branch * 31u + static_cast<uint32>(Dept)) % static_cast<uint32>(High - Low + 1));
    }

    float Quality(const FMarketBranchDept& D)
    {
        return MarketDepartments::Info(static_cast<EDept>(D.Dept)).bMaster ? 0.75f + 0.5f * FMath::Clamp(D.Master, 0, 100) / 100.f : 1.f;
    }

    float Ramp(const FMarketBranchDept& D, int32 Day)
    {
        return FMath::Min(1.f, 0.4f + 0.6f * FMath::Max(0, Day - D.OpenedDay) / static_cast<float>(MarketDepartments::RampDays));
    }

    // Expected daily cost of goods of a department in a branch (for the opening stock).
    int64 DailyCogs(const FMarketBranch& B, EDept Dept, int32 Day)
    {
        const FInfo& I = MarketDepartments::Info(Dept);
        const int32 Shoppers = B.LastShoppers > 0 ? B.LastShoppers : FMath::Max(30, MarketBranches::FormatInfo(B.Format).Trips / 5);
        return FMath::RoundToInt64(Shoppers * MarketDepartments::Ticket2011 * MarketPrices::ListLevel(Day) * I.Ratio * (1.f - I.Margin));
    }

    // Opens a department in a branch if the till allows. Returns the cost paid (-1 when it could not).
    int64 OpenIn(FMarketState& State, int32 Index, EDept Dept)
    {
        FMarketBranch& B = State.Branches[Index];
        const int32 Format = MarketDepartments::FormatIndex(B.Format);
        const int64 FitOut = MarketDepartments::FitOutCost(Dept, Format, State.Day);
        const int64 Stock = DailyCogs(B, Dept, State.Day) * MarketDepartments::Info(Dept).StockDays;
        if (FitOut + Stock > State.Cash) return -1;
        FMarketBranchDept& D = B.Depts.AddDefaulted_GetRef();
        D.Dept = static_cast<uint8>(Dept);
        D.OpenedDay = State.Day;
        D.Master = MarketDepartments::Info(Dept).bMaster ? RollMaster(State, Index, Dept, 35, 80) : 0;
        D.Stock = Stock;
        // C3: paid now and booked line by line (not through the family shop's counters).
        State.Cash -= FitOut + Stock;
        State.Books.PeriodPurchases += Stock; // VAT paid on the goods
        MarketLedger::Post(State, MarketLedger::EAccount::DepartmentFitOut, -FitOut, true, Index);
        MarketLedger::Post(State, MarketLedger::EAccount::DepartmentPurchases, -Stock, true, Index);
        return FitOut + Stock;
    }

    int64 CloseIn(FMarketState& State, FMarketBranch& B, int32 At)
    {
        const int64 Refund = FMath::RoundToInt64(B.Depts[At].Stock * MarketDepartments::RefundShare);
        State.Cash += Refund;
        const int32 Store = static_cast<int32>(&B - State.Branches.GetData());
        MarketLedger::Post(State, MarketLedger::EAccount::DepartmentClearance, Refund, true, Store);
        MarketLedger::Post(State, MarketLedger::EAccount::DepartmentCostOfGoods, -B.Depts[At].Stock, false, Store);
        B.Depts.RemoveAt(At);
        return Refund;
    }

    bool IsOpenBranch(const FMarketBranch& B) { return B.Stage == static_cast<uint8>(MarketBranches::EStage::Open); }

    // Brings every open branch in line with the policies. Returns (opened, closed, cost, refund, short of cash).
    struct FSync { int32 Opened = 0; int32 Closed = 0; int64 Cost = 0; int64 Refund = 0; int32 Short = 0; };
    FSync Sync(FMarketState& State)
    {
        FSync R;
        for (int32 Index = 0; Index < State.Branches.Num(); ++Index)
        {
            FMarketBranch& B = State.Branches[Index];
            if (!IsOpenBranch(B)) continue;
            const int32 Format = MarketDepartments::FormatIndex(B.Format);
            for (int32 At = B.Depts.Num() - 1; At >= 0; --At)
                if (!MarketDepartments::IsOn(State, static_cast<EDept>(B.Depts[At].Dept), Format)) { R.Refund += CloseIn(State, B, At); ++R.Closed; }
            for (int32 Dx = 0; Dx < MarketDepartments::DeptCount; ++Dx)
            {
                const EDept Dept = static_cast<EDept>(Dx);
                if (!MarketDepartments::IsOn(State, Dept, Format)) continue;
                if (B.Depts.ContainsByPredicate([Dx](const FMarketBranchDept& D) { return D.Dept == Dx; })) continue;
                const int64 Paid = OpenIn(State, Index, Dept);
                if (Paid < 0) { ++R.Short; continue; }
                R.Cost += Paid;
                ++R.Opened;
            }
        }
        return R;
    }

    FString Percent(float Value) { return FString::Printf(TEXT("%%%.0f"), Value * 100.f); }
}

const MarketDepartments::FInfo& MarketDepartments::Info(EDept Dept)
{
    return MarketDepartmentsLocal::Table[FMath::Clamp(static_cast<int32>(Dept), 0, DeptCount - 1)];
}

FString MarketDepartments::Name(EDept Dept) { return Info(Dept).Name; }

int32 MarketDepartments::FormatIndex(const FString& InFormat)
{
    const FString Format = MarketBranches::BaseFormat(InFormat); // M54: a convenience store as a discounter, a cash-and-carry as a hypermarket
    if (Format == TEXT("kucuk")) return 0;
    if (Format == TEXT("buyuk")) return 2;
    if (Format == TEXT("hiper")) return 3;
    return 1;
}

int32 MarketDepartments::SpaceCap(int32 Format)
{
    return Format == 1 ? 10 : Format == 2 ? 22 : Format == 3 ? 70 : 0;
}

int32 MarketDepartments::SpaceUsed(const FMarketState& State, int32 Format)
{
    int32 Used = 0;
    for (int32 Dx = 0; Dx < DeptCount; ++Dx) if (IsOn(State, static_cast<EDept>(Dx), Format)) Used += Info(static_cast<EDept>(Dx)).Space;
    return Used;
}

bool MarketDepartments::IsOn(const FMarketState& State, EDept Dept, int32 Format)
{
    const int32 I = MarketDepartmentsLocal::PolicyIndex(Dept, Format);
    return State.Departments.Policy.IsValidIndex(I) && State.Departments.Policy[I] != 0;
}

bool MarketDepartments::CanSet(const FMarketState& State, EDept Dept, int32 Format, bool bOn, FString& OutReason)
{
    if (static_cast<int32>(Dept) >= DeptCount || Format < 0 || Format >= FormatCount) { OutReason = TEXT("B\u00f6yle bir reyon yok."); return false; }
    if (IsOn(State, Dept, Format) == bOn) { OutReason = bOn ? TEXT("Bu reyon zaten a\u00e7\u0131k.") : TEXT("Bu reyon zaten kapal\u0131."); return false; }
    if (!bOn) return true;
    const FInfo& I = Info(Dept);
    if (Format < I.MinFormat)
    {
        OutReason = FString::Printf(TEXT("%s en az %s ister."), I.Name, MarketDepartmentsLocal::FormatShort(I.MinFormat));
        return false;
    }
    if (SpaceUsed(State, Format) + I.Space > SpaceCap(Format))
    {
        OutReason = FString::Printf(TEXT("%s alan\u0131 dolu: reyonlar %%%d, s\u0131n\u0131r %%%d. Bu reyon %%%d ister; \u00f6nce ba\u015fka bir reyonu kapat."),
            MarketDepartmentsLocal::FormatShort(Format), SpaceUsed(State, Format), SpaceCap(Format), I.Space);
        return false;
    }
    return true;
}

bool MarketDepartments::Set(FMarketState& State, EDept Dept, int32 Format, bool bOn, FString& OutMessage)
{
    if (!CanSet(State, Dept, Format, bOn, OutMessage)) return false;
    MarketDepartmentsLocal::Fit(State);
    State.Departments.Policy[MarketDepartmentsLocal::PolicyIndex(Dept, Format)] = bOn ? 1 : 0;
    const MarketDepartmentsLocal::FSync R = MarketDepartmentsLocal::Sync(State);
    if (bOn)
    {
        OutMessage = FString::Printf(TEXT("%s reyonu %s ma\u011fazalarda a\u00e7\u0131l\u0131yor"), Info(Dept).Name, MarketDepartmentsLocal::FormatShort(Format));
        OutMessage += R.Opened > 0 ? FString::Printf(TEXT(": %d \u015fubede a\u00e7\u0131ld\u0131 (tadilat ve stok %s)."), R.Opened, *MarketCountry::Money(R.Cost))
            : TEXT(": bu t\u00fcrde a\u00e7\u0131k \u015fube yok, yeni a\u00e7\u0131lanlar bu reyonla a\u00e7\u0131l\u0131r.");
        if (R.Short > 0) OutMessage += FString::Printf(TEXT(" %d \u015fubede kasa yetmedi; para gelince a\u00e7\u0131l\u0131r."), R.Short);
    }
    else OutMessage = FString::Printf(TEXT("%s reyonu %s ma\u011fazalarda kapand\u0131 (%d \u015fube). Stok %s kar\u015f\u0131l\u0131\u011f\u0131 elden \u00e7\u0131kar\u0131ld\u0131."),
        Info(Dept).Name, MarketDepartmentsLocal::FormatShort(Format), R.Closed, *MarketCountry::Money(R.Refund));
    return true;
}

int32 MarketDepartments::Stance(const FMarketState& State, EDept Dept)
{
    const int32 I = static_cast<int32>(Dept);
    return State.Departments.Stance.IsValidIndex(I) ? FMath::Clamp<int32>(State.Departments.Stance[I], 0, 2) : 1;
}

bool MarketDepartments::SetStance(FMarketState& State, EDept Dept, int32 NewStance, FString& OutMessage)
{
    if (static_cast<int32>(Dept) >= DeptCount || NewStance < 0 || NewStance > 2) { OutMessage = TEXT("B\u00f6yle bir se\u00e7enek yok."); return false; }
    MarketDepartmentsLocal::Fit(State);
    State.Departments.Stance[static_cast<int32>(Dept)] = static_cast<uint8>(NewStance);
    OutMessage = FString::Printf(TEXT("%s reyonunda fiyatlar art\u0131k %s."), Info(Dept).Name, MarketDepartmentsLocal::StanceNames[NewStance]);
    return true;
}

int32 MarketDepartments::WeakMasters(const FMarketState& State, EDept Dept)
{
    int32 Count = 0;
    for (const FMarketBranch& B : State.Branches)
        for (const FMarketBranchDept& D : B.Depts) if (D.Dept == static_cast<uint8>(Dept) && Info(Dept).bMaster && D.Master < 50) ++Count;
    return Count;
}

bool MarketDepartments::ReplaceWeakMasters(FMarketState& State, EDept Dept, FString& OutMessage)
{
    if (static_cast<int32>(Dept) >= DeptCount) { OutMessage = TEXT("B\u00f6yle bir reyon yok."); return false; }
    const int32 Weak = WeakMasters(State, Dept);
    if (Weak == 0) { OutMessage = TEXT("Zay\u0131f usta yok."); return false; }
    const int64 Cost = Weak * 7 * MarketDepartmentsLocal::MasterWage(50, State.Day);
    if (Cost > State.Cash) { OutMessage = FString::Printf(TEXT("Kasa yetmiyor: %s gerekir."), *MarketCountry::Money(Cost)); return false; }
    int32 Better = 0;
    for (int32 Index = 0; Index < State.Branches.Num(); ++Index)
        for (FMarketBranchDept& D : State.Branches[Index].Depts)
        {
            if (D.Dept != static_cast<uint8>(Dept) || D.Master >= 50) continue;
            D.Master = MarketDepartmentsLocal::RollMaster(State, Index, Dept, 50, 88);
            if (D.Master >= 60) ++Better;
        }
    State.Cash -= Cost;
    MarketLedger::Post(State, MarketLedger::EAccount::DepartmentMaster, -Cost, true, MarketLedger::HeadOfficeStore);
    OutMessage = FString::Printf(TEXT("%s: %d usta de\u011fi\u015fti (%s). Yenilerin %d tanesi i\u015fini iyi biliyor."), Info(Dept).Name, Weak, *MarketCountry::Money(Cost), Better);
    return true;
}

int32 MarketDepartments::EncodeSet(EDept Dept, int32 Format, bool bOn) { return static_cast<int32>(Dept) * 100 + Format * 10 + (bOn ? 1 : 0); }

bool MarketDepartments::DecodeSet(int32 Arg, EDept& OutDept, int32& OutFormat, bool& bOutOn)
{
    if (Arg < 0 || Arg / 100 >= DeptCount || (Arg / 10) % 10 >= FormatCount || Arg % 10 > 1) return false;
    OutDept = static_cast<EDept>(Arg / 100);
    OutFormat = (Arg / 10) % 10;
    bOutOn = Arg % 10 == 1;
    return true;
}

namespace MarketDepartmentsLocal
{
    // D3: the feast whose eve week is the butcher's peak in the active country (ulkeler.json "butcherPeak").
    const MarketCountry::FHoliday* ButcherFeast()
    {
        return MarketCountry::Active().Holidays.FindByPredicate([](const MarketCountry::FHoliday& H) { return H.bButcherPeak; });
    }
}
float MarketDepartments::SeasonFactor(EDept Dept, int32 GameDay)
{
    const MarketCalendar::FDate Date = MarketCalendar::DateOf(GameDay);
    float Factor = Info(Dept).Season[FMath::Clamp(Date.Month, 1, 12) - 1];
    if (Dept == EDept::Butcher)
    {
        // D3: the pack's butcher feast (Turkey: the week before Kurban Bayram\u0131, meat for the feast).
        const MarketCountry::FHoliday* Feast = MarketDepartmentsLocal::ButcherFeast();
        if (Feast)
        {
            const int32 Start = MarketCalendar::HolidayStart(*Feast, Date.Year);
            const int32 Until = Start == MIN_int32 ? -1 : Start - GameDay;
            if (Until >= 0 && Until <= 6) Factor *= 1.8f;
        }
        else if (Date.Month == 12 && Date.Day >= 18 && Date.Day <= 24) Factor *= 1.5f; // M35: the Christmas roast elsewhere
    }
    return Factor;
}

int64 MarketDepartments::FitOutCost(EDept Dept, int32 Format, int32 GameDay)
{
    const FString& Id = MarketBranches::FormatIds()[FMath::Clamp(Format, 0, FormatCount - 1)];
    return FMath::RoundToInt64(MarketBranches::FormatInfo(Id).FitOut * Info(Dept).FitOut * MarketPrices::ListLevel(GameDay));
}

float MarketDepartments::PullFactor(const FMarketBranch& Branch, int32 GameDay)
{
    float Pull = 1.f;
    for (const FMarketBranchDept& D : Branch.Depts)
        Pull += Info(static_cast<EDept>(D.Dept)).Pull * MarketDepartmentsLocal::Quality(D) * MarketDepartmentsLocal::Ramp(D, GameDay);
    return Pull;
}

MarketDepartments::FDay MarketDepartments::Day(FMarketState& State, int32 BranchIndex, int32 Shoppers, float Income, int32 Closed)
{
    FDay Out;
    if (!State.Branches.IsValidIndex(BranchIndex)) return Out;
    FMarketBranch& B = State.Branches[BranchIndex];
    const int32 Format = FormatIndex(B.Format);
    const double Ticket = Ticket2011 * MarketPrices::ListLevel(Closed);
    const float Buying = 1.f - MarketSourcing::VolumeDiscount(State);
    const int32 Month = MarketCalendar::DateOf(Closed).Month;
    const FString Country = MarketBranches::CountryOf(State, B);
    for (FMarketBranchDept& D : B.Depts)
    {
        const EDept Dept = static_cast<EDept>(D.Dept);
        const FInfo& I = Info(Dept);
        const int32 S = Stance(State, Dept);
        const float Quality = MarketDepartmentsLocal::Quality(D);
        // C3 (B7): the era moves non-food and fresh demand; a currency shock makes imported goods dearer.
        const MarketEras::EGoods Goods = MarketEras::GoodsOf(I.Id);
        const float Era = MarketEras::DemandFactor(State, Goods, Closed, Country);
        const float Import = Goods == MarketEras::EGoods::Grocery || Goods == MarketEras::EGoods::Fresh ? 1.f : MarketEras::ImportCostFactor(State, Goods, Closed, Country);
        const float Demand = Shoppers * I.Ratio * SeasonFactor(Dept, Closed) * FMath::Pow(FMath::Max(0.3f, Income), I.IncomeElastic)
            * MarketDepartmentsLocal::StanceDemand[S] * Quality * MarketDepartmentsLocal::Ramp(D, Closed) * Era;
        const int64 Revenue = FMath::RoundToInt64(Demand * Ticket);
        const bool bClearance = I.bClearance && (Month == 1 || Month == 7);
        const float Margin = FMath::Clamp((I.Margin + MarketDepartmentsLocal::StanceMargin[S]) * (bClearance ? 0.6f : 1.f), 0.02f, 0.8f);
        const int64 Cogs = FMath::RoundToInt64(Revenue * (1.f - Margin) * Buying * Import);
        // A master keeps waste down: a middling one (60) at the table's rate.
        const float WasteRate = I.Waste * (I.bMaster ? FMath::Clamp(1.9f - 1.5f * D.Master / 100.f, 0.5f, 1.6f) : 1.f);
        const int64 Waste = FMath::RoundToInt64(Revenue * WasteRate * (1.f - Margin));
        const int64 Shrink = FMath::RoundToInt64(Revenue * I.Shrink * (1.f - Margin));
        const int32 Staff = MarketDepartmentsLocal::Staff(Dept, Format);
        const int64 Wages = I.bMaster ? MarketDepartmentsLocal::MasterWage(D.Master, Closed) + (Staff - 1) * MarketStaff::FairWage(MarketStaff::ERole::Cashier, 50, Closed)
            : Staff * MarketStaff::FairWage(MarketStaff::ERole::Cashier, 50, Closed);
        const int64 Social = MarketStaff::EmployerShare(Wages); // B3: the employer's social security share
        const int64 Profit = Revenue - Cogs - Waste - Shrink - Wages - Social;
        D.Last30Revenue = D.Last30Revenue * 29 / 30 + Revenue;
        D.Last30Profit = D.Last30Profit * 29 / 30 + Profit;
        // The stock follows the season's pace: what was sold, spoiled or taken is bought again, plus the change of
        // the stock held (a sell-down buys less).
        const int64 NewStock = FMath::Max<int64>(0, FMath::RoundToInt64(FMath::Lerp(static_cast<double>(D.Stock), static_cast<double>(Cogs) * I.StockDays, 0.05)));
        const int64 Bought = FMath::Max<int64>(0, Cogs + Waste + Shrink + NewStock - D.Stock);
        D.Stock = NewStock;
        // C3 (B7.2): the department's day line by line; the cash moves once with the branch's day.
        MarketLedger::Post(State, MarketLedger::EAccount::DepartmentSales, Revenue, true, BranchIndex);
        MarketLedger::Post(State, MarketLedger::EAccount::DepartmentPurchases, -Bought, true, BranchIndex);
        MarketLedger::Post(State, MarketLedger::EAccount::Wages, -Wages, true, BranchIndex);
        MarketLedger::Post(State, MarketLedger::EAccount::SocialSecurity, -Social, true, BranchIndex);
        MarketLedger::Post(State, MarketLedger::EAccount::DepartmentCostOfGoods, -Cogs, false, BranchIndex);
        MarketLedger::Post(State, MarketLedger::EAccount::DepartmentWaste, -(Waste + Shrink), false, BranchIndex);
        Out.Revenue += Revenue;
        Out.Profit += Profit;
        Out.Cash += Revenue - Bought - Wages - Social;
        Out.Purchases += Bought;
    }
    return Out;
}

FString MarketDepartments::Describe(EDept Dept)
{
    const FInfo& I = Info(Dept);
    FString Line = FString::Printf(TEXT("%s \u00b7 br\u00fct k\u00e2r %s \u00b7 alan %%%d"), I.bFresh ? TEXT("Taze") : TEXT("G\u0131da d\u0131\u015f\u0131"), *MarketDepartmentsLocal::Percent(I.Margin), I.Space);
    if (I.bMaster) Line += TEXT(" \u00b7 usta ister");
    if (I.Waste >= 0.05f) Line += TEXT(" \u00b7 fire y\u00fcksek");
    if (I.Pull >= 0.05f) Line += TEXT(" \u00b7 m\u00fc\u015fteri \u00e7eker");
    if (I.StockDays >= 60) Line += FString::Printf(TEXT(" \u00b7 %d g\u00fcnl\u00fck stok"), I.StockDays);
    if (I.bClearance) Line += TEXT(" \u00b7 ocak ve temmuzda indirim sezonu");
    int32 Peak = 0;
    for (int32 M = 1; M < 12; ++M) if (I.Season[M] > I.Season[Peak]) Peak = M;
    if (I.Season[Peak] >= 1.4f)
    {
        static const TCHAR* Months[12] = { TEXT("ocak"), TEXT("\u015fubat"), TEXT("mart"), TEXT("nisan"), TEXT("may\u0131s"), TEXT("haziran"), TEXT("temmuz"), TEXT("a\u011fustos"), TEXT("eyl\u00fcl"), TEXT("ekim"), TEXT("kas\u0131m"), TEXT("aral\u0131k") };
        Line += FString::Printf(TEXT(" \u00b7 zirve: %s"), Months[Peak]);
    }
    if (Dept == EDept::Butcher)
    {
        const MarketCountry::FHoliday* Feast = MarketDepartmentsLocal::ButcherFeast(); // D3
        Line += Feast ? TEXT(" \u00b7 zirve: ") + Feast->Name + TEXT(" \u00f6ncesi") : FString(TEXT(" \u00b7 zirve: y\u0131l sonu bayramlar\u0131 \u00f6ncesi"));
    }
    return Line;
}

int32 MarketDepartments::BranchesWith(const FMarketState& State, EDept Dept)
{
    int32 Count = 0;
    for (const FMarketBranch& B : State.Branches)
        if (B.Depts.ContainsByPredicate([Dept](const FMarketBranchDept& D) { return D.Dept == static_cast<uint8>(Dept); })) ++Count;
    return Count;
}

FString MarketDepartments::Results(const FMarketState& State, EDept Dept)
{
    int64 Revenue = 0, Profit = 0;
    int32 Count = 0, MasterSum = 0;
    for (const FMarketBranch& B : State.Branches)
        for (const FMarketBranchDept& D : B.Depts)
        {
            if (D.Dept != static_cast<uint8>(Dept)) continue;
            Revenue += D.Last30Revenue; Profit += D.Last30Profit; MasterSum += D.Master; ++Count;
        }
    if (Count == 0) return TEXT("hi\u00e7bir \u015fubede yok");
    FString Line = FString::Printf(TEXT("%d \u015fube \u00b7 30 g\u00fcn ciro %s, net %s"), Count, *MarketCountry::Money(Revenue), *MarketCountry::Money(Profit));
    if (Info(Dept).bMaster) Line += FString::Printf(TEXT(" \u00b7 usta ort. %d"), MasterSum / Count);
    return Line;
}

FString MarketDepartments::BranchLine(const FMarketBranch& Branch)
{
    FString Line;
    for (const FMarketBranchDept& D : Branch.Depts)
    {
        if (!Line.IsEmpty()) Line += TEXT(", ");
        Line += Info(static_cast<EDept>(D.Dept)).Name;
    }
    return Line;
}

void MarketDepartments::CloseDay(FMarketState& State)
{
    MarketDepartmentsLocal::Fit(State);
    const MarketDepartmentsLocal::FSync R = MarketDepartmentsLocal::Sync(State);
    if (R.Opened > 0)
        State.DayNews.Add(FString::Printf(TEXT("Reyonlar: %d yeni reyon a\u00e7\u0131ld\u0131 (%s)."), R.Opened, *MarketCountry::Money(R.Cost)));
    FMarketDepartmentsState& S = State.Departments;
    if (State.Day - S.LastMonthDay < 30) return;
    S.LastMonthDay = State.Day;
    // A month: masters learn a little; weak ones are named once a month.
    for (FMarketBranch& B : State.Branches)
        for (FMarketBranchDept& D : B.Depts)
            if (Info(static_cast<EDept>(D.Dept)).bMaster && D.Master < 90) ++D.Master;
    for (int32 Dx = 0; Dx < DeptCount; ++Dx)
    {
        const int32 Weak = WeakMasters(State, static_cast<EDept>(Dx));
        if (Weak > 0)
            State.DayNews.Add(FString::Printf(TEXT("%s: %d \u015fubede usta zay\u0131f; m\u00fc\u015fteri az geliyor, fire y\u00fcksek. \u015eirket \u203a Reyonlar'dan de\u011fi\u015ftirebilirsin."), Info(static_cast<EDept>(Dx)).Name, Weak));
    }
    if (R.Short > 0)
        State.DayNews.Add(FString::Printf(TEXT("Reyonlar: %d \u015fubede kasa yetmedi\u011fi i\u00e7in reyon a\u00e7\u0131lamad\u0131."), R.Short));
}

int64 MarketDepartments::StockValue(const FMarketState& State)
{
    int64 Value = 0;
    for (const FMarketBranch& B : State.Branches) for (const FMarketBranchDept& D : B.Depts) Value += D.Stock;
    return Value;
}

void MarketDepartments::CloseAll(FMarketState& State, int32 BranchIndex)
{
    if (!State.Branches.IsValidIndex(BranchIndex)) return;
    FMarketBranch& B = State.Branches[BranchIndex];
    for (int32 At = B.Depts.Num() - 1; At >= 0; --At) MarketDepartmentsLocal::CloseIn(State, B, At);
}

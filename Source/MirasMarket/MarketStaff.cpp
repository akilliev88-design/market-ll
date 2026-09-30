#include "MarketStaff.h"
#include "MarketCountry.h"
#include "StaffPlanner.h"
#include "MarketPrices.h"

// Internal rules (not in the header). Inside namespace MarketStaff so the MarketStaff:: definitions below find them
// without a using-directive (the module is a unity build).
namespace MarketStaff
{
    // Stable 32-bit hash of (seed, day, salt), as in MarketRivals: a reload never rerolls people or tills.
    uint32 Mix(int32 Seed, int32 Day, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), static_cast<uint32>(Day), Salt };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }

    int64 Round50(double Kurus) { return FMath::Max<int64>(50, static_cast<int64>(FMath::RoundToDouble(Kurus / 50.0)) * 50); }

    FString Tl(int64 Kurus)
    {
        return MarketCountry::Money(Kurus); // G-084: the active country\'s currency
    }

    const TCHAR* FirstNames[] = { TEXT("Ay\u015fe"), TEXT("Mehmet"), TEXT("Fatma"), TEXT("Emre"), TEXT("Zeynep"), TEXT("Burak"), TEXT("Elif"), TEXT("Hasan"),
        TEXT("Merve"), TEXT("Murat"), TEXT("Esra"), TEXT("Serkan"), TEXT("Derya"), TEXT("Onur"), TEXT("G\u00fcl"), TEXT("Kemal"), TEXT("Selin"), TEXT("Hakan"),
        TEXT("B\u00fc\u015fra"), TEXT("Cem"), TEXT("H\u00fclya"), TEXT("Tolga"), TEXT("Sevgi"), TEXT("Yusuf") };
    const TCHAR* Surnames[] = { TEXT("Kaya"), TEXT("Y\u0131ld\u0131z"), TEXT("Demir"), TEXT("\u00c7elik"), TEXT("\u015eahin"), TEXT("Ayd\u0131n"), TEXT("\u00d6zt\u00fcrk"), TEXT("Arslan"),
        TEXT("Do\u011fan"), TEXT("Ko\u00e7"), TEXT("K\u0131l\u0131\u00e7"), TEXT("Aslan"), TEXT("Polat"), TEXT("Erdem") };

    bool WorksOn(const FMarketEmployee& Employee, int32 Day)
    {
        return Employee.HiredDay <= Day && Employee.OffDay != Day;
    }

    bool IsShopRole(MarketStaff::ERole Role)
    {
        return Role == MarketStaff::ERole::Cashier || Role == MarketStaff::ERole::Stocker;
    }

    int32 RoleLimit(MarketStaff::ERole Role)
    {
        switch (Role)
        {
        case MarketStaff::ERole::Cashier: return MarketStaff::MaxCashiers;
        case MarketStaff::ERole::Stocker: return FMarketState::MaxStockers;
        default: return 1;
        }
    }

    // One candidate. Slot 0 is always a cashier, slot 1 a stocker; slot 2 an HR manager while one is wanted.
    FMarketEmployee MakeCandidate(FMarketState& State, int32 Slot)
    {
        const uint32 A = Mix(State.RivalSeed ^ 0x51AFF, State.Day, static_cast<uint32>(State.NextEmployeeId) * 7919u + static_cast<uint32>(Slot));
        const uint32 B = Mix(State.RivalSeed ^ 0x2A9E1, State.Day, A);
        MarketStaff::ERole Role = Slot == 0 ? MarketStaff::ERole::Cashier : MarketStaff::ERole::Stocker;
        if (Slot == 2 && MarketStaff::HrUnlocked(State) && !MarketStaff::HasHr(State)) Role = MarketStaff::ERole::HrManager;
        else if (Slot >= 2) Role = (A >> 5) % 2 == 0 ? MarketStaff::ERole::Cashier : MarketStaff::ERole::Stocker;
        FMarketEmployee C;
        C.Id = State.NextEmployeeId++;
        C.Role = static_cast<uint8>(Role);
        // G-084: the country pack's name pools when it has them (Turkey keeps the built-in lists).
        const MarketCountry::FProfile& Country = MarketCountry::Active();
        if (Country.Id != TEXT("tr") && Country.FirstNames.Num() > 0 && Country.LastNames.Num() > 0)
            C.Name = Country.FirstNames[A % static_cast<uint32>(Country.FirstNames.Num())] + TEXT(" ") + Country.LastNames[B % static_cast<uint32>(Country.LastNames.Num())];
        else
            C.Name = FString(FirstNames[A % UE_ARRAY_COUNT(FirstNames)]) + TEXT(" ") + Surnames[B % UE_ARRAY_COUNT(Surnames)];
        C.Skill = 20 + static_cast<int32>(A % 61u);
        C.Speed = 25 + static_cast<int32>((A >> 8) % 66u);
        C.Stamina = 25 + static_cast<int32>((A >> 16) % 66u);
        // Most people are honest; about one in eight is not (never visible, see the accountant).
        const int32 Trust = static_cast<int32>((B >> 8) % 100u);
        C.Honesty = Trust < 12 ? 15 + Trust : 55 + Trust % 45;
        C.DailyWage = Round50(MarketStaff::FairWage(Role, C.Skill, State.Day) * (0.90 + ((B >> 16) % 26u) / 100.0));
        C.Morale = 70.f;
        return C;
    }

    void RefreshPool(FMarketState& State)
    {
        State.Candidates.Reset();
        State.CandidatesDay = State.Day;
        const int32 Size = MarketStaff::HasHr(State) ? MarketStaff::HrPoolSize : MarketStaff::PoolSize;
        for (int32 Slot = 0; Slot < Size; ++Slot) State.Candidates.Add(MakeCandidate(State, Slot));
    }

    // Visible without HR: a rough band.
    const TCHAR* Band(int32 Value)
    {
        return Value < 40 ? TEXT("az") : Value < 65 ? TEXT("orta") : TEXT("iyi");
    }

    // HR's reference note. Honest people always look fine; a dishonest one is caught by references 70 % of the time.
    bool ReferenceWarns(const FMarketEmployee& C)
    {
        return C.Honesty < 35 && static_cast<uint32>(C.Id) * 2654435761u % 10u < 7u;
    }

    const TCHAR* Reference(const FMarketEmployee& C)
    {
        if (ReferenceWarns(C)) return TEXT("zay\u0131f, dikkat");
        return C.Honesty >= 35 && C.Honesty < 60 ? TEXT("normal") : TEXT("sa\u011flam");
    }

    int32 IndexOf(const FMarketState& State, int32 Id)
    {
        return State.Staff.IndexOfByPredicate([Id](const FMarketEmployee& E) { return E.Id == Id; });
    }

    // Till difference of one cashier for one worked day: honest mistakes both ways (skill, fatigue), and for a
    // dishonest one a rare small shortage. Warnings deter it for a week.
    int64 TillDifference(const FMarketState& State, const FMarketEmployee& Cashier, int32 Day, int32 Served, int64 Revenue)
    {
        FRandomStream Random(static_cast<int32>(Mix(State.RivalSeed, Day, 0x7111u + static_cast<uint32>(Cashier.Id) * 31u)));
        const float ErrorRate = 0.02f + (100 - Cashier.Skill) * 0.0006f + Cashier.Fatigue * 0.0004f;
        int64 Difference = 0;
        for (int32 Customer = 0; Customer < Served; ++Customer)
        {
            if (Random.FRand() >= ErrorRate) continue;
            const int64 Amount = Random.RandRange(25, 200);
            Difference += Random.FRand() < 0.6f ? -Amount : Amount; // too much change is the usual mistake
        }
        if (Cashier.Honesty < 35 && Served > 0)
        {
            float Chance = (35 - Cashier.Honesty) / 35.f * 0.5f;
            if (Cashier.WarnedDay > 0 && Day - Cashier.WarnedDay < 7) Chance *= 0.3f;
            if (Random.FRand() < Chance)
                Difference -= FMath::Clamp<int64>(static_cast<int64>(Revenue * Random.FRandRange(0.01f, 0.03f)), 100, 2000);
        }
        return Difference;
    }
}

FString MarketStaff::RoleName(ERole Role)
{
    switch (Role)
    {
    case ERole::Cashier: return TEXT("Kasiyer");
    case ERole::Stocker: return TEXT("Reyon g\u00f6revlisi");
    case ERole::HrManager: return TEXT("\u0130K m\u00fcd\u00fcr\u00fc");
    default: return TEXT("Mali m\u00fc\u015favir");
    }
}

MarketStaff::ERole MarketStaff::RoleOf(const FMarketEmployee& Employee)
{
    return static_cast<ERole>(FMath::Min<uint8>(Employee.Role, 3));
}

int64 MarketStaff::HireCostOn(ERole Role, int32 GameDay)
{
    return MarketPrices::WageScaled(Role == ERole::HrManager ? HrHireCost : HireCost, GameDay);
}

int64 MarketStaff::FairWage(ERole Role, int32 Skill, int32 GameDay)
{
    if (Role == ERole::Accountant) return MarketPrices::WageScaled(AccountantDailyFee, GameDay);
    const double Base = Role == ERole::HrManager ? 3500.0 : 2000.0;
    return Round50(Base * (0.8 + 0.5 * FMath::Clamp(Skill, 0, 100) / 100.0) * MarketPrices::WageIndex(GameDay));
}

bool MarketStaff::OnDuty(const FMarketState& State, const FMarketEmployee& Employee)
{
    return WorksOn(Employee, State.Day);
}

int32 MarketStaff::Count(const FMarketState& State, ERole Role)
{
    int32 Total = 0;
    for (const FMarketEmployee& E : State.Staff) if (RoleOf(E) == Role) ++Total;
    return Total;
}

const FMarketEmployee* MarketStaff::OnDutyAt(const FMarketState& State, ERole Role, int32 Index)
{
    for (const FMarketEmployee& E : State.Staff)
        if (RoleOf(E) == Role && OnDuty(State, E) && Index-- == 0) return &E;
    return nullptr;
}

FMarketEmployee* MarketStaff::FindEmployee(FMarketState& State, int32 Id)
{
    return State.Staff.FindByPredicate([Id](const FMarketEmployee& E) { return E.Id == Id; });
}

bool MarketStaff::HasHr(const FMarketState& State) { return Count(State, ERole::HrManager) > 0; }
bool MarketStaff::HasAccountant(const FMarketState& State) { return Count(State, ERole::Accountant) > 0; }

bool MarketStaff::HrUnlocked(const FMarketState& State)
{
    return Count(State, ERole::Cashier) + Count(State, ERole::Stocker) >= HrUnlockStaff || State.bSecondStore;
}

void MarketStaff::Migrate(FMarketState& State)
{
    if (State.Staff.Num() > 0 || (!State.bCashier && State.Stockers <= 0)) return;
    auto Add = [&State](ERole Role, const FString& Name)
    {
        FMarketEmployee E;
        E.Id = State.NextEmployeeId++;
        E.Name = Name;
        E.Role = static_cast<uint8>(Role);
        E.DailyWage = 2000; // the v0.1 wage
        E.Morale = 65.f;
        E.HiredDay = 1;
        State.Staff.Add(E);
    };
    if (State.bCashier) Add(ERole::Cashier, TEXT("Emine Kaya"));
    for (int32 I = 0; I < FMath::Clamp(State.Stockers, 0, FMarketState::MaxStockers); ++I) Add(ERole::Stocker, StaffPlanner::WorkerName(I));
    SyncCounts(State);
}

void MarketStaff::AddStartingStaff(FMarketState& State, int32 Cashiers, int32 Stockers)
{
    auto Add = [&State](int32 Slot)
    {
        FMarketEmployee E = MakeCandidate(State, Slot);
        E.HiredDay = State.Day;
        E.Morale = 70.f;
        E.Fatigue = 0.f;
        State.Staff.Add(E);
    };
    for (int32 I = 0; I < FMath::Clamp(Cashiers, 0, MaxCashiers); ++I) Add(0);
    for (int32 I = 0; I < FMath::Clamp(Stockers, 0, FMarketState::MaxStockers); ++I) Add(1);
    SyncCounts(State);
}

void MarketStaff::SyncCounts(FMarketState& State)
{
    if (State.Staff.Num() == 0 && State.Candidates.Num() == 0 && State.NextEmployeeId == 1) return; // v0.1 state: flags are the truth
    State.bCashier = OnDutyAt(State, ERole::Cashier, 0) != nullptr;
    int32 Stockers = 0;
    while (OnDutyAt(State, ERole::Stocker, Stockers)) ++Stockers;
    State.Stockers = FMath::Min(Stockers, FMarketState::MaxStockers);
}

void MarketStaff::EnsureCandidates(FMarketState& State)
{
    if (State.Candidates.Num() == 0) RefreshPool(State);
}

FString MarketStaff::DescribeCandidate(const FMarketState& State, const FMarketEmployee& C)
{
    const FString Head = FString::Printf(TEXT("%s \u00b7 %s \u00b7 %s/g\u00fcn"), *RoleName(RoleOf(C)), *C.Name, *Tl(C.DailyWage));
    if (!HasHr(State))
        return Head + FString::Printf(TEXT(" \u00b7 deneyim %s \u00b7 h\u0131z %s"), Band(C.Skill), Band(C.Speed));
    return Head + FString::Printf(TEXT(" \u00b7 beceri %d \u00b7 h\u0131z %d \u00b7 dayan\u0131kl\u0131l\u0131k %d \u00b7 referans %s"), C.Skill, C.Speed, C.Stamina, Reference(C));
}

FString MarketStaff::DescribeEmployee(const FMarketState& State, const FMarketEmployee& E)
{
    FString Text = FString::Printf(TEXT("%s \u00b7 %s \u00b7 %s/g\u00fcn \u00b7 moral %.0f \u00b7 yorgunluk %.0f"),
        *E.Name, *RoleName(RoleOf(E)), *Tl(E.DailyWage), E.Morale, E.Fatigue);
    if (HasHr(State) && IsShopRole(RoleOf(E))) Text += FString::Printf(TEXT(" \u00b7 beceri %d \u00b7 h\u0131z %d"), E.Skill, E.Speed);
    if (E.OffDay >= State.Day) Text += FString::Printf(TEXT(" \u00b7 %d. g\u00fcn izinli"), E.OffDay);
    if (E.LeaveDay > 0) Text += FString::Printf(TEXT(" \u00b7 %d. g\u00fcn sonunda ayr\u0131l\u0131yor"), E.LeaveDay);
    return Text;
}

bool MarketStaff::Hire(FMarketState& State, int32 CandidateIndex, FString& OutMessage)
{
    if (!State.Candidates.IsValidIndex(CandidateIndex)) { OutMessage = TEXT("Bu aday art\u0131k listede yok."); return false; }
    FMarketEmployee Hired = State.Candidates[CandidateIndex];
    const ERole Role = RoleOf(Hired);
    if (Role == ERole::Accountant) return HireAccountant(State, OutMessage);
    if (Role == ERole::HrManager && !HrUnlocked(State))
    {
        OutMessage = FString::Printf(TEXT("\u0130K m\u00fcd\u00fcr\u00fc i\u00e7in \u00f6nce en az %d kasiyer/reyon g\u00f6revlisi \u00e7al\u0131\u015fmal\u0131."), HrUnlockStaff);
        return false;
    }
    if (Count(State, Role) >= RoleLimit(Role))
    {
        OutMessage = FString::Printf(TEXT("En fazla %d %s \u00e7al\u0131\u015fabilir."), RoleLimit(Role), *RoleName(Role).ToLower());
        return false;
    }
    const int64 Cost = HireCostOn(Role, State.Day);
    if (State.Cash < Cost) { OutMessage = FString::Printf(TEXT("\u0130\u015fe al\u0131m i\u00e7in kasada %s gerekiyor."), *Tl(Cost)); return false; }
    // The HR manager negotiates the offer.
    if (HasHr(State)) Hired.DailyWage = Round50(Hired.DailyWage * 0.92);
    State.Cash -= Cost;
    Hired.HiredDay = State.Day;
    Hired.Morale = 70.f;
    Hired.Fatigue = 0.f;
    State.Staff.Add(Hired);
    State.Candidates.RemoveAt(CandidateIndex);
    SyncCounts(State);
    const TCHAR* Duty = Role == ERole::Cashier ? TEXT("Kasada \u00f6deme al\u0131r; yo\u011fun g\u00fcnler onu yorar.")
        : Role == ERole::Stocker ? TEXT("Raflar\u0131 depodan doldurur, rafta olmayan \u00fcr\u00fcn\u00fc reyonuna dizer.")
        : TEXT("Her g\u00fcn en mutsuz \u00e7al\u0131\u015fanla konu\u015fur, yorgunlara izin ayarlar, ayr\u0131lan\u0131n yerine aday bulur.");
    OutMessage = FString::Printf(TEXT("%s i\u015fe ba\u015flad\u0131 (%s). G\u00fcnl\u00fck \u00fccret %s. %s"), *Hired.Name, *RoleName(Role), *Tl(Hired.DailyWage), Duty);
    return true;
}

bool MarketStaff::HireBest(FMarketState& State, ERole Role, FString& OutMessage)
{
    EnsureCandidates(State);
    int32 Best = INDEX_NONE;
    double BestValue = -1.0;
    for (int32 I = 0; I < State.Candidates.Num(); ++I)
    {
        const FMarketEmployee& C = State.Candidates[I];
        if (RoleOf(C) != Role) continue;
        // What the player can see without HR is a band; the value uses it the same way for everybody.
        const double Value = (C.Skill + C.Speed) / static_cast<double>(FMath::Max<int64>(1, C.DailyWage));
        if (Value > BestValue) { BestValue = Value; Best = I; }
    }
    if (Best == INDEX_NONE)
    {
        const int32 Next = State.CandidatesDay + (HasHr(State) ? 3 : 7);
        OutMessage = FString::Printf(TEXT("\u015eu an %s aday\u0131 yok. Aday listesi %d. g\u00fcn yenilenir (\u0130K m\u00fcd\u00fcr\u00fc daha s\u0131k ve daha \u00e7ok aday bulur)."),
            *RoleName(Role).ToLower(), Next);
        return false;
    }
    return Hire(State, Best, OutMessage);
}

bool MarketStaff::HireAccountant(FMarketState& State, FString& OutMessage)
{
    if (HasAccountant(State)) { OutMessage = TEXT("Mali m\u00fc\u015favir zaten defterlerini tutuyor."); return false; }
    FMarketEmployee E;
    E.Id = State.NextEmployeeId++;
    E.Name = TEXT("Necati Bey");
    E.Role = static_cast<uint8>(ERole::Accountant);
    E.Skill = 80; E.Speed = 50; E.Stamina = 70; E.Honesty = 90;
    // G-077 (#35, #36): the fee follows wages; taking over the books costs a week's fee up front.
    E.DailyWage = FairWage(ERole::Accountant, 80, State.Day);
    const int64 Engagement = E.DailyWage * 7;
    if (State.Cash < Engagement) { OutMessage = FString::Printf(TEXT("Necati Bey defterleri devralmak i\u00e7in bir haftal\u0131k \u00fccreti pe\u015fin ister: %s."), *Tl(Engagement)); return false; }
    State.Cash -= Engagement;
    E.Morale = 75.f;
    E.HiredDay = State.Day;
    State.Staff.Add(E);
    OutMessage = FString::Printf(TEXT("D\u00fckk\u00e2n\u0131n eski mali m\u00fc\u015faviri Necati Bey defterleri devrald\u0131 (%s/g\u00fcn, devir bedeli %s). Haftal\u0131k vergiyi o hesaplar ve zaman\u0131nda \u00f6der, kasa farklar\u0131n\u0131 takip eder. \u0130ndirim ve denetim korumas\u0131 t\u00fcm hafta \u00e7al\u0131\u015ft\u0131\u011f\u0131 haftalarda ge\u00e7erlidir."),
        *Tl(E.DailyWage), *Tl(Engagement));
    return true;
}

bool MarketStaff::Fire(FMarketState& State, int32 EmployeeId, FString& OutMessage)
{
    const int32 Index = IndexOf(State, EmployeeId);
    if (Index == INDEX_NONE) { OutMessage = TEXT("Bu ki\u015fi art\u0131k \u00e7al\u0131\u015fm\u0131yor."); return false; }
    const FMarketEmployee Leaving = State.Staff[Index];
    const int64 Severance = RoleOf(Leaving) == ERole::Accountant ? 0 : Leaving.DailyWage * SeveranceDays;
    if (State.Cash < Severance) { OutMessage = FString::Printf(TEXT("\u0130hbar tazminat\u0131 i\u00e7in kasada %s gerekiyor."), *Tl(Severance)); return false; }
    State.Cash -= Severance;
    State.Staff.RemoveAt(Index);
    // Colleagues notice.
    for (FMarketEmployee& E : State.Staff) if (IsShopRole(RoleOf(E))) E.Morale = FMath::Max(0.f, E.Morale - 3.f);
    SyncCounts(State);
    OutMessage = Severance > 0
        ? FString::Printf(TEXT("%s i\u015ften \u00e7\u0131kar\u0131ld\u0131. \u0130hbar tazminat\u0131 %s \u00f6dendi."), *Leaving.Name, *Tl(Severance))
        : FString::Printf(TEXT("%s ile s\u00f6zle\u015fme bitti. Vergiyi art\u0131k sen takip edeceksin."), *Leaving.Name);
    return true;
}

bool MarketStaff::FireLast(FMarketState& State, ERole Role, FString& OutMessage)
{
    for (int32 I = State.Staff.Num() - 1; I >= 0; --I)
        if (RoleOf(State.Staff[I]) == Role) return Fire(State, State.Staff[I].Id, OutMessage);
    OutMessage = FString::Printf(TEXT("\u00c7al\u0131\u015fan %s yok."), *RoleName(Role).ToLower());
    return false;
}

bool MarketStaff::Raise(FMarketState& State, int32 EmployeeId, FString& OutMessage)
{
    FMarketEmployee* E = FindEmployee(State, EmployeeId);
    if (!E || RoleOf(*E) == ERole::Accountant) { OutMessage = TEXT("Bu ki\u015fiye zam yap\u0131lamaz."); return false; }
    E->DailyWage = Round50(E->DailyWage * 1.10);
    E->Morale = FMath::Min(100.f, E->Morale + 12.f);
    E->LowMoraleDays = 0;
    const bool bStays = E->LeaveDay > 0 && E->Morale >= 35.f;
    if (bStays) E->LeaveDay = 0;
    OutMessage = FString::Printf(TEXT("%s: yeni \u00fccret %s/g\u00fcn (moral %.0f).%s"), *E->Name, *Tl(E->DailyWage), E->Morale,
        bStays ? TEXT(" \u0130stifas\u0131n\u0131 geri ald\u0131.") : TEXT(""));
    return true;
}

bool MarketStaff::GiveDayOff(FMarketState& State, int32 EmployeeId, bool bShopOpen, FString& OutMessage)
{
    FMarketEmployee* E = FindEmployee(State, EmployeeId);
    if (!E || RoleOf(*E) == ERole::Accountant) { OutMessage = TEXT("Bu ki\u015fiye izin verilemez."); return false; }
    const int32 Day = State.Day + (bShopOpen ? 1 : 0);
    if (E->OffDay == Day) { OutMessage = FString::Printf(TEXT("%s zaten %d. g\u00fcn izinli."), *E->Name, Day); return false; }
    E->OffDay = Day;
    E->Morale = FMath::Min(100.f, E->Morale + 4.f);
    SyncCounts(State);
    const TCHAR* Cover = RoleOf(*E) == ERole::Cashier && Count(State, ERole::Cashier) < 2 ? TEXT(" O g\u00fcn kasay\u0131 sen al\u0131rs\u0131n (E).") : TEXT("");
    OutMessage = FString::Printf(TEXT("%s %d. g\u00fcn izinli; dinlenip geri gelir (\u00fccretli izin).%s"), *E->Name, Day, Cover);
    return true;
}

bool MarketStaff::Warn(FMarketState& State, int32 EmployeeId, FString& OutMessage)
{
    FMarketEmployee* E = FindEmployee(State, EmployeeId);
    if (!E || RoleOf(*E) != ERole::Cashier) { OutMessage = TEXT("Uyar\u0131 yaln\u0131zca kasiyerler i\u00e7in."); return false; }
    E->WarnedDay = State.Day;
    // Being suspected hurts an honest person more.
    E->Morale = FMath::Max(0.f, E->Morale - (E->Honesty >= 60 ? 15.f : 8.f));
    OutMessage = FString::Printf(TEXT("%s ile kasa fark\u0131 konu\u015fuldu. Eksik ger\u00e7ekten ondan geliyorsa bir s\u00fcre azal\u0131r; de\u011filse morali d\u00fc\u015fer."), *E->Name);
    return true;
}

int64 MarketStaff::PayTax(FMarketState& State)
{
    FMarketBooks& Books = State.Books;
    const int64 Paid = FMath::Max<int64>(0, FMath::Min(State.Cash, Books.TaxDue));
    if (Paid <= 0) return 0;
    State.Cash -= Paid;
    Books.TaxDue -= Paid;
    Books.TotalTaxPaid += Paid;
    if (Books.TaxDue == 0) { Books.TaxDeclared = 0; Books.PenaltyThisTax = 0; Books.TaxDueDay = 0; }
    return Paid;
}

float MarketStaff::WorkSpeed(const FMarketEmployee& E)
{
    const float Tired = 1.f - FMath::Max(0.f, E.Fatigue - 40.f) / 200.f;
    return FMath::Clamp((0.75f + E.Speed * 0.005f) * Tired, 0.5f, 1.3f);
}

float MarketStaff::CheckoutSeconds(const FMarketEmployee& Cashier, int32 Units)
{
    const float Hands = 1.6f + 0.35f * FMath::Clamp(Units, 1, 40);
    const float Routine = 1.15f - FMath::Clamp(Cashier.Skill, 0, 100) * 0.003f;
    return FMath::Clamp(Hands * Routine / WorkSpeed(Cashier), 1.5f, 12.f);
}

int32 MarketStaff::CarryUnits(const FMarketEmployee& E)
{
    return 12 + FMath::Clamp(E.Skill, 0, 100) * 12 / 100;
}

bool MarketStaff::MayEditPlan(const FMarketEmployee& E)
{
    return E.Skill >= 45;
}

void MarketStaff::RecordWork(FMarketState& State, int32 EmployeeId, int32 Units)
{
    if (FMarketEmployee* E = FindEmployee(State, EmployeeId)) E->WorkToday += FMath::Max(0, Units);
}

void MarketStaff::CloseDay(FMarketState& State)
{
    Migrate(State);
    State.StaffNews.Reset();
    State.LastTillDifference = 0;
    State.LastTaxPaid = 0;
    State.LastPenalty = 0;
    const int32 Closed = State.Day - 1;
    if (Closed < 1) { SyncCounts(State); return; }
    TArray<FString>& News = State.StaffNews;
    auto Worked = [Closed](const FMarketEmployee& E) { return WorksOn(E, Closed); };
    const FMarketEmployee* HrToday = State.Staff.FindByPredicate([&](const FMarketEmployee& E) { return RoleOf(E) == ERole::HrManager && Worked(E); });
    const bool bHr = HrToday != nullptr;
    const bool bAccountant = HasAccountant(State);

    // 1. The till: the cashiers on duty share the day's shoppers. When nobody worked the till the player did.
    int32 Cashiers = 0;
    for (const FMarketEmployee& E : State.Staff) if (RoleOf(E) == ERole::Cashier && Worked(E)) ++Cashiers;
    const int32 ServedShare = Cashiers > 0 ? State.LastServed / Cashiers : 0;
    const int64 RevenueShare = Cashiers > 0 ? State.LastRevenue / Cashiers : 0;
    for (FMarketEmployee& E : State.Staff)
    {
        if (RoleOf(E) != ERole::Cashier || !Worked(E)) continue;
        const int64 Difference = TillDifference(State, E, Closed, ServedShare, RevenueShare);
        E.RecentTill.Add(Difference);
        if (E.RecentTill.Num() > 7) E.RecentTill.RemoveAt(0, E.RecentTill.Num() - 7);
        State.LastTillDifference += Difference;
        if (bAccountant && Difference != 0) News.Add(FString::Printf(TEXT("Kasa say\u0131m\u0131: %s %s."), *E.Name, *Tl(Difference)));
    }
    State.Cash += State.LastTillDifference;
    State.LastProfit += State.LastTillDifference;
    if (!bAccountant && State.LastTillDifference != 0)
        News.Add(FString::Printf(TEXT("Kasa fark\u0131: %s. Kimin kasas\u0131ndan geldi\u011fini mali m\u00fc\u015favir ay\u0131r\u0131r."), *Tl(State.LastTillDifference)));

    // 2. Fatigue and learning.
    for (FMarketEmployee& E : State.Staff)
    {
        const ERole Role = RoleOf(E);
        if (Worked(E))
        {
            float Load = 12.f;
            if (Role == ERole::Cashier) Load = 10.f + ServedShare * 0.5f;
            else if (Role == ERole::Stocker) Load = 10.f + E.WorkToday * 0.12f;
            Load *= 1.4f - FMath::Clamp(E.Stamina, 0, 100) * 0.008f;
            E.Fatigue = FMath::Clamp(E.Fatigue + Load - 20.f, 0.f, 100.f);
            ++E.DaysWorked;
            if (E.Skill < 90 && E.DaysWorked % 3 == 0) ++E.Skill; // learns on the job
        }
        else E.Fatigue = FMath::Max(0.f, E.Fatigue - 45.f);
        E.WorkToday = 0;
    }

    // 3. Morale moves a fifth of the way to what the job feels like: wage against the market, fatigue, HR's care.
    for (FMarketEmployee& E : State.Staff)
    {
        const ERole Role = RoleOf(E);
        if (Role == ERole::Accountant) continue;
        const float WageRatio = static_cast<float>(E.DailyWage) / static_cast<float>(FMath::Max<int64>(1, FairWage(Role, E.Skill, State.Day)));
        const float Target = 60.f + (WageRatio - 1.f) * 100.f - FMath::Max(0.f, E.Fatigue - 50.f) * 0.6f + (bHr ? 5.f : 0.f);
        E.Morale = FMath::Clamp(E.Morale + (Target - E.Morale) * 0.2f, 0.f, 100.f);
    }

    // 4. HR manager: one talk a day, a day off for the most tired person when a colleague can cover.
    if (bHr)
    {
        FMarketEmployee* Saddest = nullptr;
        for (FMarketEmployee& E : State.Staff)
            if (IsShopRole(RoleOf(E)) && E.Morale < 65.f && (!Saddest || E.Morale < Saddest->Morale)) Saddest = &E;
        if (Saddest)
        {
            Saddest->Morale = FMath::Min(100.f, Saddest->Morale + 8.f);
            News.Add(FString::Printf(TEXT("\u0130K: %s ile konu\u015ftu (moral %.0f)."), *Saddest->Name, Saddest->Morale));
        }
        for (ERole Role : { ERole::Cashier, ERole::Stocker })
        {
            FMarketEmployee* Tired = nullptr;
            for (FMarketEmployee& E : State.Staff)
                if (RoleOf(E) == Role && E.Fatigue >= 70.f && E.OffDay != State.Day && (!Tired || E.Fatigue > Tired->Fatigue)) Tired = &E;
            if (!Tired) continue;
            int32 Cover = 0;
            for (const FMarketEmployee& E : State.Staff) if (RoleOf(E) == Role && &E != Tired && E.OffDay != State.Day) ++Cover;
            if (Cover > 0)
            {
                Tired->OffDay = State.Day;
                News.Add(FString::Printf(TEXT("\u0130K: %s yar\u0131n izinli (yorgunluk %.0f)."), *Tired->Name, Tired->Fatigue));
            }
            else News.Add(FString::Printf(TEXT("\u0130K: %s \u00e7ok yorgun (%.0f) ama yerine bakacak kimse yok. \u0130kinci bir %s d\u00fc\u015f\u00fcn."),
                *Tired->Name, Tired->Fatigue, *RoleName(Role).ToLower()));
        }
    }
    else
    {
        for (const FMarketEmployee& E : State.Staff)
            if (IsShopRole(RoleOf(E)) && E.Fatigue >= 80.f)
                News.Add(FString::Printf(TEXT("%s \u00e7ok yorgun (%.0f): yava\u015fl\u0131yor ve hata yap\u0131yor. Men\u00fcden izin ver."), *E.Name, E.Fatigue));
    }

    // 5. Leaving: notices that ran out, then new notices after three unhappy days (never out of the blue).
    TArray<ERole> Left;
    TArray<int64> LeftWage;
    for (int32 I = State.Staff.Num() - 1; I >= 0; --I)
    {
        const FMarketEmployee& E = State.Staff[I];
        if (E.LeaveDay > 0 && E.LeaveDay <= Closed)
        {
            News.Add(FString::Printf(TEXT("%s i\u015ften ayr\u0131ld\u0131 (%s)."), *E.Name, *RoleName(RoleOf(E)).ToLower()));
            Left.Add(RoleOf(E));
            LeftWage.Add(E.DailyWage);
            State.Staff.RemoveAt(I);
        }
    }
    for (FMarketEmployee& E : State.Staff)
    {
        const ERole Role = RoleOf(E);
        if (Role == ERole::Accountant) continue;
        E.LowMoraleDays = E.Morale < 30.f ? E.LowMoraleDays + 1 : 0;
        if (E.LeaveDay == 0 && E.LowMoraleDays >= 3)
        {
            E.LeaveDay = State.Day + NoticeDays - 1;
            const float WageRatio = static_cast<float>(E.DailyWage) / static_cast<float>(FMath::Max<int64>(1, FairWage(Role, E.Skill, State.Day)));
            const TCHAR* Why = WageRatio < 0.95f ? TEXT("\u00fccretini d\u00fc\u015f\u00fck buluyor") : E.Fatigue >= 60.f ? TEXT("\u00e7ok yoruldu") : TEXT("i\u015finden mutsuz");
            News.Add(FString::Printf(TEXT("%s istifa dilek\u00e7esi verdi: %s. %d. g\u00fcn\u00fcn sonunda ayr\u0131lacak. Zam veya izin fikrini de\u011fi\u015ftirebilir."), *E.Name, Why, E.LeaveDay));
            if (bHr) News.Add(FString::Printf(TEXT("\u0130K: %s i\u00e7in %%10 zam \u00f6nerisi (yeni \u00fccret %s)."), *E.Name, *Tl(Round50(E.DailyWage * 1.10))));
        }
        else if (E.LeaveDay > 0 && E.Morale >= 45.f)
        {
            E.LeaveDay = 0;
            News.Add(FString::Printf(TEXT("%s istifas\u0131n\u0131 geri ald\u0131."), *E.Name));
        }
    }

    // 6. HR replaces leavers from the pool (up to 15 % more than the old wage).
    EnsureCandidates(State);
    if (bHr && State.bHrAutoReplace && HasHr(State))
    {
        for (int32 L = 0; L < Left.Num(); ++L)
        {
            if (!IsShopRole(Left[L])) continue;
            int32 Best = INDEX_NONE;
            for (int32 I = 0; I < State.Candidates.Num(); ++I)
            {
                const FMarketEmployee& C = State.Candidates[I];
                if (RoleOf(C) != Left[L] || C.DailyWage * 0.92 > LeftWage[L] * 1.15 || ReferenceWarns(C)) continue;
                if (Best == INDEX_NONE || C.Skill > State.Candidates[Best].Skill) Best = I;
            }
            FString Message;
            if (Best != INDEX_NONE && Hire(State, Best, Message))
                News.Add(FString::Printf(TEXT("\u0130K: yerine %s al\u0131nd\u0131."), *State.Staff.Last().Name));
            else News.Add(FString::Printf(TEXT("\u0130K: ayr\u0131lan %s i\u00e7in uygun aday yok; yeni ilan verildi."), *RoleName(Left[L]).ToLower()));
        }
    }

    // 7. Books. Every closed day goes into the running week; the week is declared at its last day's close.
    FMarketBooks& Books = State.Books;
    Books.PeriodSales += State.LastRevenue;
    Books.PeriodPurchases += State.LastPurchases;
    Books.PeriodProfit += State.LastProfit;
    if (Books.TaxDue > 0)
    {
        if (bAccountant && Closed >= Books.TaxDueDay - 1)
        {
            State.LastTaxPaid = PayTax(State);
            if (State.LastTaxPaid > 0) News.Add(FString::Printf(TEXT("Mali m\u00fc\u015favir: vergi \u00f6dendi (%s)."), *Tl(State.LastTaxPaid)));
            if (Books.TaxDue > 0) News.Add(FString::Printf(TEXT("Mali m\u00fc\u015favir: kasada vergiye yetecek para yok; kalan %s."), *Tl(Books.TaxDue)));
        }
        if (Books.TaxDue > 0 && Closed >= Books.TaxDueDay)
        {
            const int64 Base = FMath::Max<int64>(Books.TaxDeclared, 100);
            const int64 Cap = static_cast<int64>(Base * LatePenaltyCap);
            const int64 Step = static_cast<int64>(Base * (Books.PenaltyThisTax == 0 ? LatePenaltyFirst : LatePenaltyPerDay));
            const int64 Penalty = FMath::Clamp<int64>(FMath::Max<int64>(Step, 100), 0, FMath::Max<int64>(0, Cap - Books.PenaltyThisTax));
            if (Penalty > 0)
            {
                Books.TaxDue += Penalty;
                Books.PenaltyThisTax += Penalty;
                Books.TotalPenalties += Penalty;
                State.LastPenalty += Penalty;
                State.LastProfit -= Penalty;
                News.Add(FString::Printf(TEXT("Vergi gecikti: %s gecikme cezas\u0131 eklendi. \u00d6denecek %s."), *Tl(Penalty), *Tl(Books.TaxDue)));
            }
        }
        else if (Books.TaxDue > 0 && !bAccountant && Closed == Books.TaxDueDay - 1)
            News.Add(FString::Printf(TEXT("Yar\u0131n vergi i\u00e7in son g\u00fcn: %s. Men\u00fc > Personel > Vergiyi \u00f6de."), *Tl(Books.TaxDue)));
    }
    if (Closed % 7 == 0)
    {
        const int32 Week = (Closed - 1) / 7 + 1;
        // G-077 (#34): prices include VAT, so the VAT inside a margin is margin x rate / (1 + rate). That VAT is the
        // state's money, not profit: the income tax is taken from the profit without it.
        const double VatShare = static_cast<double>(VatRate) / (1.0 + static_cast<double>(VatRate));
        const int64 VatInside = static_cast<int64>(FMath::RoundToDouble((Books.PeriodSales - Books.PeriodPurchases) * VatShare));
        int64 Vat = VatInside - Books.VatCarry;
        Books.VatCarry = 0;
        if (Vat < 0) { Books.VatCarry = -Vat; Vat = 0; }
        const int64 Income = static_cast<int64>(FMath::RoundToDouble(FMath::Max<int64>(0, Books.PeriodProfit - FMath::Max<int64>(0, VatInside)) * static_cast<double>(IncomeTaxRate)));
        int64 Tax = Vat + Income;
        // G-077 (#35): the accountant's lower declaration and audit protection need him on the books all week.
        const FMarketEmployee* Accountant = State.Staff.FindByPredicate([](const FMarketEmployee& E) { return RoleOf(E) == ERole::Accountant; });
        const bool bWholeWeek = bAccountant && Accountant && Accountant->HiredDay <= Closed - 6;
        if (bWholeWeek) Tax = static_cast<int64>(FMath::RoundToDouble(Tax * static_cast<double>(AccountantDeduction)));
        if (!bWholeWeek && Mix(State.RivalSeed, Closed, 0xA0D17u) % AuditOneIn == 0u)
        {
            const int64 Fine = FMath::Max<int64>(MarketPrices::Scaled(AuditPenaltyMin, Closed), static_cast<int64>(Tax * AuditPenaltyRate));
            ++Books.Audits;
            Books.TaxDue += Fine;
            Books.TotalPenalties += Fine;
            State.LastPenalty += Fine;
            State.LastProfit -= Fine;
            News.Add(FString::Printf(TEXT("Vergi incelemesi: defterler d\u00fczenli tutulmad\u0131\u011f\u0131 i\u00e7in %s ceza. Mali m\u00fc\u015favirle bu olmaz."), *Tl(Fine)));
        }
        if (Tax > 0 || Books.TaxDue > 0)
        {
            // Unpaid older tax keeps its penalty count and cap; only the new week's tax is added to the base.
            const bool bOlderDebt = Books.TaxDue > 0;
            Books.TaxDue += Tax;
            Books.TaxDeclared = bOlderDebt ? Books.TaxDeclared + Tax : Books.TaxDue;
            if (!bOlderDebt) Books.PenaltyThisTax = 0;
            Books.TaxDueDay = Closed + TaxPayDays;
            News.Add(FString::Printf(TEXT("%d. hafta vergisi: KDV %s + gelir %s = %s%s. Son \u00f6deme %d. g\u00fcn.%s"), Week, *Tl(Vat), *Tl(Income), *Tl(Tax),
                bWholeWeek ? TEXT(" (belgeli giderlerle %10 az)") : TEXT(""), Books.TaxDueDay,
                bAccountant ? TEXT(" Mali m\u00fc\u015favir zaman\u0131nda \u00f6der.") : TEXT(" \u00d6demeyi sen yapmal\u0131s\u0131n.")));
        }
        else News.Add(FString::Printf(TEXT("%d. hafta vergisi \u00e7\u0131kmad\u0131 (zarar veya devreden KDV %s)."), Week, *Tl(Books.VatCarry)));
        Books.PeriodSales = Books.PeriodPurchases = Books.PeriodProfit = 0;
    }
    if (bAccountant)
    {
        const int32 Week = (Closed - 1) / 7 + 1;
        for (FMarketEmployee& E : State.Staff)
        {
            if (RoleOf(E) != ERole::Cashier || E.RecentTill.Num() < 4 || E.FlaggedWeek == Week) continue;
            int32 Short = 0;
            int64 Sum = 0;
            for (const int64 Day : E.RecentTill) { Sum += Day; if (Day < 0) ++Short; }
            if (Short < 4 || Sum > -3000) continue;
            E.FlaggedWeek = Week;
            News.Add(FString::Printf(TEXT("Mali m\u00fc\u015favir: %s'in kasas\u0131 son %d g\u00fcn\u00fcn %d'inde eksik (toplam %s). Say\u0131m hatas\u0131 da olabilir; uyarabilir ya da izleyebilirsin."),
                *E.Name, E.RecentTill.Num(), Short, *Tl(Sum)));
        }
        const int64 Need = (MarketPrices::Scaled(2200, Closed) + State.DailyPayroll()) * 2 + Books.TaxDue;
        if (State.Cash < Need) News.Add(FString::Printf(TEXT("Mali m\u00fc\u015favir: kasa %s; iki g\u00fcnl\u00fck gider ve vergi i\u00e7in %s laz\u0131m. Sipari\u015fi k\u00fc\u00e7\u00fclt."), *Tl(State.Cash), *Tl(Need)));
    }

    // 8. The hiring pool: weekly without HR, every three days with HR.
    const int32 Interval = HasHr(State) ? 3 : 7;
    if (Closed % Interval == 0) RefreshPool(State);
    SyncCounts(State);
}

FString MarketStaff::ReportText(const FMarketState& State)
{
    TArray<FString> Lines = State.StaffNews;
    if (State.Books.TaxDue > 0)
        Lines.Add(FString::Printf(TEXT("\u00d6denecek vergi: %s \u00b7 son g\u00fcn %d."), *Tl(State.Books.TaxDue), State.Books.TaxDueDay));
    return FString::Join(Lines, TEXT("\n"));
}

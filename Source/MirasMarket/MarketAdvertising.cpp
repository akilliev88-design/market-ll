#include "MarketAdvertising.h"
#include "MarketOnline.h"
#include "MarketBranches.h"
#include "MarketCountry.h"
#include "MarketCalendar.h"
#include "MarketPrices.h"
#include "MarketStaff.h"
#include "MarketLedger.h"
#include "MarketStart.h"

namespace MarketAdvertisingLocal
{
    using MarketAdvertising::EChannel;
    using MarketAdvertising::ChannelCount;

    enum : int32 { TSocial = 1 << 0, TSearch = 1 << 1, TManager = 1 << 2 };

    // Per channel: a month's cost at level 1 (start-level kurus), what it leaves in minds a month, what of it stays.
    // C8 (Codex C6: 35 million spent for ~134 million of estimated revenue, less than its gross profit; the flyers
    // 9.3 for 3.4): about 40 % cheaper, the flyers 60 %.
    const int64 BaseCost[ChannelCount] = { 1200000, 250000, 40000, 10000, 50000, 30000 };
    const float Effect[ChannelCount] = { 20.f, 9.f, 7.f, 8.f, 7.f, 10.f };
    const float Keep[ChannelCount] = { 0.85f, 0.75f, 0.8f, 0.4f, 0.6f, 0.3f };
    const float LevelCost[MarketAdvertising::MaxLevel + 1] = { 0.f, 1.f, 2.2f, 4.f };
    const float LevelEffect[MarketAdvertising::MaxLevel + 1] = { 0.f, 1.f, 1.8f, 2.8f };

    FString Tl(int64 Kurus) { return MarketCountry::Money(Kurus); }
    bool IsBrand(int32 C) { return C != static_cast<int32>(EChannel::Search); }
    bool IsOpenBranch(const FMarketBranch& B) { return B.Stage == static_cast<uint8>(MarketBranches::EStage::Open); }

    FString CountryOr(const FMarketState& State, const FString& Country) { return Country.IsEmpty() ? State.CountryId : Country; }

    const FMarketAdCountry* Find(const FMarketState& State, const FString& Country)
    {
        const FString Where = CountryOr(State, Country);
        return State.Advertising.Countries.FindByPredicate([&Where](const FMarketAdCountry& C) { return C.Country == Where; });
    }

    FMarketAdCountry& Get(FMarketState& State, const FString& Country)
    {
        const FString Where = CountryOr(State, Country);
        FMarketAdCountry* Found = State.Advertising.Countries.FindByPredicate([&Where](const FMarketAdCountry& C) { return C.Country == Where; });
        if (!Found)
        {
            FMarketAdCountry Fresh;
            Fresh.Country = Where;
            Found = &State.Advertising.Countries[State.Advertising.Countries.Add(Fresh)];
        }
        if (Found->Levels.Num() != ChannelCount) Found->Levels.Init(0, ChannelCount);
        if (Found->Stock.Num() != ChannelCount) Found->Stock.Init(0.f, ChannelCount);
        if (Found->MonthChannelSpend.Num() != ChannelCount) Found->MonthChannelSpend.Init(0, ChannelCount);
        if (Found->PrevChannelSpend.Num() != ChannelCount) Found->PrevChannelSpend.Init(0, ChannelCount);
        return *Found;
    }

    int32 ShopsIn(const FMarketState& State, const FString& Country, int32& OutProvinces)
    {
        TSet<FString> Provinces;
        int32 Shops = 0;
        if (Country == State.CountryId) { ++Shops; Provinces.Add(MarketStart::HomeProvince(State)); }
        for (const FMarketBranch& B : State.Branches)
            if (IsOpenBranch(B) && MarketBranches::CountryOf(State, B) == Country) { ++Shops; Provinces.Add(B.Province); }
        OutProvinces = Provinces.Num();
        return Shops;
    }

    float ManagerEfficiency(const FMarketState& State)
    {
        return State.Advertising.ManagerName.IsEmpty() ? 1.f : 0.9f + State.Advertising.ManagerSkill / 250.f;
    }

    float Brand(const FMarketAdCountry& C)
    {
        float Sum = 0.f;
        for (int32 I = 0; I < ChannelCount && I < C.Stock.Num(); ++I) if (IsBrand(I)) Sum += C.Stock[I];
        return Sum;
    }

    float Digital(const FMarketAdCountry& C)
    {
        const int32 S = static_cast<int32>(EChannel::Social), Q = static_cast<int32>(EChannel::Search);
        return (C.Stock.IsValidIndex(S) ? C.Stock[S] : 0.f) + (C.Stock.IsValidIndex(Q) ? C.Stock[Q] : 0.f);
    }

    // The manager's mix for a month: the best lift for the money first, until the budget is spent.
    void Mix(FMarketState& State, FMarketAdCountry& C, int64 Budget, int32 Day)
    {
        for (uint8& L : C.Levels) L = 0;
        int64 Spent = 0;
        for (int32 Step = 0; Step < ChannelCount * MarketAdvertising::MaxLevel; ++Step)
        {
            int32 Best = INDEX_NONE;
            float BestValue = 0.f;
            int64 BestCost = 0;
            for (int32 Ch = 0; Ch < ChannelCount; ++Ch)
            {
                const EChannel Channel = static_cast<EChannel>(Ch);
                const int32 Next = C.Levels[Ch] + 1;
                const float Era = MarketAdvertising::EraFactor(State, Channel, Day);
                if (Next > MarketAdvertising::MaxLevel || Era <= 0.f) continue;
                const int64 Extra = MarketAdvertising::MonthCost(State, C.Country, Channel, Next) - MarketAdvertising::MonthCost(State, C.Country, Channel, Next - 1);
                if (Extra <= 0 || Spent + Extra > Budget) continue;
                // What stays in minds per lira (a fast-fading channel counts less).
                const float Value = (LevelEffect[Next] - LevelEffect[Next - 1]) * Effect[Ch] * Era / (1.f - Keep[Ch] * 0.9f) / static_cast<float>(Extra);
                if (Value > BestValue) { BestValue = Value; Best = Ch; BestCost = Extra; }
            }
            if (Best == INDEX_NONE) break;
            ++C.Levels[Best];
            Spent += BestCost;
        }
    }
}

FString MarketAdvertising::ChannelName(EChannel Channel)
{
    switch (Channel)
    {
    case EChannel::TV: return TEXT("Televizyon");
    case EChannel::Radio: return TEXT("Radyo");
    case EChannel::Outdoor: return TEXT("A\u00e7\u0131k hava (bilbord)");
    case EChannel::Print: return TEXT("Gazete ve bro\u015f\u00fcr");
    case EChannel::Social: return TEXT("Sosyal medya");
    case EChannel::Search: return TEXT("Arama ve uygulama reklam\u0131");
    default: return TEXT("?");
    }
}

FString MarketAdvertising::ChannelNote(EChannel Channel)
{
    switch (Channel)
    {
    case EChannel::TV: return TEXT("\u00dclke \u00e7ap\u0131nda, pahal\u0131, en g\u00fc\u00e7l\u00fc ve en ge\u00e7 unutulan; b\u00fcy\u00fck a\u011fa de\u011fer.");
    case EChannel::Radio: return TEXT("Daha ucuz, daha \u00e7abuk unutulur.");
    case EChannel::Outdoor: return TEXT("Ma\u011fazam\u0131z olan illerde bilbord; il say\u0131s\u0131yla pahalan\u0131r.");
    case EChannel::Print: return TEXT("Her ma\u011faza i\u00e7in gazete eki ve bro\u015f\u00fcr; y\u0131llar ge\u00e7tik\u00e7e etkisi azal\u0131r.");
    case EChannel::Social: return TEXT("Ucuz, y\u0131llar ge\u00e7tik\u00e7e g\u00fc\u00e7lenir, gen\u00e7lere ula\u015f\u0131r; internet sipari\u015fini de art\u0131r\u0131r.");
    case EChannel::Search: return TEXT("Yaln\u0131z internet sipari\u015fi: hemen \u00e7al\u0131\u015f\u0131r, hemen biter.");
    default: return FString();
    }
}

float MarketAdvertising::EraFactor(const FMarketState& State, EChannel Channel, int32 GameDay)
{
    const int32 Start = MarketOnline::PandemicStart(State);
    switch (Channel)
    {
    case EChannel::Radio: return 0.85f;
    case EChannel::Print:
    {
        const float T = FMath::Clamp(static_cast<float>(GameDay - (Start - 4 * 365)) / (7.f * 365.f), 0.f, 1.f);
        return 1.f - 0.55f * T;
    }
    case EChannel::Social:
    {
        const int32 From = Start - 7 * 365;
        if (GameDay < From) return 0.f;
        return 0.3f + 0.7f * FMath::Clamp(static_cast<float>(GameDay - From) / (7.f * 365.f), 0.f, 1.f);
    }
    case EChannel::Search: return GameDay >= MarketOnline::OpenDay(State, MarketOnline::EChannel::Web) ? 1.f : 0.f;
    default: return 1.f;
    }
}

int64 MarketAdvertising::MonthCost(const FMarketState& State, const FString& Country, EChannel Channel, int32 Level)
{
    using namespace MarketAdvertisingLocal;
    const int32 L = FMath::Clamp(Level, 0, MaxLevel);
    if (L == 0) return 0;
    int32 Provinces = 0;
    const int32 Shops = MarketAdvertisingLocal::ShopsIn(State, CountryOr(State, Country), Provinces);
    const int32 C = static_cast<int32>(Channel);
    const int32 Reach = Channel == EChannel::Outdoor ? FMath::Max(1, Provinces) : Channel == EChannel::Print ? FMath::Max(1, Shops) : 1;
    return FMath::RoundToInt64(BaseCost[C] * Reach * LevelCost[L] * MarketPrices::ListLevel(FMath::Max(1, State.Day)));
}

int32 MarketAdvertising::LevelOf(const FMarketState& State, const FString& Country, EChannel Channel)
{
    const FMarketAdCountry* C = MarketAdvertisingLocal::Find(State, Country);
    const int32 I = static_cast<int32>(Channel);
    return C && C->Levels.IsValidIndex(I) ? C->Levels[I] : 0;
}

int32 MarketAdvertising::Encode(int32 CountryIndex, EChannel Channel, int32 Level)
{
    return CountryIndex * 100 + static_cast<int32>(Channel) * 10 + FMath::Clamp(Level, 0, MaxLevel);
}

TArray<FString> MarketAdvertising::Countries(const FMarketState& State)
{
    TArray<FString> Out;
    Out.Add(State.CountryId);
    for (const FMarketBranch& B : State.Branches)
        if (MarketAdvertisingLocal::IsOpenBranch(B)) Out.AddUnique(MarketBranches::CountryOf(State, B));
    return Out;
}

bool MarketAdvertising::SetLevel(FMarketState& State, const FString& Country, EChannel Channel, int32 Level, FString& OutMessage)
{
    const int32 L = FMath::Clamp(Level, 0, MaxLevel);
    const FString Where = MarketAdvertisingLocal::CountryOr(State, Country);
    if (!Countries(State).Contains(Where)) { OutMessage = TEXT("Bu \u00fclkede ma\u011fazam\u0131z yok."); return false; }
    if (L > 0 && EraFactor(State, Channel, State.Day) <= 0.f) { OutMessage = ChannelName(Channel) + TEXT(" hen\u00fcz yok."); return false; }
    FMarketAdCountry& C = MarketAdvertisingLocal::Get(State, Where);
    C.Levels[static_cast<int32>(Channel)] = static_cast<uint8>(L);
    const bool bWasAuto = State.Advertising.bAuto;
    State.Advertising.bAuto = false;
    OutMessage = L == 0 ? ChannelName(Channel) + TEXT(" reklam\u0131 durdu.")
        : FString::Printf(TEXT("%s: ayda %s (seviye %d)."), *ChannelName(Channel), *MarketAdvertisingLocal::Tl(MonthCost(State, Where, Channel, L)), L);
    if (bWasAuto) OutMessage += TEXT(" Reklam m\u00fcd\u00fcr\u00fc art\u0131k kar\u0131\u015f\u0131m\u0131 ayarlam\u0131yor.");
    return true;
}

bool MarketAdvertising::SetLevelArg(FMarketState& State, int32 Arg, FString& OutMessage)
{
    const TArray<FString> List = Countries(State);
    const int32 CountryIndex = Arg / 100, Channel = (Arg / 10) % 10, Level = Arg % 10;
    if (!List.IsValidIndex(CountryIndex) || Channel >= ChannelCount) { OutMessage = TEXT("B\u00f6yle bir reklam yok."); return false; }
    return SetLevel(State, List[CountryIndex], static_cast<EChannel>(Channel), Level, OutMessage);
}

int32 MarketAdvertising::BrandChannels(const FMarketState& State, const FString& Country)
{
    const FMarketAdCountry* C = MarketAdvertisingLocal::Find(State, Country);
    int32 Count = 0;
    if (C) for (int32 I = 0; I < C->Levels.Num(); ++I) if (MarketAdvertisingLocal::IsBrand(I) && C->Levels[I] > 0) ++Count;
    return Count;
}

float MarketAdvertising::Together(const FMarketState& State, const FString& Country)
{
    const int32 Count = BrandChannels(State, Country);
    return Count >= 5 ? 1.3f : Count >= 3 ? 1.2f : 1.f;
}

float MarketAdvertising::TrafficFactor(const FMarketState& State, const FString& Country)
{
    const FMarketAdCountry* C = MarketAdvertisingLocal::Find(State, Country);
    return C ? 1.f + MaxTraffic * (1.f - FMath::Exp(-MarketAdvertisingLocal::Brand(*C) / 60.f)) : 1.f;
}

float MarketAdvertising::OnlineFactor(const FMarketState& State, const FString& Country)
{
    const FMarketAdCountry* C = MarketAdvertisingLocal::Find(State, Country);
    return C ? 1.f + MaxOnline * (1.f - FMath::Exp(-MarketAdvertisingLocal::Digital(*C) / 30.f)) : 1.f;
}

void MarketAdvertising::Candidate(const FMarketState& State, FString& OutName, int32& OutSkill, int64& OutWage)
{
    const int32 Week = (State.Day - 1) / 7;
    uint32 H = 2166136261u;
    for (const uint32 Part : { static_cast<uint32>(State.RivalSeed), static_cast<uint32>(Week), 0xAD5Eu })
        for (int32 Byte = 0; Byte < 4; ++Byte) { H ^= (Part >> (Byte * 8)) & 0xFFu; H *= 16777619u; }
    const MarketCountry::FProfile& Pack = MarketCountry::Active();
    const FString First = Pack.FirstNames.Num() ? Pack.FirstNames[H % static_cast<uint32>(Pack.FirstNames.Num())] : FString(TEXT("Ece"));
    const FString Last = Pack.LastNames.Num() ? Pack.LastNames[(H >> 8) % static_cast<uint32>(Pack.LastNames.Num())] : FString(TEXT("Demir"));
    OutName = First + TEXT(" ") + Last;
    OutSkill = 45 + static_cast<int32>((H >> 16) % 46u);
    OutWage = MarketStaff::FairWage(MarketStaff::ERole::HrManager, OutSkill, State.Day) * 15 / 10;
}

bool MarketAdvertising::CanHireManager(const FMarketState& State, FString& OutReason)
{
    if (!State.Advertising.ManagerName.IsEmpty()) { OutReason = TEXT("Reklam m\u00fcd\u00fcr\u00fc zaten var."); return false; }
    if (MarketOnline::TotalShops(State) < ManagerShops) { OutReason = FString::Printf(TEXT("Reklam m\u00fcd\u00fcr\u00fc i\u00e7in en az %d ma\u011faza gerekir."), ManagerShops); return false; }
    return true;
}

bool MarketAdvertising::HireManager(FMarketState& State, FString& OutMessage)
{
    if (!CanHireManager(State, OutMessage)) return false;
    FMarketAdvertising& A = State.Advertising;
    Candidate(State, A.ManagerName, A.ManagerSkill, A.ManagerWage);
    A.ManagerSince = State.Day;
    OutMessage = FString::Printf(TEXT("Reklam m\u00fcd\u00fcr\u00fc %s i\u015fe ba\u015flad\u0131 (beceri %d, ayda %s): her lira daha \u00e7ok i\u015f g\u00f6r\u00fcr; istersen reklam kar\u0131\u015f\u0131m\u0131n\u0131 o ayarlar."),
        *A.ManagerName, A.ManagerSkill, *MarketAdvertisingLocal::Tl(A.ManagerWage * 30));
    return true;
}

bool MarketAdvertising::FireManager(FMarketState& State, FString& OutMessage)
{
    FMarketAdvertising& A = State.Advertising;
    if (A.ManagerName.IsEmpty()) { OutMessage = TEXT("Reklam m\u00fcd\u00fcr\u00fc yok."); return false; }
    const int64 Severance = A.ManagerWage * 10 + MarketStaff::SeniorityPay(A.ManagerWage, A.ManagerSince, State.Day);
    if (State.Cash < Severance + State.OtherCosts) { OutMessage = FString::Printf(TEXT("Tazminat i\u00e7in kasada %s gerekiyor."), *MarketAdvertisingLocal::Tl(Severance)); return false; }
    MarketLedger::AddStoreCost(State, Severance, MarketLedger::HeadOfficeStore); // C10
    OutMessage = FString::Printf(TEXT("%s ayr\u0131ld\u0131, tazminat %s."), *A.ManagerName, *MarketAdvertisingLocal::Tl(Severance));
    A.ManagerName.Reset();
    A.ManagerSkill = 0;
    A.ManagerWage = 0;
    A.bAuto = false;
    return true;
}

bool MarketAdvertising::SetAuto(FMarketState& State, bool bAuto, FString& OutMessage)
{
    if (bAuto && State.Advertising.ManagerName.IsEmpty()) { OutMessage = TEXT("Reklam\u0131 b\u0131rakmak i\u00e7in \u00f6nce reklam m\u00fcd\u00fcr\u00fc gerekir."); return false; }
    State.Advertising.bAuto = bAuto;
    OutMessage = bAuto ? FString::Printf(TEXT("Reklam kar\u0131\u015f\u0131m\u0131n\u0131 her ay %s ayarlar (cironun binde %d'i)."), *State.Advertising.ManagerName, State.Advertising.BudgetPermille)
                       : FString(TEXT("Reklam\u0131 sen ayarl\u0131yorsun."));
    return true;
}

bool MarketAdvertising::SetBudget(FMarketState& State, int32 Permille, FString& OutMessage)
{
    State.Advertising.BudgetPermille = FMath::Clamp(Permille, 5, 60);
    OutMessage = FString::Printf(TEXT("Reklam b\u00fct\u00e7esi: cironun binde %d'i."), State.Advertising.BudgetPermille);
    return true;
}

FString MarketAdvertising::CountryLine(const FMarketState& State, const FString& Country)
{
    const FString Where = MarketAdvertisingLocal::CountryOr(State, Country);
    const FMarketAdCountry* C = MarketAdvertisingLocal::Find(State, Where);
    int64 Month = 0;
    for (int32 I = 0; I < ChannelCount; ++I) Month += MonthCost(State, Where, static_cast<EChannel>(I), LevelOf(State, Where, static_cast<EChannel>(I)));
    const MarketCountry::FProfile* Pack = MarketCountry::Find(Where);
    FString Line = FString::Printf(TEXT("%s \u00b7 ayda %s \u00b7 ma\u011fazalara +%%%.1f m\u00fc\u015fteri, internete +%%%.0f sipari\u015f"), Pack ? *Pack->Name : *Where, *MarketAdvertisingLocal::Tl(Month),
        (TrafficFactor(State, Where) - 1.f) * 100.f, (OnlineFactor(State, Where) - 1.f) * 100.f);
    if (BrandChannels(State, Where) >= 3) Line += FString::Printf(TEXT(" \u00b7 hepsi bir arada: +%%%.0f etki"), (Together(State, Where) - 1.f) * 100.f);
    if (C && C->PrevSpend > 0)
        Line += FString::Printf(TEXT("\nGe\u00e7en ay: reklam %s, getirdi\u011fi tahmini ciro %s."), *MarketAdvertisingLocal::Tl(C->PrevSpend), *MarketAdvertisingLocal::Tl(C->PrevUplift));
    return Line;
}

FString MarketAdvertising::ChannelLine(const FMarketState& State, const FString& Country, EChannel Channel)
{
    const float Era = EraFactor(State, Channel, State.Day);
    if (Era <= 0.f) return TEXT("hen\u00fcz yok");
    const int32 Level = LevelOf(State, Country, Channel);
    const FMarketAdCountry* C = MarketAdvertisingLocal::Find(State, Country);
    const int32 I = static_cast<int32>(Channel);
    const float Stock = C && C->Stock.IsValidIndex(I) ? C->Stock[I] : 0.f;
    // C7 (A menu comparison): the era factor is the channel's strength in this era ("d\u00f6nem g\u00fcc\u00fc"), not a customer
    // uplift; the level-1 price only when another level is chosen.
    const FString LevelNowText = Level > 0 ? FString::Printf(TEXT("seviye %d, ayda %s"), Level, *MarketAdvertisingLocal::Tl(MonthCost(State, Country, Channel, Level))) : FString(TEXT("kapal\u0131"));
    const FString FirstLevelText = Level == 1 ? FString() : FString::Printf(TEXT(" \u00b7 seviye 1: ayda %s"), *MarketAdvertisingLocal::Tl(MonthCost(State, Country, Channel, 1)));
    return FString::Printf(TEXT("%s%s \u00b7 d\u00f6nem g\u00fcc\u00fc %%%.0f \u00b7 ak\u0131llarda %.0f"), *LevelNowText, *FirstLevelText, Era * 100.f, Stock);
}

void MarketAdvertising::CloseDay(FMarketState& State)
{
    using namespace MarketAdvertisingLocal;
    const int32 Closed = State.Day - 1;
    if (Closed < 1) return;
    FMarketAdvertising& A = State.Advertising;
    const bool bNewMonth = MarketCalendar::DateOf(State.Day).Day == 1;
    if (!(A.Told & TSocial) && EraFactor(State, EChannel::Social, Closed) > 0.f) { A.Told |= TSocial; State.DayNews.Add(TEXT("Sosyal medya reklam\u0131 yay\u0131l\u0131yor: ucuz, gen\u00e7lere ula\u015f\u0131yor, y\u0131llar ge\u00e7tik\u00e7e g\u00fc\u00e7lenecek.")); }
    if (!(A.Told & TSearch) && EraFactor(State, EChannel::Search, Closed) > 0.f) { A.Told |= TSearch; State.DayNews.Add(TEXT("\u0130nternet aramalar\u0131na reklam verilebiliyor: yaln\u0131z internet sipari\u015fini art\u0131r\u0131r.")); }
    if (!(A.Told & TManager) && MarketOnline::TotalShops(State) >= ManagerShops) { A.Told |= TManager; State.DayNews.Add(TEXT("Zincir b\u00fcy\u00fcd\u00fc: art\u0131k bir reklam m\u00fcd\u00fcr\u00fc al\u0131nabilir (\u015eirket > Reklam).")); }

    const float Skill = ManagerEfficiency(State);
    int64 Total = 0;
    for (const FString& Country : Countries(State))
    {
        FMarketAdCountry& C = Get(State, Country);
        const float Together = MarketAdvertising::Together(State, Country);
        for (int32 I = 0; I < ChannelCount; ++I)
        {
            const EChannel Channel = static_cast<EChannel>(I);
            const int32 Level = FMath::Min<int32>(C.Levels[I] + (Closed < C.SeasonUntil && C.Levels[I] > 0 && IsBrand(I) ? 1 : 0), MaxLevel);
            const float Era = EraFactor(State, Channel, Closed);
            const float Add = LevelEffect[Level] * Effect[I] * Era * Skill * (IsBrand(I) ? Together : 1.f) / 30.f;
            C.Stock[I] = C.Stock[I] * FMath::Pow(Keep[I], 1.f / 30.f) + Add;
            if (Level <= 0) continue;
            const int64 Day = MonthCost(State, Country, Channel, Level) / 30;
            C.MonthSpend += Day;
            C.MonthChannelSpend[I] += Day;
            Total += Day;
        }
        // What the ads brought today (the shops' revenue in the country, x the part the ads added).
        int64 Revenue = Country == State.CountryId ? FMath::Max<int64>(0, State.LastRevenue) : 0;
        for (const FMarketBranch& B : State.Branches)
            if (IsOpenBranch(B) && MarketBranches::CountryOf(State, B) == Country) Revenue += FMath::Max<int64>(0, B.LastRevenue);
        const float Lift = MarketAdvertising::TrafficFactor(State, Country);
        C.MonthUplift += FMath::RoundToInt64(Revenue * (Lift - 1.f) / Lift);
        C.MonthRevenue += Revenue;
    }
    if (!A.ManagerName.IsEmpty()) Total += MarketStaff::EmployerCost(A.ManagerWage);
    if (Total > 0)
    {
        State.Cash -= Total;
        State.LastProfit -= Total;
        State.LastBranchProfit -= Total;
        MarketLedger::Post(State, MarketLedger::EAccount::Marketing, -Total, true, MarketLedger::HeadOfficeStore);
    }

    if (!bNewMonth) return;
    const bool bDecember = MarketCalendar::DateOf(State.Day).Month == 12;
    for (FMarketAdCountry& C : A.Countries)
    {
        if (C.MonthSpend > 0)
        {
            const MarketCountry::FProfile* Pack = MarketCountry::Find(C.Country);
            State.DayNews.Add(FString::Printf(TEXT("Reklam (%s): ge\u00e7en ay %s harcand\u0131, getirdi\u011fi tahmini ciro %s."), Pack ? *Pack->Name : *C.Country, *Tl(C.MonthSpend), *Tl(C.MonthUplift)));
        }
        C.PrevSpend = C.MonthSpend; C.PrevUplift = C.MonthUplift; C.PrevRevenue = C.MonthRevenue; C.PrevChannelSpend = C.MonthChannelSpend;
        C.MonthSpend = C.MonthUplift = C.MonthRevenue = 0;
        C.MonthChannelSpend.Init(0, ChannelCount);
        if (A.bAuto && !A.ManagerName.IsEmpty() && C.PrevRevenue > 0)
        {
            const int64 Budget = C.PrevRevenue * A.BudgetPermille / 1000 * (bDecember ? 3 : 2) / 2;   // December: half again
            Mix(State, C, Budget, State.Day);
        }
    }
}

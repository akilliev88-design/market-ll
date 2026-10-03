#include "MarketEras.h"
#include "MarketCompetitors.h"
#include "MarketEconomy.h"
#include "MarketCalendar.h"
#include "MarketCountry.h"
#include "MarketEvents.h"
#include "MarketGoods.h"
#include "MarketOnline.h"
#include "MarketPrices.h"

namespace MarketEras
{
    // B7: era factors (below).
    constexpr int32 RampInDays = 14;       // demand and costs move over two weeks
    constexpr int32 DemandFadeDays = 30;   // shoppers come back over a month
    constexpr int32 CostFadeDays = 300;    // purchase prices come down slowly
    constexpr float ShockPeak = 0.25f;     // the currency's jump at full strength, passed on by the import share

    uint32 EraMix(int32 Seed, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[2] = { static_cast<uint32>(Seed), Salt };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }

    // Extra yearly inflation of an era by years from its start (x strength). The unshifted high-inflation plan
    // puts them on the built-in curve's peaks (MarketPrices: 2018 16 %, 2021-2024 19/30/28/22 %).
    const double ShockBumps[] = { 0.05, 0.02 };
    const double HighBumps[] = { 0.07, 0.18, 0.16, 0.08 };

    struct FTemplate { EKind Kind; int32 Year, Month, Day; int32 EndYear, EndMonth, EndDay; int32 Wave; };
    // The unshifted calendar of the plan (internal anchors only; never shown).
    const FTemplate Templates[] =
    {
        { EKind::CurrencyShock, 2018, 8, 10, 2019, 1, 31, 1 },
        { EKind::Recession,     2019, 2, 1,  2019, 12, 31, 1 },
        { EKind::Pandemic,      2020, 3, 1,  2021, 6, 15, 1 },
        { EKind::HighInflation, 2021, 9, 1,  2023, 12, 31, 1 },
        { EKind::Recovery,      2024, 3, 1,  2025, 2, 28, 1 },
        { EKind::CurrencyShock, 2031, 5, 1,  2031, 10, 31, 2 },
        { EKind::HighInflation, 2032, 1, 1,  2033, 6, 30, 2 },
    };

    // How hard an era hits in a country character (0 = it does not come).
    float StrengthOf(ECharacter Character, EKind Kind, int32 Wave, int32 Seed)
    {
        switch (Character)
        {
        case ECharacter::Stable:
            if (Wave == 2) return 0.f;
            return Kind == EKind::CurrencyShock ? 0.3f : Kind == EKind::HighInflation ? 0.f : 0.8f;
        case ECharacter::Volatile:
            if (Wave == 2) return Kind == EKind::CurrencyShock && EraMix(Seed, 0xE2A2u) % 2u == 0u ? 0.5f : 0.f;
            return Kind == EKind::CurrencyShock ? 0.8f : Kind == EKind::HighInflation ? 0.4f : 1.f;
        default:
            if (Wave == 2) return Kind == EKind::CurrencyShock ? 0.6f : Kind == EKind::HighInflation ? 0.5f : 0.f;
            return 1.f;
        }
    }

    struct FActive
    {
        bool bSet = false;
        TArray<FEra> Timeline;
    };
    FActive& ActivePlan() { static FActive A; return A; }

    ECharacter CharacterOfState(const FMarketState& State)
    {
        const MarketCountry::FProfile* Pack = MarketCountry::Find(State.CountryId);
        return static_cast<ECharacter>(Pack ? static_cast<uint8>(Pack->Character) : static_cast<uint8>(MarketCountry::Active().Character));
    }

    double BumpOf(const TArray<FEra>& Timeline, int32 Year, bool bFirstWaveOnly)
    {
        double Sum = 0.0;
        for (const FEra& E : Timeline)
        {
            if (bFirstWaveOnly && E.Wave != 1) continue;
            const int32 Offset = Year - E.StartYear;
            if (E.Kind == EKind::CurrencyShock && Offset >= 0 && Offset < static_cast<int32>(UE_ARRAY_COUNT(ShockBumps))) Sum += ShockBumps[Offset] * E.Strength;
            if (E.Kind == EKind::HighInflation && Offset >= 0 && Offset < static_cast<int32>(UE_ARRAY_COUNT(HighBumps))) Sum += HighBumps[Offset] * E.Strength;
        }
        return Sum;
    }

    FString EraSource(const FEra& E) { return FString::Printf(TEXT("era.%d.%d"), static_cast<int32>(E.Kind), E.Wave); }

    void AddEffects(FMarketState& State, const FEra& E, int32 FirstDay)
    {
        using MarketEvents::EModifier;
        using MarketGoods::EGroup;
        const float S = E.Strength;
        const FString Source = EraSource(E);
        auto Add = [&](EModifier Kind, uint8 Group, float Value) { MarketEvents::AddModifier(State, Kind, Group, Value, FirstDay, E.EndDay, Source); };
        switch (E.Kind)
        {
        case EKind::CurrencyShock:
            // Purchase costs follow ImportCostFactor (MarketEvents::Factor asks it): up over two weeks, slowly down.
            Add(EModifier::PriceTolerance, MarketEvents::AllGroups, -0.03f * S);
            Add(EModifier::Traffic, MarketEvents::AllGroups, 1.f - 0.03f * S);
            break;
        case EKind::Recession:
            Add(EModifier::Traffic, MarketEvents::AllGroups, 1.f - 0.05f * S);
            Add(EModifier::PriceTolerance, MarketEvents::AllGroups, -0.05f * S);
            for (EGroup G : { EGroup::Sweets, EGroup::Snacks, EGroup::Drinks, EGroup::IceCream })
                Add(EModifier::Interest, static_cast<uint8>(G), 1.f - 0.15f * S);
            Add(EModifier::Interest, static_cast<uint8>(EGroup::Staples), 1.f + 0.05f * S);
            break;
        case EKind::HighInflation:
            Add(EModifier::PriceTolerance, MarketEvents::AllGroups, -0.04f * S);
            Add(EModifier::Traffic, MarketEvents::AllGroups, 1.f - 0.02f * S);
            break;
        case EKind::Recovery:
            Add(EModifier::Traffic, MarketEvents::AllGroups, 1.f + 0.04f * S);
            Add(EModifier::PriceTolerance, MarketEvents::AllGroups, 0.02f * S);
            for (EGroup G : { EGroup::Sweets, EGroup::Snacks })
                Add(EModifier::Interest, static_cast<uint8>(G), 1.f + 0.06f * S);
            break;
        default: break; // the epidemic: MarketOnline's profile
        }
    }

    FString StartNews(const FEra& E, int32 GameDay)
    {
        const float S = E.Strength;
        switch (E.Kind)
        {
        case EKind::CurrencyShock:
            return FString::Printf(TEXT("Kur \u015foku: d\u00f6viz birka\u00e7 g\u00fcnde f\u0131rlad\u0131; kahve, ya\u011f ve temizlik \u00fcr\u00fcnlerinin al\u0131\u015f fiyat\u0131 iki hafta i\u00e7inde %%%d kadar artacak. Raf fiyatlar\u0131n\u0131 g\u00f6zden ge\u00e7ir."),
                FMath::RoundToInt32(ShockPeak * 0.35f * S * 100.f));
        case EKind::Recession:
            return FString::Printf(TEXT("Durgunluk ba\u015flad\u0131: insanlar harcamay\u0131 k\u0131s\u0131yor, sepetler %%%d k\u00fc\u00e7\u00fcl\u00fcyor; ucuz temel \u00fcr\u00fcnler \u00f6ne \u00e7\u0131kar."), FMath::RoundToInt32(10.f * S));
        case EKind::HighInflation:
            return FString::Printf(TEXT("Y\u00fcksek enflasyon d\u00f6nemi: fiyatlar her ay art\u0131yor (y\u0131ll\u0131k %%%.0f); zamlar\u0131 rafa yans\u0131tmay\u0131 geciktirme."),
                MarketPrices::YearlyInflation(MarketCalendar::DateOf(GameDay).Year) * 100.0);
        case EKind::Recovery:
            return FString::Printf(TEXT("Toparlanma: ekonomi d\u00fczeliyor, m\u00fc\u015fteriler yeniden rahat al\u0131\u015fveri\u015f yap\u0131yor (%%%d daha \u00e7ok m\u00fc\u015fteri)."), FMath::RoundToInt32(4.f * S));
        default: return FString();
        }
    }

    FString EndNews(const FEra& E)
    {
        switch (E.Kind)
        {
        case EKind::CurrencyShock: return TEXT("Kur \u015foku yat\u0131\u015ft\u0131: ithal \u00fcr\u00fcnlerin al\u0131\u015f fiyat\u0131 bundan sonra yava\u015f yava\u015f inecek.");
        case EKind::Recession: return TEXT("Durgunluk geride kald\u0131: m\u00fc\u015fteriler sepetlerini yeniden dolduruyor.");
        case EKind::HighInflation: return TEXT("Y\u00fcksek enflasyon d\u00f6nemi sona erdi: fiyatlar daha yava\u015f art\u0131yor.");
        case EKind::Recovery: return FString();
        default: return FString();
        }
    }
}

TArray<MarketEras::FEra> MarketEras::Plan(ECharacter Character, int32 Seed, int32 ShiftYears, int32 ShiftDays)
{
    TArray<FEra> Result;
    for (const FTemplate& T : Templates)
    {
        const float Strength = StrengthOf(Character, T.Kind, T.Wave, Seed);
        if (Strength <= 0.f) continue;
        FEra E;
        E.Kind = T.Kind;
        E.Wave = T.Wave;
        E.Strength = Strength;
        E.StartYear = T.Year + ShiftYears;
        E.StartDay = MarketCalendar::GameDayOf(T.Year + ShiftYears, T.Month, T.Day) + ShiftDays;
        E.EndDay = MarketCalendar::GameDayOf(T.EndYear + ShiftYears, T.EndMonth, T.EndDay) + ShiftDays;
        Result.Add(E);
    }
    return Result;
}

TArray<MarketEras::FEra> MarketEras::PlanOf(const FMarketState& State)
{
    const FMarketEras& E = State.Eras;
    return Plan(CharacterOfState(State), State.RivalSeed, E.bPlanned ? E.ShiftYears : 0, E.bPlanned ? E.ShiftDays : 0);
}

void MarketEras::Setup(FMarketState& State)
{
    FMarketEras& E = State.Eras;
    E = FMarketEras();
    E.bPlanned = true;
    E.ShiftYears = static_cast<int32>(EraMix(State.RivalSeed, 0xE7A5u) % 5u) - 2;
    E.ShiftDays = static_cast<int32>(EraMix(State.RivalSeed, 0xE7A6u) % 91u) - 45;
    Activate(State);
}

void MarketEras::Activate(const FMarketState& State)
{
    MarketCompetitors::Activate(State); // M35: the street's chain names of this campaign
    FActive& A = ActivePlan();
    A.Timeline = PlanOf(State);
    A.bSet = true;
}

void MarketEras::ActivateNominal(ECharacter Character)
{
    FActive& A = ActivePlan();
    A.Timeline = Plan(Character, 0, 0, 0);
    A.bSet = true;
}

double MarketEras::InflationBump(int32 Year)
{
    const FActive& A = ActivePlan();
    // Never activated (tests, tools): the unshifted high-inflation plan, i.e. the built-in curve.
    return A.bSet ? BumpOf(A.Timeline, Year, false) : BumpOf(Plan(ECharacter::HighInflation, 0, 0, 0), Year, false);
}

double MarketEras::BuiltInBump(int32 Year)
{
    static const TArray<FEra> BuiltIn = Plan(ECharacter::HighInflation, 0, 0, 0);
    return BumpOf(BuiltIn, Year, true);
}

int32 MarketEras::PandemicShiftDays(const FMarketState& State)
{
    const FMarketEras& E = State.Eras;
    if (!E.bPlanned) return 0;
    return MarketCalendar::GameDayOf(2020 + E.ShiftYears, 3, 1) - MarketCalendar::GameDayOf(2020, 3, 1) + E.ShiftDays;
}

bool MarketEras::Current(const FMarketState& State, int32 GameDay, FEra& OutEra)
{
    for (const FEra& E : PlanOf(State))
    {
        const bool bOn = E.Kind == EKind::Pandemic ? MarketOnline::IsPandemic(State, GameDay) : GameDay >= E.StartDay && GameDay <= E.EndDay;
        if (bOn) { OutEra = E; return true; }
    }
    return false;
}

float MarketEras::BudgetFactor(const FMarketState& State)
{
    FEra E;
    if (!Current(State, State.Day, E)) return 1.f;
    switch (E.Kind)
    {
    case EKind::Recession: return 1.f - 0.10f * E.Strength;
    case EKind::HighInflation: return 1.f - 0.05f * E.Strength;
    case EKind::Recovery: return 1.f + 0.05f * E.Strength;
    default: return 1.f;
    }
}

FString MarketEras::Name(EKind Kind)
{
    switch (Kind)
    {
    case EKind::CurrencyShock: return TEXT("kur \u015foku");
    case EKind::Recession: return TEXT("durgunluk");
    case EKind::Pandemic: return TEXT("b\u00fcy\u00fck salg\u0131n");
    case EKind::HighInflation: return TEXT("y\u00fcksek enflasyon");
    case EKind::Recovery: return TEXT("toparlanma");
    default: return TEXT("?");
    }
}

FString MarketEras::Summary(const FMarketState& State)
{
    FEra E;
    if (!Current(State, State.Day, E)) return FString();
    switch (E.Kind)
    {
    case EKind::CurrencyShock: return FString::Printf(TEXT("Ekonomi: kur \u015foku; kahve ve ya\u011f gibi ithal \u00fcr\u00fcnler %%%d pahal\u0131."),
        FMath::RoundToInt32((ImportCostFactor(State, GroupImportShare(static_cast<uint8>(MarketGoods::EGroup::TeaCoffee)), State.Day) - 1.f) * 100.f));
    case EKind::Recession: return FString::Printf(TEXT("Ekonomi: durgunluk; sepetler %%%d k\u00fc\u00e7\u00fck."), FMath::RoundToInt32(10.f * E.Strength));
    case EKind::Pandemic: return TEXT("Ekonomi: b\u00fcy\u00fck salg\u0131n; insanlar evde, sipari\u015fler art\u0131yor.");
    case EKind::HighInflation: return FString::Printf(TEXT("Ekonomi: y\u00fcksek enflasyon; y\u0131ll\u0131k %%%.0f."), MarketPrices::YearlyInflation(MarketCalendar::DateOf(State.Day).Year) * 100.0);
    case EKind::Recovery: return FString::Printf(TEXT("Ekonomi: toparlanma; %%%d daha \u00e7ok m\u00fc\u015fteri."), FMath::RoundToInt32(4.f * E.Strength));
    default: return FString();
    }
}

void MarketEras::CloseDay(FMarketState& State)
{
    Activate(State);
    FMarketEras& S = State.Eras;
    const TArray<FEra> Eras = PlanOf(State);
    const int32 Today = State.Day; // the day about to be played
    for (int32 I = 0; I < Eras.Num() && I < 31; ++I)
    {
        const FEra& E = Eras[I];
        const int32 Bit = 1 << I;
        if (!(S.Started & Bit) && Today >= E.StartDay)
        {
            S.Started |= Bit;
            if (Today <= E.EndDay && E.Kind != EKind::Pandemic)
            {
                AddEffects(State, E, Today);
                State.DayNews.Add(StartNews(E, Today));
            }
        }
        if ((S.Started & Bit) && !(S.Ended & Bit) && Today > E.EndDay)
        {
            S.Ended |= Bit;
            const FString Line = E.Kind == EKind::Pandemic ? FString() : EndNews(E);
            if (!Line.IsEmpty()) State.DayNews.Add(Line);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------
// B7: era factors for departments, purchase prices and rival chains

namespace MarketEras
{

    constexpr int32 GoodsCount = static_cast<int32>(EGoods::Count);
    // Demand change at full strength by era (rows: EKind) and goods (columns: EGoods).
    const float DemandTable[static_cast<int32>(EKind::Count)][GoodsCount] =
    {
        //  Grocery  Fresh   Electr. Cloth.  Toys    Home
        {   0.f,     0.f,    -0.15f, -0.05f, -0.05f, -0.05f },  // currency shock
        {   0.f,    -0.03f,  -0.25f, -0.20f, -0.20f, -0.15f },  // recession
        {   0.f,     0.10f,   0.15f, -0.30f,  0.05f,  0.10f },  // epidemic
        {   0.f,    -0.03f,  -0.05f, -0.10f, -0.10f, -0.05f },  // high inflation
        {   0.f,     0.02f,   0.15f,  0.12f,  0.10f,  0.08f },  // recovery
    };
    // Rival chains by era: revenue change, its tilt per 0.1 of price index below the list, openings x.
    struct FChainRow { float Revenue; float TradeDown; float Openings; };
    const FChainRow ChainTable[static_cast<int32>(EKind::Count)] =
    {
        { -0.03f,  0.02f, 0.6f },   // currency shock
        { -0.08f,  0.06f, 0.4f },   // recession
        { -0.03f,  0.00f, 0.6f },   // epidemic
        { -0.03f,  0.04f, 0.8f },   // high inflation
        {  0.05f, -0.02f, 1.5f },   // recovery
    };

    uint32 CountryHash(const FString& Country)
    {
        uint32 Hash = 2166136261u;
        for (const TCHAR Char : Country) { Hash ^= static_cast<uint32>(Char) & 0xFFFFu; Hash *= 16777619u; }
        return Hash;
    }

    bool IsHome(const FMarketState& State, const FString& Country)
    {
        return Country.IsEmpty() || Country == State.CountryId || (State.CountryId.IsEmpty() && Country == MarketCountry::DefaultId());
    }

    // The plan of a country on the campaign's timing.
    TArray<FEra> PlanFor(const FMarketState& State, const FString& Country)
    {
        if (IsHome(State, Country)) return PlanOf(State);
        const MarketCountry::FProfile* Pack = MarketCountry::Find(Country);
        const ECharacter Character = static_cast<ECharacter>(Pack ? static_cast<uint8>(Pack->Character) : static_cast<uint8>(ECharacter::Stable));
        const FMarketEras& E = State.Eras;
        return Plan(Character, State.RivalSeed ^ static_cast<int32>(CountryHash(Country)), E.bPlanned ? E.ShiftYears : 0, E.bPlanned ? E.ShiftDays : 0);
    }

    // 0..1: how much of an era is felt on a day (in over RampInDays, out over FadeDays after its end).
    float EraWeight(const FMarketState& State, const FEra& E, int32 Day, int32 FadeDays, bool bHome)
    {
        int32 Start = E.StartDay, End = E.EndDay;
        if (E.Kind == EKind::Pandemic && bHome)
        {
            // The epidemic is MarketOnline's profile (always part of the game, M32).
            Start = MarketOnline::PandemicStart(State);
            End = MarketOnline::PandemicEnd(State) - 1;
        }
        if (Day < Start) return 0.f;
        if (Day <= End) return FMath::Min(1.f, static_cast<float>(Day - Start + 1) / RampInDays);
        const float Peak = FMath::Min(1.f, static_cast<float>(End - Start + 1) / RampInDays);
        const float Left = 1.f - static_cast<float>(Day - End) / FadeDays;
        return Left > 0.f ? Peak * Left : 0.f;
    }

    bool Contains(const FString& Text, const TCHAR* Word) { return Text.Contains(Word, ESearchCase::IgnoreCase); }
}

MarketEras::EGoods MarketEras::GoodsOf(const FString& Id)
{
    for (const TCHAR* W : { TEXT("elektron"), TEXT("electron"), TEXT("telefon"), TEXT("phone"), TEXT("beyaz e"), TEXT("bilgisayar"), TEXT("computer"), TEXT("tv") })
        if (Contains(Id, W)) return EGoods::Electronics;
    for (const TCHAR* W : { TEXT("giyim"), TEXT("tekstil"), TEXT("ayakkab"), TEXT("cloth"), TEXT("textile"), TEXT("apparel"), TEXT("shoe") })
        if (Contains(Id, W)) return EGoods::Clothing;
    for (const TCHAR* W : { TEXT("oyuncak"), TEXT("toy") })
        if (Contains(Id, W)) return EGoods::Toys;
    for (const TCHAR* W : { TEXT("manav"), TEXT("kasap"), TEXT("f\u0131r\u0131n"), TEXT("firin"), TEXT("\u015fark\u00fcteri"), TEXT("sarkuteri"), TEXT("bal\u0131k"), TEXT("balik"),
                            TEXT("meyve"), TEXT("sebze"), TEXT("taze"), TEXT("fresh"), TEXT("produce"), TEXT("butcher"), TEXT("bakery"), TEXT("deli"), TEXT("fish") })
        if (Contains(Id, W)) return EGoods::Fresh;
    for (const TCHAR* W : { TEXT("z\u00fccaciye"), TEXT("zuccaciye"), TEXT("mutfak"), TEXT("h\u0131rdavat"), TEXT("hirdavat"), TEXT("bah\u00e7e"), TEXT("bahce"), TEXT("ev "),
                            TEXT("ev-"), TEXT("ev_"), TEXT("ev."), TEXT("home"), TEXT("kitchen"), TEXT("garden"), TEXT("hardware") })
        if (Contains(Id, W)) return EGoods::Home;
    if (Id.Equals(TEXT("ev"), ESearchCase::IgnoreCase)) return EGoods::Home;
    return EGoods::Grocery;
}

FString MarketEras::GoodsName(EGoods Goods)
{
    switch (Goods)
    {
    case EGoods::Fresh: return TEXT("taze \u00fcr\u00fcn");
    case EGoods::Electronics: return TEXT("elektronik");
    case EGoods::Clothing: return TEXT("giyim");
    case EGoods::Toys: return TEXT("oyuncak");
    case EGoods::Home: return TEXT("ev e\u015fyas\u0131");
    default: return TEXT("g\u0131da");
    }
}

float MarketEras::DemandFactor(const FMarketState& State, EGoods Goods, int32 Day, const FString& Country)
{
    const int32 G = static_cast<int32>(Goods);
    if (G < 0 || G >= GoodsCount) return 1.f;
    const bool bHome = IsHome(State, Country);
    float Result = 1.f;
    for (const FEra& E : PlanFor(State, Country))
    {
        const float W = EraWeight(State, E, Day, DemandFadeDays, bHome);
        if (W > 0.f) Result *= 1.f + DemandTable[static_cast<int32>(E.Kind)][G] * E.Strength * W;
    }
    return FMath::Clamp(Result, 0.4f, 1.6f);
}

float MarketEras::NonFoodDemand(const FMarketState& State, int32 Day, const FString& Country)
{
    float Sum = 0.f;
    for (EGoods G : { EGoods::Electronics, EGoods::Clothing, EGoods::Toys, EGoods::Home }) Sum += DemandFactor(State, G, Day, Country);
    return Sum / 4.f;
}

float MarketEras::FreshDemand(const FMarketState& State, int32 Day, const FString& Country)
{
    return DemandFactor(State, EGoods::Fresh, Day, Country);
}

float MarketEras::ImportShare(EGoods Goods)
{
    switch (Goods)
    {
    case EGoods::Electronics: return 1.f;
    case EGoods::Toys: return 0.7f;
    case EGoods::Home: return 0.5f;
    case EGoods::Clothing: return 0.4f;
    case EGoods::Grocery: return 0.15f;
    default: return 0.f;
    }
}

float MarketEras::GroupImportShare(uint8 Group)
{
    using MarketGoods::EGroup;
    switch (static_cast<EGroup>(Group))
    {
    case EGroup::TeaCoffee: case EGroup::OilSauce: return 0.35f;
    case EGroup::PersonalCare: case EGroup::Household: return 0.3f;
    case EGroup::Sweets: return 0.2f;
    case EGroup::Snacks: return 0.15f;
    case EGroup::Drinks: return 0.1f;
    default: return 0.f;
    }
}

float MarketEras::ImportCostFactor(const FMarketState& State, float Share, int32 Day, const FString& Country)
{
    if (Share <= 0.f) return 1.f;
    const bool bHome = IsHome(State, Country);
    float Jump = 0.f;
    for (const FEra& E : PlanFor(State, Country))
        if (E.Kind == EKind::CurrencyShock) Jump += ShockPeak * E.Strength * EraWeight(State, E, Day, CostFadeDays, bHome);
    return 1.f + FMath::Clamp(Share, 0.f, 1.f) * Jump;
}

float MarketEras::ImportCostFactor(const FMarketState& State, EGoods Goods, int32 Day, const FString& Country)
{
    return ImportCostFactor(State, ImportShare(Goods), Day, Country);
}

float MarketEras::ChainRevenueFactor(const FMarketState& State, const FString& Country, float PriceIndex, int32 Day)
{
    const bool bHome = IsHome(State, Country);
    const float Cheapness = FMath::Clamp((1.f - PriceIndex) / 0.1f, -2.f, 2.f);
    float Result = 1.f;
    for (const FEra& E : PlanFor(State, Country))
    {
        const float W = EraWeight(State, E, Day, DemandFadeDays, bHome);
        const FChainRow& Row = ChainTable[static_cast<int32>(E.Kind)];
        if (W > 0.f) Result *= 1.f + (Row.Revenue + Row.TradeDown * Cheapness) * E.Strength * W;
    }
    return FMath::Clamp(Result, 0.6f, 1.4f);
}

float MarketEras::ChainOpeningFactor(const FMarketState& State, const FString& Country, int32 Day)
{
    const bool bHome = IsHome(State, Country);
    float Result = 1.f;
    for (const FEra& E : PlanFor(State, Country))
    {
        const float W = EraWeight(State, E, Day, DemandFadeDays, bHome);
        if (W > 0.f) Result *= 1.f + (ChainTable[static_cast<int32>(E.Kind)].Openings - 1.f) * E.Strength * W;
    }
    return FMath::Clamp(Result, 0.2f, 2.f);
}

int32 MarketEras::ChainRedTurnsToSell(const FMarketState& State, const FString& Country, int32 Day)
{
    const bool bHome = IsHome(State, Country);
    for (const FEra& E : PlanFor(State, Country))
        if ((E.Kind == EKind::Recession || E.Kind == EKind::CurrencyShock) && E.Strength * EraWeight(State, E, Day, DemandFadeDays, bHome) >= 0.5f) return 2;
    return 3;
}

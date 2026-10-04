#include "MarketStrategy.h"
#include "MarketEconomy.h"
#include "MarketBranches.h"
#include "MarketChains.h"
#include "MarketCountry.h"
#include "MarketEvents.h"
#include "MarketLedger.h"
#include "MarketPrices.h"
#include "MarketBanking.h"
#include "MarketManagers.h"
#include "MarketStart.h"

namespace MarketStrategyLocal
{
    using namespace MarketStrategy;

    uint32 Mix(int32 Seed, int32 Day, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), static_cast<uint32>(Day), Salt };
        for (uint32 Part : Parts) for (int32 B = 0; B < 4; ++B) { Hash ^= (Part >> (B * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }
    bool Roll(const FMarketState& State, int32 Day, uint32 Salt, float Chance)
    {
        return (Mix(State.RivalSeed, Day, Salt) % 10000u) < static_cast<uint32>(FMath::Clamp(Chance, 0.f, 1.f) * 10000.f);
    }
    FString Key(const FString& Country, const FString& Province) { return Country + TEXT("|") + Province; }
    FString CountryOr(const FMarketState& State, const FString& Country) { return Country.IsEmpty() ? State.CountryId : Country; }

    // Every rival chain's shops in a province (not ours, not gone).
    int32 ChainStoresIn(const FMarketChain& C, const FString& Province)
    {
        for (const FMarketChainSpot& S : C.Spots) if (S.Province == Province) return S.Stores;
        return 0;
    }
    int32 LeaderIndex(const FMarketState& State, const FString& Country, const FString& Province)
    {
        int32 Best = INDEX_NONE, Most = 0;
        for (int32 I = 0; I < State.Rivals.Chains.Num(); ++I)
        {
            const FMarketChain& C = State.Rivals.Chains[I];
            if (C.bGone || C.bOurs || C.Country != Country) continue;
            const int32 N = ChainStoresIn(C, Province);
            if (N > Most) { Most = N; Best = I; }
        }
        return Best;
    }
    const FMarketPush* LastPush(const FMarketState& State, const FString& Country, const FString& Province)
    {
        const FMarketPush* Found = nullptr;
        for (const FMarketPush& P : State.Strategy.Pushes) if (P.Country == Country && P.Province == Province) Found = &P;
        return Found;
    }
    int32 Tier(int64 Value, int64 A, int64 B, int64 C) { return Value >= C ? 3 : Value >= B ? 2 : Value >= A ? 1 : 0; }

    void Celebrate(FMarketState& State, const FString& Title, const FString& Text, uint8 Importance)
    {
        FMarketCelebration C;
        C.Day = FMath::Max(1, State.Day - 1);
        C.Title = Title;
        C.Text = Text;
        C.Importance = Importance;
        State.Goals.Celebrations.Add(C);
        while (State.Goals.Celebrations.Num() > 60) State.Goals.Celebrations.RemoveAt(0);
    }

    FString PathName(EPath Path)
    {
        switch (Path)
        {
        case EPath::Champion: return TEXT("\u0130l \u015fampiyonu");
        case EPath::Efficient: return TEXT("Verimli \u015firket");
        case EPath::People: return TEXT("\u0130nsan yeti\u015ftiren");
        case EPath::Favourite: return TEXT("Halk\u0131n marketi");
        default: return FString();
        }
    }

    int64 VerticalCost(const FMarketState& State, EVertical Kind)
    {
        const int64 Revenue = FMath::Max<int64>(0, MarketBanking::Picture(State).Revenue);
        if (Kind == EVertical::Production) return FMath::Max<int64>(MarketPrices::Scaled(2000000, State.Day), Revenue * 3 / 100);
        if (Kind == EVertical::Logistics) return FMath::Max<int64>(MarketPrices::Scaled(800000, State.Day), Revenue / 100);
        return 0;
    }

    bool HasPending(const FMarketState& State, const TCHAR* Prefix)
    {
        for (const FMarketDecision& D : State.Decisions) if (D.Id.StartsWith(Prefix)) return true;
        return false;
    }

    void OfferFork(FMarketState& State, const FString& Id, const FString& Title, const FString& Text, const TArray<FString>& Options)
    {
        FMarketDecision D;
        D.Id = Id;
        D.Title = Title;
        D.Text = Text;
        D.Options = Options;
        D.Deadline = State.Day + DecisionDays;
        D.DefaultOption = Options.Num() - 1; // "not now"
        MarketEvents::Offer(State, D);
        State.DayNews.Add(FString::Printf(TEXT("Yol ayr\u0131m\u0131: %s. Karar %d g\u00fcn i\u00e7inde; \u00d6zet sayfas\u0131nda bekliyor."), *Title, DecisionDays));
    }
}

// ---------------------------------------------------------------------------------------------------------------
// M48 province markets

int32 MarketStrategy::RivalStoresIn(const FMarketState& State, const FString& InCountry, const FString& Province, FString* OutLeader)
{
    using namespace MarketStrategyLocal;
    const FString Country = CountryOr(State, InCountry);
    const int32 Leader = LeaderIndex(State, Country, Province);
    if (OutLeader) *OutLeader = Leader != INDEX_NONE ? State.Rivals.Chains[Leader].Name : FString();
    return Leader != INDEX_NONE ? ChainStoresIn(State.Rivals.Chains[Leader], Province) : 0;
}

bool MarketStrategy::IsChampion(const FMarketState& State, const FString& InCountry, const FString& Province)
{
    const FString Country = MarketStrategyLocal::CountryOr(State, InCountry);
    const int32 Ours = MarketBranches::ShopsIn(State, Country, Province);
    return Ours >= 2 && Ours > RivalStoresIn(State, Country, Province);
}

int32 MarketStrategy::ChampionCount(const FMarketState& State)
{
    int32 Count = 0;
    for (const FString& K : OurProvinces(State, 1000))
    {
        FString Country, Province;
        if (K.Split(TEXT("|"), &Country, &Province) && IsChampion(State, Country, Province)) ++Count;
    }
    return Count;
}

int64 MarketStrategy::PushDailyCost(const FMarketState& State, const FString& InCountry, const FString& Province)
{
    const FString Country = MarketStrategyLocal::CountryOr(State, InCountry);
    return MarketPrices::Scaled(PushDailyPerShopStart, State.Day) * MarketBranches::ShopsIn(State, Country, Province);
}

bool MarketStrategy::PushActive(const FMarketState& State, const FString& InCountry, const FString& Province, int32 Day)
{
    const FString Country = MarketStrategyLocal::CountryOr(State, InCountry);
    for (const FMarketPush& P : State.Strategy.Pushes)
        if (P.Country == Country && P.Province == Province && Day >= P.StartDay && Day <= P.EndDay) return true;
    return false;
}

bool MarketStrategy::CanPush(const FMarketState& State, const FString& InCountry, const FString& Province, FString& OutReason)
{
    using namespace MarketStrategyLocal;
    const FString Country = CountryOr(State, InCountry);
    const FString Name = MarketChains::ProvinceName(Country, Province);
    if (MarketBranches::ShopsIn(State, Country, Province) < PushMinShops) { OutReason = FString::Printf(TEXT("%s ata\u011f\u0131 i\u00e7in orada en az %d ma\u011faza gerekir."), *Name, PushMinShops); return false; }
    if (const FMarketPush* Last = LastPush(State, Country, Province))
    {
        if (State.Day <= Last->EndDay) { OutReason = FString::Printf(TEXT("%s: atak s\u00fcr\u00fcyor (%d g\u00fcn kald\u0131)."), *Name, Last->EndDay - State.Day + 1); return false; }
        if (State.Day < Last->EndDay + PushRestDays) { OutReason = FString::Printf(TEXT("%s pazar\u0131 son ataktan dinleniyor: %d g\u00fcn sonra."), *Name, Last->EndDay + PushRestDays - State.Day); return false; }
    }
    const int64 Need = PushDailyCost(State, Country, Province) * 15;
    if (State.Cash < Need) { OutReason = FString::Printf(TEXT("Ata\u011f\u0131n ilk iki haftas\u0131 i\u00e7in kasada %s olmal\u0131."), *MarketCountry::Money(Need)); return false; }
    return true;
}

bool MarketStrategy::StartPush(FMarketState& State, const FString& InCountry, const FString& Province, FString& OutMessage)
{
    const FString Country = MarketStrategyLocal::CountryOr(State, InCountry);
    if (!CanPush(State, Country, Province, OutMessage)) return false;
    FMarketPush P;
    P.Country = Country;
    P.Province = Province;
    P.StartDay = State.Day;
    P.EndDay = State.Day + PushDays - 1;
    State.Strategy.Pushes.Add(P);
    while (State.Strategy.Pushes.Num() > 40) State.Strategy.Pushes.RemoveAt(0);
    OutMessage = FString::Printf(TEXT("%s ata\u011f\u0131 ba\u015flad\u0131: %d g\u00fcn yerel kampanya, bro\u015f\u00fcr ve kasa \u00f6n\u00fc indirimi, g\u00fcnde %s. \u015eubelerimize %%%.0f daha \u00e7ok m\u00fc\u015fteri gelir; rakip fiyat k\u0131rarak cevap verebilir. Sonunda k\u00fc\u00e7\u00fck rakipler oradaki bir ma\u011fazas\u0131n\u0131 kapatabilir."),
        *MarketChains::ProvinceName(Country, Province), PushDays, *MarketCountry::Money(PushDailyCost(State, Country, Province)), (PushPull - 1.f) * 100.f);
    return true;
}

TArray<FString> MarketStrategy::OurProvinces(const FMarketState& State, int32 MaxCount)
{
    TMap<FString, int32> Count;
    Count.FindOrAdd(MarketStrategyLocal::Key(State.CountryId, MarketStart::HomeProvince(State))) += 1;
    for (const FMarketBranch& B : State.Branches)
        if (B.Stage != static_cast<uint8>(MarketBranches::EStage::Closed))
            Count.FindOrAdd(MarketStrategyLocal::Key(MarketBranches::CountryOf(State, B), B.Province)) += 1;
    TArray<TPair<FString, int32>> List;
    for (const TPair<FString, int32>& P : Count) List.Add(P);
    List.Sort([](const TPair<FString, int32>& A, const TPair<FString, int32>& B) { return A.Value == B.Value ? A.Key < B.Key : A.Value > B.Value; });
    TArray<FString> Out;
    for (int32 I = 0; I < List.Num() && I < MaxCount; ++I) Out.Add(List[I].Key);
    return Out;
}

FString MarketStrategy::ProvinceLine(const FMarketState& State, const FString& InCountry, const FString& Province)
{
    const FString Country = MarketStrategyLocal::CountryOr(State, InCountry);
    FString Leader;
    const int32 Theirs = RivalStoresIn(State, Country, Province, &Leader);
    const int32 Ours = MarketBranches::ShopsIn(State, Country, Province);
    FString Line = FString::Printf(TEXT("%s: %d ma\u011fazam\u0131z"), *MarketChains::ProvinceName(Country, Province), Ours);
    if (Theirs > 0) Line += FString::Printf(TEXT(" \u00b7 en b\u00fcy\u00fck rakip %s %d"), *Leader, Theirs);
    else Line += TEXT(" \u00b7 rakip zincir yok");
    if (IsChampion(State, Country, Province)) Line += TEXT(" \u00b7 \u0130L \u015eAMP\u0130YONU");
    for (const FMarketPush& P : State.Strategy.Pushes)
        if (P.Country == Country && P.Province == Province && State.Day <= P.EndDay) Line += FString::Printf(TEXT(" \u00b7 atak %d g\u00fcn"), P.EndDay - State.Day + 1);
    return Line;
}

// ---------------------------------------------------------------------------------------------------------------
// M49 strategic forks

MarketStrategy::EFocus MarketStrategy::Focus(const FMarketState& State) { return static_cast<EFocus>(State.Strategy.Focus); }
MarketStrategy::EGrowth MarketStrategy::Growth(const FMarketState& State) { return static_cast<EGrowth>(State.Strategy.Growth); }
MarketStrategy::EVertical MarketStrategy::Vertical(const FMarketState& State) { return static_cast<EVertical>(State.Strategy.Vertical); }

bool MarketStrategy::ProductionReady(const FMarketState& State)
{
    return Vertical(State) == EVertical::Production && State.Day >= State.Strategy.VerticalDay + ProductionDays;
}

bool MarketStrategy::Resolve(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& Decision, int32 Option, FString& OutMessage)
{
    using namespace MarketStrategyLocal;
    FMarketStrategyState& S = State.Strategy;
    const bool bLater = Option >= Decision.Options.Num() - 1;
    if (Decision.Id == TEXT("strategy.focus"))
    {
        if (bLater) { S.AskFocusDay = State.Day + AskAgainDays; OutMessage = TEXT("Odak karar\u0131 ertelendi; alt\u0131 ay sonra yeniden sorulur."); return true; }
        S.Focus = static_cast<uint8>(Option + 1);
        S.FocusDay = State.Day;
        OutMessage = FString::Printf(TEXT("Zincirin oda\u011f\u0131: %s."), *Decision.Options[Option]);
        return true;
    }
    if (Decision.Id == TEXT("strategy.growth"))
    {
        if (bLater) { S.AskGrowthDay = State.Day + AskAgainDays; OutMessage = TEXT("B\u00fcy\u00fcme modeli karar\u0131 ertelendi; alt\u0131 ay sonra yeniden sorulur."); return true; }
        S.Growth = static_cast<uint8>(Option + 1);
        S.GrowthDay = State.Day;
        OutMessage = FString::Printf(TEXT("B\u00fcy\u00fcme modeli: %s. Yeni a\u00e7\u0131l\u0131\u015flar bu modelle yap\u0131l\u0131r."), *Decision.Options[Option]);
        return true;
    }
    if (Decision.Id == TEXT("strategy.vertical"))
    {
        if (bLater) { S.AskVerticalDay = State.Day + AskAgainDays; OutMessage = TEXT("B\u00fcy\u00fck yat\u0131r\u0131m karar\u0131 ertelendi; alt\u0131 ay sonra yeniden sorulur."); return true; }
        const EVertical Kind = static_cast<EVertical>(Option + 1);
        const int64 Cost = VerticalCost(State, Kind);
        if (Cost > 0 && State.Cash < Cost)
        {
            S.AskVerticalDay = State.Day + 90;
            OutMessage = FString::Printf(TEXT("Kasada %s yok: yat\u0131r\u0131m yap\u0131lamad\u0131. \u00dc\u00e7 ay sonra yeniden sorulur (banka kredisiyle de kar\u015f\u0131lanabilir)."), *MarketCountry::Money(Cost));
            return true;
        }
        if (Cost > 0)
        {
            State.Cash -= Cost;
            MarketLedger::Post(State, MarketLedger::EAccount::Investment, -Cost, true, MarketLedger::HeadOfficeStore);
        }
        S.Vertical = static_cast<uint8>(Kind);
        S.VerticalDay = State.Day;
        OutMessage = Kind == EVertical::Production
            ? FString::Printf(TEXT("Kendi \u00fcretim tesisimiz kuruluyor (%s). Bir y\u0131l sonra \u00f6zel markal\u0131 \u00fcr\u00fcnler raflara gelir."), *MarketCountry::Money(Cost))
            : Kind == EVertical::Logistics
                ? FString::Printf(TEXT("Lojistik a\u011f\u0131na yat\u0131r\u0131m yap\u0131ld\u0131 (%s): depodan \u015fubeye kay\u0131p yar\u0131ya iner, raflar daha dolu kal\u0131r."), *MarketCountry::Money(Cost))
                : FString(TEXT("Sadakat program\u0131 ba\u015flad\u0131: m\u00fc\u015fteriler daha s\u0131k gelir; her ay \u015fube cirosunun binde d\u00f6rd\u00fc kadar \u00f6denir."));
        return true;
    }
    OutMessage = TEXT("Bilinmeyen strateji karar\u0131.");
    return false;
}

FString MarketStrategy::StrategyLine(const FMarketState& State)
{
    TArray<FString> Parts;
    switch (Focus(State))
    {
    case EFocus::Home: Parts.Add(TEXT("Odak: ev b\u00f6lgesinin kalesi")); break;
    case EFocus::BigCities: Parts.Add(TEXT("Odak: b\u00fcy\u00fck \u015fehirler")); break;
    case EFocus::SmallTowns: Parts.Add(TEXT("Odak: k\u00fc\u00e7\u00fck \u015fehirler")); break;
    default: break;
    }
    switch (Growth(State))
    {
    case EGrowth::Fast: Parts.Add(TEXT("B\u00fcy\u00fcme: h\u0131zl\u0131 yay\u0131lma")); break;
    case EGrowth::Owned: Parts.Add(TEXT("B\u00fcy\u00fcme: kendi binalar\u0131m\u0131z")); break;
    case EGrowth::Flexible: Parts.Add(TEXT("B\u00fcy\u00fcme: k\u0131sa kira")); break;
    default: break;
    }
    switch (Vertical(State))
    {
    case EVertical::Production: Parts.Add(ProductionReady(State) ? FString(TEXT("Yat\u0131r\u0131m: kendi \u00fcretimimiz")) : FString::Printf(TEXT("Yat\u0131r\u0131m: \u00fcretim tesisi (%d g\u00fcn sonra haz\u0131r)"), State.Strategy.VerticalDay + ProductionDays - State.Day)); break;
    case EVertical::Logistics: Parts.Add(TEXT("Yat\u0131r\u0131m: lojistik a\u011f\u0131")); break;
    case EVertical::Loyalty: Parts.Add(TEXT("Yat\u0131r\u0131m: sadakat program\u0131")); break;
    default: break;
    }
    return FString::Join(Parts, TEXT(" \u00b7 "));
}

// ---------------------------------------------------------------------------------------------------------------
// M50 paths

int32 MarketStrategy::PathTier(const FMarketState& State, EPath Path)
{
    const int32 I = static_cast<int32>(Path);
    return State.Strategy.Tiers.IsValidIndex(I) ? State.Strategy.Tiers[I] : 0;
}

int32 MarketStrategy::MeasurePathTier(const FMarketState& State, EPath Path)
{
    using namespace MarketStrategyLocal;
    switch (Path)
    {
    case EPath::Champion:
        return Tier(ChampionCount(State), 1, 5, 15);
    case EPath::Efficient:
    {
        const MarketBanking::FPicture P = MarketBanking::Picture(State);
        if (P.Months < 12 || P.Revenue <= 0 || MarketBranches::OpenCount(State) < 2) return 0;
        return Tier(P.Ebitda * 1000 / P.Revenue, 40, 70, 100); // per mille of revenue
    }
    case EPath::People:
    {
        // D9: store managers live on the branches.
        int32 Count = 0, Sum = 0;
        for (const FMarketBranch& B : State.Branches)
            if (B.Stage == static_cast<uint8>(MarketBranches::EStage::Open) && !B.ManagerName.IsEmpty()) { ++Count; Sum += B.ManagerSkill; }
        return Count >= 5 ? Tier(Sum / Count, 60, 68, 76) : 0;
    }
    case EPath::Favourite:
    {
        int32 Count = 0;
        float Sum = 0.f;
        for (const FMarketBranch& B : State.Branches)
            if (B.Stage == static_cast<uint8>(MarketBranches::EStage::Open)) { ++Count; Sum += B.Satisfaction; }
        return Count >= 3 ? Tier(FMath::RoundToInt32(Sum / Count), 62, 72, 82) : 0;
    }
    default: return 0;
    }
}

FString MarketStrategy::PathLine(const FMarketState& State, EPath Path)
{
    using namespace MarketStrategyLocal;
    const int32 T = PathTier(State, Path);
    FString Measure, Steps, Perk;
    switch (Path)
    {
    case EPath::Champion:
        Measure = FString::Printf(TEXT("%d ilde \u015fampiyonuz"), State.Strategy.Champions.Num());
        Steps = TEXT("1 / 5 / 15 il");
        Perk = FString::Printf(TEXT("\u015fampiyon oldu\u011fumuz illerde %%%d daha \u00e7ok m\u00fc\u015fteri"), 2 * FMath::Max(1, T));
        break;
    case EPath::Efficient:
    {
        const MarketBanking::FPicture P = MarketBanking::Picture(State);
        Measure = P.Revenue > 0 ? FString::Printf(TEXT("y\u0131ll\u0131k faaliyet marj\u0131 %%%.1f"), 100.0 * static_cast<double>(P.Ebitda) / static_cast<double>(P.Revenue)) : FString(TEXT("hen\u00fcz bir y\u0131ll\u0131k defter yok"));
        Steps = TEXT("%4 / %7 / %10");
        Perk = FString::Printf(TEXT("bankalar %.2f puan ucuz kredi verir"), 0.25 * FMath::Max(1, T));
        break;
    }
    case EPath::People:
    {
        int32 Count = 0, Sum = 0;
        for (const FMarketBranch& B : State.Branches)
            if (B.Stage == static_cast<uint8>(MarketBranches::EStage::Open) && !B.ManagerName.IsEmpty()) { ++Count; Sum += B.ManagerSkill; }
        Measure = Count > 0 ? FString::Printf(TEXT("%d ma\u011faza m\u00fcd\u00fcr\u00fc, ortalama beceri %d"), Count, Sum / Count) : FString(TEXT("ma\u011faza m\u00fcd\u00fcr\u00fc yok"));
        Steps = TEXT("en az 5 m\u00fcd\u00fcr, beceri 60 / 68 / 76");
        Perk = FString::Printf(TEXT("y\u00f6netim y\u0131lda %d s\u00f6zle\u015fme daha takip eder"), 2 * FMath::Max(1, T));
        break;
    }
    case EPath::Favourite:
    {
        int32 Count = 0;
        float Sum = 0.f;
        for (const FMarketBranch& B : State.Branches)
            if (B.Stage == static_cast<uint8>(MarketBranches::EStage::Open)) { ++Count; Sum += B.Satisfaction; }
        Measure = Count > 0 ? FString::Printf(TEXT("\u015fubelerde ortalama memnuniyet %.0f"), Sum / Count) : FString(TEXT("a\u00e7\u0131k \u015fube yok"));
        Steps = TEXT("en az 3 \u015fube, memnuniyet 62 / 72 / 82");
        Perk = FString::Printf(TEXT("her \u015fubede %%%.1f daha \u00e7ok m\u00fc\u015fteri"), 1.5f * FMath::Max(1, T));
        break;
    }
    default: return FString();
    }
    const FString Stars = T == 0 ? FString(TEXT("\u2014")) : FString::ChrN(T, TCHAR('*'));
    return FString::Printf(TEXT("%s %s \u00b7 %s (basamaklar: %s) \u00b7 %s: %s"), *PathName(Path), *Stars, *Measure, *Steps, T > 0 ? TEXT("kazan\u0131m") : TEXT("ilk basama\u011f\u0131n kazan\u0131m\u0131"), *Perk);
}

// ---------------------------------------------------------------------------------------------------------------
// Factors

float MarketStrategy::ServiceFactor(const FMarketState& State)
{
    return Growth(State) == EGrowth::Fast ? 0.98f : 1.f;
}

float MarketStrategy::PullFactor(const FMarketState& State, const FString& InCountry, const FString& Province, int32 Day)
{
    const FString Country = MarketStrategyLocal::CountryOr(State, InCountry);
    float F = 1.f;
    if (PushActive(State, Country, Province, Day)) F *= PushPull;
    const int32 Champion = PathTier(State, EPath::Champion);
    if (Champion > 0 && State.Strategy.Champions.Contains(MarketStrategyLocal::Key(Country, Province))) F *= 1.f + 0.02f * Champion;
    const int32 Favourite = PathTier(State, EPath::Favourite);
    if (Favourite > 0) F *= 1.f + 0.015f * Favourite;
    if (Vertical(State) == EVertical::Loyalty) F *= 1.05f;
    else if (Vertical(State) == EVertical::Logistics) F *= 1.01f;
    const EFocus FocusNow = Focus(State);
    if (FocusNow != EFocus::None)
    {
        const MarketCountry::FCity* City = MarketCountry::FindCity(Country, Province);
        if (City)
        {
            if (FocusNow == EFocus::BigCities && City->PopulationK >= BigCityK) F *= 1.06f;
            else if (FocusNow == EFocus::SmallTowns && City->PopulationK < SmallTownK) F *= 1.05f;
            else if (FocusNow == EFocus::Home && Country == State.CountryId)
            {
                const MarketCountry::FCity* Home = MarketCountry::FindCity(State.CountryId, MarketStart::HomeProvince(State));
                if (Home && Home->Region == City->Region) F *= 1.06f;
            }
        }
    }
    return F;
}

float MarketStrategy::ShelfPriceFactor(const FMarketState& State) { return ProductionReady(State) ? 1.02f : 1.f; }
float MarketStrategy::DepotLossFactor(const FMarketState& State) { return Vertical(State) == EVertical::Logistics ? 0.5f : 1.f; }

float MarketStrategy::FitOutFactor(const FMarketState& State)
{
    return Growth(State) == EGrowth::Fast ? 0.75f : Growth(State) == EGrowth::Owned ? 1.6f : 1.f;
}

float MarketStrategy::RentFactor(const FMarketState& State, const FString& InCountry, const FString& Province)
{
    float F = Growth(State) == EGrowth::Owned ? 0.55f : Growth(State) == EGrowth::Flexible ? 1.05f : 1.f;
    if (Focus(State) == EFocus::SmallTowns)
    {
        const MarketCountry::FCity* City = MarketCountry::FindCity(MarketStrategyLocal::CountryOr(State, InCountry), Province);
        if (City && City->PopulationK < SmallTownK) F *= 0.85f;
    }
    return F;
}

float MarketStrategy::HastyFactor(const FMarketState& State) { return Growth(State) == EGrowth::Flexible ? 0.5f : 1.f; }
float MarketStrategy::CapacityFactor(const FMarketState& State) { return Growth(State) == EGrowth::Fast ? 1.25f : 1.f; }
int32 MarketStrategy::CapacityBonus(const FMarketState& State) { return 2 * PathTier(State, EPath::People); }
double MarketStrategy::RateDiscount(const FMarketState& State) { return 0.0025 * PathTier(State, EPath::Efficient); }

// ---------------------------------------------------------------------------------------------------------------
// Day close

void MarketStrategy::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    using namespace MarketStrategyLocal;
    FMarketStrategyState& S = State.Strategy;
    TArray<FString>& News = State.DayNews;
    const int32 Open = MarketBranches::OpenCount(State);

    // The forks, one at a time.
    if (!HasPending(State, TEXT("strategy.")))
    {
        if (S.Focus == 0 && Open >= FocusAt && State.Day >= S.AskFocusDay)
            OfferFork(State, TEXT("strategy.focus"), TEXT("Zincir nereye y\u00f6nelsin?"),
                TEXT("Be\u015f \u015fubeyle art\u0131k bir zincirsin. Nerede g\u00fc\u00e7l\u00fc olmak istedi\u011fine karar ver: bu se\u00e7im kal\u0131c\u0131d\u0131r ve o yerlerde m\u00fc\u015fteri \u00e7eker."),
                { TEXT("Ev b\u00f6lgesinin kalesi: ana b\u00f6lgemizde %6 daha \u00e7ok m\u00fc\u015fteri"),
                  TEXT("B\u00fcy\u00fck \u015fehirler: milyonluk illerde %6 daha \u00e7ok m\u00fc\u015fteri"),
                  TEXT("K\u00fc\u00e7\u00fck \u015fehirler: yar\u0131m milyonun alt\u0131ndaki illerde %5 daha \u00e7ok m\u00fc\u015fteri, kira %15 ucuz"),
                  TEXT("\u015eimdilik karar verme") });
        else if (S.Growth == 0 && Open >= GrowthAt && State.Day >= S.AskGrowthDay)
            OfferFork(State, TEXT("strategy.growth"), TEXT("Nas\u0131l b\u00fcy\u00fcyece\u011fiz?"),
                TEXT("Yirmi be\u015f \u015fube: b\u00fcy\u00fcmenin yolu bundan sonra \u015firketin \u015feklini belirler. Bu se\u00e7im yeni a\u00e7\u0131l\u0131\u015flar\u0131 etkiler."),
                { TEXT("H\u0131zl\u0131 yay\u0131lma: tadilat %25 ucuz, y\u00f6netim %25 daha \u00e7ok a\u00e7\u0131l\u0131\u015f takip eder; hizmet %2 d\u00fc\u015fer"),
                  TEXT("Kendi binalar\u0131m\u0131z: yeni kiralar %45 ucuz, ama tadilat ve sat\u0131n alma %60 pahal\u0131"),
                  TEXT("K\u0131sa kira: kira %5 pahal\u0131, ama aceleyle se\u00e7ilen k\u00f6t\u00fc yer riski yar\u0131ya iner"),
                  TEXT("\u015eimdilik karar verme") });
        else if (S.Vertical == 0 && Open >= VerticalAt && State.Day >= S.AskVerticalDay)
            OfferFork(State, TEXT("strategy.vertical"), TEXT("B\u00fcy\u00fck yat\u0131r\u0131m"),
                FString::Printf(TEXT("Y\u00fcz \u015fube: art\u0131k tedarik zincirine el atabiliriz. \u00dcretim tesisi %s, lojistik a\u011f\u0131 %s tutar."),
                    *MarketCountry::Money(VerticalCost(State, EVertical::Production)), *MarketCountry::Money(VerticalCost(State, EVertical::Logistics))),
                { TEXT("Kendi \u00fcretimimiz: bir y\u0131l sonra \u00f6zel markayla raf fiyatlar\u0131 %2 k\u00e2rl\u0131"),
                  TEXT("Lojistik a\u011f\u0131: depodan \u015fubeye kay\u0131p yar\u0131ya iner, raflar daha dolu"),
                  TEXT("Sadakat program\u0131: yat\u0131r\u0131m yok, %5 daha \u00e7ok m\u00fc\u015fteri, her ay cironun binde d\u00f6rd\u00fc"),
                  TEXT("\u015eimdilik karar verme") });
    }
    if (S.Vertical == static_cast<uint8>(EVertical::Production) && State.Day == S.VerticalDay + ProductionDays)
        News.Add(TEXT("\u00dcretim tesisimiz \u00e7al\u0131\u015fmaya ba\u015flad\u0131: \u00f6zel markal\u0131 \u00fcr\u00fcnler b\u00fct\u00fcn \u015fubelerde raflarda."));

    // Pushes: the daily cost, a rival's answer now and then, the end.
    for (int32 I = 0; I < S.Pushes.Num(); ++I)
    {
        FMarketPush& P = S.Pushes[I];
        if (P.bEnded) continue;
        const FString Name = MarketChains::ProvinceName(P.Country, P.Province);
        if (State.Day <= P.EndDay)
        {
            const int64 Cost = PushDailyCost(State, P.Country, P.Province);
            if (Cost > 0)
            {
                State.Cash -= Cost;
                State.LastProfit -= Cost;
                S.PushPaid += Cost;
                MarketLedger::Post(State, MarketLedger::EAccount::Marketing, -Cost, true, MarketLedger::HeadOfficeStore);
            }
            const int32 Age = State.Day - P.StartDay;
            if (Age > 0 && Age % 15 == 0 && Roll(State, State.Day, 0xA77Au + static_cast<uint32>(I), 0.2f))
            {
                const int32 Leader = LeaderIndex(State, P.Country, P.Province);
                FString Answer;
                if (Leader != INDEX_NONE && MarketChains::ForceWar(State, Leader, P.Province, Answer))
                    News.Add(FString::Printf(TEXT("%s ata\u011f\u0131m\u0131za cevap: %s"), *Name, *Answer));
            }
            continue;
        }
        P.bEnded = true;
        TArray<FString> Gave;
        for (int32 C = 0; C < State.Rivals.Chains.Num(); ++C)
        {
            const FMarketChain& Chain = State.Rivals.Chains[C];
            if (Chain.bGone || Chain.bOurs || Chain.Country != P.Country) continue;
            const int32 There = ChainStoresIn(Chain, P.Province);
            if (There <= 0) continue;
            if (!Roll(State, State.Day, 0xA77Bu + static_cast<uint32>(C) * 31u, There <= 3 ? 0.4f : 0.2f)) continue;
            FString Line;
            if (MarketChains::Withdraw(State, C, P.Province, Line)) { ++P.Withdrawn; Gave.Add(State.Rivals.Chains[C].Name); }
        }
        News.Add(Gave.Num() > 0
            ? FString::Printf(TEXT("%s ata\u011f\u0131 bitti: %s oradaki birer ma\u011fazas\u0131n\u0131 kapatt\u0131."), *Name, *FString::Join(Gave, TEXT(", ")))
            : FString::Printf(TEXT("%s ata\u011f\u0131 bitti: rakipler yerinde kald\u0131, ama \u015fubelerimiz yeni m\u00fc\u015fteri tan\u0131d\u0131."), *Name));
    }

    // The loyalty programme, once a month.
    if (S.Vertical == static_cast<uint8>(EVertical::Loyalty) && State.Day - S.LoyaltyDay >= 30)
    {
        S.LoyaltyDay = State.Day;
        int64 Revenue = 0;
        for (const FMarketBranch& B : State.Branches) if (B.Stage == static_cast<uint8>(MarketBranches::EStage::Open)) Revenue += B.Last30Revenue;
        const int64 Cost = Revenue * 4 / 1000;
        if (Cost > 0)
        {
            State.Cash -= Cost;
            State.LastProfit -= Cost;
            S.LoyaltyPaid += Cost;
            MarketLedger::Post(State, MarketLedger::EAccount::Marketing, -Cost, true, MarketLedger::HeadOfficeStore);
        }
    }

    // The paths, once a month.
    if (State.Day - S.LastLookDay >= 30 && Open > 0)
    {
        S.LastLookDay = State.Day;
        S.Champions.Reset();
        for (const FString& K : OurProvinces(State, 1000))
        {
            FString Country, Province;
            if (K.Split(TEXT("|"), &Country, &Province) && IsChampion(State, Country, Province)) S.Champions.Add(K);
        }
        S.Tiers.SetNumZeroed(static_cast<int32>(EPath::Count));
        S.BestTiers.SetNumZeroed(static_cast<int32>(EPath::Count));
        for (int32 I = 0; I < static_cast<int32>(EPath::Count); ++I)
        {
            const EPath Path = static_cast<EPath>(I);
            const int32 Was = S.Tiers[I];
            const int32 Now = MeasurePathTier(State, Path);
            S.Tiers[I] = static_cast<uint8>(Now);
            if (Now > S.BestTiers[I])
            {
                S.BestTiers[I] = static_cast<uint8>(Now);
                Celebrate(State, FString::Printf(TEXT("%s: %d. basamak"), *PathName(Path), Now), PathLine(State, Path), static_cast<uint8>(Now >= 3 ? 2 : 1));
                News.Add(FString::Printf(TEXT("Yeni basamak: %s %d. basamakta."), *PathName(Path), Now));
            }
            else if (Now < Was) News.Add(FString::Printf(TEXT("%s bir basamak geriledi (%d). Kazan\u0131m\u0131 da azald\u0131."), *PathName(Path), Now));
        }
    }
}

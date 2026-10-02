#include "MarketRumors.h"
#include "MarketChains.h"
#include "MarketBranches.h"
#include "MarketManagers.h"
#include "MarketStaff.h"
#include "MarketCountry.h"

namespace MarketRumorsLocal
{
    using MarketRumors::EKind;

    uint32 Mix(uint32 A, uint32 B, uint32 C)
    {
        uint32 H = A * 0x9E3779B1u ^ (B + 0x7F4A7C15u) * 0x85EBCA77u ^ (C + 0x165667B1u) * 0xC2B2AE3Du;
        H ^= H >> 15; H *= 0x2C1B3C6Du; H ^= H >> 12; H *= 0x297A2D39u; H ^= H >> 15;
        return H;
    }
    float Roll(const FMarketState& State, uint32 A, uint32 B) { return static_cast<float>(Mix(static_cast<uint32>(State.RivalSeed), A, B) % 10000u) / 10000.f; }

    bool Reliable(uint8 S) { return (S & 1u) != 0; }
    bool Confirms(uint8 S) { return (S & 2u) != 0; }

    FString NameOf(const FMarketState& State, const FString& Id)
    {
        const int32 I = MarketChains::FindChainIndex(State, Id);
        return I == INDEX_NONE ? Id : State.Rivals.Chains[I].Name;
    }

    // Countries with a shop of ours (the campaign's first).
    TArray<FString> OurCountries(const FMarketState& State)
    {
        TArray<FString> List;
        List.Add(State.CountryId);
        for (const FMarketBranch& B : State.Branches)
            if (B.Stage != static_cast<uint8>(MarketBranches::EStage::Closed)) List.AddUnique(MarketBranches::CountryOf(State, B));
        return List;
    }

    bool Busy(const FMarketState& State, const FString& ChainId)
    {
        return State.Rumors.Active.ContainsByPredicate([&ChainId](const FMarketRumor& R) { return R.ChainId == ChainId || R.OtherId == ChainId; });
    }

    bool Usable(const FMarketState& State, const FMarketChain& C, const TArray<FString>& Countries)
    {
        if (C.bGone || C.bOurs || C.bForSale || !Countries.Contains(C.Country) || MarketChains::TotalStores(C) <= 0 || Busy(State, C.Id)) return false;
        // A local family chain is news only where we are.
        if (C.Scope == static_cast<uint8>(MarketChains::EScope::Local))
            return MarketBranches::ShopsIn(State, C.Country, C.Home) > 0;
        return true;
    }

    int32 StoresIn(const FMarketChain& C, const FString& Province)
    {
        for (const FMarketChainSpot& S : C.Spots) if (S.Province == Province) return S.Stores;
        return 0;
    }

    // A candidate for a kind (chain, other chain, province); false when none.
    bool Pick(const FMarketState& State, EKind Kind, uint32 Salt, int32& OutChain, int32& OutOther, FString& OutProvince, float& OutPrior)
    {
        const TArray<FString> Countries = OurCountries(State);
        const TArray<FMarketChain>& Chains = State.Rivals.Chains;
        TArray<int32> Pool;
        for (int32 I = 0; I < Chains.Num(); ++I)
        {
            const FMarketChain& C = Chains[I];
            if (!Usable(State, C, Countries)) continue;
            const bool bForeign = C.Scope == static_cast<uint8>(MarketChains::EScope::Foreign);
            if ((Kind == EKind::ForSale || Kind == EKind::Acquires) && bForeign) continue;
            if (Kind == EKind::Acquires && C.Scope != static_cast<uint8>(MarketChains::EScope::National)) continue;
            Pool.Add(I);
        }
        if (Pool.Num() == 0) return false;
        // Weak chains are talked about for a sale, strong ones for buying and entering.
        Pool.Sort([&Chains, Kind](int32 A, int32 B)
        {
            const float SA = static_cast<float>(Chains[A].RedTurns) * 1.0e12f - static_cast<float>(Chains[A].Cash);
            const float SB = static_cast<float>(Chains[B].RedTurns) * 1.0e12f - static_cast<float>(Chains[B].Cash);
            return Kind == EKind::ForSale ? SA > SB : SA < SB;
        });
        const int32 Top = FMath::Min(Pool.Num(), 4);
        const int32 Chain = Pool[Mix(Salt, 0x51u, static_cast<uint32>(Top)) % static_cast<uint32>(Top)];
        const FMarketChain& C = Chains[Chain];
        OutChain = Chain;
        OutOther = INDEX_NONE;
        OutProvince.Reset();
        switch (Kind)
        {
        case EKind::ForSale:
            OutPrior = 0.30f + 0.15f * FMath::Min(C.RedTurns, 2) + (C.Cash < 0 ? 0.1f : 0.f);
            return true;
        case EKind::Enters:
        {
            TArray<FString> Ours = MarketBranches::ProvincesWithShops(State, C.Country);
            Ours.RemoveAll([&C](const FString& P) { return StoresIn(C, P) > 0; });
            if (Ours.Num() == 0 || C.Cash <= 0) return false;
            OutProvince = Ours[Mix(Salt, 0x52u, static_cast<uint32>(Ours.Num())) % static_cast<uint32>(Ours.Num())];
            OutPrior = 0.45f;
            return true;
        }
        case EKind::Acquires:
        {
            TArray<int32> Targets;
            for (int32 J = 0; J < Chains.Num(); ++J)
            {
                const FMarketChain& T = Chains[J];
                if (J == Chain || T.bGone || T.bOurs || T.Country != C.Country || Busy(State, T.Id)) continue;
                if (T.Scope != static_cast<uint8>(MarketChains::EScope::Regional) && T.Scope != static_cast<uint8>(MarketChains::EScope::Local)) continue;
                if (MarketChains::TotalStores(T) <= 0 || C.Cash < MarketChains::BidPrice(State, J)) continue;
                Targets.Add(J);
            }
            if (Targets.Num() == 0) return false;
            OutOther = Targets[Mix(Salt, 0x53u, static_cast<uint32>(Targets.Num())) % static_cast<uint32>(Targets.Num())];
            OutPrior = 0.35f;
            return true;
        }
        case EKind::PriceWar:
        {
            TArray<FString> Where;
            for (const FMarketChainSpot& S : C.Spots)
                if (S.Stores >= 2 && MarketBranches::ShopsIn(State, C.Country, S.Province) > 0 && MarketChains::WarIn(State, C.Country, S.Province, State.Day) == INDEX_NONE) Where.Add(S.Province);
            if (Where.Num() == 0 || !C.WarProvince.IsEmpty()) return false;
            OutProvince = Where[Mix(Salt, 0x54u, static_cast<uint32>(Where.Num())) % static_cast<uint32>(Where.Num())];
            OutPrior = 0.5f;
            return true;
        }
        default: return false;
        }
    }
}

float MarketRumors::Belief(const FMarketRumor& Rumor)
{
    using namespace MarketRumorsLocal;
    double Odds = Prior / (1.0 - Prior);
    for (const uint8 S : Rumor.Sources)
    {
        const double Right = Reliable(S) ? ReliableRight : UnreliableRight;
        Odds *= Confirms(S) ? Right / (1.0 - Right) : (1.0 - Right) / Right;
    }
    return static_cast<float>(Odds / (1.0 + Odds));
}

FString MarketRumors::BeliefName(float Value)
{
    return Value < 0.3f ? TEXT("zay\u0131f") : Value < 0.6f ? TEXT("belirsiz") : Value < 0.85f ? TEXT("g\u00fc\u00e7l\u00fc") : TEXT("\u00e7ok g\u00fc\u00e7l\u00fc");
}

FString MarketRumors::Headline(const FMarketState& State, const FMarketRumor& R)
{
    using namespace MarketRumorsLocal;
    const FString Name = NameOf(State, R.ChainId);
    switch (static_cast<EKind>(R.Kind))
    {
    case EKind::ForSale: return FString::Printf(TEXT("%s sat\u0131\u015fa \u00e7\u0131kmay\u0131 d\u00fc\u015f\u00fcn\u00fcyor"), *Name);
    case EKind::Enters: return FString::Printf(TEXT("%s, %s pazar\u0131na girmeye haz\u0131rlan\u0131yor"), *Name, *MarketChains::ProvinceName(R.Country, R.Province));
    case EKind::Acquires: return FString::Printf(TEXT("%s, %s zincirini almak istiyor"), *Name, *NameOf(State, R.OtherId));
    case EKind::PriceWar: return FString::Printf(TEXT("%s, %s'da fiyat k\u0131rmaya haz\u0131rlan\u0131yor"), *Name, *MarketChains::ProvinceName(R.Country, R.Province));
    default: return Name;
    }
}

FString MarketRumors::SourcesText(const FMarketRumor& R)
{
    using namespace MarketRumorsLocal;
    int32 RC = 0, RD = 0, UC = 0, UD = 0;
    for (const uint8 S : R.Sources)
    {
        if (Reliable(S)) (Confirms(S) ? RC : RD)++;
        else (Confirms(S) ? UC : UD)++;
    }
    TArray<FString> Parts;
    if (RC > 0) Parts.Add(FString::Printf(TEXT("%d g\u00fcvenilir kaynak do\u011fruluyor"), RC));
    if (UC > 0) Parts.Add(FString::Printf(TEXT("%d g\u00fcvensiz kaynak do\u011fruluyor"), UC));
    if (RD > 0) Parts.Add(FString::Printf(TEXT("%d g\u00fcvenilir kaynak yalanl\u0131yor"), RD));
    if (UD > 0) Parts.Add(FString::Printf(TEXT("%d g\u00fcvensiz kaynak yalanl\u0131yor"), UD));
    return FString::Join(Parts, TEXT(", "));
}

FString MarketRumors::Describe(const FMarketState& State, const FMarketRumor& R)
{
    const float B = Belief(R);
    return FString::Printf(TEXT("%s \u00b7 %s \u00b7 asistan: %s s\u00f6ylenti \u00b7 yakla\u015f\u0131k %d g\u00fcn i\u00e7inde belli olur"),
        *Headline(State, R), *SourcesText(R), *BeliefName(B), FMath::Max(1, R.DueDay - State.Day + 1));
}

FString MarketRumors::DescribePast(const FMarketState& State, const FMarketRumor& R)
{
    return FString::Printf(TEXT("%s \u00b7 %s \u00b7 %s"), *Headline(State, R), *SourcesText(R), R.Outcome == 1 ? TEXT("DO\u011eRU \u00c7IKTI") : TEXT("bo\u015fa \u00e7\u0131kt\u0131"));
}

void MarketRumors::AddSource(FMarketState& State, FMarketRumor& R)
{
    using namespace MarketRumorsLocal;
    const uint32 Salt = static_cast<uint32>(R.Id) * 131u + static_cast<uint32>(R.Sources.Num());
    // A country manager there and an accountant have better contacts.
    float ReliableShare = 0.40f;
    if (MarketManagers::FindManager(State, MarketManagers::ELevel::Country, R.Country, R.Country) != INDEX_NONE) ReliableShare += 0.15f;
    if (MarketStaff::HasAccountant(State)) ReliableShare += 0.05f;
    const bool bReliable = Roll(State, Salt, 0x5E1u) < ReliableShare;
    const float Right = bReliable ? ReliableRight : UnreliableRight;
    const bool bConfirms = Roll(State, Salt, 0x5E2u) < (R.bTrue ? Right : 1.f - Right);
    R.Sources.Add(static_cast<uint8>((bReliable ? 1u : 0u) | (bConfirms ? 2u : 0u)));
}

int32 MarketRumors::Start(FMarketState& State, EKind Kind, int32 ChainIndex, int32 OtherIndex, const FString& Province, bool bTrue, int32 DueDay)
{
    if (!State.Rivals.Chains.IsValidIndex(ChainIndex) || State.Rumors.Active.Num() >= MaxActive) return INDEX_NONE;
    const FMarketChain& C = State.Rivals.Chains[ChainIndex];
    FMarketRumor R;
    R.Id = State.Rumors.NextId++;
    R.Kind = static_cast<uint8>(Kind);
    R.ChainId = C.Id;
    R.OtherId = State.Rivals.Chains.IsValidIndex(OtherIndex) ? State.Rivals.Chains[OtherIndex].Id : FString();
    R.Country = C.Country;
    R.Province = Province;
    R.bTrue = bTrue;
    R.StartDay = State.Day;
    R.DueDay = FMath::Max(State.Day + 3, DueDay);
    R.Sources.Add(2u); // the market talk itself: an unreliable confirmation
    R.NextSourceDay = State.Day + 4 + static_cast<int32>(MarketRumorsLocal::Mix(static_cast<uint32>(State.RivalSeed), static_cast<uint32>(R.Id), 0x4Du) % 5u);
    ++State.Rumors.Started;
    return State.Rumors.Active.Add(R);
}

void MarketRumors::CloseDay(FMarketState& State)
{
    using namespace MarketRumorsLocal;
    const int32 Closed = State.Day - 1;
    if (Closed < 1) return;
    FMarketRumorsState& RS = State.Rumors;
    TArray<FString>& News = State.DayNews;

    // Sources and endings.
    for (int32 I = RS.Active.Num() - 1; I >= 0; --I)
    {
        FMarketRumor& R = RS.Active[I];
        if (Closed < R.DueDay)
        {
            if (Closed >= R.NextSourceDay && R.Sources.Num() < MaxSources && Closed < R.DueDay - 1)
            {
                AddSource(State, R);
                R.NextSourceDay = Closed + 4 + static_cast<int32>(Mix(static_cast<uint32>(State.RivalSeed), static_cast<uint32>(R.Id), static_cast<uint32>(R.Sources.Num())) % 5u);
                const bool bReliable = Reliable(R.Sources.Last()), bConfirms = Confirms(R.Sources.Last());
                News.Add(FString::Printf(TEXT("Kulis: %s. %s kaynak %s. Asistan: %s s\u00f6ylenti (%s)."), *Headline(State, R),
                    bReliable ? TEXT("G\u00fcvenilir bir") : TEXT("G\u00fcvensiz bir"), bConfirms ? TEXT("do\u011fruluyor") : TEXT("yalanl\u0131yor"), *BeliefName(Belief(R)), *SourcesText(R)));
            }
            continue;
        }
        FString Happened;
        bool bDone = false;
        if (R.bTrue)
        {
            const int32 Chain = MarketChains::FindChainIndex(State, R.ChainId);
            switch (static_cast<EKind>(R.Kind))
            {
            case EKind::ForSale: bDone = MarketChains::ForceForSale(State, Chain, Happened); break;
            case EKind::Enters: bDone = MarketChains::ForceEnter(State, Chain, R.Province, Happened); break;
            case EKind::Acquires: bDone = MarketChains::ForceAcquire(State, Chain, MarketChains::FindChainIndex(State, R.OtherId), Happened); break;
            case EKind::PriceWar: bDone = MarketChains::ForceWar(State, Chain, R.Province, Happened); break;
            default: break;
            }
        }
        R.Outcome = bDone ? 1 : 2;
        if (bDone) { ++RS.CameTrue; News.Add(TEXT("Kulis do\u011fru \u00e7\u0131kt\u0131: ") + Happened); }
        else News.Add(FString::Printf(TEXT("Kulis bo\u015fa \u00e7\u0131kt\u0131: %s haberi yalanland\u0131 (%s)."), *Headline(State, R), *SourcesText(R)));
        RS.Past.Add(R);
        RS.Active.RemoveAt(I);
    }
    while (RS.Past.Num() > KeepPast) RS.Past.RemoveAt(0);

    // A new rumour now and then, once the company has a branch.
    if (MarketBranches::OpenCount(State) <= 0) return;
    if (RS.NextDay <= 0) { RS.NextDay = Closed + 30; return; }
    if (Closed < RS.NextDay || RS.Active.Num() >= MaxActive) return;
    const uint32 Salt = Mix(static_cast<uint32>(State.RivalSeed), static_cast<uint32>(Closed), 0x7A11u);
    RS.NextDay = Closed + 25 + static_cast<int32>(Salt % 21u);
    // Kinds by weight (for sale 30, entering 30, buying 20, a price war 20), the next ones if a kind has no candidate.
    static const EKind Order[4] = { EKind::ForSale, EKind::Enters, EKind::Acquires, EKind::PriceWar };
    static const uint32 Weights[4] = { 30, 30, 20, 20 };
    uint32 Pick100 = Mix(Salt, 0x01u, 0u) % 100u;
    int32 First = 0;
    for (; First < 3 && Pick100 >= Weights[First]; ++First) Pick100 -= Weights[First];
    for (int32 Step = 0; Step < 4; ++Step)
    {
        const EKind Kind = Order[(First + Step) % 4];
        int32 Chain = INDEX_NONE, Other = INDEX_NONE;
        FString Province;
        float PriorTrue = Prior;
        if (!Pick(State, Kind, Salt + static_cast<uint32>(Step) * 7u, Chain, Other, Province, PriorTrue)) continue;
        const bool bTrue = Roll(State, Salt, 0x7Eu + static_cast<uint32>(Step)) < FMath::Clamp(PriorTrue, 0.1f, 0.9f);
        const int32 Due = Closed + 21 + static_cast<int32>(Mix(Salt, 0x0Du, static_cast<uint32>(Step)) % 25u);
        const int32 Index = Start(State, Kind, Chain, Other, Province, bTrue, Due);
        if (Index != INDEX_NONE)
            News.Add(FString::Printf(TEXT("Kulis: %s. \u015eimdilik yaln\u0131zca piyasada konu\u015fuluyor. Asistan: %s s\u00f6ylenti; kaynaklar geldik\u00e7e bildiririm (Ma\u011fazalar \u203a \u015eirket)."),
                *Headline(State, RS.Active[Index]), *BeliefName(Belief(RS.Active[Index]))));
        break;
    }
}

#include "MarketChains.h"
#include "MarketBranches.h"
#include "MarketCountry.h"
#include "MarketPrices.h"
#include "MarketStart.h"
#include "MarketStory.h"
#include "MarketStoreAssign.h"

namespace MarketChainsLocal
{
    using MarketChains::EArchetype;
    using MarketChains::EScope;

    // Economy of a chain store by character (start-level kurus). A discounter's store sells about what our ucuzcu
    // sells; weights say how much of a province's appetite a store takes (a hypermarket counts as twelve).
    struct FArch
    {
        float RevDay;        // sales a day at a comfortable province
        float Weight;
        float Gross;         // gross margin
        float Growth;        // stores a year at full ambition, share of its size
        const TCHAR* Format; // MarketBranches format it resembles
        const TCHAR* Name;
        float IncomePref;    // -1 likes poorer provinces, +1 richer ones
        int32 MinPopK;       // not in smaller provinces
    };

    const FArch& Arch(EArchetype Archetype)
    {
        static const FArch Table[] = {
            { 55000.f, 1.0f, 0.17f, 0.12f, TEXT("kucuk"), TEXT("indirim marketi"), -0.3f, 0 },
            { 45000.f, 0.9f, 0.16f, 0.25f, TEXT("kucuk"), TEXT("h\u0131zl\u0131 b\u00fcy\u00fcyen ucuzcu"), -0.5f, 0 },
            { 150000.f, 2.5f, 0.24f, 0.08f, TEXT("buyuk"), TEXT("s\u00fcpermarket"), 0.4f, 60 },
            { 900000.f, 12.f, 0.20f, 0.03f, TEXT("hiper"), TEXT("hipermarket"), 0.3f, 400 },
            { 120000.f, 2.0f, 0.28f, 0.05f, TEXT("buyuk"), TEXT("se\u00e7kin market"), 1.0f, 300 },
            { 100000.f, 1.8f, 0.22f, 0.07f, TEXT("mahalle"), TEXT("b\u00f6lge zinciri"), 0.f, 0 },
            { 40000.f, 0.8f, 0.20f, 0.03f, TEXT("mahalle"), TEXT("aile marketi"), 0.f, 0 },
            { 1200000.f, 8.f, 0.10f, 0.02f, TEXT("hiper"), TEXT("toptan market"), 0.2f, 500 },
            { 1500000.f, 15.f, 0.11f, 0.03f, TEXT("hiper"), TEXT("\u00fcyelikli depo market"), 0.6f, 800 },
        };
        return Table[FMath::Clamp(static_cast<int32>(Archetype), 0, static_cast<int32>(EArchetype::Count) - 1)];
    }

    // Our own shops weigh like the chains' of the same size.
    float FormatWeight(const FString& Format)
    {
        if (Format == TEXT("kucuk")) return 1.f;
        if (Format == TEXT("buyuk")) return 2.5f;
        if (Format == TEXT("hiper")) return 12.f;
        return 0.9f;
    }

    uint32 Mix(uint32 A, uint32 B, uint32 C)
    {
        uint32 H = A * 0x9E3779B1u ^ (B + 0x7F4A7C15u) * 0x85EBCA6Bu ^ (C + 0x165667B1u) * 0xC2B2AE35u;
        H ^= H >> 16; H *= 0x85EBCA6Bu; H ^= H >> 13; H *= 0xC2B2AE35u; H ^= H >> 16;
        return H;
    }

    uint32 Hash(const FString& Text) { return MarketStoreAssign::StableHash(Text); }

    // 0..1, seeded by the campaign, a day and a salt.
    float Roll(const FMarketState& State, uint32 A, uint32 B)
    {
        return static_cast<float>(Mix(static_cast<uint32>(State.RivalSeed), A, B) % 10000u) / 10000.f;
    }

    FString Key(const FString& Country, const FString& Province) { return Country + TEXT("|") + Province; }

    FString CountryOf(const FMarketState& State, const FString& Country) { return Country.IsEmpty() ? State.CountryId : Country; }

    // Difficulty: how well the chains read the market and how hard they push (never money).
    float Noise(const FMarketState& State) { return State.Difficulty == 0 ? 0.35f : State.Difficulty >= 2 ? 0.08f : 0.2f; }
    float Push(const FMarketState& State) { return State.Difficulty == 0 ? 0.75f : State.Difficulty >= 2 ? 1.25f : 1.f; }

    float OursWeighted(const FMarketState& State, const FString& Country, const FString& Province)
    {
        float Sum = 0.f;
        for (const FMarketBranch& B : State.Branches)
        {
            if (B.Stage == static_cast<uint8>(MarketBranches::EStage::Closed)) continue;
            if (MarketBranches::CountryOf(State, B) != Country) continue;
            if ((B.Province.IsEmpty() ? MarketStart::HomeProvince(State) : B.Province) != Province) continue;
            Sum += FormatWeight(B.Format);
        }
        if (Country == State.CountryId && Province == MarketStart::HomeProvince(State)) Sum += 0.8f; // the family shop
        return Sum;
    }

    int32 FindChain(const FMarketState& State, const FString& Id)
    {
        return State.Rivals.Chains.IndexOfByPredicate([&Id](const FMarketChain& C) { return C.Id == Id; });
    }

    FMarketChainSpot& SpotOf(FMarketChain& Chain, const FString& Province)
    {
        for (FMarketChainSpot& S : Chain.Spots) if (S.Province == Province) return S;
        FMarketChainSpot& New = Chain.Spots.AddDefaulted_GetRef();
        New.Province = Province;
        return New;
    }

    int32 StoresOf(const FMarketChain& Chain, const FString& Province)
    {
        for (const FMarketChainSpot& S : Chain.Spots) if (S.Province == Province) return S.Stores;
        return 0;
    }

    // Fixed costs of a store a month (start-level kurus): what leaves a comfortable store 4.5 % net.
    float FixedMonth(EArchetype Archetype, float Rent)
    {
        const FArch& A = Arch(Archetype);
        return A.RevDay * 30.f * (A.Gross - 0.045f) * (0.7f + 0.3f * Rent);
    }

    float OpenCost(EArchetype Archetype) { return Arch(Archetype).RevDay * 60.f; }

    // How much a chain of this character likes a province (income, size), 0 when it would never go there.
    float Fit(EArchetype Archetype, const MarketCountry::FCity& City)
    {
        const FArch& A = Arch(Archetype);
        if (City.PopulationK < A.MinPopK) return 0.f;
        const float Income = FMath::Clamp(City.Income, 0.5f, 2.f);
        return FMath::Max(0.2f, 1.f + A.IncomePref * (Income - 1.f) * 1.5f);
    }

    const TArray<FString>& BossQuotes()
    {
        static const TArray<FString> Lines = {
            TEXT("Bu ilde son s\u00f6z\u00fc biz s\u00f6yleriz."),
            TEXT("Fiyatlar\u0131 biz belirleriz, onlar takip eder."),
            TEXT("Birka\u00e7 haftaya kalmaz, m\u00fc\u015fteri kimin oldu\u011funu hat\u0131rlar."),
            TEXT("Rekabet iyidir; kazand\u0131\u011f\u0131m\u0131z s\u00fcrece."),
        };
        return Lines;
    }

    const TArray<FString>& NemesisQuotes()
    {
        static const TArray<FString> Lines = {
            TEXT("Miras'\u0131 yak\u0131ndan izliyoruz."),
            TEXT("Onlar\u0131n a\u00e7t\u0131\u011f\u0131 her ma\u011fazan\u0131n kar\u015f\u0131s\u0131na bir tane de biz a\u00e7ar\u0131z."),
            TEXT("K\u00fc\u00e7\u00fck bir aile d\u00fckk\u00e2n\u0131yd\u0131lar. H\u00e2l\u00e2 \u00f6yle g\u00f6r\u00fcyoruz."),
        };
        return Lines;
    }

    FString PersonName(const FMarketState& State, const FString& Country, uint32 Salt)
    {
        const MarketCountry::FProfile* Pack = MarketCountry::Find(Country);
        if (!Pack || Pack->FirstNames.Num() == 0 || Pack->LastNames.Num() == 0) return TEXT("Kurucu");
        const uint32 H = Mix(static_cast<uint32>(State.RivalSeed), Salt, 0xB055u);
        return Pack->FirstNames[H % Pack->FirstNames.Num()] + TEXT(" ") + Pack->LastNames[(H / 7u) % Pack->LastNames.Num()];
    }

    FString LastName(const FMarketState& State, const FString& Country, uint32 Salt)
    {
        const MarketCountry::FProfile* Pack = MarketCountry::Find(Country);
        if (!Pack || Pack->LastNames.Num() == 0) return TEXT("Aile");
        return Pack->LastNames[Mix(static_cast<uint32>(State.RivalSeed), Salt, 0x1A57u) % Pack->LastNames.Num()];
    }

    FString CityName(const FString& Country, const FString& Province)
    {
        const MarketCountry::FCity* City = MarketCountry::FindCity(Country, Province);
        return City ? City->Name : Province;
    }

    // Largest-remainder split of Total over weights.
    TArray<int32> Split(int32 Total, const TArray<float>& Weights)
    {
        TArray<int32> Out;
        Out.Init(0, Weights.Num());
        float Sum = 0.f;
        for (const float W : Weights) Sum += FMath::Max(0.f, W);
        if (Sum <= 0.f || Total <= 0) return Out;
        TArray<TPair<float, int32>> Rest;
        int32 Given = 0;
        for (int32 I = 0; I < Weights.Num(); ++I)
        {
            const float Exact = Total * FMath::Max(0.f, Weights[I]) / Sum;
            Out[I] = FMath::FloorToInt32(Exact);
            Given += Out[I];
            Rest.Add(TPair<float, int32>(Exact - Out[I], I));
        }
        Rest.Sort([](const TPair<float, int32>& A, const TPair<float, int32>& B) { return A.Key > B.Key; });
        for (int32 K = 0; Given < Total && K < Rest.Num(); ++K, ++Given) ++Out[Rest[K].Value];
        return Out;
    }

    struct FNews
    {
        FMarketState& State;
        int32 Count = 0;
        explicit FNews(FMarketState& InState) : State(InState) {}
        void Add(const FString& Line)
        {
            if (Count >= MarketChains::MaxNewsPerDay) return;
            State.DayNews.Add(Line);
            ++Count;
        }
    };

    bool IsGiantArm(const FMarketState& State, const FMarketChain& Chain)
    {
        return State.Rivals.Giants.ContainsByPredicate([&Chain](const FMarketGiant& G) { return G.Id == Chain.Home; });
    }

    // Our shops in the provinces a chain is in.
    int32 OursAround(const FMarketState& State, const FMarketChain& Chain)
    {
        int32 Sum = 0;
        for (const FMarketChainSpot& S : Chain.Spots) if (S.Stores > 0) Sum += MarketBranches::ShopsIn(State, Chain.Country, S.Province);
        return Sum;
    }
}

// ---------------------------------------------------------------------------------------------------------------
// Roster

const TArray<MarketChains::FRosterChain>& MarketChains::NationalRoster()
{
    using E = EArchetype;
    static const TArray<FRosterChain> Rows = {
        // Turkey (names as in Config/zincirler.json, L12)
        { TEXT("bim"), TEXT("tr"), TEXT("B\u0130N"), TEXT("Haluk Sezer"), E::Discount, 3500, 0.90f, 0.80f, 0.60f, 0.80f, TEXT("") },
        { TEXT("a101"), TEXT("tr"), TEXT("A110"), TEXT("Kaan Y\u0131ld\u0131r"), E::FastDiscount, 1900, 0.91f, 0.78f, 0.85f, 1.00f, TEXT("") },
        { TEXT("sok"), TEXT("tr"), TEXT("\u015eAK"), TEXT("Deniz Arslan"), E::Discount, 1300, 0.93f, 0.82f, 0.50f, 0.70f, TEXT("") },
        { TEXT("migros"), TEXT("tr"), TEXT("Migron"), TEXT("Selin Tuna"), E::Super, 1100, 1.05f, 1.10f, 0.50f, 0.60f, TEXT("") },
        { TEXT("carrefoursa"), TEXT("tr"), TEXT("CarreFive SA"), TEXT("Emre \u00c7elikta\u015f"), E::Hyper, 250, 1.03f, 1.05f, 0.30f, 0.40f, TEXT("") },
        { TEXT("kipa"), TEXT("tr"), TEXT("Tesko Kupa"), TEXT("Burak Aky\u00fcz"), E::Hyper, 170, 1.02f, 1.00f, 0.20f, 0.25f, TEXT("ege") },
        { TEXT("onur"), TEXT("tr"), TEXT("Onar Market"), TEXT("Onur Karaca"), E::Regional, 150, 0.99f, 1.00f, 0.40f, 0.50f, TEXT("marmara") },
        { TEXT("metro"), TEXT("tr"), TEXT("Metra Toptan"), TEXT("Klaus Weber"), E::Wholesale, 30, 0.95f, 0.90f, 0.20f, 0.30f, TEXT("") },
        // Germany
        { TEXT("alda_de"), TEXT("de"), TEXT("Alda"), TEXT("Karl Brenner"), E::Discount, 4200, 0.88f, 0.80f, 0.50f, 0.50f, TEXT("") },
        { TEXT("lidel_de"), TEXT("de"), TEXT("Lidel"), TEXT("Dieter Schwarm"), E::FastDiscount, 3200, 0.89f, 0.82f, 0.70f, 0.60f, TEXT("") },
        { TEXT("edecka"), TEXT("de"), TEXT("Edecka"), TEXT("Markus Hahn"), E::Super, 11000, 1.04f, 1.10f, 0.40f, 0.40f, TEXT("") },
        { TEXT("rewa"), TEXT("de"), TEXT("Rewa"), TEXT("Anna Keller"), E::Super, 6000, 1.03f, 1.05f, 0.50f, 0.50f, TEXT("") },
        { TEXT("kauffeld"), TEXT("de"), TEXT("Kauffeld"), TEXT("Jonas Wolf"), E::Hyper, 660, 0.97f, 0.95f, 0.40f, 0.40f, TEXT("") },
        { TEXT("nettol"), TEXT("de"), TEXT("Nettol"), TEXT("Petra Lang"), E::Discount, 4200, 0.90f, 0.78f, 0.50f, 0.50f, TEXT("") },
        // United Kingdom
        { TEXT("tesko"), TEXT("gb"), TEXT("Tesko"), TEXT("Oliver Grant"), E::Super, 3400, 1.00f, 1.00f, 0.60f, 0.50f, TEXT("") },
        { TEXT("sainsbee"), TEXT("gb"), TEXT("Sainsbee's"), TEXT("Emily Hart"), E::Super, 1400, 1.05f, 1.10f, 0.40f, 0.40f, TEXT("") },
        { TEXT("asdale"), TEXT("gb"), TEXT("Asdale"), TEXT("Harry Moss"), E::Hyper, 630, 0.95f, 0.95f, 0.50f, 0.40f, TEXT("") },
        { TEXT("morrisen"), TEXT("gb"), TEXT("Morrisen"), TEXT("Jack Pike"), E::Super, 500, 1.00f, 1.00f, 0.40f, 0.30f, TEXT("england") },
        { TEXT("koop"), TEXT("gb"), TEXT("Ko-op"), TEXT("Sophie Reed"), E::Regional, 2600, 1.06f, 1.00f, 0.30f, 0.30f, TEXT("") },
        { TEXT("alda_gb"), TEXT("gb"), TEXT("Alda"), TEXT("Karl Brenner"), E::Discount, 900, 0.88f, 0.80f, 0.60f, 0.80f, TEXT("") },
        { TEXT("lidel_gb"), TEXT("gb"), TEXT("Lidel"), TEXT("Dieter Schwarm"), E::FastDiscount, 800, 0.89f, 0.82f, 0.70f, 0.80f, TEXT("") },
        // United States
        { TEXT("wallmark"), TEXT("us"), TEXT("Wallmark"), TEXT("Sam Walden"), E::Hyper, 3000, 0.92f, 0.85f, 0.70f, 0.50f, TEXT("") },
        { TEXT("krueger"), TEXT("us"), TEXT("Krueger"), TEXT("Rodney Mack"), E::Super, 2600, 1.00f, 1.00f, 0.50f, 0.40f, TEXT("") },
        { TEXT("alberton"), TEXT("us"), TEXT("Alberton"), TEXT("Vivek Rao"), E::Super, 2200, 1.03f, 1.00f, 0.40f, 0.30f, TEXT("west") },
        { TEXT("publick"), TEXT("us"), TEXT("Publick"), TEXT("Todd Jones"), E::Regional, 1100, 1.02f, 1.20f, 0.30f, 0.40f, TEXT("south") },
        { TEXT("costko"), TEXT("us"), TEXT("Costko"), TEXT("Craig Jelly"), E::Club, 440, 0.90f, 0.90f, 0.40f, 0.40f, TEXT("") },
        { TEXT("alda_us"), TEXT("us"), TEXT("Alda"), TEXT("Karl Brenner"), E::Discount, 1250, 0.88f, 0.80f, 0.60f, 0.70f, TEXT("") },
        { TEXT("traders"), TEXT("us"), TEXT("Traders Jo"), TEXT("Dan Bane"), E::Premium, 360, 1.08f, 1.30f, 0.30f, 0.40f, TEXT("") },
    };
    return Rows;
}

const TArray<MarketChains::FRosterGiant>& MarketChains::GiantRoster()
{
    using E = EArchetype;
    static const TArray<FRosterGiant> Rows = {
        { TEXT("wallmark"), TEXT("Wallmark"), TEXT("ABD"), TEXT("us"), E::Hyper, 447.f, 0.030f },
        { TEXT("carrefive"), TEXT("CarreFive"), TEXT("Fransa"), TEXT(""), E::Hyper, 113.f, 0.005f },
        { TEXT("tesko"), TEXT("Tesko"), TEXT("Birle\u015fik Krall\u0131k"), TEXT("gb"), E::Super, 102.f, 0.010f },
        { TEXT("metra"), TEXT("Metra Group"), TEXT("Almanya"), TEXT(""), E::Wholesale, 93.f, -0.010f },
        { TEXT("krueger"), TEXT("Krueger"), TEXT("ABD"), TEXT("us"), E::Super, 90.f, 0.030f },
        { TEXT("costko"), TEXT("Costko"), TEXT("ABD"), TEXT("us"), E::Club, 89.f, 0.060f },
        { TEXT("schwarm"), TEXT("Schwarm Grubu"), TEXT("Almanya"), TEXT("de"), E::FastDiscount, 87.f, 0.050f },
        { TEXT("alda"), TEXT("Alda"), TEXT("Almanya"), TEXT("de"), E::Discount, 73.f, 0.040f },
        { TEXT("aeonn"), TEXT("Aeonn"), TEXT("Japonya"), TEXT(""), E::Super, 60.f, 0.020f },
        { TEXT("edecka"), TEXT("Edecka"), TEXT("Almanya"), TEXT("de"), E::Super, 55.f, 0.020f },
        { TEXT("sevenme"), TEXT("Seven & Me"), TEXT("Japonya"), TEXT(""), E::Regional, 56.f, 0.030f },
        { TEXT("ahald"), TEXT("Ahald"), TEXT("Hollanda"), TEXT(""), E::Super, 40.f, 0.020f },
    };
    return Rows;
}

FString MarketChains::ArchetypeName(EArchetype Archetype) { return MarketChainsLocal::Arch(Archetype).Name; }
FString MarketChains::FormatOf(EArchetype Archetype) { return MarketChainsLocal::Arch(Archetype).Format; }

// ---------------------------------------------------------------------------------------------------------------
// Seeding

void MarketChains::EnsureCountry(FMarketState& State, const FString& InCountry)
{
    using namespace MarketChainsLocal;
    const FString Country = CountryOf(State, InCountry);
    FMarketChainsState& R = State.Rivals;
    // The giants once (their arms at home are the national chains that carry their id).
    if (R.Giants.Num() == 0)
    {
        for (const FRosterGiant& G : GiantRoster())
        {
            FMarketGiant& Giant = R.Giants.AddDefaulted_GetRef();
            Giant.Id = G.Id; Giant.Name = G.Name; Giant.Home = G.Home; Giant.RevenueB = G.RevenueB;
            Giant.Growth = G.Growth; Giant.Archetype = static_cast<uint8>(G.Archetype);
        }
        R.LastYearDay = State.Day;
    }
    if (R.Countries.Contains(Country)) return;
    const MarketCountry::FProfile* Pack = MarketCountry::Find(Country);
    if (!Pack || Pack->Cities.Num() == 0) return;
    R.Countries.Add(Country);

    auto Seed = [&State, &R, Pack, &Country](FMarketChain& Chain, int32 Total, const FString& OnlyRegion, const FString& OnlySub)
    {
        TArray<float> Weights;
        for (const MarketCountry::FCity& City : Pack->Cities)
        {
            float W = static_cast<float>(FMath::Max(1, City.PopulationK)) * Fit(static_cast<EArchetype>(Chain.Archetype), City);
            if (!OnlyRegion.IsEmpty() && City.Region != OnlyRegion) W *= 0.02f;
            if (!OnlySub.IsEmpty() && City.SubRegion != OnlySub) W = 0.f;
            if (City.ChainDensity > 0.f) W *= 0.5f + 0.5f * FMath::Clamp(City.ChainDensity / 12.f, 0.2f, 2.f);
            Weights.Add(W);
        }
        const TArray<int32> Stores = Split(Total, Weights);
        float Fixed = 0.f;
        for (int32 I = 0; I < Pack->Cities.Num(); ++I)
        {
            if (Stores[I] <= 0) continue;
            SpotOf(Chain, Pack->Cities[I].Id).Stores = Stores[I];
            Fixed += Stores[I] * FixedMonth(static_cast<EArchetype>(Chain.Archetype), Pack->Cities[I].Rent);
        }
        Chain.Cash = FMath::RoundToInt64(4.0 * Fixed * MarketPrices::ListLevel(State.Day));
        Chain.TurnDay = State.Day - 1 - static_cast<int32>(Mix(static_cast<uint32>(State.RivalSeed), Hash(Chain.Id), 0x7D11u) % TurnDays);
        R.Chains.Add(Chain);
    };

    for (const FRosterChain& Row : NationalRoster())
    {
        if (Country != Row.Country) continue;
        FMarketChain Chain;
        Chain.Id = Row.Id; Chain.Country = Country; Chain.Name = Row.Name; Chain.Boss = Row.Boss;
        Chain.Archetype = static_cast<uint8>(Row.Archetype);
        Chain.Scope = static_cast<uint8>(Row.Archetype == EArchetype::Regional ? EScope::Regional : EScope::National);
        Chain.PriceIndex = Row.PriceIndex; Chain.Service = Row.Service; Chain.Aggression = Row.Aggression; Chain.Ambition = Row.Ambition;
        // A giant's home arm carries the giant's id (the league counts it once).
        for (const FRosterGiant& G : GiantRoster())
            if (Country == G.HomePack && (Chain.Name == G.Name || Chain.Id == G.Id)) Chain.Home = G.Id;
        Seed(Chain, Row.StartStores, Row.HomeRegion, FString());
    }
    // The campaign's country: one regional chain in every sub-region that has room for it.
    if (Country == State.CountryId)
    {
        const TArray<FString> Suffix = { TEXT(" Market"), TEXT(" Gross"), TEXT(" S\u00fcper"), TEXT(" G\u0131da") };
        for (const MarketCountry::FRegion& Sub : Pack->SubRegions)
        {
            if (Sub.Provinces.Num() < 2) continue;
            int32 PopK = 0;
            for (const FString& P : Sub.Provinces) if (const MarketCountry::FCity* City = MarketCountry::FindCity(Country, P)) PopK += City->PopulationK;
            FMarketChain Chain;
            Chain.Id = TEXT("bolge.") + Sub.Id; Chain.Country = Country;
            Chain.Name = Sub.Name + Suffix[Hash(Sub.Id) % Suffix.Num()];
            Chain.Boss = PersonName(State, Country, Hash(Chain.Id));
            Chain.Archetype = static_cast<uint8>(EArchetype::Regional);
            Chain.Scope = static_cast<uint8>(EScope::Regional);
            Chain.Home = Sub.Id;
            Chain.PriceIndex = 0.98f + 0.06f * Roll(State, Hash(Chain.Id), 1u);
            Chain.Service = 0.95f + 0.15f * Roll(State, Hash(Chain.Id), 2u);
            Chain.Aggression = 0.2f + 0.5f * Roll(State, Hash(Chain.Id), 3u);
            Chain.Ambition = 0.3f + 0.4f * Roll(State, Hash(Chain.Id), 4u);
            const int32 Total = FMath::Clamp(PopK / 250 + static_cast<int32>(20.f * Roll(State, Hash(Chain.Id), 5u)), 6, 120);
            Seed(Chain, Total, FString(), Sub.Id);
        }
    }
    // What the provinces carry at the start: our branches' competition is measured against it.
    for (const MarketCountry::FCity& City : Pack->Cities)
        R.Baseline.Add(Key(Country, City.Id), FMath::Max(1.f, WeightedIn(State, Country, City.Id)));
}

void MarketChains::EnsureLocal(FMarketState& State, const FString& InCountry, const FString& Province)
{
    using namespace MarketChainsLocal;
    const FString Country = CountryOf(State, InCountry);
    EnsureCountry(State, Country);
    const FString Pool = Key(Country, Province);
    if (State.Rivals.LocalPools.Contains(Pool)) return;
    const MarketCountry::FCity* City = MarketCountry::FindCity(Country, Province);
    if (!City) return;
    State.Rivals.LocalPools.Add(Pool);
    const int32 Count = FMath::Clamp(FMath::RoundToInt32(City->Competition * 1.3f + Roll(State, Hash(Pool), 1u)), 1, 3);
    const TArray<FString> Pattern = { TEXT(" Market"), TEXT(" G\u0131da"), TEXT(" S\u00fcpermarket"), TEXT(" Karde\u015fler") };
    for (int32 N = 0; N < Count; ++N)
    {
        FMarketChain Chain;
        Chain.Id = FString::Printf(TEXT("yerel.%s.%d"), *Province, N);
        Chain.Country = Country;
        const uint32 H = Hash(Chain.Id);
        Chain.Name = LastName(State, Country, H) + Pattern[H % Pattern.Num()];
        Chain.Boss = PersonName(State, Country, H ^ 0x5Au);
        Chain.Archetype = static_cast<uint8>(EArchetype::Family);
        Chain.Scope = static_cast<uint8>(EScope::Local);
        Chain.Home = Province;
        Chain.PriceIndex = 1.02f + 0.05f * Roll(State, H, 2u);
        Chain.Service = 1.05f + 0.1f * Roll(State, H, 3u);
        Chain.Aggression = 0.15f + 0.35f * Roll(State, H, 4u);
        Chain.Ambition = 0.2f + 0.4f * Roll(State, H, 5u);
        const int32 Stores = 1 + static_cast<int32>(7.f * Roll(State, H, 6u));
        SpotOf(Chain, Province).Stores = Stores;
        Chain.Cash = FMath::RoundToInt64(3.0 * Stores * FixedMonth(EArchetype::Family, City->Rent) * MarketPrices::ListLevel(State.Day));
        Chain.TurnDay = State.Day - 1 - static_cast<int32>(Mix(static_cast<uint32>(State.RivalSeed), H, 0x7D11u) % TurnDays);
        State.Rivals.Chains.Add(Chain);
    }
    // The local shops are part of the province's start (they were there before us).
    float& Base = State.Rivals.Baseline.FindOrAdd(Pool);
    Base = FMath::Max(1.f, WeightedIn(State, Country, Province));
}

void MarketChains::Ensure(FMarketState& State)
{
    EnsureCountry(State, State.CountryId);
    EnsureLocal(State, State.CountryId, MarketStart::HomeProvince(State));
    for (const FMarketBranch& B : State.Branches)
    {
        if (B.Stage == static_cast<uint8>(MarketBranches::EStage::Closed)) continue;
        const FString Country = MarketBranches::CountryOf(State, B);
        EnsureLocal(State, Country, B.Province.IsEmpty() ? MarketStart::HomeProvince(State) : B.Province);
    }
}

// ---------------------------------------------------------------------------------------------------------------
// Province measures

int32 MarketChains::TotalStores(const FMarketChain& Chain)
{
    int32 Sum = 0;
    for (const FMarketChainSpot& S : Chain.Spots) Sum += S.Stores;
    return Sum;
}

int32 MarketChains::StoresIn(const FMarketState& State, const FString& Country, const FString& Province)
{
    int32 Sum = 0;
    for (const FMarketChain& C : State.Rivals.Chains)
        if (!C.bGone && C.Country == Country) Sum += MarketChainsLocal::StoresOf(C, Province);
    return Sum;
}

float MarketChains::WeightedIn(const FMarketState& State, const FString& Country, const FString& Province)
{
    float Sum = 0.f;
    for (const FMarketChain& C : State.Rivals.Chains)
        if (!C.bGone && C.Country == Country) Sum += MarketChainsLocal::StoresOf(C, Province) * MarketChainsLocal::Arch(static_cast<EArchetype>(C.Archetype)).Weight;
    return Sum;
}

float MarketChains::Saturation(const FMarketState& State, const FString& Country, const FString& Province)
{
    const MarketCountry::FCity* City = MarketCountry::FindCity(Country, Province);
    if (!City) return 1.f;
    const float Healthy = FMath::Max(1.f, City->PopulationK / 100.f * HealthyPer100k);
    const float Total = WeightedIn(State, Country, Province) + MarketChainsLocal::OursWeighted(State, Country, Province) + 0.5f;
    return FMath::Clamp(FMath::Sqrt(Healthy / Total), 0.55f, 1.15f);
}

int32 MarketChains::WarIn(const FMarketState& State, const FString& Country, const FString& Province, int32 Day)
{
    const TArray<FMarketChain>& Chains = State.Rivals.Chains;
    for (int32 I = 0; I < Chains.Num(); ++I)
        if (!Chains[I].bGone && Chains[I].Country == Country && Chains[I].WarProvince == Province && Day <= Chains[I].WarUntil) return I;
    return INDEX_NONE;
}

float MarketChains::PressureFactor(const FMarketState& State, const FString& InCountry, const FString& Province, int32 Day)
{
    const FString Country = MarketChainsLocal::CountryOf(State, InCountry);
    const float* Base = State.Rivals.Baseline.Find(MarketChainsLocal::Key(Country, Province));
    if (!Base) return 1.f; // country not seeded yet (older save before its first close)
    const float Now = WeightedIn(State, Country, Province);
    const float Factor = FMath::Clamp(FMath::Sqrt((Now + 1.f) / (*Base + 1.f)), 0.6f, 1.8f);
    return Factor * (WarIn(State, Country, Province, Day) != INDEX_NONE ? WarPressure : 1.f);
}

// ---------------------------------------------------------------------------------------------------------------
// Money and tables

int64 MarketChains::YearRevenue(const FMarketState& State, int32 ChainIndex)
{
    if (!State.Rivals.Chains.IsValidIndex(ChainIndex)) return 0;
    const FMarketChain& C = State.Rivals.Chains[ChainIndex];
    if (C.bGone) return 0;
    if (C.MonthRevenue > 0) return C.MonthRevenue * 12;
    // Before its first month: its stores at a comfortable province.
    const float Level = static_cast<float>(MarketPrices::ListLevel(State.Day));
    return FMath::RoundToInt64(static_cast<double>(TotalStores(C)) * MarketChainsLocal::Arch(static_cast<EArchetype>(C.Archetype)).RevDay * 365.0 * Level);
}

int64 MarketChains::OurYearRevenue(const FMarketState& State, const FString& InCountry)
{
    const FString Country = MarketChainsLocal::CountryOf(State, InCountry);
    int64 Sum = 0;
    for (const FMarketBranch& B : State.Branches)
        if (B.Stage == static_cast<uint8>(MarketBranches::EStage::Open) && MarketBranches::CountryOf(State, B) == Country) Sum += B.Last30Revenue * 365 / 30;
    if (Country == State.CountryId)
    {
        int64 Family = 0;
        int32 Days = 0;
        for (int32 I = State.History.Num() - 1; I >= 0 && Days < 30; --I, ++Days) Family += State.History[I].Revenue;
        if (Days > 0) Sum += Family * 365 / Days;
    }
    return Sum;
}

double MarketChains::ToWorld(const FMarketState& State, const FString& InCountry, int64 Kurus)
{
    const MarketCountry::FProfile* Pack = MarketCountry::Find(MarketChainsLocal::CountryOf(State, InCountry));
    const double Scale = Pack ? Pack->DisplayScale : 1.0;
    const double Fx = Pack && Pack->FxPerWorld > 0.0 ? Pack->FxPerWorld : 1.0;
    const double Level = FMath::Max(0.01, MarketPrices::ListLevel(State.Day));
    return static_cast<double>(Kurus) / 100.0 * Scale / Level / Fx;
}

TArray<MarketChains::FStanding> MarketChains::NationalTable(const FMarketState& State, const FString& InCountry)
{
    const FString Country = MarketChainsLocal::CountryOf(State, InCountry);
    TArray<FStanding> Table;
    const TArray<FMarketChain>& Chains = State.Rivals.Chains;
    for (int32 I = 0; I < Chains.Num(); ++I)
    {
        const FMarketChain& C = Chains[I];
        if (C.bGone || C.Country != Country || TotalStores(C) < 3) continue;
        FStanding Row;
        Row.Name = C.Name;
        Row.Stores = TotalStores(C);
        Row.Detail = FString::Printf(TEXT("%s \u00b7 %d ma\u011faza"), *ArchetypeName(static_cast<EArchetype>(C.Archetype)), Row.Stores);
        Row.Revenue = static_cast<double>(YearRevenue(State, I));
        Row.Chain = I;
        Row.bNemesis = C.Id == State.Rivals.Nemesis;
        Table.Add(Row);
    }
    FStanding Us;
    Us.bUs = true;
    Us.Name = TEXT("Miras");
    for (const FMarketBranch& B : State.Branches)
        if (B.Stage == static_cast<uint8>(MarketBranches::EStage::Open) && MarketBranches::CountryOf(State, B) == Country) ++Us.Stores;
    if (Country == State.CountryId) ++Us.Stores;
    Us.Detail = FString::Printf(TEXT("biz \u00b7 %d ma\u011faza"), Us.Stores);
    Us.Revenue = static_cast<double>(OurYearRevenue(State, Country));
    if (Us.Stores > 0) Table.Add(Us);
    Table.StableSort([](const FStanding& A, const FStanding& B) { return A.Revenue > B.Revenue; });
    return Table;
}

TArray<MarketChains::FStanding> MarketChains::WorldTable(const FMarketState& State)
{
    // Game-scale world units of a real billion: a modelled discounter store against a real one, compressed.
    constexpr double GameToReal = 0.142;
    TArray<FStanding> Table;
    for (const FMarketGiant& G : State.Rivals.Giants)
    {
        FStanding Row;
        Row.Name = G.Name;
        Row.Detail = G.Home;
        Row.Revenue = static_cast<double>(G.RevenueB) * 1.0e9 * GameToReal * LeagueCompression;
        Table.Add(Row);
    }
    const TArray<FMarketChain>& Chains = State.Rivals.Chains;
    for (int32 I = 0; I < Chains.Num(); ++I)
    {
        const FMarketChain& C = Chains[I];
        if (C.bGone || C.Scope != static_cast<uint8>(EScope::National) || MarketChainsLocal::IsGiantArm(State, C)) continue;
        const MarketCountry::FProfile* Pack = MarketCountry::Find(C.Country);
        FStanding Row;
        Row.Name = C.Name;
        Row.Detail = Pack ? Pack->Name : C.Country;
        Row.Revenue = ToWorld(State, C.Country, YearRevenue(State, I));
        Row.Chain = I;
        Row.bNemesis = C.Id == State.Rivals.Nemesis;
        Table.Add(Row);
    }
    FStanding Us;
    Us.bUs = true;
    Us.Name = TEXT("Miras");
    TArray<FString> Ours;
    Ours.Add(State.CountryId);
    for (const FMarketBranch& B : State.Branches) if (B.Stage == static_cast<uint8>(MarketBranches::EStage::Open)) Ours.AddUnique(MarketBranches::CountryOf(State, B));
    for (const FString& Country : Ours) Us.Revenue += ToWorld(State, Country, OurYearRevenue(State, Country));
    Us.Detail = Ours.Num() > 1 ? FString::Printf(TEXT("%d \u00fclke"), Ours.Num()) : FString(TEXT("biz"));
    Table.Add(Us);
    Table.StableSort([](const FStanding& A, const FStanding& B) { return A.Revenue > B.Revenue; });
    return Table;
}

int32 MarketChains::OurRank(const TArray<FStanding>& Table)
{
    const int32 Index = Table.IndexOfByPredicate([](const FStanding& S) { return S.bUs; });
    return Index == INDEX_NONE ? 0 : Index + 1;
}

// ---------------------------------------------------------------------------------------------------------------
// Buying a chain

int64 MarketChains::Price(const FMarketState& State, int32 ChainIndex)
{
    return YearRevenue(State, ChainIndex) * 8 / 12;
}

bool MarketChains::CanBuy(const FMarketState& State, int32 ChainIndex, FString& OutReason)
{
    if (!State.Rivals.Chains.IsValidIndex(ChainIndex)) { OutReason = TEXT("B\u00f6yle bir zincir yok."); return false; }
    const FMarketChain& C = State.Rivals.Chains[ChainIndex];
    if (C.bGone) { OutReason = FString::Printf(TEXT("%s art\u0131k yok."), *C.Name); return false; }
    if (!C.bForSale) { OutReason = FString::Printf(TEXT("%s sat\u0131l\u0131k de\u011fil."), *C.Name); return false; }
    if (C.Country != State.CountryId && !State.Branches.ContainsByPredicate([&State, &C](const FMarketBranch& B) { return MarketBranches::CountryOf(State, B) == C.Country; }))
    {
        OutReason = TEXT("Bu \u00fclkede hen\u00fcz ma\u011fazan yok.");
        return false;
    }
    const int64 Cost = Price(State, ChainIndex);
    if (State.Cash < Cost) { OutReason = FString::Printf(TEXT("Sat\u0131n almak i\u00e7in %s gerekiyor."), *MarketCountry::Money(Cost)); return false; }
    return true;
}

bool MarketChains::Buy(FMarketState& State, const TArray<FMarketProduct>& Products, int32 ChainIndex, FString& OutMessage)
{
    using namespace MarketChainsLocal;
    if (!CanBuy(State, ChainIndex, OutMessage)) return false;
    const int64 Cost = Price(State, ChainIndex);
    FMarketChain Chain = State.Rivals.Chains[ChainIndex]; // a copy: the branches below may grow arrays
    const FString Format = FormatOf(static_cast<EArchetype>(Chain.Archetype));
    constexpr int32 MaxTaken = 60;
    int32 Taken = 0, Sold = 0;
    for (const FMarketChainSpot& Spot : Chain.Spots)
    {
        const MarketBranches::FSite Site = MarketBranches::SiteOf(State, Chain.Country, Spot.Province);
        const int32 Free = FMath::Max(0, MarketBranches::Room(Site) - MarketBranches::ShopsIn(State, Chain.Country, Spot.Province));
        const int32 Take = FMath::Min3(Spot.Stores, Free, MaxTaken - Taken);
        for (int32 N = 0; N < Take; ++N)
            if (MarketBranches::AddAcquired(State, Products, Chain.Country, Spot.Province, Format) != INDEX_NONE) ++Taken;
        Sold += Spot.Stores - Take;
    }
    // The stores we cannot take are sold on (a third of what opening one costs).
    const int64 Back = FMath::RoundToInt64(Sold * OpenCost(static_cast<EArchetype>(Chain.Archetype)) / 3.0 * MarketPrices::ListLevel(State.Day));
    State.Cash += Back - Cost;
    FMarketChain& Gone = State.Rivals.Chains[ChainIndex];
    Gone.bGone = true;
    Gone.bForSale = false;
    Gone.Spots.Reset();
    if (State.Rivals.Nemesis == Gone.Id) State.Rivals.Nemesis.Reset();
    for (FMarketChain& Other : State.Rivals.Chains) if (!Other.bGone && Other.Country == Chain.Country) Other.Rivalry = FMath::Min(100.f, Other.Rivalry + 5.f);
    MarketStory::AddMemory(State, FString::Printf(TEXT("%s zincirini sat\u0131n ald\u0131k (%d ma\u011faza)"), *Chain.Name, Taken));
    OutMessage = FString::Printf(TEXT("%s art\u0131k bizim: %d ma\u011faza \u015fubemiz oldu%s. \u00d6denen %s."), *Chain.Name, Taken,
        Sold > 0 ? *FString::Printf(TEXT(", yer olmayan %d ma\u011faza sat\u0131ld\u0131 (%s geri geldi)"), Sold, *MarketCountry::Money(Back)) : TEXT(""), *MarketCountry::Money(Cost));
    return true;
}

// ---------------------------------------------------------------------------------------------------------------
// The monthly turn

namespace MarketChainsLocal
{
    void MonthlyBooks(FMarketState& State, FMarketChain& Chain, int32 Day)
    {
        const FArch& A = Arch(static_cast<EArchetype>(Chain.Archetype));
        const double Level = MarketPrices::ListLevel(Day);
        double Revenue = 0.0, Profit = 0.0;
        for (const FMarketChainSpot& Spot : Chain.Spots)
        {
            if (Spot.Stores <= 0) continue;
            const MarketCountry::FCity* City = MarketCountry::FindCity(Chain.Country, Spot.Province);
            const float Income = City ? FMath::Clamp(City->Income, 0.5f, 2.f) : 1.f;
            const float Rent = City ? FMath::Clamp(City->Rent, 0.4f, 2.f) : 1.f;
            const float Sat = MarketChains::Saturation(State, Chain.Country, Spot.Province);
            const bool bWar = Chain.WarProvince == Spot.Province && Day <= Chain.WarUntil;
            // Its everyday prices are already in its character's margin; a war cuts them further for its days.
            const float Cut = bWar ? 1.f - MarketChains::WarPrice : 0.f;
            const float Boost = 1.f + 2.f * Cut;
            const double StoreRevenue = A.RevDay * 30.0 * Income * Sat * Boost * Level;
            Revenue += StoreRevenue * Spot.Stores;
            Profit += Spot.Stores * (StoreRevenue * (A.Gross - Cut) - FixedMonth(static_cast<EArchetype>(Chain.Archetype), Rent) * Level);
        }
        Chain.MonthRevenue = FMath::RoundToInt64(Revenue);
        Chain.MonthProfit = FMath::RoundToInt64(Profit);
        Chain.Cash += Chain.MonthProfit;
    }

    // Provinces a chain may grow into.
    bool MayEnter(const FMarketState& State, const FMarketChain& Chain, const MarketCountry::FCity& City, int32 Day)
    {
        if (Chain.Scope == static_cast<uint8>(EScope::Local)) return City.Id == Chain.Home;
        if (Chain.Scope == static_cast<uint8>(EScope::Regional))
        {
            if (Chain.Id.StartsWith(TEXT("bolge."))) return City.SubRegion == Chain.Home || (Day > 5 * 365 && StoresOf(Chain, City.Id) == 0 && Chain.Spots.ContainsByPredicate(
                [&Chain, &City](const FMarketChainSpot& S) { const MarketCountry::FCity* Near = MarketCountry::FindCity(Chain.Country, S.Province); return S.Stores > 0 && Near && Near->Region == City.Region; }));
        }
        return true;
    }

    void Turn(FMarketState& State, int32 Index, int32 Day, FNews& News)
    {
        FMarketChain& Chain = State.Rivals.Chains[Index];
        const EArchetype Kind = static_cast<EArchetype>(Chain.Archetype);
        const FArch& A = Arch(Kind);
        const MarketCountry::FProfile* Pack = MarketCountry::Find(Chain.Country);
        Chain.TurnDay = Day;
        if (!Pack) return;
        const double Level = MarketPrices::ListLevel(Day);
        MonthlyBooks(State, Chain, Day);
        const uint32 Salt = Hash(Chain.Id) ^ static_cast<uint32>(Day);
        const int32 Stores = MarketChains::TotalStores(Chain);
        double FixedAll = 0.0;
        for (const FMarketChainSpot& S : Chain.Spots)
        {
            const MarketCountry::FCity* City = MarketCountry::FindCity(Chain.Country, S.Province);
            FixedAll += S.Stores * FixedMonth(Kind, City ? City->Rent : 1.f) * Level;
        }
        const bool bVisible = Chain.Scope != static_cast<uint8>(EScope::Local) || OursAround(State, Chain) > 0;

        // For sale: the player first; after two months a strong chain of the country buys it; long unsold: closes.
        if (Chain.bForSale)
        {
            ++Chain.ForSaleTurns;
            if (Chain.ForSaleTurns >= 3)
            {
                int32 Buyer = INDEX_NONE;
                double BestCash = 0.0;
                const int64 Cost = MarketChains::Price(State, Index);
                for (int32 J = 0; J < State.Rivals.Chains.Num(); ++J)
                {
                    const FMarketChain& Other = State.Rivals.Chains[J];
                    if (J == Index || Other.bGone || Other.bForSale || Other.Country != Chain.Country || Other.Scope == static_cast<uint8>(EScope::Local)) continue;
                    if (Other.Cash > Cost * 2 && Other.Cash > BestCash) { BestCash = static_cast<double>(Other.Cash); Buyer = J; }
                }
                if (Buyer != INDEX_NONE)
                {
                    FMarketChain& B = State.Rivals.Chains[Buyer];
                    B.Cash -= Cost;
                    for (const FMarketChainSpot& S : Chain.Spots) SpotOf(B, S.Province).Stores += S.Stores;
                    News.Add(FString::Printf(TEXT("%s, sat\u0131l\u0131k %s zincirini sat\u0131n ald\u0131 (%d ma\u011faza)."), *B.Name, *Chain.Name, Stores));
                    FMarketChain& Sold = State.Rivals.Chains[Index];
                    Sold.bGone = true; Sold.bForSale = false; Sold.Spots.Reset();
                    return;
                }
                if (Chain.ForSaleTurns >= 6)
                {
                    const FString Where = Chain.Spots.Num() > 0 ? CityName(Chain.Country, Chain.Spots[0].Province) : FString();
                    if (bVisible) News.Add(FString::Printf(TEXT("%s kepenk indirdi: %d ma\u011faza kapand\u0131. %s'da m\u00fc\u015fteri yeni adres ar\u0131yor."), *Chain.Name, Stores, *Where));
                    Chain.bGone = true; Chain.bForSale = false; Chain.Spots.Reset();
                }
            }
            return;
        }
        // Money trouble.
        if (Chain.Cash < -FixedAll * 2.0) ++Chain.RedTurns; else Chain.RedTurns = FMath::Max(0, Chain.RedTurns - 1);
        if (Chain.RedTurns >= 3)
        {
            Chain.bForSale = true;
            Chain.ForSaleTurns = 0;
            if (bVisible)
                News.Add(FString::Printf(TEXT("%s sat\u0131l\u0131\u011fa \u00e7\u0131kt\u0131 (%d ma\u011faza, fiyat\u0131 %s). \u0130stersen Ma\u011fazalar \u203a \u015eirket'ten sat\u0131n alabilirsin."),
                    *Chain.Name, Stores, *MarketCountry::Money(MarketChains::Price(State, Index))));
            return;
        }

        // Closures where the province cannot feed the stores.
        for (FMarketChainSpot& Spot : Chain.Spots)
        {
            if (Spot.Stores <= 1) continue;
            if (MarketChains::Saturation(State, Chain.Country, Spot.Province) < 0.7f || (Chain.Cash < 0 && Roll(State, Salt, Hash(Spot.Province)) < 0.3f))
                Spot.Stores -= FMath::Max(1, Spot.Stores / 20);
        }
        Chain.Spots.RemoveAll([](const FMarketChainSpot& S) { return S.Stores <= 0; });

        // Openings: size x growth x ambition, where the gap is, as far as the cash goes.
        float SatAvg = 0.f;
        int32 SatN = 0;
        for (const FMarketChainSpot& S : Chain.Spots) { SatAvg += MarketChains::Saturation(State, Chain.Country, S.Province); ++SatN; }
        SatAvg = SatN > 0 ? SatAvg / SatN : 1.1f;
        const float Mood = FMath::Clamp((SatAvg - 0.85f) / 0.3f, 0.f, 1.2f);
        const float Want = FMath::Max(1, Stores) * A.Growth * (Chain.Ambition * 2.f) / 12.f * Mood;
        int32 Openings = FMath::FloorToInt32(Want) + (Roll(State, Salt, 11u) < FMath::Frac(Want) ? 1 : 0);
        const double Capex = OpenCost(Kind) * Level;
        if (Chain.Cash <= 0) Openings = 0;
        else Openings = FMath::Min<int32>(Openings, static_cast<int32>(FMath::Min(1.0e6, static_cast<double>(Chain.Cash) / FMath::Max(1.0, Capex))));
        Openings = FMath::Min(Openings, 400);
        if (Openings > 0)
        {
            const float NoiseLevel = Noise(State);
            const float PushLevel = Push(State);
            TArray<TPair<float, int32>> Scores;
            for (int32 C = 0; C < Pack->Cities.Num(); ++C)
            {
                const MarketCountry::FCity& City = Pack->Cities[C];
                if (!MayEnter(State, Chain, City, Day)) continue;
                const float F = Fit(Kind, City);
                if (F <= 0.f) continue;
                const float Sat = MarketChains::Saturation(State, Chain.Country, City.Id);
                const int32 Here = StoresOf(Chain, City.Id);
                float Network = Here > 0 ? 1.f : 0.4f;
                if (Here == 0 && Chain.Spots.ContainsByPredicate([&Chain, &City](const FMarketChainSpot& S) { const MarketCountry::FCity* Near = MarketCountry::FindCity(Chain.Country, S.Province); return Near && Near->SubRegion == City.SubRegion; })) Network = 0.8f;
                if (Chain.Scope == static_cast<uint8>(EScope::National) && Kind == EArchetype::FastDiscount) Network = FMath::Max(Network, 0.7f);
                // We are there: an aggressive chain follows us, a timid one keeps away.
                float Player = 1.f;
                const int32 Ours = MarketBranches::ShopsIn(State, Chain.Country, City.Id);
                if (Ours > 0)
                {
                    const float Bold = Chain.Aggression * PushLevel;
                    Player = Bold > 0.6f ? 1.f + 0.4f * Bold + 0.004f * Chain.Rivalry : Bold < 0.35f ? 0.7f : 1.f;
                }
                const float Size = FMath::Pow(static_cast<float>(FMath::Max(1, City.PopulationK)) / 300.f, 0.6f);
                const float Luck = 1.f + NoiseLevel * (Roll(State, Salt, Hash(City.Id)) - 0.5f) * 2.f;
                Scores.Add(TPair<float, int32>(Sat * Sat * F * Size * Network * Player * Luck, C));
            }
            Scores.Sort([](const TPair<float, int32>& L, const TPair<float, int32>& R) { return L.Key > R.Key; });
            TMap<FString, int32> Opened;
            for (int32 N = 0; N < Openings && Scores.Num() > 0; ++N)
            {
                TPair<float, int32>& Best = Scores[0];
                const MarketCountry::FCity& City = Pack->Cities[Best.Value];
                const int32 Cap = Chain.Scope == static_cast<uint8>(EScope::Local) ? FMath::Max(2, City.PopulationK / 25) : FMath::Max(3, City.PopulationK / 8);
                if (StoresOf(Chain, City.Id) >= Cap) { Scores.RemoveAt(0); continue; }
                ++SpotOf(Chain, City.Id).Stores;
                Opened.FindOrAdd(City.Id) += 1;
                Best.Key *= 0.9f;
                Scores.Sort([](const TPair<float, int32>& L, const TPair<float, int32>& R) { return L.Key > R.Key; });
            }
            int32 Total = 0;
            FString Most;
            int32 MostN = 0;
            for (const TPair<FString, int32>& P : Opened) { Total += P.Value; if (P.Value > MostN) { MostN = P.Value; Most = P.Key; } }
            Chain.Cash -= FMath::RoundToInt64(Total * Capex);
            // News only where it touches us: in a province with our shop, or the nemesis.
            bool bNearUs = false;
            for (const TPair<FString, int32>& P : Opened) if (MarketBranches::ShopsIn(State, Chain.Country, P.Key) > 0) { bNearUs = true; Most = P.Key; MostN = P.Value; break; }
            if (Total > 0 && (bNearUs || Chain.Id == State.Rivals.Nemesis))
                News.Add(bNearUs ? FString::Printf(TEXT("%s, ma\u011fazan\u0131n oldu\u011fu %s'a %d yeni ma\u011faza a\u00e7t\u0131."), *Chain.Name, *CityName(Chain.Country, Most), MostN)
                    : FString::Printf(TEXT("Ezeli rakibin %s bu ay %d ma\u011faza a\u00e7t\u0131 (en \u00e7ok %s)."), *Chain.Name, Total, *CityName(Chain.Country, Most)));
        }

        // Rivalry: our shops spreading into its provinces make it mind us; the memory fades slowly.
        const int32 OursNow = OursAround(State, Chain);
        Chain.Rivalry = FMath::Clamp(Chain.Rivalry + 3.f * FMath::Max(0, OursNow - Chain.OursSeen) - 2.f, 0.f, 100.f);
        const bool bWeGrew = OursNow > Chain.OursSeen;
        Chain.OursSeen = OursNow;

        // A price war where we grow and it is strong, if it can afford one.
        if (Day > Chain.WarUntil && Chain.WarProvince.IsEmpty() && Chain.Cash > FixedAll * 2.0)
        {
            const float Heat = Chain.Aggression * Push(State) * (0.5f + Chain.Rivalry / 50.f);
            if ((bWeGrew || Chain.Rivalry >= 50.f) && Roll(State, Salt, 21u) < Heat * 0.5f)
            {
                FString Target;
                int32 Strength = 0;
                for (const FMarketChainSpot& S : Chain.Spots)
                {
                    const int32 Ours = MarketBranches::ShopsIn(State, Chain.Country, S.Province);
                    const int32 Need = Chain.Scope == static_cast<uint8>(EScope::Local) ? 1 : 3;
                    if (Ours > 0 && S.Stores >= Need && S.Stores > Strength && MarketChains::WarIn(State, Chain.Country, S.Province, Day) == INDEX_NONE) { Strength = S.Stores; Target = S.Province; }
                }
                if (!Target.IsEmpty())
                {
                    Chain.WarProvince = Target;
                    Chain.WarUntil = Day + MarketChains::WarDays;
                    Chain.Rivalry = FMath::Min(100.f, Chain.Rivalry + 8.f);
                    const TArray<FString>& Quotes = BossQuotes();
                    News.Add(FString::Printf(TEXT("%s, %s'da fiyatlar\u0131 %%8 k\u0131rd\u0131: hedefi sensin. Patronu %s: \"%s\""), *Chain.Name, *CityName(Chain.Country, Target), *Chain.Boss,
                        *Quotes[Mix(static_cast<uint32>(State.RivalSeed), Salt, 31u) % Quotes.Num()]));
                }
            }
        }

        // A strong chain buys a weak one of the country that has been for sale a while.
        if (Chain.Scope != static_cast<uint8>(EScope::Local))
        {
            for (int32 J = 0; J < State.Rivals.Chains.Num(); ++J)
            {
                FMarketChain& Other = State.Rivals.Chains[J];
                if (J == Index || Other.bGone || !Other.bForSale || Other.ForSaleTurns < 2 || Other.Country != Chain.Country) continue;
                const int64 Cost = MarketChains::Price(State, J);
                if (Chain.Cash < Cost * 3 || Roll(State, Salt, Hash(Other.Id)) > Chain.Ambition) continue;
                Chain.Cash -= Cost;
                for (const FMarketChainSpot& S : Other.Spots) SpotOf(Chain, S.Province).Stores += S.Stores;
                News.Add(FString::Printf(TEXT("%s, %s zincirini sat\u0131n ald\u0131 (%d ma\u011faza)."), *Chain.Name, *Other.Name, MarketChains::TotalStores(Other)));
                Other.bGone = true; Other.bForSale = false; Other.Spots.Reset();
                break;
            }
        }
    }

    void EndWars(FMarketState& State, int32 Day, FNews& News)
    {
        for (FMarketChain& Chain : State.Rivals.Chains)
        {
            if (Chain.WarProvince.IsEmpty() || Day <= Chain.WarUntil) continue;
            // We held on when a branch of ours there is still open with a good mark.
            bool bHeld = false;
            for (int32 I = 0; I < State.Branches.Num(); ++I)
            {
                const FMarketBranch& B = State.Branches[I];
                if (B.Stage != static_cast<uint8>(MarketBranches::EStage::Open) || MarketBranches::CountryOf(State, B) != Chain.Country || B.Province != Chain.WarProvince) continue;
                const FString Grade = MarketBranches::Grade(State, I);
                if (Grade == TEXT("A") || Grade == TEXT("B")) bHeld = true;
            }
            const bool bHome = Chain.Country == State.CountryId && Chain.WarProvince == MarketStart::HomeProvince(State);
            if (bHeld || bHome)
            {
                ++Chain.WarsLost;
                Chain.Rivalry = FMath::Min(100.f, Chain.Rivalry + 5.f);
                News.Add(FString::Printf(TEXT("%s, %s'daki fiyat sava\u015f\u0131ndan \u00e7ekildi. Ma\u011fazan ayakta kald\u0131."), *Chain.Name, *CityName(Chain.Country, Chain.WarProvince)));
            }
            else News.Add(FString::Printf(TEXT("%s'daki fiyat sava\u015f\u0131 bitti; %s fiyatlar\u0131 eski d\u00fczeyine d\u00f6nd\u00fc."), *CityName(Chain.Country, Chain.WarProvince), *Chain.Name));
            Chain.WarProvince.Reset();
        }
    }

    void Giants(FMarketState& State, int32 Day, FNews& News)
    {
        FMarketChainsState& R = State.Rivals;
        if (Day - R.LastYearDay < 365) return;
        R.LastYearDay = Day;
        TArray<FString> Ours = R.Countries;
        for (FMarketGiant& G : R.Giants)
        {
            const float Swing = (Roll(State, Hash(G.Id), static_cast<uint32>(Day)) - 0.5f) * 0.03f;
            G.RevenueB = FMath::Max(1.f, G.RevenueB * (1.f + G.Growth + Swing));
            // Entering one of our countries (not its home, not twice): rare, and more likely where the market grows.
            for (const FString& Country : Ours)
            {
                if (G.Countries.Contains(Country)) continue;
                const bool bHasArm = R.Chains.ContainsByPredicate([&G, &Country](const FMarketChain& C) { return !C.bGone && C.Country == Country && C.Home == G.Id; });
                if (bHasArm) { G.Countries.Add(Country); continue; }
                if (Day < 6 * 365 || Roll(State, Hash(G.Id) ^ Hash(Country), static_cast<uint32>(Day)) > 0.04f) continue;
                const MarketCountry::FProfile* Pack = MarketCountry::Find(Country);
                if (!Pack) continue;
                FMarketChain Arm;
                Arm.Id = G.Id + TEXT(".") + Country;
                Arm.Country = Country; Arm.Name = G.Name; Arm.Boss = PersonName(State, Country, Hash(Arm.Id));
                Arm.Archetype = G.Archetype; Arm.Scope = static_cast<uint8>(EScope::Foreign); Arm.Home = G.Id;
                Arm.PriceIndex = 0.95f; Arm.Service = 1.05f; Arm.Aggression = 0.6f; Arm.Ambition = 0.8f;
                // It starts in the biggest provinces with deep pockets.
                TArray<const MarketCountry::FCity*> Big;
                for (const MarketCountry::FCity& City : Pack->Cities) Big.Add(&City);
                Big.Sort([](const MarketCountry::FCity& A, const MarketCountry::FCity& B) { return A.PopulationK > B.PopulationK; });
                for (int32 K = 0; K < FMath::Min(3, Big.Num()); ++K) SpotOf(Arm, Big[K]->Id).Stores = Arch(static_cast<EArchetype>(G.Archetype)).Weight > 5.f ? 2 : 10;
                Arm.Cash = FMath::RoundToInt64(24.0 * 20.0 * FixedMonth(static_cast<EArchetype>(G.Archetype), 1.2f) * MarketPrices::ListLevel(Day));
                Arm.TurnDay = Day;
                R.Chains.Add(Arm);
                G.Countries.Add(Country);
                News.Add(FString::Printf(TEXT("D\u00fcnya devi %s, %s pazar\u0131na girdi. \u0130lk ma\u011fazalar\u0131 b\u00fcy\u00fck \u015fehirlerde."), *G.Name, *Pack->Name));
            }
        }
    }

    void Nemesis(FMarketState& State, FNews& News)
    {
        FMarketChainsState& R = State.Rivals;
        int32 Best = INDEX_NONE;
        float Top = 40.f;
        for (int32 I = 0; I < R.Chains.Num(); ++I)
            if (!R.Chains[I].bGone && R.Chains[I].Rivalry >= Top) { Top = R.Chains[I].Rivalry; Best = I; }
        const FString Id = Best != INDEX_NONE ? R.Chains[Best].Id : FString();
        if (Id == R.Nemesis) return;
        const FString Before = R.Nemesis;
        R.Nemesis = Id;
        if (Best == INDEX_NONE) return;
        // A new nemesis only when it clearly overtakes the old one (no flip-flop every month).
        const int32 Old = FindChain(State, Before);
        if (Old != INDEX_NONE && !R.Chains[Old].bGone && R.Chains[Old].Rivalry + 10.f > Top) { R.Nemesis = Before; return; }
        const FMarketChain& C = R.Chains[Best];
        const TArray<FString>& Quotes = NemesisQuotes();
        News.Add(FString::Printf(TEXT("Ezeli rakibin art\u0131k %s. Patronu %s: \"%s\""), *C.Name, *C.Boss, *Quotes[Hash(C.Id) % Quotes.Num()]));
    }

    void Ranks(FMarketState& State, int32 Day, FNews& News)
    {
        FMarketChainsState& R = State.Rivals;
        if (Day - R.LastLeagueDay < MarketChains::TurnDays) return;
        R.LastLeagueDay = Day;
        const int32 National = MarketChains::OurRank(MarketChains::NationalTable(State, State.CountryId));
        const int32 World = MarketChains::OurRank(MarketChains::WorldTable(State));
        if (National > 0 && R.NationalRank > 0 && National < R.NationalRank && National <= 10)
            News.Add(FString::Printf(TEXT("\u00dclkede perakendeciler aras\u0131nda %d. s\u0131raya \u00e7\u0131kt\u0131n."), National));
        if (World > 0 && R.LeagueRank > 0 && World < R.LeagueRank && World <= 25)
            News.Add(FString::Printf(TEXT("D\u00fcnya perakende liginde %d. s\u0131radas\u0131n."), World));
        auto Mark = [&State](int32 Now, int32& BestRank, const TCHAR* Scope)
        {
            if (Now <= 0) return;
            const bool bBetter = BestRank == 0 || Now < BestRank;
            for (const int32 Step : { 10, 3, 1 })
                if (bBetter && Now <= Step && (BestRank == 0 || BestRank > Step))
                {
                    MarketStory::AddMemory(State, Step == 1 ? FString::Printf(TEXT("%s birincisi olduk"), Scope) : FString::Printf(TEXT("%s ilk %d'e girdik"), Scope, Step));
                    break;
                }
            if (bBetter) BestRank = Now;
        };
        Mark(National, R.BestNationalRank, TEXT("\u00dclkede"));
        Mark(World, R.BestLeagueRank, TEXT("D\u00fcnya liginde"));
        R.NationalRank = National;
        R.LeagueRank = World;
    }
}

void MarketChains::CloseDay(FMarketState& State)
{
    using namespace MarketChainsLocal;
    Ensure(State);
    const int32 Day = State.Day;
    FNews News(State);
    EndWars(State, Day, News);
    for (int32 I = 0; I < State.Rivals.Chains.Num(); ++I)
    {
        if (State.Rivals.Chains[I].bGone) continue;
        if (Day - State.Rivals.Chains[I].TurnDay >= TurnDays) Turn(State, I, Day, News);
    }
    Giants(State, Day, News);
    Nemesis(State, News);
    Ranks(State, Day, News);
}

FString MarketChains::Describe(const FMarketState& State, int32 ChainIndex)
{
    if (!State.Rivals.Chains.IsValidIndex(ChainIndex)) return FString();
    const FMarketChain& C = State.Rivals.Chains[ChainIndex];
    FString Line = FString::Printf(TEXT("%s \u00b7 %s \u00b7 %d ma\u011faza \u00b7 y\u0131ll\u0131k ciro %s"), *C.Name, *ArchetypeName(static_cast<EArchetype>(C.Archetype)),
        TotalStores(C), *MarketCountry::Money(YearRevenue(State, ChainIndex)));
    if (C.Id == State.Rivals.Nemesis) Line += TEXT(" \u00b7 ezeli rakip");
    if (!C.WarProvince.IsEmpty() && State.Day <= C.WarUntil) Line += FString::Printf(TEXT(" \u00b7 %s'da sana kar\u015f\u0131 fiyat sava\u015f\u0131nda"), *MarketChainsLocal::CityName(C.Country, C.WarProvince));
    if (C.bForSale) Line += FString::Printf(TEXT(" \u00b7 SATILIK (%s)"), *MarketCountry::Money(Price(State, ChainIndex)));
    return Line;
}

FString MarketChains::NemesisLine(const FMarketState& State)
{
    const int32 Index = MarketChainsLocal::FindChain(State, State.Rivals.Nemesis);
    if (Index == INDEX_NONE) return FString();
    const FMarketChain& C = State.Rivals.Chains[Index];
    return FString::Printf(TEXT("Ezeli rakip: %s (%s) \u00b7 %d ma\u011faza \u00b7 senden \u00e7ekildi\u011fi sava\u015f: %d"), *C.Name, *C.Boss, TotalStores(C), C.WarsLost);
}

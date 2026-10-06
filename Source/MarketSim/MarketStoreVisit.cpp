#include "MarketStoreVisit.h"
#include "MarketEconomy.h"
#include "MarketBranches.h"
#include "MarketStart.h"

namespace MarketStoreVisitLocal
{
    bool BranchOpen(const FMarketBranch& B) { return B.Stage == static_cast<uint8>(MarketBranches::EStage::Open); }

    FString ProvinceOf(const FMarketState& State, const FMarketBranch& B)
    {
        return B.Province.IsEmpty() ? MarketStart::HomeProvince(State) : B.Province;
    }

    // Where a store stands and what it is.
    bool Describe(const FMarketState& State, int32 Store, FString& OutCountry, FString& OutProvince, FString& OutFormat)
    {
        if (Store == MarketStoreVisit::FirstStore)
        {
            if (State.FirstStoreStatus != 0) return false;
            OutCountry = State.CountryId;
            OutProvince = MarketStart::HomeProvince(State);
            OutFormat = TEXT("mahalle");
            return true;
        }
        if (!State.Branches.IsValidIndex(Store) || !BranchOpen(State.Branches[Store])) return false;
        const FMarketBranch& B = State.Branches[Store];
        OutCountry = MarketBranches::CountryOf(State, B);
        OutProvince = ProvinceOf(State, B);
        OutFormat = B.Format;
        return true;
    }

    float EmptyShare(const TArray<FMarketStock>& Items)
    {
        int32 Sold = 0, Empty = 0;
        for (const FMarketStock& Item : Items) { Sold += Item.Yesterday.Sold; Empty += Item.Yesterday.Empty; }
        return Sold + Empty > 0 ? static_cast<float>(Empty) / (Sold + Empty) : 0.f;
    }

    // A small, stable mixer: the same seed gives the same store.
    uint32 Mix(uint32 A, uint32 B)
    {
        uint32 H = 2166136261u ^ A;
        H *= 16777619u;
        H ^= B + 0x9E3779B9u + (H << 6) + (H >> 2);
        H *= 16777619u;
        return H;
    }
}

TArray<int32> MarketStoreVisit::StoresOf(const FMarketState& State, const FString& Country, const FString& Province, const FString& Format)
{
    TArray<int32> Out;
    FString C, P, F;
    if (MarketStoreVisitLocal::Describe(State, FirstStore, C, P, F) && C == Country && P == Province && F == Format) Out.Add(FirstStore);
    for (int32 I = 0; I < State.Branches.Num(); ++I)
        if (MarketStoreVisitLocal::Describe(State, I, C, P, F) && C == Country && P == Province && F == Format) Out.Add(I);
    return Out;
}

TArray<MarketStoreVisit::FTypeCount> MarketStoreVisit::TypesIn(const FMarketState& State, const FString& Country, const FString& Province)
{
    TArray<FTypeCount> Out;
    TArray<FString> Order = MarketBranches::FormatsIn(Country);
    // A type no longer sold in the country can still have an old store of ours: it comes last.
    for (const FMarketBranch& B : State.Branches) if (!B.Format.IsEmpty()) Order.AddUnique(B.Format);
    Order.AddUnique(TEXT("mahalle"));
    for (const FString& Format : Order)
    {
        const int32 Count = StoresOf(State, Country, Province, Format).Num();
        if (Count <= 0) continue;
        FTypeCount Row;
        Row.Format = Format;
        Row.Name = MarketBranches::FormatInfo(Format).Short;
        Row.Count = Count;
        Out.Add(Row);
    }
    return Out;
}

float MarketStoreVisit::Weight(const FMarketState& State, int32 Store)
{
    if (Store == FirstStore)
    {
        if (State.FirstStoreStatus != 0) return 0.f;
        float W = 1.f + 3.f * MarketStoreVisitLocal::EmptyShare(State.Stock);
        if (State.LastLostWaiting > 0) W += 1.f;
        return W;
    }
    if (!State.Branches.IsValidIndex(Store) || !MarketStoreVisitLocal::BranchOpen(State.Branches[Store])) return 0.f;
    const FMarketBranch& B = State.Branches[Store];
    float W = 1.f + 3.f * MarketStoreVisitLocal::EmptyShare(B.Items);
    if (B.LastQueueLost > 0) W += 1.f;
    if (B.Satisfaction < 55.f) W += 1.f;
    const FString Grade = MarketBranches::Grade(State, Store);
    if (Grade == TEXT("D") || Grade == TEXT("E")) W += 1.f;
    if (State.Day - B.OpenedDay < 30) W += 0.5f;
    // Seen in the last three days: let another one come first.
    if (B.VisitedDay > 0 && State.Day - B.VisitedDay <= 3) W *= 0.4f;
    return W;
}

int32 MarketStoreVisit::Pick(const FMarketState& State, const FString& Country, const FString& Province, const FString& Format, int32 Seed)
{
    const TArray<int32> Stores = StoresOf(State, Country, Province, Format);
    if (Stores.Num() == 0) return None;
    if (Stores.Num() == 1) return Stores[0];
    TArray<float> Weights;
    float Total = 0.f;
    for (const int32 Store : Stores) { const float W = FMath::Max(0.05f, Weight(State, Store)); Weights.Add(W); Total += W; }
    const uint32 Hash = MarketStoreVisitLocal::Mix(static_cast<uint32>(Seed), static_cast<uint32>(State.Day * 7919 + State.RivalSeed));
    float Draw = static_cast<float>(Hash % 100000u) / 100000.f * Total;
    for (int32 I = 0; I < Stores.Num(); ++I)
    {
        if (Draw < Weights[I]) return Stores[I];
        Draw -= Weights[I];
    }
    return Stores.Last();
}

int32 MarketStoreVisit::Next(const FMarketState& State, int32 Store)
{
    FString C, P, F;
    if (!MarketStoreVisitLocal::Describe(State, Store, C, P, F)) return None;
    const TArray<int32> Stores = StoresOf(State, C, P, F);
    const int32 At = Stores.IndexOfByKey(Store);
    return Stores.Num() == 0 ? None : At == INDEX_NONE ? Stores[0] : Stores[(At + 1) % Stores.Num()];
}

FString MarketStoreVisit::Header(const FMarketState& State, int32 Store)
{
    FString C, P, F;
    if (!MarketStoreVisitLocal::Describe(State, Store, C, P, F)) return FString();
    const TArray<int32> Stores = StoresOf(State, C, P, F);
    const MarketBranches::FSite Site = MarketBranches::SiteOf(State, C, P);
    const FString Place = Site.bValid ? Site.Name : P;
    return FString::Printf(TEXT("%s \u00b7 %s %d/%d"), *Place, MarketBranches::FormatInfo(F).Short, Stores.IndexOfByKey(Store) + 1, Stores.Num());
}

int32 MarketStoreVisit::OnlyStore(const FMarketState& State)
{
    int32 Found = None, Count = 0;
    if (State.FirstStoreStatus == 0) { Found = FirstStore; ++Count; }
    for (int32 I = 0; I < State.Branches.Num(); ++I)
        if (MarketStoreVisitLocal::BranchOpen(State.Branches[I])) { Found = I; ++Count; }
    return Count == 1 ? Found : None;
}

bool MarketStoreVisit::StartsInStore(const FMarketState& State)
{
    return OnlyStore(State) != None;
}

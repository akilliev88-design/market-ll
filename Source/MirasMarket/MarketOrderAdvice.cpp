#include "MarketOrderAdvice.h"

int32 MarketOrderAdvice::CaseUnits(const FMarketProduct& Product)
{
    return FMath::Clamp(Product.CaseUnits, 1, 48);
}

int32 MarketOrderAdvice::SuggestCases(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index, const TArray<float>* DemandScale)
{
    if (!State.Stock.IsValidIndex(Index) || !Products.IsValidIndex(Index)) return 0;
    const FMarketStock& Item = State.Stock[Index];
    if (Item.Capacity <= 0) return 0;
    const int32 Units = CaseUnits(Products[Index]);
    const int32 Demand = Item.Yesterday.Sold + 2 * Item.Yesterday.Empty;
    const float Scale = DemandScale && DemandScale->IsValidIndex(Index) ? FMath::Clamp((*DemandScale)[Index], 0.3f, 3.f) : 1.f;
    const int32 Target = FMath::Max(Item.Capacity, FMath::CeilToInt32(Demand * SafetyFactor * Scale));
    const int32 Have = Item.Shelf + Item.Warehouse + Item.Dock + Item.Incoming;
    const int32 Need = FMath::Max(0, Target - Have);
    const int32 Room = FMath::Max(0, FMarketState::StorageCapacity - Item.Warehouse - Item.Dock - Item.Incoming);
    return FMath::Clamp(FMath::Min((Need + Units - 1) / Units, Room / Units), 0, MaxCases);
}

int32 MarketOrderAdvice::FillSuggested(const FMarketState& State, const TArray<FMarketProduct>& Products, TArray<int32>& Draft, const TArray<float>* DemandScale)
{
    if (Draft.Num() != Products.Num()) Draft.Init(0, Products.Num());
    int32 Changed = 0;
    for (int32 I = 0; I < Products.Num(); ++I)
    {
        const int32 Suggested = SuggestCases(State, Products, I, DemandScale);
        if (Suggested > Draft[I]) { Draft[I] = Suggested; ++Changed; }
    }
    return Changed;
}

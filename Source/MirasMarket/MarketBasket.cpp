#include "MarketBasket.h"

namespace
{
    FMarketLoyalty* MutableCustomer(FMarketState& State, int32 CustomerId)
    {
        return State.Loyalty.FindByPredicate([CustomerId](const FMarketLoyalty& Entry) { return Entry.CustomerId == CustomerId; });
    }
}

TArray<int32> MarketBasket::BuildList(const FMarketState& State, int32 DesiredCount, FRandomStream& Random)
{
    TArray<int32> Result;
    const int32 Count = FMath::Min(FMath::Clamp(DesiredCount, MinListSize, MaxListSize), State.Stock.Num());
    for (int32 Attempt = 0; Result.Num() < Count && Attempt < Count * 12; ++Attempt)
    {
        const int32 Product = MarketDemand::PickWanted(State, Random.FRand(), Random.FRand());
        if (Product != INDEX_NONE) Result.AddUnique(Product);
    }
    // A tiny catalog or unlucky repeated rolls must still produce the requested number of distinct lines.
    for (int32 I = 0; Result.Num() < Count && I < State.Stock.Num(); ++I) Result.AddUnique(I);
    return Result;
}

int32 MarketBasket::FindSubstitute(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Wanted,
    const TArray<int32>& Available, const TSet<int32>& Excluded, float RivalDiscount)
{
    if (!Products.IsValidIndex(Wanted)) return INDEX_NONE;
    int32 Best = INDEX_NONE;
    double BestRatio = TNumericLimits<double>::Max();
    for (int32 I = 0; I < Products.Num(); ++I)
    {
        if (I == Wanted || Excluded.Contains(I) || !State.Stock.IsValidIndex(I) || !Available.IsValidIndex(I) ||
            Available[I] <= 0 || State.Stock[I].Capacity <= 0 ||
            !Products[I].Category.Equals(Products[Wanted].Category, ESearchCase::IgnoreCase)) continue;
        const double Ratio = MarketDemand::PriceRatio(State.Stock[I].Price, MarketDemand::RivalPrice(Products[I], RivalDiscount));
        if (Ratio < BestRatio || (FMath::IsNearlyEqual(Ratio, BestRatio) && I < Best))
        {
            Best = I;
            BestRatio = Ratio;
        }
    }
    return Best;
}

const FMarketLoyalty* MarketBasket::FindCustomer(const FMarketState& State, int32 CustomerId)
{
    return State.Loyalty.FindByPredicate([CustomerId](const FMarketLoyalty& Entry) { return Entry.CustomerId == CustomerId; });
}

int32 MarketBasket::ChooseCustomer(FMarketState& State, float RepeatRoll, float IdentityRoll, bool& bOutReturning)
{
    bOutReturning = false;
    TArray<int32> Returning;
    for (int32 I = 0; I < State.Loyalty.Num(); ++I) if (State.Loyalty[I].Visits > 0) Returning.Add(I);
    const float PickRoll = FMath::Clamp(IdentityRoll, 0.f, 1.f);
    if (Returning.Num() > 0 && RepeatRoll < RepeatChance)
    {
        const int32 Pick = FMath::Clamp(FMath::FloorToInt32(PickRoll * Returning.Num()), 0, Returning.Num() - 1);
        bOutReturning = true;
        return State.Loyalty[Returning[Pick]].CustomerId;
    }

    const int32 Start = FMath::Clamp(FMath::FloorToInt32(PickRoll * NeighbourhoodSize), 0, NeighbourhoodSize - 1);
    for (int32 Step = 0; Step < NeighbourhoodSize; ++Step)
    {
        const int32 Id = (Start + Step) % NeighbourhoodSize;
        if (!FindCustomer(State, Id))
        {
            FMarketLoyalty Entry;
            Entry.CustomerId = Id;
            State.Loyalty.Add(Entry);
            return Id;
        }
    }
    const int32 Pick = FMath::Clamp(FMath::FloorToInt32(PickRoll * State.Loyalty.Num()), 0, State.Loyalty.Num() - 1);
    bOutReturning = State.Loyalty[Pick].Visits > 0;
    return State.Loyalty[Pick].CustomerId;
}

float MarketBasket::EffectiveMarketShare(const FMarketState& State, int32 CustomerId)
{
    const FMarketLoyalty* Customer = FindCustomer(State, CustomerId);
    const float Personal = Customer ? (Customer->Satisfaction - 50.f) * 0.30f : 0.f;
    return FMath::Clamp(State.MarketShare + Personal, 0.f, 100.f);
}

void MarketBasket::RecordVisit(FMarketState& State, int32 CustomerId, int32 Requested, int32 Fulfilled, bool bWaited)
{
    FMarketLoyalty* Customer = MutableCustomer(State, CustomerId);
    if (!Customer) return;
    const float Ratio = Requested > 0 ? FMath::Clamp(static_cast<float>(Fulfilled) / Requested, 0.f, 1.f) : 0.f;
    const float VisitScore = FMath::Clamp(15.f + 85.f * Ratio - (bWaited ? 25.f : 0.f), 0.f, 100.f);
    Customer->Satisfaction = FMath::Clamp(Customer->Satisfaction * 0.65f + VisitScore * 0.35f, 0.f, 100.f);
    ++Customer->Visits;
}

FString MarketBasket::Summary(const FMarketState& State)
{
    if (State.Loyalty.Num() == 0) return TEXT("Sadik musteri kaydi henuz yok");
    float Total = 0.f;
    int32 Visits = 0;
    int32 Returning = 0;
    for (const FMarketLoyalty& Customer : State.Loyalty)
    {
        Total += Customer.Satisfaction;
        Visits += Customer.Visits;
        if (Customer.Visits > 1) ++Returning;
    }
    return FString::Printf(TEXT("Mahalle musteri havuzu %d  \u00b7  ort. memnuniyet %%%d  \u00b7  tekrar gelen %d  \u00b7  ziyaret %d"),
        State.Loyalty.Num(), FMath::RoundToInt(Total / State.Loyalty.Num()), Returning, Visits);
}

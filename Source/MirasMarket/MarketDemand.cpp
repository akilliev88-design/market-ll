#include "MarketDemand.h"
#include "MarketCountry.h"

int64 MarketDemand::RivalPrice(const FMarketProduct& Product, float RivalDiscount)
{
    return FMath::Max<int64>(1, FMath::RoundToInt64(static_cast<double>(Product.BasePrice) * FMath::Clamp(RivalDiscount, 0.1f, 2.f)));
}

double MarketDemand::PriceRatio(int64 OurPrice, int64 TheirPrice)
{
    return static_cast<double>(FMath::Max<int64>(1, OurPrice)) / static_cast<double>(FMath::Max<int64>(1, TheirPrice));
}

double MarketDemand::BuyChance(double Ratio, float MarketShare, double PriceTolerance)
{
    const double Half = HalfBuyRatio + FMath::Clamp(MarketShare, 0.f, 100.f) * ShareBonusPerPercent + PriceTolerance;
    return 1.0 / (1.0 + FMath::Exp((Ratio - Half) / Steepness));
}

float MarketDemand::ElasticityOf(const FMarketProduct& Product)
{
    return Product.Elasticity > 0.f ? FMath::Clamp(Product.Elasticity, 0.5f, 6.f) : DefaultElasticity;
}

double MarketDemand::BuyChanceFor(double Ratio, float MarketShare, double PriceTolerance, float Elasticity, float Kvi)
{
    const double Gain = (1.0 - Ratio) + (FMath::Clamp(MarketShare, 0.f, 100.f) - NeutralShare) * ShareBonusPerPercent + PriceTolerance;
    const double Known = 1.0 + FMath::Clamp(static_cast<double>(Kvi), 0.0, 1.0);
    const double Beta = FMath::Clamp(static_cast<double>(Elasticity), 0.5, 6.0) * PriceSensitivity * Known * (Gain < 0.0 ? LossAversion : 1.0);
    const double Utility = FMath::Loge(ParityChance / (1.0 - ParityChance)) + Beta * Gain;
    return 1.0 / (1.0 + FMath::Exp(-Utility));
}

FString MarketDemand::PriceWarning(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index)
{
    if (!State.Stock.IsValidIndex(Index) || !Products.IsValidIndex(Index)) return FString();
    const int64 Cost = State.UnitCost(Index, Products);
    const int64 Loss = Cost - State.Stock[Index].Price;
    if (Loss <= 0) return FString();
    return FString::Printf(TEXT("Bu fiyat maliyetin alt\u0131nda: her sat\u0131\u015fta %s zarar."), *MarketCountry::Money(Loss));
}

int64 MarketDemand::PriceStep(const FMarketProduct& Product)
{
    return FMath::Max<int64>(5, FMath::RoundToInt64(static_cast<double>(Product.BasePrice) / 100.0) * 5);
}

int32 MarketDemand::PickWanted(const FMarketState& State, float RollPool, float RollIndex)
{
    if (State.Stock.Num() == 0) return INDEX_NONE;
    TArray<int32> Pool;
    if (RollPool < CarriedShare)
    {
        for (int32 I = 0; I < State.Stock.Num(); ++I)
            if (State.Stock[I].Capacity > 0) Pool.Add(I);
    }
    if (Pool.Num() == 0)
    {
        for (int32 I = 0; I < State.Stock.Num(); ++I) Pool.Add(I);
    }
    const int32 Pick = FMath::Clamp(FMath::FloorToInt32(FMath::Clamp(RollIndex, 0.f, 1.f) * Pool.Num()), 0, Pool.Num() - 1);
    return Pool[Pick];
}

MarketDemand::FVisit MarketDemand::Decide(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Wanted, int32 Available,
    float RivalDiscount, int32 WantedQuantity, float RollPrice, float MarketShareOverride, double PriceTolerance, int64 OurPrice)
{
    FVisit Visit;
    Visit.Product = Wanted;
    if (!State.Stock.IsValidIndex(Wanted) || !Products.IsValidIndex(Wanted) || State.Stock[Wanted].Capacity <= 0)
    {
        Visit.Result = EVisit::NotCarried;
        return Visit;
    }
    if (Available <= 0)
    {
        Visit.Result = EVisit::Empty;
        return Visit;
    }
    const double Ratio = PriceRatio(OurPrice > 0 ? OurPrice : State.Stock[Wanted].Price, RivalPrice(Products[Wanted], RivalDiscount));
    const float Share = MarketShareOverride >= 0.f ? MarketShareOverride : State.MarketShare;
    if (RollPrice >= BuyChanceFor(Ratio, Share, PriceTolerance, ElasticityOf(Products[Wanted]), Products[Wanted].Kvi))
    {
        Visit.Result = EVisit::Expensive;
        return Visit;
    }
    int32 Quantity = FMath::Clamp(WantedQuantity, 1, 8); // traders buy by the half case (MarketCustomers)
    if (Ratio <= 0.92) ++Quantity;                          // a clear bargain: one more
    else if (Ratio > 1.10) Quantity = FMath::Min(Quantity, 2); // pricey: only what is needed
    Visit.Result = EVisit::Buy;
    Visit.Quantity = FMath::Clamp(Quantity, 1, Available);  // takes what is left on the shelf
    return Visit;
}

void MarketDemand::RecordLoss(FMarketState& State, const FVisit& Visit)
{
    if (Visit.Result == EVisit::Buy) return;
    ++State.Lost;
    RecordItemFailure(State, Visit);
}

void MarketDemand::RecordItemFailure(FMarketState& State, const FVisit& Visit)
{
    if (Visit.Result == EVisit::Buy) return;
    if (!State.Stock.IsValidIndex(Visit.Product)) return;
    FMarketDemandStats& Today = State.Stock[Visit.Product].Today;
    if (Visit.Result == EVisit::NotCarried) ++Today.NotCarried;
    else if (Visit.Result == EVisit::Empty) ++Today.Empty;
    else ++Today.Expensive;
}

void MarketDemand::RecordWaitingLoss(FMarketState& State)
{
    ++State.Lost;
    ++State.LostWaiting;
}

TArray<MarketDemand::FProblem> MarketDemand::TopProblems(const FMarketState& State, int32 MaxCount)
{
    TArray<FProblem> All;
    auto AddProblem = [&All](EProblem Kind, int32 Product, int32 Count)
    {
        if (Count <= 0) return;
        FProblem Problem;
        Problem.Kind = Kind; Problem.Product = Product; Problem.Count = Count;
        All.Add(Problem);
    };
    AddProblem(EProblem::Waiting, INDEX_NONE, State.LastLostWaiting);
    for (int32 I = 0; I < State.Stock.Num(); ++I)
    {
        const FMarketDemandStats& Day = State.Stock[I].Yesterday;
        AddProblem(EProblem::NotCarried, I, Day.NotCarried);
        AddProblem(EProblem::Empty, I, Day.Empty);
        AddProblem(EProblem::Expensive, I, Day.Expensive);
    }
    All.Sort([](const FProblem& A, const FProblem& B)
    {
        if (A.Count != B.Count) return A.Count > B.Count;
        if (A.Kind != B.Kind) return static_cast<uint8>(A.Kind) < static_cast<uint8>(B.Kind);
        return A.Product < B.Product;
    });
    if (All.Num() > MaxCount) All.SetNum(FMath::Max(0, MaxCount));
    return All;
}

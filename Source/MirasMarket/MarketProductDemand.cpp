#include "MarketProductDemand.h"
#include "MarketCalendar.h"
#include "MarketDemand.h"
#include "MarketOnline.h"

float MarketProductDemand::DayFactor(const FMarketState& State, MarketGoods::EGroup Group, int32 Day)
{
    return MarketCalendar::GroupFactor(Day, State.RivalSeed, Group) * MarketOnline::GroupFactorOn(State, Day, Group);
}

float MarketProductDemand::SegmentWish(const FMarketState& State, const FMarketProduct& Product, MarketCustomers::ESegment Segment, int32 Day)
{
    const MarketGoods::EGroup Group = MarketGoods::Classify(Product.Category);
    return MarketCustomers::Profile(Segment).Preference[static_cast<int32>(Group)] * DayFactor(State, Group, Day);
}

TArray<float> MarketProductDemand::MixWishes(const FMarketState& State, const TArray<FMarketProduct>& Products, const int32 (&Mix)[6], int32 Day)
{
    TArray<float> Weights;
    float Total = 0.f;
    for (const FMarketProduct& P : Products)
    {
        float W = 0.f;
        for (int32 S = 0; S < 6; ++S) W += Mix[S] / 100.f * SegmentWish(State, P, static_cast<MarketCustomers::ESegment>(S), Day);
        Weights.Add(W);
        Total += W;
    }
    if (Total > 0.f) for (float& W : Weights) W /= Total;
    return Weights;
}

double MarketProductDemand::MixTolerance(const int32 (&Mix)[6])
{
    double Sum = 0.0;
    int32 Weight = 0;
    for (int32 S = 0; S < 6; ++S)
    {
        Sum += Mix[S] * MarketCustomers::Profile(static_cast<MarketCustomers::ESegment>(S)).PriceTolerance;
        Weight += Mix[S];
    }
    return Weight > 0 ? Sum / Weight : 0.0;
}

double MarketProductDemand::IncomeTolerance(float Income)
{
    return 0.2 * (static_cast<double>(Income) - 1.0);
}

double MarketProductDemand::Acceptance(double Ratio, float SharePercent, double Tolerance, const FMarketProduct& Product)
{
    return MarketDemand::BuyChanceFor(Ratio, SharePercent, Tolerance, MarketDemand::ElasticityOf(Product), Product.Kvi);
}

double MarketProductDemand::AcceptanceFactor(double Ratio, float SharePercent, double Tolerance, const FMarketProduct& Product)
{
    return Acceptance(Ratio, SharePercent, Tolerance, Product) / MarketDemand::ParityChance;
}

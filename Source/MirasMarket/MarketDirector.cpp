#include "MarketDirector.h"
#include "MarketCalendar.h"
#include "MarketRivals.h"
#include "MarketStaff.h"

float MarketDirector::TrafficFactor(const FMarketState& State, const TArray<FString>& Aisles)
{
    return MarketCalendar::TrafficFactor(State.Day, State.RivalSeed) * MarketRivals::TrafficFactor(State.Day, State.RivalSeed, Aisles);
}

float MarketDirector::DemandWeight(const FMarketState& State, const FMarketProduct& Product)
{
    return MarketCalendar::CategoryFactor(State.Day, State.RivalSeed, Product.Category);
}

float MarketDirector::OrderScale(const FMarketState& State, const FMarketProduct& Product)
{
    // The order arrives at the next day close and is sold the day after the running day.
    const float Then = MarketCalendar::CategoryFactor(State.Day + 1, State.RivalSeed, Product.Category);
    const float Before = FMath::Max(0.2f, MarketCalendar::CategoryFactor(FMath::Max(1, State.Day - 1), State.RivalSeed, Product.Category));
    return FMath::Clamp(Then / Before, 0.3f, 3.f);
}

TArray<float> MarketDirector::OrderScales(const FMarketState& State, const TArray<FMarketProduct>& Products)
{
    TArray<float> Scales;
    Scales.Init(1.f, Products.Num());
    for (int32 I = 0; I < Products.Num(); ++I) Scales[I] = OrderScale(State, Products[I]);
    return Scales;
}

FString MarketDirector::DateText(const FMarketState& State)
{
    return MarketCalendar::DateText(State.Day);
}

FString MarketDirector::TodayText(const FMarketState& State)
{
    return MarketCalendar::Describe(State.Day, State.RivalSeed);
}

FString MarketDirector::TomorrowText(const FMarketState& State)
{
    return MarketCalendar::Forecast(State.Day, State.RivalSeed);
}

void MarketDirector::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    MarketStaff::CloseDay(State); // till, fatigue, morale, notices, HR, accountant and the weekly tax (G-060)
}

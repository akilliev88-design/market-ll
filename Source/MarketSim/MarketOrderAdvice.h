#pragma once
#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Order help at the office desk, independent of the world (test: MarketSim.Economy.OrderAdvice).
// The order itself is FMarketState::SubmitOrder (Codex, G-052); this only suggests cases and holds the
// wholesaler's minimum order.
namespace MarketOrderAdvice
{
    constexpr int32 MaxCases = 9;          // same line limit as FMarketState::SubmitOrder
    constexpr int64 MinimumOrder = 5000;   // 50 TL: the wholesaler does not come for less
    constexpr float SafetyFactor = 1.25f;  // yesterday's demand + 25 %
    // G-077 (#36): the 50 TL minimum at today's list prices.
    int64 MinimumOrderOn(int32 GameDay);

    int32 CaseUnits(const FMarketProduct& Product);
    // Cases so that tomorrow the product can fill its shelf and cover yesterday's demand (sold + about two units
    // for every shopper who found the shelf empty), minus what is on the shelf, in the depot, at the rear door or
    // on the way. 0 for a product that is on no shelf. Limited by the 120-unit depot room and MaxCases.
    // DemandScale (optional, indexed like Products): how much more or less the product is expected to sell on the
    // day the order arrives than yesterday (calendar, MarketDirector::OrderScales). Missing = 1.
    int32 SuggestCases(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index, const TArray<float>* DemandScale = nullptr);
    // Raises every line of the draft (indexed like Products) to its suggestion; never lowers a line.
    // Returns the number of lines changed.
    int32 FillSuggested(const FMarketState& State, const TArray<FMarketProduct>& Products, TArray<int32>& Draft, const TArray<float>* DemandScale = nullptr);
}

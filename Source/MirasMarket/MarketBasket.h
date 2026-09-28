#pragma once
#include "CoreMinimal.h"
#include "MarketDemand.h"

// Shopping-list, substitution and repeat-customer rules. This module has no world/actor dependency.
namespace MarketBasket
{
    constexpr int32 MinListSize = 1;
    constexpr int32 MaxListSize = 4;
    constexpr int32 NeighbourhoodSize = 24;
    constexpr float RepeatChance = 0.60f;

    // Builds 1..4 distinct product wishes. The stream makes gameplay and tests deterministic.
    TArray<int32> BuildList(const FMarketState& State, int32 DesiredCount, FRandomStream& Random);

    // Picks an in-stock product from the same category. Available is shelf stock after basket reservations;
    // Excluded contains products already in this shopper's basket. Lowest price ratio wins.
    int32 FindSubstitute(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Wanted,
        const TArray<int32>& Available, const TSet<int32>& Excluded, float RivalDiscount);

    // Returns a stable neighbourhood id. Existing satisfied/unsatisfied customers can therefore return later.
    int32 ChooseCustomer(FMarketState& State, float RepeatRoll, float IdentityRoll, bool& bOutReturning);
    const FMarketLoyalty* FindCustomer(const FMarketState& State, int32 CustomerId);
    float EffectiveMarketShare(const FMarketState& State, int32 CustomerId);

    // Updates the saved satisfaction after the visit. Fulfilled is the number of requested list lines bought.
    void RecordVisit(FMarketState& State, int32 CustomerId, int32 Requested, int32 Fulfilled, bool bWaited);
    FString Summary(const FMarketState& State);
}

#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// G-083 supply network (karar K01-K03, Docs/Kurgu/07_AKIL_ISBOLUMU.md C2c). Independent of the world, tested
// (MirasMarket.Sourcing.*). A tycoon-level view of buying, on top of the day-to-day wholesaler (MarketSuppliers):
//  - Four supply lines by aisle: drinks, dairy (the cold chain), dry food, household and care.
//  - Each line buys from a tier: the local wholesaler (start), a regional distributor, a national distributor or
//    straight from the producers. A higher tier is cheaper but asks for size (shops, a depot, central buying) and a
//    monthly minimum; a line that stays under its minimum two months running is dropped one tier.
//  - Buying power (K01): the company's purchases of the last 30 days against a small shop's make everything
//    cheaper, 3 % per doubling, at most 12 %.
//  - The dairy line above the local tier keeps the cold chain: branches spoil 10 % less dairy.
//  - Prices reach the game through MarketDirector::ApplyPrices (every product's cost x CostFactor).
namespace MarketSourcing
{
    enum class ELine : uint8 { Drinks = 0, Dairy, DryFood, Household, Count };
    enum class ETier : uint8 { Local = 0, Regional, National, Producer, Count };

    constexpr int32 LineCount = static_cast<int32>(ELine::Count);
    constexpr int32 TierCount = static_cast<int32>(ETier::Count);
    constexpr float MaxVolumeDiscount = 0.12f;
    constexpr int64 BaseVolume2011 = 400000;     // a small shop's 30-day purchases at the start level (4.000 TL)

    FString LineName(ELine Line);
    FString TierName(ETier Tier);
    // The line a product's aisle buys through.
    ELine LineOf(const FString& Category);
    // Discount of a tier (0, 2.5, 4.5, 7 %) and its monthly minimum for a line (start-level kurus).
    float TierDiscount(ETier Tier);
    int64 TierMinimum(ETier Tier);

    ETier TierOf(const FMarketState& State, ELine Line);
    // Can the line move to this tier now? (shops, a depot, central buying; the minimum is judged monthly)
    bool CanSet(const FMarketState& State, ELine Line, ETier Tier, FString& OutReason);
    bool Set(FMarketState& State, ELine Line, ETier Tier, FString& OutMessage);
    // Menu argument: line x 10 + tier.
    int32 Encode(ELine Line, ETier Tier);
    bool Decode(int32 Arg, ELine& OutLine, ETier& OutTier);

    // Buying power from the last 30 days' purchases (0 .. MaxVolumeDiscount).
    float VolumeDiscount(const FMarketState& State);
    // x a product's cost: its line's tier and the buying power.
    float CostFactor(const FMarketState& State, const FString& Category);
    // x dairy spoilage in the branches (the cold chain).
    float DairySpoilFactor(const FMarketState& State);

    // A purchase counted for its line's monthly minimum (branches' orders; the family shop is counted at the close).
    void RecordPurchase(FMarketState& State, const FString& Category, int64 Cost);

    // Menu lines: a line's tier, its discount and the next step.
    FString Describe(const FMarketState& State, ELine Line);
    // The next tier a line could take now ("" when none).
    FString NextStep(const FMarketState& State, ELine Line);

    // Day close: the family shop's day counted, and at the month's end the minimums.
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);
}

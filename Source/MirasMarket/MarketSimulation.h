#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Strategic advance and difficulty (G-071, Docs/Kurgu/00_KURGU_KITABI.md \u00a713). Independent of the world, tested
// (MirasMarket.Simulation.*).
//  - PlayDay plays the family shop's day without walking people, with the same rules the world uses: the same
//    shoppers (segment, list, budget, price tolerance), the same shelf decisions (MarketDemand), substitutes, the
//    payment method and the credit book, the till (SellBasket), then the day close of every system. The family
//    does the routine: passes the monthly price rise on to the shelf, pays the declared tax, pays 50 TL of the
//    father's debt when the till can bear it, keeps the shelves filled and orders what the order advice suggests.
//  - Advance plays up to N days and stops early when the player is needed: a decision waits (story, event,
//    money trouble), cash went below zero, or a week ended (the weekly report).
//  - Difficulty: easy = more shoppers and more forgiving prices, hard = fewer and stricter. It never changes
//    history (inflation, bayrams, rival openings); karar open question 3 (softer inflation) stays open.
namespace MarketSimulation
{
    enum class EDifficulty : uint8 { Easy = 0, Normal, Hard };

    constexpr int32 ShoppersPerDay = 55;   // an ordinary day of the world (240 s, a shopper every ~4.5 s)

    FString DifficultyName(EDifficulty Difficulty);
    float TrafficFactor(const FMarketState& State);
    double ToleranceBonus(const FMarketState& State);
    bool SetDifficulty(FMarketState& State, int32 Difficulty, FString& OutMessage);

    bool AdjustPrice(FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index, bool bUp);

    struct FDay
    {
        int32 Shoppers = 0;
        int32 Served = 0;
        int32 Lost = 0;
        int64 Revenue = 0;
        int64 Profit = 0;
        int64 FamilyProfit = 0;
        int64 Ordered = 0;
        int32 AuditFailures = 0; // independently checked checkout/order/core-close and stock transfers
        int64 BackgroundCashDelta = 0; // Director systems; full ledger audit is integrated by stream C
    };

    // Plays and closes one day (State.Day moves on). Products get the new day's prices from CatalogBase.
    FDay PlayDay(FMarketState& State, const TArray<FMarketProduct>& CatalogBase, TArray<FMarketProduct>& Products, bool bAutoOrder = true);
    // Plays up to Days days; returns the days played. OutSummary: totals and why it stopped.
    int32 Advance(FMarketState& State, const TArray<FMarketProduct>& CatalogBase, TArray<FMarketProduct>& Products, int32 Days, FString& OutSummary);
}

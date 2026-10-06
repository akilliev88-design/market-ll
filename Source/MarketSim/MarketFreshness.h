#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Perishable goods (G-067, Docs/Kurgu/00_KURGU_KITABI.md \u00a79). Independent of the world, tested
// (MarketSim.Freshness.*). Products of groups with a shelf life (MarketGoods::ShelfLifeDays: milk and yogurt 7
// days, snacks and sweets months) are followed in batches without changing the stock model: at every day close
// the batches are matched with the units the shop holds (shelf + depot + rear door). New units become a new batch;
// units that left (sold, broken) leave the oldest batch first, because workers put the old ones in front (FEFO).
// On a batch's last day the shop either sells it 30 % cheaper (markdown, default) or gives it away to families in
// need (donate: counted as waste, the neighbourhood notices). After the last day it is waste: it leaves the
// shelves and the depot, and its cost is a loss in the day report.
namespace MarketFreshness
{
    enum class EPolicy : uint8 { Nothing = 0, Markdown = 1, Donate = 2 };
    constexpr int32 MarkdownPercent = 30;

    // Units of a product whose last sellable day is today.
    int32 LastDayUnits(const FMarketState& State, const FString& ProductId);
    // x price today (0.7 with markdown on the last day of an old batch).
    // B1 (#43): only while units of that batch are left: sold units leave the oldest batch first (FEFO), so after
    // the day's sales passed the last-day units the fresh ones sell at the full price.
    float PriceFactor(const FMarketState& State, int32 Index);
    // x price of each of Quantity units bought now: the markdown on the last-day units left, full price on the rest.
    double PriceFactor(const FMarketState& State, int32 Index, int32 Quantity);
    // Last-day units of the product not sold yet today.
    int32 MarkdownUnitsLeft(const FMarketState& State, int32 Index);
    // Days until the oldest batch of the product expires (INDEX_NONE = not perishable / nothing held).
    int32 DaysLeft(const FMarketState& State, const FString& ProductId);
    FString PolicyName(EPolicy Policy);
    bool SetPolicy(FMarketState& State, EPolicy Policy, FString& OutMessage);

    // Day close (after FMarketState::CloseDay, before the books): match batches, spoil the expired ones, donate.
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);

    // E3c2 (M63): one batch rule for every store (the first store's State.Batches, a branch's own Batches).
    // Held: the units of product Id the store holds now; Arrived: units that came in since the last match. New units
    // become a batch sellable for Life days from Day; units that left came from the oldest batches (FEFO). Expired
    // batches (last day before Day) are waste; under the donation policy a batch on its last day (Day) is given away.
    // The caller takes Wasted + Donated units out of its stock.
    struct FBatchDay
    {
        int32 Wasted = 0;
        int32 Donated = 0;
    };
    FBatchDay MatchBatches(TArray<FMarketBatch>& Batches, const FString& Id, int32 Held, int32 Arrived, int32 Life, int32 Day, EPolicy Policy);
    // Units of Id in Batches whose last sellable day is Day.
    int32 LastDayUnitsIn(const TArray<FMarketBatch>& Batches, const FString& Id, int32 Day);
}

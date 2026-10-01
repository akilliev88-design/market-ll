#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Perishable goods (G-067, Docs/Kurgu/00_KURGU_KITABI.md \u00a79). Independent of the world, tested
// (MirasMarket.Freshness.*). Products of groups with a shelf life (MarketGoods::ShelfLifeDays: milk and yogurt 7
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
}

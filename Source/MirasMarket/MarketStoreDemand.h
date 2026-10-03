#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// E2 (M51, Docs/Kurgu/11_TEK_EKONOMI.md): one shopper formula for every store. A store's day draws the shopping
// trips of its catchment (province size, market type), takes its share against the province's chains and
// traditional trade (pull / (pull + competition x the chains' pressure)), and grows into it as it matures; our own
// shops in the same province share the room. The branches (MarketBranches::CloseDay) and the family shop (store 0)
// ask the same function. Independent of the world, tested (MirasMarket.StoreDemand.*).
namespace MarketStoreDemand
{
    // What one store brings to its day. Self: the branch index, or FamilyShop for the family shop.
    constexpr int32 FamilyShop = -1;
    // How fast shoppers turn away from a dearer shelf: pull x exp(-(price index - 1) / this).
    constexpr float PriceSensitivity = 0.12f;

    struct FStoreDay
    {
        FString Country;                 // empty = the campaign's country
        FString Province;
        FString Format = TEXT("mahalle"); // MarketBranches format id
        int32 Self = FamilyShop;
        float PriceIndex = 1.f;          // shelf prices against the list
        float Availability = 0.9f;       // share of what was asked that was on the shelf (0.2..1)
        float Service = 1.f;             // the format's service x the people (manager, tills, waiting)
        float Satisfaction = 50.f;       // 0..100
        float Maturity = 1.f;            // 0..1: habit; a new store grows into its share in a month
        bool bNew = false;               // its first week: the curious come (x1.3)
        bool bHasty = false;             // C15: a hastily chosen site (fewer trips)
        float PullExtra = 1.f;           // the store's own draw on top (departments: fresh bread, a good butcher)
        float TripsExtra = 1.f;          // the store's own trips on top (family shop: its promotions, events, card)
    };

    // Shopping trips a day in the store's catchment before its share (province size x format), 0 on a shut day.
    float Trips(const FMarketState& State, const FStoreDay& Store, int32 Day);
    // How strongly the store pulls the province's shoppers (price, shelves, service, habit, the company).
    float Pull(const FMarketState& State, const FStoreDay& Store);
    // The store's share of its catchment: pull / (pull + competition x the chains' pressure on Day).
    float Share(const FMarketState& State, const FStoreDay& Store, int32 Day);
    // Our other open shops in the province (the family shop counts as one) take customers once it is full.
    float Cannibalization(const FMarketState& State, const FStoreDay& Store);
    // The day's shoppers through the door (before the tills): Trips x Share x habit x our shops nearby.
    float ShoppersExact(const FMarketState& State, const FStoreDay& Store, int32 Day);
    int32 Shoppers(const FMarketState& State, const FStoreDay& Store, int32 Day);

    // The family shop as a store on Day (its shelf prices, yesterday's shelves and queue, its customers' mood,
    // its promotions, events and card terminal).
    FStoreDay FamilyDay(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Day);
    // Its real shoppers today (the walked world shows this many people; one figure walks for one shopper).
    int32 FamilyShoppers(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Day);
    // E2 calibration (11_TEK_EKONOMI E2.8, measured 03.10.2026: the formula gave the family shop 1.40 x the old
    // street model's shoppers): the family shop is an old sign in a side street; its catchment's trips x this
    // (like a hastily chosen branch site, MarketBranches::HastyTrips).
    constexpr float FamilySiteTrips = 0.72f;
    // Day close: the family shop's share of its province (State.MarketShare, percent) moves a little towards the
    // formula's share of the day that was played; habits change slowly. A day without visitors keeps it.
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);
}

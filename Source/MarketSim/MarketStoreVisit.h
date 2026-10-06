#pragma once

#include "CoreMinimal.h"

struct FMarketState;

// M70 (Mustafa 06.10.2026: "D\u00fckk\u00e2na git tu\u015funu istemiyorum; ildeki ma\u011faza t\u00fcrlerinden birine t\u0131klar gider"): a store
// is entered from the map. The province card counts our open stores by type; clicking a type enters one of them,
// picked at random with more weight on a store that needs a look (empty shelves, a queue, unhappy customers, a poor
// report card, just opened) and less on the one entered last. Tab goes to the next store of the same type in the
// province. Independent of the world, tested (MarketSim.StoreVisit.*); the 3D entry is G-119 (Codex).
// Store numbers: a branch index, or FirstStore (-1) for the first store; None when there is nothing to enter.
namespace MarketStoreVisit
{
    constexpr int32 FirstStore = -1;
    constexpr int32 None = -2;

    struct FTypeCount
    {
        FString Format;
        FString Name;
        int32 Count = 0;
    };

    // Our open stores of a type in a province (the first store counts as a neighbourhood store of the home province
    // while it is open), in a fixed order: the first store, then branches by index.
    TArray<int32> StoresOf(const FMarketState& State, const FString& Country, const FString& Province, const FString& Format);
    // The province card: our open stores by type, the types in the country's order.
    TArray<FTypeCount> TypesIn(const FMarketState& State, const FString& Country, const FString& Province);
    // How much a store asks to be looked at (1 = nothing special).
    float Weight(const FMarketState& State, int32 Store);
    // The store the click enters (None: no store of that type there). Seed: any number that changes per click.
    int32 Pick(const FMarketState& State, const FString& Country, const FString& Province, const FString& Format, int32 Seed);
    // Tab: the next store of the same type in the same province (the same store when it is the only one).
    int32 Next(const FMarketState& State, int32 Store);
    // The line on top inside the store: "Kad\u0131k\u00f6y \u00b7 S\u00fcpermarket 2/5".
    FString Header(const FMarketState& State, int32 Store);
    // The game opens inside the store while the company has one open store; from the second on, on the map.
    bool StartsInStore(const FMarketState& State);
    // The store a new session starts in (the only one), or None.
    int32 OnlyStore(const FMarketState& State);
}

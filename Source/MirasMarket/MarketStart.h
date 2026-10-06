#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// A new campaign (G-084, karar L02-L03; M69): the player picks a country, a city, the market's name, their own name
// and the difficulty. The one story is the start: a small market (not a corner kiosk), the only shop of its kind in
// its town, which a family ran for years; they grew tired and handed it over to the player. The building comes with
// it (no rent for the first store). One cashier and two shelf stockers, a light debt to the wholesaler the shop
// already had (no deadline, no interest, it locks nothing) and shelves in order but not full.
// Independent of the world, tested (MirasMarket.Start.*).
namespace MarketStart
{
    // M37: the inherited debt in months of the shop's fixed costs (the till holds one month).
    constexpr double StartDebtMonths = 1.5;
    constexpr int32 MaxPlayerNameLength = 30;

    // Seeds the campaign: country, city, the starting staff and the debt. Call after FMarketState::Initialize
    // (the market's and the player's names and the difficulty are already on State). Day 1 is the first morning.
    void Setup(FMarketState& State, const FString& CountryId, const FString& CityId, int32 Seed);
    // After the shelf capacities are known (ApplyShelfCapacities): 40-75 % of every shelf from the warehouse,
    // seeded. Returns the units moved.
    int32 StockShelvesPartly(FMarketState& State, int32 Seed);
    // There is no default start province (Mustafa, 29.09.2026): the player picks one. This is only the fallback
    // of automated runs without a province: the country's median province by population (M61b: no province is
    // special), else its first province.
    FString FallbackProvince(const FString& CountryId);
    // The province of the first store (the campaign's, else FallbackProvince).
    FString HomeProvince(const FMarketState& State);
    // "<city>, <country>" (the country alone when the province is unknown).
    FString PlaceText(const FMarketState& State);
    // M69: the first store is called by its name: the market's name and its city ("<brand> <city>").
    FString FirstStoreName(const FMarketState& State);
    // The player's name as written at the start ("" when none was given).
    FString PlayerName(const FMarketState& State);
    // The first notice of a new campaign.
    FString IntroText(const FMarketState& State);
}

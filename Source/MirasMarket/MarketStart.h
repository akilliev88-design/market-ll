#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// A new campaign (G-084, karar L02-L03; M36): the player picks a country and a city; father and mother retire and
// leave their market to us (the building stays theirs: the shop pays them rent like any other shop): one cashier
// and two shelf stockers, the father's debt to the wholesaler and shelves in order but not full. Independent of the world, tested (MirasMarket.Start.*).
namespace MarketStart
{
    enum class ECase : uint8 { Plain = 0, Genitive, Ablative, With, Mine };
    // M37: the father's debt in months of the shop's fixed costs (the till holds one month).
    constexpr double StartDebtMonths = 1.5;

    // Seeds the campaign: country, city, the relative, the starting staff and the debt. Call after
    // FMarketState::Initialize. Days of the prototype stay as they are (day 1 = the first morning).
    void Setup(FMarketState& State, const FString& CountryId, const FString& CityId, int32 Seed);
    // After the shelf capacities are known (ApplyShelfCapacities): 40-75 % of every shelf from the warehouse,
    // seeded. Returns the units moved.
    int32 StockShelvesPartly(FMarketState& State, int32 Seed);
    // The father who left us the shop (M36), in Turkish with the possessive ("baban", "baban\u0131n", "babandan",
    // "babanla"; Mine = "babam\u0131n", as the player says it). bCapital: first letter upper case.
    FString Relative(const FMarketState& State, ECase Case, bool bCapital = false);
    // There is no default start province (Mustafa, 29.09.2026): the player picks one. This is only the fallback
    // of automated runs without a province: the country's median province by population (M61b: no province is
    // special), else its first province.
    FString FallbackProvince(const FString& CountryId);
    // The province of the family shop (the campaign's, else FallbackProvince).
    FString HomeProvince(const FMarketState& State);
    // "K\u0131rklareli, T\u00fcrkiye" (the country alone when the province is unknown).
    FString PlaceText(const FMarketState& State);
    // The first notice of a new campaign.
    FString IntroText(const FMarketState& State);
    // Keys of the relatives the game knows.
    const TArray<FString>& RelativeKeys();
}

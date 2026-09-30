#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// A new campaign (G-084, karar L02-L03): the player picks a country and a city; the shop was left by a relative
// (not the father): a small market with one cashier and two shelf stockers, its own debt to the wholesaler and
// shelves in order but not full. Independent of the world, tested (MirasMarket.Start.*).
namespace MarketStart
{
    enum class ECase : uint8 { Plain = 0, Genitive, Ablative, With, Mine };
    // Days of the starting staff's wages found in the till (Claude's balance proposal next to karar L03).
    constexpr int32 StartWageDays = 7;

    // Seeds the campaign: country, city, the relative, the starting staff and the debt. Call after
    // FMarketState::Initialize. Days of the prototype stay as they are (day 1 = the first morning).
    void Setup(FMarketState& State, const FString& CountryId, const FString& CityId, int32 Seed);
    // After the shelf capacities are known (ApplyShelfCapacities): 40-75 % of every shelf from the warehouse,
    // seeded. Returns the units moved.
    int32 StockShelvesPartly(FMarketState& State, int32 Seed);
    // The relative who left the shop, in Turkish with the possessive ("teyzen", "teyzenin", "teyzenden",
    // "teyzenle"; Mine = "teyzemin", as the player says it). Older saves: the father ("baban"...). bCapital: first letter upper case.
    FString Relative(const FMarketState& State, ECase Case, bool bCapital = false);
    // There is no default start province (Mustafa, 29.09.2026): the player picks one. This is only the fallback
    // for older saves (no province yet) and automated runs: the pack's reference province (Turkey: Kirklareli,
    // where the prototype's shop was), else its first province.
    FString LegacyProvince(const FString& CountryId);
    // The province of the family shop (the campaign's, else LegacyProvince).
    FString HomeProvince(const FMarketState& State);
    // "K\u0131rklareli, T\u00fcrkiye" (the country alone when the province is unknown).
    FString PlaceText(const FMarketState& State);
    // The first notice of a new campaign.
    FString IntroText(const FMarketState& State);
    // Keys of the relatives the game knows.
    const TArray<FString>& RelativeKeys();
}

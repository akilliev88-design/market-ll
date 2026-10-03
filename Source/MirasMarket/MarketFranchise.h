#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// D6 (M67, 09_DUNYA_YENIDEN S5): a second way into a country besides an own store or a chain bought there. A master
// franchise: a local partner opens stores under our brand and pays a royalty on their sales every month (the
// country's withholding tax is taken from it). Little money and no risk at the start, but the partner keeps the
// stores and the country: while it runs we open no store of our own there. Ending it costs a year of royalty
// (at least the contract fee); the partner's stores take their own name back. It needs what an own store needs
// abroad: the company's scale (MarketCompany::AbroadOpen) and a ready market study (MarketResearch). Independent of
// the world, tested (MirasMarket.Expansion.*).
namespace MarketFranchise
{
    constexpr int32 StartStores = 3;           // the partner opens with three stores
    constexpr int32 GrowEvery = 45;            // and one more every 45 days up to MaxStores
    constexpr float RoyaltyRate = 0.04f;       // of the partner stores' sales
    constexpr int64 StoreDaySales = 80000;     // start-level kurus a typical partner store sells a day
    constexpr int32 EndRoyaltyMonths = 12;     // ending the contract: a year of royalty

    // The running franchise in a country (nullptr: none, or ended).
    const FMarketFranchise* Find(const FMarketState& State, const FString& Country);
    // Stores a partner opens at most in a country (8..60 by its people).
    int32 MaxStores(const FString& Country);
    // The contract and brand registration fee (our money): as much as a market study of the country.
    int64 Fee(const FMarketState& State, const FString& Country);
    bool CanSign(const FMarketState& State, const FString& Country, FString& OutReason);
    bool Sign(FMarketState& State, const FString& Country, FString& OutMessage);
    int64 EndCost(const FMarketState& State, const FString& Country);
    bool End(FMarketState& State, const FString& Country, FString& OutMessage);
    // Partner stores and their sales of a year at today's pace (home-level kurus) in a country; countries with a
    // running franchise.
    int32 StoresIn(const FMarketState& State, const FString& Country);
    int64 YearSales(const FMarketState& State, const FString& Country);
    TArray<FString> Countries(const FMarketState& State);
    // One line for the menu.
    FString StatusText(const FMarketState& State, const FString& Country);
    // Day close: the day's sales, a new partner store every 45 days, the royalty on the first of the month.
    void CloseDay(FMarketState& State);
}

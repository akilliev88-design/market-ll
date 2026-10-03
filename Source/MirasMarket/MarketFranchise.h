#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// D6 (M67, 09_DUNYA_YENIDEN S5): a second way into a country besides an own store or a chain bought there. A master
// franchise: a local partner opens stores under our brand when it can (Mustafa 04.10.2026: no day rule; when it has
// saved a store's opening from its own stores' profit and the country still has room) and pays a royalty on their
// sales every month (the
// country's withholding tax is taken from it). Little money and no risk at the start, but the partner keeps the
// stores and the country: while it runs we open no store of our own there. Ending it costs a year of royalty
// (at least the contract fee); the partner's stores take their own name back. It needs what an own store needs
// abroad: the company's scale (MarketCompany::AbroadOpen) and a ready market study (MarketResearch). Independent of
// the world, tested (MirasMarket.Expansion.*).
namespace MarketFranchise
{
    constexpr int32 StartStores = 3;           // the partner opens with three stores
    constexpr float PartnerMargin = 0.035f;    // what a partner store keeps of its sales after its costs and our royalty
    constexpr float Crowding = 0.35f;          // a full country (MaxStores) sells 35 % less a store than an empty one
    constexpr float MinStoreShare = 0.7f;      // a new store only while each store would still sell 70 % of a lone one
    constexpr float RoyaltyRate = 0.04f;       // of the partner stores' sales
    constexpr int64 StoreDaySales = 80000;     // start-level kurus a typical partner store sells a day
    constexpr int32 EndRoyaltyMonths = 12;     // ending the contract: a year of royalty

    // The running franchise in a country (nullptr: none, or ended).
    const FMarketFranchise* Find(const FMarketState& State, const FString& Country);
    // Stores a partner opens at most in a country (8..60 by its people).
    int32 MaxStores(const FString& Country);
    // A day of one partner store's sales today (home-level kurus; fewer as the brand fills the country) and what a
    // new store costs the partner (home-level kurus: a neighbourhood store's fit-out and two months' rent).
    int64 StoreDaySalesNow(const FMarketState& State, const FMarketFranchise& F);
    int64 OpeningCost(const FMarketState& State);
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
    // Day close: the day's sales and the partner's savings; a new store when the savings pay for one and the country
    // has room; the royalty on the first of the month.
    void CloseDay(FMarketState& State);
}

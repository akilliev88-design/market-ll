#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Competing companies of the district (G-065, Docs/Kurgu/00_KURGU_KITABI.md \u00a77). Independent of the world,
// tested (MirasMarket.Competitors.*). MarketRivals keeps the chains' daily news; this module is their memory and
// strategy, and the district's share model:
//  - Every day the district's shopping is split by attractiveness: price level, availability (full shelves),
//    service (no waiting), loyalty, promotions and distance. Our result pulls State.MarketShare, and the share
//    brings more or fewer shoppers through the door (TrafficFactor).
//  - The shopper's idea of "the rival's price" for a product is the share-weighted price of the open rivals
//    (RivalPriceFactor), including their campaigns and our local rival's price wars.
//  - Bereket Market (fictional, same street): proud and touchy. Loses share to us -> gets angry -> starts a
//    price war on the aisle we sell most; the war costs him money; when the money is gone he gives up and raises
//    prices. Real chains only do ordinary business (prices, campaigns, openings): B\u0130M is always cheap, Migros is
//    dearer with better service, A101 opens on day 15 and pushes hard for a month, \u015eok arrives in summer 2011.
//  - A chain sometimes offers one of our good people a job; a content employee says no, an unhappy one (morale < 45) resigns.
namespace MarketCompetitors
{
    // G-079: the neighbourhood's traditional trade joins the rivals: the corner grocers (several small shops, close,
    // a little dearer, credit to regulars) and the weekly street market (Tuesdays, fresh goods only, cheap).
    enum class ECompany : uint8 { Bereket = 0, Bim, Migros, A101, Sok, Bakkal, Pazar, Count };

    struct FProfile
    {
        const TCHAR* Name = TEXT("");
        const TCHAR* Format = TEXT("");
        float BaseIndex = 1.f;
        float Service = 1.f;
        float Proximity = 1.f;
        int32 OpenDay = 1;
        int64 StartCash = 0;
        int32 NewsRival = INDEX_NONE;    // index in MarketRivals (its daily news), or none
        int32 StartStores = 1;           // shops serving our street at the start
        int32 Weekday = -1;              // open only on this weekday (0 = Monday), -1 = every day
        bool bFreshOnly = false;         // sells only fresh goods (dairy now; produce later)
    };

    // The name shown to the player: the real chain name, or its fictional stand-in when Config/zincirler.json says
    // "useFictional": true (karar A12).
    FString DisplayName(ECompany Company);
    // Whether the company sells this aisle at all (the street market sells only fresh goods).
    bool Sells(ECompany Company, const FString& Category);
    // Open on a given game day (opening day, market day).
    bool IsOpenOn(const FMarketState& State, ECompany Company, int32 GameDay);
    // A "rival.*" decision (MarketEvents::Decide): buying Bereket Market when it is for sale.
    bool Resolve(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& Decision, int32 Option, FString& OutMessage);
    constexpr int32 SaleAfterRedDays = 30;
    constexpr int64 BereketPrice2011 = 600000;   // 6.000 TL: goods, fittings, the name (x price level)

    constexpr float HomeAdvantage = 1.7f;    // the family shop is on the street where people live
    constexpr float PriceSensitivity = 0.12f;
    constexpr int32 WarPercent = 15;
    constexpr int32 WarDays = 5;

    const FProfile& Profile(ECompany Company);
    void Ensure(FMarketState& State);
    const FMarketCompetitor* Find(const FMarketState& State, ECompany Company);
    bool IsOpen(const FMarketState& State, ECompany Company);
    // A rival's price level for one aisle today (everyday level x its campaigns x price wars).
    float PriceIndex(const FMarketState& State, ECompany Company, const FString& Category, const TArray<FString>& Aisles);
    // What shoppers think the rivals charge for this aisle, relative to the list price (share-weighted).
    float RivalPriceFactor(const FMarketState& State, const FString& Category, const TArray<FString>& Aisles);
    // x shoppers: a larger local share brings more people through our door.
    float TrafficFactor(const FMarketState& State);

    // How attractive our shop was yesterday (price level, availability, service, loyalty, promotions).
    float OurAttraction(const FMarketState& State, const TArray<FMarketProduct>& Products);
    // Target share (0..1) of our shop from today's attractiveness of everybody.
    float TargetShare(const FMarketState& State, const TArray<FMarketProduct>& Products, const TArray<FString>& Aisles);

    // Day close (after FMarketState::CloseDay and the promotions): shares, the rivals' decisions, poaching.
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products, const TArray<FString>& Aisles);
    // One line for the menu: share, price level, mood.
    FString Describe(const FMarketState& State, ECompany Company);
    // MarketRivals news index -> company (for the menu's per-rival prices).
    float NewsRivalIndex(const FMarketState& State, int32 NewsRival);
}

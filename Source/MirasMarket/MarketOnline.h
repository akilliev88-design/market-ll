#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"
#include "MarketGoods.h"

// Online orders (G-069, Docs/Kurgu/00_KURGU_KITABI.md \u00a710, karar D11). Independent of the world, tested
// (MirasMarket.Online.*). Channels open with the era:
//  Phone     2011+  the bakkal tradition: regulars ring, a courier on a bicycle brings the bag. Small baskets,
//                   mostly pensioners and families; being served at home makes them more loyal.
//  Web       2014+  own web shop: set-up cost and hosting, card payment (commission), bigger baskets from the
//                   whole district, a slow start while people learn the site.
//  Platform  2016+  a fast-delivery platform ("Getirsin", fictional): its couriers, 18 % commission, many orders;
//                   its star rating follows how well we pick, and the stars decide how many orders come.
// The district's shopping moves online year by year (1 % in 2016, ~5 % in 2023). Those trips leave every shop's
// door, ours too; only a shop that is online wins some of them back as orders. So staying offline slowly costs
// walk-in shoppers, and going online costs couriers, commissions and picking work.
// Orders are picked at the day close from the depot first, then the shelf, so they share the shop's stock: an
// empty item is substituted by the chosen rule (ask, same aisle, leave out). More own-delivered orders than the
// couriers can carry arrive late or are cancelled. Picking tires the stocker on duty.
// The 2020-2021 profile (optional, on by default, karar D11): panic buying in early 2020, two waves of weekend
// curfews with short opening hours, a full closure in spring 2021, and online orders jump; no illness content.
// Dates and lengths differ in every campaign, so the period is a challenge, not a timetable.
namespace MarketOnline
{
    enum class EChannel : uint8 { Phone = 0, Web, Platform, Count };

    constexpr double PlatformCommission = 0.18;
    constexpr double WebCardCommission = 0.018;
    constexpr int64 WebSetupCost = 150000;         // start-level kurus x price list
    constexpr int64 WebMonthlyHosting = 5000;
    constexpr int64 CourierDailyWage = 1800;       // x wage index
    constexpr int64 PackagingPerOrder = 25;        // bags, receipt
    constexpr int32 OrdersPerCourier = 14;
    constexpr int32 OrdersWithoutCourier = 4;      // the owner delivers phone orders after closing
    constexpr int32 MaxCouriers = 4;

    FString ChannelName(EChannel Channel);
    int32 OpenDay(EChannel Channel);
    bool IsOn(const FMarketState& State, EChannel Channel);
    // Part of the district's grocery trips made online on this day (0..1).
    float DistrictOnlineShare(const FMarketState& State, int32 GameDay);
    // The 2020-2021 profile: its first and last day differ per campaign (early March 2020, late spring 2021).
    int32 PandemicStart(const FMarketState& State);
    int32 PandemicEnd(const FMarketState& State);
    bool IsPandemic(const FMarketState& State, int32 GameDay);
    bool IsCurfew(const FMarketState& State, int32 GameDay);
    // x walk-in shoppers: trips gone online, the 2020-2021 curfews and panic days.
    float StoreTrafficFactor(const FMarketState& State);
    // x how much a group is wanted (panic buying of staples and cleaning in March 2020).
    float GroupFactor(const FMarketState& State, MarketGoods::EGroup Group);
    // Platform stars 1..5 from the online reputation.
    float Stars(const FMarketState& State);
    // Own deliveries a day (phone + web).
    int32 DeliveryCapacity(const FMarketState& State);
    // Orders picked a day before they are late (staff on duty).
    int32 PickCapacity(const FMarketState& State);
    // Expected orders a day per channel (menu, advice), from yesterday's shoppers.
    float ExpectedOrders(const FMarketState& State, EChannel Channel);

    bool SetChannel(FMarketState& State, EChannel Channel, bool bOn, FString& OutMessage);
    bool HireCourier(FMarketState& State, FString& OutMessage);
    bool FireCourier(FMarketState& State, FString& OutMessage);
    bool SetSubstitute(FMarketState& State, int32 Rule, FString& OutMessage);
    bool SetFreeDelivery(FMarketState& State, bool bFree, FString& OutMessage);
    FString Summary(const FMarketState& State);

    // Call after FMarketState::CloseDay and before MarketStaff::CloseDay (tax books see the online sales).
    // Simulates the closed day's orders, books revenue and costs, moves stock and writes news.
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);
}

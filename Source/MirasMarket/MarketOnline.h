#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"
#include "MarketGoods.h"

// Online selling of the whole company (karar M32, Mustafa 01.10.2026; replaces G-069's phone orders, the family
// shop's couriers and the single dark store). Independent of the world, tested (MirasMarket.Online.*).
//
// When: the campaign's epidemic (MarketEras, seeded) is the anchor, not a calendar year. Web shops come about six
// years before it, the country's fast-delivery platform about four, ordering by phone app a year and a half before
// it, and our own 30-minute delivery half a year after it starts. Nothing is shown at the start: the assistant
// hears the first rumours a few months before, then the news arrive one by one.
// How much: the country's grocery bought online (ulkeler.json "online.plateau") grows slowly before the epidemic,
// jumps while people stay home, keeps moving online after it, overshoots a little while the shops' own trade
// grows back, and settles. Every shop of the company loses that part of its walk-ins; only what we sell online wins
// some of it back, so missing a technology makes the company stumble, never fall.
// Channels (company level, each with its condition):
//   Web       a site, from 2 shops: big baskets, slow start, card commission.
//   App       from the web site and 8 shops, built by a software house (a decision card: cheap / solid / premium);
//             the dearest orders to deliver but the biggest and most loyal baskets.
//   Platform  the country's platform (any shop, the family shop too): its couriers, its commission, its stars.
//   Quick     our own 30-minute delivery: the app and a dark store in the province (from 4 of our shops there).
// Where: every province with our shops is an area; it starts with the company's rule. A province manager does not
// flip channels: at a month's end he looks at the last months (three losing months of a channel, a channel the
// company has but the province misses, a dark store worth building) and sends a proposal up the line (the country
// manager, else the region's, else the sub-region's); nothing changes until we approve the card. Without a
// province manager the company's rule applies. The player can set an area himself and give it back. There is no courier to hire in a shop: own deliveries are paid per order (couriers of the company).
// Orders are picked from the shops' stock (the family shop's depot and shelf, a branch's shelves); an empty item is
// replaced by the chosen rule. More own orders than a shop can pick come late or are cancelled.
// Rivals: the country's chains go online in their own time (the assistant tells), the platform opens its own
// market later and asks for more commission now and then (decision cards). An e-commerce manager raises the stars,
// the picking and can keep the policy.
// The epidemic (a two-year profile, campaign years 10-11 before the era shift) is always part of the game now: panic buying, closure days with short hours, a
// full closure, orders jump. Dates differ in every campaign.
namespace MarketOnline
{
    enum class EChannel : uint8 { Web = 0, App, Platform, Quick, Count };
    constexpr int32 ChannelCount = static_cast<int32>(EChannel::Count);

    // Timeline against the epidemic's first day (days).
    constexpr int32 WebBefore = 6 * 365;
    constexpr int32 PlatformBefore = 4 * 365;
    constexpr int32 AppBefore = 548;
    constexpr int32 QuickAfter = 180;
    constexpr int32 PlatformMarketAfter = 540;
    constexpr int32 CommissionAfter = 900;
    constexpr int32 CommissionEvery = 730;
    constexpr int32 RumourDays = 90;

    // Conditions.
    constexpr int32 WebShops = 2;
    constexpr int32 AppShops = 8;
    constexpr int32 DarkStoreShops = 4;            // our shops in the province
    constexpr int32 ManagerShops = 10;

    // Money (start-level kurus x the price list; wages x the wage index).
    constexpr int64 WebSetupCost = 300000;
    constexpr int64 WebMonthly = 10000;
    const int64 AppCosts[3] = { 1500000, 3000000, 6000000 };
    const float AppQuality[3] = { 0.6f, 0.85f, 1.f };
    constexpr float AppUpkeepMonthly = 0.02f;      // of what the app cost
    constexpr int64 PlatformJoinCost = 30000;      // a tablet and the listing, per province
    constexpr int64 DarkStoreCost = 2000000;       // x the province's rent
    constexpr int64 DarkStoreMonthly = 60000;      // x the province's rent
    constexpr int64 CourierDailyWage = 1800;       // x wage index; a courier carries OrdersPerCourier a day
    constexpr int32 OrdersPerCourier = 14;
    constexpr float QuickCourierFactor = 1.3f;
    constexpr int64 PackagingPerOrder = 25;
    constexpr float CardCommission = 0.018f;
    constexpr float StartCommission = 0.18f;
    constexpr int32 DarkStorePicks = 250;          // orders a day a dark store picks

    FString ChannelName(EChannel Channel);
    // The day a channel can open in this campaign, and the day the platform opens its own market.
    int32 OpenDay(const FMarketState& State, EChannel Channel);
    int32 PlatformMarketDay(const FMarketState& State);
    // The menu shows online selling from the first rumour.
    bool Visible(const FMarketState& State);
    bool IsOn(const FMarketState& State, EChannel Channel);

    // Part of a country's grocery trips made online on a day (0..0.4). Country "" = the campaign's.
    float OnlineShare(const FMarketState& State, int32 GameDay, const FString& Country = FString());
    // The epidemic (always on; dates per campaign).
    int32 PandemicStart(const FMarketState& State);
    int32 PandemicEnd(const FMarketState& State);
    bool IsPandemic(const FMarketState& State, int32 GameDay);
    bool IsCurfew(const FMarketState& State, int32 GameDay);
    // x walk-in shoppers of every shop: trips gone online, closure days, panic days, the shops' trade growing back
    // after the epidemic.
    float StoreTrafficFactor(const FMarketState& State);
    float StoreTrafficFactorOn(const FMarketState& State, int32 GameDay, const FString& Country = FString());
    // x how much a group is wanted (panic buying of staples and cleaning).
    float GroupFactor(const FMarketState& State, MarketGoods::EGroup Group);
    // The same on a given day (E3b: MarketProductDemand::DayFactor, the branches' closed day).
    float GroupFactorOn(const FMarketState& State, int32 GameDay, MarketGoods::EGroup Group);
    // Stars 1..5 from the online reputation (the platform shows them; the app's follow its quality too).
    float Stars(const FMarketState& State);
    float AppStars(const FMarketState& State);

    // Our shops (family shop and open branches) in a province, and in the company.
    int32 ShopsIn(const FMarketState& State, const FString& Country, const FString& Province);
    int32 TotalShops(const FMarketState& State);

    bool CanOpen(const FMarketState& State, EChannel Channel, FString& OutReason);
    // Web, Platform, Quick open at once; the App asks for a software house first (a decision card).
    bool Open(FMarketState& State, EChannel Channel, FString& OutMessage);
    bool Close(FMarketState& State, EChannel Channel, FString& OutMessage);
    bool OpenApp(FMarketState& State, int32 Tier, FString& OutMessage);

    // Areas: provinces with our shops (kept by the day close). Flags: 1 platform, 2 own, 4 quick.
    int32 AreaFlags(const FMarketOnlineArea& Area);
    FString AreaName(const FMarketOnlineArea& Area);
    // Who decides there: the province manager's name, "sen" (the player) or "" (the company's rule).
    FString AreaDecider(const FMarketState& State, int32 AreaIndex);
    int32 EncodeArea(int32 AreaIndex, int32 Flags);
    bool DecodeArea(int32 Arg, int32& OutArea, int32& OutFlags);
    bool SetArea(FMarketState& State, int32 AreaIndex, int32 Flags, FString& OutMessage);
    bool ReturnArea(FMarketState& State, int32 AreaIndex, FString& OutMessage);
    bool SetDefault(FMarketState& State, int32 Flags, FString& OutMessage);
    int64 DarkStorePrice(const FMarketState& State, int32 AreaIndex);
    bool CanBuildDarkStore(const FMarketState& State, int32 AreaIndex, FString& OutReason);
    bool BuildDarkStore(FMarketState& State, int32 AreaIndex, FString& OutMessage);

    // Policy and the e-commerce manager.
    bool SetFee(FMarketState& State, int32 Level, FString& OutMessage);
    bool SetMinBasket(FMarketState& State, int32 Level, FString& OutMessage);
    bool SetPriceGap(FMarketState& State, int32 Level, FString& OutMessage);
    bool SetAds(FMarketState& State, int32 Level, FString& OutMessage);   // M34: the search channel of MarketAdvertising
    bool SetSubstitute(FMarketState& State, int32 Rule, FString& OutMessage);
    bool SetAutoPolicy(FMarketState& State, bool bAuto, FString& OutMessage);
    // This week's candidate (seeded): his name, skill and wage a day.
    void Candidate(const FMarketState& State, FString& OutName, int32& OutSkill, int64& OutWage);
    bool CanHireManager(const FMarketState& State, FString& OutReason);
    bool HireManager(FMarketState& State, FString& OutMessage);
    bool FireManager(FMarketState& State, FString& OutMessage);

    // Rivals online in a country on a day (0 none .. 3), and one line per chain online.
    float RivalOnline(const FMarketState& State, const FString& Country, int32 GameDay);
    TArray<FString> RivalLines(const FMarketState& State);

    // Menu lines.
    FString Summary(const FMarketState& State);
    FString ChannelLine(const FMarketState& State, EChannel Channel);       // status, condition or cost
    FString ChannelStats(const FMarketState& State, EChannel Channel);      // last month: orders, revenue, profit
    FString AreaLine(const FMarketState& State, int32 AreaIndex);
    FString AdsLine(const FMarketState& State);                             // spend, new customers, cost of one
    // The assistant's note (empty when nothing new happened in the last three weeks).
    FString Hint(const FMarketState& State);
    // Expected orders a day over the company (menu, advice).
    float ExpectedOrders(const FMarketState& State);

    // An "online.*" decision (called by MarketEvents::Decide).
    bool Resolve(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& Decision, int32 Option, FString& OutMessage);

    // Day close (after the branches and the payments, before the bills): areas and their deciders, the timeline's
    // news and cards, rivals going online, the orders of every shop, the costs, the books.
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);
    // C11: the head office's fixed online costs of a day (web hosting, app upkeep, dark stores, the e-commerce manager).
    int64 DailyFixedCost(const FMarketState& State, int32 GameDay);
}

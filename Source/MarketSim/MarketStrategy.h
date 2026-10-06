#pragma once

#include "CoreMinimal.h"

struct FMarketState;
struct FMarketProduct;
struct FMarketBranch;
struct FMarketDecision;

// C16 (Mustafa 03.10.2026: "oyun bir yerden sonra sadece \u015fube a\u00e7ma hissi veriyor"); D9 (04.10.2026): moved onto the one
// store model: the strategy's draw is part of every store's pull (MarketStoreDemand::Pull), the first store's too.
// The middle game's own choices.
// Independent of the world, tested (MarketSim.Strategy.*).
//
//  M48 Province markets. Our shops in a province against each rival chain's. More shops than every single rival
//  there (and at least two) = the province's champion. A province push (60 days of local campaigns, a daily cost
//  per shop there) pulls more shoppers to our branches there; a rival may answer with a price war; at the end
//  small rivals there may give up a shop. A province rests 180 days after a push.
//  M49 Strategic forks. At 5, 25 and 100 open branches the company chooses a lasting direction (a decision with a
//  30-day deadline; "not now" asks again in 180 days):
//    focus     - the home region's stronghold (+6 % shoppers in the home's main region) / the big cities (+6 %
//                shoppers where a million or more live) / the small towns (+5 % shoppers and new leases x0.85 where
//                under half a million live). (The shop's identity is the story's, M38; this is where the chain goes.)
//    growth    - fast roll-out (fit-out x0.75, management follows 25 % more, service -2 %) / own buildings (rent
//                of new leases x0.55, fit-out x1.6) / short leases (rent x1.05, a hasty site half as likely);
//    vertical  - own production (3 % of a year's revenue, ready in a year: shelf prices +2 % on private label) /
//                logistics (1 % of a year's revenue: depot losses halved, availability) / a loyalty programme
//                (no outlay, 0.4 % of the branches' revenue a month, +5 % shoppers).
//  M50 Paths. Four ways to be good besides size, each with three tiers, measured monthly, with a small lasting perk:
//    champion   - champion provinces 1 / 5 / 15: +2 % shoppers a tier in those provinces;
//    efficient  - a year's operating margin 4 / 7 / 10 %: banks lend 0.25 points cheaper a tier;
//    people     - five or more store managers with average skill 60 / 68 / 76: management follows 2 more leases
//                 a year a tier (M45);
//    favourite  - three or more branches with average satisfaction 62 / 72 / 82: +1.5 % shoppers a tier.
//  A path can drop a tier again; reaching a new best tier is celebrated.
namespace MarketStrategy
{
    enum class EFocus : uint8 { None = 0, Home, BigCities, SmallTowns };
    enum class EGrowth : uint8 { None = 0, Fast, Owned, Flexible };
    enum class EVertical : uint8 { None = 0, Production, Logistics, Loyalty };
    enum class EPath : uint8 { Champion = 0, Efficient, People, Favourite, Count };

    constexpr int32 FocusAt = 5;
    constexpr int32 BigCityK = 1000;
    constexpr int32 SmallTownK = 500;
    constexpr int32 GrowthAt = 25;
    constexpr int32 VerticalAt = 100;
    constexpr int32 DecisionDays = 30;
    constexpr int32 AskAgainDays = 180;
    constexpr int32 ProductionDays = 365;

    constexpr int32 PushDays = 60;
    constexpr int32 PushRestDays = 180;
    constexpr int64 PushDailyPerShopStart = 10000;  // 100 TL a day for each of our shops there (start-level kurus)
    constexpr float PushPull = 1.12f;
    constexpr int32 PushMinShops = 2;

    // --- M48 province markets
    int32 RivalStoresIn(const FMarketState& State, const FString& Country, const FString& Province, FString* OutLeader = nullptr);
    bool IsChampion(const FMarketState& State, const FString& Country, const FString& Province);
    int32 ChampionCount(const FMarketState& State);
    int64 PushDailyCost(const FMarketState& State, const FString& Country, const FString& Province);
    bool PushActive(const FMarketState& State, const FString& Country, const FString& Province, int32 Day);
    bool CanPush(const FMarketState& State, const FString& Country, const FString& Province, FString& OutReason);
    bool StartPush(FMarketState& State, const FString& Country, const FString& Province, FString& OutMessage);
    // Our provinces with the most shops first (the menu's list): "country|province".
    TArray<FString> OurProvinces(const FMarketState& State, int32 MaxCount);
    FString ProvinceLine(const FMarketState& State, const FString& Country, const FString& Province);

    // --- M49 strategic forks
    EFocus Focus(const FMarketState& State);
    EGrowth Growth(const FMarketState& State);
    EVertical Vertical(const FMarketState& State);
    bool ProductionReady(const FMarketState& State);
    bool Resolve(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& Decision, int32 Option, FString& OutMessage);
    FString StrategyLine(const FMarketState& State);   // "Odak: ... \u00b7 B\u00fcy\u00fcme: ... \u00b7 Dikey: ..." ("" before the first)

    // --- M50 paths
    int32 PathTier(const FMarketState& State, EPath Path);        // 0..3 at the last monthly look
    int32 MeasurePathTier(const FMarketState& State, EPath Path); // 0..3 now
    FString PathLine(const FMarketState& State, EPath Path);      // name, tier, the measure, the next step, the perk

    // --- factors the branch day and other systems ask (1 / 0 when nothing applies)
    float ServiceFactor(const FMarketState& State);
    float PullFactor(const FMarketState& State, const FString& Country, const FString& Province, int32 Day);
    float ShelfPriceFactor(const FMarketState& State);            // own production's private label: goods cost / this
    float DepotLossFactor(const FMarketState& State);
    float FitOutFactor(const FMarketState& State);
    float RentFactor(const FMarketState& State, const FString& Country, const FString& Province);
    float HastyFactor(const FMarketState& State);
    float CapacityFactor(const FMarketState& State);
    int32 CapacityBonus(const FMarketState& State);               // leases a year (the people path)
    double RateDiscount(const FMarketState& State);               // off a bank's yearly rate (the efficient path)

    // Day close (MarketDirector, after MarketBranches): offers the forks, runs and ends pushes, pays the loyalty
    // programme, measures the paths once a month.
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);
}

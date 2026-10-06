#pragma once

#include "CoreMinimal.h"

struct FMarketState;
struct FMarketProduct;
struct FMarketDecision;

// D9b (M47, Mustafa 04.10.2026: "kararlar bize son onay olarak gelebilir; belli etkiye kadar m\u00fcd\u00fcrler kendi
// hiyerar\u015filerinde ayarlar; o de\u011fi\u015fiklikleri il baz\u0131nda al\u0131nan kararlar ve m\u00fcdahale olarak ba\u015fka bir men\u00fcde g\u00f6rebiliriz").
// Answers to rivals' moves and crises. Independent of the world, tested (MarketSim.Response.*).
//  - Moves: a chain starts a price war against us in a province (MarketChains), rival chains open stores in a
//    province where we are (looked at weekly: at least 5 % more of their stores, at least one), a crisis starts in
//    the campaign's economy (MarketEras: currency shock, recession, the epidemic, high inflation).
//  - Who answers: in a province its manager, else the sub-region's, the region's, the country's; with none of them
//    the manager of our biggest store there; for a crisis the general manager, the home continent's director, the
//    home country's manager. Each answers by his style (careful waits, generous spends on service, price-minded
//    cuts prices). Nobody to ask: the player decides.
//  - The player's last word: a move in a province that brings 10 % or more of our stores' revenue, and every
//    crisis, comes as a card with the manager's proposal as the default (five days). Anything smaller the managers
//    settle; every answer goes into the log (Magazalar > Mudahaleler). A full desk (another answer waiting) leaves
//    it to the managers, or to the player's standing order (wait) when there are none.
//  - The answers (each until the war ends, 30 days for an opening, the crisis's length at most a year):
//      war      - match the price (our shelves -4 %), service and campaigns (+6 % pull, 80 TL a shop a day), hold;
//                 answering with price or service makes the rival's war count as lost when it ends;
//      opening  - an opening-week campaign (+5 % pull, 60 TL a shop a day), a small price cut (-2 %), hold;
//      crisis   - tighten the belt (running costs -30 %, orders -10 %, -2 % pull), carry on, look for chances
//                 (new leases -15 %, fit-outs -10 %, shelves -2 % everywhere).
namespace MarketResponse
{
    enum class EKind : uint8 { War = 0, Opening, Crisis, Count };

    constexpr int32 LogSize = 80;
    constexpr int32 DecisionDays = 5;
    constexpr float BigShare = 0.10f;
    constexpr int32 LookEvery = 7;
    constexpr float OpeningGrowth = 0.05f;
    constexpr int32 OpeningDays = 30;
    constexpr int32 CrisisMaxDays = 365;
    constexpr float WarPrice = 0.96f;
    constexpr float WarPull = 1.06f;
    constexpr int64 WarDailyPerShopStart = 8000;
    constexpr float OpeningPull = 1.05f;
    constexpr float OpeningPrice = 0.98f;
    constexpr int64 OpeningDailyPerShopStart = 6000;
    constexpr float CutRunning = 0.7f;
    constexpr float CutOrder = 0.9f;
    constexpr float CutPull = 0.98f;
    constexpr float ChanceLease = 0.85f;
    constexpr float ChanceFitOut = 0.9f;
    constexpr float ChancePrice = 0.98f;

    FString KindName(EKind Kind);
    // The three answers of a kind (short names) and what each does, in the active country's money.
    TArray<FString> Options(const FMarketState& State, EKind Kind);
    FString OptionName(EKind Kind, int32 Option);
    // The player's standing order when nobody can answer for him: hold / carry on.
    int32 HoldOption(EKind Kind);

    // Who answers a move in a province (Province empty: a crisis, company-wide).
    struct FDecider
    {
        bool bPlayer = true;
        FString Name;
        FString Title;                   // "Tekirda\u011f il m\u00fcd\u00fcr\u00fc"
        uint8 Style = 0;                 // MarketManagers::EStyle
        int32 Skill = 0;
    };
    FDecider DeciderFor(const FMarketState& State, const FString& Country, const FString& Province);
    int32 Proposal(EKind Kind, const FDecider& Decider);
    // The province's share of our stores' last 30 days' revenue (branches; 0..1).
    float Impact(const FMarketState& State, const FString& Country, const FString& Province);

    // A move happened: the managers answer it or the player gets the card. False when it was not raised (nothing
    // of ours there).
    bool Raise(FMarketState& State, EKind Kind, const FString& Country, const FString& Province, const FString& Rival, int32 EndDay);
    // Books an answer (also the card's choice): its effects start today.
    void Answer(FMarketState& State, EKind Kind, const FString& Country, const FString& Province, const FString& Rival, int32 Option,
        const FString& Decider, bool bPlayer, bool bProposed, int32 EndDay);
    // A "response.*" decision (MarketEvents::Decide).
    bool Resolve(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& Decision, int32 Option, FString& OutMessage);

    // --- effects (1 / false when nothing applies). Country empty = the campaign's.
    float PullFactor(const FMarketState& State, const FString& Country, const FString& Province, int32 Day);
    float PriceFactor(const FMarketState& State, const FString& Country, const FString& Province, int32 Day);
    float RunningFactor(const FMarketState& State, int32 Day);
    float OrderFactor(const FMarketState& State, int32 Day);
    float LeaseFactor(const FMarketState& State);
    float FitOutFactor(const FMarketState& State);
    // We answered the war that ends on WarUntil in that province with price or service (MarketChains' war end).
    bool Fought(const FMarketState& State, const FString& Country, const FString& Province, int32 WarUntil);

    // --- the log (Magazalar > Mudahaleler)
    int32 LogCount(const FMarketState& State);
    // The answer at Index from the newest (0 = newest): "12. g\u00fcn \u00b7 Tekirda\u011f \u00b7 X fiyat sava\u015f\u0131 \u2192 hizmet ...".
    FString LogLine(const FMarketState& State, int32 Index);
    bool LogActive(const FMarketState& State, int32 Index);
    FString Summary(const FMarketState& State);   // counts, the active answers, what they cost

    // Day close (MarketDirector, after the chains and the strategy): pays the running answers, looks for new moves.
    void CloseDay(FMarketState& State);
}

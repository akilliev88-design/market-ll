#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"
#include "MarketGoods.h"

// Choices, lasting effects and neighbourhood events (G-066, Docs/Kurgu/00_KURGU_KITABI.md \u00a79). Independent of the
// world, tested (MirasMarket.Events.*).
//  - A decision waits for the player (menu, day report); when its deadline passes, the default option is taken.
//    Story scenes (MarketStory) use the same decisions with ids "story.*".
//  - A modifier is a lasting effect with a day range: shopper traffic, interest in a demand group, price tolerance
//    (the shop's identity), unit cost. The game asks the factors through MarketDirector.
//  - Events: at most one a day, about one day in five after day 3, never the same one within two weeks, each with
//    its own conditions (season, stock, accountant). Some are news, most ask for a decision:
//      fridge breakdown, power cut, municipal inspection, a wedding's bulk order, a spoiled-milk complaint,
//      roadworks, derby night, a condolence house, the wholesaler's broken truck; snow delays deliveries.
namespace MarketEvents
{
    enum class EModifier : uint8 { Traffic = 0, Interest, PriceTolerance, CostFactor, Count };
    constexpr uint8 AllGroups = 255;
    constexpr int32 EventChancePercent = 22;
    constexpr int32 CooldownDays = 14;
    constexpr int32 MaxPending = 2;

    void AddModifier(FMarketState& State, EModifier Kind, uint8 Group, float Value, int32 FirstDay, int32 LastDay, const FString& Source);
    // Product of the active multiplicative modifiers of a kind for a group (Traffic ignores the group).
    float Factor(const FMarketState& State, EModifier Kind, MarketGoods::EGroup Group = MarketGoods::EGroup::Other);
    // Sum of the active price tolerance modifiers for a group (added to the shopper's tolerance).
    double Tolerance(const FMarketState& State, MarketGoods::EGroup Group);

    void Offer(FMarketState& State, const FMarketDecision& Decision);
    const FMarketDecision* Pending(const FMarketState& State);
    // Resolves the first waiting decision with Option. "story.*" go to MarketStory, "finance.*" to MarketFinance.
    bool Decide(FMarketState& State, const TArray<FMarketProduct>& Products, int32 Option, FString& OutMessage);
    bool Happened(const FMarketState& State, const FString& Id, int32 WithinDays);
    void Log(FMarketState& State, const FString& Id);

    // Day close: expired modifiers go, decisions past their deadline take the default, snow delays the
    // delivery, and maybe a new event comes (news in State.DayNews).
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);
    // Starts a specific event now (tests, story). False when its conditions are not met.
    bool Trigger(FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Id);
}

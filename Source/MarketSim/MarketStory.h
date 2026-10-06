#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"
#include "MarketGoods.h"

// The company's own story (M69). There is one written story, the start (MarketStart::IntroText); everything after is
// the player's game: milestones become memories, and once, in the first weeks, the market chooses its identity
// (it carries to every store). No chapters, no chapter goals, no finale: the game goes on as long as the company
// exists. Independent of the world, tested (MarketSim.Story.*).
//  Mahalle Marketi      loyal regulars forgive a little more; staples and milk sell a bit better
//  Kaliteli Market      shoppers accept higher prices; better goods cost 4 % more
//  H\u0131zl\u0131 \u0130ndirim        4 % cheaper purchases and a little more traffic, but price-hunting shoppers
namespace MarketStory
{
    enum class EIdentity : uint8 { None = 0, Bakkal, Kaliteli, Indirim };
    // The identity is offered once, on this day of the campaign (or the first close after it).
    constexpr int32 IdentityDay = 10;

    FString IdentityName(EIdentity Identity);
    // M38 (Mustafa 02.10.2026: the identity is the company's, not a corner shop's): what the chosen identity does
    // in every branch (the first store takes it from the modifiers): x wish of a group.
    float IdentityDemand(const FMarketState& State, MarketGoods::EGroup Group);
    void AddMemory(FMarketState& State, const FString& Text);

    // Day close: milestones and the identity choice (news in State.DayNews).
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);
    // True once the player sold the company and ended the campaign (karar J03): nothing new is offered.
    bool StoryClosed(const FMarketState& State);
    // A "story.*" decision (called by MarketEvents::Decide).
    bool Resolve(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& Decision, int32 Option, FString& OutMessage);
}

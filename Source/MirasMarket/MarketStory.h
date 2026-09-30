#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// The main story (G-066, Docs/Kurgu/00_KURGU_KITABI.md \u00a74-5). Independent of the world, tested
// (MirasMarket.Story.*). Chapters have goals measured from the game state; scenes with the characters play at day
// closes; choices go through MarketEvents decisions ("story.*"); milestones become memories.
//  1 Defter            first order, first profitable day, first week
//  2 Kar\u015f\u0131 D\u00fckk\u00e2n      father's debt closed, 35 % local share, 15 regulars, sell-or-continue answered
//  3 \u0130kinci Tabela     second shop, an HR manager, an accountant (the shop runs without you)
//  4-7                 \u0130ller, \u00dclke \u00c7ap\u0131nda, S\u0131n\u0131r \u00d6tesi, Miras: goals of the branch/company systems (G-086)
// Scenes: Nermin teyze's welcome (day 1) and her complaint when there is no milk; Cem, the father's apprentice,
// asks for a job (day 2 pool); Selim's visit (day 3); Kadir Bereketo\u011flu's taunt (day 5); Kadir's offer to buy the
// shop (from day 10 or when the debt closes). Continuing asks for the shop's identity:
//  Mahallenin Bakkal\u0131   loyal regulars forgive a little more; staples and milk sell a bit better
//  Kaliteli Yerel       shoppers accept higher prices; better goods cost 4 % more
//  H\u0131zl\u0131 \u0130ndirim        4 % cheaper purchases and a little more traffic, but price-hunting shoppers; Bereket is furious
namespace MarketStory
{
    enum class EIdentity : uint8 { None = 0, Bakkal, Kaliteli, Indirim };
    enum class EEnding : uint8 { None = 0, Sold, Legacy, TimeUp };
    // Karar J02: the campaign's last day (31.12.2040). If the Legacy finale has not come by then, the time-up
    // finale does. Either way it is shown once and the game goes on without new story content.
    constexpr int32 FinalYear = 2040;
    // B1 (#45): chapter 5 goal, percent of the country's grocery retail by revenue (karar bekliyor: Mustafa).
    constexpr float NationalShareGoal = 0.1f;

    struct FObjective
    {
        FString Text;
        bool bDone = false;
        bool bLater = false;                  // needs a system of a later update
    };

    FString ChapterTitle(int32 Chapter);
    TArray<FObjective> Objectives(const FMarketState& State);
    FString IdentityName(EIdentity Identity);
    void AddMemory(FMarketState& State, const FString& Text);
    // What Kadir Bey offers for the business today (not the building: that stays in the family).
    int64 SaleOffer(const FMarketState& State, const TArray<FMarketProduct>& Products);

    // Day close: scenes, milestones and the next chapter (news in State.DayNews). Call after the staff's day close
    // (Cem joins the fresh hiring pool).
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);
    // The one finale (karar J02): remember it, show the summary once, then free play without new content.
    // Returns false if a finale was already reached.
    bool ReachFinale(FMarketState& State, EEnding Ending);
    // True once the finale was shown or the campaign ended: no new chapters, scenes or story decisions.
    bool StoryClosed(const FMarketState& State);
    // A "story.*" decision (called by MarketEvents::Decide).
    bool Resolve(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& Decision, int32 Option, FString& OutMessage);
}

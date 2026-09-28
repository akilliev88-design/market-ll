#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// The credit book, "veresiye defteri" (G-067, Docs/Kurgu/00_KURGU_KITABI.md \u00a76). Independent of the world, tested
// (MirasMarket.Credit.*). The player sets a limit per neighbour (0 = no credit). A known customer (three visits,
// content) sometimes asks to write the basket in the book: the sale counts today, the money comes later. On paydays
// (1st and 15th) most accounts are paid; some pay half; a page left unpaid for 45 days may be lost for good.
// Credit given makes the customer loyal; a refusal because of the limit hurts. The "Mahallenin Bakkal\u0131" identity
// makes neighbours ask more often.
namespace MarketCredit
{
    constexpr int32 MinVisits = 3;
    constexpr float MinSatisfaction = 50.f;
    constexpr float AskChance = 0.15f;         // a known customer at the till
    constexpr float BakkalAskChance = 0.25f;
    constexpr int32 LostAfterDays = 45;
    const int64 Limits[4] = { 0, 2000, 5000, 10000 };

    int64 Outstanding(const FMarketState& State);
    const FMarketCreditAccount* Find(const FMarketState& State, int32 CustomerId);
    // After a paid basket: may turn the receipt into credit. Roll 0..1. Returns a Turkish note ("" = paid cash).
    FString OnCheckout(FMarketState& State, int32 CustomerId, int64 Receipt, float Roll);
    // Limit step 0..3 (0, 20, 50, 100 TL at 2011 prices).
    bool SetLimit(FMarketState& State, int32 Step, FString& OutMessage);
    // Ask everybody to pay now: some pay, everybody is a little offended.
    int64 CollectAll(FMarketState& State, FString& OutMessage);

    // Day close: paydays, late pages, lost debts.
    void CloseDay(FMarketState& State);
}

#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"
#include "MarketCustomers.h"

// How shoppers pay (G-069, Docs/Kurgu/00_KURGU_KITABI.md \u00a76, karar C08). Independent of the world, tested
// (MirasMarket.Payments.*).
//  - Every basket is paid in cash, by card or with a meal card. How many want to pay by card follows the year
//    (a quarter of baskets in 2011, most of them after the contactless years) and the kind of shopper
//    (pensioners pay cash, office workers and students use cards; children always pay cash).
//  - Without a POS terminal a card shopper either pays cash grudgingly or leaves the basket (35 %).
//  - A POS costs a monthly rent and 1.8 % commission; the money arrives at the next day close. With cards
//    accepted, card shoppers put a little more in the basket.
//  - Meal cards (office workers, some tradesmen): 6 % commission and a monthly fee, but lunch-time workers
//    come to a shop that takes them.
namespace MarketPayments
{
    enum class EMethod : uint8 { Cash = 0, Card, MealCard, NoCard };

    constexpr double CardCommission = 0.018;
    constexpr double MealCommission = 0.06;
    constexpr float NoCardLeaveChance = 0.35f;
    constexpr int64 PosMonthlyRent = 2500;     // 2011 kurus, follows the price list
    constexpr int64 MealMonthlyFee = 1500;

    // Share of baskets whose owner would rather pay by card in this game day's year (0..1).
    float CardShare(int32 GameDay);
    float SegmentCardFactor(MarketCustomers::ESegment Segment);
    float MealCardShare(MarketCustomers::ESegment Segment);
    // Method of this basket. Roll 0..1. NoCard = wanted a card, the shop has no POS (the game then asks
    // LeavesWithoutCard with a second roll).
    EMethod Choose(const FMarketState& State, MarketCustomers::ESegment Segment, float Roll);
    bool LeavesWithoutCard(FMarketState& State, float Roll);
    // After FMarketState::SellBasket: card money goes to tomorrow's bank transfer, minus the commission.
    // NoCard means the shopper paid cash after all. Returns a short note ("" for cash).
    FString Settle(FMarketState& State, EMethod Method, int64 Receipt);
    // x visit budget: card shoppers buy a little more where cards are taken.
    float BudgetFactor(const FMarketState& State, MarketCustomers::ESegment Segment);
    // x shoppers: office workers come where meal cards are taken.
    float TrafficFactor(const FMarketState& State);

    bool SetCard(FMarketState& State, bool bOn, FString& OutMessage);
    bool SetMealCard(FMarketState& State, bool bOn, FString& OutMessage);
    FString Summary(const FMarketState& State);

    // Call after FMarketState::CloseDay: yesterday's card money arrives, commissions and fees are the day's costs.
    void CloseDay(FMarketState& State);
}

#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// The one place where the world-independent systems meet the game (Docs/Kurgu/00_KURGU_KITABI.md \u00a713).
// The game mode asks the director for today's factors and calls CloseDay once per day close; the director calls
// every system in a fixed order. A new system is added here, not in MarketGame.cpp.
// The campaign seed for weather, news and people is State.RivalSeed (set once per new campaign).
namespace MarketDirector
{
    // Shopper traffic today: calendar (weekday, weather, paydays, bayrams) x rival news.
    float TrafficFactor(const FMarketState& State, const TArray<FString>& Aisles);
    // What shoppers think the rivals charge for an aisle today, relative to the list price (MarketCompetitors).
    float RivalPriceFactor(const FMarketState& State, const TArray<FString>& Aisles, const FString& Category);
    // Extra price tolerance of shoppers for a product today (the shop's identity, events), added to the segment's.
    double ToleranceBonus(const FMarketState& State, const FMarketProduct& Product);
    // How much a product is wanted today relative to an ordinary day (calendar season, weather, special days).
    float DemandWeight(const FMarketState& State, const FMarketProduct& Product);
    // For the order suggestion: expected demand on the day an order placed now is on the shelf (the next day),
    // relative to yesterday. Indexed like Products.
    TArray<float> OrderScales(const FMarketState& State, const TArray<FMarketProduct>& Products);
    float OrderScale(const FMarketState& State, const FMarketProduct& Product);
    // "7 Mart 2011 Pazartesi" for the running day.
    FString DateText(const FMarketState& State);
    // Day line for the HUD/menu: date, weather, special days.
    FString TodayText(const FMarketState& State);
    // Evening report: tomorrow's calendar forecast (State.Day is already tomorrow after a day close).
    FString TomorrowText(const FMarketState& State);

    // Today's costs and list prices (monthly price list, wholesaler discount) from the 2011 catalog values.
    // Call after loading/starting a campaign and after every day close.
    void ApplyPrices(const FMarketState& State, const TArray<FMarketProduct>& CatalogBase, TArray<FMarketProduct>& Products);
    // How this shopper wants to pay (MarketPayments::EMethod as uint8), before the basket is sold. Roll 0..1.
    uint8 PaymentMethod(const FMarketState& State, uint8 Segment, float Roll);
    // The shopper wanted to pay by card and the shop has no POS: true = leaves the basket. Roll 0..1.
    bool LeavesWithoutCard(FMarketState& State, float Roll);
    // x visit budget of a shopper (card shoppers spend a little more where cards are taken).
    float BudgetFactor(const FMarketState& State, uint8 Segment);
    // After a basket was paid at the till: a known neighbour may write it in the credit book; otherwise the
    // payment method settles (card money arrives tomorrow). Returns a note.
    FString OnCheckout(FMarketState& State, int32 CustomerId, int64 Receipt, float Roll, uint8 Method = 0);
    // After a successful FMarketState::SubmitOrder: wholesaler volume and payment terms. Returns an extra line.
    FString OnOrder(FMarketState& State, int64 Bill);
    // Management decisions of the background systems that are not staff decisions. False + message when nothing
    // changed. Actions: Supplier (Arg = MarketSuppliers::ESupplier), PayBills, PassOnPriceRise,
    // Discount10 / Discount20 / MultiBuy / Endcap (Arg = product), Flyer, StopPromotion (Arg = index), AcceptOffer, DeclineOffer,
    // Decide (Arg = option of the first waiting decision: story scenes and events),
    // FreshPolicy (Arg 0..2), CreditLimit (Arg step 0..3), CollectCredit, TakeLoan (Arg step 0..2), RepayLoan,
    // OpenBranch (Arg = district * 10 + format 0 k\u00fc\u00e7\u00fck / 1 mahalle / 2 b\u00fcy\u00fck), CloseBranch (Arg = index),
    // Promote (Arg = employee id; runs the newest open branch),
    // OnlineChannel (Arg = channel * 10 + 1 on / 0 off; 0 phone, 1 web, 2 platform), HireCourier, FireCourier,
    // Substitute (Arg 0 ask / 1 same aisle / 2 leave out), FreeDelivery (Arg 1/0), Card (Arg 1/0), MealCard (Arg 1/0),
    // PandemicProfile (Arg 1/0), Difficulty (Arg 0 easy / 1 normal / 2 hard).
    bool Command(FMarketState& State, const TArray<FMarketProduct>& Products, FName Action, int32 Arg, FString& OutMessage);
    // Evening report of the background systems: wholesalers, staff, tax, ...
    FString ReportText(const FMarketState& State);

    // Call right after FMarketState::CloseDay (State.Day is already the next day), before MarketCampaign::CloseDay.
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);
}

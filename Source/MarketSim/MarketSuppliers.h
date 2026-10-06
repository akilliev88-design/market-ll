#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Wholesalers, payment terms and price lists (G-063, Docs/Kurgu/00_KURGU_KITABI.md \u00a78). Independent of the world,
// tested (MarketSim.Suppliers.*).
//  - Prices: the catalog's cost and list price are start-level values (M24); the game uses them x MarketPrices::ListLevel, which
//    moves on the 1st of every month (the wholesaler's "zam listesi"). The rival's shelf price follows the same
//    list, so a player who does not pass the rise on sells cheaper than the rivals but earns less.
//  - The shop's old wholesaler (named from the country, MarketCast::Wholesaler; its man MarketCast::Salesman). Cash at first; trust grows with orders and
//    payments on time and brings 7, then 14 days of payment terms; monthly volume brings a 3-5 % discount.
//  - A cash-and-carry (MarketCast::CashCarry): from day 14, 4 % cheaper, but three times the missing/broken goods and no terms.
//  - A late payment costs trust, closes the terms and adds a 2 % late fee.
namespace MarketSuppliers
{
    enum class ESupplier : uint8 { Regular = 0, CashCarry = 1, Count };

    struct FInfo
    {
        FString Name;                // M30: from the country pack (MarketCast)
        FString Contact;
        float BaseDiscount = 0.f;
        uint32 DeliveryRisk = 1;     // x missing/damaged odds of the v0.1 delivery (FMarketState::CloseDay)
        int32 UnlockDay = 1;
        bool bOffersTerms = false;
    };

    constexpr int32 TermsTrust = 60;        // 7 days of payment terms
    constexpr int32 LongTermsTrust = 80;    // 14 days
    constexpr int64 VolumeTier1 = 150000;   // 1.500 TL in about 30 days: 3 %
    constexpr int64 VolumeTier2 = 400000;   // 4.000 TL: 5 %
    constexpr float LateFee = 0.02f;
    constexpr int32 LateFeeDays = 5;
    // C9 (Codex C8: with no cash the shop could not order, empty shelves sold nothing, the fall fed itself): in a cash
    // crisis the shop's old wholesaler still gives about three days of goods on three days' terms, once a week, while
    // no bill is more than two weeks late ("d\u00fckk\u00e2n\u0131n hat\u0131r\u0131na").
    constexpr int32 LifelineGoodsDays = 3;
    constexpr int32 LifelineTerms = 3;
    constexpr int32 LifelineEvery = 7;
    constexpr int32 LifelineMaxLate = 14;
    // M64 (Mustafa 03.10.2026): the shop's old standing opens the door three times in a campaign, never shown as a
    // count; the third time the salesman says it is the last. Leaving the old wholesaler for the cash-and-carry ends it.
    constexpr int32 LifelineMax = 3;
    int64 LifelineAllowance(const FMarketState& State);   // #41: the late fee runs five days at most (about 10 %), then only the trust suffers

    FInfo Info(ESupplier Supplier);
    ESupplier Current(const FMarketState& State);
    const FMarketSupplierAccount* FindAccount(const FMarketState& State, ESupplier Supplier);
    FMarketSupplierAccount& Account(FMarketState& State, ESupplier Supplier);
    bool Available(const FMarketState& State, ESupplier Supplier);
    // Total discount off the list for the current relationship (base + volume).
    float Discount(const FMarketState& State, ESupplier Supplier);
    // Days of payment terms the supplier gives today (0 = cash).
    int32 TermsDays(const FMarketState& State, ESupplier Supplier);

    // What the shop pays for one unit today and the market's reference retail price (rivals price around it).
    // Index (catalog order) lets a wholesaler-funded promotion lower one product's cost (MarketPromotions).
    // The wholesaler's price over the catalog cost (karar G10: a tighter margin than the catalog, ~25 % instead of
    // ~34 %, so the early game is a climb; volume discounts and terms matter).
    constexpr double WholesaleFactor = 1.10;
    // This month's price list of the wholesaler against the country's level: -2 %..+2 %, different in every
    // campaign and month (the first month is exact), so no month's rise can be read off a table.
    double MonthSwing(const FMarketState& State);
    int64 UnitCost(const FMarketState& State, const FMarketProduct& Base, int32 Index = INDEX_NONE);
    int64 ListPrice(const FMarketState& State, const FMarketProduct& Base);
    // Writes today's costs and list prices of the catalog (Base, start-level values) into the game's products.
    void ApplyPrices(const FMarketState& State, const TArray<FMarketProduct>& Base, TArray<FMarketProduct>& Out);

    // After FMarketState::SubmitOrder succeeded with Bill: counts the volume and, with terms, gives the cash back
    // and writes the bill to be paid later. Returns a Turkish line for the player ("" = nothing to add).
    FString OnOrder(FMarketState& State, int64 Bill);
    // E3c2b (M63): a branch's order from the current wholesaler, on the first store's account (the same volume,
    // terms, bills, late fees and trust). With terms the cash the order took goes back and a bill waits; without
    // terms it was paid in cash. No lifeline: that is the old wholesaler's favour to the first store.
    // Returns the amount bought on terms (0 = paid in cash).
    int64 OnBranchOrder(FMarketState& State, int64 Bill);
    // G-077 (#33): how much more than the cash in the till the current wholesaler lets us order on terms today:
    // 500 TL (at today's list level) + half of the last 30 days' purchases, minus the open bills. 0 without terms.
    int64 OrderAllowance(const FMarketState& State);
    bool Switch(FMarketState& State, ESupplier Supplier, FString& OutMessage);
    // Pays every open bill now if the cash allows; returns the amount paid.
    int64 PayBills(FMarketState& State);
    int64 OpenBills(const FMarketState& State);

    // How far the shelf prices are behind the list (0.05 = the list is 5 % above the last price update).
    double PriceGap(const FMarketState& State);
    // Raises every shelf price by the gap (rounded to 5 kurus), within the allowed price range. Returns the number
    // of prices changed. Products are the game's current (today's) products.
    int32 PassOnPriceRise(FMarketState& State, const TArray<FMarketProduct>& Products);

    // One line for the order page: who, trust, terms, discount, open bills, price gap.
    FString Summary(const FMarketState& State);

    // Day close (after FMarketState::CloseDay): volume decay, trust, bills due, next month's price list and the
    // wholesalers' news in State.DayNews.
    void CloseDay(FMarketState& State);
}

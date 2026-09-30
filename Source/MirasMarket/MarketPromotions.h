#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// The player's promotions (G-064, Docs/Kurgu/00_KURGU_KITABI.md \u00a79). Independent of the world, tested
// (MirasMarket.Promotions.*). A promotion changes three things the shoppers see: the price they pay, how often a
// product lands on a shopping list (visibility) and how many shoppers come (flyers). Every promotion ends with a
// report line: units sold against the same days before, and the margin given away, so the player learns whether it
// paid. Rivals notice promotions (MarketCompetitors).
//  AisleDiscount  - one aisle (category) 10 or 20 % off for 3 days
//  MultiBuy       - "3 al 2 \u00f6de" on one product for 7 days: a shopper who takes it takes three and pays for two
//  Flyer          - neighbourhood flyer (40 TL x price level): 4 days more shoppers, promoted products more visible
//  Endcap         - gondola head: one product much more visible, free, one at a time, until moved
//  SupplierDeal   - the wholesaler's funded offer: 15 % off the unit cost for 7 days if the shelf price drops 10 %
//  Scoped         - G-078 (karar J07): the player's own campaign. Scope = one product, a brand, a subcategory, an
//                   aisle (category) or the whole store; mechanic = % off, 3 al 2 \u00f6de, 2 al 1 \u00f6de or
//                   "2. \u00fcr\u00fcn %50"; percent 5-50; 1-14 days. The older kinds stay for saves and the offer.
namespace MarketPromotions
{
    enum class EKind : uint8 { AisleDiscount = 0, MultiBuy, Flyer, Endcap, SupplierDeal, Scoped, Count };
    enum class EScope : uint8 { Product = 0, Brand, Subcategory, Category, Store, Count };
    enum class EMechanic : uint8 { Percent = 0, ThreeForTwo, TwoForOne, SecondHalf, Count };
    constexpr int32 MaxScopedDays = 14;

    FString ScopeName(EScope Scope);
    FString MechanicName(EMechanic Mechanic, int32 Percent);
    // The name the scope uses for the product (its brand, subcategory or aisle); "" when it has none.
    FString ScopeKeyOf(const TArray<FMarketProduct>& Products, int32 Product, EScope Scope);
    // Menu/Director argument: product + 10000 x scope + 100000 x mechanic + 1000000 x percent + 100000000 x days.
    int32 PackArg(int32 Product, EScope Scope, EMechanic Mechanic, int32 Percent, int32 Days);
    bool StartScoped(FMarketState& State, const TArray<FMarketProduct>& Products, int32 PackedArg, FString& OutMessage);
    // How many catalog products a scope reaches from the selected product.
    int32 ScopeSize(const TArray<FMarketProduct>& Products, int32 Product, EScope Scope);

    constexpr int32 MaxRunning = 3;          // besides the endcap
    constexpr int32 DiscountDays = 3;
    constexpr int32 MultiBuyDays = 7;
    constexpr int32 FlyerDays = 4;
    constexpr int32 DealDays = 7;
    constexpr int64 FlyerCost = 4000;        // 40 TL at the start price level
    constexpr int32 DealCostCut = 15;        // % off the unit cost
    constexpr int32 DealShelfCut = 10;       // % off the shelf price required
    constexpr int32 OfferTrust = 70;         // wholesaler trust for funded offers

    bool IsActive(const FMarketPromotion& Promo, int32 Day);
    TArray<const FMarketPromotion*> Active(const FMarketState& State);

    // What a shopper pays for one unit when buying Quantity of product Index today (discounts, 3-for-2).
    int64 UnitPrice(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index, int32 Quantity);
    // Quantity after the 3-for-2: a shopper who wants 2 or more takes 3.
    int32 AdjustQuantity(const FMarketState& State, int32 Index, int32 Quantity);
    // x how often the product is on a shopping list today (1 = no promotion).
    float Interest(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index);
    // x shoppers today (flyers).
    float TrafficFactor(const FMarketState& State);
    // x unit cost today (wholesaler-funded deal), for MarketSuppliers::UnitCost.
    float CostFactor(const FMarketState& State, int32 Index);
    // Short label for the shelf tag / menu, e.g. "%20 indirim", "3 al 2 \u00f6de" ("" when none).
    FString Badge(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index);

    // Starts a promotion. Product = catalog index (the aisle is the product's category for AisleDiscount).
    bool Start(FMarketState& State, const TArray<FMarketProduct>& Products, EKind Kind, int32 Product, int32 Percent, FString& OutMessage);
    bool Stop(FMarketState& State, int32 PromotionIndex, FString& OutMessage);
    bool AcceptOffer(FMarketState& State, const TArray<FMarketProduct>& Products, FString& OutMessage);
    void DeclineOffer(FMarketState& State);

    // Day close (after FMarketState::CloseDay): counts yesterday's sales of running promotions, reports finished
    // ones, and lets the wholesaler make a funded offer now and then.
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);
    FString Describe(const FMarketPromotion& Promo, const TArray<FMarketProduct>& Products);
}

#pragma once
#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Shopper decisions, independent of the world (tested in MarketTests.cpp: MarketSim.Customers.PriceAndDemand).
// A shopper comes in wanting one product. Most ask for something the shop carries (on a shelf plan); the rest ask
// for any active product, so missing products show up as lost shoppers in the day report. Whether they buy depends
// on the shelf price against the rival's price for the same product and on the shop's local share (loyalty).
// Every lost shopper gets a reason (empty shelf / too expensive / not carried / waited too long) for the report.
namespace MarketDemand
{
    // Share of shoppers who ask for a product that is on a shelf plan (the rest ask for any product).
    constexpr float CarriedShare = 0.85f;
    // Price ratio (ours / rival) at which half of the shoppers still buy, before the local share bonus.
    constexpr double HalfBuyRatio = 1.15;
    // Loyal shops are forgiven more: +0.1 ratio at 25 % local share, +0.26 at 65 %.
    constexpr double ShareBonusPerPercent = 1.0 / 250.0;
    // How sharply shoppers react around the half point (smaller = sharper).
    constexpr double Steepness = 0.07;

    enum class EVisit : uint8 { Buy, NotCarried, Empty, Expensive };

    struct FVisit
    {
        EVisit Result = EVisit::NotCarried;
        int32 Product = INDEX_NONE;
        int32 Quantity = 0;
    };

    // The rival's shelf price for the same product (list price, minus the rival's campaign discount).
    int64 RivalPrice(const FMarketProduct& Product, float RivalDiscount);
    double PriceRatio(int64 OurPrice, int64 TheirPrice);
    // 0..1: the share of shoppers who accept this price ratio.
    // PriceTolerance: the shopper's segment (MarketCustomers), added to the half-buy ratio.
    // The v0.1 curve, the same for every product (kept for older callers and tests); shoppers use BuyChanceFor.
    double BuyChance(double Ratio, float MarketShare, double PriceTolerance = 0.0);

    // B1 (#24): the shelf decision depends on the product's price elasticity (catalog "elasticity": 1.5 staples ..
    // 4 snacks and drinks). At the rival's price 90 % buy whatever the product; cheaper wins a little for a staple
    // and a lot for a snack; dearer loses more than cheaper wins (loss aversion x1.4). The local share and the
    // shopper's segment move the point of indifference like before (25 % share = neutral). A product whose price
    // everybody knows (catalog "kvi" 0..1: bread, milk, tea) is compared harder, both ways (x 1 + kvi).
    //   U = logit(0.9) + E x K x (1 + kvi) x g (x 1.4 when g < 0),  g = (1 - Ratio) + (Share - 25) / 250 + Tolerance
    constexpr double ParityChance = 0.90;
    constexpr double PriceSensitivity = 3.0;    // K
    constexpr double LossAversion = 1.4;
    constexpr float DefaultElasticity = 2.5f;   // products without a catalog value (also MarketPromotions)
    constexpr float NeutralShare = 25.f;
    // The product's elasticity, or DefaultElasticity.
    float ElasticityOf(const FMarketProduct& Product);
    double BuyChanceFor(double Ratio, float MarketShare, double PriceTolerance, float Elasticity, float Kvi = 0.f);
    // B1 (#27): a warning when the shelf price is below the unit cost ("" when not): one sentence and one number.
    FString PriceWarning(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index);
    // Step of the +/- price keys: about 5 % of the list price in whole 5 kurus, at least 5 kurus.
    int64 PriceStep(const FMarketProduct& Product);
    // Which product a new shopper wants. Rolls are 0..1 (FRandomStream in the game, fixed values in tests).
    // INDEX_NONE only when there are no products at all.
    int32 PickWanted(const FMarketState& State, float RollPool, float RollIndex);
    // OurPrice: the price the shopper actually pays (promotions, MarketPromotions::UnitPrice); 0 = the shelf price.
    // Available = shelf units not already in other shoppers' baskets. WantedQuantity 1..8 (segment, MarketCustomers).
    FVisit Decide(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Wanted, int32 Available,
        float RivalDiscount, int32 WantedQuantity, float RollPrice, float MarketShareOverride = -1.f, double PriceTolerance = 0.0, int64 OurPrice = 0);
    // Counts a shopper who did not buy (buyers are counted by FMarketState::Sell at the till).
    void RecordLoss(FMarketState& State, const FVisit& Visit);
    // Records the product reason without counting a whole lost shopper. Used by multi-item baskets;
    // the visit is counted once when the shopper pays or leaves with an empty basket.
    void RecordItemFailure(FMarketState& State, const FVisit& Visit);
    // A shopper who gave up in the shop (crowd, waited too long at the till, still inside at closing).
    void RecordWaitingLoss(FMarketState& State);

    enum class EProblem : uint8 { Waiting, NotCarried, Empty, Expensive };

    struct FProblem
    {
        EProblem Kind = EProblem::Waiting;
        int32 Product = INDEX_NONE; // INDEX_NONE for Waiting
        int32 Count = 0;
    };

    // Yesterday's biggest reasons for lost shoppers, largest first (ties: kind order, then catalog order).
    TArray<FProblem> TopProblems(const FMarketState& State, int32 MaxCount = 3);
}

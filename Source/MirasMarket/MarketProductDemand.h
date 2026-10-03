#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"
#include "MarketCustomers.h"
#include "MarketGoods.h"

// E3b (Docs/Kurgu/11_TEK_EKONOMI.md, Mustafa 03.10.2026: "ortak urun istegi"): one product demand for every store.
// What a shopper wants (taste of the segment x the day) and how the shelf price against the rivals turns a wish into
// a sale are computed here once. The family shop's shoppers draw their lists and roll their price decision from it
// one by one (MarketCustomers::BuildList, MarketDemand::Decide); the branches use the same numbers as expected
// values for the whole day (MarketBranches::CloseDay). On average both give the same sales. Independent of the
// world, tested (MirasMarket.ProductDemand.*).
namespace MarketProductDemand
{
    // The day's pull on a product group in every store: the calendar (season, weather, holidays) x the epidemic's
    // panic buying (MarketOnline, M32).
    float DayFactor(const FMarketState& State, MarketGoods::EGroup Group, int32 Day);
    // How much one segment wants a product on Day: its taste for the product's group x DayFactor. A store multiplies
    // its own extras on top (its promotions, events, identity).
    float SegmentWish(const FMarketState& State, const FMarketProduct& Product, MarketCustomers::ESegment Segment, int32 Day);
    // A segment mix's (percent per segment) wish for each product, normalized to 1 over the products.
    TArray<float> MixWishes(const FMarketState& State, const TArray<FMarketProduct>& Products, const int32 (&Mix)[6], int32 Day);
    // The mix's average price tolerance (MarketCustomers profiles, weighted by the mix).
    double MixTolerance(const int32 (&Mix)[6]);
    // The purchasing power's part of the price tolerance: income 1.25 -> +5 points, 0.75 -> -5 points.
    double IncomeTolerance(float Income);
    // The chance a wished item is bought at Ratio (our shelf price / the rivals') with the store's share of its
    // surroundings (percent) and the shoppers' tolerance: MarketDemand::BuyChanceFor with the product's elasticity
    // and how well known its price is (KVI).
    double Acceptance(double Ratio, float SharePercent, double Tolerance, const FMarketProduct& Product);
    // Acceptance against parity (ratio 1, neutral share, no tolerance gives MarketDemand::ParityChance): 1 at parity,
    // above for a cheaper shelf, below for a dearer one. The branches' wishes are calibrated at parity.
    double AcceptanceFactor(double Ratio, float SharePercent, double Tolerance, const FMarketProduct& Product);
}

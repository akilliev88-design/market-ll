#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Karar M25 (Mustafa 30.09.2026): brands ask for room on our shelves, and inside every aisle the brands compete for
// share. Independent of the world, tested (MirasMarket.Brands.*).
//  - A brand's shelf share in an aisle = its shelf capacity / the aisle's, over the family shop and every branch.
//    Its share of the country (National) starts from its standing (Config/markalar.json tier: lider, guclu, orta)
//    and drifts a little with our shelves once we are big.
//  - Once a month the brands that feel short on our shelves make offers (at most three open, 14 days to answer):
//    shelf share (they pay a month while their share of the aisle stays above a target, three months), listing
//    money (once, when a product of theirs we do not carry goes on a shelf) and a volume rebate (a percent of the
//    month's sales of their products above a floor).
//  - Deals are checked at the month's close: kept -> paid and trust grows; missed twice -> cancelled, trust falls.
//  - Trust changes their prices to us: a brand we starve (under half its national share) cools and charges 3 %
//    more; a friend gives 2 % off (MarketDirector::ApplyPrices).
//  - Fun (06 section 2b): the offer is a small decision with a clear trade (money now against the aisle's mix) and
//    its result shows at the month's close.
namespace MarketBrands
{
    enum class EKind : uint8 { ShelfShare = 0, Listing, Rebate };

    constexpr int32 OfferDays = 14;
    constexpr int32 DealMonths = 3;
    constexpr int32 MaxOpenOffers = 2;   // C3 (A6): 3 -> 2
    constexpr int32 MonthDays = 30;

    struct FBrandInfo
    {
        FString Real;
        FString Fictional;
        FString Category;
        float Power = 0.45f;             // lider 1.0, guclu 0.7, orta 0.45
    };
    // Config/markalar.json (loaded once); tests set their own.
    const TArray<FBrandInfo>& Table();
    void SetTable(const TArray<FBrandInfo>& Brands);
    void ResetTable();
    float Power(const FString& Brand, const FString& Category);
    // The name the player sees (fictional unless State.bRealBrands).
    FString NameOf(const FMarketState& State, const FString& Brand);

    // Shelf capacity of a brand in an aisle and of the whole aisle (family shop + branches).
    int32 ShelfUnits(const FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Brand, const FString& Category);
    int32 AisleUnits(const FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Category);
    float ShelfShare(const FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Brand, const FString& Category);
    float NationalShare(const FMarketState& State, const FString& Brand, const FString& Category);
    float Trust(const FMarketState& State, const FString& Brand);
    // x the brand's purchase cost for us (1.03 cool, 0.98 friend, else 1).
    float CostFactor(const FMarketState& State, const FString& Brand);

    // Seeds the brands of the catalog's aisles (national shares from their standing). Idempotent.
    void Ensure(FMarketState& State, const TArray<FMarketProduct>& Products);

    bool Accept(FMarketState& State, int32 OfferId, FString& OutMessage);
    bool Reject(FMarketState& State, int32 OfferId, FString& OutMessage);

    // Menu lines.
    FString DescribeOffer(const FMarketState& State, const FMarketBrandOffer& Offer);
    FString DescribeDeal(const FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketBrandDeal& Deal);
    // "S\u00fct: Punar %42 \u00b7 S\u00fctka\u015f %31 \u00b7 ..." for one aisle (national share, our shelf share in brackets).
    FString AisleLine(const FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Category);
    TArray<FString> Categories(const TArray<FMarketProduct>& Products);

    // Day close (after the branches): the day's sales by brand, listing deals fulfilled, and at the month's end
    // the deals, trust, national drift and new offers.
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);
}

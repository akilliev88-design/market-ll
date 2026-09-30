#pragma once

#include "CoreMinimal.h"
#include "MarketLedger.generated.h"

struct FMarketState;
struct FMarketProduct;

// Akis B (Docs/Kurgu/07_AKIL_ISBOLUMU.md \u00a74): the company's books. One field of FMarketState (Ledger); older
// saves load it empty and it starts on the next day close. Independent of the world, tested (MirasMarket.Ledger.*).

// B1 (#27): what a running promotion earned, counted at every day close (MarketPromotions). Key: the promotion's
// kind, product and first day. Removed with the promotion's final report.
USTRUCT()
struct FMarketPromoTally
{
    GENERATED_BODY()
    UPROPERTY() uint8 Kind = 0;
    UPROPERTY() int32 Product = INDEX_NONE;
    UPROPERTY() int32 StartDay = 0;
    UPROPERTY() int64 Revenue = 0;       // estimated from the deal price of the covered units sold
    UPROPERTY() int64 CostOfGoods = 0;   // the units' book cost
    UPROPERTY() int64 Support = 0;       // the wholesaler's money: units received on the deal x the cost cut
};

USTRUCT()
struct FMarketLedger
{
    GENERATED_BODY()
    // B1 (#43): a losing tax week's loss, taken off the next weeks' taxable profit (MarketStaff).
    UPROPERTY() int64 TaxLossCarry = 0;
    // B1 (#45): our revenue per day in the campaign country, smoothed over about a month (MarketCompany::NationalShare).
    UPROPERTY() int64 CountryRevenueDay = 0;
    // B1 (#27, #30): running promotions' results; online units of the last closed day by catalog row (a promotion's
    // "before" counts the shop only, like its "during").
    UPROPERTY() TArray<FMarketPromoTally> PromoTallies;
    UPROPERTY() TArray<int32> OnlineSold;
    // B1 (#27): units sold below their cost on the last closed day and the loss on them (day report line).
    UPROPERTY() int32 LastBelowCostUnits = 0;
    UPROPERTY() int64 LastBelowCostLoss = 0;
};

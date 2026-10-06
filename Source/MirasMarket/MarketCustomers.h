#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"
#include "MarketGoods.h"

// Who the shopper is (G-062, Docs/Kurgu/00_KURGU_KITABI.md \u00a76). Independent of the world, tested
// (MirasMarket.Customers.Segments). Every neighbourhood customer (MarketBasket loyalty id) has a fixed segment, so a
// returning customer is recognisably the same kind of person.
// The segment decides when they come, what is on their list, how much they buy, how much they can spend, how far
// the price may be above the rival's, how long they wait, how fast they walk and how long they look at a shelf.
namespace MarketCustomers
{
    enum class ESegment : uint8 { Retired, Family, Worker, Student, Trader, Child, Count };

    struct FProfile
    {
        int32 MinList = 1;
        int32 MaxList = 3;
        int32 MinQuantity = 1;
        int32 MaxQuantity = 3;
        int64 Budget = 3000;          // kurus per visit on an ordinary day
        double PriceTolerance = 0.0;  // added to the price ratio at which half of them still buy
        float Patience = 90.f;        // seconds in the shop before giving up
        float WalkSpeed = 140.f;      // cm/s
        float BrowseSeconds = 1.5f;   // looking at a shelf before deciding
        float Preference[static_cast<int32>(MarketGoods::EGroup::Count)] = {};
    };

    // District mix of the starting street, percent: retired, family, worker, student, trader, child.
    constexpr int32 StationMix[static_cast<int32>(ESegment::Count)] = { 22, 28, 24, 12, 6, 8 };

    const FProfile& Profile(ESegment Segment);
    FString SegmentName(ESegment Segment);
    // Fixed segment of a neighbourhood customer (campaign seed + id), following the district mix.
    ESegment SegmentOf(int32 CustomerId, int32 Seed, const int32* Mix = StationMix);
    // 0..2: how likely this segment is in the shop at this point of the day (0 = opening, 1 = closing),
    // on this game day (weekend families, summer holidays).
    float TimeWeight(ESegment Segment, float DayProgress, int32 GameDay);
    // Whether a chosen customer comes now; Roll 0..1. Keeps the time-of-day pattern without a second pool.
    bool ComesNow(ESegment Segment, float DayProgress, int32 GameDay, float Roll);

    // 1..MaxList distinct wishes, weighted by the segment's preferences and the day (calendar). Carried products
    // are asked for about 85 % of the time (as MarketDemand::CarriedShare); the rest shows what is missing.
    TArray<int32> BuildList(const FMarketState& State, const TArray<FMarketProduct>& Products, ESegment Segment, FRandomStream& Random);
    int32 Quantity(ESegment Segment, FRandomStream& Random);
    // Money for this visit: segment budget x the calendar's wallet factor (paydays, month end) x wage level.
    int64 VisitBudget(ESegment Segment, int32 GameDay);
    // Units of a product that fit in what is left of the budget (at least 1 when one unit fits).
    int32 Affordable(int64 BudgetLeft, int64 Price, int32 Wanted);
}

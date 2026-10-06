#pragma once

#include "CoreMinimal.h"

struct FMarketState;
struct FMarketProduct;

// G-110 (M69, M70; Mustafa 06.10.2026): the first store is a store like the others and can be closed. Independent of
// the world, tested (MarketSim.FirstStore.*).
//  - Closing it: its cashiers and stockers leave with their severance (the HR manager and the accountant work for
//    the head office and stay); its goods go to our other open stores (same province first, then the country) as far
//    as they have room; perishable goods and what does not fit go to the wholesaler at half the cost; its running
//    campaigns end. The player chooses what happens to the building (it is ours):
//      sell  - today's value comes into the till, the building leaves the balance sheet, the store never reopens;
//      keep  - the building stays empty (a quarter of its upkeep), it can be reopened any day;
//      lease - the building stays ours and brings a monthly rent (its reference rent); reopening it pays the tenant
//              one month's rent to leave.
//  - A closed first store has no shoppers, no shop staff, no orders and no online picking; the company goes on with
//    its branches. With no store at all the game goes on from the map: a new store can always be opened.
//  - Reopening starts with empty shelves and no shop staff: the player hires and orders like on the first day.
namespace MarketFirstStore
{
    enum class EStatus : uint8 { Open = 0, Empty = 1, Sold = 2, Leased = 3 };
    enum class EBuilding : uint8 { Sell = 0, Keep = 1, Lease = 2 };

    constexpr double EmptyUpkeep = 0.25;      // share of the running costs an empty building still has
    constexpr int32 LeaseNoticeMonths = 1;    // a tenant leaves for one month's rent

    EStatus StatusOf(const FMarketState& State);
    bool IsOpen(const FMarketState& State);
    // Share of the first store's daily running costs (electricity, water, upkeep) the company pays today.
    double RunningShare(const FMarketState& State);
    // The rent the building would bring a month (and what a tenant pays): the building's reference rent.
    int64 MonthlyLease(const FMarketState& State);

    bool CanClose(const FMarketState& State, FString& OutReason);
    bool Close(FMarketState& State, const TArray<FMarketProduct>& Products, EBuilding Building, FString& OutMessage);
    bool CanReopen(const FMarketState& State, FString& OutReason);
    bool Reopen(FMarketState& State, FString& OutMessage);
    // The day close: a leased building's rent of the day.
    void CloseDay(FMarketState& State);

    // Goods leaving a closing store: our other open stores take what fits on the way (Incoming), the closing store's
    // province first, then its country, then anywhere. SkipBranch: the closing branch (INDEX_NONE for the first store).
    // Returns the units placed.
    int32 SendToStores(FMarketState& State, const FString& Country, const FString& Province, const FString& ProductId, int32 Units, int32 SkipBranch);

    // Menu texts.
    FString StatusText(const FMarketState& State);
    FString CloseQuestion(const FMarketState& State, EBuilding Building);
    FString ReopenQuestion(const FMarketState& State);
}

#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"
#include "Planogram.h"

// What the shelf staff (reyon gorevlisi) decides to do next. Pure rules without the world, so they are tested
// (MarketSim.Staff.Planner). The worker in the game (MarketWorkers.cpp) walks, carries and applies the result.
// Workers use exactly the player's arranging rules (MarketPlanogramEdit): they only add blocks in free space or
// grow a block into free space next to it; they never move, shrink or remove a block.
namespace StaffPlanner
{
    enum class EJob : uint8
    {
        None,
        Refill,   // bring units from the depot to the product's shelf
        Place,    // product is not on a shelf: put a new block on its category's shelves, then fill it
        Widen,    // product's blocks cannot hold one case: one more facing where there is room, then fill
    };

    struct FJob
    {
        EJob Kind = EJob::None;
        int32 Product = INDEX_NONE;   // catalog / stock index
        FPlanogramPlacement Block;    // Place: the planned block. Widen: the block to grow (found again by position)
    };

    constexpr int32 CarryUnits = 24;  // units one trip carries

    // Shelf and product categories match ignoring case and Turkish letters (I-dot / c-cedilla fold to i / c).
    MARKETSIM_API bool SameCategory(const FString& A, const FString& B);
    // Facings a new block asks for: enough for one case (at least 12 units), 2..6.
    MARKETSIM_API int32 WantedFacings(const FMarketPlanogram& Planogram, const FMarketProduct& Product, const FPlanogramPlacement& Block);
    // Best free spot for a product that is not on a shelf: a shelf of its category, next to its brand if possible,
    // as many of the wanted facings as fit, eye level preferred. False + OutReason (Turkish) when there is none.
    MARKETSIM_API bool PlanNewBlock(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 ProductIndex,
        FPlanogramPlacement& OutBlock, FString& OutReason);
    // Next job, most urgent first: a shelf at half or less > a product with stock but no shelf > topping up > widening.
    // Busy = products other workers are handling. Unplaceable = products known to have no room (skipped).
    // bAllowPlanEdits = false (player is arranging): refills only. OutNoRoom receives products found to have no room.
    MARKETSIM_API FJob ChooseJob(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FMarketState& State,
        const TSet<int32>& Busy, const TSet<int32>& Unplaceable, bool bAllowPlanEdits, TArray<TPair<int32, FString>>* OutNoRoom = nullptr);
    // At the shelf: puts the planned block (within a few cm of the plan). False when the spot was taken meanwhile.
    MARKETSIM_API bool TryPlace(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FPlanogramPlacement& Block, FString& OutMessage);
    // At the shelf: one more facing on the block at that position. False when it is gone or has no room any more.
    MARKETSIM_API bool TryWiden(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FPlanogramPlacement& Block, FString& OutMessage);
    // Block of the same product on the same row within 2 cm of Block's position, or INDEX_NONE.
    MARKETSIM_API int32 FindBlock(const FMarketPlanogram& Planogram, const FPlanogramPlacement& Block);
    // Worker names by hiring order.
    MARKETSIM_API FString WorkerName(int32 Index);
}

#pragma once

#include "CoreMinimal.h"
#include "Planogram.h"

// Automatic shelf plan for a new branch (G-068, Docs/Kurgu/00_KURGU_KITABI.md \u00a79, karar H03). Independent of the
// world, tested (MirasMarket.Layout.*). The family shop stays hand-arranged (Mustafa's G-045 decision); this lays out
// the shelves of branches the way a good store planner would, with exactly the player's rules (MarketPlanogramEdit):
//  1. Aisles in a customer path: drinks, snacks and sweets at the entrance; tea, staples and oil in the middle;
//     dairy at the back (it pulls shoppers through the shop); cleaning and personal care on their own fixtures,
//     away from food.
//  2. Fixtures closer to the entrance get the earlier aisles; an aisle gets fixtures in proportion to its demand.
//  3. Shelf space per product in proportion to expected sales and margin, at least one case, at most 6 facings.
//  4. Levels: high-margin products at eye level, big and heavy packages at the bottom, small light ones on top.
//  5. Brand blocks: products of one brand side by side.
//  6. Whatever does not fit on its aisle goes to the nearest aisle with room; the report names what found no room.
namespace MarketLayout
{
    struct FResult
    {
        int32 Placed = 0;
        TArray<FString> NoRoom;             // product ids
        TArray<FString> Aisles;             // fixture id -> category, "gondol_1: i\u00e7ecek"
    };

    // Customer-path rank of a category (lower = nearer the entrance; household goods last).
    int32 AisleRank(const FString& Category);
    // Shelf levels from best to worst for this product (eye level first for high margin, bottom first when heavy).
    TArray<int32> LevelPreference(const FPlanogramEquipment& Equipment, const FMarketProduct& Product);

    // Fixtures of a branch format: "mahalle" (4 gondolas + 6 wall shelves, like the family shop), "kucuk"
    // (2 gondolas + 1 wall shelf), "buyuk" (8 gondolas + 3 wall shelves). Entrance at -Y.
    FMarketPlanogram Fixtures(const FString& Format);
    // Lays out Products on the plan's fixtures (existing placements are cleared). Demand = expected daily units per
    // product (index like Products; missing = 1).
    FResult Plan(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const TArray<float>& Demand);
    // Copies a template's blocks onto a plan whose fixtures have the same ids and equipment (a saved layout).
    int32 CopyTemplate(const FMarketPlanogram& Template, FMarketPlanogram& Target, const TArray<FMarketProduct>& Products);
    // Units each product holds on the plan (index like Products).
    TArray<int32> Capacities(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products);
}

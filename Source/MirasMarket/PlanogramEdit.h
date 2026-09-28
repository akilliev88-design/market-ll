#pragma once

#include "CoreMinimal.h"
#include "Planogram.h"

// Hand arrangement of the shelves, block by block. The Raf Plani editor and the in-game arrange mode (R)
// both use these functions, so both follow the same rules:
// - a block stays exactly where it was put (fixture-local X); nothing moves or fills by itself;
// - the same product may have any number of blocks;
// - a block never passes the shelf edge or touches a neighbouring block (MarketPlanogram::ProductGapCm);
// - a rejected edit leaves the plan unchanged and OutMessage says why (Turkish, for the UI).
// Blocks are addressed by their index in FMarketPlanogram::Placements (indices change after Add/Remove).
namespace MarketPlanogramEdit
{
    // What a block would become at a spot: the in-game ghost preview shows exactly this, and AddBlock /
    // MoveBlock store exactly this.
    struct FBlockPlan
    {
        bool bOk = false;
        FPlanogramPlacement Block; // final block: snapped X, facings maybe reduced, stack limited, physical depth
        FString Reason;            // why it does not fit (bOk == false)
        FString Note;              // what was adjusted to make it fit (bOk == true), may be empty
    };

    // Block = wished settings (product, fixture, face, level, facings, orientation, stack). DesiredX = the aimed
    // centre. The block snaps to the nearest free spot within MaxShiftCm of the wish; when the facings do not fit
    // there, fewer facings are tried (bAllowFewerFacings). IgnoreIndex = the block being moved.
    MIRASMARKET_API FBlockPlan PlanBlock(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
        const FPlanogramPlacement& Block, float DesiredX, int32 IgnoreIndex = INDEX_NONE, float MaxShiftCm = 1.0e6f, bool bAllowFewerFacings = true);

    MIRASMARKET_API bool AddBlock(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FPlanogramPlacement& Block,
        float DesiredX, float MaxShiftCm, FString& OutMessage, int32* OutIndex = nullptr);
    // Editor helper: a new block of the product at the right end of the row (default 2 facings, upright).
    MIRASMARKET_API bool AddToRowEnd(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FString& ProductId,
        const FString& FixtureId, const FString& Face, int32 Level, FString& OutMessage, int32* OutIndex = nullptr);
    MIRASMARKET_API bool MoveBlock(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index,
        const FString& FixtureId, const FString& Face, int32 Level, float DesiredX, float MaxShiftCm, FString& OutMessage);
    MIRASMARKET_API bool RemoveBlock(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index, FString& OutMessage);
    // Grows/shrinks around the block centre; slides a little when one side is blocked.
    MIRASMARKET_API bool ChangeFacings(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index, int32 Delta, FString& OutMessage);
    // Slides the block; stops against a neighbour or the shelf edge.
    MIRASMARKET_API bool Nudge(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index, float DeltaCm, FString& OutMessage);
    MIRASMARKET_API bool CycleOrientation(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index, FString& OutMessage);
    // bWrap: going above the maximum starts again at 1 (single-key control in the game).
    MIRASMARKET_API bool ChangeStack(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index, int32 Delta, bool bWrap, FString& OutMessage);
    // Extra space the block keeps to its neighbours (0 = side by side). Rejected when a neighbour is too close.
    MIRASMARKET_API bool ChangeGap(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index, float DeltaCm, FString& OutMessage);
    // Same level on the other aisle side of a double-sided gondola, as close to the same X as possible.
    MIRASMARKET_API bool ToggleFace(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index, FString& OutMessage);

    // Helpers for the UIs.
    MIRASMARKET_API int32 NextOrientation(const FMarketProduct& Product, int32 Orientation);
    MIRASMARKET_API int32 MaxStack(const FMarketPlanogram& Planogram, const FMarketProduct& Product, const FPlanogramPlacement& Block);
    MIRASMARKET_API const TCHAR* OrientationName(int32 Orientation);
    // "\u00f6nde 3 \u00d7 derinlik 4 \u00d7 1 kat = 12 adet"
    MIRASMARKET_API FString CapacityText(const FPlanogramPlacement& Block);
}

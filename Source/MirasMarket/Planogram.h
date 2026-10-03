#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

struct MIRASMARKET_API FPlanogramFixture
{
    FString Id;
    FString EquipmentId = TEXT("gondola_double_1200");
    FString Label;
    FString Category;
    // Optional per-face override. Empty is meaningful (uncategorised); old saves inherit Category.
    TMap<FString, FString> FaceCategories;
    FString CategoryForFace(const FString& Face) const { const FString* C = FaceCategories.Find(Face); return C ? *C : Category; }
    FVector Location = FVector::ZeroVector;
    float Yaw = 0.f;
};

struct MIRASMARKET_API FPlanogramPlacement
{
    FString ProductId;
    FString FixtureId;
    FString Face = TEXT("front");
    int32 Level = 0;
    int32 Facings = 2;
    int32 Depth = 3;
    // Block centre along the shelf, fixture-local X in cm (0 = middle of the shelf). Schema v3 stores it
    // for every block; blocks sit exactly where they were put and never move by themselves.
    float XCm = 0.f;
    bool bHasX = false;
    // Extra free space this block keeps to its neighbours (0 = side by side with a tiny tolerance).
    float GapCm = 0.f;
    // Legacy (schema v1/v2): left-to-right order + fine offset of the old automatic packing. Only used
    // once by ResolvePositions to turn old files into XCm; not written any more.
    int32 Order = 0;
    float OffsetCm = 0.f;
    // 0 = front facing, 1 = quarter-turn on the shelf, 2 = laid on its side (boxes/bags only).
    int32 Orientation = 0;
    // Units vertically stacked at every facing/depth slot. Limited by package type and shelf clearance.
    int32 Stack = 1;
};

// Physical data of one fixture model. Lengths in cm, fixture local space: front customer face is -Y.
struct MIRASMARKET_API FPlanogramEquipment
{
    FString Id;
    FString MeshPath;
    // Full shelf board (Blender: 1.16 m). The uprights stand in the spine, not in front of the products.
    float UsableWidthCm = 116.f;
    int32 Levels = 4;
    float LevelTopZ[8] = { 19.5f, 53.5f, 87.5f, 121.5f, 0.f, 0.f, 0.f, 0.f };
    // Unreal's FBX import mirrors Blender's Y axis: a kit authored with its front at Blender -Y faces +Y
    // in Unreal. MeshYaw turns the mesh so its customer face matches the fixture's -Y front.
    float MeshYaw = 0.f;
    float FrontY = -42.f;          // product front line of the front face (back face mirrors it)
    float UsableDepthCm = 37.f;
    bool bDoubleSided = true;
    float RailFrontY[8] = { 45.f, 45.f, 45.f, 45.f, 0.f, 0.f, 0.f, 0.f }; // |y| of the price-rail front
    float RailAboveTopZ = -1.2f;   // slim ticket strip hangs below the shelf top
    float LevelClearanceCm[8] = { 30.f, 30.f, 30.f, 38.f, 0.f, 0.f, 0.f, 0.f };
    float SignZ = 176.f;           // category sign center height
    float SignY = 0.f;             // sign center |y| (0 = on top, both faces)
    float SignWidthCm = 112.f;
    bool bSignOnTop = true;
    bool bShowCategorySign = true; // independent of the fixture's product category
    FVector DimensionsCm = FVector(120, 90, 160);
    FString Family;
    int32 CheckoutCount = 0;
};

struct MIRASMARKET_API FMarketPlanogram
{
    TArray<FPlanogramFixture> Fixtures;
    // Only what the author placed: one entry per product block. The same product may have any number of
    // blocks (several places, several shelves). A catalog product without a block is "not on a shelf":
    // the game gives it no shelf space and customers do not ask for it. Nothing is placed automatically.
    TArray<FPlanogramPlacement> Placements;

    const FPlanogramFixture* FindFixture(const FString& Id) const;
    FPlanogramFixture* FindFixture(const FString& Id);
    // First block of a product, or null when it is not on a shelf.
    const FPlanogramPlacement* FindPlacement(const FString& ProductId) const;
    FPlanogramPlacement* FindPlacement(const FString& ProductId);
};

namespace MarketPlanogram
{
    constexpr int32 SchemaVersion = 3;
    constexpr int32 MaxLevels = 8;
    constexpr int32 MaxFacings = 30;
    constexpr int32 MaxDepth = 12;
    constexpr float ItemGapCm = 0.5f;    // side by side between facings of the same product
    constexpr float RowGapCm = 2.f;      // front-to-back between depth rows
    constexpr float ProductGapCm = 0.3f; // tolerance between neighbouring blocks (plus each block's GapCm)
    constexpr float MaxBlockGapCm = 20.f;

    MIRASMARKET_API FString DefaultPath();
    // Known fixture models: gondola_double_1200, wall_shelf_2400. Unknown ids use gondola sizes
    // (double sided only when the id contains "double").
    MIRASMARKET_API FPlanogramEquipment Equipment(const FString& EquipmentId);
    MIRASMARKET_API bool IsKnownEquipment(const FString& EquipmentId);
    MIRASMARKET_API FPlanogramEquipment EquipmentFor(const FMarketPlanogram& Planogram, const FString& FixtureId);
    MIRASMARKET_API bool Parse(const FString& Json, FMarketPlanogram& OutPlanogram, TArray<FString>& OutErrors);
    MIRASMARKET_API FString Serialize(const FMarketPlanogram& Planogram);
    MIRASMARKET_API bool LoadFile(const FString& Path, FMarketPlanogram& OutPlanogram, TArray<FString>& OutErrors);
    MIRASMARKET_API bool SaveFile(const FString& Path, const FMarketPlanogram& Planogram, FString& OutError);
    MIRASMARKET_API float NominalWidthCm(const FMarketProduct& Product);
    MIRASMARKET_API float NominalDepthCm(const FMarketProduct& Product);
    MIRASMARKET_API float NominalHeightCm(const FMarketProduct& Product);
    MIRASMARKET_API float OrientedWidthCm(const FMarketProduct& Product, int32 Orientation);
    MIRASMARKET_API float OrientedDepthCm(const FMarketProduct& Product, int32 Orientation);
    MIRASMARKET_API float OrientedHeightCm(const FMarketProduct& Product, int32 Orientation);
    MIRASMARKET_API bool CanLayOnSide(const FMarketProduct& Product);
    MIRASMARKET_API int32 MaxStackFor(const FMarketProduct& Product, int32 Orientation, float ClearanceCm);
    // Rows of the block's package (in its orientation) that fit front-to-back on its shelf.
    MIRASMARKET_API int32 PhysicalDepth(const FMarketPlanogram& Planogram, const FMarketProduct& Product, const FPlanogramPlacement& Placement);
    // Shelf units of one placement (facings x depth x stack).
    MIRASMARKET_API int32 Capacity(const FPlanogramPlacement& Placement);
    // Shelf units of a product (facings x depth x stack), stack limited to what the package and level
    // clearance allow (what the game draws). 0 = not on a shelf.
    MIRASMARKET_API int32 ProductCapacity(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FString& ProductId);
    // Stack that physically fits for this block (package type + level clearance).
    MIRASMARKET_API int32 EffectiveStack(const FMarketPlanogram& Planogram, const FMarketProduct& Product, const FPlanogramPlacement& Placement);
    // Depth is physical: every block is as many rows deep as the shelf holds for its package/orientation.
    MIRASMARKET_API void FitDepth(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products);
    // Block centre (fixture-local X, cm). XCm for v3 blocks; legacy packing (order + offset) otherwise.
    MIRASMARKET_API float PlacementCenterX(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FPlanogramPlacement& Placement);
    // Old files: gives every block without XCm the position the old packing showed. Call after loading.
    MIRASMARKET_API void ResolvePositions(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products);
    // Blocks of one shelf row (fixture + face + level) as indices into Placements, left to right.
    MIRASMARKET_API TArray<int32> RowBlocks(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
        const FString& FixtureId, const FString& Face, int32 Level);
    // Block under X on that row (within ToleranceCm of its edges), or INDEX_NONE.
    MIRASMARKET_API int32 BlockAt(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
        const FString& FixtureId, const FString& Face, int32 Level, float X, float ToleranceCm = 0.f);
    // Nearest centre to DesiredX where a block WidthCm wide sits on the shelf without touching another block
    // (IgnoreIndex = the block being moved). Snaps to shelf edges and neighbour edges. False = no gap wide enough.
    MIRASMARKET_API bool FindFreeX(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
        const FString& FixtureId, const FString& Face, int32 Level, float WidthCm, float DesiredX, int32 IgnoreIndex, float& OutX, float GapCm = 0.f);
    // Widest free gap on a row (cm), for messages.
    MIRASMARKET_API float WidestGapCm(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
        const FString& FixtureId, const FString& Face, int32 Level, int32 IgnoreIndex = INDEX_NONE);
    // Block Index is on an existing level/face, inside the shelf and clear of every other block.
    MIRASMARKET_API bool BlockFits(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index, FString* OutReason = nullptr);

    // Width of a block: facings x oriented package width + the small gap between facings.
    MIRASMARKET_API float BlockWidthCm(const FMarketProduct& Product, const FPlanogramPlacement& Placement);
    // Used width of a level (blocks + tolerance between them).
    MIRASMARKET_API float LevelUsedWidthCm(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
        const FString& FixtureId, const FString& Face, int32 Level);
    // Human readable warning per overflowing level (ASCII). Empty = everything fits.
    MIRASMARKET_API void FindOverflows(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, TArray<FString>& OutWarnings);
}

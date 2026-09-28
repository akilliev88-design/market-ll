#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

struct MIRASMARKET_API FPlanogramFixture
{
    FString Id;
    FString EquipmentId = TEXT("gondola_double_1200");
    FString Label;
    FString Category;
    FString Strategy = TEXT("manual");
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
    int32 Order = 0;
    // Fine placement along the shelf, measured from the automatically packed position. The editor
    // changes this in 5 cm steps and rejects shelf-edge or neighbouring-block overlaps.
    float OffsetCm = 0.f;
    // 0 = front facing, 1 = quarter-turn on the shelf, 2 = laid on its side (boxes/bags only).
    int32 Orientation = 0;
    // Units vertically stacked at every facing/depth slot. Limited by package type and shelf clearance.
    int32 Stack = 1;
    // Runtime-only extra block created by FillToCapacity on an empty level (never saved).
    bool bExtra = false;
};

// Physical data of one fixture model. Lengths in cm, fixture local space: front customer face is -Y.
struct MIRASMARKET_API FPlanogramEquipment
{
    FString Id;
    FString MeshPath;
    float UsableWidthCm = 110.f;
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
};

struct MIRASMARKET_API FMarketPlanogram
{
    TArray<FPlanogramFixture> Fixtures;
    TArray<FPlanogramPlacement> Placements;
    // When true the game widens every block to use the free width of its level and makes it as deep
    // as the shelf allows ("put as many as fit"). Author facings then act as minimums; depth is physical.
    bool bAutoFill = true;

    const FPlanogramFixture* FindFixture(const FString& Id) const;
    FPlanogramFixture* FindFixture(const FString& Id);
    // Primary (authored) placement of a product. Extra runtime blocks are listed after it.
    const FPlanogramPlacement* FindPlacement(const FString& ProductId) const;
    FPlanogramPlacement* FindPlacement(const FString& ProductId);
};

namespace MarketPlanogram
{
    constexpr int32 SchemaVersion = 2;
    constexpr float UsableWidthCm = 110.f;
    constexpr float ShelfFrontY = -42.f;
    constexpr float ShelfBaseZ = 19.5f;
    constexpr float ShelfLevelStepZ = 34.f;
    constexpr int32 LevelCount = 4;       // gondola default; use Equipment(...).Levels per fixture
    constexpr int32 MaxLevels = 8;
    constexpr float UsableDepthCm = 37.f;  // per customer face (equipment.json shelfZones depthMm 370)
    constexpr int32 MaxFacings = 30;
    constexpr int32 MaxDepth = 12;
    constexpr float ItemGapCm = 2.f;     // between facings of the same product
    constexpr float ProductGapCm = 3.f;  // between neighbouring product blocks

    MIRASMARKET_API FString DefaultPath();
    // Known fixture models: gondola_double_1200, wall_shelf_2400. Unknown ids use gondola sizes
    // (double sided only when the id contains "double").
    MIRASMARKET_API FPlanogramEquipment Equipment(const FString& EquipmentId);
    MIRASMARKET_API FPlanogramEquipment EquipmentFor(const FMarketPlanogram& Planogram, const FString& FixtureId);
    MIRASMARKET_API bool Parse(const FString& Json, FMarketPlanogram& OutPlanogram, TArray<FString>& OutErrors);
    MIRASMARKET_API FString Serialize(const FMarketPlanogram& Planogram);
    MIRASMARKET_API bool LoadFile(const FString& Path, FMarketPlanogram& OutPlanogram, TArray<FString>& OutErrors);
    MIRASMARKET_API bool SaveFile(const FString& Path, const FMarketPlanogram& Planogram, FString& OutError);
    // Keeps author choices and assigns any new active catalog product to a suitable fixture.
    MIRASMARKET_API void Reconcile(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products);
    MIRASMARKET_API float NominalWidthCm(const FMarketProduct& Product);
    MIRASMARKET_API float NominalDepthCm(const FMarketProduct& Product);
    MIRASMARKET_API float NominalHeightCm(const FMarketProduct& Product);
    MIRASMARKET_API float OrientedWidthCm(const FMarketProduct& Product, int32 Orientation);
    MIRASMARKET_API float OrientedDepthCm(const FMarketProduct& Product, int32 Orientation);
    MIRASMARKET_API float OrientedHeightCm(const FMarketProduct& Product, int32 Orientation);
    MIRASMARKET_API bool CanLayOnSide(const FMarketProduct& Product);
    MIRASMARKET_API int32 MaxStackFor(const FMarketProduct& Product, int32 Orientation, float ClearanceCm);
    // Rows of this product that fit front-to-back on one shelf face.
    MIRASMARKET_API int32 DepthThatFits(const FMarketProduct& Product, float UsableDepth = UsableDepthCm);
    // Shelf units of one placement (facings x depth).
    MIRASMARKET_API int32 Capacity(const FPlanogramPlacement& Placement);
    // Shelf units of a product over all its blocks (primary + extra).
    MIRASMARKET_API int32 ProductCapacity(const FMarketPlanogram& Planogram, const FString& ProductId);
    // "Put as many as fit". With bUseEmptyLevels, every empty level/face of a fixture first gets an extra
    // block of a product that belongs there (placed on that fixture or same category). Then every block
    // becomes as deep as the shelf and the free width of each level is shared as facings (fewest first).
    // Never creates an overflow. Returns facings added.
    MIRASMARKET_API int32 FillToCapacity(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, bool bUseEmptyLevels = true);
    MIRASMARKET_API float PlacementCenterX(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FPlanogramPlacement& Placement);

    // Width rules. A level is one fixture + face + shelf level; its blocks must fit in the equipment width.
    MIRASMARKET_API bool IsDoubleSided(const FPlanogramFixture& Fixture);
    MIRASMARKET_API float BlockWidthCm(const FMarketProduct& Product, int32 Facings);
    MIRASMARKET_API float BlockWidthCm(const FMarketProduct& Product, const FPlanogramPlacement& Placement);
    // Used width of a level (blocks + gaps). IgnoreProductId leaves one product out (for "what if" checks).
    MIRASMARKET_API float LevelUsedWidthCm(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
        const FString& FixtureId, const FString& Face, int32 Level, const FString& IgnoreProductId = FString());
    // Would ProductId with Facings fit on this level (its own current block is ignored)?
    MIRASMARKET_API bool FitsOnLevel(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
        const FString& FixtureId, const FString& Face, int32 Level, const FString& ProductId, int32 Facings, float* OutWidthCm = nullptr);
    // Finds a level/face on Fixture where the product fits; tries Facings first, then fewer facings down to 1.
    // Returns false (and leaves Placement unchanged) when no level has room.
    MIRASMARKET_API bool PlaceOnFixture(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
        const FPlanogramFixture& Fixture, FPlanogramPlacement& Placement, int32 PreferredLevel = 0);
    // Human readable warning per overflowing level (ASCII). Empty = everything fits.
    MIRASMARKET_API void FindOverflows(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, TArray<FString>& OutWarnings);
    // Tests a fine-position edit against the shelf edges and other blocks on the same level.
    MIRASMARKET_API bool CanSetOffset(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
        const FPlanogramPlacement& Placement, float NewOffsetCm, FString* OutReason = nullptr);
}

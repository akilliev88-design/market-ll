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
};

struct MIRASMARKET_API FMarketPlanogram
{
    TArray<FPlanogramFixture> Fixtures;
    TArray<FPlanogramPlacement> Placements;

    const FPlanogramFixture* FindFixture(const FString& Id) const;
    FPlanogramFixture* FindFixture(const FString& Id);
    const FPlanogramPlacement* FindPlacement(const FString& ProductId) const;
    FPlanogramPlacement* FindPlacement(const FString& ProductId);
};

namespace MarketPlanogram
{
    constexpr int32 SchemaVersion = 1;
    constexpr float UsableWidthCm = 110.f;
    constexpr float ShelfFrontY = -42.f;
    constexpr float ShelfBaseZ = 19.5f;
    constexpr float ShelfLevelStepZ = 34.f;

    MIRASMARKET_API FString DefaultPath();
    MIRASMARKET_API bool Parse(const FString& Json, FMarketPlanogram& OutPlanogram, TArray<FString>& OutErrors);
    MIRASMARKET_API FString Serialize(const FMarketPlanogram& Planogram);
    MIRASMARKET_API bool LoadFile(const FString& Path, FMarketPlanogram& OutPlanogram, TArray<FString>& OutErrors);
    MIRASMARKET_API bool SaveFile(const FString& Path, const FMarketPlanogram& Planogram, FString& OutError);
    // Keeps author choices and assigns any new active catalog product to a suitable fixture.
    MIRASMARKET_API void Reconcile(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products);
    MIRASMARKET_API float NominalWidthCm(const FMarketProduct& Product);
    MIRASMARKET_API float PlacementCenterX(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FPlanogramPlacement& Placement);
}

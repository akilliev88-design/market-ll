#pragma once
#include "CoreMinimal.h"
#include "Planogram.h"
class UWorld;
struct FStorePoint { FVector At = FVector::ZeroVector; float Yaw = 0; };
struct FStoreProp { FString Mesh; FVector At = FVector::ZeroVector; float Yaw = 0; };
struct FStoreObstacle { FString Id, Kind; FVector At = FVector::ZeroVector, Size = FVector::ZeroVector; FString Shape=TEXT("rectangle"); };
struct FStoreSection { FString Label; FVector At = FVector::ZeroVector; float Yaw = -90, WidthCm = 300; };
struct FStoreStats
{
    double ShelfFrontM = 0, CoolerM = 0, FreezerM = 0, ProduceM2 = 0;
    int32 Checkouts = 0, SelfCheckouts = 0, BackroomPallets = 0;
    TArray<FString> Counters;
};
struct FStoreTemplate
{
    FString Id, Format, Name, Theme, Shell, Roof;
    FVector2D FootprintCm = FVector2D::ZeroVector;
    double SalesAreaM2 = 0, BackroomM2 = 0, CeilingCm = 0;
    FStorePoint Entrance, Receiving, PlayerStart;
    FStorePoint DepotDoor;
    bool bHasDepotDoor=true;
    TArray<FVector> CustomerSpawn;
    FBox Backroom = FBox(ForceInit);
    TArray<FPlanogramFixture> Fixtures;
    TArray<FStoreProp> Props;
    TArray<FVector2D> Outline;
    TArray<FStoreObstacle> Obstacles;
    TArray<FStoreSection> Sections;
    FStoreStats Stats;
    // Editor-authored architecture; old baked shells keep their original appearance.
    bool bEditableShell = false;
    FLinearColor FloorColor = FLinearColor(.62f,.60f,.56f);
    FString FloorFinish = TEXT("tile");
};
namespace MarketStoreKit
{
    MARKETSIM_API bool Parse(const FString& Json, TArray<FStoreTemplate>& Out, TArray<FString>& Errors);
    MARKETSIM_API bool Validate(const FStoreTemplate& Store, TArray<FString>& Errors);
    MARKETSIM_API FStoreStats CalculateStats(const FStoreTemplate& Store);
    MARKETSIM_API bool Load(TArray<FString>& Errors);
    MARKETSIM_API TArray<FString> TemplatesFor(const FString& Format);
    MARKETSIM_API const FStoreTemplate* Find(const FString& Id);
    // Overrides: fixture id = both faces, fixture id/front or fixture id/back = a single face.
    MARKETSIM_API FMarketPlanogram ToPlanogram(const FStoreTemplate& Store, const TMap<FString,FString>& Overrides = {});
    // Uses MarketLayout::Plan per assigned department; preserves refrigeration and face overrides.
    MARKETSIM_API void Fill(FMarketPlanogram& Plan, const TArray<FMarketProduct>& Products);
    // Test dressing only: seeded assortment order, original catalogue/prices remain untouched.
    MARKETSIM_API void FillRandom(FMarketPlanogram& Plan, const TArray<FMarketProduct>& Products, int32 Seed);
    MARKETSIM_API bool Build(UWorld* World, const FStoreTemplate& Store, const FMarketPlanogram& Filled, bool bDesignPreview = false);
    MARKETSIM_API void Clear(UWorld* World);
}

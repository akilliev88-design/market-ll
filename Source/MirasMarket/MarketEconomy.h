#pragma once
#include "CoreMinimal.h"
#include "MarketEconomy.generated.h"

// Static product definition. Loaded from Config/products.json (see ProductCatalog.h).
USTRUCT()
struct FMarketProduct
{
    GENERATED_BODY()
    UPROPERTY() FString Id;
    UPROPERTY() FString RealName;
    UPROPERTY() FString FictionalName;
    UPROPERTY() FString Category;
    UPROPERTY() int64 Cost = 0;
    UPROPERTY() int64 BasePrice = 0;
    UPROPERTY() int32 CaseUnits = 12;
    UPROPERTY() FColor Color = FColor::White;
    // Optional visual produced by the Product Studio. Empty MeshPath = prototype colored boxes.
    UPROPERTY() FString MeshPath;
    // Per material slot override (index = mesh slot). Empty entry = keep the package mesh's own material.
    UPROPERTY() TArray<FString> Materials;
    // Optional correction for an imported custom mesh. Ready packages keep the identity values.
    UPROPERTY() float VisualScale = 1.f;
    UPROPERTY() FRotator VisualRotation = FRotator::ZeroRotator;
    // Placement offset after automatic bottom/center alignment, in centimetres.
    UPROPERTY() FVector VisualOffsetCm = FVector::ZeroVector;
    // Studio-only production data. Inactive products stay in the catalog (preparation list)
    // but are not stocked in the game.
    UPROPERTY() bool bActive = true;
    UPROPERTY() FString Brand;
    UPROPERTY() FString PackageType;     // kutu, pet_sise, cam_sise, teneke, kavanoz, kase, poset
    UPROPERTY() int32 WidthMm = 0;       // kutu/poset: width; others: 0
    UPROPERTY() int32 DepthMm = 0;       // kutu: depth; poset: filled thickness
    UPROPERTY() int32 HeightMm = 0;
    UPROPERTY() int32 DiameterMm = 0;    // round packages
    UPROPERTY() int32 LabelHeightMm = 0; // round packages: printed band height
    UPROPERTY() FString Parts;           // material slots the model must have, e.g. "Etiket,Cam,Kapak"
    UPROPERTY() FString Notes;           // colors/materials for the external agent
    UPROPERTY() bool bSizeEstimated = false;
    UPROPERTY() FString Preset;          // ready package from Config/ambalajlar.json (studio), e.g. "pet_sise_1l"
    UPROPERTY() FString Colors;          // per-part colors, e.g. "Cam=2B1A12/0.85;Kapak=E30613" (hex, optional /opacity)
};

USTRUCT()
struct FMarketStock
{
    GENERATED_BODY()
    UPROPERTY() FString Id;
    UPROPERTY() int32 Shelf = 8;
    UPROPERTY() int32 Warehouse = 24;
    UPROPERTY() int32 Incoming = 0;
    UPROPERTY() int64 Price = 0;
};

// Money uses integer kurus. Inventory is removed only when a checkout succeeds.
USTRUCT()
struct FMarketState
{
    GENERATED_BODY()
    static constexpr int32 ShelfCapacity = 24;
    static constexpr int32 StorageCapacity = 120;

    UPROPERTY() int32 Version = 1;
    UPROPERTY() int32 Day = 1;
    UPROPERTY() int64 Cash = 35000;
    UPROPERTY() TArray<FMarketStock> Stock;
    UPROPERTY() bool bCashier = false;
    UPROPERTY() bool bSecondStore = false;
    UPROPERTY() bool bRealBrands = true;
    UPROPERTY() int32 ProfitableDays = 0;
    UPROPERTY() float MarketShare = 25.f;
    UPROPERTY() int64 Revenue = 0;
    UPROPERTY() int64 CostOfGoods = 0;
    UPROPERTY() int64 LastProfit = 0;
    UPROPERTY() int64 LastRevenue = 0;
    UPROPERTY() int64 LastCostOfGoods = 0;
    UPROPERTY() int64 LastOperatingCost = 0;
    UPROPERTY() int64 LastBranchProfit = 0;
    UPROPERTY() int32 Served = 0;
    UPROPERTY() int32 Lost = 0;
    UPROPERTY() int32 LastServed = 0;
    UPROPERTY() int32 LastLost = 0;

    void Initialize(const TArray<FMarketProduct>& Products);
    bool Order(int32 Index, const TArray<FMarketProduct>& Products);
    int32 Restock(int32 Index);
    bool Sell(int32 Index, int32 Quantity, int64 QuotedPrice, const TArray<FMarketProduct>& Products);
    void CloseDay();
    // Save data is internally consistent (ranges, unique ids). Does not look at the catalog.
    bool IsStructurallyValid() const;
    // Structurally valid AND stock rows match the catalog one-to-one in the same order.
    bool IsValidFor(const TArray<FMarketProduct>& Products) const;
    // Aligns stock rows with the catalog by product id. Products added to the catalog
    // start with empty shelves (they must be ordered); removed products are dropped.
    // Returns the number of added + removed products.
    int32 ReconcileWith(const TArray<FMarketProduct>& Products, TArray<FString>* OutAdded = nullptr, TArray<FString>* OutRemoved = nullptr);
};

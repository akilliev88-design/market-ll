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

// One product's shoppers in one day (MarketDemand.h). Older saves load zeros.
USTRUCT()
struct FMarketDemandStats
{
    GENERATED_BODY()
    UPROPERTY() int32 Sold = 0;        // units sold at the till
    UPROPERTY() int32 Buyers = 0;      // shoppers who paid for it
    UPROPERTY() int32 Empty = 0;       // wanted it, the shelf was empty
    UPROPERTY() int32 Expensive = 0;   // found it too expensive against the rival
    UPROPERTY() int32 NotCarried = 0;  // asked for it, but it is on no shelf
};

USTRUCT()
struct FMarketStock
{
    GENERATED_BODY()
    UPROPERTY() FString Id;
    // A new game starts with empty shelves so the first stocking pass is part of the experience.
    // The inherited opening inventory is kept in the warehouse; loaded saves preserve their values.
    UPROPERTY() int32 Shelf = 0;
    UPROPERTY() int32 Warehouse = 32;
    // Arrived at the rear door but not carried into storage yet.
    UPROPERTY() int32 Dock = 0;
    UPROPERTY() int32 Incoming = 0;
    UPROPERTY() int64 Price = 0;
    // Units that physically fit this product's shelf block (planogram facings x depth).
    // Older saves have no value and load as the v0.1 default.
    UPROPERTY() int32 Capacity = 24;
    // Shoppers of the running day and of the last closed day (the day report reads Yesterday).
    UPROPERTY() FMarketDemandStats Today;
    UPROPERTY() FMarketDemandStats Yesterday;
};

// Money uses integer kurus. Inventory is removed only when a checkout succeeds.
USTRUCT()
struct FMarketState
{
    GENERATED_BODY()
    // Default shelf block size when no planogram capacity is known (v0.1 value).
    static constexpr int32 DefaultShelfCapacity = 24;
    // Sanity bound for save validation only; real capacity comes from the planogram.
    static constexpr int32 MaxShelfCapacity = 5000;
    static constexpr int32 StorageCapacity = 120;
    // Shelf staff (reyon gorevlisi): hired at the office, paid every day like the cashier.
    static constexpr int32 MaxStockers = 3;
    static constexpr int64 StockerHireCost = 12000;
    static constexpr int64 StockerDailyWage = 2000;

    UPROPERTY() int32 Version = 1;
    UPROPERTY() int32 Day = 1;
    UPROPERTY() int64 Cash = 35000;
    UPROPERTY() TArray<FMarketStock> Stock;
    UPROPERTY() bool bCashier = false;
    // Shelf staff count (older saves load 0).
    UPROPERTY() int32 Stockers = 0;
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
    // Part of Lost: shoppers who gave up inside the shop (crowd, till queue, closing time).
    UPROPERTY() int32 LostWaiting = 0;
    UPROPERTY() int32 LastLostWaiting = 0;
    // Last morning's simple supplier event. Damaged/missing units were paid for but never enter stock.
    UPROPERTY() int32 LastDeliveryMissing = 0;
    UPROPERTY() int32 LastDeliveryDamaged = 0;

    void Initialize(const TArray<FMarketProduct>& Products);
    bool Order(int32 Index, const TArray<FMarketProduct>& Products);
    // Atomically submits a multi-product order. Cases is indexed like Products; no money or stock is
    // changed when any line is invalid. Returns the paid bill and ordered units when requested.
    bool SubmitOrder(const TArray<int32>& Cases, const TArray<FMarketProduct>& Products, int64* OutBill = nullptr, int32* OutUnits = nullptr);
    // Rear door -> warehouse. The caller supplies a case-sized limit for visible carrying.
    int32 ReceiveDelivery(int32 Index, int32 MaxUnits = MAX_int32);
    // Warehouse -> shelf, at most MaxUnits (the player moves all that fits, a worker one unit at a time).
    int32 Restock(int32 Index, int32 MaxUnits = MAX_int32);
    bool Sell(int32 Index, int32 Quantity, int64 QuotedPrice, const TArray<FMarketProduct>& Products);
    void CloseDay();
    // Sets each row's shelf capacity (index = catalog order). Units above a smaller capacity go
    // back to the warehouse while storage has room; returns units that did not fit anywhere.
    int32 ApplyShelfCapacities(const TArray<int32>& Capacities);
    // TEST MODE: fills the shelf to capacity without using warehouse stock or cash.
    int32 FillShelfFree(int32 Index);
    // TEST MODE: delivers Units straight to the warehouse (storage limit kept), no cash.
    int32 ReceiveFree(int32 Index, int32 Units);
    int32 DeliveryUnits() const;
    // Save data is internally consistent (ranges, unique ids). Does not look at the catalog.
    bool IsStructurallyValid() const;
    // Structurally valid AND stock rows match the catalog one-to-one in the same order.
    bool IsValidFor(const TArray<FMarketProduct>& Products) const;
    // Aligns stock rows with the catalog by product id. Products added to the catalog
    // start with empty shelves (they must be ordered); removed products are dropped.
    // Returns the number of added + removed products.
    int32 ReconcileWith(const TArray<FMarketProduct>& Products, TArray<FString>* OutAdded = nullptr, TArray<FString>* OutRemoved = nullptr);
};

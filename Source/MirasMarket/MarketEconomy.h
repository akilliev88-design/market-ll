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

// A small, stable neighbourhood pool makes repeat shoppers meaningful without saving world actors.
// Older saves simply start with an empty list.
USTRUCT()
struct FMarketLoyalty
{
    GENERATED_BODY()
    UPROPERTY() int32 CustomerId = INDEX_NONE;
    UPROPERTY() float Satisfaction = 50.f;
    UPROPERTY() int32 Visits = 0;
};

// One checkout line. Runtime-only; SellBasket validates every line before changing money or stock.
struct FMarketSaleLine
{
    int32 Product = INDEX_NONE;
    int32 Quantity = 0;
    int64 QuotedPrice = 0;
};

// One closed day for the statistics and the weekly chart (G-059). Older saves start with an empty history.
USTRUCT()
struct FMarketDayRecord
{
    GENERATED_BODY()
    UPROPERTY() int32 Day = 0;
    UPROPERTY() int64 Revenue = 0;
    UPROPERTY() int64 Profit = 0;
    UPROPERTY() int32 Served = 0;
    UPROPERTY() int32 Lost = 0;
    UPROPERTY() float MarketShare = 0.f;
    UPROPERTY() int64 Cash = 0;
};

// One person on the payroll or in the hiring pool (MarketStaff.h). Skill/Speed/Stamina are learned by the player
// only through the HR manager; Honesty is never shown. Older saves have no employees and are migrated from the
// bCashier/Stockers flags (MarketStaff::Migrate).
USTRUCT()
struct FMarketEmployee
{
    GENERATED_BODY()
    UPROPERTY() int32 Id = 0;
    UPROPERTY() FString Name;
    UPROPERTY() uint8 Role = 0;            // MarketStaff::ERole
    UPROPERTY() int32 Skill = 50;          // 0..100: fewer till errors, faster checkout, may re-plan shelves
    UPROPERTY() int32 Speed = 50;          // 0..100: walking and hand speed
    UPROPERTY() int32 Stamina = 50;        // 0..100: how fast fatigue builds up
    UPROPERTY() int32 Honesty = 70;        // 0..100, hidden: low values can mean small till shortages
    UPROPERTY() int64 DailyWage = 2000;    // kurus per day
    UPROPERTY() float Morale = 70.f;       // 0..100
    UPROPERTY() float Fatigue = 0.f;       // 0..100
    UPROPERTY() int32 HiredDay = 0;
    UPROPERTY() int32 DaysWorked = 0;
    UPROPERTY() int32 OffDay = 0;          // day number the employee has off (0 = none)
    UPROPERTY() int32 LowMoraleDays = 0;
    UPROPERTY() int32 LeaveDay = 0;        // gave notice: leaves at the close of this day (0 = staying)
    UPROPERTY() int32 WarnedDay = 0;       // last warning about the till
    UPROPERTY() int32 FlaggedWeek = 0;     // accountant already reported this person's till in this week
    UPROPERTY() int32 WorkToday = 0;       // units shelved today (stockers; the world reports them)
    UPROPERTY() TArray<int64> RecentTill;  // cashiers: till difference of the last worked days, newest last (max 7)
};

// One wholesaler relationship (MarketSuppliers.h). Older saves start with no accounts (created on first use).
USTRUCT()
struct FMarketSupplierAccount
{
    GENERATED_BODY()
    UPROPERTY() uint8 Supplier = 0;     // MarketSuppliers::ESupplier
    UPROPERTY() int32 Trust = 40;       // 0..100: payment terms and support grow with it
    UPROPERTY() int64 Volume30 = 0;     // purchases of about the last 30 days (decays by 1/30 a day), kurus
    UPROPERTY() int32 OnTime = 0;
    UPROPERTY() int32 Late = 0;
};

// A bill bought on payment terms, due at the close of DueDay.
USTRUCT()
struct FMarketPayable
{
    GENERATED_BODY()
    UPROPERTY() uint8 Supplier = 0;
    UPROPERTY() int64 Amount = 0;
    UPROPERTY() int32 DueDay = 0;
};

// A promotion the player runs (MarketPromotions.h). Also used for the wholesaler's pending offer.
USTRUCT()
struct FMarketPromotion
{
    GENERATED_BODY()
    UPROPERTY() uint8 Kind = 0;          // MarketPromotions::EKind
    UPROPERTY() int32 Product = INDEX_NONE;
    UPROPERTY() FString Category;        // aisle promotions
    UPROPERTY() int32 Percent = 0;
    UPROPERTY() int32 StartDay = 0;      // 0 = not started (an offer)
    UPROPERTY() int32 EndDay = 0;        // last day in effect
    UPROPERTY() int64 Cost = 0;          // money paid for it (flyers)
    UPROPERTY() int32 Sold = 0;          // units of the promoted products sold while it ran
    UPROPERTY() int32 Baseline = 0;      // units the same days sold before (the report compares)
    UPROPERTY() int64 MarginLost = 0;    // price given away, kurus
};

// The accountant's books for the running tax period (one week) and the declared tax (MarketStaff.h).
USTRUCT()
struct FMarketBooks
{
    GENERATED_BODY()
    UPROPERTY() int64 PeriodSales = 0;
    UPROPERTY() int64 PeriodPurchases = 0;
    UPROPERTY() int64 PeriodProfit = 0;
    UPROPERTY() int64 VatCarry = 0;        // input VAT larger than output VAT carried into the next period
    UPROPERTY() int64 TaxDue = 0;          // declared and unpaid (penalties included)
    UPROPERTY() int64 TaxDeclared = 0;     // the last declaration without penalties (late penalty base)
    UPROPERTY() int32 TaxDueDay = 0;       // must be paid by the close of this day
    UPROPERTY() int64 PenaltyThisTax = 0;  // late penalty already added to the last declaration
    UPROPERTY() int64 TotalTaxPaid = 0;
    UPROPERTY() int64 TotalPenalties = 0;
    UPROPERTY() int32 Audits = 0;
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
    // Inherited debt to the wholesaler (G-054, MarketCampaign.h). Paid at the office desk (P); the second branch
    // waits until it is 0. There is no deadline: the story goes on until the shop stands on its own feet.
    UPROPERTY() int64 InheritedDebt = 30000;
    UPROPERTY() int32 DebtClearedDay = 0;   // day the debt was closed (0 = still open)
    // Running week (days 1-7, 8-14, ...) and the last finished week, for the weekly report.
    UPROPERTY() int64 WeekRevenue = 0;
    UPROPERTY() int64 WeekProfit = 0;
    UPROPERTY() int64 WeekDebtPaid = 0;
    UPROPERTY() int32 WeekServed = 0;
    UPROPERTY() int32 WeekLost = 0;
    UPROPERTY() int32 LastWeekNumber = 0;   // 0 = no week finished yet
    UPROPERTY() int64 LastWeekRevenue = 0;
    UPROPERTY() int64 LastWeekProfit = 0;
    UPROPERTY() int64 LastWeekDebtPaid = 0;
    UPROPERTY() int32 LastWeekServed = 0;
    UPROPERTY() int32 LastWeekLost = 0;
    // Seed of the rival shops' news (MarketRivals.h); set for each new campaign, so a reload cannot reroll it.
    UPROPERTY() int32 RivalSeed = 0;
    // Every closed day, oldest first (MarketCampaign::CloseDay). Kept for ten game years at most.
    UPROPERTY() TArray<FMarketDayRecord> History;
    UPROPERTY() TArray<FMarketLoyalty> Loyalty;
    // People (MarketStaff.h). bCashier/Stockers above are kept in step with the roster by MarketStaff::SyncCounts:
    // they say who is ON DUTY today, which is what the world (till, walking workers) needs.
    UPROPERTY() TArray<FMarketEmployee> Staff;
    UPROPERTY() TArray<FMarketEmployee> Candidates;
    UPROPERTY() int32 NextEmployeeId = 1;
    UPROPERTY() int32 CandidatesDay = 0;    // day the hiring pool was last refreshed (0 = never)
    UPROPERTY() bool bHrAutoReplace = true; // HR manager hires a replacement when someone leaves
    UPROPERTY() FMarketBooks Books;
    // Goods bought from the wholesaler today / on the last closed day (the accountant's input VAT).
    UPROPERTY() int64 Purchases = 0;
    UPROPERTY() int64 LastPurchases = 0;
    // What the staff and the books did at the last day close (day report).
    UPROPERTY() int64 LastTillDifference = 0;
    UPROPERTY() int64 LastTaxPaid = 0;
    UPROPERTY() int64 LastPenalty = 0;
    UPROPERTY() TArray<FString> StaffNews;
    // Wholesale (MarketSuppliers.h): the chosen wholesaler, relationships, bills on terms and the list-price level
    // the shelf prices were last raised to (the "zam" the player has passed on).
    UPROPERTY() uint8 Supplier = 0;
    UPROPERTY() TArray<FMarketSupplierAccount> SupplierAccounts;
    UPROPERTY() TArray<FMarketPayable> Payables;
    UPROPERTY() double ShelfPriceLevel = 1.0;
    // Promotions (MarketPromotions.h): running and finished-but-not-reported ones, the wholesaler's offer, and the
    // marketing money spent today (paid at the day close with the other operating costs).
    UPROPERTY() TArray<FMarketPromotion> Promotions;
    UPROPERTY() FMarketPromotion Offer;
    UPROPERTY() int64 Marketing = 0;
    // Evening report lines of the background systems (MarketDirector clears it at every day close).
    UPROPERTY() TArray<FString> DayNews;

    // Wages of everyone on the payroll (paid days off included). Staff empty = the v0.1 flags (older saves, tests).
    int64 DailyPayroll() const;

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
    // Atomically sells a whole shopper basket and counts it as one served customer.
    bool SellBasket(const TArray<FMarketSaleLine>& Lines, const TArray<FMarketProduct>& Products, int64* OutReceipt = nullptr, int32* OutUnits = nullptr);
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

#pragma once
#include "CoreMinimal.h"
// ===== Ak\u0131\u015f B =====
#include "MarketLedger.h"
#include "MarketEras.h"
#include "MarketGoals.h"
// ===== Ak\u0131\u015f B son =====
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
    // G-078: retail data (products.json "retail", all optional; 0 / -1 = use the category's default).
    UPROPERTY() FString Subcategory;     // e.g. "gazl\u0131", "su", "\u00e7ay": substitution and campaign scope
    UPROPERTY() float Kvi = 0.f;         // 0..1: how well shoppers know its price (bread, milk, tea = 1)
    UPROPERTY() float VatRate = -1.f;    // e.g. 0.08 food, 0.18 cleaning (at the start)
    UPROPERTY() int32 ShelfLifeDays = 0; // days a batch can be sold; 0 = group default (MarketGoods)
    UPROPERTY() float Elasticity = 0.f;  // price/promotion sensitivity (1.5 staples .. 4 snacks); 0 = group default
    UPROPERTY() float Stockpile = -1.f;  // how much shoppers stock up on a deal (0.2 milk .. 1.5 detergent)
    UPROPERTY() float TrafficPull = 0.f; // 0..1: a deal on it brings people into the shop (loss leader)
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
    // No value: the default.
    UPROPERTY() int32 Capacity = 24;
    // Shoppers of the running day and of the last closed day (the day report reads Yesterday).
    UPROPERTY() FMarketDemandStats Today;
    UPROPERTY() FMarketDemandStats Yesterday;
    // New units that came into the shop since the last freshness check (deliveries, test fills, a closed
    // branch's goods). MarketFreshness turns them into a new batch; everything else that left came from old batches.
    UPROPERTY() int32 Received = 0;
    // G-078 (#26): weighted average purchase cost of the units held (shelf, depot, dock, on the way), in kurus.
    // The cost of goods sold and the waste use it, so a price rise or a cheaper deal changes only new purchases.
    // 0 = unknown (the inherited stock): today's catalog cost is used until the first purchase.
    UPROPERTY() int64 AvgCost = 0;
    // G-078 shopper memory (MarketPromotions): how often it has been on a deal lately (decaying day count; many
    // deals teach shoppers to wait for the next one) and what they stocked up at home on the last deal (the
    // after-promotion dip).
    UPROPERTY() float PromoHeat = 0.f;
    UPROPERTY() int32 IdleDays = 0;          // M38: days in a row it was on hand and nobody bought it (the manager's clearance)
    UPROPERTY() float Pantry = 0.f;
    // M33: a store manager's clearance (a slow item marked down for a week; branches).
    UPROPERTY() uint8 Markdown = 0;          // percent off
    UPROPERTY() int32 MarkdownUntil = 0;

    // E3c (11_TEK_EKONOMI): one product record for every store. The first store's rows start with the inherited
    // stock in the back room; a branch's rows start empty and keep all their goods on the shelves (Shelf; the
    // branch has no separate back room), yesterday's sales and empty-shelf wishes in Yesterday.
    static FMarketStock Empty(const FString& InId = FString())
    {
        FMarketStock Row;
        Row.Id = InId;
        Row.Warehouse = 0;
        Row.Capacity = 0;
        return Row;
    }
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
// only through the HR manager; Honesty is never shown.
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
    UPROPERTY() int32 LifelineDay = 0;  // C9: the last day the old wholesaler gave goods in a cash crisis
    UPROPERTY() int32 LifelineUses = 0; // M64: times the shop's old standing opened the door (MarketSuppliers::LifelineMax in a campaign)
};

// A bill bought on payment terms, due at the close of DueDay.
USTRUCT()
struct FMarketPayable
{
    GENERATED_BODY()
    UPROPERTY() uint8 Supplier = 0;
    UPROPERTY() int64 Amount = 0;
    UPROPERTY() int32 DueDay = 0;
    // Day the bill first went unpaid (0 = not late). The due day itself never moves, so the bill stays overdue.
    UPROPERTY() int32 LateSince = 0;
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
    // G-078 (karar J07): scoped campaigns (Kind = MarketPromotions::EKind::Scoped). Older saves load 0.
    UPROPERTY() uint8 Scope = 0;         // MarketPromotions::EScope
    UPROPERTY() FString ScopeKey;        // brand / subcategory / category name the campaign covers
    UPROPERTY() uint8 Mechanic = 0;      // MarketPromotions::EMechanic
    // M38: every store has the same campaigns. Store: MarketLedger::FirstStore (-1), a branch index, or
    // MarketLedger::AllStores (the company's campaign in every store). bManager: the store manager started it.
    UPROPERTY() int32 Store = -1;
    UPROPERTY() bool bManager = false;
};

// A choice waiting for the player (MarketEvents.h): story scenes and neighbourhood events.
USTRUCT()
struct FMarketDecision
{
    GENERATED_BODY()
    UPROPERTY() FString Id;              // "event.fridge", "story.sell", ...
    UPROPERTY() FString Title;
    UPROPERTY() FString Text;
    UPROPERTY() TArray<FString> Options;
    UPROPERTY() int32 Deadline = 0;      // decided automatically (DefaultOption) at the close of this day
    UPROPERTY() int32 DefaultOption = 0;
    UPROPERTY() int32 Arg = 0;           // event data (product, amount, ...)
};

// A lasting effect of an event, the story or the shop's identity (MarketEvents.h).
USTRUCT()
struct FMarketModifier
{
    GENERATED_BODY()
    UPROPERTY() uint8 Kind = 0;          // MarketEvents::EModifier
    UPROPERTY() uint8 Group = 255;       // MarketGoods::EGroup, 255 = every group
    UPROPERTY() float Value = 1.f;       // factor (traffic, interest, cost) or amount (price tolerance)
    UPROPERTY() int32 FirstDay = 0;
    UPROPERTY() int32 LastDay = 0;
    UPROPERTY() FString Source;
};

// The company's own history (MarketStory.h; M69: no chapters, no finale).
USTRUCT()
struct FMarketStoryState
{
    GENERATED_BODY()
    UPROPERTY() int64 Beats = 0;             // bit flags of milestones already written
    UPROPERTY() uint8 Identity = 0;          // MarketStory::EIdentity (0 = not chosen)
    // Karar J03: the player sold the shop and chose "Burada bitsin": the campaign is over (new game only).
    UPROPERTY() bool bCampaignOver = false;
    UPROPERTY() TArray<FString> Memories;    // milestones, newest last ("12 Mart, 1. y\u0131l: ilk k\u00e2rl\u0131 g\u00fcn")
};

// Units of one product that arrived together and spoil together (MarketFreshness.h).
USTRUCT()
struct FMarketBatch
{
    GENERATED_BODY()
    UPROPERTY() FString ProductId;
    UPROPERTY() int32 Units = 0;
    UPROPERTY() int32 ExpiresDay = 0;    // last day it may be sold
};

// Karar M37: the owner's personal money, apart from the company's (MarketOwner.h).
USTRUCT()
struct FMarketOwner
{
    GENERATED_BODY()
    UPROPERTY() int32 SalaryX10 = 15;       // monthly salary: tenths of the minimum wage
    UPROPERTY() int64 Wealth = 0;           // personal money, kurus
    UPROPERTY() int64 LastNet = 0;          // last month's net salary (0 = not paid)
    UPROPERTY() int64 LastLiving = 0;
    UPROPERTY() int64 YearSalary = 0;       // net, this calendar year
    UPROPERTY() int64 YearDividends = 0;    // net, this calendar year
    UPROPERTY() int64 TotalSalary = 0;      // net, the whole campaign
    UPROPERTY() int64 TotalDividends = 0;
    UPROPERTY() int64 CapitalIn = 0;        // personal money put into the company
    UPROPERTY() int32 DividendYear = 0;     // the closed year the dividends below were paid from
    UPROPERTY() int64 DividendPaid = 0;     // gross, from DividendYear's profit
    UPROPERTY() int32 MissedSalaries = 0;   // months the till could not pay us
};

// A bank loan (MarketFinance.h).
USTRUCT()
struct FMarketLoan
{
    GENERATED_BODY()
    UPROPERTY() int64 Principal = 0;
    UPROPERTY() int64 Remaining = 0;     // principal still owed
    UPROPERTY() float MonthlyRate = 0.f;
    UPROPERTY() int64 Installment = 0;
    UPROPERTY() int32 NextDueDay = 0;
    UPROPERTY() bool bMortgage = false;  // the first store's deed stands behind it
    UPROPERTY() int32 LateSince = 0;     // C7: first missed day (0 = on time); a late fee once a month, not every day
};

// A branch of the company, simulated from the same rules without walking customers (MarketBranches.h).
// Karar M26: a department running in a branch (MarketDepartments.h).
USTRUCT()
struct FMarketBranchDept
{
    GENERATED_BODY()
    UPROPERTY() uint8 Dept = 0;          // MarketDepartments::EDept
    UPROPERTY() int32 OpenedDay = 0;
    UPROPERTY() int32 Master = 0;        // the master's skill (0 = the department needs none)
    UPROPERTY() int64 Stock = 0;         // goods on hand at cost, kurus
    UPROPERTY() int64 Last30Revenue = 0; // running sums over about 30 days
    UPROPERTY() int64 Last30Profit = 0;
};

USTRUCT()
struct FMarketBranch
{
    GENERATED_BODY()
    UPROPERTY() FString Name;
    // G-086: the province the branch is in (MarketCountry) and its country. Older saves: empty = the home province.
    UPROPERTY() FString Province;
    UPROPERTY() FString Country;
    UPROPERTY() FString Format;          // kucuk (ucuzcu), mahalle, buyuk (supermarket), hiper (MarketLayout::Fixtures)
    UPROPERTY() uint8 Stage = 0;         // MarketBranches::EStage
    UPROPERTY() int32 StageUntil = 0;
    UPROPERTY() int32 OpenedDay = 0;
    UPROPERTY() int64 Rent = 0;          // per month, kurus at the time of signing
    UPROPERTY() int32 Workers = 0;        // the positions the store needs (MarketStoreAssign::WorkersFor)
    UPROPERTY() TArray<FMarketEmployee> Staff; // E3c2c (M63): the people in those positions (cashiers and stockers), run by the manager
    UPROPERTY() FString ManagerName;
    UPROPERTY() int32 ManagerSkill = 0;  // 0 = no manager: the player's standing orders
    UPROPERTY() int32 ManagerHonesty = 70;
    UPROPERTY() int64 ManagerWage = 0;
    UPROPERTY() float PriceIndex = 1.f;  // shelf prices / list price
    UPROPERTY() float Maturity = 0.f;    // 0..1: the district's habit of shopping here
    UPROPERTY() float Satisfaction = 55.f;
    UPROPERTY() TArray<FMarketStock> Items; // E3c: the same product record as the first store (FMarketStock::Empty)
    UPROPERTY() TArray<FMarketBatch> Batches; // E3c2 (M63): its perishable goods in batches, the first store's rule (MarketFreshness)
    UPROPERTY() int64 LastRevenue = 0;
    UPROPERTY() int64 LastProfit = 0;
    UPROPERTY() int32 LastShoppers = 0;
    UPROPERTY() int64 WeekProfit = 0;
    UPROPERTY() int64 Last30Profit = 0;  // running sum over about 30 days
    // G-086b (Docs/Kurgu/03_MAGAZA_AGI.md \u00a74.1, MarketManagers.h): the manager's hidden style (MarketManagers::EStyle;
    // 0 = not set yet, seeded once), morale (-1 = not set yet), warnings, the day they took over (0 = long ago:
    // no settling-in week), weekly marks in a row, the last bonus and warning, the day the till was found short.
    UPROPERTY() uint8 ManagerStyle = 0;
    UPROPERTY() float ManagerMorale = -1.f;
    UPROPERTY() int32 ManagerWarnings = 0;
    UPROPERTY() int32 ManagerSince = 0;
    UPROPERTY() int32 ManagerBadWeeks = 0;
    UPROPERTY() int32 ManagerGoodWeeks = 0;
    UPROPERTY() int32 ManagerBonusDay = 0;
    UPROPERTY() int32 ManagerWarnedDay = 0;
    UPROPERTY() int32 ManagerCaughtDay = 0;
    // G-086b ek (M21): the manager's hidden ceiling of skill (55..95). 0 = derived from the skill (MarketManagers::PotentialOf)
    // derives it once from the skill (+5..20, at most 95).
    UPROPERTY() int32 ManagerPotential = 0;
    // G-088 stage C (MarketStoreViews.h): the ready-made store view signed for this branch and a copy of its
    // measured size (lengths in metres, areas in m2). Empty view: the format's nominal store.
    UPROPERTY() FString StoreView;
    UPROPERTY() float ViewShelfM = 0.f;
    UPROPERTY() float ViewCoolerM = 0.f;
    UPROPERTY() float ViewFreezerM = 0.f;
    UPROPERTY() float ViewProduceM2 = 0.f;
    UPROPERTY() float ViewAreaM2 = 0.f;
    UPROPERTY() int32 ViewCounters = 0;
    UPROPERTY() int32 ViewCheckouts = 0;
    UPROPERTY() int32 ViewSelfCheckouts = 0;
    UPROPERTY() int32 ViewPallets = 0;
    UPROPERTY() int32 LastQueueLost = 0;   // shoppers the tills lost on the last closed day
    UPROPERTY() int64 Last30Revenue = 0;   // running sum over about 30 days (national table, world league)
    UPROPERTY() TArray<FMarketBranchDept> Depts; // karar M26: its departments (MarketDepartments.h)
    UPROPERTY() int32 VisitedDay = 0;      // C3: the player last walked through it (MarketBranches::Visit)
    UPROPERTY() int32 LossMonths = 0;      // M33: months in a row in the red after its first three months
    UPROPERTY() int32 QuietUntil = 0;      // M33: a turned-down closing proposal: none before this day
    UPROPERTY() int32 SignedDay = 0;       // C15 (M45): the lease was signed (MarketBranches::Open); acquired shops 0
    UPROPERTY() uint8 bHasty = 0;          // C15 (M45): picked while growth outran management: a weaker site, found out at the opening
    // D9b (M46, MarketPortfolio.h): the store's portfolio. RenewedDay: the last renovation, relocation or change of
    // type was finished (0: never; its age counts from OpenedDay). Works: the job under way while Stage is
    // Renovation again (0 none = a new opening, 1 renovation, 2 change of type, 3 relocation) and the type it
    // becomes. The year's running sums for the yearly report card, and the last card (1 A .. 5 E, 0 none).
    UPROPERTY() int32 RenewedDay = 0;
    UPROPERTY() uint8 Works = 0;
    UPROPERTY() FString WorksFormat;
    UPROPERTY() int64 YearRevenue = 0;
    UPROPERTY() int64 YearProfit = 0;
    UPROPERTY() int32 YearDays = 0;
    UPROPERTY() uint8 Card = 0;
    UPROPERTY() int32 CardYear = 0;
    UPROPERTY() int64 CardRevenue = 0;
    UPROPERTY() int64 CardProfit = 0;
};

// G-089 (karar M23): a big depot in a province (MarketDepots.h). It serves our branches of its country within
// 600 km; its manager is an FMarketManager of level MarketManagers::ELevel::Depot with the same country and
// province.
USTRUCT()
struct FMarketDepot
{
    GENERATED_BODY()
    UPROPERTY() FString Country;            // pack id
    UPROPERTY() FString Province;           // MarketCountry::FCity id
    UPROPERTY() int32 OpenedDay = 0;
    UPROPERTY() int64 Rent = 0;             // per month, start-level kurus (x the province's rent; paid with the list level)
    UPROPERTY() int32 Capacity = 60;        // branches it serves well; beyond that it works slower
    UPROPERTY() int64 WeekLoss = 0;         // goods short or broken on the way this week (at cost)
    UPROPERTY() int64 WeekSkim = 0;         // goods a dishonest depot manager took this week (at cost)
    UPROPERTY() FString CaughtName;         // the depot manager caught taking goods (until he is replaced)
};

// The company beyond the first store (MarketCompany.h). Older saves: nothing built.
// M65 (Mustafa 03.10.2026): our company in a country. The campaign's own country holds the parent company; every
// other country we enter gets a subsidiary with its own registered name and its own accounts (its stores'
// statements), whose month profit goes to the parent (MarketSubsidiaries.h).
USTRUCT()
struct FMarketSubsidiary
{
    GENERATED_BODY()
    UPROPERTY() FString Country;
    UPROPERTY() FString LegalName;          // the registered name ("Brand Handels GmbH"); the player may change it
    UPROPERTY() int32 FoundedDay = 0;
    UPROPERTY() int64 LastMonthProfit = 0;  // its stores' net result of the last closed month, our money
    UPROPERTY() int64 LastTransfer = 0;     // what reached the parent after the withholding tax
    UPROPERTY() int64 LastWithheld = 0;
    UPROPERTY() int64 TotalTransferred = 0;
};

// M58: a market study of a country before entering it (MarketResearch.h).
USTRUCT()
struct FMarketResearch
{
    GENERATED_BODY()
    UPROPERTY() FString Country;
    UPROPERTY() int32 StartDay = 0;
    UPROPERTY() int32 ReadyDay = 0;         // the report comes at this day's close; valid for a year after it
    UPROPERTY() int64 Cost = 0;
};

// C16/D9 (M48-M50): province pushes, strategic forks, paths (MarketStrategy.h).
USTRUCT()
struct FMarketPush
{
    GENERATED_BODY()
    UPROPERTY() FString Country;
    UPROPERTY() FString Province;
    UPROPERTY() int32 StartDay = 0;
    UPROPERTY() int32 EndDay = 0;            // last day of the campaign
    UPROPERTY() bool bEnded = false;         // its end was told and its effects drawn
    UPROPERTY() int32 Withdrawn = 0;         // rival shops that gave up at the end
};

USTRUCT()
struct FMarketStrategyState
{
    GENERATED_BODY()
    UPROPERTY() uint8 Focus = 0;             // MarketStrategy::EFocus
    UPROPERTY() uint8 Growth = 0;            // MarketStrategy::EGrowth
    UPROPERTY() uint8 Vertical = 0;          // MarketStrategy::EVertical
    UPROPERTY() int32 FocusDay = 0;
    UPROPERTY() int32 GrowthDay = 0;
    UPROPERTY() int32 VerticalDay = 0;
    UPROPERTY() int32 AskFocusDay = 0;       // "not now": the fork is asked again from this day
    UPROPERTY() int32 AskGrowthDay = 0;
    UPROPERTY() int32 AskVerticalDay = 0;
    UPROPERTY() TArray<FMarketPush> Pushes;
    UPROPERTY() TArray<uint8> Tiers;         // MarketStrategy::EPath order, at the last monthly look
    UPROPERTY() TArray<uint8> BestTiers;
    UPROPERTY() TArray<FString> Champions;   // "country|province" at the last monthly look
    UPROPERTY() int32 LastLookDay = 0;
    UPROPERTY() int32 LoyaltyDay = 0;        // the loyalty programme was last paid on this day
    UPROPERTY() int64 LoyaltyPaid = 0;       // counters for the reports
    UPROPERTY() int64 PushPaid = 0;
};

// D9b (M47, MarketResponse.h): an answer to a rival's move or a crisis, decided by a manager or the player, and its
// effect until EndDay. Kind: MarketResponse::EKind; Option: the answer's index in that kind's options.
USTRUCT()
struct FMarketResponse
{
    GENERATED_BODY()
    UPROPERTY() uint8 Kind = 0;
    UPROPERTY() FString Country;             // pack id
    UPROPERTY() FString Province;            // empty: company-wide (a crisis)
    UPROPERTY() FString Rival;               // the chain's name ("" for a crisis)
    UPROPERTY() uint8 Option = 0;
    UPROPERTY() FString Decider;             // "Tekirda\u011f il m\u00fcd\u00fcr\u00fc Ay\u015fe Kaya" / "sen"
    UPROPERTY() bool bPlayer = false;        // the player approved or chose it
    UPROPERTY() bool bProposed = false;      // a manager proposed it and the player had the last word
    UPROPERTY() int32 Day = 0;
    UPROPERTY() int32 EndDay = 0;
    UPROPERTY() int64 Spent = 0;             // what the answer cost so far (kurus)
};

USTRUCT()
struct FMarketResponses
{
    GENERATED_BODY()
    UPROPERTY() TArray<FMarketResponse> Log;  // newest last; the oldest go beyond MarketResponse::LogSize
    UPROPERTY() TMap<FString, int32> RivalSeen; // "country|province" -> the chains' stores there at the last look
    UPROPERTY() TMap<FString, int32> WarSeen;   // "chain id|province" -> the WarUntil already answered
    UPROPERTY() int32 EraSeen = -1;           // the crisis already answered (era kind x 10 + wave)
    UPROPERTY() int32 Answered = 0;           // every answer so far
    UPROPERTY() int32 ByPlayer = 0;           // of them, the player's
};

// D6 (M67): a master franchise in a country: a local partner opens stores under our brand and pays a royalty
// (MarketFranchise.h).
USTRUCT()
struct FMarketFranchise
{
    GENERATED_BODY()
    UPROPERTY() FString Country;
    UPROPERTY() FString Partner;            // the partner company's name ("Schmidt Markt")
    UPROPERTY() int32 StartDay = 0;
    UPROPERTY() int32 Stores = 0;           // partner stores under our brand
    UPROPERTY() int32 Quality = 100;        // the partner's hand, 80..120 (% of a typical store's sales)
    UPROPERTY() int64 MonthSales = 0;       // the running month's sales of the partner stores (home-level kurus)
    UPROPERTY() int64 Savings = 0;          // the partner's profit kept for its next store (home-level kurus)
    UPROPERTY() int64 LastRoyalty = 0;      // last month's royalty (our money, before withholding)
    UPROPERTY() int64 TotalRoyalty = 0;
    UPROPERTY() bool bEnded = false;
    UPROPERTY() int32 EndDay = 0;
};

USTRUCT()
struct FMarketCompany
{
    GENERATED_BODY()
    UPROPERTY() FString BrandName;          // M65/M69: the market's name, chosen at the start (signs, news, lists)
    UPROPERTY() TArray<FMarketSubsidiary> Subsidiaries; // M65: the parent (own country) and one per country entered
    UPROPERTY() TArray<FMarketResearch> Research;        // M58: market studies of countries not entered yet
    UPROPERTY() TArray<FMarketFranchise> Franchises;     // D6 (M67): partners abroad under our brand
    UPROPERTY() TArray<FMarketDepot> DepotSites; // G-089: depots in provinces (MarketDepots.h)
    UPROPERTY() int32 Trucks = 0;
    UPROPERTY() bool bCentralBuying = false;  // buying for all stores at once
    UPROPERTY() bool bPrivateLabel = false;   // the player's own brand
    UPROPERTY() int64 LastProfit = 0;         // all city stores + head office, last closed day
    UPROPERTY() int64 WeekProfit = 0;
};

// G-086b: a manager above the shops (province / sub-region / main region / country) or the first store's manager
// (MarketManagers.h, Docs/Kurgu/03_MAGAZA_AGI.md \u00a74). Store managers of branches live in FMarketBranch.
USTRUCT()
struct FMarketManager
{
    GENERATED_BODY()
    UPROPERTY() uint8 Level = 1;            // MarketManagers::ELevel
    UPROPERTY() FString Country;            // pack id
    UPROPERTY() FString Area;               // province / sub-region / main region id; the country id; first store: home province
    UPROPERTY() FString Name;
    UPROPERTY() int32 Skill = 50;
    UPROPERTY() int32 Honesty = 70;
    UPROPERTY() int64 BaseWage = 0;         // per day, start-level kurus (paid as MarketPrices::WageScaled)
    UPROPERTY() float Morale = 70.f;
    UPROPERTY() int32 Warnings = 0;
    UPROPERTY() int32 AppointedDay = 0;
    UPROPERTY() int32 BonusDay = 0;
    UPROPERTY() int32 WarnedDay = 0;
    UPROPERTY() bool bPromoted = false;     // came up from a store manager
    // G-086b ek (M19, M21): hidden style (MarketManagers::EStyle; the first store's manager runs the shop by it;
    // 0 = not set yet, seeded once) and hidden ceiling of skill (55..95; 0 = not set yet, derived once).
    UPROPERTY() uint8 Style = 0;
    UPROPERTY() int32 Potential = 0;
};

// G-086b: the management hierarchy (MarketManagers.h). Older saves: nobody appointed.
USTRUCT()
struct FMarketManagement
{
    GENERATED_BODY()
    UPROPERTY() TArray<FMarketManager> Managers;
    UPROPERTY() int32 Hires = 0;            // outside candidates taken so far (seeds the next one)
    UPROPERTY() int64 LastWages = 0;        // managers' wages of the last closed day
    UPROPERTY() int64 WeekWages = 0;
    UPROPERTY() int32 MissingCountryDays = 0; // days a required country manager was missing in a row
    // G-086b ek (M22): names of managers hired or candidates turned down in this campaign; a candidate never
    // comes back with one of them.
    UPROPERTY() TArray<FString> UsedNames;
};

// M34 (Mustafa 01.10.2026): the company's advertising in a country (MarketAdvertising.h).
USTRUCT()
struct FMarketAdCountry
{
    GENERATED_BODY()
    UPROPERTY() FString Country;
    UPROPERTY() TArray<uint8> Levels;        // per MarketAdvertising::EChannel: 0 off .. 3 heavy
    UPROPERTY() TArray<float> Stock;         // what each channel left in people's minds (decays every month)
    UPROPERTY() int64 MonthSpend = 0;
    UPROPERTY() int64 PrevSpend = 0;
    UPROPERTY() int64 MonthUplift = 0;       // the shops' revenue the ads brought (estimate)
    UPROPERTY() int64 PrevUplift = 0;
    UPROPERTY() int64 MonthRevenue = 0;      // our shops' revenue in the country (the manager's budget base)
    UPROPERTY() int64 PrevRevenue = 0;
    UPROPERTY() TArray<int64> MonthChannelSpend;
    UPROPERTY() TArray<int64> PrevChannelSpend;
    UPROPERTY() int32 SeasonUntil = 0;       // the advertising manager's holiday push
};

USTRUCT()
struct FMarketAdvertising
{
    GENERATED_BODY()
    UPROPERTY() TArray<FMarketAdCountry> Countries;
    UPROPERTY() FString ManagerName;
    UPROPERTY() int32 ManagerSkill = 0;
    UPROPERTY() int64 ManagerWage = 0;
    UPROPERTY() int32 ManagerSince = 0;
    UPROPERTY() bool bAuto = false;          // the manager sets the mix every month
    UPROPERTY() int32 BudgetPermille = 20;   // his budget: per mille of the country's last month revenue
    UPROPERTY() int32 Told = 0;
};

// M32 (Mustafa 01.10.2026): online selling of the whole company, per province (MarketOnline.h).
USTRUCT()
struct FMarketOnlineArea
{
    GENERATED_BODY()
    UPROPERTY() FString Country;
    UPROPERTY() FString Province;
    UPROPERTY() bool bPlatform = false;      // the platform's couriers take our shops' orders here
    UPROPERTY() bool bOwn = false;           // our web site / app delivers from our shops here
    UPROPERTY() bool bQuick = false;         // our 30-minute delivery from the province's dark store
    UPROPERTY() bool bPlayerSet = false;     // the player decided; else the province manager (or the company default)
    UPROPERTY() int32 DarkStoreDay = 0;      // 0 = no dark store here
    UPROPERTY() int32 LastOrders = 0;
    UPROPERTY() int32 Orders30 = 0;          // x 29/30 a day + today
    UPROPERTY() int64 Revenue30 = 0;
    UPROPERTY() int64 Profit30 = 0;
    // The province manager's month review (MarketOnline): proposals go up the line, we approve.
    UPROPERTY() int64 MonthPlatformProfit = 0;
    UPROPERTY() int64 MonthOwnProfit = 0;
    UPROPERTY() int32 PlatformLossMonths = 0;
    UPROPERTY() int32 OwnLossMonths = 0;
    UPROPERTY() int32 QuietUntil = 0;        // a turned-down proposal: no new one before this day
    UPROPERTY() int32 PlatformDropDay = 0;   // C7: the platform / own delivery was closed here for losses (no
    UPROPERTY() int32 OwnDropDay = 0;        // proposal to bring it back for a year)
};

USTRUCT()
struct FMarketOnline
{
    GENERATED_BODY()
    // Company channels (MarketOnline::EChannel).
    UPROPERTY() bool bWeb = false;
    UPROPERTY() int32 WebDay = 0;
    UPROPERTY() bool bApp = false;
    UPROPERTY() int32 AppDay = 0;
    UPROPERTY() uint8 AppTier = 1;           // the software house: 0 cheap, 1 solid, 2 premium
    UPROPERTY() int64 AppCost = 0;           // what the app cost (its upkeep follows it)
    UPROPERTY() bool bPlatform = false;      // contract with the country's platform
    UPROPERTY() bool bQuick = false;         // our own fast delivery programme (dark stores per province)
    // Where no province manager decides: the company's rule for every province.
    UPROPERTY() bool bDefaultPlatform = true;
    UPROPERTY() bool bDefaultOwn = true;
    UPROPERTY() bool bDefaultQuick = true;
    // Policy (the e-commerce manager can keep it).
    UPROPERTY() uint8 Fee = 1;               // 0 free, 1 below the free basket, 2 always
    UPROPERTY() uint8 MinBasket = 0;         // 0 none, 1 small, 2 big
    UPROPERTY() uint8 PriceGap = 0;          // online prices: 0 as the shelf, 1 +5 %, 2 +10 %
    UPROPERTY() uint8 Substitute = 1;        // 0 call and ask, 1 same aisle, 2 leave it out
    UPROPERTY() bool bAutoPolicy = false;    // the e-commerce manager sets the policy every month
    UPROPERTY() FString ManagerName;
    UPROPERTY() int32 ManagerSkill = 0;
    UPROPERTY() int64 ManagerWage = 0;       // a day, at hiring
    UPROPERTY() int32 ManagerSince = 0;
    // The platform's terms and our own state.
    UPROPERTY() float Commission = 0.18f;
    UPROPERTY() int32 DealUntil = 0;         // exclusive deal with the platform: lower commission, no leaving
    UPROPERTY() int32 SurgeUntil = 0;        // the epidemic's extra couriers
    UPROPERTY() int32 NextCommissionDay = 0;
    UPROPERTY() float Reputation = 60.f;     // online customers' opinion 0..100 (stars follow it)
    UPROPERTY() int32 Told = 0;              // bits: news and cards already given (MarketOnline)
    UPROPERTY() TArray<FString> RivalsTold;  // "chain id|stage"
    UPROPERTY() FString Hint;                // the assistant's last note about online selling
    UPROPERTY() int32 HintDay = 0;
    UPROPERTY() TArray<FMarketOnlineArea> Areas;
    // The closed day.
    UPROPERTY() int32 LastOrders = 0;
    UPROPERTY() int32 LastLate = 0;
    UPROPERTY() int32 LastCancelled = 0;
    UPROPERTY() int32 LastMissing = 0;
    UPROPERTY() int32 LastSubstituted = 0;
    UPROPERTY() int64 LastRevenue = 0;
    UPROPERTY() int64 LastCosts = 0;         // couriers, packaging, commissions, the site, the app, dark stores, ads
    UPROPERTY() int64 LastProfit = 0;
    UPROPERTY() int64 LastBranchProfit = 0;  // the part of LastProfit the branches picked (already in their LastProfit)
    UPROPERTY() int32 WeekOrders = 0;
    UPROPERTY() int64 WeekProfit = 0;
    UPROPERTY() int32 TotalOrders = 0;
    // Per channel (MarketOnline::EChannel): this month and the last closed month.
    UPROPERTY() TArray<int32> MonthOrders;
    UPROPERTY() TArray<int64> MonthRevenue;
    UPROPERTY() TArray<int64> MonthProfit;
    UPROPERTY() TArray<int32> PrevOrders;
    UPROPERTY() TArray<int64> PrevRevenue;
    UPROPERTY() TArray<int64> PrevProfit;
    UPROPERTY() int32 MonthNew = 0;          // new online customers (ads and word of mouth)
    UPROPERTY() int32 PrevNew = 0;
};

// How shoppers pay (MarketPayments.h). Card money arrives one day later, minus the bank's commission.
USTRUCT()
struct FMarketPayments
{
    GENERATED_BODY()
    UPROPERTY() bool bCard = false;           // a POS terminal (monthly rent, commission)
    UPROPERTY() bool bMealCard = false;       // meal cards (higher commission, office workers come)
    UPROPERTY() int64 CardToday = 0;          // card sales of the running day (net of commission)
    UPROPERTY() int64 CardTomorrow = 0;       // yesterday's card sales, paid in at this day's close
    UPROPERTY() int64 Commission = 0;         // commission of the running day
    // Baskets of the running day by method; NoCard = wanted to pay by card without a POS, NoCardLost = left.
    UPROPERTY() int32 Cash = 0;
    UPROPERTY() int32 Card = 0;
    UPROPERTY() int32 Meal = 0;
    UPROPERTY() int32 NoCard = 0;
    UPROPERTY() int32 NoCardLost = 0;
    UPROPERTY() int32 LastCash = 0;
    UPROPERTY() int32 LastCard = 0;
    UPROPERTY() int32 LastMeal = 0;
    UPROPERTY() int32 LastNoCard = 0;
    UPROPERTY() int32 LastNoCardLost = 0;
    UPROPERTY() int64 LastCommission = 0;
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

// Akis C2 (Docs/Kurgu/07_AKIL_ISBOLUMU.md C2b, MarketChains.h): the rival chains of the countries we play in and
// the world's giants. Older saves start empty; MarketChains seeds a country the first time it is needed.
USTRUCT()
struct FMarketChainSpot
{
    GENERATED_BODY()
    UPROPERTY() FString Province;
    UPROPERTY() int32 Stores = 0;
};

USTRUCT()
struct FMarketChain
{
    GENERATED_BODY()
    UPROPERTY() FString Id;              // roster id ("bim"); local ones "yerel.<province>.<n>", regional "bolge.<subregion>"
    UPROPERTY() FString Country;
    UPROPERTY() FString Name;
    UPROPERTY() FString Boss;            // the owner or chief the news quotes
    UPROPERTY() uint8 Archetype = 0;     // MarketChains::EArchetype
    UPROPERTY() uint8 Scope = 0;         // MarketChains::EScope
    UPROPERTY() FString Home;            // local / regional: its province or sub-region; foreign arm: the giant's id
    UPROPERTY() TArray<FMarketChainSpot> Spots;
    UPROPERTY() int64 Cash = 0;          // kurus
    UPROPERTY() float PriceIndex = 1.f;  // everyday shelf prices against the list
    UPROPERTY() float Service = 1.f;
    UPROPERTY() float Aggression = 0.5f; // 0..1: attacks where we grow
    UPROPERTY() float Ambition = 0.5f;   // 0..1: how fast it wants to grow
    UPROPERTY() float Rivalry = 0.f;     // 0..100: how much it minds us
    UPROPERTY() FString WarProvince;     // a price war against us in this province until WarUntil
    UPROPERTY() int32 WarUntil = 0;
    UPROPERTY() int32 WarsLost = 0;
    UPROPERTY() uint8 GoneReason = 0;   // C3: 1 closed (bankrupt), 2 bought by a rival, 3 bought by us
    UPROPERTY() int32 BidDay = 0;       // M29: our last takeover bid it refused
    UPROPERTY() int32 BidAnswerDay = 0;  // C13 (M43): our bid is with its owner until this day
    UPROPERTY() int32 BidAcceptedUntil = 0; // C13: the owner said yes; we pay by this day
    UPROPERTY() int64 BidAgreed = 0;     // C13: the agreed price (kurus)
    UPROPERTY() bool bOurs = false;     // M30: bought, runs as our subsidiary under its own name
    UPROPERTY() int32 OursSince = 0;
    UPROPERTY() bool bExitSale = false; // M30: a giant leaving the country sells its arm cheap
    UPROPERTY() int32 RedTurns = 0;      // monthly turns in a row deep in the red
    UPROPERTY() bool bForSale = false;
    UPROPERTY() int32 ForSaleTurns = 0;
    UPROPERTY() bool bGone = false;
    UPROPERTY() int32 TurnDay = 0;       // day of its last monthly turn
    UPROPERTY() int32 OursSeen = 0;      // our shops in its provinces at its last turn
    UPROPERTY() int64 MonthRevenue = 0;  // last month, kurus
    UPROPERTY() int64 MonthProfit = 0;
};

USTRUCT()
struct FMarketGiant
{
    GENERATED_BODY()
    UPROPERTY() FString Id;
    UPROPERTY() FString Name;
    UPROPERTY() FString Home;            // its home country (name for the menu)
    UPROPERTY() float RevenueB = 0.f;    // billions of world units a year at the start price level (real)
    UPROPERTY() float Growth = 0.03f;    // real growth a year
    UPROPERTY() uint8 Archetype = 0;
    UPROPERTY() TArray<FString> Countries; // pack ids where it runs an arm (our countries only)
};

USTRUCT()
struct FMarketChainsState
{
    GENERATED_BODY()
    UPROPERTY() TArray<FMarketChain> Chains;
    UPROPERTY() TArray<FMarketGiant> Giants;
    UPROPERTY() TArray<FString> Countries;       // seeded countries
    UPROPERTY() TArray<FString> LocalPools;      // "country|province" whose local chains exist
    UPROPERTY() TMap<FString, float> Baseline;   // "country|province" -> weighted chain stores at seeding
    UPROPERTY() FString Nemesis;                 // chain id
    UPROPERTY() int32 LastYearDay = 0;           // giants' yearly turn
    UPROPERTY() int32 LastLeagueDay = 0;
    UPROPERTY() int32 LeagueYearDay = 0;  // C3: the last league year closed (MarketGoals::OnLeagueYear, J02)
    UPROPERTY() TMap<FString, int32> WarRest; // C3: "country|province" -> the day its last price war ended
    UPROPERTY() int32 Closures = 0;       // C3 counters for the reports: chains closed, bought by rivals, bought by us
    UPROPERTY() int32 Takeovers = 0;
    UPROPERTY() int32 OurBuys = 0;
    UPROPERTY() int32 ConvertMonthDay = 0; // M30: stores of subsidiaries turned into our branches this month
    UPROPERTY() int32 ConvertedInMonth = 0;
    UPROPERTY() int32 LeagueRank = 0;            // 0 = not ranked yet
    UPROPERTY() int32 BestLeagueRank = 0;
    UPROPERTY() int32 NationalRank = 0;
    UPROPERTY() int32 BestNationalRank = 0;
};

// Akis C2 / karar M25 (MarketBrands.h): brands ask for room on our shelves. Older saves start empty.
USTRUCT()
struct FMarketBrandShare
{
    GENERATED_BODY()
    UPROPERTY() FString Brand;           // Product.Brand (the real name is the key; the menu shows the fictional one)
    UPROPERTY() FString Category;
    UPROPERTY() float National = 0.f;    // 0..1 share of the category in the country
    UPROPERTY() float Trust = 0.f;       // -100..100: how the brand sees us
};

USTRUCT()
struct FMarketBrandOffer
{
    GENERATED_BODY()
    UPROPERTY() int32 Id = 0;
    UPROPERTY() FString Brand;
    UPROPERTY() FString Category;
    UPROPERTY() uint8 Kind = 0;          // MarketBrands::EKind
    UPROPERTY() float Target = 0.f;      // shelf share (0..1) or the rebate's percent (0..1)
    UPROPERTY() int64 Amount = 0;        // kurus: a month (shelf share), once (listing), the rebate's monthly sales floor
    UPROPERTY() int32 Months = 0;
    UPROPERTY() int32 ExpireDay = 0;
    UPROPERTY() FString ProductId;       // listing: the product it wants on our shelves
    UPROPERTY() FString ProductName;     // listing: shown to the player
};

USTRUCT()
struct FMarketBrandDeal
{
    GENERATED_BODY()
    UPROPERTY() FMarketBrandOffer Terms;
    UPROPERTY() int32 StartDay = 0;
    UPROPERTY() int32 UntilDay = 0;
    UPROPERTY() int32 Strikes = 0;
    UPROPERTY() int64 Paid = 0;
};

USTRUCT()
struct FMarketBrandsState
{
    GENERATED_BODY()
    UPROPERTY() TArray<FMarketBrandShare> Shares;
    UPROPERTY() TArray<FMarketBrandOffer> Offers;
    UPROPERTY() TArray<FMarketBrandDeal> Deals;
    UPROPERTY() TMap<FString, int64> MonthSales;   // brand -> this month's sales in our shops (kurus)
    UPROPERTY() int32 LastMonthDay = 0;
    UPROPERTY() int32 NextOfferId = 1;
    UPROPERTY() int64 TotalReceived = 0;
};

// Akis C2c / G-083 (MarketSourcing.h): where each supply line buys from. Older saves: every line local.
// Karar M28: a company loan from a bank (MarketBanking.h).
USTRUCT()
struct FMarketCorpLoan
{
    GENERATED_BODY()
    UPROPERTY() int32 Id = 0;
    UPROPERTY() uint8 Bank = 0;          // MarketBanking::Bank index (4 = bond)
    UPROPERTY() uint8 Kind = 0;          // MarketBanking::EKind
    UPROPERTY() int64 Principal = 0;
    UPROPERTY() int64 Balance = 0;       // principal still owed (+ late fees)
    UPROPERTY() float YearRate = 0.f;
    UPROPERTY() int32 Months = 0;
    UPROPERTY() int32 PaidMonths = 0;
    UPROPERTY() int32 Grace = 0;         // months of interest only at the start
    UPROPERTY() int32 StartDay = 0;
    UPROPERTY() int32 NextDueDay = 0;
    UPROPERTY() int64 Installment = 0;   // after the grace (a bond: the monthly interest)
    UPROPERTY() int32 LateSince = 0;     // 0 = on time
    // E4b (11_TEK_EKONOMI): the country of the bank (empty = the campaign's). A loan abroad is owed in that country's
    // money: Principal, Balance and Installment are local units, paid at each day's exchange rate.
    UPROPERTY() FString Country;
};

// C13 (M43): a loan application to a bank; the answer comes in a few days, an offer is open for a week.
USTRUCT()
struct FMarketLoanApp
{
    GENERATED_BODY()
    UPROPERTY() int32 Id = 0;
    UPROPERTY() uint8 Bank = 0;          // MarketBanking::Bank index (4 = bond)
    UPROPERTY() uint8 Purpose = 0;       // MarketBanking::EPurpose
    UPROPERTY() int32 Chain = INDEX_NONE; // an acquisition: State.Rivals.Chains index
    UPROPERTY() int64 Asked = 0;         // kurus
    UPROPERTY() int32 Tenor = 0;         // MarketBanking::Tenors index
    UPROPERTY() bool bGrace = false;
    UPROPERTY() int32 AppliedDay = 0;
    UPROPERTY() int32 AnswerDay = 0;
    UPROPERTY() uint8 Status = 0;        // MarketBanking::EAppStatus
    UPROPERTY() int64 Offered = 0;
    UPROPERTY() float YearRate = 0.f;
    UPROPERTY() int32 Months = 0;
    UPROPERTY() int32 ValidUntil = 0;
    UPROPERTY() FString Reason;          // a refusal's or a partial offer's reason
};

// Karar M28: the company's banking (MarketBanking.h).
USTRUCT()
struct FMarketBankingState
{
    GENERATED_BODY()
    UPROPERTY() TArray<FMarketCorpLoan> Loans;
    UPROPERTY() TArray<FMarketLoanApp> Apps; // C13 (M43): open applications and offers
    UPROPERTY() int32 NextAppId = 1;
    UPROPERTY() bool bLine = false;
    UPROPERTY() bool bLineAuto = true;
    UPROPERTY() int64 LineLimit = 0;
    UPROPERTY() int64 LineDrawn = 0;
    UPROPERTY() int32 LineDueDay = 0;
    UPROPERTY() uint8 Rating = 2;        // MarketBanking::ERating (B until the first month)
    UPROPERTY() int32 LastRatingDay = 0;
    UPROPERTY() TArray<int32> LateDays;  // days an installment was missed (the last 365 count)
    UPROPERTY() int32 BreachMonths = 0;
    UPROPERTY() int32 RestructuredUntil = 0;
    UPROPERTY() int32 DevelopmentYear = 0; // the development bank's yearly loan
    UPROPERTY() int32 NextLoanId = 1;
    UPROPERTY() int64 InterestPaid = 0;
};

// C13 (M43): market rumours (MarketRumors.h). A rumour says a rival will do something; its truth is decided when
// it starts and kept hidden; sources (reliable or not, confirming or denying) come in over the days.
USTRUCT()
struct FMarketRumor
{
    GENERATED_BODY()
    UPROPERTY() int32 Id = 0;
    UPROPERTY() uint8 Kind = 0;          // MarketRumors::EKind
    UPROPERTY() FString ChainId;         // the rival it is about
    UPROPERTY() FString OtherId;         // an acquisition: the chain it would buy
    UPROPERTY() FString Country;
    UPROPERTY() FString Province;        // entering / a price war: where
    UPROPERTY() bool bTrue = false;      // hidden
    UPROPERTY() int32 StartDay = 0;
    UPROPERTY() int32 DueDay = 0;
    UPROPERTY() int32 NextSourceDay = 0;
    UPROPERTY() TArray<uint8> Sources;   // bit 0 reliable, bit 1 confirms
    UPROPERTY() uint8 Outcome = 0;       // 0 open, 1 it happened, 2 it did not
};

USTRUCT()
struct FMarketRumorsState
{
    GENERATED_BODY()
    UPROPERTY() TArray<FMarketRumor> Active;
    UPROPERTY() TArray<FMarketRumor> Past;   // the last few resolved, newest last
    UPROPERTY() int32 NextDay = 0;           // the next rumour may start from this day
    UPROPERTY() int32 NextId = 1;
    UPROPERTY() int32 Started = 0;           // counters for the reports
    UPROPERTY() int32 CameTrue = 0;
};

// Karar M26: departments per store type and their price stance (MarketDepartments.h). Older saves: none.
USTRUCT()
struct FMarketDepartmentsState
{
    GENERATED_BODY()
    UPROPERTY() TArray<uint8> Policy;    // store type x 14 + department: 1 = runs in every branch of that type
    UPROPERTY() TArray<uint8> Stance;    // per department: 0 cheap, 1 normal, 2 dear
    UPROPERTY() int32 LastMonthDay = 0;
};

USTRUCT()
struct FMarketSourcingState
{
    GENERATED_BODY()
    UPROPERTY() TArray<uint8> Tiers;       // per line: MarketSourcing::ETier
    UPROPERTY() TArray<uint8> Missed;      // per line: months in a row under the tier's minimum
    UPROPERTY() TArray<int64> MonthBuy;    // per line: this month's purchases (kurus)
    UPROPERTY() int32 LastMonthDay = 0;
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

    // Save format version. 2 (G-076): story finale flags, test-mode mark. Older saves load and are migrated.
    static constexpr int32 CurrentVersion = 22; // D9b: portfolio and answers (M46, M47); // M69: no chapters, the player's and the market's names, the first store owns its building; // D9: strategy (M48-M50); // D6: franchises; // M54/M58/M59; // E4b: loans abroad; // M65: subsidiaries; // E4: the exchange difference account; // E3c2c: branch staff are people; // M64: the father's favour is counted; // E3c2 (M27): branch goods use the first store's record, the v0.1 staff flags are gone; older saves start a new game
    UPROPERTY() int32 Version = CurrentVersion;
    UPROPERTY() int32 Day = 1;
    UPROPERTY() int64 Cash = 35000;
    UPROPERTY() TArray<FMarketStock> Stock;
    UPROPERTY() bool bRealBrands = false;   // karar L12: fictional brands close to the real ones; F8 shows real names (development)
    // G-076: true once free test controls (F2/F3, free orders) were used in this campaign; shown in the menu.
    UPROPERTY() bool bUsedTestMode = false;
    // G-078 (#5): the campaign's own shelf plan (planograms.json text), written at every save. Empty:
    // they keep Config/planograms.json.
    UPROPERTY() FString PlanogramJson;
    // G-084: the country pack (Config/ulkeler.json) and start city of the campaign. Older saves: Turkey.
    UPROPERTY() FString CountryId = TEXT("tr");
    UPROPERTY() FString CityId;
    // M69: the player's own name, written on the new-game screen (MarketStart::PlayerName).
    UPROPERTY() FString PlayerName;
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
    // Seed of the campaign (weather, people, rival chains); set for each new campaign, so a reload cannot reroll it.
    UPROPERTY() int32 RivalSeed = 0;
    // Every closed day, oldest first (MarketCampaign::CloseDay). Kept for ten game years at most.
    UPROPERTY() TArray<FMarketDayRecord> History;
    UPROPERTY() TArray<FMarketLoyalty> Loyalty;
    // People (MarketStaff.h). Who is on duty today comes from the roster (MarketStaff::CashierOnDuty, StockersOnDuty;
    // E3c2: the v0.1 bCashier/Stockers flags are gone).
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
    // Other costs of today (repairs, fines, a rented generator): paid at the day close with the operating costs.
    UPROPERTY() int64 OtherCosts = 0;
    // Book losses of today with no cash cost at the close (stock sold below cost, an early-repayment fee paid at
    // once): taken into the next close's profit so reports and the tax books see them.
    UPROPERTY() int64 PendingLoss = 0;
    // Story, choices waiting for the player, lasting effects and the history of events (MarketStory, MarketEvents).
    UPROPERTY() FMarketStoryState Story;
    UPROPERTY() TArray<FMarketDecision> Decisions;
    UPROPERTY() TArray<FMarketModifier> Modifiers;
    UPROPERTY() TArray<FString> EventLog;      // "id@day" of events that happened (cooldowns)
    UPROPERTY() int32 DeliveryDelayDay = 0;    // the delivery of this day's close waits one more day (snow, broken truck)
    // Freshness (MarketFreshness.h): batches of perishable goods, the last-day policy, yesterday's waste.
    UPROPERTY() TArray<FMarketBatch> Batches;
    UPROPERTY() uint8 FreshPolicy = 1;         // 0 nothing, 1 last-day markdown, 2 donate the last day
    UPROPERTY() int32 LastWasteUnits = 0;
    UPROPERTY() int64 LastWasteCost = 0;
    // Bank and the money trouble ladder (MarketFinance.h).
    UPROPERTY() TArray<FMarketLoan> Loans;
    UPROPERTY() int32 NegativeCashDays = 0;
    UPROPERTY() int32 Rescues = 0;          // M31: the bank's rescue plans so far (MarketFinance::Rescue)
    UPROPERTY() int32 RescueUntil = 0;      // C7: under the bank's plan until this day (no new loans or branches)
    UPROPERTY() FMarketOwner Owner;         // M37: our salary and personal wealth (MarketOwner)
    UPROPERTY() int32 LowCashWarnDay = 0;   // C9: the last "goods money is running out" warning
    UPROPERTY() int32 PromoStore = -1;      // M38: where the player's next campaign runs (first store / all stores)
    UPROPERTY() int64 StartDebt = 30000;    // M37/M69: the inherited debt at the start (the progress bar)
    UPROPERTY() int32 TroubleStage = 0;
    // Branches (MarketBranches.h).
    UPROPERTY() TArray<FMarketBranch> Branches;
    // G-088 stage C: the store view of every site (MarketStoreAssign::SiteKey -> magazalar.json id), so a province
    // keeps its view for every branch of that type and across saves (MarketStoreViews).
    UPROPERTY() TMap<FString, FString> StoreViews;
    // Akis C2: rival chains of our countries and the world giants (MarketChains.h).
    UPROPERTY() FMarketChainsState Rivals;
    // Karar M25: brands, their shares, offers and deals (MarketBrands.h).
    UPROPERTY() FMarketBrandsState Brands;
    // G-083: supply lines and their tiers (MarketSourcing.h).
    UPROPERTY() FMarketSourcingState Sourcing;
    UPROPERTY() FMarketDepartmentsState Departments; // karar M26 (MarketDepartments.h)
    UPROPERTY() FMarketBankingState Banking; // karar M28 (MarketBanking.h)
    UPROPERTY() FMarketRumorsState Rumors;   // C13 (M43): market rumours (MarketRumors.h)
    // Online orders and payment methods (MarketOnline.h, MarketPayments.h).
    UPROPERTY() FMarketOnline Online;
    UPROPERTY() FMarketAdvertising Advertising; // M34
    UPROPERTY() TMap<FString, int32> ProposalQuiet; // M33: "country|province" -> no new opening proposal before this day
    UPROPERTY() FMarketPayments Payments;
    // Difficulty (MarketSimulation.h): 0 easy, 1 normal, 2 hard. Days played by the strategic advance.
    UPROPERTY() uint8 Difficulty = 1;
    // Growth beyond the first store (MarketCompany.h).
    UPROPERTY() FMarketCompany Company;
    // G-086b: province, regional and country managers (MarketManagers.h).
    UPROPERTY() FMarketManagement Management;
    UPROPERTY() int32 AdvancedDays = 0;
    // Local share at the start of the last day close (MarketStoreDemand::CloseDay replaces the simple satisfaction update).
    UPROPERTY() float ShareBeforeClose = 25.f;
    // Evening report lines of the background systems (MarketDirector clears it at every day close).
    UPROPERTY() TArray<FString> DayNews;

    // ===== Ak\u0131\u015f B =====
    // Docs/Surec/akislar/B.md: the books (MarketLedger.h).
    UPROPERTY() FMarketLedger Ledger;
    // B4: eras of the economy (MarketEras.h).
    UPROPERTY() FMarketEras Eras;
    UPROPERTY() FMarketStrategyState Strategy; // D9 (M48-M50): province pushes, strategic forks, paths (MarketStrategy.h)
    UPROPERTY() FMarketResponses Responses;    // D9b (M47): answers to rivals' moves and crises (MarketResponse.h)
    // B6: goals, firsts, records, celebrations, the rhythm guard (MarketGoals.h).
    UPROPERTY() FMarketGoals Goals;
    // ===== Ak\u0131\u015f B son =====

    // Wages of everyone on the payroll (paid days off included).
    int64 DailyPayroll() const;

    void Initialize(const TArray<FMarketProduct>& Products);
    bool Order(int32 Index, const TArray<FMarketProduct>& Products);
    // Atomically submits a multi-product order. Cases is indexed like Products; no money or stock is
    // changed when any line is invalid. Returns the paid bill and ordered units when requested.
    // CreditAllowance (G-077, #33): what the wholesaler lets us buy on terms beyond the cash in the till
    // (MarketSuppliers::OrderAllowance). The till may go below zero here; MarketSuppliers::OnOrder gives it back.
    bool SubmitOrder(const TArray<int32>& Cases, const TArray<FMarketProduct>& Products, int64* OutBill = nullptr, int32* OutUnits = nullptr, int64 CreditAllowance = 0);
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
    // G-078: book cost of one unit of a product (weighted average purchase cost, or today's cost if unknown).
    int64 UnitCost(int32 Index, const TArray<FMarketProduct>& Products) const;
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

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
    UPROPERTY() float VatRate = -1.f;    // e.g. 0.08 food, 0.18 cleaning (2011)
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
    // Older saves have no value and load as the v0.1 default.
    UPROPERTY() int32 Capacity = 24;
    // Shoppers of the running day and of the last closed day (the day report reads Yesterday).
    UPROPERTY() FMarketDemandStats Today;
    UPROPERTY() FMarketDemandStats Yesterday;
    // New units that came into the shop since the last freshness check (deliveries, test fills, a closed
    // branch's goods). MarketFreshness turns them into a new batch; everything else that left came from old batches.
    UPROPERTY() int32 Received = 0;
    // G-078 (#26): weighted average purchase cost of the units held (shelf, depot, dock, on the way), in kurus.
    // The cost of goods sold and the waste use it, so a price rise or a cheaper deal changes only new purchases.
    // 0 = unknown (older saves, the inherited stock): today's catalog cost is used until the first purchase.
    UPROPERTY() int64 AvgCost = 0;
    // G-078 shopper memory (MarketPromotions): how often it has been on a deal lately (decaying day count; many
    // deals teach shoppers to wait for the next one) and what they stocked up at home on the last deal (the
    // after-promotion dip).
    UPROPERTY() float PromoHeat = 0.f;
    UPROPERTY() float Pantry = 0.f;
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
};

// A competing company in the district (MarketCompetitors.h). Older saves start without them (created on first use).
USTRUCT()
struct FMarketCompetitor
{
    GENERATED_BODY()
    UPROPERTY() uint8 Company = 0;       // MarketCompetitors::ECompany
    UPROPERTY() int64 Cash = 0;          // what the company can spend on a fight in this district
    UPROPERTY() float BaseIndex = 1.f;   // its everyday prices / list price
    UPROPERTY() float Service = 1.f;
    UPROPERTY() int32 Stores = 1;        // its shops that serve our street
    UPROPERTY() float Share = 0.f;       // 0..1 of the district's shopping
    UPROPERTY() float Anger = 0.f;       // 0..100: how much it wants to hurt us (the local rival)
    UPROPERTY() FString WarCategory;     // aisle of a running price war
    UPROPERTY() int32 WarUntil = 0;
    UPROPERTY() int32 WarCooldownUntil = 0;
    UPROPERTY() int32 Told = 0;          // bit flags of one-time news already told
    // G-079: days in a row with no money left (the local rival sells out after a month) and a slow average of our
    // local share the chains watch (they answer a rising shop).
    UPROPERTY() int32 RedDays = 0;
    UPROPERTY() float WatchShare = 0.f;
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

// Where the story is (MarketStory.h).
USTRUCT()
struct FMarketStoryState
{
    GENERATED_BODY()
    UPROPERTY() int32 Chapter = 1;
    UPROPERTY() int64 Beats = 0;             // bit flags of scenes already played
    UPROPERTY() uint8 Identity = 0;          // MarketStory::EIdentity (0 = not chosen)
    UPROPERTY() uint8 Ending = 0;            // MarketStory::EEnding reached last (0 = none)
    // Karar J02 (Game Dev Tycoon style): the finale is shown once; afterwards the simulation goes on but no new
    // chapter, scene or story content arrives. Replaces the old chapter 99 trick.
    UPROPERTY() bool bEnded = false;
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

// A neighbour's page in the credit book (MarketCredit.h).
USTRUCT()
struct FMarketCreditAccount
{
    GENERATED_BODY()
    UPROPERTY() int32 CustomerId = 0;
    UPROPERTY() int64 Balance = 0;
    UPROPERTY() int32 SinceDay = 0;      // oldest unpaid day
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
    UPROPERTY() bool bMortgage = false;  // the family shop's deed stands behind it
};

// One product in a branch that is not visited (MarketBranches.h).
USTRUCT()
struct FMarketBranchItem
{
    GENERATED_BODY()
    UPROPERTY() FString ProductId;
    UPROPERTY() int32 Units = 0;         // shelf + back room
    UPROPERTY() int32 Capacity = 0;      // from the branch's automatic shelf plan (MarketLayout)
    UPROPERTY() int32 Incoming = 0;      // ordered by the manager, arrives at the next close
    UPROPERTY() int32 LastSold = 0;
    UPROPERTY() int32 LastEmpty = 0;
};

// A branch of the company, simulated from the same rules without walking customers (MarketBranches.h).
USTRUCT()
struct FMarketBranch
{
    GENERATED_BODY()
    UPROPERTY() FString Name;
    UPROPERTY() uint8 District = 0;      // G-068 prototype district (unused since G-086; older saves)
    // G-086: the province the branch is in (MarketCountry) and its country. Older saves: empty = the home province.
    UPROPERTY() FString Province;
    UPROPERTY() FString Country;
    UPROPERTY() FString Format;          // kucuk (ucuzcu), mahalle, buyuk (supermarket), hiper (MarketLayout::Fixtures)
    UPROPERTY() uint8 Stage = 0;         // MarketBranches::EStage
    UPROPERTY() int32 StageUntil = 0;
    UPROPERTY() int32 OpenedDay = 0;
    UPROPERTY() int64 Rent = 0;          // per month, kurus at the time of signing
    UPROPERTY() int32 Workers = 0;
    UPROPERTY() FString ManagerName;
    UPROPERTY() int32 ManagerSkill = 0;  // 0 = no manager: the player's standing orders
    UPROPERTY() int32 ManagerHonesty = 70;
    UPROPERTY() int64 ManagerWage = 0;
    UPROPERTY() float PriceIndex = 1.f;  // shelf prices / list price
    UPROPERTY() float Maturity = 0.f;    // 0..1: the district's habit of shopping here
    UPROPERTY() float Satisfaction = 55.f;
    UPROPERTY() TArray<FMarketBranchItem> Items;
    UPROPERTY() int64 LastRevenue = 0;
    UPROPERTY() int64 LastProfit = 0;
    UPROPERTY() int32 LastShoppers = 0;
    UPROPERTY() int64 WeekProfit = 0;
    UPROPERTY() int64 Last30Profit = 0;  // running sum over about 30 days
    // G-086b (Docs/Kurgu/03_MAGAZA_AGI.md \u00a74.1, MarketManagers.h): the manager's hidden style (MarketManagers::EStyle;
    // 0 = an older save, seeded once), morale (-1 = an older save), warnings, the day they took over (0 = long ago:
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
    // G-086b ek (M21): the manager's hidden ceiling of skill (55..95). 0 = an older save: MarketManagers::Migrate
    // derives it once from the skill (+5..20, at most 95).
    UPROPERTY() int32 ManagerPotential = 0;
};

// G-072 aggregate city stores (older saves only). G-086 turns every row into real branches in its province
// (MarketBranches::Migrate) and empties the list.
USTRUCT()
struct FMarketCityStores
{
    GENERATED_BODY()
    UPROPERTY() uint8 City = 0;             // G-072 city index (MarketBranches::Migrate maps it to a province)
    UPROPERTY() int32 Stores = 0;
    UPROPERTY() int32 FirstDay = 0;         // day the first store opened
    UPROPERTY() float Maturity = 0.f;       // 0..1: shoppers' habit (grows over 60 days, a new store dilutes it)
    UPROPERTY() int64 LastProfit = 0;       // all stores of the city, last closed day
    UPROPERTY() int64 Last30Profit = 0;     // running sum over the last 30 days (approximate)
    // G-077 (#38): deposits actually paid for the open stores (older saves: 0, then the day's value is used once).
    UPROPERTY() int64 DepositsPaid = 0;
};

// G-089 (karar M23): a big depot in a province (MarketDepots.h). It serves our branches of its country within
// 600 km; its manager is an FMarketManager of level MarketManagers::ELevel::Depot with the same country and
// province. Older saves: none (their sub-region depots move here once, MarketDepots::Migrate).
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

// The company beyond the family shop (MarketCompany.h). Older saves: nothing built.
USTRUCT()
struct FMarketCompany
{
    GENERATED_BODY()
    UPROPERTY() TArray<FMarketCityStores> Cities;
    UPROPERTY() bool bDepot = false;          // G-072 single depot (older saves; G-086 moves it to Depots)
    UPROPERTY() TArray<FString> Depots;       // G-086: sub-regions with a regional depot (older saves; G-089 moves them to DepotSites)
    UPROPERTY() TArray<FMarketDepot> DepotSites; // G-089: depots in provinces (MarketDepots.h)
    UPROPERTY() int32 Trucks = 0;
    UPROPERTY() bool bCentralBuying = false;  // buying for all stores at once
    UPROPERTY() bool bPrivateLabel = false;   // "Miras" own brand
    UPROPERTY() bool bDarkStore = false;      // a depot that only picks online orders
    UPROPERTY() int32 LeadershipDays = 0;     // chapter 7: days leading on every measure in a row
    UPROPERTY() int64 LastProfit = 0;         // all city stores + head office, last closed day
    UPROPERTY() int64 WeekProfit = 0;
};

// G-086b: a manager above the shops (province / sub-region / main region / country) or the family shop's manager
// (MarketManagers.h, Docs/Kurgu/03_MAGAZA_AGI.md \u00a74). Store managers of branches live in FMarketBranch.
USTRUCT()
struct FMarketManager
{
    GENERATED_BODY()
    UPROPERTY() uint8 Level = 1;            // MarketManagers::ELevel
    UPROPERTY() FString Country;            // pack id
    UPROPERTY() FString Area;               // province / sub-region / main region id; the country id; family shop: home province
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
    // G-086b ek (M19, M21): hidden style (MarketManagers::EStyle; the family shop's manager runs the shop by it;
    // 0 = an older save, seeded once) and hidden ceiling of skill (55..95; 0 = an older save, derived once).
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

// Online orders (MarketOnline.h): channels by era, the delivery team and what the last closed day did.
// Older saves load everything off.
USTRUCT()
struct FMarketOnline
{
    GENERATED_BODY()
    UPROPERTY() bool bPhone = false;          // phone orders + delivery to the street (2011+)
    UPROPERTY() bool bWeb = false;            // own web shop (2014+)
    UPROPERTY() bool bPlatform = false;       // fast-delivery platform (2016+), commission
    UPROPERTY() bool bFreeDelivery = true;    // false = a delivery fee below the free basket
    UPROPERTY() uint8 Substitute = 1;         // 0 call and ask, 1 same aisle, 2 leave it out
    UPROPERTY() bool bPandemic = true;        // the 2020-2021 period profile (karar D11, open question)
    UPROPERTY() int32 Couriers = 0;
    UPROPERTY() int32 WebOpenedDay = 0;
    UPROPERTY() float Reputation = 60.f;      // online customers' opinion 0..100 (platform stars follow it)
    UPROPERTY() int32 LastOrders = 0;
    UPROPERTY() int32 LastLate = 0;
    UPROPERTY() int32 LastCancelled = 0;
    UPROPERTY() int32 LastMissing = 0;
    UPROPERTY() int32 LastSubstituted = 0;
    UPROPERTY() int64 LastRevenue = 0;
    UPROPERTY() int64 LastCosts = 0;          // couriers, packaging, commissions, the site
    UPROPERTY() int64 LastProfit = 0;
    UPROPERTY() int32 WeekOrders = 0;
    UPROPERTY() int64 WeekProfit = 0;
    UPROPERTY() int32 TotalOrders = 0;
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
    static constexpr int32 CurrentVersion = 2;
    UPROPERTY() int32 Version = CurrentVersion;
    UPROPERTY() int32 Day = 1;
    UPROPERTY() int64 Cash = 35000;
    UPROPERTY() TArray<FMarketStock> Stock;
    UPROPERTY() bool bCashier = false;
    // Shelf staff count (older saves load 0).
    UPROPERTY() int32 Stockers = 0;
    UPROPERTY() bool bSecondStore = false;
    UPROPERTY() bool bRealBrands = false;   // karar L12: fictional brands close to the real ones; F8 shows real names (development)
    // G-076: true once free test controls (F2/F3, free orders) were used in this campaign; shown in the menu.
    UPROPERTY() bool bUsedTestMode = false;
    // G-078 (#5): the campaign's own shelf plan (planograms.json text), written at every save. Empty in older saves:
    // they keep Config/planograms.json.
    UPROPERTY() FString PlanogramJson;
    // G-084: the country pack (Config/ulkeler.json) and start city of the campaign. Older saves: Turkey.
    UPROPERTY() FString CountryId = TEXT("tr");
    UPROPERTY() FString CityId;
    // G-084: who left the shop (MarketStart.h: teyze, dayi, hala, amca, buyukanne). Empty in older saves = the father.
    UPROPERTY() FString RelativeKey;
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
    // Other costs of today (repairs, fines, a rented generator): paid at the day close with the operating costs.
    UPROPERTY() int64 OtherCosts = 0;
    // Book losses of today with no cash cost at the close (stock sold below cost, an early-repayment fee paid at
    // once): taken into the next close's profit so reports and the tax books see them.
    UPROPERTY() int64 PendingLoss = 0;
    // Competing companies of the district (MarketCompetitors.h).
    UPROPERTY() TArray<FMarketCompetitor> Competitors;
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
    // Credit book (MarketCredit.h).
    UPROPERTY() TArray<FMarketCreditAccount> Credit;
    UPROPERTY() int64 CreditLimit = 0;          // per neighbour, kurus (0 = no credit)
    // Bank and the money trouble ladder (MarketFinance.h).
    UPROPERTY() TArray<FMarketLoan> Loans;
    UPROPERTY() int32 NegativeCashDays = 0;
    // The family lives from the shop: money taken home this month (MarketFinance, not a business cost).
    UPROPERTY() int64 MonthHousehold = 0;
    UPROPERTY() int32 TroubleStage = 0;
    // Branches (MarketBranches.h). bSecondStore stays true while at least one branch exists (older code and saves).
    UPROPERTY() TArray<FMarketBranch> Branches;
    // Online orders and payment methods (MarketOnline.h, MarketPayments.h).
    UPROPERTY() FMarketOnline Online;
    UPROPERTY() FMarketPayments Payments;
    // Difficulty (MarketSimulation.h): 0 easy, 1 normal, 2 hard. Days played by the strategic advance.
    UPROPERTY() uint8 Difficulty = 1;
    // Growth beyond the family shop (MarketCompany.h).
    UPROPERTY() FMarketCompany Company;
    // G-086b: province, regional and country managers (MarketManagers.h).
    UPROPERTY() FMarketManagement Management;
    UPROPERTY() int32 AdvancedDays = 0;
    // Local share at the start of the last day close (MarketCompetitors replaces the simple satisfaction update).
    UPROPERTY() float ShareBeforeClose = 25.f;
    // Evening report lines of the background systems (MarketDirector clears it at every day close).
    UPROPERTY() TArray<FString> DayNews;

    // ===== Ak\u0131\u015f B =====
    // Docs/Surec/akislar/B.md: the books (MarketLedger.h). Older saves load it empty.
    UPROPERTY() FMarketLedger Ledger;
    // B4: eras of the economy (MarketEras.h). Older saves: the unshifted plan.
    UPROPERTY() FMarketEras Eras;
    // B6: goals, firsts, records, celebrations, the rhythm guard (MarketGoals.h). Older saves start silently.
    UPROPERTY() FMarketGoals Goals;
    // ===== Ak\u0131\u015f B son =====

    // Wages of everyone on the payroll (paid days off included). Staff empty = the v0.1 flags (older saves, tests).
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

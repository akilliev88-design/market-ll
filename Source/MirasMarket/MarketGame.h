#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/HUD.h"
#include "GameFramework/SaveGame.h"
#include "MarketEconomy.h"
#include "Planogram.h"
#include "MarketVisuals.h"
#include "MarketPeople.h"
#include "MarketMotion.h"
#include "PlanogramEdit.h"
#include "StaffPlanner.h"
#include "MarketDemand.h"
#include "MarketCampaign.h"
#include "MarketRivals.h"
#include "MarketBasket.h"
#include "MarketOrderAdvice.h"
#include "MarketStaff.h"
#include "MarketDirector.h"
#include "MarketCustomers.h"
#include "MarketPromotions.h"
#include "MarketBranches.h"
#include "MarketGame.generated.h"

class UTextRenderComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;
class UInstancedStaticMeshComponent;
class SWidget;
class SMarketMenu;
DECLARE_DELEGATE_OneParam(FMarketCommandDelegate, FName);

// How one product looks on a shelf: mesh, slot materials and the studio's model correction. Shared by the
// shelf stock and the arrange-mode ghost preview (plain struct: the holder keeps the objects alive).
struct FProductLook
{
    UStaticMesh* Mesh = nullptr;
    TArray<UMaterialInterface*> Materials;
    FVector ItemScale = FVector(1.f);
    FRotator ModelRotation = FRotator::ZeroRotator;
    FVector PlacementOffset = FVector::ZeroVector;
    FBox SourceBounds = FBox(FVector(-50.f), FVector(50.f));
};

UCLASS()
class UMarketSave : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY() FMarketState State;
};

UCLASS()
class AMarketCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    AMarketCharacter();
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    void Forward(float Value);
    void Right(float Value);
    void Turn(float Value);
    void Look(float Value);
    void Interact();
    void ToggleShop();
    void NextProduct();
    void Order();
    void PriceUp();
    void PriceDown();
    void Hire();
    void Expand();
    void Save();
    void Load();
    void Brands();
    void NewCampaign();
    void ToggleDetails();
    void ToggleTestMode();
    void FillAll();
    void NextStore();
    void PreviousStore();
    void RandomizeShelves();
    void NextMood();
    void ToggleFullscreen();
    void Quit();
    void SendCommand(FName Action);
};

USTRUCT()
struct FMarketBasketItem
{
    GENERATED_BODY()
    int32 Product = INDEX_NONE;
    int32 Quantity = 0;
    int64 QuotedPrice = 0;
    bool bSubstitute = false;
};

USTRUCT()
struct FMarketCustomer
{
    GENERATED_BODY()
    UPROPERTY() TObjectPtr<AActor> Actor = nullptr;
    int32 CustomerId = INDEX_NONE;
    bool bReturning = false;
    uint8 Segment = 0;        // MarketCustomers::ESegment: pace, patience, taste, budget
    int64 BudgetLeft = 0;     // money left for this visit
    float Browse = 0.f;       // seconds already spent looking at the current shelf
    TArray<int32> ShoppingList;
    int32 ShoppingIndex = 0;
    TArray<FMarketBasketItem> Basket;
    int32 Product = INDEX_NONE; // current shelf target (wanted product or substitute)
    bool bTryingSubstitute = false;
    uint8 OriginalFailure = static_cast<uint8>(MarketDemand::EVisit::NotCarried);
    int32 Fulfilled = 0;
    float Age = 0;
    int32 Stage = 0; // 0 shopping, 1 approaching till, 2 queued, 3 leaving without a purchase
    int32 QueueTicket = INDEX_NONE;
    // MetaHuman visual state (unused for the simple box shoppers).
    bool bHuman = false;
    MarketPeople::FShopper Shopper;
    // Aisle waypoints before the stage target (shoppers walk around fixtures, not through them).
    TArray<FVector> Route;
    int32 RouteStage = -1;
    int32 RouteProduct = INDEX_NONE;
    // How this person moves (MarketMotion, G-070): walking style, a stop to look around or chat, time on the shelf.
    MarketMotion::FGait Gait;
    float Pause = 0.f;
    float WalkTime = 0.f;
    float BrowseNeed = -1.f;  // seconds needed at the current shelf (-1 = not decided yet)
    bool bChatted = false;
};

enum class EWorkerStage : uint8 { Idle, ToDepot, Loading, ToShelf, Working, ToDelivery, DeliveryLoading, DeliveryToDepot, DeliveryUnloading };

// One shelf worker (reyon gorevlisi) walking in the store. Units stay in the depot until the worker puts
// them on the shelf one by one, so nothing is lost if the trip is cut short.
USTRUCT()
struct FMarketWorker
{
    GENERATED_BODY()
    UPROPERTY() TObjectPtr<AActor> Actor = nullptr;
    UPROPERTY() TObjectPtr<AActor> Carton = nullptr;   // carried case (only while carrying)
    UPROPERTY() TObjectPtr<UTextRenderComponent> Tag = nullptr; // name above the head
    FString Name;
    int32 EmployeeId = INDEX_NONE;  // MarketStaff roster person (speed, carried units, fatigue)
    bool bHuman = false;
    MarketPeople::FShopper Shopper;
    EWorkerStage Stage = EWorkerStage::Idle;
    StaffPlanner::FJob Job;
    TArray<FVector> Route;          // floor waypoints; empty = standing at the goal
    FVector Facing = FVector(0, 1, 0);
    float Timer = 0.f;
    int32 Carry = 0;                // units still to put on the shelf this trip
    int32 DeliveryProduct = INDEX_NONE;
    int32 Retries = 0;              // new spots tried after the planned one was taken
    bool bResting = false;
};

// A thing the player should look at now (G-074): the HUD shows the first three as notices, the menu's Ozet lists
// them with a button to the page that solves it. Built once a frame by AMarketGameMode::Todos (MarketMenu.cpp).
struct FMarketTodo
{
    int32 Severity = 0;          // 0 information, 1 warning, 2 urgent
    FString Title;
    FString Text;
    int32 Page = 0;              // SMarketMenu::EPage that solves it
    int32 Product = INDEX_NONE;  // product to select on that page
};

struct FMarketQueueRules
{
    static int32 Rank(const TArray<FMarketCustomer>& Customers, int32 CustomerIndex);
    static int32 FindFront(const TArray<FMarketCustomer>& Customers);
};

UCLASS(config=Game)
class AMarketGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AMarketGameMode();
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    UPROPERTY() TArray<FMarketProduct> Products;
    UPROPERTY() FMarketState State;
    FMarketPlanogram Planogram;
    UPROPERTY() TArray<TObjectPtr<UTextRenderComponent>> CategorySigns;
    TArray<FString> CategorySignKeys;
    TSharedPtr<SWidget> CategoryPicker;
    int32 CategorySelection = 0;
    FString CategoryFixture, CategoryFace;
    TArray<FString> CategoryChoices;
    int32 CategoryTarget(FString& OutFace) const;
    bool CategoryCommand(FName Action);
    void RefreshCategorySigns();
    void ShowCategoryPicker();
    void CloseCategoryPicker(bool bAccept);
    UPROPERTY() TArray<FMarketCustomer> Customers;
    // Price text on every shelf tag; ShelfLabelProduct holds the product index of each label.
    UPROPERTY() TArray<TObjectPtr<UTextRenderComponent>> ShelfLabels;
    TArray<int32> ShelfLabelProduct;
    UPROPERTY() TMap<int32, TObjectPtr<UMaterialInterface>> SurfaceCache;
    // Test phase: shelves can be filled without warehouse stock or cash (F2 toggles, F3 fills all).
    // Default comes from DefaultGame.ini [/Script/MirasMarket.MarketGameMode] bTestModeAtStart.
    UPROPERTY(Config) bool bTestModeAtStart = false;   // G-076: off for players; developers turn it on in the ini
    // Shoppers: MetaHumans when assembled (DefaultGame.ini can turn them off or fix their facing).
    UPROPERTY(Config) bool bUseMetaHumans = true;
    UPROPERTY(Config) float MetaHumanYawOffset = 0.f;
    MarketPeople::FLibrary People;
    UPROPERTY() TArray<TObjectPtr<UObject>> PeopleAssets; // keeps loaded classes/animations alive
    bool bTestMode = false;
    bool bShowDetails = false;
    float ReportTime = 0;
    UPROPERTY() TObjectPtr<UStaticMesh> Cube;
    UPROPERTY() TObjectPtr<UStaticMesh> Sphere;
    // One instanced mesh per product (index = catalog order). ShelfSlots holds every world slot of the
    // product over all its blocks, front rows first; RefreshShelfItems shows the first Shelf of them.
    UPROPERTY() TArray<TObjectPtr<UInstancedStaticMeshComponent>> ShelfInstances;
    TArray<TArray<FTransform>> ShelfSlots;
    // Fallback when a product's materials cannot be instanced: one component per slot (owned by the holder actor).
    TArray<TArray<TWeakObjectPtr<UStaticMeshComponent>>> ShelfSingles;
    // Where a player/customer stands to reach each block of a product (primary block first).
    TArray<TArray<FVector>> ShelfApproach;
    MarketVisuals::FStoreLighting Lighting;
    int32 Mood = 0;
    int32 StoreRows = 2;
    bool bOpen = false;
    int32 Selected = 0;
    float DayTime = 0;
    float SpawnTimer = 0;
    float MessageTime = 0;
    FString Message;
    FRandomStream Random;
    float AutoCheckoutTimer = 0;
    int32 NextQueueTicket = 0;
    float ResetConfirmUntil = -1;
    int32 CaptureStage = 0;
    float CaptureAt = 0;
    float CaptureReadySince = -1;
    int32 SmokeStage = 0;
    // Shelf staff (J at the office). They refill shelves from the depot, put products that are not on a shelf
    // onto their category's shelves and widen blocks that cannot hold one case. MarketWorkers.cpp, StaffPlanner.h.
    UPROPERTY() TArray<FMarketWorker> Workers;
    TSet<int32> Unplaceable;        // products with no room on their category's shelves (cleared when the plan changes)
    TSet<int32> NoRoomTold;         // "no room" already reported to the player for these products
    int32 WorkerPlanVersion = -1;
    // Office multi-product order draft. One integer case count per catalog row; the draft itself is
    // deliberately not a save concern until N submits it atomically into State.Incoming.
    TArray<int32> OrderDraftCases;
    UPROPERTY() TArray<TObjectPtr<AActor>> DeliveryCrates;
    UPROPERTY() TArray<TObjectPtr<UTextRenderComponent>> DeliveryCrateLabels;
    int32 CarriedDeliveryProduct = INDEX_NONE;
    UPROPERTY() TObjectPtr<AActor> CarriedDeliveryCrate = nullptr;
    void SyncWorkers();
    void ResetWorkerJobs();
    void TickWorkers(float DeltaTime);
    void WorkerThink(int32 WorkerIndex);
    void WorkerLoaded(FMarketWorker& Worker);
    void WorkerAtShelf(FMarketWorker& Worker);
    void WorkerFinish(FMarketWorker& Worker);
    bool WorkerWalk(FMarketWorker& Worker, float DeltaTime); // true = at the goal
    void WorkerGoTo(FMarketWorker& Worker, const FVector& Goal);
    FString WorkerSummary() const;
    FVector DepotSpot() const;
    FVector DeliverySpot(int32 ProductIndex) const;
    FVector WorkerRestSpot(int32 Index) const;
    int32 NearbyDelivery() const;
    void RefreshDeliveryCrates();
    bool StartPlayerDelivery(int32 ProductIndex);
    bool FinishPlayerDelivery();
    // New campaign / load: the crate in the player's hands is put back (its units never left the rear door).
    void DropCarriedDelivery();
    void TickPlayerDelivery();
    int32 OrderDraftCaseCount() const;
    int64 OrderDraftBill() const;
    FString OrderDraftSummary() const;
    // Shopper / worker standing spot in front of a block (floor level).
    FVector BlockApproachSpot(const FPlanogramPlacement& Placement, float CenterX) const;
    // Walk along the open lanes (x = +/-150), crossing at the front corridor or behind the last gondola.
    TArray<FVector> AisleRoute(const FVector& From, const FVector& To) const;
    float StoreBack() const { return 400.f + StoreRows * 320.f; }
    // Saves the plan and rebuilds everything on the shelves (arrange mode and workers). False = not saved.
    bool CommitPlan(FString& OutError);
    // Simple box person (when no MetaHuman is assembled). Floor = feet position.
    AActor* SimplePerson(const FVector& Floor, const FLinearColor& Color);
    // Smoke test / screenshot runs (MarketAutomation.cpp). False = a smoke step failed: skip this tick.
    bool TickAutomation();
    FString StorePreviewId;
    UPROPERTY() TObjectPtr<AActor> StorePreviewCamera;
    double StoreFrameSeconds = 0;
    int32 StoreFrameCount = 0;
    FString ActiveStoreKitId;
    bool bStoreTour = false;
    int32 StoreTourSeed = 1;
    TSharedPtr<SWidget> StoreTourOverlay;
    FString StoreTourNotice;
    bool StartStoreTour(const FString& Id, bool bRandom = false);
    bool StoreTourCommand(FName Action);
    void TickStoreTour();
    TMap<FString, FString> StoreCategoryOverrides;
    TFunction<void(const TMap<FString, FString>&)> OnStoreCategoriesChanged;
    // In-game shelf arranging (R while the shop is closed). See MarketArrange.cpp.
    // The player aims at any shelf: a ghost of the product in hand shows where it would go (green = fits);
    // click/E puts it there. The same product can be placed any number of times.
    bool bArrange = false;
    FPlanogramPlacement ArrangeHand;          // product + facings/orientation/stack to place next
    int32 ArrangeMoving = INDEX_NONE;         // block picked up with F (moved by the next click)
    bool bArrangeAim = false;                 // crosshair is on a shelf
    int32 AimFixture = INDEX_NONE;
    FString AimFace = TEXT("front");
    int32 AimLevel = 0;
    float AimX = 0.f;
    int32 ArrangeHover = INDEX_NONE;          // block under the crosshair
    MarketPlanogramEdit::FBlockPlan ArrangePlan; // what a click would do right now
    // Panel text (MarketHudWidget reads these every frame).
    FString ArrangeTitle;
    FString ArrangeRow;
    FString ArrangeHandTitle;
    FString ArrangeHandText;
    FString ArrangeTargetTitle;
    FString ArrangeTargetText;
    FString ArrangeStatus;
    float ArrangeRowFill = 0.f;
    bool bArrangeStatusOk = false;
    TArray<TPair<FString, FString>> ArrangeKeys;
    // Ghost preview: product meshes + a coloured strip on the shelf lip + a floating label.
    UPROPERTY() TObjectPtr<AActor> GhostHolder = nullptr;
    UPROPERTY() TObjectPtr<AActor> GhostStrip = nullptr;
    UPROPERTY() TObjectPtr<AActor> HoverStrip = nullptr;
    // Grey strips under every block of the aimed row, so reserved but empty blocks are visible too.
    UPROPERTY() TArray<TObjectPtr<AActor>> RowMarks;
    FString RowMarksKey;
    int32 ArrangeVersion = 0; // bumps on every plan change (refreshes the row strips)
    UPROPERTY() TObjectPtr<UTextRenderComponent> GhostLabel = nullptr;
    UPROPERTY() TObjectPtr<UStaticMesh> GhostMesh = nullptr;
    UPROPERTY() TArray<TObjectPtr<UMaterialInterface>> GhostMaterials;
    FProductLook GhostLook;
    FString GhostLookProduct;
    FString GhostShapeKey;
    FString GhostStripKey;
    FString HoverStripKey;
    TArray<TWeakObjectPtr<UStaticMeshComponent>> GhostItems;
    // Every actor that shows shelf stock or price cards; rebuilt when the plan changes.
    TArray<TWeakObjectPtr<AActor>> ShelfContentActors;
    bool ArrangeCommand(FName Action);
    void TickArrange();
    void UpdateArrangeAim();
    void UpdateArrangeView();
    void UpdateGhost();
    void ClearGhost();
    void ArrangeApplied(bool bChanged, const FString& Text, const FString& ProductId);
    FString ArrangeHint() const;
    void ExitArrange(const FString& Text);
    void RebuildShelfContents();
    // Fixture the player stands in front of (and which aisle side), or INDEX_NONE.
    int32 NearbyFixture(FString* OutFace = nullptr) const;
    FProductLook LoadProductLook(int32 Index);
    // World transforms of one block's units (front row first, centre out). SeedBase/SeqStart keep the
    // hand-stocked jitter stable. bFrontRowOnly: the ghost preview shows only the visible front row.
    void BlockSlotTransforms(const FProductLook& Look, const FMarketProduct& Product, const FPlanogramPlacement& Placement,
        float CenterX, int32 SeedBase, int32 SeqStart, bool bFrontRowOnly, TArray<FTransform>& OutTransforms, TArray<int32>* OutRows, FVector* OutApproach) const;
    void BuildShelfContents();
    void Command(FName Action);
    void Notify(const FString& Text);
    void RefreshLabels();
    FString ProductName(int32 Index) const;
    FString ContextHint() const;
    int32 NearbyShelf() const;
    bool NearOffice() const;
    bool NearCounter() const;
    int32 QueueSize() const;
    int32 ReservedUnits(int32 Product) const;
    TArray<int32> AvailableShelfUnits() const;
    FVector CustomerBrowseLocation(int32 Product) const;
    void ResolveCustomerItem(FMarketCustomer& Customer);
    void AdvanceCustomerList(FMarketCustomer& Customer);
    FString LoyaltySummary() const;
    // Rival shops (MarketRivals.h): the factor that applies to every product today (weekend sales) and the
    // rival price factor for one product (its aisle). RivalAisles = the product categories rivals can target.
    float RivalDiscount() const;
    float RivalPriceFactor(int32 ProductIndex) const;
    TArray<FString> RivalAisles;
    bool bWeekJustEnded = false;             // the last closed day finished a week: the report shows the week
    FString RivalNewsText() const;           // tomorrow's rival news (evening report)
    FString WeekReportText() const;          // last finished week (MarketCampaign)
    // Office line for one product: our price, the rival's price and the share of shoppers who accept it.
    FString PriceSummary(int32 Index) const;
    // Office line for one product: shelf, depot, rear door, yesterday's demand and the suggested cases (MarketOrderAdvice).
    FString OrderAdvice(int32 Index) const;
    // Day report: yesterday's biggest reasons for lost shoppers, one per line (MarketDemand::TopProblems).
    FString DayProblemsText() const;
    // Clickable management menu (G-059, MarketMenu.cpp + MarketMenuWidget). M opens it anywhere; the world pauses.
    bool bMenuOpen = false;
    int32 MenuPage = 0;                  // SMarketMenu::EPage
    bool bMenuDayReport = false;         // opened by the day close: shows "Yeni gune basla"
    bool bLightTheme = true;             // GameUserSettings.ini [MirasMarket.Menu] LightTheme
    bool bMenuSettingsLoaded = false;
    bool bMenuAction = false;            // a menu button is running Command(): the office-desk distance is not needed
    int32 MenuProduct = 0;               // product shown on the price page
    int32 MapProvince = INDEX_NONE;      // province selected on the Subeler map (MarketMapData index)
    // G-086: there is no default start province. A fresh game (or an empty save slot) waits on the new-game
    // screen until the player picks a country and a province; the menu cannot be closed meanwhile.
    bool bNeedStart = false;
    bool bNewGameAsk = false;            // the player opened the new-game screen (can go back)
    void AskNewGame();
    // Game speed (G-075): Space pauses, 1/2/3 run the world at 1x/2x/3x (global time dilation). The menu has the
    // same controls and the world keeps running behind it. Automation runs never touch it.
    int32 GameSpeed = 1;
    bool bTimePaused = false;
    bool bPauseInMenu = false;           // GameUserSettings.ini [MirasMarket.Menu] PauseInMenu (karar A02: player's choice)
    void SetPauseInMenu(bool bPause);
    int32 MenuTextSize = 1;              // GameUserSettings.ini [MirasMarket.Menu] TextSize: 0 small, 1 normal, 2 large
    void SetGameSpeed(int32 Speed);
    void SetTimePaused(bool bPaused);
    void ApplyGameSpeed();
    void SetMenuTextSize(int32 Size);
    // x size of the menu and the HUD (MenuTextSize).
    float UiTextFactor() const;
    // Notices for the HUD and the menu's to-do list, most urgent first (cached for the current frame).
    const TArray<FMarketTodo>& Todos() const;
    mutable TArray<FMarketTodo> TodoCache;
    mutable uint64 TodoFrame = MAX_uint64;
    void OpenMenu(int32 Page, bool bDayReport = false);
    void CloseMenu();
    void OpenDayReport();
    void MenuCommand(FName Action, int32 Product = INDEX_NONE);
    // Personnel and tax decisions on one person (MarketStaff.h): HireCandidate, Fire, Raise, DayOff, Warn
    // (Id = employee/candidate id), HireAccountant, PayTax, HrAutoReplace. From the menu; no desk distance needed.
    void StaffCommand(FName Action, int32 Id = INDEX_NONE);
    void ClearOrderDraft();
    // G-086f order list window: move a line's cases to another product (as many as its limits allow; returns
    // the cases moved) and drop one line.
    int32 MoveOrderDraft(int32 From, int32 To);
    void ClearOrderLine(int32 Product);
    void ToggleMenuTheme();
    void LoadMenuSettings();
    void LoadCatalog();
    // The catalog's 2011 costs and list prices; Products carries today's (MarketDirector::ApplyPrices).
    TArray<FMarketProduct> CatalogBase;
    void RefreshPrices();
    void ApplyCapacities();
    int32 FillAllShelves();
    UMaterialInterface* Surface(EMarketSurface Kind);
    AActor* SurfaceBox(FVector Location, FVector Size, EMarketSurface Kind, bool bCollision = true);
    void ApplyKitSurfaces(class UStaticMeshComponent* Component);
    void LoadPlanogram();
    FVector ProductFixtureLocation(int32 Index) const;
    void BuildStore();
    void BuildShelfItems(int32 Index);
    void RefreshShelfItems();
    void SpawnCustomer();
    void Checkout();
    void CloseShop();
    bool SaveCampaign();
    // bQuiet: the automatic load at start does not announce a missing save.
    void LoadCampaign(bool bQuiet = false);
    // G-076: three save slots. The last used slot is remembered in GameUserSettings.ini and loaded at start.
    static constexpr int32 SlotCount = 3;
    int32 ActiveSlot = 1;
    static FString SlotName(int32 Slot);     // save-game slot name (slot 1 keeps the old v1 file)
    bool SlotExists(int32 Slot) const;
    FString SlotSummary(int32 Slot) const;   // cached: "Gun 42, 12 Nisan 2011" or "bos"
    TArray<FString> SlotSummaries;
    void RefreshSlotSummaries();
    void SelectSlot(int32 Slot);              // switch campaign: load it, or start a new one there
    void ResetCampaign();                     // fresh campaign in the active slot
    // G-084 (karar L02-L03): the inherited shop (relative, 1 cashier + 2 stockers, shelves 40-75 % full). Smoke and
    // capture runs keep the prototype's empty shop.
    void StartShop();
    // Menu "Yeni oyun": a fresh campaign in the active slot, in the chosen country and city.
    void StartNewCampaign(const FString& Country, const FString& City);
    AActor* Box(FVector Location, FVector Size, FLinearColor Color, bool bCollision = true);
    // bCenter: text is vertically centered on Location (signs, tags); otherwise it hangs from it.
    UTextRenderComponent* Label(FVector Location, FRotator Rotation, const FString& Text, float Size = 20, FColor Color = FColor::White, bool bCenter = false);
};

UCLASS()
class AMarketHUD : public AHUD
{
    GENERATED_BODY()
public:
    // The HUD is a Slate overlay (MarketHudWidget); DrawHUD only creates it once the market is ready.
    virtual void DrawHUD() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    TSharedPtr<SMarketMenu> MenuWidget() const { return Menu; }
private:
    TSharedPtr<SWidget> Overlay;
    TSharedPtr<SMarketMenu> Menu;           // G-059 management menu (collapsed until M)
};

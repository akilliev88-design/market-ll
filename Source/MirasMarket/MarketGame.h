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
#include "MarketGame.generated.h"

class UTextRenderComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;
class UInstancedStaticMeshComponent;
class SWidget;

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
    void NextMood();
    void ToggleFullscreen();
    void Quit();
};

USTRUCT()
struct FMarketCustomer
{
    GENERATED_BODY()
    UPROPERTY() TObjectPtr<AActor> Actor = nullptr;
    int32 Product = 0;
    int32 Quantity = 1;
    int64 QuotedPrice = 0;
    float Age = 0;
    int32 Stage = 0;
    int32 QueueTicket = INDEX_NONE;
    // MetaHuman visual state (unused for the simple box shoppers).
    bool bHuman = false;
    MarketPeople::FShopper Shopper;
    // Aisle waypoints before the stage target (shoppers walk around fixtures, not through them).
    TArray<FVector> Route;
    int32 RouteStage = -1;
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
    UPROPERTY() TArray<FMarketCustomer> Customers;
    // Price text on every shelf tag; ShelfLabelProduct holds the product index of each label.
    UPROPERTY() TArray<TObjectPtr<UTextRenderComponent>> ShelfLabels;
    TArray<int32> ShelfLabelProduct;
    UPROPERTY() TMap<int32, TObjectPtr<UMaterialInterface>> SurfaceCache;
    // Test phase: shelves can be filled without warehouse stock or cash (F2 toggles, F3 fills all).
    // Default comes from DefaultGame.ini [/Script/MirasMarket.MarketGameMode] bTestModeAtStart.
    UPROPERTY(Config) bool bTestModeAtStart = true;
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
    void Command(FName Action);
    void Notify(const FString& Text);
    void RefreshLabels();
    FString ProductName(int32 Index) const;
    FString ContextHint() const;
    int32 NearbyShelf() const;
    bool NearOffice() const;
    bool NearCounter() const;
    int32 QueueSize() const;
    float RivalDiscount() const;
    void LoadCatalog();
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
    void LoadCampaign();
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
private:
    TSharedPtr<SWidget> Overlay;
};

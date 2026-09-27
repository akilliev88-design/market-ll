#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/HUD.h"
#include "GameFramework/SaveGame.h"
#include "MarketEconomy.h"
#include "MarketGame.generated.h"

class UTextRenderComponent;
class UStaticMesh;
class UStaticMeshComponent;

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
};

struct FMarketQueueRules
{
    static int32 Rank(const TArray<FMarketCustomer>& Customers, int32 CustomerIndex);
    static int32 FindFront(const TArray<FMarketCustomer>& Customers);
};

UCLASS()
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
    UPROPERTY() TArray<FMarketCustomer> Customers;
    UPROPERTY() TArray<TObjectPtr<UTextRenderComponent>> ShelfLabels;
    UPROPERTY() TObjectPtr<UStaticMesh> Cube;
    UPROPERTY() TObjectPtr<UStaticMesh> Sphere;
    // Visible product units on each shelf; flattened, ShelfItemStart/Count index per product.
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> ShelfItems;
    TArray<int32> ShelfItemStart;
    TArray<int32> ShelfItemCount;
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
    bool bCaptureRequested = false;
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
    void BuildStore();
    void BuildShelfItems(int32 Index);
    void RefreshShelfItems();
    void SpawnCustomer();
    void Checkout();
    void CloseShop();
    bool SaveCampaign();
    void LoadCampaign();
    AActor* Box(FVector Location, FVector Size, FLinearColor Color, bool bCollision = true);
    UTextRenderComponent* Label(FVector Location, FRotator Rotation, const FString& Text, float Size = 20, FColor Color = FColor::White);
};

UCLASS()
class AMarketHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};

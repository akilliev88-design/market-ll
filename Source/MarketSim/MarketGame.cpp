#include "MarketGame.h"
#include "MarketChains.h"
#include "MarketStaff.h"
#include "MarketTelevisionDisplay.h"
#include "MarketSimulation.h"
#include "MarketWorldText.h"
#include "MarketCountry.h"
#include "MarketStart.h"
#include "MarketCalendar.h"
#include "MarketVisuals.h"
#include "ProductCatalog.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "MarketHudWidget.h"
#include "MarketMenuWidget.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
    constexpr int32 CampaignSeedBase = 7919; // the world's random stream base (any fixed number)
}

namespace
{
    // Price tags and notices: "2.40 TL".
    FString Money(int64 Value) { return MarketCountry::Money(Value); } // G-084
    AMarketGameMode* GetMarket(const AActor* Actor) { return Cast<AMarketGameMode>(UGameplayStatics::GetGameMode(Actor)); }
}

AMarketCharacter::AMarketCharacter()
{
    GetCapsuleComponent()->InitCapsuleSize(30, 88);
    auto* Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(GetCapsuleComponent());
    Camera->SetRelativeLocation(FVector(0, 0, 65));
    // 90 degrees (engine default) stretches the edges like a wide lens; a human-eye field reads less "model kit".
    Camera->SetFieldOfView(78.f);
    Camera->bUsePawnControlRotation = true;
    GetCharacterMovement()->MaxWalkSpeed = 330;
    bUseControllerRotationYaw = true;
}

void AMarketCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    FInputKeyBinding CategoryKey(FInputChord(EKeys::T), IE_Pressed);
    CategoryKey.KeyDelegate.GetDelegateForManualSet().BindLambda([this]() { SendCommand("Category"); });
    Input->KeyBindings.Add(CategoryKey);
    Input->BindAxis("MoveForward", this, &AMarketCharacter::Forward);
    Input->BindAxis("MoveRight", this, &AMarketCharacter::Right);
    Input->BindAxis("Turn", this, &AMarketCharacter::Turn);
    Input->BindAxis("LookUp", this, &AMarketCharacter::Look);
    Input->BindAction("Interact", IE_Pressed, this, &AMarketCharacter::Interact);
    Input->BindAction("ToggleShop", IE_Pressed, this, &AMarketCharacter::ToggleShop);
    Input->BindAction("NextProduct", IE_Pressed, this, &AMarketCharacter::NextProduct);
    Input->BindAction("Order", IE_Pressed, this, &AMarketCharacter::Order);
    Input->BindAction("PriceUp", IE_Pressed, this, &AMarketCharacter::PriceUp);
    Input->BindAction("PriceDown", IE_Pressed, this, &AMarketCharacter::PriceDown);
    Input->BindAction("Hire", IE_Pressed, this, &AMarketCharacter::Hire);
    Input->BindAction("Expand", IE_Pressed, this, &AMarketCharacter::Expand);
    Input->BindAction("Save", IE_Pressed, this, &AMarketCharacter::Save);
    Input->BindAction("Load", IE_Pressed, this, &AMarketCharacter::Load);
    Input->BindAction("Brands", IE_Pressed, this, &AMarketCharacter::Brands);
    Input->BindAction("NewCampaign", IE_Pressed, this, &AMarketCharacter::NewCampaign);
    Input->BindAction("ToggleDetails", IE_Pressed, this, &AMarketCharacter::ToggleDetails);
    Input->BindAction("TestMode", IE_Pressed, this, &AMarketCharacter::ToggleTestMode);
    Input->BindAction("FillAll", IE_Pressed, this, &AMarketCharacter::FillAll);
    Input->BindKey(EKeys::F10, IE_Pressed, this, &AMarketCharacter::NextStore);
    Input->BindKey(FInputChord(EKeys::F10, true, false, false, false), IE_Pressed, this, &AMarketCharacter::PreviousStore);
    Input->BindKey(EKeys::F7, IE_Pressed, this, &AMarketCharacter::RandomizeShelves);
    Input->BindAction("Mood", IE_Pressed, this, &AMarketCharacter::NextMood);
    Input->BindAction("Fullscreen", IE_Pressed, this, &AMarketCharacter::ToggleFullscreen);
    Input->BindAction("Quit", IE_Pressed, this, &AMarketCharacter::Quit);
    // Shelf arranging (R) and product browsing: every key forwards its action name to the game mode.
    static const TCHAR* Forwarded[] = { TEXT("Arrange"), TEXT("ArrangePlace"), TEXT("ArrangeUp"), TEXT("ArrangeDown"), TEXT("ArrangeLeft"),
        TEXT("ArrangeRight"), TEXT("ArrangeRemove"), TEXT("ArrangeTurn"), TEXT("ArrangeStack"), TEXT("ArrangeGrab"), TEXT("ArrangePick"),
        TEXT("ArrangeNext"), TEXT("ArrangePrev"), TEXT("ArrangeGapDown"), TEXT("ArrangeGapUp"), TEXT("PrevProduct"),
        TEXT("HireStocker"), TEXT("FireStocker"), TEXT("PayDebt"), TEXT("ConfirmOrder"), TEXT("RemoveOrder"), TEXT("SuggestOrder"), TEXT("Menu") };
    for (const TCHAR* Name : Forwarded)
    {
        FInputActionBinding& Binding = Input->BindAction<FMarketCommandDelegate>(FName(Name), IE_Pressed, this, &AMarketCharacter::SendCommand, FName(Name));
        Binding.bExecuteWhenPaused = FCString::Strcmp(Name, TEXT("Menu")) == 0; // the menu opens while time stands still
    }
    // G-075 game speed: Space pauses, 1/2/3 set the speed. They work while the world is paused.
    static const TCHAR* TimeKeys[] = { TEXT("TimePause"), TEXT("Speed1"), TEXT("Speed2"), TEXT("Speed3") };
    for (const TCHAR* Name : TimeKeys)
        Input->BindAction<FMarketCommandDelegate>(FName(Name), IE_Pressed, this, &AMarketCharacter::SendCommand, FName(Name)).bExecuteWhenPaused = true;
}
void AMarketCharacter::SendCommand(FName Action) { if (auto* Game = GetMarket(this)) Game->Command(Action); }
void AMarketCharacter::Forward(float Value) { AddMovementInput(GetActorForwardVector(), Value); }
void AMarketCharacter::Right(float Value) { AddMovementInput(GetActorRightVector(), Value); }
void AMarketCharacter::Turn(float Value) { AddControllerYawInput(Value); }
void AMarketCharacter::Look(float Value) { AddControllerPitchInput(Value); }
#define MARKET_ACTION(Method, Name) void AMarketCharacter::Method() { if (auto* Game = GetMarket(this)) Game->Command(Name); }
MARKET_ACTION(Interact, "Interact")
MARKET_ACTION(ToggleShop, "ToggleShop")
MARKET_ACTION(NextProduct, "NextProduct")
MARKET_ACTION(Order, "Order")
MARKET_ACTION(PriceUp, "PriceUp")
MARKET_ACTION(PriceDown, "PriceDown")
MARKET_ACTION(Hire, "Hire")
MARKET_ACTION(Expand, "Expand")
MARKET_ACTION(Save, "Save")
MARKET_ACTION(Load, "Load")
MARKET_ACTION(Brands, "Brands")
MARKET_ACTION(NewCampaign, "NewCampaign")
MARKET_ACTION(ToggleDetails, "ToggleDetails")
MARKET_ACTION(ToggleTestMode, "TestMode")
MARKET_ACTION(FillAll, "FillAll")
MARKET_ACTION(NextStore, "TourNext")
MARKET_ACTION(PreviousStore, "TourPrevious")
MARKET_ACTION(RandomizeShelves, "RandomFill")
MARKET_ACTION(NextMood, "Mood")
MARKET_ACTION(ToggleFullscreen, "Fullscreen")
#undef MARKET_ACTION
void AMarketCharacter::Quit() { if (auto* Game = GetMarket(this)) if (Game->IsBranchVisit()) { Game->EndBranchVisit(); return; } UKismetSystemLibrary::QuitGame(this, Cast<APlayerController>(GetController()), EQuitPreference::Quit, false); }

AMarketGameMode::AMarketGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    DefaultPawnClass = AMarketCharacter::StaticClass();
    HUDClass = AMarketHUD::StaticClass();
}

void AMarketGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    GetWorld()->SpawnActor<APlayerStart>(FVector(0, -180, 100), FRotator(0, 90, 0));
}

void AMarketGameMode::BeginPlay()
{
    Super::BeginPlay();
    Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    LoadCatalog();
    LoadPlanogram();
    State.Initialize(Products);
    State.RivalSeed = FMath::Rand();
    MarketCountry::SetActive(State.CountryId, State.RivalSeed); MarketEras::Activate(State); // G-084: currency and economy of the country pack
    RefreshPrices();
    OrderDraftCases.Init(0, Products.Num());
    ApplyCapacities();
    if (bUseMetaHumans)
    {
        People = MarketPeople::Load();
        for (const TSubclassOf<AActor>& Class : People.Classes) PeopleAssets.Add(Class.Get());
        if (People.Walk.IsValid()) PeopleAssets.Add(People.Walk.Get());
        if (People.Idle.IsValid()) PeopleAssets.Add(People.Idle.Get());
    }
    bTestMode = bTestModeAtStart && !FParse::Param(FCommandLine::Get(), TEXT("SimSmoke"));
    // Opening inventory waits in the warehouse. Test mode keeps free stocking controls, but the player
    // still starts by experiencing the empty shop and choosing what to place on the shelves.
    if (FParse::Value(FCommandLine::Get(), TEXT("SimStorePreview="), StorePreviewId))
    {
        // Art review dresses every department with the full catalog, including preparation prototypes.
        // This does not publish products or change campaign stock.
        TArray<FString> Errors; MarketCatalog::LoadFile(MarketCatalog::DefaultPath(), Products, Errors);
        for (auto& Product : Products) Product.bActive = true;
        return;
    }
    FString TourId;
    if (FParse::Value(FCommandLine::Get(), TEXT("SimStoreTour="), TourId))
    {
        TArray<FString> Errors; MarketCatalog::LoadFile(MarketCatalog::DefaultPath(), Products, Errors);
        for (auto& Product : Products) Product.bActive = true;
        State.Initialize(Products); bTestMode = true; bStoreTour = true;
        if (!StartStoreTour(TourId, true)) FPlatformMisc::RequestExitWithStatus(false, 1);
        return;
    }
    BuildStore();
    RefreshDeliveryCrates();
    if (auto* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        if (APawn* Pawn = PC->GetPawn()) Pawn->SetActorLocation(FVector(0, -180, 100));
        PC->SetControlRotation(FRotator(0, 90, 0));
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
    }
    Random.Initialize(CampaignSeedBase);
    RefreshLabels();
    // G-076: the last used slot comes back by itself, so a forgotten F9 never overwrites a long campaign.
    const bool bAutomation = FParse::Param(FCommandLine::Get(), TEXT("SimSmoke")) || FParse::Param(FCommandLine::Get(), TEXT("SimCapture")) || FParse::Param(FCommandLine::Get(), TEXT("SimMenuCapture")) || FParse::Param(FCommandLine::Get(), TEXT("BranchVisitReview"));
    int32 LastSlot = 1;
    if (!bAutomation && GConfig && GConfig->GetInt(TEXT("MarketSim.Menu"), TEXT("LastSlot"), LastSlot, GGameUserSettingsIni)) ActiveSlot = FMath::Clamp(LastSlot, 1, SlotCount);
    RefreshSlotSummaries();
    const int32 DayBefore = State.Day;
    if (!bAutomation && SlotExists(ActiveSlot)) LoadCampaign(true);
    if (bTestMode) State.bUsedTestMode = true;
    if (State.Day != DayBefore) { UE_LOG(LogTemp, Display, TEXT("MarketSim: slot %d loaded (day %d)."), ActiveSlot, State.Day); return; }
    if (bAutomation)
    {
        // Smoke / capture runs keep the prototype's empty shop and never wait for a choice.
        StartShop();
        SyncWorkers();
        Notify(MarketStart::IntroText(State));
    }
    else bNeedStart = true; // G-086: no save yet; the new-game screen (country and province) opens in Tick
    UE_LOG(LogTemp, Display, TEXT("MarketSim ready: %d products, %d fixtures, starting cash %lld kurus."), Products.Num(), Planogram.Fixtures.Num(), State.Cash);
}

void AMarketGameMode::LoadCatalog()
{
    TArray<FString> Errors;
    MarketCatalog::LoadFile(MarketCatalog::DefaultPath(), Products, Errors);
    for (const FString& Error : Errors) UE_LOG(LogTemp, Warning, TEXT("MarketSim catalog: %s"), *Error);
    Products.RemoveAll([](const FMarketProduct& P) { return !P.bActive; }); // preparation list stays in the studio
    if (Products.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("MarketSim: products.json has no valid products; using placeholders."));
        for (int32 I = 0; I < 6; ++I)
        {
            FMarketProduct P;
            P.Id = FString::Printf(TEXT("fallback_%d"), I);
            P.RealName = P.FictionalName = FString::Printf(TEXT("Urun %d"), I + 1);
            P.Cost = 100; P.BasePrice = 200; Products.Add(P);
        }
    }
    CatalogBase = Products;
}

void AMarketGameMode::RefreshPrices()
{
    MarketDirector::ApplyPrices(State, CatalogBase, Products); // monthly price list, wholesaler discount (G-063)
}

void AMarketGameMode::LoadPlanogram()
{
    TArray<FString> Errors;
    MarketPlanogram::LoadFile(MarketPlanogram::DefaultPath(), Planogram, Errors);
    // The shelves show exactly the authored plan (Raf Plani editor or in-game R mode). Products that are
    // not placed are not on sale; nothing is placed or widened automatically. Only depth is physical.
    MarketPlanogram::ResolvePositions(Planogram, Products); // old files: keep what the old packing showed
    MarketPlanogram::FitDepth(Planogram, Products);
    MarketPlanogram::FindOverflows(Planogram, Products, Errors);
    for (const FString& Error : Errors) UE_LOG(LogTemp, Warning, TEXT("MarketSim planogram: %s"), *Error);
    float FurthestY = 320.f;
    for (const FPlanogramFixture& Fixture : Planogram.Fixtures) FurthestY = FMath::Max(FurthestY, Fixture.Location.Y);
    StoreRows = FMath::Max(2, FMath::CeilToInt(FurthestY / 320.f));
}

void AMarketGameMode::ApplyCapacities()
{
    TArray<int32> Capacities;
    for (const FMarketProduct& Product : Products)
    {
        // 0 = not on a shelf: no shelf stock, customers do not ask for it (units wait in the warehouse).
        Capacities.Add(MarketPlanogram::ProductCapacity(Planogram, Products, Product.Id));
    }
    TArray<int32> HeldBefore;
    for (const FMarketStock& Item : State.Stock) HeldBefore.Add(Item.Shelf + Item.Warehouse);
    const int32 Discarded = State.ApplyShelfCapacities(Capacities);
    if (Discarded <= 0) return;
    // Paid-for units that fit neither the smaller shelf nor the full depot go back to the wholesaler at half price:
    // the other half is a loss the next day report shows.
    int64 Refund = 0;
    for (int32 I = 0; I < State.Stock.Num() && I < HeldBefore.Num() && I < Products.Num(); ++I)
    {
        const int32 Gone = HeldBefore[I] - (State.Stock[I].Shelf + State.Stock[I].Warehouse);
        if (Gone > 0) Refund += static_cast<int64>(Gone) * State.UnitCost(I, Products) / 2;
    }
    State.Cash += Refund;
    State.PendingLoss += Refund;
    MarketLedger::Post(State, MarketLedger::EAccount::Divestment, Refund); // C3 (B2)
    MarketLedger::Post(State, MarketLedger::EAccount::Shrinkage, -Refund, false); // the lost half
    UE_LOG(LogTemp, Warning, TEXT("MarketSim: %d units did not fit shelf + storage after a planogram change."), Discarded);
    Notify(FString::Printf(TEXT("Rafa ve depoya s\u0131\u011fmayan %d \u00fcr\u00fcn toptanc\u0131ya yar\u0131 fiyat\u0131na geri verildi (%s)."), Discarded, *Money(Refund)));
}

int32 AMarketGameMode::FillAllShelves()
{
    int32 Added = 0;
    for (int32 I = 0; I < State.Stock.Num(); ++I) Added += State.FillShelfFree(I);
    return Added;
}

UMaterialInterface* AMarketGameMode::Surface(EMarketSurface Kind)
{
    const int32 Key = static_cast<int32>(Kind);
    if (const TObjectPtr<UMaterialInterface>* Found = SurfaceCache.Find(Key)) return *Found;
    UMaterialInterface* Material = MarketVisuals::CreateSurface(this, Kind);
    SurfaceCache.Add(Key, Material);
    return Material;
}

AActor* AMarketGameMode::SurfaceBox(FVector Location, FVector Size, EMarketSurface Kind, bool bCollision)
{
    AActor* Actor = Box(Location, Size, MarketVisuals::SurfaceColor(Kind), bCollision);
    if (UMaterialInterface* Material = Surface(Kind))
        if (auto* MeshActor = Cast<AStaticMeshActor>(Actor)) MeshActor->GetStaticMeshComponent()->SetMaterial(0, Material);
    return Actor;
}

void AMarketGameMode::ApplyKitSurfaces(UStaticMeshComponent* Component)
{
    // Imported Blender kits keep their geometry; their slots are re-skinned with the store's
    // surface library (walnut, light shelving, acrylic bins, granular bulk food, emissive fixtures).
    if (!Component || !Component->GetStaticMesh()) return;
    const TArray<FStaticMaterial>& Slots = Component->GetStaticMesh()->GetStaticMaterials();
    for (int32 Slot = 0; Slot < Slots.Num(); ++Slot)
    {
        FString Name = Slots[Slot].MaterialSlotName.ToString();
        if (Slots[Slot].MaterialInterface) Name += TEXT(" ") + Slots[Slot].MaterialInterface->GetName();
        EMarketSurface Kind;
        if (!MarketVisuals::SurfaceForSlot(Name, Kind)) continue;
        if (UMaterialInterface* Material = Surface(Kind)) Component->SetMaterial(Slot, Material);
    }
}

FVector AMarketGameMode::ProductFixtureLocation(int32 Index) const
{
    // Floor point in front of the product's primary shelf block (where a shopper stands).
    if (ShelfApproach.IsValidIndex(Index) && ShelfApproach[Index].Num() > 0) return ShelfApproach[Index][0];
    if (!Products.IsValidIndex(Index)) return FVector::ZeroVector;
    if (const FPlanogramPlacement* Placement = Planogram.FindPlacement(Products[Index].Id))
        if (const FPlanogramFixture* Fixture = Planogram.FindFixture(Placement->FixtureId)) return Fixture->Location + FVector(0, -105, 0);
    return FVector::ZeroVector;
}

AActor* AMarketGameMode::Box(FVector Location, FVector Size, FLinearColor Color, bool bCollision)
{
    auto* Actor = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    auto* Mesh = Actor->GetStaticMeshComponent();
    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetStaticMesh(Cube);
    Mesh->SetWorldScale3D(Size / 100);
    Mesh->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    auto* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Market.M_Market"));
    if (!Base) Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    auto* Material = UMaterialInstanceDynamic::Create(Base, Actor);
    Material->SetVectorParameterValue(TEXT("Color"), Color);
    Mesh->SetMaterial(0, Material);
    return Actor;
}

UTextRenderComponent* AMarketGameMode::Label(FVector Location, FRotator Rotation, const FString& Text, float Size, FColor Color, bool bCenter)
{
    auto* Actor = GetWorld()->SpawnActor<AActor>(Location, Rotation);
    auto* Component = NewObject<UTextRenderComponent>(Actor);
    Actor->SetRootComponent(Component);
    Component->RegisterComponent();
    Component->SetWorldLocationAndRotation(Location, Rotation);
    MarketWorldText::Apply(Component);
    Component->SetText(FText::FromString(Text));
    Component->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    if (bCenter) Component->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    Component->SetWorldSize(Size);
    Component->SetTextRenderColor(Color);
    return Component;
}

void AMarketGameMode::BuildStore()
{
    // Store depth grows with the number of shelf rows (3 products per row). Two rows = original v0.1 layout.
    const float Back = 400 + StoreRows * 320;
    const float Length = Back + 310;
    const float Middle = (Back - 300) / 2;
    const bool bTextured = MarketVisuals::HasSurfaceLibrary();
    SurfaceBox(FVector(0, Middle, -15), FVector(1250, Length, 30), EMarketSurface::Floor);
    Box(FVector(0, Middle, 355), FVector(1250, Length, 15), FLinearColor(.82f, .82f, .78f));
    SurfaceBox(FVector(-625, Middle, 165), FVector(20, Length, 360), EMarketSurface::Wall);
    SurfaceBox(FVector(625, Middle, 165), FVector(20, Length, 360), EMarketSurface::Wall);
    SurfaceBox(FVector(0, Back, 165), FVector(1250, 20, 360), EMarketSurface::Wall);
    SurfaceBox(FVector(-405, -300, 165), FVector(440, 20, 360), EMarketSurface::Wall);
    SurfaceBox(FVector(405, -300, 165), FVector(440, 20, 360), EMarketSurface::Wall);
    // Entrance is a visual opening with an invisible boundary for the prototype.
    auto* Boundary = Box(FVector(0, -310, 165), FVector(360, 20, 360), FLinearColor::Black);
    Boundary->SetActorHiddenInGame(true);
    Box(FVector(0, -1300, -25), FVector(1800, 1900, 20), FLinearColor(.15f, .17f, .18f), false);
    Box(FVector(0, -1750, 220), FVector(1200, 200, 440), FLinearColor(.18f, .23f, .32f), false);
    Label(FVector(0, -1635, 270), FRotator(0, 90, 0), TEXT("BEREKET MARKET\nRakibin buyumeye hazirlaniyor"), 40, FColor(250, 185, 64));
    // Store name on a terracotta band above the depot.
    SurfaceBox(FVector(0, Back - 12, 300), FVector(560, 3, 46), EMarketSurface::SignRed, false);
    Label(FVector(0, Back - 13.8f, 300), FRotator(0, -90, 0), TEXT("MIRAS MARKET"), 24, FColor::White, true);
    // The textured floor carries its own tile joints; the plain fallback gets thin grout strips.
    if (!bTextured)
    {
        for (float X = -540.f; X <= 540.f; X += 60.f)
            Box(FVector(X, Middle, 0.2f), FVector(0.8f, Length, 0.4f), FLinearColor(.58f, .54f, .47f), false);
        for (float Y = -270.f; Y < Back; Y += 60.f)
            Box(FVector(0, Y, 0.2f), FVector(1250.f, 0.8f, 0.4f), FLinearColor(.58f, .54f, .47f), false);
    }
    auto SpawnKit = [this](const FString& AssetPath, const FVector& Location, const FRotator& Rotation, bool bCollision) -> AStaticMeshActor*
    {
        UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *AssetPath);
        if (!Mesh) { UE_LOG(LogTemp, Warning, TEXT("MarketSim: store kit mesh missing: %s"), *AssetPath); return nullptr; }
        auto* Actor = GetWorld()->SpawnActor<AStaticMeshActor>(Location, Rotation);
        Actor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
        Actor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
        Actor->GetStaticMeshComponent()->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Actor->GetStaticMeshComponent()->SetCollisionProfileName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision"));
        ApplyKitSurfaces(Actor->GetStaticMeshComponent());
        return Actor;
    };

    // Wall shelves are planogram fixtures now (equipment wall_shelf_2400). Older planograms without
    // them still get the decorative perimeter shelves.
    const bool bWallFixtures = Planogram.Fixtures.ContainsByPredicate([](const FPlanogramFixture& F) { return F.EquipmentId == TEXT("wall_shelf_2400"); });
    if (!bWallFixtures)
    {
        const FString WallShelfPath = MarketPlanogram::Equipment(TEXT("wall_shelf_2400")).MeshPath;
        for (float Y : { 180.f, 430.f, 680.f })
        {
            SpawnKit(WallShelfPath, FVector(-596, Y, 0), FRotator(0, -90, 0), true);
            SpawnKit(WallShelfPath, FVector(596, Y, 0), FRotator(0, 90, 0), true);
        }
    }
    const FVector BulkLocation(0, 245, 0);
    // Mirrored FBX axis (see FPlanogramEquipment::MeshYaw): turn the island so its bins face the entrance.
    if (SpawnKit(TEXT("/Game/Environment/StoreKit/BulkIsland_1600/SM_BulkIsland_1600.SM_BulkIsland_1600"), BulkLocation, FRotator(0, 180, 0), true))
    {
        // Terracotta header over the wooden top sign (front face at y = +14 cm) and a price card on the
        // front edge of every tier, under its bin (create_store_kit.py: tiers 64 cm deep, 7.5 cm thick).
        SurfaceBox(BulkLocation + FVector(0, 13.2f, 158), FVector(130, 1.2f, 30), EMarketSurface::SignRed, false);
        Label(BulkLocation + FVector(0, 12.4f, 158), FRotator(0, -90, 0), TEXT("KURUYEM\u0130\u015e  /  D\u00d6KME"), 11, FColor::White, true);
        static const TCHAR* BulkPrices[] = { TEXT("1,90"), TEXT("2,40"), TEXT("1,15"), TEXT("3,20"), TEXT("0,99"), TEXT("2,75") };
        for (int32 Row = 0; Row < 3; ++Row)
            for (int32 Col = 0; Col < 4; ++Col)
            {
                const float TierZ = 48.f + Row * 35.f;
                const float TierFrontY = -18.f + Row * 6.f - 32.f;
                const FVector Tag = BulkLocation + FVector(-58.5f + Col * 39.f, TierFrontY - 0.3f, TierZ);
                SurfaceBox(Tag, FVector(10.f, 0.4f, 5.f), EMarketSurface::TagWhite, false);
                Label(Tag + FVector(0, -0.35f, 0), FRotator(0, -90, 0),
                    FString::Printf(TEXT("%s TL/100g"), BulkPrices[(Row * 4 + Col) % UE_ARRAY_COUNT(BulkPrices)]), 1.6f, FColor(25, 22, 20), true)->SetCullDistance(800);
            }
    }
    else Label(BulkLocation + FVector(0, -54, 157), FRotator(0, -90, 0), TEXT("KURUYEM\u0130\u015e  /  LOKUM"), 13, FColor(255, 225, 165));
    for (const FPlanogramFixture& Fixture : Planogram.Fixtures)
    {
        const FPlanogramEquipment Spec = MarketPlanogram::Equipment(Fixture.EquipmentId);
        const FVector P = Fixture.Location;
        const FRotator FixtureRotation(0, Fixture.Yaw, 0);
        const FTransform FixtureXf(FixtureRotation, P);
        if (!SpawnKit(Spec.MeshPath, P, FixtureRotation + FRotator(0, Spec.MeshYaw, 0), true))
        {
            SurfaceBox(P + FVector(0, 30, 67), FVector(120, 3, 134), EMarketSurface::ShelfBack);
            SurfaceBox(P + FVector(-58, -7, 67), FVector(5, 76, 134), EMarketSurface::ShelfMetal);
            SurfaceBox(P + FVector(58, -7, 67), FVector(5, 76, 134), EMarketSurface::ShelfMetal);
            SurfaceBox(P + FVector(0, -7, 10), FVector(120, 78, 20), EMarketSurface::WoodLight);
            for (int32 Level = 0; Level < Spec.Levels; ++Level)
            {
                const float Z = 18 + Level * 34;
                SurfaceBox(P + FVector(0, -8, Z), FVector(120, 80, 3), EMarketSurface::ShelfMetal);
                SurfaceBox(P + FVector(0, -45, Z - 2), FVector(116, 1.8f, 4), EMarketSurface::PriceRail);
            }
        }
        if (!Spec.bShowCategorySign) continue;
        // Category sign: on top of a gondola (readable from both aisles) or on a wall shelf's header.
        const FString SignText = MarketCatalog::UpperTurkish(Fixture.Label);
        if (Spec.bSignOnTop)
        {
            auto* Sign = SurfaceBox(FixtureXf.TransformPosition(FVector(0, 0, Spec.SignZ)), FVector(Spec.SignWidthCm, 2.5f, 22), EMarketSurface::SignRed, false);
            Sign->SetActorRotation(FixtureRotation);
            for (int32 Side = 0; Side < (Spec.bDoubleSided ? 2 : 1); ++Side)
            {
                auto* Text = Label(FixtureXf.TransformPosition(FVector(0, Side == 0 ? -1.45f : 1.45f, Spec.SignZ)), FixtureRotation + FRotator(0, Side == 0 ? -90.f : 90.f, 0),
                    SignText, 10, FColor::White, true);
                Text->SetCullDistance(2000);
                CategorySigns.Add(Text); CategorySignKeys.Add(Fixture.Id + (Side == 0 ? TEXT("/front") : TEXT("/back")));
            }
        }
        else
        {
            for (int32 Side = 0; Side < (Spec.bDoubleSided ? 2 : 1); ++Side)
            {
                const float SignY = Side == 0 ? Spec.SignY : -Spec.SignY;
                const float Direction = Side == 0 ? -1.f : 1.f;
                auto* Sign = SurfaceBox(FixtureXf.TransformPosition(FVector(0, SignY + Direction * .6f, Spec.SignZ)), FVector(Spec.SignWidthCm, 1.2f, 16), EMarketSurface::SignRed, false);
                Sign->SetActorRotation(FixtureRotation);
                auto* Text = Label(FixtureXf.TransformPosition(FVector(0, SignY + Direction * 1.4f, Spec.SignZ)), FixtureRotation + FRotator(0, Direction * 90.f, 0), SignText, 10, FColor::White, true);
                Text->SetCullDistance(2000); CategorySigns.Add(Text); CategorySignKeys.Add(Fixture.Id + (Side == 0 ? TEXT("/front") : TEXT("/back")));
            }
        }
    }
    RefreshCategorySigns();
    BuildShelfContents();
    // Detailed Blender fixtures replace the old blockouts while keeping interaction coordinates stable.
    if (!SpawnKit(TEXT("/Game/Environment/StoreKit/CheckoutLane_2500/SM_CheckoutLane_2500.SM_CheckoutLane_2500"), FVector(405, -45, 0), FRotator(0, 180, 0), true))
    {
        SurfaceBox(FVector(405, -45, 45), FVector(230, 90, 90), EMarketSurface::Wood);
        SurfaceBox(FVector(405, -45, 91.5f), FVector(236, 96, 3), EMarketSurface::CounterTop);
        SurfaceBox(FVector(445, -40, 98), FVector(36, 32, 10), EMarketSurface::DarkMetal);
    }
    Label(FVector(405, -105, 185), FRotator(0, -90, 0), TEXT("KASA  [E]"), 24);
    if (!SpawnKit(TEXT("/Game/Environment/StoreKit/OfficeDesk_1800/SM_OfficeDesk_1800.SM_OfficeDesk_1800"), FVector(-430, -30, 0), FRotator(0, 180, 0), true))
    {
        SurfaceBox(FVector(-430, -30, 42), FVector(210, 90, 84), EMarketSurface::Wood);
        SurfaceBox(FVector(-430, -30, 85.5f), FVector(214, 94, 3), EMarketSurface::CounterTop);
    }
    Label(FVector(-430, -85, 188), FRotator(0, -90, 0), TEXT("Y\u00d6NET\u0130M MASASI"), 18);
    // A working local market needs cold storage and a fresh-produce focal point, not only dry shelves.
    if (SpawnKit(TEXT("/Game/Environment/StoreKit/RefrigeratedWall_3000/SM_RefrigeratedWall_3000.SM_RefrigeratedWall_3000"), FVector(0, Back - 43, 0), FRotator(0, 180, 0), true))
        Label(FVector(0, Back - 88, 245), FRotator(0, -90, 0), TEXT("SO\u011eUK \u00dcR\u00dcNLER"), 16, FColor::White, true);
    if (SpawnKit(TEXT("/Game/Environment/StoreKit/ProduceIsland_2400/SM_ProduceIsland_2400.SM_ProduceIsland_2400"), FVector(350, Back - 190, 0), FRotator(0, 180, 0), true))
        Label(FVector(350, Back - 255, 152), FRotator(0, -90, 0), TEXT("MANAV"), 15, FColor(255, 238, 208), true);
    // Depot: stacked cartons of different sizes, slightly turned.
    for (int32 I = 0; I < 6; ++I)
    {
        if (I == 2 || I == 3) continue; // the rear refrigerator occupies the central depot wall
        FRandomStream Stack(301 + I);
        const FVector Base(-480 + I * 190, Back - 90, 0);
        const int32 Height = Stack.RandRange(1, 3);
        float Z = 0;
        for (int32 K = 0; K < Height; ++K)
        {
            const FVector Size(Stack.FRandRange(50, 72), Stack.FRandRange(40, 60), Stack.FRandRange(28, 45));
            auto* Carton = SurfaceBox(Base + FVector(Stack.FRandRange(-6, 6), Stack.FRandRange(-6, 6), Z + Size.Z * 0.5f), Size, EMarketSurface::Cardboard);
            Carton->SetActorRotation(FRotator(0, Stack.FRandRange(-8, 8), 0));
            Z += Size.Z;
        }
    }
    Label(FVector(-430, Back - 30, 140), FRotator(0, -90, 0), TEXT("DEPO"), 20);
    // The first store has a plain ceiling and flush light panels. Exposed services belong to large stores.
    for (float Y = 40.f; Y < Back; Y += 320.f)
        for (float X : { -300.f, 300.f })
        {
            SurfaceBox(FVector(X, Y, 346.f), FVector(125.f, 55.f, 3.f), EMarketSurface::ShelfMetal, false);
            SurfaceBox(FVector(X, Y, 344.f), FVector(120.f, 50.f, 1.f), EMarketSurface::Emissive, false);
        }
    Lighting = MarketVisuals::BuildStoreLighting(GetWorld(), Back);
    MarketVisuals::ApplyMood(Lighting, Mood);
}

void AMarketGameMode::BuildShelfContents()
{
    const auto TVProfiles = MarketTelevisionDisplay::Load();
    // Everything that depends on the planogram (stock meshes, price cards) is tracked in
    // ShelfContentActors so the in-game arrange mode can rebuild it after every change.
    for (int32 I = 0; I < Products.Num(); ++I)
    {
        BuildShelfItems(I);
        // One white price card on the price rail under every block of the product.
        for (const FPlanogramPlacement& Placement : Planogram.Placements)
        {
            if (Placement.ProductId != Products[I].Id) continue;
            const FPlanogramFixture* Fixture = Planogram.FindFixture(Placement.FixtureId);
            if (!Fixture) continue;
            const FPlanogramEquipment Spec = MarketPlanogram::Equipment(Fixture->EquipmentId);
            TArray<AActor*> TVActors;
            MarketTelevisionDisplay::Decorate(GetWorld(), Planogram, Products, Placement, ProductName(I), TVProfiles.Find(Products[I].Id), TVActors);
            for (auto* Actor : TVActors) ShelfContentActors.Add(Actor);
            const int32 Level = FMath::Clamp(Placement.Level, 0, Spec.Levels - 1);
            const bool bBack = Placement.Face == TEXT("back");
            const float FaceSign = bBack ? 1.f : -1.f;
            const FRotator FixtureRotation(0, Fixture->Yaw, 0);
            const FTransform FixtureXf(FixtureRotation, Fixture->Location);
            const float CenterX = MarketPlanogram::PlacementCenterX(Planogram, Products, Placement);
            const float RailZ = Spec.LevelTopZ[Level] + Spec.RailAboveTopZ;
            const FVector TagLocal(CenterX, FaceSign * (Spec.RailFrontY[Level] + 0.25f), RailZ);
            auto* Tag = SurfaceBox(FixtureXf.TransformPosition(TagLocal), FVector(9.f, 0.5f, 5.f), EMarketSurface::TagWhite, false);
            Tag->SetActorRotation(FixtureRotation);
            ShelfContentActors.Add(Tag);
            auto* Price = Label(FixtureXf.TransformPosition(TagLocal + FVector(0, FaceSign * 0.4f, 0)), FixtureRotation + FRotator(0, bBack ? 90.f : -90.f, 0),
                Money(State.Stock[I].Price), 2.0f, FColor(25, 22, 20), true);
            Price->SetCullDistance(900);
            ShelfContentActors.Add(Price->GetOwner());
            ShelfLabels.Add(Price);
            ShelfLabelProduct.Add(I);
        }
    }
}

FProductLook AMarketGameMode::LoadProductLook(int32 Index)
{
    // Studio products use their real package mesh + label material; others fall back to colored prototype boxes.
    const FMarketProduct& Product = Products[Index];
    FProductLook Look;
    TArray<UMaterialInterface*>& SlotMaterials = Look.Materials;
    if (!Product.MeshPath.IsEmpty())
    {
        Look.Mesh = LoadObject<UStaticMesh>(nullptr, *Product.MeshPath);
        if (!Look.Mesh) UE_LOG(LogTemp, Warning, TEXT("MarketSim: package mesh missing for %s: %s"), *Product.Id, *Product.MeshPath);
        for (const FString& Path : Product.Materials)
        {
            UMaterialInterface* Slot = Path.IsEmpty() ? nullptr : LoadObject<UMaterialInterface>(nullptr, *Path);
            if (!Path.IsEmpty() && !Slot) UE_LOG(LogTemp, Warning, TEXT("MarketSim: material missing for %s: %s"), *Product.Id, *Path);
            SlotMaterials.Add(Slot);
        }
    }
    if (UStaticMesh* Mesh = Look.Mesh)
    {
        Look.SourceBounds = Mesh->GetBoundingBox();
        Look.ItemScale = FVector(FMath::Clamp(Product.VisualScale, 0.001f, 1000.f));
        Look.ModelRotation = Product.VisualRotation;
        Look.PlacementOffset = Product.VisualOffsetCm;
        const int32 MaterialCount = FMath::Max(Mesh->GetStaticMaterials().Num(), SlotMaterials.Num());
        SlotMaterials.SetNum(MaterialCount);
        for (int32 MaterialSlot = 0; MaterialSlot < MaterialCount; ++MaterialSlot)
        {
            UMaterialInterface* Source = SlotMaterials[MaterialSlot];
            if (!Source) Source = Mesh->GetMaterial(MaterialSlot);
            const FString SlotName = Mesh->GetStaticMaterials().IsValidIndex(MaterialSlot)
                ? Mesh->GetStaticMaterials()[MaterialSlot].MaterialSlotName.ToString()
                : FString();
            SlotMaterials[MaterialSlot] = MarketVisuals::CreatePackageSurface(this, Source, SlotName);
        }
    }
    else
    {
        Look.Mesh = Cube;
        Look.ItemScale = FVector(.25f, .21f, .30f); // depth, width, height of the v0.1 placeholder boxes
        auto* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Market.M_Market"));
        if (!Base) Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
        auto* Colored = UMaterialInstanceDynamic::Create(Base, this);
        Colored->SetVectorParameterValue(TEXT("Color"), FLinearColor(Product.Color));
        SlotMaterials = { Colored };
    }
    return Look;
}

void AMarketGameMode::BlockSlotTransforms(const FProductLook& Look, const FMarketProduct& Product, const FPlanogramPlacement& Placement,
    float CenterX, int32 SeedBase, int32 SeqStart, bool bFrontRowOnly, TArray<FTransform>& OutTransforms, TArray<int32>* OutRows, FVector* OutApproach) const
{
    const FPlanogramFixture* Fixture = Planogram.FindFixture(Placement.FixtureId);
    if (!Fixture) return;
    const float Gap = MarketPlanogram::ItemGapCm;     // side by side
    const float RowGap = MarketPlanogram::RowGapCm;  // front to back
    const FPlanogramEquipment Spec = MarketPlanogram::Equipment(Fixture->EquipmentId);
    const int32 Level = FMath::Clamp(Placement.Level, 0, Spec.Levels - 1);
    const int32 Columns = FMath::Clamp(Placement.Facings, 1, MarketPlanogram::MaxFacings);
    const FRotator DisplayRotation = Placement.Orientation == 1 ? FRotator(0.f, 90.f, 0.f)
        : (Placement.Orientation == 2 ? FRotator(0.f, 0.f, 90.f) : FRotator::ZeroRotator);
    const FQuat DisplayModelRotation = DisplayRotation.Quaternion() * Look.ModelRotation.Quaternion();
    const FBox Bounds = Look.SourceBounds.TransformBy(FTransform(DisplayModelRotation, FVector::ZeroVector, Look.ItemScale));
    const FVector Size = Bounds.GetSize();   // after authored orientation: X = depth, Y = width, Z = height
    const FVector Center = Bounds.GetCenter();
    const float Bottom = Bounds.Min.Z;
    const FVector BoundsCenterOffset(Center.X, Center.Y, 0);
    // Never draw rows past the shelf's usable depth, even if the mesh is deeper than the catalog says.
    const int32 RowsByMesh = FMath::Max(1, FMath::FloorToInt((Spec.UsableDepthCm + RowGap) / FMath::Max(1.f, Size.X + RowGap)));
    const int32 Rows = bFrontRowOnly ? 1 : FMath::Clamp(FMath::Min(Placement.Depth, RowsByMesh), 1, MarketPlanogram::MaxDepth);
    const bool bBack = Placement.Face == TEXT("back");
    const float FaceSign = bBack ? 1.f : -1.f;
    const FTransform FixtureXf(FRotator(0, Fixture->Yaw, 0), Fixture->Location);
    const FRotator Facing(0, bBack ? 90.f : -90.f, 0);
    const float Front = bBack ? -Spec.FrontY : Spec.FrontY;
    const FVector BoundsOffset = Facing.RotateVector(BoundsCenterOffset) + FVector(0, 0, Bottom);
    const FVector CorrectedOffset = Facing.RotateVector(Look.PlacementOffset);
    const FQuat FinalRotation = Facing.Quaternion() * DisplayModelRotation;
    const int32 Stacks = MarketPlanogram::EffectiveStack(Planogram, Product, Placement); // same rule as capacity
    // Facings sit one catalog width apart (the width the planogram reserves), or the mesh width if it is wider.
    const float Pitch = FMath::Max(Size.Y, MarketPlanogram::OrientedWidthCm(Product, Placement.Orientation)) + Gap;
    // Every facing column, centre out (a partly sold block still looks faced up). The old stepping lost the
    // last column of even facing counts, so a 2-facing block showed one unit on twice the reserved width.
    TArray<int32> ColumnOrder;
    for (int32 Column = 0; Column < Columns; ++Column) ColumnOrder.Add(Column);
    const float Middle = (Columns - 1) * 0.5f;
    ColumnOrder.StableSort([Middle](int32 A, int32 B) { return FMath::Abs(A - Middle) < FMath::Abs(B - Middle); });
    int32 Seq = SeqStart;
    for (int32 Row = 0; Row < Rows; ++Row)
        for (const int32 Col : ColumnOrder)
            for (int32 StackIndex = 0; StackIndex < Stacks; ++StackIndex)
            {
                const FVector SlotLocal(CenterX + (Col - (Columns - 1) * 0.5f) * Pitch,
                    Front - FaceSign * (Size.X * 0.5f + Row * (Size.X + RowGap)),
                    Spec.LevelTopZ[Level] + StackIndex * (Size.Z + .6f));
                // Hand-stocked look: each unit sits a little off the grid and turned a few degrees.
                // Stacked units keep a smaller turn so the pile remains physically believable.
                FRandomStream Hand(SeedBase + Seq * 131 + 17);
                const float Nudge = FMath::Min(Gap * (StackIndex == 0 ? .45f : .20f), StackIndex == 0 ? .9f : .4f);
                const FVector Jitter(Hand.FRandRange(-Nudge, Nudge), -FaceSign * Hand.FRandRange(0.f, Row == 0 ? 2.5f : 1.f), 0.f);
                const float MaxTurn = StackIndex == 0 ? 4.f : 1.5f;
                const FQuat Turn(FVector::UpVector, FMath::DegreesToRadians(Hand.FRandRange(-MaxTurn, MaxTurn)));
                const FTransform Local(Turn * FinalRotation, SlotLocal + Jitter - BoundsOffset + CorrectedOffset, Look.ItemScale);
                OutTransforms.Add(Local * FixtureXf);
                if (OutRows) OutRows->Add(Row);
                ++Seq;
            }
    if (OutApproach) *OutApproach = BlockApproachSpot(Placement, CenterX);
}

void AMarketGameMode::BuildShelfItems(int32 Index)
{
    // One visible unit per shelf slot (facings x depth x stack = capacity) over every block of the product.
    const FMarketProduct& Product = Products[Index];
    const FProductLook Look = LoadProductLook(Index);
    UStaticMesh* Mesh = Look.Mesh;
    const TArray<UMaterialInterface*>& SlotMaterials = Look.Materials;
    TArray<FTransform>& Slots = ShelfSlots.AddDefaulted_GetRef();
    TArray<FVector>& Approach = ShelfApproach.AddDefaulted_GetRef();
    // All units of a product share one instanced mesh; slots are ordered front row first, center-out,
    // so a partly sold shelf still looks faced up.
    AActor* Holder = GetWorld()->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator);
    ShelfContentActors.Add(Holder);
    auto* Root = NewObject<USceneComponent>(Holder);
    Holder->SetRootComponent(Root);
    Root->RegisterComponent();
    // Instancing needs every material to carry the "Used with Instanced Static Meshes" flag, otherwise
    // Unreal draws the default grey material. GORSEL_HAZIRLA.cmd / the studio set the flag; until then
    // the product falls back to one ordinary mesh component per unit.
    bool bCanInstance = true;
    for (int32 MaterialSlot = 0; MaterialSlot < FMath::Max(1, SlotMaterials.Num()); ++MaterialSlot)
    {
        UMaterialInterface* Used = SlotMaterials.IsValidIndex(MaterialSlot) && SlotMaterials[MaterialSlot] ? SlotMaterials[MaterialSlot] : Mesh->GetMaterial(MaterialSlot);
        const UMaterial* Base = Used ? Used->GetMaterial() : nullptr;
        if (Base && !Base->GetUsageByFlag(MATUSAGE_InstancedStaticMeshes)) bCanInstance = false;
    }
    UInstancedStaticMeshComponent* Instances = nullptr;
    if (bCanInstance)
    {
        Instances = NewObject<UInstancedStaticMeshComponent>(Holder);
        Instances->SetupAttachment(Root);
        Instances->SetMobility(EComponentMobility::Movable);
        Instances->SetStaticMesh(Mesh);
        Instances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Instances->SetCastShadow(true);
        for (int32 MaterialSlot = 0; MaterialSlot < SlotMaterials.Num(); ++MaterialSlot)
            if (SlotMaterials[MaterialSlot]) Instances->SetMaterial(MaterialSlot, SlotMaterials[MaterialSlot]);
        Instances->RegisterComponent();
    }
    else UE_LOG(LogTemp, Warning, TEXT("MarketSim: %s materials lack the instanced-mesh usage flag; using single meshes (run GORSEL_HAZIRLA.cmd)."), *Product.Id);
    ShelfInstances.Add(Instances);
    TArray<TWeakObjectPtr<UStaticMeshComponent>>& Singles = ShelfSingles.AddDefaulted_GetRef();

    struct FSlot { int32 Row; int32 Seq; FTransform Transform; };
    TArray<FSlot> Ordered;
    for (const FPlanogramPlacement& Placement : Planogram.Placements)
    {
        if (Placement.ProductId != Product.Id || !Planogram.FindFixture(Placement.FixtureId)) continue;
        TArray<FTransform> Transforms;
        TArray<int32> Rows;
        FVector Spot = FVector::ZeroVector;
        BlockSlotTransforms(Look, Product, Placement, MarketPlanogram::PlacementCenterX(Planogram, Products, Placement),
            Index * 7919, Ordered.Num(), false, Transforms, &Rows, &Spot);
        for (int32 K = 0; K < Transforms.Num(); ++K) Ordered.Add({ Rows[K], Ordered.Num(), Transforms[K] });
        Approach.Add(Spot);
    }
    // Front rows of every block fill before any back row, so stock looks spread over the whole display.
    Ordered.Sort([](const FSlot& A, const FSlot& B) { return A.Row != B.Row ? A.Row < B.Row : A.Seq < B.Seq; });
    for (const FSlot& Slot : Ordered) Slots.Add(Slot.Transform);
    if (Instances) return;
    for (const FTransform& Slot : Slots)
    {
        auto* Item = NewObject<UStaticMeshComponent>(Holder);
        Item->SetupAttachment(Root);
        Item->SetMobility(EComponentMobility::Movable);
        Item->SetStaticMesh(Mesh);
        Item->SetWorldTransform(Slot);
        Item->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Item->SetCastShadow(false);
        for (int32 MaterialSlot = 0; MaterialSlot < SlotMaterials.Num(); ++MaterialSlot)
            if (SlotMaterials[MaterialSlot]) Item->SetMaterial(MaterialSlot, SlotMaterials[MaterialSlot]);
        Item->RegisterComponent();
        Item->SetWorldTransform(Slot);
        Singles.Add(Item);
    }
}

void AMarketGameMode::RefreshShelfItems()
{
    for (int32 I = 0; I < ShelfInstances.Num() && State.Stock.IsValidIndex(I) && ShelfSlots.IsValidIndex(I); ++I)
    {
        UInstancedStaticMeshComponent* Instances = ShelfInstances[I];
        const TArray<FTransform>& Slots = ShelfSlots[I];
        const int32 Shelf = State.Stock[I].Shelf;
        const int32 Capacity = FMath::Max(1, State.Stock[I].Capacity);
        // While arranging (R) every block is drawn full, so reserved space is visible even without stock.
        const int32 Visible = bArrange ? Slots.Num()
            : (Slots.Num() >= Capacity ? FMath::Min(Shelf, Slots.Num()) : FMath::DivideAndRoundUp(Shelf * Slots.Num(), Capacity));
        if (!Instances)
        {
            if (ShelfSingles.IsValidIndex(I))
                for (int32 K = 0; K < ShelfSingles[I].Num(); ++K)
                    if (UStaticMeshComponent* Item = ShelfSingles[I][K].Get()) Item->SetVisibility(K < Visible);
            continue;
        }
        if (Instances->GetInstanceCount() == Visible) continue;
        Instances->ClearInstances();
        for (int32 K = 0; K < Visible; ++K) Instances->AddInstance(Slots[K], true);
    }
}

FString AMarketGameMode::ProductName(int32 Index) const { return State.bRealBrands ? Products[Index].RealName : Products[Index].FictionalName; }
void AMarketGameMode::Notify(const FString& Text) { Message = Text; MessageTime = 9; }
float AMarketGameMode::RivalDiscount() const { return MarketDirector::RivalPriceFactor(State, FString()); }
float AMarketGameMode::RivalPriceFactor(int32 ProductIndex) const
{
    // E2: the home province's chains, weighted by their stores, war prices included.
    return Products.IsValidIndex(ProductIndex) ? MarketDirector::RivalPriceFactor(State, Products[ProductIndex].Category) : 1.f;
}
FString AMarketGameMode::RivalNewsText() const
{
    TArray<FString> Lines;
    Lines.Add(TEXT("\u2022 ") + MarketDirector::TomorrowText(State)); // calendar: weather, bayrams, paydays, what sells
    // E2: a price war of a chain in the home province against us.
    const int32 War = MarketChains::WarIn(State, State.CountryId, MarketStart::HomeProvince(State), State.Day);
    if (War != INDEX_NONE) Lines.Add(FString::Printf(TEXT("\u2022 %s ilde fiyat sava\u015f\u0131nda (%d. g\u00fcne kadar)."), *State.Rivals.Chains[War].Name, State.Rivals.Chains[War].WarUntil));
    else Lines.Add(TEXT("\u2022 Rakiplerden yeni bir haber yok."));
    return FString::Join(Lines, TEXT("\n"));
}
FString AMarketGameMode::WeekReportText() const
{
    if (State.LastWeekNumber <= 0) return FString();
    FString Text = FString::Printf(TEXT("Ciro %s  \u00b7  net %s  \u00b7  %d sat\u0131\u015f, %d kay\u0131p m\u00fc\u015fteri"),
        *Money(State.LastWeekRevenue), *Money(State.LastWeekProfit), State.LastWeekServed, State.LastWeekLost);
    if (State.LastWeekDebtPaid > 0 || MarketCampaign::DebtOpen(State))
        Text += FString::Printf(TEXT("\nBu hafta \u00f6denen bor\u00e7 %s  \u00b7  kalan devral\u0131nan bor\u00e7 %s"), *Money(State.LastWeekDebtPaid), *Money(State.InheritedDebt));
    return Text;
}
FString AMarketGameMode::LoyaltySummary() const { return MarketBasket::Summary(State); }
FString AMarketGameMode::OrderAdvice(int32 Index) const
{
    if (!State.Stock.IsValidIndex(Index) || !Products.IsValidIndex(Index)) return FString();
    const FMarketStock& Item = State.Stock[Index];
    TArray<float> Scale; // tomorrow's calendar demand for this product only
    Scale.Init(1.f, Products.Num());
    Scale[Index] = MarketDirector::OrderScale(State, Products[Index]);
    if (Item.Capacity <= 0)
        return FString::Printf(TEXT("Rafta yeri yok  \u00b7  depo %d  \u00b7  \u00f6nce R ile reyona koy (d\u00fcn %d m\u00fc\u015fteri sordu)"), Item.Warehouse, Item.Yesterday.NotCarried);
    return FString::Printf(TEXT("Raf %d/%d  \u00b7  depo %d  \u00b7  kabul %d  \u00b7  yolda %d  \u00b7  d\u00fcn %d sat\u0131\u015f, %d bo\u015f raf  \u00b7  \u00f6neri %d koli"),
        Item.Shelf, Item.Capacity, Item.Warehouse, Item.Dock, Item.Incoming, Item.Yesterday.Sold, Item.Yesterday.Empty,
        MarketOrderAdvice::SuggestCases(State, Products, Index, &Scale));
}
FString AMarketGameMode::PriceSummary(int32 Index) const
{
    if (!Products.IsValidIndex(Index) || !State.Stock.IsValidIndex(Index)) return FString();
    const int64 Ours = State.Stock[Index].Price;
    const int64 Theirs = MarketDemand::RivalPrice(Products[Index], RivalPriceFactor(Index));
    // C3 (B #24): the same curve the shoppers use (the product's elasticity and how well its price is known).
    const double Chance = MarketDemand::BuyChanceFor(MarketDemand::PriceRatio(Ours, Theirs), State.MarketShare, 0.0, MarketDemand::ElasticityOf(Products[Index]), Products[Index].Kvi);
    return FString::Printf(TEXT("Fiyat %s  \u00b7  rakip %s%s  \u00b7  alan m\u00fc\u015fteri ~%%%d"),
        *Money(Ours), *Money(Theirs), RivalPriceFactor(Index) < 1.f ? TEXT(" (indirimde)") : RivalPriceFactor(Index) > 1.f ? TEXT(" (pahal\u0131/yok)") : TEXT(""), FMath::RoundToInt32(Chance * 100.0));
}
FString AMarketGameMode::DayProblemsText() const
{
    const TArray<MarketDemand::FProblem> Problems = MarketDemand::TopProblems(State, 3);
    if (Problems.Num() == 0) return TEXT("Kay\u0131p m\u00fc\u015fteri yok. Herkes arad\u0131\u011f\u0131n\u0131 buldu.");
    TArray<FString> Lines;
    for (const MarketDemand::FProblem& Problem : Problems)
    {
        const FString Shown = Products.IsValidIndex(Problem.Product) ? ProductName(Problem.Product) : FString();
        switch (Problem.Kind)
        {
        case MarketDemand::EProblem::Waiting:
            Lines.Add(FString::Printf(TEXT("\u2022 %d m\u00fc\u015fteri i\u00e7eride beklemekten vazge\u00e7ti. Kasiyer al veya kasada E ile h\u0131zl\u0131 \u00f6de."), Problem.Count));
            break;
        case MarketDemand::EProblem::NotCarried:
            Lines.Add(FString::Printf(TEXT("\u2022 %d m\u00fc\u015fteri %s sordu ama rafta yok. R ile reyona koy."), Problem.Count, *Shown));
            break;
        case MarketDemand::EProblem::Empty:
            Lines.Add(FString::Printf(TEXT("\u2022 %s rafta bitti: %d m\u00fc\u015fteri eli bo\u015f d\u00f6nd\u00fc. Depodan doldur veya sipari\u015f ver."), *Shown, Problem.Count));
            break;
        case MarketDemand::EProblem::Expensive:
            Lines.Add(FString::Printf(TEXT("\u2022 %d m\u00fc\u015fteri %s fiyat\u0131n\u0131 pahal\u0131 buldu. Men\u00fcde \u00dcr\u00fcnler ve fiyat sayfas\u0131nda rakip fiyat\u0131na bak."), Problem.Count, *Shown));
            break;
        }
    }
    return FString::Join(Lines, TEXT("\n"));
}
int32 AMarketGameMode::NearbyShelf() const
{
    const auto* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Pawn) return INDEX_NONE;
    // Nearest product block, measured from the spot in front of it.
    int32 Result = INDEX_NONE; float Best = 120;
    for (int32 I = 0; I < Products.Num(); ++I)
    {
        if (!ShelfApproach.IsValidIndex(I) || ShelfApproach[I].Num() == 0) continue; // not on a shelf
        for (const FVector& Spot : ShelfApproach[I])
        {
            const float Distance = FVector::Dist2D(Pawn->GetActorLocation(), Spot);
            if (Distance < Best) { Best = Distance; Result = I; }
        }
    }
    return Result;
}
bool AMarketGameMode::NearOffice() const
{
    const auto* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    return Pawn && FVector::Dist2D(Pawn->GetActorLocation(), FVector(-430, -30, 0)) < 210;
}
bool AMarketGameMode::NearCounter() const
{
    const auto* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    return Pawn && FVector::Dist2D(Pawn->GetActorLocation(), FVector(405, -45, 0)) < 230;
}
int32 AMarketGameMode::QueueSize() const
{
    int32 Count = 0;
    for (const auto& C : Customers) if (C.Stage == 2) ++Count;
    return Count;
}

int32 FMarketQueueRules::Rank(const TArray<FMarketCustomer>& Customers, int32 CustomerIndex)
{
    if (!Customers.IsValidIndex(CustomerIndex)) return INDEX_NONE;
    const FMarketCustomer& Customer = Customers[CustomerIndex];
    if (Customer.Stage != 2 || Customer.QueueTicket == INDEX_NONE) return INDEX_NONE;
    int32 Result = 0;
    for (const FMarketCustomer& Other : Customers)
    {
        if (Other.Stage == 2 && Other.QueueTicket != INDEX_NONE && Other.QueueTicket < Customer.QueueTicket) ++Result;
    }
    return Result;
}

int32 FMarketQueueRules::FindFront(const TArray<FMarketCustomer>& Customers)
{
    int32 Result = INDEX_NONE;
    int32 FirstTicket = MAX_int32;
    for (int32 I = 0; I < Customers.Num(); ++I)
    {
        const FMarketCustomer& Customer = Customers[I];
        if (Customer.Stage == 2 && Customer.QueueTicket != INDEX_NONE && Customer.QueueTicket < FirstTicket)
        {
            FirstTicket = Customer.QueueTicket;
            Result = I;
        }
    }
    return Result;
}
FString AMarketGameMode::ContextHint() const
{
    FString TargetFace;
    if (CategoryTarget(TargetFace) != INDEX_NONE) return TEXT("Kategoriyi de\u011fi\u015ftir [T]");
    // "K: text" = key K does the action (the HUD draws K as a key cap). Proper Turkish: Slate font.
    if (bArrange) return ArrangeHint();
    if (CarriedDeliveryProduct != INDEX_NONE)
    {
        const APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
        return Pawn && FVector::Dist2D(Pawn->GetActorLocation(), DepotSpot()) < 155.f
            ? TEXT("E: Koliyi depoya birak") : TEXT("Koliyi arka depodaki kabul noktasina gotur");
    }
    if (const int32 Delivery = NearbyDelivery(); Delivery != INDEX_NONE)
        return FString::Printf(TEXT("E: %s kolisini al  \u00b7  depoya gotur"), *ProductName(Delivery));
    if (NearOffice()) return TEXT("E: Y\u00f6netim men\u00fcs\u00fcn\u00fc a\u00e7 (sipari\u015f, fiyat, personel, finans)");
    if (NearCounter()) return FString::Printf(TEXT("E: S\u0131radaki m\u00fc\u015fterinin \u00f6demesini al   \u00b7   bekleyen %d"), QueueSize());
    const int32 I = NearbyShelf();
    if (I != INDEX_NONE)
    {
        const FString Arrange = bOpen ? FString() : FString(TEXT("   \u00b7   R reyonu diz"));
        return (bTestMode
            ? FString::Printf(TEXT("E: %s raf\u0131n\u0131 doldur (test: bedava)   \u00b7   raf %d / %d"), *ProductName(I), State.Stock[I].Shelf, State.Stock[I].Capacity)
            : FString::Printf(TEXT("E: %s raf\u0131n\u0131 depodan doldur   \u00b7   raf %d / %d   \u00b7   depo %d"), *ProductName(I), State.Stock[I].Shelf, State.Stock[I].Capacity, State.Stock[I].Warehouse))
            + Arrange;
    }
    if (!bOpen && NearbyFixture() != INDEX_NONE) return TEXT("R: Bu reyonu diz (\u00fcr\u00fcn koy, s\u0131rala, kald\u0131r)");
    return TEXT("M: Y\u00f6netim men\u00fcs\u00fc   \u00b7   raf, kasa veya arka kap\u0131ya yakla\u015f");
}
void AMarketGameMode::RefreshLabels()
{
    RefreshCategorySigns();
    for (int32 L = 0; L < ShelfLabels.Num() && ShelfLabelProduct.IsValidIndex(L); ++L)
        if (ShelfLabels[L] && State.Stock.IsValidIndex(ShelfLabelProduct[L]))
        {
            // The tag shows the single-unit price the shopper pays today and the promotion (3D text: ASCII).
            const int32 P = ShelfLabelProduct[L];
            const FString Badge = MarketPromotions::Badge(State, Products, P);
            ShelfLabels[L]->SetText(FText::FromString(Money(MarketPromotions::UnitPrice(State, Products, P, 1)) + (Badge.IsEmpty() ? FString() : TEXT("\n") + MarketCatalog::UpperTurkish(Badge))));
        }
    RefreshShelfItems();
}

void AMarketGameMode::Command(FName Action)
{
    if (IsBranchVisit()) { if (Action == TEXT("ExitVisit")) EndBranchVisit(); return; }
    if (CategoryCommand(Action)) return;
    if (StoreTourCommand(Action)) return;
    if (Action == "Menu") { OpenMenu(MenuPage); return; } // G-059: clickable management menu (MarketMenu.cpp)
    if (Action == "TimePause") { SetTimePaused(!bTimePaused); return; } // G-075 game speed (MarketMenu.cpp)
    if (Action == "Speed1" || Action == "Speed2" || Action == "Speed3") { SetGameSpeed(Action == "Speed1" ? 1 : Action == "Speed2" ? 2 : 3); return; }
    if (ArrangeCommand(Action)) return; // R mode: aim + click/E, wheel/TAB/Q, +/-, Y, U, F, C, DEL, arrows (MarketArrange.cpp)
    if (Action == "ToggleShop")
    {
        if (bOpen) CloseShop();
        else if (State.Story.bCampaignOver) Notify(TEXT("Bu kampanya bitti (Satt\u0131n). Yeni oyun i\u00e7in F6'ya iki kez bas ya da men\u00fcden ba\u015fka bir kay\u0131t yuvas\u0131 se\u00e7."));
        else if (MarketCalendar::ClosedByLaw(State.Day))
        {
            // G-084: the country's law keeps shops shut today; the day passes closed (wages and rent still run).
            CloseShop();
            Notify(TEXT("Bug\u00fcn yasal tatil: d\u00fckk\u00e2nlar kapal\u0131, g\u00fcn m\u00fc\u015fterisiz ge\u00e7ti. Yar\u0131n a\u00e7abilirsin."));
        }
        else
        {
            bOpen = true; DayTime = 0; SpawnTimer = 1; AutoCheckoutTimer = 0; NextQueueTicket = 0;
            Random.Initialize(CampaignSeedBase + State.Day * 73);
            Notify(TEXT("Market acildi. Musterileri kasada bekletme. Gun 4 dakika surer; O erken kapatir."));
        }
    }
    else if (Action == "Interact")
    {
        if (CarriedDeliveryProduct != INDEX_NONE) { if (!FinishPlayerDelivery()) Notify(TEXT("Koliyi arka depodaki kabul noktasina gotur.")); }
        else if (const int32 Delivery = NearbyDelivery(); Delivery != INDEX_NONE)
        {
            if (StartPlayerDelivery(Delivery)) Notify(FString::Printf(TEXT("%s kolisini aldin. Arka depodaki kabul noktasina gotur ve E'ye bas."), *ProductName(Delivery)));
        }
        else if (NearCounter()) Checkout();
        else if (NearOffice()) OpenMenu(0); // the desk is where the management menu lives (Ozet)
        else if (const int32 I = NearbyShelf(); I != INDEX_NONE)
        {
            Selected = I;
            if (bTestMode)
            {
                const int32 Count = State.FillShelfFree(I);
                Notify(Count > 0 ? FString::Printf(TEXT("TEST: %d adet bedava rafa kondu (%d / %d)."), Count, State.Stock[I].Shelf, State.Stock[I].Capacity)
                                 : FString::Printf(TEXT("Raf zaten dolu (%d / %d)."), State.Stock[I].Shelf, State.Stock[I].Capacity));
            }
            else
            {
                const int32 Count = State.Restock(I);
                Notify(Count > 0 ? FString::Printf(TEXT("%d adet rafa yerlestirildi."), Count) : FString(TEXT("Raf dolu ya da depo bo\u015f. M ile men\u00fcy\u00fc a\u00e7, Sipari\u015f sayfas\u0131ndan sipari\u015f ver.")));
            }
            RefreshLabels();
        }
    }
    else if (Action == "Brands")
    {
        State.bRealBrands = !State.bRealBrands;
        if (Planogram.Fixtures.ContainsByPredicate([](const auto& Fixture) { return MarketTelevisionDisplay::IsDisplay(Fixture.EquipmentId); })) RebuildShelfContents();
        else RefreshLabels();
    }
    else if (Action == "ToggleDetails") bShowDetails = !bShowDetails;
    else if (Action == "Fullscreen")
    {
        // Borderless full screen at the desktop resolution keeps the HUD and the 3D view the same size.
        if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
        {
            const bool bFull = Settings->GetFullscreenMode() != EWindowMode::Windowed;
            Settings->SetFullscreenMode(bFull ? EWindowMode::Windowed : EWindowMode::WindowedFullscreen);
            Settings->SetScreenResolution(bFull ? FIntPoint(1600, 900) : Settings->GetDesktopResolution());
            Settings->ApplySettings(false);
        }
    }
    else if (Action == "Mood")
    {
        Mood = (Mood + 1) % MarketVisuals::MoodCount;
        MarketVisuals::ApplyMood(Lighting, Mood);
        Notify(FString::Printf(TEXT("I\u015f\u0131k: %s  (F4 ile de\u011fi\u015ftir)"), MarketVisuals::MoodName(Mood)));
    }
    else if (Action == "TestMode")
    {
        if (FParse::Param(FCommandLine::Get(), TEXT("SimSmoke"))) return;
        bTestMode = !bTestMode;
        if (bTestMode) State.bUsedTestMode = true;
        const int32 Added = bTestMode ? FillAllShelves() : 0;
        RefreshLabels();
        Notify(bTestMode ? FString::Printf(TEXT("TEST MODU acik: tum raflar dolduruldu (+%d). E bedava doldurur, B bedava ve aninda depoya getirir."), Added)
                         : FString(TEXT("TEST MODU kapali: normal ekonomi (depo, siparis, nakit).")));
    }
    else if (Action == "FillAll")
    {
        if (!bTestMode) { Notify(TEXT("Tum raflari doldurmak icin once F2 ile TEST MODU'nu ac.")); return; }
        const int32 Added = FillAllShelves();
        RefreshLabels();
        Notify(FString::Printf(TEXT("TEST: tum raflar dolduruldu (+%d adet)."), Added));
    }
    else if (Action == "Save")
    {
        if (bOpen) Notify(TEXT("Kayit icin once O ile gunu kapat. Gun sonu otomatik kaydedilir."));
        else Notify(SaveCampaign() ? TEXT("Kampanya kaydedildi.") : TEXT("Kayit yazilamadi."));
    }
    else if (Action == "Load")
    {
        if (bOpen) Notify(TEXT("Yuklemek icin once gunu kapat."));
        else LoadCampaign();
    }
    else if (Action == "NewCampaign")
    {
        if (bOpen) { Notify(TEXT("Yeni kampanya icin once gunu kapat.")); return; }
        if (GetWorld()->GetTimeSeconds() > ResetConfirmUntil)
        {
            ResetConfirmUntil = GetWorld()->GetTimeSeconds() + 5;
            Notify(TEXT("Sifirlamak icin 5 saniye icinde F6'ya tekrar bas. Eski kayit sonraki kayitta degisir."));
        }
        else
        {
            ResetConfirmUntil = -1;
            AskNewGame(); // G-086: country and province first
        }
    }
    else
    {
        if (!NearOffice() && !bMenuAction) { Notify(TEXT("Bu karar i\u00e7in M ile y\u00f6netim men\u00fcs\u00fcn\u00fc a\u00e7.")); return; }
        if (Action == "NextProduct") Selected = (Selected + 1) % Products.Num();
        else if (Action == "PrevProduct") Selected = (Selected + Products.Num() - 1) % Products.Num();
        else if (Action == "Order" && bTestMode)
        {
            const int32 Units = State.ReceiveFree(Selected, FMath::Clamp(Products[Selected].CaseUnits, 1, 48));
            Notify(Units > 0 ? FString::Printf(TEXT("TEST: %d adet bedava ve hemen depoya geldi."), Units) : FString(TEXT("Depo dolu (120 adet).")));
        }
        else if (Action == "Order")
        {
            if (!OrderDraftCases.IsValidIndex(Selected)) OrderDraftCases.Init(0, Products.Num());
            const int32 Units = FMath::Clamp(Products[Selected].CaseUnits, 1, 48);
            const FMarketStock& Item = State.Stock[Selected];
            if (OrderDraftCases[Selected] >= 9 || Item.Warehouse + Item.Dock + Item.Incoming + (OrderDraftCases[Selected] + 1) * Units > FMarketState::StorageCapacity)
                Notify(TEXT("Bu urun icin depo + mal kabul + yoldaki siparis limiti 120 adet."));
            else { ++OrderDraftCases[Selected]; Notify(ProductName(Selected) + TEXT(" listeye eklendi. N ile tum listeyi onayla.\n") + OrderDraftSummary()); }
        }
        else if (Action == "RemoveOrder")
        {
            if (OrderDraftCases.IsValidIndex(Selected) && OrderDraftCases[Selected] > 0) --OrderDraftCases[Selected];
            Notify(OrderDraftSummary());
        }
        else if (Action == "SuggestOrder")
        {
            // L: raise every line of the draft to the suggestion (yesterday's demand + empty shelves); never lowers a line.
            const TArray<float> Scales = MarketDirector::OrderScales(State, Products); // tomorrow's calendar demand
            const int32 Changed = MarketOrderAdvice::FillSuggested(State, Products, OrderDraftCases, &Scales);
            Notify(Changed > 0 ? FString::Printf(TEXT("\u00d6nerilen sipari\u015f listeye yaz\u0131ld\u0131 (%d \u00fcr\u00fcn). N ile onayla.\n"), Changed) + OrderDraftSummary()
                               : FString(TEXT("\u00d6neriye g\u00f6re ek koli gerekmiyor: raf, depo, arka kap\u0131 ve yoldaki mal yeterli.")));
        }
        else if (Action == "ConfirmOrder" && OrderDraftCaseCount() > 0 && OrderDraftBill() < MarketOrderAdvice::MinimumOrderOn(State.Day))
        {
            Notify(FString::Printf(TEXT("Toptanc\u0131 en az %s sipari\u015fle gelir; liste \u015fu an %s. Koli ekle (B) veya \u00f6neriyi yaz (L)."),
                *Money(MarketOrderAdvice::MinimumOrderOn(State.Day)), *Money(OrderDraftBill())));
        }
        else if (Action == "ConfirmOrder")
        {
            int64 Bill = 0;
            int32 Units = 0;
            if (State.SubmitOrder(OrderDraftCases, Products, &Bill, &Units, MarketDirector::OrderAllowance(State))) // G-077: terms count
            {
                OrderDraftCases.Init(0, Products.Num());
                const FString Terms = MarketDirector::OnOrder(State, Bill); // wholesaler volume and payment terms
                Notify(FString::Printf(TEXT("Siparis onaylandi: %d adet, %s. Yarin sabah arka kapida; depoya tasinmasi gerekir."), Units, *Money(Bill))
                    + (Terms.IsEmpty() ? FString() : TEXT("\n") + Terms));
            }
            else Notify(TEXT("Siparis onaylanamadi: liste bos, nakit yetersiz veya urun deposu 120 adet sinirini asiyor."));
        }
        else if (Action == "PriceUp" || Action == "PriceDown")
        {
            MarketSimulation::AdjustPrice(State, Products, Selected, Action == "PriceUp");
            RefreshLabels();
            const FString Warn = MarketDemand::PriceWarning(State, Products, Selected); // C3 (B #27): below cost
            Notify(FString::Printf(TEXT("%s: %s"), *ProductName(Selected), *PriceSummary(Selected)) + (Warn.IsEmpty() ? FString() : TEXT("\n") + Warn));
        }
        else if (Action == "Hire" || Action == "HireStocker" || Action == "FireStocker")
        {
            // H / J: the best candidate of the pool (MarketStaff). The menu's Personel page picks a person.
            FString Text;
            const bool bDone = Action == "FireStocker" ? MarketStaff::FireLast(State, MarketStaff::ERole::Stocker, Text)
                : MarketStaff::HireBest(State, Action == "Hire" ? MarketStaff::ERole::Cashier : MarketStaff::ERole::Stocker, Text);
            if (bDone) SyncWorkers();
            Notify(Text);
        }
        else if (Action == "PayDebt" || Action == "PayDebtAll")
        {
            // M69: the inherited debt has no deadline and locks nothing; one installment or all of it, when the player wants.
            if (!MarketCampaign::DebtOpen(State)) Notify(TEXT("Devral\u0131nan bor\u00e7 kapand\u0131; defterde \u00f6denecek bir \u015fey kalmad\u0131."));
            else if (const int64 Paid = MarketCampaign::PayDebt(State, Action == "PayDebtAll" ? State.InheritedDebt : -1); Paid > 0)
                Notify(MarketCampaign::DebtOpen(State)
                    ? FString::Printf(TEXT("Toptanc\u0131ya %s \u00f6dendi. Kalan bor\u00e7 %s."), *Money(Paid), *Money(State.InheritedDebt))
                    : FString(TEXT("Devral\u0131nan bor\u00e7 kapand\u0131. Toptanc\u0131 defterdeki sat\u0131r\u0131n \u00fcst\u00fcn\u00fc \u00e7izdi.")));
            else Notify(TEXT("Bor\u00e7 \u00f6demek i\u00e7in kasada nakit yok."));
        }
        else if (Action == "Expand")
        {
            // G-086: G opens a neighbourhood market in the home province; the map has every province and type.
            FString Text;
            MarketDirector::Command(State, Products, TEXT("OpenBranch"), MarketBranches::EncodeSite(State.CountryId, MarketStart::HomeProvince(State), TEXT("mahalle")), Text);
            Notify(Text);
        }
    }
}

void AMarketGameMode::SpawnCustomer()
{
    if (Customers.Num() >= 9) { MarketDemand::RecordWaitingLoss(State); return; } // too crowded: turns away at the door
    FMarketCustomer C;
    // Who comes depends on the hour: retirees in the morning, children after school, workers in the evening.
    const float Progress = FMath::Clamp(DayTime / 240.f, 0.f, 1.f);
    for (int32 Try = 0; Try < 3; ++Try)
    {
        C.CustomerId = MarketBasket::ChooseCustomer(State, Random.FRand(), Random.FRand(), C.bReturning);
        C.Segment = static_cast<uint8>(MarketCustomers::SegmentOf(C.CustomerId, State.RivalSeed));
        if (MarketCustomers::ComesNow(static_cast<MarketCustomers::ESegment>(C.Segment), Progress, State.Day, Random.FRand())) break;
    }
    const MarketCustomers::ESegment Segment = static_cast<MarketCustomers::ESegment>(C.Segment);
    C.BudgetLeft = FMath::RoundToInt64(MarketCustomers::VisitBudget(Segment, State.Day) * MarketDirector::BudgetFactor(State, C.Segment)); // cards (G-069)
    C.ShoppingList = MarketCustomers::BuildList(State, Products, Segment, Random);
    if (C.ShoppingList.Num() == 0) return;
    // G-070: the person's own walk, and the list put in a walking order (families keep theirs).
    C.Gait = MarketMotion::MakeGait(Segment, C.CustomerId, State.RivalSeed);
    {
        TArray<FVector> Stops;
        for (int32 Wanted : C.ShoppingList) Stops.Add(CustomerBrowseLocation(Wanted));
        TArray<int32> Ordered;
        for (int32 Index : MarketMotion::OrderVisits(Stops, FVector(0, -230, 0), FVector(405, 60, 0), C.Gait.Style)) Ordered.Add(C.ShoppingList[Index]);
        C.ShoppingList = Ordered;
    }
    C.Product = C.ShoppingList[0];
    if (AActor* Human = MarketPeople::Spawn(GetWorld(), People, FVector(0, -230, 0), Random.RandRange(0, 1 << 20), C.Shopper))
    {
        C.Actor = Human; C.bHuman = true;
        Customers.Add(C);
        return;
    }
    C.Actor = SimplePerson(FVector(0, -230, 0), FLinearColor::MakeFromHSV8(Random.RandRange(0, 255), 130, 210));
    Customers.Add(C);
}

int32 AMarketGameMode::ReservedUnits(int32 Product) const
{
    int32 Reserved = 0;
    for (const FMarketCustomer& Customer : Customers)
        for (const FMarketBasketItem& Item : Customer.Basket)
            if (Item.Product == Product) Reserved += Item.Quantity;
    return Reserved;
}

TArray<int32> AMarketGameMode::AvailableShelfUnits() const
{
    TArray<int32> Available;
    Available.SetNum(State.Stock.Num());
    for (int32 I = 0; I < State.Stock.Num(); ++I) Available[I] = FMath::Max(0, State.Stock[I].Shelf - ReservedUnits(I));
    return Available;
}

FVector AMarketGameMode::CustomerBrowseLocation(int32 Product) const
{
    if (State.Stock.IsValidIndex(Product) && State.Stock[Product].Capacity > 0) return ProductFixtureLocation(Product);
    if (Products.IsValidIndex(Product))
    {
        for (int32 I = 0; I < Products.Num(); ++I)
            if (State.Stock.IsValidIndex(I) && State.Stock[I].Capacity > 0 &&
                Products[I].Category.Equals(Products[Product].Category, ESearchCase::IgnoreCase)) return ProductFixtureLocation(I);
    }
    // The product/category is not ranged: the shopper still enters, looks along the first aisle, then leaves.
    return FVector(Product % 2 == 0 ? -150.f : 150.f, 310.f + (Product % 3) * 55.f, 0.f);
}

void AMarketGameMode::AdvanceCustomerList(FMarketCustomer& Customer)
{
    ++Customer.ShoppingIndex;
    Customer.bTryingSubstitute = false;
    Customer.Route.Reset();
    Customer.RouteStage = -1;
    Customer.RouteProduct = INDEX_NONE;
    if (Customer.ShoppingIndex < Customer.ShoppingList.Num()) Customer.Product = Customer.ShoppingList[Customer.ShoppingIndex];
    else Customer.Stage = Customer.Basket.Num() > 0 ? 1 : 3;
}

void AMarketGameMode::ResolveCustomerItem(FMarketCustomer& Customer)
{
    if (!Customer.ShoppingList.IsValidIndex(Customer.ShoppingIndex)) { AdvanceCustomerList(Customer); return; }
    const int32 Wanted = Customer.ShoppingList[Customer.ShoppingIndex];
    const TArray<int32> Available = AvailableShelfUnits();
    const MarketCustomers::ESegment Segment = static_cast<MarketCustomers::ESegment>(Customer.Segment);
    const int32 Quantity = MarketPromotions::AdjustQuantity(State, Customer.Product, MarketCustomers::Quantity(Segment, Random)); // 3 al 2 \u00f6de
    const float PersonalShare = MarketBasket::EffectiveMarketShare(State, Customer.CustomerId);
    MarketDemand::FVisit Visit = MarketDemand::Decide(State, Products, Customer.Product,
        Available.IsValidIndex(Customer.Product) ? Available[Customer.Product] : 0,
        RivalPriceFactor(Customer.Product), Quantity, Random.FRand(), PersonalShare,
        MarketCustomers::Profile(Segment).PriceTolerance + (Products.IsValidIndex(Customer.Product) ? MarketDirector::ToleranceBonus(State, Products[Customer.Product]) : 0.0),
        MarketPromotions::UnitPrice(State, Products, Customer.Product, Quantity));
    int64 PaidUnit = 0;
    if (Visit.Result == MarketDemand::EVisit::Buy)
    {
        // The wallet: take fewer when the money runs out; nothing at all counts as "too expensive" for this shopper.
        PaidUnit = MarketPromotions::UnitPrice(State, Products, Visit.Product, Visit.Quantity);
        Visit.Quantity = MarketCustomers::Affordable(Customer.BudgetLeft, PaidUnit, Visit.Quantity);
        // A smaller quantity can lose the "3 al 2 ode" price: settle on a quantity whose own price fits the wallet.
        while (Visit.Quantity > 0)
        {
            PaidUnit = MarketPromotions::UnitPrice(State, Products, Visit.Product, Visit.Quantity);
            if (PaidUnit * Visit.Quantity <= Customer.BudgetLeft) break;
            --Visit.Quantity;
        }
        if (Visit.Quantity <= 0) Visit.Result = MarketDemand::EVisit::Expensive;
        else Customer.BudgetLeft -= PaidUnit * Visit.Quantity;
    }
    if (Visit.Result == MarketDemand::EVisit::Buy)
    {
        FMarketBasketItem Item;
        Item.Product = Visit.Product; Item.Quantity = Visit.Quantity; Item.QuotedPrice = PaidUnit; // promotion price is kept at the till
        Item.bSubstitute = Customer.bTryingSubstitute;
        Customer.Basket.Add(Item);
        ++Customer.Fulfilled;
        AdvanceCustomerList(Customer);
        return;
    }

    if (!Customer.bTryingSubstitute)
    {
        TSet<int32> Excluded;
        for (const FMarketBasketItem& Item : Customer.Basket) Excluded.Add(Item.Product);
        const int32 Substitute = MarketBasket::FindSubstitute(State, Products, Wanted, Available, Excluded, RivalPriceFactor(Wanted));
        if (Substitute != INDEX_NONE)
        {
            Customer.OriginalFailure = static_cast<uint8>(Visit.Result);
            Customer.Product = Substitute;
            Customer.bTryingSubstitute = true;
            Customer.Route.Reset(); Customer.RouteStage = -1; Customer.RouteProduct = INDEX_NONE;
            return;
        }
    }

    MarketDemand::FVisit Failure = Visit;
    Failure.Product = Wanted;
    if (Customer.bTryingSubstitute) Failure.Result = static_cast<MarketDemand::EVisit>(Customer.OriginalFailure);
    MarketDemand::RecordItemFailure(State, Failure);
    AdvanceCustomerList(Customer);
}

AActor* AMarketGameMode::SimplePerson(const FVector& Floor, const FLinearColor& Color)
{
    AActor* Body = Box(Floor + FVector(0, 0, 65), FVector(42, 42, 130), Color, false);
    auto* Head = NewObject<UStaticMeshComponent>(Body);
    Head->SetupAttachment(Body->GetRootComponent());
    Head->RegisterComponent(); Head->SetStaticMesh(Sphere);
    // Absolute scale avoids inheriting the body's non-uniform dimensions.
    Head->SetAbsolute(false, false, true);
    Head->SetWorldScale3D(FVector(.34f)); Head->SetRelativeLocation(FVector(0, 0, 64));
    Head->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    return Body;
}

void AMarketGameMode::Checkout()
{
    const int32 I = FMarketQueueRules::FindFront(Customers);
    if (I == INDEX_NONE) { Notify(TEXT("Kasada odeme bekleyen musteri yok.")); return; }
    const auto& C = Customers[I];
    TArray<FMarketSaleLine> Lines;
    for (const FMarketBasketItem& Item : C.Basket)
    {
        FMarketSaleLine Line;
        Line.Product = Item.Product; Line.Quantity = Item.Quantity; Line.QuotedPrice = Item.QuotedPrice;
        Lines.Add(Line);
    }
    // Cash, card or meal card (G-069). Without a POS some card shoppers leave the basket at the till.
    const uint8 Method = MarketDirector::PaymentMethod(State, C.Segment, Random.FRand());
    if (Method == 3 && MarketDirector::LeavesWithoutCard(State, Random.FRand()))
    {
        ++State.Lost;
        Notify(TEXT("\"Kart gecmiyor mu?\" Musteri sepeti kasada birakip gitti."));
        MarketBasket::RecordVisit(State, C.CustomerId, C.ShoppingList.Num(), 0, true);
        C.Actor->Destroy(); Customers.RemoveAt(I); RefreshLabels();
        return;
    }
    int64 Receipt = 0;
    int32 Units = 0;
    if (State.SellBasket(Lines, Products, &Receipt, &Units))
    {
        const FString Credit = MarketDirector::OnCheckout(State, C.CustomerId, Receipt, Random.FRand(), Method); // payment (G-069); M36: no credit book
        Notify(FString::Printf(TEXT("Sepet satildi: %d farkli urun, %d adet  +%s%s"), Lines.Num(), Units, *Money(Receipt), C.bReturning ? TEXT("  \u00b7  sadik musteri") : TEXT(""))
            + (Credit.IsEmpty() ? FString() : TEXT("\n") + Credit));
    }
    else { ++State.Lost; Notify(TEXT("Sepet karsilanamadi; musteri ayrildi.")); }
    MarketBasket::RecordVisit(State, C.CustomerId, C.ShoppingList.Num(), C.Fulfilled, false);
    C.Actor->Destroy(); Customers.RemoveAt(I); RefreshLabels();
}

void AMarketGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!TickMenuCapture()) return;
    if (!TickBranchVisitReview()) return;
    if (IsBranchVisit()) return;
    if (bStoreTour) { TickStoreTour(); return; }
    if (bNeedStart && !bMenuOpen) OpenMenu(0); // G-086: the new-game screen waits for a province
    // Notices fade in real seconds, whatever the game speed (G-075).
    const float RealDelta = DeltaTime / FMath::Max(0.01f, UGameplayStatics::GetGlobalTimeDilation(this));
    MessageTime = FMath::Max(0.f, MessageTime - RealDelta);
    TickArrange();
    TickPlayerDelivery();
    ReportTime = FMath::Max(0.f, ReportTime - RealDelta);
    if (!TickAutomation()) return; // -SimSmoke / -SimCapture runs (MarketAutomation.cpp)
    TickWorkers(DeltaTime); // shelf staff work whether the shop is open or not (MarketWorkers.cpp)
    if (!bOpen) return;
    DayTime += DeltaTime;
    SpawnTimer -= DeltaTime;
    if (SpawnTimer <= 0) { SpawnCustomer(); SpawnTimer = Random.FRandRange(3.5f, 5.5f) / FMath::Max(0.05f, MarketDirector::TrafficFactor(State, Products)); }
    for (int32 I = 0; I < Customers.Num();)
    {
        auto& C = Customers[I];
        C.Age += DeltaTime;
        const MarketCustomers::FProfile& Profile = MarketCustomers::Profile(static_cast<MarketCustomers::ESegment>(C.Segment));
        if (C.Age > Profile.Patience)
        {
            MarketDemand::RecordWaitingLoss(State);
            MarketBasket::RecordVisit(State, C.CustomerId, C.ShoppingList.Num(), 0, true);
            C.Actor->Destroy(); Customers.RemoveAt(I); continue;
        }
        // G-070: standing still (looking around, chatting with a neighbour).
        if (C.Pause > 0.f)
        {
            C.Pause -= DeltaTime;
            if (C.bHuman)
            {
                FVector Direction = C.Shopper.MoveDirection;
                bool bMoving = false;
                C.Actor->SetActorLocation(MarketPeople::MoveToward(C.Shopper, C.Actor->GetActorLocation(), C.Actor->GetActorLocation(), 0.f, DeltaTime, Direction, bMoving));
                MarketPeople::Update(C.Actor, People, C.Shopper, Direction, false, MetaHumanYawOffset, DeltaTime);
            }
            ++I; continue;
        }
        if (C.Stage == 0 && C.Browse <= 0.f && Random.FRand() < C.Gait.PauseChance * DeltaTime) { C.Pause = C.Gait.PauseSeconds; ++I; continue; }
        if (!C.bChatted && C.bReturning && C.Stage == 0)
            for (int32 J = 0; J < Customers.Num(); ++J)
            {
                FMarketCustomer& Other = Customers[J];
                if (J == I || Other.bChatted || !Other.bReturning || Other.Stage != 0 || !Other.Actor) continue;
                if (FVector::Dist2D(C.Actor->GetActorLocation(), Other.Actor->GetActorLocation()) > 120.f) continue;
                C.bChatted = Other.bChatted = true; // one meeting per visit
                if (MarketMotion::Chats(C.Gait, Other.Gait, Random.FRand())) C.Pause = Other.Pause = MarketMotion::ChatSeconds(Random.FRand());
                break;
            }
        FVector Target;
        if (C.Stage == 0) Target = CustomerBrowseLocation(C.Product) + FVector(0, 0, 65);
        else if (C.Stage == 1) Target = FVector(405, 60 + QueueSize() * 62, 65);
        else if (C.Stage == 2) Target = FVector(405, 60 + FMarketQueueRules::Rank(Customers, I) * 62, 65);
        else Target = FVector(0, -270, 65);
        if (C.RouteStage != C.Stage || (C.Stage == 0 && C.RouteProduct != C.Product))
        {
            C.RouteStage = C.Stage;
            C.RouteProduct = C.Product;
            const FVector Here = C.Actor->GetActorLocation();
            C.Route = AisleRoute(FVector(Here.X, Here.Y, 0.f), FVector(Target.X, Target.Y, 0.f));
            for (FVector& Point : C.Route) Point.Z = Target.Z;
        }
        const bool bOnRoute = C.Route.Num() > 0;
        if (bOnRoute) Target = C.Route[0];
        if (C.bHuman) Target.Z = 0.f; // MetaHuman origin is at the feet
        const FVector From = C.Actor->GetActorLocation();
        // G-070: the person's pace, a little drift, and room for the others (keep right, slow down behind someone).
        int32 BasketUnits = 0;
        for (const FMarketBasketItem& Item : C.Basket) BasketUnits += Item.Quantity;
        float Pace = MarketMotion::Speed(C.Gait, Profile.WalkSpeed, BasketUnits, FMath::Clamp(DayTime / 240.f, 0.f, 1.f));
        FVector Step = Target;
        if (C.Stage != 2)
        {
            TArray<FVector> Others;
            for (int32 J = 0; J < Customers.Num(); ++J) if (J != I && Customers[J].Actor) Others.Add(Customers[J].Actor->GetActorLocation());
            for (const FMarketWorker& W : Workers) if (W.Actor) Others.Add(W.Actor->GetActorLocation());
            float Yield = 1.f;
            Step = MarketMotion::Steer(From, Target, Others, C.Gait.PersonalSpace, Yield);
            Pace *= Yield;
            C.WalkTime += DeltaTime;
            const FVector Along = FVector(Target.X - From.X, Target.Y - From.Y, 0.f);
            if (Along.Size() > 80.f)
            {
                const FVector Side = FVector(-Along.Y, Along.X, 0.f).GetSafeNormal();
                Step += Side * MarketMotion::SwayOffset(C.Gait, C.WalkTime);
            }
        }
        if (C.bHuman)
        {
            FVector Direction;
            bool bMoving = false;
            C.Actor->SetActorLocation(MarketPeople::MoveToward(C.Shopper, From, Step, Pace, DeltaTime, Direction, bMoving));
            MarketPeople::Update(C.Actor, People, C.Shopper, Direction, bMoving, MetaHumanYawOffset, DeltaTime);
        }
        else C.Actor->SetActorLocation(FMath::VInterpConstantTo(From, Step, DeltaTime, Pace * 1.25f));
        if (bOnRoute)
        {
            if (FVector::Dist2D(C.Actor->GetActorLocation(), Target) < 8) C.Route.RemoveAt(0);
        }
        else if (FVector::Dist2D(C.Actor->GetActorLocation(), Target) < 5)
        {
            if (C.Stage == 0)
            {
                // Look at the shelf first: a retiree reads the labels, a child grabs and goes; a regular knows the
                // shelf, an empty one means searching, a price above the rival's means comparing (G-070).
                if (C.BrowseNeed < 0.f)
                {
                    const bool bOnShelf = State.Stock.IsValidIndex(C.Product) && State.Stock[C.Product].Shelf > ReservedUnits(C.Product);
                    const float Rival = Products.IsValidIndex(C.Product) ? static_cast<float>(Products[C.Product].BasePrice) * RivalPriceFactor(C.Product) : 0.f;
                    const float Ratio = Rival > 0.f && State.Stock.IsValidIndex(C.Product) ? static_cast<float>(State.Stock[C.Product].Price) / Rival : 0.f;
                    C.BrowseNeed = MarketMotion::BrowseSeconds(Profile.BrowseSeconds, C.Gait.Style, C.bReturning, bOnShelf, Ratio);
                }
                C.Browse += DeltaTime;
                if (C.Browse >= C.BrowseNeed) { C.Browse = 0.f; C.BrowseNeed = -1.f; ResolveCustomerItem(C); }
            }
            else if (C.Stage == 1)
            {
                // Seeing the queue: with a small basket and little time, some put the basket down and leave (G-070).
                if (MarketMotion::Balks(static_cast<MarketCustomers::ESegment>(C.Segment), QueueSize(), BasketUnits, Random.FRand()))
                {
                    MarketDemand::RecordWaitingLoss(State);
                    MarketBasket::RecordVisit(State, C.CustomerId, C.ShoppingList.Num(), 0, true);
                    C.Actor->Destroy(); Customers.RemoveAt(I); continue;
                }
                C.Stage = 2; C.QueueTicket = NextQueueTicket++;
            }
            else if (C.Stage == 3)
            {
                ++State.Lost;
                MarketBasket::RecordVisit(State, C.CustomerId, C.ShoppingList.Num(), 0, false);
                C.Actor->Destroy(); Customers.RemoveAt(I); continue;
            }
        }
        ++I;
    }
    if (MarketStaff::CashierOnDuty(State))
    {
        // The cashier's pace depends on the person (speed, routine, fatigue) and on the basket size (MarketStaff).
        AutoCheckoutTimer += DeltaTime;
        const int32 Front = FMarketQueueRules::FindFront(Customers);
        int32 Units = 0;
        if (Front != INDEX_NONE) for (const FMarketBasketItem& Item : Customers[Front].Basket) Units += Item.Quantity;
        const FMarketEmployee* Cashier = MarketStaff::OnDutyAt(State, MarketStaff::ERole::Cashier, 0);
        const float Needed = Cashier ? MarketStaff::CheckoutSeconds(*Cashier, Units) : 4.f;
        if (AutoCheckoutTimer >= Needed && QueueSize() > 0) { Checkout(); AutoCheckoutTimer = 0; }
    }
    if (DayTime >= 240) CloseShop();
}

void AMarketGameMode::CloseShop()
{
    bOpen = false;
    // No reserved goods have left inventory. Unfinished baskets are lost sales.
    for (const auto& C : Customers)
    {
        // Those already walking out empty-handed (stage 3) did not give up waiting: their items were missing.
        if (C.Stage == 3) ++State.Lost;
        else MarketDemand::RecordWaitingLoss(State);
        MarketBasket::RecordVisit(State, C.CustomerId, C.ShoppingList.Num(), 0, C.Stage != 3);
        C.Actor->Destroy();
    }
    Customers.Empty();
    State.CloseDay();
    MarketDirector::CloseDay(State, Products); // every background system (staff, books, ...) in a fixed order
    RefreshPrices();                           // a new month brings a new price list
    SyncWorkers();                // days off and leavers change who walks tomorrow
    RefreshLabels(); RefreshDeliveryCrates();
    bWeekJustEnded = MarketCampaign::CloseDay(State); // weekly report every 7 days
    ReportTime = 30;
    const bool bSaved = SaveCampaign();
    FString Delivery = State.DeliveryUnits() > 0 ? FString::Printf(TEXT(" %d urun arka kapida; depoya tasi."), State.DeliveryUnits()) : FString();
    if (State.LastDeliveryMissing + State.LastDeliveryDamaged > 0)
        Delivery += FString::Printf(TEXT(" Tedarik sorunu: %d eksik, %d hasarli."), State.LastDeliveryMissing, State.LastDeliveryDamaged);
    Notify(FString::Printf(TEXT("Gun bitti. Net sonuc: %s.%s %s"), *Money(State.LastProfit), *Delivery, bSaved ? TEXT("Otomatik kaydedildi.") : TEXT("KAYIT YAZILAMADI; F5 ile yeniden dene.")));
    OpenDayReport(); // G-059: the report stays in the menu until "Yeni gune basla"
}
bool AMarketGameMode::SaveCampaign()
{
    if (IsBranchVisit() || FParse::Param(FCommandLine::Get(), TEXT("SimMenuCapture")) || FParse::Param(FCommandLine::Get(), TEXT("BranchVisitReview"))) return false;
    auto* Save = Cast<UMarketSave>(UGameplayStatics::CreateSaveGameObject(UMarketSave::StaticClass()));
    Save->State = State;
    Save->State.Version = FMarketState::CurrentVersion;
    Save->State.PlanogramJson = MarketPlanogram::Serialize(Planogram); // G-078 (#5): the layout belongs to the campaign
    const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, SlotName(ActiveSlot), 0);
    if (bSaved) RefreshSlotSummaries();
    return bSaved;
}
void AMarketGameMode::LoadCampaign(bool bQuiet)
{
    auto* Save = Cast<UMarketSave>(UGameplayStatics::LoadGameFromSlot(SlotName(ActiveSlot), 0));
    if (Save && Save->State.Version != FMarketState::CurrentVersion) { Notify(TEXT("Bu kay\u0131t oyunun eski bir s\u00fcr\u00fcm\u00fcnden; yeni oyun ba\u015flat.")); return; }
    if (!Save || !Save->State.IsStructurallyValid()) { if (!bQuiet) Notify(TEXT("Uyumlu kayit bulunamadi. Mevcut kampanya korunuyor.")); return; }
    if (bArrange) ExitArrange(FString());
    DropCarriedDelivery();
    State = Save->State; Selected = 0; bWeekJustEnded = false; OrderDraftCases.Init(0, Products.Num());
    MarketCountry::SetActive(State.CountryId, State.RivalSeed); MarketEras::Activate(State); // G-084
    if (bTestMode) State.bUsedTestMode = true;
    TArray<FString> Added, Removed;
    RefreshPrices(); // today's list before the price range check of ReconcileWith
    State.ReconcileWith(Products, &Added, &Removed);
    if (!State.PlanogramJson.IsEmpty())
    {
        // G-078 (#5): this campaign's shelf plan. Only the blocks change; the fixtures of the shop stay.
        FMarketPlanogram Loaded;
        TArray<FString> PlanErrors;
        if (MarketPlanogram::Parse(State.PlanogramJson, Loaded, PlanErrors) && Loaded.Fixtures.Num() == Planogram.Fixtures.Num())
        {
            Planogram = Loaded;
            MarketPlanogram::ResolvePositions(Planogram, Products);
            MarketPlanogram::FitDepth(Planogram, Products);
            ++ArrangeVersion;
            RebuildShelfContents();
        }
        else UE_LOG(LogTemp, Warning, TEXT("MarketSim: the saved shelf plan does not fit this shop; Config/planograms.json is kept."));
    }
    ApplyCapacities();
    SyncWorkers();     // the walking workers follow the loaded roster
    ResetWorkerJobs(); // stock rows may have moved; workers pick new jobs
    RefreshLabels(); RefreshDeliveryCrates();
    if (Added.Num() + Removed.Num() > 0)
        Notify(FString::Printf(TEXT("Kampanya yuklendi. Katalog degismis: %d yeni urun (raf bos, siparis ver), %d kaldirilan urun."), Added.Num(), Removed.Num()));
    else Notify(TEXT("Kampanya yuklendi. Hazir oldugunda O ile marketi ac."));
}

void AMarketHUD::DrawHUD()
{
    Super::DrawHUD();
    if (Overlay.IsValid()) { if (auto* Mode = GetMarket(this)) Overlay->SetVisibility(Mode->IsBranchVisit() || Mode->bMenuOpen ? EVisibility::Collapsed : EVisibility::HitTestInvisible); return; }
    AMarketGameMode* Game = GetMarket(this);
    if (!Game || Game->bStoreTour || Game->Products.Num() == 0 || !GEngine || !GEngine->GameViewport) return;
    // Created lazily: the game mode may begin play after the HUD.
    Overlay = SNew(SMarketHud).Game(Game);
    GEngine->GameViewport->AddViewportWidgetContent(Overlay.ToSharedRef(), 10);
    Menu = SNew(SMarketMenu).Game(Game);
    GEngine->GameViewport->AddViewportWidgetContent(Menu.ToSharedRef(), 20);
}

void AMarketHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (Overlay.IsValid() && GEngine && GEngine->GameViewport) GEngine->GameViewport->RemoveViewportWidgetContent(Overlay.ToSharedRef());
    if (Menu.IsValid() && GEngine && GEngine->GameViewport) GEngine->GameViewport->RemoveViewportWidgetContent(Menu.ToSharedRef());
    Overlay.Reset();
    Menu.Reset();
    Super::EndPlay(EndPlayReason);
}

bool AMarketGameMode::AdvanceTime(MarketSimulation::ETurn Turn)
{
    if (bOpen || bNeedStart || bStoreTour || IsBranchVisit())
    {
        Notify(TEXT("Gun ilerletmek icin ilk magazanin acik gununu once kapat."));
        return false;
    }
    const int32 WeekBefore = State.LastWeekNumber;
    LastAdvance = MarketSimulation::AdvanceTurn(State, CatalogBase, Products, Turn);
    bWeekJustEnded = State.LastWeekNumber != WeekBefore;
    if (LastAdvance.Played > 0)
    {
        ResetWorkerJobs(); RefreshDeliveryCrates(); SyncWorkers();
        RefreshPrices(); RefreshLabels(); SaveCampaign();
        OpenDayReport();
    }
    Notify(LastAdvance.Message);
    return LastAdvance.Played > 0;
}

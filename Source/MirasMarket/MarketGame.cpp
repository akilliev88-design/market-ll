#include "MarketGame.h"
#include "MarketVisuals.h"
#include "ProductCatalog.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "MarketHudWidget.h"
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
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#if WITH_EDITOR
#include "ShaderCompiler.h"
#endif

namespace
{
    FString Money(int64 Value) { return FString::Printf(TEXT("%.2f TL"), Value / 100.0); }
    // World-space text uses a distance-field font without Turkish glyphs: fold to ASCII there.
    FString AsciiFold(const FString& In)
    {
        FString Out = In;
        const TCHAR* From[] = { TEXT("\u00e7"), TEXT("\u00c7"), TEXT("\u011f"), TEXT("\u011e"), TEXT("\u0131"), TEXT("\u0130"), TEXT("\u00f6"), TEXT("\u00d6"), TEXT("\u015f"), TEXT("\u015e"), TEXT("\u00fc"), TEXT("\u00dc"), TEXT("\u00e2"), TEXT("\u00ee"), TEXT("\u00fb") };
        const TCHAR* To[] = { TEXT("c"), TEXT("C"), TEXT("g"), TEXT("G"), TEXT("i"), TEXT("I"), TEXT("o"), TEXT("O"), TEXT("s"), TEXT("S"), TEXT("u"), TEXT("U"), TEXT("a"), TEXT("i"), TEXT("u") };
        for (int32 I = 0; I < UE_ARRAY_COUNT(From); ++I) Out.ReplaceInline(From[I], To[I], ESearchCase::CaseSensitive);
        return Out;
    }
    AMarketGameMode* GetMarket(const AActor* Actor) { return Cast<AMarketGameMode>(UGameplayStatics::GetGameMode(Actor)); }
    const TCHAR* MarketSaveSlot()
    {
        return FParse::Param(FCommandLine::Get(), TEXT("MirasSmoke")) ? TEXT("MirasMarket_TestOnly") : TEXT("MirasMarket_Campaign_v1");
    }
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
    Input->BindAction("Mood", IE_Pressed, this, &AMarketCharacter::NextMood);
    Input->BindAction("Fullscreen", IE_Pressed, this, &AMarketCharacter::ToggleFullscreen);
    Input->BindAction("Quit", IE_Pressed, this, &AMarketCharacter::Quit);
}
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
MARKET_ACTION(NextMood, "Mood")
MARKET_ACTION(ToggleFullscreen, "Fullscreen")
#undef MARKET_ACTION
void AMarketCharacter::Quit() { UKismetSystemLibrary::QuitGame(this, Cast<APlayerController>(GetController()), EQuitPreference::Quit, false); }

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
    ApplyCapacities();
    if (bUseMetaHumans)
    {
        People = MarketPeople::Load();
        for (const TSubclassOf<AActor>& Class : People.Classes) PeopleAssets.Add(Class.Get());
        if (People.Walk.IsValid()) PeopleAssets.Add(People.Walk.Get());
        if (People.Idle.IsValid()) PeopleAssets.Add(People.Idle.Get());
    }
    bTestMode = bTestModeAtStart && !FParse::Param(FCommandLine::Get(), TEXT("MirasSmoke"));
    // The inherited shop opens with every shelf block stocked, in test mode and in normal play alike.
    FillAllShelves();
    BuildStore();
    if (auto* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        if (APawn* Pawn = PC->GetPawn()) Pawn->SetActorLocation(FVector(0, -180, 100));
        PC->SetControlRotation(FRotator(0, 90, 0));
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
    }
    Random.Initialize(2011);
    RefreshLabels();
    Notify(bTestMode
        ? FString(TEXT("TEST MODU: raflar dolu. E rafi bedava doldurur, F3 hepsini doldurur, F2 kapatir. O ile ac."))
        : FString(TEXT("2011, Luleburgaz. Ailenden kalan market senin. Raflara yaklas: E. Sonra O ile ac.")));
    UE_LOG(LogTemp, Display, TEXT("MirasMarket ready: %d products, %d fixtures, starting cash %lld kurus."), Products.Num(), Planogram.Fixtures.Num(), State.Cash);
}

void AMarketGameMode::LoadCatalog()
{
    TArray<FString> Errors;
    MarketCatalog::LoadFile(MarketCatalog::DefaultPath(), Products, Errors);
    for (const FString& Error : Errors) UE_LOG(LogTemp, Warning, TEXT("MirasMarket catalog: %s"), *Error);
    Products.RemoveAll([](const FMarketProduct& P) { return !P.bActive; }); // preparation list stays in the studio
    if (Products.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("MirasMarket: products.json has no valid products; using placeholders."));
        for (int32 I = 0; I < 6; ++I)
        {
            FMarketProduct P;
            P.Id = FString::Printf(TEXT("fallback_%d"), I);
            P.RealName = P.FictionalName = FString::Printf(TEXT("Urun %d"), I + 1);
            P.Cost = 100; P.BasePrice = 200; Products.Add(P);
        }
    }
}

void AMarketGameMode::LoadPlanogram()
{
    TArray<FString> Errors;
    MarketPlanogram::LoadFile(MarketPlanogram::DefaultPath(), Planogram, Errors);
    MarketPlanogram::Reconcile(Planogram, Products);
    // "As many as fit": every block uses its level's free width and the full shelf depth.
    if (Planogram.bAutoFill) MarketPlanogram::FillToCapacity(Planogram, Products, true);
    MarketPlanogram::FindOverflows(Planogram, Products, Errors);
    for (const FString& Error : Errors) UE_LOG(LogTemp, Warning, TEXT("MirasMarket planogram: %s"), *Error);
    float FurthestY = 320.f;
    for (const FPlanogramFixture& Fixture : Planogram.Fixtures) FurthestY = FMath::Max(FurthestY, Fixture.Location.Y);
    StoreRows = FMath::Max(2, FMath::CeilToInt(FurthestY / 320.f));
}

void AMarketGameMode::ApplyCapacities()
{
    TArray<int32> Capacities;
    for (const FMarketProduct& Product : Products)
    {
        const int32 Capacity = MarketPlanogram::ProductCapacity(Planogram, Product.Id);
        Capacities.Add(Capacity > 0 ? Capacity : FMarketState::DefaultShelfCapacity);
    }
    const int32 Discarded = State.ApplyShelfCapacities(Capacities);
    if (Discarded > 0) UE_LOG(LogTemp, Warning, TEXT("MirasMarket: %d units did not fit shelf + storage after a planogram change."), Discarded);
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
    SurfaceBox(FVector(0, Middle, 355), FVector(1250, Length, 15), EMarketSurface::Ceiling);
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
    Label(FVector(0, Back - 13.8f, 300), FRotator(0, -90, 0), TEXT("MIRAS MARKET  /  LULEBURGAZ 2011"), 24, FColor::White, true);
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
        if (!Mesh) { UE_LOG(LogTemp, Warning, TEXT("MirasMarket: store kit mesh missing: %s"), *AssetPath); return nullptr; }
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
        Label(BulkLocation + FVector(0, 12.4f, 158), FRotator(0, -90, 0), TEXT("KURUYEMIS  /  DOKME"), 11, FColor::White, true);
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
    else Label(BulkLocation + FVector(0, -54, 157), FRotator(0, -90, 0), TEXT("KURUYEMIS  /  LOKUM"), 13, FColor(255, 225, 165));
    const TCHAR* CeilingPath = TEXT("/Game/Environment/StoreKit/CeilingBay_6000/SM_CeilingBay_6000.SM_CeilingBay_6000");
    for (float X : { -300.f, 300.f })
        for (float Y : { 200.f, 800.f })
            SpawnKit(CeilingPath, FVector(X, Y, 343), FRotator::ZeroRotator, false);

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
            for (int32 Level = 0; Level < MarketPlanogram::LevelCount; ++Level)
            {
                const float Z = 18 + Level * 34;
                SurfaceBox(P + FVector(0, -8, Z), FVector(120, 80, 3), EMarketSurface::ShelfMetal);
                SurfaceBox(P + FVector(0, -45, Z + 3), FVector(116, 3, 7), EMarketSurface::PriceRail);
            }
        }
        // Category sign: on top of a gondola (readable from both aisles) or on a wall shelf's header.
        const FString SignText = AsciiFold(Fixture.Label).ToUpper();
        if (Spec.bSignOnTop)
        {
            auto* Sign = SurfaceBox(FixtureXf.TransformPosition(FVector(0, 0, Spec.SignZ)), FVector(Spec.SignWidthCm, 2.5f, 22), EMarketSurface::SignRed, false);
            Sign->SetActorRotation(FixtureRotation);
            for (int32 Side = 0; Side < (Spec.bDoubleSided ? 2 : 1); ++Side)
                Label(FixtureXf.TransformPosition(FVector(0, Side == 0 ? -1.45f : 1.45f, Spec.SignZ)), FixtureRotation + FRotator(0, Side == 0 ? -90.f : 90.f, 0),
                    SignText, 10, FColor::White, true)->SetCullDistance(2000);
        }
        else
        {
            auto* Sign = SurfaceBox(FixtureXf.TransformPosition(FVector(0, Spec.SignY - 0.6f, Spec.SignZ)), FVector(Spec.SignWidthCm, 1.2f, 16), EMarketSurface::SignRed, false);
            Sign->SetActorRotation(FixtureRotation);
            Label(FixtureXf.TransformPosition(FVector(0, Spec.SignY - 1.4f, Spec.SignZ)), FixtureRotation + FRotator(0, -90.f, 0), SignText, 10, FColor::White, true)->SetCullDistance(2000);
        }
    }
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
            auto* Price = Label(FixtureXf.TransformPosition(TagLocal + FVector(0, FaceSign * 0.4f, 0)), FixtureRotation + FRotator(0, bBack ? 90.f : -90.f, 0),
                Money(State.Stock[I].Price), 2.0f, FColor(25, 22, 20), true);
            Price->SetCullDistance(900);
            ShelfLabels.Add(Price);
            ShelfLabelProduct.Add(I);
        }
    }
    // Checkout counter and office desk in walnut with dark tops; depot cartons.
    // Real-world proportions (a till, a thin monitor on a stand, a keyboard) instead of one big dark box.
    SurfaceBox(FVector(405, -45, 45), FVector(230, 90, 90), EMarketSurface::Wood);
    SurfaceBox(FVector(405, -45, 91.5f), FVector(236, 96, 3), EMarketSurface::CounterTop);
    SurfaceBox(FVector(405, -92, 8), FVector(232, 4, 16), EMarketSurface::DarkMetal, false);          // toe kick
    SurfaceBox(FVector(445, -40, 98), FVector(36, 32, 10), EMarketSurface::DarkMetal);               // till drawer
    SurfaceBox(FVector(445, -30, 106), FVector(6, 6, 12), EMarketSurface::DarkMetal, false);          // screen post
    SurfaceBox(FVector(445, -30, 120), FVector(28, 2.5f, 20), EMarketSurface::DarkMetal, false);      // till screen
    SurfaceBox(FVector(380, -60, 94), FVector(30, 30, 1), EMarketSurface::DarkMetal, false);          // scale plate
    Label(FVector(405, -105, 185), FRotator(0, -90, 0), TEXT("KASA  [E]"), 24);
    SurfaceBox(FVector(-430, -30, 42), FVector(210, 90, 84), EMarketSurface::Wood);
    SurfaceBox(FVector(-430, -30, 85.5f), FVector(214, 94, 3), EMarketSurface::CounterTop);
    SurfaceBox(FVector(-430, -8, 88), FVector(20, 14, 2), EMarketSurface::DarkMetal, false);          // monitor foot
    SurfaceBox(FVector(-430, -6, 96), FVector(4, 3, 16), EMarketSurface::DarkMetal, false);           // monitor neck
    SurfaceBox(FVector(-430, -8, 115), FVector(54, 2.5f, 32), EMarketSurface::DarkMetal, false);      // monitor
    SurfaceBox(FVector(-430, -45, 87.5f), FVector(40, 13, 1.5f), EMarketSurface::DarkMetal, false);   // keyboard
    SurfaceBox(FVector(-470, -50, 88), FVector(21, 29.7f, 1), EMarketSurface::TagWhite, false);      // papers
    Label(FVector(-430, -85, 188), FRotator(0, -90, 0), TEXT("YONETIM MASASI"), 18);
    // Depot: stacked cartons of different sizes, slightly turned.
    for (int32 I = 0; I < 6; ++I)
    {
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
    Label(FVector(0, Back - 30, 140), FRotator(0, -90, 0), TEXT("DEPO"), 20);
    for (float Y = 40.f; Y < Back; Y += 320.f)
    {
        SurfaceBox(FVector(0.f, Y, 337.f), FVector(1250.f, 10.f, 12.f), EMarketSurface::CeilingSteel, false);
        SurfaceBox(FVector(-300.f, Y, 329.f), FVector(290.f, 22.f, 3.f), EMarketSurface::Emissive, false);
        SurfaceBox(FVector(300.f, Y, 329.f), FVector(290.f, 22.f, 3.f), EMarketSurface::Emissive, false);
    }
    Lighting = MarketVisuals::BuildStoreLighting(GetWorld(), Back);
    MarketVisuals::ApplyMood(Lighting, Mood);
}

void AMarketGameMode::BuildShelfItems(int32 Index)
{
    // One visible unit per shelf slot (facings x depth = capacity). Studio products use their real
    // package mesh + label material; others fall back to colored prototype boxes.
    const FMarketProduct& Product = Products[Index];
    UStaticMesh* Mesh = nullptr;
    TArray<UMaterialInterface*> SlotMaterials;
    if (!Product.MeshPath.IsEmpty())
    {
        Mesh = LoadObject<UStaticMesh>(nullptr, *Product.MeshPath);
        if (!Mesh) UE_LOG(LogTemp, Warning, TEXT("MirasMarket: package mesh missing for %s: %s"), *Product.Id, *Product.MeshPath);
        for (const FString& Path : Product.Materials)
        {
            UMaterialInterface* Slot = Path.IsEmpty() ? nullptr : LoadObject<UMaterialInterface>(nullptr, *Path);
            if (!Path.IsEmpty() && !Slot) UE_LOG(LogTemp, Warning, TEXT("MirasMarket: material missing for %s: %s"), *Product.Id, *Path);
            SlotMaterials.Add(Slot);
        }
    }
    FVector ItemScale(1);
    FRotator ModelRotation = FRotator::ZeroRotator;
    FVector PlacementOffset = FVector::ZeroVector;
    FBox SourceBounds(FVector(-50), FVector(50));
    if (Mesh)
    {
        SourceBounds = Mesh->GetBoundingBox();
        ItemScale = FVector(FMath::Clamp(Product.VisualScale, 0.001f, 1000.f));
        ModelRotation = Product.VisualRotation;
        PlacementOffset = Product.VisualOffsetCm;
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
        Mesh = Cube;
        ItemScale = FVector(.25f, .21f, .30f); // depth, width, height of the v0.1 placeholder boxes
        auto* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Market.M_Market"));
        if (!Base) Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
        auto* Colored = UMaterialInstanceDynamic::Create(Base, this);
        Colored->SetVectorParameterValue(TEXT("Color"), FLinearColor(Product.Color));
        SlotMaterials = { Colored };
    }
    const FTransform ShapeTransform(ModelRotation, FVector::ZeroVector, ItemScale);
    const FBox Bounds = SourceBounds.TransformBy(ShapeTransform);
    const FVector Size = Bounds.GetSize();   // X = depth (front axis), Y = width, Z = height
    const FVector Center = Bounds.GetCenter();
    const float Bottom = Bounds.Min.Z;
    const float Gap = 2;
    TArray<FTransform>& Slots = ShelfSlots.AddDefaulted_GetRef();
    TArray<FVector>& Approach = ShelfApproach.AddDefaulted_GetRef();
    // All units of a product share one instanced mesh; slots are ordered primary block first,
    // front row first, center-out, so a partly sold shelf still looks faced up.
    AActor* Holder = GetWorld()->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator);
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
    else UE_LOG(LogTemp, Warning, TEXT("MirasMarket: %s materials lack the instanced-mesh usage flag; using single meshes (run GORSEL_HAZIRLA.cmd)."), *Product.Id);
    ShelfInstances.Add(Instances);
    TArray<TWeakObjectPtr<UStaticMeshComponent>>& Singles = ShelfSingles.AddDefaulted_GetRef();

    const FVector BoundsCenterOffset(Center.X, Center.Y, 0);
    struct FSlot { int32 Row; int32 Seq; FTransform Transform; };
    TArray<FSlot> Ordered;
    for (const FPlanogramPlacement& Placement : Planogram.Placements)
    {
        if (Placement.ProductId != Product.Id) continue;
        const FPlanogramFixture* Fixture = Planogram.FindFixture(Placement.FixtureId);
        if (!Fixture) continue;
        const FPlanogramEquipment Spec = MarketPlanogram::Equipment(Fixture->EquipmentId);
        const int32 Level = FMath::Clamp(Placement.Level, 0, Spec.Levels - 1);
        const int32 Columns = FMath::Clamp(Placement.Facings, 1, MarketPlanogram::MaxFacings);
        // Never draw rows past the shelf's usable depth, even if the mesh is deeper than the catalog says.
        const int32 RowsByMesh = FMath::Max(1, FMath::FloorToInt((Spec.UsableDepthCm + Gap) / FMath::Max(1.f, Size.X + Gap)));
        const int32 Rows = FMath::Clamp(FMath::Min(Placement.Depth, RowsByMesh), 1, MarketPlanogram::MaxDepth);
        const bool bBack = Placement.Face == TEXT("back");
        const float FaceSign = bBack ? 1.f : -1.f;
        const FRotator FixtureRotation(0, Fixture->Yaw, 0);
        const FTransform FixtureXf(FixtureRotation, Fixture->Location);
        const FRotator Facing(0, bBack ? 90.f : -90.f, 0);
        const float GroupCenterX = MarketPlanogram::PlacementCenterX(Planogram, Products, Placement);
        const float Front = bBack ? -Spec.FrontY : Spec.FrontY;
        const FVector BoundsOffset = Facing.RotateVector(BoundsCenterOffset) + FVector(0, 0, Bottom);
        const FVector CorrectedOffset = Facing.RotateVector(PlacementOffset);
        const FQuat FinalRotation = Facing.Quaternion() * ModelRotation.Quaternion();
        TArray<int32> ColumnOrder;
        for (int32 Step = 0; Step < Columns; ++Step)
        {
            const int32 Offset = Step == 0 ? 0 : (Step + 1) / 2 * (Step % 2 == 1 ? -1 : 1);
            const int32 Column = FMath::Clamp((Columns - 1) / 2 + Offset, 0, Columns - 1);
            if (!ColumnOrder.Contains(Column)) ColumnOrder.Add(Column);
        }
        for (int32 Row = 0; Row < Rows; ++Row)
            for (const int32 Col : ColumnOrder)
            {
                const FVector SlotLocal(GroupCenterX + (Col - (Columns - 1) * 0.5f) * (Size.Y + Gap),
                    Front - FaceSign * (Size.X * 0.5f + Row * (Size.X + Gap)),
                    Spec.LevelTopZ[Level]);
                // Hand-stocked look: each unit sits a little off the grid and turned a few degrees.
                // Deterministic per product/slot so the shelf looks the same every run.
                FRandomStream Hand(Index * 7919 + Ordered.Num() * 131 + 17);
                const float Nudge = FMath::Min(Gap * 0.45f, 0.9f);
                const FVector Jitter(Hand.FRandRange(-Nudge, Nudge), -FaceSign * Hand.FRandRange(0.f, Row == 0 ? 2.5f : 1.f), 0.f);
                const FQuat Turn(FVector::UpVector, FMath::DegreesToRadians(Hand.FRandRange(-4.f, 4.f)));
                const FTransform Local(Turn * FinalRotation, SlotLocal + Jitter - BoundsOffset + CorrectedOffset, ItemScale);
                Ordered.Add({ Row, Ordered.Num(), Local * FixtureXf });
            }
        // Shopper spot: 70 cm in front of the block, on the floor.
        Approach.Add(FixtureXf.TransformPosition(FVector(GroupCenterX, Front + FaceSign * 70.f, 0)));
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
        const int32 Visible = Slots.Num() >= Capacity ? FMath::Min(Shelf, Slots.Num()) : FMath::DivideAndRoundUp(Shelf * Slots.Num(), Capacity);
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
float AMarketGameMode::RivalDiscount() const { return State.Day >= 3 && State.Day % 5 <= 3 ? .85f : 1.f; }
int32 AMarketGameMode::NearbyShelf() const
{
    const auto* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Pawn) return INDEX_NONE;
    // Nearest block of any product (primary or extra), measured from the spot in front of it.
    int32 Result = INDEX_NONE; float Best = 120;
    for (int32 I = 0; I < Products.Num(); ++I)
    {
        if (!ShelfApproach.IsValidIndex(I) || ShelfApproach[I].Num() == 0)
        {
            const float Distance = FVector::Dist2D(Pawn->GetActorLocation(), ProductFixtureLocation(I));
            if (Distance < Best) { Best = Distance; Result = I; }
            continue;
        }
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
    // "K: text" = key K does the action (the HUD draws K as a key cap). Proper Turkish: Slate font.
    if (NearOffice()) return FString::Printf(TEXT("B: %d adet sipari\u015f ver   \u00b7   TAB \u00fcr\u00fcn   \u00b7   +/- fiyat   \u00b7   H kasiyer   \u00b7   G ikinci \u015fube"), Products[Selected].CaseUnits);
    if (NearCounter()) return FString::Printf(TEXT("E: S\u0131radaki m\u00fc\u015fterinin \u00f6demesini al   \u00b7   bekleyen %d"), QueueSize());
    const int32 I = NearbyShelf();
    if (I != INDEX_NONE) return bTestMode
        ? FString::Printf(TEXT("E: %s raf\u0131n\u0131 doldur (test: bedava)   \u00b7   raf %d / %d"), *ProductName(I), State.Stock[I].Shelf, State.Stock[I].Capacity)
        : FString::Printf(TEXT("E: %s raf\u0131n\u0131 depodan doldur   \u00b7   raf %d / %d   \u00b7   depo %d"), *ProductName(I), State.Stock[I].Shelf, State.Stock[I].Capacity, State.Stock[I].Warehouse);
    return TEXT("Raf, kasa veya y\u00f6netim masas\u0131na yakla\u015f.");
}
void AMarketGameMode::RefreshLabels()
{
    for (int32 L = 0; L < ShelfLabels.Num() && ShelfLabelProduct.IsValidIndex(L); ++L)
        if (ShelfLabels[L] && State.Stock.IsValidIndex(ShelfLabelProduct[L]))
            ShelfLabels[L]->SetText(FText::FromString(Money(State.Stock[ShelfLabelProduct[L]].Price)));
    RefreshShelfItems();
}

void AMarketGameMode::Command(FName Action)
{
    if (Action == "ToggleShop")
    {
        if (bOpen) CloseShop();
        else
        {
            bOpen = true; DayTime = 0; SpawnTimer = 1; AutoCheckoutTimer = 0; NextQueueTicket = 0;
            Random.Initialize(2011 + State.Day * 73);
            Notify(TEXT("Market acildi. Musterileri kasada bekletme. Gun 4 dakika surer; O erken kapatir."));
        }
    }
    else if (Action == "Interact")
    {
        if (NearCounter()) Checkout();
        else if (NearOffice()) Notify(TEXT("TAB: urun sec / B: koli siparis / +/-: fiyat / H: kasiyer / G: ikinci sube"));
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
                Notify(Count > 0 ? FString::Printf(TEXT("%d adet rafa yerlestirildi."), Count) : FString(TEXT("Raf dolu veya depo bos. Yonetim masasindan siparis ver.")));
            }
            RefreshLabels();
        }
    }
    else if (Action == "Brands") { State.bRealBrands = !State.bRealBrands; RefreshLabels(); }
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
        if (FParse::Param(FCommandLine::Get(), TEXT("MirasSmoke"))) return;
        bTestMode = !bTestMode;
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
            State.Initialize(Products); ApplyCapacities();
            FillAllShelves();
            RefreshLabels(); ResetConfirmUntil = -1; Notify(TEXT("Yeni kampanya basladi. Raflari doldur ve O ile ac."));
        }
    }
    else
    {
        if (!NearOffice()) { Notify(TEXT("Bu karar icin giristeki YONETIM MASASI'na yaklas.")); return; }
        if (Action == "NextProduct") Selected = (Selected + 1) % Products.Num();
        else if (Action == "Order" && bTestMode)
        {
            const int32 Units = State.ReceiveFree(Selected, FMath::Clamp(Products[Selected].CaseUnits, 1, 48));
            Notify(Units > 0 ? FString::Printf(TEXT("TEST: %d adet bedava ve hemen depoya geldi."), Units) : FString(TEXT("Depo dolu (120 adet).")));
        }
        else if (Action == "Order") Notify(State.Order(Selected, Products) ? FString::Printf(TEXT("%d adet siparis verildi. Odeme simdi; teslim ertesi sabah depoya."), Products[Selected].CaseUnits) : FString(TEXT("Yetersiz nakit veya depo + siparis limiti (120 adet).")));
        else if (Action == "PriceUp" || Action == "PriceDown")
        {
            auto& Item = State.Stock[Selected];
            Item.Price = FMath::Clamp<int64>(Item.Price + (Action == "PriceUp" ? 25 : -25), 10, Products[Selected].BasePrice * 3);
            RefreshLabels();
        }
        else if (Action == "Hire")
        {
            if (State.bCashier) Notify(TEXT("Kasiyerin zaten var. Gunluk ucret: 20 TL."));
            else if (State.Cash < 12000) Notify(TEXT("Ise alim icin 120 TL gerekiyor. Gunluk ucret: 20 TL."));
            else { State.Cash -= 12000; State.bCashier = true; Notify(TEXT("Kasiyer ise alindi. 4 saniyede bir odeme alir; gunluk ucret 20 TL.")); }
        }
        else if (Action == "Expand")
        {
            if (State.bSecondStore) Notify(TEXT("Ikinci sube acik. Bu prototipte subenin gunluk net katkisi hesaplanir."));
            else if (State.Cash < 95000 || State.ProfitableDays < 3 || State.MarketShare < 35)
                Notify(TEXT("Ikinci sube: 950 TL nakit, 3 karli gun ve en az %35 yerel musteri payi gerekli."));
            else { State.Cash -= 95000; State.bSecondStore = true; Notify(TEXT("IKINCI SUBEN ACILDI! Prototip hedefi tamam. Isletmeye devam edebilirsin.")); }
        }
    }
}

void AMarketGameMode::SpawnCustomer()
{
    if (Customers.Num() >= 9) { ++State.Lost; return; }
    const int32 I = Random.RandRange(0, Products.Num() - 1);
    const int32 Quantity = Random.RandRange(1, 4);
    int32 Reserved = 0;
    for (const auto& Other : Customers) if (Other.Product == I) Reserved += Other.Quantity;
    const double Reference = Products[I].BasePrice * RivalDiscount();
    const double Tolerance = Random.FRandRange(1.02f, 1.5f) + State.MarketShare / 500.0;
    if (State.Stock[I].Shelf - Reserved < Quantity || State.Stock[I].Price > Reference * Tolerance)
    { ++State.Lost; return; }
    FMarketCustomer C;
    C.Product = I; C.Quantity = Quantity; C.QuotedPrice = State.Stock[I].Price;
    if (AActor* Human = MarketPeople::Spawn(GetWorld(), People, FVector(0, -230, 0), Random.RandRange(0, 1 << 20), C.Shopper))
    {
        C.Actor = Human; C.bHuman = true;
        Customers.Add(C);
        return;
    }
    C.Actor = Box(FVector(0, -230, 65), FVector(42, 42, 130), FLinearColor::MakeFromHSV8(Random.RandRange(0, 255), 130, 210), false);
    auto* Head = NewObject<UStaticMeshComponent>(C.Actor);
    Head->SetupAttachment(C.Actor->GetRootComponent());
    Head->RegisterComponent(); Head->SetStaticMesh(Sphere);
    // Absolute scale avoids inheriting the body's non-uniform dimensions.
    Head->SetAbsolute(false, false, true);
    Head->SetWorldScale3D(FVector(.34f)); Head->SetRelativeLocation(FVector(0, 0, 64));
    Head->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Customers.Add(C);
}

void AMarketGameMode::Checkout()
{
    const int32 I = FMarketQueueRules::FindFront(Customers);
    if (I == INDEX_NONE) { Notify(TEXT("Kasada odeme bekleyen musteri yok.")); return; }
    const auto& C = Customers[I];
    if (State.Sell(C.Product, C.Quantity, C.QuotedPrice, Products))
        Notify(FString::Printf(TEXT("Satis: %d x %s  +%s"), C.Quantity, *ProductName(C.Product), *Money(C.Quantity * C.QuotedPrice)));
    else { ++State.Lost; Notify(TEXT("Sepet karsilanamadi; musteri ayrildi.")); }
    C.Actor->Destroy(); Customers.RemoveAt(I); RefreshLabels();
}

void AMarketGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    MessageTime = FMath::Max(0.f, MessageTime - DeltaTime);
    ReportTime = FMath::Max(0.f, ReportTime - DeltaTime);
    if (FParse::Param(FCommandLine::Get(), TEXT("MirasSmoke")))
    {
        const auto Require = [](bool Condition, const TCHAR* Step)
        {
            if (!Condition)
            {
                UE_LOG(LogTemp, Error, TEXT("MirasMarket smoke FAILED: %s"), Step);
                FPlatformMisc::RequestExitWithStatus(false, 1);
            }
            return Condition;
        };
        if (SmokeStage == 0 && GetWorld()->GetTimeSeconds() > 1)
        {
            auto* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
            if (!Require(Pawn != nullptr, TEXT("player spawned"))) return;
            Pawn->SetActorLocation(ProductFixtureLocation(0) + FVector(0, 0, 90));
            const FMarketStock Before = State.Stock[0];
            const int32 ExpectedShelf = Before.Shelf + FMath::Max(0, FMath::Min(Before.Capacity - Before.Shelf, Before.Warehouse));
            Command("Interact");
            if (!Require(State.Stock[0].Shelf == ExpectedShelf && State.Stock[0].Shelf + State.Stock[0].Warehouse == Before.Shelf + Before.Warehouse,
                TEXT("nearby shelf interaction"))) return;
            Pawn->SetActorLocation(FVector(-430, -180, 90));
            const int32 OrderedUnits = FMath::Clamp(Products[0].CaseUnits, 1, 48);
            const int64 CashBeforeOrder = State.Cash;
            Command("Order");
            if (!Require(State.Stock[0].Incoming == OrderedUnits && State.Cash == CashBeforeOrder - Products[0].Cost * OrderedUnits, TEXT("office order"))) return;
            const int64 CashBeforeHire = State.Cash;
            Command("Hire");
            if (!Require(State.bCashier && State.Cash == CashBeforeHire - 12000, TEXT("hire cashier"))) return;
            Command("ToggleShop");
            SmokeStage = 1;
        }
        if (SmokeStage == 1 && GetWorld()->GetTimeSeconds() > 25)
        {
            CloseShop();
            if (!Require(State.Day == 2 && State.LastServed > 0 && State.Stock[0].Incoming == 0, TEXT("customer sale and next-day delivery"))) return;
            const int64 SavedCash = State.Cash;
            State.Cash = 0;
            LoadCampaign();
            if (!Require(State.Cash == SavedCash && State.bCashier, TEXT("disk save and load"))) return;
            UE_LOG(LogTemp, Display, TEXT("MirasMarket smoke PASSED: player, restock, order, hiring, %d customer sales, day close, disk save/load."), State.LastServed);
            SmokeStage = 2;
            FPlatformMisc::RequestExitWithStatus(false, 0);
        }
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("MirasCapture")))
    {
        // Waits until shaders are compiled and the image has settled (exposure, Lumen), then saves
        // a clean scene shot and a shot with the HUD for comparison with the reference photo.
        const float Now = GetWorld()->GetTimeSeconds();
        bool bShadersReady = true;
#if WITH_EDITOR
        if (GShaderCompilingManager) bShadersReady = GShaderCompilingManager->GetNumRemainingJobs() == 0;
#endif
        if (!bShadersReady) CaptureReadySince = -1;
        else if (CaptureReadySince < 0) CaptureReadySince = Now;
        const bool bSettled = CaptureReadySince >= 0 && Now - CaptureReadySince > 5;
        // Views for review: entrance, left wall shelves, gondola aisle, bulk island close-up, right wall.
        struct FView { FVector Location; FRotator Rotation; const TCHAR* File; };
        static const FView Views[] =
        {
            { FVector(0, -180, 90), FRotator(-4, 90, 0), TEXT("MirasMarket.png") },
            { FVector(-330, 300, 90), FRotator(-6, 165, 0), TEXT("MirasMarket_1_duvar_sol.png") },
            { FVector(150, 330, 90), FRotator(-8, 120, 0), TEXT("MirasMarket_2_gondol.png") },
            { FVector(110, 120, 90), FRotator(-12, 132, 0), TEXT("MirasMarket_3_dokme.png") },
            { FVector(330, 300, 90), FRotator(-6, 15, 0), TEXT("MirasMarket_4_duvar_sag.png") },
        };
        constexpr int32 ViewCount = UE_ARRAY_COUNT(Views);
        auto SetView = [this](const FView& View)
        {
            if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0)) Pawn->SetActorLocation(View.Location, false, nullptr, ETeleportType::TeleportPhysics);
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0)) PC->SetControlRotation(View.Rotation);
        };
        const int32 ViewIndex = (CaptureStage - 1) / 2;
        if (CaptureStage == 0 && Now > 8 && (bSettled || Now > 400))
        {
            SetView(Views[0]);
            CaptureStage = 1; CaptureAt = Now;
        }
        else if (CaptureStage > 0 && CaptureStage % 2 == 1 && Now > CaptureAt + 2.5f)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Screenshots") / Views[ViewIndex].File, true, false);
            CaptureStage++; CaptureAt = Now;
        }
        else if (CaptureStage > 0 && CaptureStage % 2 == 0 && Now > CaptureAt + 1.5f)
        {
            if (ViewIndex + 1 < ViewCount) { SetView(Views[ViewIndex + 1]); CaptureStage++; CaptureAt = Now; }
            else FPlatformMisc::RequestExit(false);
        }
    }
    if (!bOpen) return;
    DayTime += DeltaTime;
    SpawnTimer -= DeltaTime;
    if (SpawnTimer <= 0) { SpawnCustomer(); SpawnTimer = Random.FRandRange(3.5f, 5.5f); }
    for (int32 I = 0; I < Customers.Num();)
    {
        auto& C = Customers[I];
        C.Age += DeltaTime;
        if (C.Age > 45)
        { ++State.Lost; C.Actor->Destroy(); Customers.RemoveAt(I); continue; }
        FVector Target;
        if (C.Stage == 0) Target = ProductFixtureLocation(C.Product) + FVector(0, 0, 65);
        else if (C.Stage == 1) Target = FVector(405, 60 + QueueSize() * 62, 65);
        else Target = FVector(405, 60 + FMarketQueueRules::Rank(Customers, I) * 62, 65);
        if (C.RouteStage != C.Stage)
        {
            // Walk the open lanes: x = +/-150 runs between the bulk island and the gondolas,
            // y = 120 is the front corridor between counter/desk and the island.
            C.RouteStage = C.Stage;
            C.Route.Reset();
            const FVector Here = C.Actor->GetActorLocation();
            if (C.Stage == 0)
            {
                const float Lane = Target.X >= 0 ? 150.f : -150.f;
                C.Route = { FVector(Lane, 120.f, Target.Z), FVector(Lane, Target.Y, Target.Z) };
            }
            else if (C.Stage == 1)
            {
                const float Lane = Here.X >= 0 ? 150.f : -150.f;
                C.Route = { FVector(Lane, Here.Y, Target.Z), FVector(Lane, 120.f, Target.Z) };
            }
        }
        const bool bOnRoute = C.Route.Num() > 0;
        if (bOnRoute) Target = C.Route[0];
        if (C.bHuman) Target.Z = 0.f; // MetaHuman origin is at the feet
        const FVector From = C.Actor->GetActorLocation();
        C.Actor->SetActorLocation(FMath::VInterpConstantTo(From, Target, DeltaTime, C.bHuman ? 140.f : 180.f));
        if (C.bHuman)
        {
            const bool bMoving = FVector::Dist2D(From, Target) > 6.f;
            MarketPeople::Update(C.Actor, People, C.Shopper, Target - From, bMoving, MetaHumanYawOffset, DeltaTime);
        }
        if (bOnRoute)
        {
            if (FVector::Dist2D(C.Actor->GetActorLocation(), Target) < 8) C.Route.RemoveAt(0);
        }
        else if (FVector::Dist2D(C.Actor->GetActorLocation(), Target) < 5)
        {
            if (C.Stage == 0) C.Stage = 1;
            else if (C.Stage == 1) { C.Stage = 2; C.QueueTicket = NextQueueTicket++; }
        }
        ++I;
    }
    if (State.bCashier)
    {
        AutoCheckoutTimer += DeltaTime;
        if (AutoCheckoutTimer >= 4 && QueueSize() > 0) { Checkout(); AutoCheckoutTimer = 0; }
    }
    if (DayTime >= 240) CloseShop();
}

void AMarketGameMode::CloseShop()
{
    bOpen = false;
    // No reserved goods have left inventory. Unfinished baskets are lost sales.
    State.Lost += Customers.Num();
    for (const auto& C : Customers) C.Actor->Destroy();
    Customers.Empty();
    State.CloseDay(); RefreshLabels();
    ReportTime = 20;
    const bool bSaved = SaveCampaign();
    Notify(FString::Printf(TEXT("Gun bitti. Net sonuc: %s. Siparisler depoya geldi. %s"), *Money(State.LastProfit), bSaved ? TEXT("Otomatik kaydedildi.") : TEXT("KAYIT YAZILAMADI; F5 ile yeniden dene.")));
}
bool AMarketGameMode::SaveCampaign()
{
    auto* Save = Cast<UMarketSave>(UGameplayStatics::CreateSaveGameObject(UMarketSave::StaticClass()));
    Save->State = State;
    return UGameplayStatics::SaveGameToSlot(Save, MarketSaveSlot(), 0);
}
void AMarketGameMode::LoadCampaign()
{
    auto* Save = Cast<UMarketSave>(UGameplayStatics::LoadGameFromSlot(MarketSaveSlot(), 0));
    if (!Save || !Save->State.IsStructurallyValid()) { Notify(TEXT("Uyumlu kayit bulunamadi. Mevcut kampanya korunuyor.")); return; }
    State = Save->State; Selected = 0;
    TArray<FString> Added, Removed;
    State.ReconcileWith(Products, &Added, &Removed);
    ApplyCapacities();
    RefreshLabels();
    if (Added.Num() + Removed.Num() > 0)
        Notify(FString::Printf(TEXT("Kampanya yuklendi. Katalog degismis: %d yeni urun (raf bos, siparis ver), %d kaldirilan urun."), Added.Num(), Removed.Num()));
    else Notify(TEXT("Kampanya yuklendi. Hazir oldugunda O ile marketi ac."));
}

void AMarketHUD::DrawHUD()
{
    Super::DrawHUD();
    if (Overlay.IsValid()) return;
    AMarketGameMode* Game = GetMarket(this);
    if (!Game || Game->Products.Num() == 0 || !GEngine || !GEngine->GameViewport) return;
    // Created lazily: the game mode may begin play after the HUD.
    Overlay = SNew(SMarketHud).Game(Game);
    GEngine->GameViewport->AddViewportWidgetContent(Overlay.ToSharedRef(), 10);
}

void AMarketHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (Overlay.IsValid() && GEngine && GEngine->GameViewport) GEngine->GameViewport->RemoveViewportWidgetContent(Overlay.ToSharedRef());
    Overlay.Reset();
    Super::EndPlay(EndPlayReason);
}

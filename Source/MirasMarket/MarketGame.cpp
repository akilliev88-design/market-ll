#include "MarketGame.h"
#include "MarketVisuals.h"
#include "ProductCatalog.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
    FString Money(int64 Value) { return FString::Printf(TEXT("%.2f TL"), Value / 100.0); }
    FVector ShelfPosition(int32 Index) { return FVector(-270 + (Index % 3) * 270, 320 + (Index / 3) * 320, 0); }
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
    State.Initialize(Products);
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
    Notify(TEXT("2011, Luleburgaz. Ailenden kalan market senin. Raflara yaklas: E. Sonra O ile ac."));
    UE_LOG(LogTemp, Display, TEXT("MirasMarket ready: %d products, %d shelves, starting cash %lld kurus."), Products.Num(), Products.Num(), State.Cash);
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
    StoreRows = FMath::Max(2, FMath::DivideAndRoundUp(Products.Num(), 3));
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

UTextRenderComponent* AMarketGameMode::Label(FVector Location, FRotator Rotation, const FString& Text, float Size, FColor Color)
{
    auto* Actor = GetWorld()->SpawnActor<AActor>(Location, Rotation);
    auto* Component = NewObject<UTextRenderComponent>(Actor);
    Actor->SetRootComponent(Component);
    Component->RegisterComponent();
    Component->SetWorldLocationAndRotation(Location, Rotation);
    Component->SetText(FText::FromString(Text));
    Component->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
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
    Box(FVector(0, Middle, -15), FVector(1250, Length, 30), FLinearColor(.50f, .39f, .28f));
    Box(FVector(0, Middle, 355), FVector(1250, Length, 15), FLinearColor(.055f, .045f, .035f));
    Box(FVector(-625, Middle, 165), FVector(20, Length, 360), FLinearColor(.38f, .47f, .39f));
    Box(FVector(625, Middle, 165), FVector(20, Length, 360), FLinearColor(.38f, .47f, .39f));
    Box(FVector(0, Back, 165), FVector(1250, 20, 360), FLinearColor(.39f, .47f, .39f));
    Box(FVector(-405, -300, 165), FVector(440, 20, 360), FLinearColor(.39f, .47f, .39f));
    Box(FVector(405, -300, 165), FVector(440, 20, 360), FLinearColor(.39f, .47f, .39f));
    // Entrance is a visual opening with an invisible boundary for the prototype.
    auto* Boundary = Box(FVector(0, -310, 165), FVector(360, 20, 360), FLinearColor::Black);
    Boundary->SetActorHiddenInGame(true);
    Box(FVector(0, -1300, -25), FVector(1800, 1900, 20), FLinearColor(.15f, .17f, .18f), false);
    Box(FVector(0, -1750, 220), FVector(1200, 200, 440), FLinearColor(.18f, .23f, .32f), false);
    Label(FVector(0, -1635, 270), FRotator(0, 90, 0), TEXT("BEREKET MARKET\nRakibin buyumeye hazirlaniyor"), 40, FColor(250, 185, 64));
    Label(FVector(0, Back - 20, 310), FRotator(0, -90, 0), TEXT("MIRAS MARKET  /  LULEBURGAZ 2011"), 26, FColor(245, 212, 131));
    // Fine grout lines break up the single-color floor without requiring a heavy tile mesh.
    for (float X = -550.f; X <= 550.f; X += 110.f)
        Box(FVector(X, Middle, 0.2f), FVector(1.2f, Length, 0.4f), FLinearColor(.31f, .32f, .30f), false);
    for (float Y = -250.f; Y < Back; Y += 110.f)
        Box(FVector(0, Y, 0.2f), FVector(1250.f, 1.2f, 0.4f), FLinearColor(.31f, .32f, .30f), false);
    for (int32 I = 0; I < Products.Num(); ++I)
    {
        const FVector P = ShelfPosition(I);
        const FLinearColor ShelfMetal(.20f, .21f, .20f);
        const FLinearColor ShelfEdge(.055f, .06f, .055f);
        const FLinearColor Wood(.28f, .13f, .055f);
        FLinearColor Header = FLinearColor(Products[I].Color) * 0.68f;
        Header.A = 1.f;
        Box(P + FVector(0, 30, 67), FVector(245, 3, 134), FLinearColor(.115f, .12f, .11f));
        Box(P + FVector(-121, -7, 67), FVector(6, 76, 134), Wood);
        Box(P + FVector(121, -7, 67), FVector(6, 76, 134), Wood);
        Box(P + FVector(0, -7, 10), FVector(245, 78, 20), Wood);
        for (int32 Level = 0; Level < 3; ++Level)
        {
            const float Z = 25 + Level * 44;
            Box(P + FVector(0, -8, Z), FVector(250, 80, 4), ShelfMetal);
            Box(P + FVector(0, -49, Z + 3), FVector(250, 3, 7), ShelfEdge);
        }
        Box(P + FVector(0, -48, 154), FVector(245, 4, 30), Header);
        BuildShelfItems(I);
        auto* ShelfLabel = Label(P + FVector(0, -51, 160), FRotator(0, -90, 0), AsciiFold(ProductName(I)), 10, FColor::White);
        ShelfLabel->SetCullDistance(700);
        ShelfLabels.Add(ShelfLabel);
    }
    Box(FVector(405, -45, 48), FVector(230, 95, 96), FLinearColor(.12f, .25f, .22f));
    Box(FVector(440, -45, 115), FVector(52, 42, 40), FLinearColor(.06f, .08f, .09f));
    Label(FVector(405, -105, 185), FRotator(0, -90, 0), TEXT("KASA  [E]"), 24);
    Box(FVector(-430, -30, 45), FVector(210, 90, 90), FLinearColor(.3f, .2f, .12f));
    Box(FVector(-430, -30, 113), FVector(65, 28, 46), FLinearColor(.1f, .13f, .16f));
    Label(FVector(-430, -85, 188), FRotator(0, -90, 0), TEXT("YONETIM MASASI\nTAB / B / +/- / H / G"), 18);
    for (int32 I = 0; I < 6; ++I)
        Box(FVector(-480 + I * 190, Back - 90, 30), FVector(70, 60, 60), FLinearColor(.54f, .34f, .17f));
    Label(FVector(0, Back - 30, 140), FRotator(0, -90, 0), TEXT("DEPO  /  Siparisler ertesi sabah gelir"), 20);
    for (float Y = 40.f; Y < Back; Y += 430.f)
    {
        Box(FVector(0.f, Y, 337.f), FVector(1250.f, 10.f, 12.f), FLinearColor(.045f, .04f, .035f), false);
        Box(FVector(-300.f, Y, 347.f), FVector(290.f, 28.f, 4.f), FLinearColor(.95f, .89f, .72f), false);
        Box(FVector(300.f, Y, 347.f), FVector(290.f, 28.f, 4.f), FLinearColor(.95f, .89f, .72f), false);
    }
    MarketVisuals::BuildStoreLighting(GetWorld(), Back);
}

void AMarketGameMode::BuildShelfItems(int32 Index)
{
    // One visible unit per stocked item (shelf capacity 24). Studio products use their real
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
    const FRotator Facing(0, -90, 0);                    // product front (+X) faces the aisle (-Y)
    const float Gap = 2;
    const int32 Columns = FMath::Clamp(FMath::FloorToInt32(230 / (Size.Y + Gap)), 1, 16);
    // Only the front row is visible: the shelf body behind the boards is solid.
    const int32 Rows = 1;
    const int32 Levels = Size.Z <= 38 ? 3 : 1;           // tall items only fit on the open top board
    const FVector P = ShelfPosition(Index);
    AActor* Holder = GetWorld()->SpawnActor<AActor>(P, FRotator::ZeroRotator);
    auto* Root = NewObject<USceneComponent>(Holder);
    Holder->SetRootComponent(Root);
    Root->RegisterComponent();
    Root->SetWorldLocation(P);
    ShelfItemStart.Add(ShelfItems.Num());
    int32 Count = 0;
    for (int32 Row = 0; Row < Rows && Count < FMarketState::ShelfCapacity; ++Row)
        for (int32 Col = 0; Col < Columns && Count < FMarketState::ShelfCapacity; ++Col)
            for (int32 Level = 0; Level < Levels && Count < FMarketState::ShelfCapacity; ++Level)
            {
                const int32 Board = Levels == 3 ? Level : 2;
                const FVector Slot(((Col - (Columns - 1) * 0.5f) * (Size.Y + Gap)), -45 + Size.X * 0.5f + Row * (Size.X + Gap), 25 + Board * 44 + 2.5f);
                const FVector BoundsOffset = Facing.RotateVector(FVector(Center.X, Center.Y, 0)) + FVector(0, 0, Bottom);
                const FVector CorrectedOffset = Facing.RotateVector(PlacementOffset);
                const FQuat FinalRotation = Facing.Quaternion() * ModelRotation.Quaternion();
                auto* Item = NewObject<UStaticMeshComponent>(Holder);
                Item->SetupAttachment(Root);
                Item->SetMobility(EComponentMobility::Movable);
                Item->SetStaticMesh(Mesh);
                Item->SetRelativeLocationAndRotation(Slot - BoundsOffset + CorrectedOffset, FinalRotation.Rotator());
                Item->SetRelativeScale3D(ItemScale);
                Item->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                Item->SetCastShadow(false);
                for (int32 MaterialSlot = 0; MaterialSlot < SlotMaterials.Num(); ++MaterialSlot)
                    if (SlotMaterials[MaterialSlot]) Item->SetMaterial(MaterialSlot, SlotMaterials[MaterialSlot]);
                Item->RegisterComponent();
                ShelfItems.Add(Item);
                ++Count;
            }
    ShelfItemCount.Add(Count);
}

void AMarketGameMode::RefreshShelfItems()
{
    for (int32 I = 0; I < ShelfItemStart.Num() && State.Stock.IsValidIndex(I); ++I)
    {
        const int32 Slots = ShelfItemCount[I];
        const int32 Shelf = State.Stock[I].Shelf;
        const int32 Visible = Slots >= FMarketState::ShelfCapacity ? Shelf : FMath::DivideAndRoundUp(Shelf * Slots, FMarketState::ShelfCapacity);
        for (int32 K = 0; K < Slots; ++K)
            if (UStaticMeshComponent* Item = ShelfItems[ShelfItemStart[I] + K]) Item->SetVisibility(K < Visible);
    }
}

FString AMarketGameMode::ProductName(int32 Index) const { return State.bRealBrands ? Products[Index].RealName : Products[Index].FictionalName; }
void AMarketGameMode::Notify(const FString& Text) { Message = Text; MessageTime = 9; }
float AMarketGameMode::RivalDiscount() const { return State.Day >= 3 && State.Day % 5 <= 3 ? .85f : 1.f; }
int32 AMarketGameMode::NearbyShelf() const
{
    const auto* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Pawn) return INDEX_NONE;
    int32 Result = INDEX_NONE; float Best = 170;
    for (int32 I = 0; I < Products.Num(); ++I)
    {
        const float Distance = FVector::Dist2D(Pawn->GetActorLocation(), ShelfPosition(I));
        if (Distance < Best) { Best = Distance; Result = I; }
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
    if (NearOffice()) return FString::Printf(TEXT("YONETIM: TAB urun | B: %d adet siparis | +/- fiyat | H kasiyer | G ikinci sube"), Products[Selected].CaseUnits);
    if (NearCounter()) return FString::Printf(TEXT("E: siradaki musterinin odemesini al  |  Bekleyen: %d"), QueueSize());
    const int32 I = NearbyShelf();
    if (I != INDEX_NONE) return FString::Printf(TEXT("E: %s rafini depodan doldur"), *ProductName(I));
    return TEXT("Raf, kasa veya yonetim masasina yaklas.");
}
void AMarketGameMode::RefreshLabels()
{
    for (int32 I = 0; I < ShelfLabels.Num(); ++I)
    {
        const auto& S = State.Stock[I];
        ShelfLabels[I]->SetText(FText::FromString(FString::Printf(TEXT("%s\n%s  |  Raf: %d / 24"), *AsciiFold(ProductName(I)), *Money(S.Price), S.Shelf)));
    }
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
            const int32 Count = State.Restock(I);
            Notify(Count > 0 ? FString::Printf(TEXT("%d adet rafa yerlestirildi."), Count) : TEXT("Raf dolu veya depo bos. Yonetim masasindan siparis ver."));
            RefreshLabels();
        }
    }
    else if (Action == "Brands") { State.bRealBrands = !State.bRealBrands; RefreshLabels(); }
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
        else { State.Initialize(Products); RefreshLabels(); ResetConfirmUntil = -1; Notify(TEXT("Yeni kampanya basladi. Raflari doldur ve O ile ac.")); }
    }
    else
    {
        if (!NearOffice()) { Notify(TEXT("Bu karar icin giristeki YONETIM MASASI'na yaklas.")); return; }
        if (Action == "NextProduct") Selected = (Selected + 1) % Products.Num();
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
            Pawn->SetActorLocation(FVector(-270, 210, 90));
            Command("Interact");
            if (!Require(State.Stock[0].Shelf == 24, TEXT("nearby shelf interaction"))) return;
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
        if (!bCaptureRequested && GetWorld()->GetTimeSeconds() > 8)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Screenshots/MirasMarket.png"), true, false);
            bCaptureRequested = true;
        }
        if (GetWorld()->GetTimeSeconds() > 12) FPlatformMisc::RequestExit(false);
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
        if (C.Stage == 0) Target = ShelfPosition(C.Product) + FVector(0, -105, 65);
        else if (C.Stage == 1) Target = FVector(405, 60 + QueueSize() * 62, 65);
        else Target = FVector(405, 60 + FMarketQueueRules::Rank(Customers, I) * 62, 65);
        C.Actor->SetActorLocation(FMath::VInterpConstantTo(C.Actor->GetActorLocation(), Target, DeltaTime, 180));
        if (FVector::Dist2D(C.Actor->GetActorLocation(), Target) < 5)
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
    RefreshLabels();
    if (Added.Num() + Removed.Num() > 0)
        Notify(FString::Printf(TEXT("Kampanya yuklendi. Katalog degismis: %d yeni urun (raf bos, siparis ver), %d kaldirilan urun."), Added.Num(), Removed.Num()));
    else Notify(TEXT("Kampanya yuklendi. Hazir oldugunda O ile marketi ac."));
}

void AMarketHUD::DrawHUD()
{
    Super::DrawHUD();
    auto* G = GetMarket(this);
    if (!G || !Canvas || G->Products.Num() == 0) return;
    const float Scale = FMath::Min(Canvas->SizeX / 1280.f, Canvas->SizeY / 720.f);
    const auto Text = [&](const FString& Value, float X, float Y, FLinearColor Color = FLinearColor::White, float FontScale = 1.f)
    { DrawText(Value, Color, X * Scale, Y * Scale, GEngine->GetSmallFont(), FontScale * Scale, false); };
    const auto Panel = [&](float X, float Y, float W, float H)
    { DrawRect(FLinearColor(.018f, .033f, .037f, .9f), X * Scale, Y * Scale, W * Scale, H * Scale); };
    const auto& S = G->State;
    const FLinearColor Gold(1.f, .77f, .38f);
    const FLinearColor Mint(.55f, .89f, .74f);
    Panel(16, 14, 1248, 75);
    Text(TEXT("MIRAS MARKET"), 30, 24, Gold, 1.6f);
    const int32 Minutes = 8 * 60 + static_cast<int32>(G->DayTime * 3);
    Text(FString::Printf(TEXT("Luleburgaz / 2011    GUN %d    %s    %02d:%02d    Nakit: %s"), S.Day,
        G->bOpen ? TEXT("ACIK") : TEXT("HAZIRLIK"), G->bOpen ? Minutes / 60 : 8, G->bOpen ? Minutes % 60 : 0, *Money(S.Cash)), 270, 29, FLinearColor::White, 1.2f);
    Text(FString::Printf(TEXT("Yerel musteri payi: %%%.1f    Karli gun: %d    Kasiyer: %s    Sube: %d    Rakip: %s"),
        S.MarketShare, S.ProfitableDays, S.bCashier ? TEXT("var (20 TL/gun)") : TEXT("yok"), S.bSecondStore ? 2 : 1,
        G->RivalDiscount() < 1 ? TEXT("%15 indirim kampanyasi") : TEXT("normal fiyat")), 30, 64, Mint);
    Panel(16, 104, 394, 246);
    Text(TEXT("STOK / FIYAT"), 28, 116, Gold);
    Text(TEXT("RAF"), 273, 116, Gold);
    Text(TEXT("DEPO"), 311, 116, Gold);
    Text(TEXT("YOLDA"), 356, 116, Gold);
    // Six rows fit the panel; larger catalogs scroll with the selected product (TAB).
    const int32 First = FMath::Clamp(G->Selected - 2, 0, FMath::Max(0, S.Stock.Num() - 6));
    for (int32 Row = 0; Row < 6 && First + Row < S.Stock.Num(); ++Row)
    {
        const int32 I = First + Row;
        const auto& Stock = S.Stock[I];
        Text(FString::Printf(TEXT("%s %s"), I == G->Selected ? TEXT(">") : TEXT(" "), *G->ProductName(I)), 28, 143 + Row * 30, I == G->Selected ? Gold : FLinearColor::White);
        Text(FString::Printf(TEXT("%s"), *Money(Stock.Price)), 40, 157 + Row * 30, Mint, .85f);
        Text(FString::Printf(TEXT("%2d    %3d    %3d"), Stock.Shelf, Stock.Warehouse, Stock.Incoming), 275, 148 + Row * 30);
    }
    Text(S.Stock.Num() > 6 ? FString::Printf(TEXT("%d-%d / %d urun  |  TAB ile gez"), First + 1, FMath::Min(First + 6, S.Stock.Num()), S.Stock.Num())
                            : FString(TEXT("E ile rafta dolum / Kapasite: 24")), 28, 331, Mint, .9f);
    Panel(888, 104, 376, 171);
    Text(TEXT("BUYUME HEDEFI / IKINCI SUBE"), 902, 116, Gold, 1.15f);
    Text(TEXT("950 TL + 3 karli gun + %35 yerel pay"), 902, 145);
    Text(S.bSecondStore ? TEXT("HEDEF TAMAMLANDI - isletmeye devam et!") : TEXT("Yonetim masasinda G ile yatirim yap."), 902, 168, Mint);
    Text(FString::Printf(TEXT("Bugun: %s ciro / %d satis / %d kayip"), *Money(S.Revenue), S.Served, S.Lost), 902, 199);
    Text(FString::Printf(TEXT("Kasa sirasi: %d | Gunluk sabit gider: %s"), G->QueueSize(), *Money(2200 + (S.bCashier ? 2000 : 0))), 902, 224);
    Text(TEXT("Tum tutarlar kurgusal oyun dengesi degerleridir."), 902, 250, Mint, .8f);
    if (G->NearOffice())
    {
        Panel(420, 104, 458, 100);
        Text(TEXT("SECILI URUN / TEDARIK"), 433, 117, Gold);
        Text(G->ProductName(G->Selected), 433, 140);
        const auto& Sel = G->Products[G->Selected];
        Text(FString::Printf(TEXT("Maliyet: %s | %d'li koli: %s"), *Money(Sel.Cost), Sel.CaseUnits, *Money(Sel.Cost * Sel.CaseUnits)), 433, 164, Mint);
        Text(TEXT("Teslim: ertesi sabah / H: ise alim 120 TL"), 433, 186, FLinearColor::White, .9f);
    }
    if (!G->bOpen && S.Day > 1)
    {
        Panel(420, 285, 650, 190);
        Text(FString::Printf(TEXT("GUN %d RAPORU"), S.Day - 1), 437, 299, Gold, 1.5f);
        Text(FString::Printf(TEXT("Ciro %s  -  satilan mal maliyeti %s"), *Money(S.LastRevenue), *Money(S.LastCostOfGoods)), 437, 332);
        Text(FString::Printf(TEXT("Isletme gideri %s  /  ikinci sube net katkisi %s"), *Money(S.LastOperatingCost), *Money(S.LastBranchProfit)), 437, 357);
        Text(FString::Printf(TEXT("NET SONUC: %s   |   %d satis / %d kayip musteri"), *Money(S.LastProfit), S.LastServed, S.LastLost), 437, 386, Mint, 1.15f);
        Text(TEXT("Siparisler depoda. Raflari doldur, fiyatlari belirle, O ile ac."), 437, 419);
        Text(TEXT("Nakit ile kar farklidir: stok alimi ve yatirim nakitten pesin duser."), 437, 447, Mint, .9f);
    }
    Text(TEXT("+"), 637, 357, Gold, 1.2f);
    Panel(16, 572, 1248, 132);
    Text(G->ContextHint(), 30, 584, Gold, 1.15f);
    if (G->MessageTime > 0) Text(G->Message, 30, 614, Mint);
    Text(TEXT("WASD hareket  /  Fare bakis  /  E etkilesim  /  O ac-kapat  /  F5 kaydet  /  F9 yukle  /  ESC cik"), 30, 649);
    Text(TEXT("F8 gercek / kurgu marka adlari  /  F6 yeni kampanya (iki kez)  |  v0.1 - temel geometri prototipi"), 30, 676, FLinearColor(.66f, .7f, .72f), .95f);
}

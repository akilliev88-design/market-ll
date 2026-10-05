#include "MarketArtTrial.h"
#include "MarketGame.h"
#include "MarketStoreKit.h"
#include "MarketVisuals.h"
#include "MarketWorldText.h"
#include "ProductCatalog.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/RectLightComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/RectLight.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "HAL/FileManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"
#if WITH_EDITOR
#include "ShaderCompiler.h"
#endif

namespace
{
    UMaterialInstanceDynamic* TrialMaterial(UObject* Outer, FLinearColor Color, float Roughness, float Metal = 0.f)
    {
        auto* Base = LoadObject<UMaterialInterface>(nullptr, MarketVisuals::SurfaceMasterPath);
        if (!Base) return nullptr;
        auto* M = UMaterialInstanceDynamic::Create(Base, Outer);
        M->SetVectorParameterValue(TEXT("Color"), Color);
        M->SetScalarParameterValue(TEXT("Roughness"), Roughness);
        M->SetScalarParameterValue(TEXT("Metallic"), Metal);
        M->SetScalarParameterValue(TEXT("Specular"), .35f);
        M->SetScalarParameterValue(TEXT("UseTexture"), 0.f);
        M->SetScalarParameterValue(TEXT("Wear"), 0.f);
        return M;
    }

    void Box(UWorld* W, FVector At, FVector Size, UMaterialInterface* Material)
    {
        auto* A = W->SpawnActor<AStaticMeshActor>(At, FRotator::ZeroRotator);
        auto* C = A->GetStaticMeshComponent();
        C->SetMobility(EComponentMobility::Movable);
        C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
        C->SetMaterial(0, Material);
        C->SetWorldScale3D(Size / 100.f);
        C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    void Area(UWorld* W, FVector At, FRotator Rotation, float Lumens, float Width, float Height, float Kelvin)
    {
        auto* A = W->SpawnActor<ARectLight>(At, Rotation);
        auto* C = Cast<URectLightComponent>(A->GetLightComponent());
        C->SetMobility(EComponentMobility::Movable);
        C->SetIntensityUnits(ELightUnits::Lumens);
        C->SetIntensity(Lumens);
        C->SetSourceWidth(Width);
        C->SetSourceHeight(Height);
        C->SetAttenuationRadius(1200);
        C->SetUseTemperature(true);
        C->SetTemperature(Kelvin);
        C->SetCastShadows(true);
    }
}

AMarketArtTrialGameMode::AMarketArtTrialGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    DefaultPawnClass = AMarketCharacter::StaticClass();
    HUDClass = nullptr;
}

void AMarketArtTrialGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    GetWorld()->SpawnActor<APlayerStart>(FVector(0, -370, 90), FRotator(0, 90, 0));
}

void AMarketArtTrialGameMode::BeginPlay()
{
    Super::BeginPlay();
    bCapture = FParse::Param(FCommandLine::Get(), TEXT("ArtTrialCapture"));
    bHandmade = FParse::Param(FCommandLine::Get(), TEXT("HandmadeNeighborhood"));
    StartedAt = FPlatformTime::Seconds();
    if (GEngine && GEngine->GameViewport) GEngine->GameViewport->ConsoleCommand(TEXT("r.SetRes 1600x900w"));
    OutputDirectory = FPaths::ProjectSavedDir() / TEXT("Screenshots/ArtTrial");
    if (bHandmade) OutputDirectory = FPaths::ProjectSavedDir() / TEXT("Screenshots/HandmadeNeighborhood");
    IFileManager::Get().MakeDirectory(*OutputDirectory, true);
    TArray<FMarketProduct> Products;
    TArray<FString> Errors;
    if (!MarketCatalog::LoadFile(MarketCatalog::DefaultPath(), Products, Errors) || !MarketVisuals::HasSurfaceLibrary())
    {
        UE_LOG(LogTemp, Error, TEXT("ART_TRIAL_FAILED: catalogue or surface library missing"));
        FPlatformMisc::RequestExitWithStatus(false, 1);
        return;
    }
    // Only existing studio packages with material overrides: no blank substitute boxes.
    Products.RemoveAll([](const FMarketProduct& P) { return P.MeshPath.IsEmpty() || P.Materials.IsEmpty(); });
    for (auto& P : Products) P.bActive = true;
    if (Products.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("ART_TRIAL_FAILED: no dressed products"));
        FPlatformMisc::RequestExitWithStatus(false, 1);
        return;
    }
    FStoreTemplate S;
    S.Id = TEXT("art_trial"); S.Format = TEXT("mahalle"); S.Theme = TEXT("aydinlik");
    S.bEditableShell = true; S.FootprintCm = FVector2D(800, 1000); S.CeilingCm = 310;
    S.FloorColor = FLinearColor(.52f, .54f, .55f); S.FloorFinish = TEXT("polished");
    S.Entrance.At = FVector(0, -500, 0); S.Receiving.At = FVector(330, 500, 0);
    S.PlayerStart.At = FVector(0, -370, 90);
    S.Backroom = FBox(FVector(270, 350, 0), FVector(400, 500, 310));
    S.bHasDepotDoor = false;
    if (bHandmade)
    {
        S.bEditableShell = false;
        S.Shell = TEXT("/Game/Stores/Handmade/Neighborhood/SM_HandmadeNeighborhood.SM_HandmadeNeighborhood");
    }
    auto Category = [&](const TCHAR* Id) { const auto* P = MarketCatalog::FindProduct(Products, Id); return P ? P->Category : FString(); };
    auto Fixture = [&](const TCHAR* Id, const TCHAR* Equipment, FVector At, float Yaw, const FString& C)
    {
        FPlanogramFixture F; F.Id = Id; F.EquipmentId = Equipment; F.Location = At; F.Yaw = Yaw; F.Category = C; S.Fixtures.Add(F);
    };
    Fixture(TEXT("tea"), TEXT("wall_shelf_2400"), FVector(-225, -100, 0), 90, Category(TEXT("caykur_rize_turist_500g")));
    Fixture(TEXT("pasta"), TEXT("wall_shelf_2400"), FVector(-225, 150, 0), 90, Category(TEXT("barilla_spagetti_500g")));
    Fixture(TEXT("drinks_a"), TEXT("gondola_double_1200"), FVector(200, -95, 0), 90, Category(TEXT("coca_cola_1l")));
    Fixture(TEXT("drinks_b"), TEXT("gondola_double_1200"), FVector(200, 30, 0), 90, Category(TEXT("coca_cola_1l")));
    Fixture(TEXT("drinks_c"), TEXT("gondola_double_1200"), FVector(200, 155, 0), 90, Category(TEXT("coca_cola_1l")));
    Fixture(TEXT("cold"), TEXT("drink_cooler_3door_2100"), FVector(-60, 400, 0), 0, Category(TEXT("sutas_sut_1l")));
    Fixture(TEXT("checkout"), TEXT("checkout_single"), FVector(265, -330, 0), 0, FString());
    FMarketPlanogram Plan = MarketStoreKit::ToPlanogram(S);
    // Visual dressing only: contiguous blocks on every usable level, respecting package sizes.
    // The commercial opening-stock planner intentionally leaves gaps; this trial creates no stock.
    for (const auto& F : Plan.Fixtures)
    {
        const auto E = MarketPlanogram::Equipment(F.EquipmentId);
        for (int32 Side = 0; Side < (E.bDoubleSided ? 2 : 1); ++Side)
        {
            const FString Face = Side == 0 ? TEXT("front") : TEXT("back");
            TArray<const FMarketProduct*> Assortment;
            for (const auto& Item : Products) if (Item.Category == F.CategoryForFace(Face)) Assortment.Add(&Item);
            if (Assortment.IsEmpty()) continue;
            for (int32 Level = 0; Level < E.Levels; ++Level)
            {
                float Cursor = -E.UsableWidthCm * .5f + 1.f;
                int32 Pick = Level + Side;
                for (int32 Attempt = 0; Attempt < 60 && Cursor < E.UsableWidthCm * .5f - 2.f; ++Attempt)
                {
                    const auto& Item = *Assortment[Pick++ % Assortment.Num()];
                    if (MarketPlanogram::NominalHeightCm(Item) > E.LevelClearanceCm[Level] && E.LevelClearanceCm[Level] > 0) continue;
                    const float Width = MarketPlanogram::NominalWidthCm(Item);
                    const float Available = E.UsableWidthCm * .5f - Cursor - 1.f;
                    const int32 Facings = FMath::Min(3, FMath::FloorToInt(Available / (Width + MarketPlanogram::ItemGapCm)));
                    if (Facings < 1) continue;
                    const float BlockWidth = Facings * Width + (Facings - 1) * MarketPlanogram::ItemGapCm;
                    FPlanogramPlacement B; B.ProductId = Item.Id; B.FixtureId = F.Id; B.Face = Face; B.Level = Level;
                    B.Facings = Facings; B.Depth = FMath::Clamp(FMath::FloorToInt(E.UsableDepthCm / (MarketPlanogram::NominalDepthCm(Item) + 2.f)), 1, 4);
                    B.bHasX = true; B.XCm = Cursor + BlockWidth * .5f;
                    Plan.Placements.Add(B); Cursor += BlockWidth + .7f;
                }
            }
        }
    }
    if (!MarketStoreKit::Build(GetWorld(), S, Plan, true))
    {
        UE_LOG(LogTemp, Error, TEXT("ART_TRIAL_FAILED: store build"));
        FPlatformMisc::RequestExitWithStatus(false, 1); return;
    }
    auto* White = TrialMaterial(this, FLinearColor(.72f, .75f, .76f), .48f);
    auto* Dark = TrialMaterial(this, FLinearColor(.035f, .05f, .065f), .42f, .35f);
    auto* Rail = TrialMaterial(this, FLinearColor(.08f, .17f, .28f), .5f);
    auto* Steel = TrialMaterial(this, FLinearColor(.45f, .48f, .50f), .32f, .85f);
    auto* Floor = TrialMaterial(this, FLinearColor(.82f, .85f, .87f), .48f);
    auto* Green = TrialMaterial(this, FLinearColor(.035f, .14f, .095f), .52f);
    auto* Concrete = TrialMaterial(this, FLinearColor(.38f, .40f, .39f), .85f);
    auto* Road = TrialMaterial(this, FLinearColor(.07f, .085f, .09f), .95f);
    if (bHandmade) Box(GetWorld(), FVector(0,0,-35), FVector(30000,30000,10), Road);
    if (auto* Texture = LoadObject<UTexture>(nullptr, TEXT("/Game/Materials/Miras/Textures/T_Floor_Terrazzo.T_Floor_Terrazzo")))
    {
        Floor->SetTextureParameterValue(TEXT("BaseTex"), Texture);
        Floor->SetScalarParameterValue(TEXT("UseTexture"), 1.f);
        Floor->SetScalarParameterValue(TEXT("TileCm"), 120.f);
    }
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        if (!It->ActorHasTag(TEXT("MirasStoreKit"))) continue;
        TArray<UStaticMeshComponent*> Components; It->GetComponents(Components);
        for (auto* C : Components)
        {
            UStaticMesh* Mesh = C->GetStaticMesh(); if (!Mesh) continue;
            // Product artwork retains its own materials and canonical studio UV mapping.
            if (Mesh->GetPathName().Contains(TEXT("/Products/"))) continue;
            if (bHandmade && Mesh->GetName() == TEXT("SM_HandmadeNeighborhood"))
            {
                for (int32 I = 0; I < Mesh->GetStaticMaterials().Num(); ++I)
                {
                    const FString N = Mesh->GetStaticMaterials()[I].MaterialSlotName.ToString();
                    UMaterialInterface* M = White;
                    if (N.Contains(TEXT("Floor"))) M = Floor;
                    else if (N.Contains(TEXT("Wood"))) M = MarketVisuals::CreateSurface(this, EMarketSurface::WoodLight);
                    else if (N.Contains(TEXT("Metal"))) M = Dark;
                    else if (N.Contains(TEXT("Glass"))) M = MarketVisuals::CreateSurface(this, EMarketSurface::Acrylic);
                    else if (N.Contains(TEXT("Green"))) M = Green;
                    else if (N.Contains(TEXT("Concrete"))) M = Concrete;
                    else if (N.Contains(TEXT("Road"))) M = Road;
                    else if (N.Contains(TEXT("Light"))) M = MarketVisuals::CreateSurface(this, EMarketSurface::Emissive);
                    C->SetMaterial(I, M);
                }
                continue;
            }
            if (auto* Instances = Cast<UInstancedStaticMeshComponent>(C))
            {
                FTransform First;
                if (Instances->GetInstanceTransform(0, First, true) && First.GetLocation().Z < 0 && First.GetScale3D().X > 2.f)
                    C->SetMaterial(0, Floor);
                else if (First.GetLocation().Z > 150 && First.GetScale3D().Y < .1f && First.GetScale3D().Z < 1.f)
                    C->SetMaterial(0, Dark);
            }
            for (int32 I = 0; I < Mesh->GetStaticMaterials().Num(); ++I)
            {
                const FString N = Mesh->GetStaticMaterials()[I].MaterialSlotName.ToString().ToLower();
                if (N.Contains(TEXT("pricerail"))) C->SetMaterial(I, Rail);
                else if (N.Contains(TEXT("wood")) || N.Contains(TEXT("painted")) || N.Contains(TEXT("porcelain")) || N.Contains(TEXT("back"))) C->SetMaterial(I, White);
                else if (N.Contains(TEXT("darkmetal")) || N.Contains(TEXT("anthracite"))) C->SetMaterial(I, Dark);
                else if (N.Contains(TEXT("brushedsteel"))) C->SetMaterial(I, Steel);
            }
        }
    }
    // Read the same equipment dimensions as the store builder; no duplicated package UVs.
    for (const auto& B : Plan.Placements)
    {
        const auto* F = Plan.FindFixture(B.FixtureId);
        const auto* Item = MarketCatalog::FindProduct(Products, B.ProductId);
        if (!F || !Item || B.Face != TEXT("front")) continue;
        const auto E = MarketPlanogram::Equipment(F->EquipmentId);
        const float X = MarketPlanogram::PlacementCenterX(Plan, Products, B);
        const FTransform Xf(FRotator(0, F->Yaw, 0), F->Location);
        const FVector Local(X, -E.RailFrontY[B.Level] - .7f, E.LevelTopZ[B.Level] + E.RailAboveTopZ);
        const FVector At = Xf.TransformPosition(Local);
        auto* Board = GetWorld()->SpawnActor<AStaticMeshActor>(At, FRotator(0, F->Yaw, 0));
        auto* C = Board->GetStaticMeshComponent(); C->SetMobility(EComponentMobility::Movable);
        C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
        C->SetWorldScale3D(FVector(.12f, .004f, .042f)); C->SetMaterial(0, White); C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        auto* Text = NewObject<UTextRenderComponent>(Board); Text->SetupAttachment(C); Text->RegisterComponent();
        Text->SetAbsolute(true, true, true); Text->SetWorldScale3D(FVector::OneVector);
        Text->SetWorldLocationAndRotation(Xf.TransformPosition(Local + FVector(0, -.3f, 0)), FRotator(0, F->Yaw - 90, 0));
        MarketWorldText::Apply(Text); Text->SetTextRenderColor(FColor(20, 30, 40));
        Text->SetText(FText::FromString(MarketCatalog::Money(Item->BasePrice)));
        Text->SetWorldSize(3.f); Text->SetHorizontalAlignment(EHTA_Center); Text->SetVerticalAlignment(EVRTA_TextCenter);
    }
    // Replace the general kit fills with restrained, shadowed physical area lights.
    for (TActorIterator<ARectLight> It(GetWorld()); It; ++It) if (It->ActorHasTag(TEXT("MirasStoreKit"))) It->Destroy();
    for (float Y : {-240.f, 70.f, 340.f})
    {
        Area(GetWorld(), FVector(0, Y, 286), FRotator(-90, 0, 0), 7000, 220, 35, 5000);
        Box(GetWorld(), FVector(0, Y, 299), FVector(220, 14, 4), White);
        Box(GetWorld(), FVector(0, Y, 296), FVector(213, 9, 1), MarketVisuals::CreateSurface(this, EMarketSurface::Emissive));
    }
    Area(GetWorld(), FVector(-365, -180, 175), FRotator(0, 0, 0), 9000, 220, 160, 6500);
    Area(GetWorld(), FVector(-60, 340, 185), FRotator(-25, -90, 0), 1200, 205, 25, 5500);
    if (bHandmade)
    {
        Area(GetWorld(), FVector(0, -720, 320), FRotator(-20, 90, 0), 18000, 750, 220, 6500);
        auto* Sun = GetWorld()->SpawnActor<ADirectionalLight>(FVector(0,0,800), FRotator(-48,32,0));
        auto* L = Cast<UDirectionalLightComponent>(Sun->GetLightComponent());
        L->SetMobility(EComponentMobility::Movable); L->SetIntensity(1.6f);
        L->SetAtmosphereSunLight(true);
        auto* Atmosphere = GetWorld()->SpawnActor<ASkyAtmosphere>();
        // The trial uses interior exposure and a restrained sun; balance the visible sky separately.
        Atmosphere->GetComponent()->SetSkyLuminanceFactor(FLinearColor(2000,2000,2000));
        auto* Sky = GetWorld()->SpawnActor<ASkyLight>();
        Sky->GetLightComponent()->SetMobility(EComponentMobility::Movable);
        Sky->GetLightComponent()->SetRealTimeCaptureEnabled(true);
        Sky->GetLightComponent()->SetIntensity(.4f);
    }
    // The older art trial retains its small window dressing.
    if (!bHandmade) {
    auto* Glass = MarketVisuals::CreateSurface(this, EMarketSurface::Acrylic);
    if (Glass) Box(GetWorld(), FVector(-388, -180, 170), FVector(2, 250, 160), Glass);
    for (float Y : {-305.f, -180.f, -55.f}) Box(GetWorld(), FVector(-385, Y, 170), FVector(6, 5, 170), Dark);
    for (float Z : {85.f, 255.f}) Box(GetWorld(), FVector(-385, -180, Z), FVector(6, 255, 5), Dark);
    }
    auto* PP = GetWorld()->SpawnActor<APostProcessVolume>();
    PP->bUnbound = true; PP->Priority = 50; auto& P = PP->Settings;
    P.bOverride_AutoExposureMinBrightness = true; P.AutoExposureMinBrightness = 6.5f;
    P.bOverride_AutoExposureMaxBrightness = true; P.AutoExposureMaxBrightness = 6.5f;
    P.bOverride_AutoExposureBias = true; P.AutoExposureBias = 0.f;
    P.bOverride_BloomIntensity = true; P.BloomIntensity = .08f;
    P.bOverride_VignetteIntensity = true; P.VignetteIntensity = 0.f;
    P.bOverride_FilmGrainIntensity = true; P.FilmGrainIntensity = 0.f;
    P.bOverride_SceneFringeIntensity = true; P.SceneFringeIntensity = 0.f;
    People = MarketPeople::Load();
    for (const auto& C : People.Classes) PeopleAssets.Add(C.Get());
    if (People.Walk.IsValid()) PeopleAssets.Add(People.Walk.Get());
    if (People.Idle.IsValid()) PeopleAssets.Add(People.Idle.Get());
    ReviewPerson = MarketPeople::Spawn(GetWorld(), People, FVector(-65, 200, 0), 42, Shopper);
    if (!ReviewPerson)
    {
        UE_LOG(LogTemp, Error, TEXT("ART_TRIAL_FAILED: assembled human missing"));
        FPlatformMisc::RequestExitWithStatus(false, 1); return;
    }
    ReviewPerson->SetActorRotation(FRotator(0, 180, 0));
    if (bHandmade) ReviewPerson->SetActorHiddenInGame(true);
    TArray<USkeletalMeshComponent*> Garments; ReviewPerson->GetComponents(Garments);
    for (auto* Garment : Garments) for (int32 I = 0; I < Garment->GetNumMaterials(); ++I)
    {
        UMaterialInterface* Material = Garment->GetMaterial(I); if (!Material) continue;
        const FString Name = Material->GetName().ToLower();
        if (!Name.Contains(TEXT("garment"))) continue;
        // Keep the skeletal garment's authored material, normal map and skinning usage flags.
        // Parameter names were read from the existing asset with inspect_art_cloth.py.
        auto* Tint = UMaterialInstanceDynamic::Create(Material, ReviewPerson);
        const FLinearColor Color = Name.Contains(TEXT("shirt")) ? FLinearColor(.055f, .13f, .22f) : FLinearColor(.04f, .045f, .055f);
        for (const TCHAR* Parameter : {TEXT("C_color"), TEXT("diffuse_color_2"), TEXT("diffuse_color_1"), TEXT("B_diffuse_color_1")}) Tint->SetVectorParameterValue(Parameter, Color);
        Garment->SetMaterial(I, Tint);
    }
    ReviewCamera = GetWorld()->SpawnActor<ACameraActor>();
    ReviewCamera->GetCameraComponent()->SetFieldOfView(72.f);
    SetReviewView(0);
    if (bHandmade && !bCapture) if (auto* PC = GetWorld()->GetFirstPlayerController()) PC->SetViewTarget(PC->GetPawn());
    UE_LOG(LogTemp, Display, TEXT("ART_TRIAL_READY: %d dressed products, %d blocks, human %s"), Products.Num(), Plan.Placements.Num(), *ReviewPerson->GetClass()->GetName());
    bReady = true;
}

void AMarketArtTrialGameMode::SetReviewView(int32 Index)
{
    View = FMath::Clamp(Index, 0, 2);
    const FVector Eyes[] = {bHandmade ? FVector(-620,-1060,230) : FVector(0, -360, 165), bHandmade ? FVector(5,-370,165) : FVector(5, -70, 148), bHandmade ? FVector(0,120,165) : FVector(65, 270, 163)};
    const FVector Targets[] = {bHandmade ? FVector(0,-380,160) : FVector(-20, 180, 125), bHandmade ? FVector(-80,100,130) : FVector(-180, 80, 105), bHandmade ? FVector(0,-680,150) : FVector(-65, 200, 143)};
    ReviewCamera->SetActorLocationAndRotation(Eyes[View], (Targets[View] - Eyes[View]).Rotation());
    ReviewCamera->GetCameraComponent()->SetFieldOfView(View == 2 ? 58.f : 72.f);
    if (auto* PC = GetWorld()->GetFirstPlayerController()) PC->SetViewTarget(ReviewCamera);
    FrameTimes.Reset();
}

void AMarketArtTrialGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bReady) return;
    MarketPeople::Update(ReviewPerson, People, Shopper, FVector::ZeroVector, false, 0.f, DeltaSeconds);
    const double Now = FPlatformTime::Seconds();
    if (auto* PC = GetWorld()->GetFirstPlayerController())
    {
        if (PC->WasInputKeyJustPressed(EKeys::Escape)) FPlatformMisc::RequestExit(false);
        if (!bCapture)
        {
            if (PC->WasInputKeyJustPressed(EKeys::One)) SetReviewView(0);
            if (PC->WasInputKeyJustPressed(EKeys::Two)) SetReviewView(1);
            if (PC->WasInputKeyJustPressed(EKeys::Three)) SetReviewView(2);
            if (PC->WasInputKeyJustPressed(EKeys::Zero)) if (APawn* Pawn = PC->GetPawn()) PC->SetViewTarget(Pawn);
        }
    }
    if (!bCapture) return;
    if (bHandmade && !bWalkChecked && Now - StartedAt > 5)
    {
        auto* Walker = Cast<ACharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
        FCollisionQueryParams Q; if (Walker) Q.AddIgnoredActor(Walker);
        FHitResult Hit;
        const FCollisionShape Capsule = FCollisionShape::MakeCapsule(30,88);
        const bool BlockedEntry = GetWorld()->SweepSingleByChannel(Hit,FVector(0,-600,91),FVector(0,-400,91),FQuat::Identity,ECC_Pawn,Capsule,Q);
        const bool BlockedGlass = GetWorld()->SweepSingleByChannel(Hit,FVector(250,-600,91),FVector(250,-400,91),FQuat::Identity,ECC_Pawn,Capsule,Q);
        const bool Floor = Walker && Walker->GetCharacterMovement()->IsMovingOnGround();
        UE_LOG(LogTemp, Display, TEXT("HANDMADE_WALK_CHECK: grounded=%d entry_clear=%d glass_solid=%d"), Floor,!BlockedEntry,BlockedGlass);
        if (!Floor || BlockedEntry || !BlockedGlass) { UE_LOG(LogTemp, Error, TEXT("ART_TRIAL_FAILED: handmade walk validation")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
        bWalkChecked = true;
    }
    if (Now - StartedAt > 300)
    {
        UE_LOG(LogTemp, Error, TEXT("ART_TRIAL_FAILED: capture timeout")); FPlatformMisc::RequestExitWithStatus(false, 1); return;
    }
    bool ShadersReady = true;
#if WITH_EDITOR
    if (GShaderCompilingManager) ShadersReady = GShaderCompilingManager->GetNumRemainingJobs() == 0;
#endif
    if (!ShadersReady) { ReadyAt = -1; return; }
    if (ReadyAt < 0) ReadyAt = Now;
    if (Stage == 0 && Now - ReadyAt > 15) { Stage = 1; StageAt = Now; LastFrameAt = Now; FrameTimes.Reset(); }
    else if (Stage == 1)
    {
        FrameTimes.Add(static_cast<float>(Now - LastFrameAt)); LastFrameAt = Now;
        if (Now - StageAt < 8) return;
        FrameTimes.Sort();
        const float Median = FrameTimes[FrameTimes.Num() / 2];
        const FIntPoint Pixels = GEngine && GEngine->GameViewport ? GEngine->GameViewport->Viewport->GetSizeXY() : FIntPoint::ZeroValue;
        UE_LOG(LogTemp, Display, TEXT("ART_TRIAL_FRAME view=%d resolution=%dx%d median_ms=%.2f p95_ms=%.2f frames=%d (wall-clock game frame, not GPU-only)"), View + 1, Pixels.X, Pixels.Y, Median * 1000, FrameTimes[FMath::Min(FrameTimes.Num()-1, FMath::FloorToInt(FrameTimes.Num()*.95f))] * 1000, FrameTimes.Num());
        FScreenshotRequest::RequestScreenshot(OutputDirectory / FString::Printf(TEXT("%02d.png"), View + 1), false, false);
        Stage = 2; StageAt = Now;
    }
    else if (Stage == 2 && Now - StageAt > 3)
    {
        const FString Shot = OutputDirectory / FString::Printf(TEXT("%02d.png"), View + 1);
        if (IFileManager::Get().FileSize(*Shot) <= 0)
        {
            UE_LOG(LogTemp, Error, TEXT("ART_TRIAL_FAILED: screenshot missing %s"), *Shot);
            FPlatformMisc::RequestExitWithStatus(false, 1); return;
        }
        if (View < 2) { SetReviewView(View + 1); Stage = 0; ReadyAt = Now; }
        else { UE_LOG(LogTemp, Display, TEXT("ART_TRIAL_PASSED: three rendered views")); FPlatformMisc::RequestExitWithStatus(false, 0); }
    }
}

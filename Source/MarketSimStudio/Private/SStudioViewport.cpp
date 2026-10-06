#include "SStudioViewport.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/Package.h"

FStudioViewportClient::FStudioViewportClient(const TSharedRef<FPreviewScene>& InScene, const TSharedRef<SEditorViewport>& InWidget)
    : FEditorViewportClient(nullptr, &InScene.Get(), InWidget)
    , SceneRef(InScene)
{
    SetRealtime(true);
    DrawHelper.bDrawGrid = false;
    DrawHelper.bDrawPivot = false;
    EngineShowFlags.SetGrid(false);
    ViewFOV = 30.f;
    SetViewModes(VMI_Lit, VMI_Lit);
}

void FStudioViewportClient::Tick(float DeltaSeconds)
{
    FEditorViewportClient::Tick(DeltaSeconds);
    if (OnTickScene) OnTickScene(DeltaSeconds);
    if (UWorld* World = SceneRef->GetWorld()) World->Tick(LEVELTICK_All, DeltaSeconds);
}

FLinearColor FStudioViewportClient::GetBackgroundColor() const
{
    return FLinearColor(0.009f, 0.010f, 0.012f);
}

void SStudioViewport::Construct(const FArguments& InArgs)
{
    // Key light comes from the camera side (front-left, above): the product front is +X, left is +Y.
    Scene = MakeShared<FPreviewScene>(FPreviewScene::ConstructionValues().SetLightBrightness(4.f).SetSkyBrightness(1.5f).SetLightRotation(FRotator(-37.f, -143.f, 0.f)));
    // Fill light from the opposite side so back/right faces never go black while spinning.
    UDirectionalLightComponent* Fill = NewObject<UDirectionalLightComponent>(GetTransientPackage());
    Fill->SetIntensity(1.8f);
    Fill->SetCastShadows(false);
    // Lower priority than the scene's key light, otherwise the renderer warns that two directional
    // lights compete for forward shading / translucency.
    Fill->ForwardShadingPriority = -1;
    Scene->AddComponent(Fill, FTransform(FRotator(-25.f, 50.f, 0.f)));
    Turntable = NewObject<USceneComponent>(GetTransientPackage());
    Scene->AddComponent(Turntable, FTransform::Identity);
    Item = NewObject<UStaticMeshComponent>(GetTransientPackage());
    Item->SetupAttachment(Turntable);
    Scene->AddComponent(Item, FTransform::Identity);
    Floor = NewObject<UStaticMeshComponent>(GetTransientPackage());
    Floor->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
    if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Market.M_Market")))
    {
        UMaterialInstanceDynamic* Dark = UMaterialInstanceDynamic::Create(Base, Floor);
        Dark->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.03f, 0.033f, 0.037f));
        Floor->SetMaterial(0, Dark);
    }
    Scene->AddComponent(Floor, FTransform(FRotator::ZeroRotator, FVector(0, 0, -0.6f), FVector(0.6f, 0.6f, 0.01f)));
    SEditorViewport::Construct(SEditorViewport::FArguments());
}

SStudioViewport::~SStudioViewport()
{
    if (StudioClient.IsValid())
    {
        StudioClient->OnTickScene = nullptr;
        StudioClient->Viewport = nullptr;
    }
}

TSharedRef<FEditorViewportClient> SStudioViewport::MakeEditorViewportClient()
{
    StudioClient = MakeShared<FStudioViewportClient>(Scene.ToSharedRef(), SharedThis(this));
    StudioClient->OnTickScene = [this](float DeltaSeconds) { TickScene(DeltaSeconds); };
    ResetView();
    return StudioClient.ToSharedRef();
}

void SStudioViewport::TickScene(float DeltaSeconds)
{
    if (!bSpin || !Turntable) return;
    Yaw = FMath::Fmod(Yaw + DeltaSeconds * 18.f, 360.f);
    Turntable->SetRelativeRotation(FRotator(0.f, Yaw, 0.f));
}

void SStudioViewport::ShowItem(UStaticMesh* Mesh, const TArray<UMaterialInterface*>& Materials, const FTransform& Correction)
{
    if (!Item) return;
    Item->SetStaticMesh(Mesh);
    Item->EmptyOverrideMaterials();
    Item->SetVisibility(Mesh != nullptr);
    if (!Mesh) return;
    for (int32 Slot = 0; Slot < Materials.Num(); ++Slot)
        if (Materials[Slot]) Item->SetMaterial(Slot, Materials[Slot]);
    if (LastMesh.Get() == Mesh && LastCorrection.Equals(Correction)) return; // same package/correction: keep the user's camera
    LastMesh = Mesh;
    LastCorrection = Correction;
    const FTransform ShapeTransform(Correction.GetRotation(), FVector::ZeroVector, Correction.GetScale3D());
    const FBox Bounds = Mesh->GetBoundingBox().TransformBy(ShapeTransform);
    const FVector Center = Bounds.GetCenter();
    Item->SetRelativeRotation(Correction.Rotator());
    Item->SetRelativeScale3D(Correction.GetScale3D());
    Item->SetRelativeLocation(FVector(-Center.X, -Center.Y, -Bounds.Min.Z) + Correction.GetTranslation());
    const float Radius = FMath::Max(1.f, float(Bounds.GetExtent().Size()));
    const float FloorScale = Radius * 2.6f / 100.f;
    Floor->SetRelativeScale3D(FVector(FloorScale, FloorScale, 0.01f));
    LookAt = FVector(0, 0, Bounds.GetSize().Z * 0.5f) + Correction.GetTranslation();
    Distance = Radius / FMath::Tan(FMath::DegreesToRadians(15.f)) * 1.15f;
    Yaw = 0.f;
    Turntable->SetRelativeRotation(FRotator::ZeroRotator);
    ResetView();
}

void SStudioViewport::ResetView()
{
    if (!StudioClient.IsValid()) return;
    // Three-quarter view of the front (+X) and left (+Y) faces, slightly from above.
    const FVector Direction = FVector(1.f, 0.62f, 0.38f).GetSafeNormal();
    StudioClient->SetViewLocation(LookAt + Direction * Distance);
    StudioClient->SetViewRotation((-Direction).Rotation());
    StudioClient->SetLookAtLocation(LookAt);
    StudioClient->Invalidate();
}

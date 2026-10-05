#include "MarketGeneratedStore.h"
#include "MarketGame.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
#if WITH_EDITOR
#include "ShaderCompiler.h"
#endif
namespace {
TSharedPtr<FJsonObject> Manifest()
{
    FString Text;
    if (!FFileHelper::LoadFileToString(Text, *(FPaths::ProjectDir() / TEXT("AssetInbox/ImageBlaster/MahalleMarket/unreal-manifest.json"))))
        FFileHelper::LoadFileToString(Text, *(FPaths::ProjectSavedDir() / TEXT("ImageBlaster/unreal-manifest.json")));
    TSharedPtr<FJsonObject> Obj;
    FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Obj);
    return Obj;
}
}
AMarketGeneratedStoreGameMode::AMarketGeneratedStoreGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    DefaultPawnClass = AMarketCharacter::StaticClass();
    HUDClass = nullptr;
}
void AMarketGeneratedStoreGameMode::InitGame(const FString& Map, const FString& Options, FString& Error)
{
    Super::InitGame(Map, Options, Error);
    if (auto M = Manifest()) Origin.Z = M->GetNumberField(TEXT("floor_cm"));
    GetWorld()->SpawnActor<APlayerStart>(Origin + FVector(0, 0, 95), FRotator(0, 90, 0));
}
void AMarketGeneratedStoreGameMode::BeginPlay()
{
    Super::BeginPlay();
    auto M = Manifest();
    if (!M) { UE_LOG(LogTemp, Error, TEXT("GENERATED_STORE_FAILED: manifest missing")); FPlatformMisc::RequestExitWithStatus(false, 1); return; }
    const TArray<TSharedPtr<FJsonValue>>* Meshes;
    if (!M->TryGetArrayField(TEXT("meshes"), Meshes) || Meshes->IsEmpty()) { FPlatformMisc::RequestExitWithStatus(false, 1); return; }
    int32 Count = 0;
    for (auto Entry : *Meshes)
    {
        auto* Mesh = LoadObject<UStaticMesh>(nullptr, *Entry->AsString());
        if (!Mesh) continue;
        auto* A = GetWorld()->SpawnActor<AStaticMeshActor>();
        A->GetStaticMeshComponent()->SetStaticMesh(Mesh);
        A->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        ++Count;
    }
    if (Count != Meshes->Num()) { UE_LOG(LogTemp, Error, TEXT("GENERATED_STORE_FAILED: incomplete assets")); FPlatformMisc::RequestExitWithStatus(false, 1); return; }
    const TArray<TSharedPtr<FJsonValue>>* Collision;
    if (!M->TryGetArrayField(TEXT("collision_meshes"), Collision) || Collision->IsEmpty()) { FPlatformMisc::RequestExitWithStatus(false, 1); return; }
    for (auto Entry : *Collision)
    {
        auto* Mesh = LoadObject<UStaticMesh>(nullptr, *Entry->AsString());
        if (!Mesh) { FPlatformMisc::RequestExitWithStatus(false, 1); return; }
        auto* A = GetWorld()->SpawnActor<AStaticMeshActor>();
        auto* C = A->GetStaticMeshComponent();
        C->SetStaticMesh(Mesh);
        C->SetCollisionProfileName(TEXT("BlockAll"));
        C->SetVisibility(false);
        C->SetCastShadow(false);
    }
    auto* PP = GetWorld()->SpawnActor<APostProcessVolume>();
    PP->bUnbound = true;
    PP->Settings.bOverride_AutoExposureMethod = true;
    PP->Settings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
    PP->Settings.bOverride_AutoExposureBias = true;
    PP->Settings.AutoExposureBias = 0;
    PP->Settings.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
    PP->Settings.AutoExposureApplyPhysicalCameraExposure = false;
    PP->Settings.bOverride_BloomIntensity = true;
    PP->Settings.BloomIntensity = 0;
    Camera = GetWorld()->SpawnActor<ACameraActor>();
    Camera->GetCameraComponent()->SetFieldOfView(78);
    PhysicsOnly = FParse::Param(FCommandLine::Get(), TEXT("GeneratedStorePhysics"));
    Capture = PhysicsOnly || FParse::Param(FCommandLine::Get(), TEXT("GeneratedStoreCapture"));
    if (Capture) View(0);
    Started = FPlatformTime::Seconds();
    Ready = true;
    UE_LOG(LogTemp, Display, TEXT("GENERATED_STORE_READY: meshes=%d floor=%.1f"), Count, Origin.Z);
}
void AMarketGeneratedStoreGameMode::View(int32 Index)
{
    const FVector Positions[] = {FVector(0, 0, 165), FVector(0, 160, 165), FVector(160, 0, 165)};
    const float Angles[] = {90, 0, 180};
    Camera->SetActorLocationAndRotation(Origin + Positions[Index], FRotator(0, Angles[Index], 0));
    GetWorld()->GetFirstPlayerController()->SetViewTarget(Camera);
}
void AMarketGeneratedStoreGameMode::Tick(float Dt)
{
    Super::Tick(Dt);
    if (!Ready) return;
    auto* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;
    if (PC->WasInputKeyJustPressed(EKeys::Escape)) FPlatformMisc::RequestExit(false);
    if (!Capture) return;
#if WITH_EDITOR
    if (GShaderCompilingManager && GShaderCompilingManager->IsCompiling()) { Started = FPlatformTime::Seconds(); return; }
#endif
    if (FPlatformTime::Seconds() - Started < 5) return;
    if (Stage == 0)
    {
        auto* Walker = Cast<ACharacter>(PC->GetPawn());
        const bool Grounded = Walker && Walker->GetCharacterMovement()->IsMovingOnGround();
        UE_LOG(LogTemp, Display, TEXT("GENERATED_STORE_GROUND_CHECK: grounded=%d z=%.1f floor=%.1f"), Grounded, Walker ? Walker->GetActorLocation().Z : 0.f, Origin.Z);
        if (!Grounded) { UE_LOG(LogTemp, Error, TEXT("GENERATED_STORE_FAILED: player has no walkable floor")); FPlatformMisc::RequestExitWithStatus(false, 1); return; }
        if (PhysicsOnly) { UE_LOG(LogTemp, Display, TEXT("GENERATED_STORE_PHYSICS_PASSED")); FPlatformMisc::RequestExit(false); return; }
    }
    if (Stage < 5)
    {
        const FString Dir = FPaths::ProjectSavedDir() / TEXT("Screenshots/GeneratedStore");
        IFileManager::Get().MakeDirectory(*Dir, true);
        if (Stage % 2 == 0) FScreenshotRequest::RequestScreenshot(Dir / FString::Printf(TEXT("%02d.png"), Stage / 2 + 1), false, false);
        else View((Stage + 1) / 2);
        ++Stage;
        Started = FPlatformTime::Seconds();
    }
    else { UE_LOG(LogTemp, Display, TEXT("GENERATED_STORE_PASSED")); FPlatformMisc::RequestExit(false); }
}

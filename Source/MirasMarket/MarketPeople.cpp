#include "MarketPeople.h"

#include "Animation/AnimSequence.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Modules/ModuleManager.h"

namespace MarketPeople
{
namespace
{
    ELoadFlags QuietLoad() { return static_cast<ELoadFlags>(LOAD_NoWarn | LOAD_Quiet); }

    UAnimSequence* FirstAnim(std::initializer_list<const TCHAR*> Paths)
    {
        for (const TCHAR* Path : Paths)
            if (UAnimSequence* Anim = LoadObject<UAnimSequence>(nullptr, Path, nullptr, QuietLoad())) return Anim;
        return nullptr;
    }

    // The MetaHuman blueprint drives face and grooms from the body mesh; pick that body.
    USkeletalMeshComponent* FindBody(AActor* Actor)
    {
        TArray<USkeletalMeshComponent*> Meshes;
        Actor->GetComponents(Meshes);
        for (USkeletalMeshComponent* Mesh : Meshes)
            if (Mesh->GetName().Equals(TEXT("Body"), ESearchCase::IgnoreCase)) return Mesh;
        for (USkeletalMeshComponent* Mesh : Meshes)
            if (Mesh->GetSkeletalMeshAsset() && Mesh->GetSkeletalMeshAsset()->GetName().Contains(TEXT("Body"))) return Mesh;
        return Meshes.Num() > 0 ? Meshes[0] : nullptr;
    }

    void Play(FShopper& Shopper, UAnimSequence* Anim)
    {
        USkeletalMeshComponent* Body = Shopper.Body.Get();
        if (!Body || !Anim) return;
        Body->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        Body->PlayAnimation(Anim, true);
        Body->SetPosition(Shopper.AnimationPhase * Anim->GetPlayLength(), false);
    }
}

FLibrary Load()
{
    FLibrary Library;
    // Every assembled BP_MH_* below /Game/MetaHumans is available automatically. Adding a new
    // customer appearance no longer requires a C++ name list or another build.
    IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
    TArray<FString> PathsToScan{TEXT("/Game/MetaHumans")};
    Registry.ScanPathsSynchronous(PathsToScan, true);
    FARFilter Filter;
    Filter.PackagePaths.Add(TEXT("/Game/MetaHumans"));
    Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
    Filter.bRecursivePaths = true;
    TArray<FAssetData> Assets;
    Registry.GetAssets(Filter, Assets);
    Assets.Sort([](const FAssetData& A, const FAssetData& B) { return A.AssetName.LexicalLess(B.AssetName); });
    for (const FAssetData& Asset : Assets)
    {
        if (!Asset.AssetName.ToString().StartsWith(TEXT("BP_MH_"))) continue;
        if (UBlueprint* Blueprint = Cast<UBlueprint>(Asset.GetAsset()))
            if (Blueprint->GeneratedClass && Blueprint->GeneratedClass->IsChildOf(AActor::StaticClass()))
                Library.Classes.AddUnique(TSubclassOf<AActor>(Blueprint->GeneratedClass));
    }
    Library.Walk = FirstAnim({
        TEXT("/Game/MetaHumans/Animasyon/MF_Unarmed_Walk_Fwd.MF_Unarmed_Walk_Fwd"),
        TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd.MF_Unarmed_Walk_Fwd") });
    Library.Idle = FirstAnim({
        TEXT("/Game/MetaHumans/Animasyon/MM_Idle.MM_Idle"),
        TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle.MM_Idle") });
    UE_LOG(LogTemp, Display, TEXT("MirasMarket shoppers: %d MetaHuman(s), walk %s, idle %s."), Library.Classes.Num(),
        Library.Walk.IsValid() ? *Library.Walk->GetPathName() : TEXT("-"), Library.Idle.IsValid() ? *Library.Idle->GetPathName() : TEXT("-"));
    return Library;
}

AActor* Spawn(UWorld* World, const FLibrary& Library, const FVector& FloorLocation, int32 Seed, FShopper& OutShopper)
{
    if (!World || !Library.IsReady()) return nullptr;
    FRandomStream Pick(Seed);
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Actor = World->SpawnActor<AActor>(Library.Classes[Pick.RandRange(0, Library.Classes.Num() - 1)], FloorLocation, FRotator::ZeroRotator, Params);
    if (!Actor) return nullptr;
    Actor->SetActorEnableCollision(false);
    OutShopper = FShopper();
    OutShopper.Body = FindBody(Actor);
    OutShopper.SpeedScale = Pick.FRandRange(.88f, 1.08f);
    OutShopper.Acceleration = Pick.FRandRange(185.f, 245.f);
    OutShopper.Deceleration = Pick.FRandRange(285.f, 355.f);
    OutShopper.TurnResponse = Pick.FRandRange(5.5f, 8.5f);
    OutShopper.AnimationPhase = Pick.FRand();
    if (OutShopper.Body.IsValid()) OutShopper.MeshYaw = OutShopper.Body->GetComponentRotation().Yaw - Actor->GetActorRotation().Yaw;
    return Actor;
}

FVector MoveToward(FShopper& Shopper, const FVector& Here, const FVector& Target, float BaseSpeed,
    float DeltaTime, FVector& OutDirection, bool& bOutMoving)
{
    FVector Delta = Target - Here;
    Delta.Z = 0.f;
    const float Distance = Delta.Size();
    OutDirection = Distance > KINDA_SMALL_NUMBER ? Delta / Distance : FVector::ZeroVector;
    if (DeltaTime <= 0.f || Distance < 1.f)
    {
        Shopper.CurrentSpeed = 0.f;
        bOutMoving = false;
        return FVector(Target.X, Target.Y, Here.Z);
    }

    // Brake early enough to avoid the old glide-and-snap stop. At a sharp corner the actor first
    // shortens its step, giving the visible body time to turn before it continues down the next aisle.
    const float StopLimitedSpeed = FMath::Sqrt(FMath::Max(0.f, 2.f * Shopper.Deceleration * Distance));
    float DesiredSpeed = FMath::Min(BaseSpeed * Shopper.SpeedScale, StopLimitedSpeed);
    if (!Shopper.MoveDirection.IsNearlyZero() && !OutDirection.IsNearlyZero())
    {
        const float Alignment = FVector::DotProduct(Shopper.MoveDirection, OutDirection);
        if (Alignment < .25f) DesiredSpeed *= FMath::GetMappedRangeValueClamped(FVector2D(-1.f, .25f), FVector2D(.22f, 1.f), Alignment);
    }
    const float Rate = DesiredSpeed > Shopper.CurrentSpeed ? Shopper.Acceleration : Shopper.Deceleration;
    Shopper.CurrentSpeed = FMath::FInterpConstantTo(Shopper.CurrentSpeed, DesiredSpeed, DeltaTime, Rate);
    const float Step = FMath::Min(Distance, Shopper.CurrentSpeed * DeltaTime);
    bOutMoving = Step > .05f;
    if (bOutMoving) Shopper.MoveDirection = OutDirection;
    return Here + OutDirection * Step;
}

float WalkPlaybackRate(float WorldSpeed)
{
    constexpr float AuthoredWalkSpeed = 376.2764f / 1.5f;
    return FMath::Clamp(WorldSpeed / AuthoredWalkSpeed, .15f, 1.25f);
}

void Update(AActor* Actor, const FLibrary& Library, FShopper& Shopper, const FVector& Direction, bool bMoving, float YawOffset, float DeltaTime)
{
    if (!Actor) return;
    const bool bWalkCycle = bMoving && Shopper.CurrentSpeed >= 12.f;
    if (bMoving && !Direction.IsNearlyZero())
    {
        // Skeletal meshes face +Y in mesh space: turn the actor so the body looks along Direction,
        // whatever rotation the blueprint gives the body. YawOffset (DefaultGame.ini) is a manual fix-up.
        const FRotator Want(0.f, Direction.Rotation().Yaw - 90.f - Shopper.MeshYaw + YawOffset, 0.f);
        Actor->SetActorRotation(FMath::RInterpTo(Actor->GetActorRotation(), Want, DeltaTime, Shopper.TurnResponse));
    }
    if (!Shopper.bStarted || Shopper.bWalking != bWalkCycle)
    {
        Shopper.bStarted = true;
        Shopper.bWalking = bWalkCycle;
        Play(Shopper, bWalkCycle ? Library.Walk.Get() : Library.Idle.Get());
    }
    if (USkeletalMeshComponent* Body = Shopper.Body.Get())
    {
        // The current prototype uses an in-place walk. Matching its playback to world speed removes
        // most visible foot sliding until the shared retargeted Animation Blueprint is installed.
        Body->SetPlayRate(bWalkCycle ? WalkPlaybackRate(Shopper.CurrentSpeed) : FMath::Lerp(.94f, 1.04f, Shopper.AnimationPhase));
    }
}
}

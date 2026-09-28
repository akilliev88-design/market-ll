#include "MarketPeople.h"

#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

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
    }
}

FLibrary Load()
{
    FLibrary Library;
    for (const TCHAR* Name : { TEXT("MH_Teyze"), TEXT("MH_Amca"), TEXT("MH_Anne"), TEXT("MH_Genc") })
    {
        const FString Path = FString::Printf(TEXT("/Game/MetaHumans/%s/BP_%s.BP_%s_C"), Name, Name, Name);
        if (UClass* Class = LoadClass<AActor>(nullptr, *Path, nullptr, QuietLoad(), nullptr)) Library.Classes.Add(Class);
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
    if (OutShopper.Body.IsValid()) OutShopper.MeshYaw = OutShopper.Body->GetComponentRotation().Yaw - Actor->GetActorRotation().Yaw;
    return Actor;
}

void Update(AActor* Actor, const FLibrary& Library, FShopper& Shopper, const FVector& Direction, bool bMoving, float YawOffset, float DeltaTime)
{
    if (!Actor) return;
    if (bMoving && !Direction.IsNearlyZero())
    {
        // Skeletal meshes face +Y in mesh space: turn the actor so the body looks along Direction,
        // whatever rotation the blueprint gives the body. YawOffset (DefaultGame.ini) is a manual fix-up.
        const FRotator Want(0.f, Direction.Rotation().Yaw - 90.f - Shopper.MeshYaw + YawOffset, 0.f);
        Actor->SetActorRotation(FMath::RInterpTo(Actor->GetActorRotation(), Want, DeltaTime, 8.f));
    }
    if (!Shopper.bStarted || Shopper.bWalking != bMoving)
    {
        Shopper.bStarted = true;
        Shopper.bWalking = bMoving;
        Play(Shopper, bMoving ? Library.Walk.Get() : Library.Idle.Get());
    }
}
}

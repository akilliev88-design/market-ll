#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Templates/SubclassOf.h"
#include "UObject/WeakObjectPtrTemplates.h"

class AActor;
class UAnimSequence;
class USkeletalMeshComponent;
class UWorld;

// MetaHuman shoppers (Content/MetaHumans/MH_<Name>/BP_MH_<Name>). When no MetaHuman is assembled
// the game keeps its simple box shoppers, so the store always works.
namespace MarketPeople
{
    struct FLibrary
    {
        TArray<TSubclassOf<AActor>> Classes;
        TWeakObjectPtr<UAnimSequence> Walk;
        TWeakObjectPtr<UAnimSequence> Idle;
        bool IsReady() const { return Classes.Num() > 0; }
    };

    struct FShopper
    {
        TWeakObjectPtr<USkeletalMeshComponent> Body;
        bool bWalking = false;
        bool bStarted = false;
        float MeshYaw = 0.f; // body mesh yaw relative to the actor (read at spawn)
        // Per-person locomotion profile. Small deterministic differences keep a group from walking
        // in lockstep, while CurrentSpeed lets the animation follow the actual world movement.
        float SpeedScale = 1.f;
        float CurrentSpeed = 0.f;
        float Acceleration = 220.f;
        float Deceleration = 320.f;
        float TurnResponse = 7.f;
        float AnimationPhase = 0.f;
        FVector MoveDirection = FVector::ZeroVector;
    };

    // Finds assembled MetaHumans and walk/idle animations (retargeted copies in
    // Content/MetaHumans/Animasyon first, then the Third Person mannequin set).
    FLibrary Load();
    // Spawns a random MetaHuman standing on FloorLocation; nullptr when the library is empty.
    AActor* Spawn(UWorld* World, const FLibrary& Library, const FVector& FloorLocation, int32 Seed, FShopper& OutShopper);
    // Accelerates, brakes before the destination and slows for sharp corners. Returns a position that
    // never overshoots Target; OutDirection/OutMoving drive the animation and body orientation.
    FVector MoveToward(FShopper& Shopper, const FVector& Here, const FVector& Target, float BaseSpeed,
        float DeltaTime, FVector& OutDirection, bool& bOutMoving);
    // Faces the walking direction and switches walk / idle animation.
    void Update(AActor* Actor, const FLibrary& Library, FShopper& Shopper, const FVector& Direction, bool bMoving, float YawOffset, float DeltaTime);
}

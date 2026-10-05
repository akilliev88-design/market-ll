#include "MarketStoreWalkAudit.h"
#include "MarketStoreKit.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

bool AuditStoreSalesFaces(UWorld* World, const FStoreTemplate& Store, AActor* Walker)
{
    FCollisionQueryParams Q; if (Walker) Q.AddIgnoredActor(Walker);
    int32 Samples = 0;
    for (const auto& F : Store.Fixtures)
    {
        const auto E = MarketPlanogram::Equipment(F.EquipmentId);
        const bool Checkout = E.Family == TEXT("checkout");
        const int32 Sides = E.bDoubleSided || E.Family == TEXT("produce") ? 2 : 1;
        const FTransform T(FRotator(0, F.Yaw, 0), F.Location);
        for (int32 Side = 0; Side < Sides; ++Side)
        {
            const float Sign = Checkout || Side == 1 ? 1.f : -1.f;
            for (float Along : { -.3f, 0.f, .3f })
            {
                const FVector At = T.TransformPosition(FVector(E.DimensionsCm.X * Along,
                    Sign * (E.DimensionsCm.Y * .5f + 60.f), 91.f));
                FHitResult Hit;
                if (World->SweepSingleByChannel(Hit, At, At + FVector(0,0,.1f), FQuat::Identity,
                    ECC_Pawn, FCollisionShape::MakeCapsule(35,88), Q))
                {
                    const auto* Mesh = Cast<UStaticMeshComponent>(Hit.GetComponent());
                    UE_LOG(LogTemp, Error, TEXT("STORE_FACE_FAILED: %s %s at %s hit %s mesh %s instance %d"),
                        *Store.Id, *F.Id, *At.ToString(), *GetNameSafe(Hit.GetActor()),
                        Mesh && Mesh->GetStaticMesh() ? *Mesh->GetStaticMesh()->GetPathName() : TEXT("none"), Hit.Item);
                    return false;
                }
                ++Samples;
            }
        }
    }
    UE_LOG(LogTemp, Display, TEXT("STORE_FACES_PASSED: %s %d capsule samples"), *Store.Id, Samples);
    return true;
}

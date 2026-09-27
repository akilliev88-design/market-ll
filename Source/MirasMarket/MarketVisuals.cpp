#include "MarketVisuals.h"

#include "Components/PointLightComponent.h"
#include "Engine/PointLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace MarketVisuals
{
namespace
{
    bool IsPrintedLabel(const FString& SlotName)
    {
        const FString Normalized = SlotName.ToLower();
        return Normalized.Contains(TEXT("label")) || Normalized.Contains(TEXT("etiket"));
    }
}

UMaterialInterface* CreatePackageSurface(UObject* Outer, UMaterialInterface* Source, const FString& SlotName)
{
    if (!Outer || !Source || !IsPrintedLabel(SlotName)) return Source;

    UMaterialInstanceDynamic* Surface = UMaterialInstanceDynamic::Create(Source, Outer);
    if (!Surface) return Source;

    // Most supermarket cartons use coated paper rather than a mirror-like clear coat.
    // A high roughness and restrained specular response keep artwork readable under fixtures.
    Surface->SetScalarParameterValue(TEXT("Roughness"), 0.82f);
    Surface->SetScalarParameterValue(TEXT("Specular"), 0.20f);
    return Surface;
}

void BuildStoreLighting(UWorld* World, float BackWallY)
{
    if (!World) return;

    APostProcessVolume* Look = World->SpawnActor<APostProcessVolume>();
    if (Look)
    {
        Look->bUnbound = true;
        Look->BlendWeight = 1.f;
        Look->Priority = 10.f;
        FPostProcessSettings& Settings = Look->Settings;
        Settings.bOverride_BloomIntensity = true;
        Settings.BloomIntensity = 0.03f;
        Settings.bOverride_VignetteIntensity = true;
        Settings.VignetteIntensity = 0.10f;
        Settings.bOverride_WhiteTemp = true;
        Settings.WhiteTemp = 4850.f;
        Settings.bOverride_AutoExposureBias = true;
        Settings.AutoExposureBias = 0.35f;
        Settings.bOverride_ColorSaturation = true;
        Settings.ColorSaturation = FVector4(1.08f, 1.08f, 1.08f, 1.f);
        Settings.bOverride_ColorContrast = true;
        Settings.ColorContrast = FVector4(1.06f, 1.06f, 1.06f, 1.f);
    }

    for (float Y = 40.f; Y < BackWallY; Y += 430.f)
    {
        for (const float X : { -300.f, 300.f })
        {
            APointLight* Light = World->SpawnActor<APointLight>(FVector(X, Y, 320.f), FRotator::ZeroRotator);
            if (!Light) continue;
            Light->SetMobility(EComponentMobility::Movable);
            Light->PointLightComponent->SetIntensity(2600.f);
            Light->PointLightComponent->SetAttenuationRadius(720.f);
            Light->PointLightComponent->SetSourceRadius(135.f);
            Light->PointLightComponent->SetSoftSourceRadius(210.f);
            Light->PointLightComponent->SetUseTemperature(true);
            Light->PointLightComponent->SetTemperature(4850.f);
            Light->PointLightComponent->SetCastShadows(false);
        }
    }

    // Retail interiors use reflected aisle light as well as ceiling fixtures. These soft,
    // low-level fills face the merchandise and prevent colorful packaging from collapsing
    // into black silhouettes while preserving the darker open-ceiling look.
    for (float Y = 180.f; Y < BackWallY; Y += 320.f)
    {
        APointLight* Fill = World->SpawnActor<APointLight>(FVector(0.f, Y, 165.f), FRotator::ZeroRotator);
        if (!Fill) continue;
        Fill->SetMobility(EComponentMobility::Movable);
        Fill->PointLightComponent->SetIntensity(1250.f);
        Fill->PointLightComponent->SetAttenuationRadius(520.f);
        Fill->PointLightComponent->SetSourceRadius(150.f);
        Fill->PointLightComponent->SetSoftSourceRadius(230.f);
        Fill->PointLightComponent->SetUseTemperature(true);
        Fill->PointLightComponent->SetTemperature(4650.f);
        Fill->PointLightComponent->SetCastShadows(false);
    }
}
}

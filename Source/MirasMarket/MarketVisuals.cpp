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
        Settings.BloomIntensity = 0.08f;
        Settings.bOverride_VignetteIntensity = true;
        Settings.VignetteIntensity = 0.10f;
        Settings.bOverride_WhiteTemp = true;
        Settings.WhiteTemp = 5050.f;
        Settings.bOverride_ColorSaturation = true;
        Settings.ColorSaturation = FVector4(1.03f, 1.03f, 1.03f, 1.f);
        Settings.bOverride_ColorContrast = true;
        Settings.ColorContrast = FVector4(1.04f, 1.04f, 1.04f, 1.f);
    }

    for (float Y = 40.f; Y < BackWallY; Y += 430.f)
    {
        for (const float X : { -300.f, 300.f })
        {
            APointLight* Light = World->SpawnActor<APointLight>(FVector(X, Y, 320.f), FRotator::ZeroRotator);
            if (!Light) continue;
            Light->SetMobility(EComponentMobility::Movable);
            Light->PointLightComponent->SetIntensity(2400.f);
            Light->PointLightComponent->SetAttenuationRadius(720.f);
            Light->PointLightComponent->SetSourceRadius(135.f);
            Light->PointLightComponent->SetSoftSourceRadius(210.f);
            Light->PointLightComponent->SetUseTemperature(true);
            Light->PointLightComponent->SetTemperature(5050.f);
            Light->PointLightComponent->SetCastShadows(false);
        }
    }
}
}

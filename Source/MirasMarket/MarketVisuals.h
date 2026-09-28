#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtrTemplates.h"

class APointLight;
class APostProcessVolume;
class UMaterialInterface;
class UObject;
class UWorld;

// Surface kinds used by the procedural store and to re-skin imported store-kit meshes.
enum class EMarketSurface : uint8
{
    Floor,
    Wall,
    Ceiling,
    CeilingSteel,
    Galvanized,
    Emissive,
    Wood,
    WoodLight,
    ShelfMetal,
    ShelfBack,
    DarkMetal,
    PriceRail,
    Rubber,
    Acrylic,
    SignRed,
    TagWhite,
    Cardboard,
    CounterTop,
    FoodHazelnut,
    FoodChickpea,
    FoodLentil,
    FoodPistachio,
    Count
};

namespace MarketVisuals
{
    // Master materials created by Tools/gorsel_malzemeler.py (GORSEL_HAZIRLA.cmd).
    // When they are missing every surface falls back to /Game/Materials/M_Market with a plain color.
    constexpr const TCHAR* SurfaceMasterPath = TEXT("/Game/Materials/Miras/M_MirasSurface.M_MirasSurface");
    constexpr const TCHAR* AcrylicMasterPath = TEXT("/Game/Materials/Miras/M_MirasAcrylic.M_MirasAcrylic");

    UMaterialInterface* CreatePackageSurface(UObject* Outer, UMaterialInterface* Source, const FString& SlotName);
    // New material for a surface kind (callers cache it). Never null while the engine basic material exists.
    UMaterialInterface* CreateSurface(UObject* Outer, EMarketSurface Kind);
    // True when the textured master material exists (floor grout, wood grain, food textures).
    bool HasSurfaceLibrary();
    // Maps an imported mesh material slot name (e.g. "MI_Shelf_WarmWood") to a surface kind.
    bool SurfaceForSlot(const FString& SlotName, EMarketSurface& OutKind);
    FLinearColor SurfaceColor(EMarketSurface Kind);

    // Spawned lights + post process so the look ("mood") can be switched at runtime (F4).
    struct FStoreLighting
    {
        TWeakObjectPtr<APostProcessVolume> Look;
        TArray<TWeakObjectPtr<APointLight>> Ceiling;
        TArray<TWeakObjectPtr<APointLight>> Fill;
    };
    constexpr int32 MoodCount = 3;
    // 0 = Sicak (warm, cozy, saturated), 1 = Aydinlik (bright daylight), 2 = Aksam (evening, amber).
    const TCHAR* MoodName(int32 Mood);
    FStoreLighting BuildStoreLighting(UWorld* World, float BackWallY);
    void ApplyMood(FStoreLighting& Lighting, int32 Mood);
}

#include "MarketVisuals.h"

#include "Components/PointLightComponent.h"
#include "Engine/PointLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/Texture.h"
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

    struct FSurfaceSpec
    {
        FLinearColor Color;      // tint (textured) or base color
        FLinearColor Fallback;   // color used without the textured master
        float Roughness;
        float Metallic;
        float Specular;
        float Emissive;
        const TCHAR* Texture;    // asset name under /Game/Materials/Miras/Textures, or nullptr
        float TileCm;
    };

    const FSurfaceSpec& Spec(EMarketSurface Kind)
    {
        // Colors follow the reference photo: bright polished floor, light painted shelving,
        // dark walnut on feature fixtures, red category signs and white price tags.
        static const FSurfaceSpec Specs[] =
        {
            // Warm, softly saturated palette (sand limewash walls, honey oak + walnut, terracotta signs).
            /* Floor        */ { FLinearColor(1.f, .95f, .86f),     FLinearColor(.78f, .70f, .58f), .20f, 0.f, .5f, 0.f, TEXT("T_Floor_Terrazzo"), 120.f },
            /* Wall         */ { FLinearColor(.95f, .91f, .84f),    FLinearColor(.95f, .91f, .84f), .85f, 0.f, .35f, 0.f, nullptr, 100.f },
            /* Ceiling      */ { FLinearColor(.30f, .25f, .21f),    FLinearColor(.30f, .25f, .21f), .90f, 0.f, .3f, 0.f, nullptr, 100.f },
            /* CeilingSteel */ { FLinearColor(.16f, .13f, .11f),    FLinearColor(.16f, .13f, .11f), .55f, .4f, .5f, 0.f, nullptr, 100.f },
            /* Galvanized   */ { FLinearColor(.52f, .49f, .45f),    FLinearColor(.52f, .49f, .45f), .40f, .8f, .5f, 0.f, nullptr, 100.f },
            /* Emissive     */ { FLinearColor(1.f, .93f, .82f),     FLinearColor(1.f, .93f, .82f), .30f, 0.f, .5f, 4.f, nullptr, 100.f },
            /* Wood         */ { FLinearColor(.78f, .70f, .66f),   FLinearColor(.24f, .11f, .05f), .40f, 0.f, .45f, 0.f, TEXT("T_Wood_Walnut"), 80.f },
            /* WoodLight    */ { FLinearColor(1.f, .96f, .90f),     FLinearColor(.67f, .47f, .27f), .45f, 0.f, .45f, 0.f, TEXT("T_Wood_Oak"), 90.f },
            /* ShelfMetal   */ { FLinearColor(.96f, .95f, .92f),    FLinearColor(.96f, .95f, .92f), .42f, .1f, .5f, 0.f, nullptr, 100.f },
            /* ShelfBack    */ { FLinearColor(.95f, .94f, .90f),    FLinearColor(.95f, .94f, .90f), .60f, 0.f, .4f, 0.f, nullptr, 100.f },
            /* DarkMetal    */ { FLinearColor(.05f, .04f, .035f),   FLinearColor(.05f, .04f, .035f), .40f, .4f, .5f, 0.f, nullptr, 100.f },
            /* PriceRail    */ { FLinearColor(.98f, .97f, .94f),    FLinearColor(.98f, .97f, .94f), .45f, 0.f, .5f, 0.f, nullptr, 100.f },
            /* Rubber       */ { FLinearColor(.035f, .03f, .028f),  FLinearColor(.035f, .03f, .028f), .80f, 0.f, .3f, 0.f, nullptr, 100.f },
            /* Acrylic      */ { FLinearColor(.97f, .96f, .92f),    FLinearColor(.97f, .96f, .92f), .04f, 0.f, .8f, 0.f, nullptr, 100.f },
            /* SignRed      */ { FLinearColor(.66f, .15f, .06f),    FLinearColor(.66f, .15f, .06f), .55f, 0.f, .4f, .12f, nullptr, 100.f },
            /* TagWhite     */ { FLinearColor(.97f, .95f, .90f),    FLinearColor(.97f, .95f, .90f), .60f, 0.f, .4f, 0.f, nullptr, 100.f },
            /* Cardboard    */ { FLinearColor(.62f, .44f, .26f),    FLinearColor(.62f, .44f, .26f), .90f, 0.f, .3f, 0.f, nullptr, 100.f },
            /* CounterTop   */ { FLinearColor(.93f, .88f, .79f),    FLinearColor(.93f, .88f, .79f), .35f, 0.f, .5f, 0.f, nullptr, 100.f },
            /* FoodHazelnut */ { FLinearColor(1.05f, 1.f, .95f),    FLinearColor(.45f, .25f, .10f), .55f, 0.f, .45f, 0.f, TEXT("T_Food_Hazelnut"), 14.f },
            /* FoodChickpea */ { FLinearColor(1.05f, 1.f, .95f),    FLinearColor(.78f, .62f, .36f), .70f, 0.f, .40f, 0.f, TEXT("T_Food_Chickpea"), 12.f },
            /* FoodLentil   */ { FLinearColor(1.05f, 1.f, .95f),    FLinearColor(.80f, .30f, .08f), .60f, 0.f, .40f, 0.f, TEXT("T_Food_Lentil"), 8.f },
            /* FoodPistachio*/ { FLinearColor(1.05f, 1.f, .95f),    FLinearColor(.50f, .55f, .22f), .60f, 0.f, .40f, 0.f, TEXT("T_Food_Pistachio"), 12.f },
        };
        static_assert(UE_ARRAY_COUNT(Specs) == static_cast<int32>(EMarketSurface::Count), "Surface table must match EMarketSurface");
        return Specs[FMath::Clamp(static_cast<int32>(Kind), 0, static_cast<int32>(EMarketSurface::Count) - 1)];
    }

    // How much large-scale grime / tone variation (T_Macro_Variation) a surface shows. 0 = clean.
    float WearFor(EMarketSurface Kind)
    {
        switch (Kind)
        {
        case EMarketSurface::Floor: return 0.8f;
        case EMarketSurface::Wall: return 0.55f;
        case EMarketSurface::Ceiling: case EMarketSurface::CeilingSteel: case EMarketSurface::Galvanized: return 0.6f;
        case EMarketSurface::ShelfMetal: case EMarketSurface::ShelfBack: case EMarketSurface::PriceRail: return 0.35f;
        case EMarketSurface::Wood: case EMarketSurface::WoodLight: case EMarketSurface::CounterTop: return 0.3f;
        case EMarketSurface::Cardboard: case EMarketSurface::DarkMetal: case EMarketSurface::Rubber: return 0.5f;
        case EMarketSurface::SignRed: case EMarketSurface::TagWhite: return 0.15f;
        default: return 0.f;
        }
    }

    // Free CC0 photo textures (AssetInbox/Textures/Harici -> T_Ext_<Folder>_BC / _R). Used when imported,
    // otherwise the procedural texture / flat colour above stays.
    struct FExternalSpec
    {
        const TCHAR* Folder = nullptr;
        FLinearColor Tint = FLinearColor::White;
        float TileCm = 100.f;
    };

    FExternalSpec ExternalFor(EMarketSurface Kind)
    {
        switch (Kind)
        {
        case EMarketSurface::Floor: return { TEXT("Terrazzo004"), FLinearColor(1.f, .95f, .86f), 120.f };
        case EMarketSurface::Wall: return { TEXT("beige_wall_001"), FLinearColor(1.f, .98f, .94f), 250.f };
        case EMarketSurface::Wood: return { TEXT("american_walnut_veneer"), FLinearColor(1.f, 1.f, 1.f), 120.f };
        case EMarketSurface::WoodLight: return { TEXT("ash_veneer"), FLinearColor(1.f, .96f, .90f), 120.f };
        case EMarketSurface::Cardboard: return { TEXT("Cardboard004"), FLinearColor(1.f, 1.f, 1.f), 60.f };
        default: return {};
        }
    }

    ELoadFlags QuietLoad() { return static_cast<ELoadFlags>(LOAD_NoWarn | LOAD_Quiet); }

    UTexture* LoadTexture(const FString& Name)
    {
        const FString Path = FString::Printf(TEXT("/Game/Materials/Miras/Textures/%s.%s"), *Name, *Name);
        return LoadObject<UTexture>(nullptr, *Path, nullptr, QuietLoad());
    }

    UMaterialInterface* LoadMaster(const TCHAR* Path)
    {
        return LoadObject<UMaterialInterface>(nullptr, Path, nullptr, QuietLoad());
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

bool HasSurfaceLibrary()
{
    return LoadMaster(SurfaceMasterPath) != nullptr;
}

FLinearColor SurfaceColor(EMarketSurface Kind)
{
    return Spec(Kind).Fallback;
}

UMaterialInterface* CreateSurface(UObject* Outer, EMarketSurface Kind)
{
    const FSurfaceSpec& S = Spec(Kind);
    if (Kind == EMarketSurface::Acrylic)
    {
        UMaterialInterface* Master = LoadMaster(AcrylicMasterPath);
        if (!Master) return nullptr; // caller keeps the mesh's own material
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Master, Outer);
        Material->SetVectorParameterValue(TEXT("Color"), S.Color);
        Material->SetScalarParameterValue(TEXT("Opacity"), 0.14f);
        Material->SetScalarParameterValue(TEXT("Roughness"), S.Roughness);
        return Material;
    }

    if (UMaterialInterface* Master = LoadMaster(SurfaceMasterPath))
    {
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Master, Outer);
        FLinearColor Tint = S.Color;
        float TileCm = S.TileCm;
        float Wear = WearFor(Kind);
        UTexture* Texture = nullptr;
        UTexture* Rough = nullptr;
        const FExternalSpec External = ExternalFor(Kind);
        if (External.Folder)
        {
            Texture = LoadTexture(FString::Printf(TEXT("T_Ext_%s_BC"), External.Folder));
            if (Texture)
            {
                Rough = LoadTexture(FString::Printf(TEXT("T_Ext_%s_R"), External.Folder));
                Tint = External.Tint;
                TileCm = External.TileCm;
                Wear *= 0.6f; // photo textures already carry their own detail
            }
        }
        if (!Texture && S.Texture) Texture = LoadTexture(S.Texture);
        Material->SetVectorParameterValue(TEXT("Color"), Texture ? Tint : S.Fallback);
        Material->SetScalarParameterValue(TEXT("Roughness"), S.Roughness);
        Material->SetScalarParameterValue(TEXT("Metallic"), S.Metallic);
        Material->SetScalarParameterValue(TEXT("Specular"), S.Specular);
        Material->SetScalarParameterValue(TEXT("Emissive"), S.Emissive);
        Material->SetScalarParameterValue(TEXT("UseTexture"), Texture ? 1.f : 0.f);
        Material->SetScalarParameterValue(TEXT("TileCm"), TileCm);
        Material->SetScalarParameterValue(TEXT("Wear"), Wear);
        // The polished floor keeps half of its flat low roughness so it still reflects the lights.
        Material->SetScalarParameterValue(TEXT("UseRoughTex"), Rough ? (Kind == EMarketSurface::Floor ? .5f : 1.f) : 0.f);
        if (Texture) Material->SetTextureParameterValue(TEXT("BaseTex"), Texture);
        if (Rough) Material->SetTextureParameterValue(TEXT("RoughTex"), Rough);
        return Material;
    }

    UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Market.M_Market"));
    if (!Base) Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Base) return nullptr;
    UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Outer);
    Material->SetVectorParameterValue(TEXT("Color"), S.Fallback);
    return Material;
}

bool SurfaceForSlot(const FString& SlotName, EMarketSurface& OutKind)
{
    const FString N = SlotName.ToLower();
    struct FRule { const TCHAR* Key; EMarketSurface Kind; };
    // Order matters: the most specific keys come first.
    static const FRule Rules[] =
    {
        { TEXT("bulk_amber"), EMarketSurface::FoodLentil },
        { TEXT("bulk_gold"), EMarketSurface::FoodChickpea },
        { TEXT("bulk_green"), EMarketSurface::FoodPistachio },
        { TEXT("bulk_brown"), EMarketSurface::FoodHazelnut },
        { TEXT("clear"), EMarketSurface::Acrylic },
        { TEXT("glass"), EMarketSurface::Acrylic },
        { TEXT("ceiling_light"), EMarketSurface::Emissive },
        { TEXT("blacksteel"), EMarketSurface::CeilingSteel },
        { TEXT("galvan"), EMarketSurface::Galvanized },
        { TEXT("bulkisland_wood"), EMarketSurface::Wood },   // feature island stays walnut
        { TEXT("wood"), EMarketSurface::WoodLight },          // shelf plinths/headers: honey oak
        { TEXT("pricerail"), EMarketSurface::PriceRail },
        { TEXT("rubber"), EMarketSurface::Rubber },
        { TEXT("backpanel"), EMarketSurface::ShelfBack },
        { TEXT("wallshelf_back"), EMarketSurface::ShelfBack },
        { TEXT("paintedmetal"), EMarketSurface::ShelfMetal },
        { TEXT("darkmetal"), EMarketSurface::ShelfMetal },
        { TEXT("black"), EMarketSurface::DarkMetal },
    };
    for (const FRule& Rule : Rules)
        if (N.Contains(Rule.Key)) { OutKind = Rule.Kind; return true; }
    return false;
}

const TCHAR* MoodName(int32 Mood)
{
    switch (Mood)
    {
    case 1: return TEXT("Ayd\u0131nl\u0131k");
    case 2: return TEXT("Ak\u015fam");
    default: return TEXT("S\u0131cak");
    }
}

FStoreLighting BuildStoreLighting(UWorld* World, float BackWallY)
{
    FStoreLighting Result;
    if (!World) return Result;

    if (APostProcessVolume* Look = World->SpawnActor<APostProcessVolume>())
    {
        Look->bUnbound = true;
        Look->BlendWeight = 1.f;
        Look->Priority = 10.f;
        FPostProcessSettings& Settings = Look->Settings;
        // Lumen gives bounce light from the warm floor and real reflections on the polished tiles.
        Settings.bOverride_DynamicGlobalIlluminationMethod = true;
        Settings.DynamicGlobalIlluminationMethod = EDynamicGlobalIlluminationMethod::Lumen;
        Settings.bOverride_ReflectionMethod = true;
        Settings.ReflectionMethod = EReflectionMethod::Lumen;
        // Adaptive exposure (EV100 range, see DefaultEngine.ini); the mood sets the bias.
        Settings.bOverride_AutoExposureMethod = true;
        Settings.AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;
        Settings.bOverride_AutoExposureMinBrightness = true;
        Settings.AutoExposureMinBrightness = -2.f;
        Settings.bOverride_AutoExposureMaxBrightness = true;
        Settings.AutoExposureMaxBrightness = 14.f;
        Settings.bOverride_AutoExposureSpeedUp = true;
        Settings.AutoExposureSpeedUp = 4.f;
        Settings.bOverride_AutoExposureSpeedDown = true;
        Settings.AutoExposureSpeedDown = 2.f;
        // Photographic finish: local exposure keeps bright walls and dark shelf bottoms both readable,
        // a trace of film grain and lens fringing breaks the perfectly clean CG image.
        Settings.bOverride_LocalExposureHighlightContrastScale = true;
        Settings.LocalExposureHighlightContrastScale = 0.75f;
        Settings.bOverride_LocalExposureShadowContrastScale = true;
        Settings.LocalExposureShadowContrastScale = 0.85f;
        Settings.bOverride_FilmGrainIntensity = true;
        Settings.FilmGrainIntensity = 0.06f;
        Settings.bOverride_SceneFringeIntensity = true;
        Settings.SceneFringeIntensity = 0.25f;
        Result.Look = Look;
    }

    // Ceiling fixtures: two lines of neutral-warm linear sources over the aisles.
    for (float Y = 40.f; Y < BackWallY; Y += 320.f)
    {
        for (const float X : { -300.f, 300.f })
        {
            APointLight* Light = World->SpawnActor<APointLight>(FVector(X, Y, 320.f), FRotator::ZeroRotator);
            if (!Light) continue;
            Light->SetMobility(EComponentMobility::Movable);
            Light->PointLightComponent->SetIntensityUnits(ELightUnits::Lumens);
            Light->PointLightComponent->SetAttenuationRadius(950.f);
            Light->PointLightComponent->SetSourceRadius(60.f);
            Light->PointLightComponent->SetSoftSourceRadius(180.f);
            Light->PointLightComponent->SetSourceLength(120.f);
            Light->PointLightComponent->SetUseTemperature(true);
            // Soft shadows from the ceiling rows ground shelves and products (less "model kit" look).
            Light->PointLightComponent->SetCastShadows(true);
            Result.Ceiling.Add(Light);
        }
    }

    // Low aisle fills keep package fronts readable; Lumen adds the floor bounce on top.
    for (float Y = 180.f; Y < BackWallY; Y += 320.f)
    {
        APointLight* Fill = World->SpawnActor<APointLight>(FVector(0.f, Y, 165.f), FRotator::ZeroRotator);
        if (!Fill) continue;
        Fill->SetMobility(EComponentMobility::Movable);
        Fill->PointLightComponent->SetIntensityUnits(ELightUnits::Lumens);
        Fill->PointLightComponent->SetAttenuationRadius(600.f);
        Fill->PointLightComponent->SetSourceRadius(150.f);
        Fill->PointLightComponent->SetSoftSourceRadius(240.f);
        Fill->PointLightComponent->SetUseTemperature(true);
        Fill->PointLightComponent->SetCastShadows(false);
        Result.Fill.Add(Fill);
    }
    ApplyMood(Result, 0);
    return Result;
}

void ApplyMood(FStoreLighting& Lighting, int32 Mood)
{
    struct FMood
    {
        float CeilingLumens, FillLumens, CeilingKelvin, FillKelvin, WhiteTemp, Bias;
        float Saturation, Contrast, Bloom, Vignette;
        FLinearColor Gain;
    };
    // The default is a neutral-warm supermarket look: whites stay white while packaging remains lively.
    // The evening preset keeps the deliberately amber alternative on F4.
    static const FMood Moods[MoodCount] =
    {
        /* Sicak   */ { 8500.f, 1650.f, 4600.f, 4300.f, 4700.f, 0.10f, 1.12f, 1.03f, 0.20f, 0.18f, FLinearColor(1.00f, 1.00f, 0.98f) },
        /* Aydinlik*/ { 10000.f, 1850.f, 5200.f, 5000.f, 5100.f, 0.25f, 1.08f, 1.02f, 0.12f, 0.14f, FLinearColor(1.00f, 1.00f, 1.00f) },
        /* Aksam   */ { 6000.f, 1350.f, 3400.f, 3200.f, 4800.f, 0.05f, 1.18f, 1.06f, 0.32f, 0.30f, FLinearColor(1.03f, 0.99f, 0.94f) },
    };
    const FMood& M = Moods[FMath::Clamp(Mood, 0, MoodCount - 1)];
    for (const TWeakObjectPtr<APointLight>& Light : Lighting.Ceiling)
        if (Light.IsValid())
        {
            Light->PointLightComponent->SetIntensity(M.CeilingLumens);
            Light->PointLightComponent->SetTemperature(M.CeilingKelvin);
        }
    for (const TWeakObjectPtr<APointLight>& Light : Lighting.Fill)
        if (Light.IsValid())
        {
            Light->PointLightComponent->SetIntensity(M.FillLumens);
            Light->PointLightComponent->SetTemperature(M.FillKelvin);
        }
    if (APostProcessVolume* Look = Lighting.Look.Get())
    {
        FPostProcessSettings& Settings = Look->Settings;
        Settings.bOverride_AutoExposureBias = true;
        Settings.AutoExposureBias = M.Bias;
        Settings.bOverride_WhiteTemp = true;
        Settings.WhiteTemp = M.WhiteTemp;
        Settings.bOverride_ColorSaturation = true;
        Settings.ColorSaturation = FVector4(M.Saturation, M.Saturation, M.Saturation, 1.f);
        Settings.bOverride_ColorContrast = true;
        Settings.ColorContrast = FVector4(M.Contrast, M.Contrast, M.Contrast, 1.f);
        Settings.bOverride_ColorGain = true;
        Settings.ColorGain = FVector4(M.Gain.R, M.Gain.G, M.Gain.B, 1.f);
        Settings.bOverride_BloomIntensity = true;
        Settings.BloomIntensity = M.Bloom;
        Settings.bOverride_VignetteIntensity = true;
        Settings.VignetteIntensity = M.Vignette;
    }
}
}

// Product Studio: ready package library (Config/ambalajlar.json) and part colors.
#include "StudioBackend.h"
#include "StudioBackendInternal.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "AssetToolsModule.h"
#include "AutomatedAssetImportData.h"
#include "Dom/JsonObject.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "FileHelpers.h"
#include "HAL/FileManager.h"
#include "IAssetTools.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "MeshDescription.h"
#include "Misc/DateTime.h"
#include "Misc/DefaultValueHelper.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "PhysicsEngine/BodySetup.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "StaticMeshAttributes.h"
#include "UObject/Package.h"

namespace MirasStudio
{
using namespace Internal;

// ---------------------------------------------------------------------------------------- ready packages

FString PresetsPath()
{
    return FPaths::ConvertRelativePathToFull(FPaths::ProjectConfigDir() / TEXT("ambalajlar.json"));
}

TArray<FPackagePreset> LoadPresets(TArray<FString>* OutErrors)
{
    TArray<FPackagePreset> Result;
    const TSharedPtr<FJsonObject> Root = ReadJson(PresetsPath());
    const TArray<TSharedPtr<FJsonValue>>* Items = nullptr;
    if (!Root.IsValid() || !Root->TryGetArrayField(TEXT("packages"), Items))
    {
        if (OutErrors) OutErrors->Add(TEXT("Config/ambalajlar.json okunamad\u0131."));
        return Result;
    }
    for (const TSharedPtr<FJsonValue>& Value : *Items)
    {
        const TSharedPtr<FJsonObject> O = Value.IsValid() ? Value->AsObject() : nullptr;
        if (!O.IsValid()) continue;
        FPackagePreset P;
        O->TryGetStringField(TEXT("id"), P.Id);
        O->TryGetStringField(TEXT("name"), P.Name);
        O->TryGetStringField(TEXT("type"), P.Type);
        O->TryGetStringField(TEXT("shape"), P.Shape);
        O->TryGetStringField(TEXT("parts"), P.Parts);
        O->TryGetStringField(TEXT("colors"), P.Colors);
        O->TryGetStringField(TEXT("notes"), P.Notes);
        O->TryGetNumberField(TEXT("widthMm"), P.WidthMm);
        O->TryGetNumberField(TEXT("depthMm"), P.DepthMm);
        O->TryGetNumberField(TEXT("heightMm"), P.HeightMm);
        O->TryGetNumberField(TEXT("diameterMm"), P.DiameterMm);
        O->TryGetNumberField(TEXT("labelHeightMm"), P.LabelHeightMm);
        O->TryGetNumberField(TEXT("labelBottomMm"), P.LabelBottomMm);
        O->TryGetNumberField(TEXT("neckDiameterMm"), P.NeckDiameterMm);
        O->TryGetNumberField(TEXT("capHeightMm"), P.CapHeightMm);
        O->TryGetBoolField(TEXT("topCap"), P.bTopCap);
        O->TryGetBoolField(TEXT("metalCap"), P.bMetalCap);
        const bool bSizes = P.IsBox() ? (P.WidthMm > 0 && P.DepthMm > 0 && P.HeightMm > 0) : (P.DiameterMm > 0 && P.HeightMm > 0);
        if (!MarketCatalog::IsValidId(P.Id) || P.Name.IsEmpty() || P.Type.IsEmpty() || !bSizes)
        {
            if (OutErrors) OutErrors->Add(TEXT("Hatal\u0131 ambalaj kayd\u0131 atland\u0131: ") + (P.Id.IsEmpty() ? P.Name : P.Id));
            continue;
        }
        if (!P.IsBox())
        {
            if (P.Shape.IsEmpty()) P.Shape = P.Type == TEXT("teneke") ? TEXT("teneke") : P.Type == TEXT("kavanoz") ? TEXT("kavanoz") : P.Type == TEXT("kase") ? TEXT("kase") : TEXT("sise");
            if (P.LabelHeightMm <= 0) P.LabelHeightMm = P.HeightMm / 3;
        }
        if (P.Parts.IsEmpty())
            P.Parts = P.IsBox() ? TEXT("Etiket") : P.Type == TEXT("teneke") ? TEXT("Etiket,Kapak") : P.Type == TEXT("kase") ? TEXT("Etiket,Govde,Kapak") : TEXT("Etiket,Cam,Kapak");
        if (P.bTopCap && !P.Parts.Contains(TEXT("Kapak"))) P.Parts += TEXT(",Kapak");
        Result.Add(P);
    }
    return Result;
}

const FPackagePreset* FindPreset(const TArray<FPackagePreset>& Presets, const FString& Id)
{
    return Id.IsEmpty() ? nullptr : Presets.FindByPredicate([&](const FPackagePreset& P) { return P.Id == Id; });
}

FString PresetPackageId(const FPackagePreset& Preset)
{
    return Preset.IsBox() ? FBoxPackageLayout::PackageId(Preset.WidthMm, Preset.DepthMm, Preset.HeightMm) : TEXT("shape_") + Preset.Id;
}

UStaticMesh* EnsurePresetPackage(const FPackagePreset& Preset, FString& OutPackageId, FString& OutError)
{
    if (Preset.IsBox()) return CreateBoxPackage(Preset.WidthMm, Preset.DepthMm, Preset.HeightMm, OutPackageId, OutError);
    return CreateShapePackage(Preset, OutPackageId, OutError);
}

namespace
{
    struct FColorEntry { FString Slot; FString Value; };
    TArray<FColorEntry> SplitColors(const FString& Colors)
    {
        TArray<FColorEntry> Out;
        TArray<FString> Items;
        Colors.ParseIntoArray(Items, TEXT(";"));
        for (FString Item : Items)
        {
            FString Key, Value;
            if (!Item.Split(TEXT("="), &Key, &Value)) continue;
            Key.TrimStartAndEndInline();
            Value.TrimStartAndEndInline();
            if (!Key.IsEmpty() && !Value.IsEmpty()) Out.Add({ Key, Value });
        }
        return Out;
    }
    FString JoinColors(const TArray<FColorEntry>& Entries)
    {
        TArray<FString> Items;
        for (const FColorEntry& E : Entries) Items.Add(E.Slot + TEXT("=") + E.Value);
        return FString::Join(Items, TEXT(";"));
    }
    bool IsHex6(const FString& Hex)
    {
        if (Hex.Len() != 6) return false;
        for (TCHAR C : Hex) if (!FChar::IsHexDigit(C)) return false;
        return true;
    }
}

FString ColorText(const FString& Colors, const FString& Slot)
{
    for (const FColorEntry& E : SplitColors(Colors)) if (E.Slot.Equals(Slot, ESearchCase::IgnoreCase)) return E.Value;
    return FString();
}

bool FindColor(const FString& Colors, const FString& Slot, FLinearColor& OutColor, float& OutOpacity)
{
    FString Value = ColorText(Colors, Slot), Hex, Opacity;
    if (Value.IsEmpty()) return false;
    if (!Value.Split(TEXT("/"), &Hex, &Opacity)) Hex = Value;
    Hex.RemoveFromStart(TEXT("#"));
    if (!IsHex6(Hex)) return false;
    OutColor = FLinearColor(FColor::FromHex(Hex));
    OutOpacity = Opacity.IsEmpty() ? -1.f : FMath::Clamp(FCString::Atof(*Opacity.Replace(TEXT(","), TEXT("."))), 0.02f, 1.f);
    return true;
}

FString WithColor(const FString& Colors, const FString& Slot, const FString& Value)
{
    TArray<FColorEntry> Entries = SplitColors(Colors);
    Entries.RemoveAll([&](const FColorEntry& E) { return E.Slot.Equals(Slot, ESearchCase::IgnoreCase); });
    FString Clean = Value.TrimStartAndEnd();
    Clean.RemoveFromStart(TEXT("#"));
    if (!Clean.IsEmpty()) Entries.Add({ Slot, Clean.ToUpper() });
    return JoinColors(Entries);
}

void ApplyPreset(const FPackagePreset& Preset, FStudioDraft& Draft)
{
    const auto Mm = [](int32 Value) { return Value > 0 ? FString::FromInt(Value) : FString(); };
    Draft.Preset = Preset.Id;
    Draft.PackageType = Preset.Type;
    Draft.WidthMm = Mm(Preset.WidthMm);
    Draft.DepthMm = Mm(Preset.DepthMm);
    Draft.HeightMm = Mm(Preset.HeightMm);
    Draft.DiameterMm = Mm(Preset.DiameterMm);
    Draft.LabelHeightMm = Mm(Preset.LabelHeightMm);
    Draft.Parts = Preset.Parts;
    // Product colors win; the preset only fills parts the product has no color for.
    FString Colors = Draft.Colors;
    for (const FColorEntry& E : SplitColors(Preset.Colors))
        if (ColorText(Colors, E.Slot).IsEmpty()) Colors = WithColor(Colors, E.Slot, E.Value);
    // Drop colors of parts the new package does not have.
    for (const FColorEntry& E : SplitColors(Colors))
        if (!Preset.Parts.Contains(E.Slot)) Colors = WithColor(Colors, E.Slot, FString());
    Draft.Colors = Colors;
}

FString SuggestPreset(const TArray<FPackagePreset>& Presets, const FStudioDraft& Draft)
{
    const int32 W = FCString::Atoi(*Draft.WidthMm), D = FCString::Atoi(*Draft.DepthMm), H = FCString::Atoi(*Draft.HeightMm);
    const int32 Dia = FCString::Atoi(*Draft.DiameterMm);
    FString Best;
    double BestScore = TNumericLimits<double>::Max();
    for (const FPackagePreset& P : Presets)
    {
        if (P.Type != Draft.PackageType) continue;
        const double Score = P.IsBox()
            ? FMath::Abs(P.WidthMm - W) + FMath::Abs(P.DepthMm - D) + FMath::Abs(P.HeightMm - H)
            : FMath::Abs(P.DiameterMm - Dia) * 2.0 + FMath::Abs(P.HeightMm - H);
        if (Score < BestScore) { BestScore = Score; Best = P.Id; }
    }
    return Best;
}
}

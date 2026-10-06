#pragma once

// Helpers shared by the Product Studio backend files (StudioBackend, StudioMeshes, StudioPresets,
// StudioProducts, StudioPrompts). Not part of the studio API (see StudioBackend.h).

#include "StudioBackend.h"

class UMaterialInstanceConstant;
class UPackage;
class UTexture2D;
class FJsonObject;

namespace SimStudio
{
namespace Internal
{
    // If boxes ever render inside-out, flip this single switch (see Docs/Surec/DURUM.md).
    inline constexpr bool bFlipBoxWinding = false;
    inline constexpr int32 LabelAtlasSize = 2048;
    inline constexpr int32 MaxLabelSize = 4096;

    FString ObjectPath(const FString& Folder, const FString& Name);
    UPackage* OpenPackage(const FString& Folder, const FString& Name);
    bool SaveAll(const TArray<UObject*>& Assets);
    bool WriteJson(const FString& Path, const TSharedRef<FJsonObject>& Root);
    TSharedPtr<FJsonObject> ReadJson(const FString& Path);
    FString Normalized(FString Path);
    UTexture2D* WriteTexture(const FString& Folder, const FString& Name, const FStudioImage& Image, FString& OutError);
    UMaterialInstanceConstant* WriteMaterialInstance(const FString& Folder, const FString& Name, UMaterialInterface* Parent, UTexture2D* Label, FString& OutError);
    FString CopySource(const FString& Source, const FString& Directory, const FString& BaseName);
    FString FaceNameTr(int32 Face);
    bool SavePng(const FString& Path, const FStudioImage& Image, FString& OutError);
    void DrawUvLine(FStudioImage& Image, const FVector2f& From, const FVector2f& To, const FColor& Color, int32 Thickness = 2);
    void FillRect(FStudioImage& Image, const FIntRect& Rect, const FColor& Color);
    void StrokeRect(FStudioImage& Image, const FIntRect& Rect, const FColor& Color, int32 Thickness = 3);
    void DrawPixelLine(FStudioImage& Image, FIntPoint A, FIntPoint B, const FColor& Color, int32 Thickness = 2);
    FIntPoint ProductionSize(double WidthMm, double HeightMm);
    int32 FindSlot(const FStudioPackage& Package, ESlotRole Role);
}
}

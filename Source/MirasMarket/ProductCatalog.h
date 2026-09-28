#pragma once
#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Product catalog file (Config/products.json) shared by the game and the Product Studio.
// Schema v2 is backward compatible with v1: every new field is optional.
namespace MarketCatalog
{
    constexpr int32 SchemaVersion = 2;

    MIRASMARKET_API FString DefaultPath();
    // Active products are stocked in the game (no upper limit); inactive ones are the studio's preparation list.
    MIRASMARKET_API int32 CountActive(const TArray<FMarketProduct>& Products);
    // Lowercase ASCII letters, digits and '_' ; 3-40 chars; starts with a letter.
    MIRASMARKET_API bool IsValidId(const FString& Id);
    // Turkish-aware ASCII slug for ids ("Sutas Sut 1 L" -> "sutas_sut_1_l").
    MIRASMARKET_API FString MakeId(const FString& DisplayName);
    MIRASMARKET_API FString ColorHex(const FColor& Color);
    MIRASMARKET_API FString Money(int64 Kurus); // "2.50"
    // Parses "2,50" or "2.50" into kurus. Returns false for invalid / non-positive input.
    MIRASMARKET_API bool ParseMoney(const FString& Text, int64& OutKurus);

    // Invalid rows are skipped and reported in OutErrors. Returns false only if the JSON itself is unreadable.
    MIRASMARKET_API bool Parse(const FString& Json, TArray<FMarketProduct>& OutProducts, TArray<FString>& OutErrors, FString* OutNote = nullptr);
    // Deterministic, human-diffable JSON (UTF-8, two decimals for money).
    MIRASMARKET_API FString Serialize(const TArray<FMarketProduct>& Products, const FString& Note);
    MIRASMARKET_API bool LoadFile(const FString& Path, TArray<FMarketProduct>& OutProducts, TArray<FString>& OutErrors, FString* OutNote = nullptr);
    MIRASMARKET_API bool SaveFile(const FString& Path, const TArray<FMarketProduct>& Products, const FString& Note, FString& OutError);
}

// Deterministic atlas layout for the parametric rectangular box package.
// Product axes: +X front, +Y width, +Z up; pivot = bottom center; sizes in mm, mesh in cm.
// The same function drives the mesh UVs (Product Studio) and the label atlas composition,
// so labels and geometry can never drift apart.
struct MIRASMARKET_API FBoxPackageLayout
{
    enum EFace : int32 { Front = 0, Back, Right, Left, Top, Bottom, FaceCount };

    int32 WidthMm = 0;
    int32 DepthMm = 0;
    int32 HeightMm = 0;
    int32 AtlasSize = 2048;
    int32 Padding = 16;
    FIntRect Rects[FaceCount];

    static bool Make(int32 WidthMm, int32 DepthMm, int32 HeightMm, FBoxPackageLayout& Out, int32 AtlasSize = 2048);
    // Corners in cm, ordered as the face image is seen from outside: top-left, top-right, bottom-right, bottom-left.
    void GetFace(int32 Face, FVector (&OutCorners)[4], FVector2D (&OutUVs)[4], FVector& OutNormal) const;
    static FString PackageId(int32 WidthMm, int32 DepthMm, int32 HeightMm); // box_70x50x200
    static bool ParsePackageId(const FString& Id, int32& OutW, int32& OutD, int32& OutH);
    static const TCHAR* FaceKey(int32 Face); // front, back, right, left, top, bottom
};

#pragma once

#include "CoreMinimal.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SLeafWidget.h"

// Province map of Turkey for the Subeler page (G-074). Data: Config/iller.json, built from the MIT licensed
// turkey-map-react outlines (simplified and triangulated offline; see Docs/MENU.md). Store counts in the file are
// sample data from the menu design; the player's own stores come from the game state.
namespace MarketMapData
{
    struct FProvince
    {
        FString Id;                      // "kirklareli"
        FString Name;                    // "Kirklareli" with Turkish letters
        int32 Plate = 0;
        int32 PopulationK = 0;           // thousands
        FVector2f Center = FVector2f::ZeroVector;
        TMap<FString, int32> Stores;     // sample rival store counts: bim, a101, sok, migros, onur
        TArray<FVector2f> Points;        // map units
        TArray<uint32> Triangles;        // indices into Points
        TArray<FIntPoint> Rings;         // outline rings: (first point, count)
    };

    struct FData
    {
        bool bLoaded = false;
        FBox2f Bounds = FBox2f(ForceInit);
        FBox2f ThraceBounds = FBox2f(ForceInit);
        TArray<FProvince> Provinces;
    };

    // Loaded once from Config/iller.json (empty when the file is missing: the page then shows a list only).
    const FData& Get();
    // Province by its Turkish name ("Kirklareli" / "Istanbul" with or without Turkish letters), INDEX_NONE if unknown.
    int32 FindByName(const FString& Name);
}

// Draws the provinces (filled, outlined), marks the player's stores and reports clicks. Colours come from the owner
// every frame, so layers and selection need no rebuild.
class SMarketMap : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SMarketMap) {}
        SLATE_ARGUMENT(TFunction<FLinearColor(int32)>, FillOf)
        SLATE_ARGUMENT(TFunction<FLinearColor()>, LineColor)
        SLATE_ARGUMENT(TFunction<int32()>, Selected)
        SLATE_ARGUMENT(TFunction<bool()>, Thrace)
        SLATE_ARGUMENT(TFunction<bool(int32)>, HasPin)
        SLATE_ARGUMENT(TFunction<void(int32)>, OnPick)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
        FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override { return FVector2D(640.0, 340.0); }
    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override { Hovered = INDEX_NONE; }
    virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;

    // Province under the mouse (for the owner's tooltip line).
    int32 GetHovered() const { return Hovered; }

private:
    TFunction<FLinearColor(int32)> FillOf;
    TFunction<FLinearColor()> LineColor;
    TFunction<int32()> Selected;
    TFunction<bool()> Thrace;
    TFunction<bool(int32)> HasPin;
    TFunction<void(int32)> OnPick;
    int32 Hovered = INDEX_NONE;
    FSlateRoundedBoxBrush PinBrush = FSlateRoundedBoxBrush(FLinearColor::White, 7.f, FLinearColor::White, 2.f);

    // Map units -> local widget space: Local = Offset + Map * Scale (keeps the aspect, centred).
    void Frame(const FVector2f& Size, float& OutScale, FVector2f& OutOffset) const;
    int32 PickAt(const FGeometry& Geometry, const FVector2D& ScreenPosition) const;
};

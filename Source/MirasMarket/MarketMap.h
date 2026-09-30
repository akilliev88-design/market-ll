#pragma once

#include "CoreMinimal.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SLeafWidget.h"

// Province map of Turkey for the main screen (G-074, G-086: regions zoom in). Data: Config/iller.json, built from the MIT licensed
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
        FVector2f LabelAt = FVector2f::ZeroVector; // G-086f: the roomiest point inside (pins and names sit here)
        TMap<FString, int32> Stores;     // sample rival store counts: bim, a101, sok, migros, onur
        TArray<FVector2f> Points;        // map units
        TArray<uint32> Triangles;        // indices into Points
        TArray<FIntPoint> Rings;         // outline rings: (first point, count)
    };

    struct FData
    {
        bool bLoaded = false;
        FBox2f Bounds = FBox2f(ForceInit);
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
        // The provinces the view frames (a region chosen on the main screen); unset or none = the whole map.
        // The view glides to a new frame; zoomed in, the names of the framed provinces are drawn.
        SLATE_ARGUMENT(TFunction<bool(int32)>, InView)
        SLATE_ARGUMENT(TFunction<bool(int32)>, HasPin)
        SLATE_ARGUMENT(TFunction<void(int32)>, OnPick)
        // G-086 main screen: a number in the pin (0 = plain pin), the pin's colour, a warning outline and the
        // theme's colours for the selected / hovered outline and the pin text.
        SLATE_ARGUMENT(TFunction<int32(int32)>, PinCount)
        SLATE_ARGUMENT(TFunction<FLinearColor(int32)>, PinColor)
        SLATE_ARGUMENT(TFunction<bool(int32)>, WarnOf)
        SLATE_ARGUMENT(TFunction<FLinearColor()>, WarnColor)
        SLATE_ARGUMENT(TFunction<FLinearColor()>, StrongColor)
        SLATE_ARGUMENT(TFunction<FLinearColor(int32)>, PinTextOf)
        // G-086d: provinces named under their pin, and the colour of those names.
        SLATE_ARGUMENT(TFunction<bool(int32)>, LabelOf)
        SLATE_ARGUMENT(TFunction<FLinearColor()>, LabelColor)
        // G-086e: pixels on the right covered by a panel; the map glides left to stay in view.
        SLATE_ARGUMENT(TFunction<float()>, RightInset)
        // G-089: provinces with a depot of ours (a square marker with "D" beside the pin) and its colours; one
        // range ring around a province's centre (RingOf: the province, INDEX_NONE = none; RingRadius in map units).
        SLATE_ARGUMENT(TFunction<bool(int32)>, DepotOf)
        SLATE_ARGUMENT(TFunction<FLinearColor()>, DepotColor)
        SLATE_ARGUMENT(TFunction<FLinearColor()>, DepotTextColor)
        SLATE_ARGUMENT(TFunction<int32()>, RingOf)
        SLATE_ARGUMENT(TFunction<float()>, RingRadius)
        SLATE_ARGUMENT(TFunction<FLinearColor()>, RingColor)
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
    TFunction<bool(int32)> InView;
    TFunction<bool(int32)> HasPin;
    TFunction<void(int32)> OnPick;
    TFunction<int32(int32)> PinCount;
    TFunction<FLinearColor(int32)> PinColor;
    TFunction<bool(int32)> WarnOf;
    TFunction<FLinearColor()> WarnColor;
    TFunction<FLinearColor()> StrongColor;
    TFunction<FLinearColor(int32)> PinTextOf;
    TFunction<bool(int32)> LabelOf;
    TFunction<FLinearColor()> LabelColor;
    TFunction<float()> RightInset;
    TFunction<bool(int32)> DepotOf;
    TFunction<FLinearColor()> DepotColor;
    TFunction<FLinearColor()> DepotTextColor;
    TFunction<int32()> RingOf;
    TFunction<float()> RingRadius;
    TFunction<FLinearColor()> RingColor;
    int32 Hovered = INDEX_NONE;
    FSlateRoundedBoxBrush PinBrush = FSlateRoundedBoxBrush(FLinearColor::White, 12.f);
    FSlateRoundedBoxBrush HaloBrush = FSlateRoundedBoxBrush(FLinearColor::White, 5.f);
    FSlateRoundedBoxBrush DepotBrush = FSlateRoundedBoxBrush(FLinearColor::White, 3.f);

    // The view shown now (map units); it follows TargetView() a little every frame.
    mutable FBox2f ShownView = FBox2f(ForceInit);
    FBox2f TargetView() const;
    bool Zoomed() const;
    // Map units -> local widget space: Local = Offset + Map * Scale (keeps the aspect, centred).
    void Frame(const FVector2f& FullSize, float& OutScale, FVector2f& OutOffset) const;
    int32 PickAt(const FGeometry& Geometry, const FVector2D& ScreenPosition) const;
};

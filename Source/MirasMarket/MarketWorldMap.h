#pragma once

#include "CoreMinimal.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SLeafWidget.h"

// D7: the schematic world map of the main screen's world card. Every country pack with a place ("world" in
// Config/ulkeler.json) is a dot sized by its people inside a soft area of its continent; the owner colours the dots
// (our standing there) every frame and gets the clicked country. No outlines: the places are roughly geographic but
// spread so the names never touch.
class SMarketWorldMap : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SMarketWorldMap) {}
        SLATE_ARGUMENT(TFunction<FLinearColor(const FString&)>, DotColor)
        SLATE_ARGUMENT(TFunction<bool(const FString&)>, IsSelected)
        SLATE_ARGUMENT(TFunction<void(const FString&)>, OnPick)
        SLATE_ARGUMENT(TFunction<FLinearColor()>, AreaColor)
        SLATE_ARGUMENT(TFunction<FLinearColor()>, TextColor)
        SLATE_ARGUMENT(TFunction<FLinearColor()>, MutedColor)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
        FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
    virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override { return FVector2D(580.0, 290.0); }
    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override { Hovered.Reset(); }
    virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;

    // The country under the mouse ("" none), for the owner's line under the map.
    const FString& GetHovered() const { return Hovered; }

private:
    TFunction<FLinearColor(const FString&)> DotColor;
    TFunction<bool(const FString&)> IsSelected;
    TFunction<void(const FString&)> OnPick;
    TFunction<FLinearColor()> AreaColor;
    TFunction<FLinearColor()> TextColor;
    TFunction<FLinearColor()> MutedColor;
    FString Hovered;
    FSlateRoundedBoxBrush DotBrush = FSlateRoundedBoxBrush(FLinearColor::White, 12.f);
    FSlateRoundedBoxBrush AreaBrush = FSlateRoundedBoxBrush(FLinearColor::White, 18.f);

    // Map units (x 0..100, y 0..50) -> local space, keeping the aspect, centred.
    void Frame(const FVector2f& Size, float& OutScale, FVector2f& OutOffset) const;
    // A country's dot radius in pixels (by its people).
    static float Radius(int32 PopulationK);
    FString PickAt(const FGeometry& Geometry, const FVector2D& ScreenPosition) const;
};

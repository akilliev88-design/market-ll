#include "MarketWorldMap.h"
#include "MarketCountry.h"
#include "MarketTheme.h"

#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "InputCoreTypes.h"

namespace MarketWorldMapLocal
{
    constexpr float Width = 100.f;
    constexpr float Height = 50.f;

    bool Placed(const MarketCountry::FProfile& P) { return P.WorldX >= 0.f && P.WorldY >= 0.f && P.Cities.Num() > 0; }
}

void SMarketWorldMap::Construct(const FArguments& InArgs)
{
    DotColor = InArgs._DotColor;
    IsSelected = InArgs._IsSelected;
    OnPick = InArgs._OnPick;
    AreaColor = InArgs._AreaColor;
    TextColor = InArgs._TextColor;
    MutedColor = InArgs._MutedColor;
    DotBrush.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
    SetClipping(EWidgetClipping::ClipToBounds);
}

void SMarketWorldMap::Frame(const FVector2f& Size, float& OutScale, FVector2f& OutOffset) const
{
    OutScale = FMath::Max(0.01f, FMath::Min(Size.X / MarketWorldMapLocal::Width, Size.Y / MarketWorldMapLocal::Height));
    OutOffset = (Size - FVector2f(MarketWorldMapLocal::Width, MarketWorldMapLocal::Height) * OutScale) * 0.5f;
}

float SMarketWorldMap::Radius(int32 PopulationK)
{
    return FMath::Clamp(5.f + 7.f * FMath::Sqrt(static_cast<float>(FMath::Max(0, PopulationK)) / 330000.f), 5.f, 12.f);
}

int32 SMarketWorldMap::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    using namespace MarketWorldMapLocal;
    const FVector2f Size(AllottedGeometry.GetLocalSize());
    float Scale = 1.f;
    FVector2f Offset = FVector2f::ZeroVector;
    Frame(Size, Scale, Offset);
    const TSharedRef<FSlateFontMeasure> Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
    const FLinearColor Area = AreaColor ? AreaColor() : FLinearColor(0.f, 0.f, 0.f, 0.05f);
    const FLinearColor Ink = TextColor ? TextColor() : FLinearColor::Black;
    const FLinearColor Muted = MutedColor ? MutedColor() : FLinearColor::Gray;

    // The continents: a soft area around their countries, the name at its top left.
    const FSlateFontInfo AreaFont = MarketTheme::Font(MarketTheme::EFace::Semi, 10.f);
    for (const FString& Continent : MarketCountry::Continents())
    {
        FBox2f Box(ForceInit);
        for (const MarketCountry::FProfile& P : MarketCountry::All())
            if (P.Continent == Continent && Placed(P)) Box += FVector2f(P.WorldX, P.WorldY);
        if (!Box.bIsValid) continue;
        const FVector2f Min = Offset + (Box.Min - FVector2f(5.f, 6.f)) * Scale;
        const FVector2f Max = Offset + (Box.Max + FVector2f(5.f, 5.f)) * Scale;
        FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(Max - Min, FSlateLayoutTransform(1.f, Min)),
            &AreaBrush, ESlateDrawEffect::None, Area);
        const FString Name = MarketCountry::ContinentName(Continent).ToUpper();
        const FVector2f Text(Measure->Measure(Name, AreaFont));
        FSlateDrawElement::MakeText(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(Text, FSlateLayoutTransform(1.f, Min + FVector2f(8.f, 5.f))),
            Name, AreaFont, ESlateDrawEffect::None, Muted);
    }

    // The countries: a dot (a ring around the shown or hovered one) and the name on its side.
    for (const MarketCountry::FProfile& P : MarketCountry::All())
    {
        if (!Placed(P)) continue;
        const FVector2f At = Offset + FVector2f(P.WorldX, P.WorldY) * Scale;
        const float R = Radius(MarketCountry::PopulationK(P.Id));
        const bool bOn = (IsSelected && IsSelected(P.Id)) || P.Id == Hovered;
        if (bOn)
        {
            const float Ring = R + 3.f;
            FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(FVector2f(Ring * 2.f, Ring * 2.f), FSlateLayoutTransform(1.f, At - FVector2f(Ring, Ring))),
                &DotBrush, ESlateDrawEffect::None, Ink);
        }
        FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 3, AllottedGeometry.ToPaintGeometry(FVector2f(R * 2.f, R * 2.f), FSlateLayoutTransform(1.f, At - FVector2f(R, R))),
            &DotBrush, ESlateDrawEffect::None, DotColor ? DotColor(P.Id) : FLinearColor::Gray);
        const FSlateFontInfo Font = MarketTheme::Font(bOn ? MarketTheme::EFace::Bold : MarketTheme::EFace::Medium, 11.f);
        const FVector2f Text(Measure->Measure(P.Name, Font));
        FVector2f Corner(At.X - Text.X * 0.5f, At.Y + R + 2.f);
        if (P.WorldLabel == TEXT("above")) Corner = FVector2f(At.X - Text.X * 0.5f, At.Y - R - 2.f - Text.Y);
        else if (P.WorldLabel == TEXT("left")) Corner = FVector2f(At.X - R - 4.f - Text.X, At.Y - Text.Y * 0.5f);
        else if (P.WorldLabel == TEXT("right")) Corner = FVector2f(At.X + R + 4.f, At.Y - Text.Y * 0.5f);
        FSlateDrawElement::MakeText(OutDrawElements, LayerId + 4, AllottedGeometry.ToPaintGeometry(Text, FSlateLayoutTransform(1.f, Corner)),
            P.Name, Font, ESlateDrawEffect::None, Ink);
    }
    return LayerId + 5;
}

FString SMarketWorldMap::PickAt(const FGeometry& Geometry, const FVector2D& ScreenPosition) const
{
    using namespace MarketWorldMapLocal;
    float Scale = 1.f;
    FVector2f Offset = FVector2f::ZeroVector;
    Frame(FVector2f(Geometry.GetLocalSize()), Scale, Offset);
    const FVector2f Local(Geometry.AbsoluteToLocal(ScreenPosition));
    FString Best;
    float BestDistance = TNumericLimits<float>::Max();
    for (const MarketCountry::FProfile& P : MarketCountry::All())
    {
        if (!Placed(P)) continue;
        const float Distance = FVector2f::Distance(Local, Offset + FVector2f(P.WorldX, P.WorldY) * Scale);
        if (Distance <= Radius(MarketCountry::PopulationK(P.Id)) + 8.f && Distance < BestDistance) { BestDistance = Distance; Best = P.Id; }
    }
    return Best;
}

FReply SMarketWorldMap::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton) return FReply::Unhandled();
    const FString Picked = PickAt(MyGeometry, MouseEvent.GetScreenSpacePosition());
    if (!Picked.IsEmpty() && OnPick) OnPick(Picked);
    return FReply::Handled();
}

FReply SMarketWorldMap::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    Hovered = PickAt(MyGeometry, MouseEvent.GetScreenSpacePosition());
    return FReply::Unhandled();
}

FCursorReply SMarketWorldMap::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
    return Hovered.IsEmpty() ? FCursorReply::Unhandled() : FCursorReply::Cursor(EMouseCursor::Hand);
}

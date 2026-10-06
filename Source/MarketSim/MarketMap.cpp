#include "MarketMap.h"
#include "MarketTheme.h"

#include "ProductCatalog.h"
#include "Dom/JsonObject.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Styling/CoreStyle.h"
#include "InputCoreTypes.h"

namespace MarketMapData
{
    FString NameKey(const FString& Name)
    {
        return MarketCatalog::FoldTurkish(Name.TrimStartAndEnd()).ToLower().Replace(TEXT(" "), TEXT(""));
    }

    // G-086f label placement. Signed distance to the outline (positive inside, even-odd over every ring).
    float ProvinceDistance(const FProvince& P, const FVector2f& Q)
    {
        bool bInside = false;
        float Best = TNumericLimits<float>::Max();
        for (const FIntPoint& Ring : P.Rings)
            for (int32 K = 0, J = Ring.Y - 1; K < Ring.Y; J = K++)
            {
                const FVector2f& A = P.Points[Ring.X + K];
                const FVector2f& B = P.Points[Ring.X + J];
                if ((A.Y > Q.Y) != (B.Y > Q.Y) && Q.X < (B.X - A.X) * (Q.Y - A.Y) / (B.Y - A.Y) + A.X) bInside = !bInside;
                const FVector2f Edge = B - A;
                const float T = FMath::Clamp(FVector2f::DotProduct(Q - A, Edge) / FMath::Max(1e-6f, Edge.SizeSquared()), 0.f, 1.f);
                Best = FMath::Min(Best, FVector2f::DistSquared(Q, A + Edge * T));
            }
        return (bInside ? 1.f : -1.f) * FMath::Sqrt(Best);
    }

    // The point deepest inside the province (pole of inaccessibility, sampled then refined), a little favouring
    // wide spots: the distance counts double sideways, where a name needs the room.
    FVector2f ProvincePole(const FProvince& P)
    {
        FBox2f Box(ForceInit);
        for (const FVector2f& Point : P.Points) Box += Point;
        if (!Box.bIsValid) return P.Center;
        auto Score = [&P](const FVector2f& Q)
        {
            const float Inside = ProvinceDistance(P, Q);
            if (Inside <= 0.f) return Inside;
            const float Side = FMath::Min(ProvinceDistance(P, Q + FVector2f(Inside, 0.f)), ProvinceDistance(P, Q - FVector2f(Inside, 0.f)));
            return Inside + 0.5f * FMath::Max(0.f, Side);
        };
        FVector2f Best = P.Center;
        float BestScore = Score(Best);
        const int32 Steps = 24;
        const FVector2f Size = Box.GetSize();
        for (int32 I = 0; I <= Steps; ++I)
            for (int32 J = 0; J <= Steps; ++J)
            {
                const FVector2f Q = Box.Min + FVector2f(Size.X * I / Steps, Size.Y * J / Steps);
                const float S = Score(Q);
                if (S > BestScore) { BestScore = S; Best = Q; }
            }
        FVector2f Cell = Size / static_cast<float>(Steps);
        for (int32 Round = 0; Round < 3; ++Round)
        {
            const FVector2f Around = Best;
            for (int32 I = -3; I <= 3; ++I)
                for (int32 J = -3; J <= 3; ++J)
                {
                    const FVector2f Q = Around + FVector2f(Cell.X * I / 3.f, Cell.Y * J / 3.f);
                    const float S = Score(Q);
                    if (S > BestScore) { BestScore = S; Best = Q; }
                }
            Cell /= 3.f;
        }
        return Best;
    }

    // The inside stretch of the horizontal line through (X, Y): false when the line misses the province.
    bool ProvinceSpan(const FProvince& P, float X, float Y, float& OutLeft, float& OutRight)
    {
        TArray<float, TInlineAllocator<16>> Cuts;
        for (const FIntPoint& Ring : P.Rings)
            for (int32 K = 0, J = Ring.Y - 1; K < Ring.Y; J = K++)
            {
                const FVector2f& A = P.Points[Ring.X + K];
                const FVector2f& B = P.Points[Ring.X + J];
                if ((A.Y > Y) != (B.Y > Y)) Cuts.Add(A.X + (Y - A.Y) * (B.X - A.X) / (B.Y - A.Y));
            }
        if (Cuts.Num() < 2) return false;
        Cuts.Sort();
        float BestGap = TNumericLimits<float>::Max();
        for (int32 I = 0; I + 1 < Cuts.Num(); I += 2)
        {
            const float Gap = X < Cuts[I] ? Cuts[I] - X : X > Cuts[I + 1] ? X - Cuts[I + 1] : 0.f;
            if (Gap < BestGap) { BestGap = Gap; OutLeft = Cuts[I]; OutRight = Cuts[I + 1]; }
        }
        return true;
    }

    void Load(FData& Data)
    {
        FString Text;
        const FString Path = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("iller.json"));
        if (!FFileHelper::LoadFileToString(Text, *Path)) { UE_LOG(LogTemp, Warning, TEXT("MarketSim map: %s missing."), *Path); return; }
        TSharedPtr<FJsonObject> Root;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
        if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid()) { UE_LOG(LogTemp, Warning, TEXT("MarketSim map: iller.json unreadable.")); return; }
        const TArray<TSharedPtr<FJsonValue>>* List = nullptr;
        if (!Root->TryGetArrayField(TEXT("provinces"), List)) return;
        for (const TSharedPtr<FJsonValue>& Value : *List)
        {
            const TSharedPtr<FJsonObject> Obj = Value.IsValid() ? Value->AsObject() : nullptr;
            if (!Obj.IsValid()) continue;
            FProvince P;
            P.Id = Obj->GetStringField(TEXT("id"));
            P.Name = Obj->GetStringField(TEXT("name"));
            P.Plate = static_cast<int32>(Obj->GetNumberField(TEXT("plate")));
            P.PopulationK = static_cast<int32>(Obj->GetNumberField(TEXT("pop")));
            P.Center = FVector2f(static_cast<float>(Obj->GetNumberField(TEXT("cx"))), static_cast<float>(Obj->GetNumberField(TEXT("cy"))));
            const TSharedPtr<FJsonObject>* Counts = nullptr;
            if (Obj->TryGetObjectField(TEXT("c"), Counts) && Counts && Counts->IsValid())
                for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*Counts)->Values)
                    if (Pair.Key != TEXT("me") && Pair.Value.IsValid()) P.Stores.Add(Pair.Key, static_cast<int32>(Pair.Value->AsNumber()));
            const TArray<TSharedPtr<FJsonValue>>* V = nullptr;
            const TArray<TSharedPtr<FJsonValue>>* T = nullptr;
            const TArray<TSharedPtr<FJsonValue>>* R = nullptr;
            if (!Obj->TryGetArrayField(TEXT("v"), V) || !Obj->TryGetArrayField(TEXT("t"), T) || !Obj->TryGetArrayField(TEXT("r"), R)) continue;
            for (int32 I = 0; I + 1 < V->Num(); I += 2)
                P.Points.Add(FVector2f(static_cast<float>((*V)[I]->AsNumber()), static_cast<float>((*V)[I + 1]->AsNumber())));
            for (const TSharedPtr<FJsonValue>& Index : *T)
            {
                const int32 At = static_cast<int32>(Index->AsNumber());
                if (P.Points.IsValidIndex(At)) P.Triangles.Add(static_cast<uint32>(At));
            }
            P.Triangles.SetNum(P.Triangles.Num() - P.Triangles.Num() % 3);
            for (int32 I = 0; I + 1 < R->Num(); I += 2)
            {
                const FIntPoint Ring(static_cast<int32>((*R)[I]->AsNumber()), static_cast<int32>((*R)[I + 1]->AsNumber()));
                if (Ring.X >= 0 && Ring.Y >= 3 && Ring.X + Ring.Y <= P.Points.Num()) P.Rings.Add(Ring);
            }
            for (const FVector2f& Point : P.Points) Data.Bounds += Point;
            P.LabelAt = P.Rings.Num() > 0 ? ProvincePole(P) : P.Center;
            Data.Provinces.Add(MoveTemp(P));
        }
        Data.bLoaded = Data.Provinces.Num() > 0 && Data.Bounds.bIsValid;
    }

    const FData& Get()
    {
        static FData Data;
        static bool bTried = false;
        if (!bTried) { bTried = true; Load(Data); }
        return Data;
    }

    int32 FindByName(const FString& Name)
    {
        const FString Key = NameKey(Name);
        const FData& Data = Get();
        for (int32 I = 0; I < Data.Provinces.Num(); ++I)
            if (NameKey(Data.Provinces[I].Name) == Key || Data.Provinces[I].Id == Key) return I;
        return INDEX_NONE;
    }
}

void SMarketMap::Construct(const FArguments& InArgs)
{
    FillOf = InArgs._FillOf;
    LineColor = InArgs._LineColor;
    Selected = InArgs._Selected;
    InView = InArgs._InView;
    HasPin = InArgs._HasPin;
    OnPick = InArgs._OnPick;
    PinCount = InArgs._PinCount;
    PinColor = InArgs._PinColor;
    WarnOf = InArgs._WarnOf;
    WarnColor = InArgs._WarnColor;
    StrongColor = InArgs._StrongColor;
    PinTextOf = InArgs._PinTextOf;
    LabelOf = InArgs._LabelOf;
    LabelColor = InArgs._LabelColor;
    RightInset = InArgs._RightInset;
    DepotOf = InArgs._DepotOf;
    DepotColor = InArgs._DepotColor;
    DepotTextColor = InArgs._DepotTextColor;
    RingOf = InArgs._RingOf;
    RingRadius = InArgs._RingRadius;
    RingColor = InArgs._RingColor;
    PinBrush.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
    SetClipping(EWidgetClipping::ClipToBounds);
}

FBox2f SMarketMap::TargetView() const
{
    const MarketMapData::FData& Data = MarketMapData::Get();
    if (!InView) return Data.Bounds;
    FBox2f View(ForceInit);
    for (int32 P = 0; P < Data.Provinces.Num(); ++P)
        if (InView(P)) for (const FVector2f& Point : Data.Provinces[P].Points) View += Point;
    if (!View.bIsValid) return Data.Bounds;
    return View.ExpandBy(View.GetSize().GetMax() * 0.08f);
}

bool SMarketMap::Zoomed() const
{
    const MarketMapData::FData& Data = MarketMapData::Get();
    return ShownView.bIsValid && ShownView.GetSize().X < Data.Bounds.GetSize().X * 0.75f;
}

void SMarketMap::Frame(const FVector2f& FullSize, float& OutScale, FVector2f& OutOffset) const
{
    const FVector2f Size(FMath::Max(40.f, FullSize.X - (RightInset ? FMath::Max(0.f, RightInset()) : 0.f)), FullSize.Y);
    const FBox2f View = ShownView.bIsValid ? ShownView : TargetView();
    const FVector2f ViewSize = View.GetSize();
    const float Margin = 8.f;
    OutScale = FMath::Min((Size.X - 2.f * Margin) / FMath::Max(1.f, ViewSize.X), (Size.Y - 2.f * Margin) / FMath::Max(1.f, ViewSize.Y));
    OutScale = FMath::Max(OutScale, 0.01f);
    OutOffset = (Size - ViewSize * OutScale) * 0.5f - View.Min * OutScale;
}

int32 SMarketMap::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const MarketMapData::FData& Data = MarketMapData::Get();
    if (!Data.bLoaded) return LayerId;
    const FVector2f Size(AllottedGeometry.GetLocalSize());
    // Glide towards the chosen frame (about a third of a second).
    const FBox2f Target = TargetView();
    if (!ShownView.bIsValid) ShownView = Target;
    else
    {
        const float Step = FMath::Clamp(static_cast<float>(FSlateApplication::Get().GetDeltaTime()), 0.f, 0.1f);
        const float Blend = 1.f - FMath::Exp(-10.f * Step);
        ShownView.Min += (Target.Min - ShownView.Min) * Blend;
        ShownView.Max += (Target.Max - ShownView.Max) * Blend;
        if (FVector2f::DistSquared(ShownView.Min, Target.Min) + FVector2f::DistSquared(ShownView.Max, Target.Max) < 0.01f) ShownView = Target;
    }
    float Scale = 1.f;
    FVector2f Offset = FVector2f::ZeroVector;
    Frame(Size, Scale, Offset);
    const FSlateRenderTransform& Render = AllottedGeometry.GetAccumulatedRenderTransform();

    // Filled provinces: one custom-vertex batch with the white brush, coloured per vertex.
    const FSlateBrush* White = FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
    const FSlateResourceHandle Handle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*White, FVector2f::ZeroVector, 1.f);
    TArray<FSlateVertex> Verts;
    TArray<SlateIndex> Indexes;
    for (int32 P = 0; P < Data.Provinces.Num(); ++P)
    {
        const MarketMapData::FProvince& Province = Data.Provinces[P];
        const FColor Fill = (FillOf ? FillOf(P) : FLinearColor(0.8f, 0.8f, 0.8f)).ToFColor(true);
        const uint32 Base = static_cast<uint32>(Verts.Num());
        for (const FVector2f& Point : Province.Points)
        {
            FSlateVertex Vertex;
            FMemory::Memzero(Vertex);
            Vertex.Position = Render.TransformPoint(Offset + Point * Scale);
            Vertex.Color = Fill;
            Vertex.TexCoords[0] = 0.5f; Vertex.TexCoords[1] = 0.5f; Vertex.TexCoords[2] = 1.f; Vertex.TexCoords[3] = 1.f;
            Verts.Add(Vertex);
        }
        for (const uint32 Index : Province.Triangles) Indexes.Add(static_cast<SlateIndex>(Base + Index));
    }
    if (Indexes.Num() > 0) FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, Handle, Verts, Indexes, nullptr, 0, 0);

    // Borders in the paper's colour (1.2 design units, like the boards); a problem province gets a warm outline,
    // the hovered and the selected one a dark line on top.
    const float Unit = FMath::Clamp(Scale, 0.6f, 1.6f);
    const FLinearColor Line = LineColor ? LineColor() : FLinearColor::White;
    const int32 Chosen = Selected ? Selected() : INDEX_NONE;
    auto Outline = [&](int32 P, const FLinearColor& Color, float Thickness, int32 Layer)
    {
        const MarketMapData::FProvince& Province = Data.Provinces[P];
        for (const FIntPoint& Ring : Province.Rings)
        {
            TArray<FVector2f> Points;
            Points.Reserve(Ring.Y + 1);
            for (int32 K = 0; K < Ring.Y; ++K) Points.Add(Offset + Province.Points[Ring.X + K] * Scale);
            const FVector2f First = Points[0]; // a copy: Add() must not read from the array it grows
            Points.Add(First);
            FSlateDrawElement::MakeLines(OutDrawElements, Layer, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, Color, true, Thickness);
        }
    };
    for (int32 P = 0; P < Data.Provinces.Num(); ++P) Outline(P, Line, 1.2f * Unit, LayerId + 1);
    const FLinearColor Strong = StrongColor ? StrongColor() : FLinearColor(0.09f, 0.10f, 0.12f);
    if (WarnOf)
    {
        const FLinearColor Warn = WarnColor ? WarnColor() : FLinearColor(0.95f, 0.70f, 0.35f);
        for (int32 P = 0; P < Data.Provinces.Num(); ++P) if (P != Chosen && WarnOf(P)) Outline(P, Warn, 2.4f * Unit, LayerId + 2);
    }
    if (Data.Provinces.IsValidIndex(Hovered) && Hovered != Chosen) Outline(Hovered, Strong * FLinearColor(1.f, 1.f, 1.f, 0.45f), 1.6f * Unit, LayerId + 2);
    if (Data.Provinces.IsValidIndex(Chosen)) Outline(Chosen, Strong, 2.6f * Unit, LayerId + 2);

    // G-089: a depot's range, a soft disc with a dashed-looking rim around the province's centre (the same centre
    // the depot distances use), under the pins.
    const int32 RingProvince = RingOf ? RingOf() : INDEX_NONE;
    const float RingUnits = RingRadius ? RingRadius() : 0.f;
    if (Data.Provinces.IsValidIndex(RingProvince) && RingUnits > 0.f)
    {
        const FLinearColor RingInk = RingColor ? RingColor() : FLinearColor(0.086f, 0.467f, 0.373f);
        const FVector2f Middle = Offset + Data.Provinces[RingProvince].Center * Scale;
        const float Pixels = RingUnits * Scale;
        constexpr int32 Segments = 72;
        TArray<FSlateVertex> DiscVerts;
        TArray<SlateIndex> DiscIndexes;
        FLinearColor DiscFill = RingInk;
        DiscFill.A = 0.10f;
        const FColor DiscColor = DiscFill.ToFColor(true);
        auto DiscVertex = [&Render, &DiscColor](const FVector2f& At)
        {
            FSlateVertex Vertex;
            FMemory::Memzero(Vertex);
            Vertex.Position = Render.TransformPoint(At);
            Vertex.Color = DiscColor;
            Vertex.TexCoords[0] = 0.5f; Vertex.TexCoords[1] = 0.5f; Vertex.TexCoords[2] = 1.f; Vertex.TexCoords[3] = 1.f;
            return Vertex;
        };
        DiscVerts.Add(DiscVertex(Middle));
        TArray<FVector2f> Rim;
        Rim.Reserve(Segments + 1);
        for (int32 K = 0; K <= Segments; ++K)
        {
            const float Angle = static_cast<float>(UE_TWO_PI) * static_cast<float>(K) / static_cast<float>(Segments);
            const FVector2f At = Middle + FVector2f(FMath::Cos(Angle), FMath::Sin(Angle)) * Pixels;
            Rim.Add(At);
            DiscVerts.Add(DiscVertex(At));
            if (K > 0)
            {
                DiscIndexes.Add(0);
                DiscIndexes.Add(static_cast<SlateIndex>(K));
                DiscIndexes.Add(static_cast<SlateIndex>(K + 1));
            }
        }
        FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId + 2, Handle, DiscVerts, DiscIndexes, nullptr, 0, 0);
        // Every other segment of the rim: reads as a range, not as a border.
        for (int32 K = 0; K + 1 < Rim.Num(); K += 2)
        {
            TArray<FVector2f> Dash;
            Dash.Add(Rim[K]);
            Dash.Add(Rim[K + 1]);
            FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(), Dash, ESlateDrawEffect::None, RingInk, true, 1.6f * Unit);
        }
    }

    // Pins and the chosen province's name (G-086f). Both sit on the roomiest point of the province: a pin alone, a name alone
    // centred there, or the pin with the name under it, the pair centred. A name fits its province: it slides
    // sideways inside, shrinks down to 8 px, and only then gets a soft paper halo so it reads across the border.
    const float Radius = 10.f * FMath::Clamp(Scale, 0.8f, 1.35f);
    const FSlateFontInfo PinFont = MarketTheme::Font(MarketTheme::EFace::Mono, 11.f * FMath::Clamp(Scale, 0.85f, 1.25f));
    const TSharedRef<FSlateFontMeasure> Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
    const float TextScale = FMath::Clamp(Scale, 0.85f, 1.3f);
    const FLinearColor Soft = LabelColor ? LabelColor() : Strong;
    FLinearColor Halo = Line;
    Halo.A = 0.88f;
    for (int32 P = 0; P < Data.Provinces.Num(); ++P)
    {
        const MarketMapData::FProvince& Province = Data.Provinces[P];
        const bool bPin = HasPin && HasPin(P);
        // Mustafa (G-086f): only the chosen province shows its name.
        const bool bName = P == Chosen;
        const bool bDepot = DepotOf && DepotOf(P);
        if (!bPin && !bName && !bDepot) continue;
        const FVector2f Anchor = Offset + Province.LabelAt * Scale;
        const float R = P == Chosen ? Radius * 1.2f : Radius;
        // G-089: the depot marker, a small square with "D": beside the pin (upper right), on the anchor without one.
        if (bDepot)
        {
            const float Side = R * 1.5f;
            const FVector2f Corner = bPin ? Anchor + FVector2f(R * 0.55f, -R * 1.9f) : Anchor - FVector2f(Side * 0.5f, Side * 0.5f);
            FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 5,
                AllottedGeometry.ToPaintGeometry(FVector2f(Side, Side), FSlateLayoutTransform(1.f, Corner)),
                &DepotBrush, ESlateDrawEffect::None, DepotColor ? DepotColor() : FLinearColor(0.35f, 0.28f, 0.60f));
            const FSlateFontInfo DepotFont = MarketTheme::Font(MarketTheme::EFace::Bold, Side * 0.62f);
            const FVector2f Letter(Measure->Measure(FString(TEXT("D")), DepotFont));
            FSlateDrawElement::MakeText(OutDrawElements, LayerId + 6,
                AllottedGeometry.ToPaintGeometry(Letter, FSlateLayoutTransform(1.f, Corner + (FVector2f(Side, Side) - Letter) * 0.5f)),
                FString(TEXT("D")), DepotFont, ESlateDrawEffect::None, DepotTextColor ? DepotTextColor() : FLinearColor::White);
        }
        if (!bPin && !bName) continue;

        MarketTheme::EFace Face = MarketTheme::EFace::Semi;
        float Pixels = 10.f * TextScale;
        FLinearColor Ink = Soft;
        if (P == Hovered) { Pixels = 12.f * TextScale; Ink = Strong; }
        if (P == Chosen) { Face = MarketTheme::EFace::Bold; Pixels = 13.f * TextScale; Ink = Strong; }
        FSlateFontInfo Font = MarketTheme::Font(Face, Pixels);
        FVector2f Text = bName ? FVector2f(Measure->Measure(Province.Name, Font)) : FVector2f::ZeroVector;

        const float Block = (bPin ? 2.f * R : 0.f) + (bPin && bName ? 3.f : 0.f) + Text.Y;
        const float Top = Anchor.Y - Block * 0.5f;
        if (bPin)
        {
            const FVector2f Pin(Anchor.X, Top + R);
            FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 3,
                AllottedGeometry.ToPaintGeometry(FVector2f(R * 2.f, R * 2.f), FSlateLayoutTransform(1.f, Pin - FVector2f(R, R))),
                &PinBrush, ESlateDrawEffect::None, PinColor ? PinColor(P) : FLinearColor(0.086f, 0.467f, 0.373f));
            const int32 Count = PinCount ? PinCount(P) : 0;
            if (Count > 0)
            {
                const FString Number = Count > 99 ? FString(TEXT("99+")) : FString::FromInt(Count);
                const FVector2f Box(Measure->Measure(Number, PinFont));
                FSlateDrawElement::MakeText(OutDrawElements, LayerId + 4,
                    AllottedGeometry.ToPaintGeometry(Box, FSlateLayoutTransform(1.f, Pin - Box * 0.5f)),
                    Number, PinFont, ESlateDrawEffect::None, PinTextOf ? PinTextOf(P) : FLinearColor::White);
            }
        }
        if (!bName) continue;

        // The room inside the province on the name's line (a little margin from the border).
        const float TextTop = Top + (bPin ? 2.f * R + 3.f : 0.f);
        float Left = Anchor.X - 1000.f, Right = Anchor.X + 1000.f;
        float MapLeft = 0.f, MapRight = 0.f;
        if (MarketMapData::ProvinceSpan(Province, Province.LabelAt.X, (TextTop + Text.Y * 0.5f - Offset.Y) / Scale, MapLeft, MapRight))
        {
            Left = Offset.X + MapLeft * Scale + 4.f;
            Right = Offset.X + MapRight * Scale - 4.f;
        }
        const float Room = FMath::Max(0.f, Right - Left);
        if (Text.X > Room && Room > 0.f)
        {
            Pixels = FMath::Max(8.f, Pixels * Room / Text.X);
            Font = MarketTheme::Font(Face, Pixels);
            Text = FVector2f(Measure->Measure(Province.Name, Font));
        }
        const bool bFits = Text.X <= Room;
        float X = Anchor.X - Text.X * 0.5f;
        if (bFits) X = FMath::Clamp(X, Left, Right - Text.X);
        const FVector2f At(X, TextTop);
        if (!bFits)
        {
            const FVector2f Pad(4.f, 1.f);
            FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 5,
                AllottedGeometry.ToPaintGeometry(Text + Pad * 2.f, FSlateLayoutTransform(1.f, At - Pad)), &HaloBrush, ESlateDrawEffect::None, Halo);
        }
        FSlateDrawElement::MakeText(OutDrawElements, LayerId + 6, AllottedGeometry.ToPaintGeometry(Text, FSlateLayoutTransform(1.f, At)),
            Province.Name, Font, ESlateDrawEffect::None, Ink);
    }
    return LayerId + 7;
}

int32 SMarketMap::PickAt(const FGeometry& Geometry, const FVector2D& ScreenPosition) const
{
    const MarketMapData::FData& Data = MarketMapData::Get();
    if (!Data.bLoaded) return INDEX_NONE;
    float Scale = 1.f;
    FVector2f Offset = FVector2f::ZeroVector;
    Frame(FVector2f(Geometry.GetLocalSize()), Scale, Offset);
    const FVector2f Local(Geometry.AbsoluteToLocal(ScreenPosition));
    const FVector2f Map = (Local - Offset) / Scale;
    for (int32 P = 0; P < Data.Provinces.Num(); ++P)
    {
        const MarketMapData::FProvince& Province = Data.Provinces[P];
        bool bInside = false;
        for (const FIntPoint& Ring : Province.Rings)
        {
            // Even-odd rule over every ring (islands and lakes included).
            for (int32 K = 0, J = Ring.Y - 1; K < Ring.Y; J = K++)
            {
                const FVector2f& A = Province.Points[Ring.X + K];
                const FVector2f& B = Province.Points[Ring.X + J];
                if ((A.Y > Map.Y) != (B.Y > Map.Y) && Map.X < (B.X - A.X) * (Map.Y - A.Y) / (B.Y - A.Y) + A.X) bInside = !bInside;
            }
        }
        if (bInside) return P;
    }
    return INDEX_NONE;
}

FReply SMarketMap::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton) return FReply::Unhandled();
    const int32 Picked = PickAt(MyGeometry, MouseEvent.GetScreenSpacePosition());
    if (Picked != INDEX_NONE && OnPick) OnPick(Picked);
    return FReply::Handled();
}

FReply SMarketMap::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    Hovered = PickAt(MyGeometry, MouseEvent.GetScreenSpacePosition());
    return FReply::Unhandled();
}

FCursorReply SMarketMap::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
    return Hovered != INDEX_NONE ? FCursorReply::Cursor(EMouseCursor::Hand) : FCursorReply::Unhandled();
}

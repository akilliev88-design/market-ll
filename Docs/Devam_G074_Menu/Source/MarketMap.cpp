#include "MarketMap.h"

#include "ProductCatalog.h"
#include "Dom/JsonObject.h"
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

    void Load(FData& Data)
    {
        FString Text;
        const FString Path = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("iller.json"));
        if (!FFileHelper::LoadFileToString(Text, *Path)) { UE_LOG(LogTemp, Warning, TEXT("MirasMarket map: %s missing."), *Path); return; }
        TSharedPtr<FJsonObject> Root;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
        if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid()) { UE_LOG(LogTemp, Warning, TEXT("MirasMarket map: iller.json unreadable.")); return; }
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
            if (P.Id == TEXT("edirne") || P.Id == TEXT("kirklareli") || P.Id == TEXT("tekirdag") || P.Id == TEXT("istanbul") || P.Id == TEXT("canakkale"))
                for (const FVector2f& Point : P.Points) Data.ThraceBounds += Point;
            Data.Provinces.Add(MoveTemp(P));
        }
        Data.bLoaded = Data.Provinces.Num() > 0 && Data.Bounds.bIsValid;
        if (Data.ThraceBounds.bIsValid) Data.ThraceBounds = Data.ThraceBounds.ExpandBy(Data.ThraceBounds.GetSize().GetMax() * 0.06f);
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
    Thrace = InArgs._Thrace;
    HasPin = InArgs._HasPin;
    OnPick = InArgs._OnPick;
    SetClipping(EWidgetClipping::ClipToBounds);
}

void SMarketMap::Frame(const FVector2f& Size, float& OutScale, FVector2f& OutOffset) const
{
    const MarketMapData::FData& Data = MarketMapData::Get();
    const FBox2f View = Thrace && Thrace() && Data.ThraceBounds.bIsValid ? Data.ThraceBounds : Data.Bounds;
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

    // Borders; the selected and the hovered province get a darker, thicker line on top.
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
            Points.Add(Points[0]);
            FSlateDrawElement::MakeLines(OutDrawElements, Layer, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, Color, true, Thickness);
        }
    };
    for (int32 P = 0; P < Data.Provinces.Num(); ++P) Outline(P, Line, 0.8f, LayerId + 1);
    const FLinearColor Strong = FillOf ? FLinearColor(0.09f, 0.10f, 0.12f) : FLinearColor::Black;
    if (Data.Provinces.IsValidIndex(Hovered) && Hovered != Chosen) Outline(Hovered, Strong * FLinearColor(1.f, 1.f, 1.f, 0.5f), 1.5f, LayerId + 2);
    if (Data.Provinces.IsValidIndex(Chosen)) Outline(Chosen, Strong, 2.2f, LayerId + 2);

    // Pins on provinces with our stores.
    for (int32 P = 0; P < Data.Provinces.Num(); ++P)
    {
        if (!HasPin || !HasPin(P)) continue;
        const FVector2f At = Offset + Data.Provinces[P].Center * Scale;
        const float Radius = 7.f;
        FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 3,
            AllottedGeometry.ToPaintGeometry(FVector2f(Radius * 2.f, Radius * 2.f), FSlateLayoutTransform(1.f, At - FVector2f(Radius, Radius))),
            &PinBrush, ESlateDrawEffect::None, FLinearColor(0.086f, 0.467f, 0.373f));
    }
    return LayerId + 4;
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

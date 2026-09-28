#include "Planogram.h"

#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ProductCatalog.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"


const FPlanogramFixture* FMarketPlanogram::FindFixture(const FString& Id) const
{
    return Fixtures.FindByPredicate([&](const FPlanogramFixture& Fixture) { return Fixture.Id == Id; });
}

FPlanogramFixture* FMarketPlanogram::FindFixture(const FString& Id)
{
    return Fixtures.FindByPredicate([&](const FPlanogramFixture& Fixture) { return Fixture.Id == Id; });
}

const FPlanogramPlacement* FMarketPlanogram::FindPlacement(const FString& ProductId) const
{
    return Placements.FindByPredicate([&](const FPlanogramPlacement& Placement) { return Placement.ProductId == ProductId; });
}

FPlanogramPlacement* FMarketPlanogram::FindPlacement(const FString& ProductId)
{
    return Placements.FindByPredicate([&](const FPlanogramPlacement& Placement) { return Placement.ProductId == ProductId; });
}

FString MarketPlanogram::DefaultPath()
{
    return FPaths::ProjectConfigDir() / TEXT("planograms.json");
}

bool MarketPlanogram::Parse(const FString& Json, FMarketPlanogram& OutPlanogram, TArray<FString>& OutErrors)
{
    OutPlanogram = FMarketPlanogram();
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid())
    {
        OutErrors.Add(TEXT("planograms.json okunamadi: gecersiz JSON."));
        return false;
    }

    const TArray<TSharedPtr<FJsonValue>>* Fixtures = nullptr;
    if (!Root->TryGetArrayField(TEXT("fixtures"), Fixtures))
    {
        OutErrors.Add(TEXT("planograms.json icinde fixtures listesi yok."));
        return false;
    }
    TSet<FString> FixtureIds;
    for (int32 Index = 0; Index < Fixtures->Num(); ++Index)
    {
        const TSharedPtr<FJsonObject> Obj = (*Fixtures)[Index]->AsObject();
        FPlanogramFixture Fixture;
        if (!Obj.IsValid() || !Obj->TryGetStringField(TEXT("id"), Fixture.Id) || !MarketCatalog::IsValidId(Fixture.Id) || FixtureIds.Contains(Fixture.Id))
        {
            OutErrors.Add(FString::Printf(TEXT("Fixture #%d: gecersiz veya tekrar eden id."), Index + 1));
            continue;
        }
        FixtureIds.Add(Fixture.Id);
        Obj->TryGetStringField(TEXT("equipment"), Fixture.EquipmentId);
        Obj->TryGetStringField(TEXT("label"), Fixture.Label);
        Obj->TryGetStringField(TEXT("category"), Fixture.Category);
        double X = 0, Y = 0, Z = 0, Yaw = 0;
        Obj->TryGetNumberField(TEXT("x"), X); Obj->TryGetNumberField(TEXT("y"), Y);
        Obj->TryGetNumberField(TEXT("z"), Z); Obj->TryGetNumberField(TEXT("yaw"), Yaw);
        Fixture.Location = FVector(X, Y, Z);
        Fixture.Yaw = Yaw;
        OutPlanogram.Fixtures.Add(Fixture);
    }

    // v2 files may still contain "autoFill": it is ignored. The shelves show exactly the authored plan.
    const TArray<TSharedPtr<FJsonValue>>* Placements = nullptr;
    if (!Root->TryGetArrayField(TEXT("placements"), Placements)) return true;
    for (int32 Index = 0; Index < Placements->Num(); ++Index)
    {
        const TSharedPtr<FJsonObject> Obj = (*Placements)[Index]->AsObject();
        FPlanogramPlacement Placement;
        // The same product may appear in several blocks (schema v3).
        if (!Obj.IsValid() || !Obj->TryGetStringField(TEXT("productId"), Placement.ProductId) ||
            !Obj->TryGetStringField(TEXT("fixtureId"), Placement.FixtureId) || !FixtureIds.Contains(Placement.FixtureId))
        {
            OutErrors.Add(FString::Printf(TEXT("Placement #%d: urun id yok veya fixture yok."), Index + 1));
            continue;
        }
        double X = 0.0, Gap = 0.0;
        Placement.bHasX = Obj->TryGetNumberField(TEXT("x"), X);
        Placement.XCm = FMath::Clamp(static_cast<float>(X), -500.f, 500.f);
        Obj->TryGetNumberField(TEXT("gap"), Gap);
        Placement.GapCm = FMath::Clamp(static_cast<float>(Gap), 0.f, MaxBlockGapCm);
        Obj->TryGetStringField(TEXT("face"), Placement.Face);
        Obj->TryGetNumberField(TEXT("level"), Placement.Level);
        Obj->TryGetNumberField(TEXT("facings"), Placement.Facings);
        Obj->TryGetNumberField(TEXT("depth"), Placement.Depth);
        Obj->TryGetNumberField(TEXT("order"), Placement.Order);
        Obj->TryGetNumberField(TEXT("offsetCm"), Placement.OffsetCm);
        Obj->TryGetNumberField(TEXT("orientation"), Placement.Orientation);
        Obj->TryGetNumberField(TEXT("stack"), Placement.Stack);
        Placement.Face = Placement.Face == TEXT("back") ? TEXT("back") : TEXT("front");
        Placement.Level = FMath::Clamp(Placement.Level, 0, MaxLevels - 1);
        Placement.Facings = FMath::Clamp(Placement.Facings, 1, MaxFacings);
        Placement.Depth = FMath::Clamp(Placement.Depth, 1, MaxDepth);
        Placement.OffsetCm = FMath::Clamp(Placement.OffsetCm, -500.f, 500.f);
        Placement.Orientation = FMath::Clamp(Placement.Orientation, 0, 2);
        Placement.Stack = FMath::Clamp(Placement.Stack, 1, 8);
        OutPlanogram.Placements.Add(Placement);
    }
    return true;
}

FString MarketPlanogram::Serialize(const FMarketPlanogram& Planogram)
{
    FString Out = FString::Printf(TEXT("{\n  \"schemaVersion\": %d,\n  \"fixtures\": [\n"), SchemaVersion);
    for (int32 I = 0; I < Planogram.Fixtures.Num(); ++I)
    {
        const FPlanogramFixture& F = Planogram.Fixtures[I];
        Out += FString::Printf(TEXT("    {\"id\":%s,\"equipment\":%s,\"label\":%s,\"category\":%s,\"x\":%.1f,\"y\":%.1f,\"z\":%.1f,\"yaw\":%.1f}%s\n"),
            *MarketCatalog::JsonQuote(F.Id), *MarketCatalog::JsonQuote(F.EquipmentId), *MarketCatalog::JsonQuote(F.Label), *MarketCatalog::JsonQuote(F.Category),
            F.Location.X, F.Location.Y, F.Location.Z, F.Yaw, I + 1 < Planogram.Fixtures.Num() ? TEXT(",") : TEXT(""));
    }
    Out += TEXT("  ],\n  \"placements\": [\n");
    TArray<const FPlanogramPlacement*> Saved;
    for (const FPlanogramPlacement& P : Planogram.Placements) Saved.Add(&P);
    for (int32 I = 0; I < Saved.Num(); ++I)
    {
        const FPlanogramPlacement& P = *Saved[I];
        // v3: every positioned block stores its centre "x"; an unresolved legacy block keeps order/offset.
        FString Position = P.bHasX ? FString::Printf(TEXT("\"x\":%.1f"), P.XCm)
                                   : FString::Printf(TEXT("\"order\":%d,\"offsetCm\":%.1f"), P.Order, P.OffsetCm);
        if (P.GapCm > 0.f) Position += FString::Printf(TEXT(",\"gap\":%.1f"), P.GapCm);
        Out += FString::Printf(TEXT("    {\"productId\":%s,\"fixtureId\":%s,\"face\":%s,\"level\":%d,%s,\"facings\":%d,\"depth\":%d,\"orientation\":%d,\"stack\":%d}%s\n"),
            *MarketCatalog::JsonQuote(P.ProductId), *MarketCatalog::JsonQuote(P.FixtureId), *MarketCatalog::JsonQuote(P.Face), P.Level, *Position, P.Facings, P.Depth, P.Orientation, P.Stack,
            I + 1 < Saved.Num() ? TEXT(",") : TEXT(""));
    }
    return Out + TEXT("  ]\n}\n");
}

bool MarketPlanogram::LoadFile(const FString& Path, FMarketPlanogram& OutPlanogram, TArray<FString>& OutErrors)
{
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *Path))
    {
        OutErrors.Add(TEXT("planograms.json bulunamadi: ") + Path);
        return false;
    }
    return Parse(Json, OutPlanogram, OutErrors);
}

bool MarketPlanogram::SaveFile(const FString& Path, const FMarketPlanogram& Planogram, FString& OutError)
{
    const FString Temp = Path + TEXT(".tmp");
    if (!FFileHelper::SaveStringToFile(Serialize(Planogram), *Temp, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
    {
        OutError = TEXT("Gecici planogram dosyasi yazilamadi.");
        return false;
    }
    IFileManager::Get().Copy(*(Path + TEXT(".bak")), *Path, true);
    if (!IFileManager::Get().Move(*Path, *Temp, true, true))
    {
        OutError = TEXT("planograms.json degistirilemedi.");
        return false;
    }
    return true;
}

FPlanogramEquipment MarketPlanogram::Equipment(const FString& EquipmentId)
{
    FPlanogramEquipment E; // gondola_double_1200 (Tools/Blender/create_gondola_shelf.py)
    E.Id = EquipmentId;
    E.MeshPath = TEXT("/Game/Environment/Shelves/Gondola_1200/SM_Gondola_1200.SM_Gondola_1200");
    E.SignZ = 171.f;
    if (EquipmentId == TEXT("wall_shelf_2400"))
    {
        // Tools/Blender/create_store_kit.py: 2.35 m boards at z .14/.50/.86/1.22/1.58 (+1.4 cm half
        // thickness); the 6th board sits under the header and is left empty. Price rails at y -.255 / -.225.
        E.MeshPath = TEXT("/Game/Environment/StoreKit/WallShelf_2400/SM_WallShelf_2400.SM_WallShelf_2400");
        E.UsableWidthCm = 235.f; // full 2.35 m board; the posts stand behind the products
        E.Levels = 5;
        const float Tops[] = { 15.4f, 51.4f, 87.4f, 123.4f, 159.4f };
        const float Rails[] = { 27.f, 27.f, 24.f, 24.f, 24.f };
        for (int32 I = 0; I < 5; ++I) { E.LevelTopZ[I] = Tops[I]; E.RailFrontY[I] = Rails[I]; }
        E.FrontY = -21.f;
        E.MeshYaw = 180.f;
        E.UsableDepthCm = 37.f;
        E.bDoubleSided = false;
        E.RailAboveTopZ = -1.8f;
        for (int32 I = 0; I < 5; ++I) E.LevelClearanceCm[I] = I < 4 ? 31.f : 38.f;
        E.bSignOnTop = false;
        E.SignZ = 213.f;
        E.SignY = 11.2f;
        E.SignWidthCm = 200.f;
        return E;
    }
    E.bDoubleSided = EquipmentId.IsEmpty() || EquipmentId.Contains(TEXT("double"));
    return E;
}

FPlanogramEquipment MarketPlanogram::EquipmentFor(const FMarketPlanogram& Planogram, const FString& FixtureId)
{
    const FPlanogramFixture* Fixture = Planogram.FindFixture(FixtureId);
    return Equipment(Fixture ? Fixture->EquipmentId : FString(TEXT("gondola_double_1200")));
}

float MarketPlanogram::BlockWidthCm(const FMarketProduct& Product, const FPlanogramPlacement& Placement)
{
    return FMath::Max(1, Placement.Facings) * (OrientedWidthCm(Product, Placement.Orientation) + ItemGapCm) - ItemGapCm;
}

float MarketPlanogram::LevelUsedWidthCm(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
    const FString& FixtureId, const FString& Face, int32 Level)
{
    float Total = 0.f;
    int32 Blocks = 0;
    for (const FPlanogramPlacement& Placement : Planogram.Placements)
    {
        if (Placement.FixtureId != FixtureId || Placement.Face != Face || Placement.Level != Level) continue;
        if (const FMarketProduct* Product = MarketCatalog::FindProduct(Products, Placement.ProductId))
        {
            Total += BlockWidthCm(*Product, Placement);
            ++Blocks;
        }
    }
    return Total + FMath::Max(0, Blocks - 1) * ProductGapCm;
}

void MarketPlanogram::FindOverflows(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, TArray<FString>& OutWarnings)
{
    const TCHAR* Faces[] = { TEXT("front"), TEXT("back") };
    TSet<FString> CrowdedRows; // rows whose blocks cannot fit at all: reported once, not per block
    for (const FPlanogramFixture& Fixture : Planogram.Fixtures)
    {
        const FPlanogramEquipment Spec = Equipment(Fixture.EquipmentId);
        for (const TCHAR* Face : Faces)
            for (int32 Level = 0; Level < MaxLevels; ++Level)
            {
                const float Width = LevelUsedWidthCm(Planogram, Products, Fixture.Id, Face, Level);
                if (Width <= 0.f) continue;
                const FString Key = FString::Printf(TEXT("%s|%s|%d"), *Fixture.Id, Face, Level);
                if (Level >= Spec.Levels)
                {
                    OutWarnings.Add(FString::Printf(TEXT("%s: seviye %d bu ekipmanda yok (%d seviye)."), *Fixture.Id, Level + 1, Spec.Levels));
                    CrowdedRows.Add(Key);
                }
                else if (Width > Spec.UsableWidthCm + KINDA_SMALL_NUMBER)
                {
                    OutWarnings.Add(FString::Printf(TEXT("%s / %s yuz / seviye %d: %.1f cm dolu, raf %.1f cm."),
                        *Fixture.Id, FCString::Strcmp(Face, TEXT("back")) == 0 ? TEXT("arka") : TEXT("on"), Level + 1, Width, Spec.UsableWidthCm));
                    CrowdedRows.Add(Key);
                }
            }
    }
    // Per-block checks run once for the whole plan (not once per fixture).
    for (int32 Index = 0; Index < Planogram.Placements.Num(); ++Index)
    {
        const FPlanogramPlacement& Placement = Planogram.Placements[Index];
        const FMarketProduct* Product = MarketCatalog::FindProduct(Products, Placement.ProductId);
        if (!Product) continue;
        if (Placement.Orientation == 2 && !CanLayOnSide(*Product))
            OutWarnings.Add(Placement.ProductId + TEXT(": bu ambalaj yan yatirilamaz."));
        const int32 MaxStack = EffectiveStack(Planogram, *Product, Placement);
        if (Placement.Stack > MaxStack)
            OutWarnings.Add(FString::Printf(TEXT("%s: %d kat sigmiyor; en fazla %d."), *Placement.ProductId, Placement.Stack, MaxStack));
        if (CrowdedRows.Contains(FString::Printf(TEXT("%s|%s|%d"), *Placement.FixtureId, *Placement.Face, Placement.Level))) continue;
        FString Reason;
        if (!BlockFits(Planogram, Products, Index, &Reason))
            OutWarnings.Add(Placement.ProductId + TEXT(": ") + Reason);
    }
}

float MarketPlanogram::NominalWidthCm(const FMarketProduct& Product)
{
    const float Millimeters = Product.WidthMm > 0 ? Product.WidthMm : (Product.DiameterMm > 0 ? Product.DiameterMm : 70.f);
    // Catalog dimensions describe the final real package. VisualScale only repairs imported
    // source units and must not shrink the planogram footprint a second time.
    return FMath::Max(2.f, Millimeters / 10.f);
}

float MarketPlanogram::NominalDepthCm(const FMarketProduct& Product)
{
    const float Millimeters = Product.DepthMm > 0 ? Product.DepthMm : (Product.DiameterMm > 0 ? Product.DiameterMm : 70.f);
    return FMath::Max(2.f, Millimeters / 10.f);
}

float MarketPlanogram::NominalHeightCm(const FMarketProduct& Product)
{
    return FMath::Max(2.f, (Product.HeightMm > 0 ? Product.HeightMm : 200.f) / 10.f);
}

float MarketPlanogram::OrientedWidthCm(const FMarketProduct& Product, int32 Orientation)
{
    if (Orientation == 1) return NominalDepthCm(Product);
    if (Orientation == 2) return NominalHeightCm(Product);
    return NominalWidthCm(Product);
}

float MarketPlanogram::OrientedDepthCm(const FMarketProduct& Product, int32 Orientation)
{
    return Orientation == 1 ? NominalWidthCm(Product) : NominalDepthCm(Product);
}

float MarketPlanogram::OrientedHeightCm(const FMarketProduct& Product, int32 Orientation)
{
    return Orientation == 2 ? NominalWidthCm(Product) : NominalHeightCm(Product);
}

bool MarketPlanogram::CanLayOnSide(const FMarketProduct& Product)
{
    return Product.PackageType == TEXT("kutu") || Product.PackageType == TEXT("poset");
}

int32 MarketPlanogram::MaxStackFor(const FMarketProduct& Product, int32 Orientation, float ClearanceCm)
{
    const bool bStable = Product.PackageType == TEXT("kutu") || Product.PackageType == TEXT("poset") ||
        Product.PackageType == TEXT("teneke") || Product.PackageType == TEXT("kase");
    if (!bStable || (Orientation == 2 && !CanLayOnSide(Product))) return 1;
    constexpr float StackGapCm = .6f;
    return FMath::Clamp(FMath::FloorToInt((ClearanceCm + StackGapCm) /
        FMath::Max(1.f, OrientedHeightCm(Product, Orientation) + StackGapCm)), 1, 8);
}

int32 MarketPlanogram::PhysicalDepth(const FMarketPlanogram& Planogram, const FMarketProduct& Product, const FPlanogramPlacement& Placement)
{
    return FMath::Clamp(FMath::FloorToInt((EquipmentFor(Planogram, Placement.FixtureId).UsableDepthCm + RowGapCm) /
        (OrientedDepthCm(Product, Placement.Orientation) + RowGapCm)), 1, MaxDepth);
}

int32 MarketPlanogram::Capacity(const FPlanogramPlacement& Placement)
{
    return FMath::Max(1, Placement.Facings) * FMath::Max(1, Placement.Depth) * FMath::Max(1, Placement.Stack);
}

int32 MarketPlanogram::EffectiveStack(const FMarketPlanogram& Planogram, const FMarketProduct& Product, const FPlanogramPlacement& Placement)
{
    const FPlanogramEquipment Spec = EquipmentFor(Planogram, Placement.FixtureId);
    const int32 Level = FMath::Clamp(Placement.Level, 0, FMath::Max(0, Spec.Levels - 1));
    return FMath::Clamp(Placement.Stack, 1, MaxStackFor(Product, Placement.Orientation, Spec.LevelClearanceCm[Level]));
}

int32 MarketPlanogram::ProductCapacity(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FString& ProductId)
{
    const FMarketProduct* Product = MarketCatalog::FindProduct(Products, ProductId);
    if (!Product) return 0;
    int32 Total = 0; // 0 = product is not on any shelf
    for (const FPlanogramPlacement& Placement : Planogram.Placements)
        if (Placement.ProductId == ProductId)
            Total += FMath::Max(1, Placement.Facings) * FMath::Max(1, Placement.Depth) * EffectiveStack(Planogram, *Product, Placement);
    return Total;
}

void MarketPlanogram::FitDepth(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products)
{
    for (FPlanogramPlacement& Placement : Planogram.Placements)
        if (const FMarketProduct* Product = MarketCatalog::FindProduct(Products, Placement.ProductId))
            Placement.Depth = PhysicalDepth(Planogram, *Product, Placement);
}

float MarketPlanogram::PlacementCenterX(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FPlanogramPlacement& Placement)
{
    if (Placement.bHasX) return Placement.XCm;
    // Legacy packing of the old schema: unresolved blocks of the row, centred, in Order.
    TArray<const FPlanogramPlacement*> Group;
    for (const FPlanogramPlacement& Candidate : Planogram.Placements)
        if (!Candidate.bHasX && Candidate.FixtureId == Placement.FixtureId && Candidate.Face == Placement.Face &&
            Candidate.Level == Placement.Level && MarketCatalog::FindProduct(Products, Candidate.ProductId))
            Group.Add(&Candidate);
    Group.StableSort([](const FPlanogramPlacement& A, const FPlanogramPlacement& B) { return A.Order < B.Order; });

    TArray<float> Spans;
    float Total = 0.f;
    for (const FPlanogramPlacement* Candidate : Group)
    {
        const float Span = BlockWidthCm(*MarketCatalog::FindProduct(Products, Candidate->ProductId), *Candidate);
        Spans.Add(Span); Total += Span;
    }
    Total += FMath::Max(0, Group.Num() - 1) * ProductGapCm;
    float Cursor = -Total * 0.5f;
    for (int32 I = 0; I < Group.Num(); ++I)
    {
        const float Center = Cursor + Spans[I] * 0.5f;
        if (Group[I] == &Placement) return Center + Placement.OffsetCm;
        Cursor += Spans[I] + ProductGapCm;
    }
    return Placement.OffsetCm;
}

void MarketPlanogram::ResolvePositions(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products)
{
    // Compute every legacy centre first (they depend on each other), then store them.
    TArray<float> Centers;
    Centers.SetNum(Planogram.Placements.Num());
    for (int32 I = 0; I < Planogram.Placements.Num(); ++I)
        Centers[I] = PlacementCenterX(Planogram, Products, Planogram.Placements[I]);
    for (int32 I = 0; I < Planogram.Placements.Num(); ++I)
    {
        FPlanogramPlacement& P = Planogram.Placements[I];
        if (P.bHasX) continue;
        P.XCm = FMath::RoundToFloat(Centers[I] * 10.f) / 10.f;
        P.bHasX = true;
        P.OffsetCm = 0.f;
    }
}

TArray<int32> MarketPlanogram::RowBlocks(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
    const FString& FixtureId, const FString& Face, int32 Level)
{
    TArray<int32> Row;
    TArray<float> Centers;
    for (int32 I = 0; I < Planogram.Placements.Num(); ++I)
    {
        const FPlanogramPlacement& P = Planogram.Placements[I];
        if (P.FixtureId == FixtureId && P.Face == Face && P.Level == Level) Row.Add(I);
    }
    Centers.SetNum(Planogram.Placements.Num());
    for (const int32 I : Row) Centers[I] = PlacementCenterX(Planogram, Products, Planogram.Placements[I]);
    Row.StableSort([&](int32 A, int32 B) { return Centers[A] < Centers[B]; });
    return Row;
}

int32 MarketPlanogram::BlockAt(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
    const FString& FixtureId, const FString& Face, int32 Level, float X, float ToleranceCm)
{
    int32 Best = INDEX_NONE;
    float BestDistance = TNumericLimits<float>::Max();
    for (const int32 I : RowBlocks(Planogram, Products, FixtureId, Face, Level))
    {
        const FPlanogramPlacement& P = Planogram.Placements[I];
        const FMarketProduct* Product = MarketCatalog::FindProduct(Products, P.ProductId);
        if (!Product) continue;
        const float Center = PlacementCenterX(Planogram, Products, P);
        const float Half = BlockWidthCm(*Product, P) * .5f;
        const float Distance = FMath::Abs(X - Center);
        if (Distance <= Half + ToleranceCm && Distance < BestDistance) { BestDistance = Distance; Best = I; }
    }
    return Best;
}

namespace
{
    struct FShelfSpan { float Left; float Right; };

    // Occupied intervals of a row. Each block is widened on both sides by the tolerance plus the larger of
    // its own gap and the gap wanted by the block being placed (OwnGap).
    TArray<FShelfSpan> OccupiedSpans(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
        const FString& FixtureId, const FString& Face, int32 Level, int32 IgnoreIndex, float OwnGap)
    {
        TArray<FShelfSpan> Spans;
        for (int32 I = 0; I < Planogram.Placements.Num(); ++I)
        {
            const FPlanogramPlacement& P = Planogram.Placements[I];
            if (I == IgnoreIndex || P.FixtureId != FixtureId || P.Face != Face || P.Level != Level) continue;
            const FMarketProduct* Product = MarketCatalog::FindProduct(Products, P.ProductId);
            if (!Product) continue;
            const float Center = MarketPlanogram::PlacementCenterX(Planogram, Products, P);
            const float Half = MarketPlanogram::BlockWidthCm(*Product, P) * .5f;
            const float Pad = MarketPlanogram::ProductGapCm + FMath::Max(OwnGap, P.GapCm);
            Spans.Add({ Center - Half - Pad, Center + Half + Pad });
        }
        Spans.Sort([](const FShelfSpan& A, const FShelfSpan& B) { return A.Left < B.Left; });
        return Spans;
    }

    bool CenterIsFree(const TArray<FShelfSpan>& Spans, float Center, float Half, float ShelfHalf)
    {
        constexpr float Eps = 0.05f;
        if (Center - Half < -ShelfHalf - Eps || Center + Half > ShelfHalf + Eps) return false;
        for (const FShelfSpan& Span : Spans)
            if (Center + Half > Span.Left + Eps && Center - Half < Span.Right - Eps) return false;
        return true;
    }
}

bool MarketPlanogram::FindFreeX(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
    const FString& FixtureId, const FString& Face, int32 Level, float WidthCm, float DesiredX, int32 IgnoreIndex, float& OutX, float GapCm)
{
    const float ShelfHalf = EquipmentFor(Planogram, FixtureId).UsableWidthCm * .5f;
    const float Half = WidthCm * .5f;
    if (Half > ShelfHalf + 0.05f) return false;
    const TArray<FShelfSpan> Spans = OccupiedSpans(Planogram, Products, FixtureId, Face, Level, IgnoreIndex, GapCm);
    // Candidates: the wish itself (kept inside the shelf), both shelf edges and both sides of every neighbour.
    TArray<float> Candidates = { FMath::Clamp(DesiredX, -ShelfHalf + Half, ShelfHalf - Half), -ShelfHalf + Half, ShelfHalf - Half };
    for (const FShelfSpan& Span : Spans) { Candidates.Add(Span.Left - Half); Candidates.Add(Span.Right + Half); }
    bool bFound = false;
    float Best = 0.f;
    for (const float Candidate : Candidates)
    {
        if (!CenterIsFree(Spans, Candidate, Half, ShelfHalf)) continue;
        if (!bFound || FMath::Abs(Candidate - DesiredX) < FMath::Abs(Best - DesiredX)) { Best = Candidate; bFound = true; }
    }
    if (bFound) OutX = FMath::RoundToFloat(Best * 10.f) / 10.f;
    return bFound;
}

float MarketPlanogram::WidestGapCm(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
    const FString& FixtureId, const FString& Face, int32 Level, int32 IgnoreIndex)
{
    const float ShelfHalf = EquipmentFor(Planogram, FixtureId).UsableWidthCm * .5f;
    const TArray<FShelfSpan> Spans = OccupiedSpans(Planogram, Products, FixtureId, Face, Level, IgnoreIndex, 0.f);
    float Cursor = -ShelfHalf, Widest = 0.f;
    for (const FShelfSpan& Span : Spans)
    {
        Widest = FMath::Max(Widest, Span.Left - Cursor);
        Cursor = FMath::Max(Cursor, Span.Right);
    }
    return FMath::Max(Widest, ShelfHalf - Cursor);
}

bool MarketPlanogram::BlockFits(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index, FString* OutReason)
{
    if (!Planogram.Placements.IsValidIndex(Index)) return false;
    const FPlanogramPlacement& P = Planogram.Placements[Index];
    const FPlanogramFixture* Fixture = Planogram.FindFixture(P.FixtureId);
    if (!Fixture) { if (OutReason) *OutReason = TEXT("Reyon bulunamadi."); return false; }
    const FPlanogramEquipment Spec = Equipment(Fixture->EquipmentId);
    if (P.Level < 0 || P.Level >= Spec.Levels) { if (OutReason) *OutReason = FString::Printf(TEXT("Bu reyonda %d seviye var."), Spec.Levels); return false; }
    if (P.Face == TEXT("back") && !Spec.bDoubleSided) { if (OutReason) *OutReason = TEXT("Bu reyonun arka yuzu yok."); return false; }
    const FMarketProduct* Product = MarketCatalog::FindProduct(Products, P.ProductId);
    if (!Product) return true; // unknown (inactive) product: kept, not drawn
    const float Half = BlockWidthCm(*Product, P) * .5f;
    const float Center = PlacementCenterX(Planogram, Products, P);
    const float ShelfHalf = Spec.UsableWidthCm * .5f;
    if (Center - Half < -ShelfHalf - 0.05f || Center + Half > ShelfHalf + 0.05f)
    {
        if (OutReason) *OutReason = TEXT("Urun blogu raf kenarini asiyor.");
        return false;
    }
    const TArray<FShelfSpan> Spans = OccupiedSpans(Planogram, Products, P.FixtureId, P.Face, P.Level, Index, P.GapCm);
    if (!CenterIsFree(Spans, Center, Half, ShelfHalf))
    {
        if (OutReason) *OutReason = TEXT("Urun blogu komsu blokla cakisiyor.");
        return false;
    }
    return true;
}

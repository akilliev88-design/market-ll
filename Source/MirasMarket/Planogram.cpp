#include "Planogram.h"

#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ProductCatalog.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
    FString Quote(const FString& Value)
    {
        FString Out = TEXT("\"");
        for (const TCHAR C : Value)
        {
            if (C == TEXT('"')) Out += TEXT("\\\"");
            else if (C == TEXT('\\')) Out += TEXT("\\\\");
            else if (C == TEXT('\n')) Out += TEXT("\\n");
            else Out.AppendChar(C);
        }
        return Out + TEXT("\"");
    }

    const FMarketProduct* FindProduct(const TArray<FMarketProduct>& Products, const FString& Id)
    {
        return Products.FindByPredicate([&](const FMarketProduct& Product) { return Product.Id == Id; });
    }

    bool ValidFixtureId(const FString& Id)
    {
        return MarketCatalog::IsValidId(Id);
    }
}

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
        if (!Obj.IsValid() || !Obj->TryGetStringField(TEXT("id"), Fixture.Id) || !ValidFixtureId(Fixture.Id) || FixtureIds.Contains(Fixture.Id))
        {
            OutErrors.Add(FString::Printf(TEXT("Fixture #%d: gecersiz veya tekrar eden id."), Index + 1));
            continue;
        }
        FixtureIds.Add(Fixture.Id);
        Obj->TryGetStringField(TEXT("equipment"), Fixture.EquipmentId);
        Obj->TryGetStringField(TEXT("label"), Fixture.Label);
        Obj->TryGetStringField(TEXT("category"), Fixture.Category);
        Obj->TryGetStringField(TEXT("strategy"), Fixture.Strategy);
        double X = 0, Y = 0, Z = 0, Yaw = 0;
        Obj->TryGetNumberField(TEXT("x"), X); Obj->TryGetNumberField(TEXT("y"), Y);
        Obj->TryGetNumberField(TEXT("z"), Z); Obj->TryGetNumberField(TEXT("yaw"), Yaw);
        Fixture.Location = FVector(X, Y, Z);
        Fixture.Yaw = Yaw;
        OutPlanogram.Fixtures.Add(Fixture);
    }

    Root->TryGetBoolField(TEXT("autoFill"), OutPlanogram.bAutoFill);
    const TArray<TSharedPtr<FJsonValue>>* Placements = nullptr;
    if (!Root->TryGetArrayField(TEXT("placements"), Placements)) return true;
    TSet<FString> ProductIds;
    for (int32 Index = 0; Index < Placements->Num(); ++Index)
    {
        const TSharedPtr<FJsonObject> Obj = (*Placements)[Index]->AsObject();
        FPlanogramPlacement Placement;
        if (!Obj.IsValid() || !Obj->TryGetStringField(TEXT("productId"), Placement.ProductId) || ProductIds.Contains(Placement.ProductId) ||
            !Obj->TryGetStringField(TEXT("fixtureId"), Placement.FixtureId) || !FixtureIds.Contains(Placement.FixtureId))
        {
            OutErrors.Add(FString::Printf(TEXT("Placement #%d: urun tekrar ediyor veya fixture yok."), Index + 1));
            continue;
        }
        ProductIds.Add(Placement.ProductId);
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
    FString Out = FString::Printf(TEXT("{\n  \"schemaVersion\": %d,\n  \"autoFill\": %s,\n  \"fixtures\": [\n"), SchemaVersion, Planogram.bAutoFill ? TEXT("true") : TEXT("false"));
    for (int32 I = 0; I < Planogram.Fixtures.Num(); ++I)
    {
        const FPlanogramFixture& F = Planogram.Fixtures[I];
        Out += FString::Printf(TEXT("    {\"id\":%s,\"equipment\":%s,\"label\":%s,\"category\":%s,\"strategy\":%s,\"x\":%.1f,\"y\":%.1f,\"z\":%.1f,\"yaw\":%.1f}%s\n"),
            *Quote(F.Id), *Quote(F.EquipmentId), *Quote(F.Label), *Quote(F.Category), *Quote(F.Strategy),
            F.Location.X, F.Location.Y, F.Location.Z, F.Yaw, I + 1 < Planogram.Fixtures.Num() ? TEXT(",") : TEXT(""));
    }
    Out += TEXT("  ],\n  \"placements\": [\n");
    TArray<const FPlanogramPlacement*> Saved; // runtime extra blocks are never written
    for (const FPlanogramPlacement& P : Planogram.Placements) if (!P.bExtra) Saved.Add(&P);
    for (int32 I = 0; I < Saved.Num(); ++I)
    {
        const FPlanogramPlacement& P = *Saved[I];
        Out += FString::Printf(TEXT("    {\"productId\":%s,\"fixtureId\":%s,\"face\":%s,\"level\":%d,\"facings\":%d,\"depth\":%d,\"order\":%d,\"offsetCm\":%.1f,\"orientation\":%d,\"stack\":%d}%s\n"),
            *Quote(P.ProductId), *Quote(P.FixtureId), *Quote(P.Face), P.Level, P.Facings, P.Depth, P.Order, P.OffsetCm, P.Orientation, P.Stack,
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

void MarketPlanogram::Reconcile(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products)
{
    if (Planogram.Fixtures.Num() == 0)
    {
        FPlanogramFixture Fixture;
        Fixture.Id = TEXT("gondola_auto_1"); Fixture.Label = TEXT("Yeni Reyon"); Fixture.Location = FVector(0, 320, 0);
        Planogram.Fixtures.Add(Fixture);
    }
    int32 NextOrder = 0;
    for (const FPlanogramPlacement& Existing : Planogram.Placements) NextOrder = FMath::Max(NextOrder, Existing.Order + 1);
    for (const FMarketProduct& Product : Products)
    {
        if (Planogram.FindPlacement(Product.Id)) continue;
        FPlanogramPlacement Placement;
        Placement.ProductId = Product.Id; Placement.Facings = 2; Placement.Depth = 3; Placement.Order = NextOrder++;

        // Category fixtures first, then every other fixture in file order. Only levels with room are used.
        TArray<int32> Candidates;
        for (int32 I = 0; I < Planogram.Fixtures.Num(); ++I)
            if (!Product.Category.IsEmpty() && Planogram.Fixtures[I].Category == Product.Category) Candidates.Add(I);
        for (int32 I = 0; I < Planogram.Fixtures.Num(); ++I) Candidates.AddUnique(I);

        bool bPlaced = false;
        for (const int32 FixtureIndex : Candidates)
            if (PlaceOnFixture(Planogram, Products, Planogram.Fixtures[FixtureIndex], Placement, 0)) { bPlaced = true; break; }
        if (!bPlaced)
        {
            // The store is full. Keep the product visible with one facing on the emptiest level of its
            // best fixture; FindOverflows reports the overflow so the author can fix the plan.
            const FPlanogramFixture& Fixture = Planogram.Fixtures[Candidates[0]];
            Placement.FixtureId = Fixture.Id; Placement.Face = TEXT("front"); Placement.Facings = 1;
            float BestWidth = TNumericLimits<float>::Max();
            for (int32 Level = 0; Level < Equipment(Fixture.EquipmentId).Levels; ++Level)
            {
                const float Width = LevelUsedWidthCm(Planogram, Products, Fixture.Id, Placement.Face, Level);
                if (Width < BestWidth) { BestWidth = Width; Placement.Level = Level; }
            }
        }
        Planogram.Placements.Add(Placement);
    }
}

bool MarketPlanogram::IsDoubleSided(const FPlanogramFixture& Fixture)
{
    return Equipment(Fixture.EquipmentId).bDoubleSided;
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
        E.UsableWidthCm = 230.f;
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

float MarketPlanogram::BlockWidthCm(const FMarketProduct& Product, int32 Facings)
{
    return FMath::Max(1, Facings) * (NominalWidthCm(Product) + ItemGapCm) - ItemGapCm;
}

float MarketPlanogram::BlockWidthCm(const FMarketProduct& Product, const FPlanogramPlacement& Placement)
{
    return FMath::Max(1, Placement.Facings) * (OrientedWidthCm(Product, Placement.Orientation) + ItemGapCm) - ItemGapCm;
}

float MarketPlanogram::LevelUsedWidthCm(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
    const FString& FixtureId, const FString& Face, int32 Level, const FString& IgnoreProductId)
{
    float Total = 0.f;
    int32 Blocks = 0;
    for (const FPlanogramPlacement& Placement : Planogram.Placements)
    {
        if (Placement.FixtureId != FixtureId || Placement.Face != Face || Placement.Level != Level) continue;
        if (!IgnoreProductId.IsEmpty() && Placement.ProductId == IgnoreProductId) continue;
        if (const FMarketProduct* Product = FindProduct(Products, Placement.ProductId))
        {
            Total += BlockWidthCm(*Product, Placement);
            ++Blocks;
        }
    }
    return Total + FMath::Max(0, Blocks - 1) * ProductGapCm;
}

bool MarketPlanogram::FitsOnLevel(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
    const FString& FixtureId, const FString& Face, int32 Level, const FString& ProductId, int32 Facings, float* OutWidthCm)
{
    const float Others = LevelUsedWidthCm(Planogram, Products, FixtureId, Face, Level, ProductId);
    const FMarketProduct* Product = FindProduct(Products, ProductId);
    FPlanogramPlacement Candidate;
    if (const FPlanogramPlacement* Existing = Planogram.FindPlacement(ProductId)) Candidate = *Existing;
    Candidate.Facings = Facings;
    const float Width = Product ? Others + (Others > 0.f ? ProductGapCm : 0.f) + BlockWidthCm(*Product, Candidate) : Others;
    if (OutWidthCm) *OutWidthCm = Width;
    return Width <= EquipmentFor(Planogram, FixtureId).UsableWidthCm + KINDA_SMALL_NUMBER;
}

bool MarketPlanogram::PlaceOnFixture(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
    const FPlanogramFixture& Fixture, FPlanogramPlacement& Placement, int32 PreferredLevel)
{
    if (!FindProduct(Products, Placement.ProductId)) return false;
    TArray<FString> Faces = { TEXT("front") };
    if (IsDoubleSided(Fixture)) Faces.Add(TEXT("back"));
    const int32 Levels = Equipment(Fixture.EquipmentId).Levels;
    const int32 Start = FMath::Clamp(PreferredLevel, 0, Levels - 1);
    // Prefer keeping the facing count (using the back face if needed) before shrinking it.
    for (int32 Facings = FMath::Clamp(Placement.Facings, 1, MaxFacings); Facings >= 1; --Facings)
        for (const FString& Face : Faces)
            for (int32 Step = 0; Step < Levels; ++Step)
            {
                const int32 Level = (Start + Step) % Levels;
                if (!FitsOnLevel(Planogram, Products, Fixture.Id, Face, Level, Placement.ProductId, Facings)) continue;
                Placement.FixtureId = Fixture.Id; Placement.Face = Face; Placement.Level = Level; Placement.Facings = Facings;
                return true;
            }
    return false;
}

void MarketPlanogram::FindOverflows(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, TArray<FString>& OutWarnings)
{
    const TCHAR* Faces[] = { TEXT("front"), TEXT("back") };
    for (const FPlanogramFixture& Fixture : Planogram.Fixtures)
    {
        const FPlanogramEquipment Spec = Equipment(Fixture.EquipmentId);
        for (const TCHAR* Face : Faces)
            for (int32 Level = 0; Level < MaxLevels; ++Level)
            {
                const float Width = LevelUsedWidthCm(Planogram, Products, Fixture.Id, Face, Level);
                if (Width <= 0.f) continue;
                if (Level >= Spec.Levels)
                    OutWarnings.Add(FString::Printf(TEXT("%s: seviye %d bu ekipmanda yok (%d seviye)."), *Fixture.Id, Level + 1, Spec.Levels));
                else if (Width > Spec.UsableWidthCm + KINDA_SMALL_NUMBER)
                    OutWarnings.Add(FString::Printf(TEXT("%s / %s yuz / seviye %d: %.1f cm dolu, raf %.1f cm."),
                        *Fixture.Id, FCString::Strcmp(Face, TEXT("back")) == 0 ? TEXT("arka") : TEXT("on"), Level + 1, Width, Spec.UsableWidthCm));
            }
    for (const FPlanogramPlacement& Placement : Planogram.Placements)
    {
        const FMarketProduct* Product = FindProduct(Products, Placement.ProductId);
        if (!Product) continue;
        const FPlanogramEquipment PlacementSpec = EquipmentFor(Planogram, Placement.FixtureId);
        const int32 Level = FMath::Clamp(Placement.Level, 0, FMath::Max(0, PlacementSpec.Levels - 1));
        if (Placement.Orientation == 2 && !CanLayOnSide(*Product))
            OutWarnings.Add(Placement.ProductId + TEXT(": bu ambalaj yan yatirilamaz."));
        const int32 MaxStack = MaxStackFor(*Product, Placement.Orientation, PlacementSpec.LevelClearanceCm[Level]);
        if (Placement.Stack > MaxStack)
            OutWarnings.Add(FString::Printf(TEXT("%s: %d kat sigmiyor; en fazla %d."), *Placement.ProductId, Placement.Stack, MaxStack));
        if (!FMath::IsNearlyZero(Placement.OffsetCm))
        {
            FString Reason;
            if (!CanSetOffset(Planogram, Products, Placement, Placement.OffsetCm, &Reason))
                OutWarnings.Add(Placement.ProductId + TEXT(": ") + Reason);
        }
    }
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

int32 MarketPlanogram::DepthThatFits(const FMarketProduct& Product, float UsableDepth)
{
    return FMath::Clamp(FMath::FloorToInt((UsableDepth + ItemGapCm) / (NominalDepthCm(Product) + ItemGapCm)), 1, MaxDepth);
}

int32 MarketPlanogram::Capacity(const FPlanogramPlacement& Placement)
{
    return FMath::Max(1, Placement.Facings) * FMath::Max(1, Placement.Depth) * FMath::Max(1, Placement.Stack);
}

int32 MarketPlanogram::ProductCapacity(const FMarketPlanogram& Planogram, const FString& ProductId)
{
    int32 Total = 0;
    for (const FPlanogramPlacement& Placement : Planogram.Placements)
        if (Placement.ProductId == ProductId) Total += Capacity(Placement);
    return Total;
}

int32 MarketPlanogram::FillToCapacity(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, bool bUseEmptyLevels)
{
    if (bUseEmptyLevels)
    {
        int32 NextOrder = 0;
        for (const FPlanogramPlacement& Existing : Planogram.Placements) NextOrder = FMath::Max(NextOrder, Existing.Order + 1);
        for (const FPlanogramFixture& Fixture : Planogram.Fixtures)
        {
            // Products that belong on this fixture: authored here first, then same category (catalog order).
            TArray<FString> Candidates;
            TArray<const FPlanogramPlacement*> Here;
            for (const FPlanogramPlacement& Placement : Planogram.Placements)
                if (Placement.FixtureId == Fixture.Id && !Placement.bExtra && FindProduct(Products, Placement.ProductId)) Here.Add(&Placement);
            Here.Sort([](const FPlanogramPlacement& A, const FPlanogramPlacement& B)
                { return A.Level != B.Level ? A.Level < B.Level : A.Order < B.Order; });
            for (const FPlanogramPlacement* Placement : Here) Candidates.AddUnique(Placement->ProductId);
            for (const FMarketProduct& Product : Products)
                if (!Fixture.Category.IsEmpty() && Product.Category == Fixture.Category) Candidates.AddUnique(Product.Id);
            if (Candidates.Num() == 0) continue;

            const FPlanogramEquipment Spec = Equipment(Fixture.EquipmentId);
            TArray<FString> Faces = { TEXT("front") };
            if (Spec.bDoubleSided) Faces.Add(TEXT("back"));
            int32 Next = 0;
            for (const FString& Face : Faces)
                for (int32 Level = 0; Level < Spec.Levels; ++Level)
                {
                    if (LevelUsedWidthCm(Planogram, Products, Fixture.Id, Face, Level) > 0.f) continue;
                    FPlanogramPlacement Extra;
                    Extra.ProductId = Candidates[Next++ % Candidates.Num()];
                    Extra.FixtureId = Fixture.Id; Extra.Face = Face; Extra.Level = Level;
                    Extra.Facings = 1; Extra.Depth = 1; Extra.Order = NextOrder++; Extra.bExtra = true;
                    Planogram.Placements.Add(Extra);
                }
        }
    }

    for (FPlanogramPlacement& Placement : Planogram.Placements)
        if (const FMarketProduct* Product = FindProduct(Products, Placement.ProductId))
            Placement.Depth = FMath::Clamp(FMath::FloorToInt((EquipmentFor(Planogram, Placement.FixtureId).UsableDepthCm + ItemGapCm) /
                (OrientedDepthCm(*Product, Placement.Orientation) + ItemGapCm)), 1, MaxDepth); // physical fit

    int32 Added = 0;
    bool bGrew = true;
    while (bGrew)
    {
        bGrew = false;
        // One pass adds at most one facing per level: the block with the fewest facings grows first,
        // so neighbouring brands share the free width evenly.
        TSet<FString> LevelsDone;
        TArray<int32> Order;
        for (int32 I = 0; I < Planogram.Placements.Num(); ++I) Order.Add(I);
        Order.Sort([&](int32 A, int32 B)
        {
            const FPlanogramPlacement& PA = Planogram.Placements[A];
            const FPlanogramPlacement& PB = Planogram.Placements[B];
            return PA.Facings != PB.Facings ? PA.Facings < PB.Facings : PA.Order < PB.Order;
        });
        for (const int32 Index : Order)
        {
            FPlanogramPlacement& Placement = Planogram.Placements[Index];
            if (!FindProduct(Products, Placement.ProductId) || Placement.Facings >= MaxFacings) continue;
            const FString LevelKey = FString::Printf(TEXT("%s|%s|%d"), *Placement.FixtureId, *Placement.Face, Placement.Level);
            if (LevelsDone.Contains(LevelKey)) continue;
            if (!FitsOnLevel(Planogram, Products, Placement.FixtureId, Placement.Face, Placement.Level, Placement.ProductId, Placement.Facings + 1)) continue;
            ++Placement.Facings;
            ++Added;
            LevelsDone.Add(LevelKey);
            bGrew = true;
        }
    }
    return Added;
}

float MarketPlanogram::PlacementCenterX(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FPlanogramPlacement& Placement)
{
    TArray<const FPlanogramPlacement*> Group;
    for (const FPlanogramPlacement& Candidate : Planogram.Placements)
        if (Candidate.FixtureId == Placement.FixtureId && Candidate.Face == Placement.Face && Candidate.Level == Placement.Level && FindProduct(Products, Candidate.ProductId))
            Group.Add(&Candidate);
    Group.Sort([](const FPlanogramPlacement& A, const FPlanogramPlacement& B) { return A.Order < B.Order; });

    TArray<float> Spans;
    float Total = 0.f;
    for (const FPlanogramPlacement* Candidate : Group)
    {
        const FMarketProduct* Product = FindProduct(Products, Candidate->ProductId);
        const float Span = BlockWidthCm(*Product, *Candidate);
        Spans.Add(Span); Total += Span;
    }
    Total += FMath::Max(0, Group.Num() - 1) * ProductGapCm;
    float Cursor = -Total * 0.5f;
    for (int32 I = 0; I < Group.Num(); ++I)
    {
        const float Center = Cursor + Spans[I] * 0.5f;
        if (Group[I]->ProductId == Placement.ProductId) return Center + Placement.OffsetCm;
        Cursor += Spans[I] + ProductGapCm;
    }
    return 0.f;
}

bool MarketPlanogram::CanSetOffset(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
    const FPlanogramPlacement& Placement, float NewOffsetCm, FString* OutReason)
{
    const FMarketProduct* Product = FindProduct(Products, Placement.ProductId);
    if (!Product) return false;
    const FPlanogramEquipment Spec = EquipmentFor(Planogram, Placement.FixtureId);
    const float AutoCenter = PlacementCenterX(Planogram, Products, Placement) - Placement.OffsetCm;
    const float Center = AutoCenter + NewOffsetCm;
    const float Half = BlockWidthCm(*Product, Placement) * .5f;
    if (Center - Half < -Spec.UsableWidthCm * .5f - KINDA_SMALL_NUMBER || Center + Half > Spec.UsableWidthCm * .5f + KINDA_SMALL_NUMBER)
    {
        if (OutReason) *OutReason = TEXT("Urun blogu raf kenarini asiyor.");
        return false;
    }
    for (const FPlanogramPlacement& Other : Planogram.Placements)
    {
        if (&Other == &Placement || Other.ProductId == Placement.ProductId || Other.FixtureId != Placement.FixtureId ||
            Other.Face != Placement.Face || Other.Level != Placement.Level) continue;
        const FMarketProduct* OtherProduct = FindProduct(Products, Other.ProductId);
        if (!OtherProduct) continue;
        const float OtherCenter = PlacementCenterX(Planogram, Products, Other);
        const float Required = Half + BlockWidthCm(*OtherProduct, Other) * .5f + ProductGapCm;
        if (FMath::Abs(Center - OtherCenter) < Required - KINDA_SMALL_NUMBER)
        {
            if (OutReason) *OutReason = TEXT("Urun blogu komsu urunle cakisiyor.");
            return false;
        }
    }
    return true;
}

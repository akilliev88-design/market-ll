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
        Placement.Face = Placement.Face == TEXT("back") ? TEXT("back") : TEXT("front");
        Placement.Level = FMath::Clamp(Placement.Level, 0, 3);
        Placement.Facings = FMath::Clamp(Placement.Facings, 1, 12);
        Placement.Depth = FMath::Clamp(Placement.Depth, 1, 8);
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
        Out += FString::Printf(TEXT("    {\"id\":%s,\"equipment\":%s,\"label\":%s,\"category\":%s,\"strategy\":%s,\"x\":%.1f,\"y\":%.1f,\"z\":%.1f,\"yaw\":%.1f}%s\n"),
            *Quote(F.Id), *Quote(F.EquipmentId), *Quote(F.Label), *Quote(F.Category), *Quote(F.Strategy),
            F.Location.X, F.Location.Y, F.Location.Z, F.Yaw, I + 1 < Planogram.Fixtures.Num() ? TEXT(",") : TEXT(""));
    }
    Out += TEXT("  ],\n  \"placements\": [\n");
    for (int32 I = 0; I < Planogram.Placements.Num(); ++I)
    {
        const FPlanogramPlacement& P = Planogram.Placements[I];
        Out += FString::Printf(TEXT("    {\"productId\":%s,\"fixtureId\":%s,\"face\":%s,\"level\":%d,\"facings\":%d,\"depth\":%d,\"order\":%d}%s\n"),
            *Quote(P.ProductId), *Quote(P.FixtureId), *Quote(P.Face), P.Level, P.Facings, P.Depth, P.Order,
            I + 1 < Planogram.Placements.Num() ? TEXT(",") : TEXT(""));
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
    int32 NextOrder = Planogram.Placements.Num();
    for (const FMarketProduct& Product : Products)
    {
        if (Planogram.FindPlacement(Product.Id)) continue;
        const FPlanogramFixture* Fixture = Planogram.Fixtures.FindByPredicate([&](const FPlanogramFixture& F) { return !Product.Category.IsEmpty() && F.Category == Product.Category; });
        if (!Fixture) Fixture = &Planogram.Fixtures[NextOrder % Planogram.Fixtures.Num()];
        FPlanogramPlacement Placement;
        Placement.ProductId = Product.Id; Placement.FixtureId = Fixture->Id;
        Placement.Level = NextOrder % 4; Placement.Facings = 2; Placement.Depth = 3; Placement.Order = NextOrder++;
        Planogram.Placements.Add(Placement);
    }
}

float MarketPlanogram::NominalWidthCm(const FMarketProduct& Product)
{
    const float Millimeters = Product.WidthMm > 0 ? Product.WidthMm : (Product.DiameterMm > 0 ? Product.DiameterMm : 70.f);
    // Catalog dimensions describe the final real package. VisualScale only repairs imported
    // source units and must not shrink the planogram footprint a second time.
    return FMath::Max(2.f, Millimeters / 10.f);
}

float MarketPlanogram::PlacementCenterX(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FPlanogramPlacement& Placement)
{
    TArray<const FPlanogramPlacement*> Group;
    for (const FPlanogramPlacement& Candidate : Planogram.Placements)
        if (Candidate.FixtureId == Placement.FixtureId && Candidate.Face == Placement.Face && Candidate.Level == Placement.Level && FindProduct(Products, Candidate.ProductId))
            Group.Add(&Candidate);
    Group.Sort([](const FPlanogramPlacement& A, const FPlanogramPlacement& B) { return A.Order < B.Order; });

    constexpr float ItemGap = 2.f;
    constexpr float ProductGap = 3.f;
    TArray<float> Spans;
    float Total = 0.f;
    for (const FPlanogramPlacement* Candidate : Group)
    {
        const FMarketProduct* Product = FindProduct(Products, Candidate->ProductId);
        const float Span = Candidate->Facings * (NominalWidthCm(*Product) + ItemGap) - ItemGap;
        Spans.Add(Span); Total += Span;
    }
    Total += FMath::Max(0, Group.Num() - 1) * ProductGap;
    float Cursor = -Total * 0.5f;
    for (int32 I = 0; I < Group.Num(); ++I)
    {
        const float Center = Cursor + Spans[I] * 0.5f;
        if (Group[I]->ProductId == Placement.ProductId) return Center;
        Cursor += Spans[I] + ProductGap;
    }
    return 0.f;
}

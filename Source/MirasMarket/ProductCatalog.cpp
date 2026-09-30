#include "ProductCatalog.h"

FString MarketCatalog::UpperTurkish(const FString& Text)
{
    FString Result = Text;
    for (TCHAR& C : Result)
    {
        if (C == TEXT('i')) C = 0x0130;
        else if (C == 0x0131) C = TEXT('I');
        else if (C == 0x00e7) C = 0x00c7;
        else if (C == 0x011f) C = 0x011e;
        else if (C == 0x00f6) C = 0x00d6;
        else if (C == 0x015f) C = 0x015e;
        else if (C == 0x00fc) C = 0x00dc;
        else C = FChar::ToUpper(C);
    }
    return Result;
}
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

int32 MarketCatalog::CountActive(const TArray<FMarketProduct>& Products)
{
    int32 Count = 0;
    for (const FMarketProduct& P : Products) if (P.bActive) ++Count;
    return Count;
}

FString MarketCatalog::DefaultPath()
{
    return FPaths::ProjectConfigDir() / TEXT("products.json");
}

bool MarketCatalog::IsValidId(const FString& Id)
{
    if (Id.Len() < 3 || Id.Len() > 40 || !FChar::IsLower(Id[0])) return false;
    for (const TCHAR C : Id)
    {
        const bool bOk = (C >= TEXT('a') && C <= TEXT('z')) || (C >= TEXT('0') && C <= TEXT('9')) || C == TEXT('_');
        if (!bOk) return false;
    }
    return true;
}

FString MarketCatalog::MakeId(const FString& DisplayName)
{
    FString Out;
    bool bPendingSeparator = false;
    for (TCHAR C : DisplayName)
    {
        switch (C)
        {
        case 0x00E7: case 0x00C7: C = TEXT('c'); break; // c cedilla
        case 0x011F: case 0x011E: C = TEXT('g'); break; // soft g
        case 0x0131: case 0x0130: C = TEXT('i'); break; // dotless i / dotted I
        case 0x00F6: case 0x00D6: C = TEXT('o'); break;
        case 0x015F: case 0x015E: C = TEXT('s'); break;
        case 0x00FC: case 0x00DC: C = TEXT('u'); break;
        case 0x00E2: case 0x00C2: C = TEXT('a'); break;
        case 0x00EE: case 0x00CE: C = TEXT('i'); break;
        case 0x00FB: case 0x00DB: C = TEXT('u'); break;
        default: break;
        }
        if (C >= TEXT('A') && C <= TEXT('Z')) C = C - TEXT('A') + TEXT('a');
        const bool bAlnum = (C >= TEXT('a') && C <= TEXT('z')) || (C >= TEXT('0') && C <= TEXT('9'));
        if (!bAlnum)
        {
            bPendingSeparator = !Out.IsEmpty();
            continue;
        }
        if (bPendingSeparator) Out.AppendChar(TEXT('_'));
        bPendingSeparator = false;
        Out.AppendChar(C);
        if (Out.Len() >= 40) break;
    }
    if (Out.IsEmpty() || !FChar::IsLower(Out[0])) Out = TEXT("urun_") + Out;
    while (Out.Len() < 3) Out.AppendChar(TEXT('0'));
    return Out.Left(40);
}

FString MarketCatalog::ColorHex(const FColor& Color)
{
    return FString::Printf(TEXT("%02X%02X%02X"), Color.R, Color.G, Color.B);
}

FString MarketCatalog::Money(int64 Kurus)
{
    const TCHAR* Sign = Kurus < 0 ? TEXT("-") : TEXT("");
    const int64 Abs = Kurus < 0 ? -Kurus : Kurus;
    return FString::Printf(TEXT("%s%lld.%02lld"), Sign, Abs / 100, Abs % 100);
}

bool MarketCatalog::ParseMoney(const FString& Text, int64& OutKurus)
{
    FString Clean = Text.TrimStartAndEnd().Replace(TEXT("TL"), TEXT("")).Replace(TEXT(","), TEXT(".")).TrimStartAndEnd();
    if (Clean.IsEmpty() || !Clean.IsNumeric()) return false;
    const double Value = FCString::Atod(*Clean);
    if (!FMath::IsFinite(Value) || Value <= 0 || Value > 10000) return false;
    OutKurus = FMath::RoundToInt64(Value * 100.0);
    return OutKurus > 0;
}

bool MarketCatalog::Parse(const FString& Json, TArray<FMarketProduct>& OutProducts, TArray<FString>& OutErrors, FString* OutNote)
{
    OutProducts.Reset();
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid())
    {
        OutErrors.Add(TEXT("products.json okunamadi: gecersiz JSON."));
        return false;
    }
    if (OutNote) Root->TryGetStringField(TEXT("note"), *OutNote);
    const TArray<TSharedPtr<FJsonValue>>* Rows = nullptr;
    if (!Root->TryGetArrayField(TEXT("products"), Rows))
    {
        OutErrors.Add(TEXT("products.json icinde 'products' listesi yok."));
        return false;
    }
    TSet<FString> Ids;
    for (int32 Index = 0; Index < Rows->Num(); ++Index)
    {
        const TSharedPtr<FJsonObject> Obj = (*Rows)[Index]->AsObject();
        const FString Where = FString::Printf(TEXT("Urun #%d"), Index + 1);
        if (!Obj.IsValid()) { OutErrors.Add(Where + TEXT(": nesne degil.")); continue; }
        FMarketProduct P;
        double Cost = 0, Price = 0;
        if (!Obj->TryGetStringField(TEXT("id"), P.Id) || !IsValidId(P.Id)) { OutErrors.Add(Where + TEXT(": gecersiz id.")); continue; }
        if (Ids.Contains(P.Id)) { OutErrors.Add(Where + TEXT(": tekrar eden id ") + P.Id); continue; }
        if (!Obj->TryGetStringField(TEXT("realName"), P.RealName) || P.RealName.TrimStartAndEnd().IsEmpty()) { OutErrors.Add(Where + TEXT(": realName yok.")); continue; }
        if (!Obj->TryGetStringField(TEXT("fictionalName"), P.FictionalName) || P.FictionalName.TrimStartAndEnd().IsEmpty()) P.FictionalName = P.RealName;
        Obj->TryGetStringField(TEXT("category"), P.Category);
        if (!Obj->TryGetNumberField(TEXT("cost"), Cost) || !Obj->TryGetNumberField(TEXT("price"), Price) ||
            !FMath::IsFinite(Cost) || !FMath::IsFinite(Price) || Cost <= 0 || Price <= 0 || Cost > 10000 || Price > 10000)
        {
            OutErrors.Add(Where + TEXT(": maliyet/fiyat gecersiz.")); continue;
        }
        P.Cost = FMath::RoundToInt64(Cost * 100);
        P.BasePrice = FMath::RoundToInt64(Price * 100);
        int32 CaseUnits = 12;
        if (Obj->TryGetNumberField(TEXT("caseUnits"), CaseUnits)) P.CaseUnits = FMath::Clamp(CaseUnits, 1, 48);
        FString Color;
        P.Color = Obj->TryGetStringField(TEXT("color"), Color) ? FColor::FromHex(Color) : FColor(180, 180, 180);
        bool bActive = true;
        if (Obj->TryGetBoolField(TEXT("active"), bActive)) P.bActive = bActive;
        Obj->TryGetStringField(TEXT("brand"), P.Brand);
        const TSharedPtr<FJsonObject>* Retail = nullptr;
        if (Obj->TryGetObjectField(TEXT("retail"), Retail) && Retail && Retail->IsValid())
        {
            const auto ReadFloat = [&](const TCHAR* Key, float& Target, float Min, float Max)
            {
                double Number = 0;
                if ((*Retail)->TryGetNumberField(Key, Number) && FMath::IsFinite(Number)) Target = FMath::Clamp(static_cast<float>(Number), Min, Max);
            };
            (*Retail)->TryGetStringField(TEXT("subcategory"), P.Subcategory);
            ReadFloat(TEXT("kvi"), P.Kvi, 0.f, 1.f);
            ReadFloat(TEXT("vat"), P.VatRate, 0.f, 0.5f);
            ReadFloat(TEXT("elasticity"), P.Elasticity, 0.f, 10.f);
            ReadFloat(TEXT("stockpile"), P.Stockpile, 0.f, 3.f);
            ReadFloat(TEXT("trafficPull"), P.TrafficPull, 0.f, 1.f);
            int32 Life = 0;
            if ((*Retail)->TryGetNumberField(TEXT("shelfLifeDays"), Life)) P.ShelfLifeDays = FMath::Clamp(Life, 0, 3650);
        }
        const TSharedPtr<FJsonObject>* Pack = nullptr;
        if (Obj->TryGetObjectField(TEXT("package"), Pack) && Pack && Pack->IsValid())
        {
            (*Pack)->TryGetStringField(TEXT("type"), P.PackageType);
            (*Pack)->TryGetNumberField(TEXT("widthMm"), P.WidthMm);
            (*Pack)->TryGetNumberField(TEXT("depthMm"), P.DepthMm);
            (*Pack)->TryGetNumberField(TEXT("heightMm"), P.HeightMm);
            (*Pack)->TryGetNumberField(TEXT("diameterMm"), P.DiameterMm);
            (*Pack)->TryGetNumberField(TEXT("labelHeightMm"), P.LabelHeightMm);
            (*Pack)->TryGetStringField(TEXT("parts"), P.Parts);
            (*Pack)->TryGetStringField(TEXT("notes"), P.Notes);
            (*Pack)->TryGetStringField(TEXT("preset"), P.Preset);
            (*Pack)->TryGetStringField(TEXT("colors"), P.Colors);
            bool bEstimated = false;
            if ((*Pack)->TryGetBoolField(TEXT("estimated"), bEstimated)) P.bSizeEstimated = bEstimated;
        }
        const TSharedPtr<FJsonObject>* Visual = nullptr;
        if (Obj->TryGetObjectField(TEXT("visual"), Visual) && Visual && Visual->IsValid())
        {
            (*Visual)->TryGetStringField(TEXT("package"), P.MeshPath);
            const TArray<TSharedPtr<FJsonValue>>* Slots = nullptr;
            FString Single;
            if ((*Visual)->TryGetArrayField(TEXT("materials"), Slots))
            {
                for (const auto& Slot : *Slots) P.Materials.Add(Slot.IsValid() ? Slot->AsString() : FString());
            }
            else if ((*Visual)->TryGetStringField(TEXT("material"), Single)) // schema v2 draft: label on slot 0
            {
                P.Materials.Add(Single);
            }
            const TSharedPtr<FJsonObject>* Transform = nullptr;
            if ((*Visual)->TryGetObjectField(TEXT("transform"), Transform) && Transform && Transform->IsValid())
            {
                double Value = 0;
                if ((*Transform)->TryGetNumberField(TEXT("scale"), Value))
                {
                    if (FMath::IsFinite(Value) && Value >= 0.001 && Value <= 1000.0) P.VisualScale = float(Value);
                    else OutErrors.Add(Where + TEXT(": visual.transform.scale gecersiz; 1 kullanildi."));
                }
                const auto ReadFinite = [&](const TCHAR* Key, auto& Target)
                {
                    double Number = 0;
                    if (!(*Transform)->TryGetNumberField(Key, Number)) return;
                    if (FMath::IsFinite(Number)) Target = float(Number);
                    else OutErrors.Add(Where + TEXT(": visual.transform.") + Key + TEXT(" gecersiz; 0 kullanildi."));
                };
                ReadFinite(TEXT("pitch"), P.VisualRotation.Pitch);
                ReadFinite(TEXT("yaw"), P.VisualRotation.Yaw);
                ReadFinite(TEXT("roll"), P.VisualRotation.Roll);
                ReadFinite(TEXT("offsetX"), P.VisualOffsetCm.X);
                ReadFinite(TEXT("offsetY"), P.VisualOffsetCm.Y);
                ReadFinite(TEXT("offsetZ"), P.VisualOffsetCm.Z);
            }
        }
        Ids.Add(P.Id);
        OutProducts.Add(P);
    }
    return true;
}

FString MarketCatalog::Serialize(const TArray<FMarketProduct>& Products, const FString& Note)
{
    FString Out = TEXT("{\n");
    Out += FString::Printf(TEXT("  \"schemaVersion\": %d,\n"), SchemaVersion);
    Out += TEXT("  \"note\": ") + JsonQuote(Note) + TEXT(",\n");
    Out += TEXT("  \"products\": [\n");
    for (int32 I = 0; I < Products.Num(); ++I)
    {
        const FMarketProduct& P = Products[I];
        Out += TEXT("    {");
        Out += TEXT("\"id\":") + JsonQuote(P.Id);
        Out += TEXT(",\"realName\":") + JsonQuote(P.RealName);
        Out += TEXT(",\"fictionalName\":") + JsonQuote(P.FictionalName);
        if (!P.Category.IsEmpty()) Out += TEXT(",\"category\":") + JsonQuote(P.Category);
        Out += TEXT(",\"cost\":") + Money(P.Cost);
        Out += TEXT(",\"price\":") + Money(P.BasePrice);
        Out += FString::Printf(TEXT(",\"caseUnits\":%d"), P.CaseUnits);
        Out += TEXT(",\"color\":") + JsonQuote(ColorHex(P.Color));
        if (!P.bActive) Out += TEXT(",\"active\":false");
        if (!P.Brand.IsEmpty()) Out += TEXT(",\"brand\":") + JsonQuote(P.Brand);
        if (!P.Subcategory.IsEmpty() || P.Kvi > 0.f || P.VatRate >= 0.f || P.ShelfLifeDays > 0 || P.Elasticity > 0.f || P.Stockpile >= 0.f || P.TrafficPull > 0.f)
        {
            // G-078: retail data, only what is set.
            TArray<FString> Parts;
            const auto Number = [](float Value) { return FString::Printf(TEXT("%.2f"), Value); };
            if (!P.Subcategory.IsEmpty()) Parts.Add(TEXT("\"subcategory\":") + JsonQuote(P.Subcategory));
            if (P.Kvi > 0.f) Parts.Add(TEXT("\"kvi\":") + Number(P.Kvi));
            if (P.VatRate >= 0.f) Parts.Add(TEXT("\"vat\":") + Number(P.VatRate));
            if (P.ShelfLifeDays > 0) Parts.Add(FString::Printf(TEXT("\"shelfLifeDays\":%d"), P.ShelfLifeDays));
            if (P.Elasticity > 0.f) Parts.Add(TEXT("\"elasticity\":") + Number(P.Elasticity));
            if (P.Stockpile >= 0.f) Parts.Add(TEXT("\"stockpile\":") + Number(P.Stockpile));
            if (P.TrafficPull > 0.f) Parts.Add(TEXT("\"trafficPull\":") + Number(P.TrafficPull));
            Out += TEXT(",\n     \"retail\":{") + FString::Join(Parts, TEXT(",")) + TEXT("}");
        }
        if (!P.PackageType.IsEmpty())
        {
            Out += TEXT(",\n     \"package\":{\"type\":") + JsonQuote(P.PackageType);
            const auto Num = [&Out](const TCHAR* Key, int32 Value) { if (Value > 0) Out += FString::Printf(TEXT(",\"%s\":%d"), Key, Value); };
            Num(TEXT("widthMm"), P.WidthMm);
            Num(TEXT("depthMm"), P.DepthMm);
            Num(TEXT("heightMm"), P.HeightMm);
            Num(TEXT("diameterMm"), P.DiameterMm);
            Num(TEXT("labelHeightMm"), P.LabelHeightMm);
            if (!P.Parts.IsEmpty()) Out += TEXT(",\"parts\":") + JsonQuote(P.Parts);
            if (!P.Notes.IsEmpty()) Out += TEXT(",\"notes\":") + JsonQuote(P.Notes);
            if (!P.Preset.IsEmpty()) Out += TEXT(",\"preset\":") + JsonQuote(P.Preset);
            if (!P.Colors.IsEmpty()) Out += TEXT(",\"colors\":") + JsonQuote(P.Colors);
            if (P.bSizeEstimated) Out += TEXT(",\"estimated\":true");
            Out += TEXT("}");
        }
        if (!P.MeshPath.IsEmpty())
        {
            Out += TEXT(",\n     \"visual\":{\"package\":") + JsonQuote(P.MeshPath) + TEXT(",\"materials\":[");
            for (int32 M = 0; M < P.Materials.Num(); ++M) Out += (M > 0 ? TEXT(",") : TEXT("")) + JsonQuote(P.Materials[M]);
            Out += TEXT("]");
            if (!FMath::IsNearlyEqual(P.VisualScale, 1.f) || !P.VisualRotation.IsNearlyZero() || !P.VisualOffsetCm.IsNearlyZero())
            {
                const auto Number = [](float Value) { return FString::SanitizeFloat(Value, 6); };
                Out += TEXT(",\"transform\":{\"scale\":") + Number(P.VisualScale);
                Out += TEXT(",\"pitch\":") + Number(P.VisualRotation.Pitch);
                Out += TEXT(",\"yaw\":") + Number(P.VisualRotation.Yaw);
                Out += TEXT(",\"roll\":") + Number(P.VisualRotation.Roll);
                Out += TEXT(",\"offsetX\":") + Number(P.VisualOffsetCm.X);
                Out += TEXT(",\"offsetY\":") + Number(P.VisualOffsetCm.Y);
                Out += TEXT(",\"offsetZ\":") + Number(P.VisualOffsetCm.Z) + TEXT("}");
            }
            Out += TEXT("}");
        }
        Out += I + 1 < Products.Num() ? TEXT("},\n") : TEXT("}\n");
    }
    Out += TEXT("  ]\n}\n");
    return Out;
}

bool MarketCatalog::LoadFile(const FString& Path, TArray<FMarketProduct>& OutProducts, TArray<FString>& OutErrors, FString* OutNote)
{
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *Path))
    {
        OutErrors.Add(TEXT("Katalog dosyasi bulunamadi: ") + Path);
        return false;
    }
    return Parse(Json, OutProducts, OutErrors, OutNote);
}

bool MarketCatalog::SaveFile(const FString& Path, const TArray<FMarketProduct>& Products, const FString& Note, FString& OutError)
{
    // Write to a temp file first so a crash never leaves a half-written catalog.
    const FString Temp = Path + TEXT(".tmp");
    if (!FFileHelper::SaveStringToFile(Serialize(Products, Note), *Temp, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
    {
        OutError = TEXT("Katalog yazilamadi: ") + Temp;
        return false;
    }
    IFileManager& Files = IFileManager::Get();
    if (Files.FileExists(*Path)) Files.Copy(*(Path + TEXT(".bak")), *Path);
    if (!Files.Move(*Path, *Temp, true))
    {
        OutError = TEXT("Katalog yerine konamadi: ") + Path;
        return false;
    }
    return true;
}

bool FBoxPackageLayout::Make(int32 W, int32 D, int32 H, FBoxPackageLayout& Out, int32 InAtlasSize)
{
    if (W < 5 || D < 5 || H < 5 || W > 2000 || D > 2000 || H > 2000 || InAtlasSize < 256) return false;
    Out = FBoxPackageLayout();
    Out.WidthMm = W; Out.DepthMm = D; Out.HeightMm = H; Out.AtlasSize = InAtlasSize;
    const int32 P = Out.Padding;
    // Row 1: front, back, right, left (all full height). Row 2: top, bottom.
    const double Scale = FMath::Min(double(InAtlasSize - 5 * P) / double(2 * W + 2 * D), double(InAtlasSize - 3 * P) / double(H + D));
    const int32 Ws = FMath::FloorToInt32(W * Scale);
    const int32 Ds = FMath::FloorToInt32(D * Scale);
    const int32 Hs = FMath::FloorToInt32(H * Scale);
    const auto Rect = [](int32 X, int32 Y, int32 RW, int32 RH) { return FIntRect(X, Y, X + RW, Y + RH); };
    Out.Rects[Front] = Rect(P, P, Ws, Hs);
    Out.Rects[Back] = Rect(2 * P + Ws, P, Ws, Hs);
    Out.Rects[Right] = Rect(3 * P + 2 * Ws, P, Ds, Hs);
    Out.Rects[Left] = Rect(4 * P + 2 * Ws + Ds, P, Ds, Hs);
    Out.Rects[Top] = Rect(P, 2 * P + Hs, Ws, Ds);
    Out.Rects[Bottom] = Rect(2 * P + Ws, 2 * P + Hs, Ws, Ds);
    return true;
}

void FBoxPackageLayout::GetFace(int32 Face, FVector (&C)[4], FVector2D (&UV)[4], FVector& N) const
{
    const double X = DepthMm / 20.0;  // half depth in cm
    const double Y = WidthMm / 20.0;  // half width in cm
    const double Z = HeightMm / 10.0; // full height in cm
    // Unreal is left-handed: looking along -X (at the front face) the viewer's right is -Y.
    switch (Face)
    {
    case Front:  N = FVector(1, 0, 0);  C[0] = FVector(X, Y, Z);   C[1] = FVector(X, -Y, Z);  C[2] = FVector(X, -Y, 0);  C[3] = FVector(X, Y, 0);   break;
    case Back:   N = FVector(-1, 0, 0); C[0] = FVector(-X, -Y, Z); C[1] = FVector(-X, Y, Z);  C[2] = FVector(-X, Y, 0);  C[3] = FVector(-X, -Y, 0); break;
    case Right:  N = FVector(0, -1, 0); C[0] = FVector(X, -Y, Z);  C[1] = FVector(-X, -Y, Z); C[2] = FVector(-X, -Y, 0); C[3] = FVector(X, -Y, 0);  break;
    case Left:   N = FVector(0, 1, 0);  C[0] = FVector(-X, Y, Z);  C[1] = FVector(X, Y, Z);   C[2] = FVector(X, Y, 0);   C[3] = FVector(-X, Y, 0);  break;
    case Top:    N = FVector(0, 0, 1);  C[0] = FVector(-X, Y, Z);  C[1] = FVector(-X, -Y, Z); C[2] = FVector(X, -Y, Z);  C[3] = FVector(X, Y, Z);   break;
    default:     N = FVector(0, 0, -1); C[0] = FVector(X, Y, 0);   C[1] = FVector(X, -Y, 0);  C[2] = FVector(-X, -Y, 0); C[3] = FVector(-X, Y, 0);  break;
    }
    const FIntRect& R = Rects[FMath::Clamp(Face, 0, FaceCount - 1)];
    const double A = AtlasSize;
    UV[0] = FVector2D(R.Min.X / A, R.Min.Y / A);
    UV[1] = FVector2D(R.Max.X / A, R.Min.Y / A);
    UV[2] = FVector2D(R.Max.X / A, R.Max.Y / A);
    UV[3] = FVector2D(R.Min.X / A, R.Max.Y / A);
}

FString FBoxPackageLayout::PackageId(int32 W, int32 D, int32 H)
{
    return FString::Printf(TEXT("box_%dx%dx%d"), W, D, H);
}

bool FBoxPackageLayout::ParsePackageId(const FString& Id, int32& OutW, int32& OutD, int32& OutH)
{
    if (!Id.StartsWith(TEXT("box_"))) return false;
    TArray<FString> Parts;
    Id.Mid(4).ParseIntoArray(Parts, TEXT("x"));
    if (Parts.Num() != 3 || !Parts[0].IsNumeric() || !Parts[1].IsNumeric() || !Parts[2].IsNumeric()) return false;
    OutW = FCString::Atoi(*Parts[0]); OutD = FCString::Atoi(*Parts[1]); OutH = FCString::Atoi(*Parts[2]);
    FBoxPackageLayout Unused;
    return Make(OutW, OutD, OutH, Unused);
}

const TCHAR* FBoxPackageLayout::FaceKey(int32 Face)
{
    static const TCHAR* Keys[] = { TEXT("front"), TEXT("back"), TEXT("right"), TEXT("left"), TEXT("top"), TEXT("bottom") };
    return Keys[FMath::Clamp(Face, 0, FaceCount - 1)];
}

FString MarketCatalog::JsonQuote(const FString& Value)
{
    FString Out = TEXT("\"");
    for (const TCHAR C : Value)
    {
        switch (C)
        {
        case TEXT('"'): Out += TEXT("\\\""); break;
        case TEXT('\\'): Out += TEXT("\\\\"); break;
        case TEXT('\n'): Out += TEXT("\\n"); break;
        case TEXT('\r'): Out += TEXT("\\r"); break;
        case TEXT('\t'): Out += TEXT("\\t"); break;
        default:
            if (C < 0x20) Out += FString::Printf(TEXT("\\u%04x"), static_cast<int32>(C));
            else Out.AppendChar(C);
        }
    }
    return Out + TEXT("\"");
}

const FMarketProduct* MarketCatalog::FindProduct(const TArray<FMarketProduct>& Products, const FString& Id)
{
    return Products.FindByPredicate([&](const FMarketProduct& Product) { return Product.Id == Id; });
}

int32 MarketCatalog::IndexOfProduct(const TArray<FMarketProduct>& Products, const FString& Id)
{
    return Products.IndexOfByPredicate([&](const FMarketProduct& Product) { return Product.Id == Id; });
}

FString MarketCatalog::FoldTurkish(const FString& Text)
{
    FString Out = Text;
    static const TCHAR From[] = TEXT("\u00e7\u00c7\u011f\u011e\u0131\u0130\u00f6\u00d6\u015f\u015e\u00fc\u00dc\u00e2\u00ee\u00fb\u00b7");
    static const TCHAR To[] = TEXT("cCgGiIoOsSuUaiu-");
    for (int32 I = 0; From[I]; ++I) Out.ReplaceCharInline(From[I], To[I], ESearchCase::CaseSensitive);
    return Out;
}

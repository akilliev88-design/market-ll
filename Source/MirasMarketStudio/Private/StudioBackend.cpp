#include "StudioBackend.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "AssetToolsModule.h"
#include "AutomatedAssetImportData.h"
#include "Dom/JsonObject.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "FileHelpers.h"
#include "HAL/FileManager.h"
#include "IAssetTools.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "MeshDescription.h"
#include "Misc/DateTime.h"
#include "Misc/DefaultValueHelper.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "PhysicsEngine/BodySetup.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "StaticMeshAttributes.h"
#include "UObject/Package.h"

namespace MirasStudio
{
namespace
{
    // If boxes ever render inside-out, flip this single switch (see Docs/Surec/DURUM.md).
    constexpr bool bFlipBoxWinding = false;
    constexpr int32 LabelAtlasSize = 2048;
    constexpr int32 MaxLabelSize = 4096;

    FString ObjectPath(const FString& Folder, const FString& Name) { return Folder / Name + TEXT(".") + Name; }

    UPackage* OpenPackage(const FString& Folder, const FString& Name)
    {
        UPackage* Package = CreatePackage(*(Folder / Name));
        Package->FullyLoad();
        return Package;
    }

    bool SaveAll(const TArray<UObject*>& Assets)
    {
        TArray<UPackage*> Packages;
        for (UObject* Asset : Assets)
            if (Asset) Packages.AddUnique(Asset->GetOutermost());
        return Packages.Num() == 0 || UEditorLoadingAndSavingUtils::SavePackages(Packages, false);
    }

    bool WriteJson(const FString& Path, const TSharedRef<FJsonObject>& Root)
    {
        FString Text;
        const auto Writer = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Text);
        if (!FJsonSerializer::Serialize(Root, Writer)) return false;
        return FFileHelper::SaveStringToFile(Text, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    }

    FString Normalized(FString Path)
    {
        FPaths::NormalizeFilename(Path);
        return FPaths::ConvertRelativePathToFull(Path);
    }

    UTexture2D* WriteTexture(const FString& Folder, const FString& Name, const FStudioImage& Image, FString& OutError)
    {
        UPackage* Package = OpenPackage(Folder, Name);
        UTexture2D* Texture = FindObject<UTexture2D>(Package, *Name);
        const bool bNew = Texture == nullptr;
        if (bNew) Texture = NewObject<UTexture2D>(Package, *Name, RF_Public | RF_Standalone | RF_Transactional);
        if (!Texture) { OutError = TEXT("Doku olu\u015fturulamad\u0131: ") + Name; return nullptr; }
        Texture->PreEditChange(nullptr);
        Texture->Source.Init(Image.Width, Image.Height, 1, 1, TSF_BGRA8, reinterpret_cast<const uint8*>(Image.Pixels.GetData()));
        Texture->SRGB = true;
        Texture->CompressionSettings = TC_Default;
        Texture->MipGenSettings = TMGS_FromTextureGroup;
        Texture->LODGroup = TEXTUREGROUP_World;
        Texture->AddressX = TA_Clamp;
        Texture->AddressY = TA_Clamp;
        Texture->PostEditChange();
        Package->MarkPackageDirty();
        if (bNew) FAssetRegistryModule::AssetCreated(Texture);
        return Texture;
    }

    UMaterialInstanceConstant* WriteMaterialInstance(const FString& Folder, const FString& Name, UMaterialInterface* Parent, UTexture2D* Label, FString& OutError)
    {
        UPackage* Package = OpenPackage(Folder, Name);
        UMaterialInstanceConstant* Instance = FindObject<UMaterialInstanceConstant>(Package, *Name);
        const bool bNew = Instance == nullptr;
        if (bNew) Instance = NewObject<UMaterialInstanceConstant>(Package, *Name, RF_Public | RF_Standalone | RF_Transactional);
        if (!Instance) { OutError = TEXT("Materyal olu\u015fturulamad\u0131: ") + Name; return nullptr; }
        Instance->PreEditChange(nullptr);
        Instance->SetParentEditorOnly(Parent);
        Instance->SetTextureParameterValueEditorOnly(FMaterialParameterInfo(TEXT("Label")), Label);
        Instance->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(TEXT("Roughness")), 0.82f);
        Instance->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(TEXT("Specular")), 0.20f);
        Instance->PostEditChange();
        Package->MarkPackageDirty();
        if (bNew) FAssetRegistryModule::AssetCreated(Instance);
        return Instance;
    }

    FString CopySource(const FString& Source, const FString& Directory, const FString& BaseName)
    {
        if (Source.IsEmpty()) return FString();
        const FString Destination = Directory / (BaseName + TEXT(".") + FPaths::GetExtension(Source).ToLower());
        if (Normalized(Source) != Normalized(Destination))
        {
            IFileManager::Get().MakeDirectory(*Directory, true);
            if (IFileManager::Get().Copy(*Destination, *Source, true) != COPY_OK) return FString();
        }
        return Destination;
    }

    FString FaceNameTr(int32 Face)
    {
        static const TCHAR* Names[] = { TEXT("\u00d6n"), TEXT("Arka"), TEXT("Sa\u011f"), TEXT("Sol"), TEXT("\u00dcst"), TEXT("Alt") };
        return Names[FMath::Clamp(Face, 0, 5)];
    }

    bool SavePng(const FString& Path, const FStudioImage& Image, FString& OutError)
    {
        if (!Image.IsValid()) { OutError = TEXT("PNG i\u00e7in ge\u00e7erli g\u00f6rsel olu\u015fmad\u0131."); return false; }
        IImageWrapperModule& Wrappers = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
        const TSharedPtr<IImageWrapper> Wrapper = Wrappers.CreateImageWrapper(EImageFormat::PNG);
        if (!Wrapper.IsValid() || !Wrapper->SetRaw(Image.Pixels.GetData(), Image.Pixels.Num() * sizeof(FColor), Image.Width, Image.Height, ERGBFormat::BGRA, 8))
        {
            OutError = TEXT("K\u0131lavuz PNG bi\u00e7imine d\u00f6n\u00fc\u015ft\u00fcr\u00fclemedi.");
            return false;
        }
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
        const TArray64<uint8> Bytes = Wrapper->GetCompressed(100);
        if (Bytes.IsEmpty() || !FFileHelper::SaveArrayToFile(Bytes, *Path))
        {
            OutError = TEXT("K\u0131lavuz yaz\u0131lamad\u0131: ") + Path;
            return false;
        }
        return true;
    }

    void DrawUvLine(FStudioImage& Image, const FVector2f& From, const FVector2f& To, const FColor& Color, int32 Thickness = 2)
    {
        constexpr int32 Margin = 32;
        const auto Point = [&](const FVector2f& UV)
        {
            return FIntPoint(
                FMath::RoundToInt(FMath::Lerp(float(Margin), float(Image.Width - Margin - 1), FMath::Clamp(UV.X, 0.f, 1.f))),
                FMath::RoundToInt(FMath::Lerp(float(Margin), float(Image.Height - Margin - 1), FMath::Clamp(UV.Y, 0.f, 1.f))));
        };
        const FIntPoint A = Point(From), B = Point(To);
        int32 X = A.X, Y = A.Y;
        const int32 DX = FMath::Abs(B.X - A.X), SX = A.X < B.X ? 1 : -1;
        const int32 DY = -FMath::Abs(B.Y - A.Y), SY = A.Y < B.Y ? 1 : -1;
        int32 Error = DX + DY;
        while (true)
        {
            for (int32 OY = -Thickness; OY <= Thickness; ++OY)
                for (int32 OX = -Thickness; OX <= Thickness; ++OX)
                    if (Image.Pixels.IsValidIndex((Y + OY) * Image.Width + X + OX) && X + OX >= 0 && X + OX < Image.Width && Y + OY >= 0 && Y + OY < Image.Height)
                        Image.Pixels[(Y + OY) * Image.Width + X + OX] = Color;
            if (X == B.X && Y == B.Y) break;
            const int32 Twice = 2 * Error;
            if (Twice >= DY) { Error += DY; X += SX; }
            if (Twice <= DX) { Error += DX; Y += SY; }
        }
    }

    void FillRect(FStudioImage& Image, const FIntRect& Rect, const FColor& Color)
    {
        const int32 X0 = FMath::Clamp(Rect.Min.X, 0, Image.Width);
        const int32 Y0 = FMath::Clamp(Rect.Min.Y, 0, Image.Height);
        const int32 X1 = FMath::Clamp(Rect.Max.X, 0, Image.Width);
        const int32 Y1 = FMath::Clamp(Rect.Max.Y, 0, Image.Height);
        for (int32 Y = Y0; Y < Y1; ++Y)
            for (int32 X = X0; X < X1; ++X)
                Image.Pixels[Y * Image.Width + X] = Color;
    }

    void StrokeRect(FStudioImage& Image, const FIntRect& Rect, const FColor& Color, int32 Thickness = 3)
    {
        FillRect(Image, FIntRect(Rect.Min.X, Rect.Min.Y, Rect.Max.X, Rect.Min.Y + Thickness), Color);
        FillRect(Image, FIntRect(Rect.Min.X, Rect.Max.Y - Thickness, Rect.Max.X, Rect.Max.Y), Color);
        FillRect(Image, FIntRect(Rect.Min.X, Rect.Min.Y, Rect.Min.X + Thickness, Rect.Max.Y), Color);
        FillRect(Image, FIntRect(Rect.Max.X - Thickness, Rect.Min.Y, Rect.Max.X, Rect.Max.Y), Color);
    }

    void DrawPixelLine(FStudioImage& Image, FIntPoint A, FIntPoint B, const FColor& Color, int32 Thickness = 2)
    {
        int32 X = A.X, Y = A.Y;
        const int32 DX = FMath::Abs(B.X - A.X), SX = A.X < B.X ? 1 : -1;
        const int32 DY = -FMath::Abs(B.Y - A.Y), SY = A.Y < B.Y ? 1 : -1;
        int32 Error = DX + DY;
        while (true)
        {
            FillRect(Image, FIntRect(X - Thickness, Y - Thickness, X + Thickness + 1, Y + Thickness + 1), Color);
            if (X == B.X && Y == B.Y) break;
            const int32 Twice = Error * 2;
            if (Twice >= DY) { Error += DY; X += SX; }
            if (Twice <= DX) { Error += DX; Y += SY; }
        }
    }

    FIntPoint ProductionSize(double WidthMm, double HeightMm)
    {
        if (WidthMm <= 0 || HeightMm <= 0) return FIntPoint::ZeroValue;
        const double Scale = FMath::Min3(8.0, 4096.0 / WidthMm, 4096.0 / HeightMm);
        return FIntPoint(FMath::Max(1, FMath::FloorToInt32(WidthMm * Scale)), FMath::Max(1, FMath::FloorToInt32(HeightMm * Scale)));
    }
}

FString InboxDir() { return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("AssetInbox")); }
FString PackageFolder(const FString& PackageId) { return TEXT("/Game/Products/Packages/") + PackageId; }
FString ItemFolder(const FString& ProductId) { return TEXT("/Game/Products/Items/") + ProductId; }
FString LabelMaterialPath() { return TEXT("/Game/Products/Materials/M_ProductLabel.M_ProductLabel"); }

// ---------------------------------------------------------------------------------------- images

bool LoadImageFile(const FString& Path, FStudioImage& Out, FString& OutError)
{
    TArray<uint8> Bytes;
    if (!FFileHelper::LoadFileToArray(Bytes, *Path)) { OutError = TEXT("Dosya okunamad\u0131: ") + Path; return false; }
    IImageWrapperModule& Wrappers = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
    const EImageFormat Format = Wrappers.DetectImageFormat(Bytes.GetData(), Bytes.Num());
    if (Format == EImageFormat::Invalid) { OutError = TEXT("G\u00f6rsel bi\u00e7imi tan\u0131nmad\u0131 (PNG, JPG, TGA, BMP kullan)."); return false; }
    TSharedPtr<IImageWrapper> Wrapper = Wrappers.CreateImageWrapper(Format);
    TArray64<uint8> Raw;
    if (!Wrapper.IsValid() || !Wrapper->SetCompressed(Bytes.GetData(), Bytes.Num()) || !Wrapper->GetRaw(ERGBFormat::BGRA, 8, Raw))
    {
        OutError = TEXT("G\u00f6rsel \u00e7\u00f6z\u00fclemedi: ") + FPaths::GetCleanFilename(Path);
        return false;
    }
    const int32 Width = static_cast<int32>(Wrapper->GetWidth());
    const int32 Height = static_cast<int32>(Wrapper->GetHeight());
    if (Width <= 0 || Height <= 0 || Width > 8192 || Height > 8192 || Raw.Num() != int64(Width) * Height * 4)
    {
        OutError = TEXT("G\u00f6rsel boyutu desteklenmiyor (en fazla 8192 px).");
        return false;
    }
    Out.Width = Width;
    Out.Height = Height;
    Out.Pixels.SetNumUninitialized(Width * Height);
    FMemory::Memcpy(Out.Pixels.GetData(), Raw.GetData(), static_cast<SIZE_T>(Raw.Num()));
    return true;
}

void Resize(const FStudioImage& In, int32 Width, int32 Height, FStudioImage& Out)
{
    Out.Width = FMath::Max(1, Width);
    Out.Height = FMath::Max(1, Height);
    Out.Pixels.SetNumUninitialized(Out.Width * Out.Height);
    const double SX = double(In.Width) / Out.Width;
    const double SY = double(In.Height) / Out.Height;
    for (int32 Y = 0; Y < Out.Height; ++Y)
    {
        const double FY = FMath::Clamp((Y + 0.5) * SY - 0.5, 0.0, In.Height - 1.0);
        const int32 Y0 = FMath::FloorToInt32(FY), Y1 = FMath::Min(Y0 + 1, In.Height - 1);
        const double TY = FY - Y0;
        for (int32 X = 0; X < Out.Width; ++X)
        {
            const double FX = FMath::Clamp((X + 0.5) * SX - 0.5, 0.0, In.Width - 1.0);
            const int32 X0 = FMath::FloorToInt32(FX), X1 = FMath::Min(X0 + 1, In.Width - 1);
            const double TX = FX - X0;
            const FColor& A = In.Pixels[Y0 * In.Width + X0];
            const FColor& B = In.Pixels[Y0 * In.Width + X1];
            const FColor& C = In.Pixels[Y1 * In.Width + X0];
            const FColor& D = In.Pixels[Y1 * In.Width + X1];
            const uint8* PA = reinterpret_cast<const uint8*>(&A);
            const uint8* PB = reinterpret_cast<const uint8*>(&B);
            const uint8* PC = reinterpret_cast<const uint8*>(&C);
            const uint8* PD = reinterpret_cast<const uint8*>(&D);
            FColor Result;
            uint8* PR = reinterpret_cast<uint8*>(&Result);
            for (int32 Channel = 0; Channel < 4; ++Channel)
            {
                const double Top = PA[Channel] * (1 - TX) + PB[Channel] * TX;
                const double Bottom = PC[Channel] * (1 - TX) + PD[Channel] * TX;
                PR[Channel] = static_cast<uint8>(FMath::Clamp(FMath::RoundToInt32(Top * (1 - TY) + Bottom * TY), 0, 255));
            }
            Out.Pixels[Y * Out.Width + X] = Result;
        }
    }
}

FColor AverageBorder(const FStudioImage& Image)
{
    if (!Image.IsValid()) return FColor(200, 200, 200);
    double R = 0, G = 0, B = 0, Weight = 0;
    const auto Add = [&](int32 X, int32 Y)
    {
        const FColor& C = Image.Pixels[Y * Image.Width + X];
        const double A = C.A / 255.0;
        R += C.R * A; G += C.G * A; B += C.B * A; Weight += A;
    };
    for (int32 X = 0; X < Image.Width; ++X) { Add(X, 0); Add(X, Image.Height - 1); }
    for (int32 Y = 0; Y < Image.Height; ++Y) { Add(0, Y); Add(Image.Width - 1, Y); }
    if (Weight <= 0) return FColor(200, 200, 200);
    return FColor(uint8(R / Weight), uint8(G / Weight), uint8(B / Weight), 255);
}

void ComposeAtlas(const FBoxPackageLayout& Layout, const FStudioImage* const Faces[FBoxPackageLayout::FaceCount], FStudioImage& OutAtlas, TArray<FStudioIssue>* OutIssues)
{
    const int32 Size = Layout.AtlasSize;
    const FColor Base = (Faces[0] && Faces[0]->IsValid()) ? AverageBorder(*Faces[0]) : FColor(200, 200, 200);
    OutAtlas.Width = OutAtlas.Height = Size;
    OutAtlas.Pixels.Init(Base, Size * Size);
    int32 Missing = 0;
    for (int32 F = 0; F < FBoxPackageLayout::FaceCount; ++F)
    {
        const FIntRect R = Layout.Rects[F];
        const FStudioImage* Face = Faces[F];
        if (!Face || !Face->IsValid()) { ++Missing; continue; }
        const double RectAspect = double(R.Width()) / R.Height();
        const double ImageAspect = double(Face->Width) / Face->Height;
        // Up to 35% off: fit exactly (image generators rarely hit exact ratios; a stretch
        // looks better than empty bands on a shelf). More than that: keep proportions and fill the rest.
        const double Mismatch = FMath::Abs(ImageAspect / RectAspect - 1.0);
        const bool bStretch = Mismatch <= 0.35;
        if (OutIssues && Mismatch > 0.03)
        {
            OutIssues->Add(FStudioIssue{ EStudioSeverity::Warning, bStretch
                ? FString::Printf(TEXT("%s y\u00fcz oran\u0131 %.2f, kutu y\u00fcz\u00fc %.2f: y\u00fcze oturtmak i\u00e7in hafif\u00e7e esnetildi."), *FaceNameTr(F), ImageAspect, RectAspect)
                : FString::Printf(TEXT("%s y\u00fcz oran\u0131 %.2f, kutu y\u00fcz\u00fc %.2f: fark b\u00fcy\u00fck, esnetilmedi; bo\u015f kenar g\u00f6rselin kenar rengiyle dolduruldu."), *FaceNameTr(F), ImageAspect, RectAspect) });
        }
        if (OutIssues && (Face->Width * 2 < R.Width() || Face->Height * 2 < R.Height()))
        {
            OutIssues->Add(FStudioIssue{ EStudioSeverity::Warning, FString::Printf(TEXT("%s y\u00fcz d\u00fc\u015f\u00fck \u00e7\u00f6z\u00fcn\u00fcrl\u00fckl\u00fc (%d\u00d7%d). \u00d6nerilen: %d\u00d7%d px."), *FaceNameTr(F), Face->Width, Face->Height, R.Width(), R.Height()) });
        }
        const double Scale = FMath::Min(double(R.Width()) / Face->Width, double(R.Height()) / Face->Height);
        const int32 DW = bStretch ? R.Width() : FMath::Clamp(FMath::RoundToInt32(Face->Width * Scale), 1, R.Width());
        const int32 DH = bStretch ? R.Height() : FMath::Clamp(FMath::RoundToInt32(Face->Height * Scale), 1, R.Height());
        FStudioImage Scaled;
        Resize(*Face, DW, DH, Scaled);
        const FColor Background = AverageBorder(*Face);
        const int32 OX = R.Min.X + (R.Width() - DW) / 2;
        const int32 OY = R.Min.Y + (R.Height() - DH) / 2;
        for (int32 Y = R.Min.Y; Y < R.Max.Y; ++Y)
            for (int32 X = R.Min.X; X < R.Max.X; ++X)
                OutAtlas.Pixels[Y * Size + X] = Background;
        for (int32 Y = 0; Y < DH; ++Y)
            for (int32 X = 0; X < DW; ++X)
            {
                const FColor S = Scaled.Pixels[Y * DW + X];
                FColor& D = OutAtlas.Pixels[(OY + Y) * Size + OX + X];
                const float A = S.A / 255.f;
                D = FColor(uint8(S.R * A + D.R * (1 - A)), uint8(S.G * A + D.G * (1 - A)), uint8(S.B * A + D.B * (1 - A)), 255);
            }
    }
    // Bleed each face into its padding so mip levels do not pick up neighbour colors.
    const int32 Bleed = Layout.Padding / 2;
    for (int32 F = 0; F < FBoxPackageLayout::FaceCount; ++F)
    {
        const FIntRect R = Layout.Rects[F];
        for (int32 Y = FMath::Max(0, R.Min.Y - Bleed); Y < FMath::Min(Size, R.Max.Y + Bleed); ++Y)
            for (int32 X = FMath::Max(0, R.Min.X - Bleed); X < FMath::Min(Size, R.Max.X + Bleed); ++X)
            {
                if (X >= R.Min.X && X < R.Max.X && Y >= R.Min.Y && Y < R.Max.Y) continue;
                const int32 SX = FMath::Clamp(X, R.Min.X, R.Max.X - 1);
                const int32 SY = FMath::Clamp(Y, R.Min.Y, R.Max.Y - 1);
                OutAtlas.Pixels[Y * Size + X] = OutAtlas.Pixels[SY * Size + SX];
            }
    }
    if (OutIssues && Missing > 0 && Missing < FBoxPackageLayout::FaceCount)
        OutIssues->Add(FStudioIssue{ EStudioSeverity::Warning, FString::Printf(TEXT("%d y\u00fcz bo\u015f; \u00f6n y\u00fcz\u00fcn kenar rengiyle doldurulacak."), Missing) });
}

UTexture2D* MakeTransientTexture(const FStudioImage& Image)
{
    if (!Image.IsValid()) return nullptr;
    UTexture2D* Texture = UTexture2D::CreateTransient(Image.Width, Image.Height, PF_B8G8R8A8);
    if (!Texture) return nullptr;
    void* Data = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(Data, Image.Pixels.GetData(), Image.Pixels.Num() * sizeof(FColor));
    Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
    Texture->SRGB = true;
    Texture->UpdateResource();
    return Texture;
}

// ---------------------------------------------------------------------------------------- assets


ESlotRole SlotRole(const FString& SlotName)
{
    const FString N = SlotName.ToLower();
    const auto Has = [&N](std::initializer_list<const TCHAR*> Keys)
    {
        for (const TCHAR* Key : Keys) if (N.Contains(Key)) return true;
        return false;
    };
    if (Has({ TEXT("etiket"), TEXT("label") })) return ESlotRole::Label;
    if (Has({ TEXT("kapak"), TEXT("cap"), TEXT("lid") })) return ESlotRole::Cap;
    if (Has({ TEXT("cam"), TEXT("glass") })) return ESlotRole::Glass;
    if (Has({ TEXT("govde"), TEXT("g\u00f6vde"), TEXT("body") })) return ESlotRole::Body;
    return ESlotRole::Other;
}

FString NetRectsFile(const FString& NetFile)
{
    if (NetFile.IsEmpty()) return FString();
    const FString Own = FPaths::ChangeExtension(NetFile, TEXT("json"));
    if (FPaths::FileExists(Own)) return Own;
    const FString Shared = FPaths::GetPath(NetFile) / TEXT("acilim.json");
    return FPaths::FileExists(Shared) ? Shared : FString();
}


namespace
{
    int32 ColorDistance(const FColor& A, const FColor& B)
    {
        return FMath::Abs(int32(A.R) - int32(B.R)) + FMath::Abs(int32(A.G) - int32(B.G)) + FMath::Abs(int32(A.B) - int32(B.B));
    }

    int32 Median(TArray<int32>& Values)
    {
        if (Values.Num() == 0) return -1;
        Values.Sort();
        return Values[Values.Num() / 2];
    }

    // Finds the six panels of a cross-shaped net (left, front, right, back in the middle row; top
    // and bottom above/below the front) on a plain background. Labels, coordinates or notes the
    // image generator drew outside the net are ignored: only the largest connected shape counts.
    bool DetectNetPanels(const FStudioImage& Net, int32 W, int32 D, FIntRect (&Out)[FBoxPackageLayout::FaceCount])
    {
        const int32 IW = Net.Width, IH = Net.Height;
        if (IW < 64 || IH < 64 || W <= 0 || D <= 0) return false;
        // The four corners of a cross net are always empty: they give the background color.
        const int32 P = FMath::Max(2, FMath::Min(IW, IH) / 100);
        const FColor Corners[4] = { Net.Pixels[P * IW + P], Net.Pixels[P * IW + IW - 1 - P],
                                    Net.Pixels[(IH - 1 - P) * IW + P], Net.Pixels[(IH - 1 - P) * IW + IW - 1 - P] };
        for (int32 I = 1; I < 4; ++I)
            if (ColorDistance(Corners[0], Corners[I]) > 36) return false;
        const FColor Bg = Corners[0];

        // Foreground mask + largest 4-connected component.
        TArray<int32> Label;
        Label.Init(0, IW * IH);
        for (int32 I = 0; I < IW * IH; ++I)
            Label[I] = ColorDistance(Net.Pixels[I], Bg) > 48 ? -1 : 0; // -1 = foreground, not yet labelled
        int32 Best = 0, BestSize = 0, Next = 0;
        TArray<int32> Stack;
        for (int32 Start = 0; Start < IW * IH; ++Start)
        {
            if (Label[Start] != -1) continue;
            ++Next;
            int32 Size = 0;
            Stack.Reset();
            Stack.Add(Start);
            Label[Start] = Next;
            while (Stack.Num() > 0)
            {
                const int32 I = Stack.Pop(EAllowShrinking::No);
                ++Size;
                const int32 X = I % IW, Y = I / IW;
                if (X > 0 && Label[I - 1] == -1) { Label[I - 1] = Next; Stack.Add(I - 1); }
                if (X < IW - 1 && Label[I + 1] == -1) { Label[I + 1] = Next; Stack.Add(I + 1); }
                if (Y > 0 && Label[I - IW] == -1) { Label[I - IW] = Next; Stack.Add(I - IW); }
                if (Y < IH - 1 && Label[I + IW] == -1) { Label[I + IW] = Next; Stack.Add(I + IW); }
            }
            if (Size > BestSize) { BestSize = Size; Best = Next; }
        }
        if (Best == 0 || BestSize < IW * IH / 20) return false;

        // Horizontal extent of the shape on each row.
        TArray<int32> RowMin, RowMax;
        RowMin.Init(-1, IH);
        RowMax.Init(-1, IH);
        int32 MaxSpan = 0;
        for (int32 Y = 0; Y < IH; ++Y)
        {
            for (int32 X = 0; X < IW; ++X)
                if (Label[Y * IW + X] == Best) { if (RowMin[Y] < 0) RowMin[Y] = X; RowMax[Y] = X; }
            if (RowMin[Y] >= 0) MaxSpan = FMath::Max(MaxSpan, RowMax[Y] - RowMin[Y]);
        }
        // Middle row of the net = the longest run of rows that span (almost) the whole width.
        int32 M0 = -1, M1 = -1;
        for (int32 Y = 0, RunStart = -1; Y <= IH; ++Y)
        {
            const bool bWide = Y < IH && RowMin[Y] >= 0 && RowMax[Y] - RowMin[Y] >= MaxSpan * 85 / 100;
            if (bWide && RunStart < 0) RunStart = Y;
            if (!bWide && RunStart >= 0)
            {
                if (Y - 1 - RunStart > M1 - M0) { M0 = RunStart; M1 = Y - 1; }
                RunStart = -1;
            }
        }
        int32 T0 = -1, B1 = -1;
        for (int32 Y = 0; Y < IH; ++Y) if (RowMin[Y] >= 0) { if (T0 < 0) T0 = Y; B1 = Y; }
        if (M0 < 0 || M0 - T0 < IH / 50 || B1 - M1 < IH / 50) return false;

        // Front column = where the top and bottom panels sit.
        TArray<int32> L, R, MidL, MidR;
        const auto Collect = [&](int32 Y0, int32 Y1, TArray<int32>& OutL, TArray<int32>& OutR)
        {
            const int32 Margin = (Y1 - Y0) / 5; // skip ragged rows next to the corners
            for (int32 Y = Y0 + Margin; Y <= Y1 - Margin; ++Y)
                if (RowMin[Y] >= 0) { OutL.Add(RowMin[Y]); OutR.Add(RowMax[Y]); }
        };
        Collect(T0, M0 - 1, L, R);
        Collect(M1 + 1, B1, L, R);
        Collect(M0, M1, MidL, MidR);
        const int32 FX0 = Median(L), FX1 = Median(R) + 1, X0 = Median(MidL), X1 = Median(MidR) + 1;
        const int32 Span = X1 - X0;
        if (FX0 < 0 || MidL.Num() == 0 || FX0 - X0 < Span * 3 / 100 || X1 - FX1 < Span * 6 / 100 || FX1 <= FX0) return false;

        // Right/back boundary: expected from the box proportions, snapped to a dark outline if one is there.
        const int32 Rest = X1 - FX1;
        const int32 Guess = FX1 + FMath::RoundToInt32(double(Rest) * D / double(D + W));
        int32 Split = Guess, BestScore = 0, Samples = 0;
        const int32 Window = FMath::Max(4, Rest * 12 / 100);
        for (int32 X = FMath::Max(FX1 + 4, Guess - Window); X <= FMath::Min(X1 - 5, Guess + Window); ++X)
        {
            int32 Score = 0;
            Samples = 0;
            for (int32 Y = M0 + 2; Y < M1 - 2; Y += 3)
            {
                ++Samples;
                const FColor C = Net.Pixels[Y * IW + X];
                const int32 Luma = (C.R * 299 + C.G * 587 + C.B * 114) / 1000;
                if (Luma < 100) ++Score;
            }
            if (Score > BestScore) { BestScore = Score; Split = X; }
        }
        if (BestScore < Samples * 6 / 10) Split = Guess;

        Out[FBoxPackageLayout::Left] = FIntRect(X0, M0, FX0, M1 + 1);
        Out[FBoxPackageLayout::Front] = FIntRect(FX0, M0, FX1, M1 + 1);
        Out[FBoxPackageLayout::Right] = FIntRect(FX1, M0, Split, M1 + 1);
        Out[FBoxPackageLayout::Back] = FIntRect(Split, M0, X1, M1 + 1);
        Out[FBoxPackageLayout::Top] = FIntRect(FX0, T0, FX1, M0);
        Out[FBoxPackageLayout::Bottom] = FIntRect(FX0, M1 + 1, FX1, B1 + 1);
        for (const FIntRect& Rect : Out)
            if (Rect.Width() < 8 || Rect.Height() < 8) return false;
        return true;
    }
}

void SplitNet(const FString& NetFile, const FStudioImage& Net, int32 W, int32 D, int32 H, FStudioImage (&OutFaces)[FBoxPackageLayout::FaceCount], TArray<FStudioIssue>* OutIssues)
{
    const auto CopyRect = [&Net](int32 L, int32 T, int32 R, int32 B, FStudioImage& Out)
    {
        L = FMath::Clamp(L, 0, Net.Width - 1); T = FMath::Clamp(T, 0, Net.Height - 1);
        R = FMath::Clamp(R, L + 1, Net.Width); B = FMath::Clamp(B, T + 1, Net.Height);
        Out.Width = R - L;
        Out.Height = B - T;
        Out.Pixels.SetNumUninitialized(Out.Width * Out.Height);
        for (int32 Y = 0; Y < Out.Height; ++Y)
            FMemory::Memcpy(&Out.Pixels[Y * Out.Width], &Net.Pixels[(T + Y) * Net.Width + L], Out.Width * sizeof(FColor));
    };
    // 1) Explicit panel rectangles --------------------------------------------------------
    const FString RectsFile = NetRectsFile(NetFile);
    if (!RectsFile.IsEmpty())
    {
        FString Text;
        TSharedPtr<FJsonObject> Root;
        if (FFileHelper::LoadFileToString(Text, *RectsFile) && FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Root) && Root.IsValid())
        {
            double Inset = 4;
            Root->TryGetNumberField(TEXT("inset"), Inset);
            int32 Found = 0;
            for (int32 F = 0; F < FBoxPackageLayout::FaceCount; ++F)
            {
                const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
                if (!Root->TryGetArrayField(FBoxPackageLayout::FaceKey(F), Values) || Values->Num() != 4) continue;
                const int32 I = FMath::RoundToInt32(Inset);
                CopyRect(int32((*Values)[0]->AsNumber()) + I, int32((*Values)[1]->AsNumber()) + I,
                         int32((*Values)[2]->AsNumber()) - I, int32((*Values)[3]->AsNumber()) - I, OutFaces[F]);
                ++Found;
            }
            if (Found == FBoxPackageLayout::FaceCount)
            {
                if (OutIssues) OutIssues->Add(FStudioIssue{ EStudioSeverity::Ok, TEXT("A\u00e7\u0131l\u0131m, ") + FPaths::GetCleanFilename(RectsFile) + TEXT(" i\u00e7indeki panel s\u0131n\u0131rlar\u0131yla kesildi.") });
                return;
            }
            if (OutIssues) OutIssues->Add(FStudioIssue{ EStudioSeverity::Warning, FPaths::GetCleanFilename(RectsFile) + TEXT(" 6 y\u00fcz\u00fcn hepsini i\u00e7ermiyor (front, back, right, left, top, bottom); oranla kesildi.") });
        }
        else if (OutIssues) OutIssues->Add(FStudioIssue{ EStudioSeverity::Warning, FPaths::GetCleanFilename(RectsFile) + TEXT(" okunamad\u0131; oranla kesildi.") });
    }

    // 2) Automatic panel detection (net on a plain background, with or without notes around it)
    FIntRect Found[FBoxPackageLayout::FaceCount];
    if (DetectNetPanels(Net, W, D, Found))
    {
        const int32 Inset = FMath::Max(2, FMath::RoundToInt32(FMath::Min(Net.Width, Net.Height) * 0.004));
        for (int32 F = 0; F < FBoxPackageLayout::FaceCount; ++F)
            CopyRect(Found[F].Min.X + Inset, Found[F].Min.Y + Inset, Found[F].Max.X - Inset, Found[F].Max.Y - Inset, OutFaces[F]);
        if (OutIssues) OutIssues->Add(FStudioIssue{ EStudioSeverity::Ok, TEXT("A\u00e7\u0131l\u0131m\u0131n 6 paneli g\u00f6rselden otomatik bulundu; panel d\u0131\u015f\u0131ndaki yaz\u0131lar yok say\u0131ld\u0131.") });
        return;
    }

    const double UnitW = 2.0 * W + 2.0 * D;
    const double UnitH = 2.0 * D + H;
    const double Expected = UnitW / UnitH;
    const double Actual = double(Net.Width) / FMath::Max(1, Net.Height);
    if (OutIssues && FMath::Abs(Actual / Expected - 1.0) > 0.03)
    {
        OutIssues->Add(FStudioIssue{ EStudioSeverity::Warning, FString::Printf(TEXT("A\u00e7\u0131l\u0131m g\u00f6rselinin oran\u0131 %.3f, kutu \u00f6l\u00e7\u00fcs\u00fcne g\u00f6re beklenen %.3f. Y\u00fczler orant\u0131l\u0131 kesildi; kenarlar kayabilir."), Actual, Expected) });
    }
    // 3) Proportional split of a clean net ------------------------------------------------
    const double SX = Net.Width / UnitW;
    const double SY = Net.Height / UnitH;
    const auto Crop = [&](double X0, double Y0, double CW, double CH, FStudioImage& Out)
    {
        CopyRect(FMath::RoundToInt32(X0 * SX), FMath::RoundToInt32(Y0 * SY), FMath::RoundToInt32((X0 + CW) * SX), FMath::RoundToInt32((Y0 + CH) * SY), Out);
    };
    Crop(D, D, W, H, OutFaces[FBoxPackageLayout::Front]);
    Crop(0, D, D, H, OutFaces[FBoxPackageLayout::Left]);
    Crop(D + W, D, D, H, OutFaces[FBoxPackageLayout::Right]);
    Crop(2.0 * D + W, D, W, H, OutFaces[FBoxPackageLayout::Back]);
    Crop(D, 0, W, D, OutFaces[FBoxPackageLayout::Top]);
    Crop(D, D + H, W, D, OutFaces[FBoxPackageLayout::Bottom]);
}

// ---------------------------------------------------------------------------------------- assets

namespace
{
    // Creates (once) a master material under /Game/Products/Materials with the given graph.
    UMaterialInterface* EnsureMaster(const FString& Name, TFunctionRef<bool(UMaterial*)> Build, FString& OutError)
    {
        const FString Folder = TEXT("/Game/Products/Materials");
        if (UMaterialInterface* Existing = LoadObject<UMaterialInterface>(nullptr, *ObjectPath(Folder, Name), nullptr, LOAD_NoWarn | LOAD_Quiet))
        {
            // The game draws shelf stock with instanced meshes; masters made before 28.09.2026 lack the flag.
            if (UMaterial* Base = Cast<UMaterial>(Existing); Base && !Base->GetUsageByFlag(MATUSAGE_InstancedStaticMeshes))
            {
                bool bNeedsRecompile = false;
                Base->SetMaterialUsage(bNeedsRecompile, MATUSAGE_InstancedStaticMeshes);
                Base->MarkPackageDirty();
                SaveAll({ Base });
            }
            return Existing;
        }
        UPackage* Package = OpenPackage(Folder, Name);
        UMaterial* Material = NewObject<UMaterial>(Package, *Name, RF_Public | RF_Standalone | RF_Transactional);
        if (!Material || !Build(Material)) { OutError = Name + TEXT(" ana materyali olu\u015fturulamad\u0131."); return nullptr; }
        Material->bUsedWithInstancedStaticMeshes = true;
        FAssetRegistryModule::AssetCreated(Material);
        UMaterialEditingLibrary::RecompileMaterial(Material);
        Package->MarkPackageDirty();
        SaveAll({ Material });
        return Material;
    }

    template <typename T>
    T* Expression(UMaterial* Material, int32 X, int32 Y)
    {
        return Cast<T>(UMaterialEditingLibrary::CreateMaterialExpression(Material, T::StaticClass(), X, Y));
    }

    UMaterialExpressionScalarParameter* Scalar(UMaterial* Material, const TCHAR* Name, float Value, int32 Y)
    {
        auto* Param = Expression<UMaterialExpressionScalarParameter>(Material, -420, Y);
        if (Param) { Param->ParameterName = Name; Param->DefaultValue = Value; }
        return Param;
    }

    UMaterialExpressionVectorParameter* Color(UMaterial* Material, const FLinearColor& Value, int32 Y)
    {
        auto* Param = Expression<UMaterialExpressionVectorParameter>(Material, -420, Y);
        if (Param) { Param->ParameterName = TEXT("Color"); Param->DefaultValue = Value; }
        return Param;
    }

    // Package-level material for a named mesh slot (glass, cap, body) from malzeme.json.
    UMaterialInstanceConstant* WriteSlotMaterial(const FString& Folder, const FString& Name, UMaterialInterface* Parent,
        const FLinearColor& Tint, float Roughness, float Metallic, float Opacity, FString& OutError)
    {
        UPackage* Package = OpenPackage(Folder, Name);
        UMaterialInstanceConstant* Instance = FindObject<UMaterialInstanceConstant>(Package, *Name);
        const bool bNew = Instance == nullptr;
        if (bNew) Instance = NewObject<UMaterialInstanceConstant>(Package, *Name, RF_Public | RF_Standalone | RF_Transactional);
        if (!Instance) { OutError = TEXT("Par\u00e7a materyali olu\u015fturulamad\u0131: ") + Name; return nullptr; }
        Instance->PreEditChange(nullptr);
        Instance->SetParentEditorOnly(Parent);
        Instance->SetVectorParameterValueEditorOnly(FMaterialParameterInfo(TEXT("Color")), Tint);
        Instance->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(TEXT("Roughness")), Roughness);
        Instance->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(TEXT("Metallic")), Metallic);
        Instance->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(TEXT("Opacity")), Opacity);
        Instance->PostEditChange();
        Package->MarkPackageDirty();
        if (bNew) FAssetRegistryModule::AssetCreated(Instance);
        return Instance;
    }

    TSharedPtr<FJsonObject> ReadJson(const FString& Path)
    {
        FString Text;
        TSharedPtr<FJsonObject> Root;
        if (!FFileHelper::LoadFileToString(Text, *Path) || !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Root)) return nullptr;
        return Root;
    }

    int32 FindSlot(const FStudioPackage& Package, ESlotRole Role)
    {
        for (int32 I = 0; I < Package.SlotNames.Num(); ++I) if (SlotRole(Package.SlotNames[I]) == Role) return I;
        return INDEX_NONE;
    }
}

UMaterialInterface* EnsureLabelMaterial(FString& OutError)
{
    return EnsureMaster(TEXT("M_ProductLabel"), [](UMaterial* M)
    {
        auto* Label = Expression<UMaterialExpressionTextureSampleParameter2D>(M, -420, 0);
        auto* Roughness = Scalar(M, TEXT("Roughness"), 0.82f, 260);
        auto* Specular = Scalar(M, TEXT("Specular"), 0.20f, 380);
        if (!Label || !Roughness || !Specular) return false;
        Label->ParameterName = TEXT("Label");
        Label->Texture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"));
        Label->SamplerType = SAMPLERTYPE_Color;
        UMaterialEditingLibrary::ConnectMaterialProperty(Label, TEXT("RGB"), MP_BaseColor);
        UMaterialEditingLibrary::ConnectMaterialProperty(Roughness, TEXT(""), MP_Roughness);
        UMaterialEditingLibrary::ConnectMaterialProperty(Specular, TEXT(""), MP_Specular);
        return true;
    }, OutError);
}

UMaterialInterface* EnsureSolidMaterial(FString& OutError)
{
    return EnsureMaster(TEXT("M_ProductSolid"), [](UMaterial* M)
    {
        auto* Tint = Color(M, FLinearColor(0.8f, 0.8f, 0.8f), 0);
        auto* Roughness = Scalar(M, TEXT("Roughness"), 0.45f, 200);
        auto* Metallic = Scalar(M, TEXT("Metallic"), 0.f, 320);
        if (!Tint || !Roughness || !Metallic) return false;
        UMaterialEditingLibrary::ConnectMaterialProperty(Tint, TEXT(""), MP_BaseColor);
        UMaterialEditingLibrary::ConnectMaterialProperty(Roughness, TEXT(""), MP_Roughness);
        UMaterialEditingLibrary::ConnectMaterialProperty(Metallic, TEXT(""), MP_Metallic);
        return true;
    }, OutError);
}

UMaterialInterface* EnsureGlassMaterial(FString& OutError)
{
    return EnsureMaster(TEXT("M_ProductGlass"), [](UMaterial* M)
    {
        M->BlendMode = BLEND_Translucent;
        M->TranslucencyLightingMode = TLM_SurfacePerPixelLighting;
        auto* Tint = Color(M, FLinearColor(0.85f, 0.93f, 0.9f), 0);
        auto* Opacity = Scalar(M, TEXT("Opacity"), 0.3f, 200);
        auto* Roughness = Scalar(M, TEXT("Roughness"), 0.05f, 320);
        if (!Tint || !Opacity || !Roughness) return false;
        UMaterialEditingLibrary::ConnectMaterialProperty(Tint, TEXT(""), MP_BaseColor);
        UMaterialEditingLibrary::ConnectMaterialProperty(Opacity, TEXT(""), MP_Opacity);
        UMaterialEditingLibrary::ConnectMaterialProperty(Roughness, TEXT(""), MP_Roughness);
        return true;
    }, OutError);
}


UStaticMesh* CreateBoxPackage(int32 W, int32 D, int32 H, FString& OutPackageId, FString& OutError)
{
    FBoxPackageLayout Layout;
    if (!FBoxPackageLayout::Make(W, D, H, Layout, LabelAtlasSize))
    {
        OutError = TEXT("Kutu \u00f6l\u00e7\u00fcs\u00fc 5\u20132000 mm aras\u0131nda olmal\u0131.");
        return nullptr;
    }
    OutPackageId = FBoxPackageLayout::PackageId(W, D, H);
    const FString Folder = PackageFolder(OutPackageId);
    const FString Name = TEXT("SM_") + OutPackageId;
    if (UStaticMesh* Existing = LoadObject<UStaticMesh>(nullptr, *ObjectPath(Folder, Name), nullptr, LOAD_NoWarn | LOAD_Quiet))
        return Existing;
    UMaterialInterface* Label = EnsureLabelMaterial(OutError);
    if (!Label) return nullptr;

    FMeshDescription Mesh;
    FStaticMeshAttributes Attributes(Mesh);
    Attributes.Register();
    auto Positions = Attributes.GetVertexPositions();
    auto Normals = Attributes.GetVertexInstanceNormals();
    auto UVs = Attributes.GetVertexInstanceUVs();
    UVs.SetNumChannels(1);
    const FPolygonGroupID Group = Mesh.CreatePolygonGroup();
    Attributes.GetPolygonGroupMaterialSlotNames()[Group] = FName(TEXT("Label"));
    for (int32 Face = 0; Face < FBoxPackageLayout::FaceCount; ++Face)
    {
        FVector Corner[4]; FVector2D UV[4]; FVector Normal;
        Layout.GetFace(Face, Corner, UV, Normal);
        FVertexInstanceID Instance[4];
        for (int32 K = 0; K < 4; ++K)
        {
            const FVertexID Vertex = Mesh.CreateVertex();
            Positions[Vertex] = FVector3f(Corner[K]);
            Instance[K] = Mesh.CreateVertexInstance(Vertex);
            UVs.Set(Instance[K], 0, FVector2f(UV[K]));
            Normals[Instance[K]] = FVector3f(Normal);
        }
        // Unreal derives a triangle's facing from (P2 - P0) x (P1 - P0); pick the order that faces outward.
        const auto Triangle = [&](int32 A, int32 B, int32 C)
        {
            const FVector Facing = FVector::CrossProduct(Corner[C] - Corner[A], Corner[B] - Corner[A]);
            const bool bOutward = (FVector::DotProduct(Facing, Normal) > 0) != bFlipBoxWinding;
            const TArray<FVertexInstanceID> Ids = bOutward ? TArray<FVertexInstanceID>{ Instance[A], Instance[B], Instance[C] }
                                                           : TArray<FVertexInstanceID>{ Instance[A], Instance[C], Instance[B] };
            Mesh.CreateTriangle(Group, Ids);
        };
        Triangle(0, 1, 2);
        Triangle(0, 2, 3);
    }

    UPackage* Package = OpenPackage(Folder, Name);
    UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, *Name, RF_Public | RF_Standalone | RF_Transactional);
    StaticMesh->InitResources();
    StaticMesh->SetLightingGuid();
    FStaticMeshSourceModel& Source = StaticMesh->AddSourceModel();
    Source.BuildSettings.bRecomputeNormals = false;
    Source.BuildSettings.bRecomputeTangents = true;
    Source.BuildSettings.bRemoveDegenerates = false;
    Source.BuildSettings.bGenerateLightmapUVs = true;
    Source.BuildSettings.SrcLightmapIndex = 0;
    Source.BuildSettings.DstLightmapIndex = 1;
    StaticMesh->CreateMeshDescription(0, MoveTemp(Mesh));
    StaticMesh->CommitMeshDescription(0);
    StaticMesh->SetLightMapCoordinateIndex(1);
    StaticMesh->GetStaticMaterials().Add(FStaticMaterial(Label, FName(TEXT("Label")), FName(TEXT("Label"))));
    StaticMesh->CreateBodySetup();
    if (UBodySetup* Body = StaticMesh->GetBodySetup())
    {
        FKBoxElem Box(D / 10.f, W / 10.f, H / 10.f);
        Box.Center = FVector(0, 0, H / 20.0);
        Body->AggGeom.BoxElems.Add(Box);
    }
    StaticMesh->Build(false);
    StaticMesh->PostEditChange();
    Package->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(StaticMesh);
    if (!SaveAll({ StaticMesh })) { OutError = TEXT("Kutu modeli kaydedilemedi."); return nullptr; }

    const TSharedRef<FJsonObject> Meta = MakeShared<FJsonObject>();
    Meta->SetNumberField(TEXT("schemaVersion"), 1);
    Meta->SetStringField(TEXT("id"), OutPackageId);
    Meta->SetStringField(TEXT("type"), TEXT("box"));
    Meta->SetNumberField(TEXT("widthMm"), W);
    Meta->SetNumberField(TEXT("depthMm"), D);
    Meta->SetNumberField(TEXT("heightMm"), H);
    Meta->SetNumberField(TEXT("atlasSize"), LabelAtlasSize);
    Meta->SetStringField(TEXT("mesh"), StaticMesh->GetPathName());
    IFileManager::Get().MakeDirectory(*(InboxDir() / TEXT("Packages") / OutPackageId), true);
    WriteJson(InboxDir() / TEXT("Packages") / OutPackageId / TEXT("package.json"), Meta);
    return StaticMesh;
}

UStaticMesh* ImportModelPackage(const FString& File, FString& OutPackageId, FString& OutError)
{
    if (!FPaths::FileExists(File)) { OutError = TEXT("Model dosyas\u0131 bulunamad\u0131."); return nullptr; }
    // Uretim/<urun>/model/model.fbx -> package named after the product folder, not "model".
    FString BaseName = FPaths::GetBaseFilename(File);
    const FString Parent = FPaths::GetCleanFilename(FPaths::GetPath(File));
    if (BaseName.Equals(TEXT("model"), ESearchCase::IgnoreCase) && Parent.Equals(TEXT("model"), ESearchCase::IgnoreCase))
        BaseName = FPaths::GetCleanFilename(FPaths::GetPath(FPaths::GetPath(File)));
    const FString Base = TEXT("model_") + MarketCatalog::MakeId(BaseName).Left(30);
    OutPackageId = Base;
    for (int32 Suffix = 2; FPaths::DirectoryExists(InboxDir() / TEXT("Packages") / OutPackageId) ||
         FPackageName::DoesPackageExist(PackageFolder(OutPackageId) / (TEXT("SM_") + OutPackageId)); ++Suffix)
    {
        OutPackageId = FString::Printf(TEXT("%s_%d"), *Base, Suffix);
    }
    // Keep the raw files in AssetInbox (never edited), and import from that copy so the
    // mesh gets a predictable name.
    const FString Directory = InboxDir() / TEXT("Packages") / OutPackageId;
    const FString Copy = CopySource(File, Directory, TEXT("SM_") + OutPackageId);
    if (Copy.IsEmpty()) { OutError = TEXT("Model dosyas\u0131 AssetInbox'a kopyalanamad\u0131."); return nullptr; }
    const FString SpecFile = FPaths::GetPath(File) / TEXT("malzeme.json");
    if (FPaths::FileExists(SpecFile)) CopySource(SpecFile, Directory, TEXT("malzeme"));
    const FString UvFile = FPaths::GetPath(File) / TEXT("uv_sablon.png");
    if (FPaths::FileExists(UvFile)) CopySource(UvFile, Directory, TEXT("uv_sablon"));

    UAutomatedAssetImportData* Import = NewObject<UAutomatedAssetImportData>();
    Import->bReplaceExisting = true;
    Import->DestinationPath = PackageFolder(OutPackageId);
    Import->Filenames.Add(Copy);
    IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
    const TArray<UObject*> Imported = AssetTools.ImportAssetsAutomated(Import);
    UStaticMesh* Mesh = nullptr;
    TArray<UObject*> ToSave;
    for (UObject* Object : Imported)
    {
        if (!Mesh) Mesh = Cast<UStaticMesh>(Object);
        ToSave.Add(Object);
    }
    if (!Mesh)
    {
        // Some importers (Interchange) return only the factory result; look in the folder.
        TArray<FAssetData> Assets;
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get().GetAssetsByPath(FName(*PackageFolder(OutPackageId)), Assets, true);
        for (const FAssetData& Asset : Assets)
            if (!Mesh && Asset.AssetClassPath == UStaticMesh::StaticClass()->GetClassPathName()) Mesh = Cast<UStaticMesh>(Asset.GetAsset());
    }
    if (!Mesh)
    {
        OutError = TEXT("Dosyadan statik model \u00e7\u0131kmad\u0131. FBX/OBJ/GLB i\u00e7inde tek par\u00e7a (iskeletsiz) model olmal\u0131.");
        return nullptr;
    }
    ToSave.AddUnique(Mesh);

    // Glass / cap / body slots get package-level materials. malzeme.json (from prompt B1/C1)
    // decides; without it only glass slots are replaced (glass never survives FBX import well).
    const TSharedPtr<FJsonObject> Spec = ReadJson(SpecFile);
    const TArray<TSharedPtr<FJsonValue>>* SpecSlots = nullptr;
    if (Spec.IsValid()) Spec->TryGetArrayField(TEXT("slots"), SpecSlots);
    TArray<FString> Applied;
    for (int32 Index = 0; Index < Mesh->GetStaticMaterials().Num(); ++Index)
    {
        const FString SlotName = Mesh->GetStaticMaterials()[Index].MaterialSlotName.ToString();
        const ESlotRole Role = SlotRole(SlotName);
        FString Type = Role == ESlotRole::Glass ? TEXT("glass") : TEXT("keep");
        FString Hex = Role == ESlotRole::Glass ? TEXT("D9EEE6") : TEXT("CCCCCC");
        double Roughness = Role == ESlotRole::Glass ? 0.05 : 0.45, Metallic = 0.0, Opacity = 0.3;
        bool bFromSpec = false;
        if (SpecSlots)
        {
            for (const TSharedPtr<FJsonValue>& Value : *SpecSlots)
            {
                const TSharedPtr<FJsonObject> Entry = Value.IsValid() ? Value->AsObject() : nullptr;
                FString Name;
                if (!Entry.IsValid() || !Entry->TryGetStringField(TEXT("name"), Name) || !Name.Equals(SlotName, ESearchCase::IgnoreCase)) continue;
                Entry->TryGetStringField(TEXT("type"), Type);
                Entry->TryGetStringField(TEXT("color"), Hex);
                Entry->TryGetNumberField(TEXT("roughness"), Roughness);
                Entry->TryGetNumberField(TEXT("metallic"), Metallic);
                Entry->TryGetNumberField(TEXT("opacity"), Opacity);
                bFromSpec = true;
            }
        }
        if (Role == ESlotRole::Label || (Type != TEXT("glass") && Type != TEXT("solid"))) continue;
        UMaterialInterface* Master = Type == TEXT("glass") ? EnsureGlassMaterial(OutError) : EnsureSolidMaterial(OutError);
        if (!Master) return nullptr;
        const FString Name = TEXT("MI_") + OutPackageId + TEXT("_") + MarketCatalog::MakeId(SlotName);
        UMaterialInstanceConstant* Instance = WriteSlotMaterial(PackageFolder(OutPackageId), Name, Master, FLinearColor(FColor::FromHex(Hex)),
            float(Roughness), float(Metallic), float(FMath::Clamp(Opacity, 0.02, 1.0)), OutError);
        if (!Instance) return nullptr;
        Mesh->SetMaterial(Index, Instance);
        ToSave.Add(Instance);
        Applied.Add(SlotName + (bFromSpec ? TEXT(" (malzeme.json)") : TEXT(" (otomatik cam)")));
    }
    Mesh->PostEditChange();
    Mesh->MarkPackageDirty();
    SaveAll(ToSave);

    const TSharedRef<FJsonObject> Meta = MakeShared<FJsonObject>();
    Meta->SetNumberField(TEXT("schemaVersion"), 1);
    Meta->SetStringField(TEXT("id"), OutPackageId);
    Meta->SetStringField(TEXT("type"), TEXT("model"));
    Meta->SetStringField(TEXT("source"), File);
    Meta->SetStringField(TEXT("mesh"), Mesh->GetPathName());
    Meta->SetStringField(TEXT("slotMaterials"), FString::Join(Applied, TEXT(", ")));
    WriteJson(Directory / TEXT("package.json"), Meta);
    return Mesh;
}

bool ExportUvTemplate(const FStudioPackage& Package, const FString& OutputFile, FString& OutError)
{
    UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *Package.MeshPath);
    const FMeshDescription* Description = Mesh ? Mesh->GetMeshDescription(0) : nullptr;
    if (!Description) { OutError = TEXT("UV k\u0131lavuzu i\u00e7in kaynak model verisi bulunamad\u0131."); return false; }
    const FStaticMeshConstAttributes Attributes(*Description);
    const auto UVs = Attributes.GetVertexInstanceUVs();
    const auto SlotNames = Attributes.GetPolygonGroupMaterialSlotNames();
    if (!UVs.IsValid() || UVs.GetNumChannels() < 1) { OutError = TEXT("Modelde UV0 kanal\u0131 yok."); return false; }

    FStudioImage Image;
    Image.Width = Image.Height = 2048;
    Image.Pixels.Init(FColor::White, Image.Width * Image.Height);
    const FColor Grid(215, 220, 225), Edge(24, 35, 45), Border(95, 105, 115);
    for (int32 Step = 0; Step <= 4; ++Step)
    {
        const float T = Step / 4.f;
        DrawUvLine(Image, FVector2f(T, 0), FVector2f(T, 1), Step == 0 || Step == 4 ? Border : Grid, 0);
        DrawUvLine(Image, FVector2f(0, T), FVector2f(1, T), Step == 0 || Step == 4 ? Border : Grid, 0);
    }

    int32 TriangleCount = 0;
    for (const FTriangleID Triangle : Description->Triangles().GetElementIDs())
    {
        const FPolygonGroupID Group = Description->GetTrianglePolygonGroup(Triangle);
        const FString Slot = SlotNames.IsValid() ? SlotNames[Group].ToString() : FString();
        const bool bLabel = SlotRole(Slot) == ESlotRole::Label || Group.GetValue() == Package.LabelSlot;
        if (!bLabel) continue;
        const TArrayView<const FVertexInstanceID> Vertices = Description->GetTriangleVertexInstances(Triangle);
        if (Vertices.Num() != 3) continue;
        for (int32 EdgeIndex = 0; EdgeIndex < 3; ++EdgeIndex)
            DrawUvLine(Image, UVs.Get(Vertices[EdgeIndex], 0), UVs.Get(Vertices[(EdgeIndex + 1) % 3], 0), Edge);
        ++TriangleCount;
    }
    if (TriangleCount == 0) { OutError = TEXT("Etiket malzeme yuvas\u0131nda UV \u00fc\u00e7geni bulunamad\u0131."); return false; }
    return SavePng(OutputFile, Image, OutError);
}

// ---------------------------------------------------------------------------------------- ready packages

FString PresetsPath()
{
    return FPaths::ConvertRelativePathToFull(FPaths::ProjectConfigDir() / TEXT("ambalajlar.json"));
}

TArray<FPackagePreset> LoadPresets(TArray<FString>* OutErrors)
{
    TArray<FPackagePreset> Result;
    const TSharedPtr<FJsonObject> Root = ReadJson(PresetsPath());
    const TArray<TSharedPtr<FJsonValue>>* Items = nullptr;
    if (!Root.IsValid() || !Root->TryGetArrayField(TEXT("packages"), Items))
    {
        if (OutErrors) OutErrors->Add(TEXT("Config/ambalajlar.json okunamad\u0131."));
        return Result;
    }
    for (const TSharedPtr<FJsonValue>& Value : *Items)
    {
        const TSharedPtr<FJsonObject> O = Value.IsValid() ? Value->AsObject() : nullptr;
        if (!O.IsValid()) continue;
        FPackagePreset P;
        O->TryGetStringField(TEXT("id"), P.Id);
        O->TryGetStringField(TEXT("name"), P.Name);
        O->TryGetStringField(TEXT("type"), P.Type);
        O->TryGetStringField(TEXT("shape"), P.Shape);
        O->TryGetStringField(TEXT("parts"), P.Parts);
        O->TryGetStringField(TEXT("colors"), P.Colors);
        O->TryGetStringField(TEXT("notes"), P.Notes);
        O->TryGetNumberField(TEXT("widthMm"), P.WidthMm);
        O->TryGetNumberField(TEXT("depthMm"), P.DepthMm);
        O->TryGetNumberField(TEXT("heightMm"), P.HeightMm);
        O->TryGetNumberField(TEXT("diameterMm"), P.DiameterMm);
        O->TryGetNumberField(TEXT("labelHeightMm"), P.LabelHeightMm);
        O->TryGetNumberField(TEXT("labelBottomMm"), P.LabelBottomMm);
        O->TryGetNumberField(TEXT("neckDiameterMm"), P.NeckDiameterMm);
        O->TryGetNumberField(TEXT("capHeightMm"), P.CapHeightMm);
        O->TryGetBoolField(TEXT("topCap"), P.bTopCap);
        O->TryGetBoolField(TEXT("metalCap"), P.bMetalCap);
        const bool bSizes = P.IsBox() ? (P.WidthMm > 0 && P.DepthMm > 0 && P.HeightMm > 0) : (P.DiameterMm > 0 && P.HeightMm > 0);
        if (!MarketCatalog::IsValidId(P.Id) || P.Name.IsEmpty() || P.Type.IsEmpty() || !bSizes)
        {
            if (OutErrors) OutErrors->Add(TEXT("Hatal\u0131 ambalaj kayd\u0131 atland\u0131: ") + (P.Id.IsEmpty() ? P.Name : P.Id));
            continue;
        }
        if (!P.IsBox())
        {
            if (P.Shape.IsEmpty()) P.Shape = P.Type == TEXT("teneke") ? TEXT("teneke") : P.Type == TEXT("kavanoz") ? TEXT("kavanoz") : P.Type == TEXT("kase") ? TEXT("kase") : TEXT("sise");
            if (P.LabelHeightMm <= 0) P.LabelHeightMm = P.HeightMm / 3;
        }
        if (P.Parts.IsEmpty())
            P.Parts = P.IsBox() ? TEXT("Etiket") : P.Type == TEXT("teneke") ? TEXT("Etiket,Kapak") : P.Type == TEXT("kase") ? TEXT("Etiket,Govde,Kapak") : TEXT("Etiket,Cam,Kapak");
        if (P.bTopCap && !P.Parts.Contains(TEXT("Kapak"))) P.Parts += TEXT(",Kapak");
        Result.Add(P);
    }
    return Result;
}

const FPackagePreset* FindPreset(const TArray<FPackagePreset>& Presets, const FString& Id)
{
    return Id.IsEmpty() ? nullptr : Presets.FindByPredicate([&](const FPackagePreset& P) { return P.Id == Id; });
}

FString PresetPackageId(const FPackagePreset& Preset)
{
    return Preset.IsBox() ? FBoxPackageLayout::PackageId(Preset.WidthMm, Preset.DepthMm, Preset.HeightMm) : TEXT("shape_") + Preset.Id;
}

UStaticMesh* EnsurePresetPackage(const FPackagePreset& Preset, FString& OutPackageId, FString& OutError)
{
    if (Preset.IsBox()) return CreateBoxPackage(Preset.WidthMm, Preset.DepthMm, Preset.HeightMm, OutPackageId, OutError);
    return CreateShapePackage(Preset, OutPackageId, OutError);
}

namespace
{
    struct FColorEntry { FString Slot; FString Value; };
    TArray<FColorEntry> SplitColors(const FString& Colors)
    {
        TArray<FColorEntry> Out;
        TArray<FString> Items;
        Colors.ParseIntoArray(Items, TEXT(";"));
        for (FString Item : Items)
        {
            FString Key, Value;
            if (!Item.Split(TEXT("="), &Key, &Value)) continue;
            Key.TrimStartAndEndInline();
            Value.TrimStartAndEndInline();
            if (!Key.IsEmpty() && !Value.IsEmpty()) Out.Add({ Key, Value });
        }
        return Out;
    }
    FString JoinColors(const TArray<FColorEntry>& Entries)
    {
        TArray<FString> Items;
        for (const FColorEntry& E : Entries) Items.Add(E.Slot + TEXT("=") + E.Value);
        return FString::Join(Items, TEXT(";"));
    }
    bool IsHex6(const FString& Hex)
    {
        if (Hex.Len() != 6) return false;
        for (TCHAR C : Hex) if (!FChar::IsHexDigit(C)) return false;
        return true;
    }
}

FString ColorText(const FString& Colors, const FString& Slot)
{
    for (const FColorEntry& E : SplitColors(Colors)) if (E.Slot.Equals(Slot, ESearchCase::IgnoreCase)) return E.Value;
    return FString();
}

bool FindColor(const FString& Colors, const FString& Slot, FLinearColor& OutColor, float& OutOpacity)
{
    FString Value = ColorText(Colors, Slot), Hex, Opacity;
    if (Value.IsEmpty()) return false;
    if (!Value.Split(TEXT("/"), &Hex, &Opacity)) Hex = Value;
    Hex.RemoveFromStart(TEXT("#"));
    if (!IsHex6(Hex)) return false;
    OutColor = FLinearColor(FColor::FromHex(Hex));
    OutOpacity = Opacity.IsEmpty() ? -1.f : FMath::Clamp(FCString::Atof(*Opacity.Replace(TEXT(","), TEXT("."))), 0.02f, 1.f);
    return true;
}

FString WithColor(const FString& Colors, const FString& Slot, const FString& Value)
{
    TArray<FColorEntry> Entries = SplitColors(Colors);
    Entries.RemoveAll([&](const FColorEntry& E) { return E.Slot.Equals(Slot, ESearchCase::IgnoreCase); });
    FString Clean = Value.TrimStartAndEnd();
    Clean.RemoveFromStart(TEXT("#"));
    if (!Clean.IsEmpty()) Entries.Add({ Slot, Clean.ToUpper() });
    return JoinColors(Entries);
}

void ApplyPreset(const FPackagePreset& Preset, FStudioDraft& Draft)
{
    const auto Mm = [](int32 Value) { return Value > 0 ? FString::FromInt(Value) : FString(); };
    Draft.Preset = Preset.Id;
    Draft.PackageType = Preset.Type;
    Draft.WidthMm = Mm(Preset.WidthMm);
    Draft.DepthMm = Mm(Preset.DepthMm);
    Draft.HeightMm = Mm(Preset.HeightMm);
    Draft.DiameterMm = Mm(Preset.DiameterMm);
    Draft.LabelHeightMm = Mm(Preset.LabelHeightMm);
    Draft.Parts = Preset.Parts;
    // Product colors win; the preset only fills parts the product has no color for.
    FString Colors = Draft.Colors;
    for (const FColorEntry& E : SplitColors(Preset.Colors))
        if (ColorText(Colors, E.Slot).IsEmpty()) Colors = WithColor(Colors, E.Slot, E.Value);
    // Drop colors of parts the new package does not have.
    for (const FColorEntry& E : SplitColors(Colors))
        if (!Preset.Parts.Contains(E.Slot)) Colors = WithColor(Colors, E.Slot, FString());
    Draft.Colors = Colors;
}

FString SuggestPreset(const TArray<FPackagePreset>& Presets, const FStudioDraft& Draft)
{
    const int32 W = FCString::Atoi(*Draft.WidthMm), D = FCString::Atoi(*Draft.DepthMm), H = FCString::Atoi(*Draft.HeightMm);
    const int32 Dia = FCString::Atoi(*Draft.DiameterMm);
    FString Best;
    double BestScore = TNumericLimits<double>::Max();
    for (const FPackagePreset& P : Presets)
    {
        if (P.Type != Draft.PackageType) continue;
        const double Score = P.IsBox()
            ? FMath::Abs(P.WidthMm - W) + FMath::Abs(P.DepthMm - D) + FMath::Abs(P.HeightMm - H)
            : FMath::Abs(P.DiameterMm - Dia) * 2.0 + FMath::Abs(P.HeightMm - H);
        if (Score < BestScore) { BestScore = Score; Best = P.Id; }
    }
    return Best;
}

// ---------------------------------------------------------------------------------------- turned shapes

namespace
{
    enum EShapeSlot : int32 { SlotLabel = 0, SlotBody, SlotCap, SlotCount };

    struct FProfilePoint { double R; double Z; }; // cm
    struct FProfileRun { TArray<FProfilePoint> Points; EShapeSlot Slot; };

    // Smooth step used for shoulders and domes.
    void AddCurve(TArray<FProfilePoint>& Out, double R0, double Z0, double R1, double Z1, int32 Steps)
    {
        for (int32 I = 1; I <= Steps; ++I)
        {
            const double T = double(I) / Steps;
            const double S = 0.5 - 0.5 * FMath::Cos(T * UE_PI);
            Out.Add({ FMath::Lerp(R0, R1, S), FMath::Lerp(Z0, Z1, T) });
        }
    }

    // Outline of the package from bottom center to top center, split into runs by part.
    // Every run starts where the previous one ended. Units: cm.
    TArray<FProfileRun> BuildProfile(const FPackagePreset& P, double& OutLabelZ0, double& OutLabelZ1, double& OutBottomR, double& OutTopR)
    {
        const double R = P.DiameterMm / 20.0;
        const double H = P.HeightMm / 10.0;
        const double LH = FMath::Min(P.LabelHeightMm / 10.0, H * 0.95);
        TArray<FProfileRun> Runs;
        const auto Run = [&Runs](EShapeSlot Slot, std::initializer_list<FProfilePoint> Points)
        {
            FProfileRun& New = Runs.AddDefaulted_GetRef();
            New.Slot = Slot;
            New.Points = Points;
            return &New;
        };
        const auto Label = [&](double Z0, double Z1, double R0, double R1)
        {
            OutLabelZ0 = Z0; OutLabelZ1 = Z1;
            Run(SlotLabel, { { R0, Z0 }, { R1, Z1 } });
        };
        const FString& S = P.Shape;
        if (S == TEXT("teneke") || S == TEXT("aerosol"))
        {
            const bool bSpray = S == TEXT("aerosol");
            const double Z0 = H * 0.03, Z1 = bSpray ? H * 0.76 : H * 0.95;
            OutBottomR = R * 0.9;
            Run(SlotCap, { { R * 0.9, 0 }, { R, Z0 } });
            const double Mid = (Z0 + Z1) / 2, Half = FMath::Min(LH, Z1 - Z0) / 2;
            if (Mid - Half > Z0 + 0.01) Run(SlotCap, { { R, Z0 }, { R, Mid - Half } });
            Label(Mid - Half, Mid + Half, R, R);
            if (Z1 > Mid + Half + 0.01) Run(SlotCap, { { R, Mid + Half }, { R, Z1 } });
            if (bSpray)
            {
                // Plastic cap over a metal dome.
                Run(SlotCap, { { R, Z1 }, { R * 0.97, Z1 }, { R * 0.97, H * 0.97 } });
                FProfileRun* Top = Run(SlotCap, { { R * 0.97, H * 0.97 } });
                AddCurve(Top->Points, R * 0.97, H * 0.97, R * 0.8, H, 3);
                OutTopR = R * 0.8;
            }
            else
            {
                Run(SlotCap, { { R, Z1 }, { R * 0.9, H } });
                OutTopR = R * 0.9;
            }
            return Runs;
        }
        if (S == TEXT("kase"))
        {
            // Cup / tub: slightly conical wall, flat printed lid with a small rim.
            const double RB = R * 0.82, ZL = H * 0.96;
            OutBottomR = RB;
            const double Z0 = FMath::Max(H * 0.04, ZL - LH), Z1 = ZL;
            const auto RAt = [&](double Z) { return FMath::Lerp(RB, R, Z / ZL); };
            Run(SlotBody, { { RB, 0 }, { RAt(Z0), Z0 } });
            Label(Z0, Z1, RAt(Z0), R);
            Run(SlotCap, { { R, ZL }, { R * 1.03, ZL }, { R * 1.03, H } });
            OutTopR = R * 1.03;
            return Runs;
        }
        if (S == TEXT("kavanoz"))
        {
            const double Shoulder = H * 0.8, Lid = H * 0.86, RL = R * 0.9;
            OutBottomR = R * 0.92;
            const double Z0 = FMath::Max(H * 0.05, (Shoulder - LH) / 2 + 0.02), Z1 = FMath::Min(Shoulder, Z0 + LH);
            Run(SlotBody, { { R * 0.92, 0 }, { R, H * 0.02 }, { R, Z0 } });
            Label(Z0, Z1, R, R);
            FProfileRun* Top = Run(SlotBody, { { R, Z1 }, { R, Shoulder } });
            AddCurve(Top->Points, R, Shoulder, R * 0.86, Lid, 3);
            Run(SlotCap, { { R * 0.86, Lid }, { RL, Lid }, { RL, H } });
            OutTopR = RL;
            return Runs;
        }
        // Bottles: sise (narrow neck), sise_genis (wide neck, detergent/shampoo), sikma (squeeze, big cap).
        const bool bWide = S == TEXT("sise_genis"), bSqueeze = S == TEXT("sikma");
        const double RN = P.NeckDiameterMm > 0 ? P.NeckDiameterMm / 20.0 : bSqueeze ? R * 0.5 : bWide ? R * 0.42 : FMath::Max(1.2, R * 0.36);
        const double CapH = P.CapHeightMm > 0 ? P.CapHeightMm / 10.0 : bSqueeze ? H * 0.16 : bWide ? H * 0.1 : 1.8;
        const double RC = bSqueeze ? R * 0.55 : RN * 1.12;
        const double CapZ = H - CapH;
        double Shoulder = bSqueeze ? H * 0.66 : bWide ? H * 0.78 : H * 0.6;
        const double NeckStart = bSqueeze ? CapZ : bWide ? H * 0.86 : H * 0.84;
        const double Base = H * 0.03;
        double Z0 = P.LabelBottomMm >= 0 ? P.LabelBottomMm / 10.0 : (Base + Shoulder) / 2 - LH / 2;
        Z0 = FMath::Max(Z0, Base + 0.05);
        double Z1 = Z0 + LH;
        if (Z1 > Shoulder - 0.05) Shoulder = FMath::Min(Z1 + 0.05, NeckStart - 0.5);
        Z1 = FMath::Min(Z1, Shoulder);
        OutBottomR = R * 0.85;
        Run(SlotBody, { { R * 0.85, 0 }, { R, Base }, { R, Z0 } });
        Label(Z0, Z1, R, R);
        FProfileRun* Upper = Run(SlotBody, { { R, Z1 }, { R, Shoulder } });
        AddCurve(Upper->Points, R, Shoulder, RN, NeckStart, 6);
        if (CapZ > NeckStart + 0.01) Upper->Points.Add({ RN, CapZ });
        Run(SlotCap, { { RN, CapZ }, { RC, CapZ }, { RC, H } });
        OutTopR = RC;
        return Runs;
    }
}

UStaticMesh* CreateShapePackage(const FPackagePreset& Preset, FString& OutPackageId, FString& OutError)
{
    OutPackageId = PresetPackageId(Preset);
    const FString Folder = PackageFolder(OutPackageId);
    const FString Name = TEXT("SM_") + OutPackageId;
    if (UStaticMesh* Existing = LoadObject<UStaticMesh>(nullptr, *ObjectPath(Folder, Name), nullptr, LOAD_NoWarn | LOAD_Quiet))
        return Existing;
    if (Preset.DiameterMm < 10 || Preset.DiameterMm > 600 || Preset.HeightMm < 10 || Preset.HeightMm > 1000)
    {
        OutError = TEXT("Ambalaj \u00f6l\u00e7\u00fcs\u00fc ge\u00e7ersiz: ") + Preset.Id;
        return nullptr;
    }
    // Slot names come from the parts list: body is "Govde" for opaque plastic, otherwise "Cam".
    const FString BodyName = Preset.Parts.Contains(TEXT("Govde")) ? TEXT("Govde") : TEXT("Cam");
    const FString SlotNames[SlotCount] = { TEXT("Etiket"), BodyName, TEXT("Kapak") };

    double LabelZ0 = 0, LabelZ1 = 1, BottomR = 1, TopR = 1;
    const TArray<FProfileRun> Runs = BuildProfile(Preset, LabelZ0, LabelZ1, BottomR, TopR);
    bool bUsed[SlotCount] = { true, false, false };
    for (const FProfileRun& Run : Runs) bUsed[Run.Slot] = true;
    const EShapeSlot BottomSlot = Preset.Shape == TEXT("teneke") || Preset.Shape == TEXT("aerosol") ? SlotCap : SlotBody;
    bUsed[BottomSlot] = true;
    bUsed[SlotCap] = true;

    FMeshDescription Mesh;
    FStaticMeshAttributes Attributes(Mesh);
    Attributes.Register();
    auto Positions = Attributes.GetVertexPositions();
    auto Normals = Attributes.GetVertexInstanceNormals();
    auto UVs = Attributes.GetVertexInstanceUVs();
    UVs.SetNumChannels(1);
    FPolygonGroupID Groups[SlotCount];
    int32 MaterialIndex[SlotCount] = { INDEX_NONE, INDEX_NONE, INDEX_NONE };
    TArray<FName> MaterialNames;
    for (int32 Slot = 0; Slot < SlotCount; ++Slot)
    {
        if (!bUsed[Slot]) continue;
        Groups[Slot] = Mesh.CreatePolygonGroup();
        Attributes.GetPolygonGroupMaterialSlotNames()[Groups[Slot]] = FName(*SlotNames[Slot]);
        MaterialIndex[Slot] = MaterialNames.Add(FName(*SlotNames[Slot]));
    }

    constexpr int32 Sides = 64;
    // u = 0.5 faces the product front (+X); increasing u moves to the viewer's right (-Y).
    const auto Ring = [](double R, double Z, double U) { const double A = (U - 0.5) * 2.0 * UE_PI; return FVector(R * FMath::Cos(A), -R * FMath::Sin(A), Z); };
    const auto RadialNormal = [](double NR, double NZ, double U) { const double A = (U - 0.5) * 2.0 * UE_PI; return FVector(NR * FMath::Cos(A), -NR * FMath::Sin(A), NZ).GetSafeNormal(); };
    const auto TopViewUV = [](const FVector& P, double Radius) { return FVector2D(0.5 - 0.49 * P.Y / Radius, 0.5 + 0.49 * P.X / Radius); };
    const auto Triangle = [&](EShapeSlot Slot, const FVertexInstanceID (&Ids)[3], const FVector (&P)[3], const FVector& Outward)
    {
        const FVector Facing = FVector::CrossProduct(P[2] - P[0], P[1] - P[0]);
        if (Facing.SizeSquared() < 1e-12) return;
        if (FVector::DotProduct(Facing, Outward) > 0) Mesh.CreateTriangle(Groups[Slot], TArray<FVertexInstanceID>{ Ids[0], Ids[1], Ids[2] });
        else Mesh.CreateTriangle(Groups[Slot], TArray<FVertexInstanceID>{ Ids[0], Ids[2], Ids[1] });
    };
    const auto Vertex = [&](const FVector& P, const FVector& N, const FVector2D& UV)
    {
        const FVertexID V = Mesh.CreateVertex();
        Positions[V] = FVector3f(P);
        const FVertexInstanceID I = Mesh.CreateVertexInstance(V);
        Normals[I] = FVector3f(N);
        UVs.Set(I, 0, FVector2f(UV));
        return I;
    };

    // Side walls: one band per profile segment, normals smoothed inside a run.
    const double MaxR = Preset.DiameterMm / 20.0 * 1.05;
    for (const FProfileRun& Run : Runs)
    {
        const TArray<FProfilePoint>& Pts = Run.Points;
        if (Pts.Num() < 2) continue;
        TArray<FVector2D> SegN; // (nr, nz) per segment
        for (int32 K = 0; K + 1 < Pts.Num(); ++K)
            SegN.Add(FVector2D(Pts[K + 1].Z - Pts[K].Z, -(Pts[K + 1].R - Pts[K].R)).GetSafeNormal());
        for (int32 K = 0; K + 1 < Pts.Num(); ++K)
        {
            const FProfilePoint A = Pts[K], B = Pts[K + 1];
            if (FMath::IsNearlyEqual(A.R, B.R) && FMath::IsNearlyEqual(A.Z, B.Z)) continue;
            const auto JointN = [&](int32 Joint, int32 Seg)
            {
                // Average with the neighbour segment when the bend is gentle (< 40 degrees).
                const int32 Other = Joint == Seg ? Seg - 1 : Seg + 1;
                if (!SegN.IsValidIndex(Other) || FVector2D::DotProduct(SegN[Seg], SegN[Other]) < 0.77) return SegN[Seg];
                return (SegN[Seg] + SegN[Other]).GetSafeNormal();
            };
            const FVector2D NA = JointN(K, K), NB = JointN(K + 1, K);
            TArray<FVertexInstanceID> RowA, RowB;
            TArray<FVector> PosA, PosB;
            for (int32 I = 0; I <= Sides; ++I)
            {
                const double U = double(I) / Sides;
                const FVector PA = Ring(A.R, A.Z, U), PB = Ring(B.R, B.Z, U);
                FVector2D UVA, UVB;
                if (Run.Slot == SlotLabel)
                {
                    UVA = FVector2D(U, (LabelZ1 - A.Z) / FMath::Max(0.01, LabelZ1 - LabelZ0));
                    UVB = FVector2D(U, (LabelZ1 - B.Z) / FMath::Max(0.01, LabelZ1 - LabelZ0));
                }
                else
                {
                    UVA = TopViewUV(PA, FMath::Max(MaxR, A.R));
                    UVB = TopViewUV(PB, FMath::Max(MaxR, B.R));
                }
                RowA.Add(Vertex(PA, RadialNormal(NA.X, NA.Y, U), UVA)); PosA.Add(PA);
                RowB.Add(Vertex(PB, RadialNormal(NB.X, NB.Y, U), UVB)); PosB.Add(PB);
            }
            for (int32 I = 0; I < Sides; ++I)
            {
                const double U = (I + 0.5) / Sides;
                const FVector2D Mid = SegN[K];
                const FVector Out = RadialNormal(Mid.X, Mid.Y, U);
                Triangle(Run.Slot, { RowA[I], RowA[I + 1], RowB[I + 1] }, { PosA[I], PosA[I + 1], PosB[I + 1] }, Out);
                Triangle(Run.Slot, { RowA[I], RowB[I + 1], RowB[I] }, { PosA[I], PosB[I + 1], PosB[I] }, Out);
            }
        }
    }
    // Bottom and top discs.
    const auto Disc = [&](EShapeSlot Slot, double R, double Z, bool bUp)
    {
        const FVector N(0, 0, bUp ? 1 : -1);
        const FVector C(0, 0, Z);
        const FVertexInstanceID Center = Vertex(C, N, FVector2D(0.5, 0.5));
        TArray<FVertexInstanceID> Rim;
        TArray<FVector> RimPos;
        for (int32 I = 0; I <= Sides; ++I)
        {
            const FVector P = Ring(R, Z, double(I) / Sides);
            Rim.Add(Vertex(P, N, TopViewUV(P, R)));
            RimPos.Add(P);
        }
        for (int32 I = 0; I < Sides; ++I) Triangle(Slot, { Center, Rim[I], Rim[I + 1] }, { C, RimPos[I], RimPos[I + 1] }, N);
    };
    Disc(BottomSlot, BottomR, 0.0, false);
    Disc(SlotCap, TopR, Preset.HeightMm / 10.0, true);

    // Package-level materials: label master for Etiket, glass/solid instances for the rest.
    UMaterialInterface* LabelMaster = EnsureLabelMaterial(OutError);
    UMaterialInterface* Glass = EnsureGlassMaterial(OutError);
    UMaterialInterface* Solid = EnsureSolidMaterial(OutError);
    if (!LabelMaster || !Glass || !Solid) return nullptr;
    TArray<UObject*> ToSave;
    TArray<UMaterialInterface*> SlotMaterials;
    SlotMaterials.Init(nullptr, MaterialNames.Num());
    SlotMaterials[MaterialIndex[SlotLabel]] = LabelMaster;
    for (int32 Slot = SlotBody; Slot < SlotCount; ++Slot)
    {
        if (MaterialIndex[Slot] == INDEX_NONE) continue;
        const bool bGlass = Slot == SlotBody && BodyName == TEXT("Cam");
        FLinearColor Tint = bGlass ? FLinearColor(FColor(0xD9, 0xEE, 0xE6)) : FLinearColor(FColor(0xEE, 0xEE, 0xEE));
        float Opacity = bGlass ? 0.3f : 1.f;
        FLinearColor PresetTint; float PresetOpacity = -1.f;
        if (FindColor(Preset.Colors, SlotNames[Slot], PresetTint, PresetOpacity)) { Tint = PresetTint; if (PresetOpacity > 0) Opacity = PresetOpacity; }
        const bool bMetal = Slot == SlotCap && Preset.bMetalCap;
        UMaterialInstanceConstant* Instance = WriteSlotMaterial(Folder, TEXT("MI_") + OutPackageId + TEXT("_") + MarketCatalog::MakeId(SlotNames[Slot]),
            bGlass ? Glass : Solid, Tint, bGlass ? 0.05f : bMetal ? 0.3f : 0.4f, bMetal ? 0.9f : 0.f, Opacity, OutError);
        if (!Instance) return nullptr;
        ToSave.Add(Instance);
        SlotMaterials[MaterialIndex[Slot]] = Instance;
    }

    UPackage* Package = OpenPackage(Folder, Name);
    UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, *Name, RF_Public | RF_Standalone | RF_Transactional);
    StaticMesh->InitResources();
    StaticMesh->SetLightingGuid();
    FStaticMeshSourceModel& Source = StaticMesh->AddSourceModel();
    Source.BuildSettings.bRecomputeNormals = false;
    Source.BuildSettings.bRecomputeTangents = true;
    Source.BuildSettings.bRemoveDegenerates = true;
    Source.BuildSettings.bGenerateLightmapUVs = true;
    Source.BuildSettings.SrcLightmapIndex = 0;
    Source.BuildSettings.DstLightmapIndex = 1;
    StaticMesh->CreateMeshDescription(0, MoveTemp(Mesh));
    StaticMesh->CommitMeshDescription(0);
    StaticMesh->SetLightMapCoordinateIndex(1);
    for (int32 I = 0; I < MaterialNames.Num(); ++I)
        StaticMesh->GetStaticMaterials().Add(FStaticMaterial(SlotMaterials[I], MaterialNames[I], MaterialNames[I]));
    StaticMesh->CreateBodySetup();
    if (UBodySetup* Body = StaticMesh->GetBodySetup())
    {
        const double R = Preset.DiameterMm / 20.0, H = Preset.HeightMm / 10.0;
        FKBoxElem Box(R * 2, R * 2, H);
        Box.Center = FVector(0, 0, H / 2);
        Body->AggGeom.BoxElems.Add(Box);
    }
    StaticMesh->Build(false);
    StaticMesh->PostEditChange();
    Package->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(StaticMesh);
    ToSave.Add(StaticMesh);
    if (!SaveAll(ToSave)) { OutError = TEXT("Ambalaj modeli kaydedilemedi."); return nullptr; }

    const TSharedRef<FJsonObject> Meta = MakeShared<FJsonObject>();
    Meta->SetNumberField(TEXT("schemaVersion"), 1);
    Meta->SetStringField(TEXT("id"), OutPackageId);
    Meta->SetStringField(TEXT("type"), TEXT("shape"));
    Meta->SetStringField(TEXT("preset"), Preset.Id);
    Meta->SetStringField(TEXT("shape"), Preset.Shape);
    Meta->SetNumberField(TEXT("diameterMm"), Preset.DiameterMm);
    Meta->SetNumberField(TEXT("heightMm"), Preset.HeightMm);
    Meta->SetNumberField(TEXT("labelHeightMm"), FMath::RoundToInt32((LabelZ1 - LabelZ0) * 10.0));
    Meta->SetStringField(TEXT("mesh"), StaticMesh->GetPathName());
    IFileManager::Get().MakeDirectory(*(InboxDir() / TEXT("Packages") / OutPackageId), true);
    WriteJson(InboxDir() / TEXT("Packages") / OutPackageId / TEXT("package.json"), Meta);
    return StaticMesh;
}

TArray<FStudioPackage> ScanPackages(const TArray<FMarketProduct>& Catalog)
{
    TArray<FStudioPackage> Result;
    const TArray<FPackagePreset> Presets = LoadPresets();
    IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
    Registry.ScanPathsSynchronous({ TEXT("/Game/Products/Packages") }, true);
    TArray<FAssetData> Assets;
    Registry.GetAssetsByPath(FName(TEXT("/Game/Products/Packages")), Assets, true);
    for (const FAssetData& Asset : Assets)
    {
        if (Asset.AssetClassPath != UStaticMesh::StaticClass()->GetClassPathName()) continue;
        FStudioPackage Package;
        Package.Id = FPaths::GetCleanFilename(Asset.PackagePath.ToString());
        if (Result.ContainsByPredicate([&](const FStudioPackage& P) { return P.Id == Package.Id; })) continue; // one mesh per package folder
        Package.MeshPath = Asset.GetObjectPathString();
        UStaticMesh* Mesh = Cast<UStaticMesh>(Asset.GetAsset());
        if (Mesh)
            for (const FStaticMaterial& Slot : Mesh->GetStaticMaterials()) Package.SlotNames.Add(Slot.MaterialSlotName.ToString());
        if (FBoxPackageLayout::ParsePackageId(Package.Id, Package.WidthMm, Package.DepthMm, Package.HeightMm))
        {
            Package.Kind = EPackageKind::Box;
            Package.DisplayName = FString::Printf(TEXT("Kutu %d\u00d7%d\u00d7%d mm"), Package.WidthMm, Package.DepthMm, Package.HeightMm);
            Package.SizeCm = FVector(Package.DepthMm, Package.WidthMm, Package.HeightMm) / 10.0;
            Package.LabelSlot = 0;
            for (const FPackagePreset& Preset : Presets)
                if (Preset.IsBox() && PresetPackageId(Preset) == Package.Id) { Package.DisplayName = Preset.Name; break; }
        }
        else
        {
            Package.Kind = EPackageKind::Model;
            Package.DisplayName = Package.Id.StartsWith(TEXT("model_")) ? Package.Id.Mid(6) : Package.Id;
            if (Package.Id.StartsWith(TEXT("shape_")))
                if (const FPackagePreset* Preset = FindPreset(Presets, Package.Id.Mid(6))) Package.DisplayName = Preset->Name;
            if (Mesh) Package.SizeCm = Mesh->GetBoundingBox().GetSize();
            const int32 Label = FindSlot(Package, ESlotRole::Label);
            Package.LabelSlot = Label != INDEX_NONE ? Label : 0;
            Package.CapSlot = FindSlot(Package, ESlotRole::Cap);
            Package.BodySlot = FindSlot(Package, ESlotRole::Body);
        }
        for (const FMarketProduct& Product : Catalog)
            if (Product.MeshPath == Package.MeshPath) ++Package.UsedBy;
        Result.Add(Package);
    }
    Result.Sort([](const FStudioPackage& A, const FStudioPackage& B) { return A.Kind != B.Kind ? A.Kind < B.Kind : A.Id < B.Id; });
    return Result;
}

const FStudioPackage* FindPackage(const TArray<FStudioPackage>& Packages, const FString& Id)
{
    return Packages.FindByPredicate([&](const FStudioPackage& P) { return P.Id == Id; });
}

FString PackageIdForMesh(const TArray<FStudioPackage>& Packages, const FString& MeshPath)
{
    const FStudioPackage* Found = Packages.FindByPredicate([&](const FStudioPackage& P) { return P.MeshPath == MeshPath; });
    return Found ? Found->Id : FString();
}

// ---------------------------------------------------------------------------------------- products

namespace
{
    bool HasExisting(const FStudioDraft& Draft, int32 Slot)
    {
        return Draft.ExistingMaterials.IsValidIndex(Slot) && !Draft.ExistingMaterials[Slot].IsEmpty();
    }

    bool ParseDraftFloat(const FString& Text, float& OutValue)
    {
        FString Normalized = Text.TrimStartAndEnd().Replace(TEXT(","), TEXT("."));
        return FDefaultValueHelper::ParseFloat(Normalized, OutValue) && FMath::IsFinite(OutValue);
    }

    bool ParseDraftFloat(const FString& Text, double& OutValue)
    {
        FString Normalized = Text.TrimStartAndEnd().Replace(TEXT(","), TEXT("."));
        return FDefaultValueHelper::ParseDouble(Normalized, OutValue) && FMath::IsFinite(OutValue);
    }
}

TArray<FStudioIssue> Validate(const FStudioDraft& Draft, const TArray<FMarketProduct>& Catalog, const TArray<FStudioPackage>& Packages, bool bForGame)
{
    TArray<FStudioIssue> Issues;
    const auto Error = [&](const FString& Text) { Issues.Add(FStudioIssue{ EStudioSeverity::Error, Text }); };
    const auto Warn = [&](const FString& Text) { Issues.Add(FStudioIssue{ EStudioSeverity::Warning, Text }); };
    if (Draft.RealName.TrimStartAndEnd().IsEmpty()) Error(TEXT("\u00dcr\u00fcn ad\u0131 gerekli."));
    if (!MarketCatalog::IsValidId(Draft.Id)) Error(TEXT("\u00dcr\u00fcn kimli\u011fi 3\u201340 karakter olmal\u0131: k\u00fc\u00e7\u00fck harf, rakam ve _ (\u00f6rn. sutas_sut_1l)."));
    else if (!Draft.bExisting && Catalog.ContainsByPredicate([&](const FMarketProduct& P) { return P.Id == Draft.Id; }))
        Error(TEXT("Bu kimlik ba\u015fka bir \u00fcr\u00fcnde kullan\u0131l\u0131yor: ") + Draft.Id);
    const bool bAlreadyActive = Catalog.ContainsByPredicate([&](const FMarketProduct& P) { return P.Id == Draft.Id && P.bActive; });
    (void)bAlreadyActive; // no upper limit on active products since 28.09.2026
    int64 Cost = 0, Price = 0;
    const bool bCost = MarketCatalog::ParseMoney(Draft.Cost, Cost);
    const bool bPrice = MarketCatalog::ParseMoney(Draft.Price, Price);
    if (!bCost) Error(TEXT("Maliyet ge\u00e7ersiz (\u00f6rn. 1,70)."));
    if (!bPrice) Error(TEXT("Sat\u0131\u015f fiyat\u0131 ge\u00e7ersiz (\u00f6rn. 2,50)."));
    if (bCost && bPrice && Price <= Cost) Warn(TEXT("Sat\u0131\u015f fiyat\u0131 maliyetten y\u00fcksek de\u011fil: her sat\u0131\u015f zarar ettirir."));
    const int32 Units = FCString::Atoi(*Draft.CaseUnits);
    if (!Draft.CaseUnits.IsNumeric() || Units < 1 || Units > 48) Error(TEXT("Koli adedi 1\u201348 aras\u0131nda olmal\u0131."));
    const FStudioPackage* Package = FindPackage(Packages, Draft.PackageId);
    if (Package && Package->Id.StartsWith(TEXT("model_")))
    {
        float Scale = 1.f;
        if (!ParseDraftFloat(Draft.ModelScale, Scale) || Scale < 0.001f || Scale > 1000.f)
            Error(TEXT("Model \u00f6l\u00e7e\u011fi 0,001\u20131000 aras\u0131nda olmal\u0131."));
        const FString* Rotations[] = { &Draft.ModelPitch, &Draft.ModelYaw, &Draft.ModelRoll };
        for (const FString* Value : Rotations)
        {
            float Number = 0.f;
            if (!ParseDraftFloat(*Value, Number) || FMath::Abs(Number) > 3600.f)
            {
                Error(TEXT("Model d\u00f6n\u00fc\u015f\u00fc ge\u00e7ersiz; derece olarak -3600\u20133600 kullan."));
                break;
            }
        }
        const FString* Offsets[] = { &Draft.ModelOffsetX, &Draft.ModelOffsetY, &Draft.ModelOffsetZ };
        for (const FString* Value : Offsets)
        {
            float Number = 0.f;
            if (!ParseDraftFloat(*Value, Number) || FMath::Abs(Number) > 1000.f)
            {
                Error(TEXT("Model raf konumu ge\u00e7ersiz; santimetre olarak -1000\u20131000 kullan."));
                break;
            }
        }
    }
    if (!bForGame)
    {
        if (!Draft.PackageId.IsEmpty() && !Package) Warn(TEXT("Se\u00e7ili 3B ambalaj bulunamad\u0131: ") + Draft.PackageId);
        return Issues; // preparation list: product data is enough
    }
    if (Draft.PackageId.IsEmpty()) Error(TEXT("Oyuna eklemek i\u00e7in 3B ambalaj gerekli: kutu \u015fablonu olu\u015ftur ya da modeli i\u00e7e al."));
    else if (!Package) Error(TEXT("Se\u00e7ili ambalaj bulunamad\u0131: ") + Draft.PackageId);
    else if (Package->Kind == EPackageKind::Box)
    {
        if (Draft.NetFile.IsEmpty() && Draft.FaceFiles[FBoxPackageLayout::Front].IsEmpty() && !HasExisting(Draft, 0))
            Error(TEXT("\u00d6n y\u00fcz g\u00f6rseli ya da tek g\u00f6rsel a\u00e7\u0131l\u0131m gerekli."));
    }
    else
    {
        if (Draft.LabelFile.IsEmpty() && !HasExisting(Draft, Package->LabelSlot))
            Warn(TEXT("Etiket yok: modelin kendi dokusu kullan\u0131lacak."));
        if (Package->SlotNames.Num() > 1 && FindSlot(*Package, ESlotRole::Label) == INDEX_NONE)
            Warn(TEXT("Modelde 'Etiket' adl\u0131 malzeme yuvas\u0131 yok; etiket ilk yuvaya uygulanacak."));
        if (!Draft.CapFile.IsEmpty() && Package->CapSlot == INDEX_NONE) Warn(TEXT("Kapak g\u00f6rseli se\u00e7ildi ama modelde 'Kapak' yuvas\u0131 yok; kullan\u0131lmayacak."));
        if (!Draft.BodyFile.IsEmpty() && Package->BodySlot == INDEX_NONE) Warn(TEXT("G\u00f6vde g\u00f6rseli se\u00e7ildi ama modelde 'Govde' yuvas\u0131 yok; kullan\u0131lmayacak."));
    }
    if (Package && Package->Kind == EPackageKind::Model)
    {
        FVector CorrectedSize = Package->SizeCm;
        if (Package->Id.StartsWith(TEXT("model_")))
        {
            float Scale = 1.f;
            double Pitch = 0, Yaw = 0, Roll = 0;
            ParseDraftFloat(Draft.ModelScale, Scale);
            ParseDraftFloat(Draft.ModelPitch, Pitch);
            ParseDraftFloat(Draft.ModelYaw, Yaw);
            ParseDraftFloat(Draft.ModelRoll, Roll);
            const FBox Source(-Package->SizeCm * 0.5, Package->SizeCm * 0.5);
            CorrectedSize = Source.TransformBy(FTransform(FRotator(Pitch, Yaw, Roll), FVector::ZeroVector, FVector(Scale))).GetSize();
        }
        if (CorrectedSize.GetMax() > 150 || CorrectedSize.GetMax() < 1)
            Warn(FString::Printf(TEXT("D\u00fczeltilmi\u015f model \u00f6l\u00e7\u00fcs\u00fc %.1f\u00d7%.1f\u00d7%.1f cm; birim hatas\u0131 olabilir (1 birim = 1 cm)."), CorrectedSize.X, CorrectedSize.Y, CorrectedSize.Z));
    }
    return Issues;
}

bool HasErrors(const TArray<FStudioIssue>& Issues)
{
    return Issues.ContainsByPredicate([](const FStudioIssue& I) { return I.Severity == EStudioSeverity::Error; });
}

bool Publish(const FStudioDraft& Draft, const TArray<FStudioPackage>& Packages, TArray<FMarketProduct>& Catalog, const FString& Note, bool bActivate, FString& OutMessage)
{
    const TArray<FStudioIssue> Issues = Validate(Draft, Catalog, Packages, bActivate);
    if (HasErrors(Issues))
    {
        for (const FStudioIssue& Issue : Issues) if (Issue.Severity == EStudioSeverity::Error) { OutMessage = Issue.Text; break; }
        return false;
    }
    const FStudioPackage* PackagePtr = FindPackage(Packages, Draft.PackageId);
    FString Error;
    FMarketProduct* Existing = Catalog.FindByPredicate([&](const FMarketProduct& P) { return P.Id == Draft.Id; });
    FMarketProduct Product = Existing ? *Existing : FMarketProduct();
    Product.Id = Draft.Id;
    Product.RealName = Draft.RealName.TrimStartAndEnd();
    Product.FictionalName = Draft.FictionalName.TrimStartAndEnd().IsEmpty() ? Product.RealName : Draft.FictionalName.TrimStartAndEnd();
    Product.Category = Draft.Category.TrimStartAndEnd();
    MarketCatalog::ParseMoney(Draft.Cost, Product.Cost);
    MarketCatalog::ParseMoney(Draft.Price, Product.BasePrice);
    Product.CaseUnits = FMath::Clamp(FCString::Atoi(*Draft.CaseUnits), 1, 48);
    Product.Brand = Draft.Brand.TrimStartAndEnd();
    Product.PackageType = Draft.PackageType;
    Product.WidthMm = FCString::Atoi(*Draft.WidthMm);
    Product.DepthMm = FCString::Atoi(*Draft.DepthMm);
    Product.HeightMm = FCString::Atoi(*Draft.HeightMm);
    Product.DiameterMm = FCString::Atoi(*Draft.DiameterMm);
    Product.LabelHeightMm = FCString::Atoi(*Draft.LabelHeightMm);
    Product.Parts = Draft.Parts.TrimStartAndEnd();
    Product.Notes = Draft.Notes.TrimStartAndEnd();
    Product.bSizeEstimated = Draft.bSizeEstimated;
    Product.Preset = Draft.Preset;
    Product.Colors = Draft.Colors;
    Product.VisualScale = 1.f;
    Product.VisualRotation = FRotator::ZeroRotator;
    Product.VisualOffsetCm = FVector::ZeroVector;
    if (PackagePtr && PackagePtr->Id.StartsWith(TEXT("model_")))
    {
        ParseDraftFloat(Draft.ModelScale, Product.VisualScale);
        ParseDraftFloat(Draft.ModelPitch, Product.VisualRotation.Pitch);
        ParseDraftFloat(Draft.ModelYaw, Product.VisualRotation.Yaw);
        ParseDraftFloat(Draft.ModelRoll, Product.VisualRotation.Roll);
        ParseDraftFloat(Draft.ModelOffsetX, Product.VisualOffsetCm.X);
        ParseDraftFloat(Draft.ModelOffsetY, Product.VisualOffsetCm.Y);
        ParseDraftFloat(Draft.ModelOffsetZ, Product.VisualOffsetCm.Z);
    }
    Product.bActive = bActivate || (Existing && Existing->bActive);
    if (!PackagePtr)
    {
        // Preparation list only: no 3D package yet.
        Product.MeshPath.Reset();
        Product.Materials.Reset();
        if (Existing) *Existing = Product;
        else Catalog.Add(Product);
        if (!MarketCatalog::SaveFile(MarketCatalog::DefaultPath(), Catalog, Note, Error))
        {
            if (!Existing) Catalog.Pop();
            OutMessage = Error;
            return false;
        }
        OutMessage = Product.RealName + TEXT(" kaydedildi (haz\u0131rl\u0131k listesi).");
        return true;
    }
    const FStudioPackage& Package = *PackagePtr;
    Product.MeshPath = Package.MeshPath;
    // Start from what the product already had on this package; new images replace their slot.
    Product.Materials = Draft.ExistingMaterials;
    Product.Materials.SetNum(FMath::Max(1, Package.SlotNames.Num()));

    const FString ProductInbox = InboxDir() / TEXT("Products") / Draft.Id;
    const FString Folder = ItemFolder(Draft.Id);
    const TSharedRef<FJsonObject> Manifest = MakeShared<FJsonObject>();
    const TSharedRef<FJsonObject> Labels = MakeShared<FJsonObject>();
    TArray<UObject*> ToSave;
    UMaterialInterface* Parent = EnsureLabelMaterial(Error);
    if (!Parent) { OutMessage = Error; return false; }

    const auto Remember = [&](const FString& Source, const FString& Key)
    {
        const FString Copy = CopySource(Source, ProductInbox / TEXT("Labels"), Key);
        if (!Copy.IsEmpty()) Labels->SetStringField(Key, TEXT("Labels/") + FPaths::GetCleanFilename(Copy));
    };
    // Writes texture + material instance for one slot of this product.
    const auto WriteSlot = [&](int32 Slot, const FString& Suffix, const FStudioImage& Image) -> bool
    {
        UTexture2D* Texture = WriteTexture(Folder, TEXT("T_") + Draft.Id + Suffix, Image, Error);
        if (!Texture) return false;
        UMaterialInstanceConstant* Instance = WriteMaterialInstance(Folder, TEXT("MI_") + Draft.Id + (Suffix == TEXT("_Label") ? FString() : Suffix), Parent, Texture, Error);
        if (!Instance) return false;
        ToSave.Add(Texture);
        ToSave.Add(Instance);
        if (Product.Materials.IsValidIndex(Slot)) Product.Materials[Slot] = Instance->GetPathName();
        return true;
    };
    const auto LoadLimited = [&](const FString& File, FStudioImage& Out) -> bool
    {
        if (!LoadImageFile(File, Out, Error)) return false;
        if (Out.Width > MaxLabelSize || Out.Height > MaxLabelSize)
        {
            const double Scale = double(MaxLabelSize) / FMath::Max(Out.Width, Out.Height);
            FStudioImage Smaller;
            Resize(Out, FMath::RoundToInt32(Out.Width * Scale), FMath::RoundToInt32(Out.Height * Scale), Smaller);
            Out = MoveTemp(Smaller);
        }
        return true;
    };

    if (Package.Kind == EPackageKind::Box)
    {
        FBoxPackageLayout Layout;
        FBoxPackageLayout::Make(Package.WidthMm, Package.DepthMm, Package.HeightMm, Layout, LabelAtlasSize);
        FStudioImage FaceImages[FBoxPackageLayout::FaceCount];
        const FStudioImage* FacePointers[FBoxPackageLayout::FaceCount] = {};
        if (!Draft.NetFile.IsEmpty())
        {
            FStudioImage Net;
            if (!LoadImageFile(Draft.NetFile, Net, Error)) { OutMessage = Error; return false; }
            SplitNet(Draft.NetFile, Net, Package.WidthMm, Package.DepthMm, Package.HeightMm, FaceImages, nullptr);
            for (int32 F = 0; F < FBoxPackageLayout::FaceCount; ++F) FacePointers[F] = &FaceImages[F];
            const FString RectsFile = NetRectsFile(Draft.NetFile);
            Remember(Draft.NetFile, TEXT("net"));
            if (!RectsFile.IsEmpty()) CopySource(RectsFile, ProductInbox / TEXT("Labels"), TEXT("net")); // -> net.json beside net.<ext>
        }
        for (int32 F = 0; F < FBoxPackageLayout::FaceCount; ++F)
        {
            if (Draft.FaceFiles[F].IsEmpty()) continue;
            if (!LoadImageFile(Draft.FaceFiles[F], FaceImages[F], Error)) { OutMessage = Error; return false; }
            FacePointers[F] = &FaceImages[F];
            Remember(Draft.FaceFiles[F], FBoxPackageLayout::FaceKey(F));
        }
        if (FacePointers[FBoxPackageLayout::Front])
        {
            FStudioImage Atlas;
            ComposeAtlas(Layout, FacePointers, Atlas, nullptr);
            if (!WriteSlot(0, TEXT("_Label"), Atlas)) { OutMessage = Error; return false; }
            Product.Color = AverageBorder(*FacePointers[FBoxPackageLayout::Front]);
        }
        Manifest->SetStringField(TEXT("labelMode"), Draft.NetFile.IsEmpty() ? TEXT("box_faces") : TEXT("box_net"));
    }
    else
    {
        FStudioImage Image;
        if (!Draft.LabelFile.IsEmpty())
        {
            if (!LoadLimited(Draft.LabelFile, Image) || !WriteSlot(Package.LabelSlot, TEXT("_Label"), Image)) { OutMessage = Error; return false; }
            Product.Color = AverageBorder(Image);
            Remember(Draft.LabelFile, TEXT("label"));
        }
        if (!Draft.CapFile.IsEmpty() && Package.CapSlot != INDEX_NONE)
        {
            if (!LoadLimited(Draft.CapFile, Image) || !WriteSlot(Package.CapSlot, TEXT("_Kapak"), Image)) { OutMessage = Error; return false; }
            Remember(Draft.CapFile, TEXT("cap"));
        }
        if (!Draft.BodyFile.IsEmpty() && Package.BodySlot != INDEX_NONE)
        {
            if (!LoadLimited(Draft.BodyFile, Image) || !WriteSlot(Package.BodySlot, TEXT("_Govde"), Image)) { OutMessage = Error; return false; }
            Remember(Draft.BodyFile, TEXT("body"));
        }
        // Part colors (Cam / Govde / Kapak) for slots without their own image: a product instance of
        // the package's slot material with only Color (and Opacity) changed.
        UStaticMesh* PackageMesh = LoadObject<UStaticMesh>(nullptr, *Package.MeshPath);
        for (int32 Slot = 0; PackageMesh && Slot < Package.SlotNames.Num(); ++Slot)
        {
            const FString& SlotName = Package.SlotNames[Slot];
            const ESlotRole Role = SlotRole(SlotName);
            if (Role == ESlotRole::Label || Role == ESlotRole::Other) continue;
            if ((Role == ESlotRole::Cap && !Draft.CapFile.IsEmpty()) || (Role == ESlotRole::Body && !Draft.BodyFile.IsEmpty())) continue;
            FLinearColor Tint; float Opacity = -1.f;
            UMaterialInterface* Base = PackageMesh->GetMaterial(Slot);
            if (!FindColor(Draft.Colors, SlotName, Tint, Opacity) || !Base || Base->IsA<UMaterial>())
            {
                if (Product.Materials.IsValidIndex(Slot)) Product.Materials[Slot].Reset(); // package default
                continue;
            }
            const FString Name = TEXT("MI_") + Draft.Id + TEXT("_") + MarketCatalog::MakeId(SlotName);
            UPackage* Asset = OpenPackage(Folder, Name);
            UMaterialInstanceConstant* Instance = FindObject<UMaterialInstanceConstant>(Asset, *Name);
            const bool bNew = Instance == nullptr;
            if (bNew) Instance = NewObject<UMaterialInstanceConstant>(Asset, *Name, RF_Public | RF_Standalone | RF_Transactional);
            Instance->PreEditChange(nullptr);
            Instance->SetParentEditorOnly(Base);
            Instance->SetVectorParameterValueEditorOnly(FMaterialParameterInfo(TEXT("Color")), Tint);
            if (Opacity > 0) Instance->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(TEXT("Opacity")), Opacity);
            Instance->PostEditChange();
            Asset->MarkPackageDirty();
            if (bNew) FAssetRegistryModule::AssetCreated(Instance);
            ToSave.Add(Instance);
            if (Product.Materials.IsValidIndex(Slot)) Product.Materials[Slot] = Instance->GetPathName();
        }
        Manifest->SetStringField(TEXT("labelMode"), TEXT("model_uv"));
    }
    if (!SaveAll(ToSave)) { OutMessage = TEXT("Doku/materyal kaydedilemedi."); return false; }
    // Trailing empty slots add nothing.
    while (Product.Materials.Num() > 0 && Product.Materials.Last().IsEmpty()) Product.Materials.Pop();

    if (Existing) *Existing = Product;
    else Catalog.Add(Product);
    if (!MarketCatalog::SaveFile(MarketCatalog::DefaultPath(), Catalog, Note, Error))
    {
        if (!Existing) Catalog.Pop();
        OutMessage = Error;
        return false;
    }
    TArray<TSharedPtr<FJsonValue>> MaterialValues;
    for (const FString& Path : Product.Materials) MaterialValues.Add(MakeShared<FJsonValueString>(Path));
    Manifest->SetNumberField(TEXT("schemaVersion"), 1);
    Manifest->SetStringField(TEXT("productId"), Draft.Id);
    Manifest->SetStringField(TEXT("package"), Package.Id);
    Manifest->SetObjectField(TEXT("labels"), Labels);
    Manifest->SetArrayField(TEXT("materials"), MaterialValues);
    Manifest->SetStringField(TEXT("publishedAt"), FDateTime::Now().ToIso8601());
    IFileManager::Get().MakeDirectory(*ProductInbox, true);
    WriteJson(ProductInbox / TEXT("manifest.json"), Manifest);

    OutMessage = Product.bActive
        ? FString::Printf(TEXT("%s oyunda %s. Oyunu (yeniden) ba\u015flat\u0131nca rafta g\u00f6r\u00fcn\u00fcr."), *Product.RealName, (Existing && Existing->bActive && !bActivate) ? TEXT("g\u00fcncellendi") : TEXT("haz\u0131r"))
        : Product.RealName + TEXT(" kaydedildi (haz\u0131rl\u0131k listesi; g\u00f6rseli haz\u0131r).");
    return true;
}

bool SetActive(const FString& Id, bool bActive, TArray<FMarketProduct>& Catalog, const FString& Note, FString& OutMessage)
{
    FMarketProduct* Product = Catalog.FindByPredicate([&](const FMarketProduct& P) { return P.Id == Id; });
    if (!Product) { OutMessage = TEXT("\u00dcr\u00fcn katalogda yok."); return false; }
    if (!bActive && Product->bActive && MarketCatalog::CountActive(Catalog) <= 1)
    {
        OutMessage = TEXT("Oyunda en az bir \u00fcr\u00fcn kalmal\u0131.");
        return false;
    }
    const bool bPrevious = Product->bActive;
    Product->bActive = bActive;
    FString Error;
    if (!MarketCatalog::SaveFile(MarketCatalog::DefaultPath(), Catalog, Note, Error))
    {
        Product->bActive = bPrevious;
        OutMessage = Error;
        return false;
    }
    OutMessage = Product->RealName + (bActive ? TEXT(" oyuna al\u0131nd\u0131.") : TEXT(" oyundan \u00e7\u0131kar\u0131ld\u0131 (haz\u0131rl\u0131k listesinde duruyor)."));
    return true;
}

void FillDraft(const FMarketProduct& P, FStudioDraft& Draft)
{
    const auto CommaMoney = [](int64 Kurus) { return MarketCatalog::Money(Kurus).Replace(TEXT("."), TEXT(",")); };
    const auto Mm = [](int32 Value) { return Value > 0 ? FString::FromInt(Value) : FString(); };
    Draft = FStudioDraft();
    Draft.bExisting = true;
    Draft.bActive = P.bActive;
    Draft.Id = P.Id;
    Draft.RealName = P.RealName;
    Draft.FictionalName = P.FictionalName == P.RealName ? FString() : P.FictionalName;
    Draft.Category = P.Category;
    Draft.Cost = CommaMoney(P.Cost);
    Draft.Price = CommaMoney(P.BasePrice);
    Draft.CaseUnits = FString::FromInt(P.CaseUnits);
    Draft.Brand = P.Brand;
    Draft.PackageType = P.PackageType.IsEmpty() ? FString(TEXT("kutu")) : P.PackageType;
    Draft.WidthMm = Mm(P.WidthMm);
    Draft.DepthMm = Mm(P.DepthMm);
    Draft.HeightMm = Mm(P.HeightMm);
    Draft.DiameterMm = Mm(P.DiameterMm);
    Draft.LabelHeightMm = Mm(P.LabelHeightMm);
    Draft.Parts = P.Parts;
    Draft.Notes = P.Notes;
    Draft.bSizeEstimated = P.bSizeEstimated;
    Draft.Preset = P.Preset;
    Draft.Colors = P.Colors;
    const auto Number = [](float Value) { return FString::SanitizeFloat(Value, 4); };
    Draft.ModelScale = Number(P.VisualScale);
    Draft.ModelPitch = Number(P.VisualRotation.Pitch);
    Draft.ModelYaw = Number(P.VisualRotation.Yaw);
    Draft.ModelRoll = Number(P.VisualRotation.Roll);
    Draft.ModelOffsetX = Number(P.VisualOffsetCm.X);
    Draft.ModelOffsetY = Number(P.VisualOffsetCm.Y);
    Draft.ModelOffsetZ = Number(P.VisualOffsetCm.Z);
    Draft.ExistingMaterials = P.Materials;
}

// ---------------------------------------------------------------------------------------- prompts

bool IsRoundPackage(const FString& Type)
{
    return Type == TEXT("pet_sise") || Type == TEXT("cam_sise") || Type == TEXT("teneke") || Type == TEXT("kavanoz") || Type == TEXT("kase");
}

FString PackageTypeLabel(const FString& Type)
{
    if (Type == TEXT("kutu")) return TEXT("dikd\u00f6rtgen karton kutu");
    if (Type == TEXT("pet_sise")) return TEXT("plastik (PET) \u015fi\u015fe");
    if (Type == TEXT("cam_sise")) return TEXT("cam \u015fi\u015fe");
    if (Type == TEXT("teneke")) return TEXT("teneke / al\u00fcminyum kutu");
    if (Type == TEXT("kavanoz")) return TEXT("kavanoz");
    if (Type == TEXT("kase")) return TEXT("kase / bardak");
    if (Type == TEXT("poset")) return TEXT("po\u015fet / yumu\u015fak paket");
    return Type;
}

TArray<FPromptChoice> PromptsFor(const FString& Type)
{
    TArray<FPromptChoice> Result;
    if (Type == TEXT("kutu"))
    {
        Result.Add({ TEXT("A1"), TEXT("Etiket: tek g\u00f6rsel a\u00e7\u0131l\u0131m"), true });
        Result.Add({ TEXT("A2"), TEXT("Etiket: 6 ayr\u0131 y\u00fcz"), true });
        Result.Add({ TEXT("A3"), TEXT("A\u00e7\u0131l\u0131m s\u0131n\u0131r d\u00fczeltme"), true });
    }
    else if (Type == TEXT("poset"))
    {
        // Bags are built as flat boxes: same net as a box.
        Result.Add({ TEXT("A1"), TEXT("Bask\u0131: tek g\u00f6rsel a\u00e7\u0131l\u0131m"), true });
        Result.Add({ TEXT("A2"), TEXT("Bask\u0131: 6 ayr\u0131 y\u00fcz"), true });
    }
    else
    {
        Result.Add({ TEXT("B2"), TEXT("Etiket + kapak (d\u00f6nem)"), true });
    }
    Result.Add({ TEXT("D"), TEXT("D\u00f6nem ara\u015ft\u0131rmas\u0131"), false });
    Result.Add({ TEXT("E"), TEXT("Teslim kontrol\u00fc"), true });
    if (IsRoundPackage(Type)) Result.Add({ TEXT("B1"), TEXT("\u00d6zel 3B model (yaln\u0131z haz\u0131r \u015fekil yetmezse)"), false });
    return Result;
}

FString DeliveryFolder(const FString& ProductId, const FString& Era)
{
    return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Uretim") / ProductId / Era);
}

FString ModelFolder(const FString& ProductId)
{
    return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Uretim") / ProductId / TEXT("model"));
}

bool ExportReadyPackageTemplates(const FStudioDraft& D, const FString& Era, TArray<FString>& OutFiles, FString& OutError)
{
    OutFiles.Reset();
    if (!MarketCatalog::IsValidId(D.Id))
    {
        OutError = TEXT("\u00d6nce ge\u00e7erli bir \u00fcr\u00fcn kimli\u011fi gir.");
        return false;
    }
    const FString Folder = DeliveryFolder(D.Id, Era);
    IFileManager::Get().MakeDirectory(*Folder, true);
    const FColor Border(38, 48, 58), Guide(240, 92, 75);
    const bool bBoxLike = D.PackageType == TEXT("kutu") || D.PackageType == TEXT("poset");
    if (bBoxLike)
    {
        const int32 W = FCString::Atoi(*D.WidthMm), Depth = FCString::Atoi(*D.DepthMm), H = FCString::Atoi(*D.HeightMm);
        if (W <= 0 || Depth <= 0 || H <= 0)
        {
            OutError = TEXT("A\u00e7\u0131l\u0131m \u015fablonu i\u00e7in geni\u015flik, derinlik ve y\u00fckseklik gerekli.");
            return false;
        }
        const double NetW = 2.0 * W + 2.0 * Depth, NetH = H + 2.0 * Depth;
        const double Scale = FMath::Min3(8.0, 4096.0 / NetW, 4096.0 / NetH);
        const auto Px = [Scale](double Mm) { return FMath::FloorToInt32(Mm * Scale); };
        FStudioImage Image;
        Image.Width = Px(NetW); Image.Height = Px(NetH);
        Image.Pixels.Init(FColor::White, Image.Width * Image.Height);
        const FIntRect Left(0, Px(Depth), Px(Depth), Px(Depth + H));
        const FIntRect Front(Px(Depth), Px(Depth), Px(Depth + W), Px(Depth + H));
        const FIntRect Right(Px(Depth + W), Px(Depth), Px(2.0 * Depth + W), Px(Depth + H));
        const FIntRect Back(Px(2.0 * Depth + W), Px(Depth), Image.Width, Px(Depth + H));
        const FIntRect Top(Px(Depth), 0, Px(Depth + W), Px(Depth));
        const FIntRect Bottom(Px(Depth), Px(Depth + H), Px(Depth + W), Image.Height);
        const FIntRect Panels[] = { Left, Front, Right, Back, Top, Bottom };
        const FColor Colors[] = {
            FColor(215, 238, 220), FColor(190, 224, 250), FColor(215, 238, 220),
            FColor(225, 214, 242), FColor(252, 226, 184), FColor(247, 207, 215)
        };
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(Panels); ++Index)
        {
            FillRect(Image, Panels[Index], Colors[Index]);
            StrokeRect(Image, Panels[Index], Border, 3);
        }
        // The X marks only the front guide region. The production prompt explicitly removes it.
        DrawPixelLine(Image, Front.Min + FIntPoint(10, 10), Front.Max - FIntPoint(11, 11), Guide, 2);
        DrawPixelLine(Image, FIntPoint(Front.Max.X - 11, Front.Min.Y + 10), FIntPoint(Front.Min.X + 10, Front.Max.Y - 11), Guide, 2);
        const FString Output = Folder / TEXT("acilim_sablonu.png");
        if (!SavePng(Output, Image, OutError)) return false;
        OutFiles.Add(Output);
        return true;
    }

    if (!IsRoundPackage(D.PackageType))
    {
        OutError = TEXT("Bu ambalaj t\u00fcr\u00fc i\u00e7in haz\u0131r \u00fcretim \u015fablonu yok.");
        return false;
    }
    const int32 Diameter = FCString::Atoi(*D.DiameterMm), LabelHeight = FCString::Atoi(*D.LabelHeightMm);
    const FIntPoint Size = ProductionSize(UE_PI * Diameter, LabelHeight);
    if (Size.X <= 0 || Size.Y <= 0)
    {
        OutError = TEXT("Etiket \u015fablonu i\u00e7in \u00e7ap ve etiket band\u0131 y\u00fcksekli\u011fi gerekli.");
        return false;
    }
    FStudioImage Label;
    Label.Width = Size.X; Label.Height = Size.Y;
    Label.Pixels.Init(FColor(221, 234, 244), Label.Width * Label.Height);
    const int32 Seam = FMath::Max(3, FMath::RoundToInt32(Label.Width * 0.03f));
    const int32 FrontHalf = FMath::Max(4, FMath::RoundToInt32(Label.Width * 0.20f));
    FillRect(Label, FIntRect(0, 0, Seam, Label.Height), FColor(249, 213, 210));
    FillRect(Label, FIntRect(Label.Width - Seam, 0, Label.Width, Label.Height), FColor(249, 213, 210));
    FillRect(Label, FIntRect(Label.Width / 2 - FrontHalf, 0, Label.Width / 2 + FrontHalf, Label.Height), FColor(210, 239, 218));
    StrokeRect(Label, FIntRect(0, 0, Label.Width, Label.Height), Border, 3);
    DrawPixelLine(Label, FIntPoint(Label.Width / 2, 0), FIntPoint(Label.Width / 2, Label.Height - 1), Guide, 2);
    const FString LabelOutput = Folder / TEXT("label_sablonu.png");
    if (!SavePng(LabelOutput, Label, OutError)) return false;
    OutFiles.Add(LabelOutput);

    const bool bHasCap = D.Parts.Contains(TEXT("Kapak")) && D.PackageType != TEXT("teneke");
    if (bHasCap)
    {
        FStudioImage Cap;
        Cap.Width = Cap.Height = 512;
        Cap.Pixels.Init(FColor(237, 239, 242), Cap.Width * Cap.Height);
        StrokeRect(Cap, FIntRect(0, 0, Cap.Width, Cap.Height), Border, 3);
        const FIntPoint Center(Cap.Width / 2, Cap.Height / 2);
        const int32 Radius = 230;
        FIntPoint Previous(Center.X + Radius, Center.Y);
        for (int32 Degree = 1; Degree <= 360; ++Degree)
        {
            const double Angle = FMath::DegreesToRadians(double(Degree));
            const FIntPoint Next(Center.X + FMath::RoundToInt32(FMath::Cos(Angle) * Radius), Center.Y + FMath::RoundToInt32(FMath::Sin(Angle) * Radius));
            DrawPixelLine(Cap, Previous, Next, Guide, 2);
            Previous = Next;
        }
        DrawPixelLine(Cap, FIntPoint(Center.X, 20), FIntPoint(Center.X, Cap.Height - 21), FColor(145, 153, 161), 1);
        DrawPixelLine(Cap, FIntPoint(20, Center.Y), FIntPoint(Cap.Width - 21, Center.Y), FColor(145, 153, 161), 1);
        const FString CapOutput = Folder / TEXT("kapak_sablonu.png");
        if (!SavePng(CapOutput, Cap, OutError)) return false;
        OutFiles.Add(CapOutput);
    }
    return true;
}

bool BuildPrompt(const FString& TemplateId, const FStudioDraft& D, const FString& Era, FString& OutText, FString& OutError)
{
    const FString Dir = FPaths::ProjectDir() / TEXT("Docs/Uretim/Sablonlar");
    FString Text, Product, General, Period;
    if (!FFileHelper::LoadFileToString(Text, *(Dir / (TemplateId + TEXT(".txt")))))
    {
        OutError = TEXT("\u015eablon bulunamad\u0131: Docs/Uretim/Sablonlar/") + TemplateId + TEXT(".txt");
        return false;
    }
    FFileHelper::LoadFileToString(Product, *(Dir / TEXT("_urun.txt")));
    FFileHelper::LoadFileToString(General, *(Dir / TEXT("_genel.txt")));
    FFileHelper::LoadFileToString(Period, *(Dir / TEXT("_donem.txt")));
    Text.ReplaceInline(TEXT("{{URUN}}"), *Product.TrimEnd());
    Text.ReplaceInline(TEXT("{{GENEL_KURALLAR}}"), *General.TrimEnd());
    Text.ReplaceInline(TEXT("{{DONEM_KURALLARI}}"), *Period.TrimEnd());

    const int32 W = FCString::Atoi(*D.WidthMm), Dp = FCString::Atoi(*D.DepthMm), H = FCString::Atoi(*D.HeightMm);
    const int32 Dia = FCString::Atoi(*D.DiameterMm), LabelH = FCString::Atoi(*D.LabelHeightMm);
    const auto Q = [](int32 V) { return V > 0 ? FString::FromInt(V) : FString(TEXT("?")); };
    const auto Ratio = [](double A, double B) { return B > 0 ? FString::Printf(TEXT("%.2f"), A / B).Replace(TEXT("."), TEXT(",")) : FString(TEXT("?")); };
    TMap<FString, FString> V;
    V.Add(TEXT("URUN_ADI"), D.RealName.IsEmpty() ? FString(TEXT("?")) : D.RealName);
    V.Add(TEXT("MARKA"), D.Brand.IsEmpty() ? FString(TEXT("?")) : D.Brand);
    V.Add(TEXT("URUN_KIMLIGI"), D.Id);
    V.Add(TEXT("DONEM"), Era);
    V.Add(TEXT("AMBALAJ_TURU"), PackageTypeLabel(D.PackageType));
    V.Add(TEXT("GENISLIK"), Q(W));
    V.Add(TEXT("DERINLIK"), Q(Dp));
    V.Add(TEXT("YUKSEKLIK"), Q(H));
    V.Add(TEXT("CAP"), Q(Dia));
    V.Add(TEXT("ETIKET_YUKSEKLIGI"), Q(LabelH));
    V.Add(TEXT("PARCALAR"), D.Parts.IsEmpty() ? FString(TEXT("Etiket")) : D.Parts.Replace(TEXT(","), TEXT(", ")));
    V.Add(TEXT("NOTLAR"), D.Notes.IsEmpty() ? FString(TEXT("-")) : D.Notes);
    V.Add(TEXT("OLCU_DURUMU"), TEXT("sabit \u00f6l\u00e7\u00fc: aynen kullan, ara\u015ft\u0131rma veya de\u011fi\u015ftirme"));
    if (D.PackageType == TEXT("kutu"))
        V.Add(TEXT("OLCULER"), FString::Printf(TEXT("Geni\u015flik %s \u00d7 Derinlik %s \u00d7 Y\u00fckseklik %s mm"), *Q(W), *Q(Dp), *Q(H)));
    else if (D.PackageType == TEXT("poset"))
        V.Add(TEXT("OLCULER"), FString::Printf(TEXT("Geni\u015flik %s \u00d7 Y\u00fckseklik %s mm, dolu kal\u0131nl\u0131k %s mm"), *Q(W), *Q(H), *Q(Dp)));
    else
        V.Add(TEXT("OLCULER"), FString::Printf(TEXT("\u00c7ap %s mm, y\u00fckseklik %s mm, etiket band\u0131 %s mm"), *Q(Dia), *Q(H), *Q(LabelH)));
    V.Add(TEXT("KLASOR"), FString::Printf(TEXT("Uretim/%s/%s/"), *D.Id, *Era));
    V.Add(TEXT("MODEL_KLASOR"), FString::Printf(TEXT("Uretim/%s/model/"), *D.Id));

    // Box net and faces (same math as FBoxPackageLayout / studio tiles).
    FString NetFrontPx = TEXT("?"), NetSidePx = TEXT("?"), NetTopPx = TEXT("?");
    FString NetPx = TEXT("?"), Panels = TEXT("(\u00f6l\u00e7\u00fc eksik)"), Front = TEXT("?"), Side = TEXT("?"), Top = TEXT("?");
    if (W > 0 && Dp > 0 && H > 0)
    {
        const double S = FMath::Min3(8.0, 4096.0 / (2.0 * W + 2.0 * Dp), 4096.0 / (H + 2.0 * Dp));
        const auto Px = [S](double Mm) { return FMath::FloorToInt32(Mm * S); };
        NetPx = FString::Printf(TEXT("%d x %d"), Px(2.0 * W + 2.0 * Dp), Px(H + 2.0 * Dp));
        NetFrontPx = FString::Printf(TEXT("%d x %d"), Px(W), Px(H));
        NetSidePx = FString::Printf(TEXT("%d x %d"), Px(Dp), Px(H));
        NetTopPx = FString::Printf(TEXT("%d x %d"), Px(W), Px(Dp));
        const auto Panel = [&](const TCHAR* Name, double X0, double Y0, double X1, double Y1)
        { return FString::Printf(TEXT("- %s: (%d, %d) \u2192 (%d, %d)\n"), Name, Px(X0), Px(Y0), Px(X1), Px(Y1)); };
        Panels = Panel(TEXT("SOL "), 0, Dp, Dp, Dp + H) + Panel(TEXT("\u00d6N  "), Dp, Dp, Dp + W, Dp + H) + Panel(TEXT("SA\u011e "), Dp + W, Dp, 2.0 * Dp + W, Dp + H)
               + Panel(TEXT("ARKA"), 2.0 * Dp + W, Dp, 2.0 * Dp + 2.0 * W, Dp + H) + Panel(TEXT("\u00dcST "), Dp, 0, Dp + W, Dp) + Panel(TEXT("ALT "), Dp, Dp + H, Dp + W, 2.0 * Dp + H);
        Panels.TrimEndInline();
        FBoxPackageLayout L;
        if (FBoxPackageLayout::Make(W, Dp, H, L))
        {
            Front = FString::Printf(TEXT("%d x %d piksel"), L.Rects[FBoxPackageLayout::Front].Width(), L.Rects[FBoxPackageLayout::Front].Height());
            Side = FString::Printf(TEXT("%d x %d piksel"), L.Rects[FBoxPackageLayout::Right].Width(), L.Rects[FBoxPackageLayout::Right].Height());
            Top = FString::Printf(TEXT("%d x %d piksel"), L.Rects[FBoxPackageLayout::Top].Width(), L.Rects[FBoxPackageLayout::Top].Height());
        }
    }
    V.Add(TEXT("ACILIM_PX"), NetPx);
    V.Add(TEXT("ACILIM_PANELLER"), Panels);
    V.Add(TEXT("ACILIM_ON_PX"), NetFrontPx);
    V.Add(TEXT("ACILIM_YAN_PX"), NetSidePx);
    V.Add(TEXT("ACILIM_UST_PX"), NetTopPx);
    V.Add(TEXT("YUZ_ON"), Front);
    V.Add(TEXT("YUZ_YAN"), Side);
    V.Add(TEXT("YUZ_UST"), Top);
    V.Add(TEXT("ORAN_YAN"), Ratio(Dp, H));
    V.Add(TEXT("ORAN_ON"), Ratio(W, H));
    V.Add(TEXT("ORAN_UST"), Ratio(W, Dp));

    const auto Capped = [](double Wmm, double Hmm)
    {
        if (Wmm <= 0 || Hmm <= 0) return FString(TEXT("?"));
        const double S = FMath::Min3(8.0, 4096.0 / Wmm, 4096.0 / Hmm);
        return FString::Printf(TEXT("%d x %d"), FMath::FloorToInt32(Wmm * S), FMath::FloorToInt32(Hmm * S));
    };
    V.Add(TEXT("ETIKET_PX"), Capped(UE_PI * Dia, LabelH));
    V.Add(TEXT("POSET_PX"), Capped(2.0 * W, H));

    // malzeme.json example matching the product's parts.
    TArray<FString> PartList;
    (D.Parts.IsEmpty() ? FString(TEXT("Etiket")) : D.Parts).ParseIntoArray(PartList, TEXT(","));
    TArray<FString> SlotLines;
    for (FString Part : PartList)
    {
        Part.TrimStartAndEndInline();
        const ESlotRole Role = SlotRole(Part);
        if (Role == ESlotRole::Label) SlotLines.Add(FString::Printf(TEXT("  {\"name\":\"%s\",\"type\":\"label\"}"), *Part));
        else if (Role == ESlotRole::Glass) SlotLines.Add(FString::Printf(TEXT("  {\"name\":\"%s\",\"type\":\"glass\",\"color\":\"D9EEE6\",\"opacity\":0.3,\"roughness\":0.05}"), *Part));
        else SlotLines.Add(FString::Printf(TEXT("  {\"name\":\"%s\",\"type\":\"solid\",\"color\":\"FFFFFF\",\"roughness\":0.4,\"metallic\":0.0}"), *Part));
    }
    V.Add(TEXT("MALZEME_ORNEK"), TEXT("{\"slots\":[\n") + FString::Join(SlotLines, TEXT(",\n")) + TEXT("\n]}"));

    // Caps: a carton's cap is printed on its top face; bottles/jars/cups get a real 3D cap whose
    // look comes from kapak.png (top view) or from the part color set in the studio.
    const bool bHasCap = D.Parts.Contains(TEXT("Kapak"));
    const bool bBoxLike = D.PackageType == TEXT("kutu") || D.PackageType == TEXT("poset");
    if (bBoxLike && bHasCap)
        V.Add(TEXT("KAPAK_NOTU"), TEXT("- KAPAK: Bu kutunun \u00fcst\u00fcnde vidal\u0131 plastik kapak olabilir. \u00dcr\u00fcn\u00fcn ") + Era + TEXT(" ambalaj\u0131nda kapak VARSA, kapa\u011f\u0131 \u00dcST panelinde, \u00d6N kenara yak\u0131n ve yatayda ortada, TAM \u00dcSTTEN g\u00f6r\u00fcn\u00fc\u015f\u00fcyle d\u00fcz bir daire olarak \u00e7iz (\u00e7ap\u0131 kutu geni\u015fli\u011finin yakla\u015f\u0131k \u00fc\u00e7te biri, ger\u00e7ek kapak rengiyle, hafif i\u00e7 halka \u00e7izgisiyle). G\u00f6lge, kabartma, perspektif YOK. O y\u0131l\u0131n ambalaj\u0131nda kapak YOKSA \u00e7izme; \u00fcst y\u00fcz\u00fc d\u00fcz karton bask\u0131s\u0131 b\u0131rak."));
    else if (bBoxLike)
        V.Add(TEXT("KAPAK_NOTU"), TEXT("- KAPAK: Bu ambalajda kapak yok; \u00dcST paneline kapak, delik veya i\u015faret \u00e7izme."));
    else
        V.Add(TEXT("KAPAK_NOTU"), FString());
    const bool bCustomModel = D.PackageId.StartsWith(TEXT("model_"));
    V.Add(TEXT("KAPAK_BASLIK"), bHasCap && D.PackageType != TEXT("teneke") ? TEXT(" ve kapa\u011f\u0131n \u00fcst g\u00f6rselini") : TEXT(""));
    V.Add(TEXT("ETIKET_OLCU"), bCustomModel
        ? TEXT("uv_sablon.png ile AYNI piksel \u00f6l\u00e7\u00fcs\u00fc (model klas\u00f6r\u00fc: Uretim/") + D.Id + TEXT("/model/)")
        : V[TEXT("ETIKET_PX")] + TEXT(" piksel"));
    if (!bHasCap || D.PackageType == TEXT("teneke"))
        V.Add(TEXT("KAPAK_GORSEL"), TEXT("- kapak.png GEREKMEZ (") + FString(D.PackageType == TEXT("teneke") ? TEXT("teneke \u00fcst/alt metal; rengi st\u00fcdyoda") : TEXT("bu ambalajda kapak yok")) + TEXT(")."));
    else
        V.Add(TEXT("KAPAK_GORSEL"), TEXT("- kapak.png: kapa\u011f\u0131n TAM \u00dcSTTEN g\u00f6r\u00fcn\u00fc\u015f\u00fc, 512 x 512 piksel kare. Kapak dairesi g\u00f6rselin tamam\u0131n\u0131 kaplas\u0131n; daire d\u0131\u015f\u0131nda kalan k\u00f6\u015feleri de kapak rengiyle doldur (beyaz b\u0131rakma). Kapak \u00fcst\u00fcnde bask\u0131/logo varsa ortada; yoksa d\u00fcz kapak rengi ve hafif i\u00e7 halka \u00e7izgisi. Kapa\u011f\u0131n yan y\u00fcz\u00fc bu g\u00f6rselin kenar renginden gelir. I\u015f\u0131k, g\u00f6lge, perspektif YOK."));

    if (D.PackageType == TEXT("kutu") || D.PackageType == TEXT("poset"))
    {
        V.Add(TEXT("BEKLENEN_DOSYALAR"), TEXT("acilim.png YA DA front/back/right/left/top/bottom.png; kaynak.md"));
        V.Add(TEXT("BEKLENEN_PIKSEL"), TEXT("acilim.png ") + NetPx + TEXT("; y\u00fczler: \u00f6n/arka ") + Front + TEXT(", yan ") + Side + TEXT(", \u00fcst/alt ") + Top);
        V.Add(TEXT("SABLON_DOSYALARI"), TEXT("acilim_sablonu.png"));
        V.Add(TEXT("SABLON_KULLANIMI"), D.Preset.IsEmpty() ? FString() :
            TEXT("AJANA EKLENECEK \u015eABLON\n- St\u00fcdyoda \"\u015eablonlar\u0131 olu\u015ftur\" ile \u00fcretilen acilim_sablonu.png dosyas\u0131n\u0131 bu promptla birlikte y\u00fckledim.\n")
            TEXT("- Son acilim.png, y\u00fckledi\u011fim \u015fablonla TAM AYNI piksel boyutunda ve ayn\u0131 panel s\u0131n\u0131rlar\u0131nda olmal\u0131. Tuvali yeniden boyutland\u0131rma, k\u0131rpma veya panel yerlerini de\u011fi\u015ftirme.\n")
            TEXT("- \u015eablondaki pastel b\u00f6lge renkleri ve k\u0131rm\u0131z\u0131 X yaln\u0131z k\u0131lavuzdur; ambalaj bask\u0131s\u0131yla tamamen de\u011fi\u015ftir. Koyu kenarlar panel s\u0131n\u0131rlar\u0131n\u0131 g\u00f6sterir: ayn\u0131 yerde ADIM 3'te istenen temiz 2-3 piksel panel \u00e7izgisini yeniden \u00fcret.\n"));
    }
    else
    {
        V.Add(TEXT("BEKLENEN_DOSYALAR"), bHasCap && D.PackageType != TEXT("teneke") ? TEXT("label.png, kapak.png, kaynak.md") : TEXT("label.png, kaynak.md"));
        V.Add(TEXT("BEKLENEN_PIKSEL"), TEXT("label.png ") + V[TEXT("ETIKET_OLCU")] + (bHasCap && D.PackageType != TEXT("teneke") ? TEXT("; kapak.png 512 x 512") : TEXT("")));
        const FString CapTemplate = bHasCap && D.PackageType != TEXT("teneke") ? TEXT(", kapak_sablonu.png") : TEXT("");
        V.Add(TEXT("SABLON_DOSYALARI"), bCustomModel ? TEXT("uv_sablon.png") : TEXT("label_sablonu.png") + CapTemplate);
        V.Add(TEXT("SABLON_KULLANIMI"), bCustomModel
            ? TEXT("AJANA EKLENECEK \u015eABLON\n- Model klas\u00f6r\u00fcndeki uv_sablon.png dosyas\u0131n\u0131 bu promptla birlikte y\u00fckledim. label.png ayn\u0131 tuvalde UV adalar\u0131na g\u00f6re haz\u0131rlanmal\u0131; k\u0131lavuz \u00e7izgileri nihai dosyada kalmamal\u0131.\n")
            : D.Preset.IsEmpty() ? FString() :
              TEXT("AJANA EKLENECEK \u015eABLONLAR\n- St\u00fcdyoda \"\u015eablonlar\u0131 olu\u015ftur\" ile \u00fcretilen label_sablonu.png") + CapTemplate + TEXT(" dosyalar\u0131n\u0131 bu promptla birlikte y\u00fckledim.\n")
              TEXT("- Her nihai PNG, kar\u015f\u0131l\u0131k gelen \u015fablonla TAM AYNI piksel boyutunda olmal\u0131; tuvali yeniden boyutland\u0131rma veya k\u0131rpma.\n")
              TEXT("- Etiket \u015fablonunda ye\u015fil orta b\u00f6lge \u00fcr\u00fcn\u00fcn \u00f6n\u00fc, k\u0131rm\u0131z\u0131 kenarlar arka diki\u015f g\u00fcvenlik alan\u0131d\u0131r. Pastel renkler, orta \u00e7izgi, daire ve b\u00fct\u00fcn k\u0131lavuzlar\u0131 temiz bask\u0131yla tamamen de\u011fi\u015ftir; nihai dosyada kalmas\u0131n.\n"));
    }
    for (const TPair<FString, FString>& Pair : V)
        Text.ReplaceInline(*(TEXT("{{") + Pair.Key + TEXT("}}")), *Pair.Value);
    Text.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
    OutText = Text.TrimStartAndEnd();
    return true;
}

bool RemoveProduct(const FString& Id, TArray<FMarketProduct>& Catalog, const FString& Note, FString& OutMessage)
{
    const int32 Index = Catalog.IndexOfByPredicate([&](const FMarketProduct& P) { return P.Id == Id; });
    if (Index == INDEX_NONE) { OutMessage = TEXT("\u00dcr\u00fcn katalogda yok."); return false; }
    if (Catalog.Num() <= 1) { OutMessage = TEXT("Katalogda en az bir \u00fcr\u00fcn kalmal\u0131."); return false; }
    const FMarketProduct Removed = Catalog[Index];
    Catalog.RemoveAt(Index);
    FString Error;
    if (!MarketCatalog::SaveFile(MarketCatalog::DefaultPath(), Catalog, Note, Error))
    {
        Catalog.Insert(Removed, Index);
        OutMessage = Error;
        return false;
    }
    OutMessage = Removed.RealName + TEXT(" katalogdan \u00e7\u0131kar\u0131ld\u0131. Dosyalar\u0131 AssetInbox ve Content'te duruyor; yeniden eklenebilir.");
    return true;
}

void LoadDraftSources(const FString& ProductId, FStudioDraft& Draft)
{
    const FString Directory = InboxDir() / TEXT("Products") / ProductId;
    const TSharedPtr<FJsonObject> Root = ReadJson(Directory / TEXT("manifest.json"));
    const TSharedPtr<FJsonObject>* Labels = nullptr;
    if (!Root.IsValid() || !Root->TryGetObjectField(TEXT("labels"), Labels) || !Labels) return;
    const auto Read = [&](const TCHAR* Key, FString& Target)
    {
        FString File;
        if ((*Labels)->TryGetStringField(Key, File) && FPaths::FileExists(Directory / File)) Target = Directory / File;
    };
    for (int32 F = 0; F < FBoxPackageLayout::FaceCount; ++F) Read(FBoxPackageLayout::FaceKey(F), Draft.FaceFiles[F]);
    Read(TEXT("net"), Draft.NetFile);
    Read(TEXT("label"), Draft.LabelFile);
    Read(TEXT("cap"), Draft.CapFile);
    Read(TEXT("body"), Draft.BodyFile);
}
}

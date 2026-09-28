// Product Studio backend: shared helpers, asset paths, label images and box nets.
// Meshes: StudioMeshes.cpp, ready packages: StudioPresets.cpp, products: StudioProducts.cpp, prompts: StudioPrompts.cpp.
#include "StudioBackend.h"
#include "StudioBackendInternal.h"
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
namespace Internal
{

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

    void DrawUvLine(FStudioImage& Image, const FVector2f& From, const FVector2f& To, const FColor& Color, int32 Thickness)
    {
        constexpr int32 Margin = 32;
        const auto Point = [&](const FVector2f& UV)
        {
            return FIntPoint(
                FMath::RoundToInt(FMath::Lerp(float(Margin), float(Image.Width - Margin - 1), FMath::Clamp(UV.X, 0.f, 1.f))),
                FMath::RoundToInt(FMath::Lerp(float(Margin), float(Image.Height - Margin - 1), FMath::Clamp(UV.Y, 0.f, 1.f))));
        };
        DrawPixelLine(Image, Point(From), Point(To), Color, Thickness);
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

    void StrokeRect(FStudioImage& Image, const FIntRect& Rect, const FColor& Color, int32 Thickness)
    {
        FillRect(Image, FIntRect(Rect.Min.X, Rect.Min.Y, Rect.Max.X, Rect.Min.Y + Thickness), Color);
        FillRect(Image, FIntRect(Rect.Min.X, Rect.Max.Y - Thickness, Rect.Max.X, Rect.Max.Y), Color);
        FillRect(Image, FIntRect(Rect.Min.X, Rect.Min.Y, Rect.Min.X + Thickness, Rect.Max.Y), Color);
        FillRect(Image, FIntRect(Rect.Max.X - Thickness, Rect.Min.Y, Rect.Max.X, Rect.Max.Y), Color);
    }

    void DrawPixelLine(FStudioImage& Image, FIntPoint A, FIntPoint B, const FColor& Color, int32 Thickness)
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

using namespace Internal;

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
}

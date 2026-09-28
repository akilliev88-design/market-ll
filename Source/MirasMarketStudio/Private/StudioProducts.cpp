// Product Studio: draft validation, publishing to the catalog, activation.
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
using namespace Internal;

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
    const auto DraftMoney = [](int64 Kurus) { return MarketCatalog::Money(Kurus).Replace(TEXT("."), TEXT(",")); };
    const auto Mm = [](int32 Value) { return Value > 0 ? FString::FromInt(Value) : FString(); };
    Draft = FStudioDraft();
    Draft.bExisting = true;
    Draft.bActive = P.bActive;
    Draft.Id = P.Id;
    Draft.RealName = P.RealName;
    Draft.FictionalName = P.FictionalName == P.RealName ? FString() : P.FictionalName;
    Draft.Category = P.Category;
    Draft.Cost = DraftMoney(P.Cost);
    Draft.Price = DraftMoney(P.BasePrice);
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
}

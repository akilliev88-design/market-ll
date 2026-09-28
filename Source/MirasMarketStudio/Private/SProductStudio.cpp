#include "SProductStudio.h"
#include "DesktopPlatformModule.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformProcess.h"
#include "IDesktopPlatform.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "SStudioViewport.h"
#include "StudioStyle.h"
#include "UObject/Package.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

using namespace MirasStudio;

namespace
{
    FText T(const FString& Text) { return FText::FromString(Text); }
    FString CommaMoney(int64 Kurus) { return MarketCatalog::Money(Kurus).Replace(TEXT("."), TEXT(",")); }
    const TCHAR* FaceTitles[] = { TEXT("\u00d6n"), TEXT("Arka"), TEXT("Sa\u011f"), TEXT("Sol"), TEXT("\u00dcst"), TEXT("Alt") };
    const TCHAR* ImageTypes = TEXT("G\u00f6rsel (png, jpg, tga, bmp)|*.png;*.jpg;*.jpeg;*.tga;*.bmp");
}

// ============================================================================ construction

void SProductStudio::Construct(const FArguments& InArgs)
{
    const FStudioStyle& S = FStudioStyle::Get();
    ChildSlot
    [
        SNew(SBorder).BorderImage(&S.BgBrush).Padding(FMargin(22.f, 18.f))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ SAssignNew(HeaderSlot, SBox) ]
            + SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 16.f, 0.f, 0.f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth()[ SNew(SBox).WidthOverride(290.f)[ SAssignNew(LeftSlot, SBox) ] ]
                + SHorizontalBox::Slot().FillWidth(1.f).Padding(16.f, 0.f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().FillHeight(1.f)
                    [
                        SNew(SBorder).BorderImage(&S.ViewportFrame).Padding(1.f)
                        [
                            SNew(SOverlay)
                            + SOverlay::Slot()[ SAssignNew(Viewport, SStudioViewport) ]
                            + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(FMargin(26.f, 22.f))[ SAssignNew(TitleSlot, SBox) ]
                            + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(16.f)
                            [
                                SNew(SHorizontalBox)
                                + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)
                                [
                                    MakeButton(TEXT("D\u00f6nd\u00fcr / Durdur"), &S.Chip, FOnClicked::CreateLambda([this]()
                                    {
                                        if (Viewport.IsValid()) Viewport->SetSpinning(!Viewport->IsSpinning());
                                        return FReply::Handled();
                                    }), S.Muted, true, 9)
                                ]
                                + SHorizontalBox::Slot().AutoWidth()
                                [
                                    MakeButton(TEXT("G\u00f6r\u00fcn\u00fcm\u00fc s\u0131f\u0131rla"), &S.Chip, FOnClicked::CreateLambda([this]()
                                    {
                                        if (Viewport.IsValid()) Viewport->ResetView();
                                        return FReply::Handled();
                                    }), S.Muted, true, 9)
                                ]
                            ]
                        ]
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)[ SAssignNew(FacesSlot, SBox) ]
                ]
                + SHorizontalBox::Slot().AutoWidth()[ SNew(SBox).WidthOverride(390.f)[ SAssignNew(RightSlot, SBox) ] ]
            ]
        ]
    ];
    ReloadData();
    if (Catalog.Num() > 0) SelectProduct(0);
    else NewProduct(FString());
}

namespace
{
    FString PresetSize(const MirasStudio::FPackagePreset& P)
    {
        return P.IsBox() ? FString::Printf(TEXT("%d\u00d7%d\u00d7%d mm"), P.WidthMm, P.DepthMm, P.HeightMm)
                         : FString::Printf(TEXT("\u00d8%d \u00d7 %d mm"), P.DiameterMm, P.HeightMm);
    }
}

// ============================================================================ state

void SProductStudio::ReloadData()
{
    TArray<FString> Errors;
    Catalog.Reset();
    CatalogNote.Reset();
    MarketCatalog::LoadFile(MarketCatalog::DefaultPath(), Catalog, Errors, &CatalogNote);
    Packages = ScanPackages(Catalog);
    Presets = LoadPresets(&Errors);
    if (Errors.Num() > 0) Toast(TEXT("Katalog uyar\u0131s\u0131: ") + Errors[0], true);
}

void SProductStudio::SetMode(EMode NewMode)
{
    Mode = NewMode;
    if (Mode == EMode::Packages && !FindPackage(Packages, SelectedPackageId))
        SelectedPackageId = Packages.Num() > 0 ? Packages[0].Id : FString();
    RebuildAll();
    UpdatePreview();
}

void SProductStudio::SelectProduct(int32 Index)
{
    if (!Catalog.IsValidIndex(Index)) return;
    const FMarketProduct& P = Catalog[Index];
    Mode = EMode::Products;
    bListActive = P.bActive;
    SelectedProduct = Index;
    FillDraft(P, Draft);
    bCustomPackage = false;
    PresetFilter = Draft.PackageType;
    Draft.PackageId = PackageIdForMesh(Packages, P.MeshPath);
    const FPackagePreset* Preset = FindPreset(Presets, Draft.Preset);
    if (Draft.PackageId.IsEmpty() && Preset)
    {
        // The product has a ready package: make sure its 3D shape exists and show it.
        FString Id, Error;
        if (EnsurePresetPackage(*Preset, Id, Error))
        {
            if (!FindPackage(Packages, Id)) Packages = ScanPackages(Catalog);
            Draft.PackageId = Id;
        }
        else Toast(Error, true);
        Draft.ExistingMaterials.Reset();
    }
    else if (Draft.PackageId.IsEmpty())
    {
        // Suggest the package this product's production data points to, if it already exists.
        const int32 W = FCString::Atoi(*Draft.WidthMm), D = FCString::Atoi(*Draft.DepthMm), H = FCString::Atoi(*Draft.HeightMm);
        const FString BoxId = FBoxPackageLayout::PackageId(W, D, H);
        const FString ModelId = TEXT("model_") + MarketCatalog::MakeId(P.Id).Left(30);
        if (Draft.PackageType == TEXT("kutu") && FindPackage(Packages, BoxId)) Draft.PackageId = BoxId;
        else if (Draft.PackageType != TEXT("kutu") && FindPackage(Packages, ModelId)) Draft.PackageId = ModelId;
        Draft.ExistingMaterials.Reset();
    }
    LoadDraftSources(P.Id, Draft);
    OriginalPackageId = Draft.PackageId;
    bIdTouched = true;
    const TArray<FPromptChoice> Choices = PromptsFor(Draft.PackageType);
    if (!Choices.ContainsByPredicate([&](const FPromptChoice& C) { return C.Id == SelectedPrompt; }))
        SelectedPrompt = Choices.Num() > 0 ? Choices[0].Id : FString();
    RebuildAll();
    UpdatePreview();
}

void SProductStudio::NewProduct(const FString& PackageId)
{
    Mode = EMode::Products;
    bListActive = false;
    SelectedProduct = INDEX_NONE;
    Draft = FStudioDraft();
    Draft.PackageId = PackageId;
    if (const FStudioPackage* Package = FindPackage(Packages, PackageId))
    {
        if (Package->Kind == EPackageKind::Box)
        {
            Draft.PackageType = TEXT("kutu");
            Draft.WidthMm = FString::FromInt(Package->WidthMm);
            Draft.DepthMm = FString::FromInt(Package->DepthMm);
            Draft.HeightMm = FString::FromInt(Package->HeightMm);
        }
        else Draft.PackageType = TEXT("pet_sise");
    }
    OriginalPackageId.Reset();
    bIdTouched = false;
    bCustomPackage = false;
    PresetFilter = Draft.PackageType;
    SelectedPrompt = PromptsFor(Draft.PackageType)[0].Id;
    RebuildAll();
    UpdatePreview();
}

void SProductStudio::ChoosePreset(const FString& PresetId)
{
    const FPackagePreset* Preset = FindPreset(Presets, PresetId);
    if (!Preset) return;
    ApplyPreset(*Preset, Draft);
    Draft.ModelScale = TEXT("1");
    Draft.ModelPitch = Draft.ModelYaw = Draft.ModelRoll = TEXT("0");
    Draft.ModelOffsetX = Draft.ModelOffsetY = Draft.ModelOffsetZ = TEXT("0");
    bCustomPackage = false;
    PresetFilter = Preset->Type;
    FString PackageId, Error;
    if (!EnsurePresetPackage(*Preset, PackageId, Error))
    {
        Toast(Error, true);
        RebuildAll();
        return;
    }
    Packages = ScanPackages(Catalog);
    const TArray<FPromptChoice> Choices = PromptsFor(Draft.PackageType);
    if (!Choices.ContainsByPredicate([&](const FPromptChoice& C) { return C.Id == SelectedPrompt; }))
        SelectedPrompt = Choices.Num() > 0 ? Choices[0].Id : FString();
    Toast(Preset->Name + TEXT(" se\u00e7ildi; 3B ambalaj haz\u0131r. \u015eimdi promptu kopyala, g\u00f6rseller gelince y\u00fckle."));
    ChoosePackage(PackageId);
}

void SProductStudio::SelectPackage(const FString& PackageId)
{
    SelectedPackageId = PackageId;
    RebuildAll();
    UpdatePreview();
}

void SProductStudio::ChoosePackage(const FString& PackageId)
{
    Draft.PackageId = PackageId;
    // A label made for another package shape does not fit this one.
    Draft.ExistingMaterials = (PackageId == OriginalPackageId && SelectedProduct != INDEX_NONE) ? Catalog[SelectedProduct].Materials : TArray<FString>();
    RebuildAll();
    UpdatePreview();
}

// ============================================================================ actions

void SProductStudio::SelectById(const FString& Id)
{
    const int32 Index = Catalog.IndexOfByPredicate([&](const FMarketProduct& P) { return P.Id == Id; });
    if (Index != INDEX_NONE) SelectProduct(Index);
    else RebuildAll();
}

FReply SProductStudio::OnActivate()
{
    FString Message;
    const bool bOk = Publish(Draft, Packages, Catalog, CatalogNote, true, Message);
    const FString Id = Draft.Id;
    if (bOk) { ReloadData(); SelectById(Id); }
    Toast(Message, !bOk);
    return FReply::Handled();
}

FReply SProductStudio::OnSave()
{
    FString Message;
    const bool bOk = Publish(Draft, Packages, Catalog, CatalogNote, false, Message);
    const FString Id = Draft.Id;
    if (bOk) { ReloadData(); SelectById(Id); }
    Toast(Message, !bOk);
    return FReply::Handled();
}

FReply SProductStudio::OnDeactivate()
{
    FString Message;
    const bool bOk = SetActive(Draft.Id, false, Catalog, CatalogNote, Message);
    const FString Id = Draft.Id;
    if (bOk) { ReloadData(); SelectById(Id); }
    Toast(Message, !bOk);
    return FReply::Handled();
}

bool SProductStudio::CurrentPrompt(FString& OutText, FString& OutError) const
{
    if (SelectedPrompt.IsEmpty()) { OutError = TEXT("Bir prompt se\u00e7."); return false; }
    return BuildPrompt(SelectedPrompt, Draft, SelectedEra, OutText, OutError);
}

FReply SProductStudio::OnCopyPrompt()
{
    if (!Draft.Preset.IsEmpty())
    {
        TArray<FString> TemplateFiles;
        FString TemplateError;
        if (!ExportReadyPackageTemplates(Draft, SelectedEra, TemplateFiles, TemplateError))
        {
            Toast(TemplateError, true);
            return FReply::Handled();
        }
    }
    FString Text, Error;
    if (!CurrentPrompt(Text, Error)) { Toast(Error, true); return FReply::Handled(); }
    FPlatformApplicationMisc::ClipboardCopy(*Text);
    Toast(FString::Printf(TEXT("%s promptu panoya kopyaland\u0131 (%s, %s). Ajana yap\u0131\u015ft\u0131r; \u00e7\u0131kt\u0131y\u0131 teslim klas\u00f6r\u00fcne koy."), *SelectedPrompt, *Draft.RealName, *SelectedEra));
    return FReply::Handled();
}

FReply SProductStudio::OnOpenDelivery()
{
    if (!MarketCatalog::IsValidId(Draft.Id)) { Toast(TEXT("\u00d6nce ge\u00e7erli bir \u00fcr\u00fcn kimli\u011fi gir."), true); return FReply::Handled(); }
    const bool bModel = SelectedPrompt == TEXT("B1") || SelectedPrompt == TEXT("C1");
    const FString Folder = bModel ? ModelFolder(Draft.Id) : DeliveryFolder(Draft.Id, SelectedEra);
    IFileManager::Get().MakeDirectory(*Folder, true);
    FPlatformProcess::ExploreFolder(*Folder);
    return FReply::Handled();
}

FReply SProductStudio::OnExportReadyTemplates()
{
    if (Draft.Preset.IsEmpty())
    {
        Toast(TEXT("\u00d6nce haz\u0131r ambalaj k\u00fct\u00fcphanesinden bir ambalaj se\u00e7."), true);
        return FReply::Handled();
    }
    TArray<FString> Files;
    FString Error;
    if (!ExportReadyPackageTemplates(Draft, SelectedEra, Files, Error))
    {
        Toast(Error, true);
        return FReply::Handled();
    }
    const FString Folder = DeliveryFolder(Draft.Id, SelectedEra);
    FPlatformProcess::ExploreFolder(*Folder);
    TArray<FString> Names;
    for (const FString& File : Files) Names.Add(FPaths::GetCleanFilename(File));
    Toast(TEXT("Ajan \u015fablonlar\u0131 olu\u015fturuldu: ") + FString::Join(Names, TEXT(", ")));
    return FReply::Handled();
}

FReply SProductStudio::OnExportUvTemplate()
{
    const FStudioPackage* Package = DraftPackage();
    if (!Package || Package->Kind != EPackageKind::Model || Package->Id.StartsWith(TEXT("shape_")))
    {
        Toast(TEXT("UV k\u0131lavuzu yaln\u0131z i\u00e7e al\u0131nm\u0131\u015f \u00f6zel modeller i\u00e7indir."), true);
        return FReply::Handled();
    }
    if (!MarketCatalog::IsValidId(Draft.Id)) { Toast(TEXT("\u00d6nce ge\u00e7erli bir \u00fcr\u00fcn kimli\u011fi gir."), true); return FReply::Handled(); }
    const FString Folder = ModelFolder(Draft.Id);
    const FString Output = Folder / TEXT("uv_sablon.png");
    FString Error;
    if (!ExportUvTemplate(*Package, Output, Error)) { Toast(Error, true); return FReply::Handled(); }
    FPlatformProcess::ExploreFolder(*Folder);
    Toast(TEXT("UV k\u0131lavuzu olu\u015fturuldu: uv_sablon.png"));
    return FReply::Handled();
}

FReply SProductStudio::OnRemove()
{
    if (!Draft.bExisting) return FReply::Handled();
    const FText Question = T(FString::Printf(TEXT("%s katalogdan \u00e7\u0131kar\u0131ls\u0131n m\u0131?\n\nDosyalar silinmez; kay\u0131tl\u0131 oyunlarda bu \u00fcr\u00fcn\u00fcn sto\u011fu sonraki y\u00fcklemede kald\u0131r\u0131l\u0131r."), *Draft.RealName));
    if (FMessageDialog::Open(EAppMsgType::YesNo, Question) != EAppReturnType::Yes) return FReply::Handled();
    FString Message;
    const bool bOk = RemoveProduct(Draft.Id, Catalog, CatalogNote, Message);
    ReloadData();
    if (bOk) { if (Catalog.Num() > 0) SelectProduct(0); else NewProduct(FString()); }
    Toast(Message, !bOk);
    return FReply::Handled();
}

FReply SProductStudio::OnCreateBox()
{
    const auto ToInt = [](const FString& Text) { return Text.IsNumeric() ? FCString::Atoi(*Text) : 0; };
    int32 W = ToInt(BoxWidth), D = ToInt(BoxDepth), H = ToInt(BoxHeight);
    if (Mode == EMode::Products && (Draft.PackageType == TEXT("kutu") || Draft.PackageType == TEXT("poset")))
    {
        W = ToInt(Draft.WidthMm); D = ToInt(Draft.DepthMm); H = ToInt(Draft.HeightMm);
        Draft.Preset.Reset();
    }
    FString PackageId, Error;
    UStaticMesh* Mesh = CreateBoxPackage(W, D, H, PackageId, Error);
    if (!Mesh) { Toast(Error, true); return FReply::Handled(); }
    Packages = ScanPackages(Catalog);
    if (Mode == EMode::Products) ChoosePackage(PackageId);
    else SelectPackage(PackageId);
    Toast(FString::Printf(TEXT("Kutu \u015fablonu haz\u0131r: %d\u00d7%d\u00d7%d mm. \u015eimdi etiket g\u00f6rselini y\u00fckle."), W, D, H));
    return FReply::Handled();
}

FReply SProductStudio::OnImportModel()
{
    if (Mode == EMode::Products && MarketCatalog::IsValidId(Draft.Id) && FPaths::DirectoryExists(ModelFolder(Draft.Id)))
        LastDirectory = ModelFolder(Draft.Id);
    FString File;
    if (!PickFile(TEXT("3B model se\u00e7"), TEXT("3B model (fbx, obj, glb, gltf)|*.fbx;*.obj;*.glb;*.gltf"), File)) return FReply::Handled();
    FString PackageId, Error;
    UStaticMesh* Mesh = ImportModelPackage(File, PackageId, Error);
    if (!Mesh) { Toast(Error, true); return FReply::Handled(); }
    if (Mode == EMode::Products) Draft.Preset.Reset();
    Packages = ScanPackages(Catalog);
    if (Mode == EMode::Products) ChoosePackage(PackageId);
    else SelectPackage(PackageId);
    Toast(TEXT("Model i\u00e7e al\u0131nd\u0131: ") + PackageId + TEXT(". \u015eimdi etiketini y\u00fckle."));
    return FReply::Handled();
}

FReply SProductStudio::OnPickFace(int32 Face)
{
    FString File, Error;
    if (!PickFile(FString::Printf(TEXT("%s y\u00fcz g\u00f6rseli"), FaceTitles[Face]), ImageTypes, File)) return FReply::Handled();
    if (!ImageFor(File, &Error)) { Toast(Error, true); return FReply::Handled(); }
    Draft.FaceFiles[Face] = File;
    RebuildFaces();
    UpdatePreview();
    return FReply::Handled();
}

FReply SProductStudio::OnClearFace(int32 Face)
{
    Draft.FaceFiles[Face].Reset();
    RebuildFaces();
    UpdatePreview();
    return FReply::Handled();
}

FString& SProductStudio::ExtraFile(int32 Which)
{
    switch (Which)
    {
    case 1: return Draft.CapFile;
    case 2: return Draft.BodyFile;
    case 3: return Draft.NetFile;
    default: return Draft.LabelFile;
    }
}

FReply SProductStudio::OnPickExtra(int32 Which)
{
    static const TCHAR* Titles[] = { TEXT("Etiket g\u00f6rseli (modelin UV a\u00e7\u0131l\u0131m\u0131na g\u00f6re)"), TEXT("Kapak g\u00f6rseli"), TEXT("G\u00f6vde g\u00f6rseli"), TEXT("Kutunun tek g\u00f6rsel a\u00e7\u0131l\u0131m\u0131") };
    FString File, Error;
    if (!PickFile(Titles[FMath::Clamp(Which, 0, 3)], ImageTypes, File)) return FReply::Handled();
    if (!ImageFor(File, &Error)) { Toast(Error, true); return FReply::Handled(); }
    ExtraFile(Which) = File;
    RebuildFaces();
    UpdatePreview();
    return FReply::Handled();
}

FReply SProductStudio::OnClearExtra(int32 Which)
{
    ExtraFile(Which).Reset();
    RebuildFaces();
    UpdatePreview();
    return FReply::Handled();
}

bool SProductStudio::PickFile(const FString& Title, const FString& Types, FString& OutFile)
{
    IDesktopPlatform* Platform = FDesktopPlatformModule::Get();
    if (!Platform) return false;
    TArray<FString> Files;
    const void* Parent = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(AsShared());
    const FString Start = LastDirectory.IsEmpty() ? FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) : LastDirectory;
    if (!Platform->OpenFileDialog(Parent, Title, Start, FString(), Types, EFileDialogFlags::None, Files) || Files.Num() == 0) return false;
    OutFile = FPaths::ConvertRelativePathToFull(Files[0]);
    LastDirectory = FPaths::GetPath(OutFile);
    return true;
}

// ============================================================================ data helpers

const FStudioImage* SProductStudio::ImageFor(const FString& File, FString* OutError)
{
    if (File.IsEmpty()) return nullptr;
    const FString Key = File + TEXT("|") + IFileManager::Get().GetTimeStamp(*File).ToString();
    if (const TSharedPtr<FStudioImage>* Found = Images.Find(Key)) return Found->Get();
    TSharedPtr<FStudioImage> Image = MakeShared<FStudioImage>();
    FString Error;
    if (!LoadImageFile(File, *Image, Error))
    {
        if (OutError) *OutError = Error;
        return nullptr;
    }
    Images.Add(Key, Image);
    return Image.Get();
}

const FSlateBrush* SProductStudio::ThumbnailFor(const FString& File)
{
    if (File.IsEmpty()) return nullptr;
    if (const TSharedPtr<FSlateBrush>* Found = ThumbBrushes.Find(File)) return Found->Get();
    const FStudioImage* Image = ImageFor(File);
    if (!Image) return nullptr;
    const double Scale = FMath::Min(1.0, 160.0 / FMath::Max(Image->Width, Image->Height));
    FStudioImage Small;
    Resize(*Image, FMath::Max(1, FMath::RoundToInt32(Image->Width * Scale)), FMath::Max(1, FMath::RoundToInt32(Image->Height * Scale)), Small);
    UTexture2D* Texture = MakeTransientTexture(Small);
    if (!Texture) return nullptr;
    Thumbs.Add(File, TStrongObjectPtr<UTexture2D>(Texture));
    TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
    Brush->SetResourceObject(Texture);
    Brush->ImageSize = FVector2D(Small.Width, Small.Height);
    Brush->DrawAs = ESlateBrushDrawType::Image;
    ThumbBrushes.Add(File, Brush);
    return Brush.Get();
}

const FStudioPackage* SProductStudio::DraftPackage() const
{
    return FindPackage(Packages, Mode == EMode::Products ? Draft.PackageId : SelectedPackageId);
}

FString SProductStudio::PackageSummary(const FString& MeshPath) const
{
    if (MeshPath.IsEmpty()) return TEXT("Prototip kutu");
    const FStudioPackage* Found = Packages.FindByPredicate([&](const FStudioPackage& P) { return P.MeshPath == MeshPath; });
    return Found ? Found->DisplayName : TEXT("Ambalaj bulunamad\u0131");
}

void SProductStudio::UpdatePreview()
{
    ImageIssues.Reset();
    PreviewTextures.Reset();
    PreviewMaterials.Reset();
    const FStudioPackage* Package = DraftPackage();
    UStaticMesh* Mesh = Package ? LoadObject<UStaticMesh>(nullptr, *Package->MeshPath) : nullptr;
    TArray<UMaterialInterface*> Materials;
    if (Mode == EMode::Products && Package)
    {
        Materials.SetNumZeroed(FMath::Max(1, Package->SlotNames.Num()));
        for (int32 Slot = 0; Slot < Materials.Num(); ++Slot)
            if (Draft.ExistingMaterials.IsValidIndex(Slot) && !Draft.ExistingMaterials[Slot].IsEmpty())
                Materials[Slot] = LoadObject<UMaterialInterface>(nullptr, *Draft.ExistingMaterials[Slot]);
        FString Error;
        UMaterialInterface* Parent = EnsureLabelMaterial(Error);
        const auto Preview = [&](int32 Slot, const FStudioImage& Image)
        {
            UTexture2D* Texture = MakeTransientTexture(Image);
            if (!Texture || !Parent || !Materials.IsValidIndex(Slot)) return;
            UMaterialInstanceDynamic* Instance = UMaterialInstanceDynamic::Create(Parent, GetTransientPackage());
            Instance->SetTextureParameterValue(TEXT("Label"), Texture);
            PreviewTextures.Emplace(Texture);
            PreviewMaterials.Emplace(Instance);
            Materials[Slot] = Instance;
        };
        if (Package->Kind == EPackageKind::Box)
        {
            FStudioImage NetFaces[FBoxPackageLayout::FaceCount];
            const FStudioImage* Faces[FBoxPackageLayout::FaceCount] = {};
            bool bAny = false;
            if (const FStudioImage* Net = ImageFor(Draft.NetFile))
            {
                SplitNet(Draft.NetFile, *Net, Package->WidthMm, Package->DepthMm, Package->HeightMm, NetFaces, &ImageIssues);
                for (int32 F = 0; F < FBoxPackageLayout::FaceCount; ++F) Faces[F] = &NetFaces[F];
                bAny = true;
            }
            for (int32 F = 0; F < FBoxPackageLayout::FaceCount; ++F)
                if (const FStudioImage* Face = ImageFor(Draft.FaceFiles[F])) { Faces[F] = Face; bAny = true; }
            FBoxPackageLayout Layout;
            if (bAny && FBoxPackageLayout::Make(Package->WidthMm, Package->DepthMm, Package->HeightMm, Layout))
            {
                FStudioImage Atlas;
                ComposeAtlas(Layout, Faces, Atlas, &ImageIssues);
                Preview(0, Atlas);
            }
        }
        else
        {
            if (const FStudioImage* Label = ImageFor(Draft.LabelFile)) Preview(Package->LabelSlot, *Label);
            if (Package->CapSlot != INDEX_NONE)
                if (const FStudioImage* Cap = ImageFor(Draft.CapFile)) Preview(Package->CapSlot, *Cap);
            if (Package->BodySlot != INDEX_NONE)
                if (const FStudioImage* Body = ImageFor(Draft.BodyFile)) Preview(Package->BodySlot, *Body);
            // Part colors (same rule as Publish: an image on the slot wins).
            for (int32 Slot = 0; Mesh && Slot < Package->SlotNames.Num() && Slot < Materials.Num(); ++Slot)
            {
                const ESlotRole Role = SlotRole(Package->SlotNames[Slot]);
                if (Role == ESlotRole::Label || Role == ESlotRole::Other) continue;
                if ((Role == ESlotRole::Cap && !Draft.CapFile.IsEmpty()) || (Role == ESlotRole::Body && !Draft.BodyFile.IsEmpty())) continue;
                FLinearColor Tint; float Opacity = -1.f;
                UMaterialInterface* Base = Mesh->GetMaterial(Slot);
                if (!Base || Base->IsA<UMaterial>()) continue;
                if (!FindColor(Draft.Colors, Package->SlotNames[Slot], Tint, Opacity)) { Materials[Slot] = nullptr; continue; }
                UMaterialInstanceDynamic* Instance = UMaterialInstanceDynamic::Create(Base, GetTransientPackage());
                Instance->SetVectorParameterValue(TEXT("Color"), Tint);
                if (Opacity > 0) Instance->SetScalarParameterValue(TEXT("Opacity"), Opacity);
                PreviewMaterials.Emplace(Instance);
                Materials[Slot] = Instance;
            }
        }
        if (!Parent) Toast(Error, true);
    }
    FTransform Correction = FTransform::Identity;
    if (Package && Package->Id.StartsWith(TEXT("model_")))
    {
        const auto Number = [](const FString& Text, float Default)
        {
            const FString Normalized = Text.TrimStartAndEnd().Replace(TEXT(","), TEXT("."));
            return Normalized.IsNumeric() ? FCString::Atof(*Normalized) : Default;
        };
        const float Scale = FMath::Clamp(Number(Draft.ModelScale, 1.f), 0.001f, 1000.f);
        const FRotator Rotation(Number(Draft.ModelPitch, 0.f), Number(Draft.ModelYaw, 0.f), Number(Draft.ModelRoll, 0.f));
        const FVector Offset(Number(Draft.ModelOffsetX, 0.f), Number(Draft.ModelOffsetY, 0.f), Number(Draft.ModelOffsetZ, 0.f));
        Correction = FTransform(Rotation, Offset, FVector(Scale));
    }
    if (Viewport.IsValid()) Viewport->ShowItem(Mesh, Materials, Correction);
    RebuildIssues();
}

void SProductStudio::Toast(const FString& Text, bool bError)
{
    ToastText = Text;
    bToastError = bError;
    RebuildHeader();
}

// ============================================================================ widget helpers

TSharedRef<SWidget> SProductStudio::MakeText(const FString& Text, int32 Size, const FLinearColor& Color, const TCHAR* Weight, bool bWrap) const
{
    return SNew(STextBlock).Text(T(Text)).Font(FStudioStyle::Get().Font(Size, Weight)).ColorAndOpacity(FSlateColor(Color)).AutoWrapText(bWrap);
}

TSharedRef<SWidget> SProductStudio::MakeSection(const FString& Title) const
{
    const FStudioStyle& S = FStudioStyle::Get();
    return SNew(SBox).Padding(FMargin(0.f, 18.f, 0.f, 10.f))[ MakeText(Title, 8, S.Faint, TEXT("Bold")) ];
}

TSharedRef<SWidget> SProductStudio::MakeButton(const FString& Label, const FButtonStyle* Style, FOnClicked OnClicked, const FLinearColor& TextColor, bool bEnabled, int32 Size) const
{
    return SNew(SButton)
        .ButtonStyle(Style)
        .IsEnabled(bEnabled)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        .ContentPadding(FMargin(14.f, 9.f))
        .OnClicked(OnClicked)
        [ MakeText(Label, Size, bEnabled ? TextColor : FStudioStyle::Get().Faint, TEXT("Bold")) ];
}

TSharedRef<SWidget> SProductStudio::MakeField(const FString& Label, const FString& Value, TFunction<void(const FString&)> OnChanged, const FString& Hint, bool bReadOnly, TSharedPtr<SEditableTextBox>* OutBox) const
{
    const FStudioStyle& S = FStudioStyle::Get();
    TSharedPtr<SEditableTextBox> Box;
    TSharedRef<SWidget> Widget = SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ MakeText(Label, 9, S.Muted) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SAssignNew(Box, SEditableTextBox)
            .Style(&S.Input)
            .Font(S.Font(11))
            .Text(T(Value))
            .HintText(T(Hint))
            .IsReadOnly(bReadOnly)
            .SelectAllTextWhenFocused(true)
            .OnTextChanged_Lambda([OnChanged](const FText& Text) { OnChanged(Text.ToString()); })
        ];
    if (OutBox) *OutBox = Box;
    return Widget;
}

TSharedRef<SWidget> SProductStudio::MakeTile(const FString& Title, const FString& Sub, const FString& File, FOnClicked OnPick, FOnClicked OnClear)
{
    const FStudioStyle& S = FStudioStyle::Get();
    const FSlateBrush* Thumb = ThumbnailFor(File);
    TSharedRef<SWidget> Picture = Thumb
        ? StaticCastSharedRef<SWidget>(SNew(SScaleBox).Stretch(EStretch::ScaleToFit)[ SNew(SImage).Image(Thumb) ])
        : StaticCastSharedRef<SWidget>(SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)[ MakeText(TEXT("+"), 22, S.Faint, TEXT("Light")) ]);
    TSharedRef<SOverlay> Overlay = SNew(SOverlay)
        + SOverlay::Slot()
        [
            SNew(SButton).ButtonStyle(&S.Tile).ContentPadding(FMargin(8.f)).OnClicked(OnPick)
            .ToolTipText(T(File.IsEmpty() ? TEXT("G\u00f6rsel se\u00e7") : File))
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[ SNew(SBox).WidthOverride(78.f).HeightOverride(78.f)[ Picture ] ]
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f)[ MakeText(Title, 10, S.Text, TEXT("Bold")) ]
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[ MakeText(Sub, 8, File.IsEmpty() ? S.Faint : S.Accent) ]
            ]
        ];
    if (!File.IsEmpty())
    {
        Overlay->AddSlot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(4.f)
        [
            SNew(SButton).ButtonStyle(&S.Ghost).ContentPadding(FMargin(6.f, 1.f)).OnClicked(OnClear).ToolTipText(T(TEXT("Kald\u0131r")))
            [ MakeText(TEXT("\u00d7"), 11, S.Muted, TEXT("Bold")) ]
        ];
    }
    return Overlay;
}

TSharedRef<SWidget> SProductStudio::MakeBoxCreator()
{
    const FStudioStyle& S = FStudioStyle::Get();
    if (Mode == EMode::Products)
    {
        // The product's own production data decides what to create.
        if (Draft.PackageType == TEXT("kutu") || Draft.PackageType == TEXT("poset"))
        {
            const FString Size = FString::Printf(TEXT("%s\u00d7%s\u00d7%s mm"), *Draft.WidthMm, *Draft.DepthMm, *Draft.HeightMm);
            return MakeButton(TEXT("Bu \u00f6l\u00e7\u00fcyle kutu \u015fablonu olu\u015ftur (") + Size + TEXT(")"), &S.Secondary, FOnClicked::CreateSP(this, &SProductStudio::OnCreateBox), S.Text);
        }
        return MakeButton(TEXT("Modeli i\u00e7e al\u2026 (Uretim/") + Draft.Id + TEXT("/model/model.fbx)"), &S.Secondary, FOnClicked::CreateSP(this, &SProductStudio::OnImportModel), S.Text);
    }
    const auto Small = [this](const FString& Label, FString* Target)
    {
        return MakeField(Label, *Target, [Target](const FString& Value) { *Target = Value; });
    };
    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 6.f, 0.f)[ Small(TEXT("Geni\u015flik mm"), &BoxWidth) ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 6.f, 0.f)[ Small(TEXT("Derinlik mm"), &BoxDepth) ]
            + SHorizontalBox::Slot().FillWidth(1.f)[ Small(TEXT("Y\u00fckseklik mm"), &BoxHeight) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 6.f, 0.f)
            [ MakeButton(TEXT("Kutu \u015fablonu olu\u015ftur"), &S.Secondary, FOnClicked::CreateSP(this, &SProductStudio::OnCreateBox), S.Text) ]
            + SHorizontalBox::Slot().FillWidth(1.f)
            [ MakeButton(TEXT("Model i\u00e7e al\u2026"), &S.Secondary, FOnClicked::CreateSP(this, &SProductStudio::OnImportModel), S.Text) ]
        ];
}

TSharedRef<SWidget> SProductStudio::MakeProductCard(int32 Index) const
{
    const FStudioStyle& S = FStudioStyle::Get();
    const FMarketProduct& P = Catalog[Index];
    const bool bSelected = Mode == EMode::Products && Index == SelectedProduct;
    const bool bVisual = !P.MeshPath.IsEmpty();
    FString Sub = CommaMoney(P.BasePrice) + TEXT(" TL  \u00b7  ") + (P.PackageType.IsEmpty() ? FString(TEXT("ambalaj yok")) : PackageTypeLabel(P.PackageType));
    SProductStudio* Self = const_cast<SProductStudio*>(this);
    return SNew(SButton)
        .ButtonStyle(bSelected ? &S.CardSelected : &S.Card)
        .ContentPadding(FMargin(10.f, 8.f))
        .OnClicked_Lambda([Self, Index]() { Self->SelectProduct(Index); return FReply::Handled(); })
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ SNew(SBox).WidthOverride(30.f).HeightOverride(30.f)[ SNew(SBorder).BorderImage(&S.SwatchBrush).BorderBackgroundColor(FLinearColor(P.Color)) ] ]
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(12.f, 0.f, 6.f, 0.f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ MakeText(P.RealName, 10, S.Text, TEXT("Bold")) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)[ MakeText(Sub, 8, S.Muted) ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ MakeText(bVisual ? TEXT("3B") : TEXT(""), 8, S.Accent, TEXT("Bold")) ]
        ];
}

TSharedRef<SWidget> SProductStudio::MakePackageCard(const FStudioPackage& Package, bool bSelected, FOnClicked OnClicked) const
{
    const FStudioStyle& S = FStudioStyle::Get();
    const FString Kind = Package.Kind == EPackageKind::Box ? TEXT("Kutu \u015fablonu") : TEXT("\u0130\u00e7e al\u0131nm\u0131\u015f model");
    return SNew(SButton)
        .ButtonStyle(bSelected ? &S.CardSelected : &S.Card)
        .ContentPadding(FMargin(10.f, 9.f))
        .OnClicked(OnClicked)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ MakeText(Package.DisplayName, 10, S.Text, TEXT("Bold")) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
            [ MakeText(FString::Printf(TEXT("%s  \u00b7  %.1f\u00d7%.1f\u00d7%.1f cm  \u00b7  %d \u00fcr\u00fcn"), *Kind, Package.SizeCm.Y, Package.SizeCm.X, Package.SizeCm.Z, Package.UsedBy), 8, S.Muted) ]
        ];
}

// ============================================================================ rebuilds

void SProductStudio::RebuildAll()
{
    RebuildHeader();
    RebuildLeft();
    RebuildRight();
    RebuildFaces();
    RebuildTitle();
    RebuildPrompt();
}

void SProductStudio::RebuildHeader()
{
    if (!HeaderSlot.IsValid()) return;
    const FStudioStyle& S = FStudioStyle::Get();
    int32 Visual = 0;
    for (const FMarketProduct& P : Catalog) if (!P.MeshPath.IsEmpty()) ++Visual;
    const int32 Active = MarketCatalog::CountActive(Catalog);
    HeaderSlot->SetContent(
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ MakeText(TEXT("M\u0130RAS MARKET"), 8, S.Faint, TEXT("Bold")) ]
            + SVerticalBox::Slot().AutoHeight()[ MakeText(TEXT("\u00dcr\u00fcn St\u00fcdyosu"), 22, S.Text, TEXT("Light")) ]
        ]
        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(28.f, 0.f)
        [ MakeText(ToastText, 10, bToastError ? S.Danger : S.Accent, TEXT("Regular"), true) ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
        [
            SNew(SBorder).BorderImage(&S.PillBrush).Padding(FMargin(14.f, 7.f))
            [ MakeText(FString::Printf(TEXT("%d oyunda  \u00b7  %d haz\u0131rl\u0131k  \u00b7  %d 3B  \u00b7  %d ambalaj"), Active, Catalog.Num() - Active, Visual, Packages.Num()), 9, S.Muted) ]
        ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
        [
            MakeButton(TEXT("Yenile"), &S.Ghost, FOnClicked::CreateLambda([this]()
            {
                const FString Id = Draft.bExisting ? Draft.Id : FString();
                ReloadData();
                const int32 Index = Catalog.IndexOfByPredicate([&](const FMarketProduct& P) { return P.Id == Id; });
                if (Index != INDEX_NONE) SelectProduct(Index); else RebuildAll();
                return FReply::Handled();
            }), S.Muted)
        ]);
}

void SProductStudio::RebuildLeft()
{
    if (!LeftSlot.IsValid()) return;
    const FStudioStyle& S = FStudioStyle::Get();
    TSharedRef<SVerticalBox> List = SNew(SVerticalBox);
    const int32 ActiveCount = MarketCatalog::CountActive(Catalog);
    if (Mode == EMode::Products)
    {
        FString LastCategory;
        for (int32 I = 0; I < Catalog.Num(); ++I)
        {
            if (Catalog[I].bActive != bListActive) continue;
            if (Catalog[I].Category != LastCategory)
            {
                LastCategory = Catalog[I].Category;
                List->AddSlot().AutoHeight().Padding(4.f, 10.f, 0.f, 4.f)[ MakeText(LastCategory.ToUpper(), 8, S.Faint, TEXT("Bold")) ];
            }
            List->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)[ MakeProductCard(I) ];
        }
        if (List->NumSlots() == 0)
            List->AddSlot().AutoHeight()[ MakeText(bListActive ? TEXT("Oyunda \u00fcr\u00fcn yok.") : TEXT("Haz\u0131rl\u0131k listesi bo\u015f."), 9, S.Muted) ];
    }
    else
    {
        for (const FStudioPackage& Package : Packages)
        {
            const FString Id = Package.Id;
            List->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
            [ MakePackageCard(Package, Id == SelectedPackageId, FOnClicked::CreateLambda([this, Id]() { SelectPackage(Id); return FReply::Handled(); })) ];
        }
        if (Packages.Num() == 0)
            List->AddSlot().AutoHeight()[ MakeText(TEXT("Hen\u00fcz ambalaj yok. Bir \u00fcr\u00fcn se\u00e7ip kutu \u015fablonu olu\u015ftur ya da model i\u00e7e al."), 9, S.Muted, TEXT("Regular"), true) ];
    }
    const auto Segment = [this, &S](const FString& Label, bool bOn, TFunction<void()> Action)
    {
        return MakeButton(Label, bOn ? &S.SegmentActive : &S.Segment,
            FOnClicked::CreateLambda([Action]() { Action(); return FReply::Handled(); }), bOn ? S.Text : S.Muted, true, 9);
    };
    LeftSlot->SetContent(
        SNew(SBorder).BorderImage(&S.SurfaceBrush).Padding(12.f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SBorder).BorderImage(&S.RaisedBrush).Padding(4.f)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().FillWidth(1.f)[ Segment(FString::Printf(TEXT("Oyunda %d"), ActiveCount), Mode == EMode::Products && bListActive,
                        [this]() { bListActive = true; SetMode(EMode::Products); }) ]
                    + SHorizontalBox::Slot().FillWidth(1.f)[ Segment(FString::Printf(TEXT("Haz\u0131rl\u0131k %d"), Catalog.Num() - ActiveCount), Mode == EMode::Products && !bListActive,
                        [this]() { bListActive = false; SetMode(EMode::Products); }) ]
                    + SHorizontalBox::Slot().FillWidth(1.f)[ Segment(TEXT("Ambalaj"), Mode == EMode::Packages, [this]() { SetMode(EMode::Packages); }) ]
                ]
            ]
            + SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 8.f)[ SNew(SScrollBox) + SScrollBox::Slot()[ List ] ]
            + SVerticalBox::Slot().AutoHeight()
            [
                Mode == EMode::Products
                    ? MakeButton(TEXT("+  Yeni \u00fcr\u00fcn"), &S.Primary, FOnClicked::CreateLambda([this]() { NewProduct(FString()); return FReply::Handled(); }), S.AccentText)
                    : MakeButton(TEXT("+  Bu ambalajla yeni \u00fcr\u00fcn"), &S.Primary, FOnClicked::CreateLambda([this]() { NewProduct(SelectedPackageId); return FReply::Handled(); }), S.AccentText, !SelectedPackageId.IsEmpty())
            ]
        ]);
}

void SProductStudio::RebuildTitle()
{
    if (!TitleSlot.IsValid()) return;
    const FStudioStyle& S = FStudioStyle::Get();
    FString Title, Sub;
    const FStudioPackage* Package = DraftPackage();
    if (Mode == EMode::Products)
    {
        Title = Draft.RealName.TrimStartAndEnd().IsEmpty() ? TEXT("Yeni \u00fcr\u00fcn") : Draft.RealName;
        Sub = (Draft.Price.IsEmpty() ? FString(TEXT("Fiyat yok")) : Draft.Price + TEXT(" TL")) + TEXT("  \u00b7  ") + (Package ? Package->DisplayName : FString(TEXT("Ambalaj se\u00e7ilmedi")));
    }
    else
    {
        Title = Package ? Package->DisplayName : TEXT("Ambalaj");
        Sub = Package ? FString::Printf(TEXT("%.1f \u00d7 %.1f \u00d7 %.1f cm"), Package->SizeCm.Y, Package->SizeCm.X, Package->SizeCm.Z) : TEXT("Soldan bir ambalaj se\u00e7");
    }
    TitleSlot->SetContent(
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ MakeText(Title, 24, S.Text, TEXT("Light")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)[ MakeText(Sub, 10, S.Muted) ]);
}

void SProductStudio::RebuildFaces()
{
    if (!FacesSlot.IsValid()) return;
    const FStudioStyle& S = FStudioStyle::Get();
    const FStudioPackage* Package = DraftPackage();
    TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
    FString Hint;
    if (Mode != EMode::Products)
    {
        Hint = TEXT("Ambalajlar birden \u00e7ok \u00fcr\u00fcnde kullan\u0131labilir: ayn\u0131 kutu, farkl\u0131 etiket.");
    }
    else if (!Package)
    {
        Hint = TEXT("\u00d6nce sa\u011fdan bir ambalaj se\u00e7 ya da olu\u015ftur.");
    }
    else if (Package->Kind == EPackageKind::Box)
    {
        Hint = TEXT("\u0130ki yol: tek g\u00f6rsel a\u00e7\u0131l\u0131m (6 y\u00fcz bir arada; st\u00fcdyo b\u00f6ler) ya da y\u00fczleri tek tek y\u00fckle. Tek tek y\u00fcklenen y\u00fcz, a\u00e7\u0131l\u0131mdakinin yerine ge\u00e7er. Yaln\u0131z \u00f6n y\u00fcz zorunlu.");
        FBoxPackageLayout Layout;
        FBoxPackageLayout::Make(Package->WidthMm, Package->DepthMm, Package->HeightMm, Layout);
        {
            const FString File = Draft.NetFile;
            const double NetW = 2.0 * Package->WidthMm + 2.0 * Package->DepthMm;
            const double NetH = Package->HeightMm + 2.0 * Package->DepthMm;
            const double Scale = FMath::Min3(8.0, 4096.0 / NetW, 4096.0 / NetH);
            const FString Sub = File.IsEmpty()
                ? FString::Printf(TEXT("%d\u00d7%d px"), FMath::FloorToInt32(NetW * Scale), FMath::FloorToInt32(NetH * Scale))
                : FPaths::GetCleanFilename(File);
            Row->AddSlot().AutoWidth().Padding(4.f, 0.f, 12.f, 0.f)
            [ SNew(SBox).WidthOverride(150.f)[ MakeTile(TEXT("Tek g\u00f6rsel (a\u00e7\u0131l\u0131m)"), Sub, File, FOnClicked::CreateSP(this, &SProductStudio::OnPickExtra, 3), FOnClicked::CreateSP(this, &SProductStudio::OnClearExtra, 3)) ] ];
        }
        for (int32 F = 0; F < FBoxPackageLayout::FaceCount; ++F)
        {
            const FString File = Draft.FaceFiles[F];
            const FString Sub = File.IsEmpty() ? FString::Printf(TEXT("%d\u00d7%d px"), Layout.Rects[F].Width(), Layout.Rects[F].Height()) : FPaths::GetCleanFilename(File);
            Row->AddSlot().FillWidth(1.f).Padding(4.f, 0.f)
            [ MakeTile(FaceTitles[F], Sub, File, FOnClicked::CreateSP(this, &SProductStudio::OnPickFace, F), FOnClicked::CreateSP(this, &SProductStudio::OnClearFace, F)) ];
        }
    }
    else
    {
        const bool bShape = Package->Id.StartsWith(TEXT("shape_"));
        Hint = bShape
            ? FString(TEXT("Haz\u0131r \u015fekil. Etiket: a\u00e7\u0131lm\u0131\u015f etiket g\u00f6rseli (yatay orta = \u00fcr\u00fcn\u00fcn \u00f6n\u00fc). Kapak: kapa\u011f\u0131n \u00fcstten kare g\u00f6rseli; yoksa sa\u011fdaki PAR\u00c7A RENKLER\u0130 kullan\u0131l\u0131r."))
            : FString::Printf(TEXT("Model yuvalar\u0131: %s. Etiket: UV a\u00e7\u0131l\u0131m\u0131na g\u00f6re tek g\u00f6rsel. Cam yuvas\u0131 otomatik saydam. Kapak/G\u00f6vde dokusu iste\u011fe ba\u011fl\u0131."),
            *FString::Join(Package->SlotNames, TEXT(", ")));
        const auto AddTile = [&](const FString& Title, int32 Which)
        {
            const FString File = ExtraFile(Which);
            Row->AddSlot().AutoWidth().Padding(4.f, 0.f)
            [
                SNew(SBox).WidthOverride(150.f)
                [ MakeTile(Title, File.IsEmpty() ? FString(TEXT("g\u00f6rsel se\u00e7")) : FPaths::GetCleanFilename(File), File,
                    FOnClicked::CreateSP(this, &SProductStudio::OnPickExtra, Which), FOnClicked::CreateSP(this, &SProductStudio::OnClearExtra, Which)) ]
            ];
        };
        AddTile(bShape ? TEXT("Etiket") : TEXT("Etiket (UV)"), 0);
        if (Package->CapSlot != INDEX_NONE) AddTile(bShape ? TEXT("Kapak (\u00fcstten)") : TEXT("Kapak"), 1);
        if (Package->BodySlot != INDEX_NONE && !bShape) AddTile(TEXT("G\u00f6vde"), 2);
        if (!bShape)
        {
            Row->AddSlot().AutoWidth().Padding(8.f, 0.f, 4.f, 0.f).VAlign(VAlign_Center)
            [ SNew(SBox).WidthOverride(170.f).HeightOverride(42.f)
                [ MakeButton(TEXT("UV k\u0131lavuzu olu\u015ftur"), &S.Secondary, FOnClicked::CreateSP(this, &SProductStudio::OnExportUvTemplate), S.Text, true, 9) ] ];
        }
    }
    FacesSlot->SetContent(
        SNew(SBorder).BorderImage(&S.SurfaceBrush).Padding(FMargin(14.f, 12.f))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ MakeText(TEXT("ET\u0130KET"), 8, S.Faint, TEXT("Bold")) ]
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(14.f, 0.f, 0.f, 0.f)[ MakeText(Hint, 9, S.Muted, TEXT("Regular"), true) ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, Row->NumSlots() > 0 ? 10.f : 0.f, 0.f, 0.f)[ Row ]
        ]);
}

void SProductStudio::RebuildIssues()
{
    if (!IssuesSlot.IsValid() || Mode != EMode::Products) return;
    const FStudioStyle& S = FStudioStyle::Get();
    TArray<FStudioIssue> Issues = Validate(Draft, Catalog, Packages, true);
    Issues.Append(ImageIssues);
    const bool bCanPlay = !HasErrors(Issues);
    const bool bCanSave = !HasErrors(Validate(Draft, Catalog, Packages, false));
    if (bCanPlay)
        Issues.Insert(FStudioIssue{ EStudioSeverity::Ok, FString(Draft.bActive ? TEXT("Oyunda. De\u011fi\u015fiklikler uygulanabilir.") : TEXT("Oyuna eklenmeye haz\u0131r.")) }, 0);
    else if (bCanSave)
        Issues.Insert(FStudioIssue{ EStudioSeverity::Ok, FString(TEXT("Haz\u0131rl\u0131k listesine kaydedilebilir. Oyuna eklemek i\u00e7in a\u015fa\u011f\u0131dakiler gerekli:")) }, 0);
    TSharedRef<SVerticalBox> List = SNew(SVerticalBox);
    for (const FStudioIssue& Issue : Issues)
    {
        // In preparation, missing game requirements are to-dos, not failures.
        const bool bTodo = Issue.Severity == EStudioSeverity::Error && bCanSave && !Draft.bActive;
        const FLinearColor Color = Issue.Severity == EStudioSeverity::Ok ? S.Accent : (Issue.Severity == EStudioSeverity::Warning || bTodo) ? S.Warn : S.Danger;
        List->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 5.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[ MakeText(TEXT("\u2022"), 12, Color, TEXT("Bold")) ]
            + SHorizontalBox::Slot().FillWidth(1.f)[ MakeText(Issue.Text, 9, Issue.Severity == EStudioSeverity::Ok ? S.Text : S.Muted, TEXT("Regular"), true) ]
        ];
    }
    TSharedRef<SHorizontalBox> Buttons = SNew(SHorizontalBox);
    if (Draft.bActive)
    {
        Buttons->AddSlot().FillWidth(1.f).Padding(0.f, 0.f, 6.f, 0.f)
        [ SNew(SBox).HeightOverride(44.f)[ MakeButton(TEXT("De\u011fi\u015fiklikleri uygula"), &S.Primary, FOnClicked::CreateSP(this, &SProductStudio::OnActivate), S.AccentText, bCanPlay, 11) ] ];
        Buttons->AddSlot().AutoWidth()
        [ SNew(SBox).HeightOverride(44.f)[ MakeButton(TEXT("Oyundan \u00e7\u0131kar"), &S.Secondary, FOnClicked::CreateSP(this, &SProductStudio::OnDeactivate), S.Text, true, 10) ] ];
    }
    else
    {
        Buttons->AddSlot().FillWidth(1.f).Padding(0.f, 0.f, 6.f, 0.f)
        [ SNew(SBox).HeightOverride(44.f)[ MakeButton(TEXT("Oyuna ekle"), &S.Primary, FOnClicked::CreateSP(this, &SProductStudio::OnActivate), S.AccentText, bCanPlay, 11) ] ];
        Buttons->AddSlot().AutoWidth()
        [ SNew(SBox).HeightOverride(44.f)[ MakeButton(TEXT("Kaydet"), &S.Secondary, FOnClicked::CreateSP(this, &SProductStudio::OnSave), S.Text, bCanSave, 10) ] ];
    }
    IssuesSlot->SetContent(
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ SNew(SBorder).BorderImage(&S.RaisedBrush).Padding(FMargin(14.f, 12.f))[ SNew(SBox).MaxDesiredHeight(170.f)[ SNew(SScrollBox) + SScrollBox::Slot()[ List ] ] ] ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)[ Buttons ]);
    RebuildTitle();
}

void SProductStudio::RebuildPrompt()
{
    if (!PromptSlot.IsValid() || Mode != EMode::Products) return;
    const FStudioStyle& S = FStudioStyle::Get();
    FString Text, Error;
    if (!CurrentPrompt(Text, Error)) Text = Error;
    PromptSlot->SetContent(
        SNew(SBox).MaxDesiredHeight(240.f)
        [
            SNew(SMultiLineEditableTextBox)
            .Style(&S.Input)
            .Font(S.Font(8))
            .Text(FText::FromString(Text))
            .IsReadOnly(true)
            .AutoWrapText(true)
        ]);
}

void SProductStudio::RebuildRight()
{
    if (!RightSlot.IsValid()) return;
    const FStudioStyle& S = FStudioStyle::Get();
    TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);
    TSharedRef<SVerticalBox> Footer = SNew(SVerticalBox);

    if (Mode == EMode::Products)
    {
        AddProductFields(Content);
        AddPackageChoice(Content);
        AddModelSettings(Content);
        AddProduction(Content);
        Footer->AddSlot().AutoHeight()[ SAssignNew(IssuesSlot, SBox) ];
        if (Draft.bExisting)
        {
            Footer->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
            [ MakeButton(TEXT("Katalogdan sil"), &S.DangerGhost, FOnClicked::CreateSP(this, &SProductStudio::OnRemove), S.Danger, true, 9) ];
        }
    }
    else
    {
        AddPackageInfo(Content, Footer);
    }

    RightSlot->SetContent(
        SNew(SBorder).BorderImage(&S.SurfaceBrush).Padding(FMargin(20.f, 18.f))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().FillHeight(1.f)[ SNew(SScrollBox) + SScrollBox::Slot().Padding(0.f, 0.f, 8.f, 0.f)[ Content ] ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[ Footer ]
        ]);
    RebuildIssues();
}

// Right panel, product mode: heading, name/brand/id, prices.
void SProductStudio::AddProductFields(const TSharedRef<SVerticalBox>& Content)
{
    const FStudioStyle& S = FStudioStyle::Get();
    const auto Changed = [this]() { RebuildIssues(); RebuildPrompt(); };
    const FString Heading = !Draft.bExisting ? TEXT("Yeni \u00fcr\u00fcn") : Draft.bActive ? TEXT("Oyundaki \u00fcr\u00fcn") : TEXT("Haz\u0131rl\u0131ktaki \u00fcr\u00fcn");
    Content->AddSlot().AutoHeight()[ MakeText(Heading, 16, S.Text, TEXT("Light")) ];

    // Product --------------------------------------------------------------------------
    Content->AddSlot().AutoHeight()[ MakeSection(TEXT("\u00dcR\u00dcN")) ];
    Content->AddSlot().AutoHeight()
    [
        MakeField(TEXT("\u00dcr\u00fcn ad\u0131 (ger\u00e7ek marka)"), Draft.RealName, [this, Changed](const FString& Value)
        {
            Draft.RealName = Value;
            if (!Draft.bExisting && !bIdTouched && IdBox.IsValid())
            {
                Draft.Id = MarketCatalog::MakeId(Value);
                IdBox->SetText(T(Draft.Id));
            }
            Changed();
        }, TEXT("\u00f6rn. S\u00fcta\u015f S\u00fct 1 L"))
    ];
    Content->AddSlot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 8.f, 0.f)
        [ MakeField(TEXT("Marka"), Draft.Brand, [this, Changed](const FString& Value) { Draft.Brand = Value; Changed(); }, TEXT("\u00f6rn. S\u00fcta\u015f")) ]
        + SHorizontalBox::Slot().FillWidth(1.f)
        [ MakeField(TEXT("Kategori"), Draft.Category, [this](const FString& Value) { Draft.Category = Value; }, TEXT("\u00f6rn. s\u00fct")) ]
    ];
    Content->AddSlot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 8.f, 0.f)
        [
            MakeField(TEXT("\u00dcr\u00fcn kimli\u011fi"), Draft.Id, [this, Changed](const FString& Value)
            {
                if (IdBox.IsValid() && IdBox->HasKeyboardFocus()) bIdTouched = true;
                Draft.Id = Value;
                Changed();
            }, TEXT("otomatik"), Draft.bExisting, &IdBox)
        ]
        + SHorizontalBox::Slot().FillWidth(1.f)
        [ MakeField(TEXT("Kurgu ad\u0131 (F8)"), Draft.FictionalName, [this](const FString& Value) { Draft.FictionalName = Value; }, TEXT("bo\u015f = \u00fcr\u00fcn ad\u0131")) ]
    ];

    // Price ----------------------------------------------------------------------------
    Content->AddSlot().AutoHeight()[ MakeSection(TEXT("F\u0130YAT")) ];
    Content->AddSlot().AutoHeight()
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 8.f, 0.f)
        [ MakeField(TEXT("Al\u0131\u015f (TL)"), Draft.Cost, [this](const FString& Value) { Draft.Cost = Value; RebuildIssues(); }, TEXT("1,70")) ]
        + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 8.f, 0.f)
        [ MakeField(TEXT("Sat\u0131\u015f (TL)"), Draft.Price, [this](const FString& Value) { Draft.Price = Value; RebuildIssues(); }, TEXT("2,50")) ]
        + SHorizontalBox::Slot().FillWidth(0.8f)
        [ MakeField(TEXT("Koli adedi"), Draft.CaseUnits, [this](const FString& Value) { Draft.CaseUnits = Value; RebuildIssues(); }, TEXT("12")) ]
    ];
}

// Right panel, product mode: package library, custom package sizes, cap toggle.
void SProductStudio::AddPackageChoice(const TSharedRef<SVerticalBox>& Content)
{
    const FStudioStyle& S = FStudioStyle::Get();
    const auto Changed = [this]() { RebuildIssues(); RebuildPrompt(); };
    // Package: a ready package from the library (Config/ambalajlar.json) ---------------------
    Content->AddSlot().AutoHeight()[ MakeSection(TEXT("AMBALAJ")) ];
    const FPackagePreset* CurrentPreset = FindPreset(Presets, Draft.Preset);
    const FStudioPackage* CurrentPackage = DraftPackage();
    {
        const FString Summary = CurrentPreset ? CurrentPreset->Name + TEXT("  \u00b7  ") + PresetSize(*CurrentPreset)
            : bCustomPackage ? FString(TEXT("\u00d6zel ambalaj (listede olmayan)"))
            : CurrentPackage ? CurrentPackage->DisplayName
            : FString(TEXT("Ambalaj se\u00e7ilmedi: a\u015fa\u011f\u0131dan birini se\u00e7."));
        Content->AddSlot().AutoHeight()[ MakeText(Summary, 11, (CurrentPreset || CurrentPackage || bCustomPackage) ? S.Text : S.Warn, TEXT("Bold"), true) ];
        Content->AddSlot().AutoHeight().Padding(0.f, 3.f, 0.f, 8.f)
        [ MakeText(CurrentPackage ? TEXT("3B haz\u0131r: ") + CurrentPackage->DisplayName + TEXT("  \u00b7  par\u00e7alar: ") + (Draft.Parts.IsEmpty() ? FString(TEXT("Etiket")) : Draft.Parts)
                                  : FString(TEXT("3B yok. Bir ambalaj se\u00e7ince st\u00fcdyo 3B \u015feklini kendisi yapar.")), 9, CurrentPackage ? S.Accent : S.Muted, TEXT("Regular"), true) ];
    }
    const TCHAR* Types[] = { TEXT("kutu"), TEXT("poset"), TEXT("pet_sise"), TEXT("cam_sise"), TEXT("teneke"), TEXT("kavanoz"), TEXT("kase") };
    const TCHAR* TypeNames[] = { TEXT("Kutu"), TEXT("Po\u015fet / paket"), TEXT("PET / plastik \u015fi\u015fe"), TEXT("Cam \u015fi\u015fe"), TEXT("Teneke"), TEXT("Kavanoz"), TEXT("Kase / bardak") };
    TSharedRef<SHorizontalBox> TypeRow1 = SNew(SHorizontalBox);
    TSharedRef<SHorizontalBox> TypeRow2 = SNew(SHorizontalBox);
    for (int32 I = 0; I < 7; ++I)
    {
        const FString Type = Types[I];
        const bool bOn = PresetFilter == Type;
        (I < 3 ? TypeRow1 : TypeRow2)->AddSlot().AutoWidth().Padding(0.f, 0.f, 6.f, 6.f)
        [
            MakeButton(TypeNames[I], bOn ? &S.ChipActive : &S.Chip, FOnClicked::CreateLambda([this, Type]()
            {
                PresetFilter = Type;
                if (bCustomPackage)
                {
                    Draft.PackageType = Type;
                    Draft.Parts = Type == TEXT("cam_sise") || Type == TEXT("pet_sise") || Type == TEXT("kavanoz") ? TEXT("Etiket,Cam,Kapak")
                                : Type == TEXT("teneke") ? TEXT("Etiket,Kapak") : Type == TEXT("kase") ? TEXT("Etiket,Govde,Kapak") : TEXT("Etiket");
                    SelectedPrompt = PromptsFor(Type)[0].Id;
                    RebuildPrompt();
                }
                RebuildRight();
                return FReply::Handled();
            }), bOn ? S.Accent : S.Muted, true, 9)
        ];
    }
    Content->AddSlot().AutoHeight()[ TypeRow1 ];
    Content->AddSlot().AutoHeight()[ TypeRow2 ];
    const FString Suggested = CurrentPreset ? FString() : SuggestPreset(Presets, Draft);
    int32 Shown = 0;
    for (const FPackagePreset& Preset : Presets)
    {
        if (Preset.Type != PresetFilter) continue;
        ++Shown;
        const FString Id = Preset.Id;
        const bool bOn = Id == Draft.Preset;
        Content->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
        [
            SNew(SButton)
            .ButtonStyle(bOn ? &S.CardSelected : &S.Card)
            .ContentPadding(FMargin(10.f, 7.f))
            .ToolTipText(T(Preset.Notes.IsEmpty() ? Preset.Parts : Preset.Notes + TEXT(" \u00b7 ") + Preset.Parts))
            .OnClicked_Lambda([this, Id]() { ChoosePreset(Id); return FReply::Handled(); })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ MakeText(Preset.Name, 10, S.Text, TEXT("Bold"), true) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)[ MakeText(PresetSize(Preset), 9, S.Muted) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                [ MakeText(bOn ? TEXT("se\u00e7ili") : Id == Suggested ? TEXT("\u00f6nerilen") : TEXT(""), 8, bOn ? S.Accent : S.Warn, TEXT("Bold")) ]
            ]
        ];
    }
    if (Shown == 0) Content->AddSlot().AutoHeight()[ MakeText(TEXT("Bu t\u00fcrde haz\u0131r ambalaj yok. Config/ambalajlar.json dosyas\u0131na eklenebilir."), 9, S.Muted, TEXT("Regular"), true) ];
    Content->AddSlot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
    [
        MakeButton(bCustomPackage ? TEXT("\u00d6zel ambalaj\u0131 kapat") : TEXT("Listede yok: \u00f6zel ambalaj"), &S.Chip, FOnClicked::CreateLambda([this]()
        {
            bCustomPackage = !bCustomPackage;
            if (bCustomPackage)
            {
                Draft.Preset.Reset();
                Draft.PackageType = PresetFilter.IsEmpty() ? FString(TEXT("kutu")) : PresetFilter;
                SelectedPrompt = PromptsFor(Draft.PackageType)[0].Id;
            }
            RebuildRight();
            RebuildPrompt();
            return FReply::Handled();
        }), S.Muted, true, 9)
    ];
    if (bCustomPackage)
    {
        const bool bRound = IsRoundPackage(Draft.PackageType);
        const bool bBag = Draft.PackageType == TEXT("poset");
        FString* First = bRound ? &Draft.DiameterMm : &Draft.WidthMm;
        FString* Second = bRound ? &Draft.HeightMm : bBag ? &Draft.HeightMm : &Draft.DepthMm;
        FString* Third = bRound ? &Draft.LabelHeightMm : bBag ? &Draft.DepthMm : &Draft.HeightMm;
        const FString L1 = bRound ? TEXT("\u00c7ap mm") : TEXT("Geni\u015flik mm");
        const FString L2 = bRound ? TEXT("Y\u00fckseklik mm") : bBag ? TEXT("Y\u00fckseklik mm") : TEXT("Derinlik mm");
        const FString L3 = bRound ? TEXT("Etiket band\u0131 mm") : bBag ? TEXT("Dolu kal\u0131nl\u0131k mm") : TEXT("Y\u00fckseklik mm");
        const auto Dim = [this, Changed](const FString& Label, FString* Target)
        {
            return MakeField(Label, *Target, [this, Target, Changed](const FString& Value) { *Target = Value; Changed(); });
        };
        Content->AddSlot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 8.f, 0.f)[ Dim(L1, First) ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 8.f, 0.f)[ Dim(L2, Second) ]
            + SHorizontalBox::Slot().FillWidth(1.f)[ Dim(L3, Third) ]
        ];
        if (Draft.PackageType != TEXT("kutu") && Draft.PackageType != TEXT("poset"))
        {
            Content->AddSlot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
            [ MakeField(TEXT("Par\u00e7alar (model yuvalar\u0131)"), Draft.Parts, [this, Changed](const FString& Value) { Draft.Parts = Value; Changed(); }, TEXT("Etiket,Cam,Kapak")) ];
        }
        Content->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[ MakeBoxCreator() ];
    }
    if (Draft.PackageType == TEXT("kutu") || Draft.PackageType == TEXT("poset"))
    {
        const bool bCap = Draft.Parts.Contains(TEXT("Kapak"));
        Content->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [
            MakeButton(bCap ? TEXT("\u00dcstte kapak var (etiketin \u00fcst y\u00fcz\u00fcne \u00e7izilir)") : TEXT("Kapak yok (varsa t\u0131kla)"), bCap ? &S.ChipActive : &S.Chip,
                FOnClicked::CreateLambda([this]()
                {
                    TArray<FString> Parts;
                    (Draft.Parts.IsEmpty() ? FString(TEXT("Etiket")) : Draft.Parts).ParseIntoArray(Parts, TEXT(","));
                    if (Parts.Contains(TEXT("Kapak"))) Parts.Remove(TEXT("Kapak")); else Parts.Add(TEXT("Kapak"));
                    Draft.Parts = FString::Join(Parts, TEXT(","));
                    RebuildRight();
                    RebuildPrompt();
                    return FReply::Handled();
                }), bCap ? S.Accent : S.Muted, true, 9)
        ];
    }
}

// Right panel, product mode: imported-model transform fixes and part colours.
void SProductStudio::AddModelSettings(const TSharedRef<SVerticalBox>& Content)
{
    const FStudioStyle& S = FStudioStyle::Get();
    const FStudioPackage* CurrentPackage = DraftPackage();
    if (CurrentPackage && CurrentPackage->Id.StartsWith(TEXT("model_")))
    {
        Content->AddSlot().AutoHeight()[ MakeSection(TEXT("\u00d6ZEL MODEL D\u00dcZELTME")) ];
        Content->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
        [ MakeText(TEXT("Yanl\u0131\u015f birim, \u00f6n y\u00f6n veya pivotla gelen FBX/OBJ/GLB modelini burada d\u00fczelt. \u00d6nizleme ve raftaki \u00fcr\u00fcn ayn\u0131 de\u011ferleri kullan\u0131r. Konum birimi cm'dir."), 9, S.Muted, TEXT("Regular"), true) ];
        const auto TransformField = [this](const FString& Label, FString* Target)
        {
            return MakeField(Label, *Target, [this, Target](const FString& Value)
            {
                *Target = Value;
                RebuildIssues();
                UpdatePreview();
            });
        };
        Content->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 6.f, 0.f)[ TransformField(TEXT("\u00d6l\u00e7ek (1 = ayn\u0131)"), &Draft.ModelScale) ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 6.f, 0.f)[ TransformField(TEXT("Pitch \u00b0"), &Draft.ModelPitch) ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 6.f, 0.f)[ TransformField(TEXT("Yaw \u00b0"), &Draft.ModelYaw) ]
            + SHorizontalBox::Slot().FillWidth(1.f)[ TransformField(TEXT("Roll \u00b0"), &Draft.ModelRoll) ]
        ];
        Content->AddSlot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 6.f, 0.f)[ TransformField(TEXT("Pivot/raf X cm"), &Draft.ModelOffsetX) ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 6.f, 0.f)[ TransformField(TEXT("Pivot/raf Y cm"), &Draft.ModelOffsetY) ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 6.f, 0.f)[ TransformField(TEXT("Pivot/raf Z cm"), &Draft.ModelOffsetZ) ]
            + SHorizontalBox::Slot().FillWidth(0.8f).VAlign(VAlign_Bottom)
            [ MakeButton(TEXT("S\u0131f\u0131rla"), &S.Ghost, FOnClicked::CreateLambda([this]()
                {
                    Draft.ModelScale = TEXT("1");
                    Draft.ModelPitch = Draft.ModelYaw = Draft.ModelRoll = TEXT("0");
                    Draft.ModelOffsetX = Draft.ModelOffsetY = Draft.ModelOffsetZ = TEXT("0");
                    RebuildRight(); UpdatePreview(); return FReply::Handled();
                }), S.Muted, true, 9) ]
        ];
    }

    // Part colors for turned shapes / imported models (Cam, Govde, Kapak) ------------------
    if (CurrentPackage && CurrentPackage->Kind == EPackageKind::Model)
    {
        bool bHeader = false;
        for (const FString& SlotName : CurrentPackage->SlotNames)
        {
            const ESlotRole Role = SlotRole(SlotName);
            if (Role != ESlotRole::Glass && Role != ESlotRole::Body && Role != ESlotRole::Cap) continue;
            if (!bHeader)
            {
                Content->AddSlot().AutoHeight()[ MakeSection(TEXT("PAR\u00c7A RENKLER\u0130")) ];
                Content->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
                [ MakeText(TEXT("HEX renk (\u00f6rn. E30613). Cam i\u00e7in saydaml\u0131k: 0,1 = \u00e7ok saydam, 0,9 = i\u00e7i dolu/koyu. Kapak g\u00f6rseli y\u00fckl\u00fcyse kapak rengi yerine o kullan\u0131l\u0131r."), 9, S.Muted, TEXT("Regular"), true) ];
                bHeader = true;
            }
            const FString Slot = SlotName;
            FString Hex = ColorText(Draft.Colors, Slot), Opacity;
            if (Hex.Contains(TEXT("/"))) { FString H; Hex.Split(TEXT("/"), &H, &Opacity); Hex = H; }
            const bool bGlass = Role == ESlotRole::Glass;
            const auto Store = [this, Slot](const FString& NewHex, const FString& NewOpacity)
            {
                FString Value = NewHex.TrimStartAndEnd();
                if (!Value.IsEmpty() && !NewOpacity.TrimStartAndEnd().IsEmpty()) Value += TEXT("/") + NewOpacity.TrimStartAndEnd().Replace(TEXT(","), TEXT("."));
                Draft.Colors = WithColor(Draft.Colors, Slot, Value);
                FLinearColor Unused; float UnusedOpacity;
                if (Value.IsEmpty() || FindColor(Draft.Colors, Slot, Unused, UnusedOpacity)) UpdatePreview();
            };
            TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom).Padding(0.f, 0.f, 8.f, 4.f)
                [
                    SNew(SBox).WidthOverride(26.f).HeightOverride(26.f)
                    [
                        SNew(SBorder).BorderImage(&S.SwatchBrush)
                        .BorderBackgroundColor_Lambda([this, Slot]()
                        {
                            FLinearColor Color; float Opacity;
                            return FSlateColor(FindColor(Draft.Colors, Slot, Color, Opacity) ? Color : FLinearColor(0.2f, 0.2f, 0.2f));
                        })
                    ]
                ]
                + SHorizontalBox::Slot().FillWidth(1.f)
                [ MakeField(Slot + TEXT(" rengi"), Hex, [this, Slot, Store](const FString& Value)
                    {
                        FString OldOpacity, OldHex = ColorText(Draft.Colors, Slot);
                        if (OldHex.Contains(TEXT("/"))) { FString H; OldHex.Split(TEXT("/"), &H, &OldOpacity); }
                        Store(Value, OldOpacity);
                    }, TEXT("varsay\u0131lan")) ];
            if (bGlass)
            {
                Row->AddSlot().FillWidth(0.6f).Padding(8.f, 0.f, 0.f, 0.f)
                [ MakeField(TEXT("Saydaml\u0131k"), Opacity.Replace(TEXT("."), TEXT(",")), [this, Slot, Store](const FString& Value)
                    {
                        FString OldHex = ColorText(Draft.Colors, Slot), H = OldHex, Unused;
                        OldHex.Split(TEXT("/"), &H, &Unused);
                        Store(H.IsEmpty() ? FString(TEXT("D9EEE6")) : H, Value);
                    }, TEXT("0,3")) ];
            }
            Content->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Row ];
        }
    }
}

// Right panel, product mode: notes, external production prompt, special shapes.
void SProductStudio::AddProduction(const TSharedRef<SVerticalBox>& Content)
{
    const FStudioStyle& S = FStudioStyle::Get();
    const auto Changed = [this]() { RebuildIssues(); RebuildPrompt(); };
    Content->AddSlot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
    [ MakeField(TEXT("Notlar (ajana gider: renk, malzeme)"), Draft.Notes, [this, Changed](const FString& Value) { Draft.Notes = Value; Changed(); }, TEXT("\u00f6rn. ye\u015fil cam, k\u0131rm\u0131z\u0131 metal kapak")) ];

    // External production ------------------------------------------------------------------
    Content->AddSlot().AutoHeight()[ MakeSection(TEXT("DI\u015e \u00dcRET\u0130M \u2014 AJANA G\u0130DECEK PROMPT")) ];
    TSharedRef<SHorizontalBox> Eras = SNew(SHorizontalBox);
    for (const TCHAR* Era : { TEXT("2011"), TEXT("2018"), TEXT("2025"), TEXT("2033") })
    {
        const FString Value = Era;
        const bool bOn = SelectedEra == Value;
        Eras->AddSlot().AutoWidth().Padding(0.f, 0.f, 6.f, 6.f)
        [ MakeButton(Value, bOn ? &S.ChipActive : &S.Chip, FOnClicked::CreateLambda([this, Value]() { SelectedEra = Value; RebuildRight(); RebuildPrompt(); return FReply::Handled(); }), bOn ? S.Accent : S.Muted, true, 9) ];
    }
    Content->AddSlot().AutoHeight()[ Eras ];
    TSharedRef<SVerticalBox> Choices = SNew(SVerticalBox);
    for (const FPromptChoice& Choice : PromptsFor(Draft.PackageType))
    {
        const FString Id = Choice.Id;
        const bool bOn = SelectedPrompt == Id;
        Choices->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
        [ MakeButton(Id + TEXT("  \u00b7  ") + Choice.Label, bOn ? &S.ChipActive : &S.Chip, FOnClicked::CreateLambda([this, Id]() { SelectedPrompt = Id; RebuildRight(); RebuildPrompt(); return FReply::Handled(); }), bOn ? S.Accent : S.Muted, true, 9) ];
    }
    Content->AddSlot().AutoHeight()[ Choices ];
    Content->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 6.f, 0.f)
        [ MakeButton(TEXT("\u015eablonlar\u0131 olu\u015ftur"), &S.Secondary, FOnClicked::CreateSP(this, &SProductStudio::OnExportReadyTemplates), S.Text, !Draft.Preset.IsEmpty(), 9) ]
        + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 6.f, 0.f)
        [ MakeButton(TEXT("Promptu kopyala"), &S.Primary, FOnClicked::CreateSP(this, &SProductStudio::OnCopyPrompt), S.AccentText, true, 10) ]
        + SHorizontalBox::Slot().FillWidth(1.f)
        [ MakeButton(TEXT("Teslim klas\u00f6r\u00fcn\u00fc a\u00e7"), &S.Secondary, FOnClicked::CreateSP(this, &SProductStudio::OnOpenDelivery), S.Text, true, 10) ]
    ];
    if (!Draft.Preset.IsEmpty())
        Content->AddSlot().AutoHeight().Padding(0.f, 5.f, 0.f, 0.f)
        [ MakeText(TEXT("\u00d6nce \u015fablonlar\u0131 olu\u015ftur; PNG dosyalar\u0131n\u0131 promptla birlikte ajana y\u00fckle. Ajan ayn\u0131 tuval ve b\u00f6lge \u00f6l\u00e7\u00fclerinde temiz bask\u0131 dosyas\u0131 verir."), 8, S.Muted, TEXT("Regular"), true) ];
    Content->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[ SAssignNew(PromptSlot, SBox) ];

    // Special shapes the library cannot make ------------------------------------------------
    if (!bCustomPackage && IsRoundPackage(Draft.PackageType))
    {
        Content->AddSlot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
        [ MakeButton(TEXT("\u00d6zel 3B model i\u00e7e al\u2026 (yaln\u0131z haz\u0131r \u015fekil yetmezse)"), &S.Ghost, FOnClicked::CreateSP(this, &SProductStudio::OnImportModel), S.Muted, true, 9) ];
    }
}

// Right panel, package mode: package info, products using it, new package.
void SProductStudio::AddPackageInfo(const TSharedRef<SVerticalBox>& Content, const TSharedRef<SVerticalBox>& Footer)
{
    const FStudioStyle& S = FStudioStyle::Get();
    const FStudioPackage* Package = FindPackage(Packages, SelectedPackageId);
    Content->AddSlot().AutoHeight()[ MakeText(TEXT("Ambalaj"), 16, S.Text, TEXT("Light")) ];
    if (Package)
    {
        Content->AddSlot().AutoHeight()[ MakeSection(TEXT("B\u0130LG\u0130")) ];
        Content->AddSlot().AutoHeight()[ MakeText(Package->DisplayName, 12, S.Text, TEXT("Bold")) ];
        Content->AddSlot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
        [ MakeText(Package->Kind == EPackageKind::Box ? TEXT("Kutu \u015fablonu: alt\u0131 y\u00fcz g\u00f6rseli otomatik atlasa dizilir.") : TEXT("\u0130\u00e7e al\u0131nm\u0131\u015f model: etiket modelin kendi UV a\u00e7\u0131l\u0131m\u0131na g\u00f6re haz\u0131rlan\u0131r."), 9, S.Muted, TEXT("Regular"), true) ];
        Content->AddSlot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
        [ MakeText(FString::Printf(TEXT("\u00d6l\u00e7\u00fc: %.1f \u00d7 %.1f \u00d7 %.1f cm (G \u00d7 D \u00d7 Y)"), Package->SizeCm.Y, Package->SizeCm.X, Package->SizeCm.Z), 9, S.Muted) ];
        if (Package->SlotNames.Num() > 0)
            Content->AddSlot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)[ MakeText(TEXT("Yuvalar: ") + FString::Join(Package->SlotNames, TEXT(", ")), 9, S.Muted, TEXT("Regular"), true) ];
        Content->AddSlot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)[ MakeText(TEXT("Dosya: ") + Package->MeshPath, 8, S.Faint, TEXT("Regular"), true) ];
        Content->AddSlot().AutoHeight()[ MakeSection(TEXT("KULLANAN \u00dcR\u00dcNLER")) ];
        int32 Count = 0;
        for (int32 I = 0; I < Catalog.Num(); ++I)
        {
            if (Catalog[I].MeshPath != Package->MeshPath) continue;
            ++Count;
            Content->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[ MakeProductCard(I) ];
        }
        if (Count == 0) Content->AddSlot().AutoHeight()[ MakeText(TEXT("Bu ambalaj\u0131 hen\u00fcz kullanan \u00fcr\u00fcn yok."), 9, S.Muted) ];
    }
    Content->AddSlot().AutoHeight()[ MakeSection(TEXT("YEN\u0130 AMBALAJ")) ];
    Content->AddSlot().AutoHeight()[ MakeBoxCreator() ];
    Footer->AddSlot().AutoHeight()
    [
        SNew(SBox).HeightOverride(44.f)
        [ MakeButton(TEXT("Bu ambalajla yeni \u00fcr\u00fcn"), &S.Primary, FOnClicked::CreateLambda([this]() { NewProduct(SelectedPackageId); return FReply::Handled(); }), S.AccentText, Package != nullptr, 11) ]
    ];
    IssuesSlot.Reset();
    PromptSlot.Reset();
}

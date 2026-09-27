#pragma once
#include "CoreMinimal.h"
#include "StudioBackend.h"
#include "Framework/SlateDelegates.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SCompoundWidget.h"

class SBox;
class SEditableTextBox;
class SStudioViewport;
class UMaterialInstanceDynamic;
class UTexture2D;
struct FButtonStyle;
struct FSlateBrush;

// Product Studio window: pick a package (box template or imported model), drop label
// images on it, type the product data and press "Oyuna ekle". All file work is done by
// MirasStudio::Publish (StudioBackend.h).
class SProductStudio : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SProductStudio) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    enum class EMode : uint8 { Products, Packages };

    // State changes
    void ReloadData();
    void SetMode(EMode NewMode);
    void SelectProduct(int32 Index);
    void NewProduct(const FString& PackageId);
    void SelectPackage(const FString& PackageId);
    void ChoosePackage(const FString& PackageId);
    void ChoosePreset(const FString& PresetId);

    // Actions
    FReply OnActivate();      // save + put in game
    FReply OnSave();          // save to preparation list
    FReply OnDeactivate();    // take out of the game (keeps the product)
    FReply OnCopyPrompt();
    FReply OnOpenDelivery();
    FReply OnExportUvTemplate();
    FReply OnRemove();
    FReply OnCreateBox();
    FReply OnImportModel();
    FReply OnPickFace(int32 Face);
    FReply OnClearFace(int32 Face);
    // Which: 0 = model label, 1 = cap, 2 = body, 3 = box net (single unfolded image)
    FReply OnPickExtra(int32 Which);
    FReply OnClearExtra(int32 Which);
    FString& ExtraFile(int32 Which);
    bool PickFile(const FString& Title, const FString& Types, FString& OutFile);

    // View
    void RebuildAll();
    void RebuildHeader();
    void RebuildLeft();
    void RebuildRight();
    void RebuildFaces();
    void RebuildIssues();
    void RebuildTitle();
    void RebuildPrompt();
    bool CurrentPrompt(FString& OutText, FString& OutError) const;
    void SelectById(const FString& Id);
    void UpdatePreview();
    void Toast(const FString& Text, bool bError = false);

    TSharedRef<SWidget> MakeText(const FString& Text, int32 Size, const FLinearColor& Color, const TCHAR* Weight = TEXT("Regular"), bool bWrap = false) const;
    TSharedRef<SWidget> MakeSection(const FString& Title) const;
    TSharedRef<SWidget> MakeButton(const FString& Label, const FButtonStyle* Style, FOnClicked OnClicked, const FLinearColor& TextColor, bool bEnabled = true, int32 Size = 10) const;
    TSharedRef<SWidget> MakeField(const FString& Label, const FString& Value, TFunction<void(const FString&)> OnChanged, const FString& Hint = FString(), bool bReadOnly = false, TSharedPtr<SEditableTextBox>* OutBox = nullptr) const;
    TSharedRef<SWidget> MakeTile(const FString& Title, const FString& Sub, const FString& File, FOnClicked OnPick, FOnClicked OnClear);
    TSharedRef<SWidget> MakeBoxCreator();
    TSharedRef<SWidget> MakeProductCard(int32 Index) const;
    TSharedRef<SWidget> MakePackageCard(const MirasStudio::FStudioPackage& Package, bool bSelected, FOnClicked OnClicked) const;

    const MirasStudio::FStudioImage* ImageFor(const FString& File, FString* OutError = nullptr);
    const FSlateBrush* ThumbnailFor(const FString& File);
    const MirasStudio::FStudioPackage* DraftPackage() const;
    FString PackageSummary(const FString& MeshPath) const;

    EMode Mode = EMode::Products;
    bool bListActive = true;          // Products mode: "Oyunda" vs "Hazirlik" list
    FString SelectedEra = TEXT("2011");
    FString SelectedPrompt;
    TArray<FMarketProduct> Catalog;
    FString CatalogNote;
    TArray<MirasStudio::FStudioPackage> Packages;
    TArray<MirasStudio::FPackagePreset> Presets;
    FString PresetFilter = TEXT("kutu");  // package type shown in the library
    bool bCustomPackage = false;          // "not in the list": type sizes by hand
    int32 SelectedProduct = INDEX_NONE;
    FString SelectedPackageId;
    MirasStudio::FStudioDraft Draft;
    FString OriginalPackageId;
    TArray<MirasStudio::FStudioIssue> ImageIssues;
    bool bIdTouched = false;
    FString ToastText;
    bool bToastError = false;
    FString BoxWidth = TEXT("70"), BoxDepth = TEXT("50"), BoxHeight = TEXT("200");
    FString LastDirectory;

    TMap<FString, TSharedPtr<MirasStudio::FStudioImage>> Images;
    TMap<FString, TStrongObjectPtr<UTexture2D>> Thumbs;
    TMap<FString, TSharedPtr<FSlateBrush>> ThumbBrushes;
    TArray<TStrongObjectPtr<UTexture2D>> PreviewTextures;
    TArray<TStrongObjectPtr<UMaterialInstanceDynamic>> PreviewMaterials;

    TSharedPtr<SStudioViewport> Viewport;
    TSharedPtr<SBox> HeaderSlot, LeftSlot, RightSlot, FacesSlot, IssuesSlot, TitleSlot, PromptSlot;
    TSharedPtr<SEditableTextBox> IdBox;
};

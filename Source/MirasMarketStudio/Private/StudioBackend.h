#pragma once
#include "CoreMinimal.h"
#include "MarketEconomy.h"
#include "ProductCatalog.h"

class UStaticMesh;
class UTexture2D;
class UMaterialInterface;

// Product Studio back end: everything that touches files and assets lives here, UI-free,
// so it can be driven from Slate today and from tests / commandlets later.
namespace MirasStudio
{
    struct FStudioImage
    {
        int32 Width = 0;
        int32 Height = 0;
        TArray<FColor> Pixels; // BGRA, row-major, top-left origin
        bool IsValid() const { return Width > 0 && Height > 0 && Pixels.Num() == Width * Height; }
    };

    enum class EPackageKind : uint8 { Box, Model };
    // Mesh material slots are recognised by name (Blender material name):
    // Etiket/Label, Kapak/Cap/Lid, Govde/Body, Cam/Glass. Anything else keeps its imported material.
    enum class ESlotRole : uint8 { Label, Cap, Body, Glass, Other };
    ESlotRole SlotRole(const FString& SlotName);

    struct FStudioPackage
    {
        FString Id;          // box_70x50x200 or model_<name>
        EPackageKind Kind = EPackageKind::Model;
        FString MeshPath;    // object path usable by LoadObject
        FString DisplayName;
        int32 WidthMm = 0, DepthMm = 0, HeightMm = 0; // Box only
        FVector SizeCm = FVector::ZeroVector;           // bounds (depth X, width Y, height Z)
        int32 UsedBy = 0;
        TArray<FString> SlotNames;
        int32 LabelSlot = 0;
        int32 CapSlot = INDEX_NONE;
        int32 BodySlot = INDEX_NONE;
    };

    enum class EStudioSeverity : uint8 { Ok, Warning, Error };
    struct FStudioIssue
    {
        EStudioSeverity Severity = EStudioSeverity::Ok;
        FString Text;
    };

    // Everything the user typed or picked. Text stays as typed until publish.
    struct FStudioDraft
    {
        bool bExisting = false;
        bool bActive = false;
        FString Id, RealName, FictionalName, Category, Cost, Price, CaseUnits = TEXT("12");
        // Package description used for external production prompts (kutu, pet_sise, cam_sise, teneke, kavanoz, kase, poset)
        FString Brand, PackageType = TEXT("kutu"), WidthMm, DepthMm, HeightMm, DiameterMm, LabelHeightMm, Parts, Notes;
        bool bSizeEstimated = true;
        FString Preset;                                   // ready package id (Config/ambalajlar.json)
        FString Colors;                                   // per-part colors "Cam=2B1A12/0.85;Kapak=E30613"
        // Imported custom model correction; decimal text accepts comma or dot.
        FString ModelScale = TEXT("1"), ModelPitch = TEXT("0"), ModelYaw = TEXT("0"), ModelRoll = TEXT("0");
        FString ModelOffsetX = TEXT("0"), ModelOffsetY = TEXT("0"), ModelOffsetZ = TEXT("0");
        FString PackageId;
        FString NetFile;                                  // box: one image with the whole unfolded box (cross net)
        FString FaceFiles[FBoxPackageLayout::FaceCount]; // box: single faces (override the net face)
        FString LabelFile;                                // model: label slot, UV 0-1
        FString CapFile;                                  // model: optional cap/lid texture
        FString BodyFile;                                 // model: optional opaque body texture
        TArray<FString> ExistingMaterials;                // per slot, from the catalog
    };

    // Paths --------------------------------------------------------------------------------
    FString InboxDir();                                // <Project>/AssetInbox
    FString PackageFolder(const FString& PackageId);   // /Game/Products/Packages/<id>
    FString ItemFolder(const FString& ProductId);      // /Game/Products/Items/<id>
    FString LabelMaterialPath();                       // master material object path

    // Images -------------------------------------------------------------------------------
    bool LoadImageFile(const FString& Path, FStudioImage& Out, FString& OutError);
    void Resize(const FStudioImage& In, int32 Width, int32 Height, FStudioImage& Out);
    FColor AverageBorder(const FStudioImage& Image);
    // Six face images (nullptr = missing) -> label atlas that matches the box mesh UVs.
    // Cuts a cross-shaped net into faces. Layout (mm): top row [ - | top | - | - ],
    // middle row [ left | front | right | back ], bottom row [ - | bottom | - | - ];
    // columns D, W, D, W wide; rows D, H, D high. Scale-independent; warns if aspect is off.
    // If "<net name>.json" or "acilim.json" sits next to the net image, its pixel rectangles
    // ({"front":[x0,y0,x1,y1], \u2026 , "inset":4}) are used instead of proportions. This rescues
    // annotated dielines from image generators (labels, dimension lines, gaps between panels).
    void SplitNet(const FString& NetFile, const FStudioImage& Net, int32 WidthMm, int32 DepthMm, int32 HeightMm, FStudioImage (&OutFaces)[FBoxPackageLayout::FaceCount], TArray<FStudioIssue>* OutIssues);
    FString NetRectsFile(const FString& NetFile); // empty if none
    void ComposeAtlas(const FBoxPackageLayout& Layout, const FStudioImage* const Faces[FBoxPackageLayout::FaceCount], FStudioImage& OutAtlas, TArray<FStudioIssue>* OutIssues);
    UTexture2D* MakeTransientTexture(const FStudioImage& Image);

    // Assets -------------------------------------------------------------------------------
    UMaterialInterface* EnsureLabelMaterial(FString& OutError);   // opaque, texture "Label"
    UMaterialInterface* EnsureSolidMaterial(FString& OutError);   // opaque, "Color" "Roughness" "Metallic"
    UMaterialInterface* EnsureGlassMaterial(FString& OutError);   // translucent, "Color" "Opacity" "Roughness"
    UStaticMesh* CreateBoxPackage(int32 WidthMm, int32 DepthMm, int32 HeightMm, FString& OutPackageId, FString& OutError);
    UStaticMesh* ImportModelPackage(const FString& File, FString& OutPackageId, FString& OutError);
    // Draws the UV0 edges of the package's Etiket/Label material slot to a 2048px PNG.
    bool ExportUvTemplate(const FStudioPackage& Package, const FString& OutputFile, FString& OutError);
    TArray<FStudioPackage> ScanPackages(const TArray<FMarketProduct>& Catalog);
    const FStudioPackage* FindPackage(const TArray<FStudioPackage>& Packages, const FString& Id);
    FString PackageIdForMesh(const TArray<FStudioPackage>& Packages, const FString& MeshPath);

    // Ready package library (Config/ambalajlar.json) ----------------------------------------
    // A preset is a finished package: the studio builds its 3D shape itself (box for kutu/poset,
    // a turned shape with Etiket/Cam|Govde/Kapak parts for bottles, cans, jars and cups).
    struct FPackagePreset
    {
        FString Id, Name, Type, Shape, Parts, Colors, Notes;
        int32 WidthMm = 0, DepthMm = 0, HeightMm = 0;          // kutu / poset
        int32 DiameterMm = 0, LabelHeightMm = 0;               // round
        int32 LabelBottomMm = -1, NeckDiameterMm = 0, CapHeightMm = 0;
        bool bTopCap = false;   // kutu: screw cap on top, drawn on the top face of the label
        bool bMetalCap = false; // round: Kapak part is metal (cans, crown caps)
        bool IsBox() const { return Type == TEXT("kutu") || Type == TEXT("poset"); }
    };
    FString PresetsPath();
    TArray<FPackagePreset> LoadPresets(TArray<FString>* OutErrors = nullptr);
    const FPackagePreset* FindPreset(const TArray<FPackagePreset>& Presets, const FString& Id);
    FString PresetPackageId(const FPackagePreset& Preset);   // box_WxDxH or shape_<id>
    // Turned (lathe) package for round presets: slots Etiket (UV 0-1 band, u=0.5 faces front),
    // Cam or Govde, Kapak (top-view UV so a square kapak.png maps onto the cap).
    UStaticMesh* CreateShapePackage(const FPackagePreset& Preset, FString& OutPackageId, FString& OutError);
    // Creates the preset's 3D package (once) and returns its mesh.
    UStaticMesh* EnsurePresetPackage(const FPackagePreset& Preset, FString& OutPackageId, FString& OutError);
    // Copies type, sizes, parts and default colors into the draft (keeps colors the product already set).
    void ApplyPreset(const FPackagePreset& Preset, FStudioDraft& InOutDraft);
    // Closest preset of the draft's package type (exact size match first). Empty if none.
    FString SuggestPreset(const TArray<FPackagePreset>& Presets, const FStudioDraft& Draft);
    // "Cam=2B1A12/0.85;Kapak=E30613" helpers. Keys match slot names case-insensitively.
    bool FindColor(const FString& Colors, const FString& Slot, FLinearColor& OutColor, float& OutOpacity);
    FString ColorText(const FString& Colors, const FString& Slot);                  // "2B1A12/0.85" or ""
    FString WithColor(const FString& Colors, const FString& Slot, const FString& Value); // Value "" removes

    // Products -----------------------------------------------------------------------------
    // bForGame: also require what the game needs (package + label, in-game limit).
    TArray<FStudioIssue> Validate(const FStudioDraft& Draft, const TArray<FMarketProduct>& Catalog, const TArray<FStudioPackage>& Packages, bool bForGame = true);
    bool HasErrors(const TArray<FStudioIssue>& Issues);
    // bActivate: put the product in the game (active). Otherwise it is saved to the preparation list.
    bool Publish(const FStudioDraft& Draft, const TArray<FStudioPackage>& Packages, TArray<FMarketProduct>& InOutCatalog, const FString& Note, bool bActivate, FString& OutMessage);
    bool SetActive(const FString& Id, bool bActive, TArray<FMarketProduct>& InOutCatalog, const FString& Note, FString& OutMessage);
    void FillDraft(const FMarketProduct& Product, FStudioDraft& OutDraft);

    // External production prompts ------------------------------------------------------------
    struct FPromptChoice { FString Id; FString Label; bool bNeedsEra = true; };
    TArray<FPromptChoice> PromptsFor(const FString& PackageType);
    FString PackageTypeLabel(const FString& PackageType);
    bool IsRoundPackage(const FString& PackageType);
    // Creates exact-size guide PNGs for ready packages in Uretim/<product>/<era>/.
    // Box/bag: acilim_sablonu.png. Round packages: label_sablonu.png and, when used, kapak_sablonu.png.
    bool ExportReadyPackageTemplates(const FStudioDraft& Draft, const FString& Era, TArray<FString>& OutFiles, FString& OutError);
    // Fills Docs/Uretim/Sablonlar/<TemplateId>.txt with the draft's data and computed pixel sizes.
    bool BuildPrompt(const FString& TemplateId, const FStudioDraft& Draft, const FString& Era, FString& OutText, FString& OutError);
    FString DeliveryFolder(const FString& ProductId, const FString& Era); // absolute
    FString ModelFolder(const FString& ProductId);                        // absolute
    bool RemoveProduct(const FString& Id, TArray<FMarketProduct>& InOutCatalog, const FString& Note, FString& OutMessage);
    // Fills face/label files from AssetInbox/Products/<id>/manifest.json when present.
    void LoadDraftSources(const FString& ProductId, FStudioDraft& InOutDraft);
}

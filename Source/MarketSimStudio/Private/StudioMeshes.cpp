// Product Studio: master materials, generated box / turned packages, imported models, UV guides, package scan.
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

namespace SimStudio
{
using namespace Internal;

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

    // Shared tail of the box and turned-shape builders: a static mesh asset with lightmap UVs and one box collision
    // (BoxSize in cm, centred at BoxCenterZ). The caller saves it.
    UStaticMesh* BuildMeshAsset(const FString& Folder, const FString& Name, FMeshDescription&& Mesh, bool bRemoveDegenerates,
        const TArray<FStaticMaterial>& Materials, const FVector& BoxSize, double BoxCenterZ)
    {
        UPackage* Package = OpenPackage(Folder, Name);
        UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, *Name, RF_Public | RF_Standalone | RF_Transactional);
        StaticMesh->InitResources();
        StaticMesh->SetLightingGuid();
        FStaticMeshSourceModel& Source = StaticMesh->AddSourceModel();
        Source.BuildSettings.bRecomputeNormals = false;
        Source.BuildSettings.bRecomputeTangents = true;
        Source.BuildSettings.bRemoveDegenerates = bRemoveDegenerates;
        Source.BuildSettings.bGenerateLightmapUVs = true;
        Source.BuildSettings.SrcLightmapIndex = 0;
        Source.BuildSettings.DstLightmapIndex = 1;
        StaticMesh->CreateMeshDescription(0, MoveTemp(Mesh));
        StaticMesh->CommitMeshDescription(0);
        StaticMesh->SetLightMapCoordinateIndex(1);
        StaticMesh->GetStaticMaterials().Append(Materials);
        StaticMesh->CreateBodySetup();
        if (UBodySetup* Body = StaticMesh->GetBodySetup())
        {
            FKBoxElem Box(BoxSize.X, BoxSize.Y, BoxSize.Z);
            Box.Center = FVector(0, 0, BoxCenterZ);
            Body->AggGeom.BoxElems.Add(Box);
        }
        StaticMesh->Build(false);
        StaticMesh->PostEditChange();
        Package->MarkPackageDirty();
        FAssetRegistryModule::AssetCreated(StaticMesh);
        return StaticMesh;
    }

    // <AssetInbox>/Packages/<id>/package.json
    void WritePackageMeta(const FString& PackageId, const TSharedRef<FJsonObject>& Meta)
    {
        const FString PackageDir = InboxDir() / TEXT("Packages") / PackageId;
        IFileManager::Get().MakeDirectory(*PackageDir, true);
        WriteJson(PackageDir / TEXT("package.json"), Meta);
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

    UStaticMesh* StaticMesh = BuildMeshAsset(Folder, Name, MoveTemp(Mesh), false,
        { FStaticMaterial(Label, FName(TEXT("Label")), FName(TEXT("Label"))) }, FVector(D / 10.f, W / 10.f, H / 10.f), H / 20.0);
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
    WritePackageMeta(OutPackageId, Meta);
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
    WritePackageMeta(OutPackageId, Meta);
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

    TArray<FStaticMaterial> Materials;
    for (int32 I = 0; I < MaterialNames.Num(); ++I)
        Materials.Add(FStaticMaterial(SlotMaterials[I], MaterialNames[I], MaterialNames[I]));
    const double BoxR = Preset.DiameterMm / 20.0, BoxH = Preset.HeightMm / 10.0;
    UStaticMesh* StaticMesh = BuildMeshAsset(Folder, Name, MoveTemp(Mesh), true, Materials, FVector(BoxR * 2, BoxR * 2, BoxH), BoxH / 2);
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
    WritePackageMeta(OutPackageId, Meta);
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
}

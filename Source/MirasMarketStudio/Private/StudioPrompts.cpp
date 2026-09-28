// Product Studio: external production prompts, delivery folders, ready package templates.
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

#include "SPlanogramStudio.h"

#include "AssetThumbnail.h"
#include "AssetRegistry/AssetData.h"
#include "ProductCatalog.h"
#include "PlanogramEdit.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SCanvas.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
    FSlateColor Ink() { return FSlateColor(FLinearColor(.88f, .90f, .87f)); }
    FSlateColor Muted() { return FSlateColor(FLinearColor(.55f, .61f, .58f)); }
}

void SPlanogramStudio::Construct(const FArguments& InArgs)
{
    ThumbnailPool = MakeShared<FAssetThumbnailPool>(64);
    ChildSlot
    [
        SNew(SBorder).Padding(18).BorderBackgroundColor(FLinearColor(.035f, .045f, .043f))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
            [ SNew(STextBlock).Text(FText::FromString(TEXT("RAF PLANI EDIT\u00d6R\u00dc"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 22)).ColorAndOpacity(Ink()) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
            [ SNew(STextBlock).Text(FText::FromString(TEXT("Bir gondolda farkl\u0131 markalar\u0131 yan yana yerle\u015ftir; \u00f6ne bakan adet ve arka derinli\u011fi ayr\u0131 ayarla."))).ColorAndOpacity(Muted()) ]
            + SVerticalBox::Slot().FillHeight(1)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(.28f).Padding(0, 0, 12, 0)
                [ SNew(SScrollBox) + SScrollBox::Slot()[SAssignNew(FixtureBox, SVerticalBox)] ]
                + SHorizontalBox::Slot().FillWidth(.72f)
                [ SNew(SScrollBox) + SScrollBox::Slot()[SAssignNew(ProductBox, SVerticalBox)] ]
            ]
        ]
    ];
    Reload();
}

void SPlanogramStudio::Reload()
{
    TArray<FString> Errors;
    FString Note;
    MarketCatalog::LoadFile(MarketCatalog::DefaultPath(), Products, Errors, &Note);
    Products.RemoveAll([](const FMarketProduct& Product) { return !Product.bActive; });
    MarketPlanogram::LoadFile(MarketPlanogram::DefaultPath(), Planogram, Errors);
    // No automatic placement: catalog products without a block are listed as "Rafta degil".
    MarketPlanogram::ResolvePositions(Planogram, Products); // old files: keep what the old packing showed
    MarketPlanogram::FitDepth(Planogram, Products);
    SelectedFixture = FMath::Clamp(SelectedFixture, 0, FMath::Max(0, Planogram.Fixtures.Num() - 1));
    Status = Errors.Num() ? Errors[0] : TEXT("Planogram y\u00fcklendi.");
    Rebuild();
}

void SPlanogramStudio::Save(const FString& Message)
{
    FString Error;
    MarketPlanogram::FitDepth(Planogram, Products);
    Status = MarketPlanogram::SaveFile(MarketPlanogram::DefaultPath(), Planogram, Error) ? Message : Error;
    Rebuild();
}

void SPlanogramStudio::SelectFixture(int32 Index) { SelectedFixture = Index; Rebuild(); }

void SPlanogramStudio::Run(bool bOk, const FString& Message)
{
    // Every edit goes through MarketPlanogramEdit (same rules as the in-game R mode): a rejected edit
    // leaves the plan unchanged and only shows why.
    if (bOk) Save(Message);
    else { Status = Message; Rebuild(); }
}

void SPlanogramStudio::AddFixture()
{
    FPlanogramFixture F;
    F.Id = FString::Printf(TEXT("gondola_%d"), Planogram.Fixtures.Num() + 1);
    while (Planogram.FindFixture(F.Id)) F.Id += TEXT("_yeni");
    F.Label = TEXT("Yeni Gondol"); F.Location = FVector(0, 320.f + Planogram.Fixtures.Num() * 160.f, 0);
    Planogram.Fixtures.Add(F); SelectedFixture = Planogram.Fixtures.Num() - 1;
    Save(TEXT("Yeni gondol eklendi."));
}

FString SPlanogramStudio::PlacementSummary(const FPlanogramPlacement& P) const
{
    const TCHAR* Orientation = P.Orientation == 1 ? TEXT("yana donuk") : (P.Orientation == 2 ? TEXT("yan yatmis") : TEXT("dik"));
    return FString::Printf(TEXT("Seviye %d | Onde %d | Derinlik %d | %d kat = %d adet | %s | konum %+.0f cm | aralik %.0f cm | %s"),
        P.Level + 1, P.Facings, P.Depth, P.Stack, MarketPlanogram::Capacity(P), Orientation, MarketPlanogram::PlacementCenterX(Planogram, Products, P), P.GapCm,
        P.Face == TEXT("back") ? TEXT("arka yuz") : TEXT("on yuz"));
}

TSharedRef<SWidget> SPlanogramStudio::BuildShelfPreview(const FPlanogramFixture& Fixture)
{
    const FPlanogramEquipment Spec = MarketPlanogram::Equipment(Fixture.EquipmentId);
    TSharedRef<SVerticalBox> Preview = SNew(SVerticalBox);
    TArray<FString> Faces = { TEXT("front") };
    if (Spec.bDoubleSided) Faces.Add(TEXT("back"));
    constexpr float CanvasWidth = 740.f;
    constexpr float InnerWidth = 710.f;
    constexpr float CanvasHeight = 58.f;
    const float Scale = InnerWidth / Spec.UsableWidthCm;
    for (const FString& Face : Faces)
    {
        Preview->AddSlot().AutoHeight().Padding(0, 7, 0, 3)
        [ SNew(STextBlock).Text(FText::FromString(Face == TEXT("back") ? TEXT("ARKA YUZ RAF ONIZLEMESI") : TEXT("ON YUZ RAF ONIZLEMESI"))).ColorAndOpacity(Muted()) ];
        for (int32 Level = Spec.Levels - 1; Level >= 0; --Level)
        {
            TSharedRef<SCanvas> Canvas = SNew(SCanvas);
            Canvas->AddSlot().Position(FVector2D(15.f, 5.f)).Size(FVector2D(InnerWidth, 48.f))
            [ SNew(SBorder).Padding(0).BorderBackgroundColor(FLinearColor(.075f, .085f, .082f)) ];
            for (const FPlanogramPlacement& Placement : Planogram.Placements)
            {
                if (Placement.FixtureId != Fixture.Id || Placement.Face != Face || Placement.Level != Level) continue;
                const FMarketProduct* Product = MarketCatalog::FindProduct(Products, Placement.ProductId);
                if (!Product) continue;
                const float Span = MarketPlanogram::BlockWidthCm(*Product, Placement);
                const float Center = MarketPlanogram::PlacementCenterX(Planogram, Products, Placement);
                const float X = 15.f + (Center + Spec.UsableWidthCm * .5f - Span * .5f) * Scale;
                const float W = FMath::Max(26.f, Span * Scale);
                const float H = FMath::Min(44.f, 23.f + (Placement.Stack - 1) * 7.f);
                const FString Marker = Placement.Orientation == 1 ? TEXT(" >") : (Placement.Orientation == 2 ? TEXT(" =") : TEXT(""));
                Canvas->AddSlot().Position(FVector2D(X, 50.f - H)).Size(FVector2D(W, H))
                [ SNew(SBorder).Padding(FMargin(4,2)).BorderBackgroundColor(FLinearColor(Product->Color).CopyWithNewOpacity(.82f))
                  [ SNew(STextBlock).Text(FText::FromString((Product->Brand.IsEmpty() ? Product->RealName : Product->Brand) + Marker)).ColorAndOpacity(FLinearColor::White) ] ];
            }
            Preview->AddSlot().AutoHeight()
            [ SNew(SHorizontalBox)
              + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,7,0)[SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("S%d"), Level + 1))).ColorAndOpacity(Muted())]
              + SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(CanvasWidth).HeightOverride(CanvasHeight)[Canvas]] ];
        }
    }
    return Preview;
}

TSharedRef<SWidget> SPlanogramStudio::BuildProductThumbnail(const FMarketProduct& Product)
{
    if (ThumbnailPool && !Product.MeshPath.IsEmpty())
        if (UObject* Asset = LoadObject<UObject>(nullptr, *Product.MeshPath))
        {
            TSharedPtr<FAssetThumbnail> Thumbnail = MakeShared<FAssetThumbnail>(FAssetData(Asset), 72, 72, ThumbnailPool);
            Thumbnails.Add(Thumbnail);
            return SNew(SBox).WidthOverride(72).HeightOverride(72)[Thumbnail->MakeThumbnailWidget()];
        }
    return SNew(SBox).WidthOverride(72).HeightOverride(72)
    [ SNew(SBorder).BorderBackgroundColor(FLinearColor(Product.Color)).HAlign(HAlign_Center).VAlign(VAlign_Center)
      [ SNew(STextBlock).Text(FText::FromString(Product.Brand.Left(1))).ColorAndOpacity(FLinearColor::White) ] ];
}

void SPlanogramStudio::Rebuild()
{
    if (!FixtureBox || !ProductBox) return;
    Thumbnails.Reset();
    FixtureBox->ClearChildren(); ProductBox->ClearChildren();
    FixtureBox->AddSlot().AutoHeight().Padding(0, 0, 0, 8)
    [ SNew(SButton).Text(FText::FromString(TEXT("+ Yeni gondol"))).OnClicked_Lambda([this]{ AddFixture(); return FReply::Handled(); }) ];
    for (int32 I = 0; I < Planogram.Fixtures.Num(); ++I)
    {
        const FPlanogramFixture& F = Planogram.Fixtures[I];
        FixtureBox->AddSlot().AutoHeight().Padding(0, 2)
        [ SNew(SButton).ButtonColorAndOpacity(I == SelectedFixture ? FLinearColor(.18f, .42f, .34f) : FLinearColor(.10f, .12f, .12f))
          .Text(FText::FromString(F.Label + TEXT("\n") + F.Id)).OnClicked_Lambda([this, I]{ SelectFixture(I); return FReply::Handled(); }) ];
    }
    if (!Planogram.Fixtures.IsValidIndex(SelectedFixture)) return;
    const FPlanogramFixture& F = Planogram.Fixtures[SelectedFixture];
    ProductBox->AddSlot().AutoHeight().Padding(0, 0, 0, 8)
    [ SNew(STextBlock).Text(FText::FromString(F.Label + TEXT(" \u2014 sat\u0131\u015f yerle\u015fimi"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 17)).ColorAndOpacity(Ink()) ];
    // Result of the last action right under the title (the product list below can be long).
    ProductBox->AddSlot().AutoHeight().Padding(0, 0, 0, 8)
    [ SNew(STextBlock).AutoWrapText(true).Text(FText::FromString(Status)).ColorAndOpacity(FSlateColor(FLinearColor(.95f, .80f, .45f))) ];
    ProductBox->AddSlot().AutoHeight().Padding(0, 0, 0, 10)[BuildShelfPreview(F)];
    {
        // Width usage per level; the game places blocks centered within UsableWidthCm.
        TArray<FString> Faces = { TEXT("front") };
        if (MarketPlanogram::Equipment(F.EquipmentId).bDoubleSided) Faces.Add(TEXT("back"));
        for (const FString& Face : Faces)
        {
            FString Line = Face == TEXT("back") ? TEXT("Arka y\u00fcz doluluk:") : TEXT("\u00d6n y\u00fcz doluluk:");
            const FPlanogramEquipment Spec = MarketPlanogram::Equipment(F.EquipmentId);
            for (int32 Level = 0; Level < Spec.Levels; ++Level)
                Line += FString::Printf(TEXT("   S%d %.0f/%.0f cm"), Level + 1,
                    MarketPlanogram::LevelUsedWidthCm(Planogram, Products, F.Id, Face, Level), Spec.UsableWidthCm);
            ProductBox->AddSlot().AutoHeight().Padding(0, 0, 0, 4)[SNew(STextBlock).Text(FText::FromString(Line)).ColorAndOpacity(Muted())];
        }
        TArray<FString> Overflows;
        MarketPlanogram::FindOverflows(Planogram, Products, Overflows);
        for (const FString& Warning : Overflows)
            ProductBox->AddSlot().AutoHeight().Padding(0, 0, 0, 4)
            [ SNew(STextBlock).Text(FText::FromString(TEXT("Ta\u015fma: ") + Warning)).ColorAndOpacity(FSlateColor(FLinearColor(.95f, .55f, .35f))) ];
        ProductBox->AddSlot().AutoHeight().Padding(0, 0, 0, 8)
        [ SNew(STextBlock).AutoWrapText(true).ColorAndOpacity(Muted()).Text(FText::FromString(TEXT("Oyunda raflar tam burada dizdi\u011fin gibi g\u00f6r\u00fcn\u00fcr; bo\u015f b\u0131rakt\u0131\u011f\u0131n yer bo\u015f kal\u0131r. Rafta olmayan \u00fcr\u00fcn sat\u0131lmaz. Oyun i\u00e7inde de reyonun \u00f6n\u00fcnde R ile dizebilirsin (market kapal\u0131yken)."))) ];
        ProductBox->AddSlot().AutoHeight().Padding(0, 0, 0, 8)[SNew(SSeparator)];
    }
    // 1) Every block on this fixture, row by row (left to right). 2) Add any product (again and again).
    const FPlanogramEquipment FixtureSpec = MarketPlanogram::Equipment(F.EquipmentId);
    TArray<FString> AllFaces = { TEXT("front") };
    if (FixtureSpec.bDoubleSided) AllFaces.Add(TEXT("back"));
    ProductBox->AddSlot().AutoHeight().Padding(0, 4, 0, 6)
    [ SNew(STextBlock).Text(FText::FromString(TEXT("BU REYONDAK\u0130 \u00dcR\u00dcN BLOKLARI"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)).ColorAndOpacity(Ink()) ];
    int32 Shown = 0;
    for (const FString& Face : AllFaces)
        for (int32 Level = FixtureSpec.Levels - 1; Level >= 0; --Level)
        {
            const TArray<int32> Row = MarketPlanogram::RowBlocks(Planogram, Products, F.Id, Face, Level);
            if (Row.Num() == 0) continue;
            ProductBox->AddSlot().AutoHeight().Padding(0, 8, 0, 2)
            [ SNew(STextBlock).ColorAndOpacity(Muted()).Text(FText::FromString(FString::Printf(TEXT("%s y\u00fcz  \u00b7  seviye %d  \u00b7  %.0f / %.0f cm dolu  \u00b7  en geni\u015f bo\u015fluk %.0f cm"),
                Face == TEXT("back") ? TEXT("Arka") : TEXT("\u00d6n"), Level + 1, MarketPlanogram::LevelUsedWidthCm(Planogram, Products, F.Id, Face, Level),
                FixtureSpec.UsableWidthCm, MarketPlanogram::WidestGapCm(Planogram, Products, F.Id, Face, Level)))) ];
            for (const int32 Index : Row) { ProductBox->AddSlot().AutoHeight().Padding(0, 2)[BuildBlockRow(Index, F)]; ++Shown; }
        }
    if (Shown == 0)
        ProductBox->AddSlot().AutoHeight().Padding(0, 0, 0, 6)
        [ SNew(STextBlock).ColorAndOpacity(Muted()).Text(FText::FromString(TEXT("Bu reyon bo\u015f. A\u015fa\u011f\u0131dan \u00fcr\u00fcn ekle."))) ];

    ProductBox->AddSlot().AutoHeight().Padding(0, 16, 0, 6)[SNew(SSeparator)];
    ProductBox->AddSlot().AutoHeight().Padding(0, 0, 0, 2)
    [ SNew(STextBlock).Text(FText::FromString(TEXT("\u00dcR\u00dcN EKLE"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)).ColorAndOpacity(Ink()) ];
    ProductBox->AddSlot().AutoHeight().Padding(0, 0, 0, 6)
    [ SNew(STextBlock).AutoWrapText(true).ColorAndOpacity(Muted()).Text(FText::FromString(TEXT("S d\u00fc\u011fmesi \u00fcr\u00fcn\u00fcn yeni bir blo\u011funu o seviyenin sa\u011f ucundaki bo\u015flu\u011fa koyar. Ayn\u0131 \u00fcr\u00fcn\u00fc istedi\u011fin kadar ekleyebilirsin; yer yetmezse \u00f6nde adet azalt\u0131l\u0131r."))) ];
    TArray<int32> Order;
    for (int32 I = 0; I < Products.Num(); ++I) if (!Planogram.FindPlacement(Products[I].Id)) Order.Add(I); // not on a shelf first
    for (int32 I = 0; I < Products.Num(); ++I) Order.AddUnique(I);
    for (const int32 I : Order) ProductBox->AddSlot().AutoHeight().Padding(0, 2)[BuildAddRow(I, F)];
}

TSharedRef<SWidget> SPlanogramStudio::BuildBlockRow(int32 BlockIndex, const FPlanogramFixture& Fixture)
{
    const FPlanogramPlacement& Block = Planogram.Placements[BlockIndex];
    const FMarketProduct* Product = MarketCatalog::FindProduct(Products, Block.ProductId);
    const FPlanogramEquipment Spec = MarketPlanogram::Equipment(Fixture.EquipmentId);
    const FString FixtureId = Fixture.Id, Face = Block.Face;
    const int32 CurrentLevel = Block.Level;
    const float X = MarketPlanogram::PlacementCenterX(Planogram, Products, Block);

    using FEdit = TFunction<bool(FMarketPlanogram&, const TArray<FMarketProduct>&, FString&)>;
    auto Button = [this](const FString& Label, FEdit Edit, bool bEnabled) -> TSharedRef<SWidget>
    {
        return SNew(SButton).Text(FText::FromString(Label)).IsEnabled(bEnabled)
            .OnClicked_Lambda([this, Edit] { FString Message; const bool bOk = Edit(Planogram, Products, Message); Run(bOk, Message); return FReply::Handled(); });
    };
    const int32 I = BlockIndex;
    TSharedRef<SHorizontalBox> Levels = SNew(SHorizontalBox);
    Levels->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 6, 0)[SNew(STextBlock).Text(FText::FromString(TEXT("Seviyeye ta\u015f\u0131:"))).ColorAndOpacity(Muted())];
    for (int32 Level = 0; Level < Spec.Levels; ++Level)
        Levels->AddSlot().AutoWidth().Padding(0, 0, 3, 0)
        [ Button(FString::Printf(TEXT("S%d"), Level + 1), [I, FixtureId, Face, Level, X](FMarketPlanogram& Pl, const TArray<FMarketProduct>& Pr, FString& M)
            { return MarketPlanogramEdit::MoveBlock(Pl, Pr, I, FixtureId, Face, Level, X, 1.0e6f, M); }, Level != CurrentLevel) ];

    TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
    Box->AddSlot().AutoHeight()
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)[Product ? BuildProductThumbnail(*Product) : SNullWidget::NullWidget]
        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(Product ? Product->RealName + TEXT("  \u2022  ") + Product->Brand : Block.ProductId)).ColorAndOpacity(Ink())]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 3)[SNew(STextBlock).AutoWrapText(true).Text(FText::FromString(PlacementSummary(Block))).ColorAndOpacity(Muted())]
        ]
    ];
    Box->AddSlot().AutoHeight().Padding(0, 4, 0, 0)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 3, 0)[Button(TEXT("\u2190 5 cm"), [I](FMarketPlanogram& Pl, const TArray<FMarketProduct>& Pr, FString& M) { return MarketPlanogramEdit::Nudge(Pl, Pr, I, -5.f, M); }, true)]
        + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)[Button(TEXT("5 cm \u2192"), [I](FMarketPlanogram& Pl, const TArray<FMarketProduct>& Pr, FString& M) { return MarketPlanogramEdit::Nudge(Pl, Pr, I, 5.f, M); }, true)]
        + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 3, 0)[Button(TEXT("\u00d6nde \u2212"), [I](FMarketPlanogram& Pl, const TArray<FMarketProduct>& Pr, FString& M) { return MarketPlanogramEdit::ChangeFacings(Pl, Pr, I, -1, M); }, true)]
        + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)[Button(TEXT("\u00d6nde +"), [I](FMarketPlanogram& Pl, const TArray<FMarketProduct>& Pr, FString& M) { return MarketPlanogramEdit::ChangeFacings(Pl, Pr, I, 1, M); }, true)]
        + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)[Button(TEXT("Y\u00f6n\u00fc de\u011fi\u015ftir"), [I](FMarketPlanogram& Pl, const TArray<FMarketProduct>& Pr, FString& M) { return MarketPlanogramEdit::CycleOrientation(Pl, Pr, I, M); }, true)]
        + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 3, 0)[Button(TEXT("Kat \u2212"), [I](FMarketPlanogram& Pl, const TArray<FMarketProduct>& Pr, FString& M) { return MarketPlanogramEdit::ChangeStack(Pl, Pr, I, -1, false, M); }, true)]
        + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)[Button(TEXT("Kat +"), [I](FMarketPlanogram& Pl, const TArray<FMarketProduct>& Pr, FString& M) { return MarketPlanogramEdit::ChangeStack(Pl, Pr, I, 1, false, M); }, true)]
        + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 3, 0)[Button(TEXT("Aral\u0131k \u2212"), [I](FMarketPlanogram& Pl, const TArray<FMarketProduct>& Pr, FString& M) { return MarketPlanogramEdit::ChangeGap(Pl, Pr, I, -1.f, M); }, true)]
        + SHorizontalBox::Slot().AutoWidth()[Button(TEXT("Aral\u0131k +"), [I](FMarketPlanogram& Pl, const TArray<FMarketProduct>& Pr, FString& M) { return MarketPlanogramEdit::ChangeGap(Pl, Pr, I, 1.f, M); }, true)]
    ];
    Box->AddSlot().AutoHeight().Padding(0, 4, 0, 0)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth()[Levels]
        + SHorizontalBox::Slot().AutoWidth().Padding(8, 0, 3, 0)[Button(TEXT("Y\u00fcz\u00fc \u00e7evir"), [I](FMarketPlanogram& Pl, const TArray<FMarketProduct>& Pr, FString& M) { return MarketPlanogramEdit::ToggleFace(Pl, Pr, I, M); }, Spec.bDoubleSided)]
        + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 3, 0)[Button(TEXT("Ayn\u0131s\u0131ndan ekle"), [I](FMarketPlanogram& Pl, const TArray<FMarketProduct>& Pr, FString& M)
            {
                const FPlanogramPlacement Copy = Pl.Placements[I];
                return MarketPlanogramEdit::AddToRowEnd(Pl, Pr, Copy.ProductId, Copy.FixtureId, Copy.Face, Copy.Level, M);
            }, true)]
        + SHorizontalBox::Slot().AutoWidth()[Button(TEXT("Kald\u0131r"), [I](FMarketPlanogram& Pl, const TArray<FMarketProduct>& Pr, FString& M) { return MarketPlanogramEdit::RemoveBlock(Pl, Pr, I, M); }, true)]
    ];
    return SNew(SBorder).Padding(8).BorderBackgroundColor(FLinearColor(.09f, .16f, .14f))[Box];
}

TSharedRef<SWidget> SPlanogramStudio::BuildAddRow(int32 ProductIndex, const FPlanogramFixture& Fixture)
{
    const FMarketProduct& Product = Products[ProductIndex];
    const FString Id = Product.Id, FixtureId = Fixture.Id;
    const FPlanogramEquipment Spec = MarketPlanogram::Equipment(Fixture.EquipmentId);
    int32 Blocks = 0;
    for (const FPlanogramPlacement& Block : Planogram.Placements) if (Block.ProductId == Id) ++Blocks;
    TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
    Row->AddSlot().FillWidth(1.f).VAlign(VAlign_Center)
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(Product.RealName + TEXT("  \u2022  ") + Product.Brand)).ColorAndOpacity(Ink())]
        + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).ColorAndOpacity(Muted()).Text(FText::FromString(Blocks == 0
            ? FString(TEXT("Rafta de\u011fil: oyunda sat\u0131lmaz."))
            : FString::Printf(TEXT("Rafta %d blok"), Blocks)))]
    ];
    TArray<FString> Faces = { TEXT("front") };
    if (Spec.bDoubleSided) Faces.Add(TEXT("back"));
    for (const FString& Face : Faces)
    {
        Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(8, 0, 4, 0)
        [ SNew(STextBlock).Text(FText::FromString(Face == TEXT("back") ? TEXT("Arka:") : TEXT("\u00d6n:"))).ColorAndOpacity(Muted()) ];
        for (int32 Level = 0; Level < Spec.Levels; ++Level)
            Row->AddSlot().AutoWidth().Padding(0, 0, 3, 0)
            [
                SNew(SButton).Text(FText::FromString(FString::Printf(TEXT("S%d"), Level + 1)))
                .OnClicked_Lambda([this, Id, FixtureId, Face, Level]
                {
                    FString Message;
                    const bool bOk = MarketPlanogramEdit::AddToRowEnd(Planogram, Products, Id, FixtureId, Face, Level, Message);
                    Run(bOk, Message);
                    return FReply::Handled();
                })
            ];
    }
    return SNew(SBorder).Padding(6).BorderBackgroundColor(Blocks == 0 ? FLinearColor(.16f, .11f, .07f) : FLinearColor(.065f, .07f, .07f))[Row];
}

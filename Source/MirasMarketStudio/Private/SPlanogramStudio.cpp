#include "SPlanogramStudio.h"

#include "ProductCatalog.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
    FSlateColor Ink() { return FSlateColor(FLinearColor(.88f, .90f, .87f)); }
    FSlateColor Muted() { return FSlateColor(FLinearColor(.55f, .61f, .58f)); }
}

void SPlanogramStudio::Construct(const FArguments& InArgs)
{
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
    MarketPlanogram::Reconcile(Planogram, Products);
    SelectedFixture = FMath::Clamp(SelectedFixture, 0, FMath::Max(0, Planogram.Fixtures.Num() - 1));
    Status = Errors.Num() ? Errors[0] : TEXT("Planogram y\u00fcklendi.");
    Rebuild();
}

void SPlanogramStudio::Save(const FString& Message)
{
    FString Error;
    Status = MarketPlanogram::SaveFile(MarketPlanogram::DefaultPath(), Planogram, Error) ? Message : Error;
    Rebuild();
}

void SPlanogramStudio::SelectFixture(int32 Index) { SelectedFixture = Index; Rebuild(); }

void SPlanogramStudio::MoveProductHere(int32 ProductIndex)
{
    if (!Products.IsValidIndex(ProductIndex) || !Planogram.Fixtures.IsValidIndex(SelectedFixture)) return;
    const FPlanogramFixture& Fixture = Planogram.Fixtures[SelectedFixture];
    FPlanogramPlacement Candidate;
    if (const FPlanogramPlacement* Existing = Planogram.FindPlacement(Products[ProductIndex].Id)) Candidate = *Existing;
    Candidate.ProductId = Products[ProductIndex].Id;
    if (!MarketPlanogram::PlaceOnFixture(Planogram, Products, Fixture, Candidate, Candidate.Level))
    {
        Status = TEXT("Bu gondolda yer yok: hi\u00e7bir seviyeye tek adet bile s\u0131\u011fm\u0131yor. \u00d6nce ba\u015fka bir \u00fcr\u00fcn\u00fcn \u00f6n adedini azalt.");
        Rebuild();
        return;
    }
    Candidate.Order = Planogram.Placements.Num();
    if (FPlanogramPlacement* Existing = Planogram.FindPlacement(Candidate.ProductId)) *Existing = Candidate;
    else Planogram.Placements.Add(Candidate);
    Save(TEXT("\u00dcr\u00fcn se\u00e7ili gondola ta\u015f\u0131nd\u0131."));
}

void SPlanogramStudio::ChangeValue(int32 ProductIndex, int32 Field, int32 Delta)
{
    if (!Products.IsValidIndex(ProductIndex)) return;
    FPlanogramPlacement* P = Planogram.FindPlacement(Products[ProductIndex].Id);
    if (!P) return;
    if (Field == 2)
    {
        P->Depth = FMath::Clamp(P->Depth + Delta, 1, MarketPlanogram::MaxDepth);
        Save(TEXT("Raf \u00f6l\u00e7\u00fcs\u00fc kaydedildi."));
        return;
    }
    const FPlanogramEquipment Spec = MarketPlanogram::EquipmentFor(Planogram, P->FixtureId);
    const int32 Level = Field == 0 ? FMath::Clamp(P->Level + Delta, 0, Spec.Levels - 1) : P->Level;
    const int32 Facings = Field == 1 ? FMath::Clamp(P->Facings + Delta, 1, MarketPlanogram::MaxFacings) : P->Facings;
    if (Level == P->Level && Facings == P->Facings) return;
    float Width = 0.f;
    if (!MarketPlanogram::FitsOnLevel(Planogram, Products, P->FixtureId, P->Face, Level, P->ProductId, Facings, &Width))
    {
        Status = FString::Printf(TEXT("S\u0131\u011fmaz: seviye %d %.1f cm olur, raf %.1f cm."), Level + 1, Width, Spec.UsableWidthCm);
        Rebuild();
        return;
    }
    P->Level = Level; P->Facings = Facings;
    Save(TEXT("Raf \u00f6l\u00e7\u00fcs\u00fc kaydedildi."));
}

void SPlanogramStudio::ToggleFace(int32 ProductIndex)
{
    if (!Products.IsValidIndex(ProductIndex)) return;
    FPlanogramPlacement* P = Planogram.FindPlacement(Products[ProductIndex].Id);
    const FPlanogramFixture* Fixture = P ? Planogram.FindFixture(P->FixtureId) : nullptr;
    if (!P || !Fixture) return;
    const FString Face = P->Face == TEXT("back") ? TEXT("front") : TEXT("back");
    if (Face == TEXT("back") && !MarketPlanogram::IsDoubleSided(*Fixture))
    {
        Status = TEXT("Bu ekipman\u0131n arka y\u00fcz\u00fc yok.");
        Rebuild();
        return;
    }
    float Width = 0.f;
    if (!MarketPlanogram::FitsOnLevel(Planogram, Products, P->FixtureId, Face, P->Level, P->ProductId, P->Facings, &Width))
    {
        Status = FString::Printf(TEXT("S\u0131\u011fmaz: di\u011fer y\u00fczde seviye %d %.1f cm olur, raf %.1f cm."), P->Level + 1, Width, MarketPlanogram::Equipment(Fixture->EquipmentId).UsableWidthCm);
        Rebuild();
        return;
    }
    P->Face = Face;
    Save(TEXT("Gondol y\u00fcz\u00fc de\u011fi\u015ftirildi."));
}

void SPlanogramStudio::ApplyStrategy(const FString& Strategy)
{
    if (!Planogram.Fixtures.IsValidIndex(SelectedFixture)) return;
    FPlanogramFixture& Fixture = Planogram.Fixtures[SelectedFixture];
    Fixture.Strategy = Strategy;
    const FString FixtureId = Fixture.Id;
    TArray<FPlanogramPlacement*> Rows;
    for (FPlanogramPlacement& P : Planogram.Placements) if (P.FixtureId == FixtureId) Rows.Add(&P);
    if (Strategy == TEXT("margin")) Rows.Sort([&](const FPlanogramPlacement& A, const FPlanogramPlacement& B)
    {
        const FMarketProduct* PA = Products.FindByPredicate([&](const FMarketProduct& X){ return X.Id == A.ProductId; });
        const FMarketProduct* PB = Products.FindByPredicate([&](const FMarketProduct& X){ return X.Id == B.ProductId; });
        return PA && PB && PA->BasePrice - PA->Cost > PB->BasePrice - PB->Cost;
    });
    else if (Strategy == TEXT("brand_block")) Rows.Sort([&](const FPlanogramPlacement& A, const FPlanogramPlacement& B)
    {
        const FMarketProduct* PA = Products.FindByPredicate([&](const FMarketProduct& X){ return X.Id == A.ProductId; });
        const FMarketProduct* PB = Products.FindByPredicate([&](const FMarketProduct& X){ return X.Id == B.ProductId; });
        return PA && PB && PA->Brand < PB->Brand;
    });
    // Empty the fixture, then place rows one by one so every level stays within the usable width.
    for (FPlanogramPlacement* Row : Rows) Row->FixtureId.Reset();
    int32 Squeezed = 0;
    for (int32 I = 0; I < Rows.Num(); ++I)
    {
        FPlanogramPlacement Candidate = *Rows[I];
        Candidate.Order = I;
        Candidate.Facings = Strategy == TEXT("margin") && I < 2 ? 4 : 2;
        Candidate.Depth = Strategy == TEXT("margin") ? 4 : 3;
        const int32 Levels = MarketPlanogram::Equipment(Planogram.Fixtures[SelectedFixture].EquipmentId).Levels;
        if (!MarketPlanogram::PlaceOnFixture(Planogram, Products, Planogram.Fixtures[SelectedFixture], Candidate, I % Levels))
        {
            Candidate.FixtureId = FixtureId; Candidate.Face = TEXT("front");
            Candidate.Level = I % Levels; Candidate.Facings = 1;
            ++Squeezed;
        }
        *Rows[I] = Candidate;
    }
    Save(Squeezed ? FString::Printf(TEXT("Strateji uyguland\u0131; %d \u00fcr\u00fcn gondola s\u0131\u011fmad\u0131 (a\u015fa\u011f\u0131daki uyar\u0131ya bak)."), Squeezed)
                  : FString(TEXT("Sat\u0131\u015f stratejisi raf plan\u0131na uyguland\u0131.")));
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
    return FString::Printf(TEXT("Seviye %d  |  \u00d6nde %d  |  Derinlik %d  |  %s"), P.Level + 1, P.Facings, P.Depth, P.Face == TEXT("back") ? TEXT("arka y\u00fcz") : TEXT("\u00f6n y\u00fcz"));
}

void SPlanogramStudio::Rebuild()
{
    if (!FixtureBox || !ProductBox) return;
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
    ProductBox->AddSlot().AutoHeight().Padding(0, 0, 0, 8)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().Padding(0,0,5,0)[SNew(SButton).Text(FText::FromString(TEXT("Dengeli"))).OnClicked_Lambda([this]{ApplyStrategy(TEXT("balanced")); return FReply::Handled();})]
        + SHorizontalBox::Slot().AutoWidth().Padding(0,0,5,0)[SNew(SButton).Text(FText::FromString(TEXT("K\u00e2r odakl\u0131"))).OnClicked_Lambda([this]{ApplyStrategy(TEXT("margin")); return FReply::Handled();})]
        + SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("Marka blo\u011fu"))).OnClicked_Lambda([this]{ApplyStrategy(TEXT("brand_block")); return FReply::Handled();})]
    ];
    {
        // Width usage per level; the game places blocks centered within UsableWidthCm.
        TArray<FString> Faces = { TEXT("front") };
        if (MarketPlanogram::IsDoubleSided(F)) Faces.Add(TEXT("back"));
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
        ProductBox->AddSlot().AutoHeight().Padding(0, 0, 0, 4)
        [ SNew(STextBlock).Text(FText::FromString(Planogram.bAutoFill
            ? TEXT("Otomatik dolum a\u00e7\u0131k: oyunda her blok bo\u015f geni\u015fli\u011fe yay\u0131l\u0131r ve raf derinli\u011fince dizilir; buradaki say\u0131lar en az de\u011ferdir.")
            : TEXT("Otomatik dolum kapal\u0131: oyunda tam buradaki say\u0131lar kullan\u0131l\u0131r."))).ColorAndOpacity(Muted()) ];
        ProductBox->AddSlot().AutoHeight().Padding(0, 0, 0, 8)
        [ SNew(SButton).Text(FText::FromString(Planogram.bAutoFill ? TEXT("Otomatik dolumu kapat") : TEXT("Otomatik dolumu a\u00e7")))
          .OnClicked_Lambda([this]{ Planogram.bAutoFill = !Planogram.bAutoFill; Save(TEXT("Otomatik dolum ayar\u0131 kaydedildi.")); return FReply::Handled(); }) ];
        ProductBox->AddSlot().AutoHeight().Padding(0, 0, 0, 8)[SNew(SSeparator)];
    }
    for (int32 I = 0; I < Products.Num(); ++I)
    {
        const FPlanogramPlacement* P = Planogram.FindPlacement(Products[I].Id);
        const bool bHere = P && P->FixtureId == F.Id;
        ProductBox->AddSlot().AutoHeight().Padding(0, 3)
        [
            SNew(SBorder).Padding(8).BorderBackgroundColor(bHere ? FLinearColor(.09f,.16f,.14f) : FLinearColor(.065f,.07f,.07f))
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(Products[I].RealName + TEXT("  \u2022  ") + Products[I].Brand)).ColorAndOpacity(Ink())]
                + SVerticalBox::Slot().AutoHeight().Padding(0,3)[SNew(STextBlock).Text(FText::FromString(bHere ? PlacementSummary(*P) : TEXT("Ba\u015fka gondolda"))).ColorAndOpacity(Muted())]
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().Padding(0,0,5,0)[SNew(SButton).Text(FText::FromString(bHere ? TEXT("Bu gondolda") : TEXT("Bu gondola ta\u015f\u0131"))).IsEnabled(!bHere).OnClicked_Lambda([this,I]{MoveProductHere(I); return FReply::Handled();})]
                    + SHorizontalBox::Slot().AutoWidth().Padding(0,0,3,0)[SNew(SButton).Text(FText::FromString(TEXT("Seviye \u2212"))).IsEnabled(bHere).OnClicked_Lambda([this,I]{ChangeValue(I,0,-1); return FReply::Handled();})]
                    + SHorizontalBox::Slot().AutoWidth().Padding(0,0,5,0)[SNew(SButton).Text(FText::FromString(TEXT("+"))).IsEnabled(bHere).OnClicked_Lambda([this,I]{ChangeValue(I,0,1); return FReply::Handled();})]
                    + SHorizontalBox::Slot().AutoWidth().Padding(0,0,3,0)[SNew(SButton).Text(FText::FromString(TEXT("\u00d6n \u2212"))).IsEnabled(bHere).OnClicked_Lambda([this,I]{ChangeValue(I,1,-1); return FReply::Handled();})]
                    + SHorizontalBox::Slot().AutoWidth().Padding(0,0,5,0)[SNew(SButton).Text(FText::FromString(TEXT("+"))).IsEnabled(bHere).OnClicked_Lambda([this,I]{ChangeValue(I,1,1); return FReply::Handled();})]
                    + SHorizontalBox::Slot().AutoWidth().Padding(0,0,3,0)[SNew(SButton).Text(FText::FromString(TEXT("Derinlik \u2212"))).IsEnabled(bHere).OnClicked_Lambda([this,I]{ChangeValue(I,2,-1); return FReply::Handled();})]
                    + SHorizontalBox::Slot().AutoWidth().Padding(0,0,5,0)[SNew(SButton).Text(FText::FromString(TEXT("+"))).IsEnabled(bHere).OnClicked_Lambda([this,I]{ChangeValue(I,2,1); return FReply::Handled();})]
                    + SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FText::FromString(TEXT("Y\u00fcz\u00fc \u00e7evir"))).IsEnabled(bHere).OnClicked_Lambda([this,I]{ToggleFace(I); return FReply::Handled();})]
                ]
            ]
        ];
    }
    ProductBox->AddSlot().AutoHeight().Padding(0, 10)[SNew(STextBlock).Text(FText::FromString(Status)).ColorAndOpacity(Muted())];
}

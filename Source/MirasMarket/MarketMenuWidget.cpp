// Management menu (G-059, G-074): frame, theme, building blocks and the Ozet / Siparis / Raporlar pages.
// The other pages are in MarketMenuPages.cpp; helpers shared by both files are declared in MarketMenuInternal.h.
#include "MarketMenuWidget.h"
#include "MarketCountry.h"
#include "MarketStart.h"
#include "MarketMenuInternal.h"

#include "MarketGame.h"
#include "ProductCatalog.h"
#include "MarketSuppliers.h"
#include "MarketPromotions.h"
#include "MarketCompetitors.h"
#include "MarketEvents.h"
#include "MarketStory.h"
#include "MarketFinance.h"
#include "MarketOnline.h"
#include "MarketPayments.h"
#include "MarketSimulation.h"
#include "MarketFreshness.h"
#include "MarketBranches.h"
#include "MarketCompany.h"
#include "MarketCampaign.h"
#include "MarketCalendar.h"
#include "MarketStaff.h"
#include "MarketRivals.h"
#include "MarketDirector.h"
#include "MarketDemand.h"
#include "MarketOrderAdvice.h"
#include "MarketPrices.h"
#include "MarketMap.h"
#include "MarketTheme.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Brushes/SlateColorBrush.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "InputCoreTypes.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SBackgroundBlur.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "Widgets/SToolTip.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

// Named namespace (not anonymous): the module is built as a unity build and the HUD has its own helpers.
namespace MarketMenuUi
{
    FSlateFontInfo MenuFont(bool bBold, int32 Size)
    {
        // G-086d: IBM Plex Sans (semi bold for bold text), titles in Bricolage Grotesque. Size is in points; small
        // print stays at 9 at least (the menu itself scales with the screen).
        using MarketTheme::EFace;
        if (bBold && Size >= 17) return MarketTheme::Font(EFace::DisplayBold, Size / 0.75f);
        return MarketTheme::Font(bBold ? EFace::Semi : EFace::Regular, FMath::Max(Size, 9) / 0.75f);
    }

    // 1234.5 TL -> "1.234,50 TL"
    FString Tl(int64 Kurus)
    {
        return MarketCountry::Money(Kurus); // G-084: the active country\'s currency
    }

    FString TlShort(int64 Kurus)
    {
        const double Lira = static_cast<double>(Kurus) * MarketCountry::Active().DisplayScale / 100.0; // the country's money, as Tl shows it
        const double Size = FMath::Abs(Lira);
        if (Size >= 1.0e9) return MarketCountry::Decorate(FString::Printf(TEXT("%.2f milyar"), Lira / 1.0e9).Replace(TEXT("."), TEXT(",")));
        if (Size >= 1.0e6) return MarketCountry::Decorate(FString::Printf(TEXT("%.1f milyon"), Lira / 1.0e6).Replace(TEXT("."), TEXT(",")));
        if (Size < 1.0e4) return MarketCountry::Money(Kurus);
        const FString Digits = FString::Printf(TEXT("%lld"), static_cast<long long>(FMath::RoundToDouble(Size)));
        FString Grouped;
        for (int32 I = 0; I < Digits.Len(); ++I)
        {
            if (I > 0 && (Digits.Len() - I) % 3 == 0) Grouped.AppendChar(TEXT('.'));
            Grouped.AppendChar(Digits[I]);
        }
        return MarketCountry::Decorate(Lira < 0.0 ? TEXT("-") + Grouped : Grouped);
    }

    FLinearColor Hex(const TCHAR* Code, float Alpha)
    {
        FLinearColor Result(FColor::FromHex(Code));
        Result.A = Alpha;
        return Result;
    }

    // "s\u00fct" -> "S\u00fct", "i\u00e7ecek" -> "\u0130\u00e7ecek" (Turkish dotted capital I).
    FString Title(const FString& Text)
    {
        if (Text.IsEmpty()) return Text;
        FString Result = Text;
        const TCHAR First = Result[0];
        Result[0] = First == TEXT('i') ? TCHAR(0x0130) : First == TCHAR(0x0131) ? TEXT('I') : FChar::ToUpper(First);
        return Result;
    }

    FString Initials(const FString& Name)
    {
        TArray<FString> Words;
        Name.ParseIntoArrayWS(Words);
        FString Result;
        for (const FString& Word : Words)
        {
            if (Word.IsEmpty() || !FChar::IsAlnum(Word[0])) continue;
            Result.AppendChar(FChar::ToUpper(Word[0]));
            if (Result.Len() >= 2) break;
        }
        return Result.IsEmpty() ? FString(TEXT("?")) : Result;
    }

    FLinearColor RivalColor(int32 Rival)
    {
        switch (Rival)
        {
        case 0: return Hex(TEXT("C8102E"));
        case 1: return Hex(TEXT("F28C00"));
        default: return Hex(TEXT("0068A8"));
        }
    }

    // Price of one rival for one product today; false = the rival's shelf is empty.
    bool RivalShelfPrice(const AMarketGameMode& G, int32 Product, int32 Rival, int64& OutPrice)
    {
        OutPrice = 0;
        if (!G.Products.IsValidIndex(Product)) return false;
        bool bEmpty = false;
        const float Factor = MarketRivals::RivalFactor(G.State.Day, G.State.RivalSeed, G.RivalAisles, G.Products[Product].Category, Rival, &bEmpty)
            * MarketCompetitors::NewsRivalIndex(G.State, Rival); // the chain's everyday price level (G-065)
        OutPrice = MarketDemand::RivalPrice(G.Products[Product], Factor);
        return !bEmpty;
    }

    // Cheapest open rival with the product on its shelf (INDEX_NONE = nobody has it today).
    int32 CheapestRival(const AMarketGameMode& G, int32 Product, int64& OutPrice)
    {
        int32 Best = INDEX_NONE;
        OutPrice = 0;
        for (int32 Rival = 0; Rival < MarketRivals::RivalCount(G.State.Day); ++Rival)
        {
            int64 Price = 0;
            if (RivalShelfPrice(G, Product, Rival, Price) && (Best == INDEX_NONE || Price < OutPrice)) { Best = Rival; OutPrice = Price; }
        }
        return Best;
    }

    double BuyChanceOf(const AMarketGameMode& G, int32 Product)
    {
        if (!G.Products.IsValidIndex(Product) || !G.State.Stock.IsValidIndex(Product)) return 0.0;
        const int64 Theirs = MarketDemand::RivalPrice(G.Products[Product], G.RivalPriceFactor(Product));
        return MarketDemand::BuyChanceFor(MarketDemand::PriceRatio(G.State.Stock[Product].Price, Theirs), G.State.MarketShare, 0.0, MarketDemand::ElasticityOf(G.Products[Product]), G.Products[Product].Kvi); // C3 (B #24)
    }

    FString ProblemText(const AMarketGameMode& G, const MarketDemand::FProblem& Problem)
    {
        const FString Shown = G.Products.IsValidIndex(Problem.Product) ? G.ProductName(Problem.Product) : FString();
        switch (Problem.Kind)
        {
        case MarketDemand::EProblem::Waiting: return FString::Printf(TEXT("%d m\u00fc\u015fteri i\u00e7eride beklemekten vazge\u00e7ti."), Problem.Count);
        case MarketDemand::EProblem::NotCarried: return FString::Printf(TEXT("%d m\u00fc\u015fteri %s sordu ama rafta yok (R ile reyona koy)."), Problem.Count, *Shown);
        case MarketDemand::EProblem::Empty: return FString::Printf(TEXT("%s rafta bitti: %d m\u00fc\u015fteri eli bo\u015f d\u00f6nd\u00fc."), *Shown, Problem.Count);
        case MarketDemand::EProblem::Expensive: return FString::Printf(TEXT("%d m\u00fc\u015fteri %s fiyat\u0131n\u0131 pahal\u0131 buldu."), Problem.Count, *Shown);
        }
        return FString();
    }

    FString RivalsToday(const AMarketGameMode& G, int32 OnlyRival)
    {
        TArray<FString> Lines;
        for (const MarketRivals::FEvent& Event : MarketRivals::ActiveOn(G.State.Day, G.State.RivalSeed, G.RivalAisles))
            if (OnlyRival == INDEX_NONE || Event.Rival == OnlyRival) Lines.Add(MarketRivals::Describe(Event));
        return Lines.Num() > 0 ? FString::Join(Lines, TEXT("\n")) : FString(TEXT("Sakin: \u00f6zel bir kampanya yok."));
    }
}
// ---------------------------------------------------------------------------------------------------------------
// Theme

bool SMarketMenu::IsLight() const
{
    const AMarketGameMode* G = Game.Get();
    return !G || G->bLightTheme;
}

FLinearColor SMarketMenu::Color(ERole Role) const
{
    using MarketMenuUi::Hex;
    const bool bLight = IsLight();
    // G-086d: the tokens of the design boards (light: paper F4F1EA; dark: 101316).
    switch (Role)
    {
    case ERole::Page: return bLight ? Hex(TEXT("F4F1EA"), 0.98f) : Hex(TEXT("101316"), 0.98f);
    case ERole::Panel: return bLight ? Hex(TEXT("FFFFFF")) : Hex(TEXT("1B1E22"));
    case ERole::Inset: return bLight ? Hex(TEXT("EDE9E1")) : Hex(TEXT("2C3137"));
    case ERole::Text: return bLight ? Hex(TEXT("1B1E22")) : Hex(TEXT("F2F3F4"));
    case ERole::Muted: return bLight ? Hex(TEXT("6B7178")) : Hex(TEXT("9AA3AC"));
    case ERole::Accent: return bLight ? Hex(TEXT("2F8A70")) : Hex(TEXT("71C6AC"));
    case ERole::Good: return bLight ? Hex(TEXT("2F8A70")) : Hex(TEXT("71C6AC"));
    case ERole::Bad: return bLight ? Hex(TEXT("C4453A")) : Hex(TEXT("F07F6E"));
    case ERole::Warn: return bLight ? Hex(TEXT("D08A1E")) : Hex(TEXT("E8A94E"));
    case ERole::Line: return bLight ? Hex(TEXT("E2DDD3")) : Hex(TEXT("343A41"));
    case ERole::Button: return bLight ? Hex(TEXT("EDE9E1")) : Hex(TEXT("2C3137"));
    case ERole::ButtonText: return bLight ? Hex(TEXT("1B1E22")) : Hex(TEXT("F2F3F4"));
    case ERole::Primary: return bLight ? Hex(TEXT("1B1E22")) : Hex(TEXT("F2F3F4"));
    case ERole::PrimaryText: return bLight ? Hex(TEXT("FFFFFF")) : Hex(TEXT("101316"));
    case ERole::Dim: return FLinearColor(0.f, 0.f, 0.f, bLight ? 0.30f : 0.55f);
    case ERole::Info: return bLight ? Hex(TEXT("2F5FA8")) : Hex(TEXT("7FA7E0"));
    case ERole::Solid: return bLight ? Hex(TEXT("FFFFFF")) : Hex(TEXT("1B1E22"), 0.95f);
    case ERole::Stage: return bLight ? Hex(TEXT("F4F1EA")) : Hex(TEXT("101316"));
    case ERole::Land: return bLight ? Hex(TEXT("DEDAD1")) : Hex(TEXT("23282E"));
    case ERole::Ours: return bLight ? Hex(TEXT("9ED0BE")) : Hex(TEXT("2E6E5D"));
    case ERole::Home: return Hex(TEXT("3F9A80"));
    case ERole::Hairline: return bLight ? Hex(TEXT("ECE8E0")) : Hex(TEXT("2A2F35"));
    case ERole::OnAccent: return bLight ? Hex(TEXT("FFFFFF")) : Hex(TEXT("0F1A16"));
    case ERole::Sheet: return bLight ? Hex(TEXT("FBFAF6")) : Hex(TEXT("16191C"));
    case ERole::WarnSoft: return bLight ? Hex(TEXT("FFF4E2")) : Hex(TEXT("3A2E17"));
    case ERole::AccentSoft: return bLight ? Hex(TEXT("E3F2EC")) : Hex(TEXT("1F3A33"));
    case ERole::MapLabel: return bLight ? Hex(TEXT("4A5057")) : Hex(TEXT("C9D0D6"));
    case ERole::DockText: return bLight ? Hex(TEXT("3A4046")) : Hex(TEXT("D6DBE0"));
    }
    return FLinearColor::White;
}

TAttribute<FSlateColor> SMarketMenu::Col(ERole Role) const
{
    return TAttribute<FSlateColor>::CreateLambda([this, Role] { return FSlateColor(Color(Role)); });
}

TAttribute<FSlateColor> SMarketMenu::ColBy(TFunction<ERole()> Role) const
{
    return TAttribute<FSlateColor>::CreateLambda([this, Role] { return FSlateColor(Color(Role ? Role() : ERole::Text)); });
}

// ---------------------------------------------------------------------------------------------------------------
// Building blocks

TSharedRef<SWidget> SMarketMenu::Label(TFunction<FString()> Make, int32 Size, ERole Role, bool bBold, bool bWrap)
{
    return SNew(STextBlock).Font(MarketMenuUi::MenuFont(bBold, Size)).ColorAndOpacity(Col(Role)).AutoWrapText(bWrap)
        .Text_Lambda([Make] { return FText::FromString(Make ? Make() : FString()); });
}

TSharedRef<SWidget> SMarketMenu::LabelBy(TFunction<FString()> Make, int32 Size, TFunction<ERole()> Role, bool bBold)
{
    return SNew(STextBlock).Font(MarketMenuUi::MenuFont(bBold, Size)).ColorAndOpacity(ColBy(Role))
        .Text_Lambda([Make] { return FText::FromString(Make ? Make() : FString()); });
}

TSharedRef<SWidget> SMarketMenu::Fixed(const FString& Text, int32 Size, ERole Role, bool bBold)
{
    return SNew(STextBlock).Font(MarketMenuUi::MenuFont(bBold, Size)).ColorAndOpacity(Col(Role)).Text(FText::FromString(Text));
}

TSharedRef<SWidget> SMarketMenu::Card(const TSharedRef<SWidget>& Content, ERole Role, const FMargin& Padding)
{
    return SNew(SBorder).BorderImage(&CardBrush).BorderBackgroundColor(Col(Role)).Padding(Padding)[ Content ];
}

TSharedRef<SWidget> SMarketMenu::Button(TFunction<FString()> Text, TFunction<void()> OnClick, bool bPrimary, TFunction<bool()> Enabled)
{
    return SNew(SButton).ButtonStyle(&PillStyle).IsFocusable(false).ContentPadding(FMargin(16.f, 7.f))
        .HAlign(HAlign_Center).VAlign(VAlign_Center)
        .ButtonColorAndOpacity(Col(bPrimary ? ERole::Primary : ERole::Button))
        .IsEnabled_Lambda([Enabled] { return !Enabled || Enabled(); })
        .OnClicked_Lambda([OnClick] { if (OnClick) OnClick(); return FReply::Handled(); })
    [
        Label(Text, 11, bPrimary ? ERole::PrimaryText : ERole::ButtonText, true)
    ];
}

TSharedRef<SWidget> SMarketMenu::Bar(TFunction<float()> Value, ERole Role)
{
    return SNew(SBox).HeightOverride(8.f)
    [
        SNew(SBorder).BorderImage(&TrackBrush).BorderBackgroundColor(Col(ERole::Line)).Padding(0.f)
        [
        SNew(SProgressBar).Style(&BarStyle).BorderPadding(FVector2D(0.f, 0.f))
        .FillColorAndOpacity(Col(Role))
        .Percent_Lambda([Value]() -> TOptional<float> { return FMath::Clamp(Value ? Value() : 0.f, 0.f, 1.f); })
        ]
    ];
}

const FSlateBrush* SMarketMenu::LogoBrush(const FString& Key)
{
    if (const TSharedPtr<FSlateBrush>* Found = Logos.Find(Key)) return Found->Get();
    TSharedPtr<FSlateBrush> Brush;
    const FString Path = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Brands"), Key, TEXT("logo.png"));
    if (!Key.IsEmpty() && FPaths::FileExists(Path))
    {
        if (UTexture2D* Texture = FImageUtils::ImportFileAsTexture2D(Path))
        {
            LogoTextures.Emplace(Texture);
            Brush = MakeShared<FSlateBrush>();
            Brush->SetResourceObject(Texture);
            Brush->DrawAs = ESlateBrushDrawType::Image;
            Brush->ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());
        }
    }
    Logos.Add(Key, Brush);
    return Brush.Get();
}

TSharedRef<SWidget> SMarketMenu::Badge(const FString& Key, const FString& Initials, const FLinearColor& Fill, float Size)
{
    if (const FSlateBrush* Logo = LogoBrush(Key))
        return SNew(SBox).WidthOverride(Size).HeightOverride(Size)[ SNew(SImage).Image(Logo) ];
    return SNew(SBox).WidthOverride(Size).HeightOverride(Size)
    [
        SNew(SBorder).BorderImage(&BadgeBrush).BorderBackgroundColor(Fill).HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.f)
        [
            SNew(STextBlock).Font(MarketMenuUi::MenuFont(true, FMath::RoundToInt32(Size * 0.34f))).ColorAndOpacity(FLinearColor::White)
            .Text(FText::FromString(Initials))
        ]
    ];
}

TSharedRef<SWidget> SMarketMenu::Stat(const FString& Heading, TFunction<FString()> Value, TFunction<FString()> Sub, TFunction<ERole()> ValueRole)
{
    return Card(
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Fixed(Heading, 9, ERole::Muted, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[ LabelBy(Value, 21, ValueRole ? ValueRole : TFunction<ERole()>([] { return ERole::Text; }), true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)[ Label(Sub, 10, ERole::Muted, false, true) ]); // C3 (A5): long sub lines wrap
}

TSharedRef<SWidget> SMarketMenu::CategoryChips(FString* Chosen)
{
    TSharedRef<SWrapBox> Box = SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 6.f));
    TArray<FString> Names = { FString() };
    if (const AMarketGameMode* G = Game.Get()) Names.Append(MarketRivals::Aisles(G->Products));
    for (const FString& Name : Names)
    {
        Box->AddSlot()
        [
            SNew(SButton).ButtonStyle(&PillStyle).IsFocusable(false).ContentPadding(FMargin(13.f, 5.f))
            .ButtonColorAndOpacity_Lambda([this, Chosen, Name] { return FSlateColor(Color(*Chosen == Name ? ERole::Primary : ERole::Button)); })
            .OnClicked_Lambda([Chosen, Name] { *Chosen = Name; return FReply::Handled(); })
            [
                SNew(STextBlock).Font(MarketMenuUi::MenuFont(true, 10))
                .Text(FText::FromString(Name.IsEmpty() ? FString(TEXT("T\u00fcm\u00fc")) : MarketMenuUi::Title(Name)))
                .ColorAndOpacity_Lambda([this, Chosen, Name] { return FSlateColor(Color(*Chosen == Name ? ERole::PrimaryText : ERole::ButtonText)); })
            ]
        ];
    }
    return Box;
}

TSharedRef<SWidget> SMarketMenu::Dot(TFunction<bool()> Done)
{
    return SNew(SBox).WidthOverride(10.f).HeightOverride(10.f).VAlign(VAlign_Center)
    [ SNew(SBorder).BorderImage(&BadgeBrush).BorderBackgroundColor(ColBy([Done] { return Done && Done() ? ERole::Good : ERole::Line; })) ];
}

void SMarketMenu::Do(FName Action, int32 Product)
{
    if (AMarketGameMode* G = Game.Get()) G->MenuCommand(Action, Product);
}

void SMarketMenu::Manage(FName Action, int32 Arg)
{
    if (AMarketGameMode* G = Game.Get()) G->StaffCommand(Action, Arg);
}

void SMarketMenu::Go(int32 Page)
{
    if (AMarketGameMode* G = Game.Get()) G->MenuPage = FMath::Clamp(Page, 0, PageCount - 1);
}


// ---------------------------------------------------------------------------------------------------------------
// Page names, confirmations, pictures


const TCHAR* SMarketMenu::PageName(int32 Page)
{
    switch (Page)
    {
    case Summary: return TEXT("Ana ekran");
    case Orders: return TEXT("Sipari\u015f");
    case Prices: return TEXT("\u00dcr\u00fcnler ve fiyat");
    case Promotions: return TEXT("Kampanyalar");
    case Rivals: return TEXT("Rakipler");
    case Staff: return TEXT("Personel");
    case Finance: return TEXT("Finans");
    case Channels: return TEXT("Sat\u0131\u015f kanallar\u0131");
    case Branches: return TEXT("Ma\u011fazalar");
    default: return TEXT("Raporlar");
    }
}

void SMarketMenu::Ask(const FString& Question, TFunction<void()> OnYes)
{
    ConfirmText = Question;
    ConfirmAction = MoveTemp(OnYes);
}

TSharedRef<SWidget> SMarketMenu::RiskyButton(TFunction<FString()> Text, TFunction<FString()> Question, TFunction<void()> OnClick, TFunction<bool()> Enabled)
{
    return Button(Text, [this, Question, OnClick] { Ask(Question ? Question() : FString(TEXT("Emin misin?")), OnClick); }, false, Enabled);
}

TSharedRef<SWidget> SMarketMenu::Choice(const FString& Text, TFunction<bool()> Selected, TFunction<void()> OnClick, TFunction<bool()> Enabled)
{
    return SNew(SButton).ButtonStyle(&PillStyle).IsFocusable(false).ContentPadding(FMargin(12.f, 5.f))
        .ButtonColorAndOpacity_Lambda([this, Selected] { return FSlateColor(Color(Selected && Selected() ? ERole::Primary : ERole::Button)); })
        .IsEnabled_Lambda([Enabled] { return !Enabled || Enabled(); })
        .OnClicked_Lambda([OnClick] { if (OnClick) OnClick(); return FReply::Handled(); })
    [
        SNew(STextBlock).Font(MarketMenuUi::MenuFont(true, 10)).Text(FText::FromString(Text))
        .ColorAndOpacity_Lambda([this, Selected] { return FSlateColor(Color(Selected && Selected() ? ERole::PrimaryText : ERole::ButtonText)); })
    ];
}

TSharedRef<SWidget> SMarketMenu::Section(const FString& Title)
{
    return Fixed(Title, 9, ERole::Muted, true);
}

TSharedRef<SWidget> SMarketMenu::Why(TFunction<FString()> Reason)
{
    return SNew(SBox).Visibility_Lambda([Reason] { return Reason && !Reason().IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed; })
    [ Label(Reason, 9, ERole::Warn, false, true) ];
}

TSharedRef<SWidget> SMarketMenu::LayerFade(const TSharedRef<SWidget>& Inner)
{
    // M15: a layer inside a card (the appointment, the candidates, the depot builder) never pops in.
    return SNew(SBorder).BorderImage(&NoBrush).Padding(0.f)
        .ColorAndOpacity_Lambda([this] { const float T = LayerAnim; return FLinearColor(1.f, 1.f, 1.f, T * T * (3.f - 2.f * T)); })
        .RenderTransform_Lambda([this] { const float T = LayerAnim; return TOptional<FSlateRenderTransform>(FSlateRenderTransform(FVector2f(0.f, 6.f * (1.f - T * T * (3.f - 2.f * T))))); })
    [ Inner ];
}

const FSlateBrush* SMarketMenu::PictureBrush(int32 Product)
{
    const AMarketGameMode* G = Game.Get();
    if (!G || !G->Products.IsValidIndex(Product)) return nullptr;
    const FMarketProduct& P = G->Products[Product];
    if (const TSharedPtr<FSlateBrush>* Found = Pictures.Find(P.Id)) return Found->Get();
    TSharedPtr<FSlateBrush> Brush;
    // The studio writes every label to /Game/Products/Items/<id>/T_<id>_Label (MirasMarketStudio).
    const FString Path = FString::Printf(TEXT("/Game/Products/Items/%s/T_%s_Label.T_%s_Label"), *P.Id, *P.Id, *P.Id);
    UTexture2D* Texture = P.MeshPath.IsEmpty() ? nullptr : LoadObject<UTexture2D>(nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet);
    if (Texture)
    {
        // Box packages: the front face of the atlas (FBoxPackageLayout, the same layout the studio used).
        // Round packages: the middle of the label band, the part the shopper sees.
        FBox2f Region(FVector2f(0.32f, 0.f), FVector2f(0.68f, 1.f));
        FVector2D Size(72.0, 100.0);
        // The box size comes from the catalog; older rows only have it in the package folder name (box_70x50x200).
        int32 W = P.WidthMm, D = P.DepthMm, H = P.HeightMm;
        if (W <= 0 || D <= 0 || H <= 0)
        {
            FString Folder;
            const int32 PackagesAt = P.MeshPath.Find(TEXT("/Packages/"));
            if (PackagesAt != INDEX_NONE)
            {
                Folder = P.MeshPath.Mid(PackagesAt + 10);
                int32 Slash = INDEX_NONE;
                if (Folder.FindChar(TEXT('/'), Slash)) Folder = Folder.Left(Slash);
            }
            if (!FBoxPackageLayout::ParsePackageId(Folder, W, D, H)) W = D = H = 0;
        }
        FBoxPackageLayout Layout;
        const bool bBox = P.PackageType.IsEmpty() || P.PackageType == TEXT("kutu");
        if (bBox && W > 0 && D > 0 && H > 0 && FBoxPackageLayout::Make(W, D, H, Layout))
        {
            const FIntRect Front = Layout.Rects[FBoxPackageLayout::Front];
            const float Atlas = static_cast<float>(FMath::Max(1, Layout.AtlasSize));
            Region = FBox2f(FVector2f(Front.Min.X / Atlas, Front.Min.Y / Atlas), FVector2f(Front.Max.X / Atlas, Front.Max.Y / Atlas));
            Size = FVector2D(FMath::Max(1, Front.Width()), FMath::Max(1, Front.Height()));
        }
        PictureTextures.Emplace(Texture);
        Brush = MakeShared<FSlateBrush>();
        Brush->SetResourceObject(Texture);
        Brush->DrawAs = ESlateBrushDrawType::Image;
        Brush->ImageSize = Size;
        Brush->SetUVRegion(Region);
    }
    Pictures.Add(P.Id, Brush);
    return Brush.Get();
}

TSharedRef<SWidget> SMarketMenu::ProductPicture(int32 Product, float Size)
{
    const AMarketGameMode* G = Game.Get();
    if (const FSlateBrush* Picture = PictureBrush(Product))
    {
        return SNew(SBox).WidthOverride(Size).HeightOverride(Size)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(3.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
            [ SNew(SScaleBox).Stretch(EStretch::ScaleToFit)[ SNew(SImage).Image(Picture) ] ]
        ];
    }
    const FString Name = G && G->Products.IsValidIndex(Product) ? G->ProductName(Product) : FString();
    const FLinearColor Fill = G && G->Products.IsValidIndex(Product) ? FLinearColor(G->Products[Product].Color) : Color(ERole::Muted);
    return Badge(FString(), MarketMenuUi::Initials(Name), Fill, Size);
}

// ---------------------------------------------------------------------------------------------------------------
// Frame

void SMarketMenu::Construct(const FArguments& InArgs)
{
    Game = InArgs._Game;
    // G-086d: pills and round buttons have half-height corners whatever their size.
    for (FSlateBrush* Round : { static_cast<FSlateBrush*>(&PillBrush), static_cast<FSlateBrush*>(&PillHover), static_cast<FSlateBrush*>(&PillPress),
        static_cast<FSlateBrush*>(&RoundBrush), static_cast<FSlateBrush*>(&CircleBrush) })
        Round->OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
    PillStyle = FButtonStyle().SetNormal(PillBrush).SetHovered(PillHover).SetPressed(PillPress).SetDisabled(PillBrush)
        .SetNormalPadding(FMargin(0.f)).SetPressedPadding(FMargin(0.f));
    RowStyle = FButtonStyle().SetNormal(SmallBrush).SetHovered(SmallHover).SetPressed(SmallPress).SetDisabled(SmallBrush)
        .SetNormalPadding(FMargin(0.f)).SetPressedPadding(FMargin(0.f));
    CircleStyle = FButtonStyle().SetNormal(CircleBrush).SetHovered(CircleBrush).SetPressed(CircleBrush).SetDisabled(CircleBrush)
        .SetNormalPadding(FMargin(0.f)).SetPressedPadding(FMargin(0.f));
    RoundStyle = PillStyle;
    ItemStyle = FButtonStyle().SetNormal(ItemBrush).SetHovered(ItemHover).SetPressed(ItemPress).SetDisabled(ItemBrush)
        .SetNormalPadding(FMargin(0.f)).SetPressedPadding(FMargin(0.f));
    BarStyle = FProgressBarStyle().SetBackgroundImage(NoBrush).SetFillImage(TrackBrush).SetMarqueeImage(TrackBrush);

    // G-075: full screen over the running shop. The 3D view behind is blurred and tinted (glass); the content is
    // laid out for 1440 x 820 and scaled to fill the screen (UiScale).
    // G-086 sade ana ekran: the map page fills the screen; the other pages sit between the top pills and the dock.
    auto Framed = [this](const TSharedRef<SWidget>& Page) -> TSharedRef<SWidget>
    {
        return SNew(SBox).Padding(FMargin(24.f, 84.f, 24.f, 104.f))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ PageTitle() ]
            + SVerticalBox::Slot().FillHeight(1.f)[ Page ]
        ];
    };
    ChildSlot
    [
        SNew(SOverlay)
        .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && G->bMenuOpen ? EVisibility::Visible : EVisibility::Collapsed; })
        + SOverlay::Slot()[ SNew(SBackgroundBlur).BlurStrength(18.f).Padding(0.f) ]
        + SOverlay::Slot()[ SNew(SBorder).BorderImage(&FlatBrush).BorderBackgroundColor(Col(ERole::Page)) ]
        + SOverlay::Slot()
        [
            SNew(SDPIScaler).DPIScale_Lambda([this] { return UiScale(); })
            [
                SNew(SOverlay)
                + SOverlay::Slot()
                [
                    SNew(SBorder).BorderImage(&NoBrush).Padding(0.f)
                    .ColorAndOpacity_Lambda([this] { const float T = PageAnim; return FLinearColor(1.f, 1.f, 1.f, T * T * (3.f - 2.f * T)); })
                    .RenderTransform_Lambda([this] { const float T = PageAnim; return TOptional<FSlateRenderTransform>(FSlateRenderTransform(FVector2f(0.f, 10.f * (1.f - T * T * (3.f - 2.f * T))))); })
                    [
                    SNew(SWidgetSwitcher)
                    .WidgetIndex_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G ? FMath::Clamp(G->MenuPage, 0, PageCount - 1) : 0; })
                    + SWidgetSwitcher::Slot()[ HomePage() ]
                    + SWidgetSwitcher::Slot()[ Framed(OrdersPage()) ]
                    + SWidgetSwitcher::Slot()[ Framed(PricesPage()) ]
                    + SWidgetSwitcher::Slot()[ Framed(PromotionsPage()) ]
                    + SWidgetSwitcher::Slot()[ Framed(RivalsPage()) ]
                    + SWidgetSwitcher::Slot()[ Framed(StaffPage()) ]
                    + SWidgetSwitcher::Slot()[ Framed(FinancePage()) ]
                    + SWidgetSwitcher::Slot()[ Framed(ChannelsPage()) ]
                    + SWidgetSwitcher::Slot()[ Framed(BranchesPage()) ]
                    + SWidgetSwitcher::Slot()[ Framed(ReportsPage()) ]
                    ]
                ]
                + SOverlay::Slot().VAlign(VAlign_Top)[ TopBar() ]
                + SOverlay::Slot().VAlign(VAlign_Bottom)[ BottomNav() ]
                // The short message of the last action, under the chips (it fades by itself).
                + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0.f, 128.f, 0.f, 0.f)
                [
                    SNew(SBox).MaxDesiredWidth(460.f)
                    .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && G->MessageTime > 0.f && !G->Message.IsEmpty() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
                    [ Raised(SNew(SBorder).BorderImage(&TileBrush).BorderBackgroundColor(Col(ERole::Solid)).Padding(FMargin(14.f, 9.f))
                        [ TextPx([this] { const AMarketGameMode* G = Game.Get(); return G ? G->Message : FString(); }, 13.f, [] { return ERole::Text; }, false, true) ]) ]
                ]
                + SOverlay::Slot()[ OrderListLayer() ]
                + SOverlay::Slot()[ DecisionsLayer() ]
                + SOverlay::Slot()[ SettingsLayer() ]
                + SOverlay::Slot()[ NewGameLayer() ]
                + SOverlay::Slot()[ ConfirmLayer() ]
            ]
        ]
    ];
}

void SMarketMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
    LastSize = FVector2D(AllottedGeometry.GetLocalSize());
    // G-086e: the panel and the "Di\u011fer" card glide (about a quarter of a second) instead of popping.
    if (!MapProvinceId.IsEmpty()) PanelId = MapProvinceId;
    const float Blend = 1.f - FMath::Exp(-14.f * FMath::Clamp(InDeltaTime, 0.f, 0.1f));
    auto Glide = [Blend](float& Value, float Target) { Value += (Target - Value) * Blend; if (FMath::Abs(Target - Value) < 0.002f) Value = Target; };
    Glide(PanelAnim, PanelOpen() ? 1.f : 0.f);
    Glide(MoreAnim, bMoreOpen ? 1.f : 0.f);
    Glide(OrderAnim, bOrderListOpen ? 1.f : 0.f);
    // A new page fades and rises in (no hard cut between tabs).
    if (const AMarketGameMode* G = Game.Get())
        if (G->MenuPage != ShownPage) { ShownPage = G->MenuPage; PageAnim = 0.f; bMoreOpen = false; }
    PageAnim += (1.f - PageAnim) * (1.f - FMath::Exp(-16.f * FMath::Clamp(InDeltaTime, 0.f, 0.1f)));
    if (PageAnim > 0.998f) PageAnim = 1.f;
    // G-086b ek / G-089: a layer inside a card (appointment, candidates, depot builder) fades in when it changes.
    const int32 Layers = static_cast<int32>(HashCombine(HashCombine(GetTypeHash(AppointArea), GetTypeHash(PickBranch)),
        HashCombine(GetTypeHash(DepotLayer * 1000 + DepotSel), GetTypeHash(PromoteBranch))));
    if (Layers != LayerKey) { LayerKey = Layers; LayerAnim = 0.f; }
    LayerAnim += (1.f - LayerAnim) * (1.f - FMath::Exp(-16.f * FMath::Clamp(InDeltaTime, 0.f, 0.1f)));
    if (LayerAnim > 0.998f) LayerAnim = 1.f;
}

float SMarketMenu::UiScale() const
{
    const FVector2D Size = LastSize.X > 1.0 && LastSize.Y > 1.0 ? LastSize : FVector2D(1920.0, 1080.0);
    const float Fit = FMath::Min(static_cast<float>(Size.X) / 1440.f, static_cast<float>(Size.Y) / 820.f);
    const AMarketGameMode* G = Game.Get();
    return FMath::Clamp(Fit * (G ? G->UiTextFactor() : 1.f), 0.5f, 4.f);
}

TSharedRef<SToolTip> SMarketMenu::Tip(TFunction<FString()> Text)
{
    // Tooltips open in their own window, outside the menu's scaler: the font and the width follow UiScale here.
    return SNew(SToolTip)
    [
        SNew(SBox).MaxDesiredWidth_Lambda([this]() -> FOptionalSize { return 440.f * UiScale(); })
        [
            SNew(STextBlock).AutoWrapText(true)
            .Font_Lambda([this] { return MarketMenuUi::MenuFont(false, FMath::RoundToInt32(12.f * UiScale())); })
            .Text_Lambda([Text] { return FText::FromString(Text ? Text() : FString()); })
        ]
    ];
}

TSharedRef<SWidget> SMarketMenu::Info(TFunction<FString()> Text)
{
    return SNew(SBox).WidthOverride(18.f).HeightOverride(18.f).VAlign(VAlign_Center)
        .ToolTip(Tip(Text))
    [
        SNew(SBorder).BorderImage(&BadgeBrush).BorderBackgroundColor(Col(ERole::Inset)).HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.f)
        [ Fixed(TEXT("i"), 10, ERole::Muted, true) ]
    ];
}

TSharedRef<SWidget> SMarketMenu::More(TFunction<FString()> Text)
{
    return SNew(SBox).HAlign(HAlign_Left).ToolTip(Tip(Text))
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 6.f, 0.f)
        [
            SNew(SBox).WidthOverride(18.f).HeightOverride(18.f)
            [
                SNew(SBorder).BorderImage(&BadgeBrush).BorderBackgroundColor(Col(ERole::Inset)).HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.f)
                [ Fixed(TEXT("i"), 10, ERole::Info, true) ]
            ]
        ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Fixed(TEXT("Nas\u0131l i\u015fler?"), 10, ERole::Info) ]
    ];
}

TSharedRef<SWidget> SMarketMenu::Heading(const FString& Title, TFunction<FString()> Hint)
{
    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Section(Title) ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)[ Info(Hint) ];
}

TSharedRef<SWidget> SMarketMenu::ConfirmLayer()
{
    return SNew(SOverlay)
        .Visibility_Lambda([this] { return ConfirmAction ? EVisibility::Visible : EVisibility::Collapsed; })
        + SOverlay::Slot()[ SNew(SBorder).BorderImage(&FlatBrush).BorderBackgroundColor(Col(ERole::Dim)) ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [
            SNew(SBox).WidthOverride(520.f)
            [
                SNew(SBorder).BorderImage(&CardBrush).BorderBackgroundColor(Col(ERole::Solid)).Padding(FMargin(28.f, 24.f))
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("EM\u0130N M\u0130S\u0130N?"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 20.f)[ Label([this] { return ConfirmText; }, 14, ERole::Text, false, true) ]
                    + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
                        [ Button([] { return FString(TEXT("Vazge\u00e7  (Esc)")); }, [this] { ConfirmAction = nullptr; }) ]
                        + SHorizontalBox::Slot().AutoWidth()
                        [ Button([] { return FString(TEXT("Evet  (Enter)")); }, [this] { TFunction<void()> Run = MoveTemp(ConfirmAction); ConfirmAction = nullptr; if (Run) Run(); }, true) ]
                    ]
                ]
            ]
        ];
}

FReply SMarketMenu::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    AMarketGameMode* G = Game.Get();
    if (!G || !G->bMenuOpen) return FReply::Unhandled();
    const FKey Key = InKeyEvent.GetKey();
    if (ConfirmAction)
    {
        if (Key == EKeys::Escape) { ConfirmAction = nullptr; return FReply::Handled(); }
        if (Key == EKeys::Enter) { TFunction<void()> Run = MoveTemp(ConfirmAction); ConfirmAction = nullptr; if (Run) Run(); return FReply::Handled(); }
        return FReply::Handled();
    }
    if (G->bNeedStart) return FReply::Handled(); // G-086: the new-game screen waits for a province
    if (Key == EKeys::Escape && (bSettingsOpen || bDecisionsOpen || bMoreOpen)) { bSettingsOpen = bDecisionsOpen = bMoreOpen = false; return FReply::Handled(); }
    if (Key == EKeys::Escape && bOrderListOpen) { bOrderListOpen = false; OrderSwapFrom = INDEX_NONE; return FReply::Handled(); }
    if (Key == EKeys::Escape && G->bNewGameAsk) { G->bNewGameAsk = false; return FReply::Handled(); }
    // G-086b ek / G-089: Esc closes a picker inside a card first (candidates, appointment, depot builder).
    if (Key == EKeys::Escape && G->MenuPage == Branches && (AppointArea != INDEX_NONE || PickBranch != INDEX_NONE || PromoteBranch != INDEX_NONE || DepotLayer != 0))
    {
        AppointArea = INDEX_NONE; PickBranch = INDEX_NONE; PromoteBranch = INDEX_NONE; DepotLayer = 0;
        return FReply::Handled();
    }
    // G-086 main screen: Esc closes the province panel first, then leaves the zoomed region.
    if (Key == EKeys::Escape && G->MenuPage == Summary && PanelOpen()) { MapProvinceId.Reset(); DepotSel = INDEX_NONE; return FReply::Handled(); }
    if (Key == EKeys::Escape && G->MenuPage == Summary && !MapRegion.IsEmpty()) { MapRegion.Reset(); return FReply::Handled(); }
    if (Key == EKeys::Escape || Key == EKeys::M) { G->CloseMenu(); return FReply::Handled(); }
    // G-075 game speed: Space pauses, + / - change the speed (digits choose pages here).
    if (Key == EKeys::SpaceBar) { G->SetTimePaused(!G->bTimePaused); return FReply::Handled(); }
    if (Key == EKeys::Add || Key == EKeys::Equals) { G->SetGameSpeed(G->bTimePaused ? G->GameSpeed : G->GameSpeed + 1); return FReply::Handled(); }
    if (Key == EKeys::Subtract || Key == EKeys::Hyphen) { if (G->GameSpeed <= 1) G->SetTimePaused(true); else G->SetGameSpeed(G->GameSpeed - 1); return FReply::Handled(); }
    static const FKey Digits[PageCount] = { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six, EKeys::Seven, EKeys::Eight, EKeys::Nine, EKeys::Zero };
    for (int32 Page = 0; Page < PageCount; ++Page)
        if (Key == Digits[Page]) { Go(Page); return FReply::Handled(); }
    return FReply::Unhandled();
}

FReply SMarketMenu::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    // A click on an empty part must not hand the keyboard to the 3D view.
    return FReply::Handled().SetUserFocus(SharedThis(this), EFocusCause::Mouse);
}

// The dock: pages in the order of the design board, their icons and short names.
namespace MarketMenuFrame
{
    // bMore: the page sits behind "Di\u011fer" (the dock keeps the six of the board).
    struct FDockPage { int32 Page; const TCHAR* Icon; const TCHAR* Name; bool bMore; };
    const TArray<FDockPage>& DockPages()
    {
        static const TArray<FDockPage> Pages = {
            { SMarketMenu::Summary, TEXT("map"), TEXT("Ana ekran"), false },
            { SMarketMenu::Branches, TEXT("store"), TEXT("Ma\u011fazalar"), false },
            { SMarketMenu::Orders, TEXT("box"), TEXT("Sipari\u015f"), false },
            { SMarketMenu::Prices, TEXT("tag"), TEXT("Fiyat"), false },
            { SMarketMenu::Promotions, TEXT("percent"), TEXT("Kampanya"), false },
            { SMarketMenu::Staff, TEXT("person"), TEXT("Personel"), false },
            { SMarketMenu::Reports, TEXT("bars"), TEXT("Raporlar"), false },
            { SMarketMenu::Rivals, TEXT("flag"), TEXT("Rakipler"), true },
            { SMarketMenu::Finance, TEXT("wallet"), TEXT("Finans"), true },
            { SMarketMenu::Channels, TEXT("bag"), TEXT("Sat\u0131\u015f"), true } };
        return Pages;
    }
}

bool SMarketMenu::PanelOpen() const
{
    const AMarketGameMode* G = Game.Get();
    return G && G->MenuPage == Summary && !MapProvinceId.IsEmpty() && MarketCountry::FindCity(ShownCountry(), MapProvinceId) != nullptr;
}

FMargin SMarketMenu::EdgePadding(float Left, float Top, float Bottom) const
{
    // G-086e: the frame never moves; the province panel floats over the map.
    return FMargin(Left, Top, 24.f, Bottom);
}

TSharedRef<SWidget> SMarketMenu::Raised(const TSharedRef<SWidget>& Surface)
{
    // G-086e: the image shadow showed as grey boxes behind the pills; surfaces carry a faint outline instead
    // (their brushes), so this only keeps the call sites.
    return Surface;
}

TSharedRef<SWidget> SMarketMenu::IconImage(const FString& Name, float Size, TFunction<ERole()> Role)
{
    const FSlateBrush* Brush = MarketTheme::Icon(Name);
    return SNew(SBox).WidthOverride(Size).HeightOverride(Size)
    [
        SNew(SImage).Image(Brush ? Brush : &NoBrush)
        .ColorAndOpacity_Lambda([this, Role] { return FSlateColor(Color(Role ? Role() : ERole::Text)); })
    ];
}

TSharedRef<SWidget> SMarketMenu::Mono(TFunction<FString()> Make, float Pixels, TFunction<ERole()> Role)
{
    return SNew(STextBlock).Font(MarketTheme::Font(MarketTheme::EFace::Mono, Pixels)).ColorAndOpacity(ColBy(Role))
        .Text_Lambda([Make] { return FText::FromString(Make ? Make() : FString()); });
}

TSharedRef<SWidget> SMarketMenu::Display(TFunction<FString()> Make, float Pixels, ERole Role)
{
    return SNew(STextBlock).Font(MarketTheme::Font(MarketTheme::EFace::DisplayBold, Pixels)).ColorAndOpacity(Col(Role))
        .Text_Lambda([Make] { return FText::FromString(Make ? Make() : FString()); });
}

TSharedRef<SWidget> SMarketMenu::TextPx(TFunction<FString()> Make, float Pixels, TFunction<ERole()> Role, bool bSemi, bool bWrap)
{
    return SNew(STextBlock).Font(MarketTheme::Font(bSemi ? MarketTheme::EFace::Semi : MarketTheme::EFace::Regular, Pixels))
        .ColorAndOpacity(ColBy(Role)).AutoWrapText(bWrap)
        .Text_Lambda([Make] { return FText::FromString(Make ? Make() : FString()); });
}

TSharedRef<SWidget> SMarketMenu::IconButton(const FString& Icon, float Size, TFunction<bool()> Active, TFunction<void()> OnClick, const FString& Hint)
{
    // A round button with an icon (the speed buttons, the settings): accent when active, otherwise quiet.
    return SNew(SBox).WidthOverride(Size).HeightOverride(Size).ToolTip(Tip([Hint] { return Hint; }))
    [
        SNew(SButton).ButtonStyle(&RoundStyle).IsFocusable(false).ContentPadding(FMargin(0.f)).HAlign(HAlign_Center).VAlign(VAlign_Center)
        .ButtonColorAndOpacity_Lambda([this, Active] { return FSlateColor(Active && Active() ? Color(ERole::Accent) : FLinearColor::Transparent); })
        .OnClicked_Lambda([OnClick] { if (OnClick) OnClick(); return FReply::Handled(); })
        [ IconImage(Icon, Size * 0.4f, [Active] { return Active && Active() ? ERole::OnAccent : ERole::Muted; }) ]
    ];
}

TSharedRef<SWidget> SMarketMenu::NavItem(int32 Page, const FString& Text, const FString& Key)
{
    // One page of the dock (72 x 58): its icon over its name; a red dot when something urgent waits there.
    auto IsCurrent = [this, Page] { const AMarketGameMode* G = Game.Get(); return G && G->MenuPage == Page; };
    FString Icon = TEXT("store");
    for (const MarketMenuFrame::FDockPage& Item : MarketMenuFrame::DockPages()) if (Item.Page == Page) Icon = Item.Icon;
    auto Tone = [IsCurrent] { return IsCurrent() ? ERole::Accent : ERole::DockText; };
    return SNew(SBox).WidthOverride(68.f).HeightOverride(58.f)
        .ToolTip(Tip([Page, Key] { return FString::Printf(TEXT("%s  (%s tu\u015fu)"), PageName(Page), *Key); }))
    [
        SNew(SButton).ButtonStyle(&ItemStyle).IsFocusable(false).ContentPadding(FMargin(0.f)).HAlign(HAlign_Fill).VAlign(VAlign_Fill)
        .ButtonColorAndOpacity_Lambda([this, IsCurrent] { return FSlateColor(IsCurrent() ? Color(ERole::Inset) : FLinearColor::Transparent); })
        .OnClicked_Lambda([this, Page] { Go(Page); bMoreOpen = false; return FReply::Handled(); })
        [
            SNew(SOverlay)
            + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[ IconImage(Icon, 22.f, Tone) ]
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 0.f)[ TextPx([Text] { return Text; }, 11.f, Tone) ]
            ]
            + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0.f, 8.f, 14.f, 0.f)
            [
                SNew(SBox).WidthOverride(7.f).HeightOverride(7.f)
                .Visibility_Lambda([this, Page]
                {
                    const AMarketGameMode* G = Game.Get();
                    if (!G) return EVisibility::Collapsed;
                    for (const FMarketTodo& Todo : G->Todos()) if (Todo.Page == Page && Todo.Severity >= 2) return EVisibility::Visible;
                    return EVisibility::Collapsed;
                })
                [ SNew(SBorder).BorderImage(&CircleBrush).BorderBackgroundColor(Col(ERole::Bad)) ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SMarketMenu::TopBar()
{
    // G-086d (design boards 4 and 5): the date pill on the left; the till pill on the right, or next to the date
    // while the province panel is open.
    auto G = [this] { return Game.Get(); };
    auto Column = [this](const FString& Heading, TFunction<FString()> Value, TFunction<ERole()> Role) -> TSharedRef<SWidget>
    {
        return SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)[ TextPx([Heading] { return Heading; }, 11.f, [] { return ERole::Muted; }) ]
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)[ Mono(Value, 16.f, Role) ];
    };
    auto Till = [this, G, Column]() -> TSharedRef<SWidget>
    {
        return Raised(SNew(SBox).HeightOverride(52.f)
        [
            SNew(SBorder).BorderImage(&RoundBrush).BorderBackgroundColor(Col(ERole::Solid)).Padding(FMargin(22.f, 0.f)).VAlign(VAlign_Center)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 22.f, 0.f)
                [ Column(TEXT("Kasa"), [G] { return G() ? MarketMenuUi::Tl(G()->State.Cash) : FString(); }, [G] { return G() && G()->State.Cash < 0 ? ERole::Bad : ERole::Text; }) ]
                // M37: our own money next to the company's till (not the company's money).
                + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 22.f, 0.f)
                [ Column(TEXT("Servet"), [G] { return G() ? MarketMenuUi::Tl(G()->State.Owner.Wealth) : FString(); }, [] { return ERole::Text; }) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 22.f, 0.f)
                [ Column(TEXT("D\u00fcn"), [G]
                {
                    if (!G() || G()->State.Day <= 1) return FString(TEXT("\u2014"));
                    const int64 Net = G()->State.LastProfit;
                    return (Net > 0 ? TEXT("+") : TEXT("")) + MarketMenuUi::Tl(Net);
                }, [G] { return G() && G()->State.LastProfit < 0 ? ERole::Bad : ERole::Accent; }) ]
                + SHorizontalBox::Slot().AutoWidth()
                [ Column(TEXT("Ma\u011faza"), [G] { return G() ? FString::FromInt(MarketCompany::TotalStores(G()->State)) : FString(); }, [] { return ERole::Text; }) ]
            ]
        ]);
    };
    return SNew(SBox).Padding(FMargin(24.f, 20.f, 24.f, 0.f))
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)[ TimeControls() ]
        + SHorizontalBox::Slot().FillWidth(1.f)[ SNew(SSpacer) ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)[ Till() ]
    ];
}

TSharedRef<SWidget> SMarketMenu::BottomNav()
{
    // G-086d: the dock at the bottom (every page; the map item only away from the map; digits 1-0 still work),
    // the way into the family shop, and the bell of the waiting decisions (bottom right; bottom left while the
    // province panel is open).
    auto G = [this] { return Game.Get(); };
    TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
    TSharedRef<SVerticalBox> Extra = SNew(SVerticalBox);
    for (const MarketMenuFrame::FDockPage& Item : MarketMenuFrame::DockPages())
    {
        const int32 Page = Item.Page;
        if (Item.bMore)
        {
            Extra->AddSlot().AutoHeight().Padding(0.f, 2.f)[ NavItem(Page, Item.Name, FString::FromInt((Page + 1) % 10)) ];
            continue;
        }
        Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 4.f, 0.f)
        [
            NavItem(Page, Item.Name, FString::FromInt((Page + 1) % 10))
        ];
    }
    // "Di\u011fer": the three pages behind it open in a small card above the dock.
    auto InMore = [G]
    {
        if (!G()) return false;
        for (const MarketMenuFrame::FDockPage& Item : MarketMenuFrame::DockPages()) if (Item.bMore && Item.Page == G()->MenuPage) return true;
        return false;
    };
    auto MoreTone = [this, InMore] { return bMoreOpen || InMore() ? ERole::Accent : ERole::DockText; };
    Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 4.f, 0.f)
    [
        SNew(SBox).WidthOverride(68.f).HeightOverride(58.f).ToolTip(Tip([] { return FString(TEXT("Rakipler (5), Finans (7), Sat\u0131\u015f kanallar\u0131 (8)")); }))
        [
            SNew(SButton).ButtonStyle(&ItemStyle).IsFocusable(false).ContentPadding(FMargin(0.f)).HAlign(HAlign_Center).VAlign(VAlign_Center)
            .ButtonColorAndOpacity_Lambda([this, InMore] { return FSlateColor(bMoreOpen || InMore() ? Color(ERole::Inset) : FLinearColor::Transparent); })
            .OnClicked_Lambda([this] { bMoreOpen = !bMoreOpen; return FReply::Handled(); })
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[ IconImage(TEXT("more"), 22.f, MoreTone) ]
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 0.f)[ TextPx([] { return FString(TEXT("Di\u011fer")); }, 11.f, MoreTone) ]
            ]
        ]
    ];
    Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(2.f, 0.f, 6.f, 0.f)
    [ SNew(SBox).WidthOverride(1.f).HeightOverride(36.f)[ SNew(SBorder).BorderImage(&FlatBrush).BorderBackgroundColor(Col(ERole::Line)) ] ];
    Row->AddSlot().AutoWidth().VAlign(VAlign_Center)
    [
        SNew(SBox).HeightOverride(58.f).ToolTip(Tip([] { return FString(TEXT("Aile d\u00fckk\u00e2n\u0131na d\u00f6n (M ya da Esc)")); }))
        [
            SNew(SButton).ButtonStyle(&ItemStyle).IsFocusable(false).ContentPadding(FMargin(18.f, 0.f)).VAlign(VAlign_Center)
            .ButtonColorAndOpacity(Col(ERole::Accent))
            .IsEnabled_Lambda([G] { return G() && !G()->bNeedStart; })
            .OnClicked_Lambda([this] { if (AMarketGameMode* Mode = Game.Get()) Mode->CloseMenu(); return FReply::Handled(); })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)[ IconImage(TEXT("house"), 18.f, [] { return ERole::OnAccent; }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ TextPx([] { return FString(TEXT("D\u00fckk\u00e2na gir")); }, 14.f, [] { return ERole::OnAccent; }, true) ]
            ]
        ]
    ];
    auto Waiting = [G]() -> int32
    {
        if (!G()) return 0;
        int32 Count = G()->State.Decisions.Num();
        for (const FMarketTodo& Todo : G()->Todos()) if (Todo.Severity >= 2) ++Count;
        return Count;
    };
    auto MakeBell = [this, Waiting]() -> TSharedRef<SWidget>
    {
        return Raised(SNew(SBox).WidthOverride(64.f).HeightOverride(64.f).ToolTip(Tip([] { return FString(TEXT("Kararlar: bekleyen se\u00e7imler, yap\u0131lacaklar, b\u00f6l\u00fcm\u00fcn hedefleri, i\u015fletmenin borcu")); }))
    [
        SNew(SButton).ButtonStyle(&RoundStyle).IsFocusable(false).ContentPadding(FMargin(0.f))
        .ButtonColorAndOpacity(Col(ERole::Solid))
        .OnClicked_Lambda([this] { bDecisionsOpen = !bDecisionsOpen; bSettingsOpen = false; return FReply::Handled(); })
        [
            SNew(SOverlay)
            + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)[ IconImage(TEXT("bell"), 24.f, [] { return ERole::Text; }) ]
            + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0.f, 8.f, 8.f, 0.f)
            [
                SNew(SBox).MinDesiredWidth(20.f).HeightOverride(20.f)
                .Visibility_Lambda([Waiting] { return Waiting() > 0 ? EVisibility::Visible : EVisibility::Collapsed; })
                [
                    SNew(SBorder).BorderImage(&CircleBrush).BorderBackgroundColor(Col(ERole::Warn)).HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(FMargin(5.f, 0.f))
                    [ SNew(STextBlock).Font(MarketTheme::Font(MarketTheme::EFace::Bold, 12.f)).ColorAndOpacity(Col(ERole::OnAccent))
                        .Text_Lambda([Waiting] { return FText::AsNumber(Waiting()); }) ]
                ]
            ]
        ]
    ]);
    };
    return SNew(SOverlay)
        + SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Bottom)
        [
            // The card fades and rises above the dock; it floats, so nothing else moves.
            SNew(SBox).HAlign(HAlign_Center).Padding(FMargin(24.f, 0.f, 24.f, 112.f))
            .Visibility_Lambda([this] { return MoreAnim > 0.01f ? EVisibility::Visible : EVisibility::Collapsed; })
            .RenderTransform_Lambda([this] { return TOptional<FSlateRenderTransform>(FSlateRenderTransform(FVector2f(163.f, 12.f * (1.f - MoreAnim)))); })
            [
                SNew(SBorder).BorderImage(&DockBrush).BorderBackgroundColor(Col(ERole::Solid)).Padding(FMargin(8.f))
                .ColorAndOpacity_Lambda([this] { return FLinearColor(1.f, 1.f, 1.f, MoreAnim); })
                [ Extra ]
            ]
        ]
        + SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Bottom)
        [
            SNew(SBox).HAlign(HAlign_Center).Padding(FMargin(24.f, 0.f, 24.f, 24.f))
            [ Raised(SNew(SBorder).BorderImage(&DockBrush).BorderBackgroundColor(Col(ERole::Solid)).Padding(FMargin(8.f))[ Row ]) ]
        ]
        + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 24.f, 28.f)[ MakeBell() ];
}

TSharedRef<SWidget> SMarketMenu::PageTitle()
{
    // The main screen needs no title; the other pages keep theirs.
    return SNew(SBox).Padding(FMargin(4.f, 4.f, 0.f, 12.f))
        .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && G->MenuPage != Summary ? EVisibility::Visible : EVisibility::Collapsed; })
    [ Label([this] { const AMarketGameMode* G = Game.Get(); return FString(PageName(G ? G->MenuPage : 0)); }, 24, ERole::Text, true) ];
}

TSharedRef<SWidget> SMarketMenu::SettingsLayer()
{
    auto G = [this] { return Game.Get(); };
    auto TextSize = [this, G](const FString& Text, int32 Value) -> TSharedRef<SWidget>
    {
        return Choice(Text, [G, Value] { return G() && G()->MenuTextSize == Value; }, [G, Value] { if (G()) G()->SetMenuTextSize(Value); });
    };
    auto Difficulty = [this, G](const FString& Text, int32 Value) -> TSharedRef<SWidget>
    {
        return Choice(Text, [G, Value] { return G() && G()->State.Difficulty == Value; }, [this, Value] { Manage(TEXT("Difficulty"), Value); },
            [G] { return G() && !G()->bOpen && G()->State.Day <= 1; });
    };
    auto SlotChoice = [this, G](int32 Value) -> TSharedRef<SWidget>
    {
        return Choice(FString::FromInt(Value), [G, Value] { return G() && G()->ActiveSlot == Value; }, [G, Value] { if (G()) G()->SelectSlot(Value); },
            [G] { return G() && !G()->bOpen; });
    };
    auto Row = [this](TSharedRef<SWidget> Heading, TSharedRef<SWidget> Body) -> TSharedRef<SWidget>
    {
        return SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 6.f)[ Heading ]
            + SVerticalBox::Slot().AutoHeight()[ Body ];
    };
    return SNew(SOverlay)
        .Visibility_Lambda([this] { return bSettingsOpen ? EVisibility::Visible : EVisibility::Collapsed; })
        + SOverlay::Slot()[ SNew(SBorder).BorderImage(&FlatBrush).BorderBackgroundColor(Col(ERole::Dim)) ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [
            SNew(SBox).WidthOverride(620.f)
            [
                SNew(SBorder).BorderImage(&CardBrush).BorderBackgroundColor(Col(ERole::Solid)).Padding(FMargin(28.f, 24.f))
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Fixed(TEXT("Ayarlar"), 22, ERole::Text, true) ]
                        + SHorizontalBox::Slot().AutoWidth()[ Button([] { return FString(TEXT("Kapat  (Esc)")); }, [this] { bSettingsOpen = false; }) ]
                    ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ Row(Section(TEXT("YAZI BOYUTU")),
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)[ TextSize(TEXT("K\u00fc\u00e7\u00fck"), 0) ]
                        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)[ TextSize(TEXT("Orta"), 1) ]
                        + SHorizontalBox::Slot().AutoWidth()[ TextSize(TEXT("B\u00fcy\u00fck"), 2) ]) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ Row(SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth()[ Section(TEXT("TEMA")) ],
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)
                        [ Choice(TEXT("A\u00e7\u0131k"), [this] { return IsLight(); }, [this] { if (!IsLight()) if (AMarketGameMode* Mode = Game.Get()) Mode->ToggleMenuTheme(); }) ]
                        + SHorizontalBox::Slot().AutoWidth()
                        [ Choice(TEXT("Koyu"), [this] { return !IsLight(); }, [this] { if (IsLight()) if (AMarketGameMode* Mode = Game.Get()) Mode->ToggleMenuTheme(); }) ]) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ Row(Heading(TEXT("MEN\u00dc A\u00c7IKKEN ZAMAN"), [] { return FString(TEXT("Akar: men\u00fc a\u00e7\u0131kken d\u00fckk\u00e2n i\u015flemeye devam eder. Durur: men\u00fc a\u00e7\u0131kken zaman bekler (karar A02).")); }),
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)
                        [ Choice(TEXT("Akar"), [G] { return G() && !G()->bPauseInMenu; }, [G] { if (G()) G()->SetPauseInMenu(false); }) ]
                        + SHorizontalBox::Slot().AutoWidth()
                        [ Choice(TEXT("Durur"), [G] { return G() && G()->bPauseInMenu; }, [G] { if (G()) G()->SetPauseInMenu(true); }) ]) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ Row(Heading(TEXT("ZORLUK"), [] { return FString(TEXT("Rahat: biraz daha \u00e7ok m\u00fc\u015fteri, fiyata daha ho\u015fg\u00f6r\u00fcl\u00fc. Zor: tersi. Yaln\u0131z kampanyan\u0131n ilk g\u00fcn\u00fcnde, d\u00fckk\u00e2n a\u00e7\u0131lmadan se\u00e7ilir.")); }),
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)[ Difficulty(TEXT("Rahat"), 0) ]
                        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)[ Difficulty(TEXT("Normal"), 1) ]
                        + SHorizontalBox::Slot().AutoWidth()[ Difficulty(TEXT("Zor"), 2) ]) ] // M32: the epidemic always comes
                    + SVerticalBox::Slot().AutoHeight()
                    [ Row(Section(TEXT("KAYIT YUVASI")),
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)[ SlotChoice(1) ]
                        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)[ SlotChoice(2) ]
                        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 12.f, 0.f)[ SlotChoice(3) ]
                        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Label([G] { return G() ? G()->SlotSummary(G()->ActiveSlot) : FString(); }, 10, ERole::Muted, false, true) ]) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ Row(Section(TEXT("YEN\u0130 OYUN")),
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth()
                        [ Button([] { return FString(TEXT("\u00dclke ve il se\u00e7erek yeni oyun")); }, [this, G] { bSettingsOpen = false; if (G()) G()->AskNewGame(); }, false,
                            [G] { return G() && !G()->bOpen; }) ]
                        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(12.f, 0.f, 0.f, 0.f)
                        [ Fixed(TEXT("Bu yuvadaki kampanya, yeni oyun ba\u015flay\u0131nca silinir."), 10, ERole::Muted) ]) ]
                ]
            ]
        ];
}

TSharedRef<SWidget> SMarketMenu::DecisionsLayer()
{
    auto G = [this] { return Game.Get(); };
    TSharedRef<SWidget> Debt = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("\u0130\u015eLETMEN\u0130N BORCU")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 8.f)
        [ LabelBy([G] { return !G() ? FString() : MarketCampaign::DebtOpen(G()->State) ? MarketMenuUi::Tl(G()->State.InheritedDebt) + TEXT(" kald\u0131") : FString(TEXT("Kapand\u0131")); }, 19,
            [G] { return G() && !MarketCampaign::DebtOpen(G()->State) ? ERole::Good : ERole::Warn; }, true) ]
        + SVerticalBox::Slot().AutoHeight()[ Bar([G] { return G() ? MarketCampaign::DebtProgress(G()->State) : 0.f; }, ERole::Accent) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 10.f)
        [ More([G] { return FString::Printf(TEXT("Baban\u0131n toptanc\u0131ya %s borcu \u00b7 s\u00fcre yok \u00b7 kapanmadan \u015fube a\u00e7\u0131lmaz"), *MarketMenuUi::Tl(G() ? G()->State.StartDebt : MarketCampaign::StartingDebt)); }) ]
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left)
        [ Button([G] { return FString::Printf(TEXT("%s \u00f6de"), *MarketMenuUi::Tl(G() ? MarketCampaign::InstallmentOf(G()->State) : MarketCampaign::Installment)); }, [this] { Do(TEXT("PayDebt")); }, true,
            [G] { return G() && MarketCampaign::DebtOpen(G()->State) && G()->State.Cash > 0; }) ]);
    TSharedRef<SWidget> FirstBranch = SNew(SBox)
        .Visibility_Lambda([G] { return G() && MarketBranches::OpenCount(G()->State) == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
        .Padding(FMargin(0.f, 12.f, 0.f, 0.f))
    [ Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[ Section(TEXT("\u0130LK \u015eUBE \u0130\u00c7\u0130N")) ]
        + SVerticalBox::Slot().AutoHeight()[ GoalList() ]) ];
    return SNew(SOverlay)
        .Visibility_Lambda([this] { return bDecisionsOpen ? EVisibility::Visible : EVisibility::Collapsed; })
        + SOverlay::Slot()[ SNew(SBorder).BorderImage(&FlatBrush).BorderBackgroundColor(Col(ERole::Dim)) ]
        + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Fill).Padding(0.f, 70.f, 24.f, 70.f)
        [
            SNew(SBox).WidthOverride(620.f)
            [
                SNew(SBorder).BorderImage(&CardBrush).BorderBackgroundColor(Col(ERole::Solid)).Padding(FMargin(22.f, 18.f))
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Fixed(TEXT("Kararlar ve yap\u0131lacaklar"), 20, ERole::Text, true) ]
                        + SHorizontalBox::Slot().AutoWidth()[ Button([] { return FString(TEXT("Kapat  (Esc)")); }, [this] { bDecisionsOpen = false; }) ]
                    ]
                    + SVerticalBox::Slot().FillHeight(1.f)
                    [
                        SNew(SScrollBox)
                        + SScrollBox::Slot()[ DecisionCard() ]
                        + SScrollBox::Slot()[ TodoList() ]
                        + SScrollBox::Slot().Padding(0.f, 12.f, 0.f, 0.f)[ StoryCard() ]
                        + SScrollBox::Slot().Padding(0.f, 12.f, 0.f, 0.f)[ Debt ]
                        + SScrollBox::Slot()[ FirstBranch ]
                    ]
                ]
            ]
        ];
}

TSharedRef<SWidget> SMarketMenu::TimeControls()
{
    // G-086d: the date pill (board 4): "12 Nisan \u00b7 Sal\u0131" over "3. y\u0131l \u00b7 08:40 \u00b7 K\u0131rklareli"; pause and the three
    // speeds as round icon buttons; the settings behind a quiet button at the end.
    auto G = [this] { return Game.Get(); };
    auto Parts = [G](bool bMain) -> FString
    {
        if (!G()) return FString();
        const FString Date = MarketCalendar::DateText(G()->State.Day); // "8 Mart, 1. y\u0131l Sal\u0131"
        FString DayMonth, Rest, Year, Weekday;
        if (!Date.Split(TEXT(", "), &DayMonth, &Rest)) { DayMonth = Date; }
        if (!Rest.Split(TEXT(" "), &Year, &Weekday, ESearchCase::IgnoreCase, ESearchDir::FromEnd)) Year = Rest;
        if (bMain) return Weekday.IsEmpty() ? DayMonth : DayMonth + TEXT(" \u00b7 ") + Weekday;
        const int32 Minutes = 8 * 60 + (G()->bOpen ? static_cast<int32>(G()->DayTime * 3.f) : 0);
        const FString Clock = FString::Printf(TEXT("%02d:%02d"), Minutes / 60, Minutes % 60);
        const FString Now = G()->bTimePaused ? FString(TEXT("durdu")) : G()->bOpen ? FString(TEXT("a\u00e7\u0131k")) : FString(TEXT("kapal\u0131"));
        TArray<FString> Bits;
        if (!Year.IsEmpty()) Bits.Add(Year);
        Bits.Add(Clock);
        Bits.Add(Now);
        Bits.Add(MarketStart::PlaceText(G()->State));
        return FString::Join(Bits, TEXT(" \u00b7 "));
    };
    auto Speed = [G](int32 Value) { return [G, Value] { return G() && !G()->bTimePaused && G()->GameSpeed == Value; }; };
    auto SetSpeed = [G](int32 Value) { return [G, Value] { if (G()) G()->SetGameSpeed(Value); }; };
    return Raised(SNew(SBox).HeightOverride(52.f)
    [
        SNew(SBorder).BorderImage(&RoundBrush).BorderBackgroundColor(Col(ERole::Solid)).Padding(FMargin(18.f, 0.f, 8.f, 0.f)).VAlign(VAlign_Center)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 14.f, 0.f)
            [
                SNew(SBox).MaxDesiredWidth(280.f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ TextPx([Parts] { return Parts(true); }, 15.f, [] { return ERole::Text; }, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ TextPx([Parts] { return Parts(false); }, 11.f, [G] { return G() && G()->bTimePaused ? ERole::Warn : ERole::Muted; }) ]
                ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 2.f, 0.f)
            [ IconButton(TEXT("pause"), 36.f, [G] { return G() && G()->bTimePaused; }, [G] { if (G()) G()->SetTimePaused(!G()->bTimePaused); }, TEXT("Zaman\u0131 durdur / devam (Space)")) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 2.f, 0.f)[ IconButton(TEXT("play"), 36.f, Speed(1), SetSpeed(1), TEXT("Normal h\u0131z (1x)")) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 2.f, 0.f)[ IconButton(TEXT("fast"), 36.f, Speed(2), SetSpeed(2), TEXT("H\u0131zl\u0131 (2x, men\u00fcde +)")) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ IconButton(TEXT("faster"), 36.f, Speed(3), SetSpeed(3), TEXT("\u00c7ok h\u0131zl\u0131 (3x)")) ]
            // C3 (A3): play whole days, a week or a month without walking (the same rules); it stops when you are needed.
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f)
            [ SNew(SBox).WidthOverride(1.f).HeightOverride(24.f)[ SNew(SBorder).BorderImage(&FlatBrush).BorderBackgroundColor(Col(ERole::Line)) ] ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 4.f, 0.f)
            [ Button([] { return FString(TEXT("+1 g\u00fcn")); }, [G] { if (G()) G()->AdvanceTime(MarketSimulation::ETurn::Day); }, false, [G] { return G() && !G()->bOpen; }) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 4.f, 0.f)
            [ Button([] { return FString(TEXT("+1 hafta")); }, [G] { if (G()) G()->AdvanceTime(MarketSimulation::ETurn::Week); }, false, [G] { return G() && !G()->bOpen; }) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ Button([] { return FString(TEXT("+1 ay")); }, [G] { if (G()) G()->AdvanceTime(MarketSimulation::ETurn::Month); }, false, [G] { return G() && !G()->bOpen; }) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f)
            [ SNew(SBox).WidthOverride(1.f).HeightOverride(24.f)[ SNew(SBorder).BorderImage(&FlatBrush).BorderBackgroundColor(Col(ERole::Line)) ] ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ IconButton(TEXT("sliders"), 36.f, [this] { return bSettingsOpen; }, [this] { bSettingsOpen = !bSettingsOpen; bDecisionsOpen = false; }, TEXT("Ayarlar: yaz\u0131 boyutu, tema, zorluk, kay\u0131t, yeni oyun")) ]
        ]
    ]);
}

// ---------------------------------------------------------------------------------------------------------------
// Pages

TSharedRef<SWidget> SMarketMenu::GoalList()
{
    auto Row = [this](TFunction<bool()> Done, TFunction<FString()> Text) -> TSharedRef<SWidget>
    {
        return SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)[ Dot(Done) ]
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Label(Text, 11, ERole::Text) ];
    };
    auto G = [this] { return Game.Get(); };
    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)
        [ Row([G] { return G() && !MarketCampaign::DebtOpen(G()->State); }, [] { return FString(TEXT("\u0130\u015fletmenin borcu kapans\u0131n")); }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)
        [ Row([G] { return G() && G()->State.Cash >= MarketCampaign::ExpandCashOn(G()->State.Day); },
              [G] { return FString::Printf(TEXT("Kasada %s  (\u015fu an %s)"), *MarketMenuUi::Tl(G() ? MarketCampaign::ExpandCashOn(G()->State.Day) : MarketCampaign::ExpandCash), G() ? *MarketMenuUi::Tl(G()->State.Cash) : TEXT("")); }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)
        [ Row([G] { return G() && G()->State.ProfitableDays >= MarketCampaign::ExpandProfitableDays; },
              [G] { return FString::Printf(TEXT("%d k\u00e2rl\u0131 g\u00fcn  (%d / %d)"), MarketCampaign::ExpandProfitableDays, G() ? G()->State.ProfitableDays : 0, MarketCampaign::ExpandProfitableDays); }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)
        [ Row([G] { return G() && G()->State.MarketShare >= MarketCampaign::ExpandShare; },
              [G] { return FString::Printf(TEXT("Yerel pay en az %%%.0f  (\u015fu an %%%.0f)"), MarketCampaign::ExpandShare, G() ? G()->State.MarketShare : 0.f); }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f).HAlign(HAlign_Left)
        [
            // G-074: every district and format is on the Subeler page (the old single "second shop" is a branch there).
            Button([G] { return G() && MarketBranches::OpenCount(G()->State) > 0 ? FString(TEXT("\u015eubelere bak")) : FString(TEXT("\u015eube a\u00e7\u0131l\u0131\u015f\u0131na bak")); },
                [this] { BranchTab = 0; bDecisionsOpen = false; Go(Branches); }, true)
        ];
}

TSharedRef<SWidget> SMarketMenu::DecisionCard()
{
    // G-066: the first choice waiting for the player (story scene or neighbourhood event), with its options.
    auto G = [this] { return Game.Get(); };
    auto Pending = [G]() -> const FMarketDecision* { return G() ? MarketEvents::Pending(G()->State) : nullptr; };
    TSharedRef<SHorizontalBox> Options = SNew(SHorizontalBox);
    for (int32 Option = 0; Option < 3; ++Option)
    {
        Options->AddSlot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([Pending, Option] { const FMarketDecision* D = Pending(); return D && D->Options.IsValidIndex(Option) ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Button([Pending, Option] { const FMarketDecision* D = Pending(); return D && D->Options.IsValidIndex(Option) ? D->Options[Option] : FString(); },
                [this, Option] { Manage(TEXT("Decide"), Option); }, Option == 0) ]
        ];
    }
    return SNew(SBox).Visibility_Lambda([Pending] { return Pending() ? EVisibility::Visible : EVisibility::Collapsed; }).Padding(FMargin(0.f, 0.f, 0.f, 12.f))
    [
        Card(SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ Label([Pending] { const FMarketDecision* D = Pending(); return D ? D->Title.ToUpper() : FString(); }, 9, ERole::Accent, true) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 10.f)[ Label([Pending] { const FMarketDecision* D = Pending(); return D ? D->Text : FString(); }, 12, ERole::Text, false, true) ]
            + SVerticalBox::Slot().AutoHeight()[ Options ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
            [ Label([G, Pending]
            {
                const FMarketDecision* D = Pending();
                if (!D || !G()) return FString();
                const int32 More = G()->State.Decisions.Num() - 1;
                return FString::Printf(TEXT("Se\u00e7mezsen %d. g\u00fcn\u00fcn sonunda \"%s\" ge\u00e7erli olur.%s"), D->Deadline,
                    D->Options.IsValidIndex(D->DefaultOption) ? *D->Options[D->DefaultOption] : TEXT(""), More > 0 ? *FString::Printf(TEXT(" S\u0131rada %d karar daha var."), More) : TEXT(""));
            }, 9, ERole::Muted, false, true) ])
    ];
}

TSharedRef<SWidget> SMarketMenu::StoryCard()
{
    // G-066: the chapter, its goals and the shop's identity.
    auto G = [this] { return Game.Get(); };
    TSharedRef<SVerticalBox> Goals = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 5; ++Slot)
    {
        auto Goal = [G, Slot](MarketStory::FObjective& Out)
        {
            if (!G()) return false;
            const TArray<MarketStory::FObjective> All = MarketStory::Objectives(G()->State);
            if (!All.IsValidIndex(Slot)) return false;
            Out = All[Slot];
            return true;
        };
        Goals->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SHorizontalBox).Visibility_Lambda([Goal] { MarketStory::FObjective O; return Goal(O) ? EVisibility::Visible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)[ Dot([Goal] { MarketStory::FObjective O; return Goal(O) && O.bDone; }) ]
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [ Label([Goal] { MarketStory::FObjective O; return Goal(O) ? O.Text + (O.bLater ? FString(TEXT(" (sonraki g\u00fcncelleme)")) : FString()) : FString(); }, 11, ERole::Text) ]
        ];
    }
    return Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [ Label([G] { return G() ? FString::Printf(TEXT("B\u00d6L\u00dcM %d \u00b7 %s"), G()->State.Story.Chapter, *MarketStory::ChapterTitle(G()->State.Story.Chapter).ToUpper()) : FString(); }, 9, ERole::Muted, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[ Goals ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [ Label([G]
        {
            if (!G()) return FString();
            const FMarketStoryState& Story = G()->State.Story;
            FString Text = FString::Printf(TEXT("Kimlik: %s"), *MarketStory::IdentityName(static_cast<MarketStory::EIdentity>(Story.Identity)));
            if (Story.Memories.Num() > 0) Text += TEXT(" \u00b7 son hat\u0131ra: ") + Story.Memories.Last();
            return Text;
        }, 10, ERole::Muted, false, true) ]);
}

TSharedRef<SWidget> SMarketMenu::TodoList()
{
    // G-074: what to do now (AMarketGameMode::Todos, most urgent first), each with a button to the page that solves it.
    auto G = [this] { return Game.Get(); };
    static constexpr int32 Shown = 6;
    auto TodoAt = [G](int32 Slot, FMarketTodo& Out)
    {
        if (!G()) return false;
        const TArray<FMarketTodo>& All = G()->Todos();
        if (!All.IsValidIndex(Slot)) return false;
        Out = All[Slot];
        return true;
    };
    TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < Shown; ++Slot)
    {
        Rows->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 8.f))
            .Visibility_Lambda([TodoAt, Slot] { FMarketTodo T; return TodoAt(Slot, T) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
                [
                    SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
                    [
                        SNew(SBorder).BorderImage(&BadgeBrush)
                        .BorderBackgroundColor(ColBy([TodoAt, Slot] { FMarketTodo T; if (!TodoAt(Slot, T)) return ERole::Line; return T.Severity >= 2 ? ERole::Bad : T.Severity == 1 ? ERole::Warn : ERole::Info; }))
                    ]
                ]
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Label([TodoAt, Slot] { FMarketTodo T; return TodoAt(Slot, T) ? T.Title : FString(); }, 11, ERole::Text, true, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ Label([TodoAt, Slot] { FMarketTodo T; return TodoAt(Slot, T) ? T.Text : FString(); }, 10, ERole::Muted, false, true) ]
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
                [
                    Button([TodoAt, Slot] { FMarketTodo T; return TodoAt(Slot, T) ? FString::Printf(TEXT("%s  \u203a"), PageName(T.Page)) : FString(); },
                        [this, TodoAt, Slot]
                        {
                            FMarketTodo T;
                            if (!TodoAt(Slot, T)) return;
                            if (AMarketGameMode* Mode = Game.Get(); Mode && Mode->Products.IsValidIndex(T.Product)) Mode->MenuProduct = T.Product;
                            if (T.Page != Summary) bDecisionsOpen = false; // G-086: the list lives in the decisions layer
                            Go(T.Page);
                        })
                ]
            ]
        ];
    }
    return Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f)[ Section(TEXT("\u015e\u0130MD\u0130 NE YAPMALI")) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ Label([G] { const int32 Count = G() ? G()->Todos().Num() : 0; return Count > Shown ? FString::Printf(TEXT("+%d daha"), Count - Shown) : FString(); }, 9, ERole::Muted) ]
        ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SBox).Visibility_Lambda([G] { return G() && G()->Todos().Num() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Fixed(TEXT("Acil bir i\u015f yok. Raflar dolu, kasa yolunda."), 11, ERole::Good) ]
        ]
        + SVerticalBox::Slot().AutoHeight()[ Rows ]);
}

TSharedRef<SWidget> SMarketMenu::GoalsCard()
{
    // C3 (B6, 06 \u00a72b): always a goal close to done, at three scales; the last celebrations under them.
    auto G = [this] { return Game.Get(); };
    auto GoalAt = [G](int32 Slot, MarketGoals::FGoalView& Out)
    {
        if (!G()) return false;
        const TArray<MarketGoals::FGoalView> All = MarketGoals::Goals(G()->State);
        if (!All.IsValidIndex(Slot)) return false;
        Out = All[Slot];
        return true;
    };
    static const TCHAR* Scales[3] = { TEXT("Bu hafta"), TEXT("Bu ay"), TEXT("Bu y\u0131l") };
    TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 3; ++Slot)
    {
        Rows->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(10.f, 6.f))
            .Visibility_Lambda([GoalAt, Slot] { MarketGoals::FGoalView V; return GoalAt(Slot, V) ? EVisibility::Visible : EVisibility::Collapsed; })
            .ToolTipText_Lambda([GoalAt, Slot] { MarketGoals::FGoalView V; return FText::FromString(GoalAt(Slot, V) ? V.Why + TEXT("\nBitince: ") + V.Reward : FString()); })
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().FillWidth(1.f)
                    [ Label([GoalAt, Slot]
                    {
                        MarketGoals::FGoalView V;
                        if (!GoalAt(Slot, V)) return FString();
                        // C5 (A menu list): "Bu hafta: Bu hafta ..." -> the title alone when it already names its scale.
                        const TCHAR* ScaleName = Scales[FMath::Clamp(static_cast<int32>(V.Scale), 0, 2)];
                        return V.Title.StartsWith(ScaleName) ? V.Title : FString::Printf(TEXT("%s: %s"), ScaleName, *V.Title);
                    }, 10, ERole::Text, true, true) ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
                    [ Label([GoalAt, Slot] { MarketGoals::FGoalView V; return GoalAt(Slot, V) ? FString::Printf(TEXT("%d g\u00fcn"), V.DaysLeft) : FString(); }, 9, ERole::Muted) ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
                [ Bar([GoalAt, Slot] { MarketGoals::FGoalView V; return GoalAt(Slot, V) ? FMath::Clamp(V.Progress, 0.f, 1.f) : 0.f; }, ERole::Accent) ]
            ]
        ];
    }
    return Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[ Section(TEXT("HEDEFLER")) ]
        + SVerticalBox::Slot().AutoHeight()[ Rows ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
        [ Label([G]
        {
            if (!G()) return FString();
            const TArray<FMarketCelebration> Last = MarketGoals::RecentCelebrations(G()->State, 2);
            TArray<FString> Lines;
            // C3 (A): only the last month's (an old one under a later year confuses); the dot is in the font, the star was not.
            for (const FMarketCelebration& C : Last)
                if (G()->State.Day - C.Day <= 30) Lines.Add(FString::Printf(TEXT("\u00b7 %s: %s %s"), *MarketCalendar::DateText(C.Day), *C.Title, *C.Text));
            return FString::Join(Lines, TEXT("\n"));
        }, 9, ERole::Good, false, true) ]);
}

TSharedRef<SWidget> SMarketMenu::RecordsView()
{
    // C3 (B6): Raporlar \u203a Rekorlar: the records and the last ten celebrations ("N. y\u0131l" dates).
    auto G = [this] { return Game.Get(); };
    return SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        // C7 (A menu comparison): before the first closed day one line instead of two empty cards.
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SBox).Visibility_Lambda([G] { return G() && G()->State.Day <= 1 ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Label([] { return FString(TEXT("\u0130lk g\u00fcn\u00fc kapat\u0131nca rekorlar ve kutlamalar burada birikir.")); }, 13, ERole::Muted, true) ]
        ]
        + SVerticalBox::Slot().AutoHeight()
        [ SNew(SBox).Visibility_Lambda([G] { return G() && G()->State.Day > 1 ? EVisibility::Visible : EVisibility::Collapsed; })
        [ Card(SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("REKORLAR")) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
            [ Label([G]
            {
                if (!G()) return FString();
                TArray<FString> Lines;
                for (const MarketGoals::FRecordView& R : MarketGoals::Records(G()->State)) Lines.Add(FString::Printf(TEXT("%s: %s"), *R.Name, *R.Value));
                return Lines.Num() > 0 ? FString::Join(Lines, TEXT("\n")) : FString(TEXT("Hen\u00fcz rekor yok: ilk iki haftadan sonra say\u0131l\u0131r."));
            }, 11, ERole::Text, false, true) ]) ] ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [ SNew(SBox).Visibility_Lambda([G] { return G() && G()->State.Day > 1 ? EVisibility::Visible : EVisibility::Collapsed; })
        [ Card(SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("KUTLAMALAR")) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
            [ Label([G]
            {
                if (!G()) return FString();
                TArray<FString> Lines;
                for (const FMarketCelebration& C : MarketGoals::RecentCelebrations(G()->State, 10))
                    Lines.Add(FString::Printf(TEXT("%s \u00b7 %s: %s"), *MarketCalendar::DateText(C.Day), *C.Title, *C.Text));
                return Lines.Num() > 0 ? FString::Join(Lines, TEXT("\n")) : FString(TEXT("Hen\u00fcz kutlama yok."));
            }, 10, ERole::Text, false, true) ]) ] ]
    ];
}

TSharedRef<SWidget> SMarketMenu::OrdersPage()
{
    auto G = [this] { return Game.Get(); };
    auto Cell = [this](TFunction<FString()> Make, float Width, ERole Role = ERole::Text, bool bBold = false) -> TSharedRef<SWidget>
    {
        return SNew(SBox).WidthOverride(Width).HAlign(HAlign_Right)[ Label(Make, 11, Role, bBold) ];
    };
    auto Head = [this](const FString& Text, float Width) -> TSharedRef<SWidget>
    {
        return SNew(SBox).WidthOverride(Width).HAlign(HAlign_Right)[ Fixed(Text, 9, ERole::Muted, true) ];
    };

    TSharedRef<SScrollBox> Rows = SNew(SScrollBox);
    const int32 Count = G() ? G()->Products.Num() : 0;
    for (int32 I = 0; I < Count; ++I)
    {
        auto Item = [G, I]() -> const FMarketStock* { return G() && G()->State.Stock.IsValidIndex(I) ? &G()->State.Stock[I] : nullptr; };
        Rows->AddSlot().Padding(0.f, 3.f)
        [
            SNew(SBox)
            .Visibility_Lambda([this, G, I] { return G() && G()->Products.IsValidIndex(I) && (OrderCategory.IsEmpty() || G()->Products[I].Category == OrderCategory) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Panel)).Padding(FMargin(12.f, 6.f))
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
                    [ ProductPicture(I, 32.f) ]
                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()[ Label([G, I] { return G() && G()->Products.IsValidIndex(I) ? G()->ProductName(I) : FString(); }, 11, ERole::Text, true) ]
                        + SVerticalBox::Slot().AutoHeight()
                        [ Label([G, I] { return G() && G()->Products.IsValidIndex(I) ? FString::Printf(TEXT("%s \u00b7 koli %d adet \u00b7 %s/adet"), *MarketMenuUi::Title(G()->Products[I].Category), MarketOrderAdvice::CaseUnits(G()->Products[I]), *MarketMenuUi::Tl(G()->Products[I].Cost)) : FString(); }, 9, ERole::Muted) ]
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::Printf(TEXT("%d/%d"), Item()->Shelf, Item()->Capacity) : FString(); }, 70.f) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::FromInt(Item()->Warehouse) : FString(); }, 66.f) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::FromInt(Item()->Dock) : FString(); }, 66.f) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::FromInt(Item()->Incoming) : FString(); }, 66.f) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::FromInt(Item()->Yesterday.Sold) : FString(); }, 66.f) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::FromInt(Item()->Yesterday.Empty) : FString(); }, 66.f, ERole::Warn) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [ Cell([G, I] { if (!G() || !G()->Products.IsValidIndex(I) || !G()->State.Stock.IsValidIndex(I)) return FString(); TArray<float> Scale; Scale.Init(1.f, G()->Products.Num()); Scale[I] = MarketDirector::OrderScale(G()->State, G()->Products[I]);
                        return FString::FromInt(MarketOrderAdvice::SuggestCases(G()->State, G()->Products, I, &Scale)); }, 60.f, ERole::Accent, true) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(18.f, 0.f, 0.f, 0.f)
                    [
                        SNew(SBox).WidthOverride(150.f).HAlign(HAlign_Right)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Button([] { return FString(TEXT("-")); }, [this, I] { Do(TEXT("RemoveOrder"), I); }, false,
                                [G, I] { return G() && G()->OrderDraftCases.IsValidIndex(I) && G()->OrderDraftCases[I] > 0; }) ]
                            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f)
                            [ SNew(SBox).WidthOverride(46.f).HAlign(HAlign_Center)
                                [ Label([G, I] { return G() && G()->OrderDraftCases.IsValidIndex(I) ? FString::Printf(TEXT("%d koli"), G()->OrderDraftCases[I]) : FString(TEXT("0 koli")); }, 11, ERole::Text, true) ] ]
                            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Button([] { return FString(TEXT("+")); }, [this, I] { Do(TEXT("Order"), I); }, true) ]
                        ]
                    ]
                ]
            ]
        ];
    }

    // G-063 wholesaler: who we buy from, terms, open bills and passing the monthly price rise on to the shelves.
    auto Act = [this](FName Action, int32 Arg) { if (AMarketGameMode* Mode = Game.Get()) Mode->StaffCommand(Action, Arg); };
    return SNew(SVerticalBox)
        // G-086e: the cards above and below the list have fixed heights, so a new message or a longer total never
        // pushes the list around.
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
        [
            SNew(SBox).HeightOverride(64.f)
            [
            Card(
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [ Label([G] { return G() ? MarketSuppliers::Summary(G()->State) : FString(); }, 11, ERole::Text, false, true) ]
                // C5 (A menu list): each button only while it has something to do (day one: none of them).
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                [
                    SNew(SBox).Visibility_Lambda([G] { return G() && (G()->State.Supplier != 0 || MarketSuppliers::Available(G()->State, MarketSuppliers::ESupplier::CashCarry)) ? EVisibility::Visible : EVisibility::Collapsed; })
                    [ Button([G] { return FString(G() && G()->State.Supplier == 0 ? TEXT("Ucuz toptanc\u0131ya ge\u00e7") : TEXT("Eski toptanc\u0131ya d\u00f6n")); },
                        [Act, G] { if (G()) Act(TEXT("Supplier"), G()->State.Supplier == 0 ? 1 : 0); }) ]
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                [
                    SNew(SBox).Visibility_Lambda([G] { return G() && MarketSuppliers::OpenBills(G()->State) > 0 ? EVisibility::Visible : EVisibility::Collapsed; })
                    [ Button([] { return FString(TEXT("Faturalar\u0131 \u00f6de")); }, [Act] { Act(TEXT("PayBills"), 0); }) ]
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                [
                    SNew(SBox).Visibility_Lambda([G] { return G() && MarketSuppliers::PriceGap(G()->State) >= 0.005 ? EVisibility::Visible : EVisibility::Collapsed; })
                    [ Button([] { return FString(TEXT("Zamm\u0131 yans\u0131t")); }, [Act] { Act(TEXT("PassOnPriceRise"), 0); }) ]
                ],
                ERole::Panel, FMargin(18.f, 10.f))
            ]
        ]
        + SVerticalBox::Slot().AutoHeight()[ CategoryChips(&OrderCategory) ]
        + SVerticalBox::Slot().AutoHeight().Padding(12.f, 14.f, 12.f, 4.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f)[ Fixed(TEXT("\u00dcR\u00dcN"), 9, ERole::Muted, true) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("RAF"), 70.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("DEPO"), 66.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("KABUL"), 66.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("YOLDA"), 66.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("D\u00dcN SAT."), 66.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("BO\u015e RAF"), 66.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("\u00d6NER\u0130"), 60.f) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(18.f, 0.f, 0.f, 0.f)[ Head(TEXT("L\u0130STE"), 150.f) ]
        ]
        + SVerticalBox::Slot().FillHeight(1.f)[ Rows ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
        [
            SNew(SBox).HeightOverride(76.f)
            [
            Card(
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [ Label([G] { return G() ? FString::Printf(TEXT("Liste: %d koli \u00b7 %s"), G()->OrderDraftCaseCount(), *MarketMenuUi::Tl(G()->OrderDraftBill())) : FString(); }, 15, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
                    [
                        LabelBy([G]
                        {
                            if (!G()) return FString();
                            if (G()->bTestMode) return FString(TEXT("TEST MODU: + bedava ve an\u0131nda depoya getirir."));
                            if (G()->OrderDraftCaseCount() > 0 && G()->OrderDraftBill() < MarketOrderAdvice::MinimumOrderOn(G()->State.Day))
                                return FString::Printf(TEXT("Toptanc\u0131 en az %s sipari\u015fle gelir."), *MarketMenuUi::Tl(MarketOrderAdvice::MinimumOrderOn(G()->State.Day)));
                            if (G()->OrderDraftBill() > G()->State.Cash) return FString(TEXT("Kasadaki nakit bu listeye yetmiyor."));
                            return FString(TEXT("\u00d6deme onayda yap\u0131l\u0131r; koliler yar\u0131n sabah arka kap\u0131da."));
                        }, 10, [G] { return G() && (G()->OrderDraftBill() > G()->State.Cash || (G()->OrderDraftCaseCount() > 0 && G()->OrderDraftBill() < MarketOrderAdvice::MinimumOrderOn(G()->State.Day))) ? ERole::Warn : ERole::Muted; })
                    ]
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
                [ Button([] { return FString(TEXT("Temizle")); }, [this] { if (AMarketGameMode* M = Game.Get()) M->ClearOrderDraft(); }, false,
                    [G] { return G() && G()->OrderDraftCaseCount() > 0; }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
                [ Button([] { return FString(TEXT("\u00d6neriyi yaz")); }, [this] { Do(TEXT("SuggestOrder")); bOrderListOpen = true; }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
                [ Button([] { return FString(TEXT("Listeyi a\u00e7")); }, [this] { bOrderListOpen = true; }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [ Button([] { return FString(TEXT("Sipari\u015fi onayla")); }, [this] { Do(TEXT("ConfirmOrder")); }, true,
                    [G] { return G() && G()->OrderDraftCaseCount() > 0; }) ],
                ERole::Panel, FMargin(18.f, 12.f))
            ]
        ];
}

TSharedRef<SWidget> SMarketMenu::DayReport()
{
    auto G = [this] { return Game.Get(); };
    TSharedRef<SVerticalBox> Problems = SNew(SVerticalBox);
    for (int32 Index = 0; Index < 3; ++Index)
    {
        auto Problem = [G, Index](MarketDemand::FProblem& Out)
        {
            if (!G()) return false;
            const TArray<MarketDemand::FProblem> All = MarketDemand::TopProblems(G()->State, 3);
            if (!All.IsValidIndex(Index)) return false;
            Out = All[Index];
            return true;
        };
        Problems->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 8.f))
            .Visibility_Lambda([Problem] { MarketDemand::FProblem P; return Problem(P) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [ Label([G, Problem] { MarketDemand::FProblem P; return Problem(P) ? MarketMenuUi::ProblemText(*G(), P) : FString(); }, 11, ERole::Text, false, true) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
                [
                    Button([Problem]
                    {
                        MarketDemand::FProblem P;
                        if (!Problem(P)) return FString();
                        switch (P.Kind)
                        {
                        case MarketDemand::EProblem::Empty: return FString(TEXT("Sipari\u015f ver"));
                        case MarketDemand::EProblem::Expensive: return FString(TEXT("Fiyata bak"));
                        case MarketDemand::EProblem::Waiting: return FString(TEXT("Personel"));
                        default: return FString(TEXT("\u00dcr\u00fcne bak"));
                        }
                    }, [this, Problem]
                    {
                        MarketDemand::FProblem P;
                        if (!Problem(P)) return;
                        if (AMarketGameMode* M = Game.Get(); M && M->Products.IsValidIndex(P.Product)) M->MenuProduct = P.Product;
                        Go(P.Kind == MarketDemand::EProblem::Empty ? Orders : P.Kind == MarketDemand::EProblem::Waiting ? Staff : Prices);
                    })
                ]
            ]
        ];
    }

    return SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ DecisionCard() ]
        + SVerticalBox::Slot().AutoHeight()
        [
            Label([G] { return G() && G()->State.Day > 1 ? FString::Printf(TEXT("%d. g\u00fcn kapand\u0131"), G()->State.Day - 1) : FString(TEXT("\u0130lk g\u00fcn\u00fc kapat\u0131nca burada g\u00fcn\u00fcn raporu g\u00f6r\u00fcn\u00fcr.")); }, 13, ERole::Muted, true)
        ]
        // C5 (A menu list): before the first closed day only the line above (no grid of zeros).
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            .Visibility_Lambda([G] { return G() && G()->State.Day > 1 ? EVisibility::Visible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            // C7 (A menu comparison): the net covers the whole network, revenue and goods only the family shop.
            [ Stat(TEXT("NET SONU\u00c7 \u00b7 A\u011e DAH\u0130L"), [G] { return G() ? MarketMenuUi::Tl(G()->State.LastProfit) : FString(); },
                   [G] { return G() ? FString::Printf(TEXT("%d k\u00e2rl\u0131 g\u00fcn"), G()->State.ProfitableDays) : FString(); },
                   [G] { return G() && G()->State.LastProfit < 0 ? ERole::Bad : ERole::Good; }) ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [ Stat(TEXT("C\u0130RO \u00b7 A\u0130LE D\u00dcKK\u00c2NI"), [G] { return G() ? MarketMenuUi::Tl(G()->State.LastRevenue) : FString(); },
                   [G] { return G() ? FString::Printf(TEXT("%d sat\u0131\u015f \u00b7 %d kay\u0131p"), G()->State.LastServed, G()->State.LastLost) : FString(); }) ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [ Stat(TEXT("MAL MAL\u0130YET\u0130 \u00b7 A\u0130LE"), [G] { return G() ? MarketMenuUi::Tl(G()->State.LastCostOfGoods) : FString(); }, [] { return FString(TEXT("sat\u0131lan \u00fcr\u00fcnlerin al\u0131\u015f\u0131")); }) ]
            + SHorizontalBox::Slot().FillWidth(1.f)
            [ Stat(TEXT("G\u0130DER"), [G] { return G() ? MarketMenuUi::Tl(G()->State.LastOperatingCost) : FString(); },
                   [G] { return G() && (G()->State.bSecondStore || MarketBranches::OpenCount(G()->State) > 0) ? FString::Printf(TEXT("\u015fubeler %s"), *MarketMenuUi::Tl(G()->State.LastBranchProfit)) : FString(TEXT("kira, elektrik, maa\u015f")); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            .Visibility_Lambda([G] { return G() && G()->State.Day > 1 ? EVisibility::Visible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().FillWidth(1.4f).Padding(0.f, 0.f, 12.f, 0.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Fixed(TEXT("NEREDE M\u00dc\u015eTER\u0130 KAYBETT\u0130N"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        // C5 (A menu list): "everyone found it" only when there were shoppers.
                        SNew(SBox).Visibility_Lambda([G] { return G() && G()->State.LastServed > 0 && MarketDemand::TopProblems(G()->State, 1).Num() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
                        [ Fixed(TEXT("Kay\u0131p m\u00fc\u015fteri yok. Herkes arad\u0131\u011f\u0131n\u0131 buldu."), 11, ERole::Good) ]
                    ]
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        SNew(SBox).Visibility_Lambda([G] { return G() && G()->State.LastServed <= 0 && MarketDemand::TopProblems(G()->State, 1).Num() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
                        [ Fixed(TEXT("D\u00fcn sat\u0131\u015f olmad\u0131."), 11, ERole::Muted) ]
                    ]
                    + SVerticalBox::Slot().AutoHeight()[ Problems ])
            ]
            + SHorizontalBox::Slot().FillWidth(1.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Fixed(TEXT("YARIN RAK\u0130PLERDE"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ Label([G] { return G() ? G()->RivalNewsText() : FString(); }, 11, ERole::Text, false, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 6.f)[ Fixed(TEXT("MAL KABUL VE BOR\u00c7"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        Label([G]
                        {
                            if (!G()) return FString();
                            FString Text = G()->State.DeliveryUnits() > 0 ? FString::Printf(TEXT("%d \u00fcr\u00fcn arka kap\u0131da, depoya ta\u015f\u0131nmal\u0131."), G()->State.DeliveryUnits()) : FString(TEXT("Arka kap\u0131da bekleyen koli yok."));
                            if (G()->State.LastDeliveryMissing + G()->State.LastDeliveryDamaged > 0)
                                Text += FString::Printf(TEXT("\nTedarik sorunu: %d eksik, %d hasarl\u0131."), G()->State.LastDeliveryMissing, G()->State.LastDeliveryDamaged);
                            Text += MarketCampaign::DebtOpen(G()->State) ? FString::Printf(TEXT("\nKalan bor\u00e7 %s."), *MarketMenuUi::Tl(G()->State.InheritedDebt)) : FString(TEXT("\nBor\u00e7 kapand\u0131."));
                            return Text;
                        }, 11, ERole::Text, false, true)
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 6.f)[ Fixed(TEXT("\u0130\u015eLETME: TEDAR\u0130K, PERSONEL, VERG\u0130"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ Label([G] { const FString Text = G() ? MarketDirector::ReportText(G()->State) : FString(); return Text.IsEmpty() ? FString(TEXT("Olay yok.")) : Text; }, 11, ERole::Text, false, true) ])
            ]
        ]
    ];
}

TSharedRef<SWidget> SMarketMenu::WeekReport()
{
    auto G = [this] { return Game.Get(); };
    // Last 7 closed days from the history (oldest left).
    auto DayAt = [G](int32 Slot, FMarketDayRecord& Out)
    {
        if (!G()) return false;
        const TArray<FMarketDayRecord>& History = G()->State.History;
        const int32 Index = History.Num() - 7 + Slot;
        if (!History.IsValidIndex(Index)) return false;
        Out = History[Index];
        return true;
    };
    auto Peak = [G]
    {
        int64 Max = 1;
        if (G())
            for (int32 I = FMath::Max(0, G()->State.History.Num() - 7); I < G()->State.History.Num(); ++I) Max = FMath::Max(Max, FMath::Abs(G()->State.History[I].Profit));
        return Max;
    };
    TSharedRef<SHorizontalBox> Bars = SNew(SHorizontalBox);
    for (int32 Slot = 0; Slot < 7; ++Slot)
    {
        Bars->AddSlot().FillWidth(1.f).Padding(6.f, 0.f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
            [ Label([DayAt, Slot] { FMarketDayRecord R; return DayAt(Slot, R) ? MarketCatalog::Money(R.Profit) : FString(); }, 9, ERole::Muted) ]
            // C9 (Codex C8: losses grew upwards like profits): a signed axis, profits above the zero line, losses below.
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
            [
                SNew(SBox).HeightOverride(75.f).VAlign(VAlign_Bottom)
                [
                    SNew(SBox)
                    .HeightOverride_Lambda([DayAt, Peak, Slot]() -> FOptionalSize { FMarketDayRecord R; return DayAt(Slot, R) && R.Profit > 0 ? FMath::Max(3.f, 75.f * static_cast<float>(R.Profit) / static_cast<float>(Peak())) : 0.f; })
                    [ SNew(SBorder).BorderImage(&BadgeBrush).BorderBackgroundColor(Col(ERole::Accent)) ]
                ]
            ]
            + SVerticalBox::Slot().AutoHeight()
            [ SNew(SBox).HeightOverride(2.f)[ SNew(SBorder).BorderImage(&FlatBrush).BorderBackgroundColor(Col(ERole::Line)) ] ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
            [
                SNew(SBox).HeightOverride(75.f).VAlign(VAlign_Top)
                [
                    SNew(SBox)
                    .HeightOverride_Lambda([DayAt, Peak, Slot]() -> FOptionalSize { FMarketDayRecord R; return DayAt(Slot, R) && R.Profit < 0 ? FMath::Max(3.f, 75.f * static_cast<float>(-R.Profit) / static_cast<float>(Peak())) : 0.f; })
                    [ SNew(SBorder).BorderImage(&BadgeBrush).BorderBackgroundColor(Col(ERole::Bad)) ]
                ]
            ]
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
            [ Label([DayAt, Slot] { FMarketDayRecord R; return DayAt(Slot, R) ? FString::Printf(TEXT("%d. g\u00fcn"), R.Day) : FString(TEXT("\u2014")); }, 10, ERole::Text, true) ]
        ];
    }

    return SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            Label([G] { return G() && G()->State.LastWeekNumber > 0 ? FString::Printf(TEXT("%d. hafta"), G()->State.LastWeekNumber) : (G() && G()->State.History.Num() > 0 ? FString(TEXT("\u0130lk hafta raporu 7. g\u00fcn\u00fcn sonunda gelir. Grafik \u015fimdiden son g\u00fcnleri g\u00f6sterir."))
                : FString(TEXT("\u0130lk g\u00fcn\u00fc kapat\u0131nca burada son g\u00fcnlerin grafi\u011fi g\u00f6r\u00fcn\u00fcr; hafta raporu 7. g\u00fcn\u00fcn sonunda gelir."))); }, 13, ERole::Muted, true, true)
        ]
        // C5 (A menu list): the week's numbers once a week has closed, the chart once a day has.
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            .Visibility_Lambda([G] { return G() && G()->State.LastWeekNumber > 0 ? EVisibility::Visible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [ Stat(TEXT("HAFTANIN NET\u0130"), [G] { return G() ? MarketMenuUi::Tl(G()->State.LastWeekProfit) : FString(); }, [G] { return G() ? FString::Printf(TEXT("ciro %s"), *MarketMenuUi::Tl(G()->State.LastWeekRevenue)) : FString(); },
                   [G] { return G() && G()->State.LastWeekProfit < 0 ? ERole::Bad : ERole::Good; }) ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [ Stat(TEXT("M\u00dc\u015eTER\u0130"), [G] { return G() ? FString::Printf(TEXT("%d sat\u0131\u015f"), G()->State.LastWeekServed) : FString(); }, [G] { return G() ? FString::Printf(TEXT("%d kay\u0131p m\u00fc\u015fteri"), G()->State.LastWeekLost) : FString(); }) ]
            + SHorizontalBox::Slot().FillWidth(1.f)
            [ Stat(TEXT("\u00d6DENEN BOR\u00c7"), [G] { return G() ? MarketMenuUi::Tl(G()->State.LastWeekDebtPaid) : FString(); },
                   [G] { return !G() ? FString() : MarketCampaign::DebtOpen(G()->State) ? FString::Printf(TEXT("kalan %s"), *MarketMenuUi::Tl(G()->State.InheritedDebt)) : FString::Printf(TEXT("%d. g\u00fcnde kapand\u0131"), G()->State.DebtClearedDay); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([G] { return G() && G()->State.History.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed; })
            [
            Card(SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[ Fixed(TEXT("G\u00dcNL\u00dcK NET \u00b7 SON 7 G\u00dcN"), 9, ERole::Muted, true) ]
                + SVerticalBox::Slot().AutoHeight()[ Bars ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
                [
                    Label([DayAt]
                    {
                        FMarketDayRecord First, Last;
                        int32 A = 0;
                        while (A < 7 && !DayAt(A, First)) ++A;
                        if (A >= 7 || !DayAt(6, Last)) return FString();
                        return FString::Printf(TEXT("Yerel pay: ba\u015fta %%%.0f, sonda %%%.0f  \u00b7  kasa: ba\u015fta %s, sonda %s"), First.MarketShare, Last.MarketShare, *MarketMenuUi::Tl(First.Cash), *MarketMenuUi::Tl(Last.Cash));
                    }, 10, ERole::Muted)
                ])
            ]
        ]
    ];
}

TSharedRef<SWidget> SMarketMenu::ReportsPage()
{
    auto G = [this] { return Game.Get(); };
    auto Tab = [this](const FString& Text, bool bWeek) -> TSharedRef<SWidget>
    {
        return SNew(SButton).ButtonStyle(&PillStyle).IsFocusable(false).ContentPadding(FMargin(16.f, 6.f))
            .ButtonColorAndOpacity_Lambda([this, bWeek] { return FSlateColor(Color(!bRecordsTab && bWeekTab == bWeek ? ERole::Primary : ERole::Button)); })
            .OnClicked_Lambda([this, bWeek] { bWeekTab = bWeek; bRecordsTab = false; return FReply::Handled(); })
            [
                SNew(STextBlock).Font(MarketMenuUi::MenuFont(true, 11)).Text(FText::FromString(Text))
                .ColorAndOpacity_Lambda([this, bWeek] { return FSlateColor(Color(!bRecordsTab && bWeekTab == bWeek ? ERole::PrimaryText : ERole::ButtonText)); })
            ];
    };
    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[ Tab(TEXT("G\u00fcn sonu"), false) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[ Tab(TEXT("Hafta"), true) ]
            + SHorizontalBox::Slot().AutoWidth()
            [
                SNew(SButton).ButtonStyle(&PillStyle).IsFocusable(false).ContentPadding(FMargin(16.f, 6.f))
                .ButtonColorAndOpacity_Lambda([this] { return FSlateColor(Color(bRecordsTab ? ERole::Primary : ERole::Button)); })
                .OnClicked_Lambda([this] { bRecordsTab = true; return FReply::Handled(); })
                [
                    SNew(STextBlock).Font(MarketMenuUi::MenuFont(true, 11)).Text(FText::FromString(TEXT("Rekorlar")))
                    .ColorAndOpacity_Lambda([this] { return FSlateColor(Color(bRecordsTab ? ERole::PrimaryText : ERole::ButtonText)); })
                ]
            ]
        ]
        + SVerticalBox::Slot().FillHeight(1.f)
        [
            SNew(SWidgetSwitcher).WidgetIndex_Lambda([this] { return bRecordsTab ? 2 : bWeekTab ? 1 : 0; })
            + SWidgetSwitcher::Slot()[ DayReport() ]
            + SWidgetSwitcher::Slot()[ WeekReport() ]
            + SWidgetSwitcher::Slot()[ RecordsView() ]
        ]
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 10.f, 0.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([G] { return G() && G()->bMenuDayReport ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Button([] { return FString(TEXT("Yeni g\u00fcne ba\u015fla")); }, [this] { if (AMarketGameMode* M = Game.Get()) M->CloseMenu(); }, true) ]
        ];
}

void SMarketMenu::ShowWeek(bool bWeek)
{
    bWeekTab = bWeek;
    bRecordsTab = false;
}

// ---------------------------------------------------------------------------------------------------------------
// G-086f: the order list as a window. What is on the list, the wholesaler, each line's product and cases; a
// product can be swapped for another (its cases move over) or added from the picker on the right.

TSharedRef<SWidget> SMarketMenu::OrderListLayer()
{
    auto G = [this] { return Game.Get(); };
    auto Muted = [] { return ERole::Muted; };
    auto Plain = [] { return ERole::Text; };
    auto Cases = [G](int32 I) { return G() && G()->OrderDraftCases.IsValidIndex(I) ? G()->OrderDraftCases[I] : 0; };
    auto CaseUnits = [G](int32 I) { return G() && G()->Products.IsValidIndex(I) ? FMath::Clamp(G()->Products[I].CaseUnits, 1, 48) : 1; };
    auto Ease = [this] { const float T = OrderAnim; return T * T * (3.f - 2.f * T); };
    auto Small = [this](TFunction<FString()> Text, TFunction<void()> OnClick, TFunction<bool()> Enabled, TFunction<bool()> On) -> TSharedRef<SWidget>
    {
        return SNew(SBox).HeightOverride(28.f)
        [
            SNew(SButton).ButtonStyle(&RoundStyle).IsFocusable(false).ContentPadding(FMargin(12.f, 0.f)).VAlign(VAlign_Center)
            .ButtonColorAndOpacity_Lambda([this, On] { return FSlateColor(Color(On && On() ? ERole::Primary : ERole::Button)); })
            .IsEnabled_Lambda([Enabled] { return !Enabled || Enabled(); })
            .OnClicked_Lambda([OnClick] { OnClick(); return FReply::Handled(); })
            [
                SNew(STextBlock).Font(MarketTheme::Font(MarketTheme::EFace::Semi, 12.f))
                .Text_Lambda([Text] { return FText::FromString(Text()); })
                .ColorAndOpacity_Lambda([this, On] { return FSlateColor(Color(On && On() ? ERole::PrimaryText : ERole::ButtonText)); })
            ]
        ];
    };

    // The wholesaler.
    TSharedRef<SHorizontalBox> Suppliers = SNew(SHorizontalBox);
    for (int32 S = 0; S < static_cast<int32>(MarketSuppliers::ESupplier::Count); ++S)
    {
        const MarketSuppliers::ESupplier Which = static_cast<MarketSuppliers::ESupplier>(S);
        const MarketSuppliers::FInfo& About = MarketSuppliers::Info(Which);
        const FString Name = About.Name;
        const FString Cheaper = About.BaseDiscount > 0.f ? FString::Printf(TEXT("%%%.0f ucuz \u00b7 "), About.BaseDiscount * 100.f) : FString();
        const FString Note = Cheaper + (About.bOffersTerms ? TEXT("vadeli yazar, mal sa\u011flam") : TEXT("vade yok, mal eksik gelebilir"));
        Suppliers->AddSlot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
        [
            SNew(SBox).ToolTip(Tip([Note] { return Note; }))
            [ Small([Name] { return Name; }, [this, S] { Manage(TEXT("Supplier"), S); },
                [G, Which] { return G() && MarketSuppliers::Available(G()->State, Which); },
                [G, S] { return G() && G()->State.Supplier == S; }) ]
        ];
    }

    // The lines of the list (every product has a row; the empty ones hide).
    TSharedRef<SVerticalBox> Lines = SNew(SVerticalBox);
    const int32 Count = G() ? G()->Products.Num() : 0;
    for (int32 I = 0; I < Count; ++I)
    {
        auto Swapping = [this, I] { return OrderSwapFrom == I; };
        Lines->AddSlot().AutoHeight()
        [
            SNew(SBox).Visibility_Lambda([Cases, I] { return Cases(I) > 0 ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SBorder).BorderImage(&TileBrush).Padding(FMargin(10.f, 8.f))
                .BorderBackgroundColor_Lambda([this, Swapping] { return FSlateColor(Swapping() ? Color(ERole::AccentSoft) : FLinearColor::Transparent); })
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)[ ProductPicture(I, 34.f) ]
                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()[ TextPx([G, I] { return G() ? G()->ProductName(I) : FString(); }, 13.f, Plain, true) ]
                        + SVerticalBox::Slot().AutoHeight()
                        [ TextPx([G, I, CaseUnits] { return G() && G()->Products.IsValidIndex(I) ? FString::Printf(TEXT("koli %d adet \u00b7 %s/adet"), CaseUnits(I), *MarketMenuUi::Tl(G()->Products[I].Cost)) : FString(); }, 11.f, Muted) ]
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f)
                    [ Small([Swapping] { return FString(Swapping() ? TEXT("Se\u00e7iliyor\u2026") : TEXT("De\u011fi\u015ftir")); },
                        [this, I] { OrderSwapFrom = OrderSwapFrom == I ? INDEX_NONE : I; if (OrderSwapFrom != INDEX_NONE) { if (const AMarketGameMode* Mode = Game.Get()) OrderPickCategory = Mode->Products[I].Category; } },
                        nullptr, Swapping) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Small([] { return FString(TEXT("\u2212")); }, [this, I] { Do(TEXT("RemoveOrder"), I); }, nullptr, nullptr) ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                        [ SNew(SBox).WidthOverride(58.f).HAlign(HAlign_Center)[ Mono([Cases, I] { return FString::Printf(TEXT("%d koli"), Cases(I)); }, 13.f, Plain) ] ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Small([] { return FString(TEXT("+")); }, [this, I] { Do(TEXT("Order"), I); }, nullptr, nullptr) ]
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [
                        SNew(SBox).WidthOverride(110.f).HAlign(HAlign_Right)
                        [ Mono([G, I, Cases, CaseUnits] { return G() && G()->Products.IsValidIndex(I) ? MarketMenuUi::Tl(static_cast<int64>(Cases(I)) * CaseUnits(I) * G()->Products[I].Cost) : FString(); }, 13.f, Plain) ]
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                    [
                        SNew(SBox).WidthOverride(28.f).HeightOverride(28.f).ToolTip(Tip([] { return FString(TEXT("Sat\u0131r\u0131 sil")); }))
                        [
                            SNew(SButton).ButtonStyle(&RoundStyle).IsFocusable(false).ContentPadding(FMargin(0.f)).HAlign(HAlign_Center).VAlign(VAlign_Center)
                            .ButtonColorAndOpacity(Col(ERole::Button))
                            .OnClicked_Lambda([this, I] { if (AMarketGameMode* Mode = Game.Get()) Mode->ClearOrderLine(I); if (OrderSwapFrom == I) OrderSwapFrom = INDEX_NONE; return FReply::Handled(); })
                            [ IconImage(TEXT("close"), 12.f, Muted) ]
                        ]
                    ]
                ]
            ]
        ];
    }
    Lines->AddSlot().AutoHeight().Padding(10.f, 16.f)
    [
        SNew(SBox).Visibility_Lambda([G] { return G() && G()->OrderDraftCaseCount() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
        [ TextPx([] { return FString(TEXT("Liste bo\u015f. Sa\u011fdan \u00fcr\u00fcn ekle ya da \"\u00d6neriyi yaz\".")); }, 13.f, Muted) ]
    ];

    // The picker: add a product, or (after "De\u011fi\u015ftir") the product to move a line's cases to.
    TSharedRef<SVerticalBox> Pick = SNew(SVerticalBox);
    for (int32 I = 0; I < Count; ++I)
    {
        Pick->AddSlot().AutoHeight().Padding(0.f, 2.f)
        [
            SNew(SBox).Visibility_Lambda([this, G, I]
            {
                if (!G() || !G()->Products.IsValidIndex(I) || I == OrderSwapFrom) return EVisibility::Collapsed;
                return OrderPickCategory.IsEmpty() || G()->Products[I].Category == OrderPickCategory ? EVisibility::Visible : EVisibility::Collapsed;
            })
            [
                SNew(SButton).ButtonStyle(&RowStyle).IsFocusable(false).ContentPadding(FMargin(8.f, 6.f))
                .ButtonColorAndOpacity(Col(ERole::Panel))
                .OnClicked_Lambda([this, I]
                {
                    AMarketGameMode* Mode = Game.Get();
                    if (!Mode) return FReply::Handled();
                    if (OrderSwapFrom != INDEX_NONE) { Mode->MoveOrderDraft(OrderSwapFrom, I); OrderSwapFrom = INDEX_NONE; }
                    else Do(TEXT("Order"), I);
                    return FReply::Handled();
                })
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)[ ProductPicture(I, 26.f) ]
                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ TextPx([G, I] { return G() ? G()->ProductName(I) : FString(); }, 12.f, Plain) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [ Mono([G, I] { return G() && G()->State.Stock.IsValidIndex(I) ? FString::Printf(TEXT("raf %d"), G()->State.Stock[I].Shelf) : FString(); }, 11.f, Muted) ]
                ]
            ]
        ];
    }

    auto Warning = [G]() -> FString
    {
        if (!G()) return FString();
        if (G()->bTestMode) return FString(TEXT("TEST MODU: + bedava ve an\u0131nda depoya getirir."));
        if (G()->OrderDraftCaseCount() > 0 && G()->OrderDraftBill() < MarketOrderAdvice::MinimumOrderOn(G()->State.Day))
            return FString::Printf(TEXT("Toptanc\u0131 en az %s sipari\u015fle gelir."), *MarketMenuUi::Tl(MarketOrderAdvice::MinimumOrderOn(G()->State.Day)));
        if (G()->OrderDraftBill() > G()->State.Cash) return FString(TEXT("Kasadaki nakit bu listeye yetmiyor."));
        return FString(TEXT("\u00d6deme onayda yap\u0131l\u0131r; koliler yar\u0131n sabah arka kap\u0131da."));
    };
    auto Close = [this] { bOrderListOpen = false; OrderSwapFrom = INDEX_NONE; };

    TSharedRef<SWidget> Window = SNew(SBorder).BorderImage(&CardBrush).BorderBackgroundColor(Col(ERole::Sheet)).Padding(FMargin(26.f, 22.f))
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ Display([] { return FString(TEXT("Sipari\u015f listesi")); }, 26.f) ]
                + SVerticalBox::Slot().AutoHeight()
                [ TextPx([G] { return G() ? FString::Printf(TEXT("%d koli \u00b7 %s \u00b7 kasada %s"), G()->OrderDraftCaseCount(), *MarketMenuUi::Tl(G()->OrderDraftBill()), *MarketMenuUi::Tl(G()->State.Cash)) : FString(); }, 13.f, Muted) ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)
            [
                SNew(SBox).WidthOverride(36.f).HeightOverride(36.f).ToolTip(Tip([] { return FString(TEXT("Kapat (Esc)")); }))
                [
                    SNew(SButton).ButtonStyle(&RoundStyle).IsFocusable(false).ContentPadding(FMargin(0.f)).HAlign(HAlign_Center).VAlign(VAlign_Center)
                    .ButtonColorAndOpacity(Col(ERole::Inset))
                    .OnClicked_Lambda([Close] { Close(); return FReply::Handled(); })
                    [ IconImage(TEXT("close"), 16.f, Plain) ]
                ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 12.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)[ TextPx([] { return FString(TEXT("Toptanc\u0131")); }, 13.f, Muted, true) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Suppliers ]
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
            [ TextPx([G] { return G() ? MarketSuppliers::Summary(G()->State) : FString(); }, 11.f, Muted) ]
        ]
        + SVerticalBox::Slot().FillHeight(1.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 16.f, 0.f)
            [ SNew(SScrollBox) + SScrollBox::Slot()[ Lines ] ]
            + SHorizontalBox::Slot().AutoWidth()
            [
                SNew(SBox).WidthOverride(320.f)
                [
                    SNew(SBorder).BorderImage(&TileBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f))
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                            [ TextPx([this, G] { return OrderSwapFrom != INDEX_NONE && G() ? TEXT("Yerine: ") + G()->ProductName(OrderSwapFrom) : FString(TEXT("\u00dcr\u00fcn ekle")); }, 13.f, Plain, true) ]
                            + SHorizontalBox::Slot().AutoWidth()
                            [ SNew(SBox).Visibility_Lambda([this] { return OrderSwapFrom != INDEX_NONE ? EVisibility::Visible : EVisibility::Collapsed; })
                                [ Small([] { return FString(TEXT("Vazge\u00e7")); }, [this] { OrderSwapFrom = INDEX_NONE; }, nullptr, nullptr) ] ]
                        ]
                        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[ CategoryChips(&OrderPickCategory) ]
                        + SVerticalBox::Slot().FillHeight(1.f)[ SNew(SScrollBox) + SScrollBox::Slot()[ Pick ] ]
                    ]
                ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [ TextPx(Warning, 12.f, [G] { return G() && (G()->OrderDraftBill() > G()->State.Cash || (G()->OrderDraftCaseCount() > 0 && G()->OrderDraftBill() < MarketOrderAdvice::MinimumOrderOn(G()->State.Day))) ? ERole::Warn : ERole::Muted; }) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
            [ Button([] { return FString(TEXT("Temizle")); }, [this] { if (AMarketGameMode* M = Game.Get()) M->ClearOrderDraft(); OrderSwapFrom = INDEX_NONE; }, false, [G] { return G() && G()->OrderDraftCaseCount() > 0; }) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
            [ Button([] { return FString(TEXT("\u00d6neriyi yaz")); }, [this] { Do(TEXT("SuggestOrder")); }) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ Button([] { return FString(TEXT("Sipari\u015fi onayla")); }, [this, G, Close] { Do(TEXT("ConfirmOrder")); if (G() && G()->OrderDraftCaseCount() == 0) Close(); }, true,
                [G] { return G() && G()->OrderDraftCaseCount() > 0; }) ]
        ]
    ];

    return SNew(SOverlay)
        .Visibility_Lambda([this] { return OrderAnim > 0.01f ? (bOrderListOpen ? EVisibility::Visible : EVisibility::HitTestInvisible) : EVisibility::Collapsed; })
        + SOverlay::Slot()
        [
            SNew(SBorder).BorderImage(&FlatBrush)
            .BorderBackgroundColor_Lambda([this, Ease] { FLinearColor Shade = Color(ERole::Dim); Shade.A *= Ease(); return FSlateColor(Shade); })
            .OnMouseButtonDown_Lambda([Close](const FGeometry&, const FPointerEvent&) { Close(); return FReply::Handled(); })
        ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [
            SNew(SBox).WidthOverride(980.f).HeightOverride(620.f)
            .RenderTransformPivot(FVector2D(0.5f, 0.5f))
            .RenderTransform_Lambda([Ease] { const float S = 0.96f + 0.04f * Ease(); return TOptional<FSlateRenderTransform>(FSlateRenderTransform(S, FVector2f(0.f, 12.f * (1.f - Ease())))); })
            [
                SNew(SBorder).BorderImage(&NoBrush).Padding(0.f)
                .ColorAndOpacity_Lambda([Ease] { return FLinearColor(1.f, 1.f, 1.f, Ease()); })
                [ Window ]
            ]
        ];
}

#include "MarketMenuWidget.h"

#include "MarketGame.h"
#include "ProductCatalog.h"
#include "MarketSuppliers.h"
#include "MarketPromotions.h"
#include "Brushes/SlateColorBrush.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "InputCoreTypes.h"
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
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

// Named namespace (not anonymous): the module is built as a unity build and the HUD has its own helpers.
namespace MarketMenuUi
{
    FSlateFontInfo MenuFont(bool bBold, int32 Size)
    {
        return FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", Size);
    }

    // 1234.5 TL -> "1.234,50 TL"
    FString Tl(int64 Kurus)
    {
        const bool bNegative = Kurus < 0;
        const int64 Abs = FMath::Abs(Kurus);
        FString Whole = FString::Printf(TEXT("%lld"), Abs / 100);
        for (int32 I = Whole.Len() - 3; I > 0; I -= 3) Whole.InsertAt(I, TEXT('.'));
        return FString::Printf(TEXT("%s%s,%02lld TL"), bNegative ? TEXT("-") : TEXT(""), *Whole, Abs % 100);
    }

    FLinearColor Hex(const TCHAR* Code, float Alpha = 1.f)
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

    const TCHAR* PageTitle(int32 Page)
    {
        switch (Page)
        {
        case SMarketMenu::Summary: return TEXT("\u00d6zet");
        case SMarketMenu::Orders: return TEXT("Sipari\u015f");
        case SMarketMenu::Prices: return TEXT("\u00dcr\u00fcnler ve fiyat");
        case SMarketMenu::Rivals: return TEXT("Rakipler");
        case SMarketMenu::Staff: return TEXT("Personel");
        case SMarketMenu::Branches: return TEXT("\u015eubeler");
        default: return TEXT("Raporlar");
        }
    }

    // Price of one rival for one product today; false = the rival's shelf is empty.
    bool RivalShelfPrice(const AMarketGameMode& G, int32 Product, int32 Rival, int64& OutPrice)
    {
        bool bEmpty = false;
        const float Factor = MarketRivals::RivalFactor(G.State.Day, G.State.RivalSeed, G.RivalAisles, G.Products[Product].Category, Rival, &bEmpty);
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
        const int64 Theirs = MarketDemand::RivalPrice(G.Products[Product], G.RivalPriceFactor(Product));
        return MarketDemand::BuyChance(MarketDemand::PriceRatio(G.State.Stock[Product].Price, Theirs), G.State.MarketShare);
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

    FString RivalsToday(const AMarketGameMode& G, int32 OnlyRival = INDEX_NONE)
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
    switch (Role)
    {
    case ERole::Page: return bLight ? Hex(TEXT("F4F5F7")) : Hex(TEXT("111315"));
    case ERole::Panel: return bLight ? Hex(TEXT("FFFFFF")) : Hex(TEXT("1B1E22"));
    case ERole::Inset: return bLight ? Hex(TEXT("E9ECF0")) : Hex(TEXT("262A30"));
    case ERole::Text: return bLight ? Hex(TEXT("171A1E")) : Hex(TEXT("F4F5F6"));
    case ERole::Muted: return bLight ? Hex(TEXT("5F6670")) : Hex(TEXT("AAB1BA"));
    case ERole::Accent: return bLight ? Hex(TEXT("16775F")) : Hex(TEXT("71C6AC"));
    case ERole::Good: return bLight ? Hex(TEXT("16775F")) : Hex(TEXT("71C6AC"));
    case ERole::Bad: return bLight ? Hex(TEXT("C4453A")) : Hex(TEXT("F07F6E"));
    case ERole::Warn: return bLight ? Hex(TEXT("B7791F")) : Hex(TEXT("F2B45A"));
    case ERole::Line: return bLight ? Hex(TEXT("DDE1E6")) : Hex(TEXT("33383F"));
    case ERole::Button: return bLight ? Hex(TEXT("E9ECEF")) : Hex(TEXT("2C3137"));
    case ERole::ButtonText: return bLight ? Hex(TEXT("171A1E")) : Hex(TEXT("F4F5F6"));
    case ERole::Primary: return bLight ? Hex(TEXT("171A1E")) : Hex(TEXT("71C6AC"));
    case ERole::PrimaryText: return bLight ? Hex(TEXT("FFFFFF")) : Hex(TEXT("0F1A16"));
    case ERole::Dim: return FLinearColor(0.f, 0.f, 0.f, bLight ? 0.35f : 0.55f);
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
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)[ Label(Sub, 10, ERole::Muted) ]);
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
// Frame

void SMarketMenu::Construct(const FArguments& InArgs)
{
    Game = InArgs._Game;
    PillStyle = FButtonStyle().SetNormal(PillBrush).SetHovered(PillHover).SetPressed(PillPress).SetDisabled(PillBrush)
        .SetNormalPadding(FMargin(0.f)).SetPressedPadding(FMargin(0.f));
    RowStyle = FButtonStyle().SetNormal(SmallBrush).SetHovered(SmallHover).SetPressed(SmallPress).SetDisabled(SmallBrush)
        .SetNormalPadding(FMargin(0.f)).SetPressedPadding(FMargin(0.f));
    BarStyle = FProgressBarStyle().SetBackgroundImage(NoBrush).SetFillImage(TrackBrush).SetMarqueeImage(TrackBrush);

    ChildSlot
    [
        SNew(SOverlay)
        .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && G->bMenuOpen ? EVisibility::Visible : EVisibility::Collapsed; })
        + SOverlay::Slot()
        [ SNew(SBorder).BorderImage(&FlatBrush).BorderBackgroundColor(Col(ERole::Dim)) ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(20.f)
        [
            SNew(SBox).WidthOverride(1260.f).HeightOverride(740.f)
            [
                SNew(SBorder).BorderImage(&PanelBrush).BorderBackgroundColor(Col(ERole::Page)).Padding(0.f)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth()[ Sidebar() ]
                    + SHorizontalBox::Slot().FillWidth(1.f)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()[ Header() ]
                        + SVerticalBox::Slot().FillHeight(1.f).Padding(24.f, 0.f, 24.f, 22.f)
                        [
                            SNew(SWidgetSwitcher)
                            .WidgetIndex_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G ? FMath::Clamp(G->MenuPage, 0, PageCount - 1) : 0; })
                            + SWidgetSwitcher::Slot()[ SummaryPage() ]
                            + SWidgetSwitcher::Slot()[ OrdersPage() ]
                            + SWidgetSwitcher::Slot()[ PricesPage() ]
                            + SWidgetSwitcher::Slot()[ RivalsPage() ]
                            + SWidgetSwitcher::Slot()[ StaffPage() ]
                            + SWidgetSwitcher::Slot()[ BranchesPage() ]
                            + SWidgetSwitcher::Slot()[ ReportsPage() ]
                        ]
                    ]
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
    if (Key == EKeys::Escape || Key == EKeys::M || Key == EKeys::Tab) { G->CloseMenu(); return FReply::Handled(); }
    static const FKey Digits[PageCount] = { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six, EKeys::Seven };
    for (int32 Page = 0; Page < PageCount; ++Page)
        if (Key == Digits[Page]) { Go(Page); return FReply::Handled(); }
    return FReply::Unhandled();
}

TSharedRef<SWidget> SMarketMenu::NavItem(int32 Page, const FString& Text, const FString& Key)
{
    auto IsCurrent = [this, Page] { const AMarketGameMode* G = Game.Get(); return G && G->MenuPage == Page; };
    return SNew(SButton).ButtonStyle(&RowStyle).IsFocusable(false).ContentPadding(FMargin(12.f, 9.f))
        .ButtonColorAndOpacity_Lambda([this, IsCurrent] { return FSlateColor(Color(IsCurrent() ? ERole::Inset : ERole::Panel)); })
        .OnClicked_Lambda([this, Page] { Go(Page); return FReply::Handled(); })
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
        [ Fixed(Key, 9, ERole::Muted, true) ]
        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
        [ LabelBy([Text] { return Text; }, 12, [IsCurrent] { return IsCurrent() ? ERole::Accent : ERole::Text; }, true) ]
    ];
}

TSharedRef<SWidget> SMarketMenu::Sidebar()
{
    TSharedRef<SVerticalBox> Nav = SNew(SVerticalBox);
    for (int32 Page = 0; Page < PageCount; ++Page)
        Nav->AddSlot().AutoHeight().Padding(0.f, 2.f)[ NavItem(Page, MarketMenuUi::PageTitle(Page), FString::FromInt(Page + 1)) ];

    return SNew(SBox).WidthOverride(230.f)
    [
        SNew(SBorder).BorderImage(&PanelBrush).BorderBackgroundColor(Col(ERole::Panel)).Padding(FMargin(16.f, 22.f))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(8.f, 0.f, 0.f, 0.f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
                [ Badge(TEXT("miras"), TEXT("MM"), Color(ERole::Accent), 34.f) ]
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("M\u0130RAS MARKET"), 12, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("L\u00fcleburgaz \u00b7 2011"), 9, ERole::Muted) ]
                ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 22.f, 0.f, 0.f)[ Nav ]
            + SVerticalBox::Slot().FillHeight(1.f)[ SNew(SSpacer) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
            [
                Button([this] { return IsLight() ? FString(TEXT("Koyu tema")) : FString(TEXT("A\u00e7\u0131k tema")); },
                    [this] { if (AMarketGameMode* G = Game.Get()) G->ToggleMenuTheme(); })
            ]
            + SVerticalBox::Slot().AutoHeight()
            [
                Button([] { return FString(TEXT("Oyuna d\u00f6n  (M / Esc)")); }, [this] { if (AMarketGameMode* G = Game.Get()) G->CloseMenu(); }, true)
            ]
        ]
    ];
}

TSharedRef<SWidget> SMarketMenu::Header()
{
    auto Pill = [this](TFunction<FString()> Make, TFunction<ERole()> Role) -> TSharedRef<SWidget>
    {
        return SNew(SBorder).BorderImage(&PillBrush).BorderBackgroundColor(Col(ERole::Panel)).Padding(FMargin(14.f, 6.f))
            [ LabelBy(Make, 11, Role, true) ];
    };
    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(24.f, 20.f, 16.f, 16.f)
        [
            Label([this] { const AMarketGameMode* G = Game.Get(); return FString(MarketMenuUi::PageTitle(G ? G->MenuPage : 0)); }, 22, ERole::Text, true)
        ]
        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(0.f, 20.f, 12.f, 16.f)
        [
            SNew(SBox).MaxDesiredWidth(560.f)
            .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && G->MessageTime > 0.f && !G->Message.IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 6.f))
                [ Label([this] { const AMarketGameMode* G = Game.Get(); return G ? G->Message : FString(); }, 10, ERole::Text, false, true) ]
            ]
        ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 20.f, 8.f, 16.f)
        [
            Pill([this] { const AMarketGameMode* G = Game.Get(); return G ? FString::Printf(TEXT("G\u00fcn %d  \u00b7  %s  \u00b7  %s"), G->State.Day, *MarketDirector::DateText(G->State), G->bOpen ? TEXT("a\u00e7\u0131k") : TEXT("kapal\u0131")) : FString(); },
                [this] { const AMarketGameMode* G = Game.Get(); return G && G->bOpen ? ERole::Accent : ERole::Muted; })
        ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 20.f, 24.f, 16.f)
        [
            Pill([this] { const AMarketGameMode* G = Game.Get(); return G ? TEXT("Kasa  ") + MarketMenuUi::Tl(G->State.Cash) : FString(); },
                [] { return ERole::Text; })
        ];
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
        [ Row([G] { return G() && !MarketCampaign::DebtOpen(G()->State); }, [] { return FString(TEXT("Baban\u0131n borcu kapans\u0131n")); }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)
        [ Row([G] { return G() && G()->State.Cash >= MarketCampaign::ExpandCash; },
              [G] { return FString::Printf(TEXT("Kasada %s  (\u015fu an %s)"), *MarketMenuUi::Tl(MarketCampaign::ExpandCash), G() ? *MarketMenuUi::Tl(G()->State.Cash) : TEXT("")); }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)
        [ Row([G] { return G() && G()->State.ProfitableDays >= MarketCampaign::ExpandProfitableDays; },
              [G] { return FString::Printf(TEXT("%d k\u00e2rl\u0131 g\u00fcn  (%d / %d)"), MarketCampaign::ExpandProfitableDays, G() ? G()->State.ProfitableDays : 0, MarketCampaign::ExpandProfitableDays); }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)
        [ Row([G] { return G() && G()->State.MarketShare >= MarketCampaign::ExpandShare; },
              [G] { return FString::Printf(TEXT("Yerel pay en az %%%.0f  (\u015fu an %%%.0f)"), MarketCampaign::ExpandShare, G() ? G()->State.MarketShare : 0.f); }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f).HAlign(HAlign_Left)
        [
            Button([G] { return G() && G()->State.bSecondStore ? FString(TEXT("\u0130kinci \u015fube a\u00e7\u0131k")) : FString::Printf(TEXT("\u0130kinci \u015fubeyi a\u00e7  (%s)"), *MarketMenuUi::Tl(MarketCampaign::ExpandCash)); },
                [this] { Do(TEXT("Expand")); }, true,
                [G] { return G() && MarketCampaign::ExpandBlock(G()->State) == MarketCampaign::EExpandBlock::None; })
        ];
}

TSharedRef<SWidget> SMarketMenu::SummaryPage()
{
    auto G = [this] { return Game.Get(); };
    return SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [ Stat(TEXT("KASA"), [G] { return G() ? MarketMenuUi::Tl(G()->State.Cash) : FString(); },
                   [G] { return G() && G()->bTestMode ? FString(TEXT("test modu a\u00e7\u0131k (F2)")) : FString(TEXT("nakit")); }) ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [ Stat(TEXT("D\u00dcN NET"), [G] { return G() && G()->State.Day > 1 ? MarketMenuUi::Tl(G()->State.LastProfit) : FString(TEXT("\u2014")); },
                   [G] { return G() && G()->State.Day > 1 ? FString::Printf(TEXT("ciro %s"), *MarketMenuUi::Tl(G()->State.LastRevenue)) : FString(TEXT("ilk g\u00fcn")); },
                   [G] { return G() && G()->State.LastProfit < 0 ? ERole::Bad : ERole::Good; }) ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [ Stat(TEXT("BUG\u00dcN"), [G] { return G() ? MarketMenuUi::Tl(G()->State.Revenue) : FString(); },
                   [G] { return G() ? FString::Printf(TEXT("%d sat\u0131\u015f \u00b7 %d kay\u0131p m\u00fc\u015fteri"), G()->State.Served, G()->State.Lost) : FString(); }) ]
            + SHorizontalBox::Slot().FillWidth(1.f)
            [ Stat(TEXT("YEREL PAY"), [G] { return G() ? FString::Printf(TEXT("%%%.0f"), G()->State.MarketShare) : FString(); },
                   [G] { return G() ? FString::Printf(TEXT("%d k\u00e2rl\u0131 g\u00fcn"), G()->State.ProfitableDays) : FString(); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("BABANIN BORCU"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 8.f)
                    [ LabelBy([G] { return !G() ? FString() : MarketCampaign::DebtOpen(G()->State) ? MarketMenuUi::Tl(G()->State.InheritedDebt) + TEXT(" kald\u0131") : FString(TEXT("Kapand\u0131")); }, 20,
                        [G] { return G() && !MarketCampaign::DebtOpen(G()->State) ? ERole::Good : ERole::Warn; }, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ Bar([G] { return G() ? MarketCampaign::DebtProgress(G()->State) : 0.f; }, ERole::Accent) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 12.f)
                    [ Label([G] { return G() ? FString::Printf(TEXT("Toptanc\u0131ya %s bor\u00e7 \u00b7 s\u00fcre yok \u00b7 kapanmadan ikinci \u015fube a\u00e7\u0131lmaz"), *MarketMenuUi::Tl(MarketCampaign::StartingDebt)) : FString(); }, 10, ERole::Muted, false, true) ]
                    + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left)
                    [ Button([] { return FString::Printf(TEXT("%s \u00f6de"), *MarketMenuUi::Tl(MarketCampaign::Installment)); }, [this] { Do(TEXT("PayDebt")); }, true,
                        [G] { return G() && MarketCampaign::DebtOpen(G()->State) && G()->State.Cash > 0; }) ])
            ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[ Fixed(TEXT("HEDEF \u00b7 \u0130K\u0130NC\u0130 \u015eUBE"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ GoalList() ])
            ]
            + SHorizontalBox::Slot().FillWidth(1.2f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[ Fixed(TEXT("RAK\u0130PLERDE BUG\u00dcN"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ Label([G] { return G() ? MarketMenuUi::RivalsToday(*G()) : FString(); }, 11, ERole::Text, false, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f).HAlign(HAlign_Left)
                    [ Button([] { return FString(TEXT("Fiyatlar\u0131 kar\u015f\u0131la\u015ft\u0131r")); }, [this] { Go(Prices); }) ])
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.4f).Padding(0.f, 0.f, 12.f, 0.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[ Fixed(TEXT("D\u00dcN NEREDE M\u00dc\u015eTER\u0130 KAYBETT\u0130N"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ Label([G] { return G() ? G()->DayProblemsText() : FString(); }, 11, ERole::Text, false, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f).HAlign(HAlign_Left)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[ Button([] { return FString(TEXT("Sipari\u015f ver")); }, [this] { Go(Orders); }) ]
                        + SHorizontalBox::Slot().AutoWidth()[ Button([] { return FString(TEXT("G\u00fcn raporu")); }, [this] { bWeekTab = false; Go(Reports); }) ]
                    ])
            ]
            + SHorizontalBox::Slot().FillWidth(1.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[ Fixed(TEXT("MA\u011eAZA"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ Label([G] { return !G() ? FString() : G()->bOpen ? FString::Printf(TEXT("A\u00e7\u0131k \u00b7 i\u00e7eride %d m\u00fc\u015fteri"), G()->Customers.Num()) : FString(TEXT("Kapal\u0131")); }, 14, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
                    [ Label([G] { return G() ? G()->LoyaltySummary() : FString(); }, 10, ERole::Muted, false, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f).HAlign(HAlign_Left)
                    [ Button([G] { return G() && G()->bOpen ? FString(TEXT("G\u00fcn\u00fc kapat")) : FString(TEXT("Ma\u011fazay\u0131 a\u00e7")); },
                        [this] { Do(TEXT("ToggleShop")); }, false) ])
            ]
        ]
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
            .Visibility_Lambda([this, G, I] { return G() && (OrderCategory.IsEmpty() || G()->Products[I].Category == OrderCategory) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Panel)).Padding(FMargin(12.f, 6.f))
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
                    [ Badge(FString(), MarketMenuUi::Initials(G()->ProductName(I)), FLinearColor(G()->Products[I].Color), 26.f) ]
                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()[ Label([G, I] { return G() ? G()->ProductName(I) : FString(); }, 11, ERole::Text, true) ]
                        + SVerticalBox::Slot().AutoHeight()
                        [ Label([G, I] { return G() ? FString::Printf(TEXT("%s \u00b7 koli %d adet \u00b7 %s/adet"), *MarketMenuUi::Title(G()->Products[I].Category), MarketOrderAdvice::CaseUnits(G()->Products[I]), *MarketMenuUi::Tl(G()->Products[I].Cost)) : FString(); }, 9, ERole::Muted) ]
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::Printf(TEXT("%d/%d"), Item()->Shelf, Item()->Capacity) : FString(); }, 70.f) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::FromInt(Item()->Warehouse) : FString(); }, 56.f) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::FromInt(Item()->Dock) : FString(); }, 56.f) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::FromInt(Item()->Incoming) : FString(); }, 56.f) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::FromInt(Item()->Yesterday.Sold) : FString(); }, 56.f) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::FromInt(Item()->Yesterday.Empty) : FString(); }, 56.f, ERole::Warn) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [ Cell([G, I] { if (!G() || !G()->Products.IsValidIndex(I)) return FString(); TArray<float> Scale; Scale.Init(1.f, G()->Products.Num()); Scale[I] = MarketDirector::OrderScale(G()->State, G()->Products[I]);
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
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
        [
            Card(
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [ Label([G] { return G() ? MarketSuppliers::Summary(G()->State) : FString(); }, 11, ERole::Text, false, true) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                [ Button([G] { return FString(G() && G()->State.Supplier == 0 ? TEXT("\u00d6zdemir'e ge\u00e7") : TEXT("Selim'e d\u00f6n")); },
                    [Act, G] { if (G()) Act(TEXT("Supplier"), G()->State.Supplier == 0 ? 1 : 0); }, false,
                    [G] { return G() && (G()->State.Supplier != 0 || MarketSuppliers::Available(G()->State, MarketSuppliers::ESupplier::Ozdemir)); }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                [ Button([] { return FString(TEXT("Faturalar\u0131 \u00f6de")); }, [Act] { Act(TEXT("PayBills"), 0); }, false, [G] { return G() && MarketSuppliers::OpenBills(G()->State) > 0; }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                [ Button([] { return FString(TEXT("Zamm\u0131 yans\u0131t")); }, [Act] { Act(TEXT("PassOnPriceRise"), 0); }, false, [G] { return G() && MarketSuppliers::PriceGap(G()->State) >= 0.005; }) ],
                ERole::Panel, FMargin(18.f, 10.f))
        ]
        + SVerticalBox::Slot().AutoHeight()[ CategoryChips(&OrderCategory) ]
        + SVerticalBox::Slot().AutoHeight().Padding(12.f, 14.f, 12.f, 4.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f)[ Fixed(TEXT("\u00dcR\u00dcN"), 9, ERole::Muted, true) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("RAF"), 70.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("DEPO"), 56.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("KABUL"), 56.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("YOLDA"), 56.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("D\u00dcN SAT."), 56.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("BO\u015e RAF"), 56.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("\u00d6NER\u0130"), 60.f) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(18.f, 0.f, 0.f, 0.f)[ Head(TEXT("L\u0130STE"), 150.f) ]
        ]
        + SVerticalBox::Slot().FillHeight(1.f)[ Rows ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
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
                            if (G()->OrderDraftCaseCount() > 0 && G()->OrderDraftBill() < MarketOrderAdvice::MinimumOrder)
                                return FString::Printf(TEXT("Toptanc\u0131 en az %s sipari\u015fle gelir."), *MarketMenuUi::Tl(MarketOrderAdvice::MinimumOrder));
                            if (G()->OrderDraftBill() > G()->State.Cash) return FString(TEXT("Kasadaki nakit bu listeye yetmiyor."));
                            return FString(TEXT("\u00d6deme onayda yap\u0131l\u0131r; koliler yar\u0131n sabah arka kap\u0131da."));
                        }, 10, [G] { return G() && (G()->OrderDraftBill() > G()->State.Cash || (G()->OrderDraftCaseCount() > 0 && G()->OrderDraftBill() < MarketOrderAdvice::MinimumOrder)) ? ERole::Warn : ERole::Muted; })
                    ]
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
                [ Button([] { return FString(TEXT("Temizle")); }, [this] { if (AMarketGameMode* M = Game.Get()) M->ClearOrderDraft(); }, false,
                    [G] { return G() && G()->OrderDraftCaseCount() > 0; }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
                [ Button([] { return FString(TEXT("\u00d6neriyi yaz")); }, [this] { Do(TEXT("SuggestOrder")); }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [ Button([] { return FString(TEXT("Sipari\u015fi onayla")); }, [this] { Do(TEXT("ConfirmOrder")); }, true,
                    [G] { return G() && G()->OrderDraftCaseCount() > 0; }) ],
                ERole::Panel, FMargin(18.f, 12.f))
        ];
}

TSharedRef<SWidget> SMarketMenu::PricesPage()
{
    auto G = [this] { return Game.Get(); };
    auto Current = [G] { return G() && G()->Products.IsValidIndex(G()->MenuProduct) ? G()->MenuProduct : 0; };

    TSharedRef<SScrollBox> List = SNew(SScrollBox);
    const int32 Count = G() ? G()->Products.Num() : 0;
    for (int32 I = 0; I < Count; ++I)
    {
        List->AddSlot().Padding(0.f, 3.f)
        [
            SNew(SBox)
            .Visibility_Lambda([this, G, I] { return G() && (PriceCategory.IsEmpty() || G()->Products[I].Category == PriceCategory) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SButton).ButtonStyle(&RowStyle).IsFocusable(false).ContentPadding(FMargin(12.f, 8.f))
                .ButtonColorAndOpacity_Lambda([this, Current, I] { return FSlateColor(Color(Current() == I ? ERole::Inset : ERole::Panel)); })
                .OnClicked_Lambda([G, I] { if (AMarketGameMode* M = G()) M->MenuProduct = I; return FReply::Handled(); })
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
                    [ Badge(FString(), MarketMenuUi::Initials(G()->ProductName(I)), FLinearColor(G()->Products[I].Color), 34.f) ]
                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()[ Label([G, I] { return G() ? G()->ProductName(I) : FString(); }, 11, ERole::Text, true) ]
                        + SVerticalBox::Slot().AutoHeight()[ Label([G, I] { return G() ? MarketMenuUi::Title(G()->Products[I].Category) : FString(); }, 9, ERole::Muted) ]
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)[ Label([G, I] { return G() ? MarketMenuUi::Tl(G()->State.Stock[I].Price) : FString(); }, 12, ERole::Text, true) ]
                        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
                        [
                            Label([G, I]
                            {
                                int64 Price = 0;
                                return G() && MarketMenuUi::CheapestRival(*G(), I, Price) != INDEX_NONE ? FString::Printf(TEXT("rakip en ucuz %s"), *MarketMenuUi::Tl(Price)) : FString(TEXT("rakiplerde yok"));
                            }, 9, ERole::Muted)
                        ]
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [
                        SNew(SBox).WidthOverride(64.f).HAlign(HAlign_Right)
                        [
                            LabelBy([G, I]
                            {
                                int64 Price = 0;
                                if (!G() || MarketMenuUi::CheapestRival(*G(), I, Price) == INDEX_NONE) return FString(TEXT("tek biz"));
                                const int64 Ours = G()->State.Stock[I].Price;
                                return Ours < Price ? FString(TEXT("ucuz")) : Ours > Price ? FString(TEXT("pahal\u0131")) : FString(TEXT("ayn\u0131"));
                            }, 10, [G, I]
                            {
                                int64 Price = 0;
                                if (!G() || MarketMenuUi::CheapestRival(*G(), I, Price) == INDEX_NONE) return ERole::Good;
                                const int64 Ours = G()->State.Stock[I].Price;
                                return Ours < Price ? ERole::Good : Ours > Price ? ERole::Bad : ERole::Muted;
                            }, true)
                        ]
                    ]
                ]
            ]
        ];
    }

    TSharedRef<SVerticalBox> RivalRows = SNew(SVerticalBox);
    for (int32 Rival = 0; Rival < 3; ++Rival)
    {
        RivalRows->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 8.f))
            .Visibility_Lambda([G, Rival] { return G() && Rival < MarketRivals::RivalCount(G()->State.Day) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)
                [ Badge(MarketRivals::RivalLogoKey(Rival), MarketMenuUi::Initials(MarketRivals::RivalName(Rival)), MarketMenuUi::RivalColor(Rival), 34.f) ]
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(MarketRivals::RivalName(Rival), 12, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        Label([G, Current, Rival]
                        {
                            if (!G()) return FString();
                            bool bEmpty = false;
                            const float Factor = MarketRivals::RivalFactor(G()->State.Day, G()->State.RivalSeed, G()->RivalAisles, G()->Products[Current()].Category, Rival, &bEmpty);
                            const FString Kind = MarketRivals::RivalFormat(Rival);
                            if (bEmpty) return Kind + TEXT(" \u00b7 bu reyon bo\u015f");
                            if (Factor < 1.f) return Kind + FString::Printf(TEXT(" \u00b7 kampanyada %%%d"), FMath::RoundToInt32((1.f - Factor) * 100.f));
                            if (Factor > 1.f) return Kind + FString::Printf(TEXT(" \u00b7 zam %%%d"), FMath::RoundToInt32((Factor - 1.f) * 100.f));
                            return Kind;
                        }, 9, ERole::Muted)
                    ]
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f)
                [
                    Label([G, Current, Rival]
                    {
                        int64 Price = 0;
                        return G() && MarketMenuUi::RivalShelfPrice(*G(), Current(), Rival, Price) ? MarketMenuUi::Tl(Price) : FString(TEXT("Rafta yok"));
                    }, 14, ERole::Text, true)
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [
                    SNew(SBox).WidthOverride(70.f).HAlign(HAlign_Right)
                    [
                        LabelBy([G, Current, Rival]
                        {
                            int64 Price = 0;
                            if (!G() || !MarketMenuUi::RivalShelfPrice(*G(), Current(), Rival, Price) || Price <= 0) return FString();
                            const int32 Diff = FMath::RoundToInt32((static_cast<double>(G()->State.Stock[Current()].Price) / Price - 1.0) * 100.0);
                            return Diff == 0 ? FString(TEXT("ayn\u0131")) : FString::Printf(TEXT("biz %s%%%d"), Diff > 0 ? TEXT("+") : TEXT("-"), FMath::Abs(Diff));
                        }, 10, [G, Current, Rival]
                        {
                            int64 Price = 0;
                            if (!G() || !MarketMenuUi::RivalShelfPrice(*G(), Current(), Rival, Price)) return ERole::Muted;
                            return G()->State.Stock[Current()].Price > Price ? ERole::Bad : ERole::Good;
                        }, true)
                    ]
                ]
            ]
        ];
    }

    TSharedRef<SWidget> Detail = Card(
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ Label([G, Current] { return G() ? G()->ProductName(Current()) : FString(); }, 20, ERole::Text, true) ]
                + SVerticalBox::Slot().AutoHeight()
                [ Label([G, Current] { return G() ? FString::Printf(TEXT("%s \u00b7 %s"), *MarketMenuUi::Title(G()->Products[Current()].Category), *G()->Products[Current()].Brand) : FString(); }, 10, ERole::Muted) ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("B\u0130Z\u0130M F\u0130YAT"), 9, ERole::Muted, true) ]
                + SVerticalBox::Slot().AutoHeight()[ Label([G, Current] { return G() ? MarketMenuUi::Tl(G()->State.Stock[Current()].Price) : FString(); }, 28, ERole::Text, true) ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(18.f, 0.f, 6.f, 0.f)
            [ Button([] { return FString(TEXT("-")); }, [this, Current] { Do(TEXT("PriceDown"), Current()); }) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ Button([] { return FString(TEXT("+")); }, [this, Current] { Do(TEXT("PriceUp"), Current()); }) ]
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).HAlign(HAlign_Right)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)[ Fixed(TEXT("ALAN M\u00dc\u015eTER\u0130"), 9, ERole::Muted, true) ]
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
                [ LabelBy([G, Current] { return G() ? FString::Printf(TEXT("~%%%d"), FMath::RoundToInt32(MarketMenuUi::BuyChanceOf(*G(), Current()) * 100.0)) : FString(); }, 24,
                    [G, Current] { const double Chance = G() ? MarketMenuUi::BuyChanceOf(*G(), Current()) : 1.0; return Chance >= 0.6 ? ERole::Good : Chance >= 0.35 ? ERole::Warn : ERole::Bad; }, true) ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [
            Label([G, Current]
            {
                if (!G()) return FString();
                const FMarketProduct& P = G()->Products[Current()];
                const int64 Ours = G()->State.Stock[Current()].Price;
                const int32 Margin = Ours > 0 ? FMath::RoundToInt32(static_cast<double>(Ours - P.Cost) / Ours * 100.0) : 0;
                return FString::Printf(TEXT("Al\u0131\u015f %s \u00b7 liste fiyat\u0131 %s \u00b7 k\u00e2r marj\u0131 %%%d"), *MarketMenuUi::Tl(P.Cost), *MarketMenuUi::Tl(P.BasePrice), Margin);
            }, 10, ERole::Muted)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 6.f)[ Fixed(TEXT("RAK\u0130P F\u0130YATLARI \u00b7 L\u00dcLEBURGAZ"), 9, ERole::Muted, true) ]
        + SVerticalBox::Slot().AutoHeight()[ RivalRows ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
        [
            Label([G, Current]
            {
                if (!G()) return FString();
                const int64 Theirs = MarketDemand::RivalPrice(G()->Products[Current()], G()->RivalPriceFactor(Current()));
                return FString::Printf(TEXT("M\u00fc\u015fterinin akl\u0131ndaki rakip fiyat\u0131: %s. Pahal\u0131 bulan m\u00fc\u015fteri \u00fcr\u00fcn\u00fc almaz; sad\u0131k m\u00fc\u015fteri biraz daha ho\u015fg\u00f6r\u00fcl\u00fcd\u00fcr."), *MarketMenuUi::Tl(Theirs));
            }, 10, ERole::Muted, false, true)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [ Label([G, Current] { return G() ? G()->OrderAdvice(Current()) : FString(); }, 10, ERole::Muted, false, true) ]
        // G-064 promotions for the selected product / its aisle; running ones and the wholesaler's offer.
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 6.f)[ Fixed(TEXT("KAMPANYA"), 9, ERole::Muted, true) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 6.f))
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Reyonda %10")); }, [this, Current] { Manage(TEXT("Discount10"), Current()); }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Reyonda %20")); }, [this, Current] { Manage(TEXT("Discount20"), Current()); }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("3 al 2 \u00f6de")); }, [this, Current] { Manage(TEXT("MultiBuy"), Current()); }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Gondol ba\u015f\u0131na koy")); }, [this, Current] { Manage(TEXT("Endcap"), Current()); }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Bro\u015f\u00fcr da\u011f\u0131t")); }, [this] { Manage(TEXT("Flyer"), INDEX_NONE); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [
            Label([G]
            {
                if (!G()) return FString();
                TArray<FString> Lines;
                for (const FMarketPromotion* P : MarketPromotions::Active(G()->State))
                    Lines.Add(FString::Printf(TEXT("\u2022 %s \u00b7 %d. g\u00fcne kadar"), *MarketPromotions::Describe(*P, G()->Products), P->EndDay));
                if (G()->State.Offer.Product != INDEX_NONE && G()->State.Offer.EndDay >= G()->State.Day)
                    Lines.Add(TEXT("Toptanc\u0131 teklifi bekliyor: ") + MarketPromotions::Describe(G()->State.Offer, G()->Products));
                return Lines.Num() > 0 ? FString::Join(Lines, TEXT("\n")) : FString(TEXT("Y\u00fcr\u00fcyen kampanya yok."));
            }, 10, ERole::Text, false, true)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)
            [ Button([] { return FString(TEXT("Teklifi kabul et")); }, [this] { Manage(TEXT("AcceptOffer"), INDEX_NONE); }, true,
                [G] { return G() && G()->State.Offer.Product != INDEX_NONE && G()->State.Offer.EndDay >= G()->State.Day; }) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)
            [ Button([] { return FString(TEXT("Geri \u00e7evir")); }, [this] { Manage(TEXT("DeclineOffer"), INDEX_NONE); }, false,
                [G] { return G() && G()->State.Offer.Product != INDEX_NONE && G()->State.Offer.EndDay >= G()->State.Day; }) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ Button([] { return FString(TEXT("Son kampanyay\u0131 durdur")); }, [this, G]
                {
                    if (!G()) return;
                    for (int32 I = G()->State.Promotions.Num() - 1; I >= 0; --I)
                        if (MarketPromotions::IsActive(G()->State.Promotions[I], G()->State.Day)) { Manage(TEXT("StopPromotion"), I); return; }
                }, false, [G] { return G() && MarketPromotions::Active(G()->State).Num() > 0; }) ]
        ],
        ERole::Panel, FMargin(22.f, 20.f));

    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(0.9f).Padding(0.f, 0.f, 16.f, 0.f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[ CategoryChips(&PriceCategory) ]
            + SVerticalBox::Slot().FillHeight(1.f)[ List ]
        ]
        + SHorizontalBox::Slot().FillWidth(1.f)
        [ SNew(SScrollBox) + SScrollBox::Slot()[ Detail ] ];
}

TSharedRef<SWidget> SMarketMenu::RivalsPage()
{
    auto G = [this] { return Game.Get(); };
    TSharedRef<SHorizontalBox> Cards = SNew(SHorizontalBox);
    for (int32 Rival = 0; Rival < 3; ++Rival)
    {
        Cards->AddSlot().FillWidth(1.f).Padding(0.f, 0.f, Rival < 2 ? 12.f : 0.f, 0.f)
        [
            SNew(SBox)
            .Visibility_Lambda([G, Rival] { return G() && Rival < MarketRivals::RivalCount(G()->State.Day) ? EVisibility::Visible : EVisibility::Hidden; })
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)
                        [ Badge(MarketRivals::RivalLogoKey(Rival), MarketMenuUi::Initials(MarketRivals::RivalName(Rival)), MarketMenuUi::RivalColor(Rival), 44.f) ]
                        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot().AutoHeight()[ Fixed(MarketRivals::RivalName(Rival), 16, ERole::Text, true) ]
                            + SVerticalBox::Slot().AutoHeight()[ Fixed(MarketRivals::RivalFormat(Rival), 10, ERole::Muted) ]
                        ]
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 6.f)[ Fixed(TEXT("BUG\u00dcN"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ Label([G, Rival] { return G() ? MarketMenuUi::RivalsToday(*G(), Rival) : FString(); }, 11, ERole::Text, false, true) ])
            ]
        ];
    }
    return SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            Card(SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("YEREL PAZAR \u00b7 L\u00dcLEBURGAZ"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 8.f)
                    [ Label([G] { return G() ? FString::Printf(TEXT("Mahalle m\u00fc\u015fterilerinin %%%.0f'i bizden al\u0131\u015fveri\u015f yap\u0131yor"), G()->State.MarketShare) : FString(); }, 16, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ Bar([G] { return G() ? G()->State.MarketShare / 100.f : 0.f; }, ERole::Accent) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
                    [ Fixed(TEXT("Pay her g\u00fcn sonunda m\u00fc\u015fteri memnuniyetiyle de\u011fi\u015fir: rafta bulma, fiyat ve bekleme."), 10, ERole::Muted) ]
                ])
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[ Cards ]
        + SVerticalBox::Slot().AutoHeight().Padding(4.f, 14.f, 0.f, 0.f)
        [ Fixed(TEXT("Ulusal ve uluslararas\u0131 pazar paylar\u0131 \u015fube sistemiyle gelecek. Logo i\u00e7in: Content/Brands/<bim|migros|a101>/logo.png"), 10, ERole::Muted) ]
    ];
}

TSharedRef<SWidget> SMarketMenu::StaffPage()
{
    // G-060: people are persons (MarketStaff). Rows are fixed slots shown while the roster/pool has that many
    // entries; every button reads the person's id at click time, so a changed roster never hits the wrong person.
    auto G = [this] { return Game.Get(); };
    auto Act = [this](FName Action, int32 Id) { if (AMarketGameMode* Mode = Game.Get()) Mode->StaffCommand(Action, Id); };
    auto PersonAt = [G](int32 Slot) -> const FMarketEmployee* { return G() && G()->State.Staff.IsValidIndex(Slot) ? &G()->State.Staff[Slot] : nullptr; };
    auto CandidateAt = [G](int32 Slot) -> const FMarketEmployee* { return G() && G()->State.Candidates.IsValidIndex(Slot) ? &G()->State.Candidates[Slot] : nullptr; };
    auto RoleAt = [PersonAt](int32 Slot) { const FMarketEmployee* E = PersonAt(Slot); return E ? MarketStaff::RoleOf(*E) : MarketStaff::ERole::Accountant; };

    TSharedRef<SVerticalBox> People = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 8; ++Slot)
    {
        People->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBox).Visibility_Lambda([PersonAt, Slot] { return PersonAt(Slot) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
                [ Label([G, PersonAt, Slot] { const FMarketEmployee* E = PersonAt(Slot); return E ? MarketStaff::DescribeEmployee(G()->State, *E) : FString(); }, 11, ERole::Text, false, true) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
                [ Button([] { return FString(TEXT("Zam %10")); }, [Act, PersonAt, Slot] { if (const FMarketEmployee* E = PersonAt(Slot)) Act(TEXT("Raise"), E->Id); }, false,
                    [RoleAt, PersonAt, Slot] { return PersonAt(Slot) && RoleAt(Slot) != MarketStaff::ERole::Accountant; }) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
                [ Button([] { return FString(TEXT("\u0130zin ver")); }, [Act, PersonAt, Slot] { if (const FMarketEmployee* E = PersonAt(Slot)) Act(TEXT("DayOff"), E->Id); }, false,
                    [RoleAt, PersonAt, Slot] { return PersonAt(Slot) && RoleAt(Slot) != MarketStaff::ERole::Accountant; }) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
                [ Button([] { return FString(TEXT("Uyar")); }, [Act, PersonAt, Slot] { if (const FMarketEmployee* E = PersonAt(Slot)) Act(TEXT("Warn"), E->Id); }, false,
                    [RoleAt, PersonAt, Slot] { return PersonAt(Slot) && RoleAt(Slot) == MarketStaff::ERole::Cashier; }) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
                [ Button([RoleAt, Slot] { return FString(RoleAt(Slot) == MarketStaff::ERole::Accountant ? TEXT("S\u00f6zle\u015fmeyi bitir") : TEXT("\u00c7\u0131kar")); },
                    [Act, PersonAt, Slot] { if (const FMarketEmployee* E = PersonAt(Slot)) Act(TEXT("Fire"), E->Id); }, false, [PersonAt, Slot] { return PersonAt(Slot) != nullptr; }) ]
            ]
        ];
    }

    TSharedRef<SVerticalBox> Pool = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < MarketStaff::HrPoolSize; ++Slot)
    {
        Pool->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBox).Visibility_Lambda([CandidateAt, Slot] { return CandidateAt(Slot) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
                [ Label([G, CandidateAt, Slot] { const FMarketEmployee* C = CandidateAt(Slot); return C ? MarketStaff::DescribeCandidate(G()->State, *C) : FString(); }, 11, ERole::Text, false, true) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
                [ Button([] { return FString(TEXT("\u0130\u015fe al")); }, [Act, CandidateAt, Slot] { if (const FMarketEmployee* C = CandidateAt(Slot)) Act(TEXT("HireCandidate"), C->Id); }, true,
                    [CandidateAt, Slot] { return CandidateAt(Slot) != nullptr; }) ]
            ]
        ];
    }

    return SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("EK\u0130P"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 4.f)
                    [ Label([G] { return G() ? FString::Printf(TEXT("%d ki\u015fi \u00b7 g\u00fcnl\u00fck %s"), G()->State.Staff.Num(), *MarketMenuUi::Tl(G()->State.DailyPayroll())) : FString(); }, 18, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ Label([G] { return G() ? FString::Printf(TEXT("Bug\u00fcn kasada: %s \u00b7 reyonda %d g\u00f6revli"),
                        G()->State.bCashier ? TEXT("kasiyer") : TEXT("sen (E)"), G()->State.Stockers) : FString(); }, 10, ERole::Muted, false, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
                    [ Label([G] { return G() ? G()->WorkerSummary() : FString(); }, 10, ERole::Muted, false, true) ])
            ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("MAL\u0130 M\u00dc\u015eAV\u0130R \u00b7 VERG\u0130"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 4.f)
                    [ LabelBy([G] { return G() && G()->State.Books.TaxDue > 0 ? FString::Printf(TEXT("%s \u00b7 son g\u00fcn %d"), *MarketMenuUi::Tl(G()->State.Books.TaxDue), G()->State.Books.TaxDueDay)
                        : FString(TEXT("\u00d6denecek vergi yok")); }, 18, [G] { return G() && G()->State.Books.TaxDue > 0 ? ERole::Warn : ERole::Good; }, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ Label([G] { return G() && MarketStaff::HasAccountant(G()->State)
                        ? FString(TEXT("Necati Bey defterleri tutuyor: haftal\u0131k vergiyi zaman\u0131nda \u00f6der, kasa farklar\u0131n\u0131 izler."))
                        : FString(TEXT("Vergi her 7. g\u00fcn\u00fcn sonunda \u00e7\u0131kar; 3 g\u00fcn i\u00e7inde \u00f6denmezse ceza i\u015fler. Defter tutulmazsa inceleme gelebilir.")); }, 10, ERole::Muted, false, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f).HAlign(HAlign_Left)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
                        [ Button([] { return FString(TEXT("Vergiyi \u00f6de")); }, [Act] { Act(TEXT("PayTax"), INDEX_NONE); }, true, [G] { return G() && G()->State.Books.TaxDue > 0; }) ]
                        + SHorizontalBox::Slot().AutoWidth()
                        [ Button([] { return FString::Printf(TEXT("M\u00fc\u015favirle anla\u015f (%s/g\u00fcn)"), *MarketMenuUi::Tl(MarketStaff::AccountantDailyFee)); },
                            [Act] { Act(TEXT("HireAccountant"), INDEX_NONE); }, false, [G] { return G() && !MarketStaff::HasAccountant(G()->State); }) ]
                    ])
            ]
            + SHorizontalBox::Slot().FillWidth(1.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("\u0130NSAN KAYNAKLARI"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 4.f)
                    [ Label([G]
                    {
                        if (!G()) return FString();
                        if (MarketStaff::HasHr(G()->State)) return FString(TEXT("\u0130K m\u00fcd\u00fcr\u00fc \u00e7al\u0131\u015f\u0131yor"));
                        return MarketStaff::HrUnlocked(G()->State) ? FString(TEXT("Aday listesinde \u0130K m\u00fcd\u00fcr\u00fc var")) : FString::Printf(TEXT("%d \u00e7al\u0131\u015fandan sonra"), MarketStaff::HrUnlockStaff);
                    }, 18, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ Fixed(TEXT("Her g\u00fcn en mutsuz ki\u015fiyle konu\u015fur, yorgunlara izin ayarlar, ayr\u0131lan\u0131n yerine aday bulur, \u00fccret pazarl\u0131\u011f\u0131 yapar ve adaylar\u0131n ger\u00e7ek de\u011ferlerini g\u00f6sterir."), 10, ERole::Muted) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f).HAlign(HAlign_Left)
                    [ Button([G] { return FString(G() && G()->State.bHrAutoReplace ? TEXT("Ayr\u0131lan\u0131n yerine al: a\u00e7\u0131k") : TEXT("Ayr\u0131lan\u0131n yerine al: kapal\u0131")); },
                        [Act] { Act(TEXT("HrAutoReplace"), INDEX_NONE); }, false, [G] { return G() && MarketStaff::HasHr(G()->State); }) ])
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            Card(SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Fixed(TEXT("\u00c7ALI\u015eANLAR"), 9, ERole::Muted, true) ]
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(SBox).Visibility_Lambda([G] { return G() && G()->State.Staff.Num() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
                    [ Fixed(TEXT("Kimse yok. Kasay\u0131 ve raflar\u0131 sen yap\u0131yorsun."), 11, ERole::Muted) ]
                ]
                + SVerticalBox::Slot().AutoHeight()[ People ])
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            Card(SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Fixed(TEXT("\u0130\u015e BA\u015eVURULARI"), 9, ERole::Muted, true) ]
                + SVerticalBox::Slot().AutoHeight()
                [ Label([G] { return G() ? FString::Printf(TEXT("\u0130\u015fe alma %s (\u0130K m\u00fcd\u00fcr\u00fc %s). Liste %s yenilenir; son yenileme %d. g\u00fcn."),
                    *MarketMenuUi::Tl(MarketStaff::HireCost), *MarketMenuUi::Tl(MarketStaff::HrHireCost),
                    MarketStaff::HasHr(G()->State) ? TEXT("3 g\u00fcnde bir") : TEXT("haftada bir"), G()->State.CandidatesDay) : FString(); }, 10, ERole::Muted, false, true) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[ Pool ])
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(4.f, 14.f, 0.f, 0.f)
        [ Fixed(TEXT("Moral \u00fccrete, yorgunlu\u011fa ve ilgiye g\u00f6re de\u011fi\u015fir. \u00dc\u00e7 g\u00fcn \u00e7ok mutsuz olan istifa dilek\u00e7esi verir ve iki g\u00fcn sonra ayr\u0131l\u0131r; zam veya izin fikrini de\u011fi\u015ftirebilir."), 10, ERole::Muted) ]
    ];
}

TSharedRef<SWidget> SMarketMenu::BranchesPage()
{
    auto G = [this] { return Game.Get(); };
    return SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
        [
            Card(SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("\u015eUBELER\u0130N"), 9, ERole::Muted, true) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
                [
                    SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 10.f))
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("L\u00fcleburgaz \u00b7 Merkez"), 13, ERole::Text, true) ]
                        + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("Babadan kalan mahalle marketi \u00b7 buradas\u0131n"), 10, ERole::Muted) ]
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
                [
                    SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 10.f))
                    .Visibility_Lambda([G] { return G() && G()->State.bSecondStore ? EVisibility::Visible : EVisibility::Collapsed; })
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("\u0130kinci \u015fube"), 13, ERole::Text, true) ]
                        + SVerticalBox::Slot().AutoHeight()
                        [ Label([G] { return G() ? FString::Printf(TEXT("D\u00fcnk\u00fc katk\u0131s\u0131 %s"), *MarketMenuUi::Tl(G()->State.LastBranchProfit)) : FString(); }, 10, ERole::Muted) ]
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
                [ Fixed(TEXT("\u0130l haritas\u0131 ve \u015fube a\u00e7ma plan\u0131 sonraki ad\u0131mda gelecek."), 10, ERole::Muted) ])
        ]
        + SHorizontalBox::Slot().FillWidth(1.f)
        [
            Card(SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[ Fixed(TEXT("\u0130K\u0130NC\u0130 \u015eUBE \u0130\u00c7\u0130N"), 9, ERole::Muted, true) ]
                + SVerticalBox::Slot().AutoHeight()[ GoalList() ])
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
        + SVerticalBox::Slot().AutoHeight()
        [
            Label([G] { return G() && G()->State.Day > 1 ? FString::Printf(TEXT("%d. g\u00fcn kapand\u0131"), G()->State.Day - 1) : FString(TEXT("Hen\u00fcz kapanm\u0131\u015f g\u00fcn yok. O ile a\u00e7\u0131p g\u00fcn\u00fc bitirince rapor burada.")); }, 13, ERole::Muted, true)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [ Stat(TEXT("NET SONU\u00c7"), [G] { return G() ? MarketMenuUi::Tl(G()->State.LastProfit) : FString(); },
                   [G] { return G() ? FString::Printf(TEXT("%d k\u00e2rl\u0131 g\u00fcn"), G()->State.ProfitableDays) : FString(); },
                   [G] { return G() && G()->State.LastProfit < 0 ? ERole::Bad : ERole::Good; }) ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [ Stat(TEXT("C\u0130RO"), [G] { return G() ? MarketMenuUi::Tl(G()->State.LastRevenue) : FString(); },
                   [G] { return G() ? FString::Printf(TEXT("%d sat\u0131\u015f \u00b7 %d kay\u0131p"), G()->State.LastServed, G()->State.LastLost) : FString(); }) ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [ Stat(TEXT("MAL MAL\u0130YET\u0130"), [G] { return G() ? MarketMenuUi::Tl(G()->State.LastCostOfGoods) : FString(); }, [] { return FString(TEXT("sat\u0131lan \u00fcr\u00fcnlerin al\u0131\u015f\u0131")); }) ]
            + SHorizontalBox::Slot().FillWidth(1.f)
            [ Stat(TEXT("G\u0130DER"), [G] { return G() ? MarketMenuUi::Tl(G()->State.LastOperatingCost) : FString(); },
                   [G] { return G() && G()->State.bSecondStore ? FString::Printf(TEXT("ikinci \u015fube %s"), *MarketMenuUi::Tl(G()->State.LastBranchProfit)) : FString(TEXT("kira, elektrik, maa\u015f")); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.4f).Padding(0.f, 0.f, 12.f, 0.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Fixed(TEXT("NEREDE M\u00dc\u015eTER\u0130 KAYBETT\u0130N"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        SNew(SBox).Visibility_Lambda([G] { return G() && MarketDemand::TopProblems(G()->State, 1).Num() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
                        [ Fixed(TEXT("Kay\u0131p m\u00fc\u015fteri yok. Herkes arad\u0131\u011f\u0131n\u0131 buldu."), 11, ERole::Good) ]
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
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)
            [
                SNew(SBox).HeightOverride(150.f).VAlign(VAlign_Bottom)
                [
                    SNew(SBox)
                    .HeightOverride_Lambda([DayAt, Peak, Slot]() -> FOptionalSize { FMarketDayRecord R; return DayAt(Slot, R) ? FMath::Max(3.f, 150.f * static_cast<float>(FMath::Abs(R.Profit)) / static_cast<float>(Peak())) : 0.f; })
                    [
                        SNew(SBorder).BorderImage(&BadgeBrush)
                        .BorderBackgroundColor(ColBy([DayAt, Slot] { FMarketDayRecord R; return DayAt(Slot, R) && R.Profit < 0 ? ERole::Bad : ERole::Accent; }))
                    ]
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
            Label([G] { return G() && G()->State.LastWeekNumber > 0 ? FString::Printf(TEXT("%d. hafta"), G()->State.LastWeekNumber) : FString(TEXT("\u0130lk hafta raporu 7. g\u00fcn\u00fcn sonunda gelir. Grafik \u015fimdiden son g\u00fcnleri g\u00f6sterir.")); }, 13, ERole::Muted, true)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
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
    ];
}

TSharedRef<SWidget> SMarketMenu::ReportsPage()
{
    auto G = [this] { return Game.Get(); };
    auto Tab = [this](const FString& Text, bool bWeek) -> TSharedRef<SWidget>
    {
        return SNew(SButton).ButtonStyle(&PillStyle).IsFocusable(false).ContentPadding(FMargin(16.f, 6.f))
            .ButtonColorAndOpacity_Lambda([this, bWeek] { return FSlateColor(Color(bWeekTab == bWeek ? ERole::Primary : ERole::Button)); })
            .OnClicked_Lambda([this, bWeek] { bWeekTab = bWeek; return FReply::Handled(); })
            [
                SNew(STextBlock).Font(MarketMenuUi::MenuFont(true, 11)).Text(FText::FromString(Text))
                .ColorAndOpacity_Lambda([this, bWeek] { return FSlateColor(Color(bWeekTab == bWeek ? ERole::PrimaryText : ERole::ButtonText)); })
            ];
    };
    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[ Tab(TEXT("G\u00fcn sonu"), false) ]
            + SHorizontalBox::Slot().AutoWidth()[ Tab(TEXT("Hafta"), true) ]
        ]
        + SVerticalBox::Slot().FillHeight(1.f)
        [
            SNew(SWidgetSwitcher).WidgetIndex_Lambda([this] { return bWeekTab ? 1 : 0; })
            + SWidgetSwitcher::Slot()[ DayReport() ]
            + SWidgetSwitcher::Slot()[ WeekReport() ]
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
}

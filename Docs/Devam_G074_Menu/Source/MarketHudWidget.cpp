#include "MarketHudWidget.h"

#include "MarketGame.h"
#include "MarketMenuWidget.h"
#include "MarketVisuals.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

// Named namespace (the module is a unity build; MarketGame.cpp and the menu have their own helpers).
namespace MarketHudUi
{
    FSlateFontInfo HudFont(bool bBold, int32 Size, int32 Spacing = 0)
    {
        FSlateFontInfo Info = FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", Size);
        Info.LetterSpacing = Spacing;
        return Info;
    }

    // 1234.5 TL -> "1.234,50 TL"
    FString HudLira(int64 Kurus)
    {
        const bool bNegative = Kurus < 0;
        const int64 Abs = FMath::Abs(Kurus);
        FString Whole = FString::Printf(TEXT("%lld"), Abs / 100);
        for (int32 I = Whole.Len() - 3; I > 0; I -= 3) Whole.InsertAt(I, TEXT('.'));
        return FString::Printf(TEXT("%s%s,%02lld TL"), bNegative ? TEXT("-") : TEXT(""), *Whole, Abs % 100);
    }

    FString HudClock(const AMarketGameMode* G)
    {
        const int32 Minutes = 8 * 60 + (G && G->bOpen ? static_cast<int32>(G->DayTime * 3) : 0);
        return FString::Printf(TEXT("%02d:%02d"), Minutes / 60, Minutes % 60);
    }

    FLinearColor HudHex(const TCHAR* Code, float Alpha = 1.f)
    {
        FLinearColor Result(FColor::FromHex(Code));
        Result.A = Alpha;
        return Result;
    }
}

// ---------------------------------------------------------------------------------------------------------------
// Theme (the menu's light / dark choice, design tokens of "Miras Market Menu Tasarimi")

bool SMarketHud::IsLight() const
{
    const AMarketGameMode* G = Game.Get();
    return !G || G->bLightTheme;
}

FLinearColor SMarketHud::Color(ETone Tone) const
{
    using MarketHudUi::HudHex;
    const bool bLight = IsLight();
    switch (Tone)
    {
    case ETone::Card: return bLight ? HudHex(TEXT("FFFFFF"), 0.95f) : HudHex(TEXT("111315"), 0.88f);
    case ETone::Text: return bLight ? HudHex(TEXT("171A1E")) : HudHex(TEXT("F4F5F6"));
    case ETone::Muted: return bLight ? HudHex(TEXT("5F6670")) : HudHex(TEXT("AAB1BA"));
    case ETone::Accent: return bLight ? HudHex(TEXT("16775F")) : HudHex(TEXT("71C6AC"));
    case ETone::Good: return bLight ? HudHex(TEXT("16775F")) : HudHex(TEXT("9AD9C4"));
    case ETone::Bad: return bLight ? HudHex(TEXT("B4432C")) : HudHex(TEXT("E07A5F"));
    case ETone::Warn: return bLight ? HudHex(TEXT("9A6200")) : HudHex(TEXT("E8B86A"));
    case ETone::Info: return bLight ? HudHex(TEXT("2F5FA8")) : HudHex(TEXT("7FA7E0"));
    case ETone::KeyFill: return bLight ? HudHex(TEXT("171A1E")) : HudHex(TEXT("F4F5F6"));
    case ETone::KeyText: return bLight ? HudHex(TEXT("FFFFFF")) : HudHex(TEXT("111315"));
    case ETone::PrimaryFill: return bLight ? HudHex(TEXT("171A1E"), 0.96f) : HudHex(TEXT("71C6AC"));
    case ETone::PrimaryText: return bLight ? HudHex(TEXT("FFFFFF")) : HudHex(TEXT("0E1A17"));
    case ETone::Track: return bLight ? HudHex(TEXT("E6E8EB")) : HudHex(TEXT("2C3036"));
    case ETone::OpenFill: return bLight ? HudHex(TEXT("E3F2EC")) : HudHex(TEXT("1F3A33"));
    }
    return FLinearColor::White;
}

TAttribute<FSlateColor> SMarketHud::Col(ETone Tone) const
{
    return TAttribute<FSlateColor>::CreateLambda([this, Tone] { return FSlateColor(Color(Tone)); });
}

TSharedRef<SWidget> SMarketHud::Text(TFunction<FString()> Make, int32 Size, ETone Tone, bool bBold, bool bWrap)
{
    return SNew(STextBlock).Font(MarketHudUi::HudFont(bBold, Size)).ColorAndOpacity(Col(Tone)).AutoWrapText(bWrap)
        .Text_Lambda([Make] { return FText::FromString(Make ? Make() : FString()); });
}

TSharedRef<SWidget> SMarketHud::Fixed(const FString& Value, int32 Size, ETone Tone, bool bBold)
{
    return SNew(STextBlock).Font(MarketHudUi::HudFont(bBold, Size)).ColorAndOpacity(Col(Tone)).Text(FText::FromString(Value));
}

TSharedRef<SWidget> SMarketHud::Card(const TSharedRef<SWidget>& Content, const FMargin& Padding)
{
    return SNew(SBorder).BorderImage(&CardBrush).BorderBackgroundColor(Col(ETone::Card)).Padding(Padding)[ Content ];
}

TSharedRef<SWidget> SMarketHud::Bar(TFunction<float()> Value, ETone Tone, float Width)
{
    TSharedRef<SWidget> Progress = SNew(SBorder).BorderImage(&TrackBrush).BorderBackgroundColor(Col(ETone::Track)).Padding(0.f)
    [
        SNew(SProgressBar).Style(&BarStyle).BorderPadding(FVector2D(0.f, 0.f)).FillColorAndOpacity(Col(Tone))
        .Percent_Lambda([Value]() -> TOptional<float> { return FMath::Clamp(Value ? Value() : 0.f, 0.f, 1.f); })
    ];
    if (Width > 0.f) return SNew(SBox).WidthOverride(Width).HeightOverride(4.f)[ Progress ];
    return SNew(SBox).HeightOverride(4.f)[ Progress ];
}

TSharedRef<SWidget> SMarketHud::KeyCap(TFunction<FString()> Key)
{
    return SNew(SBorder).BorderImage(&KeyBrush).BorderBackgroundColor(Col(ETone::KeyFill)).Padding(FMargin(7.f, 2.f)).VAlign(VAlign_Center)
    [
        SNew(STextBlock).Font(MarketHudUi::HudFont(true, 10)).ColorAndOpacity(Col(ETone::KeyText))
        .Text_Lambda([Key] { return FText::FromString(Key ? Key() : FString()); })
    ];
}

TSharedRef<SWidget> SMarketHud::KeyRow(const FString& Key, const FString& Label)
{
    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ KeyCap([Key] { return Key; }) ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 3.f, 18.f, 3.f)[ Fixed(Label, 10, ETone::Text) ];
}

TSharedRef<SWidget> SMarketHud::Divider()
{
    return SNew(SBox).WidthOverride(1.f).HeightOverride(34.f)[ SNew(SBorder).BorderImage(&TrackBrush).BorderBackgroundColor(Col(ETone::Track)) ];
}

// ---------------------------------------------------------------------------------------------------------------

void SMarketHud::Construct(const FArguments& InArgs)
{
    Game = InArgs._Game;
    BarStyle = FProgressBarStyle().SetBackgroundImage(TrackBrush).SetFillImage(TrackBrush).SetMarqueeImage(TrackBrush);
    auto NotArranging = [this] { const AMarketGameMode* G = Game.Get(); return G && G->bArrange ? EVisibility::Collapsed : EVisibility::HitTestInvisible; };
    auto Details = [this] { const AMarketGameMode* G = Game.Get(); return G && G->bShowDetails ? EVisibility::HitTestInvisible : EVisibility::Collapsed; };

    ChildSlot
    [
        SNew(SOverlay)
        // The menu covers the screen and has its own status: the game screen steps aside.
        .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && G->bMenuOpen ? EVisibility::Collapsed : EVisibility::HitTestInvisible; })
        + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(24.f, 20.f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left)[ StatusBar() ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f).HAlign(HAlign_Left)
            [ SNew(SBox).Visibility_Lambda(Details)[ StockCard() ] ]
        ]
        + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(24.f, 20.f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ ArrangeCard() ]
            + SVerticalBox::Slot().AutoHeight()[ SNew(SBox).Visibility_Lambda(NotArranging)[ Notices() ] ]
        ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [ SNew(SBox).WidthOverride(6.f).HeightOverride(6.f)[ SNew(SBorder).BorderImage(&CrossBrush) ] ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.f, 0.f, 0.f, 120.f)
        [ ReportCard() ]
        + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(24.f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[ SNew(SBox).Visibility_Lambda(Details)[ ControlsCard() ] ]
            + SVerticalBox::Slot().AutoHeight()[ TodayCards() ]
        ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 28.f)
        [ HintCard() ]
        + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(24.f)
        [ MenuButtons() ]
    ];
}

TSharedRef<SWidget> SMarketHud::StatusBar()
{
    auto G = [this] { return Game.Get(); };
    auto Column = [this](const FString& Heading, TFunction<FString()> Value) -> TSharedRef<SWidget>
    {
        return SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ Fixed(Heading, 9, ETone::Muted) ]
            + SVerticalBox::Slot().AutoHeight()[ Text(Value, 17, ETone::Text, true) ];
    };
    return Card(
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ Text([G] { return G() ? FString::Printf(TEXT("Gün %d · %s"), G()->State.Day, *MarketDirector::DateText(G()->State)) : FString(); }, 9, ETone::Muted) ]
            + SVerticalBox::Slot().AutoHeight()[ Text([G] { return MarketHudUi::HudClock(G()); }, 17, ETone::Text, true) ]
        ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16.f, 0.f)[ Divider() ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
        [ Column(TEXT("Kasa"), [G] { return G() ? MarketHudUi::HudLira(G()->State.Cash) : FString(); }) ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16.f, 0.f)
        [ SNew(SBox).Visibility_Lambda([G] { return G() && MarketCampaign::DebtOpen(G()->State) ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })[ Divider() ] ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
        [
            SNew(SBox).WidthOverride(140.f)
            .Visibility_Lambda([G] { return G() && MarketCampaign::DebtOpen(G()->State) ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ Text([G] { return G() ? FString::Printf(TEXT("Borç · %s kaldı"), *MarketHudUi::HudLira(G()->State.InheritedDebt)) : FString(); }, 9, ETone::Muted) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 7.f, 0.f, 0.f)[ Bar([G] { return G() ? MarketCampaign::DebtProgress(G()->State) : 0.f; }, ETone::Text) ]
            ]
        ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16.f, 0.f, 0.f, 0.f)
        [
            SNew(SBorder).BorderImage(&PillBrush).Padding(FMargin(12.f, 4.f))
            .BorderBackgroundColor_Lambda([this, G] { return FSlateColor(Color(G() && G()->bOpen ? ETone::OpenFill : ETone::Track)); })
            [
                SNew(STextBlock).Font(MarketHudUi::HudFont(true, 11))
                .ColorAndOpacity_Lambda([this, G] { return FSlateColor(Color(G() && G()->bOpen ? ETone::Accent : ETone::Muted)); })
                .Text_Lambda([G] { return FText::FromString(G() && G()->bOpen ? TEXT("Açık") : TEXT("Kapalı")); })
            ]
        ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
        [
            SNew(SBorder).BorderImage(&PillBrush).Padding(FMargin(10.f, 4.f)).BorderBackgroundColor(Col(ETone::Warn))
            .Visibility_Lambda([G] { return G() && G()->bTestMode ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
            [ SNew(STextBlock).Font(MarketHudUi::HudFont(true, 10)).ColorAndOpacity(FLinearColor::White).Text(FText::FromString(TEXT("TEST · F2"))) ]
        ],
        FMargin(18.f, 10.f));
}

TSharedRef<SWidget> SMarketHud::Notices()
{
    TSharedRef<SVerticalBox> List = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 3; ++Slot)
    {
        auto Todo = [this, Slot]() -> const FMarketTodo* { const AMarketGameMode* G = Game.Get(); return G && G->Todos().IsValidIndex(Slot) ? &G->Todos()[Slot] : nullptr; };
        List->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
        [
            SNew(SBox).WidthOverride(340.f)
            .Visibility_Lambda([Todo] { return Todo() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
            [
                Card(
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top).Padding(0.f, 5.f, 12.f, 0.f)
                    [
                        SNew(SBox).WidthOverride(8.f).HeightOverride(8.f)
                        [
                            SNew(SBorder).BorderImage(&DotBrush)
                            .BorderBackgroundColor_Lambda([this, Todo] { const FMarketTodo* T = Todo(); return FSlateColor(Color(!T ? ETone::Muted : T->Severity >= 2 ? ETone::Bad : T->Severity == 1 ? ETone::Warn : ETone::Info)); })
                        ]
                    ]
                    + SHorizontalBox::Slot().FillWidth(1.f)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()[ Text([Todo] { const FMarketTodo* T = Todo(); return T ? T->Title : FString(); }, 11, ETone::Text, true, true) ]
                        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)[ Text([Todo] { const FMarketTodo* T = Todo(); return T ? T->Text : FString(); }, 10, ETone::Text, false, true) ]
                        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
                        [ Text([Todo] { const FMarketTodo* T = Todo(); return T ? FString::Printf(TEXT("M › %s"), SMarketMenu::PageName(T->Page)) : FString(); }, 9, ETone::Muted, true) ]
                    ],
                    FMargin(14.f, 12.f))
            ]
        ];
    }
    // The last notice (a deal, a warning, a sale) floats under the list for a few seconds.
    List->AddSlot().AutoHeight()
    [
        SNew(SBox).WidthOverride(340.f)
        .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && G->MessageTime > 0.f && !G->Message.IsEmpty() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
        [ Card(Text([this] { const AMarketGameMode* G = Game.Get(); return G ? G->Message : FString(); }, 10, ETone::Text, false, true), FMargin(14.f, 10.f)) ]
    ];
    return List;
}

TSharedRef<SWidget> SMarketHud::TodayCards()
{
    auto G = [this] { return Game.Get(); };
    auto Small = [this](const FString& Heading, TFunction<FString()> Value) -> TSharedRef<SWidget>
    {
        return Card(SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ Fixed(Heading, 9, ETone::Muted) ]
            + SVerticalBox::Slot().AutoHeight()[ Text(Value, 13, ETone::Text, true) ], FMargin(14.f, 9.f));
    };
    return SNew(SHorizontalBox)
        .Visibility_Lambda([G] { return G() && G()->bOpen && !G()->bArrange ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
        [ Small(TEXT("Kasa kuyruğu"), [G] { return !G() ? FString() : G()->QueueSize() > 0 ? FString::Printf(TEXT("%d müşteri bekliyor"), G()->QueueSize()) : FString(TEXT("Kimse beklemiyor")); }) ]
        + SHorizontalBox::Slot().AutoWidth()
        [ Small(TEXT("Bugün"), [G] { return G() ? FString::Printf(TEXT("%d satış · %d kayıp · %s"), G()->State.Served, G()->State.Lost, *MarketHudUi::HudLira(G()->State.Revenue)) : FString(); }) ];
}

FString SMarketHud::HintKey() const
{
    const AMarketGameMode* G = Game.Get();
    if (!G) return FString();
    const FString Hint = G->ContextHint();
    int32 Colon = INDEX_NONE;
    return Hint.FindChar(TEXT(':'), Colon) && Colon > 0 && Colon <= 4 ? Hint.Left(Colon) : FString();
}

FString SMarketHud::HintText() const
{
    const AMarketGameMode* G = Game.Get();
    if (!G) return FString();
    const FString Hint = G->ContextHint();
    const FString Key = HintKey();
    return Key.IsEmpty() ? Hint : Hint.Mid(Key.Len() + 1).TrimStart();
}

TSharedRef<SWidget> SMarketHud::HintCard()
{
    return SNew(SBorder).BorderImage(&PillBrush).BorderBackgroundColor(Col(ETone::Card)).Padding(FMargin(16.f, 8.f))
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([this] { return HintKey().IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible; })
            [ KeyCap([this] { return HintKey(); }) ]
        ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Text([this] { return HintText(); }, 12, ETone::Text) ]
    ];
}

TSharedRef<SWidget> SMarketHud::MenuButtons()
{
    auto G = [this] { return Game.Get(); };
    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
        [
            SNew(SBorder).BorderImage(&PillBrush).BorderBackgroundColor(Col(ETone::Card)).Padding(FMargin(14.f, 9.f))
            .Visibility_Lambda([G] { return G() && !G()->bArrange ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)[ KeyCap([] { return FString(TEXT("O")); }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [ Text([G] { return G() && G()->bOpen ? FString(TEXT("Günü kapat")) : FString(TEXT("Marketi aç")); }, 12, ETone::Text, true) ]
            ]
        ]
        + SHorizontalBox::Slot().AutoWidth()
        [
            SNew(SBorder).BorderImage(&PillBrush).BorderBackgroundColor(Col(ETone::PrimaryFill)).Padding(FMargin(16.f, 9.f))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)[ Fixed(TEXT("Yönetim"), 12, ETone::PrimaryText, true) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [
                    SNew(SBorder).BorderImage(&KeyBrush).BorderBackgroundColor(Col(ETone::PrimaryText)).Padding(FMargin(6.f, 1.f))
                    [ SNew(STextBlock).Font(MarketHudUi::HudFont(true, 10)).ColorAndOpacity(Col(ETone::PrimaryFill)).Text(FText::FromString(TEXT("M"))) ]
                ]
            ]
        ];
}

TSharedRef<SWidget> SMarketHud::StockCard()
{
    // F1: a quick look at the shelves around the selected product (the full table is Yonetim > Siparis).
    TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f)[ Fixed(TEXT("STOK"), 9, ETone::Muted, true) ]
            + SHorizontalBox::Slot().AutoWidth()[ SNew(SBox).WidthOverride(120.f)[ Fixed(TEXT("RAF"), 9, ETone::Muted, true) ] ]
            + SHorizontalBox::Slot().AutoWidth()[ SNew(SBox).WidthOverride(44.f)[ Fixed(TEXT("DEPO"), 9, ETone::Muted, true) ] ]
            + SHorizontalBox::Slot().AutoWidth()[ SNew(SBox).WidthOverride(44.f)[ Fixed(TEXT("KABUL"), 9, ETone::Muted, true) ] ]
        ];
    const AMarketGameMode* G0 = Game.Get();
    const int32 Count = G0 ? G0->Products.Num() : 0;
    constexpr int32 Window = 9;
    for (int32 I = 0; I < Count; ++I)
    {
        auto InWindow = [this, I, Count]
        {
            const AMarketGameMode* G = Game.Get();
            if (!G) return false;
            const int32 First = FMath::Clamp(G->Selected - Window / 2, 0, FMath::Max(0, Count - Window));
            return I >= First && I < First + Window;
        };
        auto Stock = [this, I]() -> const FMarketStock* { const AMarketGameMode* G = Game.Get(); return G && G->State.Stock.IsValidIndex(I) ? &G->State.Stock[I] : nullptr; };
        Rows->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SHorizontalBox)
            .Visibility_Lambda([InWindow] { return InWindow() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [ Text([this, I] { const AMarketGameMode* G = Game.Get(); return G && G->Products.IsValidIndex(I) ? G->ProductName(I) : FString(); }, 10, ETone::Text) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12.f, 0.f, 0.f, 0.f)
            [
                SNew(SBox).WidthOverride(108.f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Bar([Stock] { const FMarketStock* S = Stock(); return S ? S->Shelf / static_cast<float>(FMath::Max(1, S->Capacity)) : 0.f; }, ETone::Accent) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
                    [ Text([Stock] { const FMarketStock* S = Stock(); return S ? FString::Printf(TEXT("%d / %d"), S->Shelf, S->Capacity) : FString(); }, 8, ETone::Muted) ]
                ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12.f, 0.f, 0.f, 0.f)
            [ SNew(SBox).WidthOverride(44.f)[ Text([Stock] { const FMarketStock* S = Stock(); return S ? FString::FromInt(S->Warehouse) : FString(); }, 10, ETone::Text) ] ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ SNew(SBox).WidthOverride(44.f)[ Text([Stock] { const FMarketStock* S = Stock(); return S ? FString::FromInt(S->Dock) : FString(); }, 10, ETone::Warn) ] ]
        ];
    }
    return SNew(SBox).MinDesiredWidth(380.f)[ Card(Rows) ];
}

TSharedRef<SWidget> SMarketHud::ControlsCard()
{
    return Card(
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[ Fixed(TEXT("KISAYOLLAR"), 9, ETone::Muted, true) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SUniformGridPanel).SlotPadding(FMargin(0.f, 2.f))
            + SUniformGridPanel::Slot(0, 0)[ KeyRow(TEXT("WASD"), TEXT("Yürü")) ]
            + SUniformGridPanel::Slot(1, 0)[ KeyRow(TEXT("E"), TEXT("Etkileşim")) ]
            + SUniformGridPanel::Slot(2, 0)[ KeyRow(TEXT("M"), TEXT("Yönetim menüsü")) ]
            + SUniformGridPanel::Slot(0, 1)[ KeyRow(TEXT("O"), TEXT("Marketi aç / kapat")) ]
            + SUniformGridPanel::Slot(1, 1)[ KeyRow(TEXT("R"), TEXT("Reyonu diz (kapalıyken)")) ]
            + SUniformGridPanel::Slot(2, 1)[ KeyRow(TEXT("F1"), TEXT("Bu paneller")) ]
            + SUniformGridPanel::Slot(0, 2)[ KeyRow(TEXT("F2"), TEXT("Test modu")) ]
            + SUniformGridPanel::Slot(1, 2)[ KeyRow(TEXT("F3"), TEXT("Rafları doldur (test)")) ]
            + SUniformGridPanel::Slot(2, 2)[ KeyRow(TEXT("F4"), TEXT("Işık")) ]
            + SUniformGridPanel::Slot(0, 3)[ KeyRow(TEXT("F5 / F9"), TEXT("Kaydet / yükle")) ]
            + SUniformGridPanel::Slot(1, 3)[ KeyRow(TEXT("F6"), TEXT("Yeni kampanya")) ]
            + SUniformGridPanel::Slot(2, 3)[ KeyRow(TEXT("F8"), TEXT("Marka adları")) ]
            + SUniformGridPanel::Slot(0, 4)[ KeyRow(TEXT("F11"), TEXT("Tam ekran")) ]
            + SUniformGridPanel::Slot(1, 4)[ KeyRow(TEXT("Esc"), TEXT("Oyundan çık")) ]
        ]);
}

TSharedRef<SWidget> SMarketHud::ReportCard()
{
    auto Line = [this](TFunction<FString(const AMarketGameMode&)> Make, int32 Size, ETone Tone, bool bBold) -> TSharedRef<SWidget>
    {
        return Text([this, Make] { const AMarketGameMode* G = Game.Get(); return G ? Make(*G) : FString(); }, Size, Tone, bBold, true);
    };
    return SNew(SBox).MinDesiredWidth(520.f).MaxDesiredWidth(600.f)
        .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && !G->bOpen && G->State.Day > 1 && G->ReportTime > 0.f ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
    [
        Card(
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ Line([](const AMarketGameMode& G) { return FString::Printf(TEXT("GÜN %d RAPORU"), G.State.Day - 1); }, 9, ETone::Muted, true) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
            [ Line([](const AMarketGameMode& G) { return FString::Printf(TEXT("Net sonuç  %s"), *MarketHudUi::HudLira(G.State.LastProfit)); }, 20, ETone::Text, true) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
            [ Line([](const AMarketGameMode& G) { return FString::Printf(TEXT("Ciro %s  ·  mal maliyeti %s  ·  gider %s"), *MarketHudUi::HudLira(G.State.LastRevenue), *MarketHudUi::HudLira(G.State.LastCostOfGoods), *MarketHudUi::HudLira(G.State.LastOperatingCost)); }, 11, ETone::Text, false) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
            [ Line([](const AMarketGameMode& G) { return FString::Printf(TEXT("%d satış  ·  %d kayıp müşteri"), G.State.LastServed, G.State.LastLost); }, 11, ETone::Text, false) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
            [ Line([](const AMarketGameMode& G) { return G.DayProblemsText(); }, 10, ETone::Warn, false) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
            [ Line([](const AMarketGameMode& G) { return G.RivalNewsText(); }, 10, ETone::Text, false) ],
            FMargin(24.f, 18.f))
    ];
}

TSharedRef<SWidget> SMarketHud::ArrangeCard()
{
    // Everything about shelf arranging in one place: where the crosshair is, what is in hand, the block under
    // the crosshair, what a click would do (green / red) and only the keys that work right now.
    auto Str = [this](FString AMarketGameMode::* Field)
    {
        return [this, Field] { const AMarketGameMode* G = Game.Get(); return G ? G->*Field : FString(); };
    };
    TSharedRef<SVerticalBox> Keys = SNew(SVerticalBox);
    for (int32 I = 0; I < 14; ++I)
    {
        Keys->AddSlot().AutoHeight().Padding(0.f, 2.f)
        [
            SNew(SHorizontalBox)
            .Visibility_Lambda([this, I] { const AMarketGameMode* G = Game.Get(); return G && G->ArrangeKeys.IsValidIndex(I) ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ KeyCap([this, I] { const AMarketGameMode* G = Game.Get(); return G && G->ArrangeKeys.IsValidIndex(I) ? G->ArrangeKeys[I].Key : FString(); }) ]
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
            [ Text([this, I] { const AMarketGameMode* G = Game.Get(); return G && G->ArrangeKeys.IsValidIndex(I) ? G->ArrangeKeys[I].Value : FString(); }, 10, ETone::Text, false, true) ]
        ];
    }
    return SNew(SBox).WidthOverride(400.f)
        .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && G->bArrange ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
    [
        Card(
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("RAF DÜZENİ"), 9, ETone::Muted, true) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)[ Text(Str(&AMarketGameMode::ArrangeTitle), 15, ETone::Text, true, true) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 4.f)[ Text(Str(&AMarketGameMode::ArrangeRow), 9, ETone::Muted, false, true) ]
            + SVerticalBox::Slot().AutoHeight()[ Bar([this] { const AMarketGameMode* G = Game.Get(); return G ? G->ArrangeRowFill : 0.f; }, ETone::Accent) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)[ Text(Str(&AMarketGameMode::ArrangeHandTitle), 12, ETone::Accent, true, true) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)[ Text(Str(&AMarketGameMode::ArrangeHandText), 10, ETone::Text, false, true) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
            [
                SNew(SVerticalBox)
                .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && !G->ArrangeTargetTitle.IsEmpty() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
                + SVerticalBox::Slot().AutoHeight()[ Text(Str(&AMarketGameMode::ArrangeTargetTitle), 12, ETone::Warn, true, true) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)[ Text(Str(&AMarketGameMode::ArrangeTargetText), 10, ETone::Text, false, true) ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
            [
                SNew(STextBlock).Font(MarketHudUi::HudFont(true, 11)).AutoWrapText(true)
                .Text_Lambda([this] { const AMarketGameMode* G = Game.Get(); return FText::FromString(G ? G->ArrangeStatus : FString()); })
                .ColorAndOpacity_Lambda([this] { const AMarketGameMode* G = Game.Get(); return FSlateColor(Color(G && G->bArrangeStatusOk ? ETone::Good : ETone::Bad)); })
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[ Fixed(TEXT("TUŞLAR"), 9, ETone::Muted, true) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)[ Keys ])
    ];
}

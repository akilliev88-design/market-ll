#include "MarketHudWidget.h"
#include "MarketCountry.h"

#include "MarketGame.h"
#include "MarketStoreVisit.h"
#include "MarketMenuWidget.h"
#include "MarketVisuals.h"
#include "MarketTheme.h"
#include "Widgets/Images/SImage.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SDPIScaler.h"
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
        // G-086d: the menu's design language (IBM Plex Sans; big bold text in Bricolage Grotesque). Size in points.
        using MarketTheme::EFace;
        FSlateFontInfo Info = bBold && Size >= 17 ? MarketTheme::Font(EFace::DisplayBold, Size / 0.75f)
            : MarketTheme::Font(bBold ? EFace::Semi : EFace::Regular, Size / 0.75f);
        Info.LetterSpacing = Spacing;
        return Info;
    }

    // 1234.5 TL -> "1.234,50 TL"
    FString HudLira(int64 Kurus)
    {
        return MarketCountry::Money(Kurus); // G-084: the active country\'s currency
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
// Theme (the menu's light / dark choice, design tokens of "MarketSim Menu Tasarimi")

bool SMarketHud::IsLight() const
{
    const AMarketGameMode* G = Game.Get();
    return !G || G->bLightTheme;
}

FLinearColor SMarketHud::Color(ETone Tone) const
{
    using MarketHudUi::HudHex;
    const bool bLight = IsLight();
    // G-086d: the tokens of the design boards (the menu uses the same).
    switch (Tone)
    {
    case ETone::Card: return bLight ? HudHex(TEXT("FFFFFF"), 0.97f) : HudHex(TEXT("1B1E22"), 0.94f);
    case ETone::Text: return bLight ? HudHex(TEXT("1B1E22")) : HudHex(TEXT("F2F3F4"));
    case ETone::Muted: return bLight ? HudHex(TEXT("6B7178")) : HudHex(TEXT("9AA3AC"));
    case ETone::Accent: return bLight ? HudHex(TEXT("2F8A70")) : HudHex(TEXT("71C6AC"));
    case ETone::Good: return bLight ? HudHex(TEXT("2F8A70")) : HudHex(TEXT("71C6AC"));
    case ETone::Bad: return bLight ? HudHex(TEXT("C4453A")) : HudHex(TEXT("F07F6E"));
    case ETone::Warn: return bLight ? HudHex(TEXT("D08A1E")) : HudHex(TEXT("E8A94E"));
    case ETone::Info: return bLight ? HudHex(TEXT("2F5FA8")) : HudHex(TEXT("7FA7E0"));
    case ETone::KeyFill: return bLight ? HudHex(TEXT("1B1E22")) : HudHex(TEXT("F2F3F4"));
    case ETone::KeyText: return bLight ? HudHex(TEXT("FFFFFF")) : HudHex(TEXT("101316"));
    case ETone::PrimaryFill: return bLight ? HudHex(TEXT("2F8A70")) : HudHex(TEXT("71C6AC"));
    case ETone::PrimaryText: return bLight ? HudHex(TEXT("FFFFFF")) : HudHex(TEXT("0F1A16"));
    case ETone::Track: return bLight ? HudHex(TEXT("EDE9E1")) : HudHex(TEXT("2C3137"));
    case ETone::OpenFill: return bLight ? HudHex(TEXT("E3F2EC")) : HudHex(TEXT("1F3A33"));
    case ETone::WarnSoft: return bLight ? HudHex(TEXT("FFF4E2")) : HudHex(TEXT("3A2E17"));
    case ETone::BadSoft: return bLight ? HudHex(TEXT("F8E1DC")) : HudHex(TEXT("3D2220"));
    case ETone::InfoSoft: return bLight ? HudHex(TEXT("E3F2EC")) : HudHex(TEXT("1F3A33"));
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
    return Raised(SNew(SBorder).BorderImage(&CardBrush).BorderBackgroundColor(Col(ETone::Card)).Padding(Padding)[ Content ]);
}

TSharedRef<SWidget> SMarketHud::Raised(const TSharedRef<SWidget>& Surface)
{
    // G-086e: no image shadow (it showed as grey boxes); the brushes carry a faint outline.
    return Surface;
}

TSharedRef<SWidget> SMarketHud::Pill(const TSharedRef<SWidget>& Content, float Height, TFunction<ETone()> Fill)
{
    return Raised(SNew(SBox).HeightOverride(Height)
    [
        SNew(SBorder).BorderImage(&PillBrush).Padding(FMargin(20.f, 0.f)).VAlign(VAlign_Center)
        .BorderBackgroundColor_Lambda([this, Fill] { return FSlateColor(Color(Fill ? Fill() : ETone::Card)); })
        [ Content ]
    ]);
}

TSharedRef<SWidget> SMarketHud::Mono(TFunction<FString()> Make, float Pixels, TFunction<ETone()> Tone)
{
    return SNew(STextBlock).Font(MarketTheme::Font(MarketTheme::EFace::Mono, Pixels))
        .ColorAndOpacity_Lambda([this, Tone] { return FSlateColor(Color(Tone ? Tone() : ETone::Text)); })
        .Text_Lambda([Make] { return FText::FromString(Make ? Make() : FString()); });
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
    PillBrush.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
    DotBrush.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
    BarStyle = FProgressBarStyle().SetBackgroundImage(TrackBrush).SetFillImage(TrackBrush).SetMarqueeImage(TrackBrush);
    auto NotArranging = [this] { const AMarketGameMode* G = Game.Get(); return G && G->bArrange ? EVisibility::Collapsed : EVisibility::HitTestInvisible; };
    auto Details = [this] { const AMarketGameMode* G = Game.Get(); return G && G->bShowDetails ? EVisibility::HitTestInvisible : EVisibility::Collapsed; };

    ChildSlot
    [
        // G-075: the HUD is laid out for 1600 x 900 and scales with the screen (and the menu's text size setting).
        SNew(SDPIScaler).DPIScale_Lambda([this] { return UiScale(); })
        [
        SNew(SOverlay)
        // The menu covers the screen and has its own status: the game screen steps aside.
        .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && G->bMenuOpen ? EVisibility::Collapsed : EVisibility::HitTestInvisible; })
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0.f, 24.f, 0.f, 0.f)
        [ SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ Card(Text([this] { const auto* G = Game.Get(); return G ? G->StoreEntryHeader() : FString(); }, 11, ETone::Text), FMargin(12.f, 5.f)) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)[ PausedBanner() ] ]
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
        ]
    ];
}

void SMarketHud::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
    LastSize = FVector2D(AllottedGeometry.GetLocalSize());
}

float SMarketHud::UiScale() const
{
    const FVector2D Size = LastSize.X > 1.0 && LastSize.Y > 1.0 ? LastSize : FVector2D(1920.0, 1080.0);
    const float Fit = FMath::Min(static_cast<float>(Size.X) / 1600.f, static_cast<float>(Size.Y) / 900.f);
    const AMarketGameMode* G = Game.Get();
    return FMath::Clamp(Fit * (G ? G->UiTextFactor() : 1.f), 0.5f, 4.f);
}

TSharedRef<SWidget> SMarketHud::SpeedPill()
{
    auto G = [this] { return Game.Get(); };
    return SNew(SBox).HeightOverride(28.f)
    [
        SNew(SBorder).BorderImage(&PillBrush).Padding(FMargin(12.f, 0.f)).VAlign(VAlign_Center)
        .BorderBackgroundColor_Lambda([this, G] { return FSlateColor(Color(G() && G()->bTimePaused ? ETone::WarnSoft : ETone::Track)); })
        [
            SNew(STextBlock).Font(MarketHudUi::HudFont(true, 9))
            .ColorAndOpacity_Lambda([this, G] { return FSlateColor(Color(G() && G()->bTimePaused ? ETone::Warn : ETone::Text)); })
            .Text_Lambda([G]
            {
                if (!G()) return FText();
                return FText::FromString(G()->bTimePaused ? FString(TEXT("II  Durdu")) : FString::Printf(TEXT("%dx"), G()->GameSpeed));
            })
        ]
    ];
}

TSharedRef<SWidget> SMarketHud::PausedBanner()
{
    return SNew(SBox)
        .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && G->bTimePaused ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
    [
        Card(SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)[ Fixed(TEXT("ZAMAN DURDU"), 13, ETone::Warn, true) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)[ KeyCap([] { return FString(TEXT("Space")); }) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Fixed(TEXT("devam \u00b7 1 / 2 / 3 h\u0131z"), 11, ETone::Muted) ],
            FMargin(18.f, 9.f))
    ];
}

TSharedRef<SWidget> SMarketHud::StatusBar()
{
    // G-086d: the pills of the main screen. The date pill (day, clock, open or closed, speed) and the till pill
    // (cash, today, the inherited debt while it is open).
    auto G = [this] { return Game.Get(); };
    auto Column = [this](const FString& Heading, TFunction<FString()> Value, TFunction<ETone()> Tone) -> TSharedRef<SWidget>
    {
        return SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)[ Fixed(Heading, 8, ETone::Muted) ]
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)[ Mono(Value, 16.f, Tone) ];
    };
    TSharedRef<SWidget> Date = Pill(
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ Text([G] { return G() ? MarketDirector::DateText(G()->State) : FString(); }, 11, ETone::Text, true) ]
            + SVerticalBox::Slot().AutoHeight()
            [ Text([G] { return G() ? FString::Printf(TEXT("G\u00fcn %d \u00b7 %s"), G()->State.Day, *MarketHudUi::HudClock(G())) : FString(); }, 8, ETone::Muted) ]
        ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(14.f, 0.f, 0.f, 0.f)
        [
            SNew(SBox).HeightOverride(28.f)
            [
                SNew(SBorder).BorderImage(&PillBrush).Padding(FMargin(12.f, 0.f)).VAlign(VAlign_Center)
                .BorderBackgroundColor_Lambda([this, G] { return FSlateColor(Color(G() && G()->bOpen ? ETone::OpenFill : ETone::Track)); })
                [
                    SNew(STextBlock).Font(MarketHudUi::HudFont(true, 9))
                    .ColorAndOpacity_Lambda([this, G] { return FSlateColor(Color(G() && G()->bOpen ? ETone::Accent : ETone::Muted)); })
                    .Text_Lambda([G] { return FText::FromString(G() && G()->bOpen ? TEXT("A\u00e7\u0131k") : TEXT("Kapal\u0131")); })
                ]
            ]
        ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f, 0.f, 0.f)[ SpeedPill() ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f, 0.f, 0.f)
        [
            SNew(SBox).HeightOverride(28.f).Visibility_Lambda([G] { return G() && G()->bTestMode ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
            [
                SNew(SBorder).BorderImage(&PillBrush).Padding(FMargin(10.f, 0.f)).VAlign(VAlign_Center).BorderBackgroundColor(Col(ETone::WarnSoft))
                [ SNew(STextBlock).Font(MarketHudUi::HudFont(true, 9)).ColorAndOpacity(Col(ETone::Warn)).Text(FText::FromString(TEXT("TEST \u00b7 F2"))) ]
            ]
        ],
        52.f);
    TSharedRef<SWidget> Till = Pill(
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
        [ Column(TEXT("Kasa"), [G] { return G() ? MarketHudUi::HudLira(G()->State.Cash) : FString(); }, [G] { return G() && G()->State.Cash < 0 ? ETone::Bad : ETone::Text; }) ]
        // M37: our own money next to the company's till.
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(22.f, 0.f, 0.f, 0.f)
        [ Column(TEXT("Servet"), [G] { return G() ? MarketHudUi::HudLira(G()->State.Owner.Wealth) : FString(); }, [] { return ETone::Text; }) ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(22.f, 0.f, 0.f, 0.f)
        [ Column(TEXT("Bug\u00fcn"), [G] { return G() ? MarketHudUi::HudLira(G()->State.Revenue) : FString(); }, [] { return ETone::Accent; }) ],
        52.f);
    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth()[ Date ]
        + SHorizontalBox::Slot().AutoWidth().Padding(12.f, 0.f, 0.f, 0.f)[ Till ];
}

TSharedRef<SWidget> SMarketHud::Notices()
{
    TSharedRef<SVerticalBox> List = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 3; ++Slot)
    {
        auto Todo = [this, Slot]() -> const FMarketTodo* { const AMarketGameMode* G = Game.Get(); return G && G->Todos().IsValidIndex(Slot) ? &G->Todos()[Slot] : nullptr; };
        List->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
        [
            SNew(SBox).WidthOverride(360.f)
            .Visibility_Lambda([Todo] { return Todo() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
            [
                Card(
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top).Padding(0.f, 0.f, 14.f, 0.f)
                    [
                        // The design's round sign: "!" on a soft warm (or red) disc, "i" on a soft green one.
                        SNew(SBox).WidthOverride(36.f).HeightOverride(36.f)
                        [
                            SNew(SBorder).BorderImage(&DotBrush).HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.f)
                            .BorderBackgroundColor_Lambda([this, Todo] { const FMarketTodo* T = Todo(); return FSlateColor(Color(!T ? ETone::Track : T->Severity >= 2 ? ETone::BadSoft : T->Severity == 1 ? ETone::WarnSoft : ETone::InfoSoft)); })
                            [
                                SNew(STextBlock).Font(MarketTheme::Font(MarketTheme::EFace::DisplayBold, 17.f))
                                .Text_Lambda([Todo] { const FMarketTodo* T = Todo(); return FText::FromString(T && T->Severity >= 1 ? TEXT("!") : TEXT("i")); })
                                .ColorAndOpacity_Lambda([this, Todo] { const FMarketTodo* T = Todo(); return FSlateColor(Color(!T ? ETone::Muted : T->Severity >= 2 ? ETone::Bad : T->Severity == 1 ? ETone::Warn : ETone::Accent)); })
                            ]
                        ]
                    ]
                    + SHorizontalBox::Slot().FillWidth(1.f)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()[ Text([Todo] { const FMarketTodo* T = Todo(); return T ? T->Title : FString(); }, 11, ETone::Text, true, true) ]
                        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)[ Text([Todo] { const FMarketTodo* T = Todo(); return T ? T->Text : FString(); }, 9, ETone::Muted, false, true) ]
                        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
                        [ Text([Todo] { const FMarketTodo* T = Todo(); return T ? FString::Printf(TEXT("M \u203a %s"), SMarketMenu::PageName(T->Page)) : FString(); }, 9, ETone::Accent, true) ]
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
        return Pill(SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ Fixed(Heading, 8, ETone::Muted) ]
            + SVerticalBox::Slot().AutoHeight()[ Text(Value, 11, ETone::Text, true) ], 52.f);
    };
    return SNew(SHorizontalBox)
        .Visibility_Lambda([G] { return G() && G()->bOpen && !G()->bArrange ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 10.f, 0.f)
        [ Small(TEXT("Kasa kuyru\u011fu"), [G] { return !G() ? FString() : G()->QueueSize() > 0 ? FString::Printf(TEXT("%d m\u00fc\u015fteri bekliyor"), G()->QueueSize()) : FString(TEXT("Kimse beklemiyor")); }) ]
        + SHorizontalBox::Slot().AutoWidth()
        [ Small(TEXT("Bug\u00fcn"), [G] { return G() ? FString::Printf(TEXT("%d sat\u0131\u015f \u00b7 %d kay\u0131p"), G()->State.Served, G()->State.Lost) : FString(); }) ];
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
    return Pill(
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([this] { return HintKey().IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible; })
            [ KeyCap([this] { return HintKey(); }) ]
        ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Text([this] { return HintText(); }, 11, ETone::Text) ],
        44.f);
}

TSharedRef<SWidget> SMarketHud::MenuButtons()
{
    // The management menu from inside the store: an accent pill with M.
    auto G = [this] { return Game.Get(); };
    auto Icon = [this](const FString& Name, ETone Tone) -> TSharedRef<SWidget>
    {
        const FSlateBrush* Brush = MarketTheme::Icon(Name);
        return SNew(SBox).WidthOverride(18.f).HeightOverride(18.f).Visibility(Brush ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
        [ SNew(SImage).Image(Brush).ColorAndOpacity(Col(Tone)) ];
    };
    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 10.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([G] { return G() && !G()->bArrange ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
            [
                Pill(SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)[ KeyCap([] { return FString(TEXT("O")); }) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [ Text([G] { return G() && G()->bOpen ? FString(TEXT("G\u00fcn\u00fc kapat")) : FString(TEXT("Marketi a\u00e7")); }, 11, ETone::Text, true) ],
                    52.f)
            ]
        ]
        + SHorizontalBox::Slot().AutoWidth()
        [
            Pill(SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)[ Icon(TEXT("map"), ETone::PrimaryText) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)[ Fixed(TEXT("Y\u00f6netim"), 11, ETone::PrimaryText, true) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [
                    SNew(SBorder).BorderImage(&KeyBrush).BorderBackgroundColor(Col(ETone::PrimaryText)).Padding(FMargin(7.f, 1.f))
                    [ SNew(STextBlock).Font(MarketHudUi::HudFont(true, 9)).ColorAndOpacity(Col(ETone::PrimaryFill)).Text(FText::FromString(TEXT("M"))) ]
                ],
                52.f, [] { return ETone::PrimaryFill; })
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
    static constexpr int32 Window = 9;
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
            + SUniformGridPanel::Slot(0, 0)[ KeyRow(TEXT("WASD"), TEXT("Y\u00fcr\u00fc")) ]
            + SUniformGridPanel::Slot(1, 0)[ KeyRow(TEXT("E"), TEXT("Etkile\u015fim")) ]
            + SUniformGridPanel::Slot(2, 0)[ KeyRow(TEXT("M"), TEXT("Y\u00f6netim men\u00fcs\u00fc")) ]
            + SUniformGridPanel::Slot(0, 1)[ KeyRow(TEXT("O"), TEXT("Marketi a\u00e7 / kapat")) ]
            + SUniformGridPanel::Slot(1, 1)[ KeyRow(TEXT("R"), TEXT("Reyonu diz (kapal\u0131yken)")) ]
            + SUniformGridPanel::Slot(2, 1)[ KeyRow(TEXT("F1"), TEXT("Bu paneller")) ]
            + SUniformGridPanel::Slot(0, 2)[ KeyRow(TEXT("F2"), TEXT("Test modu")) ]
            + SUniformGridPanel::Slot(1, 2)[ KeyRow(TEXT("F3"), TEXT("Raflar\u0131 doldur (test)")) ]
            + SUniformGridPanel::Slot(2, 2)[ KeyRow(TEXT("F4"), TEXT("I\u015f\u0131k")) ]
            + SUniformGridPanel::Slot(0, 3)[ KeyRow(TEXT("F5 / F9"), TEXT("Kaydet / y\u00fckle")) ]
            + SUniformGridPanel::Slot(1, 3)[ KeyRow(TEXT("F6"), TEXT("Yeni kampanya")) ]
            + SUniformGridPanel::Slot(2, 3)[ KeyRow(TEXT("F8"), TEXT("Marka adlar\u0131")) ]
            + SUniformGridPanel::Slot(0, 4)[ KeyRow(TEXT("F11"), TEXT("Tam ekran")) ]
            + SUniformGridPanel::Slot(1, 4)[ KeyRow(TEXT("Esc"), TEXT("Oyundan \u00e7\u0131k")) ]
            + SUniformGridPanel::Slot(2, 4)[ KeyRow(TEXT("Space"), TEXT("Zaman\u0131 durdur")) ]
            + SUniformGridPanel::Slot(0, 5)[ KeyRow(TEXT("1 / 2 / 3"), TEXT("Oyun h\u0131z\u0131")) ]
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
            + SVerticalBox::Slot().AutoHeight()[ Line([](const AMarketGameMode& G) { return FString::Printf(TEXT("G\u00dcN %d RAPORU"), G.State.Day - 1); }, 9, ETone::Muted, true) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
            [ Line([](const AMarketGameMode& G) { return FString::Printf(TEXT("Net sonu\u00e7  %s"), *MarketHudUi::HudLira(G.State.LastProfit)); }, 20, ETone::Text, true) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
            [ Line([](const AMarketGameMode& G) { return FString::Printf(TEXT("Ciro %s  \u00b7  mal maliyeti %s  \u00b7  gider %s"), *MarketHudUi::HudLira(G.State.LastRevenue), *MarketHudUi::HudLira(G.State.LastCostOfGoods), *MarketHudUi::HudLira(G.State.LastOperatingCost)); }, 11, ETone::Text, false) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
            [ Line([](const AMarketGameMode& G) { return FString::Printf(TEXT("%d sat\u0131\u015f  \u00b7  %d kay\u0131p m\u00fc\u015fteri"), G.State.LastServed, G.State.LastLost); }, 11, ETone::Text, false) ]
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
            + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("RAF D\u00dcZEN\u0130"), 9, ETone::Muted, true) ]
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
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[ Fixed(TEXT("TU\u015eLAR"), 9, ETone::Muted, true) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)[ Keys ])
    ];
}

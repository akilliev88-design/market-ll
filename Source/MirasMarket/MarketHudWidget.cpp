#include "MarketHudWidget.h"

#include "MarketGame.h"
#include "MarketVisuals.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
    // Palette: warm charcoal glass cards, cream text, honey accent, soft teal for progress.
    const FLinearColor Cream(1.f, 0.96f, 0.90f);
    const FLinearColor Muted(0.78f, 0.72f, 0.64f, 0.85f);
    const FLinearColor Honey(1.f, 0.74f, 0.38f);
    const FLinearColor Teal(0.45f, 0.82f, 0.78f);
    const FLinearColor Coral(1.f, 0.52f, 0.38f);
    const FLinearColor Leaf(0.55f, 0.85f, 0.50f);

    FSlateFontInfo Font(const char* Style, int32 Size, int32 Spacing = 0)
    {
        FSlateFontInfo Info = FCoreStyle::GetDefaultFontStyle(Style, Size);
        Info.LetterSpacing = Spacing;
        return Info;
    }

    // 1234.5 TL -> "1.234,50 TL"
    FString Lira(int64 Kurus)
    {
        const bool bNegative = Kurus < 0;
        const int64 Abs = FMath::Abs(Kurus);
        FString Whole = FString::Printf(TEXT("%lld"), Abs / 100);
        for (int32 I = Whole.Len() - 3; I > 0; I -= 3) Whole.InsertAt(I, TEXT('.'));
        return FString::Printf(TEXT("%s%s,%02lld TL"), bNegative ? TEXT("-") : TEXT(""), *Whole, Abs % 100);
    }

    FString Clock(const AMarketGameMode* G)
    {
        const int32 Minutes = 8 * 60 + (G && G->bOpen ? static_cast<int32>(G->DayTime * 3) : 0);
        return FString::Printf(TEXT("%02d:%02d"), Minutes / 60, Minutes % 60);
    }
}

void SMarketHud::Construct(const FArguments& InArgs)
{
    Game = InArgs._Game;
    BarStyle = FProgressBarStyle()
        .SetBackgroundImage(TrackBrush)
        .SetFillImage(FillBrush)
        .SetMarqueeImage(FillBrush);

    ChildSlot
    [
        SNew(SOverlay)
        .Visibility(EVisibility::HitTestInvisible)
        + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(24.f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ StatusCard() ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[ StockCard() ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[ OfficeCard() ]
        ]
        + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(24.f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ DayCard() ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[ GoalCard() ]
        ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [
            SNew(SBox).WidthOverride(6.f).HeightOverride(6.f)
            [ SNew(SBorder).BorderImage(&DotBrush) ]
        ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.f, 0.f, 0.f, 120.f)
        [ ReportCard() ]
        + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(24.f)
        [ ControlsCard() ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 28.f)
        [ HintCard() ]
    ];
}

TSharedRef<SWidget> SMarketHud::Card(const TSharedRef<SWidget>& Content, const FMargin& Padding)
{
    return SNew(SBorder).BorderImage(&CardBrush).Padding(Padding)[ Content ];
}

TSharedRef<SWidget> SMarketHud::Bar(TFunction<float()> Value, const FLinearColor& Color, float Width)
{
    TSharedRef<SWidget> Progress = SNew(SProgressBar)
        .Style(&BarStyle)
        .BorderPadding(FVector2D(0.f, 0.f))
        .FillColorAndOpacity(Color)
        .Percent_Lambda([Value]() -> TOptional<float> { return FMath::Clamp(Value(), 0.f, 1.f); });
    if (Width > 0.f) return SNew(SBox).WidthOverride(Width).HeightOverride(6.f)[ Progress ];
    return SNew(SBox).HeightOverride(6.f)[ Progress ];
}

TSharedRef<SWidget> SMarketHud::KeyCap(const FString& Key)
{
    return SNew(SBorder).BorderImage(&KeyBrush).Padding(FMargin(7.f, 2.f)).VAlign(VAlign_Center)
    [
        SNew(STextBlock).Text(FText::FromString(Key)).Font(Font("Bold", 10)).ColorAndOpacity(FLinearColor(0.12f, 0.10f, 0.08f))
    ];
}

TSharedRef<SWidget> SMarketHud::KeyRow(const FString& Key, const FString& Label)
{
    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ KeyCap(Key) ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 3.f, 18.f, 3.f)
        [ SNew(STextBlock).Text(FText::FromString(Label)).Font(Font("Regular", 10)).ColorAndOpacity(Cream) ];
}

TSharedRef<SWidget> SMarketHud::StatusCard()
{
    return Card(
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [ SNew(STextBlock).Text(FText::FromString(TEXT("M\u0130RAS MARKET  \u00b7  L\u00dcLEBURGAZ 2011"))).Font(Font("Bold", 8, 180)).ColorAndOpacity(Muted) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [
                SNew(STextBlock).Font(Font("Bold", 22)).ColorAndOpacity(Cream)
                .Text_Lambda([this] { const AMarketGameMode* G = Game.Get(); return FText::FromString(FString::Printf(TEXT("G\u00fcn %d"), G ? G->State.Day : 1)); })
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12.f, 0.f, 0.f, 0.f)
            [
                SNew(SBorder).BorderImage(&PillBrush).Padding(FMargin(10.f, 3.f))
                .BorderBackgroundColor_Lambda([this] { const AMarketGameMode* G = Game.Get(); return FSlateColor(G && G->bOpen ? Leaf : Honey); })
                [
                    SNew(STextBlock).Font(Font("Bold", 9, 80)).ColorAndOpacity(FLinearColor(0.10f, 0.08f, 0.06f))
                    .Text_Lambda([this] { const AMarketGameMode* G = Game.Get(); return FText::FromString(G && G->bOpen ? TEXT("A\u00c7IK") : TEXT("HAZIRLIK")); })
                ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
        [
            SNew(STextBlock).Font(Font("Bold", 16)).ColorAndOpacity(Honey)
            .Text_Lambda([this] { const AMarketGameMode* G = Game.Get(); return FText::FromString(G ? Lira(G->State.Cash) : FString()); })
        ]);
}

TSharedRef<SWidget> SMarketHud::DayCard()
{
    return SNew(SBox).MinDesiredWidth(250.f)
    [
        Card(
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [ SNew(STextBlock).Text(FText::FromString(TEXT("G\u00dcN\u00dcN SAAT\u0130"))).Font(Font("Bold", 8, 180)).ColorAndOpacity(Muted) ]
                + SHorizontalBox::Slot().AutoWidth()
                [
                    SNew(STextBlock).Font(Font("Bold", 18)).ColorAndOpacity(Cream)
                    .Text_Lambda([this] { return FText::FromString(Clock(Game.Get())); })
                ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
            [ Bar([this] { const AMarketGameMode* G = Game.Get(); return G && G->bOpen ? G->DayTime / 240.f : 0.f; }, Honey) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f)[ SNew(STextBlock).Text(FText::FromString(TEXT("08:00"))).Font(Font("Regular", 8)).ColorAndOpacity(Muted) ]
                + SHorizontalBox::Slot().AutoWidth()[ SNew(STextBlock).Text(FText::FromString(TEXT("20:00"))).Font(Font("Regular", 8)).ColorAndOpacity(Muted) ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth()
                [
                    SNew(SBorder).BorderImage(&PillBrush).BorderBackgroundColor(FLinearColor(1.f, 1.f, 1.f, 0.10f)).Padding(FMargin(10.f, 3.f))
                    [
                        SNew(STextBlock).Font(Font("Regular", 9)).ColorAndOpacity(Cream)
                        .Text_Lambda([this] { const AMarketGameMode* G = Game.Get(); return FText::FromString(FString::Printf(TEXT("I\u015f\u0131k: %s  \u00b7  F4"), MarketVisuals::MoodName(G ? G->Mood : 0))); })
                    ]
                ]
                + SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
                [
                    SNew(SBorder).BorderImage(&PillBrush).BorderBackgroundColor(Coral).Padding(FMargin(10.f, 3.f))
                    .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && G->bTestMode ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
                    [ SNew(STextBlock).Text(FText::FromString(TEXT("TEST  \u00b7  F2"))).Font(Font("Bold", 9, 60)).ColorAndOpacity(FLinearColor(0.12f, 0.06f, 0.04f)) ]
                ]
            ])
    ];
}

bool SMarketHud::ShowStock() const
{
    const AMarketGameMode* G = Game.Get();
    return G && (G->bShowDetails || G->NearOffice());
}

TSharedRef<SWidget> SMarketHud::StockCard()
{
    TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f)[ SNew(STextBlock).Text(FText::FromString(TEXT("STOK"))).Font(Font("Bold", 8, 180)).ColorAndOpacity(Muted) ]
            + SHorizontalBox::Slot().AutoWidth()[ SNew(SBox).WidthOverride(150.f)[ SNew(STextBlock).Text(FText::FromString(TEXT("RAF"))).Font(Font("Bold", 8, 120)).ColorAndOpacity(Muted) ] ]
            + SHorizontalBox::Slot().AutoWidth()[ SNew(SBox).WidthOverride(46.f)[ SNew(STextBlock).Text(FText::FromString(TEXT("DEPO"))).Font(Font("Bold", 8, 120)).ColorAndOpacity(Muted) ] ]
            + SHorizontalBox::Slot().AutoWidth()[ SNew(SBox).WidthOverride(46.f)[ SNew(STextBlock).Text(FText::FromString(TEXT("YOLDA"))).Font(Font("Bold", 8, 120)).ColorAndOpacity(Muted) ] ]
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
        Rows->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SHorizontalBox)
            .Visibility_Lambda([InWindow] { return InWindow() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(STextBlock).Font(Font("Regular", 10))
                    .ColorAndOpacity_Lambda([this, I] { const AMarketGameMode* G = Game.Get(); return FSlateColor(G && G->Selected == I ? Honey : Cream); })
                    .Text_Lambda([this, I] { const AMarketGameMode* G = Game.Get(); return FText::FromString(G && G->Products.IsValidIndex(I) ? G->ProductName(I) : FString()); })
                ]
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(STextBlock).Font(Font("Regular", 8)).ColorAndOpacity(Muted)
                    .Text_Lambda([this, I] { const AMarketGameMode* G = Game.Get(); return FText::FromString(G && G->State.Stock.IsValidIndex(I) ? Lira(G->State.Stock[I].Price) : FString()); })
                ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(14.f, 0.f, 0.f, 0.f)
            [
                SNew(SBox).WidthOverride(136.f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        Bar([this, I] { const AMarketGameMode* G = Game.Get();
                            if (!G || !G->State.Stock.IsValidIndex(I)) return 0.f;
                            return G->State.Stock[I].Shelf / float(FMath::Max(1, G->State.Stock[I].Capacity)); }, Teal)
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
                    [
                        SNew(STextBlock).Font(Font("Regular", 8)).ColorAndOpacity(Muted)
                        .Text_Lambda([this, I] { const AMarketGameMode* G = Game.Get();
                            return FText::FromString(G && G->State.Stock.IsValidIndex(I) ? FString::Printf(TEXT("%d / %d"), G->State.Stock[I].Shelf, G->State.Stock[I].Capacity) : FString()); })
                    ]
                ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(14.f, 0.f, 0.f, 0.f)
            [
                SNew(SBox).WidthOverride(46.f)
                [
                    SNew(STextBlock).Font(Font("Regular", 10)).ColorAndOpacity(Cream)
                    .Text_Lambda([this, I] { const AMarketGameMode* G = Game.Get(); return FText::AsNumber(G && G->State.Stock.IsValidIndex(I) ? G->State.Stock[I].Warehouse : 0); })
                ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [
                SNew(SBox).WidthOverride(46.f)
                [
                    SNew(STextBlock).Font(Font("Regular", 10)).ColorAndOpacity(Cream)
                    .Text_Lambda([this, I] { const AMarketGameMode* G = Game.Get(); return FText::AsNumber(G && G->State.Stock.IsValidIndex(I) ? G->State.Stock[I].Incoming : 0); })
                ]
            ]
        ];
    }
    Rows->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
    [
        SNew(STextBlock).Font(Font("Regular", 8)).ColorAndOpacity(Muted)
        .Text_Lambda([Count] { return FText::FromString(FString::Printf(TEXT("%d \u00fcr\u00fcn  \u00b7  masada TAB ile se\u00e7"), Count)); })
    ];
    return SNew(SBox).MinDesiredWidth(430.f)
        .Visibility_Lambda([this] { return ShowStock() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
        [ Card(Rows) ];
}

TSharedRef<SWidget> SMarketHud::OfficeCard()
{
    return SNew(SBox).MinDesiredWidth(430.f)
        .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && G->NearOffice() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
    [
        Card(
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [ SNew(STextBlock).Text(FText::FromString(TEXT("Y\u00d6NET\u0130M MASASI"))).Font(Font("Bold", 8, 180)).ColorAndOpacity(Muted) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
            [
                SNew(STextBlock).Font(Font("Bold", 14)).ColorAndOpacity(Cream)
                .Text_Lambda([this] { const AMarketGameMode* G = Game.Get(); return FText::FromString(G && G->Products.IsValidIndex(G->Selected) ? G->ProductName(G->Selected) : FString()); })
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
            [
                SNew(STextBlock).Font(Font("Regular", 10)).ColorAndOpacity(Honey)
                .Text_Lambda([this]
                {
                    const AMarketGameMode* G = Game.Get();
                    if (!G || !G->Products.IsValidIndex(G->Selected)) return FText::GetEmpty();
                    const FMarketProduct& P = G->Products[G->Selected];
                    return FText::FromString(G->bTestMode
                        ? FString::Printf(TEXT("Koli %d adet  \u00b7  test modunda bedava, hemen depoda"), P.CaseUnits)
                        : FString::Printf(TEXT("Koli %d \u00d7 %s = %s  \u00b7  ertesi sabah depoda"), P.CaseUnits, *Lira(P.Cost), *Lira(P.Cost * P.CaseUnits)));
                })
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth()[ KeyRow(TEXT("TAB"), TEXT("\u00dcr\u00fcn")) ]
                    + SHorizontalBox::Slot().AutoWidth()[ KeyRow(TEXT("B"), TEXT("Sipari\u015f")) ]
                    + SHorizontalBox::Slot().AutoWidth()[ KeyRow(TEXT("+/-"), TEXT("Fiyat")) ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth()[ KeyRow(TEXT("H"), TEXT("Kasiyer")) ]
                    + SHorizontalBox::Slot().AutoWidth()[ KeyRow(TEXT("G"), TEXT("\u0130kinci \u015fube")) ]
                ]
            ])
    ];
}

TSharedRef<SWidget> SMarketHud::GoalCard()
{
    auto GoalRow = [this](const FString& Label, TFunction<FString()> Value, TFunction<float()> Progress, const FLinearColor& Color) -> TSharedRef<SWidget>
    {
        return SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 3.f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f)[ SNew(STextBlock).Text(FText::FromString(Label)).Font(Font("Regular", 10)).ColorAndOpacity(Cream) ]
                + SHorizontalBox::Slot().AutoWidth()[ SNew(STextBlock).Font(Font("Bold", 10)).ColorAndOpacity(Color).Text_Lambda([Value] { return FText::FromString(Value()); }) ]
            ]
            + SVerticalBox::Slot().AutoHeight()[ Bar(Progress, Color) ];
    };
    return SNew(SBox).MinDesiredWidth(250.f)
        .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && G->bShowDetails ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
    [
        Card(
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [ SNew(STextBlock).Text(FText::FromString(TEXT("HEDEF  \u00b7  \u0130K\u0130NC\u0130 \u015eUBE"))).Font(Font("Bold", 8, 180)).ColorAndOpacity(Muted) ]
            + SVerticalBox::Slot().AutoHeight()
            [
                GoalRow(TEXT("Nakit"),
                    [this] { const AMarketGameMode* G = Game.Get(); return G ? FString::Printf(TEXT("%s / 950"), *Lira(G->State.Cash).LeftChop(6)) : FString(); },
                    [this] { const AMarketGameMode* G = Game.Get(); return G ? G->State.Cash / 95000.f : 0.f; }, Honey)
            ]
            + SVerticalBox::Slot().AutoHeight()
            [
                GoalRow(TEXT("K\u00e2rl\u0131 g\u00fcn"),
                    [this] { const AMarketGameMode* G = Game.Get(); return G ? FString::Printf(TEXT("%d / 3"), G->State.ProfitableDays) : FString(); },
                    [this] { const AMarketGameMode* G = Game.Get(); return G ? G->State.ProfitableDays / 3.f : 0.f; }, Leaf)
            ]
            + SVerticalBox::Slot().AutoHeight()
            [
                GoalRow(TEXT("Yerel pay"),
                    [this] { const AMarketGameMode* G = Game.Get(); return G ? FString::Printf(TEXT("%%%.0f / %%35"), G->State.MarketShare) : FString(); },
                    [this] { const AMarketGameMode* G = Game.Get(); return G ? G->State.MarketShare / 35.f : 0.f; }, Teal)
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
            [
                SNew(STextBlock).Font(Font("Regular", 9)).ColorAndOpacity(Muted).AutoWrapText(true)
                .Text_Lambda([this]
                {
                    const AMarketGameMode* G = Game.Get();
                    if (!G) return FText::GetEmpty();
                    return FText::FromString(FString::Printf(TEXT("Bug\u00fcn %s ciro \u00b7 %d sat\u0131\u015f \u00b7 %d kay\u0131p\nKasiyer %s \u00b7 rakip %s"),
                        *Lira(G->State.Revenue), G->State.Served, G->State.Lost, G->State.bCashier ? TEXT("var") : TEXT("yok"),
                        G->RivalDiscount() < 1.f ? TEXT("%15 indirimde") : TEXT("normal fiyatta")));
                })
            ])
    ];
}

TSharedRef<SWidget> SMarketHud::ControlsCard()
{
    return SNew(SBox)
        .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && G->bShowDetails ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
    [
        Card(
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
            [ SNew(STextBlock).Text(FText::FromString(TEXT("KISAYOLLAR"))).Font(Font("Bold", 8, 180)).ColorAndOpacity(Muted) ]
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SUniformGridPanel).SlotPadding(FMargin(0.f, 2.f))
                + SUniformGridPanel::Slot(0, 0)[ KeyRow(TEXT("WASD"), TEXT("Y\u00fcr\u00fc")) ]
                + SUniformGridPanel::Slot(1, 0)[ KeyRow(TEXT("E"), TEXT("Etkile\u015fim")) ]
                + SUniformGridPanel::Slot(2, 0)[ KeyRow(TEXT("O"), TEXT("Marketi a\u00e7 / kapat")) ]
                + SUniformGridPanel::Slot(0, 1)[ KeyRow(TEXT("F1"), TEXT("Paneller")) ]
                + SUniformGridPanel::Slot(1, 1)[ KeyRow(TEXT("F2"), TEXT("Test modu")) ]
                + SUniformGridPanel::Slot(2, 1)[ KeyRow(TEXT("F3"), TEXT("Raflar\u0131 doldur (test)")) ]
                + SUniformGridPanel::Slot(0, 2)[ KeyRow(TEXT("F4"), TEXT("I\u015f\u0131k")) ]
                + SUniformGridPanel::Slot(1, 2)[ KeyRow(TEXT("F5"), TEXT("Kaydet")) ]
                + SUniformGridPanel::Slot(2, 2)[ KeyRow(TEXT("F9"), TEXT("Y\u00fckle")) ]
                + SUniformGridPanel::Slot(0, 3)[ KeyRow(TEXT("F8"), TEXT("Marka adlar\u0131")) ]
                + SUniformGridPanel::Slot(1, 3)[ KeyRow(TEXT("F6"), TEXT("Yeni kampanya")) ]
                + SUniformGridPanel::Slot(2, 3)[ KeyRow(TEXT("ESC"), TEXT("\u00c7\u0131k\u0131\u015f")) ]
                + SUniformGridPanel::Slot(0, 4)[ KeyRow(TEXT("F11"), TEXT("Tam ekran")) ]
            ])
    ];
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
    return Card(
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
            [
                SNew(SBox).Visibility_Lambda([this] { return HintKey().IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible; })
                [
                    SNew(SBorder).BorderImage(&KeyBrush).Padding(FMargin(8.f, 2.f))
                    [ SNew(STextBlock).Font(Font("Bold", 11)).ColorAndOpacity(FLinearColor(0.12f, 0.10f, 0.08f)).Text_Lambda([this] { return FText::FromString(HintKey()); }) ]
                ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ SNew(STextBlock).Font(Font("Regular", 12)).ColorAndOpacity(Cream).Text_Lambda([this] { return FText::FromString(HintText()); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
        [
            SNew(STextBlock).Font(Font("Regular", 10)).ColorAndOpacity(Honey)
            .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && G->MessageTime > 0.f ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
            .Text_Lambda([this] { const AMarketGameMode* G = Game.Get(); return FText::FromString(G ? G->Message : FString()); })
        ]
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
        [
            SNew(STextBlock).Font(Font("Regular", 8)).ColorAndOpacity(Muted)
            .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && !G->bShowDetails ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
            .Text(FText::FromString(TEXT("F1 paneller  \u00b7  F4 \u0131\u015f\u0131k")))
        ],
        FMargin(20.f, 10.f));
}

TSharedRef<SWidget> SMarketHud::ReportCard()
{
    auto Line = [this](TFunction<FString(const AMarketGameMode&)> Make, const FSlateFontInfo& InFont, const FLinearColor& Color) -> TSharedRef<SWidget>
    {
        return SNew(STextBlock).Font(InFont).ColorAndOpacity(Color)
            .Text_Lambda([this, Make] { const AMarketGameMode* G = Game.Get(); return FText::FromString(G ? Make(*G) : FString()); });
    };
    return SNew(SBox).MinDesiredWidth(520.f)
        .Visibility_Lambda([this]
        {
            const AMarketGameMode* G = Game.Get();
            return G && !G->bOpen && G->State.Day > 1 && (G->ReportTime > 0.f || G->bShowDetails) ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
        })
    [
        Card(
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [ Line([](const AMarketGameMode& G) { return FString::Printf(TEXT("G\u00dcN %d RAPORU"), G.State.Day - 1); }, Font("Bold", 9, 180), Muted) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
            [ Line([](const AMarketGameMode& G) { return FString::Printf(TEXT("Net sonu\u00e7  %s"), *Lira(G.State.LastProfit)); }, Font("Bold", 20), Honey) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
            [ Line([](const AMarketGameMode& G) { return FString::Printf(TEXT("Ciro %s  \u00b7  mal maliyeti %s"), *Lira(G.State.LastRevenue), *Lira(G.State.LastCostOfGoods)); }, Font("Regular", 11), Cream) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
            [ Line([](const AMarketGameMode& G) { return FString::Printf(TEXT("Gider %s  \u00b7  ikinci \u015fube katk\u0131s\u0131 %s"), *Lira(G.State.LastOperatingCost), *Lira(G.State.LastBranchProfit)); }, Font("Regular", 11), Cream) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
            [ Line([](const AMarketGameMode& G) { return FString::Printf(TEXT("%d sat\u0131\u015f  \u00b7  %d kay\u0131p m\u00fc\u015fteri"), G.State.LastServed, G.State.LastLost); }, Font("Regular", 11), Cream) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
            [ Line([](const AMarketGameMode&) { return FString(TEXT("Raflar\u0131 doldur, fiyatlar\u0131 ayarla, O ile a\u00e7.")); }, Font("Regular", 9), Muted) ]
        , FMargin(24.f, 18.f))
    ];
}

#pragma once

#include "CoreMinimal.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/SlateTypes.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class AMarketGameMode;

// In-game HUD (G-074, menu design A1): a plain game screen. Top left: day and clock, cash, today's revenue and
// whether the shop is open. Top right: at most three notices (AMarketGameMode::Todos) that name the menu page
// solving them. Bottom: the queue and today's sales, the context hint with its key, "M Yonetim". Everything else
// lives in the management menu (M); F1 adds the stock list and the key list. The HUD hides while the menu is open
// and follows the menu's light / dark theme. It only reads the game mode.
class SMarketHud : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMarketHud) : _Game(nullptr) {}
        SLATE_ARGUMENT(AMarketGameMode*, Game)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
    enum class ETone : uint8 { Card, Text, Muted, Accent, Good, Bad, Warn, Info, KeyFill, KeyText, PrimaryFill, PrimaryText, Track, OpenFill, WarnSoft, BadSoft, InfoSoft };

    TWeakObjectPtr<AMarketGameMode> Game;

    FSlateRoundedBoxBrush CardBrush = FSlateRoundedBoxBrush(FLinearColor::White, 16.f, FLinearColor(0.f, 0.f, 0.f, 0.08f), 1.f);
    FSlateRoundedBoxBrush PillBrush = FSlateRoundedBoxBrush(FLinearColor::White, 13.f, FLinearColor(0.f, 0.f, 0.f, 0.08f), 1.f);
    FSlateRoundedBoxBrush KeyBrush = FSlateRoundedBoxBrush(FLinearColor::White, 6.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush TrackBrush = FSlateRoundedBoxBrush(FLinearColor::White, 2.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush DotBrush = FSlateRoundedBoxBrush(FLinearColor::White, 4.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush CrossBrush = FSlateRoundedBoxBrush(FLinearColor(1.f, 1.f, 1.f, 0.85f), 3.f, FLinearColor(0.f, 0.f, 0.f, 0.4f), 1.f);
    FProgressBarStyle BarStyle;

    FVector2D LastSize = FVector2D::ZeroVector;  // the viewport: the HUD scales with it (G-075)
    float UiScale() const;
    bool IsLight() const;
    FLinearColor Color(ETone Tone) const;
    TAttribute<FSlateColor> Col(ETone Tone) const;
    TSharedRef<SWidget> Text(TFunction<FString()> Make, int32 Size, ETone Tone, bool bBold = false, bool bWrap = false);
    TSharedRef<SWidget> Fixed(const FString& Value, int32 Size, ETone Tone, bool bBold = false);
    TSharedRef<SWidget> Card(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(16.f, 12.f));
    TSharedRef<SWidget> Bar(TFunction<float()> Value, ETone Tone, float Width = 0.f);
    TSharedRef<SWidget> KeyCap(TFunction<FString()> Key);
    TSharedRef<SWidget> KeyRow(const FString& Key, const FString& Label);
    TSharedRef<SWidget> Divider();
    // G-086d design language (the menu's main screen): a soft shadow in the light theme, pills, numbers in mono.
    TSharedRef<SWidget> Raised(const TSharedRef<SWidget>& Surface);
    TSharedRef<SWidget> Pill(const TSharedRef<SWidget>& Content, float Height, TFunction<ETone()> Fill = nullptr);
    TSharedRef<SWidget> Mono(TFunction<FString()> Make, float Pixels, TFunction<ETone()> Tone);

    TSharedRef<SWidget> StatusBar();
    TSharedRef<SWidget> SpeedPill();     // G-075 pause / speed
    TSharedRef<SWidget> PausedBanner();
    TSharedRef<SWidget> Notices();
    TSharedRef<SWidget> TodayCards();
    TSharedRef<SWidget> HintCard();
    TSharedRef<SWidget> MenuButtons();
    TSharedRef<SWidget> StockCard();
    TSharedRef<SWidget> ControlsCard();
    TSharedRef<SWidget> ReportCard();   // smoke / capture runs only (the menu shows the report otherwise)
    TSharedRef<SWidget> ArrangeCard();

    FString HintKey() const;
    FString HintText() const;
};

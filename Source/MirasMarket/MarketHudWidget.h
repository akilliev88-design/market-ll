#pragma once

#include "CoreMinimal.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/SlateTypes.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class AMarketGameMode;

// In-game HUD: soft rounded cards over the scene (status, time of day, stock, goals, hints).
// Reads the game mode every frame through attribute lambdas; it never changes game state.
class SMarketHud : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMarketHud) : _Game(nullptr) {}
        SLATE_ARGUMENT(AMarketGameMode*, Game)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    TWeakObjectPtr<AMarketGameMode> Game;

    FSlateRoundedBoxBrush CardBrush = FSlateRoundedBoxBrush(FLinearColor(0.045f, 0.038f, 0.034f, 0.80f), 16.f, FLinearColor(1.f, 0.92f, 0.80f, 0.09f), 1.f);
    FSlateRoundedBoxBrush PillBrush = FSlateRoundedBoxBrush(FLinearColor::White, 11.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush KeyBrush = FSlateRoundedBoxBrush(FLinearColor(1.f, 0.97f, 0.90f, 0.95f), 6.f, FLinearColor(0.f, 0.f, 0.f, 0.35f), 1.f);
    FSlateRoundedBoxBrush TrackBrush = FSlateRoundedBoxBrush(FLinearColor(1.f, 1.f, 1.f, 0.12f), 3.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush FillBrush = FSlateRoundedBoxBrush(FLinearColor::White, 3.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush DotBrush = FSlateRoundedBoxBrush(FLinearColor(1.f, 1.f, 1.f, 0.85f), 3.f, FLinearColor(0.f, 0.f, 0.f, 0.4f), 1.f);
    FProgressBarStyle BarStyle;

    TSharedRef<SWidget> Card(const TSharedRef<SWidget>& Content, const FMargin& Padding = FMargin(18.f, 14.f));
    TSharedRef<SWidget> Bar(TFunction<float()> Value, const FLinearColor& Color, float Width = 0.f);
    TSharedRef<SWidget> KeyCap(const FString& Key);
    TSharedRef<SWidget> KeyRow(const FString& Key, const FString& Label);
    TSharedRef<SWidget> StatusCard();
    TSharedRef<SWidget> DayCard();
    TSharedRef<SWidget> StockCard();
    TSharedRef<SWidget> OfficeCard();
    TSharedRef<SWidget> GoalCard();
    TSharedRef<SWidget> ControlsCard();
    TSharedRef<SWidget> HintCard();
    TSharedRef<SWidget> ReportCard();
    TSharedRef<SWidget> ArrangeCard();

    bool ShowStock() const;
    FString HintKey() const;
    FString HintText() const;
};

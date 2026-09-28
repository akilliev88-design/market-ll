#pragma once

#include "CoreMinimal.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateNoResource.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/SlateTypes.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class AMarketGameMode;
class UTexture2D;

// Clickable management menu (G-059). M opens it anywhere (the game pauses, the mouse cursor appears);
// M, TAB or Esc closes it. Pages: Ozet, Siparis, Urunler ve fiyat, Rakipler, Personel, Subeler, Raporlar.
// The day report opens here when the shop closes and stays until "Yeni gune basla".
// Light (Tesla-like) and dark themes; the choice is kept in GameUserSettings.ini.
// The widget only reads the game mode; every decision goes through AMarketGameMode::MenuCommand, so the
// rules stay in one place (the same code as the office-desk keys).
// Logos: Content/Brands/<key>/logo.png when the player puts one there, otherwise a coloured initial badge.
class SMarketMenu : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMarketMenu) : _Game(nullptr) {}
        SLATE_ARGUMENT(AMarketGameMode*, Game)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    virtual bool SupportsKeyboardFocus() const override { return true; }
    virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

    enum EPage : int32 { Summary = 0, Orders, Prices, Rivals, Staff, Branches, Reports, PageCount };
    enum class ERole : uint8 { Page, Panel, Inset, Text, Muted, Accent, Good, Bad, Warn, Line, Button, ButtonText, Primary, PrimaryText, Dim };

private:
    TWeakObjectPtr<AMarketGameMode> Game;
    FString OrderCategory;          // "" = every category
    FString PriceCategory;
    bool bWeekTab = false;          // Raporlar: false = day report, true = week report

    FSlateRoundedBoxBrush PanelBrush = FSlateRoundedBoxBrush(FLinearColor::White, 22.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush CardBrush = FSlateRoundedBoxBrush(FLinearColor::White, 16.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush SmallBrush = FSlateRoundedBoxBrush(FLinearColor::White, 10.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush SmallHover = FSlateRoundedBoxBrush(FLinearColor(0.90f, 0.90f, 0.90f), 10.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush SmallPress = FSlateRoundedBoxBrush(FLinearColor(0.80f, 0.80f, 0.80f), 10.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush PillBrush = FSlateRoundedBoxBrush(FLinearColor::White, 15.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush PillHover = FSlateRoundedBoxBrush(FLinearColor(0.88f, 0.88f, 0.88f), 15.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush PillPress = FSlateRoundedBoxBrush(FLinearColor(0.76f, 0.76f, 0.76f), 15.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush BadgeBrush = FSlateRoundedBoxBrush(FLinearColor::White, 8.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush TrackBrush = FSlateRoundedBoxBrush(FLinearColor::White, 4.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateColorBrush FlatBrush = FSlateColorBrush(FLinearColor::White);
    FSlateNoResource NoBrush;
    FButtonStyle PillStyle;
    FButtonStyle RowStyle;
    FProgressBarStyle BarStyle;

    // Logo brushes by key (loaded once; an empty entry means "no file, draw the badge").
    TMap<FString, TSharedPtr<FSlateBrush>> Logos;
    TArray<TStrongObjectPtr<UTexture2D>> LogoTextures;

    bool IsLight() const;
    FLinearColor Color(ERole Role) const;
    TAttribute<FSlateColor> Col(ERole Role) const;
    TAttribute<FSlateColor> ColBy(TFunction<ERole()> Role) const;

    // Building blocks
    TSharedRef<SWidget> Label(TFunction<FString()> Make, int32 Size, ERole Role, bool bBold = false, bool bWrap = false);
    TSharedRef<SWidget> LabelBy(TFunction<FString()> Make, int32 Size, TFunction<ERole()> Role, bool bBold = false);
    TSharedRef<SWidget> Fixed(const FString& Text, int32 Size, ERole Role, bool bBold = false);
    TSharedRef<SWidget> Card(const TSharedRef<SWidget>& Content, ERole Role = ERole::Panel, const FMargin& Padding = FMargin(18.f, 16.f));
    TSharedRef<SWidget> Button(TFunction<FString()> Text, TFunction<void()> OnClick, bool bPrimary = false, TFunction<bool()> Enabled = nullptr);
    TSharedRef<SWidget> Bar(TFunction<float()> Value, ERole Role);
    TSharedRef<SWidget> Badge(const FString& Key, const FString& Initials, const FLinearColor& Fill, float Size = 30.f);
    TSharedRef<SWidget> Stat(const FString& Heading, TFunction<FString()> Value, TFunction<FString()> Sub, TFunction<ERole()> ValueRole = nullptr);
    TSharedRef<SWidget> Dot(TFunction<bool()> Done);
    TSharedRef<SWidget> CategoryChips(FString* Selected);
    const FSlateBrush* LogoBrush(const FString& Key);

    // Frame
    TSharedRef<SWidget> Sidebar();
    TSharedRef<SWidget> Header();
    TSharedRef<SWidget> NavItem(int32 Page, const FString& Text, const FString& Key);

    // Pages
    TSharedRef<SWidget> SummaryPage();
    TSharedRef<SWidget> OrdersPage();
    TSharedRef<SWidget> PricesPage();
    TSharedRef<SWidget> RivalsPage();
    TSharedRef<SWidget> StaffPage();
    TSharedRef<SWidget> BranchesPage();
    TSharedRef<SWidget> ReportsPage();
    TSharedRef<SWidget> DayReport();
    TSharedRef<SWidget> WeekReport();
    TSharedRef<SWidget> GoalList();
    TSharedRef<SWidget> DecisionCard();   // G-066 waiting choice (story / event)
    TSharedRef<SWidget> StoryCard();      // G-066 chapter goals, identity, last memory

public:
    // Raporlar page: which tab is shown (the game opens the day tab when the shop closes).
    void ShowWeek(bool bWeek);
private:
    void Do(FName Action, int32 Product = INDEX_NONE);
    void Go(int32 Page);
    // Management decisions of the background systems (staff, wholesaler, promotions): AMarketGameMode::StaffCommand.
    void Manage(FName Action, int32 Arg);
};

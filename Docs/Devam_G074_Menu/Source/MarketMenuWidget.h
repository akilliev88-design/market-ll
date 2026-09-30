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

// Clickable management menu (G-059, fully integrated in G-074). M (or E at the office desk) opens it anywhere: the
// game pauses and the mouse cursor appears; M, Esc or "Oyuna don" closes it. Every decision of the game is here, so
// the player never has to walk to the desk or remember letter keys:
//   1 Ozet        what to do now (notices of AMarketGameMode::Todos), the story's goals, the father's debt, time
//   2 Siparis     order list, suggested cases, wholesaler terms and bills
//   3 Urunler     products with their label image, our price against every rival, the buying chance
//   4 Kampanya    discounts, 3-for-2, end caps, flyers, the wholesaler's offer, every running promotion
//   5 Rakipler    local (live shares), national and international retail (MarketRetail)
//   6 Personel    staff, applicants, HR
//   7 Finans      till, bank loans, credit book, taxes and the accountant, perishables, household money
//   8 Satis       phone / web / platform orders, couriers, card and meal-card payments
//   9 Subeler     map of Turkey (MarketMap), Luleburgaz branches, the company in other cities
//   0 Raporlar    day and week reports (the day report opens here when the shop closes)
// The difficulty and the pandemic setting live in the header's settings strip.
// The widget only reads the game mode; every decision goes through AMarketGameMode::MenuCommand / StaffCommand, so
// the rules stay in one place. Risky decisions (firing, closing a branch, a big loan, a new campaign) ask first.
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
    // Clicks on empty parts of the menu keep the keyboard focus here (M / Esc / digits keep working).
    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

    enum EPage : int32 { Summary = 0, Orders, Prices, Promotions, Rivals, Staff, Finance, Channels, Branches, Reports, PageCount };
    enum class ERole : uint8 { Page, Panel, Inset, Text, Muted, Accent, Good, Bad, Warn, Line, Button, ButtonText, Primary, PrimaryText, Dim, Info };
    static const TCHAR* PageName(int32 Page);

    // Raporlar page: which tab is shown (the game opens the day tab when the shop closes).
    void ShowWeek(bool bWeek);

private:
    TWeakObjectPtr<AMarketGameMode> Game;
    FString OrderCategory;          // "" = every category
    FString PriceCategory;
    bool bWeekTab = false;          // Raporlar: false = day report, true = week report
    int32 RivalScope = 0;           // Rakipler: 0 local, 1 national, 2 international
    int32 BranchTab = 0;            // Subeler: 0 map, 1 Luleburgaz, 2 company
    int32 MapLayer = 0;             // 0 ours, 1 BIM, 2 A101, 3 SOK, 4 Migros, 5 Onur
    bool bMapThrace = false;
    int32 PromoteBranch = INDEX_NONE; // Subeler: the branch a manager is being chosen for

    // A risky decision waiting for "Evet".
    FString ConfirmText;
    TFunction<void()> ConfirmAction;

    FSlateRoundedBoxBrush PanelBrush = FSlateRoundedBoxBrush(FLinearColor::White, 22.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush CardBrush = FSlateRoundedBoxBrush(FLinearColor::White, 16.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush SmallBrush = FSlateRoundedBoxBrush(FLinearColor::White, 10.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush SmallHover = FSlateRoundedBoxBrush(FLinearColor(0.90f, 0.90f, 0.90f), 10.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush SmallPress = FSlateRoundedBoxBrush(FLinearColor(0.80f, 0.80f, 0.80f), 10.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush PillBrush = FSlateRoundedBoxBrush(FLinearColor::White, 15.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush PillHover = FSlateRoundedBoxBrush(FLinearColor(0.88f, 0.88f, 0.88f), 15.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush PillPress = FSlateRoundedBoxBrush(FLinearColor(0.76f, 0.76f, 0.76f), 15.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush BadgeBrush = FSlateRoundedBoxBrush(FLinearColor::White, 8.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush CircleBrush = FSlateRoundedBoxBrush(FLinearColor::White, 26.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush TrackBrush = FSlateRoundedBoxBrush(FLinearColor::White, 4.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateColorBrush FlatBrush = FSlateColorBrush(FLinearColor::White);
    FSlateNoResource NoBrush;
    FButtonStyle PillStyle;
    FButtonStyle RowStyle;
    FButtonStyle CircleStyle;
    FProgressBarStyle BarStyle;

    // Logo brushes by key (loaded once; an empty entry means "no file, draw the badge").
    TMap<FString, TSharedPtr<FSlateBrush>> Logos;
    TArray<TStrongObjectPtr<UTexture2D>> LogoTextures;
    // Product pictures cut from the studio's label textures (the box front / the middle of a label band).
    TMap<FString, TSharedPtr<FSlateBrush>> Pictures;
    TArray<TStrongObjectPtr<UTexture2D>> PictureTextures;

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
    // A button that asks "Emin misin?" first.
    TSharedRef<SWidget> RiskyButton(TFunction<FString()> Text, TFunction<FString()> Question, TFunction<void()> OnClick, TFunction<bool()> Enabled = nullptr);
    // A pill that is highlighted while Selected() is true (settings, tabs).
    TSharedRef<SWidget> Choice(const FString& Text, TFunction<bool()> Selected, TFunction<void()> OnClick, TFunction<bool()> Enabled = nullptr);
    TSharedRef<SWidget> Bar(TFunction<float()> Value, ERole Role);
    TSharedRef<SWidget> Badge(const FString& Key, const FString& Initials, const FLinearColor& Fill, float Size = 30.f);
    TSharedRef<SWidget> ProductPicture(int32 Product, float Size);
    TSharedRef<SWidget> Stat(const FString& Heading, TFunction<FString()> Value, TFunction<FString()> Sub, TFunction<ERole()> ValueRole = nullptr);
    TSharedRef<SWidget> Dot(TFunction<bool()> Done);
    TSharedRef<SWidget> Section(const FString& Title);
    // Why a button does nothing right now (empty = hidden).
    TSharedRef<SWidget> Why(TFunction<FString()> Reason);
    TSharedRef<SWidget> CategoryChips(FString* Selected);
    const FSlateBrush* LogoBrush(const FString& Key);
    const FSlateBrush* PictureBrush(int32 Product);

    // Frame
    TSharedRef<SWidget> Sidebar();
    TSharedRef<SWidget> Header();
    TSharedRef<SWidget> NavItem(int32 Page, const FString& Text, const FString& Key);
    TSharedRef<SWidget> ConfirmLayer();

    // Pages (MarketMenuWidget.cpp)
    TSharedRef<SWidget> SummaryPage();
    TSharedRef<SWidget> OrdersPage();
    TSharedRef<SWidget> ReportsPage();
    TSharedRef<SWidget> DayReport();
    TSharedRef<SWidget> WeekReport();
    TSharedRef<SWidget> GoalList();
    TSharedRef<SWidget> DecisionCard();   // G-066 waiting choice (story / event)
    TSharedRef<SWidget> StoryCard();      // G-066 chapter goals, identity, last memory
    TSharedRef<SWidget> TodoList();       // G-074 what to do now
    // Pages (MarketMenuPages.cpp)
    TSharedRef<SWidget> PricesPage();
    TSharedRef<SWidget> PromotionsPage();
    TSharedRef<SWidget> RivalsPage();
    TSharedRef<SWidget> StaffPage();
    TSharedRef<SWidget> FinancePage();
    TSharedRef<SWidget> ChannelsPage();
    TSharedRef<SWidget> BranchesPage();
    TSharedRef<SWidget> MapTab();
    TSharedRef<SWidget> LocalBranchesTab();
    TSharedRef<SWidget> CompanyTab();

    void Do(FName Action, int32 Product = INDEX_NONE);
    void Go(int32 Page);
    void Ask(const FString& Question, TFunction<void()> OnYes);
    // Management decisions of the background systems (staff, wholesaler, promotions): AMarketGameMode::StaffCommand.
    void Manage(FName Action, int32 Arg);
};

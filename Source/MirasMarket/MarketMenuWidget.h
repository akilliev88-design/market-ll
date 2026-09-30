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
class SToolTip;

// Clickable management menu (G-059, fully integrated in G-074, full screen in G-075). M (or E at the office desk)
// opens it anywhere over a blurred view of the running shop (the world keeps its speed; Space pauses, the header has
// the speed buttons); the mouse cursor appears; M, Esc or "Oyuna don" closes it. It fills the screen and scales
// with it (design size 1440 x 820, x the text size setting). Every decision of the game is here, so
// the player never has to walk to the desk or remember letter keys:
//   1 Harita      G-086 main screen: the map of provinces fills the screen; region chips zoom in, a click on a
//                 province opens its panel (numbers, our shops, open a shop); one assistant line.
//   2 Siparis     order list, suggested cases, wholesaler terms and bills
//   3 Urunler     products with their label image, our price against every rival, the buying chance
//   4 Kampanya    discounts, 3-for-2, end caps, flyers, the wholesaler's offer, every running promotion
//   5 Rakipler    local (live shares), national and international retail (MarketRetail)
//   6 Personel    staff, applicants, HR
//   7 Finans      till, bank loans, credit book, taxes and the accountant, perishables, household money
//   8 Satis       phone / web / platform orders, couriers, card and meal-card payments
//   9 Magazalar   every shop by province (grade, manager and the decisions on him, close), management (the
//                 player's span, province / regional / country managers, G-086b), the company (depots, trucks)
//   0 Raporlar    day and week reports (the day report opens here when the shop closes)
// Frame (G-086 sade ana ekran, Docs/Kurgu/03_MAGAZA_AGI.md \u00a712): floating pills on top (date and speed, the till,
// decisions, settings) and a dock of pages at the bottom; the new-game screen covers everything while no province
// is chosen.
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
    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

    enum EPage : int32 { Summary = 0, Orders, Prices, Promotions, Rivals, Staff, Finance, Channels, Branches, Reports, PageCount };
    enum class ERole : uint8 { Page, Panel, Inset, Text, Muted, Accent, Good, Bad, Warn, Line, Button, ButtonText, Primary, PrimaryText, Dim, Info, Solid, Stage, Land,
        // G-086d design tokens (tasarim tuvali, cizim 4 ve 5)
        Ours, Home, Hairline, OnAccent, Sheet, WarnSoft, AccentSoft, MapLabel, DockText };
    static const TCHAR* PageName(int32 Page);

    // Raporlar page: which tab is shown (the game opens the day tab when the shop closes).
    void ShowWeek(bool bWeek);

private:
    TWeakObjectPtr<AMarketGameMode> Game;
    FString OrderCategory;          // "" = every category
    FString PriceCategory;
    // G-078 campaign builder (Kampanyalar page): MarketPromotions::EScope / EMechanic, percent, days.
    int32 PromoScope = 0;
    int32 PromoMechanic = 0;
    int32 PromoPercent = 10;
    int32 PromoDays = 7;
    bool bWeekTab = false;          // Raporlar: false = day report, true = week report
    int32 RivalScope = 0;           // Rakipler: 0 local, 1 national, 2 international
    int32 BranchTab = 0;            // Magazalar: 0 shops, 1 company, 2 management (G-086b)
    int32 MapLayer = 0;             // main map: 0 our shops, 1 rivals, 2 opportunities
    // G-086 main screen: the country shown (empty = the campaign's), the main region the map is zoomed to (empty =
    // the whole country) and the province whose panel is open (empty = no panel).
    FString MapCountry;
    FString MapRegion;
    FString MapProvinceId;
    bool bSettingsOpen = false;
    bool bDecisionsOpen = false;
    bool bMoreOpen = false;         // the dock's "Di\u011fer" card
    // G-086e: nothing jumps. The province panel and the "Di\u011fer" card glide in (0..1, eased in Tick); the panel
    // keeps showing the last province while it slides out.
    float PanelAnim = 0.f;
    float MoreAnim = 0.f;
    FString PanelId;
    // G-086f: the order list window (opens from Siparis and after "Oneriyi yaz"), the line whose product is being
    // swapped (INDEX_NONE = the picker adds), the picker's aisle; the page cross-fade.
    bool bOrderListOpen = false;
    float OrderAnim = 0.f;
    int32 OrderSwapFrom = INDEX_NONE;
    FString OrderPickCategory;
    float PageAnim = 1.f;
    int32 ShownPage = 0;
    FString NewSearch;              // new-game screen: province search
    int32 NewLayer = 0;             // new-game map colour: 0 competition, 1 purchasing power, 2 rent
    int32 PromoteBranch = INDEX_NONE; // Subeler: the branch a manager is being chosen for
    // G-086b Magazalar > Yonetim: the area an appointment is being chosen for (MarketManagers::EncodeArea;
    // INDEX_NONE = the tree of levels is shown).
    int32 AppointArea = INDEX_NONE;
    // G-086b ek (M22): the branch whose store manager is being chosen from the 3 outside candidates ("De\u011fi\u015ftir" /
    // "M\u00fcd\u00fcr al"; INDEX_NONE = no picker).
    int32 PickBranch = INDEX_NONE;
    // G-089 (M23) Sirket > Depolar: the card's layer (0 the depots, 1 choose a province, 2 a depot manager's
    // candidates) and the depot chosen (its range ring on the map; INDEX_NONE = the open province's depot, if any).
    int32 DepotLayer = 0;
    int32 DepotSel = INDEX_NONE;
    // The layers inside a card (appointment, candidates, depot builder) fade in when they change (M15).
    float LayerAnim = 1.f;
    int32 LayerKey = 0;
    // G-084 new-game chooser (Ozet, ZAMAN card). Empty = the running campaign's country / the country's first city.
    FString NewCountry;
    FString NewCity;

    // A risky decision waiting for "Evet".
    FString ConfirmText;
    TFunction<void()> ConfirmAction;

    FSlateRoundedBoxBrush PanelBrush = FSlateRoundedBoxBrush(FLinearColor::White, 22.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush CardBrush = FSlateRoundedBoxBrush(FLinearColor::White, 16.f, FLinearColor(0.f, 0.f, 0.f, 0.08f), 1.f);
    FSlateRoundedBoxBrush SmallBrush = FSlateRoundedBoxBrush(FLinearColor::White, 10.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush SmallHover = FSlateRoundedBoxBrush(FLinearColor(0.90f, 0.90f, 0.90f), 10.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush SmallPress = FSlateRoundedBoxBrush(FLinearColor(0.80f, 0.80f, 0.80f), 10.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush PillBrush = FSlateRoundedBoxBrush(FLinearColor::White, 15.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush PillHover = FSlateRoundedBoxBrush(FLinearColor(0.88f, 0.88f, 0.88f), 15.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush PillPress = FSlateRoundedBoxBrush(FLinearColor(0.76f, 0.76f, 0.76f), 15.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush BadgeBrush = FSlateRoundedBoxBrush(FLinearColor::White, 8.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush CircleBrush = FSlateRoundedBoxBrush(FLinearColor::White, 26.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    // G-086d: true pills (half-height corners) and the design's radii.
    FSlateRoundedBoxBrush RoundBrush = FSlateRoundedBoxBrush(FLinearColor::White, 26.f, FLinearColor(0.f, 0.f, 0.f, 0.08f), 1.f);
    FSlateRoundedBoxBrush DockBrush = FSlateRoundedBoxBrush(FLinearColor::White, 22.f, FLinearColor(0.f, 0.f, 0.f, 0.08f), 1.f);
    FSlateRoundedBoxBrush TileBrush = FSlateRoundedBoxBrush(FLinearColor::White, 12.f, FLinearColor(0.f, 0.f, 0.f, 0.08f), 1.f);
    FSlateRoundedBoxBrush ItemBrush = FSlateRoundedBoxBrush(FLinearColor::White, 14.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush ItemHover = FSlateRoundedBoxBrush(FLinearColor(0.92f, 0.92f, 0.92f), 14.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush ItemPress = FSlateRoundedBoxBrush(FLinearColor(0.84f, 0.84f, 0.84f), 14.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateRoundedBoxBrush TrackBrush = FSlateRoundedBoxBrush(FLinearColor::White, 4.f, FLinearColor(0.f, 0.f, 0.f, 0.f), 0.f);
    FSlateColorBrush FlatBrush = FSlateColorBrush(FLinearColor::White);
    FSlateNoResource NoBrush;
    FButtonStyle PillStyle;
    FButtonStyle RowStyle;
    FButtonStyle CircleStyle;
    FButtonStyle RoundStyle;        // pills and round icon buttons
    FButtonStyle ItemStyle;         // dock items, the shop-type cards
    FProgressBarStyle BarStyle;

    // Logo brushes by key (loaded once; an empty entry means "no file, draw the badge").
    TMap<FString, TSharedPtr<FSlateBrush>> Logos;
    TArray<TStrongObjectPtr<UTexture2D>> LogoTextures;
    // Product pictures cut from the studio's label textures (the box front / the middle of a label band).
    TMap<FString, TSharedPtr<FSlateBrush>> Pictures;
    TArray<TStrongObjectPtr<UTexture2D>> PictureTextures;

    FVector2D LastSize = FVector2D::ZeroVector;  // the viewport (Tick): the menu scales to fill it
    float UiScale() const;

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
    // G-086d design language: a surface with the soft shadow (light theme), an icon, numbers in mono, titles in
    // the display face, and a round icon button.
    TSharedRef<SWidget> Raised(const TSharedRef<SWidget>& Surface);
    TSharedRef<SWidget> IconImage(const FString& Name, float Size, TFunction<ERole()> Role);
    TSharedRef<SWidget> Mono(TFunction<FString()> Make, float Pixels, TFunction<ERole()> Role);
    TSharedRef<SWidget> Display(TFunction<FString()> Make, float Pixels, ERole Role = ERole::Text);
    TSharedRef<SWidget> TextPx(TFunction<FString()> Make, float Pixels, TFunction<ERole()> Role, bool bSemi = false, bool bWrap = false);
    TSharedRef<SWidget> IconButton(const FString& Icon, float Size, TFunction<bool()> Active, TFunction<void()> OnClick, const FString& Hint);
    TSharedRef<SWidget> CategoryChips(FString* Selected);
    // Details on demand: a tooltip (scaled like the menu), a small (i) that shows it, a section title with one.
    TSharedRef<SToolTip> Tip(TFunction<FString()> Text);
    TSharedRef<SWidget> Info(TFunction<FString()> Text);
    TSharedRef<SWidget> Heading(const FString& Title, TFunction<FString()> Hint);
    // "(i) Nasil isler?": the rules of a card, shown when the mouse rests on it.
    TSharedRef<SWidget> More(TFunction<FString()> Text);
    const FSlateBrush* LogoBrush(const FString& Key);
    const FSlateBrush* PictureBrush(int32 Product);

    // Frame (G-086 sade ana ekran): floating pills on top, the dock of pages at the bottom; on the map page both
    // make room for the province panel.
    TSharedRef<SWidget> TopBar();
    TSharedRef<SWidget> BottomNav();
    static constexpr float PanelWidth = 400.f;
    bool PanelOpen() const;         // the province panel of the main screen is showing
    FMargin EdgePadding(float Left, float Top, float Bottom) const;
    TSharedRef<SWidget> PageTitle();
    TSharedRef<SWidget> NavItem(int32 Page, const FString& Text, const FString& Key);
    TSharedRef<SWidget> TimeControls();   // G-075 date, clock, pause / 1x / 2x / 3x (the top-left pill)
    TSharedRef<SWidget> ConfirmLayer();
    TSharedRef<SWidget> SettingsLayer();  // text size, difficulty, theme, menu time, save slots, new game
    TSharedRef<SWidget> DecisionsLayer(); // waiting decisions, what to do now, the story, the business debt
    TSharedRef<SWidget> NewGameLayer();   // G-086 country and province (MarketMenuPages.cpp)
    TSharedRef<SWidget> OrderListLayer(); // G-086f the order list window

    // Pages (MarketMenuWidget.cpp)
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
    TSharedRef<SWidget> HomePage();       // G-086 main screen: the map is the stage
    TSharedRef<SWidget> HomeMap();
    TSharedRef<SWidget> RegionChips();    // the country and its main regions (a click zooms the map)
    TSharedRef<SWidget> ProvinceCard();   // the province panel (slides in on the right)
    TSharedRef<SWidget> Assistant();      // the one most important thing today
    TSharedRef<SWidget> BranchesPage();   // Magazalar
    TSharedRef<SWidget> ShopsTab();
    TSharedRef<SWidget> CompanyTab();
    TSharedRef<SWidget> ManagementTab();  // G-086b: the player's span, the tree of levels, appointments
    // "Sana dogrudan bagli: 4 / 5" (warn colour at the limit, bad beyond it; the rules in "Nasil isler?").
    TSharedRef<SWidget> SpanCounter(bool bOpensManagement);
    // Opens Magazalar > Yonetim with the appointment of an area chosen (MarketManagers::EncodeArea).
    void OpenAppointment(int32 Area);
    // G-086b ek (M22): the 3 outside candidates of an appointment as cards of fixed height, side by side. Target:
    // an area (MarketManagers::EncodeArea, >= 0) or a branch's store manager (-2 - branch index); INDEX_NONE shows
    // nothing. A choice asks first and then sends AppointCandidate / ManagerReplaceWith / ManagerHireFor; OnDone
    // closes the layer.
    TSharedRef<SWidget> CandidateCards(TFunction<int32()> Target, TFunction<void()> OnDone);
    // G-089 (M23): the depots, a province to build one in, a depot manager (Sirket tab).
    TSharedRef<SWidget> DepotCard();
    // A layer inside a card fades and rises in when LayerKey changes.
    TSharedRef<SWidget> LayerFade(const TSharedRef<SWidget>& Inner);
    // Helpers of the map pages: the country / province shown on the main map.
    FString ShownCountry() const;
    FString ShownProvince() const;

    void Do(FName Action, int32 Product = INDEX_NONE);
    void Go(int32 Page);
    void Ask(const FString& Question, TFunction<void()> OnYes);
    // Management decisions of the background systems (staff, wholesaler, promotions): AMarketGameMode::StaffCommand.
    void Manage(FName Action, int32 Arg);
};

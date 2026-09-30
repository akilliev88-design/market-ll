// Management menu glue (G-059): opening/closing the clickable menu (MarketMenuWidget), pausing the world and
// routing the menu's buttons through the same Command() rules as the office-desk keys.
#include "MarketGame.h"
#include "MarketMenuWidget.h"
#include "ProductCatalog.h"
#include "MarketSimulation.h"
#include "MarketEvents.h"
#include "MarketRivals.h"
#include "MarketCampaign.h"
#include "MarketDemand.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Parse.h"

namespace MarketMenuGlue
{
    const TCHAR* Section = TEXT("MirasMarket.Menu");

    // Smoke test and screenshot runs never stop for a menu.
    bool AutomationRun()
    {
        return FParse::Param(FCommandLine::Get(), TEXT("MirasSmoke")) || FParse::Param(FCommandLine::Get(), TEXT("MirasCapture"));
    }
}

void AMarketGameMode::LoadMenuSettings()
{
    if (bMenuSettingsLoaded) return;
    bMenuSettingsLoaded = true;
    bool bLight = true;
    if (GConfig && GConfig->GetBool(MarketMenuGlue::Section, TEXT("LightTheme"), bLight, GGameUserSettingsIni)) bLightTheme = bLight;
}

void AMarketGameMode::ToggleMenuTheme()
{
    LoadMenuSettings();
    bLightTheme = !bLightTheme;
    if (GConfig)
    {
        GConfig->SetBool(MarketMenuGlue::Section, TEXT("LightTheme"), bLightTheme, GGameUserSettingsIni);
        GConfig->Flush(false, GGameUserSettingsIni);
    }
}

void AMarketGameMode::OpenMenu(int32 Page, bool bDayReport)
{
    if (bArrange) { Notify(TEXT("Men\u00fc i\u00e7in \u00f6nce R ile raf d\u00fczeninden \u00e7\u0131k.")); return; }
    APlayerController* Player = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    AMarketHUD* Hud = Player ? Cast<AMarketHUD>(Player->GetHUD()) : nullptr;
    TSharedPtr<SMarketMenu> Menu = Hud ? Hud->MenuWidget() : nullptr;
    if (!Player || !Menu.IsValid()) return;
    LoadMenuSettings();
    MenuPage = FMath::Clamp(Page, 0, SMarketMenu::PageCount - 1);
    bMenuDayReport = bDayReport;
    if (bDayReport) Menu->ShowWeek(bWeekJustEnded);
    if (!bDayReport) MessageTime = 0; // old notices belong to the 3D view; the report keeps the day-close notice
    if (!Products.IsValidIndex(MenuProduct)) MenuProduct = Products.IsValidIndex(Selected) ? Selected : 0;
    bMenuOpen = true;
    Player->bShowMouseCursor = true;
    FInputModeUIOnly Mode;
    Mode.SetWidgetToFocus(Menu);
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    Player->SetInputMode(Mode);
    Player->SetPause(true);
}

void AMarketGameMode::CloseMenu()
{
    if (!bMenuOpen) return;
    bMenuOpen = false;
    bMenuDayReport = false;
    ReportTime = 0;
    MessageTime = FMath::Min(MessageTime, 4.f);
    if (APlayerController* Player = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
    {
        Player->SetPause(false);
        Player->bShowMouseCursor = false;
        Player->SetInputMode(FInputModeGameOnly());
    }
}

void AMarketGameMode::OpenDayReport()
{
    if (MarketMenuGlue::AutomationRun()) return; // smoke/capture keep the old 30-second report card
    ReportTime = 0;
    OpenMenu(SMarketMenu::Reports, true);
}

void AMarketGameMode::MenuCommand(FName Action, int32 Product)
{
    if (Products.IsValidIndex(Product))
    {
        Selected = Product;
        MenuProduct = Product;
    }
    const bool bWasOpen = bOpen;
    {
        TGuardValue<bool> FromMenu(bMenuAction, true);
        Command(Action);
    }
    // Opening the shop from the menu means "let's play": back to the store. Closing it shows the day report.
    if (Action == TEXT("ToggleShop") && !bWasOpen && bOpen) CloseMenu();
}

const TArray<FMarketTodo>& AMarketGameMode::Todos() const
{
    if (TodoFrame == GFrameCounter) return TodoCache;
    TodoFrame = GFrameCounter;
    TodoCache.Reset();
    auto Add = [this](int32 Severity, const FString& Title, const FString& Text, int32 Page, int32 Product = INDEX_NONE)
    {
        FMarketTodo Todo;
        Todo.Severity = Severity; Todo.Title = Title; Todo.Text = Text; Todo.Page = Page; Todo.Product = Product;
        TodoCache.Add(Todo);
    };
    if (const FMarketDecision* Decision = MarketEvents::Pending(State))
        Add(2, TEXT("Karar bekliyor: ") + Decision->Title, FString::Printf(TEXT("Se\u00e7mezsen %d. g\u00fcn \"%s\" ge\u00e7erli olur."), Decision->Deadline,
            Decision->Options.IsValidIndex(Decision->DefaultOption) ? *Decision->Options[Decision->DefaultOption] : TEXT("")), SMarketMenu::Summary);
    // Shelves running out with nothing behind them (worst first), then shelves the depot can refill.
    TArray<int32> Out, Refill;
    for (int32 I = 0; I < Products.Num() && I < State.Stock.Num(); ++I)
    {
        const FMarketStock& S = State.Stock[I];
        if (S.Capacity <= 0 || S.Shelf * 4 > S.Capacity) continue;
        if (S.Warehouse + S.Dock + S.Incoming == 0) Out.Add(I); else if (S.Warehouse > 0) Refill.Add(I);
    }
    Out.Sort([this](int32 A, int32 B) { return State.Stock[A].Shelf < State.Stock[B].Shelf; });
    for (int32 K = 0; K < Out.Num() && K < 2; ++K)
        Add(2, ProductName(Out[K]) + TEXT(" bitiyor"), FString::Printf(TEXT("Rafta %d, depoda ve yolda yok. Sipari\u015f ver."), State.Stock[Out[K]].Shelf), SMarketMenu::Orders, Out[K]);
    if (Out.Num() > 2) Add(2, FString::Printf(TEXT("%d \u00fcr\u00fcn daha bitiyor"), Out.Num() - 2), TEXT("Sipari\u015f sayfas\u0131nda bo\u015f raf s\u00fctununa bak."), SMarketMenu::Orders);
    if (State.DeliveryUnits() > 0)
        Add(1, FString::Printf(TEXT("Arka kap\u0131da %d \u00fcr\u00fcn bekliyor"), State.DeliveryUnits()), TEXT("Depoya ta\u015f\u0131nmadan rafa konamaz: E ile ta\u015f\u0131 ya da reyon g\u00f6revlisi ta\u015f\u0131s\u0131n."), SMarketMenu::Orders);
    if (Refill.Num() > 0)
        Add(1, FString::Printf(TEXT("%s raf\u0131 bo\u015fal\u0131yor"), *ProductName(Refill[0])), FString::Printf(TEXT("Depoda %d var. Raf\u0131n \u00f6n\u00fcnde E ile doldur%s."), State.Stock[Refill[0]].Warehouse,
            Refill.Num() > 1 ? *FString::Printf(TEXT(" (%d raf daha)"), Refill.Num() - 1) : TEXT("")), SMarketMenu::Orders, Refill[0]);
    for (const FMarketPayable& Bill : State.Payables)
        if (Bill.LateSince > 0) { Add(2, TEXT("Toptanc\u0131 faturas\u0131 gecikti"), TEXT("Her g\u00fcn gecikme fark\u0131 i\u015fliyor ve vade kapand\u0131. Sipari\u015f sayfas\u0131ndan \u00f6de."), SMarketMenu::Orders); break; }
    if (State.Books.TaxDue > 0 && !MarketStaff::HasAccountant(State) && State.Books.TaxDueDay - State.Day <= 1)
        Add(2, TEXT("Vergi \u00f6deme g\u00fcn\u00fc"), FString::Printf(TEXT("%s TL, son g\u00fcn %d. Gecikirse ceza i\u015fler."), *MarketCatalog::Money(State.Books.TaxDue), State.Books.TaxDueDay), SMarketMenu::Finance);
    if (State.TroubleStage > 0)
        Add(2, TEXT("Nakit s\u0131k\u0131nt\u0131s\u0131"), TEXT("Kasa eksiye d\u00fc\u015ft\u00fc. Finans sayfas\u0131nda kredi, veresiye tahsilat\u0131 ve giderlere bak."), SMarketMenu::Finance);
    // The product whose price scares most shoppers away (only on shelves).
    int32 Dearest = INDEX_NONE;
    double DearestRatio = 1.10;
    for (int32 I = 0; I < Products.Num() && I < State.Stock.Num(); ++I)
    {
        if (State.Stock[I].Capacity <= 0) continue;
        const int64 Theirs = MarketDemand::RivalPrice(Products[I], RivalPriceFactor(I));
        const double Ratio = Theirs > 0 ? static_cast<double>(State.Stock[I].Price) / Theirs : 1.0;
        if (Ratio > DearestRatio) { DearestRatio = Ratio; Dearest = I; }
    }
    if (Dearest != INDEX_NONE)
        Add(1, ProductName(Dearest) + TEXT(" rakiplerden pahal\u0131"), FString::Printf(TEXT("Sende %s TL, m\u00fc\u015fterinin akl\u0131ndaki rakip fiyat\u0131 %s TL."),
            *MarketCatalog::Money(State.Stock[Dearest].Price), *MarketCatalog::Money(MarketDemand::RivalPrice(Products[Dearest], RivalPriceFactor(Dearest)))), SMarketMenu::Prices, Dearest);
    const TArray<MarketRivals::FEvent> News = MarketRivals::ActiveOn(State.Day, State.RivalSeed, RivalAisles);
    if (News.Num() > 0) Add(0, TEXT("Rakiplerde bug\u00fcn"), MarketRivals::Describe(News[0]), SMarketMenu::Rivals);
    if (MarketCampaign::DebtOpen(State) && State.Cash >= MarketCampaign::Installment * 3)
        Add(0, TEXT("Baban\u0131n borcu"), FString::Printf(TEXT("%s TL kald\u0131. Kasa yetiyor: bir taksit \u00f6deyebilirsin."), *MarketCatalog::Money(State.InheritedDebt)), SMarketMenu::Summary);
    if (!bOpen && OrderDraftCaseCount() > 0)
        Add(0, TEXT("Sipari\u015f listesi onay bekliyor"), FString::Printf(TEXT("%d koli, %s TL. Onaylanmazsa gelmez."), OrderDraftCaseCount(), *MarketCatalog::Money(OrderDraftBill())), SMarketMenu::Orders);
    TodoCache.StableSort([](const FMarketTodo& A, const FMarketTodo& B) { return A.Severity > B.Severity; });
    return TodoCache;
}

void AMarketGameMode::ClearOrderDraft()
{
    OrderDraftCases.Init(0, Products.Num());
    Notify(TEXT("Sipari\u015f listesi temizlendi."));
}

void AMarketGameMode::StaffCommand(FName Action, int32 Id)
{
    // Personnel and tax decisions (G-060). The rules live in MarketStaff; this only routes and tells the player.
    MarketStaff::Migrate(State);
    FString Text;
    bool bChanged = false;
    if (Action == TEXT("HireCandidate"))
        bChanged = MarketStaff::Hire(State, State.Candidates.IndexOfByPredicate([Id](const FMarketEmployee& C) { return C.Id == Id; }), Text);
    else if (Action == TEXT("Fire")) bChanged = MarketStaff::Fire(State, Id, Text);
    else if (Action == TEXT("Raise")) bChanged = MarketStaff::Raise(State, Id, Text);
    else if (Action == TEXT("DayOff")) bChanged = MarketStaff::GiveDayOff(State, Id, bOpen, Text);
    else if (Action == TEXT("Warn")) bChanged = MarketStaff::Warn(State, Id, Text);
    else if (Action == TEXT("HireAccountant")) bChanged = MarketStaff::HireAccountant(State, Text);
    else if (Action == TEXT("PayTax"))
    {
        const int64 Paid = MarketStaff::PayTax(State);
        bChanged = Paid > 0;
        Text = Paid > 0 ? FString::Printf(TEXT("Vergi \u00f6dendi: %s TL. Kalan %s TL."), *MarketCatalog::Money(Paid), *MarketCatalog::Money(State.Books.TaxDue))
            : State.Books.TaxDue > 0 ? FString(TEXT("Vergi i\u00e7in kasada para yok.")) : FString(TEXT("\u00d6denecek vergi yok."));
    }
    else if (Action == TEXT("HrAutoReplace"))
    {
        State.bHrAutoReplace = !State.bHrAutoReplace;
        bChanged = true;
        Text = State.bHrAutoReplace ? TEXT("\u0130K ayr\u0131lan\u0131n yerine uygun aday\u0131 kendisi alacak.") : TEXT("\u0130K ayr\u0131lan\u0131n yerine kimseyi almayacak; adaylar\u0131 sen se\u00e7ersin.");
    }
    else if (Action == TEXT("Advance"))
    {
        // G-071: play days without walking people (the same rules), stop when the player is needed.
        if (bOpen) Text = TEXT("D\u00fckk\u00e2n a\u00e7\u0131kken g\u00fcn ilerletilmez; \u00f6nce kapat.");
        else
        {
            const int32 WeekBefore = State.LastWeekNumber;
            MarketSimulation::Advance(State, CatalogBase, Products, Id, Text);
            bWeekJustEnded = State.LastWeekNumber != WeekBefore;
            ResetWorkerJobs();
            RefreshDeliveryCrates();
            SaveCampaign();
            bChanged = true;
            OpenDayReport();
        }
    }
    else bChanged = MarketDirector::Command(State, Products, Action, Id, Text); // wholesaler, prices, ...
    if (bChanged) { SyncWorkers(); RefreshPrices(); RefreshLabels(); } // people, supplier discount, shelf prices
    if (!Text.IsEmpty()) Notify(Text);
}

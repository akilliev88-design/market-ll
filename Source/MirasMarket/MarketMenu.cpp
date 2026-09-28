// Management menu glue (G-059): opening/closing the clickable menu (MarketMenuWidget), pausing the world and
// routing the menu's buttons through the same Command() rules as the office-desk keys.
#include "MarketGame.h"
#include "MarketMenuWidget.h"
#include "ProductCatalog.h"
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
    else bChanged = MarketDirector::Command(State, Products, Action, Id, Text); // wholesaler, prices, ...
    if (bChanged) { SyncWorkers(); RefreshPrices(); RefreshLabels(); } // people, supplier discount, shelf prices
    if (!Text.IsEmpty()) Notify(Text);
}

// Management menu glue (G-059, G-075): opening/closing the clickable menu (MarketMenuWidget), the game speed (the
// world keeps running behind the menu) and routing the menu's buttons through the same Command() rules as the
// office-desk keys.
#include "MarketGame.h"
#include "MarketCountry.h"
#include "MarketStart.h"
#include "MarketMenuWidget.h"
#include "ProductCatalog.h"
#include "MarketSimulation.h"
#include "MarketEvents.h"
#include "MarketRivals.h"
#include "MarketCampaign.h"
#include "MarketDemand.h"
#include "MarketCalendar.h"
#include "MarketManagers.h"
#include "MarketBranches.h"
#include "MarketDepots.h"
#include "MarketChains.h"
#include "MarketBrands.h"
#include "MarketSourcing.h"
#include "MarketDepartments.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
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
    int32 Size = 1;
    if (GConfig && GConfig->GetInt(MarketMenuGlue::Section, TEXT("TextSize"), Size, GGameUserSettingsIni)) MenuTextSize = FMath::Clamp(Size, 0, 2);
    bool bPause = false;
    if (GConfig && GConfig->GetBool(MarketMenuGlue::Section, TEXT("PauseInMenu"), bPause, GGameUserSettingsIni)) bPauseInMenu = bPause;
}

void AMarketGameMode::SetPauseInMenu(bool bPause)
{
    LoadMenuSettings();
    bPauseInMenu = bPause;
    if (GConfig)
    {
        GConfig->SetBool(MarketMenuGlue::Section, TEXT("PauseInMenu"), bPauseInMenu, GGameUserSettingsIni);
        GConfig->Flush(false, GGameUserSettingsIni);
    }
    ApplyGameSpeed();
}

// ---- Save slots (G-076) -------------------------------------------------------------------------------------------

FString AMarketGameMode::SlotName(int32 Slot)
{
    if (FParse::Param(FCommandLine::Get(), TEXT("MirasSmoke"))) return TEXT("MirasMarket_TestOnly");
    return Slot <= 1 ? FString(TEXT("MirasMarket_Campaign_v1")) : FString::Printf(TEXT("MirasMarket_Campaign_s%d"), Slot);
}

bool AMarketGameMode::SlotExists(int32 Slot) const
{
    return UGameplayStatics::DoesSaveGameExist(SlotName(Slot), 0);
}

FString AMarketGameMode::SlotSummary(int32 Slot) const
{
    return SlotSummaries.IsValidIndex(Slot - 1) ? SlotSummaries[Slot - 1] : FString(TEXT("bo\u015f"));
}

void AMarketGameMode::RefreshSlotSummaries()
{
    SlotSummaries.Reset();
    for (int32 Slot = 1; Slot <= SlotCount; ++Slot)
    {
        const UMarketSave* Save = SlotExists(Slot) ? Cast<UMarketSave>(UGameplayStatics::LoadGameFromSlot(SlotName(Slot), 0)) : nullptr;
        if (!Save) { SlotSummaries.Add(TEXT("bo\u015f")); continue; }
        FString Text = FString::Printf(TEXT("G\u00fcn %d \u00b7 %s"), Save->State.Day, *MarketCalendar::DateText(Save->State.Day));
        if (Save->State.Story.bCampaignOver) Text += TEXT(" \u00b7 bitti");
        else if (Save->State.Story.bEnded) Text += TEXT(" \u00b7 son sonras\u0131");
        if (Save->State.bUsedTestMode) Text += TEXT(" \u00b7 test");
        SlotSummaries.Add(Text);
    }
}

void AMarketGameMode::ResetCampaign()
{
    if (bArrange) ExitArrange(FString());
    DropCarriedDelivery();
    LoadPlanogram(); ++ArrangeVersion; // G-078 (#5): a new campaign starts from the shop's plan file
    const FString Country = State.CountryId, City = State.CityId; // a new campaign stays in the chosen country
    State.Initialize(CatalogBase); State.RivalSeed = FMath::Rand(); State.CountryId = Country; State.CityId = City;
    MarketCountry::SetActive(State.CountryId, State.RivalSeed); RefreshPrices(); bWeekJustEnded = false; RebuildShelfContents(); ApplyCapacities();
    StartShop();
    SyncWorkers(); ResetWorkerJobs(); OrderDraftCases.Init(0, Products.Num()); RefreshDeliveryCrates();
    State.bUsedTestMode = bTestMode;
    RefreshLabels();
}

void AMarketGameMode::StartShop()
{
    if (MarketMenuGlue::AutomationRun()) return;
    MarketStart::Setup(State, State.CountryId, State.CityId, State.RivalSeed);
    MarketCountry::SetActive(State.CountryId, State.RivalSeed); // Setup may have fixed an unknown country
    RefreshPrices();
    if (!bTestMode) MarketStart::StockShelvesPartly(State, State.RivalSeed); // test mode starts empty on purpose
    RebuildShelfContents();
}

void AMarketGameMode::StartNewCampaign(const FString& Country, const FString& City)
{
    if (bOpen) { Notify(TEXT("Yeni oyun i\u00e7in \u00f6nce g\u00fcn\u00fc kapat.")); return; }
    if (!MarketCountry::FindCity(Country, City)) { Notify(TEXT("\u00d6nce bir il se\u00e7.")); return; }
    State.CountryId = Country;
    State.CityId = City;
    ResetCampaign();
    SaveCampaign();
    RefreshSlotSummaries();
    bNeedStart = false;
    bNewGameAsk = false;
    MapProvince = INDEX_NONE;
    CloseMenu(); // karar L07: the game starts in the shop, in first person
    Notify(MarketStart::IntroText(State));
}

void AMarketGameMode::AskNewGame()
{
    if (bOpen) { Notify(TEXT("Yeni oyun i\u00e7in \u00f6nce g\u00fcn\u00fc kapat.")); return; }
    bNewGameAsk = true;
    OpenMenu(SMarketMenu::Summary);
}

void AMarketGameMode::SelectSlot(int32 Slot)
{
    Slot = FMath::Clamp(Slot, 1, SlotCount);
    if (bOpen) { Notify(TEXT("Kay\u0131t yuvas\u0131n\u0131 de\u011fi\u015ftirmek i\u00e7in \u00f6nce g\u00fcn\u00fc kapat.")); return; }
    if (Slot == ActiveSlot) { Notify(FString::Printf(TEXT("Zaten %d. yuvadas\u0131n."), Slot)); return; }
    SaveCampaign();   // the campaign being left keeps its place
    ActiveSlot = Slot;
    if (GConfig)
    {
        GConfig->SetInt(MarketMenuGlue::Section, TEXT("LastSlot"), ActiveSlot, GGameUserSettingsIni);
        GConfig->Flush(false, GGameUserSettingsIni);
    }
    if (SlotExists(Slot)) LoadCampaign();
    else bNeedStart = true; // G-086: an empty slot starts on the new-game screen (country and province)
    RefreshSlotSummaries();
}

void AMarketGameMode::SetMenuTextSize(int32 Size)
{
    LoadMenuSettings();
    MenuTextSize = FMath::Clamp(Size, 0, 2);
    if (GConfig)
    {
        GConfig->SetInt(MarketMenuGlue::Section, TEXT("TextSize"), MenuTextSize, GGameUserSettingsIni);
        GConfig->Flush(false, GGameUserSettingsIni);
    }
}

float AMarketGameMode::UiTextFactor() const
{
    static const float Factors[3] = { 0.88f, 1.f, 1.14f };
    return Factors[FMath::Clamp(MenuTextSize, 0, 2)];
}

void AMarketGameMode::ApplyGameSpeed()
{
    if (MarketMenuGlue::AutomationRun()) return; // smoke / capture keep their own pace
    UGameplayStatics::SetGlobalTimeDilation(this, static_cast<float>(FMath::Clamp(GameSpeed, 1, 3)));
    // Karar A02: behind the menu the world keeps its speed, unless the player chose "time stops in the menu".
    if (APlayerController* Player = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr) Player->SetPause(bTimePaused || (bMenuOpen && bPauseInMenu));
}

void AMarketGameMode::SetGameSpeed(int32 Speed)
{
    GameSpeed = FMath::Clamp(Speed, 1, 3);
    bTimePaused = false;
    ApplyGameSpeed();
}

void AMarketGameMode::SetTimePaused(bool bPaused)
{
    bTimePaused = bPaused;
    ApplyGameSpeed();
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
    ApplyGameSpeed(); // G-075: the world keeps its speed (or its pause) behind the menu
}

void AMarketGameMode::CloseMenu()
{
    if (!bMenuOpen) return;
    if (bNeedStart) return; // G-086: a province must be chosen first
    bNewGameAsk = false;
    bMenuOpen = false;
    bMenuDayReport = false;
    ReportTime = 0;
    MessageTime = FMath::Min(MessageTime, 4.f);
    if (APlayerController* Player = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
    {
        Player->bShowMouseCursor = false;
        Player->SetInputMode(FInputModeGameOnly());
    }
    ApplyGameSpeed();
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
        Add(2, TEXT("Vergi \u00f6deme g\u00fcn\u00fc"), FString::Printf(TEXT("%s, son g\u00fcn %d. Gecikirse ceza i\u015fler."), *MarketCountry::Money(State.Books.TaxDue), State.Books.TaxDueDay), SMarketMenu::Finance);
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
        Add(1, ProductName(Dearest) + TEXT(" rakiplerden pahal\u0131"), FString::Printf(TEXT("Sende %s, m\u00fc\u015fterinin akl\u0131ndaki rakip fiyat\u0131 %s."),
            *MarketCountry::Money(State.Stock[Dearest].Price), *MarketCountry::Money(MarketDemand::RivalPrice(Products[Dearest], RivalPriceFactor(Dearest)))), SMarketMenu::Prices, Dearest);
    const TArray<MarketRivals::FEvent> News = MarketRivals::ActiveOn(State.Day, State.RivalSeed, RivalAisles);
    if (News.Num() > 0) Add(0, TEXT("Rakiplerde bug\u00fcn"), MarketRivals::Describe(News[0]), SMarketMenu::Rivals);
    if (MarketCampaign::DebtOpen(State) && State.Cash >= MarketCampaign::Installment * 3)
        Add(0, TEXT("\u0130\u015fletmenin borcu"), FString::Printf(TEXT("%s kald\u0131. Kasa yetiyor: bir taksit \u00f6deyebilirsin."), *MarketCountry::Money(State.InheritedDebt)), SMarketMenu::Summary);
    if (!bOpen && OrderDraftCaseCount() > 0)
        Add(0, TEXT("Sipari\u015f listesi onay bekliyor"), FString::Printf(TEXT("%d koli, %s. Onaylanmazsa gelmez."), OrderDraftCaseCount(), *MarketCountry::Money(OrderDraftBill())), SMarketMenu::Orders);
    // G-086b management: a required country manager missing, the player's span beyond 5, a skimming store manager,
    // a level that can be filled now (a suggestion; a warning once the player is at the limit).
    for (const FString& Missing : MarketManagers::CountriesMissingManager(State))
        Add(2, TEXT("\u00dclke m\u00fcd\u00fcr\u00fc yok: ") + MarketManagers::AreaName(MarketManagers::ELevel::Country, Missing, Missing),
            TEXT("\u015eirket birden \u00e7ok \u00fclkede: her \u00fclkeye bir \u00fclke m\u00fcd\u00fcr\u00fc gerekir. Yokken oradaki m\u00fcd\u00fcrlerin becerisi 10 puan d\u00fc\u015fer. Ma\u011fazalar \u203a Y\u00f6netim."), SMarketMenu::Branches);
    const int32 Direct = MarketManagers::DirectCount(State);
    if (Direct > MarketManagers::SpanLimit)
        Add(2, FString::Printf(TEXT("Sana do\u011frudan %d ki\u015fi ba\u011fl\u0131 (s\u0131n\u0131r %d)"), Direct, MarketManagers::SpanLimit),
            FString::Printf(TEXT("Sana ba\u011fl\u0131 herkesin becerisi -%d, kasadan \u00e7alan g\u00f6r\u00fcnm\u00fcyor. Bir il m\u00fcd\u00fcr\u00fc ya da \u00fclke m\u00fcd\u00fcr\u00fc ata."), MarketManagers::SpanPenalty(State)), SMarketMenu::Branches);
    for (const FMarketBranch& Branch : State.Branches)
        if (Branch.Stage == static_cast<uint8>(MarketBranches::EStage::Open) && !Branch.ManagerName.IsEmpty() && Branch.ManagerCaughtDay > 0)
        {
            Add(2, Branch.Name + TEXT(": m\u00fcd\u00fcr kasadan al\u0131yor"), FString::Printf(TEXT("%s yakaland\u0131. Ma\u011fazalar sayfas\u0131ndan de\u011fi\u015ftir."), *Branch.ManagerName), SMarketMenu::Branches);
            break;
        }
    {
        TArray<FString> Seen;
        for (int32 I = 0; I < State.Branches.Num(); ++I)
        {
            if (State.Branches[I].Stage != static_cast<uint8>(MarketBranches::EStage::Open)) continue;
            const FString Country = MarketBranches::CountryOf(State, State.Branches[I]);
            const FString Province = MarketManagers::AreaOfBranch(State, I, MarketManagers::ELevel::Province);
            if (Province.IsEmpty() || Seen.Contains(Country + TEXT(":") + Province)) continue;
            Seen.Add(Country + TEXT(":") + Province);
            FString Reason;
            if (!MarketManagers::CanAppoint(State, MarketManagers::ELevel::Province, Country, Province, INDEX_NONE, Reason)) continue;
            const int32 Shops = MarketManagers::ProvinceBranches(State, Country, Province);
            Add(Direct >= MarketManagers::SpanLimit ? 1 : 0, FString::Printf(TEXT("%s: il m\u00fcd\u00fcr\u00fc atanabilir"), *MarketManagers::AreaName(MarketManagers::ELevel::Province, Country, Province)),
                FString::Printf(TEXT("\u0130lde %d ma\u011faza var. \u0130l m\u00fcd\u00fcr\u00fc hepsine bakar ve sana ba\u011fl\u0131 ki\u015fi say\u0131s\u0131n\u0131 azalt\u0131r. Ma\u011fazalar \u203a Y\u00f6netim."), Shops), SMarketMenu::Branches);
            break; // one at a time
        }
    }
    // G-086b ek (M19): the family shop can get a manager once the second shop is open; suggested for a week.
    if (MarketManagers::FindManager(State, MarketManagers::ELevel::FamilyShop, State.CountryId, FString()) == INDEX_NONE)
    {
        FString Reason;
        int32 FirstOpen = MAX_int32;
        for (const FMarketBranch& Branch : State.Branches)
            if (Branch.Stage == static_cast<uint8>(MarketBranches::EStage::Open)) FirstOpen = FMath::Min(FirstOpen, Branch.OpenedDay);
        if (FirstOpen != MAX_int32 && State.Day - FirstOpen < 7
            && MarketManagers::CanAppoint(State, MarketManagers::ELevel::FamilyShop, State.CountryId, FString(), INDEX_NONE, Reason))
            Add(0, TEXT("Aile d\u00fckk\u00e2n\u0131na m\u00fcd\u00fcr atanabilir"),
                TEXT("\u0130kinci ma\u011fazan a\u00e7\u0131ld\u0131. D\u00fckk\u00e2n\u0131 bir m\u00fcd\u00fcre b\u0131rak\u0131rsan sipari\u015f, zam ve raflar\u0131 o y\u00fcr\u00fct\u00fcr; sana ba\u011fl\u0131 5 ki\u015fiden biri say\u0131l\u0131r. Ma\u011fazalar \u203a Y\u00f6netim."), SMarketMenu::Branches);
    }
    // G-089 (M23) depots: one without a manager, one beyond its capacity, branches too far from any depot.
    {
        const TArray<FMarketDepot>& Sites = State.Company.DepotSites;
        int32 Unmanaged = 0, FirstUnmanaged = INDEX_NONE;
        for (int32 D = 0; D < Sites.Num(); ++D)
            if (MarketDepots::ManagerOf(State, D) == INDEX_NONE) { if (FirstUnmanaged == INDEX_NONE) FirstUnmanaged = D; ++Unmanaged; }
        if (FirstUnmanaged != INDEX_NONE)
            Add(1, Unmanaged > 1 ? FString::Printf(TEXT("%d deponun m\u00fcd\u00fcr\u00fc yok"), Unmanaged) : MarketDepots::DepotName(State, FirstUnmanaged) + TEXT(": m\u00fcd\u00fcr yok"),
                FString::Printf(TEXT("M\u00fcd\u00fcrs\u00fcz depo yar\u0131 verimle \u00e7al\u0131\u015f\u0131r: ba\u011fl\u0131 %d ma\u011fazada fire ve eksik teslimat artar. Ma\u011fazalar \u203a \u015eirket \u203a Depolar."), MarketDepots::Served(State, FirstUnmanaged)), SMarketMenu::Branches);
        for (int32 D = 0; D < Sites.Num(); ++D)
        {
            const int32 Served = MarketDepots::Served(State, D);
            if (Served <= Sites[D].Capacity) continue;
            Add(1, FString::Printf(TEXT("%s dolu: %d / %d ma\u011faza"), *MarketDepots::DepotName(State, D), Served, Sites[D].Capacity),
                TEXT("Kapasitesini a\u015fan depo yava\u015flar. Yak\u0131na ikinci bir depo kur: Ma\u011fazalar \u203a \u015eirket \u203a Depolar."), SMarketMenu::Branches);
            break;
        }
        int32 Open = 0;
        for (const FMarketBranch& Branch : State.Branches) if (Branch.Stage == static_cast<uint8>(MarketBranches::EStage::Open)) ++Open;
        if (Sites.Num() > 0 || Open + 1 >= MarketDepots::MinStores)
        {
            const TArray<MarketDepots::FLink> Links = MarketDepots::AllLinks(State);
            int32 Far = 0;
            for (int32 I = 0; I < State.Branches.Num(); ++I)
            {
                if (State.Branches[I].Stage != static_cast<uint8>(MarketBranches::EStage::Open) || (Links.IsValidIndex(I) && Links[I].Depot != INDEX_NONE)) continue;
                const FString Country = MarketBranches::CountryOf(State, State.Branches[I]);
                const FString Province = MarketManagers::AreaOfBranch(State, I, MarketManagers::ELevel::Province);
                if (Country == State.CountryId && Province == MarketStart::HomeProvince(State)) continue; // the home wholesaler is next door
                ++Far;
            }
            if (Far > 0)
            {
                const MarketDepots::FAdvice Advice = MarketDepots::SuggestDepotProvince(State, State.CountryId);
                const MarketCountry::FCity* Best = Advice.Province.IsEmpty() ? nullptr : MarketCountry::FindCity(Advice.Country, Advice.Province);
                Add(Far >= 3 ? 1 : 0, FString::Printf(TEXT("%d ma\u011fazan depoya uzak"), Far),
                    FString::Printf(TEXT("600 km i\u00e7inde depo yok: mal toptanc\u0131dan geliyor (+%%3).%s Ma\u011fazalar \u203a \u015eirket \u203a Depolar."),
                        Best ? *FString::Printf(TEXT(" \u00d6nerilen il: %s."), *Best->Name) : TEXT("")), SMarketMenu::Branches);
            }
        }
    }
    // M26: a supermarket or hypermarket running without any department.
    for (const FMarketBranch& B : State.Branches)
    {
        const int32 Format = MarketDepartments::FormatIndex(B.Format);
        if (Format < 2 || B.Stage != static_cast<uint8>(MarketBranches::EStage::Open) || MarketDepartments::SpaceUsed(State, Format) > 0) continue;
        Add(0, Format == 3 ? TEXT("Hipermarketinde reyon yok") : TEXT("S\u00fcpermarketinde taze reyon yok"),
            TEXT("Kasap, f\u0131r\u0131n ve manav her g\u00fcn m\u00fc\u015fteri getirir; hipermarkette elektronik ve giyim de sat\u0131l\u0131r. Ma\u011fazalar \u203a \u015eirket \u203a Reyonlar."), SMarketMenu::Branches);
        break;
    }
    // G-083: a supply line that could move up a tier.
    for (int32 L = 0; L < MarketSourcing::LineCount; ++L)
    {
        const MarketSourcing::ELine Line = static_cast<MarketSourcing::ELine>(L);
        const int32 Next = static_cast<int32>(MarketSourcing::TierOf(State, Line)) + 1;
        FString Why;
        if (Next >= MarketSourcing::TierCount || !MarketSourcing::CanSet(State, Line, static_cast<MarketSourcing::ETier>(Next), Why)) continue;
        Add(0, FString::Printf(TEXT("%s daha ucuza al\u0131nabilir"), *MarketSourcing::LineName(Line)), MarketSourcing::NextStep(State, Line) + TEXT(" Ma\u011fazalar \u203a \u015eirket \u203a Tedarik."), SMarketMenu::Branches);
        break;
    }
    // Karar M25: a brand's offer waiting for an answer.
    if (State.Brands.Offers.Num() > 0)
    {
        const FMarketBrandOffer& Offer = State.Brands.Offers[0];
        Add(0, FString::Printf(TEXT("%s teklif getirdi"), *MarketBrands::NameOf(State, Offer.Brand)),
            MarketBrands::DescribeOffer(State, Offer) + FString::Printf(TEXT(" %d g\u00fcn i\u00e7inde cevap ver: Kampanyalar \u203a Markalar."), FMath::Max(0, Offer.ExpireDay - State.Day)), SMarketMenu::Promotions);
    }
    // Akis C2b: a chain at war with one of our branches, a chain for sale we could buy.
    {
        const TArray<FMarketChain>& Chains = State.Rivals.Chains;
        for (const FMarketChain& Chain : Chains)
        {
            if (Chain.bGone || Chain.WarProvince.IsEmpty() || State.Day > Chain.WarUntil) continue;
            if (MarketBranches::ShopsIn(State, Chain.Country, Chain.WarProvince) <= 0) continue;
            const MarketCountry::FCity* City = MarketCountry::FindCity(Chain.Country, Chain.WarProvince);
            Add(2, FString::Printf(TEXT("%s, %s'da sana kar\u015f\u0131 fiyat sava\u015f\u0131nda"), *Chain.Name, City ? *City->Name : *Chain.WarProvince),
                FString::Printf(TEXT("Oradaki \u015fubenin m\u00fc\u015fterisi azal\u0131yor. Dolu raf ve iyi m\u00fcd\u00fcrle dayan: sava\u015f %d g\u00fcn sonra biter."), FMath::Max(0, Chain.WarUntil - State.Day)),
                SMarketMenu::Branches);
            break;
        }
        for (int32 I = 0; I < Chains.Num(); ++I)
        {
            FString Why;
            if (!Chains[I].bForSale || !MarketChains::CanBuy(State, I, Why)) continue;
            Add(1, FString::Printf(TEXT("%s sat\u0131l\u0131k"), *Chains[I].Name),
                FString::Printf(TEXT("%d ma\u011faza, fiyat\u0131 %s. Ba\u015fka bir zincir kapmadan Rakipler \u203a Ulusal'dan sat\u0131n alabilirsin."), MarketChains::TotalStores(Chains[I]), *MarketCountry::Money(MarketChains::Price(State, I))),
                SMarketMenu::Rivals);
            break;
        }
    }
    TodoCache.StableSort([](const FMarketTodo& A, const FMarketTodo& B) { return A.Severity > B.Severity; });
    return TodoCache;
}

void AMarketGameMode::ClearOrderDraft()
{
    OrderDraftCases.Init(0, Products.Num());
    Notify(TEXT("Sipari\u015f listesi temizlendi."));
}

int32 AMarketGameMode::MoveOrderDraft(int32 From, int32 To)
{
    if (!OrderDraftCases.IsValidIndex(From) || !OrderDraftCases.IsValidIndex(To) || From == To || !State.Stock.IsValidIndex(To) || !Products.IsValidIndex(To)) return 0;
    // The same limits as "Order": at most 9 cases a line, and the store room (store + dock + on the way).
    const int32 Units = FMath::Clamp(Products[To].CaseUnits, 1, 48);
    const FMarketStock& Item = State.Stock[To];
    int32 Moved = 0;
    while (OrderDraftCases[From] > 0 && OrderDraftCases[To] < 9
        && Item.Warehouse + Item.Dock + Item.Incoming + (OrderDraftCases[To] + 1) * Units <= FMarketState::StorageCapacity)
    {
        --OrderDraftCases[From];
        ++OrderDraftCases[To];
        ++Moved;
    }
    if (Moved == 0) Notify(FString::Printf(TEXT("%s i\u00e7in yer yok (sat\u0131r en \u00e7ok 9 koli, depo s\u0131n\u0131r\u0131 %d adet)."), *ProductName(To), FMarketState::StorageCapacity));
    else Notify(FString::Printf(TEXT("%d koli %s yerine %s oldu.%s"), Moved, *ProductName(From), *ProductName(To),
        OrderDraftCases[From] > 0 ? TEXT(" Kalan koliler yerinde duruyor (s\u0131n\u0131r).") : TEXT("")));
    return Moved;
}

void AMarketGameMode::ClearOrderLine(int32 Product)
{
    if (!OrderDraftCases.IsValidIndex(Product) || OrderDraftCases[Product] == 0) return;
    OrderDraftCases[Product] = 0;
    Notify(ProductName(Product) + TEXT(" listeden \u00e7\u0131kt\u0131.\n") + OrderDraftSummary());
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
        Text = Paid > 0 ? FString::Printf(TEXT("Vergi \u00f6dendi: %s. Kalan %s."), *MarketCountry::Money(Paid), *MarketCountry::Money(State.Books.TaxDue))
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

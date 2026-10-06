#include "MarketGame.h"
#include "MarketMenuWidget.h"
#include "MarketFirstStore.h"
#include "MarketStoreVisit.h"
#include "MarketStoreViews.h"
#include "MarketStoreKit.h"
#include "MarketBranchVisit.h"
#include "MarketBranches.h"
#include "MarketStart.h"
#include "MarketStaff.h"
#include "MarketCountry.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

namespace MarketEntryReview
{
    struct FRun { int32 Step = 0, FirstBranch = -1; double At = 0; bool WaitConfirm = false, FinanceCaptured = false, ClosedFinanceCaptured = false; FString Province, Format, TypeLabel; TArray<uint8> Before; };
    FRun Run;
    FString TextOf(const TSharedRef<SWidget>& Widget)
    {
        if (Widget->GetType() == TEXT("STextBlock")) return StaticCastSharedRef<STextBlock>(Widget)->GetText().ToString();
        FString Text; FChildren* Children = Widget->GetChildren();
        for (int32 I = 0; I < Children->Num(); ++I) Text += TextOf(Children->GetChildAt(I));
        return Text;
    }
    bool Click(const TSharedRef<SWidget>& Widget, const FString& Text)
    {
        if (!Widget->GetVisibility().IsVisible()) return false;
        if (Widget->GetType() == TEXT("SButton") && Widget->IsEnabled() && TextOf(Widget) == Text)
        { StaticCastSharedRef<SButton>(Widget)->SimulateClick(); return true; }
        FChildren* Children = Widget->GetChildren();
        for (int32 I = 0; I < Children->Num(); ++I) if (Click(Children->GetChildAt(I), Text)) return true;
        return false;
    }
    void ScrollTop(const TSharedRef<SWidget>& Widget)
    {
        if (!Widget->GetVisibility().IsVisible()) return;
        if (Widget->GetType() == TEXT("SScrollBox")) StaticCastSharedRef<SScrollBox>(Widget)->ScrollToStart();
        FChildren* Children = Widget->GetChildren();
        for (int32 I = 0; I < Children->Num(); ++I) ScrollTop(Children->GetChildAt(I));
    }
    void Capture(const TCHAR* Group, const TCHAR* Name)
    {
        const FString Directory = FPaths::ProjectDir() / TEXT("Docs/Images") / Group;
        IFileManager::Get().MakeDirectory(*Directory, true);
        FScreenshotRequest::RequestScreenshot(Directory / Name, true, false);
    }
}

bool AMarketGameMode::TickStoreEntryReview()
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("StoreEntryReview"))) return true;
    auto& R = MarketEntryReview::Run;
    auto Fail = [this](const TCHAR* Why)
    {
        UE_LOG(LogTemp, Error, TEXT("StoreEntryReview FAILED at %d: %s"), MarketEntryReview::Run.Step, Why);
        UGameplayStatics::DeleteGameInSlot(SlotName(ActiveSlot), 0);
        FPlatformMisc::RequestExitWithStatus(true, 1); return false;
    };
    const double Now = FPlatformTime::Seconds();
    if (R.Step && Now - R.At < 3) return false;
    auto* Player = GetWorld()->GetFirstPlayerController();
    auto* Hud = Player ? Cast<AMarketHUD>(Player->GetHUD()) : nullptr;
    const auto Menu = Hud ? Hud->MenuWidget() : nullptr;
    if (!Menu.IsValid()) return false;
    const FString Keep = TEXT("Kapat \u00b7 bina bo\u015f kals\u0131n");
    const FString Reopen = TEXT("\u0130lk ma\u011fazay\u0131 yeniden a\u00e7");
    const FString Yes = TEXT("Evet  (Enter)");
    auto Click = [&](const FString& Text) { return MarketEntryReview::Click(Menu.ToSharedRef(), Text); };
    switch (R.Step)
    {
    case 0:
        State.Initialize(CatalogBase); MarketStart::Setup(State, State.CountryId, State.CityId, 7919);
        bNeedStart = false; bInStore = true; bTestMode = false; bPauseInMenu = false; bOpen = true;
        State.Cash = 100000000; R.Province = MarketStart::HomeProvince(State);
        MarketStaff::HireBest(State, MarketStaff::ERole::Cashier, Message);
        MarketStaff::HireBest(State, MarketStaff::ERole::Stocker, Message);
        SyncWorkers(); RebuildShelfContents(); ApplyCapacities();
        OpenMenu(SMarketMenu::Finance); MarketEntryReview::ScrollTop(Menu.ToSharedRef());
        break;
    case 1:
        if (!R.FinanceCaptured) { MarketEntryReview::Capture(TEXT("G110"), TEXT("01_finans_acik.png")); R.FinanceCaptured = true; R.At = Now; return false; }
        if (!R.WaitConfirm) { if (!Click(Keep)) return Fail(TEXT("keep/confirm buttons missing: action")); R.WaitConfirm = true; R.At = Now; return false; }
        R.WaitConfirm = false; if (!Click(Yes)) return Fail(TEXT("keep/confirm buttons missing: confirmation"));
        if (!MarketFirstStore::IsOpen(State) || !Message.Contains(TEXT("\u00f6nce g\u00fcn\u00fc kapat"))) return Fail(TEXT("open-day close was not refused"));
        CloseShop(); OpenMenu(SMarketMenu::Finance); MarketEntryReview::ScrollTop(Menu.ToSharedRef());
        break;
    case 2:
        if (!R.WaitConfirm) { if (!Click(Keep)) return Fail(TEXT("closed-day keep buttons missing: action")); R.WaitConfirm = true; R.At = Now; return false; }
        R.WaitConfirm = false; if (!Click(Yes)) return Fail(TEXT("closed-day keep buttons missing: confirmation"));
        if (MarketFirstStore::IsOpen(State) || bInStore || bOpen || !Workers.IsEmpty()) return Fail(TEXT("closure did not return to map/remove workers"));
        for (const auto& Row : State.Stock) if (Row.Shelf || Row.Warehouse || Row.Incoming || Row.Dock) return Fail(TEXT("stock survived closure"));
        if (MarketStaff::OnDutyAt(State, MarketStaff::ERole::Cashier, 0)) return Fail(TEXT("cashier survived closure"));
        Command(TEXT("ToggleShop"));
        if (bOpen || !Message.Contains(TEXT("kapal\u0131"))) return Fail(TEXT("O opened closed first store"));
        if (EnterStore(MarketStoreVisit::FirstStore)) return Fail(TEXT("closed first scene accepted"));
        OpenMenu(SMarketMenu::Finance); MarketEntryReview::ScrollTop(Menu.ToSharedRef());
        break;
    case 3:
        if (!R.ClosedFinanceCaptured) { MarketEntryReview::Capture(TEXT("G110"), TEXT("02_finans_kapali.png")); R.ClosedFinanceCaptured = true; R.At = Now; return false; }
        if (!R.WaitConfirm) { if (!Click(Reopen)) return Fail(TEXT("reopen button")); R.WaitConfirm = true; R.At = Now; return false; }
        R.WaitConfirm = false; if (!Click(Yes) || !MarketFirstStore::IsOpen(State)) return Fail(TEXT("reopen failed"));
        if (!EnterStore(MarketStoreVisit::FirstStore) || bMenuOpen) return Fail(TEXT("reopened scene not available"));
        bPendingStoreEntry = true; ApplyLoadedStoreEntry();
        if (!bInStore || bMenuOpen || IsBranchVisit() || bPendingStoreEntry) return Fail(TEXT("only first store startup"));
        // Two stores of one format in one province. The first scene has another format.
        R.Format = TEXT("kucuk");
        for (int32 I = 0; I < 2; ++I)
        {
            FMarketBranch Branch; Branch.Country = State.CountryId; Branch.Province = R.Province; Branch.Format = R.Format;
            Branch.Name = FString::Printf(TEXT("MarketSim Kontrol %d"), I + 1); Branch.Stage = static_cast<uint8>(MarketBranches::EStage::Open);
            Branch.OpenedDay = State.Day; Branch.Workers = 2; Branch.ManagerName = TEXT("Deniz"); Branch.ManagerMorale = 60;
            MarketStoreViews::AssignTo(State, Branch);
            for (const auto& Product : Products) { auto Item = FMarketStock::Empty(); Item.Id = Product.Id; Item.Capacity = 24; Item.Shelf = I == 0 ? 6 : 18; Branch.Items.Add(Item); }
            State.Branches.Add(Branch);
        }
        ReturnToStoreMap(); Menu->ShowStoreProvince(State.CountryId, R.Province);
        for (const auto& Type : MarketStoreVisit::TypesIn(State, State.CountryId, R.Province)) if (Type.Format == R.Format) R.TypeLabel = FString::Printf(TEXT("%s %d"), *Type.Name, Type.Count);
        if (R.TypeLabel.IsEmpty()) return Fail(TEXT("province types missing"));
        break;
    case 4:
        MarketEntryReview::Capture(TEXT("G119"), TEXT("01_il_karti.png"));
        break; // let the screenshot finish before clicking the row
    case 5:
        if (!Click(R.TypeLabel) || !IsBranchVisit()) return Fail(TEXT("province type did not enter branch"));
        R.FirstBranch = BranchVisitIndex;
        if (StoreEntryHeader() != MarketStoreVisit::Header(State, R.FirstBranch)) return Fail(TEXT("branch header"));
        R.Before = MarketBranchVisit::StateBytes(State);
        break;
    case 6:
        if (R.Before != MarketBranchVisit::StateBytes(State)) return Fail(TEXT("branch view advanced campaign"));
        MarketEntryReview::Capture(TEXT("G119"), TEXT("02_magaza.png"));
        break;
    case 7:
        Player->InputKey(FInputKeyEventArgs(GEngine->GameViewport->Viewport, INPUTDEVICEID_NONE, EKeys::Tab, IE_Pressed, 1.f, false, FPlatformTime::Cycles64()));
        break;
    case 8:
        if (!IsBranchVisit() || BranchVisitIndex != MarketStoreVisit::Next(State, R.FirstBranch)) return Fail(TEXT("Tab did not enter next branch"));
        MarketEntryReview::Capture(TEXT("G119"), TEXT("03_sonraki_magaza.png"));
        break;
    case 9:
        Player->InputKey(FInputKeyEventArgs(GEngine->GameViewport->Viewport, INPUTDEVICEID_NONE, EKeys::Escape, IE_Pressed, 1.f, false, FPlatformTime::Cycles64()));
        break;
    case 10:
        if (IsBranchVisit() || bInStore || !bMenuOpen || MenuPage != SMarketMenu::Summary) return Fail(TEXT("Esc did not return to map"));
        CloseMenu(); if (!bMenuOpen) return Fail(TEXT("map closed through old shortcut"));
        OpenMenu(SMarketMenu::Finance); MarketEntryReview::ScrollTop(Menu.ToSharedRef());
        break;
    case 11:
        if (!R.WaitConfirm) { if (!Click(Keep)) return Fail(TEXT("second close button")); R.WaitConfirm = true; R.At = Now; return false; }
        R.WaitConfirm = false; if (!Click(Yes) || MarketFirstStore::IsOpen(State)) return Fail(TEXT("second first-store close"));
        {
            auto* Save = Cast<UMarketSave>(UGameplayStatics::CreateSaveGameObject(UMarketSave::StaticClass())); Save->State = State;
            if (!UGameplayStatics::SaveGameToSlot(Save, SlotName(ActiveSlot), 0)) return Fail(TEXT("isolated save write"));
            bInStore = true; bPendingStoreEntry = false; LoadCampaign();
            if (!bPendingStoreEntry) return Fail(TEXT("load did not schedule entry"));
        }
        break;
    case 12:
        if (bPendingStoreEntry || bInStore || !bMenuOpen || MenuPage != SMarketMenu::Summary) return Fail(TEXT("closed-first two-branch save did not start at map"));
        if (!R.WaitConfirm) { MarketEntryReview::Capture(TEXT("G119"), TEXT("04_kapali_kayit_harita.png")); R.WaitConfirm = true; R.At = Now; return false; }
        R.WaitConfirm = false;
        State.Branches.RemoveAt(1); bPendingStoreEntry = true;
        break;
    case 13:
        if (!IsBranchVisit() || BranchVisitIndex != 0) return Fail(TEXT("only remaining branch did not start inside"));
        NextStore(); if (BranchVisitIndex != 0) return Fail(TEXT("single branch Tab moved"));
        ReturnToStoreMap(); State.Branches.Reset(); bPendingStoreEntry = true;
        break;
    case 14:
        if (bInStore || !bMenuOpen) return Fail(TEXT("zero-store campaign did not start at map"));
        UGameplayStatics::DeleteGameInSlot(SlotName(ActiveSlot), 0);
        UE_LOG(LogTemp, Display, TEXT("StoreEntryReview PASSED: finance close/reopen, closed O/entry guards, province type click, actual Tab/Esc bindings, isolated closed save/load, only-branch and zero-store startup."));
        FPlatformMisc::RequestExitWithStatus(false, 0); return false;
    }
    ++R.Step; R.At = Now;
    return false; // review owns ticks; the first store cannot run behind its fixture
}

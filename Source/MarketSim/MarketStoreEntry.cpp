#include "MarketGame.h"
#include "MarketStoreVisit.h"
#include "MarketFirstStore.h"
#include "MarketMenuWidget.h"
#include "GameFramework/PlayerController.h"

// G-119: scene navigation only; selection and eligibility belong to MarketStoreVisit.
bool AMarketGameMode::EnterStore(int32 Store)
{
    if (Store == MarketStoreVisit::None || MarketStoreVisit::Header(State, Store).IsEmpty()) return false;
    if (bArrange) ExitArrange(FString());
    if (IsBranchVisit() && BranchVisitIndex == Store) { CloseMenu(); return true; }
    EndBranchVisit();
    if (Store == MarketStoreVisit::FirstStore)
    {
        if (!MarketFirstStore::IsOpen(State)) return false;
        bInStore = true;
        CloseMenu();
        return true;
    }
    if (StartBranchVisit(Store)) return true;
    ReturnToStoreMap();
    Notify(TEXT("Magaza gorunumu acilamadi."));
    return false;
}

void AMarketGameMode::EnterStoreType(const FString& Country, const FString& Province, const FString& Format)
{
    // Wrap without signed overflow; every click advances the session's seed.
    StoreEntrySeed = StoreEntrySeed == MAX_int32 ? 0 : StoreEntrySeed + 1;
    EnterStore(MarketStoreVisit::Pick(State, Country, Province, Format, StoreEntrySeed));
}

void AMarketGameMode::NextStore()
{
    if (!bInStore || bStoreTour || bArrange) return;
    const int32 Current = IsBranchVisit() ? BranchVisitIndex : MarketStoreVisit::FirstStore;
    const int32 Next = MarketStoreVisit::Next(State, Current);
    if (Next != Current && Next != MarketStoreVisit::None) EnterStore(Next);
}

void AMarketGameMode::ReturnToStoreMap()
{
    if (bArrange) ExitArrange(FString());
    EndBranchVisit();
    bInStore = false;
    OpenMenu(SMarketMenu::Summary);
}

void AMarketGameMode::ApplyLoadedStoreEntry()
{
    if (!bPendingStoreEntry) return;
    const auto* Player = GetWorld()->GetFirstPlayerController();
    const auto* Hud = Player ? Cast<AMarketHUD>(Player->GetHUD()) : nullptr;
    if (!Hud || !Hud->MenuWidget().IsValid()) return; // HUD creates Slate lazily after BeginPlay.
    bPendingStoreEntry = false;
    bNeedStart = false;
    if (MarketStoreVisit::StartsInStore(State)) EnterStore(MarketStoreVisit::OnlyStore(State));
    else ReturnToStoreMap();
}

FString AMarketGameMode::StoreEntryHeader() const
{
    return bInStore ? MarketStoreVisit::Header(State, IsBranchVisit() ? BranchVisitIndex : MarketStoreVisit::FirstStore) : FString();
}

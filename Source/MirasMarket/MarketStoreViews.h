#pragma once

#include "CoreMinimal.h"
#include "MarketStoreAssign.h"

struct FMarketState;
struct FMarketBranch;

// G-088 stage C, economy half (Docs/Kurgu/04_MAGAZA_KITI.md, 07_AKIL_ISBOLUMU.md C1): every branch gets the ready-made
// store view of its (country, province, market type) and the view's measured size shapes its sums through the
// MarketStoreAssign factors (variety, tills, fresh room, rent, fit-out, people, stock room). Independent of the
// world, tested (MirasMarket.StoreViews.*).
//  - The catalog (id, format, name, measures) is read once from Config/magazalar.json with MarketStoreKit::Parse
//    into our own copy (the store kit's loaded templates are left alone). Tests set their own catalog.
//  - The choice is saved per site in FMarketState::StoreViews (MarketStoreAssign::Assign) and the measures are
//    copied into the branch when it is signed, so a save keeps its numbers even if magazalar.json changes.
//  - No catalog, or a branch without a view: the format's nominal store, every factor 1.0.
namespace MarketStoreViews
{
    struct FView
    {
        FString Id;                      // magazalar.json id, e.g. "mahalle_01"
        FString Format;                  // kucuk, mahalle, buyuk, hiper
        FString Name;                    // shown to the player
        MarketStoreAssign::FStoreMeasures Measures;
    };

    // The views (loaded on first use; empty when the file is missing or broken).
    const TArray<FView>& Catalog();
    // Tests and tools: replace the catalog (an empty array means "no views": nominal stores).
    void SetCatalog(const TArray<FView>& Views);
    // Forget the catalog so the next Catalog() reads the file again.
    void ResetCatalog();
    const FView* Find(const FString& Id);
    TArray<FString> IdsFor(const FString& Format);

    // The view a site would get now (the saved one first), without writing anything. "" = none.
    FString ViewFor(const FMarketState& State, const FString& Country, const FString& Province, const FString& Format);
    // The measures a new branch of that site would have (the format's nominal store without a view).
    MarketStoreAssign::FStoreMeasures MeasuresFor(const FMarketState& State, const FString& Country, const FString& Province, const FString& Format);

    // Signs a view for a branch: writes State.StoreViews and copies the measures into the branch. Abroad the
    // province part of the key is empty (one view per country and type). Returns the view id ("" = none).
    FString AssignTo(FMarketState& State, FMarketBranch& Branch);
    // The same choice copied into a branch that is not signed yet (a cost preview): State is not written.
    FString PreviewTo(const FMarketState& State, FMarketBranch& Branch);
    // The branch's measures (its own copy; the nominal store when it has none).
    MarketStoreAssign::FStoreMeasures MeasuresOf(const FMarketBranch& Branch);
    bool HasMeasures(const FMarketBranch& Branch);

    // One line for the menu: "Mahalle 01 \u00b7 128 m\u00b2 \u00b7 32 m raf \u00b7 1 kasa".
    FString Describe(const FMarketBranch& Branch);
    // The weekly line when the tills lose shoppers or the fresh room is short ("" = nothing to say).
    FString WeeklyHint(const FMarketBranch& Branch, int32 Shoppers);

}

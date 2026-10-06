#pragma once

#include "CoreMinimal.h"

// G-088 stage C (Docs/Kurgu/04_MAGAZA_KITI.md): which ready-made store view a (country, province, market type) gets,
// the player's shelf category choices in it (karar M18), and what the view's measured "stats" do to the game's
// sums. Pure logic, independent of the world and of MarketStoreKit (Codex's loader): the caller passes the
// template ids and the stats it read from Config/magazalar.json, and keeps the saved maps in FMarketState.
// Tested (MarketSim.StoreAssign.*).
namespace MarketStoreAssign
{
    // The measured size of a store view: magazalar.json "stats" + "salesAreaM2" (+ its format). Lengths in metres,
    // areas in square metres. A field <= 0 where a store always has something (area, shelf, checkouts) counts as
    // "not given" and falls back to the format's nominal value, so a missing or partial stats block behaves like
    // today's game.
    struct FStoreMeasures
    {
        float ShelfFrontM = 0.f;         // front length of all product shelves
        float CoolerM = 0.f;             // chilled cabinets (dairy, deli)
        float FreezerM = 0.f;            // freezers (ice cream, frozen food)
        float ProduceM2 = 0.f;           // fruit & vegetable display
        int32 Counters = 0;              // service counters (deli, bakery, butcher, fish, customer service): the
                                         // number of entries of magazalar.json's "counters" array
        int32 Checkouts = 0;             // staffed tills
        int32 SelfCheckouts = 0;         // self-service tills
        int32 BackroomPallets = 0;       // pallet places in the stock room
        float SalesAreaM2 = 0.f;
    };

    // ---- Assignment (one walkable view per country x province x format; abroad per country x format) ----

    // Save key of a site: folded (lower case, Turkish letters to ASCII) and trimmed parts joined by '|', e.g.
    // ("TR", "Tekirda\u011f", "mahalle") -> "tr|tekirdag|mahalle". Country: the pack id already resolved (the
    // campaign's when a branch has none), so the key never depends on "empty = campaign's". Abroad there is one view
    // per country x format (03_MAGAZA_AGI.md section 5): the caller passes an empty Province there ("bg||kucuk").
    FString SiteKey(const FString& Country, const FString& Province, const FString& Format);

    // Stable 32-bit hash (FNV-1a over UTF-16 code units + a final mix). Our own code, so it does not change with
    // the engine version or the platform.
    uint32 StableHash(const FString& Text);

    // The template for a site, seeded by the site key. Highest-score choice ("rendezvous hashing"): every template
    // gets score = hash(key + template id) and the highest wins (ties: the smaller id). So the order of TemplateIds
    // does not matter, and adding a sixth template moves only about a sixth of the sites. Empty list -> "".
    FString Pick(const FString& Country, const FString& Province, const FString& Format, const TArray<FString>& TemplateIds);

    // Saved choice first: Saved[SiteKey] when it is still one of TemplateIds, else Pick. Empty list -> "".
    FString Resolve(const TMap<FString, FString>& Saved, const FString& Country, const FString& Province, const FString& Format, const TArray<FString>& TemplateIds);

    // Resolve and write the result into Saved (new site or a template that left the list). An empty list (the
    // store file is missing) changes nothing, so a save does not forget its views while the file is broken.
    FString Assign(TMap<FString, FString>& Saved, const FString& Country, const FString& Province, const FString& Format, const TArray<FString>& TemplateIds);

    // ---- Shelf category choices (karar M18) ----
    // A map fixture face -> category override. No entry = the template's own category from magazalar.json.

    // What the picker's last line writes; a fixture with it gets nothing from the shelf staff.
    inline constexpr const TCHAR* Uncategorized = TEXT("Kategorisiz");

    // Key of one face of a fixture: the fixture id for its front (and for single-sided shelves), "<id>#back" for
    // the back face of a double gondola. '#' is not allowed in fixture ids, so the keys never collide.
    FString FaceKey(const FString& FixtureId, bool bBack = false);

    // Player's choice. An empty category or any spelling of "Kategorisiz" stores Uncategorized.
    void SetCategory(TMap<FString, FString>& Choices, const FString& FaceId, const FString& Category);
    // Back to the template's category.
    void ClearCategory(TMap<FString, FString>& Choices, const FString& FaceId);
    // The category in force: the player's choice, else TemplateCategory.
    FString GetCategory(const TMap<FString, FString>& Choices, const FString& FaceId, const FString& TemplateCategory);
    // Empty or "Kategorisiz" (any case, Turkish letters folded).
    bool IsUncategorized(const FString& Category);
    // Does a product of ProductCategory belong on a shelf of FixtureCategory? Folded comparison ("\u0130\u00c7ECEK" ==
    // "i\u00e7ecek" == "icecek"), surrounding spaces ignored; an uncategorized shelf takes nothing.
    bool CategoryMatches(const FString& FixtureCategory, const FString& ProductCategory);

    // ---- Economy (factors against the format's nominal store; the nominal store gives 1.0 everywhere) ----

    // The middle of the format's bands in 04_MAGAZA_KITI.md section 3 (unknown format: mahalle). Tills are the
    // band's middle rounded down (a till is whole), cold length is split 3:1 cooler:freezer (mahalle, ucuzcu) or
    // about 2.4:1 / 2:1 (bigger stores), produce, counters and pallets are the game's own middle values.
    FStoreMeasures Nominal(const FString& Format);
    // Stats with the "not given" fields (see FStoreMeasures) filled from Nominal(Format).
    FStoreMeasures Normalized(const FStoreMeasures& Stats, const FString& Format);

    // How much can stand on the shelves at once (variety and capacity): shelf front / nominal, clamped 0.6..1.5.
    float VarietyFactor(const FStoreMeasures& Stats, const FString& Format);

    // The shoppers a day the format's nominal tills are sized for: the format's catchment Trips x 0.3, the share
    // of the catchment one shop of ours expects (as MarketBranches plans a branch's shelves with). Branch shoppers
    // (FMarketBranch::LastShoppers) are on this scale, not on the catchment's.
    float NominalShoppers(const FString& Format);

    // Sales kept at the tills. Lanes = checkouts + 0.5 x self checkouts. Load = (Shoppers / NominalShoppers) /
    // (Lanes / nominal lanes). Load above 1 loses 25% of sales per whole extra load (queues, walk-outs), below 1
    // gains at most 5% (short queues); clamped 0.6..1.05. Shoppers <= 0 means NominalShoppers.
    float QueueFactor(const FStoreMeasures& Stats, const FString& Format, float Shoppers);

    // Fresh room: cold metres + 0.5 x produce m2 + 3 m per service counter, against nominal (ratio R).
    // FreshFactor = 0.6 + 0.4 x R, clamped 0.6..1.3: demand multiplier of the fresh groups (dairy, ice cream,
    // produce). SpoilFactor = 2 - FreshFactor, clamped 0.7..1.4: waste multiplier (crowded cold room spoils more).
    float FreshFactor(const FStoreMeasures& Stats, const FString& Format);
    float SpoilFactor(const FStoreMeasures& Stats, const FString& Format);

    // Rent follows the sales area: area / nominal, clamped 0.5..1.6.
    float RentFactor(const FStoreMeasures& Stats, const FString& Format);
    // Fit-out: 0.3 fixed (permits, facade, tills) + 0.4 x area ratio + 0.3 x shelf ratio, clamped 0.6..1.5.
    float FitOutFactor(const FStoreMeasures& Stats, const FString& Format);
    // Stock room: pallet places / nominal, clamped 0.6..1.5 (how many days of stock fit behind the shop).
    float BackroomFactor(const FStoreMeasures& Stats, const FString& Format);

    // People the store needs: format's Workers x (0.5 x area ratio + 0.5 x staffed-lane ratio), rounded, kept
    // within half and twice the format's Workers (at least 1). Staffed lanes = checkouts + self checkouts / 4
    // (one attendant watches four). The nominal store needs exactly the format's Workers.
    int32 WorkersFor(const FStoreMeasures& Stats, const FString& Format);

    // Convenience in money (int64 kurus at the list level of GameDay, MarketPrices::Scaled): the format's
    // fit-out x FitOutFactor, and its monthly base rent x RentFactor (before the province's rent multiplier).
    int64 FitOutCost(const FStoreMeasures& Stats, const FString& Format, int32 GameDay);
    int64 BaseMonthlyRent(const FStoreMeasures& Stats, const FString& Format, int32 GameDay);
}

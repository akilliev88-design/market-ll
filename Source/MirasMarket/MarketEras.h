#pragma once

#include "CoreMinimal.h"
#include "MarketEras.generated.h"

struct FMarketState;

// Ak\u0131\u015f B4 (Docs/Kurgu/07_AKIL_ISBOLUMU.md \u00a74, 06_GIDIS_YOLU.md \u00a75): eras of the country's economy. Independent of
// the world, tested (MirasMarket.Eras.*).
//
// The order is fixed: currency shock -> recession -> the great epidemic -> high inflation -> recovery (and in a
// shaky economy a second, milder currency shock / inflation wave years later). Their time moves with the campaign:
// the whole plan is shifted by -2..+2 years and a few weeks (seeded), so a player who remembers history cannot read
// the future. How many come and how hard they hit depends on the country's economy character (ulkeler.json
// economy.character: stable / volatile / high inflation). Names are generic ("kur \u015foku"), no year is shown (M24).
//
// Effects: the inflation peaks of the price curve move with the plan (MarketPrices asks InflationBump; the
// unshifted plan gives exactly the built-in curve), the currency shock makes imported goods dearer for a while,
// recession and high inflation make shoppers count their money, recovery brings them back (MarketEvents
// modifiers, which the game already applies). The epidemic is MarketOnline's 2020-2021 profile, moved to the
// plan's year (MarketOnline::PandemicStart); switching the profile off still switches it off.
// Older saves and campaigns without Setup keep the unshifted plan (bPlanned false).
USTRUCT()
struct FMarketEras
{
    GENERATED_BODY()
    UPROPERTY() bool bPlanned = false;   // Setup ran for this campaign (else the unshifted plan)
    UPROPERTY() int32 ShiftYears = 0;    // -2..2
    UPROPERTY() int32 ShiftDays = 0;     // -45..45: news and effects inside the year
    UPROPERTY() int32 Started = 0;       // bit per era of the plan: its start was told and its effects added
    UPROPERTY() int32 Ended = 0;         // bit per era: its end was told
    UPROPERTY() bool bChecked = false;   // eras already running when the books first saw this campaign are marked
};

namespace MarketEras
{
    enum class EKind : uint8 { CurrencyShock = 0, Recession, Pandemic, HighInflation, Recovery, Count };
    // MarketCountry::ECharacter as uint8 (0 stable, 1 volatile, 2 high inflation).
    enum class ECharacter : uint8 { Stable = 0, Volatile, HighInflation };

    struct FEra
    {
        EKind Kind = EKind::CurrencyShock;
        int32 StartDay = 0;
        int32 EndDay = 0;
        int32 StartYear = 0;   // calendar year its inflation peak starts (internal anchor only)
        float Strength = 1.f;  // 0..1
        int32 Wave = 1;        // 2 = the later, milder repeat
    };

    // The plan of a country character for a campaign seed and shift. Ordered by start day.
    TArray<FEra> Plan(ECharacter Character, int32 Seed, int32 ShiftYears, int32 ShiftDays);
    // The plan of this campaign (its country, seed and shift).
    TArray<FEra> PlanOf(const FMarketState& State);
    // New campaign (MarketStart::Setup, after the country and seed are set): picks the shift once.
    void Setup(FMarketState& State);
    // Makes this campaign's plan the one MarketPrices uses (after loading / starting a campaign; the day close does
    // it too). MarketCountry::SetActive switches to the country's unshifted plan.
    void Activate(const FMarketState& State);
    void ActivateNominal(ECharacter Character);
    // Extra yearly inflation of the active plan in a calendar year, and what the unshifted high-inflation plan adds
    // (the built-in Turkish curve already contains that).
    double InflationBump(int32 Year);
    double BuiltInBump(int32 Year);

    // The era running on a game day (nullptr: none). The epidemic follows MarketOnline's profile.
    bool Current(const FMarketState& State, int32 GameDay, FEra& OutEra);
    // Day shift of the epidemic against its built-in date (MarketOnline).
    int32 PandemicShiftDays(const FMarketState& State);
    // x a shopper's budget today (recession: smaller baskets; recovery: a little bigger). Director::BudgetFactor.
    float BudgetFactor(const FMarketState& State);
    FString Name(EKind Kind);
    // One sentence for the menu ("Ekonomi: y\u00fcksek enflasyon d\u00f6nemi; fiyatlar her ay art\u0131yor.") or "" when calm.
    FString Summary(const FMarketState& State);

    // Day close (MarketDirector, Ak\u0131\u015f B block): activates the plan, starts and ends eras (effects, one news line).
    void CloseDay(FMarketState& State);
}

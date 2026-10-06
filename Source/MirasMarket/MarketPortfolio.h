#pragma once

#include "CoreMinimal.h"

struct FMarketState;
struct FMarketProduct;
struct FMarketBranch;

// D9b (M46, Mustafa 04.10.2026): the store portfolio. Independent of the world, tested (MirasMarket.Portfolio.*).
//  - Ageing: a store is new for six years; from then on it loses its shine, 2.5 % of its pull a year, at most 15 %
//    (twelve years). Renewing it (any of the works below) starts its age again and its first year draws 4 % more.
//  - Works close the store for two or three weeks (big types three); the rent and the people's wages run on and are
//    paid with the works:
//      renovation      - 40 % of today's fit-out; only for a store of three years or more;
//      change of type  - 70 % of the new type's fit-out, the deposit of the new rent, the new shelves' goods at the
//                        reopening; the district's habit stays at 80 %; the rules of an opening (the country's types,
//                        big provinces for big types, a depot) apply;
//      relocation      - within the same province (Mustafa: "ba\u015fka ile ta\u015f\u0131mak = kapat\u0131p a\u00e7mak"): 60 % of the
//                        fit-out and the deposit of the new rent; a hasty site is left behind, 60 % of the habit stays.
//    Changing type and relocating sign a new lease at today's rent.
//  - The yearly report card: when a campaign year ends every store open for four months or more gets A..E from its
//    year's net margin, its customers' satisfaction and its age, with a hint (renew, relocate, grow, close).
namespace MarketPortfolio
{
    enum class EWorks : uint8 { None = 0, Renovate, Reformat, Relocate };

    constexpr int32 YearDays = 365;
    constexpr int32 AgeStartYears = 6;
    constexpr int32 AgeFullYears = 12;
    constexpr float AgeLoss = 0.15f;
    constexpr int32 FreshDays = 365;
    constexpr float FreshPull = 1.04f;
    constexpr int32 RenovateMinYears = 3;
    constexpr float RenovateShare = 0.4f;
    constexpr float ReformatShare = 0.7f;
    constexpr float RelocateShare = 0.6f;
    constexpr int32 SmallWorksDays = 14;
    constexpr int32 BigWorksDays = 21;
    constexpr float ReformatHabit = 0.8f;
    constexpr float RelocateHabit = 0.6f;
    constexpr int32 CardMinDays = 120;

    // Days since the store opened or was last renewed.
    int32 AgeDays(const FMarketState& State, const FMarketBranch& Branch);
    // x a store's pull: its age (1 .. 0.85) and a renewed store's first year (x1.04). 1 for the first store (-1)
    // and anything that is not a branch.
    float AgeFactor(const FMarketState& State, int32 BranchIndex);
    float AgeOnly(const FMarketState& State, const FMarketBranch& Branch);
    bool IsAgeing(const FMarketState& State, const FMarketBranch& Branch);

    // The next larger and smaller type the store's country knows ("" none): ucuzcu / yakin -> mahalle -> supermarket
    // -> hipermarket (and toptan beside it).
    FString Larger(const FMarketState& State, int32 BranchIndex);
    FString Smaller(const FMarketState& State, int32 BranchIndex);

    int32 WorksDays(const FString& Format);
    // Everything the works cost now (Format: the new type of a change of type).
    int64 WorksCost(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 BranchIndex, EWorks Works, const FString& Format = FString());
    bool CanStart(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 BranchIndex, EWorks Works, const FString& Format, FString& OutReason);
    bool Renovate(FMarketState& State, const TArray<FMarketProduct>& Products, int32 BranchIndex, FString& OutMessage);
    bool Reformat(FMarketState& State, const TArray<FMarketProduct>& Products, int32 BranchIndex, const FString& Format, FString& OutMessage);
    bool Relocate(FMarketState& State, const TArray<FMarketProduct>& Products, int32 BranchIndex, FString& OutMessage);
    // The works are done (MarketBranches::CloseDay, the day they end): the store opens again. Returns the news line.
    FString Finish(FMarketState& State, const TArray<FMarketProduct>& Products, int32 BranchIndex);
    // Menu command argument for a change of type: branch index x 10 + index into MarketBranches::AllFormatIds().
    int32 EncodeFormat(int32 BranchIndex, const FString& Format);
    bool DecodeFormat(int32 Arg, int32& OutBranch, FString& OutFormat);

    // --- the yearly report card
    // 1 A .. 5 E from a year's revenue and profit, satisfaction and age (0: no card).
    uint8 CardOf(float Margin, float Satisfaction, float AgeFactorNow);
    FString CardLetter(uint8 Card);   // "A".."E", "-" for none
    // What the card suggests ("" nothing): renew, relocate a weak site, grow, close.
    FString Advice(const FMarketState& State, int32 BranchIndex);
    // The store's portfolio line for the menu: age, the last card, the advice or the works under way.
    FString Line(const FMarketState& State, int32 BranchIndex);

    // Day close (MarketDirector, after the branches): the yearly cards when a campaign year has ended.
    void CloseDay(FMarketState& State);
}

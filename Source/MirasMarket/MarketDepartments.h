#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Reyonlar (karar M26, Mustafa 01.10.2026: "hipermarkette sat\u0131labilecek her \u015fey"). Independent of the world, tested
// (MirasMarket.Departments.*). The packaged catalog (Config/products.json) stays the shelf of every shop; on top of
// it a branch can run departments, each an aggregate business of its own, without single products (their
// representative goods come with the in-store simulation, A\u015fama 2):
//  - Fresh (from the supermarket; the greengrocer from the neighbourhood market): greengrocer, butcher, deli,
//    bakery, fishmonger. They pull shoppers (fresh bread and good meat bring people every day), spoil, and the
//    butcher, the baker and the fishmonger need a master whose skill is their quality.
//  - Non-food (the hypermarket; baby and pet goods from the supermarket): electronics, clothing, home and kitchen,
//    toys, stationery, baby, pets, garden and car, seasonal. Big stock tied up for months, thin (electronics) or
//    fat but seasonal (clothing: clearance in January and July) margins, theft, income-sensitive demand.
//  - Tycoon level, not per shop: the player decides per store type which departments run (all branches of that
//    type open them; a new branch opens with them) and a price stance per department (cheap / normal / dear).
//    The floor is limited: a store type has a share of its area for departments, so a supermarket chooses.
//  - A day: the branch's shoppers x a ticket x the department's share x its season x income x stance x quality
//    (master) x its first month's ramp; gross margin, waste (master), theft, staff. Added to the branch's day.
//  - Fun (06 section 2b): a clear choice with visible results (30-day revenue and profit per department), seasons
//    that make a month exciting (toys at New Year, stationery in September, the butcher before Kurban Bayram\u0131).
namespace MarketDepartments
{
    enum class EDept : uint8
    {
        Produce = 0, Butcher, Deli, Bakery, Fish,
        Electronics, Clothing, Home, Toys, Stationery, Baby, Pets, Garden, Seasonal,
        Count
    };
    constexpr int32 DeptCount = static_cast<int32>(EDept::Count);
    constexpr int32 FormatCount = 4;               // kucuk, mahalle, buyuk, hiper (MarketBranches::FormatIds)
    constexpr int32 RampDays = 30;
    constexpr int64 Ticket2011 = 800;              // a shopper's packaged basket at the start level, kurus
    constexpr float RefundShare = 0.6f;            // stock sold off when a department closes

    struct FInfo
    {
        const TCHAR* Id;
        const TCHAR* Name;
        bool bFresh;
        bool bMaster;             // needs a master (butcher, baker, fishmonger)
        int32 MinFormat;          // 1 mahalle, 2 buyuk, 3 hiper
        int32 Space;              // percent of the store's area
        float Ratio;              // sales against the packaged basket
        float Margin;             // gross
        float Waste;              // of sales, at a middling master
        float Shrink;             // theft, of sales
        float Pull;               // extra pull of the store (x quality)
        float FitOut;             // x the store type's fit-out
        int32 StockDays;          // stock held, days of cost of goods
        float IncomeElastic;      // demand x income^this
        float Season[12];         // January .. December
        bool bClearance;          // January and July: sold off, margin x 0.6
    };
    const FInfo& Info(EDept Dept);
    FString Name(EDept Dept);
    // Store type index of a branch format id (0 kucuk .. 3 hiper; unknown -> 1).
    int32 FormatIndex(const FString& Format);
    // Percent of a store type's area departments may take (0, 10, 22, 70).
    int32 SpaceCap(int32 Format);
    int32 SpaceUsed(const FMarketState& State, int32 Format);

    bool IsOn(const FMarketState& State, EDept Dept, int32 Format);
    bool CanSet(const FMarketState& State, EDept Dept, int32 Format, bool bOn, FString& OutReason);
    // Turns a department on or off for every branch of a store type (opens or closes it there at once).
    bool Set(FMarketState& State, EDept Dept, int32 Format, bool bOn, FString& OutMessage);
    // 0 cheap, 1 normal, 2 dear.
    int32 Stance(const FMarketState& State, EDept Dept);
    bool SetStance(FMarketState& State, EDept Dept, int32 NewStance, FString& OutMessage);
    // Replaces masters under 50 in every branch (a week's wage each).
    int32 WeakMasters(const FMarketState& State, EDept Dept);
    bool ReplaceWeakMasters(FMarketState& State, EDept Dept, FString& OutMessage);
    // Menu arguments.
    int32 EncodeSet(EDept Dept, int32 Format, bool bOn);
    bool DecodeSet(int32 Arg, EDept& OutDept, int32& OutFormat, bool& bOutOn);

    // Seasonal demand of a department on a game day (calendar month; the butcher before Kurban Bayram\u0131).
    float SeasonFactor(EDept Dept, int32 GameDay);
    // Opening cost of a department in a store type now (fit-out; the stock comes on top).
    int64 FitOutCost(EDept Dept, int32 Format, int32 GameDay);

    // x the branch's pull (its departments' draw, x quality and ramp).
    float PullFactor(const FMarketBranch& Branch, int32 GameDay);

    struct FDay
    {
        int64 Revenue = 0;
        int64 Profit = 0;
        int64 Purchases = 0;      // goods bought (sold + spoiled)
    };
    // One branch's departments for a closed day (updates their 30-day sums and stock).
    FDay Day(FMarketState& State, int32 BranchIndex, int32 Shoppers, float Income, int32 Closed);

    // Menu: one department's line (margin, space, season) and its company numbers.
    FString Describe(EDept Dept);
    FString Results(const FMarketState& State, EDept Dept);
    int32 BranchesWith(const FMarketState& State, EDept Dept);
    // "Kasap, f\u0131r\u0131n" of a branch ("" none).
    FString BranchLine(const FMarketBranch& Branch);

    // Day close (before the branches' day): policies reach new branches, masters learn, weekly hints.
    void CloseDay(FMarketState& State);
}

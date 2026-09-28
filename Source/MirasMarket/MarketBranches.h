#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Branches (G-068, Docs/Kurgu/00_KURGU_KITABI.md \u00a73, \u00a710). Independent of the world, tested (MirasMarket.Branches.*).
// A branch opens in a district of L\u00fcleburgaz (fictional neighbourhoods): lease and fit-out -> permits -> hiring ->
// opening stock -> open. Its shelves are planned automatically (MarketLayout) and its day is simulated from the same
// rules as the family shop, without walking customers: the district's shoppers split between us and the rivals by
// attractiveness (price, full shelves, service, habit); what shoppers want follows the district's people and the
// calendar; what is not on the shelf is a lost sale. A manager orders for tomorrow (a skilled one closer to the
// real demand), keeps prices near the rivals, and a dishonest one skims a little. A new branch builds its habit
// over 30 days; two of our shops in the same or neighbouring districts take customers from each other.
namespace MarketBranches
{
    enum class EDistrict : uint8 { Istasyon = 0, Carsi, Kocasinan, YeniMahalle, Universite, Sanayi, Evrensekiz, Count };
    enum class EStage : uint8 { Renovation = 0, Permits, Hiring, Open, Closed };

    struct FDistrict
    {
        const TCHAR* Name = TEXT("");
        const TCHAR* Note = TEXT("");
        int32 Shoppers = 200;            // shopping trips a day in the district (all shops)
        float Income = 1.f;              // wallet size
        int64 Rent = 50000;              // per month, 2011 kurus
        int32 Mix[6] = { 20, 30, 25, 10, 5, 10 }; // MarketCustomers segments
        float RivalAttraction = 3.f;     // sum of the rivals' pull in the district
        uint32 Neighbours = 0;           // bit mask of adjacent districts
    };

    struct FFormat
    {
        const TCHAR* Id = TEXT("mahalle");
        const TCHAR* Name = TEXT("mahalle marketi");
        int64 FitOut = 400000;           // 2011 kurus
        int32 Workers = 3;
        float Service = 1.f;
    };

    constexpr int32 RenovationDays = 5;
    constexpr int32 PermitDays = 3;
    constexpr int32 MaturityDays = 30;
    constexpr float UnitsPerShopper = 2.2f;

    const FDistrict& DistrictInfo(EDistrict District);
    const FFormat& FormatInfo(const FString& Id);
    FString DistrictName(EDistrict District);
    // Money needed now to open: deposit (2 months' rent), fit-out and the opening stock at cost.
    int64 OpeningCost(const FMarketState& State, const TArray<FMarketProduct>& Products, EDistrict District, const FString& Format);
    bool CanOpen(const FMarketState& State, const TArray<FMarketProduct>& Products, EDistrict District, const FString& Format, FString& OutReason);
    bool Open(FMarketState& State, const TArray<FMarketProduct>& Products, EDistrict District, const FString& Format, FString& OutMessage);
    bool Close(FMarketState& State, int32 BranchIndex, FString& OutMessage);
    // Moves a person from the family shop to run a branch (Cem's road in the story).
    bool Promote(FMarketState& State, int32 EmployeeId, int32 BranchIndex, FString& OutMessage);
    // Old saves with the aggregate second shop become a mature branch in \u00c7ar\u015f\u0131.
    void Migrate(FMarketState& State, const TArray<FMarketProduct>& Products);
    int32 OpenCount(const FMarketState& State);
    // x shoppers of the family shop: our branches next door take some of its customers.
    float MainShopFactor(const FMarketState& State);
    FString Summary(const FMarketState& State, int32 BranchIndex, const TArray<FMarketProduct>& Products);

    // Day close: stages, the simulated day of every open branch, the managers' orders, weekly lines.
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);
}

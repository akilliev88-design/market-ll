#pragma once
#include "CoreMinimal.h"
#include "MarketEconomy.h"

namespace MarketAutoPlay
{
    enum class EStyle : uint8 { Careful, Balanced, Bold };
    struct FProfile
    {
        EStyle Style = EStyle::Balanced;
        FString Name;
        int32 BufferDays = 14;
        double ExpansionBuffer = 1.5;
        bool bBorrow = false;
        int32 GrowthInterval = 14;
        double PriceFactor = 1.0;
        int32 DepotAt = 8;
    };
    const TArray<FProfile>& Profiles();
    struct FOptions
    {
        int32 Days = 3652;
        int32 Seeds = 3;
        int32 FirstSeed = 21;
        FString Country = TEXT("tr");
        FString Province = TEXT("kirklareli");
    };
    struct FRow
    {
        int32 Day = 0;
        int64 Cash = 0, Debt = 0, Profit = 0, Revenue = 0;
        int32 Stores = 1, Provinces = 1, Workers = 0;
        float Share = 0;
    };
    struct FRun
    {
        FString Profile;
        int32 Seed = 0;
        TArray<FRow> Daily;
        TArray<FRow> Weekly;
        int32 NegativeDays = 0, TroubleDays = 0, AuditFailures = 0;
        TMap<FString, int32> Milestones;
        TMap<FString, int64> Expenses;
        TMap<FString, int64> Results;
        TArray<FString> Issues;
        int64 BackgroundNet = 0;
        int32 RejectedDecisions = 0;
    };
    struct FReport
    {
        FOptions Options;
        TArray<FRun> Runs;
        double Seconds = 0;
        TArray<FString> Errors;
    };
    // Pure campaign runner; never writes player saves, creates actors or grants money/stock.
    FReport Run(const FOptions& Options, const TArray<FMarketProduct>& Base, const TArray<int32>& Capacities, int32 RestoreSeed = 0);
    bool LoadInputs(TArray<FMarketProduct>& OutBase, TArray<int32>& OutCapacities, TArray<FString>& OutErrors);
    // Runtime output path is supplied separately: timestamps affect filenames only, never decisions.
    bool WriteReport(const FReport& Report, const FString& Directory);
    TArray<FString> Validate(const FMarketState& State, const TArray<FMarketProduct>& Products);
}

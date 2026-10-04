#pragma once
#include "CoreMinimal.h"
#include "MarketEconomy.h"
namespace MarketAutoPlayRescue
{
    struct FPlan
    {
        int32 Day=0, Until=0, FirstBranch=0, Paid=0, Replaced=0, Stores=0;
        int64 Principal=0, Remaining=0, WrittenOff=0;
        float Rate=0;
    };
    struct FYear { int32 Year=0, Day=0; int64 Debt=0, FirstStoreRevenue=0; };
    struct FStats
    {
        TArray<FPlan> Plans;
        TArray<FYear> Years;
        int64 BeforeDebt=0, BeforeMovement=0, BeforeTaxPenalties=0, BeforeBills=0, BeforeSupplierMovement=0, Revenue=0;
        int32 BeforeRescues=0, LastBeginDay=0, LastDay=0, BlockedDays=0;
    };
    bool Blocked(const FMarketState& State);
    void BeginDay(const FMarketState& State,FStats& Stats);
    void Observe(const FMarketState& State,int32 Year,FStats& Stats);
    FString Report(const FStats& Stats);
    FString PlansCsv(const FStats& Stats,const FString& Style,int32 Seed);
    FString YearsCsv(const FStats& Stats,const FString& Style,int32 Seed);
}

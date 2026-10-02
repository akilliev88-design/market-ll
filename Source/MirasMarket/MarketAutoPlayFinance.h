#pragma once
#include "CoreMinimal.h"
#include "MarketEconomy.h"
namespace MarketAutoPlayFinance
{
    struct FBranchResult
    {
        FString Name, Format;
        int32 Opened=0, Days=0, Closed=0;
        int64 Revenue=0, Gross=0, Rent=0, Wages=0, Sgk=0, Running=0, Logistics=0, Waste=0, Net=0;
    };
    struct FBankYear
    {
        int32 Year=0, Day=0, Rating=0, Subsidiaries=0, Stores=0;
        int64 Debt=0, Ebitda=0, Interest=0, Line=0, Limit=0;
        float Leverage=0;
    };
    struct FStats
    {
        TMap<int32,FBranchResult> Branches;
        TMap<int32,int32> RedMonths;
        TMap<FString,int32> Commands, Attempts;
        TArray<FBankYear> Years;
        TArray<int32> RescueDays;
        TSet<FString> Exits, Gates;
        int32 ObservedDay=0, LastTurn=0, LastRatingDay=0, BreachMonths=0;
        int32 Rescues=0, Closed=0, Bids=0, Accepted=0, Refused=0, Financed=0, Converted=0;
        int64 Interest=0, PeakLine=0;
    };
    int64 GoodsReserve(const FMarketState& State);
    bool WorthHiring(const FMarketState& State, int64 MonthlyBenefit, int64 DailyWage, int64 Fee = 0);
    int64 NetworkReserve(const FMarketState& State);
    bool CanExpand(const FMarketState& State,int64 Opening,int64 NewMonthly,double Buffer);
    bool LosingMonth(const FMarketBranch& Branch,int32 Day,int32& RedMonths);
    void Decide(FMarketState& State,const TArray<FMarketProduct>& Products,bool Aggressive,bool Careful,FStats& Stats);
    void Observe(const FMarketState& State,int32 Year,FStats& Stats);
    FString Report(const FStats& Stats);
    FString BranchCsv(const FStats& Stats,const FString& Style,int32 Seed);
    FString BankCsv(const FStats& Stats,const FString& Style,int32 Seed);
    FString SummaryCsv(const FStats& Stats,const FString& Style,int32 Seed);
}

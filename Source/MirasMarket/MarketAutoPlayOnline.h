#pragma once
#include "CoreMinimal.h"
#include "MarketEconomy.h"

namespace MarketAutoPlayOnline
{
    struct FYear
    {
        int32 Year=0, Day=0, Days=0, DarkStores=0, PandemicDays=0;
        int64 Orders[4]={}, Revenue[4]={}, Contribution[4]={};
        int64 StoreRevenue=0, TotalProfit=0, CommonCosts=0, Shoppers=0, PandemicOrders=0, PandemicShoppers=0;
        double CountryShare=0, Traffic=0;
    };
    struct FEvent { int32 Day=0, Option=-1, Arg=0; FString Kind, Id; };
    struct FStats
    {
        FYear Current;
        TArray<FYear> Years;
        TArray<FEvent> Events;
        TArray<int32> BeforeOrders;
        TArray<int64> BeforeRevenue, BeforeProfit;
        TSet<FString> Rivals, SeenCards;
        int32 FirstOpen[4]={}, LastDay=0, LastDecisionDay=0, Proposals=0, Accepted=0, Rejected=0;
        int32 PandemicStart=0, PandemicEnd=0;
        bool LastOn[4]={}, Offline=false, PlatformLeft=false;
        TMap<FString,int32> Commands;
    };
    int32 Choice(const FMarketState& State, const FMarketDecision& Card, int32 Style, bool Offline);
    void RecordChoice(const FMarketState& State, const FMarketDecision& Card, int32 Option, FStats& Stats);
    void Decide(FMarketState& State, const TArray<FMarketProduct>& Products, int32 Style, FStats& Stats);
    void BeginDay(const FMarketState& State, FStats& Stats);
    void Observe(const FMarketState& State, int32 Year, int32 FirstStoreShoppers, FStats& Stats);
    FString Report(const FStats& Stats);
    FString YearsCsv(const FStats& Stats, const FString& Style, int32 Seed);
    FString EventsCsv(const FStats& Stats, const FString& Style, int32 Seed);
}

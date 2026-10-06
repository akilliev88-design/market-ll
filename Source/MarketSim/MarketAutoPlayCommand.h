#pragma once
#include "CoreMinimal.h"
#include "MarketEconomy.h"
namespace MarketAutoPlayCommand
{
    struct FYear
    {
        int32 Year=0, Day=0;
        FString Country;
        int64 Spend[6]={}, Estimate[6]={};
    };
    struct FStats
    {
        TArray<FYear> Years;
        TMap<FString,FMarketAdCountry> BeforeAds;
        TMap<FString,int32> BeforeMarkdown;
        TSet<FString> SeenCards;
        TMap<int32,FString> StreetNames;
        TMap<int32,bool> StreetOpen;
        TArray<FString> Events, Weather;
        int32 OpenProposals=0, CloseProposals=0, OpenAccepted=0, CloseAccepted=0, OpenRejected=0, CloseRejected=0;
        int32 ClearanceItems=0, ClearanceWeeks=0, LastDay=0, LastDecision=0;
        int64 ManagerCosts=0;
        bool NamesLogged=false;
    };
    int32 Desired(int32 Style,int32 Shops,int32 Channel,bool Online);
    int32 Choice(const FMarketState& State,const TArray<FMarketProduct>& Products,const FMarketDecision& Card,double ExpansionBuffer);
    void RecordChoice(const FMarketState& State,const FMarketDecision& Card,int32 Option,FStats& Stats);
    void Decide(FMarketState& State,const TArray<FMarketProduct>& Products,int32 Style,FStats& Stats);
    void BeginDay(const FMarketState& State,FStats& Stats);
    void Observe(const FMarketState& State,int32 Year,FStats& Stats);
    FString Report(const FStats& Stats);
    FString YearsCsv(const FStats& Stats,const FString& Style,int32 Seed);
    FString EventsCsv(const FStats& Stats,const FString& Style,int32 Seed);
    FString WeatherCsv(const FStats& Stats,const FString& Style,int32 Seed);
}

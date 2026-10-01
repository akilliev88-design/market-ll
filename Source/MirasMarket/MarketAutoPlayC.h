#pragma once
#include "CoreMinimal.h"
#include "MarketEconomy.h"
namespace MarketAutoPlayC
{
    struct FPolicy
    {
        int32 StartDay=180, Interval=30, OpensPerTurn=1, Stance=1;
        float SpaceFraction=.5f, MinimumCover=1.25f, BrandCover=1.1f;
        bool bBuyChains=false;
        double BuyBuffer=1.5;
    };
    struct FYear { int32 Year=0, Day=0, National=0, World=0, Stores=0; double OurWorld=0, LeaderWorld=0; };
    struct FDept { int32 Day=0, Format=0, Dept=0, Branches=0; int64 Profit=0, Revenue=0; };
    struct FChange { int32 Day=0, Line=0, From=0, To=0; };
    struct FEraResult
    {
        int32 Kind=0, Wave=1, Start=0, End=0, Days=0;
        int64 FirstCash=0, LastCash=0, LowestCash=0, Profit=0;
    };
    struct FStats
    {
        TArray<FYear> Years;
        TArray<FDept> Departments;
        TArray<FChange> Sourcing;
        TMap<FString,int32> Commands, Blocked;
        TMap<FString,int64> WorstDept, BestDept, LastDept;
        TMap<FString,int32> DeptCooldown;
        TMap<FString,int64> PreviousBuy;
        TArray<TArray<int64>> DailyBuy;
        TArray<int32> Tiers;
        TSet<FString> SeenEvents, SeenDecisions, SeenOffers, SeenGone, SeenSale, SeenWars, CoolBrands, Purchased;
        TArray<int32> BadDays, BaseBadDays;
        int32 BankruptcyNews=0, Purchases=0, ChainPeak=0, Gone=0, Sale=0, Wars=0, Rejected=0;
        int32 QuietBase=0, QuietAll=0, BoringBase=0, BoringAll=0, PilesBase=0, PilesAll=0;
        bool bPileBase=false, bPileAll=false;
        int32 LastNational=0, LastWorld=0, LastStores=1;
        int64 BrandMoney=0;
        FString Nemesis;
        TArray<FEraResult> Eras;
        TSet<FString> SeenGoals;
        int32 ObservedDay=0, GapDays=0, GoalsSeen=0, GoalsCompleted=0, Celebrations=0;
        int32 Closures=0, Takeovers=0, OurBuys=0, QuietEvents=0, HeldBadEvents=0;
        int32 RhythmBoring=0, RhythmLongest=0;
        bool bRhythmBoring=false;
        int64 GapTotal=0, GapAbsolute=0;
    };
    bool SiteSuitable(const FMarketState& State,const FString& Country,const FString& Province,const FString& Format);
    bool BrandWorth(const FMarketState& State,const TArray<FMarketProduct>& Products,const FMarketBrandOffer& Offer,const FPolicy& Policy);
    double PurchaseForecast(const FMarketState& State,int32 Line,const FStats& Stats);
    void Decide(FMarketState& State,const TArray<FMarketProduct>& Products,const FPolicy& Policy,int64 Reserve,FStats& Stats);
    void Observe(const FMarketState& State,FStats& Stats);
    FString Report(const FStats& Stats);
    FString DeptCsv(const FStats& Stats,const FString& Style,int32 Seed);
    FString EraCsv(const FStats& Stats,const FString& Style,int32 Seed);
}

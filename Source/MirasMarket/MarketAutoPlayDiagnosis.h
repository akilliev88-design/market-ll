#pragma once
#include "CoreMinimal.h"
#include "MarketEconomy.h"
#include "MarketSimulation.h"
namespace MarketAutoPlayDiagnosis
{
    struct FStats
    {
        TArray<FString> Days, Events, Products;
        FString Roster;
        int32 LastDividendYear = 0;
        int32 LifelineBefore = 0, WarningBefore = 0;
        int64 SalaryBefore = 0, DividendBefore = 0;
        int64 PendingDividend = 0;
        double Shelf = 0, List = 0, Purchase = 0, Book = 0, Target = 0;
        int32 StaffCount = 0, Cashiers = 0, Stockers = 0, Hr = 0, Accountant = 0;
        int64 ManagerCost = 0, FamilyManagerCost = 0;
        FString Managers;
        TArray<FMarketManager> ManagersBefore;
    };
    void Policy(FMarketState& State, const TArray<FMarketProduct>& Products, int32 Style, FStats& Stats);
    void BeginDay(const FMarketState& State, const TArray<FMarketProduct>& Products, double PriceFactor, FStats& Stats);
    void Observe(const FMarketState& State, const MarketSimulation::FDay& Day, int64 Ordered, FStats& Stats);
    FString Header();
    FString ProductHeader();
}

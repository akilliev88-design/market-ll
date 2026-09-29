#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Growth of the company beyond L\u00fcleburgaz (G-072, Docs/Kurgu/00_KURGU_KITABI.md \u00a712, karar I02-I05).
// Independent of the world, tested (MirasMarket.Company.*). The family shop is played; the L\u00fcleburgaz branches are
// simulated shelf by shelf (MarketBranches); stores in other cities are aggregate: a store's day is revenue x
// margin minus rent, staff and running costs, following the city, the season, the shoppers' habit (maturity)
// and what the company has built:
//  Chapter 4 Trakya     stores in Babaeski, K\u0131rklareli, \u00c7orlu, Tekirda\u011f, Edirne, Ke\u015fan; a regional depot
//                       (+1.5 % margin, needed for stores far from home) and trucks (one per 8 far stores);
//                       central buying (+2 % margin) from 8 stores.
//  Chapter 5 T\u00fcrkiye    \u0130stanbul, Bursa, \u0130zmir, Ankara, Kocaeli; the "Miras" own brand (+1.5 % margin, a little
//                       more shoppers) from 20 stores; a dark store (online picking depot) for the web shop;
//                       regional managers as head office cost. National share ~0.04 % a store.
//  Chapter 6 Abroad     a pilot in K\u0131rcaali (Bulgaria: many Turkish-speaking families), then Plovdiv or
//                       Constan\u021ba (Romania). The first 90 days of a new country cost margin while the company learns.
//  Chapter 7 Miras      leading on every measure at once (local share, stores, profit, content customers) for a
//                       year gives the "Miras" ending; free play goes on.
// Opening a city store needs the chapter, an HR manager and an accountant (a company, not a shop).
namespace MarketCompany
{
    enum class ECity : uint8
    {
        Babaeski = 0, Kirklareli, Corlu, Tekirdag, Edirne, Kesan,
        IstanbulAvrupa, IstanbulAnadolu, Bursa, Izmir, Ankara, Kocaeli,
        Kircaali, Plovdiv, Kostence,
        Count
    };

    struct FCity
    {
        const TCHAR* Name = TEXT("");
        const TCHAR* Province = TEXT("");
        const TCHAR* Country = TEXT("T\u00fcrkiye");
        int32 Chapter = 4;           // opens with this chapter
        float Income = 1.f;          // wallet size
        float Competition = 1.f;     // rivals' pull (> 1 = crowded market)
        int64 Rent = 150000;         // 2011 kurus a month
        int32 MaxStores = 6;
    };

    constexpr int64 StoreDayRevenue = 250000;   // a mature store, 2011 kurus, ordinary day
    constexpr int32 StoreStaff = 5;
    constexpr int32 MaturityDays = 60;
    constexpr int32 StoresPerTruck = 8;
    constexpr int32 LearningDays = 90;          // a new country
    constexpr int32 LeadershipGoalDays = 365;

    const FCity& CityInfo(ECity City);
    const FMarketCityStores* Find(const FMarketState& State, ECity City);
    // Family shop + open L\u00fcleburgaz branches + city stores.
    int32 TotalStores(const FMarketState& State);
    int32 CountryStores(const FMarketState& State, const TCHAR* Country);
    // Turkish provinces with a store (K\u0131rklareli counts from the family shop).
    int32 Provinces(const FMarketState& State);
    // Percent of Turkish grocery retail.
    float NationalShare(const FMarketState& State);
    bool ChapterOpen(const FMarketState& State, int32 Chapter);
    // Gross margin of a city store today (0..1) and the share of revenue lost to logistics.
    float Margin(const FMarketState& State, ECity City);
    float LogisticsLoss(const FMarketState& State, ECity City);
    // One store's result on a day (kurus), at the given maturity.
    int64 StoreDayProfit(const FMarketState& State, ECity City, float Maturity, int32 GameDay);
    int64 StoreCost(const FMarketState& State, ECity City);

    bool CanOpenStore(const FMarketState& State, ECity City, FString& OutReason);
    bool OpenStore(FMarketState& State, ECity City, FString& OutMessage);
    bool CloseStore(FMarketState& State, ECity City, FString& OutMessage);
    // Arg: 0 depot, 1 truck, 2 central buying, 3 own brand, 4 dark store.
    bool Build(FMarketState& State, int32 What, FString& OutMessage);
    FString Summary(const FMarketState& State);
    // Chapter 7: the measures of leadership held today.
    bool LeadsToday(const FMarketState& State);

    // Day close: every city's stores, head office, depot and trucks, leadership; weekly line.
    void CloseDay(FMarketState& State);
}

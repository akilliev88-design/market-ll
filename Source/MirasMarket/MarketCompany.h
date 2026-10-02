#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"
#include "MarketDepots.h"

// The company around the shops (G-072, rebuilt on provinces in G-086; Docs/Kurgu/03_MAGAZA_AGI.md \u00a76, \u00a78).
// Independent of the world, tested (MirasMarket.Company.*). Every shop beyond the family shop is a branch in a
// province (MarketBranches); the whole country is open from the start. The company builds what a chain needs:
//  - depots in provinces (G-089, MarketDepots: up to 1.5 % on the goods of the branches within 600 km, the road
//    costs 0.6 % per 100 km beyond the first 100; without one a shop outside the home province pays the
//    wholesaler's van, 3 %), trucks (a load per served shop and per 300 km, 8 loads a truck, else up to 1.5 % more),
//  - central buying (+2 %, from 8 shops), the "Miras" own brand (+1.5 % and a few more shoppers, from 20 shops),
//  - (dark stores moved to MarketOnline: one per province, M32).
// Abroad: customs and paperwork 1 %, and the first 90 days in a new country cost 3 % while the company learns.
// Chapter 7 "Miras": a year leading on every measure brings the one finale (karar J02).
namespace MarketCompany
{
    constexpr int32 StoresPerTruck = 8;
    constexpr int32 LearningDays = 90;          // a new country
    constexpr int32 LeadershipGoalDays = 365;

    // Family shop + open branches.
    int32 TotalStores(const FMarketState& State);
    // Open shops in a country (the family shop counts in the campaign's). Country: pack id.
    int32 CountryStores(const FMarketState& State, const FString& Country);
    // Provinces of the campaign's country with a shop of ours (the home province counts from the family shop).
    int32 Provinces(const FMarketState& State);
    // Foreign countries with an open shop.
    int32 ForeignCountries(const FMarketState& State);
    // Percent of the campaign country's grocery retail. B1 (#45): by revenue, not by store count: our revenue in the
    // country a day (smoothed over about a month, State.Ledger.CountryRevenueDay; before the first measured day
    // the last closed day) / what the country's people spend on groceries a day (CountryMarketDay).
    float NationalShare(const FMarketState& State);
    // The country's grocery retail a day at today's prices: people x MarketCountry::FProfile::GroceryPerPersonDay.
    int64 CountryMarketDay(const FMarketState& State);
    // Our revenue of the last closed day in the campaign country (family shop with its online orders + branches there).
    int64 CountryRevenueToday(const FMarketState& State);
    // Day close (after every revenue is booked): moves the smoothed revenue a thirtieth towards the day's.
    void TrackNationalRevenue(FMarketState& State);
    bool ChapterOpen(const FMarketState& State, int32 Chapter);

    // Depots (G-089: MarketDepots, depots in provinces). HasDepot: a depot in a province of the sub-region.
    bool HasDepot(const FMarketState& State, const FString& Country, const FString& SubRegion);
    int32 DepotCount(const FMarketState& State);
    // Shops outside the home province (the trucks carry to them).
    int32 FarStores(const FMarketState& State);
    // Share of a branch's goods cost it pays on top (> 1) or saves (< 1): logistics, depot (rebate, road, trucks),
    // central buying, own brand, a new country. The second form takes the branch's depot link already known
    // (MarketDepots::AllLinks at the day close).
    float CostFactor(const FMarketState& State, const FMarketBranch& Branch);
    float CostFactor(const FMarketState& State, const FMarketBranch& Branch, const MarketDepots::FLink& Link);
    // x shoppers of every branch ("Miras" brand).
    float TrafficBonus(const FMarketState& State);

    // Arg: 1 truck, 2 central buying, 3 own brand (0: a depot in the home sub-region). Dark stores: MarketOnline (M32).
    bool Build(FMarketState& State, int32 What, FString& OutMessage);
    // The old sub-region button (G-086): a depot in the suggested province of a sub-region where the company has a
    // shop (MarketDepots::SuggestDepotProvince limited to it; one per sub-region here). New menus use
    // MarketDepots::Build with a province.
    bool BuildDepot(FMarketState& State, const FString& Country, const FString& SubRegion, FString& OutMessage);
    // A depot's price in the home province today (MarketDepots::BuildCost for any other).
    int64 DepotCost(const FMarketState& State);
    FString Summary(const FMarketState& State);
    // Chapter 7: the measures of leadership held today.
    bool LeadsToday(const FMarketState& State);

    // Day close: head office, depots and trucks, leadership; weekly line.
    void CloseDay(FMarketState& State);
    // C11: the depots' rent and the trucks of a day (MarketFinance budgets them).
    int64 DailyOfficeCost(const FMarketState& State, int32 GameDay);
}

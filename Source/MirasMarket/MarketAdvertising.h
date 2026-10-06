#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// The company's advertising (karar M34, Mustafa 01.10.2026: "reklam verip al\u0131\u015fveri\u015fi art\u0131rabiliriz; reklam
// \u00e7e\u015fitleri olsun, televizyon, sosyal medya, hepsi bir arada"). Independent of the world, tested
// (MirasMarket.Advertising.*). The first store's neighbourhood flyer stays a shop promotion (MarketPromotions).
//  - Per country where we have shops, six channels, each with a budget level 0..3 (a month):
//      TV        national, dear, the strongest and the slowest to fade; worth it for a big network.
//      Radio     cheaper, regional voice, fades faster.
//      Outdoor   billboards in the provinces where we have shops (cost x provinces).
//      Print     newspaper inserts and flyers for every shop (cost x shops); loses power over the years.
//      Social    social media: cheap, grows strong over the years, young shoppers; lifts online orders too.
//      Search    search and app-store ads: only online orders, works at once, gone at once.
//  - What a channel leaves in people's minds is a stock that fades every month at its own speed (TV slowly,
//    print and search fast). The stocks lift every shop's walk-ins in that country (up to +12 %) and the online
//    orders (up to +25 %), with diminishing returns: the first lira works best.
//  - "Hepsi bir arada": three or more of the five brand channels at once work 20 % better, all five 30 %.
//  - Eras: social media and search come with the internet (the epidemic's timeline, MarketOnline); print fades.
//  - From 20 shops an advertising manager can be hired: his skill makes every lira work harder; he can keep the
//    mix himself (a share of the country's revenue, the best channel for the money first) and pushes in the
//    holiday months.
//  - Costs are a head-office expense a day (Marketing); the menu shows each channel's spend and what the ads brought.
namespace MarketAdvertising
{
    enum class EChannel : uint8 { TV = 0, Radio, Outdoor, Print, Social, Search, Count };
    constexpr int32 ChannelCount = static_cast<int32>(EChannel::Count);
    constexpr int32 MaxLevel = 3;
    constexpr int32 ManagerShops = 20;
    constexpr float MaxTraffic = 0.12f;
    constexpr float MaxOnline = 0.25f;

    FString ChannelName(EChannel Channel);
    FString ChannelNote(EChannel Channel);
    // How well a channel works on a day (0 = not there yet; social and search come with the internet).
    float EraFactor(const FMarketState& State, EChannel Channel, int32 GameDay);
    // A month's cost of a channel at a level in a country (start-level kurus x the list level x the reach).
    int64 MonthCost(const FMarketState& State, const FString& Country, EChannel Channel, int32 Level);
    int32 LevelOf(const FMarketState& State, const FString& Country, EChannel Channel);
    // Arg: country index (Countries order, 0 = the campaign's) x 100 + channel x 10 + level.
    int32 Encode(int32 CountryIndex, EChannel Channel, int32 Level);
    bool SetLevel(FMarketState& State, const FString& Country, EChannel Channel, int32 Level, FString& OutMessage);
    bool SetLevelArg(FMarketState& State, int32 Arg, FString& OutMessage);
    // Countries with our shops (the campaign's first).
    TArray<FString> Countries(const FMarketState& State);
    // C11: what the running channels and the advertising manager cost a day at today's levels (MarketFinance budgets it).
    int64 DailySpend(const FMarketState& State);

    // x walk-in shoppers of our shops in a country, x online orders there.
    float TrafficFactor(const FMarketState& State, const FString& Country = FString());
    float OnlineFactor(const FMarketState& State, const FString& Country = FString());
    // Brand channels running (TV, radio, outdoor, print, social) and the together bonus (1, 1.2, 1.3).
    int32 BrandChannels(const FMarketState& State, const FString& Country);
    float Together(const FMarketState& State, const FString& Country);

    // The advertising manager.
    void Candidate(const FMarketState& State, FString& OutName, int32& OutSkill, int64& OutWage);
    bool CanHireManager(const FMarketState& State, FString& OutReason);
    bool HireManager(FMarketState& State, FString& OutMessage);
    bool FireManager(FMarketState& State, FString& OutMessage);
    bool SetAuto(FMarketState& State, bool bAuto, FString& OutMessage);
    bool SetBudget(FMarketState& State, int32 Permille, FString& OutMessage);

    // Menu lines.
    FString CountryLine(const FMarketState& State, const FString& Country);   // spend, lift, last month
    FString ChannelLine(const FMarketState& State, const FString& Country, EChannel Channel);

    // Day close (after the shops' day): the stocks, the day's cost, the uplift estimate, the month turn (fading,
    // the manager's mix, a news line).
    void CloseDay(FMarketState& State);
}

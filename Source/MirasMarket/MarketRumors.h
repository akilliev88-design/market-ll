#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// C13 (karar M43, Mustafa 02.10.2026: "piyasa s\u00f6ylentileri: bu bunu almay\u0131 d\u00fc\u015f\u00fcn\u00fcyor, \u015fu pazara girecek, \u015fu
// sat\u0131\u015fa \u00e7\u0131kacak; 1 g\u00fcvenilir kaynaktan onayland\u0131, 1 g\u00fcvensiz 3 g\u00fcvenilir; bazen g\u00fcvensiz kaynak do\u011fru \u00e7\u0131ks\u0131n,
// bazen 3 g\u00fcvenilir kayna\u011f\u0131n s\u00f6yledi\u011fi ger\u00e7ekle\u015fmesin"). Independent of the world, tested (MirasMarket.Rumors.*).
//  - Once the company has a branch, a rumour starts every 25-45 days (at most MaxActive open) about a rival of a
//    country we are in: it goes for sale, it enters a province of ours, it buys another chain, it starts a price
//    war in a province of ours. Whether it is true is decided at the start and hidden (about 4 in 10 are: 30-60 %
//    by kind and the chain's health).
//  - Sources come in every 4-8 days until two days before its day (at most MaxSources): reliable or not (a
//    country manager there and an accountant bring more reliable ones), confirming or denying. A reliable
//    source is right 74 % of the time, an unreliable one 55 %. The first one is the market talk itself (an
//    unreliable confirmation). So three reliable confirmations are wrong about one time in twenty, and a rumour
//    only unreliable people talk about comes true about four times in ten.
//  - The assistant's reading (zay\u0131f / belirsiz / g\u00fc\u00e7l\u00fc / \u00e7ok g\u00fc\u00e7l\u00fc) is the honest chance from the sources;
//    the player can learn to read it.
//  - On its day a true rumour happens (MarketChains::Force*); a false one is denied. Fun (06 section 2b): time to
//    prepare (money for a chain that may be sold, prices before a war), a surprise now and then.
namespace MarketRumors
{
    enum class EKind : uint8 { ForSale = 0, Enters, Acquires, PriceWar, Count };
    constexpr int32 MaxActive = 3;
    constexpr int32 MaxSources = 5;
    constexpr int32 KeepPast = 6;
    constexpr float ReliableRight = 0.74f;
    constexpr float UnreliableRight = 0.55f;
    constexpr float Prior = 0.42f;             // what the assistant assumes before any source (the real share)

    // Chance (0..1) the sources make it true, from Prior and every source.
    float Belief(const FMarketRumor& Rumor);
    FString BeliefName(float Belief);          // "zay\u0131f", "belirsiz", "g\u00fc\u00e7l\u00fc", "\u00e7ok g\u00fc\u00e7l\u00fc"
    // "<zincir>'in sat\u0131\u015fa \u00e7\u0131kaca\u011f\u0131 konu\u015fuluyor"
    FString Headline(const FMarketState& State, const FMarketRumor& Rumor);
    // "2 g\u00fcvenilir do\u011fruluyor, 1 g\u00fcvensiz yalanl\u0131yor"
    FString SourcesText(const FMarketRumor& Rumor);
    // One menu line: headline, sources, the assistant's reading, about when.
    FString Describe(const FMarketState& State, const FMarketRumor& Rumor);
    FString DescribePast(const FMarketState& State, const FMarketRumor& Rumor);

    // A source for a rumour now (seeded by the rumour and its source count).
    void AddSource(FMarketState& State, FMarketRumor& Rumor);
    // Starts a rumour of a kind about a chain (tests and the generator); INDEX_NONE when it cannot.
    int32 Start(FMarketState& State, EKind Kind, int32 ChainIndex, int32 OtherIndex, const FString& Province, bool bTrue, int32 DueDay);
    // Day close (after the chains' turn): sources, rumours whose day came, a new one now and then.
    void CloseDay(FMarketState& State);
}

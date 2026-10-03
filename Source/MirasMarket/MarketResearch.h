#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// M58 (Mustafa 03.10.2026): before the first store in a new country (or buying a chain there) the company needs a
// valid market study of it. It costs money (half to twice a neighbourhood store's opening, by the country's size),
// takes 30-45 days, and its report names the most attractive provinces, the chains' shares and the store types
// the country knows (M54). A study stays valid for a year; a country not entered by then needs a new one. Not
// needed at home nor in a country where we already have a company (M65). Independent of the world, tested
// (MirasMarket.Expansion.*).
namespace MarketResearch
{
    constexpr int32 ValidDays = 365;
    enum class EStatus : uint8 { NotNeeded = 0, None, Running, Ready, Expired };

    EStatus Status(const FMarketState& State, const FString& Country);
    // Entering Country is allowed (no study needed, or a ready one); OutReason says what is missing.
    bool AllowsEntry(const FMarketState& State, const FString& Country, FString& OutReason);
    // What a study costs today (our money) and how many days it takes.
    int64 Cost(const FMarketState& State, const FString& Country);
    int32 Days(const FMarketState& State, const FString& Country);
    bool CanStart(const FMarketState& State, const FString& Country, FString& OutReason);
    // Orders the study: its cost is paid at the day close (head office), the report comes in Days.
    bool Start(FMarketState& State, const FString& Country, FString& OutMessage);
    // The report (empty until it is ready): best provinces, the chains' shares, the store types.
    FString Report(const FMarketState& State, const FString& Country);
    // One line for the menu ("Pazar ara\u015ft\u0131rmas\u0131: 12 g\u00fcn kald\u0131" ...).
    FString StatusText(const FMarketState& State, const FString& Country);
    // Day close: a study ready today brings its report to the news.
    void CloseDay(FMarketState& State);
}

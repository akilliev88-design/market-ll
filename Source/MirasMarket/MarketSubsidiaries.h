#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// M65 (Mustafa 03.10.2026): our company is one brand and a company in every country it works in. The campaign's own
// country holds the parent company; entering another country founds a subsidiary there with a registered name
// (the brand + one of the country's legal forms, Config/ulkeler.json "company"; the player may write any name),
// a founding cost and its own accounts (its stores' statements). At every month's close a subsidiary's profit goes
// to the parent; the country takes its withholding tax on the way.
// Names: everybody says the brand (signs, news, the world league); the registered name shows where the law speaks:
// the company page, a country's own retailer table, the books. Rivals work the same way (ChainLegalName).
// Independent of the world, tested (MirasMarket.Subsidiaries.*).
namespace MarketSubsidiaries
{
    constexpr int32 MaxNameLength = 60;

    // The brand: the short name everybody uses ("Miras").
    FString Brand(const FMarketState& State);
    bool SetBrand(FMarketState& State, const FString& Name, FString& OutMessage);
    // The registered names the country suggests: the brand + each legal form (the first is the default).
    TArray<FString> SuggestedNames(const FMarketState& State, const FString& Country);
    FString DefaultLegalName(const FMarketState& State, const FString& Country);
    const FMarketSubsidiary* Find(const FMarketState& State, const FString& Country);
    // Our registered name in Country (the subsidiary's, else what it would be called).
    FString LegalName(const FMarketState& State, const FString& Country);
    bool SetLegalName(FMarketState& State, const FString& Country, const FString& Name, FString& OutMessage);
    // The next suggestion after the current name (the menu's "another name" button).
    FString NextSuggestion(const FMarketState& State, const FString& Country);
    // What founding a company in Country costs today, our money (0 at home and when it already exists).
    int64 SetupCost(const FMarketState& State, const FString& Country);
    // Founds the company of Country if it has none: the parent at home (no cost), a subsidiary abroad (bPay: its
    // founding cost leaves the till at the day close, booked to the head office). Returns true when founded now.
    bool Ensure(FMarketState& State, const FString& Country, bool bPay = true);
    // A rival's registered name: its name + a legal form of its country (stable for the chain).
    FString ChainLegalName(const FMarketChain& Chain);
    // Day close: on the first day of a month every subsidiary's last month (its stores' net result, our money) goes
    // to the parent; the country's withholding tax on a profit is paid (EAccount::Withholding).
    void CloseDay(FMarketState& State);
}

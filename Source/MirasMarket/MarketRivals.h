#pragma once
#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Rival shops in the district (G-054, replaces the fixed "rival discount 4 days of 5", G-008).
// Independent of the world (test: MirasMarket.Rivals.News). The rivals are the real chains that were in
// Luleburgaz in 2011 (Mustafa's decision: real chains, real history). Every evening the day report brings the
// news of what a rival does tomorrow: a discount on one aisle, a weekend sale, a price rise, an empty aisle,
// longer opening hours; from day 15 a new discount store opens in the district. The news changes the rival's
// price for the products of that aisle (shoppers compare our shelf price with it, MarketDemand) and the number
// of shoppers. Everything follows from the day and the campaign's RivalSeed: a reload cannot reroll it.
// Logos: RivalLogoKey names the folder Content/Brands/<key>/logo.png that the player can fill (menu, later).
// Later (bigger company) a hired manager can answer these campaigns; for now the player does.
// The national chain growth (real history, market shares) is a separate, later system; this is the district.
namespace MarketRivals
{
    enum class EKind : uint8 { AisleSale, WeekendSale, PriceRise, OutOfStock, LongerHours, NewRival };

    struct FEvent
    {
        EKind Kind = EKind::AisleSale;
        int32 Rival = 0;          // index into RivalName
        FString Category;         // aisle (product category); empty = every product
        float PriceFactor = 1.f;  // rival price = list price x factor
        float Traffic = 1.f;      // our shoppers x factor
        int32 FirstDay = 0;
        int32 Days = 1;           // active on FirstDay .. FirstDay + Days - 1
    };

    constexpr int32 QuietDays = 2;          // days 1-2: no news while the player learns the shop
    constexpr int32 ChainOpensDay = 15;     // a new discount store opens in the district
    constexpr int32 NewsChancePercent = 55;

    int32 RivalCount(int32 Day);
    FString RivalName(int32 Rival);
    // Folder name under Content/Brands for the rival's logo (ASCII, lower case).
    FString RivalLogoKey(int32 Rival);
    // Distinct product categories (aisles), sorted: the aisles a rival can target.
    TArray<FString> Aisles(const TArray<FMarketProduct>& Products);
    // News that starts on Day (the evening report shows the next day's news).
    TArray<FEvent> NewsOn(int32 Day, int32 Seed, const TArray<FString>& AisleList);
    // Events in effect on Day (started in the last few days and not over yet, plus the new store once it is open).
    TArray<FEvent> ActiveOn(int32 Day, int32 Seed, const TArray<FString>& AisleList);
    // Rival price factor for one product category on Day (all active events multiplied, 0.7..1.3).
    float PriceFactor(int32 Day, int32 Seed, const TArray<FString>& AisleList, const FString& Category);
    // Our shopper traffic factor on Day (0.8..1.0).
    float TrafficFactor(int32 Day, int32 Seed, const TArray<FString>& AisleList);
    // G-059 menu: price factor of one rival for one aisle on Day, counting only that rival's events (the
    // shoppers still compare with PriceFactor). bOutEmpty = that rival's aisle is empty today.
    float RivalFactor(int32 Day, int32 Seed, const TArray<FString>& AisleList, const FString& Category, int32 Rival, bool* bOutEmpty = nullptr);
    // Store format of a rival for the menu (Turkish), e.g. "indirim marketi".
    FString RivalFormat(int32 Rival);
    // One line of news in Turkish, e.g. "BIM: sut reyonunda %20 indirim (2 gun)".
    FString Describe(const FEvent& Event);
}

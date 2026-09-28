#include "MarketRivals.h"

namespace MarketRivalsRules
{
    // Stable 32-bit hash of (seed, day, salt): the same campaign day always brings the same news.
    uint32 Mix(int32 Seed, int32 Day, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), static_cast<uint32>(Day), Salt };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }
    constexpr int32 LongestEvent = 3; // days; ActiveOn looks back this far
}

int32 MarketRivals::RivalCount(int32 Day)
{
    return Day >= ChainOpensDay ? 3 : 2;
}

FString MarketRivals::RivalName(int32 Rival)
{
    switch (Rival)
    {
    case 0: return TEXT("B\u0130M");
    case 1: return TEXT("Migros");
    default: return TEXT("A101");
    }
}

FString MarketRivals::RivalLogoKey(int32 Rival)
{
    switch (Rival)
    {
    case 0: return TEXT("bim");
    case 1: return TEXT("migros");
    default: return TEXT("a101");
    }
}

TArray<FString> MarketRivals::Aisles(const TArray<FMarketProduct>& Products)
{
    TArray<FString> Result;
    for (const FMarketProduct& Product : Products)
        if (!Product.Category.IsEmpty()) Result.AddUnique(Product.Category);
    Result.Sort();
    return Result;
}

TArray<MarketRivals::FEvent> MarketRivals::NewsOn(int32 Day, int32 Seed, const TArray<FString>& AisleList)
{
    TArray<FEvent> News;
    if (Day == ChainOpensDay)
    {
        FEvent Chain;
        Chain.Kind = EKind::NewRival; Chain.Rival = 2; Chain.Traffic = 0.95f; Chain.FirstDay = Day; Chain.Days = 1000000;
        News.Add(Chain);
    }
    if (Day <= QuietDays) return News;
    if (static_cast<int32>(MarketRivalsRules::Mix(Seed, Day, 1u) % 100u) >= NewsChancePercent) return News;
    FEvent Event;
    Event.FirstDay = Day;
    Event.Rival = static_cast<int32>(MarketRivalsRules::Mix(Seed, Day, 2u) % static_cast<uint32>(RivalCount(Day)));
    const int32 Roll = static_cast<int32>(MarketRivalsRules::Mix(Seed, Day, 3u) % 100u);
    const uint32 Pick = MarketRivalsRules::Mix(Seed, Day, 4u);
    const FString Aisle = AisleList.Num() > 0 ? AisleList[Pick % static_cast<uint32>(AisleList.Num())] : FString();
    const bool bWeekend = Day % 7 == 6 || Day % 7 == 0; // days 6-7 of every week
    if (Roll < 50 && bWeekend && Roll >= 35)
    {
        Event.Kind = EKind::WeekendSale; Event.PriceFactor = 0.9f; Event.Days = 2;
    }
    else if (Roll < 50 && !Aisle.IsEmpty())
    {
        static const float Cuts[3] = { 0.80f, 0.85f, 0.90f };
        Event.Kind = EKind::AisleSale; Event.Category = Aisle; Event.PriceFactor = Cuts[(Pick / 7u) % 3u]; Event.Days = 2 + static_cast<int32>((Pick / 21u) % 2u);
    }
    else if (Roll < 65 && !Aisle.IsEmpty())
    {
        Event.Kind = EKind::PriceRise; Event.Category = Aisle; Event.PriceFactor = 1.10f; Event.Days = 3;
    }
    else if (Roll < 80 && !Aisle.IsEmpty())
    {
        Event.Kind = EKind::OutOfStock; Event.Category = Aisle; Event.PriceFactor = 1.25f; Event.Days = 2;
    }
    else if (Roll < 90)
    {
        Event.Kind = EKind::LongerHours; Event.Traffic = 0.90f; Event.Days = 3;
    }
    else return News;
    News.Add(Event);
    return News;
}

TArray<MarketRivals::FEvent> MarketRivals::ActiveOn(int32 Day, int32 Seed, const TArray<FString>& AisleList)
{
    TArray<FEvent> Active;
    if (Day >= ChainOpensDay)
        for (const FEvent& Event : NewsOn(ChainOpensDay, Seed, AisleList))
            if (Event.Kind == EKind::NewRival) Active.Add(Event);
    for (int32 Start = FMath::Max(1, Day - MarketRivalsRules::LongestEvent + 1); Start <= Day; ++Start)
        for (const FEvent& Event : NewsOn(Start, Seed, AisleList))
            if (Event.Kind != EKind::NewRival && Day < Event.FirstDay + Event.Days) Active.Add(Event);
    return Active;
}

float MarketRivals::PriceFactor(int32 Day, int32 Seed, const TArray<FString>& AisleList, const FString& Category)
{
    float Factor = 1.f;
    for (const FEvent& Event : ActiveOn(Day, Seed, AisleList))
        if (Event.Category.IsEmpty() || Event.Category == Category) Factor *= Event.PriceFactor;
    return FMath::Clamp(Factor, 0.7f, 1.3f);
}

float MarketRivals::TrafficFactor(int32 Day, int32 Seed, const TArray<FString>& AisleList)
{
    float Factor = 1.f;
    for (const FEvent& Event : ActiveOn(Day, Seed, AisleList)) Factor *= Event.Traffic;
    return FMath::Clamp(Factor, 0.8f, 1.f);
}

float MarketRivals::RivalFactor(int32 Day, int32 Seed, const TArray<FString>& AisleList, const FString& Category, int32 Rival, bool* bOutEmpty)
{
    float Factor = 1.f;
    bool bEmpty = false;
    for (const FEvent& Event : ActiveOn(Day, Seed, AisleList))
    {
        if (Event.Rival != Rival || !(Event.Category.IsEmpty() || Event.Category == Category)) continue;
        if (Event.Kind == EKind::OutOfStock) bEmpty = true;
        else Factor *= Event.PriceFactor;
    }
    if (bOutEmpty) *bOutEmpty = bEmpty;
    return FMath::Clamp(Factor, 0.7f, 1.3f);
}

FString MarketRivals::RivalFormat(int32 Rival)
{
    return Rival == 1 ? FString(TEXT("s\u00fcpermarket")) : FString(TEXT("indirim marketi"));
}

FString MarketRivals::Describe(const FEvent& Event)
{
    const FString Shop = RivalName(Event.Rival);
    const int32 Percent = FMath::RoundToInt32(FMath::Abs(1.f - Event.PriceFactor) * 100.f);
    switch (Event.Kind)
    {
    case EKind::AisleSale:
        return FString::Printf(TEXT("%s: %s reyonunda %%%d indirim (%d g\u00fcn). O reyonda fiyat\u0131na bak."), *Shop, *Event.Category, Percent, Event.Days);
    case EKind::WeekendSale:
        return FString::Printf(TEXT("%s: hafta sonu b\u00fct\u00fcn \u00fcr\u00fcnlerde %%%d indirim (%d g\u00fcn)."), *Shop, Percent, Event.Days);
    case EKind::PriceRise:
        return FString::Printf(TEXT("%s: %s fiyatlar\u0131n\u0131 %%%d art\u0131rd\u0131 (%d g\u00fcn). Senin i\u00e7in f\u0131rsat."), *Shop, *Event.Category, Percent, Event.Days);
    case EKind::OutOfStock:
        return FString::Printf(TEXT("%s: %s reyonu bo\u015f kald\u0131 (%d g\u00fcn). M\u00fc\u015fteriler sende arayacak; stok haz\u0131r olsun."), *Shop, *Event.Category, Event.Days);
    case EKind::LongerHours:
        return FString::Printf(TEXT("%s: art\u0131k daha erken a\u00e7\u0131l\u0131p ge\u00e7 kapan\u0131yor (%d g\u00fcn). M\u00fc\u015fteri biraz azalabilir."), *Shop, Event.Days);
    case EKind::NewRival:
        return FString::Printf(TEXT("\u0130l\u00e7eye yeni bir %s a\u00e7\u0131ld\u0131. M\u00fc\u015fterilerin bir k\u0131sm\u0131 oraya da u\u011frayacak."), *Shop);
    }
    return FString();
}

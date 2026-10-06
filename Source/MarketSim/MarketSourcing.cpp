#include "MarketSourcing.h"
#include "MarketBranches.h"
#include "MarketCompany.h"
#include "MarketCountry.h"
#include "MarketDepots.h"
#include "MarketGoods.h"
#include "MarketPrices.h"

namespace MarketSourcingLocal
{
    using MarketSourcing::ELine;
    using MarketSourcing::ETier;

    void Fit(FMarketState& State)
    {
        FMarketSourcingState& S = State.Sourcing;
        if (S.Tiers.Num() != MarketSourcing::LineCount) S.Tiers.SetNumZeroed(MarketSourcing::LineCount);
        if (S.Missed.Num() != MarketSourcing::LineCount) S.Missed.SetNumZeroed(MarketSourcing::LineCount);
        if (S.MonthBuy.Num() != MarketSourcing::LineCount) S.MonthBuy.SetNumZeroed(MarketSourcing::LineCount);
        if (S.LastMonthDay == 0) S.LastMonthDay = State.Day;
    }

    int32 Shops(const FMarketState& State)
    {
        int32 Count = 1; // the first store
        for (const FMarketBranch& B : State.Branches) if (B.Stage == static_cast<uint8>(MarketBranches::EStage::Open)) ++Count;
        return Count;
    }

    int32 ShopsNeeded(ETier Tier)
    {
        return Tier == ETier::Regional ? 3 : Tier == ETier::National ? 12 : Tier == ETier::Producer ? 30 : 0;
    }

    FString Percent(float Value) { return FString::Printf(TEXT("%%%.1f"), Value * 100.f); }
}

FString MarketSourcing::LineName(ELine Line)
{
    switch (Line)
    {
    case ELine::Drinks: return TEXT("\u0130\u00e7ecek");
    case ELine::Dairy: return TEXT("S\u00fct \u00fcr\u00fcnleri");
    case ELine::DryFood: return TEXT("Kuru g\u0131da");
    default: return TEXT("Temizlik ve bak\u0131m");
    }
}

FString MarketSourcing::TierName(ETier Tier)
{
    switch (Tier)
    {
    case ETier::Regional: return TEXT("B\u00f6lge distrib\u00fct\u00f6r\u00fc");
    case ETier::National: return TEXT("Ulusal distrib\u00fct\u00f6r");
    case ETier::Producer: return TEXT("\u00dcreticiden do\u011frudan");
    default: return TEXT("Yerel toptanc\u0131");
    }
}

MarketSourcing::ELine MarketSourcing::LineOf(const FString& Category)
{
    switch (MarketGoods::Classify(Category))
    {
    case MarketGoods::EGroup::Drinks: return ELine::Drinks;
    case MarketGoods::EGroup::Dairy:
    case MarketGoods::EGroup::IceCream: return ELine::Dairy;
    case MarketGoods::EGroup::Household:
    case MarketGoods::EGroup::PersonalCare:
    case MarketGoods::EGroup::Paper: return ELine::Household;
    default: return ELine::DryFood;
    }
}

float MarketSourcing::TierDiscount(ETier Tier)
{
    return Tier == ETier::Regional ? 0.025f : Tier == ETier::National ? 0.045f : Tier == ETier::Producer ? 0.07f : 0.f;
}

int64 MarketSourcing::TierMinimum(ETier Tier)
{
    return Tier == ETier::Regional ? 1500000 : Tier == ETier::National ? 6000000 : Tier == ETier::Producer ? 20000000 : 0;
}

MarketSourcing::ETier MarketSourcing::TierOf(const FMarketState& State, ELine Line)
{
    const int32 I = static_cast<int32>(Line);
    return State.Sourcing.Tiers.IsValidIndex(I) ? static_cast<ETier>(FMath::Min<int32>(State.Sourcing.Tiers[I], TierCount - 1)) : ETier::Local;
}

bool MarketSourcing::CanSet(const FMarketState& State, ELine Line, ETier Tier, FString& OutReason)
{
    if (static_cast<int32>(Line) >= LineCount || static_cast<int32>(Tier) >= TierCount) { OutReason = TEXT("B\u00f6yle bir se\u00e7enek yok."); return false; }
    if (TierOf(State, Line) == Tier) { OutReason = FString::Printf(TEXT("%s zaten %s ile \u00e7al\u0131\u015f\u0131yor."), *LineName(Line), *TierName(Tier)); return false; }
    if (static_cast<int32>(Tier) < static_cast<int32>(TierOf(State, Line))) return true; // stepping down is always possible
    const int32 Need = MarketSourcingLocal::ShopsNeeded(Tier);
    if (MarketSourcingLocal::Shops(State) < Need) { OutReason = FString::Printf(TEXT("%s en az %d ma\u011fazayla \u00e7al\u0131\u015f\u0131r."), *TierName(Tier), Need); return false; }
    if (Tier >= ETier::National && MarketDepots::Count(State) == 0) { OutReason = FString::Printf(TEXT("%s mal\u0131 depoya getirir: \u00f6nce bir depo kur."), *TierName(Tier)); return false; }
    if (Tier == ETier::Producer && !State.Company.bCentralBuying) { OutReason = TEXT("\u00dcreticiyle pazarl\u0131k i\u00e7in merkezi sat\u0131n alma gerekir."); return false; }
    return true;
}

bool MarketSourcing::Set(FMarketState& State, ELine Line, ETier Tier, FString& OutMessage)
{
    if (!CanSet(State, Line, Tier, OutMessage)) return false;
    MarketSourcingLocal::Fit(State);
    const int32 I = static_cast<int32>(Line);
    State.Sourcing.Tiers[I] = static_cast<uint8>(Tier);
    State.Sourcing.Missed[I] = 0;
    const int64 Minimum = FMath::RoundToInt64(TierMinimum(Tier) * MarketPrices::ListLevel(State.Day));
    OutMessage = FString::Printf(TEXT("%s art\u0131k %s ile: al\u0131\u015f fiyatlar\u0131 %s daha ucuz%s."), *LineName(Line), *TierName(Tier), *MarketSourcingLocal::Percent(TierDiscount(Tier)),
        Minimum > 0 ? *FString::Printf(TEXT(", ayda en az %s al\u0131m ister"), *MarketCountry::Money(Minimum)) : TEXT(""));
    return true;
}

int32 MarketSourcing::Encode(ELine Line, ETier Tier) { return static_cast<int32>(Line) * 10 + static_cast<int32>(Tier); }

bool MarketSourcing::Decode(int32 Arg, ELine& OutLine, ETier& OutTier)
{
    if (Arg < 0 || Arg / 10 >= LineCount || Arg % 10 >= TierCount) return false;
    OutLine = static_cast<ELine>(Arg / 10);
    OutTier = static_cast<ETier>(Arg % 10);
    return true;
}

float MarketSourcing::VolumeDiscount(const FMarketState& State)
{
    int64 Volume = 0;
    for (const FMarketSupplierAccount& A : State.SupplierAccounts) Volume += A.Volume30;
    const double Base = FMath::Max(1.0, BaseVolumeStart * MarketPrices::ListLevel(State.Day));
    if (Volume <= Base) return 0.f;
    return FMath::Clamp(0.03f * static_cast<float>(FMath::Log2(static_cast<double>(Volume) / Base)), 0.f, MaxVolumeDiscount);
}

float MarketSourcing::CostFactor(const FMarketState& State, const FString& Category)
{
    return (1.f - TierDiscount(TierOf(State, LineOf(Category)))) * (1.f - VolumeDiscount(State));
}

float MarketSourcing::DairySpoilFactor(const FMarketState& State)
{
    return TierOf(State, ELine::Dairy) != ETier::Local ? 0.9f : 1.f;
}

void MarketSourcing::RecordPurchase(FMarketState& State, const FString& Category, int64 Cost)
{
    if (Cost <= 0) return;
    MarketSourcingLocal::Fit(State);
    State.Sourcing.MonthBuy[static_cast<int32>(LineOf(Category))] += Cost;
}

FString MarketSourcing::Describe(const FMarketState& State, ELine Line)
{
    const ETier Tier = TierOf(State, Line);
    const int32 I = static_cast<int32>(Line);
    const int64 Bought = State.Sourcing.MonthBuy.IsValidIndex(I) ? State.Sourcing.MonthBuy[I] : 0;
    const int64 Minimum = FMath::RoundToInt64(TierMinimum(Tier) * MarketPrices::ListLevel(State.Day));
    FString Line1 = FString::Printf(TEXT("%s \u00b7 %s \u00b7 %s ucuz"), *LineName(Line), *TierName(Tier), *MarketSourcingLocal::Percent(TierDiscount(Tier)));
    if (Minimum > 0) Line1 += FString::Printf(TEXT(" \u00b7 bu ay %s / en az %s"), *MarketCountry::Money(Bought), *MarketCountry::Money(Minimum));
    return Line1;
}

FString MarketSourcing::NextStep(const FMarketState& State, ELine Line)
{
    const int32 Next = static_cast<int32>(TierOf(State, Line)) + 1;
    if (Next >= TierCount) return FString();
    FString Why;
    if (!CanSet(State, Line, static_cast<ETier>(Next), Why)) return Why;
    return FString::Printf(TEXT("%s ile \u00e7al\u0131\u015fabilirsin: %s ucuz."), *TierName(static_cast<ETier>(Next)), *MarketSourcingLocal::Percent(TierDiscount(static_cast<ETier>(Next))));
}

void MarketSourcing::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    MarketSourcingLocal::Fit(State);
    // The first store's day: what it sold is what it buys again.
    for (int32 I = 0; I < Products.Num() && I < State.Stock.Num(); ++I)
        RecordPurchase(State, Products[I].Category, static_cast<int64>(State.Stock[I].Yesterday.Sold) * Products[I].Cost);
    FMarketSourcingState& S = State.Sourcing;
    if (State.Day - S.LastMonthDay < 30) return;
    S.LastMonthDay = State.Day;
    const double Level = MarketPrices::ListLevel(State.Day);
    for (int32 I = 0; I < LineCount; ++I)
    {
        const ETier Tier = static_cast<ETier>(FMath::Min<int32>(S.Tiers[I], TierCount - 1));
        const int64 Minimum = FMath::RoundToInt64(TierMinimum(Tier) * Level);
        if (Minimum > 0 && S.MonthBuy[I] < Minimum * 7 / 10)
        {
            ++S.Missed[I];
            if (S.Missed[I] >= 2)
            {
                S.Tiers[I] = static_cast<uint8>(static_cast<int32>(Tier) - 1);
                S.Missed[I] = 0;
                State.DayNews.Add(FString::Printf(TEXT("%s: %s seni b\u0131rakt\u0131 (iki ay \u00fcst \u00fcste az al\u0131m). Art\u0131k %s ile."), *LineName(static_cast<ELine>(I)), *TierName(Tier),
                    *TierName(static_cast<ETier>(S.Tiers[I]))));
            }
            else State.DayNews.Add(FString::Printf(TEXT("%s: bu ay %s al\u0131m yapt\u0131n, %s en az %s istiyor. Bir ay daha b\u00f6yle giderse seni b\u0131rak\u0131r."),
                *LineName(static_cast<ELine>(I)), *MarketCountry::Money(S.MonthBuy[I]), *TierName(Tier), *MarketCountry::Money(Minimum)));
        }
        else S.Missed[I] = 0;
        S.MonthBuy[I] = 0;
    }
}

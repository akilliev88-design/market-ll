#include "MarketStoreAssign.h"
#include "MarketBranches.h"
#include "MarketGoods.h"
#include "MarketPrices.h"

namespace MarketStoreAssignLocal
{
    FString Part(const FString& Text)
    {
        return MarketGoods::Fold(Text.TrimStartAndEnd());
    }

    // Unknown formats count as mahalle (as MarketBranches::FormatInfo does).
    FString FormatKey(const FString& Format)
    {
        const FString Key = Part(Format);
        return (Key == TEXT("kucuk") || Key == TEXT("buyuk") || Key == TEXT("hiper")) ? Key : FString(TEXT("mahalle"));
    }

    float Ratio(float Value, float NominalValue)
    {
        return NominalValue > 0.f ? Value / NominalValue : 1.f;
    }

    float Lanes(const MarketStoreAssign::FStoreMeasures& Stats)
    {
        return static_cast<float>(Stats.Checkouts) + 0.5f * static_cast<float>(Stats.SelfCheckouts);
    }

    float StaffedLanes(const MarketStoreAssign::FStoreMeasures& Stats)
    {
        return static_cast<float>(Stats.Checkouts) + 0.25f * static_cast<float>(Stats.SelfCheckouts);
    }

    float FreshRoom(const MarketStoreAssign::FStoreMeasures& Stats)
    {
        return Stats.CoolerM + Stats.FreezerM + 0.5f * Stats.ProduceM2 + 3.f * static_cast<float>(Stats.Counters);
    }
}

FString MarketStoreAssign::SiteKey(const FString& Country, const FString& Province, const FString& Format)
{
    return MarketStoreAssignLocal::Part(Country) + TEXT("|") + MarketStoreAssignLocal::Part(Province) + TEXT("|") + MarketStoreAssignLocal::Part(Format);
}

uint32 MarketStoreAssign::StableHash(const FString& Text)
{
    uint32 Hash = 2166136261u;
    for (const TCHAR C : Text)
    {
        const uint32 Unit = static_cast<uint32>(C) & 0xFFFFu;
        Hash ^= Unit & 0xFFu;
        Hash *= 16777619u;
        Hash ^= Unit >> 8;
        Hash *= 16777619u;
    }
    // Final mix (murmur3 fmix32) so that keys differing in the last letter spread over the whole range.
    Hash ^= Hash >> 16;
    Hash *= 0x85EBCA6Bu;
    Hash ^= Hash >> 13;
    Hash *= 0xC2B2AE35u;
    Hash ^= Hash >> 16;
    return Hash;
}

FString MarketStoreAssign::Pick(const FString& Country, const FString& Province, const FString& Format, const TArray<FString>& TemplateIds)
{
    const FString Key = SiteKey(Country, Province, Format);
    FString Best;
    uint32 BestScore = 0;
    bool bAny = false;
    for (const FString& Id : TemplateIds)
    {
        if (Id.IsEmpty()) continue;
        const uint32 Score = StableHash(Key + TEXT("#") + Id);
        if (!bAny || Score > BestScore || (Score == BestScore && Id.Compare(Best, ESearchCase::CaseSensitive) < 0))
        {
            Best = Id;
            BestScore = Score;
            bAny = true;
        }
    }
    return Best;
}

FString MarketStoreAssign::Resolve(const TMap<FString, FString>& Saved, const FString& Country, const FString& Province, const FString& Format, const TArray<FString>& TemplateIds)
{
    if (const FString* Kept = Saved.Find(SiteKey(Country, Province, Format)))
    {
        if (!Kept->IsEmpty() && TemplateIds.Contains(*Kept)) return *Kept;
    }
    return Pick(Country, Province, Format, TemplateIds);
}

FString MarketStoreAssign::Assign(TMap<FString, FString>& Saved, const FString& Country, const FString& Province, const FString& Format, const TArray<FString>& TemplateIds)
{
    const FString Key = SiteKey(Country, Province, Format);
    if (TemplateIds.Num() == 0)
    {
        const FString* Kept = Saved.Find(Key);
        return Kept ? *Kept : FString();
    }
    const FString Chosen = Resolve(Saved, Country, Province, Format, TemplateIds);
    if (!Chosen.IsEmpty()) Saved.Add(Key, Chosen);
    return Chosen;
}

FString MarketStoreAssign::FaceKey(const FString& FixtureId, bool bBack)
{
    return bBack ? FixtureId + TEXT("#back") : FixtureId;
}

bool MarketStoreAssign::IsUncategorized(const FString& Category)
{
    const FString Key = MarketStoreAssignLocal::Part(Category);
    return Key.IsEmpty() || Key == TEXT("kategorisiz");
}

void MarketStoreAssign::SetCategory(TMap<FString, FString>& Choices, const FString& FaceId, const FString& Category)
{
    if (FaceId.IsEmpty()) return;
    Choices.Add(FaceId, IsUncategorized(Category) ? FString(Uncategorized) : Category.TrimStartAndEnd());
}

void MarketStoreAssign::ClearCategory(TMap<FString, FString>& Choices, const FString& FaceId)
{
    Choices.Remove(FaceId);
}

FString MarketStoreAssign::GetCategory(const TMap<FString, FString>& Choices, const FString& FaceId, const FString& TemplateCategory)
{
    if (const FString* Chosen = Choices.Find(FaceId)) return *Chosen;
    return TemplateCategory;
}

bool MarketStoreAssign::CategoryMatches(const FString& FixtureCategory, const FString& ProductCategory)
{
    if (IsUncategorized(FixtureCategory)) return false;
    return MarketStoreAssignLocal::Part(FixtureCategory) == MarketStoreAssignLocal::Part(ProductCategory);
}

MarketStoreAssign::FStoreMeasures MarketStoreAssign::Nominal(const FString& Format)
{
    const FString Key = MarketStoreAssignLocal::FormatKey(Format);
    FStoreMeasures S;
    if (Key == TEXT("kucuk"))
    {
        // 250-400 m2, 2-3 tills, 60-110 m shelf, 8-16 m cold
        S.SalesAreaM2 = 325.f; S.Checkouts = 2; S.ShelfFrontM = 85.f; S.CoolerM = 9.f; S.FreezerM = 3.f;
        S.ProduceM2 = 4.f; S.Counters = 0; S.BackroomPallets = 12;
    }
    else if (Key == TEXT("buyuk"))
    {
        // 600-1500 m2, 4-8 tills, 180-400 m shelf, 25-60 m cold
        S.SalesAreaM2 = 1050.f; S.Checkouts = 6; S.ShelfFrontM = 290.f; S.CoolerM = 30.f; S.FreezerM = 12.5f;
        S.ProduceM2 = 30.f; S.Counters = 2; S.BackroomPallets = 30;
    }
    else if (Key == TEXT("hiper"))
    {
        // 3000-6000 m2, 15-30 tills, 700-1500 m shelf, 100-200 m cold
        S.SalesAreaM2 = 4500.f; S.Checkouts = 22; S.ShelfFrontM = 1100.f; S.CoolerM = 100.f; S.FreezerM = 50.f;
        S.ProduceM2 = 120.f; S.Counters = 5; S.BackroomPallets = 120;
    }
    else
    {
        // mahalle: 80-200 m2, 1-2 tills, 25-60 m shelf, 4-10 m cold
        S.SalesAreaM2 = 140.f; S.Checkouts = 1; S.ShelfFrontM = 42.5f; S.CoolerM = 5.25f; S.FreezerM = 1.75f;
        S.ProduceM2 = 3.5f; S.Counters = 0; S.BackroomPallets = 6;
    }
    S.SelfCheckouts = 0;
    return S;
}

MarketStoreAssign::FStoreMeasures MarketStoreAssign::Normalized(const FStoreMeasures& Stats, const FString& Format)
{
    const FStoreMeasures Base = Nominal(Format);
    FStoreMeasures Out = Stats;
    if (Out.SalesAreaM2 <= 0.f) Out.SalesAreaM2 = Base.SalesAreaM2;
    if (Out.ShelfFrontM <= 0.f) Out.ShelfFrontM = Base.ShelfFrontM;
    if (Out.Checkouts <= 0 && Out.SelfCheckouts <= 0) Out.Checkouts = Base.Checkouts;
    if (Out.BackroomPallets <= 0) Out.BackroomPallets = Base.BackroomPallets;
    // No cold room and no produce at all: the stats block was not given (every format has a dairy cooler).
    if (Out.CoolerM <= 0.f && Out.FreezerM <= 0.f && Out.ProduceM2 <= 0.f && Out.Counters <= 0)
    {
        Out.CoolerM = Base.CoolerM; Out.FreezerM = Base.FreezerM; Out.ProduceM2 = Base.ProduceM2; Out.Counters = Base.Counters;
    }
    Out.CoolerM = FMath::Max(0.f, Out.CoolerM);
    Out.FreezerM = FMath::Max(0.f, Out.FreezerM);
    Out.ProduceM2 = FMath::Max(0.f, Out.ProduceM2);
    Out.Counters = FMath::Max(0, Out.Counters);
    Out.Checkouts = FMath::Max(0, Out.Checkouts);
    Out.SelfCheckouts = FMath::Max(0, Out.SelfCheckouts);
    return Out;
}

float MarketStoreAssign::VarietyFactor(const FStoreMeasures& Stats, const FString& Format)
{
    const FStoreMeasures S = Normalized(Stats, Format);
    return FMath::Clamp(MarketStoreAssignLocal::Ratio(S.ShelfFrontM, Nominal(Format).ShelfFrontM), 0.6f, 1.5f);
}

float MarketStoreAssign::NominalShoppers(const FString& Format)
{
    // A branch's shoppers are the catchment's trips x its share (MarketBranches::CloseDay); 0.3 is the share a
    // shop is planned for (MarketBranches' PlanShelves uses the same), so a nominal store at a typical share is at
    // load 1, not at a quarter of it.
    const int32 Trips = FMath::Max(1, MarketBranches::FormatInfo(MarketStoreAssignLocal::FormatKey(Format)).Trips);
    return 0.3f * static_cast<float>(Trips);
}

float MarketStoreAssign::QueueFactor(const FStoreMeasures& Stats, const FString& Format, float Shoppers)
{
    const FStoreMeasures S = Normalized(Stats, Format);
    const float Expected = NominalShoppers(Format);
    const float Traffic = Shoppers > 0.f ? Shoppers / Expected : 1.f;
    const float LaneRatio = FMath::Max(0.05f, MarketStoreAssignLocal::Ratio(MarketStoreAssignLocal::Lanes(S), MarketStoreAssignLocal::Lanes(Nominal(Format))));
    const float Load = Traffic / LaneRatio;
    const float Factor = Load > 1.f ? 1.f - 0.25f * (Load - 1.f) : 1.f + 0.05f * (1.f - Load);
    return FMath::Clamp(Factor, 0.6f, 1.05f);
}

float MarketStoreAssign::FreshFactor(const FStoreMeasures& Stats, const FString& Format)
{
    const FStoreMeasures S = Normalized(Stats, Format);
    const float R = MarketStoreAssignLocal::Ratio(MarketStoreAssignLocal::FreshRoom(S), MarketStoreAssignLocal::FreshRoom(Nominal(Format)));
    // 0.6 + 0.4 x R, written around 1 so that the nominal store (R == 1) gives exactly 1.
    return FMath::Clamp(1.f + 0.4f * (R - 1.f), 0.6f, 1.3f);
}

float MarketStoreAssign::SpoilFactor(const FStoreMeasures& Stats, const FString& Format)
{
    return FMath::Clamp(2.f - FreshFactor(Stats, Format), 0.7f, 1.4f);
}

float MarketStoreAssign::RentFactor(const FStoreMeasures& Stats, const FString& Format)
{
    const FStoreMeasures S = Normalized(Stats, Format);
    return FMath::Clamp(MarketStoreAssignLocal::Ratio(S.SalesAreaM2, Nominal(Format).SalesAreaM2), 0.5f, 1.6f);
}

float MarketStoreAssign::FitOutFactor(const FStoreMeasures& Stats, const FString& Format)
{
    const FStoreMeasures S = Normalized(Stats, Format);
    const FStoreMeasures Base = Nominal(Format);
    // 0.3 + 0.4 x area ratio + 0.3 x shelf ratio, written around 1 so that the nominal store gives exactly 1 (the
    // money below is rounded from it, and 0.3f + 0.4f + 0.3f is not exactly 1 in float).
    const float Factor = 1.f + 0.4f * (MarketStoreAssignLocal::Ratio(S.SalesAreaM2, Base.SalesAreaM2) - 1.f) + 0.3f * (MarketStoreAssignLocal::Ratio(S.ShelfFrontM, Base.ShelfFrontM) - 1.f);
    return FMath::Clamp(Factor, 0.6f, 1.5f);
}

float MarketStoreAssign::BackroomFactor(const FStoreMeasures& Stats, const FString& Format)
{
    const FStoreMeasures S = Normalized(Stats, Format);
    return FMath::Clamp(MarketStoreAssignLocal::Ratio(static_cast<float>(S.BackroomPallets), static_cast<float>(Nominal(Format).BackroomPallets)), 0.6f, 1.5f);
}

int32 MarketStoreAssign::WorkersFor(const FStoreMeasures& Stats, const FString& Format)
{
    const FStoreMeasures S = Normalized(Stats, Format);
    const FStoreMeasures Base = Nominal(Format);
    const int32 Workers = FMath::Max(1, MarketBranches::FormatInfo(MarketStoreAssignLocal::FormatKey(Format)).Workers);
    const float Scale = 0.5f * MarketStoreAssignLocal::Ratio(S.SalesAreaM2, Base.SalesAreaM2) + 0.5f * MarketStoreAssignLocal::Ratio(MarketStoreAssignLocal::StaffedLanes(S), MarketStoreAssignLocal::StaffedLanes(Base));
    const int32 Needed = FMath::RoundToInt(static_cast<float>(Workers) * Scale);
    return FMath::Clamp(Needed, FMath::Max(1, Workers / 2), Workers * 2);
}

int64 MarketStoreAssign::FitOutCost(const FStoreMeasures& Stats, const FString& Format, int32 GameDay)
{
    const int64 Base = MarketBranches::FormatInfo(MarketStoreAssignLocal::FormatKey(Format)).FitOut;
    const int64 Kurus2011 = FMath::RoundToInt64(static_cast<double>(Base) * static_cast<double>(FitOutFactor(Stats, Format)));
    return MarketPrices::Scaled(Kurus2011, GameDay);
}

int64 MarketStoreAssign::BaseMonthlyRent(const FStoreMeasures& Stats, const FString& Format, int32 GameDay)
{
    const int64 Base = MarketBranches::FormatInfo(MarketStoreAssignLocal::FormatKey(Format)).Rent;
    const int64 Kurus2011 = FMath::RoundToInt64(static_cast<double>(Base) * static_cast<double>(RentFactor(Stats, Format)));
    return MarketPrices::Scaled(Kurus2011, GameDay);
}

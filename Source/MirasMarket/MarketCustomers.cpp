#include "MarketCustomers.h"
#include "MarketCalendar.h"
#include "MarketDemand.h"
#include "MarketPrices.h"
#include "MarketPromotions.h"
#include "MarketEvents.h"

namespace MarketCustomers
{
    using MarketGoods::EGroup;

    FProfile MakeProfile(int32 MinList, int32 MaxList, int32 MinQ, int32 MaxQ, int64 Budget, double Tolerance, float Patience, float Walk, float Browse,
        std::initializer_list<TPair<EGroup, float>> Likes)
    {
        FProfile P;
        P.MinList = MinList; P.MaxList = MaxList; P.MinQuantity = MinQ; P.MaxQuantity = MaxQ; P.Budget = Budget;
        P.PriceTolerance = Tolerance; P.Patience = Patience; P.WalkSpeed = Walk; P.BrowseSeconds = Browse;
        for (float& Weight : P.Preference) Weight = 1.f;
        for (const TPair<EGroup, float>& Like : Likes) P.Preference[static_cast<int32>(Like.Key)] = Like.Value;
        return P;
    }

    const FProfile* BuildProfiles()
    {
        static FProfile Profiles[static_cast<int32>(ESegment::Count)];
        using P = TPair<EGroup, float>;
        // Retired: slow, patient, careful with money; milk, tea, bread-and-butter staples.
        Profiles[0] = MakeProfile(1, 3, 1, 2, 1500, -0.07, 120.f, 95.f, 3.0f,
            { P(EGroup::Dairy, 1.6f), P(EGroup::TeaCoffee, 1.8f), P(EGroup::Staples, 1.4f), P(EGroup::OilSauce, 1.3f), P(EGroup::Sweets, 0.7f),
              P(EGroup::Snacks, 0.4f), P(EGroup::Drinks, 0.6f), P(EGroup::IceCream, 0.5f), P(EGroup::PersonalCare, 0.9f) });
        // Family (weekly shop): the big basket; cleaning, staples, milk, a treat for the children.
        Profiles[1] = MakeProfile(2, 4, 1, 4, 6000, -0.02, 100.f, 125.f, 2.5f,
            { P(EGroup::Dairy, 1.4f), P(EGroup::Staples, 1.4f), P(EGroup::Household, 1.5f), P(EGroup::OilSauce, 1.3f), P(EGroup::Drinks, 1.1f),
              P(EGroup::Sweets, 1.1f), P(EGroup::Snacks, 0.9f), P(EGroup::PersonalCare, 1.3f), P(EGroup::Paper, 1.3f) });
        // Worker after work: quick, in a hurry, pays for convenience.
        Profiles[2] = MakeProfile(1, 2, 1, 2, 2500, 0.08, 60.f, 160.f, 1.0f,
            { P(EGroup::Drinks, 1.6f), P(EGroup::Snacks, 1.5f), P(EGroup::Sweets, 1.2f), P(EGroup::Dairy, 0.9f), P(EGroup::TeaCoffee, 0.8f),
              P(EGroup::Staples, 0.7f), P(EGroup::Household, 0.6f), P(EGroup::PersonalCare, 0.8f), P(EGroup::IceCream, 1.2f) });
        // Student: little money, very price sensitive; drinks, snacks, pasta.
        Profiles[3] = MakeProfile(1, 2, 1, 2, 800, -0.10, 80.f, 150.f, 1.5f,
            { P(EGroup::Drinks, 1.7f), P(EGroup::Snacks, 1.8f), P(EGroup::Sweets, 1.6f), P(EGroup::Staples, 1.2f), P(EGroup::Dairy, 0.7f),
              P(EGroup::TeaCoffee, 0.5f), P(EGroup::Household, 0.3f), P(EGroup::PersonalCare, 0.6f), P(EGroup::IceCream, 1.5f), P(EGroup::Paper, 0.8f) });
        // Trader (esnaf): buys for the shop or the workshop; tea by the kilo, drinks for customers, cleaning.
        Profiles[4] = MakeProfile(2, 4, 3, 6, 8000, 0.0, 70.f, 140.f, 1.2f,
            { P(EGroup::TeaCoffee, 2.2f), P(EGroup::Drinks, 1.5f), P(EGroup::Household, 1.4f), P(EGroup::Paper, 1.5f), P(EGroup::Dairy, 0.8f),
              P(EGroup::Staples, 0.6f), P(EGroup::IceCream, 0.6f) });
        // Child: pocket money, one thing, fast.
        Profiles[5] = MakeProfile(1, 1, 1, 1, 300, 0.05, 50.f, 170.f, 0.8f,
            { P(EGroup::Sweets, 2.5f), P(EGroup::Snacks, 2.0f), P(EGroup::Drinks, 1.5f), P(EGroup::IceCream, 2.5f), P(EGroup::Dairy, 0.8f),
              P(EGroup::TeaCoffee, 0.1f), P(EGroup::Staples, 0.1f), P(EGroup::OilSauce, 0.1f), P(EGroup::Household, 0.1f), P(EGroup::PersonalCare, 0.1f), P(EGroup::Paper, 0.3f) });
        return Profiles;
    }

    uint32 CustomerMix(int32 Seed, int32 Id, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), static_cast<uint32>(Id), Salt };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }
}

const MarketCustomers::FProfile& MarketCustomers::Profile(ESegment Segment)
{
    static const FProfile* Profiles = BuildProfiles();
    return Profiles[FMath::Clamp(static_cast<int32>(Segment), 0, static_cast<int32>(ESegment::Count) - 1)];
}

FString MarketCustomers::SegmentName(ESegment Segment)
{
    switch (Segment)
    {
    case ESegment::Retired: return TEXT("emekli");
    case ESegment::Family: return TEXT("aile");
    case ESegment::Worker: return TEXT("i\u015f \u00e7\u0131k\u0131\u015f\u0131");
    case ESegment::Student: return TEXT("\u00f6\u011frenci");
    case ESegment::Trader: return TEXT("esnaf");
    default: return TEXT("\u00e7ocuk");
    }
}

MarketCustomers::ESegment MarketCustomers::SegmentOf(int32 CustomerId, int32 Seed, const int32* Mix)
{
    if (CustomerId == NerminTeyzeId) return ESegment::Retired;
    int32 Total = 0;
    for (int32 S = 0; S < static_cast<int32>(ESegment::Count); ++S) Total += FMath::Max(0, Mix[S]);
    if (Total <= 0) return ESegment::Family;
    int32 Roll = static_cast<int32>(CustomerMix(Seed, CustomerId, 0x5E6u) % static_cast<uint32>(Total));
    for (int32 S = 0; S < static_cast<int32>(ESegment::Count); ++S)
    {
        Roll -= FMath::Max(0, Mix[S]);
        if (Roll < 0) return static_cast<ESegment>(S);
    }
    return ESegment::Family;
}

float MarketCustomers::TimeWeight(ESegment Segment, float DayProgress, int32 GameDay)
{
    // The shop day is 08:00-20:00: morning, midday (school out around 15:00), evening after work.
    static const float Morning[6] = { 1.6f, 0.8f, 0.5f, 0.6f, 1.5f, 0.5f };
    static const float Midday[6] = { 1.0f, 1.2f, 0.6f, 1.5f, 0.8f, 1.4f };
    static const float Evening[6] = { 0.4f, 1.1f, 1.9f, 1.0f, 0.5f, 0.6f };
    const int32 S = FMath::Clamp(static_cast<int32>(Segment), 0, 5);
    const float T = FMath::Clamp(DayProgress, 0.f, 1.f);
    float Weight = T < 0.33f ? Morning[S] : T < 0.66f ? Midday[S] : Evening[S];
    if (MarketCalendar::IsWeekend(GameDay))
    {
        static const float Weekend[6] = { 1.0f, 1.4f, 0.7f, 1.1f, 0.6f, 1.3f };
        Weight *= Weekend[S];
    }
    const MarketCalendar::FDate Date = MarketCalendar::DateOf(GameDay);
    const int32 Key = Date.Month * 100 + Date.Day;
    if (Key >= 615 && Key < 915) // school and university holidays
    {
        if (Segment == ESegment::Student) Weight *= 0.5f;
        if (Segment == ESegment::Child) Weight *= 1.3f;
    }
    return FMath::Clamp(Weight, 0.f, 2.f);
}

bool MarketCustomers::ComesNow(ESegment Segment, float DayProgress, int32 GameDay, float Roll)
{
    return Roll * 2.f < TimeWeight(Segment, DayProgress, GameDay);
}

TArray<int32> MarketCustomers::BuildList(const FMarketState& State, const TArray<FMarketProduct>& Products, ESegment Segment, FRandomStream& Random)
{
    const FProfile& P = Profile(Segment);
    TArray<int32> Result;
    const int32 Count = FMath::Min(Random.RandRange(P.MinList, P.MaxList), FMath::Min(State.Stock.Num(), Products.Num()));
    if (Count <= 0) return Result;
    // Weights: segment taste x the day (season, weather, bayram). Carried products get the MarketDemand share.
    TArray<float> Carried, Missing;
    Carried.Init(0.f, Products.Num());
    Missing.Init(0.f, Products.Num());
    float CarriedTotal = 0.f, MissingTotal = 0.f;
    for (int32 I = 0; I < Products.Num() && I < State.Stock.Num(); ++I)
    {
        const MarketGoods::EGroup Group = MarketGoods::Classify(Products[I].Category);
        const float Weight = P.Preference[static_cast<int32>(Group)] * MarketCalendar::GroupFactor(State.Day, State.RivalSeed, Group)
            * MarketPromotions::Interest(State, Products, I) // discounts, gondola head, flyer
            * MarketEvents::Factor(State, MarketEvents::EModifier::Interest, Group); // events, the shop's identity
        if (State.Stock[I].Capacity > 0) { Carried[I] = Weight; CarriedTotal += Weight; }
        else { Missing[I] = Weight; MissingTotal += Weight; }
    }
    auto Pick = [&Random](const TArray<float>& Weights, float Total) -> int32
    {
        float Roll = Random.FRand() * Total;
        for (int32 I = 0; I < Weights.Num(); ++I)
        {
            if (Weights[I] <= 0.f) continue;
            Roll -= Weights[I];
            if (Roll <= 0.f) return I;
        }
        for (int32 I = Weights.Num() - 1; I >= 0; --I) if (Weights[I] > 0.f) return I;
        return INDEX_NONE;
    };
    for (int32 Attempt = 0; Result.Num() < Count && Attempt < Count * 12; ++Attempt)
    {
        const bool bCarried = MissingTotal <= 0.f || (CarriedTotal > 0.f && Random.FRand() < MarketDemand::CarriedShare);
        const int32 Product = bCarried ? Pick(Carried, CarriedTotal) : Pick(Missing, MissingTotal);
        if (Product != INDEX_NONE) Result.AddUnique(Product);
    }
    for (int32 I = 0; Result.Num() < Count && I < Products.Num(); ++I) Result.AddUnique(I);
    return Result;
}

int32 MarketCustomers::Quantity(ESegment Segment, FRandomStream& Random)
{
    const FProfile& P = Profile(Segment);
    return Random.RandRange(P.MinQuantity, P.MaxQuantity);
}

int64 MarketCustomers::VisitBudget(ESegment Segment, int32 GameDay)
{
    // Wallets grow with wages (minimum wage steps), not with the shop's own prices.
    return static_cast<int64>(FMath::RoundToDouble(Profile(Segment).Budget * static_cast<double>(MarketCalendar::BudgetFactor(GameDay)) * MarketPrices::WageIndex(GameDay)));
}

int32 MarketCustomers::Affordable(int64 BudgetLeft, int64 Price, int32 Wanted)
{
    if (Price <= 0) return FMath::Max(0, Wanted);
    return static_cast<int32>(FMath::Clamp<int64>(BudgetLeft / Price, 0, FMath::Max(0, Wanted)));
}

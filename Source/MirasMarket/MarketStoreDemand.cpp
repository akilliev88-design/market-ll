#include "MarketStoreDemand.h"
#include "MarketAdvertising.h"
#include "MarketBranches.h"
#include "MarketCalendar.h"
#include "MarketChains.h"
#include "MarketCountry.h"
#include "MarketCompany.h"
#include "MarketEvents.h"
#include "MarketOnline.h"
#include "MarketPayments.h"
#include "MarketPromotions.h"
#include "MarketSimulation.h"
#include "MarketStart.h"
#include "MarketStrategy.h"
#include "MarketTuning.h"

namespace MarketStoreDemandLocal
{
    MarketBranches::FSite SiteOf(const FMarketState& State, const MarketStoreDemand::FStoreDay& Store)
    {
        return MarketBranches::SiteOf(State, Store.Country.IsEmpty() ? State.CountryId : Store.Country,
            Store.Province.IsEmpty() ? MarketStart::HomeProvince(State) : Store.Province);
    }
}

float MarketStoreDemand::Trips(const FMarketState& State, const FStoreDay& Store, int32 Day)
{
    if (MarketCalendar::ClosedByLaw(Day)) return 0.f;
    const MarketBranches::FSite Where = MarketStoreDemandLocal::SiteOf(State, Store);
    const MarketBranches::FFormat& Kind = MarketBranches::FormatInfo(Store.Format);
    // Bigger provinces are denser: a little more traffic per shop.
    const float Size = FMath::Clamp(FMath::Pow(FMath::Max(1.f, static_cast<float>(Where.PopulationK)) / static_cast<float>(MarketCountry::MedianPopK(Where.Country)), 0.1f), 0.85f, 1.3f);
    return Kind.Trips * Size * MarketCalendar::TrafficFactor(Day, State.RivalSeed)
        * MarketOnline::StoreTrafficFactorOn(State, Day, Where.Country) // M32: trips gone online, the epidemic's closure days
        * MarketAdvertising::TrafficFactor(State, Where.Country) // M34: the company's ads
        * (1.f + (MarketSimulation::TrafficFactor(State) - 1.f) * MarketBranches::BranchDifficultyShare) // C14e/C15b: the difficulty
        * (Store.bHasty ? MarketBranches::HastyTrips : 1.f) * Store.TripsExtra;
}

float MarketStoreDemand::Pull(const FMarketState& State, const FStoreDay& Store)
{
    return FMath::Exp(-(Store.PriceIndex - 1.f) / PriceSensitivity) * Store.Availability * Store.Service *
        (0.8f + Store.Satisfaction / 250.f) * (0.9f + 0.4f * Store.Maturity) * (Store.bNew ? 1.3f : 1.f) * MarketCompany::TrafficBonus(State)
        * Store.PullExtra
        // D9 (M48-M50): a province push, the paths, the chain's focus, the growth model's service; every store alike.
        * MarketStrategy::PullFactor(State, MarketStoreDemandLocal::SiteOf(State, Store).Country, MarketStoreDemandLocal::SiteOf(State, Store).Province, State.Day)
        * MarketStrategy::ServiceFactor(State);
}

float MarketStoreDemand::Share(const FMarketState& State, const FStoreDay& Store, int32 Day)
{
    const MarketBranches::FSite Where = MarketStoreDemandLocal::SiteOf(State, Store);
    const float P = Pull(State, Store);
    // Akis C2b: the province's chains against the start, a price war against us on top.
    return P / (P + MarketTuning::Get(TEXT("BranchCompetition"), 3.f) * Where.Competition * MarketChains::PressureFactor(State, Where.Country, Where.Province, Day));
}

float MarketStoreDemand::NeutralShare(const FMarketState& State, const FString& Country, const FString& Province, const FString& Format)
{
    FStoreDay Store;
    Store.Country = Country;
    Store.Province = Province;
    Store.Format = Format;
    Store.Availability = 0.9f;
    Store.Service = MarketBranches::FormatInfo(Format).Service;
    const MarketBranches::FSite Where = MarketStoreDemandLocal::SiteOf(State, Store);
    const float P = Pull(State, Store);
    return P / (P + MarketTuning::Get(TEXT("BranchCompetition"), 3.f) * Where.Competition);
}

float MarketStoreDemand::Cannibalization(const FMarketState& State, const FStoreDay& Store)
{
    const MarketBranches::FSite Where = MarketStoreDemandLocal::SiteOf(State, Store);
    float Others = Where.bHome && Store.Self != FamilyShop ? 1.f : 0.f; // the family shop
    for (int32 I = 0; I < State.Branches.Num(); ++I)
    {
        const FMarketBranch& B = State.Branches[I];
        if (I == Store.Self || B.Stage != static_cast<uint8>(MarketBranches::EStage::Open)) continue;
        if (MarketBranches::CountryOf(State, B) != Where.Country) continue;
        if ((B.Province.IsEmpty() ? MarketStart::HomeProvince(State) : B.Province) != Where.Province) continue;
        Others += MarketBranches::FormatInfo(B.Format).Weight;
    }
    const float Slots = FMath::Max(1.f, static_cast<float>(Where.PopulationK) / MarketBranches::PeoplePerSlotK);
    return 1.f / (1.f + 0.6f * Others / Slots);
}

float MarketStoreDemand::ShoppersExact(const FMarketState& State, const FStoreDay& Store, int32 Day)
{
    return Trips(State, Store, Day) * Share(State, Store, Day) * (0.5f + 0.5f * Store.Maturity) * Cannibalization(State, Store);
}

int32 MarketStoreDemand::Shoppers(const FMarketState& State, const FStoreDay& Store, int32 Day)
{
    return FMath::RoundToInt32(ShoppersExact(State, Store, Day));
}

MarketStoreDemand::FStoreDay MarketStoreDemand::FamilyDay(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Day)
{
    FStoreDay Store;
    Store.Country = State.CountryId;
    Store.Province = MarketStart::HomeProvince(State);
    Store.Format = TEXT("mahalle");
    Store.Self = FamilyShop;
    // Price level: what shoppers paid against the list, weighted by how much each product is asked for.
    double PriceSum = 0.0, WeightSum = 0.0;
    int32 Sold = 0, Empty = 0, Missing = 0;
    for (int32 I = 0; I < State.Stock.Num() && I < Products.Num(); ++I)
    {
        const FMarketStock& Item = State.Stock[I];
        const FMarketDemandStats& Y = Item.Yesterday;
        Sold += Y.Sold; Empty += Y.Empty; Missing += Y.NotCarried;
        if (Item.Capacity <= 0 || Products[I].BasePrice <= 0) continue;
        const double Weight = 1.0 + Y.Sold + Y.Empty + Y.Expensive;
        PriceSum += Weight * static_cast<double>(MarketPromotions::UnitPrice(State, Products, I, 1)) / Products[I].BasePrice;
        WeightSum += Weight;
    }
    Store.PriceIndex = WeightSum > 0.0 ? static_cast<float>(PriceSum / WeightSum) : 1.2f;
    const int32 Asked = Sold + Empty + Missing / 2;
    Store.Availability = Asked > 0 ? FMath::Clamp(static_cast<float>(Sold) / Asked, 0.2f, 1.f) : (WeightSum > 0.0 ? 0.8f : 0.3f);
    // Service: nobody gave up waiting, and most visitors left with what they came for.
    const int32 Visitors = State.LastServed + State.LastLost;
    const float Served = Visitors > 0 ? static_cast<float>(State.LastServed) / Visitors : 0.9f;
    Store.Service = MarketBranches::FormatInfo(Store.Format).Service
        * (Visitors > 0 ? FMath::Clamp(1.f - 0.8f * State.LastLostWaiting / Visitors, 0.3f, 1.f) : 1.f) * (0.55f + 0.5f * Served);
    Store.Satisfaction = 50.f;
    if (State.Loyalty.Num() > 0)
    {
        float Sum = 0.f;
        for (const FMarketLoyalty& L : State.Loyalty) Sum += L.Satisfaction;
        Store.Satisfaction = Sum / State.Loyalty.Num();
    }
    Store.Maturity = 1.f; // the shop has been on this street for years
    // The family shop's own trips: its flyers and promotions, the street's events, the card terminal.
    Store.TripsExtra = FamilySiteTrips * MarketPromotions::TrafficFactor(State) * MarketEvents::Factor(State, MarketEvents::EModifier::Traffic) * MarketPayments::TrafficFactor(State);
    (void)Day;
    return Store;
}

int32 MarketStoreDemand::FamilyShoppers(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Day)
{
    return Shoppers(State, FamilyDay(State, Products, Day), Day);
}

void MarketStoreDemand::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    const int32 Closed = State.Day - 1;
    if (Closed < 1 || State.LastServed + State.LastLost <= 0) return;
    const float Target = Share(State, FamilyDay(State, Products, Closed), Closed);
    State.MarketShare = FMath::Clamp(State.ShareBeforeClose * 0.85f + Target * 100.f * 0.15f, 5.f, 65.f);
}

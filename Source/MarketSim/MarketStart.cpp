#include "MarketStart.h"
#include "MarketCountry.h"
#include "MarketChains.h"
#include "MarketStaff.h"
#include "MarketFinance.h"
#include "MarketOwner.h"
#include "MarketSubsidiaries.h"

namespace MarketStart
{
    uint32 StartMix(int32 Seed, uint32 Salt, uint32 Extra)
    {
        uint32 Hash = 2166136261u ^ Salt;
        const uint32 Parts[2] = { static_cast<uint32>(Seed), Extra };
        for (uint32 Part : Parts) for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }
}

void MarketStart::Setup(FMarketState& State, const FString& CountryId, const FString& CityId, int32 Seed)
{
    State.CountryId = MarketCountry::FindOrDefault(CountryId).Id;
    State.CityId = CityId;
    if (!MarketCountry::FindCity(State.CountryId, State.CityId)) State.CityId = FallbackProvince(State.CountryId);
    State.RivalSeed = Seed;
    // The shop comes with its people: one cashier, two shelf stockers (karar L03).
    MarketStaff::AddStartingStaff(State, 1, 2);
    // M37 (Mustafa 02.10.2026: "ba\u015flang\u0131\u00e7taki bor\u00e7, d\u00fckk\u00e2n\u0131n elindeki para ekonomiye g\u00f6re"): the till holds a
    // month of the shop's fixed costs with our salary (people, running costs; M69: the building is ours, no rent);
    // the inherited debt to the wholesaler is a month and a half of them. Both follow the country and the province.
    MarketCountry::SetActive(State.CountryId, Seed); // the country's wages and prices for the numbers below
    State.Owner.SalaryX10 = MarketOwner::StartSalaryX10;
    const int64 Month = MarketFinance::CompanyMonthCost(State) + MarketOwner::CompanyCost(State);
    State.Cash = FMath::Max<int64>(10000, Month / 100 * 100);
    State.StartDebt = FMath::Max<int64>(10000, FMath::RoundToInt64(Month * StartDebtMonths) / 10000 * 10000);
    State.InheritedDebt = State.StartDebt;
    MarketEras::Setup(State); // C3 (B4): this campaign's eras, shifted by the seed
    MarketChains::Ensure(State); // E2: the province's chains are there from the first day (the shoppers' rivals)
    MarketSubsidiaries::Ensure(State, State.CountryId, false); // M65: the parent company
}

int32 MarketStart::StockShelvesPartly(FMarketState& State, int32 Seed)
{
    int32 Moved = 0;
    for (int32 I = 0; I < State.Stock.Num(); ++I)
    {
        FMarketStock& Item = State.Stock[I];
        if (Item.Capacity <= 0 || Item.Warehouse <= 0) continue;
        const float Fill = 0.40f + (StartMix(Seed, 0x5F11u, static_cast<uint32>(I)) % 36u) / 100.f; // 40-75 %
        const int32 Want = FMath::Max(0, FMath::RoundToInt(Item.Capacity * Fill) - Item.Shelf);
        const int32 Take = FMath::Min(Want, Item.Warehouse);
        Item.Shelf += Take;
        Item.Warehouse -= Take;
        Moved += Take;
    }
    return Moved;
}

FString MarketStart::FallbackProvince(const FString& CountryId)
{
    const MarketCountry::FProfile* Country = CountryId.IsEmpty() ? &MarketCountry::Default() : MarketCountry::Find(CountryId);
    if (!Country) return FString();
    if (!Country->MedianProvince.IsEmpty() && MarketCountry::FindCity(Country->Id, Country->MedianProvince)) return Country->MedianProvince;
    return Country->Cities.Num() > 0 ? Country->Cities[0].Id : FString();
}

FString MarketStart::HomeProvince(const FMarketState& State)
{
    return MarketCountry::FindCity(State.CountryId, State.CityId) ? State.CityId : FallbackProvince(State.CountryId);
}

FString MarketStart::PlaceText(const FMarketState& State)
{
    const MarketCountry::FProfile* Country = MarketCountry::Find(State.CountryId);
    const MarketCountry::FCity* City = MarketCountry::FindCity(State.CountryId, HomeProvince(State));
    const FString CountryName = Country ? Country->Name : MarketCountry::Default().Name;
    return City ? City->Name + TEXT(", ") + CountryName : CountryName;
}

FString MarketStart::FirstStoreName(const FMarketState& State)
{
    const MarketCountry::FCity* City = MarketCountry::FindCity(State.CountryId, HomeProvince(State));
    const FString Brand = MarketSubsidiaries::Brand(State);
    return City ? Brand + TEXT(" ") + City->Name : Brand;
}

FString MarketStart::PlayerName(const FMarketState& State)
{
    return State.PlayerName.TrimStartAndEnd().Left(MaxPlayerNameLength);
}

FString MarketStart::IntroText(const FMarketState& State)
{
    // M69: the one story of the game; what follows is the player's own.
    const FString Name = PlayerName(State);
    return FString::Printf(TEXT("%s%s. %s, il\u00e7edeki tek \u015fubeli k\u00fc\u00e7\u00fck bir marketti. Onu y\u0131llarca i\u015fleten aile yoruldu ve i\u015fi sana devretti; bina da art\u0131k senin. Bir kasiyer, iki reyon g\u00f6revlisi, toptanc\u0131ya %s bor\u00e7 ve kasada bir ayl\u0131k gider. Bor\u00e7 i\u00e7in s\u00fcre yok; ne zaman \u00f6deyece\u011fine sen karar verirsin. Raflar yar\u0131 dolu; depoya bak, eksikleri diz ve O ile a\u00e7."),
        Name.IsEmpty() ? TEXT("") : *(Name + TEXT(" \u00b7 ")), *PlaceText(State), *MarketSubsidiaries::Brand(State), *MarketCountry::Money(State.InheritedDebt));
}

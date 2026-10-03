#include "MarketFranchise.h"
#include "MarketBranches.h"
#include "MarketCalendar.h"
#include "MarketCompany.h"
#include "MarketCountry.h"
#include "MarketLedger.h"
#include "MarketPrices.h"
#include "MarketResearch.h"
#include "MarketStory.h"

namespace MarketFranchiseLocal
{
    FMarketFranchise* FindMutable(FMarketState& State, const FString& Country)
    {
        return State.Company.Franchises.FindByPredicate([&Country](const FMarketFranchise& F) { return F.Country == Country && !F.bEnded; });
    }

    uint32 Hash(const FMarketState& State, const FString& Country, uint32 Salt)
    {
        return (GetTypeHash(Country) * 2654435761u) ^ (static_cast<uint32>(State.RivalSeed) * 40503u) ^ Salt;
    }

    // A local family name and one of the country's local shop words ("Schmidt Markt").
    FString PartnerName(const FMarketState& State, const FString& Country)
    {
        const MarketCountry::FProfile& Pack = MarketCountry::FindOrDefault(Country);
        const TArray<FString>& Words = Pack.LocalSuffixes.Num() > 0 ? Pack.LocalSuffixes : MarketCountry::Default().LocalSuffixes;
        const uint32 H = Hash(State, Country, 0xF7A2u);
        const FString Family = Pack.LastNames.Num() > 0 ? Pack.LastNames[H % static_cast<uint32>(Pack.LastNames.Num())] : FString(TEXT("Yerel"));
        return Words.Num() > 0 ? Family + TEXT(" ") + Words[(H / 13u) % static_cast<uint32>(Words.Num())] : Family;
    }

    // A day of one store's sales with Stores of the brand in the country (home-level kurus, like a branch's).
    int64 PerStore(const FMarketState& State, const FMarketFranchise& F, int32 Stores)
    {
        const float Fill = static_cast<float>(Stores) / static_cast<float>(FMath::Max(1, MarketFranchise::MaxStores(F.Country)));
        const double Base = static_cast<double>(MarketFranchise::StoreDaySales) * MarketPrices::ListLevel(FMath::Max(1, State.Day)) * F.Quality / 100.0;
        return FMath::RoundToInt64(Base * (1.0 - MarketFranchise::Crowding * FMath::Clamp(Fill, 0.f, 1.f)));
    }

    int64 DaySales(const FMarketState& State, const FMarketFranchise& F)
    {
        return PerStore(State, F, F.Stores) * F.Stores;
    }

    FString NameOf(const FString& Country) { return MarketCountry::FindOrDefault(Country).Name; }
}

const FMarketFranchise* MarketFranchise::Find(const FMarketState& State, const FString& Country)
{
    return State.Company.Franchises.FindByPredicate([&Country](const FMarketFranchise& F) { return F.Country == Country && !F.bEnded; });
}

int32 MarketFranchise::MaxStores(const FString& Country)
{
    return FMath::Clamp(MarketCountry::PopulationK(Country) / 2000, 8, 60);
}

int64 MarketFranchise::StoreDaySalesNow(const FMarketState& State, const FMarketFranchise& F)
{
    return MarketFranchiseLocal::PerStore(State, F, F.Stores);
}

int64 MarketFranchise::OpeningCost(const FMarketState& State)
{
    const MarketBranches::FFormat& Kind = MarketBranches::FormatInfo(TEXT("mahalle"));
    return FMath::RoundToInt64(static_cast<double>(Kind.FitOut + 2 * Kind.Rent) * MarketPrices::ListLevel(FMath::Max(1, State.Day)));
}

int64 MarketFranchise::Fee(const FMarketState& State, const FString& Country)
{
    return MarketResearch::Cost(State, Country);
}

bool MarketFranchise::CanSign(const FMarketState& State, const FString& Country, FString& OutReason)
{
    if (Country.IsEmpty() || Country == State.CountryId || !MarketCountry::Find(Country)) { OutReason = TEXT("Ortakl\u0131k yaln\u0131z yurt d\u0131\u015f\u0131nda olur."); return false; }
    const FString Name = MarketFranchiseLocal::NameOf(Country);
    if (Find(State, Country)) { OutReason = FString::Printf(TEXT("%s: ortakl\u0131k zaten s\u00fcr\u00fcyor."), *Name); return false; }
    if (State.Branches.ContainsByPredicate([&State, &Country](const FMarketBranch& B) { return B.Stage != static_cast<uint8>(MarketBranches::EStage::Closed) && MarketBranches::CountryOf(State, B) == Country; }))
    {
        OutReason = FString::Printf(TEXT("%s: kendi ma\u011fazalar\u0131n var; ortakl\u0131k yeni girilen \u00fclke i\u00e7indir."), *Name);
        return false;
    }
    if (!MarketCompany::AbroadOpen(State)) { OutReason = MarketCompany::AbroadLock(State); return false; }
    if (!MarketResearch::AllowsEntry(State, Country, OutReason)) return false; // M58
    const int64 Price = Fee(State, Country);
    if (State.Cash < Price + State.OtherCosts) { OutReason = FString::Printf(TEXT("S\u00f6zle\u015fme ve marka tescili i\u00e7in kasada %s gerekir."), *MarketCountry::Money(Price)); return false; }
    return true;
}

bool MarketFranchise::Sign(FMarketState& State, const FString& Country, FString& OutMessage)
{
    if (!CanSign(State, Country, OutMessage)) return false;
    const int64 Price = Fee(State, Country);
    const bool bFirst = State.Company.Franchises.Num() == 0;
    FMarketFranchise F;
    F.Country = Country;
    F.Partner = MarketFranchiseLocal::PartnerName(State, Country);
    F.StartDay = State.Day;
    F.Stores = StartStores;
    F.Quality = 80 + static_cast<int32>(MarketFranchiseLocal::Hash(State, Country, 0x0A11u) % 41u);
    State.Company.Franchises.Add(F);
    MarketLedger::AddStoreCost(State, Price, MarketLedger::HeadOfficeStore); // paid at the day close
    OutMessage = FString::Printf(TEXT("%s ile ortakl\u0131k imzaland\u0131 (%s). %s ad\u0131m\u0131zla %d ma\u011fazayla ba\u015fl\u0131yor; kazand\u0131k\u00e7a ve yer bulduk\u00e7a yenisini a\u00e7ar. Sat\u0131\u015flar\u0131n %%%.0f'\u00fc her ay bize gelir."),
        *F.Partner, *MarketCountry::Money(Price), *MarketFranchiseLocal::NameOf(Country), StartStores, 100.f * RoyaltyRate);
    if (bFirst) MarketStory::AddMemory(State, FString::Printf(TEXT("ilk ortakl\u0131k: %s, %s"), *F.Partner, *MarketFranchiseLocal::NameOf(Country)));
    return true;
}

int64 MarketFranchise::EndCost(const FMarketState& State, const FString& Country)
{
    const FMarketFranchise* F = Find(State, Country);
    return F ? FMath::Max(Fee(State, Country), F->LastRoyalty * EndRoyaltyMonths) : 0;
}

bool MarketFranchise::End(FMarketState& State, const FString& Country, FString& OutMessage)
{
    FMarketFranchise* F = MarketFranchiseLocal::FindMutable(State, Country);
    if (!F) { OutMessage = TEXT("Bu \u00fclkede ortakl\u0131k yok."); return false; }
    const int64 Price = EndCost(State, Country);
    if (State.Cash < Price + State.OtherCosts) { OutMessage = FString::Printf(TEXT("Ortakl\u0131\u011f\u0131 bitirmek i\u00e7in kasada %s gerekir."), *MarketCountry::Money(Price)); return false; }
    MarketLedger::AddStoreCost(State, Price, MarketLedger::HeadOfficeStore);
    F->bEnded = true;
    F->EndDay = State.Day;
    OutMessage = FString::Printf(TEXT("%s ile ortakl\u0131k bitti (tazminat %s). %d ma\u011faza kendi ad\u0131na d\u00f6n\u00fcyor; %s kendi ma\u011fazan\u0131 a\u00e7abilirsin (pazar ara\u015ft\u0131rmas\u0131 ge\u00e7erliyse)."),
        *F->Partner, *MarketCountry::Money(Price), F->Stores, *MarketFranchiseLocal::NameOf(Country));
    return true;
}

int32 MarketFranchise::StoresIn(const FMarketState& State, const FString& Country)
{
    const FMarketFranchise* F = Find(State, Country);
    return F ? F->Stores : 0;
}

int64 MarketFranchise::YearSales(const FMarketState& State, const FString& Country)
{
    const FMarketFranchise* F = Find(State, Country);
    return F ? MarketFranchiseLocal::DaySales(State, *F) * 365 : 0;
}

TArray<FString> MarketFranchise::Countries(const FMarketState& State)
{
    TArray<FString> List;
    for (const FMarketFranchise& F : State.Company.Franchises) if (!F.bEnded) List.AddUnique(F.Country);
    return List;
}

FString MarketFranchise::StatusText(const FMarketState& State, const FString& Country)
{
    if (const FMarketFranchise* F = Find(State, Country))
        return FString::Printf(TEXT("Ortakl\u0131k: %s, ad\u0131m\u0131zla %d ma\u011faza (en \u00e7ok %d) \u00b7 ge\u00e7en ay k\u00e2r pay\u0131 %s \u00b7 toplam %s."),
            *F->Partner, F->Stores, MaxStores(Country), *MarketCountry::Money(F->LastRoyalty), *MarketCountry::Money(F->TotalRoyalty));
    if (Country.IsEmpty() || Country == State.CountryId) return FString();
    return FString::Printf(TEXT("Ortakl\u0131kla giri\u015f: yerel bir ortak ad\u0131m\u0131zla %d ma\u011fazayla a\u00e7ar, her ay sat\u0131\u015f\u0131n %%%.0f'\u00fc bize gelir. S\u00f6zle\u015fme %s; s\u00fcrerken orada kendi ma\u011fazan olmaz."),
        StartStores, 100.f * RoyaltyRate, *MarketCountry::Money(Fee(State, Country)));
}

void MarketFranchise::CloseDay(FMarketState& State)
{
    const int32 Closed = State.Day - 1;
    if (Closed < 1) return;
    const bool bMonthStart = MarketCalendar::DateOf(State.Day).Day == 1;
    for (FMarketFranchise& F : State.Company.Franchises)
    {
        if (F.bEnded) continue;
        if (Closed < F.StartDay) continue;
        const int64 Sales = MarketFranchiseLocal::DaySales(State, F);
        F.MonthSales += Sales;
        F.Savings += FMath::RoundToInt64(static_cast<double>(Sales) * PartnerMargin);
        // A new store when the partner can pay for it and one more would still sell well (no day rule).
        const int64 Cost = OpeningCost(State);
        const int64 Lone = MarketFranchiseLocal::PerStore(State, F, 0);
        if (F.Savings >= Cost && F.Stores < MaxStores(F.Country) && MarketFranchiseLocal::PerStore(State, F, F.Stores + 1) >= FMath::RoundToInt64(Lone * MinStoreShare))
        {
            F.Savings -= Cost;
            ++F.Stores;
            State.DayNews.Add(FString::Printf(TEXT("Orta\u011f\u0131m\u0131z %s (%s) ad\u0131m\u0131zla %d. ma\u011fazay\u0131 a\u00e7t\u0131."), *F.Partner, *MarketFranchiseLocal::NameOf(F.Country), F.Stores));
        }
        if (!bMonthStart || F.MonthSales <= 0) continue;
        // The month's royalty in our money; the country takes its withholding tax (M65's rate).
        const MarketBranches::FMoney Money = MarketBranches::MoneyOf(State, F.Country, State.Day);
        const int64 Royalty = MarketBranches::InHome(Money, FMath::RoundToInt64(static_cast<double>(F.MonthSales) * RoyaltyRate));
        const int64 Withheld = FMath::RoundToInt64(static_cast<double>(Royalty) * MarketCountry::FindOrDefault(F.Country).DividendWithholding);
        F.LastRoyalty = Royalty;
        F.TotalRoyalty += Royalty;
        F.MonthSales = 0;
        State.Cash += Royalty - Withheld;
        State.LastProfit += Royalty - Withheld;
        MarketLedger::Post(State, MarketLedger::EAccount::OtherIncome, Royalty, true, MarketLedger::HeadOfficeStore);
        if (Withheld > 0) MarketLedger::Post(State, MarketLedger::EAccount::Withholding, -Withheld, true, MarketLedger::HeadOfficeStore);
        State.DayNews.Add(FString::Printf(TEXT("%s: ge\u00e7en ay\u0131n k\u00e2r pay\u0131 %s geldi%s."), *F.Partner, *MarketCountry::Money(Royalty),
            Withheld > 0 ? *FString::Printf(TEXT(" (%s stopaj kesildi)"), *MarketCountry::Money(Withheld)) : TEXT("")));
    }
}

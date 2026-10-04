#include "MarketResearch.h"
#include "MarketBranches.h"
#include "MarketChains.h"
#include "MarketCompany.h"
#include "MarketCountry.h"
#include "MarketLedger.h"
#include "MarketPrices.h"
#include "MarketSubsidiaries.h"
#include "MarketStory.h"

namespace MarketResearchLocal
{
    const FMarketResearch* Find(const FMarketState& State, const FString& Country)
    {
        return State.Company.Research.FindByPredicate([&Country](const FMarketResearch& R) { return R.Country == Country; });
    }

    bool Needed(const FMarketState& State, const FString& Country)
    {
        return !Country.IsEmpty() && Country != State.CountryId && !MarketSubsidiaries::Find(State, Country);
    }

    float SizeFactor(const FString& Country)
    {
        // Half a store's opening for a small country, twice for a very big one (100 million people = 1.5).
        return FMath::Clamp(0.5f + static_cast<float>(MarketCountry::PopulationK(Country)) / 100000.f, 0.5f, 2.f);
    }
}

MarketResearch::EStatus MarketResearch::Status(const FMarketState& State, const FString& Country)
{
    if (!MarketResearchLocal::Needed(State, Country)) return EStatus::NotNeeded;
    const FMarketResearch* R = MarketResearchLocal::Find(State, Country);
    if (!R) return EStatus::None;
    if (State.Day < R->ReadyDay) return EStatus::Running;
    return State.Day - R->ReadyDay <= ValidDays ? EStatus::Ready : EStatus::Expired;
}

bool MarketResearch::AllowsEntry(const FMarketState& State, const FString& Country, FString& OutReason)
{
    const MarketCountry::FProfile& Pack = MarketCountry::FindOrDefault(Country);
    switch (Status(State, Country))
    {
    case EStatus::NotNeeded: case EStatus::Ready: return true;
    case EStatus::Running:
    {
        const FMarketResearch* R = MarketResearchLocal::Find(State, Country);
        OutReason = FString::Printf(TEXT("%s pazar ara\u015ft\u0131rmas\u0131 s\u00fcr\u00fcyor (%d g\u00fcn kald\u0131)."), *Pack.Name, R ? R->ReadyDay - State.Day : 0);
        return false;
    }
    case EStatus::Expired: OutReason = FString::Printf(TEXT("%s pazar ara\u015ft\u0131rmas\u0131n\u0131n s\u00fcresi doldu: yenisi gerekir."), *Pack.Name); return false;
    default: OutReason = FString::Printf(TEXT("%s'ya girmeden \u00f6nce pazar ara\u015ft\u0131rmas\u0131 gerekir (\u015eubeler sayfas\u0131)."), *Pack.Name); return false;
    }
}

int64 MarketResearch::Cost(const FMarketState& State, const FString& Country)
{
    const MarketBranches::FFormat& Kind = MarketBranches::FormatInfo(TEXT("mahalle"));
    const double Opening = static_cast<double>(Kind.FitOut + 2 * Kind.Rent) * MarketPrices::ListLevel(FMath::Max(1, State.Day));
    const int64 HomeLevel = FMath::RoundToInt64(Opening * MarketResearchLocal::SizeFactor(Country));
    return MarketBranches::InHome(MarketBranches::MoneyOf(State, Country, State.Day), HomeLevel) / 100 * 100;
}

int32 MarketResearch::Days(const FMarketState& State, const FString& Country)
{
    (void)State;
    return 75 + FMath::Clamp(MarketCountry::PopulationK(Country) / 2000, 0, 105); // D8 (M68): a few months
}

namespace MarketResearchLocal
{
    // What a shop earns against what it costs to run in a country: grocery spending per person over wages and rents.
    double Margin(const MarketCountry::FProfile& P)
    {
        const double Spend = P.GroceryPerPersonDay > 0.0 ? P.GroceryPerPersonDay : 65.0 * FMath::Max(0.1f, P.WageFactor); // the same default as MarketCompany
        return Spend / FMath::Max(0.1, 0.6 * P.WageFactor + 0.4 * P.RentFactor);
    }

    // Chain stores per million people of a country's national roster.
    double Crowd(const FString& Country)
    {
        int32 Stores = 0;
        for (const MarketChains::FRosterChain& Row : MarketChains::NationalRoster()) if (Row.Country == Country) Stores += Row.StartStores;
        return FMath::Max(1.0, static_cast<double>(Stores) / FMath::Max(1.0, MarketCountry::PopulationK(Country) / 1000.0));
    }
}

float MarketResearch::Attractiveness(const FMarketState& State, const FString& Country)
{
    const FString Home = State.CountryId.IsEmpty() ? MarketCountry::DefaultId() : State.CountryId;
    if (Country.IsEmpty() || Country == Home) return 1.f;
    static TMap<FString, float> Cache;
    const FString Key = FString::Printf(TEXT("%s|%s|%d"), *Home, *Country, State.RivalSeed);
    if (const float* Found = Cache.Find(Key)) return *Found;
    const MarketCountry::FProfile* Pack = MarketCountry::Find(Country);
    const MarketCountry::FProfile* Own = MarketCountry::Find(Home);
    float Score = 1.f;
    if (Pack && Own)
    {
        const double Base = MarketResearchLocal::Margin(*Pack) / FMath::Max(1.0, MarketResearchLocal::Margin(*Own));
        const double Crowding = MarketResearchLocal::Crowd(Country) / MarketResearchLocal::Crowd(Home);
        const uint32 Hash = GetTypeHash(Key) * 2654435761u;
        const double Reading = 0.9 + 0.2 * static_cast<double>(Hash % 1000u) / 999.0; // this campaign's market
        Score = static_cast<float>(FMath::Clamp(Base / FMath::Pow(Crowding, 0.25) * Reading, 0.3, 2.0));
    }
    Cache.Add(Key, Score);
    return Score;
}

MarketResearch::EVerdict MarketResearch::Verdict(const FMarketState& State, const FString& Country)
{
    const float Score = Attractiveness(State, Country);
    return Score >= GoodScore ? EVerdict::Good : Score >= HardScore ? EVerdict::Hard : EVerdict::No;
}

FString MarketResearch::VerdictText(const FMarketState& State, const FString& Country)
{
    const int32 Learning = FMath::RoundToInt32(MarketCompany::LearningDays * LearningFactor(State, Country));
    switch (Verdict(State, Country))
    {
    case EVerdict::Good: return FString::Printf(TEXT("Sonu\u00e7: girmeye de\u011fer. \u0130lk %d g\u00fcn al\u0131\u015fma d\u00f6nemi."), Learning);
    case EVerdict::Hard: return FString::Printf(TEXT("Sonu\u00e7: zor pazar; rakipler g\u00fc\u00e7l\u00fc ya da maliyet y\u00fcksek. Girersen ilk %d g\u00fcn bocalars\u0131n; ortakl\u0131k daha g\u00fcvenli."), Learning);
    default: return FString::Printf(TEXT("Sonu\u00e7: \u015fimdilik girmeyin; pazar dolu ve pahal\u0131. Girersen ilk %d g\u00fcn a\u011f\u0131r ge\u00e7er."), Learning);
    }
}

float MarketResearch::LearningFactor(const FMarketState& State, const FString& Country)
{
    return FMath::Clamp(2.2f - 1.2f * Attractiveness(State, Country), 0.6f, 2.f);
}

bool MarketResearch::CanStart(const FMarketState& State, const FString& Country, FString& OutReason)
{
    const EStatus S = Status(State, Country);
    if (S == EStatus::NotNeeded) { OutReason = TEXT("Bu \u00fclke i\u00e7in ara\u015ft\u0131rma gerekmez."); return false; }
    if (S == EStatus::Running || S == EStatus::Ready) { OutReason = TEXT("Ge\u00e7erli bir ara\u015ft\u0131rma zaten var."); return false; }
    if (!MarketCompany::AbroadOpen(State)) { OutReason = MarketCompany::AbroadLock(State); return false; } // D6 (M67)
    if (State.Cash < Cost(State, Country)) { OutReason = FString::Printf(TEXT("Ara\u015ft\u0131rma i\u00e7in kasada %s gerekir."), *MarketCountry::Money(Cost(State, Country))); return false; }
    return true;
}

bool MarketResearch::Start(FMarketState& State, const FString& Country, FString& OutMessage)
{
    if (!CanStart(State, Country, OutMessage)) return false;
    const int64 Price = Cost(State, Country);
    State.Company.Research.RemoveAll([&Country](const FMarketResearch& R) { return R.Country == Country; });
    FMarketResearch R;
    R.Country = Country;
    R.StartDay = State.Day;
    R.ReadyDay = State.Day + Days(State, Country);
    R.Cost = Price;
    State.Company.Research.Add(R);
    MarketLedger::AddStoreCost(State, Price, MarketLedger::HeadOfficeStore); // paid at the day close
    OutMessage = FString::Printf(TEXT("%s pazar ara\u015ft\u0131rmas\u0131 ba\u015flad\u0131 (%s). Rapor %d g\u00fcn sonra gelir; bir y\u0131l ge\u00e7erlidir."),
        *MarketCountry::FindOrDefault(Country).Name, *MarketCountry::Money(Price), R.ReadyDay - State.Day);
    return true;
}

FString MarketResearch::Report(const FMarketState& State, const FString& Country)
{
    const EStatus S = Status(State, Country);
    if (S != EStatus::Ready && S != EStatus::Expired) return FString();
    const MarketCountry::FProfile& Pack = MarketCountry::FindOrDefault(Country);
    // Provinces: purchasing power against rent and competition, a little for size.
    TArray<TPair<float, FString>> Best;
    for (const MarketCountry::FCity& City : Pack.Cities)
        Best.Add({ City.Income / FMath::Max(0.3f, City.Rent * City.Competition) * FMath::Pow(static_cast<float>(FMath::Max(10, City.PopulationK)), 0.15f), City.Name });
    Best.Sort([](const TPair<float, FString>& A, const TPair<float, FString>& B) { return A.Key > B.Key; });
    TArray<FString> Places;
    for (int32 I = 0; I < FMath::Min(3, Best.Num()); ++I) Places.Add(Best[I].Value);
    // Chains: the country's national chains by stores.
    int32 Total = 0;
    TArray<TPair<int32, FString>> Chains;
    for (const MarketChains::FRosterChain& Row : MarketChains::NationalRoster())
        if (Row.Country == Country) { Chains.Add({ Row.StartStores, Row.Name }); Total += Row.StartStores; }
    Chains.Sort([](const TPair<int32, FString>& A, const TPair<int32, FString>& B) { return A.Key > B.Key; });
    TArray<FString> Shares;
    for (int32 I = 0; I < FMath::Min(4, Chains.Num()); ++I)
        Shares.Add(FString::Printf(TEXT("%s %%%d"), *Chains[I].Value, Total > 0 ? FMath::RoundToInt32(100.f * Chains[I].Key / Total) : 0));
    TArray<FString> Types;
    for (const FString& Id : MarketBranches::FormatsIn(Country))
        if (!MarketBranches::FormatIds().Contains(Id)) Types.Add(MarketBranches::FormatInfo(Id).Name);
    return FString::Printf(TEXT("%s: %s En iyi giri\u015f illeri %s. Zincir paylar\u0131 (ma\u011faza say\u0131s\u0131yla): %s. %s"),
        *Pack.Name, *VerdictText(State, Country), *FString::Join(Places, TEXT(", ")), *FString::Join(Shares, TEXT(", ")),
        Types.Num() > 0 ? *FString::Printf(TEXT("Bu \u00fclkeye \u00f6zg\u00fc ma\u011faza t\u00fcr\u00fc: %s."), *FString::Join(Types, TEXT(", "))) : TEXT("D\u00f6rt bilinen ma\u011faza t\u00fcr\u00fc a\u00e7\u0131labilir."));
}

FString MarketResearch::StatusText(const FMarketState& State, const FString& Country)
{
    const FMarketResearch* R = MarketResearchLocal::Find(State, Country);
    switch (Status(State, Country))
    {
    case EStatus::NotNeeded: return FString();
    case EStatus::None: return FString::Printf(TEXT("Pazar ara\u015ft\u0131rmas\u0131 yok: %s, %d g\u00fcn s\u00fcrer, bir y\u0131l ge\u00e7erli."), *MarketCountry::Money(Cost(State, Country)), Days(State, Country));
    case EStatus::Running: return FString::Printf(TEXT("Pazar ara\u015ft\u0131rmas\u0131 s\u00fcr\u00fcyor: %d g\u00fcn kald\u0131."), R ? R->ReadyDay - State.Day : 0);
    case EStatus::Ready: return FString::Printf(TEXT("Pazar ara\u015ft\u0131rmas\u0131 haz\u0131r (%d g\u00fcn daha ge\u00e7erli). %s"), R ? R->ReadyDay + ValidDays - State.Day : 0, *Report(State, Country));
    default: return FString::Printf(TEXT("Pazar ara\u015ft\u0131rmas\u0131n\u0131n s\u00fcresi doldu; yenisi %s."), *MarketCountry::Money(Cost(State, Country)));
    }
}

void MarketResearch::CloseDay(FMarketState& State)
{
    for (const FMarketResearch& R : State.Company.Research)
        if (R.ReadyDay == State.Day && MarketResearchLocal::Needed(State, R.Country))
            State.DayNews.Add(FString::Printf(TEXT("Pazar ara\u015ft\u0131rmas\u0131 raporu geldi. %s Rapor bir y\u0131l ge\u00e7erli."), *Report(State, R.Country)));
}

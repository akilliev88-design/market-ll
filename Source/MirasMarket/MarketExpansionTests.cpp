#include "MarketBranches.h"
#include "MarketChains.h"
#include "MarketCountry.h"
#include "MarketResearch.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// M54: the country's own store types. M58: a market study before a new country. M59: enough firms in every country.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketExpansionTest, "MirasMarket.Expansion.TypesStudiesAndFirms", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketExpansionTest::RunTest(const FString& Parameters)
{
    using namespace MarketBranches;
    FMarketState S; S.RivalSeed = 11; S.Day = 900; S.Cash = 500000000; S.CountryId = MarketCountry::DefaultId(); S.Story.Chapter = 6;

    // M54: the four everywhere; a convenience store or a cash-and-carry only where the market knows it.
    TestEqual(TEXT("Four everywhere"), FormatIds().Num(), 4);
    TestEqual(TEXT("Six in all"), AllFormatIds().Num(), 6);
    FString WithConvenience, WithCashCarry;
    for (const MarketCountry::FProfile& P : MarketCountry::All())
    {
        if (WithConvenience.IsEmpty() && FormatsIn(P.Id).Contains(TEXT("yakin"))) WithConvenience = P.Id;
        if (WithCashCarry.IsEmpty() && FormatsIn(P.Id).Contains(TEXT("toptan"))) WithCashCarry = P.Id;
    }
    TestFalse(TEXT("A country with convenience stores"), WithConvenience.IsEmpty());
    TestFalse(TEXT("A country with cash-and-carry"), WithCashCarry.IsEmpty());
    const FFormat& Convenience = FormatInfo(TEXT("yakin"));
    const FFormat& CashCarry = FormatInfo(TEXT("toptan"));
    TestTrue(TEXT("Convenience: small baskets, dear but accepted"), Convenience.Basket < 1.f && Convenience.PriceTarget > 1.f && Convenience.Tolerance > 0.f);
    TestTrue(TEXT("Cash-and-carry: by the case, cheap, big provinces"), CashCarry.Basket > 1.f && CashCarry.PriceTarget < 1.f && CashCarry.MinPopulationK > 0);
    TestEqual(TEXT("Its ready-made store"), BaseFormat(TEXT("yakin")), FString(TEXT("kucuk")));
    TestEqual(TEXT("Its ready-made store"), BaseFormat(TEXT("toptan")), FString(TEXT("hiper")));
    TestEqual(TEXT("The four are themselves"), BaseFormat(TEXT("buyuk")), FString(TEXT("buyuk")));
    if (!FormatsIn(S.CountryId).Contains(TEXT("yakin")))
    {
        FString Why;
        TestFalse(TEXT("No convenience store where the market has none"), CanOpen(S, TArray<FMarketProduct>(), S.CountryId, MarketCountry::FindOrDefault(S.CountryId).Cities[0].Id, TEXT("yakin"), Why));
        TestFalse(TEXT("And it says why"), Why.IsEmpty());
    }
    if (!WithConvenience.IsEmpty())
    {
        FString Country, Province, Format;
        const FString City = MarketCountry::FindOrDefault(WithConvenience).Cities[0].Id;
        TestTrue(TEXT("Menu argument round trip"), DecodeSite(EncodeSite(WithConvenience, City, TEXT("yakin")), Country, Province, Format) && Format == TEXT("yakin") && Country == WithConvenience);
    }

    // M58: a country we have not entered needs a study; it costs, takes 30-45 days, is valid for a year.
    FString Abroad;
    for (const MarketCountry::FProfile& P : MarketCountry::All()) if (P.Id != S.CountryId && P.Cities.Num() > 0) { Abroad = P.Id; break; }
    if (TestFalse(TEXT("A foreign pack"), Abroad.IsEmpty()))
    {
        using MarketResearch::EStatus;
        FString Why;
        TestTrue(TEXT("Home needs none"), MarketResearch::Status(S, S.CountryId) == EStatus::NotNeeded);
        TestTrue(TEXT("None yet"), MarketResearch::Status(S, Abroad) == EStatus::None);
        TestFalse(TEXT("No entry without it"), MarketResearch::AllowsEntry(S, Abroad, Why));
        const int64 Cost = MarketResearch::Cost(S, Abroad);
        const int32 Days = MarketResearch::Days(S, Abroad);
        TestTrue(TEXT("It costs"), Cost > 0);
        TestTrue(TEXT("30-45 days"), Days >= 30 && Days <= 45);
        const int64 Other = S.OtherCosts;
        TestTrue(TEXT("Ordered"), MarketResearch::Start(S, Abroad, Why));
        TestEqual(TEXT("Paid at the close"), S.OtherCosts - Other, Cost);
        TestTrue(TEXT("Running"), MarketResearch::Status(S, Abroad) == EStatus::Running);
        TestFalse(TEXT("Not twice"), MarketResearch::CanStart(S, Abroad, Why));
        TestFalse(TEXT("Still no entry"), MarketResearch::AllowsEntry(S, Abroad, Why));
        S.Day += Days;
        S.DayNews.Reset();
        MarketResearch::CloseDay(S);
        TestTrue(TEXT("Ready"), MarketResearch::Status(S, Abroad) == EStatus::Ready && MarketResearch::AllowsEntry(S, Abroad, Why));
        TestTrue(TEXT("The report came"), S.DayNews.Num() == 1 && !MarketResearch::Report(S, Abroad).IsEmpty());
        S.Day += MarketResearch::ValidDays + 1;
        TestTrue(TEXT("A year later it is old"), MarketResearch::Status(S, Abroad) == EStatus::Expired && !MarketResearch::AllowsEntry(S, Abroad, Why));
        TestTrue(TEXT("A new one can be ordered"), MarketResearch::CanStart(S, Abroad, Why));
        FMarketSubsidiary Company; Company.Country = Abroad; S.Company.Subsidiaries.Add(Company);
        TestTrue(TEXT("Once entered, no study needed"), MarketResearch::Status(S, Abroad) == EStatus::NotNeeded);
    }

    // M59: 10-14 national chains in every pack, a regional chain in every sub-region, 15+ firms in a country's table.
    FString Counts;
    for (const MarketCountry::FProfile& P : MarketCountry::All())
    {
        if (P.Cities.Num() == 0) continue;
        TestTrue(*(P.Id + TEXT(": 10-14 national chains")), P.Roster.Num() >= 10 && P.Roster.Num() <= 14);
        MarketChains::EnsureCountry(S, P.Id);
        const int32 Firms = MarketChains::NationalTable(S, P.Id).FilterByPredicate([](const MarketChains::FStanding& R) { return !R.bUs; }).Num();
        TestTrue(*(P.Id + TEXT(": 15+ firms")), Firms >= 15);
        Counts += FString::Printf(TEXT(" %s %d"), *P.Id, Firms);
    }
    AddInfo(TEXT("OLCUM: ulkelerin firma sayisi:") + Counts);
    return true;
}

#endif

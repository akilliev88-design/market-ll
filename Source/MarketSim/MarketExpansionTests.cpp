#include "MarketBranches.h"
#include "MarketChains.h"
#include "MarketCountry.h"
#include "MarketResearch.h"
#include "MarketCompany.h"
#include "MarketFranchise.h"
#include "MarketStory.h"
#include "MarketPrices.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketExpansionTest
{
    // D6 (M67): 24 open branches in 5 home provinces: with the first store the company is big enough to go abroad.
    void GrowHome(FMarketState& S)
    {
        const MarketCountry::FProfile& Home = MarketCountry::FindOrDefault(S.CountryId);
        for (int32 I = 0; I < 24 && Home.Cities.Num() >= 5; ++I)
        {
            FMarketBranch B;
            B.Country = S.CountryId;
            B.Province = Home.Cities[I % 5].Id;
            B.Format = TEXT("mahalle");
            B.Name = FString::Printf(TEXT("Ev %d"), I + 1);
            B.Stage = static_cast<uint8>(MarketBranches::EStage::Open);
            B.OpenedDay = 1;
            S.Branches.Add(B);
        }
    }
}

// M54: the country's own store types. M58: a market study before a new country. M59: enough firms in every country.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketExpansionTest, "MarketSim.Expansion.TypesStudiesAndFirms", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketExpansionTest::RunTest(const FString& Parameters)
{
    using namespace MarketBranches;
    FMarketState S; S.RivalSeed = 11; S.Day = 900; S.Cash = 500000000; S.CountryId = MarketCountry::DefaultId();
    MarketExpansionTest::GrowHome(S); // D6 (M67): the way abroad opens with scale

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

    // M58: a country we have not entered needs a study; it costs, takes a few months (D8: 75-180 days), is valid for a year.
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
        TestTrue(TEXT("75-180 days"), Days >= 75 && Days <= 180);
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

// D6 (M67): the way abroad opens with the company's scale; a partner under our brand is a second way in.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketFranchiseTest, "MarketSim.Expansion.ScaleAndFranchise", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketFranchiseTest::RunTest(const FString& Parameters)
{
    FMarketState S; S.RivalSeed = 5; S.Day = 600; S.Cash = 500000000; S.CountryId = MarketCountry::DefaultId();
    FString Abroad;
    for (const MarketCountry::FProfile& P : MarketCountry::All()) if (P.Id != S.CountryId && P.Cities.Num() > 0) { Abroad = P.Id; break; }
    if (!TestFalse(TEXT("A foreign pack"), Abroad.IsEmpty())) return false;
    FString Why;
    TestFalse(TEXT("A small company does not open the world"), MarketCompany::AbroadOpen(S));
    TestFalse(TEXT("It says why"), MarketCompany::AbroadLock(S).IsEmpty());
    TestFalse(TEXT("No study yet"), MarketResearch::CanStart(S, Abroad, Why));
    TestFalse(TEXT("No partner yet"), MarketFranchise::CanSign(S, Abroad, Why));
    TestTrue(TEXT("World card: closed"), MarketCompany::CountryStatus(S, Abroad).StartsWith(TEXT("kapal\u0131")));

    MarketExpansionTest::GrowHome(S);
    TestTrue(TEXT("25 shops in 5 provinces open it"), MarketCompany::AbroadOpen(S) && MarketCompany::AbroadLock(S).IsEmpty());
    TestTrue(TEXT("Study"), MarketResearch::Start(S, Abroad, Why));
    TestFalse(TEXT("Not while it runs"), MarketFranchise::CanSign(S, Abroad, Why));
    S.Day += MarketResearch::Days(S, Abroad);

    const int64 Other = S.OtherCosts;
    const int64 Fee = MarketFranchise::Fee(S, Abroad);
    TestTrue(TEXT("Signed"), MarketFranchise::Sign(S, Abroad, Why));
    TestEqual(TEXT("The fee is paid at the close"), S.OtherCosts - Other, Fee);
    const FMarketFranchise* F = MarketFranchise::Find(S, Abroad);
    if (!TestNotNull(TEXT("Running"), F)) return false;
    TestEqual(TEXT("Three stores"), F->Stores, MarketFranchise::StartStores);
    TestFalse(TEXT("A partner name"), F->Partner.IsEmpty());
    TestFalse(TEXT("Not twice"), MarketFranchise::CanSign(S, Abroad, Why));
    TestEqual(TEXT("A foreign country now"), MarketCompany::ForeignPresence(S), 1);
    TestEqual(TEXT("No own store abroad"), MarketCompany::ForeignCountries(S), 0);
    TestTrue(TEXT("World card: the partner"), MarketCompany::CountryStatus(S, Abroad).Contains(F->Partner));
    TestTrue(TEXT("World card: home"), MarketCompany::CountryStatus(S, S.CountryId).Contains(TEXT("25 ma\u011faza")));

    // Days pass: sales gather, the royalty comes on the first of the month; a new store only once the partner has
    // saved its opening (no day rule), and never past the room the country has.
    const int64 Cash = S.Cash;
    for (int32 D = 0; D < 10; ++D) { ++S.Day; S.DayNews.Reset(); MarketFranchise::CloseDay(S); }
    F = MarketFranchise::Find(S, Abroad);
    TestEqual(TEXT("No store before the money is saved"), F->Stores, MarketFranchise::StartStores);
    TestTrue(TEXT("Saving"), F->Savings > 0 && F->Savings < MarketFranchise::OpeningCost(S));
    for (int32 D = 0; D < 400; ++D) { ++S.Day; S.DayNews.Reset(); MarketFranchise::CloseDay(S); }
    F = MarketFranchise::Find(S, Abroad);
    TestTrue(TEXT("A royalty came"), F->TotalRoyalty > 0 && S.Cash > Cash);
    TestTrue(TEXT("Less than the sales"), F->LastRoyalty < MarketFranchise::YearSales(S, Abroad) / 12);
    TestTrue(TEXT("The partner grew from its own profit"), F->Stores > MarketFranchise::StartStores && F->Stores <= MarketFranchise::MaxStores(Abroad));
    TestTrue(TEXT("Its stores share the country"), MarketFranchise::StoreDaySalesNow(S, *F) < FMath::RoundToInt64(MarketFranchise::StoreDaySales * MarketPrices::ListLevel(S.Day) * F->Quality / 100.0));
    const TArray<MarketChains::FStanding> Table = MarketChains::NationalTable(S, Abroad);
    const MarketChains::FStanding* Us = Table.FindByPredicate([](const MarketChains::FStanding& R) { return R.bUs; });
    TestTrue(TEXT("Our brand in the country's table"), Us && Us->Stores == F->Stores && Us->Revenue > 0.0);

    const int64 EndCost = MarketFranchise::EndCost(S, Abroad);
    TestTrue(TEXT("Ending costs at least the fee"), EndCost >= Fee);
    TestTrue(TEXT("Ended"), MarketFranchise::End(S, Abroad, Why));
    TestNull(TEXT("Gone"), MarketFranchise::Find(S, Abroad));
    TestEqual(TEXT("No foreign country"), MarketCompany::ForeignPresence(S), 0);
    TestTrue(TEXT("The world stays open by scale"), MarketCompany::AbroadOpen(S));

    // D8 (M68): the study takes a few months and says whether the country is worth it; a hard market is learned slowly.
    TestEqual(TEXT("Home is the yardstick"), MarketResearch::Attractiveness(S, S.CountryId), 1.f);
    int32 Good = 0, NotGood = 0;
    FString Scores;
    for (const MarketCountry::FProfile& P : MarketCountry::All())
    {
        if (P.Id == S.CountryId || P.Cities.Num() == 0) continue;
        const float Score = MarketResearch::Attractiveness(S, P.Id);
        TestTrue(*(P.Id + TEXT(": a score off the limits")), Score > 0.3f && Score < 2.f); // 04.10.2026: a unit slip put six packs on the floor
        TestTrue(*(P.Id + TEXT(": a few months")), MarketResearch::Days(S, P.Id) >= 75 && MarketResearch::Days(S, P.Id) <= 180);
        TestFalse(*(P.Id + TEXT(": a verdict")), MarketResearch::VerdictText(S, P.Id).IsEmpty());
        (MarketResearch::Verdict(S, P.Id) == MarketResearch::EVerdict::Good ? Good : NotGood) += 1;
        Scores += FString::Printf(TEXT(" %s %.2f"), *P.Id, Score);
        if (MarketResearch::Verdict(S, P.Id) != MarketResearch::EVerdict::Good)
            TestTrue(*(P.Id + TEXT(": a hard market is learned slowly")), MarketResearch::LearningFactor(S, P.Id) > 1.f);
    }
    TestTrue(TEXT("Some countries are worth it, some are not"), Good > 0 && NotGood > 0);
    AddInfo(TEXT("OLCUM: ulke cekiciligi:") + Scores);
    return true;
}

#endif

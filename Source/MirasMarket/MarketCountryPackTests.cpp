#include "MarketCountry.h"
#include "MarketCalendar.h"
#include "MarketChains.h"
#include "MarketDepartments.h"
#include "MarketDirector.h"
#include "MarketEconomy.h"
#include "MarketEras.h"
#include "MarketPayments.h"
#include "MarketSimulation.h"
#include "MarketStart.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS

// D3/D5 (karar M52, Docs/Kurgu/10_ULKE_STANDARDI.md): every country is a pack; the engine knows no country.
namespace MarketCountryPackTest
{
    FMarketProduct Make(const TCHAR* Id, const TCHAR* Category, int64 Cost, int64 Price)
    {
        FMarketProduct P;
        P.Id = Id; P.RealName = Id; P.Category = Category; P.Cost = Cost; P.BasePrice = Price; P.CaseUnits = 12; P.bActive = true;
        return P;
    }

    TArray<FMarketProduct> Catalog()
    {
        return {
            Make(TEXT("sut"), TEXT("s\u00fct"), 170, 250),
            Make(TEXT("ayran"), TEXT("s\u00fct"), 60, 100),
            Make(TEXT("makarna"), TEXT("makarna-bakliyat"), 210, 325),
            Make(TEXT("cay"), TEXT("\u00e7ay-kahve"), 480, 675),
            Make(TEXT("cola"), TEXT("i\u00e7ecek"), 180, 275),
            Make(TEXT("deterjan"), TEXT("temizlik"), 1400, 1990),
        };
    }

    // The packs D5 added (each must be in the file and pass the standard).
    const TArray<FString>& NewPacks()
    {
        static const TArray<FString> Ids = { TEXT("fr"), TEXT("es"), TEXT("pl"), TEXT("br"), TEXT("mx"), TEXT("jp") };
        return Ids;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCountryPackStandardTest, "MirasMarket.Country.PackStandard", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCountryPackStandardTest::RunTest(const FString& Parameters)
{
    using namespace MarketCountry;
    // The file parses without a single complaint.
    FString Json;
    TestTrue(TEXT("Config/ulkeler.json read"), FFileHelper::LoadFileToString(Json, *FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("ulkeler.json"))));
    TArray<FProfile> Parsed; TArray<FString> Errors;
    TestTrue(TEXT("Packs parse"), Parse(Json, Parsed, Errors));
    for (const FString& Error : Errors) AddError(TEXT("Paket hatasi: ") + Error);
    TArray<FGiantRow> Giants; TArray<FString> GiantErrors;
    TestTrue(TEXT("Giants parse"), ParseGiants(Json, Giants, GiantErrors) && GiantErrors.Num() == 0);

    TestNotNull(TEXT("The default country has a pack"), Find(DefaultId()));
    for (const FString& Id : MarketCountryPackTest::NewPacks()) TestNotNull(*(TEXT("D5 pack ") + Id), Find(Id));

    TSet<FString> Ids, ChainIds;
    for (const FProfile& P : All())
    {
        const FString W = P.Id + TEXT(": ");
        bool bAlready = false;
        Ids.Add(P.Id, &bAlready);
        TestFalse(*(W + TEXT("unique id")), bAlready);
        // Identity and money.
        TestFalse(*(W + TEXT("name")), P.Name.IsEmpty() || P.NameEn.IsEmpty());
        TestTrue(*(W + TEXT("currency code")), P.CurrencyCode.Len() == 3 && !P.CurrencySymbol.IsEmpty() && !P.CurrencyName.IsEmpty());
        TestTrue(*(W + TEXT("decimal mark")), P.DecimalMark == TEXT(',') || P.DecimalMark == TEXT('.'));
        TestTrue(*(W + TEXT("display scale and rate")), P.DisplayScale > 0.0 && P.FxPerWorld > 0.0);
        TestTrue(*(W + TEXT("economy")), P.InflationMean >= 0.0 && P.InflationMean < 0.5 && P.InflationVol >= 0.0 && P.LoanSpread >= 0.0 && P.WageFactor > 0.f && P.RentFactor > 0.f);
        // Traditional trade, chains, firms.
        TestFalse(*(W + TEXT("grocer and market")), P.GrocerName.IsEmpty() || P.MarketName.IsEmpty());
        for (const TCHAR* Key : { TEXT("bim"), TEXT("a101"), TEXT("sok"), TEXT("migros") })
            TestTrue(*(W + TEXT("street chain ") + Key), P.Chains.Contains(Key) && !P.Chains[Key].IsEmpty());
        TestTrue(*(W + TEXT("at least three national chains")), P.Roster.Num() >= 3);
        for (const FRosterRow& Row : P.Roster)
        {
            MarketChains::EArchetype Archetype = MarketChains::EArchetype::Super;
            TestTrue(*(W + Row.Id + TEXT(" archetype")), MarketChains::ArchetypeOf(Row.Archetype, Archetype));
            bool bTaken = false;
            ChainIds.Add(Row.Id, &bTaken);
            TestFalse(*(W + Row.Id + TEXT(" chain id unique in the world")), bTaken);
            TestTrue(*(W + Row.Id + TEXT(" numbers")), Row.Stores > 0 && Row.PriceIndex > 0.5f && Row.PriceIndex < 1.6f && Row.Service > 0.3f);
            TestTrue(*(W + Row.Id + TEXT(" home region")), Row.HomeRegion.IsEmpty() || P.Regions.ContainsByPredicate([&Row](const FRegion& R) { return R.Id == Row.HomeRegion; }));
        }
        TestTrue(*(W + TEXT("regional chain words")), P.RegionalSuffixes.Num() > 0);
        TestFalse(*(W + TEXT("firm words")), P.WholesaleWord.IsEmpty() || P.CashCarryWord.IsEmpty());
        TestEqual(*(W + TEXT("four banks")), P.Banks.Num(), 4);
        TestFalse(*(W + TEXT("delivery platform")), P.PlatformName.IsEmpty());
        // People.
        TestTrue(*(W + TEXT("name pools")), P.FirstNames.Num() >= 6 && P.LastNames.Num() >= 6);
        TestTrue(*(W + TEXT("manager pools")), (P.ManagerFirst.Num() == 0) == (P.ManagerLast.Num() == 0));
        TestTrue(*(W + TEXT("staff pools")), (P.StaffFirst.Num() == 0) == (P.StaffLast.Num() == 0));
        TestTrue(*(W + TEXT("relatives")), P.Relatives.Num() > 0);
        // Calendar and climate.
        TestTrue(*(W + TEXT("climate")), P.ClimateTemperature.Num() == 12 && P.ClimateRain.Num() == 12);
        TestTrue(*(W + TEXT("holidays")), P.Holidays.Num() >= 3);
        for (const FHoliday& H : P.Holidays)
            TestTrue(*(W + H.Name + TEXT(" has a date")), !H.Name.IsEmpty() && MarketCalendar::HolidayStart(H, 2015) != MIN_int32);
        TestTrue(*(W + TEXT("card habit")), P.CardShare > 0.f && P.CardShare < 1.f);
        // Places: every province in one sub-region, every sub-region in a main region.
        TestTrue(*(W + TEXT("provinces")), P.Cities.Num() >= 3);
        TestTrue(*(W + TEXT("regions")), P.Regions.Num() > 0 && P.SubRegions.Num() > 0);
        TSet<FString> Cities;
        for (const FCity& C : P.Cities)
        {
            bool bTwice = false;
            Cities.Add(C.Id, &bTwice);
            TestFalse(*(W + C.Id + TEXT(" unique province")), bTwice);
            TestTrue(*(W + C.Id + TEXT(" population")), C.PopulationK > 0 && !C.Name.IsEmpty());
            TestFalse(*(W + C.Id + TEXT(" in a region")), C.SubRegion.IsEmpty() || C.Region.IsEmpty());
            TestTrue(*(W + C.Id + TEXT(" derived values")), C.Income > 0.f && C.Rent > 0.f && C.Competition > 0.f);
        }
        for (const FRegion& Sub : P.SubRegions)
        {
            TestTrue(*(W + Sub.Id + TEXT(" parent")), P.Regions.ContainsByPredicate([&Sub](const FRegion& R) { return R.Id == Sub.Parent; }));
            for (const FString& Province : Sub.Provinces) TestTrue(*(W + Sub.Id + TEXT(" lists ") + Province), Cities.Contains(Province));
        }
    }
    // The world giants: a known kind, and a home pack that holds its home arm.
    for (const FGiantRow& G : Giants)
    {
        MarketChains::EArchetype Archetype = MarketChains::EArchetype::Super;
        TestTrue(*(G.Id + TEXT(" giant archetype")), MarketChains::ArchetypeOf(G.Archetype, Archetype));
        if (G.Pack.IsEmpty()) continue;
        const FProfile* Home = Find(G.Pack);
        TestNotNull(*(G.Id + TEXT(" home pack")), Home);
        if (Home) TestTrue(*(G.Id + TEXT(" home arm")), Home->Roster.ContainsByPredicate([&G](const FRosterRow& R) { return R.Id == G.Id || R.Name == G.Name; }));
    }
    // The chains of every pack reach the league roster.
    TestEqual(TEXT("National roster = all packs"), MarketChains::NationalRoster().Num(), ChainIds.Num());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCountryTurkeyPackTest, "MirasMarket.Country.TurkeyFromPack", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCountryTurkeyPackTest::RunTest(const FString& Parameters)
{
    // D3: the Turkish rules that were code branches now come from the tr pack and give the same days as before.
    using MarketCalendar::ETag;
    using MarketCalendar::GameDayOf;
    MarketCountry::SetActive(TEXT("tr"), 1);
    const MarketCountry::FProfile& Tr = MarketCountry::Active();
    TestEqual(TEXT("Turkey is the default"), MarketCountry::DefaultId(), FString(TEXT("tr")));
    TestTrue(TEXT("Built-in price curve"), Tr.bBuiltinCurve);
    TestTrue(TEXT("Real chain names until the brand switch"), Tr.bRealChainNames);
    for (const int32 Key : { 101, 423, 501, 519, 830, 1029 })
        TestTrue(*FString::Printf(TEXT("National day %d"), Key), MarketCalendar::Info(GameDayOf(2013, Key / 100, Key % 100), 3).Has(ETag::NationalHoliday));
    TestFalse(TEXT("No other national day"), MarketCalendar::Info(GameDayOf(2013, 10, 3), 3).Has(ETag::NationalHoliday));
    // Ramazan Bayrami 2012: 19 August (Ramadan the 29 days before); Kurban Bayrami 2012: 25 October, 4 days.
    TestTrue(TEXT("Ramadan"), MarketCalendar::Info(GameDayOf(2012, 8, 10), 3).Has(ETag::Ramadan));
    TestFalse(TEXT("Not yet Ramadan"), MarketCalendar::Info(GameDayOf(2012, 7, 20), 3).Has(ETag::Ramadan));
    TestTrue(TEXT("Arife"), MarketCalendar::Info(GameDayOf(2012, 8, 18), 3).Has(ETag::BayramEve));
    TestTrue(TEXT("Ramazan Bayrami 3 days"), MarketCalendar::Info(GameDayOf(2012, 8, 21), 3).Has(ETag::Bayram) && !MarketCalendar::Info(GameDayOf(2012, 8, 22), 3).Has(ETag::Bayram));
    TestTrue(TEXT("Kurban Bayrami 4 days"), MarketCalendar::Info(GameDayOf(2012, 10, 28), 3).Has(ETag::Bayram) && !MarketCalendar::Info(GameDayOf(2012, 10, 29), 3).Has(ETag::Bayram));
    TestTrue(TEXT("No holiday name in Turkey"), MarketCalendar::Info(GameDayOf(2012, 10, 25), 3).HolidayName.IsEmpty());
    TestFalse(TEXT("Describe says bayram"), MarketCalendar::Describe(GameDayOf(2012, 10, 25), 3).Contains(TEXT("Kurban")));
    // School: third Monday of September, second Friday of June (2011: 19 September; 2012: 8 June).
    TestTrue(TEXT("School starts"), MarketCalendar::Info(GameDayOf(2011, 9, 19), 3).Has(ETag::SchoolStart));
    TestTrue(TEXT("Report day"), MarketCalendar::Info(GameDayOf(2012, 6, 8), 3).Has(ETag::SchoolEnd));
    TestFalse(TEXT("Shops open on Sundays"), MarketCalendar::ClosedByLaw(7));
    // Cards: the Turkish yearly curve.
    TestTrue(TEXT("Card use 2011"), FMath::IsNearlyEqual(MarketPayments::CardShare(1), 0.25f));
    TestTrue(TEXT("Card use 2020"), FMath::IsNearlyEqual(MarketPayments::CardShare(GameDayOf(2020, 6, 1)), 0.72f));
    // The butcher's week before Kurban Bayrami.
    using MarketDepartments::EDept;
    const float Before = MarketDepartments::SeasonFactor(EDept::Butcher, GameDayOf(2012, 10, 20));
    const float Plain = MarketDepartments::SeasonFactor(EDept::Butcher, GameDayOf(2012, 10, 10));
    TestTrue(TEXT("Butcher peak"), FMath::IsNearlyEqual(Before, Plain * 1.8f, 0.001f));
    // The national chains and the regional chain words.
    TestTrue(TEXT("BIN first in the roster"), MarketChains::NationalRoster().Num() > 0 && MarketChains::NationalRoster()[0].Id == TEXT("bim") && MarketChains::NationalRoster()[0].Country == TEXT("tr"));
    TestEqual(TEXT("Four regional words"), Tr.RegionalSuffixes.Num(), 4);
    TestTrue(TEXT("Staff names"), Tr.StaffFirst.Num() == 24 && Tr.StaffLast.Num() == 14);
    // The map is Turkey's (drawn from iller.json).
    TestEqual(TEXT("Map country"), MarketCountry::MapCountry(), FString(TEXT("tr")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCountrySmokeEveryPackTest, "MirasMarket.Country.SmokeEveryPack", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCountrySmokeEveryPackTest::RunTest(const FString& Parameters)
{
    // Every pack: a family shop founded there and played for 30 days (MarketSimulation::PlayDay, the same rules as
    // the game). No cash outside the books, no stock gap, no negative stock.
    using namespace MarketCountryPackTest;
    const TArray<FMarketProduct> Base = Catalog();
    for (const MarketCountry::FProfile& Pack : MarketCountry::All())
    {
        const FString W = Pack.Id + TEXT(": ");
        TArray<FMarketProduct> Products = Base;
        FMarketState S; S.Initialize(Base);
        MarketStart::Setup(S, Pack.Id, FString(), 31);
        TestEqual(*(W + TEXT("country set")), S.CountryId, Pack.Id);
        TestTrue(*(W + TEXT("start province")), MarketCountry::FindCity(S.CountryId, MarketStart::HomeProvince(S)) != nullptr);
        MarketCountry::SetActive(S.CountryId, S.RivalSeed);
        MarketEras::Activate(S);
        S.ApplyShelfCapacities({ 24, 24, 24, 24, 24, 24 });
        MarketStart::StockShelvesPartly(S, 31);
        MarketDirector::ApplyPrices(S, Base, Products);
        int32 Shoppers = 0, Failures = 0, Gaps = 0;
        bool bStockOk = true;
        for (int32 D = 1; D <= 30; ++D)
        {
            const MarketSimulation::FDay Day = MarketSimulation::PlayDay(S, Base, Products);
            Shoppers += Day.Shoppers;
            Failures += Day.AuditFailures;
            if (D > 1 && S.Ledger.LastGap != 0) ++Gaps;
            for (const FMarketStock& Item : S.Stock) bStockOk &= Item.Shelf >= 0 && Item.Warehouse >= 0 && Item.Dock >= 0 && Item.Incoming >= 0 && Item.Shelf <= Item.Capacity;
        }
        TestEqual(*(W + TEXT("30 days played")), S.Day, 31);
        TestTrue(*(W + TEXT("people came")), Shoppers > 0);
        TestEqual(*(W + TEXT("no stock or till gap")), Failures, 0);
        TestEqual(*(W + TEXT("books agree every day")), Gaps, 0);
        TestTrue(*(W + TEXT("no negative stock")), bStockOk);
        TestTrue(*(W + TEXT("money shown in its currency")), MarketCountry::Money(12345).Contains(Pack.CurrencySymbol));
    }
    MarketCountry::SetActive(MarketCountry::DefaultId(), 1);
    return true;
}

#endif

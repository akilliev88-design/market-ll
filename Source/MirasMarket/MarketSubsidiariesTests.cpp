#include "MarketSubsidiaries.h"
#include "MarketCalendar.h"
#include "MarketCountry.h"
#include "MarketLedger.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// M65: one brand, a company in every country; the registered names follow the country; profits go to the parent.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketSubsidiariesTest, "MirasMarket.Subsidiaries.BrandCompaniesAndTransfers", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketSubsidiariesTest::RunTest(const FString& Parameters)
{
    using namespace MarketSubsidiaries;
    FMarketState S; S.RivalSeed = 3; S.Day = 40; S.Cash = 10000000; S.CountryId = MarketCountry::DefaultId();
    S.Company.BrandName = TEXT("Deniz Market"); // M69: the player names the market at the start
    const MarketCountry::FProfile& HomePack = MarketCountry::FindOrDefault(S.CountryId);

    // The parent company at home: the brand + the country's first legal form, no cost.
    TestTrue(TEXT("Parent founded"), Ensure(S, S.CountryId, false));
    TestFalse(TEXT("Only once"), Ensure(S, S.CountryId, false));
    if (HomePack.LegalForms.Num() > 0)
        TestEqual(TEXT("Parent's registered name"), LegalName(S, S.CountryId), Brand(S) + TEXT(" ") + HomePack.LegalForms[0]);
    TestEqual(TEXT("No cost at home"), SetupCost(S, S.CountryId), int64(0));

    // A country abroad: a subsidiary with that country's form and a founding cost.
    FString Abroad;
    for (const MarketCountry::FProfile& P : MarketCountry::All()) if (P.Id != S.CountryId && P.LegalForms.Num() > 1) { Abroad = P.Id; break; }
    if (!TestFalse(TEXT("A foreign pack with legal forms"), Abroad.IsEmpty())) return true;
    const MarketCountry::FProfile& Pack = MarketCountry::FindOrDefault(Abroad);
    const int64 Cost = SetupCost(S, Abroad);
    TestTrue(TEXT("Founding costs money"), Cost > 0);
    const int64 OtherBefore = S.OtherCosts;
    TestTrue(TEXT("Subsidiary founded"), Ensure(S, Abroad));
    TestEqual(TEXT("Cost paid at the close"), S.OtherCosts - OtherBefore, Cost);
    TestEqual(TEXT("Its registered name"), LegalName(S, Abroad), Brand(S) + TEXT(" ") + Pack.LegalForms[0]);
    TestEqual(TEXT("No second cost"), SetupCost(S, Abroad), int64(0));

    // The player names it; nonsense is refused; the suggestions walk the country's forms.
    FString Message;
    TestEqual(TEXT("Next suggestion"), NextSuggestion(S, Abroad), Brand(S) + TEXT(" ") + Pack.LegalForms[1]);
    TestFalse(TEXT("Empty refused"), SetLegalName(S, Abroad, TEXT("   "), Message));
    TestFalse(TEXT("Too long refused"), SetLegalName(S, Abroad, FString::ChrN(MaxNameLength + 1, TEXT('a')), Message));
    TestTrue(TEXT("Own name"), SetLegalName(S, Abroad, TEXT("Deniz Market  Avrupa   Holding"), Message));
    TestEqual(TEXT("Spaces tidied"), LegalName(S, Abroad), FString(TEXT("Deniz Market Avrupa Holding")));

    // A new brand: names that carry the old one follow it.
    TestTrue(TEXT("New brand"), SetBrand(S, TEXT("Yildiz"), Message));
    TestEqual(TEXT("Brand"), Brand(S), FString(TEXT("Yildiz")));
    TestEqual(TEXT("Subsidiary follows"), LegalName(S, Abroad), FString(TEXT("Yildiz Avrupa Holding")));
    TestTrue(TEXT("Parent follows"), LegalName(S, S.CountryId).StartsWith(TEXT("Yildiz")));

    // Rivals: everybody says the name; the registered one carries a form of their country.
    FMarketChain Chain; Chain.Id = TEXT("rival.x"); Chain.Name = TEXT("Aldo"); Chain.Country = Abroad;
    const FString Registered = ChainLegalName(Chain);
    TestTrue(TEXT("Rival's registered name"), Registered.StartsWith(TEXT("Aldo ")) && Pack.LegalForms.ContainsByPredicate([&Registered](const FString& F) { return Registered.EndsWith(F); }));
    TestEqual(TEXT("Stable"), ChainLegalName(Chain), Registered);

    // The month's profit of the subsidiary's stores goes to the parent; the country's withholding is paid.
    FMarketBranch Store; Store.Country = Abroad; Store.Stage = 3; S.Branches.Add(Store);
    MarketLedger::Post(S, MarketLedger::EAccount::Sales, 100000, true, 0);
    const MarketCalendar::FDate Date = MarketCalendar::DateOf(S.Day);
    S.Day = Date.Month == 12 ? MarketCalendar::GameDayOf(Date.Year + 1, 1, 1) : MarketCalendar::GameDayOf(Date.Year, Date.Month + 1, 1);
    const int64 CashBefore = S.Cash;
    CloseDay(S);
    const FMarketSubsidiary* Sub = Find(S, Abroad);
    const int64 Withheld = FMath::RoundToInt64(100000 * static_cast<double>(Pack.DividendWithholding));
    TestTrue(TEXT("Last month's profit"), Sub && Sub->LastMonthProfit == 100000);
    TestTrue(TEXT("Withholding"), Sub && Sub->LastWithheld == Withheld && Sub->LastTransfer == 100000 - Withheld && Sub->TotalTransferred == 100000 - Withheld);
    TestEqual(TEXT("Only the tax leaves the till"), CashBefore - S.Cash, Withheld);
    return true;
}

#endif

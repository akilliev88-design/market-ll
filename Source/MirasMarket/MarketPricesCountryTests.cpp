#include "MarketPrices.h"
#include "MarketCountry.h"
#include "MarketEconomy.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// E1 (M51, Docs/Kurgu/11_TEK_EKONOMI.md): an economy per country.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketPricesCountryTest, "MirasMarket.Prices.EveryCountryItsOwn", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketPricesCountryTest::RunTest(const FString& Parameters)
{
    MarketCountry::SetActive(TEXT("tr"), 7);
    FMarketState S; S.Initialize(TArray<FMarketProduct>());
    S.CountryId = TEXT("tr"); S.RivalSeed = 7; S.Day = 1;
    const int32 Decade = 3650;

    // The own country: the same numbers as the single-argument functions.
    TestEqual(TEXT("Home list level unchanged"), MarketPrices::ListLevel(TEXT("tr"), Decade), MarketPrices::ListLevel(Decade));
    TestEqual(TEXT("Empty country = home"), MarketPrices::WageIndex(FString(), Decade), MarketPrices::WageIndex(Decade));
    TestEqual(TEXT("Home loan rate unchanged"), MarketPrices::LoanRate(TEXT("tr"), Decade), MarketPrices::LoanRate(Decade));
    TestEqual(TEXT("Home to home"), MarketPrices::ToHome(S, TEXT("tr"), Decade), 1.0);

    const bool bGermany = MarketCountry::Find(TEXT("de")) != nullptr;
    TestTrue(TEXT("A second country in the packs"), bGermany);
    if (!bGermany) return false;

    // A stable economy next to a high-inflation one.
    TestEqual(TEXT("Every country starts at 1"), MarketPrices::PriceLevel(TEXT("de"), 1), 1.0);
    TestTrue(TEXT("Germany's prices rise slower"), MarketPrices::PriceLevel(TEXT("de"), Decade) < MarketPrices::PriceLevel(TEXT("tr"), Decade));
    TestTrue(TEXT("Germany's prices still rise"), MarketPrices::PriceLevel(TEXT("de"), Decade) > 1.05);
    TestTrue(TEXT("Its own wage index"), MarketPrices::WageIndex(TEXT("de"), Decade) > 1.0 && MarketPrices::WageIndex(TEXT("de"), Decade) < MarketPrices::WageIndex(TEXT("tr"), Decade));
    TestTrue(TEXT("Cheaper money"), MarketPrices::LoanRate(TEXT("de"), Decade) < MarketPrices::LoanRate(TEXT("tr"), Decade));
    TestTrue(TEXT("Scaled by its own level"), MarketPrices::Scaled(TEXT("de"), 10000, Decade) < MarketPrices::Scaled(TEXT("tr"), 10000, Decade));
    // The same call twice gives the same economy (seeded, cached).
    TestEqual(TEXT("Stable per campaign"), MarketPrices::PriceLevel(TEXT("de"), 2000), MarketPrices::PriceLevel(TEXT("de"), 2000));

    // Exchange: 1 at the start; a euro buys more lira as Turkish prices run ahead.
    TestTrue(TEXT("Exchange 1 at the start"), FMath::IsNearlyEqual(MarketPrices::ToHome(S, TEXT("de"), 1), 1.0, 1e-6));
    TestTrue(TEXT("The euro gains on the lira"), MarketPrices::ToHome(S, TEXT("de"), Decade) > 1.2);
    // In real terms most of it is only the inflation gap; what is left is the currency's risk (within a band).
    const double Real = MarketPrices::RealToHome(S, TEXT("de"), Decade);
    TestTrue(TEXT("Real exchange within a band"), Real > 0.4 && Real < 2.5);
    return true;
}

#endif

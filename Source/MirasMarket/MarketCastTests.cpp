#include "MarketCast.h"
#include "MarketCountry.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCastNamesTest, "MirasMarket.Cast.NamesFromTheCountry", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCastNamesTest::RunTest(const FString& Parameters)
{
    MarketCountry::FProfile P;
    P.Id = TEXT("xx");
    P.FirstNames = { TEXT("Anna"), TEXT("Lukas"), TEXT("Mia"), TEXT("Jonas"), TEXT("Lea"), TEXT("Paul") };
    P.LastNames = { TEXT("M\u00fcller"), TEXT("Schmidt"), TEXT("Schneider"), TEXT("Fischer"), TEXT("Weber"), TEXT("Meyer"), TEXT("Wagner"), TEXT("Becker") };
    P.Banks = { TEXT("Stadtsparkasse"), TEXT("Hansa Handelsbank"), TEXT("Rhein Invest"), TEXT("F\u00f6rderbank") };
    MarketCountry::SetActiveProfile(P, 42);
    TestTrue(TEXT("Names from the pack"), P.LastNames.Contains(MarketCast::LastName(MarketCast::ERole::Wholesaler)));
    TestTrue(TEXT("The wholesaler's firm carries its family name"), MarketCast::Wholesaler().StartsWith(MarketCast::LastName(MarketCast::ERole::Wholesaler)));
    TestNotEqual(TEXT("The cash and carry is another family"), MarketCast::LastName(MarketCast::ERole::Wholesaler), MarketCast::LastName(MarketCast::ERole::CashCarry));
    TestEqual(TEXT("Same campaign, same names"), MarketCast::Wholesaler(), MarketCast::Wholesaler());
    TestEqual(TEXT("The country's bank"), MarketCast::Bank(1), FString(TEXT("Hansa Handelsbank")));
    MarketCountry::SetActiveProfile(P, 43);
    const FString Other = MarketCast::Wholesaler();
    MarketCountry::SetActiveProfile(P, 42);
    TestTrue(TEXT("Another campaign may meet other people"), !Other.IsEmpty());
    MarketCountry::FProfile Empty;
    MarketCountry::SetActiveProfile(Empty, 1);
    TestFalse(TEXT("A pack without banks: a plain name"), MarketCast::Bank(0).IsEmpty());
    MarketCountry::SetActive(TEXT("tr"), 1);
    return true;
}

#endif

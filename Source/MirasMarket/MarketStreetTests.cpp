#include "MarketCompetitors.h"
#include "MarketChains.h"
#include "MarketRivals.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStreetTest, "MirasMarket.Competitors.StreetFromProvinceChains", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStreetTest::RunTest(const FString& Parameters)
{
    // M35: the street's chain shops are the home province's real chains; a chain that leaves takes its shop along.
    using namespace MarketCompetitors;
    FMarketState S;
    S.RivalSeed = 3; S.Day = 40;
    S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
    auto AddChain = [&S](const TCHAR* Id, const TCHAR* Name, MarketChains::EArchetype Kind, const TCHAR* Province, int32 Stores)
    {
        FMarketChain C; C.Id = Id; C.Name = Name; C.Country = TEXT("tr"); C.Archetype = static_cast<uint8>(Kind);
        FMarketChainSpot Spot; Spot.Province = Province; Spot.Stores = Stores; C.Spots.Add(Spot);
        S.Rivals.Chains.Add(C);
    };
    AddChain(TEXT("big"), TEXT("Uzak Ucuz"), MarketChains::EArchetype::Discount, TEXT("istanbul"), 3000);
    AddChain(TEXT("near"), TEXT("Yak\u0131n Ucuz"), MarketChains::EArchetype::Discount, TEXT("kirklareli"), 12);
    AddChain(TEXT("super"), TEXT("B\u00fcy\u00fck S\u00fcper"), MarketChains::EArchetype::Super, TEXT("kirklareli"), 4);
    Ensure(S);
    TestEqual(TEXT("The discounter on our street is the one in our province"), DisplayName(ECompany::Bim), FString(TEXT("Yak\u0131n Ucuz")));
    TestEqual(TEXT("The supermarket"), DisplayName(ECompany::Migros), FString(TEXT("B\u00fcy\u00fck S\u00fcper")));
    TestEqual(TEXT("The daily news use the same names"), MarketRivals::RivalName(0), DisplayName(ECompany::Bim));
    TestTrue(TEXT("Open"), IsOpen(S, ECompany::Bim));

    S.Rivals.Chains[1].bGone = true;
    S.DayNews.Reset();
    Bind(S);
    TestFalse(TEXT("Its chain left: the shop closed"), IsOpen(S, ECompany::Bim));
    TestTrue(TEXT("Told"), S.DayNews.Num() > 0);
    return true;
}

#endif

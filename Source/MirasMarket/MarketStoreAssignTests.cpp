#include "MarketStoreAssign.h"
#include "MarketBranches.h"
#include "MarketPrices.h"
#include "Algo/Reverse.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketStoreAssignTest
{
    TArray<FString> Templates(const FString& Format, int32 Count = 5)
    {
        TArray<FString> Ids;
        for (int32 Index = 1; Index <= Count; ++Index) Ids.Add(FString::Printf(TEXT("%s_%02d"), *Format, Index));
        return Ids;
    }

    TArray<FString> Provinces()
    {
        return {
            TEXT("Adana"), TEXT("Ankara"), TEXT("Antalya"), TEXT("Bursa"), TEXT("\u00c7anakkale"),
            TEXT("Diyarbak\u0131r"), TEXT("Edirne"), TEXT("Erzurum"), TEXT("Eski\u015fehir"), TEXT("Gaziantep"),
            TEXT("I\u011fd\u0131r"), TEXT("\u0130stanbul"), TEXT("\u0130zmir"), TEXT("Kayseri"), TEXT("K\u0131rklareli"),
            TEXT("Konya"), TEXT("Mu\u011fla"), TEXT("Samsun"), TEXT("\u015eanl\u0131urfa"), TEXT("Tekirda\u011f"),
        };
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStoreAssignKeyTest, "MirasMarket.StoreAssign.SiteKey", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStoreAssignKeyTest::RunTest(const FString& Parameters)
{
    using namespace MarketStoreAssign;
    TestEqual(TEXT("Folded, lower case"), SiteKey(TEXT("TR"), TEXT("Tekirda\u011f"), TEXT("mahalle")), FString(TEXT("tr|tekirdag|mahalle")));
    TestEqual(TEXT("Upper case Turkish letters"), SiteKey(TEXT("tr"), TEXT("TEK\u0130RDA\u011e"), TEXT("MAHALLE")), FString(TEXT("tr|tekirdag|mahalle")));
    TestEqual(TEXT("Dotted capital I"), SiteKey(TEXT("tr"), TEXT("\u0130stanbul"), TEXT("buyuk")), FString(TEXT("tr|istanbul|buyuk")));
    TestEqual(TEXT("Dotless i and capital I"), SiteKey(TEXT("tr"), TEXT("I\u011fd\u0131r"), TEXT("kucuk")), FString(TEXT("tr|igdir|kucuk")));
    TestEqual(TEXT("Spaces trimmed"), SiteKey(TEXT(" tr "), TEXT(" \u015eanl\u0131urfa "), TEXT("hiper")), FString(TEXT("tr|sanliurfa|hiper")));
    TestEqual(TEXT("Abroad: one view per country and format"), SiteKey(TEXT("bg"), FString(), TEXT("kucuk")), FString(TEXT("bg||kucuk")));
    TestTrue(TEXT("Hash is stable for a text"), StableHash(TEXT("tr|tekirdag|mahalle")) == StableHash(FString(TEXT("tr|tekirdag|mahalle"))));
    TestTrue(TEXT("Hash tells texts apart"), StableHash(TEXT("tr|tekirdag|mahalle")) != StableHash(TEXT("tr|tekirdag|kucuk")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStoreAssignPickTest, "MirasMarket.StoreAssign.Pick", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStoreAssignPickTest::RunTest(const FString& Parameters)
{
    using namespace MarketStoreAssign;
    using namespace MarketStoreAssignTest;
    const TArray<FString> Ids = Templates(TEXT("mahalle"));
    const FString First = Pick(TEXT("tr"), TEXT("Tekirda\u011f"), TEXT("mahalle"), Ids);
    TestTrue(TEXT("Picks one of the list"), Ids.Contains(First));
    TestEqual(TEXT("Same input, same view"), Pick(TEXT("tr"), TEXT("Tekirda\u011f"), TEXT("mahalle"), Ids), First);
    TestEqual(TEXT("Spelling does not matter"), Pick(TEXT("TR"), TEXT("tekirdag"), TEXT("mahalle"), Ids), First);
    TArray<FString> Reversed = Ids;
    Algo::Reverse(Reversed);
    TestEqual(TEXT("Order of the list does not matter"), Pick(TEXT("tr"), TEXT("Tekirda\u011f"), TEXT("mahalle"), Reversed), First);
    TestEqual(TEXT("Empty list gives nothing"), Pick(TEXT("tr"), TEXT("Tekirda\u011f"), TEXT("mahalle"), TArray<FString>()), FString());

    // 20 provinces over 5 views of every format: all views used, none takes the lion's share.
    for (const FString& Format : MarketBranches::FormatIds())
    {
        const TArray<FString> FormatTemplates = Templates(Format);
        TMap<FString, int32> Count;
        for (const FString& Province : Provinces()) Count.FindOrAdd(Pick(TEXT("tr"), Province, Format, FormatTemplates))++;
        TestEqual(*FString::Printf(TEXT("%s: every view used"), *Format), Count.Num(), 5);
        int32 Most = 0;
        for (const TPair<FString, int32>& Pair : Count) Most = FMath::Max(Most, Pair.Value);
        TestTrue(*FString::Printf(TEXT("%s: no view gets more than 8 of 20"), *Format), Most <= 8);
    }

    // A sixth view moves only a few sites, and only onto the new view.
    const TArray<FString> Six = Templates(TEXT("mahalle"), 6);
    int32 Moved = 0;
    for (const FString& Province : Provinces())
    {
        const FString Before = Pick(TEXT("tr"), Province, TEXT("mahalle"), Ids);
        const FString After = Pick(TEXT("tr"), Province, TEXT("mahalle"), Six);
        if (Before != After)
        {
            ++Moved;
            TestEqual(TEXT("A moved site goes to the new view"), After, FString(TEXT("mahalle_06")));
        }
    }
    TestTrue(TEXT("A new view moves at most 8 of 20 sites"), Moved <= 8);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStoreAssignResolveTest, "MirasMarket.StoreAssign.ResolveKeepsSave", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStoreAssignResolveTest::RunTest(const FString& Parameters)
{
    using namespace MarketStoreAssign;
    using namespace MarketStoreAssignTest;
    const TArray<FString> Ids = Templates(TEXT("buyuk"));
    const FString Seeded = Pick(TEXT("tr"), TEXT("\u0130zmir"), TEXT("buyuk"), Ids);
    const FString Other = Seeded == Ids[0] ? Ids[1] : Ids[0];

    TMap<FString, FString> Saved;
    TestEqual(TEXT("Nothing saved: the seeded view"), Resolve(Saved, TEXT("tr"), TEXT("\u0130zmir"), TEXT("buyuk"), Ids), Seeded);
    Saved.Add(SiteKey(TEXT("tr"), TEXT("\u0130zmir"), TEXT("buyuk")), Other);
    TestEqual(TEXT("The saved view is kept"), Resolve(Saved, TEXT("TR"), TEXT("izmir"), TEXT("buyuk"), Ids), Other);

    TArray<FString> Without = Ids;
    Without.Remove(Other);
    TestEqual(TEXT("A view that left the list is picked again"), Resolve(Saved, TEXT("tr"), TEXT("\u0130zmir"), TEXT("buyuk"), Without), Pick(TEXT("tr"), TEXT("\u0130zmir"), TEXT("buyuk"), Without));

    TMap<FString, FString> Fresh;
    const FString Assigned = Assign(Fresh, TEXT("tr"), TEXT("Konya"), TEXT("kucuk"), Templates(TEXT("kucuk")));
    TestEqual(TEXT("Assign writes the choice"), Fresh.FindRef(FString(TEXT("tr|konya|kucuk"))), Assigned);
    TestEqual(TEXT("Missing store file keeps the save"), Assign(Fresh, TEXT("tr"), TEXT("Konya"), TEXT("kucuk"), TArray<FString>()), Assigned);
    TestEqual(TEXT("... and changes nothing"), Fresh.Num(), 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStoreAssignCategoryTest, "MirasMarket.StoreAssign.ShelfCategory", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStoreAssignCategoryTest::RunTest(const FString& Parameters)
{
    using namespace MarketStoreAssign;
    TMap<FString, FString> Choices;
    const FString Front = FaceKey(TEXT("gondol_1"));
    const FString Back = FaceKey(TEXT("gondol_1"), true);
    TestTrue(TEXT("Two faces, two keys"), Front != Back);

    TestEqual(TEXT("No choice: the template's category"), GetCategory(Choices, Front, TEXT("kuru g\u0131da")), FString(TEXT("kuru g\u0131da")));
    SetCategory(Choices, Back, TEXT("i\u00e7ecek"));
    TestEqual(TEXT("Back face chosen"), GetCategory(Choices, Back, TEXT("kuru g\u0131da")), FString(TEXT("i\u00e7ecek")));
    TestEqual(TEXT("Front face untouched"), GetCategory(Choices, Front, TEXT("kuru g\u0131da")), FString(TEXT("kuru g\u0131da")));
    SetCategory(Choices, Front, TEXT("KATEGOR\u0130S\u0130Z"));
    TestEqual(TEXT("Uncategorized stored as the constant"), GetCategory(Choices, Front, TEXT("kuru g\u0131da")), FString(Uncategorized));
    TestTrue(TEXT("... and reads as uncategorized"), IsUncategorized(GetCategory(Choices, Front, TEXT("kuru g\u0131da"))));
    SetCategory(Choices, Front, FString());
    TestTrue(TEXT("Empty choice is uncategorized"), IsUncategorized(GetCategory(Choices, Front, TEXT("kuru g\u0131da"))));
    ClearCategory(Choices, Front);
    TestEqual(TEXT("Clear returns to the template"), GetCategory(Choices, Front, TEXT("kuru g\u0131da")), FString(TEXT("kuru g\u0131da")));

    TestTrue(TEXT("Upper case Turkish matches"), CategoryMatches(TEXT("\u0130\u00c7ECEK"), TEXT("i\u00e7ecek")));
    TestTrue(TEXT("ASCII spelling matches"), CategoryMatches(TEXT("kuru gida"), TEXT("kuru g\u0131da")));
    TestTrue(TEXT("Spaces ignored"), CategoryMatches(TEXT(" s\u00fct "), TEXT("S\u00dcT")));
    TestFalse(TEXT("Other category"), CategoryMatches(TEXT("s\u00fct"), TEXT("i\u00e7ecek")));
    TestFalse(TEXT("Uncategorized shelf takes nothing"), CategoryMatches(Uncategorized, TEXT("kategorisiz")));
    TestFalse(TEXT("Empty shelf category takes nothing"), CategoryMatches(FString(), FString()));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStoreAssignNominalTest, "MirasMarket.StoreAssign.NominalIsNeutral", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStoreAssignNominalTest::RunTest(const FString& Parameters)
{
    using namespace MarketStoreAssign;
    const float Tolerance = 0.001f;
    for (const FString& Format : MarketBranches::FormatIds())
    {
        const MarketBranches::FFormat& Kind = MarketBranches::FormatInfo(Format);
        for (int32 Pass = 0; Pass < 2; ++Pass)
        {
            // Pass 0: the nominal store; pass 1: no stats at all (store file missing) must behave the same.
            const MarketStoreAssign::FStoreMeasures Stats = Pass == 0 ? Nominal(Format) : MarketStoreAssign::FStoreMeasures();
            const FString Label = FString::Printf(TEXT("%s/%s"), *Format, Pass == 0 ? TEXT("nominal") : TEXT("empty"));
            TestEqual(*(Label + TEXT(" variety")), VarietyFactor(Stats, Format), 1.f, Tolerance);
            TestEqual(*(Label + TEXT(" queue")), QueueFactor(Stats, Format, NominalShoppers(Format)), 1.f, Tolerance);
            TestEqual(*(Label + TEXT(" queue, default shoppers")), QueueFactor(Stats, Format, 0.f), 1.f, Tolerance);
            TestEqual(*(Label + TEXT(" fresh")), FreshFactor(Stats, Format), 1.f, Tolerance);
            TestEqual(*(Label + TEXT(" spoil")), SpoilFactor(Stats, Format), 1.f, Tolerance);
            TestEqual(*(Label + TEXT(" rent")), RentFactor(Stats, Format), 1.f, Tolerance);
            TestEqual(*(Label + TEXT(" fit-out")), FitOutFactor(Stats, Format), 1.f, Tolerance);
            TestEqual(*(Label + TEXT(" backroom")), BackroomFactor(Stats, Format), 1.f, Tolerance);
            TestEqual(*(Label + TEXT(" workers")), WorkersFor(Stats, Format), Kind.Workers);
            TestEqual(*(Label + TEXT(" fit-out money")), FitOutCost(Stats, Format, 1), MarketPrices::Scaled(Kind.FitOut, 1));
            TestEqual(*(Label + TEXT(" rent money")), BaseMonthlyRent(Stats, Format, 1), MarketPrices::Scaled(Kind.Rent, 1));
        }
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStoreAssignSizeTest, "MirasMarket.StoreAssign.SizeMatters", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStoreAssignSizeTest::RunTest(const FString& Parameters)
{
    using namespace MarketStoreAssign;
    // The largest mahalle of the band against the smallest.
    MarketStoreAssign::FStoreMeasures Big = Nominal(TEXT("mahalle"));
    Big.SalesAreaM2 = 200.f; Big.ShelfFrontM = 60.f; Big.Checkouts = 2; Big.CoolerM = 8.f; Big.FreezerM = 2.f;
    MarketStoreAssign::FStoreMeasures Small = Nominal(TEXT("mahalle"));
    Small.SalesAreaM2 = 85.f; Small.ShelfFrontM = 26.f; Small.CoolerM = 3.f; Small.FreezerM = 1.f; Small.ProduceM2 = 1.f;
    TestTrue(TEXT("Big store pays more rent"), RentFactor(Big, TEXT("mahalle")) > 1.f);
    TestTrue(TEXT("Small store pays less rent"), RentFactor(Small, TEXT("mahalle")) < 1.f);
    TestTrue(TEXT("Big store holds more variety"), VarietyFactor(Big, TEXT("mahalle")) > 1.f);
    TestTrue(TEXT("Small store holds less"), VarietyFactor(Small, TEXT("mahalle")) < 1.f);
    TestTrue(TEXT("Big store costs more to fit out"), FitOutFactor(Big, TEXT("mahalle")) > FitOutFactor(Small, TEXT("mahalle")));
    TestTrue(TEXT("Big store needs more people"), WorkersFor(Big, TEXT("mahalle")) > WorkersFor(Small, TEXT("mahalle")));
    TestTrue(TEXT("More cold room sells more fresh"), FreshFactor(Big, TEXT("mahalle")) > 1.f && FreshFactor(Small, TEXT("mahalle")) < 1.f);
    TestTrue(TEXT("Little cold room spoils more"), SpoilFactor(Small, TEXT("mahalle")) > 1.f);

    MarketStoreAssign::FStoreMeasures Huge = Nominal(TEXT("mahalle"));
    Huge.SalesAreaM2 = 5000.f; Huge.ShelfFrontM = 900.f;
    TestEqual(TEXT("Rent factor is clamped"), RentFactor(Huge, TEXT("mahalle")), 1.6f, 0.001f);
    TestEqual(TEXT("Variety factor is clamped"), VarietyFactor(Huge, TEXT("mahalle")), 1.5f, 0.001f);
    TestTrue(TEXT("Workers at most twice the format's"), WorkersFor(Huge, TEXT("mahalle")) <= 2 * MarketBranches::FormatInfo(TEXT("mahalle")).Workers);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStoreAssignQueueTest, "MirasMarket.StoreAssign.QueueAtTills", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStoreAssignQueueTest::RunTest(const FString& Parameters)
{
    using namespace MarketStoreAssign;
    // A supermarket's catchment has 520 trips a day; a branch of ours expects 0.3 of them.
    const float Expected = NominalShoppers(TEXT("buyuk"));
    TestEqual(TEXT("Nominal shoppers: 0.3 of the catchment"), Expected, 0.3f * static_cast<float>(MarketBranches::FormatInfo(TEXT("buyuk")).Trips), 0.001f);
    TestTrue(TEXT("Catchment-sized traffic is not a normal day"), QueueFactor(Nominal(TEXT("buyuk")), TEXT("buyuk"), static_cast<float>(MarketBranches::FormatInfo(TEXT("buyuk")).Trips)) < 1.f);
    MarketStoreAssign::FStoreMeasures Few = Nominal(TEXT("buyuk"));
    Few.Checkouts = 3;
    MarketStoreAssign::FStoreMeasures Many = Nominal(TEXT("buyuk"));
    Many.Checkouts = 8;
    MarketStoreAssign::FStoreMeasures SelfTills = Nominal(TEXT("buyuk"));
    SelfTills.Checkouts = 3; SelfTills.SelfCheckouts = 6;

    const float QFew = QueueFactor(Few, TEXT("buyuk"), Expected);
    TestEqual(TEXT("Half the tills: 25% lost at the queue"), QFew, 0.75f, 0.001f);
    TestTrue(TEXT("More tills never hurt"), QueueFactor(Many, TEXT("buyuk"), Expected) >= 1.f);
    TestTrue(TEXT("At most a small bonus"), QueueFactor(Many, TEXT("buyuk"), Expected) <= 1.05f);
    TestTrue(TEXT("Self checkouts help"), QueueFactor(SelfTills, TEXT("buyuk"), Expected) > QFew);
    TestTrue(TEXT("A busier day queues more"), QueueFactor(Nominal(TEXT("buyuk")), TEXT("buyuk"), 1.5f * Expected) < 1.f);
    MarketStoreAssign::FStoreMeasures One = Nominal(TEXT("buyuk"));
    One.Checkouts = 1;
    TestEqual(TEXT("Queue loss is clamped"), QueueFactor(One, TEXT("buyuk"), 3.f * Expected), 0.6f, 0.001f);
    return true;
}

#endif

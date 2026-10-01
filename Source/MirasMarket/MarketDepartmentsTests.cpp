#include "MarketDepartments.h"
#include "MarketBranches.h"
#include "MarketCalendar.h"
#include "MarketEconomy.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketDepartmentsTest
{
    FMarketState Start()
    {
        FMarketState S; S.Initialize(TArray<FMarketProduct>());
        S.RivalSeed = 9; S.Day = 40; S.Cash = 500000000; S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
        const TCHAR* Formats[3] = { TEXT("mahalle"), TEXT("buyuk"), TEXT("hiper") };
        for (const TCHAR* Format : Formats)
        {
            FMarketBranch& B = S.Branches.AddDefaulted_GetRef();
            B.Name = Format; B.Format = Format; B.Stage = static_cast<uint8>(MarketBranches::EStage::Open); B.OpenedDay = 1; B.LastShoppers = 300;
        }
        return S;
    }

    int32 DayIn(int32 Month) { return MarketCalendar::GameDayOf(MarketCalendar::DateOf(1).Year + 1, Month, 10); }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketDepartmentsPolicyTest, "MirasMarket.Departments.PolicyAndSpace", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketDepartmentsPolicyTest::RunTest(const FString& Parameters)
{
    using namespace MarketDepartmentsTest;
    using MarketDepartments::EDept;
    FMarketState S = Start();
    FString Message;

    // Store types: the greengrocer from the neighbourhood market, the butcher from the supermarket, electronics in the hypermarket.
    TestTrue(TEXT("Greengrocer in a neighbourhood market"), MarketDepartments::CanSet(S, EDept::Produce, 1, true, Message));
    TestFalse(TEXT("No butcher in a neighbourhood market"), MarketDepartments::CanSet(S, EDept::Butcher, 1, true, Message));
    TestFalse(TEXT("No electronics in a supermarket"), MarketDepartments::CanSet(S, EDept::Electronics, 2, true, Message));
    TestFalse(TEXT("Nothing in a discounter"), MarketDepartments::CanSet(S, EDept::Produce, 0, true, Message));
    int32 Arg = MarketDepartments::EncodeSet(EDept::Seasonal, 3, true);
    EDept Dept = EDept::Produce; int32 Format = 0; bool bOn = false;
    TestTrue(TEXT("Decode"), MarketDepartments::DecodeSet(Arg, Dept, Format, bOn) && Dept == EDept::Seasonal && Format == 3 && bOn);

    // Turning the butcher on opens it at once in the supermarket, pays fit-out and stock, hires a master.
    const int64 Before = S.Cash;
    TestTrue(TEXT("Butcher on"), MarketDepartments::Set(S, EDept::Butcher, 2, true, Message));
    TestEqual(TEXT("Opened in the supermarket"), S.Branches[1].Depts.Num(), 1);
    TestEqual(TEXT("Not in the hypermarket"), S.Branches[2].Depts.Num(), 0);
    TestTrue(TEXT("Paid"), S.Cash < Before);
    TestTrue(TEXT("A master"), S.Branches[1].Depts[0].Master >= 35);
    TestTrue(TEXT("It pulls shoppers"), MarketDepartments::PullFactor(S.Branches[1], S.Day + 40) > 1.f);

    // The supermarket's floor is limited: all five fresh departments do not fit.
    TestTrue(TEXT("Greengrocer"), MarketDepartments::Set(S, EDept::Produce, 2, true, Message));
    TestTrue(TEXT("Bakery"), MarketDepartments::Set(S, EDept::Bakery, 2, true, Message));
    TestFalse(TEXT("Deli does not fit"), MarketDepartments::CanSet(S, EDept::Deli, 2, true, Message));
    TestTrue(TEXT("Space counted"), MarketDepartments::SpaceUsed(S, 2) == 19);

    // A new supermarket gets the policy at the day close.
    FMarketBranch& New = S.Branches.AddDefaulted_GetRef();
    New.Name = TEXT("yeni"); New.Format = TEXT("buyuk"); New.Stage = static_cast<uint8>(MarketBranches::EStage::Open); New.OpenedDay = S.Day;
    MarketDepartments::CloseDay(S);
    TestEqual(TEXT("New branch opened with the departments"), S.Branches[3].Depts.Num(), 3);

    // Turning off sells the stock at a loss.
    const int64 Cash = S.Cash;
    TestTrue(TEXT("Butcher off"), MarketDepartments::Set(S, EDept::Butcher, 2, false, Message));
    TestTrue(TEXT("Refund"), S.Cash > Cash);
    TestEqual(TEXT("Gone"), MarketDepartments::BranchesWith(S, EDept::Butcher), 0);
    TestTrue(TEXT("Branch line"), MarketDepartments::BranchLine(S.Branches[1]).Contains(TEXT("Manav")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketDepartmentsDayTest, "MirasMarket.Departments.DaySeasonAndStance", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketDepartmentsDayTest::RunTest(const FString& Parameters)
{
    using namespace MarketDepartmentsTest;
    using MarketDepartments::EDept;
    FMarketState S = Start();
    FString Message;
    TestTrue(TEXT("Toys on"), MarketDepartments::Set(S, EDept::Toys, 3, true, Message));
    TestTrue(TEXT("Stationery on"), MarketDepartments::Set(S, EDept::Stationery, 3, true, Message));
    for (FMarketBranchDept& D : S.Branches[2].Depts) D.OpenedDay = 1; // past the first month

    // Seasons: toys at New Year, stationery in September.
    TestTrue(TEXT("Toys peak in December"), MarketDepartments::SeasonFactor(EDept::Toys, DayIn(12)) > 2.f * MarketDepartments::SeasonFactor(EDept::Toys, DayIn(6)));
    TestTrue(TEXT("Stationery peaks in September"), MarketDepartments::SeasonFactor(EDept::Stationery, DayIn(9)) > 2.f);
    const MarketDepartments::FDay June = MarketDepartments::Day(S, 2, 1000, 1.f, DayIn(6));
    const MarketDepartments::FDay December = MarketDepartments::Day(S, 2, 1000, 1.f, DayIn(12));
    TestTrue(TEXT("A day sells"), June.Revenue > 0);
    TestTrue(TEXT("December sells more"), December.Revenue > June.Revenue);
    TestTrue(TEXT("Goods bought"), June.Purchases > 0 && June.Purchases < June.Revenue);

    // Stance: cheap sells more, earns a thinner margin.
    TestTrue(TEXT("Cheap"), MarketDepartments::SetStance(S, EDept::Toys, 0, Message));
    TestTrue(TEXT("Dear"), MarketDepartments::SetStance(S, EDept::Stationery, 2, Message));
    TestEqual(TEXT("Stance kept"), MarketDepartments::Stance(S, EDept::Toys), 0);
    const MarketDepartments::FDay Cheap = MarketDepartments::Day(S, 2, 1000, 1.f, DayIn(6));
    TestTrue(TEXT("Revenue moved"), Cheap.Revenue != June.Revenue);
    TestTrue(TEXT("Results line"), MarketDepartments::Results(S, EDept::Toys).Contains(TEXT("1")));

    // Masters: a weak one is found and replaced.
    TestTrue(TEXT("Fish on"), MarketDepartments::Set(S, EDept::Fish, 3, true, Message));
    for (FMarketBranchDept& D : S.Branches[2].Depts) if (D.Dept == static_cast<uint8>(EDept::Fish)) D.Master = 30;
    TestEqual(TEXT("One weak master"), MarketDepartments::WeakMasters(S, EDept::Fish), 1);
    TestTrue(TEXT("Replaced"), MarketDepartments::ReplaceWeakMasters(S, EDept::Fish, Message));
    TestEqual(TEXT("No weak master left"), MarketDepartments::WeakMasters(S, EDept::Fish), 0);
    return true;
}

#endif

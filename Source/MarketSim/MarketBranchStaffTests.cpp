#include "MarketStaff.h"
#include "MarketCountry.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// E3c2c (M63): a branch's cashiers and stockers are people, run by its manager.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBranchStaffTest, "MarketSim.Staff.BranchPeople", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketBranchStaffTest::RunTest(const FString& Parameters)
{
    FMarketState S; S.RivalSeed = 17; S.Day = 60; S.Cash = 10000000; S.CountryId = MarketCountry::DefaultId();
    FMarketBranch B; B.Name = TEXT("Test"); B.Workers = 4; B.Stage = 3; B.ManagerName = TEXT("M"); B.ManagerSkill = 60;
    S.Branches.Add(B);

    // The positions are filled: half at the tills.
    TestEqual(TEXT("Four hired"), MarketStaff::StaffBranch(S, 0), 4);
    int32 Cashiers = 0;
    int64 Wages = 0;
    TSet<int32> Ids;
    for (const FMarketEmployee& E : S.Branches[0].Staff)
    {
        Cashiers += MarketStaff::RoleOf(E) == MarketStaff::ERole::Cashier ? 1 : 0;
        Wages += E.DailyWage;
        Ids.Add(E.Id);
    }
    TestEqual(TEXT("Two cashiers"), Cashiers, 2);
    TestEqual(TEXT("Every person a new id"), Ids.Num(), 4);
    TestEqual(TEXT("Wages are the people's"), MarketStaff::BranchWages(S.Branches[0]), Wages);
    TestEqual(TEXT("Nothing more to fill"), MarketStaff::StaffBranch(S, 0), 0);

    // A skilled, happy team serves better than a tired, unhappy one.
    FMarketBranch Good = S.Branches[0], Poor = S.Branches[0];
    for (FMarketEmployee& E : Good.Staff) { E.Skill = 90; E.Morale = 90.f; E.Fatigue = 0.f; }
    for (FMarketEmployee& E : Poor.Staff) { E.Skill = 20; E.Morale = 20.f; E.Fatigue = 90.f; }
    const float GoodService = MarketStaff::BranchService(Good, S.Day);
    const float PoorService = MarketStaff::BranchService(Poor, S.Day);
    TestTrue(TEXT("Team matters"), GoodService > PoorService && GoodService <= 1.15f && PoorService >= 0.75f);
    FMarketBranch Empty = S.Branches[0]; Empty.Staff.Reset();
    TestEqual(TEXT("Nobody on duty"), MarketStaff::BranchService(Empty, S.Day), 0.75f);

    // Someone whose notice ran out leaves; the manager hires a replacement the same day.
    const int32 Leaving = S.Branches[0].Staff[1].Id;
    S.Branches[0].Staff[1].LeaveDay = S.Day - 1;
    const MarketStaff::FBranchStaffDay Day = MarketStaff::BranchDay(S, 0, 200, 600000, 400, S.Day - 1);
    TestTrue(TEXT("One left, one hired"), Day.Left == 1 && Day.Hired == 1);
    TestEqual(TEXT("Positions full again"), S.Branches[0].Staff.Num(), 4);
    TestFalse(TEXT("The leaver is gone"), S.Branches[0].Staff.ContainsByPredicate([Leaving](const FMarketEmployee& E) { return E.Id == Leaving; }));
    for (const FMarketEmployee& E : S.Branches[0].Staff) TestTrue(TEXT("Minimum wage kept"), E.DailyWage >= MarketStaff::MinimumDailyWage(S.Day - 1));

    // Unhappy people give notice after three bad days.
    for (FMarketEmployee& E : S.Branches[0].Staff) { E.Morale = 5.f; E.LowMoraleDays = 2; E.DailyWage = MarketStaff::MinimumDailyWage(S.Day); E.Fatigue = 100.f; }
    MarketStaff::BranchDay(S, 0, 200, 600000, 400, S.Day);
    TestTrue(TEXT("Notices given"), S.Branches[0].Staff.ContainsByPredicate([](const FMarketEmployee& E) { return E.LeaveDay > 0; }));

    // A store abroad hires people of its own country.
    if (MarketCountry::Find(TEXT("de")) && MarketCountry::DefaultId() != TEXT("de"))
    {
        FMarketBranch Abroad; Abroad.Name = TEXT("Berlin"); Abroad.Country = TEXT("de"); Abroad.Workers = 2; Abroad.Stage = 3;
        S.Branches.Add(Abroad);
        MarketStaff::StaffBranch(S, S.Branches.Num() - 1);
        TArray<FString> First, Last;
        MarketStaff::StaffNames(MarketCountry::FindOrDefault(TEXT("de")), First, Last);
        FString FirstName, Rest;
        S.Branches.Last().Staff[0].Name.Split(TEXT(" "), &FirstName, &Rest);
        TestTrue(TEXT("A local name"), First.Contains(FirstName));
    }
    return true;
}

#endif

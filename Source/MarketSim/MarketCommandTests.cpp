#include "MarketCommand.h"
#include "MarketBranches.h"
#include "MarketManagers.h"
#include "MarketCalendar.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketCommandTest
{
    FMarketState Start()
    {
        FMarketState S;
        S.RivalSeed = 4; S.Cash = 100000000;
        S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
        S.Day = MarketCalendar::GameDayOf(2014, 6, 1);
        FMarketBranch B;
        B.Country = TEXT("tr"); B.Province = TEXT("tekirdag"); B.Name = TEXT("Tekirda\u011f Mahalle 1");
        B.Stage = static_cast<uint8>(MarketBranches::EStage::Open); B.OpenedDay = S.Day - 400;
        B.ManagerName = TEXT("Deneme \u015eube"); B.ManagerSkill = 80; B.ManagerStyle = static_cast<uint8>(MarketManagers::EStyle::PriceMinded);
        for (int32 I = 0; I < 3; ++I) { FMarketStock Item = FMarketStock::Empty(); Item.Id = FString::Printf(TEXT("p%d"), I); Item.Capacity = 40; Item.Shelf = 30; Item.IdleDays = 12; B.Items.Add(Item); }
        S.Branches.Add(B);
        return S;
    }

    void AddBoss(FMarketState& S)
    {
        FMarketManager M;
        M.Level = static_cast<uint8>(MarketManagers::ELevel::Province); M.Country = TEXT("tr"); M.Area = TEXT("tekirdag");
        M.Name = TEXT("Deneme \u0130l"); M.Skill = 80; M.AppointedDay = 1;
        S.Management.Managers.Add(M);
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCommandTest, "MarketSim.Command.ClearanceAndProposals", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCommandTest::RunTest(const FString& Parameters)
{
    // M33: everyday things stay below, critical ones come up the line.
    using namespace MarketCommandTest;
    const TArray<FMarketProduct> Products;
    FMarketState Alone = Start();
    MarketBranches::Clearance(Alone, 0, Products);
    TestEqual(TEXT("Without a province manager a deep cut stops at 20 %"), static_cast<int32>(Alone.Branches[0].Items[0].Markdown), 20);
    FMarketState Led = Start();
    AddBoss(Led);
    const FString Line = MarketBranches::Clearance(Led, 0, Products);
    TestEqual(TEXT("The province manager approves 30 %"), static_cast<int32>(Led.Branches[0].Items[0].Markdown), 30);
    TestTrue(TEXT("Told in the weekly line"), Line.Contains(TEXT("Deneme \u0130l")));
    TestEqual(TEXT("No card for a clearance"), Led.Decisions.Num(), 0);

    // Three months in the red: a closing proposal, brought up; approved, the branch closes.
    FMarketState S = Start();
    AddBoss(S);
    S.Branches[0].Last30Profit = -5000;
    S.Branches[0].LossMonths = MarketCommand::LossMonthsToClose - 1;
    MarketCommand::CloseDay(S, Products);
    const int32 Card = S.Decisions.IndexOfByPredicate([](const FMarketDecision& D) { return D.Id == TEXT("command.close:0"); });
    TestTrue(TEXT("A closing proposal"), Card != INDEX_NONE);
    if (Card == INDEX_NONE) return false;
    FMarketState Kept = S;
    FString Message;
    TestTrue(TEXT("Turned down"), MarketCommand::Resolve(Kept, Products, Kept.Decisions[Card], 1, Message) && Kept.Branches[0].QuietUntil > Kept.Day);
    TestTrue(TEXT("Approved"), MarketCommand::Resolve(S, Products, S.Decisions[Card], 0, Message) && S.Branches[0].Stage == static_cast<uint8>(MarketBranches::EStage::Closed));

    // Without a province manager nobody proposes.
    FMarketState Nobody = Start();
    Nobody.Branches[0].Last30Profit = -5000;
    Nobody.Branches[0].LossMonths = 5;
    MarketCommand::CloseDay(Nobody, Products);
    TestEqual(TEXT("No proposal without a province manager"), Nobody.Decisions.Num(), 0);
    return true;
}

#endif

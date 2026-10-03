#include "MarketBranchVisit.h"
#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBranchVisitProjection, "MirasMarket.BranchVisit.ProjectionAndIsolation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FBranchVisitProjection::RunTest(const FString& Parameters)
{
    FMarketState State; FMarketBranch Branch;
    FMarketStock Item = FMarketStock::Empty(); Item.Id = TEXT("tea"); Item.Shelf = 12; Item.Capacity = 24; Branch.Items.Add(Item);
    State.Branches.Add(Branch); const auto Before = MarketBranchVisit::StateBytes(State);
    TestEqual(TEXT("Half stock shows half of visual shelf"), MarketBranchVisit::Fill(State.Branches[0],TEXT("tea")),.5f);
    TestEqual(TEXT("Absent product leaves shelf empty"), MarketBranchVisit::Fill(Branch,TEXT("missing")),0.f);
    Branch.Items[0].Shelf = 0; TestEqual(TEXT("Empty shelf stays empty"),MarketBranchVisit::Fill(Branch,TEXT("tea")),0.f);
    Branch.Items[0].Shelf = 99; TestEqual(TEXT("Over-capacity is bounded visually"),MarketBranchVisit::Fill(Branch,TEXT("tea")),1.f);
    Branch.LastShoppers = 120; Branch.LastQueueLost = 15; Branch.Workers = 3;
    TestEqual(TEXT("Visit shoppers follow last-day traffic"),MarketBranchVisit::Shoppers(Branch),6);
    TestEqual(TEXT("Queue follows last-day queue losses"),MarketBranchVisit::Queue(Branch),3);
    TestEqual(TEXT("Worker count follows branch"),MarketBranchVisit::Workers(Branch),3);
    Branch.LastShoppers = Branch.LastQueueLost = Branch.Workers = -1;
    TestEqual(TEXT("Invalid negative traffic shows no crowd"),MarketBranchVisit::Shoppers(Branch),0);
    TestEqual(TEXT("Invalid negative queue shows none"),MarketBranchVisit::Queue(Branch),0);
    TestEqual(TEXT("No invented workers"),MarketBranchVisit::Workers(Branch),0);
    TestTrue(TEXT("Projection does not mutate campaign"),Before == MarketBranchVisit::StateBytes(State));
    return true;
}
#endif
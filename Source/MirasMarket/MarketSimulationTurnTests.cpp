#include "MarketSimulation.h"
#include "MarketCalendar.h"
#include "MarketManagers.h"
#include "MarketEvents.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace MarketTurnTests
{
    TArray<FMarketProduct> Base()
    {
        FMarketProduct Product; Product.Id = TEXT("pasta"); Product.Category = TEXT("makarna-bakliyat"); Product.Cost = 100; Product.BasePrice = 200;
        return { Product };
    }
    FMarketState State(const TArray<FMarketProduct>& Products, int32 Day = 3)
    {
        FMarketState Value; Value.Initialize(Products); Value.Cash = 10000000; Value.Day = Day;
        Value.Story.bEnded = true; Value.RivalSeed = 21;
        return Value;
    }
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketTurnLengths, "MirasMarket.Simulation.TurnLengthsAndStops", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketTurnLengths::RunTest(const FString& Parameters)
{
    const auto Base = MarketTurnTests::Base();
    auto State = MarketTurnTests::State(Base);
    TestEqual(TEXT("Week is seven days even on Wednesday"), MarketSimulation::RequestedDays(State, MarketSimulation::ETurn::Week), 7);
    State.Day = MarketCalendar::GameDayOf(2012, 2, 27);
    TestEqual(TEXT("Leap February includes the 29th"), MarketSimulation::RequestedDays(State, MarketSimulation::ETurn::Month), 3);
    State.Day = MarketCalendar::GameDayOf(2011, 4, 1);
    TestEqual(TEXT("April has thirty days"), MarketSimulation::RequestedDays(State, MarketSimulation::ETurn::Month), 30);
    State.Day = MarketCalendar::GameDayOf(2011, 3, 31);
    TestEqual(TEXT("Last day of month is included"), MarketSimulation::RequestedDays(State, MarketSimulation::ETurn::Month), 1);
    bool Completed = false;
    for (int32 Seed = 1; Seed <= 32 && !Completed; ++Seed)
    {
        auto Trial = MarketTurnTests::State(Base); Trial.RivalSeed = Seed; auto Products = Base;
        const auto Turn = MarketSimulation::AdvanceTurn(Trial, Base, Products, MarketSimulation::ETurn::Week);
        if (Turn.Played == 7)
        {
            Completed = true;
            TestEqual(TEXT("Crossed intermediate report to day ten"), Trial.Day, 10);
            TestTrue(TEXT("Typed week report"), Turn.Stop == MarketSimulation::EStop::WeekReport);
            TestEqual(TEXT("All seven days recorded"), Turn.Period.RecordedDays, 7);
        }
    }
    TestTrue(TEXT("An uninterrupted actual seven-day turn completed"), Completed);
    auto Products = Base; State = MarketTurnTests::State(Base);
    State.Cash = -1;
    auto Turn = MarketSimulation::AdvanceTurn(State, Base, Products, MarketSimulation::ETurn::Month);
    TestEqual(TEXT("Negative opening cash plays zero days"), Turn.Played, 0);
    TestTrue(TEXT("Typed negative cash reason"), Turn.Stop == MarketSimulation::EStop::NegativeCash);
    State.Cash = 10000000;
    FMarketDecision Decision; Decision.Id = TEXT("event.test"); Decision.Options = {TEXT("OK")}; Decision.Deadline = State.Day + 5;
    MarketEvents::Offer(State, Decision);
    Turn = MarketSimulation::AdvanceTurn(State, Base, Products, MarketSimulation::ETurn::Week);
    TestEqual(TEXT("Pending decision plays zero days"), Turn.Played, 0);
    TestTrue(TEXT("Typed decision reason"), Turn.Stop == MarketSimulation::EStop::Decision);
    TestFalse(TEXT("One-sentence player message"), Turn.Message.IsEmpty());
    State = MarketTurnTests::State(Base);
    MarketSimulation::FHooks Hooks;
    Hooks.AfterDay = [&](const MarketSimulation::FDay&) { State.Story.Chapter = 2; State.Decisions.Reset(); };
    Turn = MarketSimulation::AdvanceTurn(State, Base, Products, MarketSimulation::ETurn::Month, Hooks);
    TestEqual(TEXT("Chapter stops after one day"), Turn.Played, 1);
    TestTrue(TEXT("Typed chapter reason"), Turn.Stop == MarketSimulation::EStop::Chapter);
    State = MarketTurnTests::State(Base);
    Hooks.AfterDay = [&](const MarketSimulation::FDay&) { State.Decisions.Reset(); State.Competitors[0].WarUntil = State.Day + 7; };
    Turn = MarketSimulation::AdvanceTurn(State, Base, Products, MarketSimulation::ETurn::Month, Hooks);
    TestEqual(TEXT("Price war stops after one day"), Turn.Played, 1);
    TestTrue(TEXT("Typed important event reason"), Turn.Stop == MarketSimulation::EStop::ImportantEvent);
    State = MarketTurnTests::State(Base); State.Day = MarketCalendar::GameDayOf(2011, 3, 31);
    Hooks.AfterDay = [&](const MarketSimulation::FDay&) { State.Decisions.Reset(); };
    Turn = MarketSimulation::AdvanceTurn(State, Base, Products, MarketSimulation::ETurn::Month, Hooks);
    TestEqual(TEXT("Real month closes exactly on April first"), State.Day, MarketCalendar::GameDayOf(2011, 4, 1));
    TestTrue(TEXT("Typed month report"), Turn.Stop == MarketSimulation::EStop::MonthReport);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketTurnSummaries, "MirasMarket.Simulation.PeriodsAndOlderSaves", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketTurnSummaries::RunTest(const FString& Parameters)
{
    auto State = MarketTurnTests::State(MarketTurnTests::Base(), 9);
    TestEqual(TEXT("Older save with no history is safe"), MarketSimulation::WeekSummary(State).RecordedDays, 0);
    TestEqual(TEXT("Missing history is explicitly reported"), MarketSimulation::WeekSummary(State).MissingDays, 1);
    for (int32 Day = 1; Day <= 8; ++Day)
    {
        FMarketDayRecord Record; Record.Day = Day; Record.Revenue = Day * 100; Record.Profit = Day * 10; Record.Cash = Day * 1000; Record.Served = 2; Record.Lost = 1;
        State.History.Add(Record);
    }
    const auto Week = MarketSimulation::WeekSummary(State, 7);
    TestEqual(TEXT("Closed first week revenue"), Week.Revenue, static_cast<int64>(2800));
    TestEqual(TEXT("Closing cash belongs to final day"), Week.ClosingCash, static_cast<int64>(7000));
    TestEqual(TEXT("Seven records"), Week.RecordedDays, 7);
    TestEqual(TEXT("Month summary includes current eighth day"), MarketSimulation::MonthSummary(State).Revenue, static_cast<int64>(3600));
    TestEqual(TEXT("Repeated report doesn't alter state"), MarketSimulation::WeekSummary(State, 7).Revenue, Week.Revenue);
    TestEqual(TEXT("History not mutated"), State.History.Num(), 8);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketTurnRoutine, "MirasMarket.Simulation.RoutineSkillAndImperfections", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketTurnRoutine::RunTest(const FString& Parameters)
{
    const auto Base = MarketTurnTests::Base();
    auto Family = MarketTurnTests::State(Base, 20);
    TestTrue(TEXT("Family forgets sometimes"), MarketSimulation::RoutineForgetPermille(Family) > 0);
    auto Strong = Family;
    FMarketManager Manager; Manager.Level = static_cast<uint8>(MarketManagers::ELevel::FamilyShop); Manager.Country = TEXT("tr"); Manager.Area = TEXT("kirklareli"); Manager.Skill = 95; Manager.Potential = 95; Manager.Morale = 100; Manager.AppointedDay = 1;
    Strong.Management.Managers.Add(Manager);
    auto Weak = Strong; Weak.Management.Managers[0].Skill = 20;
    TestTrue(TEXT("A stronger manager forgets less"), MarketSimulation::RoutineForgetPermille(Strong) < MarketSimulation::RoutineForgetPermille(Weak));
    int32 WeakDelays = 0, StrongDelays = 0;
    for (int32 Day = 20; Day <= 219; ++Day)
    {
        Strong.Day = Weak.Day = Day;
        WeakDelays += MarketSimulation::DelayPriceRise(Weak) ? 1 : 0;
        StrongDelays += MarketSimulation::DelayPriceRise(Strong) ? 1 : 0;
    }
    TestTrue(TEXT("Lower skill delays more monthly price updates"), WeakDelays > StrongDelays);
    auto First = Family, Second = Family; auto FirstProducts = Base, SecondProducts = Base;
    MarketSimulation::PlayDay(First, Base, FirstProducts); MarketSimulation::PlayDay(Second, Base, SecondProducts);
    TestEqual(TEXT("Delegated day remains deterministic"), First.Cash, Second.Cash);
    TestFalse(TEXT("Delegation does not mark test mode"), First.bUsedTestMode);
    // A generous manager may request 11 cases from a 9-case suggestion; use the same 9-case player limit.
    auto Generous = Strong; Generous.Day = 20; Generous.Cash = 10000000;
    Generous.Management.Managers[0].Style = static_cast<uint8>(MarketManagers::EStyle::Generous);
    Generous.Stock[0].Capacity = 200; Generous.Stock[0].Warehouse = 0; Generous.Stock[0].Shelf = 0;
    auto Products = Base;
    const auto Day = MarketSimulation::PlayDay(Generous, Base, Products);
    TestEqual(TEXT("Manager order fits player case limit instead of rejecting the whole order"), Day.Ordered, static_cast<int64>(9 * 12 * 100));
    return true;
}
#endif

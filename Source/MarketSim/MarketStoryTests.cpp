#include "MarketStory.h"
#include "MarketEvents.h"
#include "MarketDirector.h"
#include "MarketStaff.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketStoryTest
{
    TArray<FMarketProduct> Catalog()
    {
        FMarketProduct Milk; Milk.Id = TEXT("milk"); Milk.Category = TEXT("s\u00fct"); Milk.Cost = 170; Milk.BasePrice = 250; Milk.CaseUnits = 12;
        FMarketProduct Cola; Cola.Id = TEXT("cola"); Cola.Category = TEXT("i\u00e7ecek"); Cola.Cost = 180; Cola.BasePrice = 275; Cola.CaseUnits = 12;
        return { Milk, Cola };
    }

    void Close(FMarketState& S, const TArray<FMarketProduct>& Products, int64 Revenue = 0, int64 Purchases = 0)
    {
        S.Revenue = Revenue; S.Purchases = Purchases; S.Served = Revenue > 0 ? 10 : 0;
        S.CloseDay();
        MarketDirector::CloseDay(S, Products);
    }

    bool NewsStarts(const FMarketState& S, const TCHAR* Start)
    {
        for (const FString& Line : S.DayNews) if (Line.StartsWith(Start)) return true;
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketEventsTest, "MarketSim.Events.DecisionsAndModifiers", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketEventsTest::RunTest(const FString& Parameters)
{
    using namespace MarketEvents;
    using namespace MarketStoryTest;
    using MarketGoods::EGroup;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S; S.Initialize(Products); S.RivalSeed = 21; S.Cash = 100000;
    S.ApplyShelfCapacities({ 24, 24 });

    // Modifiers are day ranges.
    AddModifier(S, EModifier::Traffic, AllGroups, 0.5f, 2, 3, TEXT("test"));
    TestTrue(TEXT("Not yet"), FMath::IsNearlyEqual(Factor(S, EModifier::Traffic), 1.f));
    S.Day = 2;
    TestTrue(TEXT("In effect"), FMath::IsNearlyEqual(Factor(S, EModifier::Traffic), 0.5f));
    AddModifier(S, EModifier::Interest, static_cast<uint8>(EGroup::Dairy), 2.f, 1, 10, TEXT("test"));
    TestTrue(TEXT("Only the group"), Factor(S, EModifier::Interest, EGroup::Dairy) > 1.9f && FMath::IsNearlyEqual(Factor(S, EModifier::Interest, EGroup::Drinks), 1.f));
    AddModifier(S, EModifier::PriceTolerance, AllGroups, 0.04f, 1, 10, TEXT("test"));
    TestTrue(TEXT("Tolerance adds"), FMath::IsNearlyEqual(static_cast<float>(Tolerance(S, EGroup::Drinks)), 0.04f));
    S.Modifiers.Reset();

    // Fridge: the default (wait) hurts milk until repaired.
    TestTrue(TEXT("Fridge breaks"), Trigger(S, Products, TEXT("event.fridge")));
    TestTrue(TEXT("A decision waits"), Pending(S) && Pending(S)->Id == TEXT("event.fridge"));
    TestTrue(TEXT("Milk sells less"), Factor(S, EModifier::Interest, EGroup::Dairy) < 0.5f);
    FString Message;
    TestTrue(TEXT("Repair"), Decide(S, Products, 0, Message));
    TestTrue(TEXT("Repair paid at the close"), S.OtherCosts > 0 && FMath::IsNearlyEqual(Factor(S, EModifier::Interest, EGroup::Dairy), 1.f));
    TestTrue(TEXT("Nothing waits"), Pending(S) == nullptr);

    // A wedding buys from the depot.
    S.Stock[1].Warehouse = 30;
    TestTrue(TEXT("Wedding"), Trigger(S, Products, TEXT("event.wedding")));
    const int64 Cash = S.Cash;
    TestTrue(TEXT("Accept"), Decide(S, Products, 0, Message));
    TestTrue(TEXT("Paid and delivered"), S.Cash == Cash + 275 * 24 && S.Stock[1].Warehouse == 6);

    // An unanswered complaint takes the default at the end of the next day.
    FMarketLoyalty Regular; Regular.CustomerId = 3; Regular.Satisfaction = 70.f; S.Loyalty.Add(Regular);
    TestTrue(TEXT("Complaint"), Trigger(S, Products, TEXT("event.complaint")));
    Close(S, Products);
    Close(S, Products);
    TestTrue(TEXT("Decided by default"), !S.Decisions.ContainsByPredicate([](const FMarketDecision& D) { return D.Id == TEXT("event.complaint"); }));
    const FMarketLoyalty* Complainer = S.Loyalty.FindByPredicate([](const FMarketLoyalty& L) { return L.CustomerId == 3; });
    TestTrue(TEXT("Word spreads"), Complainer && Complainer->Satisfaction < 70.f);

    // A broken truck holds the next day's order one more day.
    FMarketState T; T.Initialize(Products); T.Cash = 100000; T.ApplyShelfCapacities({ 24, 24 });
    TestTrue(TEXT("Truck"), Trigger(T, Products, TEXT("event.truck")));
    T.Stock[0].Incoming = 12;
    T.CloseDay();
    TestTrue(TEXT("Still on the way"), T.Stock[0].Incoming == 12 && T.Stock[0].Dock == 0);
    T.CloseDay();
    TestTrue(TEXT("Arrives a day later"), T.Stock[0].Incoming == 0 && T.Stock[0].Dock > 0);

    // With an accountant the inspection is only news.
    FMarketState A; A.Initialize(Products); A.Day = 10; A.Cash = 100000;
    MarketStaff::HireAccountant(A, Message);
    TestTrue(TEXT("Inspection"), Trigger(A, Products, TEXT("event.inspection")));
    TestTrue(TEXT("No decision needed"), Pending(A) == nullptr);

    // Over months: events happen, never the same one within two weeks.
    FMarketState L; L.Initialize(Products); L.RivalSeed = 8; L.Cash = 10000000; L.ApplyShelfCapacities({ 24, 24 });
    for (FMarketStock& Item : L.Stock) Item.Warehouse = 60;
    for (int32 D = 0; D < 150; ++D) { Close(L, Products, 20000); while (Pending(L)) Decide(L, Products, 1, Message); for (FMarketStock& Item : L.Stock) Item.Warehouse = 60; }
    TestTrue(TEXT("Events happen"), L.EventLog.Num() >= 12);
    TMap<FString, int32> LastSeen;
    bool bTooSoon = false;
    for (const FString& Entry : L.EventLog)
    {
        int32 At = INDEX_NONE;
        if (!Entry.FindLastChar(TEXT('@'), At) || !Entry.StartsWith(TEXT("event."))) continue;
        const FString Id = Entry.Left(At);
        const int32 Day = FCString::Atoi(*Entry.Mid(At + 1));
        if (const int32* Before = LastSeen.Find(Id)) if (Day - *Before <= CooldownDays) bTooSoon = true;
        LastSeen.Add(Id, Day);
    }
    TestFalse(TEXT("Cooldown respected"), bTooSoon);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStoryTest, "MarketSim.Story.MemoriesAndIdentity", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStoryTest::RunTest(const FString& Parameters)
{
    using namespace MarketStoryTest;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S; S.Initialize(Products); S.RivalSeed = 4; S.Cash = 200000;
    S.ApplyShelfCapacities({ 24, 24 });
    // M69: no chapters; day 1's first order and first profit become memories.

    Close(S, Products, 30000, 6000);
    TestTrue(TEXT("Memories: first order and first profit"), S.Story.Memories.Num() >= 2);
    TestFalse(TEXT("No neighbour scene"), NewsStarts(S, TEXT("Kar\u015f\u0131daki")));
    FString Message;

    // The first week is a memory too.
    for (int32 D = 0; D < 7; ++D) { S.Decisions.Reset(); Close(S, Products, 30000); }
    TestTrue(TEXT("First week remembered"), S.Story.Memories.ContainsByPredicate([](const FString& M) { return M.Contains(TEXT("ilk hafta")); }));

    // The identity is asked once from day 10.
    for (int32 D = 0; D < 4 && !S.Decisions.ContainsByPredicate([](const FMarketDecision& X) { return X.Id == TEXT("story.identity"); }); ++D)
    {
        S.Decisions.RemoveAll([](const FMarketDecision& X) { return X.Id.StartsWith(TEXT("event.")); });
        Close(S, Products, 30000);
    }
    S.Decisions.RemoveAll([](const FMarketDecision& X) { return X.Id.StartsWith(TEXT("event.")); });
    TestTrue(TEXT("Identity asked"), MarketEvents::Pending(S) && MarketEvents::Pending(S)->Id == TEXT("story.identity"));
    TestFalse(TEXT("No sale offer"), S.Decisions.ContainsByPredicate([](const FMarketDecision& X) { return X.Id == TEXT("story.sell"); }));
    TestTrue(TEXT("Discount identity"), MarketEvents::Decide(S, Products, 2, Message));
    TestEqual(TEXT("Identity kept"), S.Story.Identity, static_cast<uint8>(MarketStory::EIdentity::Indirim));
    TestTrue(TEXT("Cheaper purchases"), MarketEvents::Factor(S, MarketEvents::EModifier::CostFactor, MarketGoods::EGroup::Dairy) < 1.f);
    TestTrue(TEXT("Price hunters"), MarketEvents::Tolerance(S, MarketGoods::EGroup::Dairy) < 0.0);
    TestTrue(TEXT("Identity remembered"), S.Story.Memories.ContainsByPredicate([](const FString& M) { return M.Contains(TEXT("kimli")); }));
    // Not asked twice.
    Close(S, Products, 30000);
    TestFalse(TEXT("Asked once"), S.Decisions.ContainsByPredicate([](const FMarketDecision& X) { return X.Id == TEXT("story.identity"); }));
    // The debt never locks anything and is no milestone of its own until it is paid.
    TestFalse(TEXT("Not closed"), MarketStory::StoryClosed(S));
    return true;
}

#endif

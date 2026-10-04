#include "MarketMotion.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketMotionTest, "MirasMarket.Motion.WalkingAndCrowd", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketMotionTest::RunTest(const FString& Parameters)
{
    using namespace MarketMotion;
    using MarketCustomers::ESegment;

    // Walking styles: fixed per person, different per kind of person.
    const FGait Elder = MakeGait(ESegment::Retired, 0, 42);
    TestEqual(TEXT("Same person, same walk"), MakeGait(ESegment::Retired, 0, 42).SpeedScale, Elder.SpeedScale);
    float Retired = 0.f, Workers = 0.f;
    for (int32 Id = 0; Id < 40; ++Id) { Retired += MakeGait(ESegment::Retired, Id, 42).SpeedScale; Workers += MakeGait(ESegment::Worker, Id, 42).SpeedScale; }
    TestTrue(TEXT("Pensioners walk slower than workers"), Retired < Workers);
    TestTrue(TEXT("Children dart"), MakeGait(ESegment::Child, 3, 42).Style == EStyle::Dart);
    TestNotEqual(TEXT("People differ"), MakeGait(ESegment::Worker, 1, 42).SpeedScale, MakeGait(ESegment::Worker, 2, 42).SpeedScale);
    const FGait Worker = MakeGait(ESegment::Worker, 5, 42);
    TestTrue(TEXT("A heavy basket slows"), Speed(Worker, 140.f, 12, 0.5f) < Speed(Worker, 140.f, 2, 0.5f));
    TestTrue(TEXT("A worker hurries before closing"), Speed(Worker, 140.f, 2, 0.98f) > Speed(Worker, 140.f, 2, 0.5f));
    TestFalse(TEXT("Style name"), StyleName(Elder.Style).IsEmpty());

    // Route: a short walk, except for families who keep their written order.
    const TArray<FVector> Stops = { FVector(300, 500, 0), FVector(-300, 100, 0), FVector(300, 100, 0), FVector(-300, 500, 0) };
    const FVector Door(0, -230, 0), Till(405, 60, 0);
    const TArray<int32> Written = { 0, 1, 2, 3 };
    const TArray<int32> Short = OrderVisits(Stops, Door, Till, EStyle::Brisk);
    TestEqual(TEXT("Every stop once"), Short.Num(), Stops.Num());
    TestTrue(TEXT("Shorter than the written order"), RouteLength(Stops, Short, Door, Till) < RouteLength(Stops, Written, Door, Till));
    TestTrue(TEXT("A family keeps its list"), OrderVisits(Stops, Door, Till, EStyle::Wander) == Written);
    TestEqual(TEXT("A child goes for its wish first"), OrderVisits(Stops, Door, Till, EStyle::Dart)[0], 0);

    // At the shelf.
    TestTrue(TEXT("A regular is quicker"), BrowseSeconds(2.f, EStyle::Steady, true, true, 1.f) < BrowseSeconds(2.f, EStyle::Steady, false, true, 1.f));
    TestTrue(TEXT("An empty shelf means searching"), BrowseSeconds(2.f, EStyle::Steady, false, false, 1.f) > 2.f);
    TestTrue(TEXT("A high price means comparing"), BrowseSeconds(2.f, EStyle::Steady, false, true, 1.2f) > BrowseSeconds(2.f, EStyle::Steady, false, true, 1.f));

    // Queue.
    TestFalse(TEXT("Short queue: stays"), Balks(ESegment::Worker, 1, 1, 0.f));
    TestTrue(TEXT("Long queue, small basket: a worker leaves"), Balks(ESegment::Worker, 6, 1, 0.f));
    TestFalse(TEXT("A pensioner with a full basket waits"), Balks(ESegment::Retired, 5, 8, 0.f));

    // Crowd: slow down behind someone, keep to the right, nothing changes on an empty aisle.
    float Factor = 1.f;
    const FVector Here(0, 0, 0), Target(500, 0, 0);
    TestTrue(TEXT("Empty aisle"), FVector::Dist2D(Steer(Here, Target, {}, 60.f, Factor), Target) < 0.01f);
    TestEqual(TEXT("Full speed"), Factor, 1.f);
    const FVector Nudged = Steer(Here, Target, { FVector(30, 0, 0) }, 60.f, Factor);
    TestTrue(TEXT("Slows behind someone"), Factor < 0.6f);
    TestTrue(TEXT("Steps to the right"), Nudged.Y > 0.f && Nudged.Y <= 25.f);
    Steer(Here, Target, { FVector(-30, 0, 0) }, 60.f, Factor);
    TestEqual(TEXT("Someone behind does not slow us"), Factor, 1.f);

    // Neighbours chat.
    TestTrue(TEXT("Two pensioners chat"), Chats(Elder, MakeGait(ESegment::Retired, 1, 42), 0.1f));
    TestFalse(TEXT("A worker and a child do not"), Chats(Worker, MakeGait(ESegment::Child, 3, 42), 0.1f));
    TestTrue(TEXT("A short chat"), ChatSeconds(0.5f) >= 3.f && ChatSeconds(0.5f) <= 7.f);
    return true;
}

#endif

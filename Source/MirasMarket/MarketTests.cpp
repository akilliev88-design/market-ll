#include "MarketGame.h"
#include "ProductCatalog.h"
#include "Planogram.h"
#include "PlanogramEdit.h"
#include "StaffPlanner.h"
#include "MarketDemand.h"
#include "MarketPeople.h"
#include "Misc/AutomationTest.h"
#include "Kismet/GameplayStatics.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace
{
    TArray<FMarketProduct> TestCatalog()
    {
        FMarketProduct P;
        P.Id = TEXT("milk"); P.Cost = 170; P.BasePrice = 250;
        return { P };
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketLocomotionTest, "MirasMarket.People.Locomotion", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketLocomotionTest::RunTest(const FString& Parameters)
{
    MarketPeople::FShopper Shopper;
    FVector Position = FVector::ZeroVector;
    FVector Direction;
    bool bMoving = false;
    Position = MarketPeople::MoveToward(Shopper, Position, FVector(1000, 0, 0), 140.f, .1f, Direction, bMoving);
    TestTrue(TEXT("Starts below full walking speed"), bMoving && Shopper.CurrentSpeed > 0.f && Shopper.CurrentSpeed < 140.f);
    const float FirstStep = Position.X;
    Position = MarketPeople::MoveToward(Shopper, Position, FVector(1000, 0, 0), 140.f, .1f, Direction, bMoving);
    TestTrue(TEXT("Accelerates instead of sliding at constant speed"), Position.X - FirstStep > FirstStep);

    Shopper.CurrentSpeed = 140.f;
    Position = MarketPeople::MoveToward(Shopper, FVector(99, 0, 0), FVector(100, 0, 0), 140.f, 1.f, Direction, bMoving);
    TestTrue(TEXT("Never overshoots a stop"), FMath::IsNearlyEqual(Position.X, 100.0));
    Position = MarketPeople::MoveToward(Shopper, Position, FVector(100, 0, 0), 140.f, .1f, Direction, bMoving);
    TestFalse(TEXT("Settles at the destination"), bMoving);
    TestEqual(TEXT("Settled speed is zero"), Shopper.CurrentSpeed, 0.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketPlanogramTest, "MirasMarket.Planogram.MultiBrandDepth", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketPlanogramTest::RunTest(const FString& Parameters)
{
    const FString Json = TEXT("{\"fixtures\":[{\"id\":\"sut_gondol\",\"label\":\"Sut\",\"category\":\"sut\",\"x\":0,\"y\":320}],\"placements\":[")
        TEXT("{\"productId\":\"milk_a\",\"fixtureId\":\"sut_gondol\",\"face\":\"front\",\"level\":1,\"facings\":3,\"depth\":4,\"order\":0},")
        TEXT("{\"productId\":\"milk_b\",\"fixtureId\":\"sut_gondol\",\"face\":\"front\",\"level\":1,\"facings\":2,\"depth\":3,\"order\":1}]}");
    FMarketPlanogram P; TArray<FString> Errors;
    TestTrue(TEXT("Planogram parses"), MarketPlanogram::Parse(Json, P, Errors));
    TestEqual(TEXT("Two brands share one fixture"), P.Placements.Num(), 2);
    if (P.Placements.Num() != 2) return false;
    TestEqual(TEXT("Depth retained"), P.Placements[0].Depth, 4);
    FMarketProduct A; A.Id = TEXT("milk_a"); A.WidthMm = 95;
    FMarketProduct B; B.Id = TEXT("milk_b"); B.WidthMm = 95;
    TArray<FMarketProduct> Catalog = { A, B };
    TestTrue(TEXT("Blocks get different horizontal centers"), MarketPlanogram::PlacementCenterX(P, Catalog, P.Placements[0]) < MarketPlanogram::PlacementCenterX(P, Catalog, P.Placements[1]));
    FMarketPlanogram Again; TArray<FString> Errors2;
    TestTrue(TEXT("Serialized planogram parses"), MarketPlanogram::Parse(MarketPlanogram::Serialize(P), Again, Errors2));
    TestEqual(TEXT("Round trip keeps facing count"), Again.Placements[1].Facings, 2);
    FMarketProduct Cola; Cola.Id = TEXT("cola"); Cola.Category = TEXT("sut"); Catalog.Add(Cola);
    TestNull(TEXT("New catalog product is not placed automatically"), Again.FindPlacement(TEXT("cola")));
    TestEqual(TEXT("Unplaced product has no shelf capacity"), MarketPlanogram::ProductCapacity(Again, Catalog, TEXT("cola")), 0);

    // Old (v2) files: the old packing becomes fixed positions once, and v3 writes "x" instead of order/offset.
    const float LegacyA = MarketPlanogram::PlacementCenterX(P, Catalog, P.Placements[0]);
    const float LegacyB = MarketPlanogram::PlacementCenterX(P, Catalog, P.Placements[1]);
    MarketPlanogram::ResolvePositions(P, Catalog);
    TestTrue(TEXT("Old blocks get fixed positions"), P.Placements[0].bHasX && P.Placements[1].bHasX);
    TestTrue(TEXT("Positions match what the old packing showed"),
        FMath::IsNearlyEqual(P.Placements[0].XCm, LegacyA, 0.1f) && FMath::IsNearlyEqual(P.Placements[1].XCm, LegacyB, 0.1f));
    const FString V3 = MarketPlanogram::Serialize(P);
    TestTrue(TEXT("v3 writes x"), V3.Contains(TEXT("\"x\":")) && !V3.Contains(TEXT("\"order\"")));
    FMarketPlanogram Reread; TArray<FString> Errors3;
    TestTrue(TEXT("v3 parses"), MarketPlanogram::Parse(V3, Reread, Errors3));
    TestTrue(TEXT("x survives round trip"), Reread.Placements.Num() == 2 && Reread.Placements[1].bHasX && FMath::IsNearlyEqual(Reread.Placements[1].XCm, P.Placements[1].XCm, 0.1f));

    // The same product may have several blocks.
    const FString Twice = TEXT("{\"fixtures\":[{\"id\":\"sut_gondol\"}],\"placements\":[")
        TEXT("{\"productId\":\"milk_a\",\"fixtureId\":\"sut_gondol\",\"level\":0,\"x\":-30},")
        TEXT("{\"productId\":\"milk_a\",\"fixtureId\":\"sut_gondol\",\"level\":2,\"x\":20}]}");
    FMarketPlanogram Double; TArray<FString> Errors4;
    TestTrue(TEXT("Duplicate product parses"), MarketPlanogram::Parse(Twice, Double, Errors4));
    TestEqual(TEXT("Both blocks of the same product are kept"), Double.Placements.Num(), 2);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketPlanogramWidthTest, "MirasMarket.Planogram.WidthLimit", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketPlanogramWidthTest::RunTest(const FString& Parameters)
{
    // 250 mm packages: two facings = 50.5 cm (0.5 cm between facings), two blocks = 101.3 cm (fits 116), a third does not.
    auto Wide = [](const FString& Id, int32 WidthMm) { FMarketProduct P; P.Id = Id; P.WidthMm = WidthMm; return P; };
    FPlanogramPlacement Two; Two.Facings = 2;
    TestEqual(TEXT("Block width uses facings and item gap"), MarketPlanogram::BlockWidthCm(Wide(TEXT("x"), 250), Two), 50.5f);
    FString M;

    FMarketPlanogram Single;
    FPlanogramFixture Wall; Wall.Id = TEXT("duvar"); Wall.EquipmentId = TEXT("wall_single_1200");
    Single.Fixtures.Add(Wall);
    TArray<FMarketProduct> Catalog = { Wide(TEXT("a"), 250), Wide(TEXT("b"), 250), Wide(TEXT("d"), 250) };
    TestTrue(TEXT("a on level 1"), MarketPlanogramEdit::AddToRowEnd(Single, Catalog, TEXT("a"), Wall.Id, TEXT("front"), 0, M));
    TestTrue(TEXT("b next to a"), MarketPlanogramEdit::AddToRowEnd(Single, Catalog, TEXT("b"), Wall.Id, TEXT("front"), 0, M));
    TestFalse(TEXT("Third block does not fit next to the others"), MarketPlanogramEdit::AddToRowEnd(Single, Catalog, TEXT("d"), Wall.Id, TEXT("front"), 0, M));
    TestFalse(TEXT("Single sided fixture has no back face"), MarketPlanogramEdit::AddToRowEnd(Single, Catalog, TEXT("d"), Wall.Id, TEXT("back"), 0, M));
    TestTrue(TEXT("Third block fits on level 2"), MarketPlanogramEdit::AddToRowEnd(Single, Catalog, TEXT("d"), Wall.Id, TEXT("front"), 1, M));
    TArray<FString> Warnings;
    MarketPlanogram::FindOverflows(Single, Catalog, Warnings);
    TestEqual(TEXT("Hand arranged plan has no overflow"), Warnings.Num(), 0);
    Single.Placements.Last().Level = 0; // hand-edited file: three wide blocks on one level
    Warnings.Reset();
    MarketPlanogram::FindOverflows(Single, Catalog, Warnings);
    TestEqual(TEXT("Hand-made overflow is reported once"), Warnings.Num(), 1);

    // Double sided gondola: 4 levels x 2 blocks on the front, the ninth product goes to the back face.
    FMarketPlanogram Double;
    FPlanogramFixture Gondola; Gondola.Id = TEXT("gondol");
    Double.Fixtures.Add(Gondola);
    TArray<FMarketProduct> Many;
    for (int32 I = 0; I < 9; ++I) Many.Add(Wide(FString::Printf(TEXT("p%d"), I), 250));
    for (const FMarketProduct& Product : Many)
    {
        bool bPlaced = false;
        static const TCHAR* Faces[] = { TEXT("front"), TEXT("back") };
        for (const TCHAR* Face : Faces)
            for (int32 Level = 0; Level < 4 && !bPlaced; ++Level)
                bPlaced = MarketPlanogramEdit::AddToRowEnd(Double, Many, Product.Id, Gondola.Id, Face, Level, M);
        TestTrue(TEXT("Every product finds a level"), bPlaced);
    }
    const FPlanogramPlacement* Ninth = Double.FindPlacement(TEXT("p8"));
    if (!TestNotNull(TEXT("ninth placed"), Ninth)) return false;
    TestEqual(TEXT("Ninth product uses the back face"), Ninth->Face, FString(TEXT("back")));
    TestEqual(TEXT("Ninth product keeps two facings"), Ninth->Facings, 2);
    Warnings.Reset();
    MarketPlanogram::FindOverflows(Double, Many, Warnings);
    TestEqual(TEXT("Full gondola has no overflow"), Warnings.Num(), 0);

    // A package wider than the shelf cannot be placed; the reason names the widths.
    FMarketPlanogram Tiny;
    Tiny.Fixtures.Add(Wall);
    TArray<FMarketProduct> Huge = { Wide(TEXT("dev"), 2000) };
    TestFalse(TEXT("Oversized product is refused"), MarketPlanogramEdit::AddToRowEnd(Tiny, Huge, TEXT("dev"), Wall.Id, TEXT("front"), 0, M));
    TestTrue(TEXT("Reason names the widths"), M.Contains(TEXT("cm")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketFreePositionTest, "MirasMarket.Planogram.FreePosition", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketFreePositionTest::RunTest(const FString& Parameters)
{
    FMarketProduct Box; Box.Id = TEXT("box"); Box.PackageType = TEXT("kutu"); Box.WidthMm = 100; Box.DepthMm = 50; Box.HeightMm = 120;
    FMarketProduct Other = Box; Other.Id = TEXT("other");
    TArray<FMarketProduct> Catalog = { Box, Other };
    FMarketPlanogram P;
    FPlanogramFixture Fixture; Fixture.Id = TEXT("gondol"); P.Fixtures.Add(Fixture);

    FPlanogramPlacement Probe; Probe.ProductId = Box.Id; Probe.FixtureId = Fixture.Id; Probe.Facings = 1;
    TestEqual(TEXT("Front orientation uses package width"), MarketPlanogram::BlockWidthCm(Box, Probe), 10.f);
    Probe.Orientation = 1;
    TestEqual(TEXT("Quarter turn uses package depth"), MarketPlanogram::BlockWidthCm(Box, Probe), 5.f);
    Probe.Orientation = 2;
    TestEqual(TEXT("Laid box uses package height across shelf"), MarketPlanogram::BlockWidthCm(Box, Probe), 12.f);
    TestTrue(TEXT("Boxes may be laid on side"), MarketPlanogram::CanLayOnSide(Box));
    TestEqual(TEXT("Twelve cm boxes stack twice in thirty cm clearance"), MarketPlanogram::MaxStackFor(Box, 0, 30.f), 2);

    FString M;
    FPlanogramPlacement Wish; Wish.ProductId = Box.Id; Wish.FixtureId = Fixture.Id; Wish.Level = 0; Wish.Facings = 1;
    TestTrue(TEXT("Block goes exactly where aimed"), MarketPlanogramEdit::AddBlock(P, Catalog, Wish, -30.f, 20.f, M));
    TestEqual(TEXT("Aimed position kept"), P.Placements[0].XCm, -30.f);
    FPlanogramPlacement OtherWish = Wish; OtherWish.ProductId = Other.Id;
    TestTrue(TEXT("Aiming at an occupied spot snaps next to it"), MarketPlanogramEdit::AddBlock(P, Catalog, OtherWish, -30.f, 20.f, M));
    TestEqual(TEXT("Snapped side by side (half widths + 0.3 cm tolerance)"), FMath::Abs(P.Placements[1].XCm + 30.f), 10.3f, 0.01f);
    TestTrue(TEXT("Snapped block fits"), MarketPlanogram::BlockFits(P, Catalog, 1));
    TestFalse(TEXT("Too far to snap is refused"), MarketPlanogramEdit::AddBlock(P, Catalog, OtherWish, P.Placements[0].XCm, 2.f, M));

    // Other sits left of Box: Box can slide right, and slides left only until it touches Other.
    TestTrue(TEXT("Other is left of Box"), P.Placements[1].XCm < P.Placements[0].XCm);
    TestTrue(TEXT("Nudge right"), MarketPlanogramEdit::Nudge(P, Catalog, 0, 5.f, M));
    TestEqual(TEXT("Moved 5 cm"), P.Placements[0].XCm, -25.f);
    TestTrue(TEXT("Nudge back"), MarketPlanogramEdit::Nudge(P, Catalog, 0, -5.f, M));
    TestFalse(TEXT("Stops against the neighbour"), MarketPlanogramEdit::Nudge(P, Catalog, 0, -5.f, M));
    TestFalse(TEXT("Stops at the shelf edge"), MarketPlanogramEdit::Nudge(P, Catalog, 1, -20.f, M) && MarketPlanogramEdit::Nudge(P, Catalog, 1, -5.f, M));

    // Spacing: Box (at -30) may keep 1 cm to Other (at -50), not 20 cm.
    TestTrue(TEXT("Small gap fits"), MarketPlanogramEdit::ChangeGap(P, Catalog, 0, 1.f, M));
    TestEqual(TEXT("Gap stored"), P.Placements[0].GapCm, 1.f);
    TestFalse(TEXT("Gap larger than the free space is refused"), MarketPlanogramEdit::ChangeGap(P, Catalog, 0, 20.f, M));
    TestEqual(TEXT("Refused gap leaves the block unchanged"), P.Placements[0].GapCm, 1.f);

    // The same product again, elsewhere on the shelf.
    TestTrue(TEXT("Same product can be placed again"), MarketPlanogramEdit::AddBlock(P, Catalog, Wish, 30.f, 20.f, M));
    TestEqual(TEXT("Three blocks"), P.Placements.Num(), 3);
    TestEqual(TEXT("Capacity sums every block of the product"), MarketPlanogram::ProductCapacity(P, Catalog, Box.Id),
        MarketPlanogram::Capacity(P.Placements[0]) + MarketPlanogram::Capacity(P.Placements[2]));

    P.Placements[2].Orientation = 1; P.Placements[2].Stack = 2;
    FMarketPlanogram Again; TArray<FString> Errors;
    TestTrue(TEXT("Round trip parses"), MarketPlanogram::Parse(MarketPlanogram::Serialize(P), Again, Errors));
    TestTrue(TEXT("Position survives round trip"), Again.Placements.Num() == 3 && Again.Placements[2].bHasX && FMath::IsNearlyEqual(Again.Placements[2].XCm, 30.f));
    TestEqual(TEXT("Orientation survives round trip"), Again.Placements[2].Orientation, 1);
    TestEqual(TEXT("Stack survives round trip"), Again.Placements[2].Stack, 2);
    TestEqual(TEXT("Gap survives round trip"), Again.Placements[0].GapCm, 1.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketHandArrangementTest, "MirasMarket.Planogram.HandArrangement", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketHandArrangementTest::RunTest(const FString& Parameters)
{
    // 95 mm cartons (64 mm deep, 120 mm high) and a 58 cm wide pack on a 116 cm gondola + a 235 cm wall shelf.
    FMarketProduct A; A.Id = TEXT("sut_a"); A.PackageType = TEXT("kutu"); A.WidthMm = 95; A.DepthMm = 64; A.HeightMm = 120;
    FMarketProduct B = A; B.Id = TEXT("sut_b");
    FMarketProduct C; C.Id = TEXT("genis"); C.WidthMm = 580; C.DepthMm = 100; C.HeightMm = 200;
    TArray<FMarketProduct> Catalog = { A, B, C };
    FMarketPlanogram P;
    FPlanogramFixture Gondola; Gondola.Id = TEXT("gondol"); P.Fixtures.Add(Gondola);
    FPlanogramFixture Wall; Wall.Id = TEXT("duvar"); Wall.EquipmentId = TEXT("wall_shelf_2400"); P.Fixtures.Add(Wall);
    auto Find = [&P](const FString& Id, int32 Level) { return P.Placements.IndexOfByPredicate([&](const FPlanogramPlacement& X) { return X.ProductId == Id && X.Level == Level; }); };
    FString M;

    TestEqual(TEXT("Unplaced product has no shelf capacity"), MarketPlanogram::ProductCapacity(P, Catalog, A.Id), 0);
    TestTrue(TEXT("Add A"), MarketPlanogramEdit::AddToRowEnd(P, Catalog, A.Id, Gondola.Id, TEXT("front"), 1, M));
    TestTrue(TEXT("Add B"), MarketPlanogramEdit::AddToRowEnd(P, Catalog, B.Id, Gondola.Id, TEXT("front"), 1, M));
    TestTrue(TEXT("Add A again next to B"), MarketPlanogramEdit::AddToRowEnd(P, Catalog, A.Id, Gondola.Id, TEXT("front"), 1, M));
    TestEqual(TEXT("Three blocks, two of them A"), P.Placements.Num(), 3);
    TestEqual(TEXT("Facings stay as authored (not widened)"), P.Placements[0].Facings, 2);
    TestEqual(TEXT("Depth follows shelf depth (37 cm / 8.4 cm)"), P.Placements[0].Depth, 4);
    const TArray<int32> Row = MarketPlanogram::RowBlocks(P, Catalog, Gondola.Id, TEXT("front"), 1);
    if (!TestEqual(TEXT("Row has three blocks"), Row.Num(), 3)) return false;
    TestTrue(TEXT("Left to right: A, B, A"), P.Placements[Row[0]].ProductId == A.Id && P.Placements[Row[1]].ProductId == B.Id && P.Placements[Row[2]].ProductId == A.Id);
    for (int32 I = 0; I < P.Placements.Num(); ++I) TestTrue(TEXT("Every block fits"), MarketPlanogram::BlockFits(P, Catalog, I));

    TestFalse(TEXT("Wide pack has no room"), MarketPlanogramEdit::AddToRowEnd(P, Catalog, C.Id, Gondola.Id, TEXT("front"), 1, M));
    TestTrue(TEXT("Reason names the free gap"), M.Contains(TEXT("cm")));
    TestTrue(TEXT("Remove the second A"), MarketPlanogramEdit::RemoveBlock(P, Catalog, Row[2], M));
    TestTrue(TEXT("Wide pack fits now with one facing"), MarketPlanogramEdit::AddToRowEnd(P, Catalog, C.Id, Gondola.Id, TEXT("front"), 1, M));
    const int32 CIndex = Find(C.Id, 1);
    if (!TestTrue(TEXT("C placed"), CIndex != INDEX_NONE)) return false;
    TestEqual(TEXT("Facings reduced to what fits"), P.Placements[CIndex].Facings, 1);
    TestFalse(TEXT("A second facing does not fit"), MarketPlanogramEdit::ChangeFacings(P, Catalog, CIndex, 1, M));
    TestEqual(TEXT("Rejected edit leaves the plan unchanged"), P.Placements[CIndex].Facings, 1);

    // The ghost preview is exactly what gets stored.
    FPlanogramPlacement Wish; Wish.ProductId = A.Id; Wish.FixtureId = Gondola.Id; Wish.Level = 2; Wish.Facings = 2;
    const MarketPlanogramEdit::FBlockPlan Plan = MarketPlanogramEdit::PlanBlock(P, Catalog, Wish, 0.f);
    TestTrue(TEXT("Preview fits"), Plan.bOk);
    int32 Added = INDEX_NONE;
    TestTrue(TEXT("Place what the preview shows"), MarketPlanogramEdit::AddBlock(P, Catalog, Wish, 0.f, 1.0e6f, M, &Added));
    TestTrue(TEXT("Preview matches the stored block"), P.Placements.IsValidIndex(Added) && FMath::IsNearlyEqual(P.Placements[Added].XCm, Plan.Block.XCm) && P.Placements[Added].Facings == Plan.Block.Facings);

    const int32 BIndex = Find(B.Id, 1);
    TestTrue(TEXT("Move B up onto the occupied spot: it lands beside A"), MarketPlanogramEdit::MoveBlock(P, Catalog, BIndex, Gondola.Id, TEXT("front"), 2, 0.f, 1.0e6f, M));
    TestTrue(TEXT("Moved block fits"), P.Placements[BIndex].Level == 2 && MarketPlanogram::BlockFits(P, Catalog, BIndex));

    const int32 AIndex = Find(A.Id, 1);
    TestTrue(TEXT("Stack A twice"), MarketPlanogramEdit::ChangeStack(P, Catalog, AIndex, 1, false, M));
    TestFalse(TEXT("Third layer does not fit 30 cm"), MarketPlanogramEdit::ChangeStack(P, Catalog, AIndex, 1, false, M));
    TestEqual(TEXT("Capacity sums both A blocks"), MarketPlanogram::ProductCapacity(P, Catalog, A.Id), 2 * 4 * 2 + 2 * 4 * 1);
    P.Placements[AIndex].Stack = 5; // hand-edited file
    TestEqual(TEXT("Capacity counts only the stack that fits"), MarketPlanogram::ProductCapacity(P, Catalog, A.Id), 2 * 4 * 2 + 2 * 4 * 1);
    TArray<FString> Warnings;
    MarketPlanogram::FindOverflows(P, Catalog, Warnings);
    TestEqual(TEXT("Block warning reported once with two fixtures"), Warnings.Num(), 1);
    P.Placements[AIndex].Stack = 2;

    TestTrue(TEXT("Turn A to the back face"), MarketPlanogramEdit::ToggleFace(P, Catalog, AIndex, M));
    TestEqual(TEXT("A on the back face"), P.Placements[AIndex].Face, FString(TEXT("back")));
    TestFalse(TEXT("Wall shelf has no back face"), MarketPlanogramEdit::AddToRowEnd(P, Catalog, C.Id, Wall.Id, TEXT("back"), 0, M));
    TestTrue(TEXT("Wall shelf top level"), MarketPlanogramEdit::AddToRowEnd(P, Catalog, C.Id, Wall.Id, TEXT("front"), 4, M));
    TestEqual(TEXT("Wall keeps two facings (235 cm)"), P.Placements.Last().Facings, 2);

    // Old files may still carry "autoFill": it is read without effect and no longer written.
    FMarketPlanogram Parsed; TArray<FString> Errors;
    TestTrue(TEXT("Old autoFill file parses"), MarketPlanogram::Parse(TEXT("{\"autoFill\":true,\"fixtures\":[{\"id\":\"gondol\"}],\"placements\":[]}"), Parsed, Errors));
    TestFalse(TEXT("autoFill is not written"), MarketPlanogram::Serialize(Parsed).Contains(TEXT("autoFill")));
    TestEqual(TEXT("Nothing is placed on load"), Parsed.Placements.Num(), 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCapacityTest, "MirasMarket.Economy.ShelfCapacityAndTestMode", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCapacityTest::RunTest(const FString& Parameters)
{
    auto Catalog = TestCatalog(); FMarketState S; S.Initialize(Catalog);
    TestEqual(TEXT("Default capacity for old saves"), S.Stock[0].Capacity, FMarketState::DefaultShelfCapacity);
    // Bigger planogram block: restock uses the new capacity (no fixed 24 limit).
    S.ApplyShelfCapacities({ 60 });
    TestEqual(TEXT("Capacity applied"), S.Stock[0].Capacity, 60);
    TestEqual(TEXT("Restock moves the whole warehouse"), S.Restock(0), 32);
    TestEqual(TEXT("Shelf above 24"), S.Stock[0].Shelf, 32);
    TestTrue(TEXT("Large shelf validates"), S.IsStructurallyValid());
    // Smaller block: surplus returns to the warehouse, nothing is lost.
    TestEqual(TEXT("No units discarded"), S.ApplyShelfCapacities({ 10 }), 0);
    TestEqual(TEXT("Shelf clamped"), S.Stock[0].Shelf, 10);
    TestEqual(TEXT("Surplus back in warehouse"), S.Stock[0].Warehouse, 22);
    // Test mode: free fill and free delivery, cash untouched.
    const int64 Cash = S.Cash;
    S.Sell(0, 4, 250, Catalog);
    const int64 CashAfterSale = S.Cash;
    TestEqual(TEXT("Free fill tops the shelf up"), S.FillShelfFree(0), 4);
    TestEqual(TEXT("Free fill leaves warehouse alone"), S.Stock[0].Warehouse, 22);
    TestEqual(TEXT("Free delivery respects storage"), S.ReceiveFree(0, 500), FMarketState::StorageCapacity - 22);
    TestEqual(TEXT("Test mode costs nothing"), S.Cash, CashAfterSale);
    TestTrue(TEXT("Sale still paid"), CashAfterSale > Cash);
    TestTrue(TEXT("State stays valid"), S.IsStructurallyValid());
    // A product taken off the shelves: capacity 0, its shelf units go back to the warehouse.
    FMarketState Off; Off.Initialize(Catalog);
    Off.Restock(0);
    const int32 Units = Off.Stock[0].Shelf + Off.Stock[0].Warehouse;
    TestTrue(TEXT("Some units on the shelf before"), Off.Stock[0].Shelf > 0);
    TestEqual(TEXT("Nothing lost when a product leaves the shelves"), Off.ApplyShelfCapacities({ 0 }), 0);
    TestEqual(TEXT("Shelf empty when not placed"), Off.Stock[0].Shelf, 0);
    TestEqual(TEXT("Units kept in the warehouse"), Off.Stock[0].Warehouse, Units);
    TestEqual(TEXT("Nothing to restock without shelf space"), Off.Restock(0), 0);
    TestTrue(TEXT("Zero capacity is valid (not on a shelf)"), Off.IsStructurallyValid());
    Off.Stock[0].Capacity = -1;
    TestFalse(TEXT("Negative capacity rejected"), Off.IsStructurallyValid());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketInventoryTest, "MirasMarket.Economy.InventoryAndOrder", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketInventoryTest::RunTest(const FString& Parameters)
{
    auto Catalog = TestCatalog(); FMarketState S; S.Initialize(Catalog);
    TestEqual(TEXT("Shelf filled from warehouse"), S.Restock(0), 24);
    TestEqual(TEXT("Stock conserved"), S.Stock[0].Shelf + S.Stock[0].Warehouse, 32);
    TestTrue(TEXT("Order accepted"), S.Order(0, Catalog));
    TestEqual(TEXT("Cash debited once in kurus"), S.Cash, int64(32960));
    TestEqual(TEXT("Not delivered early"), S.Stock[0].Warehouse, 8);
    TestEqual(TEXT("Twelve units in transit"), S.Stock[0].Incoming, 12);
    S.CloseDay();
    TestEqual(TEXT("Next day delivery"), S.Stock[0].Warehouse, 20);
    TestEqual(TEXT("Transit cleared"), S.Stock[0].Incoming, 0);
    TestEqual(TEXT("Purchase does not duplicate cost of goods"), S.LastProfit, int64(-2200));
    S.Cash = 0;
    TestFalse(TEXT("Cannot spend missing cash"), S.Order(0, Catalog));
    TestEqual(TEXT("Failed order preserves stock"), S.Stock[0].Incoming, 0);
    TestFalse(TEXT("Bad catalog index rejected"), S.Order(99, Catalog));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketSaleTest, "MirasMarket.Economy.SaleAndProfit", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketSaleTest::RunTest(const FString& Parameters)
{
    auto Catalog = TestCatalog(); FMarketState S; S.Initialize(Catalog);
    TestFalse(TEXT("Overselling is forbidden"), S.Sell(0, 17, 250, Catalog));
    TestFalse(TEXT("Negative quantity is forbidden"), S.Sell(0, -1, 250, Catalog));
    S.Restock(0);
    S.Stock[0].Price = 300;
    TestTrue(TEXT("Existing basket respects its 250 kurus quote"), S.Sell(0, 4, 250, Catalog));
    TestEqual(TEXT("Cash includes exact receipt"), S.Cash, int64(36000));
    TestEqual(TEXT("Sold inventory removed"), S.Stock[0].Shelf, 20);
    TestEqual(TEXT("Cost of goods recorded"), S.CostOfGoods, int64(680));
    S.CloseDay();
    TestEqual(TEXT("Profit is revenue minus cost and daily expenses"), S.LastProfit, int64(-1880));
    TestEqual(TEXT("Day cost removed from cash"), S.Cash, int64(33800));
    TestEqual(TEXT("Daily revenue reset"), S.Revenue, int64(0));
    TestEqual(TEXT("Previous revenue preserved"), S.LastRevenue, int64(1000));
    TestEqual(TEXT("Losing day gives no profitable-day unlock"), S.ProfitableDays, 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketNewGameStockTest, "MirasMarket.Economy.NewGameShelvesEmpty", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketNewGameStockTest::RunTest(const FString& Parameters)
{
    auto Catalog = TestCatalog(); FMarketState S; S.Initialize(Catalog);
    for (const FMarketStock& Item : S.Stock)
    {
        TestEqual(TEXT("New shelf starts empty"), Item.Shelf, 0);
        TestEqual(TEXT("Inherited stock waits in warehouse"), Item.Warehouse, 32);
        TestEqual(TEXT("Opening inventory is conserved"), Item.Shelf + Item.Warehouse, 32);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketSaveTest, "MirasMarket.Save.RoundTripAndValidation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketSaveTest::RunTest(const FString& Parameters)
{
    auto Catalog = TestCatalog();
    auto* Save = NewObject<UMarketSave>();
    Save->State.Initialize(Catalog);
    Save->State.Order(0, Catalog);
    Save->State.bRealBrands = false;
    Save->State.bCashier = true;
    TArray<uint8> Bytes;
    TestTrue(TEXT("Serialize save into memory"), UGameplayStatics::SaveGameToMemory(Save, Bytes));
    auto* Restored = Cast<UMarketSave>(UGameplayStatics::LoadGameFromMemory(Bytes));
    if (!TestNotNull(TEXT("Restore typed save"), Restored)) return false;
    TestEqual(TEXT("Cash survives round trip"), Restored->State.Cash, Save->State.Cash);
    TestEqual(TEXT("Pending shipment survives round trip"), Restored->State.Stock[0].Incoming, 12);
    TestFalse(TEXT("Brand presentation survives round trip"), Restored->State.bRealBrands);
    TestTrue(TEXT("Employee survives round trip"), Restored->State.bCashier);
    TestTrue(TEXT("Current catalog validates"), Restored->State.IsValidFor(Catalog));
    Restored->State.Stock[0].Shelf = -1;
    TestFalse(TEXT("Corrupt negative stock rejected"), Restored->State.IsValidFor(Catalog));
    Restored->State.Stock[0].Shelf = 8;
    Catalog[0].Id = TEXT("different_product");
    TestFalse(TEXT("Changed catalog identity rejected"), Restored->State.IsValidFor(Catalog));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketProgressTest, "MirasMarket.Economy.ProgressAndStorageLimit", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketProgressTest::RunTest(const FString& Parameters)
{
    auto Catalog = TestCatalog(); FMarketState S; S.Initialize(Catalog);
    S.Stock[0].Warehouse = 120;
    TestFalse(TEXT("Warehouse capacity includes pending deliveries"), S.Order(0, Catalog));
    S.Revenue = 20000; S.CostOfGoods = 10000; S.bCashier = true;
    S.bSecondStore = true; S.Served = 30; S.Lost = 0;
    S.CloseDay();
    TestEqual(TEXT("Staff cost counted"), S.LastOperatingCost, int64(4200));
    TestEqual(TEXT("Branch contributes separately"), S.LastBranchProfit, int64(1675));
    TestEqual(TEXT("Profit includes branch net and payroll"), S.LastProfit, int64(7475));
    TestEqual(TEXT("Profitable day advances milestone"), S.ProfitableDays, 1);
    TestTrue(TEXT("Good service improves local share"), S.MarketShare > 25);
    FMarketState Quiet; Quiet.Initialize(Catalog);
    const float ShareBeforeQuietDay = Quiet.MarketShare;
    Quiet.CloseDay();
    TestEqual(TEXT("A day without visitors preserves market share"), Quiet.MarketShare, ShareBeforeQuietDay);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketQueueTest, "MirasMarket.Customers.QueueArrivalOrder", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketQueueTest::RunTest(const FString& Parameters)
{
    FMarketCustomer Approaching; Approaching.Stage = 1;
    FMarketCustomer ArrivedLater; ArrivedLater.Stage = 2; ArrivedLater.QueueTicket = 8;
    FMarketCustomer ArrivedFirst; ArrivedFirst.Stage = 2; ArrivedFirst.QueueTicket = 3;
    const TArray<FMarketCustomer> Queue = { Approaching, ArrivedLater, ArrivedFirst };
    TestEqual(TEXT("First arrival is served first regardless of array order"), FMarketQueueRules::FindFront(Queue), 2);
    TestEqual(TEXT("First arrival stands at the front"), FMarketQueueRules::Rank(Queue, 2), 0);
    TestEqual(TEXT("Later arrival stands behind"), FMarketQueueRules::Rank(Queue, 1), 1);
    TestEqual(TEXT("Customer still walking has no queue rank"), FMarketQueueRules::Rank(Queue, 0), INDEX_NONE);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketCatalogTest, "MirasMarket.Catalog.ParseAndSerialize", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketCatalogTest::RunTest(const FString& Parameters)
{
    const FString Json = TEXT("{\"note\":\"n\",\"products\":["
        "{\"id\":\"milk_1l\",\"realName\":\"Sut\",\"cost\":1.70,\"price\":2.50,\"color\":\"EEF1E9\"},"
        "{\"id\":\"box_tea\",\"realName\":\"Cay \\\"Rize\\\"\",\"fictionalName\":\"Yamac\",\"category\":\"icecek\",\"cost\":4.8,\"price\":6.75,\"caseUnits\":6,"
        "\"visual\":{\"package\":\"/Game/Products/Packages/box_70x50x200/SM_box_70x50x200.SM_box_70x50x200\",\"material\":\"/Game/Products/Items/box_tea/MI_box_tea.MI_box_tea\"}},"
        "{\"id\":\"glass_jar\",\"realName\":\"Kavanoz\",\"cost\":2,\"price\":3,"
        "\"visual\":{\"package\":\"/Game/Products/Packages/model_jar/SM_model_jar.SM_model_jar\",\"materials\":[\"/Game/L.L\",\"\",\"/Game/C.C\"]}},"
        "{\"id\":\"Bad Id\",\"realName\":\"x\",\"cost\":1,\"price\":2},"
        "{\"id\":\"milk_1l\",\"realName\":\"dup\",\"cost\":1,\"price\":2},"
        "{\"id\":\"free_item\",\"realName\":\"x\",\"cost\":0,\"price\":2}]}");
    TArray<FMarketProduct> Products; TArray<FString> Errors; FString Note;
    TestTrue(TEXT("Catalog JSON readable"), MarketCatalog::Parse(Json, Products, Errors, &Note));
    TestEqual(TEXT("Only valid unique rows kept"), Products.Num(), 3);
    TestEqual(TEXT("Invalid id, duplicate and zero cost reported"), Errors.Num(), 3);
    if (Products.Num() != 3) return false;
    TestEqual(TEXT("v1 row defaults fictional name"), Products[0].FictionalName, FString(TEXT("Sut")));
    TestEqual(TEXT("v1 row defaults case units"), Products[0].CaseUnits, 12);
    TestEqual(TEXT("Money parsed in kurus"), Products[0].Cost, int64(170));
    TestEqual(TEXT("Case units read"), Products[1].CaseUnits, 6);
    TestTrue(TEXT("Visual mesh read"), Products[1].MeshPath.StartsWith(TEXT("/Game/Products/Packages/")));
    TestTrue(TEXT("Legacy single material maps to slot 0"), Products[1].Materials.Num() == 1 && Products[1].Materials[0].EndsWith(TEXT("MI_box_tea")));
    TestTrue(TEXT("Per-slot materials keep empty slots"), Products[2].Materials.Num() == 3 && Products[2].Materials[1].IsEmpty() && Products[2].Materials[2] == TEXT("/Game/C.C"));
    TArray<FMarketProduct> Again; TArray<FString> Errors2; FString Note2;
    TestTrue(TEXT("Serialized catalog parses"), MarketCatalog::Parse(MarketCatalog::Serialize(Products, Note), Again, Errors2, &Note2));
    TestEqual(TEXT("Round trip keeps rows"), Again.Num(), 3);
    TestEqual(TEXT("Round trip has no errors"), Errors2.Num(), 0);
    TestEqual(TEXT("Round trip keeps note"), Note2, Note);
    if (Again.Num() == 3)
    {
        TestEqual(TEXT("Round trip escapes quotes"), Again[1].RealName, Products[1].RealName);
        TestEqual(TEXT("Round trip keeps price"), Again[1].BasePrice, int64(675));
        TestTrue(TEXT("Round trip keeps slot materials"), Again[2].Materials == Products[2].Materials && Again[1].Materials == Products[1].Materials);
        TestTrue(TEXT("Round trip keeps color"), Again[0].Color == Products[0].Color);
    }
    {
        FMarketProduct Draft = Products[0];
        Draft.Id = TEXT("prep_item"); Draft.bActive = false; Draft.Brand = TEXT("Marka");
        Draft.PackageType = TEXT("cam_sise"); Draft.DiameterMm = 62; Draft.HeightMm = 230; Draft.LabelHeightMm = 70;
        Draft.Parts = TEXT("Etiket,Cam,Kapak"); Draft.Notes = TEXT("yesil cam"); Draft.bSizeEstimated = true;
        Draft.Preset = TEXT("cam_sise_250ml"); Draft.Colors = TEXT("Cam=2B1A12/0.85;Kapak=E30613");
        Draft.MeshPath = TEXT("/Game/Products/Packages/model_test/SM_model_test.SM_model_test");
        Draft.VisualScale = 0.01f; Draft.VisualRotation = FRotator(0.f, 90.f, -90.f); Draft.VisualOffsetCm = FVector(1.5f, -2.f, 3.f);
        TArray<FMarketProduct> WithPrep = Products; WithPrep.Add(Draft);
        TArray<FMarketProduct> Back; TArray<FString> Errors3;
        TestTrue(TEXT("Catalog with preparation item parses"), MarketCatalog::Parse(MarketCatalog::Serialize(WithPrep, Note), Back, Errors3));
        TestEqual(TEXT("Only active products count toward the game limit"), MarketCatalog::CountActive(Back), 3);
        if (Back.Num() == 4)
        {
            const FMarketProduct& R = Back[3];
            TestTrue(TEXT("Preparation data round trip"), !R.bActive && R.Brand == TEXT("Marka") && R.PackageType == TEXT("cam_sise") && R.DiameterMm == 62 &&
                R.HeightMm == 230 && R.LabelHeightMm == 70 && R.Parts == TEXT("Etiket,Cam,Kapak") && R.Notes == TEXT("yesil cam") && R.bSizeEstimated &&
                R.Preset == TEXT("cam_sise_250ml") && R.Colors == TEXT("Cam=2B1A12/0.85;Kapak=E30613"));
            TestTrue(TEXT("Imported model correction round trip"), FMath::IsNearlyEqual(R.VisualScale, 0.01f) && R.VisualRotation.Equals(Draft.VisualRotation) && R.VisualOffsetCm.Equals(Draft.VisualOffsetCm));
            TestTrue(TEXT("Active products default to active"), Back[0].bActive);
        }
        else AddError(TEXT("Preparation item lost in round trip"));
    }
    TestEqual(TEXT("Turkish slug"), MarketCatalog::MakeId(TEXT("\u00c7aykur R\u0130ZE \u00e7ay\u0131 500 g")), FString(TEXT("caykur_rize_cayi_500_g")));
    TestTrue(TEXT("Slug is a valid id"), MarketCatalog::IsValidId(MarketCatalog::MakeId(TEXT("1 L S\u00fct"))));
    int64 Kurus = 0;
    TestTrue(TEXT("Comma decimal money"), MarketCatalog::ParseMoney(TEXT("2,75"), Kurus) && Kurus == 275);
    TestFalse(TEXT("Negative money rejected"), MarketCatalog::ParseMoney(TEXT("-1"), Kurus));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBoxLayoutTest, "MirasMarket.Catalog.BoxLayout", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketBoxLayoutTest::RunTest(const FString& Parameters)
{
    FBoxPackageLayout L;
    TestTrue(TEXT("Reference box accepted"), FBoxPackageLayout::Make(70, 50, 200, L));
    // Values from Docs/Planlama/03 section 7 (8 px per mm).
    TestTrue(TEXT("Front rect"), L.Rects[FBoxPackageLayout::Front] == FIntRect(16, 16, 576, 1616));
    TestEqual(TEXT("Back x"), L.Rects[FBoxPackageLayout::Back].Min.X, 592);
    TestEqual(TEXT("Right x"), L.Rects[FBoxPackageLayout::Right].Min.X, 1168);
    TestEqual(TEXT("Left x"), L.Rects[FBoxPackageLayout::Left].Min.X, 1584);
    TestTrue(TEXT("Top rect"), L.Rects[FBoxPackageLayout::Top] == FIntRect(16, 1632, 576, 2032));
    TestEqual(TEXT("Bottom x"), L.Rects[FBoxPackageLayout::Bottom].Min.X, 592);
    TestFalse(TEXT("Degenerate box rejected"), FBoxPackageLayout::Make(0, 50, 200, L));
    const int32 Sizes[][3] = { {70, 50, 200}, {300, 20, 40}, {40, 40, 40}, {10, 400, 30} };
    for (const auto& S : Sizes)
    {
        FBoxPackageLayout B;
        if (!TestTrue(TEXT("Box accepted"), FBoxPackageLayout::Make(S[0], S[1], S[2], B))) continue;
        for (int32 F = 0; F < FBoxPackageLayout::FaceCount; ++F)
        {
            const FIntRect R = B.Rects[F];
            TestTrue(TEXT("Face inside atlas"), R.Min.X >= 0 && R.Min.Y >= 0 && R.Max.X <= B.AtlasSize && R.Max.Y <= B.AtlasSize && R.Width() > 0 && R.Height() > 0);
            for (int32 G = F + 1; G < FBoxPackageLayout::FaceCount; ++G)
            {
                const FIntRect O = B.Rects[G];
                TestFalse(TEXT("Faces do not overlap"), R.Min.X < O.Max.X && O.Min.X < R.Max.X && R.Min.Y < O.Max.Y && O.Min.Y < R.Max.Y);
            }
            FVector C[4]; FVector2D UV[4]; FVector N;
            B.GetFace(F, C, UV, N);
            const FVector Mid = (C[0] + C[2]) * 0.5;
            const FVector BoxCenter(0, 0, S[2] / 20.0);
            TestTrue(TEXT("Normal points outward"), FVector::DotProduct(Mid - BoxCenter, N) > 0);
            TestTrue(TEXT("Quad is planar on its normal"), FMath::IsNearlyZero(FVector::DotProduct(C[1] - C[0], N)) && FMath::IsNearlyZero(FVector::DotProduct(C[3] - C[0], N)));
            // Image must not be mirrored. In Unreal's left-handed axes (looking along -X, right is -Y)
            // image-right x image-down equals the outward normal for an unmirrored face.
            const FVector Right = C[1] - C[0];
            const FVector Down = C[3] - C[0];
            TestTrue(TEXT("Label is not mirrored"), FVector::DotProduct(FVector::CrossProduct(Right, Down), N) > 0);
            TestTrue(TEXT("UV starts top-left"), UV[0].X < UV[1].X && UV[0].Y < UV[3].Y);
        }
    }
    int32 W = 0, D = 0, H = 0;
    TestTrue(TEXT("Package id round trip"), FBoxPackageLayout::ParsePackageId(FBoxPackageLayout::PackageId(70, 50, 200), W, D, H) && W == 70 && D == 50 && H == 200);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketReconcileTest, "MirasMarket.Save.ReconcileCatalog", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketReconcileTest::RunTest(const FString& Parameters)
{
    auto Catalog = TestCatalog();
    FMarketState S; S.Initialize(Catalog);
    S.Stock[0].Shelf = 5; S.Stock[0].Price = 999;
    FMarketProduct Tea; Tea.Id = TEXT("tea"); Tea.Cost = 480; Tea.BasePrice = 675; Tea.CaseUnits = 6;
    TArray<FMarketProduct> Grown = { Tea, Catalog[0] };
    TArray<FString> Added, Removed;
    TestEqual(TEXT("One product added"), S.ReconcileWith(Grown, &Added, &Removed), 1);
    TestTrue(TEXT("Reconciled state matches new catalog"), S.IsValidFor(Grown));
    TestEqual(TEXT("New product starts empty"), S.Stock[0].Shelf + S.Stock[0].Warehouse, 0);
    TestEqual(TEXT("Existing stock kept by id"), S.Stock[1].Shelf, 5);
    TestEqual(TEXT("Out-of-range price clamped"), S.Stock[1].Price, int64(750));
    TestTrue(TEXT("Order uses case units"), S.Order(0, Grown));
    TestEqual(TEXT("Six units in transit"), S.Stock[0].Incoming, 6);
    TestEqual(TEXT("Case bill uses case units"), S.Cash, int64(35000 - 480 * 6));
    TestEqual(TEXT("Removing a product"), S.ReconcileWith({ Tea }, &Added, &Removed), 1);
    TestEqual(TEXT("Removed id reported"), Removed.Num() > 0 ? Removed[0] : FString(), FString(TEXT("milk")));
    const FMarketStock Duplicate = S.Stock[0];
    S.Stock.Add(Duplicate);
    TestFalse(TEXT("Duplicate ids rejected"), S.IsStructurallyValid());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStaffTest, "MirasMarket.Staff.Planner", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStaffTest::RunTest(const FString& Parameters)
{
    // Drinks gondola: Cola A and Ayran on level 3 (index 2); Cola B and a shampoo are not on a shelf.
    FMarketProduct ColaA; ColaA.Id = TEXT("cola_a"); ColaA.Brand = TEXT("Cola"); ColaA.Category = TEXT("i\u00e7ecek");
    ColaA.PackageType = TEXT("kutu"); ColaA.WidthMm = 80; ColaA.DepthMm = 80; ColaA.HeightMm = 250; ColaA.CaseUnits = 12; ColaA.Cost = 100; ColaA.BasePrice = 200;
    FMarketProduct Shampoo = ColaA; Shampoo.Id = TEXT("sampuan"); Shampoo.Brand = TEXT("Temiz"); Shampoo.Category = TEXT("ki\u015fisel bak\u0131m");
    FMarketProduct ColaB = ColaA; ColaB.Id = TEXT("cola_b");
    FMarketProduct Ayran = ColaA; Ayran.Id = TEXT("ayran"); Ayran.Brand = TEXT("Ayranci"); Ayran.Category = TEXT("\u0130\u00c7ECEK");
    Ayran.WidthMm = 70; Ayran.DepthMm = 70; Ayran.HeightMm = 200;
    const TArray<FMarketProduct> Catalog = { ColaA, Shampoo, ColaB, Ayran };
    TestTrue(TEXT("Categories match without case / Turkish letters"), StaffPlanner::SameCategory(Ayran.Category, TEXT("icecek")));
    TestFalse(TEXT("Empty category matches nothing"), StaffPlanner::SameCategory(TEXT(""), TEXT("")));

    FMarketPlanogram P;
    FPlanogramFixture Drinks; Drinks.Id = TEXT("icecek"); Drinks.Category = TEXT("i\u00e7ecek"); P.Fixtures.Add(Drinks);
    FPlanogramFixture Milk; Milk.Id = TEXT("sut"); Milk.EquipmentId = TEXT("wall_shelf_2400"); Milk.Category = TEXT("s\u00fct"); P.Fixtures.Add(Milk);
    FString M;
    FPlanogramPlacement Wish; Wish.FixtureId = Drinks.Id; Wish.Level = 2; Wish.Facings = 2;
    Wish.ProductId = ColaA.Id;
    TestTrue(TEXT("Cola A on the shelf"), MarketPlanogramEdit::AddBlock(P, Catalog, Wish, -40.f, 1.f, M));
    Wish.ProductId = Ayran.Id;
    TestTrue(TEXT("Ayran on the shelf"), MarketPlanogramEdit::AddBlock(P, Catalog, Wish, 40.f, 1.f, M));

    FMarketState S; S.Initialize(Catalog);
    const auto Refresh = [&]()
    {
        TArray<int32> Capacities;
        for (const FMarketProduct& Product : Catalog) Capacities.Add(MarketPlanogram::ProductCapacity(P, Catalog, Product.Id));
        S.ApplyShelfCapacities(Capacities);
    };
    const auto FillAll = [&]() { for (int32 I = 0; I < S.Stock.Num(); ++I) S.FillShelfFree(I); };
    Refresh();
    const TSet<int32> Nobody;
    TSet<int32> NoRoomSet;

    StaffPlanner::FJob Job = StaffPlanner::ChooseJob(P, Catalog, S, Nobody, NoRoomSet, true);
    TestTrue(TEXT("Empty shelves come first"), Job.Kind == StaffPlanner::EJob::Refill && Job.Product == 0);

    FillAll();
    TArray<TPair<int32, FString>> NoRoom;
    Job = StaffPlanner::ChooseJob(P, Catalog, S, Nobody, NoRoomSet, true, &NoRoom);
    if (!TestTrue(TEXT("Product with stock but no shelf gets placed"), Job.Kind == StaffPlanner::EJob::Place && Job.Product == 2)) return false;
    TestTrue(TEXT("Shampoo has no shelf of its category"), NoRoom.Num() == 1 && NoRoom[0].Key == 1 && NoRoom[0].Value.Contains(TEXT("reyon")));
    TestTrue(TEXT("New block goes on its category's shelf, same row as its brand"), Job.Block.FixtureId == Drinks.Id && Job.Block.Face == TEXT("front") && Job.Block.Level == 2);
    const float ColaARight = -40.f + MarketPlanogram::BlockWidthCm(ColaA, P.Placements[0]) * .5f;
    const float NewLeft = Job.Block.XCm - MarketPlanogram::BlockWidthCm(ColaB, Job.Block) * .5f;
    TestTrue(TEXT("Right next to the same brand"), FMath::Abs(NewLeft - ColaARight) < 1.f);
    TestEqual(TEXT("Enough facings for one case (12 / 3 deep)"), Job.Block.Facings, 4);
    TestTrue(TEXT("Worker puts the planned block"), StaffPlanner::TryPlace(P, Catalog, Job.Block, M));
    TestEqual(TEXT("Cola B has shelf space now"), MarketPlanogram::ProductCapacity(P, Catalog, ColaB.Id), 12);
    for (int32 I = 0; I < P.Placements.Num(); ++I) TestTrue(TEXT("Every block fits"), MarketPlanogram::BlockFits(P, Catalog, I));

    Refresh();
    FillAll();
    NoRoomSet.Add(1);
    Job = StaffPlanner::ChooseJob(P, Catalog, S, Nobody, NoRoomSet, true);
    TestTrue(TEXT("A block that cannot hold one case is widened"), Job.Kind == StaffPlanner::EJob::Widen && Job.Product == 0);
    const float ColaACenter = P.Placements[0].XCm;
    TestTrue(TEXT("Worker widens the block"), StaffPlanner::TryWiden(P, Catalog, Job.Block, M));
    TestEqual(TEXT("One more facing"), P.Placements[0].Facings, 3);
    TestTrue(TEXT("Widened block moved only a little"), FMath::Abs(P.Placements[0].XCm - ColaACenter) < 9.f);
    for (int32 I = 0; I < P.Placements.Num(); ++I) TestTrue(TEXT("Every block still fits"), MarketPlanogram::BlockFits(P, Catalog, I));

    TSet<int32> ColaBusy; ColaBusy.Add(0);
    Job = StaffPlanner::ChooseJob(P, Catalog, S, ColaBusy, NoRoomSet, true);
    TestTrue(TEXT("Another worker's product is skipped"), Job.Kind == StaffPlanner::EJob::Widen && Job.Product == 3);
    Job = StaffPlanner::ChooseJob(P, Catalog, S, Nobody, NoRoomSet, false);
    TestTrue(TEXT("While the player arranges: no plan edits"), Job.Kind == StaffPlanner::EJob::None);

    S.Stock[0].Shelf = 0;
    TestEqual(TEXT("A worker moves one unit at a time"), S.Restock(0, 1), 1);
    TestEqual(TEXT("Depot -> shelf"), S.Stock[0].Shelf, 1);
    S.Stockers = 2;
    S.CloseDay();
    TestEqual(TEXT("Workers are paid every day"), S.LastOperatingCost, int64(2200 + 2 * FMarketState::StockerDailyWage));
    TestTrue(TEXT("Save with two workers is valid"), S.IsStructurallyValid());
    S.Stockers = FMarketState::MaxStockers + 1;
    TestFalse(TEXT("Too many workers rejected"), S.IsStructurallyValid());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketDemandTest, "MirasMarket.Customers.PriceAndDemand", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketDemandTest::RunTest(const FString& Parameters)
{
    FMarketProduct Milk; Milk.Id = TEXT("milk"); Milk.Cost = 170; Milk.BasePrice = 250;
    FMarketProduct Cola; Cola.Id = TEXT("cola"); Cola.Cost = 180; Cola.BasePrice = 275;
    FMarketProduct Soap; Soap.Id = TEXT("soap"); Soap.Cost = 1400; Soap.BasePrice = 1990;
    FMarketProduct Ayran; Ayran.Id = TEXT("ayran"); Ayran.Cost = 45; Ayran.BasePrice = 75;
    const TArray<FMarketProduct> Catalog = { Milk, Cola, Soap };
    FMarketState S; S.Initialize(Catalog);
    S.ApplyShelfCapacities({ 10, 10, 0 }); // soap is on no shelf
    S.Stock[0].Shelf = 5; S.Stock[0].Warehouse = 0;
    S.Stock[1].Shelf = 0;

    // Prices
    TestEqual(TEXT("Rival sells at list price"), MarketDemand::RivalPrice(Milk, 1.f), int64(250));
    TestEqual(TEXT("Rival campaign lowers its price"), MarketDemand::RivalPrice(Milk, .85f), int64(213));
    TestEqual(TEXT("Price step for a 2.50 product"), MarketDemand::PriceStep(Milk), int64(15));
    TestEqual(TEXT("Cheap product still moves in 5 kurus"), MarketDemand::PriceStep(Ayran), int64(5));
    TestEqual(TEXT("Price step for an expensive product"), MarketDemand::PriceStep(Soap), int64(100));
    TestTrue(TEXT("Almost everyone buys at the rival's price"), MarketDemand::BuyChance(1.0, 25.f) > 0.95);
    TestTrue(TEXT("Half buy 25 % over the rival at 25 % share"), FMath::Abs(MarketDemand::BuyChance(1.25, 25.f) - 0.5) < 0.01);
    TestTrue(TEXT("Hardly anyone buys 50 % over the rival"), MarketDemand::BuyChance(1.5, 25.f) < 0.05);
    TestTrue(TEXT("Dearer = fewer buyers"), MarketDemand::BuyChance(1.1, 25.f) > MarketDemand::BuyChance(1.2, 25.f));
    TestTrue(TEXT("Loyal shop is forgiven more"), MarketDemand::BuyChance(1.2, 60.f) > MarketDemand::BuyChance(1.2, 10.f));

    // Who wants what
    TestEqual(TEXT("Most shoppers ask for carried products"), MarketDemand::PickWanted(S, 0.1f, 0.99f), 1);
    TestEqual(TEXT("Some ask for any product"), MarketDemand::PickWanted(S, 0.95f, 0.99f), 2);
    FMarketState Bare; Bare.Initialize(Catalog); Bare.ApplyShelfCapacities({ 0, 0, 0 });
    TestEqual(TEXT("Nothing on shelves: shoppers still ask"), MarketDemand::PickWanted(Bare, 0.1f, 0.f), 0);

    // Decisions
    MarketDemand::FVisit V = MarketDemand::Decide(S, Catalog, 2, 0, 1.f, 1, 0.f);
    TestTrue(TEXT("Product on no shelf is not carried"), V.Result == MarketDemand::EVisit::NotCarried);
    MarketDemand::RecordLoss(S, V); MarketDemand::RecordLoss(S, V);
    V = MarketDemand::Decide(S, Catalog, 1, 0, 1.f, 1, 0.f);
    TestTrue(TEXT("Empty shelf"), V.Result == MarketDemand::EVisit::Empty);
    MarketDemand::RecordLoss(S, V);
    V = MarketDemand::Decide(S, Catalog, 0, 5, 1.f, 3, 0.5f);
    TestTrue(TEXT("Fair price: buys"), V.Result == MarketDemand::EVisit::Buy && V.Quantity == 3);
    S.Stock[0].Price = 500;
    V = MarketDemand::Decide(S, Catalog, 0, 5, 1.f, 3, 0.5f);
    TestTrue(TEXT("Double price: too expensive"), V.Result == MarketDemand::EVisit::Expensive);
    MarketDemand::RecordLoss(S, V);
    S.Stock[0].Price = 290;
    V = MarketDemand::Decide(S, Catalog, 0, 5, 1.f, 4, 0.f);
    TestTrue(TEXT("Pricey: only what is needed"), V.Result == MarketDemand::EVisit::Buy && V.Quantity == 2);
    S.Stock[0].Price = 225;
    V = MarketDemand::Decide(S, Catalog, 0, 5, 1.f, 2, 0.5f);
    TestTrue(TEXT("Bargain: one more"), V.Result == MarketDemand::EVisit::Buy && V.Quantity == 3);
    V = MarketDemand::Decide(S, Catalog, 0, 1, 1.f, 3, 0.5f);
    TestTrue(TEXT("Takes what is left on the shelf"), V.Result == MarketDemand::EVisit::Buy && V.Quantity == 1);
    TestEqual(TEXT("Every shopper who did not buy is lost"), S.Lost, 4);

    // Counting and the day report
    TestTrue(TEXT("Sale"), S.Sell(0, 2, 225, Catalog));
    TestEqual(TEXT("Sold units counted"), S.Stock[0].Today.Sold, 2);
    for (int32 I = 0; I < 4; ++I) MarketDemand::RecordWaitingLoss(S);
    TestEqual(TEXT("Waiting shoppers are lost too"), S.Lost, 8);
    TestEqual(TEXT("No report before the day closes"), MarketDemand::TopProblems(S).Num(), 0);
    S.CloseDay();
    TestEqual(TEXT("Yesterday keeps the sales"), S.Stock[0].Yesterday.Sold, 2);
    TestEqual(TEXT("Today starts empty"), S.Stock[0].Today.Sold + S.Stock[2].Today.NotCarried, 0);
    TestEqual(TEXT("Waiting losses move to the report"), S.LastLostWaiting, 4);
    TestEqual(TEXT("New day, no waiting losses"), S.LostWaiting, 0);
    const TArray<MarketDemand::FProblem> Problems = MarketDemand::TopProblems(S, 3);
    TestEqual(TEXT("Three biggest problems"), Problems.Num(), 3);
    if (Problems.Num() == 3)
    {
        TestTrue(TEXT("1: waiting (4)"), Problems[0].Kind == MarketDemand::EProblem::Waiting && Problems[0].Count == 4);
        TestTrue(TEXT("2: soap not carried (2)"), Problems[1].Kind == MarketDemand::EProblem::NotCarried && Problems[1].Product == 2 && Problems[1].Count == 2);
        TestTrue(TEXT("3: tie broken by kind: empty cola before expensive milk"), Problems[2].Kind == MarketDemand::EProblem::Empty && Problems[2].Product == 1);
    }
    TestTrue(TEXT("State with demand stats is still a valid save"), S.IsStructurallyValid());
    return true;
}
#endif

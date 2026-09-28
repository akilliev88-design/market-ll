#include "MarketGame.h"
#include "ProductCatalog.h"
#include "Planogram.h"
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
    MarketPlanogram::Reconcile(Again, Catalog);
    TestNotNull(TEXT("New catalog product assigned"), Again.FindPlacement(TEXT("cola")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketPlanogramWidthTest, "MirasMarket.Planogram.WidthLimit", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketPlanogramWidthTest::RunTest(const FString& Parameters)
{
    // 250 mm packages: two facings = 52 cm, two blocks + gap = 107 cm (fits 110), a third block does not.
    auto Wide = [](const FString& Id, const FString& Category, int32 WidthMm)
    {
        FMarketProduct P; P.Id = Id; P.Category = Category; P.WidthMm = WidthMm; return P;
    };
    TestEqual(TEXT("Block width uses facings and item gap"), MarketPlanogram::BlockWidthCm(Wide(TEXT("x"), TEXT("c"), 250), 2), 52.f);

    FMarketPlanogram Single;
    FPlanogramFixture Wall; Wall.Id = TEXT("duvar"); Wall.EquipmentId = TEXT("wall_single_1200"); Wall.Category = TEXT("c");
    Single.Fixtures.Add(Wall);
    TArray<FMarketProduct> Catalog = { Wide(TEXT("a"), TEXT("c"), 250), Wide(TEXT("b"), TEXT("c"), 250), Wide(TEXT("d"), TEXT("c"), 250) };
    MarketPlanogram::Reconcile(Single, Catalog);
    const FPlanogramPlacement* A = Single.FindPlacement(TEXT("a"));
    const FPlanogramPlacement* B = Single.FindPlacement(TEXT("b"));
    const FPlanogramPlacement* D = Single.FindPlacement(TEXT("d"));
    if (!TestNotNull(TEXT("a placed"), A) || !TestNotNull(TEXT("b placed"), B) || !TestNotNull(TEXT("d placed"), D)) return false;
    TestEqual(TEXT("Two blocks share the first level"), B->Level, A->Level);
    TestNotEqual(TEXT("Third block moves to another level"), D->Level, A->Level);
    TestEqual(TEXT("Single sided fixture never uses the back face"), D->Face, FString(TEXT("front")));
    TestTrue(TEXT("Level stays within usable width"), MarketPlanogram::LevelUsedWidthCm(Single, Catalog, Wall.Id, TEXT("front"), A->Level) <= MarketPlanogram::UsableWidthCm);
    TArray<FString> Warnings;
    MarketPlanogram::FindOverflows(Single, Catalog, Warnings);
    TestEqual(TEXT("Automatic plan has no overflow"), Warnings.Num(), 0);
    TestFalse(TEXT("Third block does not fit next to the others"),
        MarketPlanogram::FitsOnLevel(Single, Catalog, Wall.Id, TEXT("front"), A->Level, TEXT("d"), 2));

    Single.FindPlacement(TEXT("d"))->Level = A->Level;
    Warnings.Reset();
    MarketPlanogram::FindOverflows(Single, Catalog, Warnings);
    TestEqual(TEXT("Hand-made overflow is reported once"), Warnings.Num(), 1);

    // Double sided gondola: 4 levels x 2 blocks on the front, the ninth product goes to the back face.
    FMarketPlanogram Double;
    FPlanogramFixture Gondola; Gondola.Id = TEXT("gondol"); Gondola.Category = TEXT("c");
    Double.Fixtures.Add(Gondola);
    TArray<FMarketProduct> Many;
    for (int32 I = 0; I < 9; ++I) Many.Add(Wide(FString::Printf(TEXT("p%d"), I), TEXT("c"), 250));
    MarketPlanogram::Reconcile(Double, Many);
    const FPlanogramPlacement* Ninth = Double.FindPlacement(TEXT("p8"));
    if (!TestNotNull(TEXT("ninth placed"), Ninth)) return false;
    TestEqual(TEXT("Ninth product uses the back face"), Ninth->Face, FString(TEXT("back")));
    TestEqual(TEXT("Ninth product keeps two facings"), Ninth->Facings, 2);
    Warnings.Reset();
    MarketPlanogram::FindOverflows(Double, Many, Warnings);
    TestEqual(TEXT("Full gondola has no overflow"), Warnings.Num(), 0);

    // A package wider than the shelf cannot fit anywhere: it is still placed (1 facing) and reported.
    FMarketPlanogram Tiny;
    Tiny.Fixtures.Add(Wall);
    TArray<FMarketProduct> Huge = { Wide(TEXT("dev"), TEXT("c"), 2000) };
    MarketPlanogram::Reconcile(Tiny, Huge);
    const FPlanogramPlacement* Dev = Tiny.FindPlacement(TEXT("dev"));
    if (!TestNotNull(TEXT("oversized product still placed"), Dev)) return false;
    TestEqual(TEXT("Oversized product falls back to one facing"), Dev->Facings, 1);
    Warnings.Reset();
    MarketPlanogram::FindOverflows(Tiny, Huge, Warnings);
    TestEqual(TEXT("Oversized product is reported"), Warnings.Num(), 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketPlanogramFillTest, "MirasMarket.Planogram.FillToCapacity", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketPlanogramFillTest::RunTest(const FString& Parameters)
{
    // Two 95 mm cartons (64 mm deep) share a 110 cm level: widths 11.5 cm per facing.
    FMarketProduct A; A.Id = TEXT("sut_a"); A.WidthMm = 95; A.DepthMm = 64;
    FMarketProduct B; B.Id = TEXT("sut_b"); B.WidthMm = 95; B.DepthMm = 64;
    TArray<FMarketProduct> Catalog = { A, B };
    FMarketPlanogram P;
    FPlanogramFixture Fixture; Fixture.Id = TEXT("gondol"); P.Fixtures.Add(Fixture);
    FPlanogramPlacement PA; PA.ProductId = A.Id; PA.FixtureId = Fixture.Id; PA.Level = 1; PA.Facings = 1; PA.Depth = 1; PA.Order = 0;
    FPlanogramPlacement PB = PA; PB.ProductId = B.Id; PB.Order = 1;
    P.Placements = { PA, PB };
    TestEqual(TEXT("Depth rows follow shelf depth (37 cm / 8.4 cm)"), MarketPlanogram::DepthThatFits(A), 4);
    FMarketPlanogram Whole = P; // copy before filling: used for the empty-level test below
    const int32 Added = MarketPlanogram::FillToCapacity(P, Catalog, false);
    const FPlanogramPlacement* FA = P.FindPlacement(A.Id);
    const FPlanogramPlacement* FB = P.FindPlacement(B.Id);
    if (!TestNotNull(TEXT("a"), FA) || !TestNotNull(TEXT("b"), FB)) return false;
    TestEqual(TEXT("Free width is shared as facings (9 in total)"), FA->Facings + FB->Facings, 9);
    TestTrue(TEXT("Brands share evenly"), FMath::Abs(FA->Facings - FB->Facings) <= 1);
    TestEqual(TEXT("Seven facings were added"), Added, 7);
    TestEqual(TEXT("Depth filled"), FA->Depth, 4);
    TestTrue(TEXT("Level still fits"), MarketPlanogram::LevelUsedWidthCm(P, Catalog, Fixture.Id, TEXT("front"), 1) <= MarketPlanogram::UsableWidthCm);
    TestEqual(TEXT("Capacity = facings x depth"), MarketPlanogram::Capacity(*FA), FA->Facings * 4);
    TestEqual(TEXT("Second fill adds nothing"), MarketPlanogram::FillToCapacity(P, Catalog, false), 0);

    // Empty levels: the 7 other level/faces of the double gondola get extra blocks of the same two
    // products, each filling its whole level. Primary placements stay first and unchanged in place.
    MarketPlanogram::FillToCapacity(Whole, Catalog, true);
    TestEqual(TEXT("Every level/face holds a block"), Whole.Placements.Num(), 9);
    int32 Extras = 0;
    for (const FPlanogramPlacement& Block : Whole.Placements) if (Block.bExtra) ++Extras;
    TestEqual(TEXT("Seven extra blocks"), Extras, 7);
    TestFalse(TEXT("FindPlacement returns the authored block"), Whole.FindPlacement(A.Id)->bExtra);
    TestEqual(TEXT("Authored block keeps its level"), Whole.FindPlacement(A.Id)->Level, 1);
    TArray<FString> FillWarnings;
    MarketPlanogram::FindOverflows(Whole, Catalog, FillWarnings);
    TestEqual(TEXT("Filled gondola has no overflow"), FillWarnings.Num(), 0);
    TestTrue(TEXT("Product capacity sums all blocks"), MarketPlanogram::ProductCapacity(Whole, A.Id) > MarketPlanogram::Capacity(*Whole.FindPlacement(A.Id)));
    FMarketPlanogram Reparsed; TArray<FString> ReparseErrors;
    TestTrue(TEXT("Serialized filled plan parses"), MarketPlanogram::Parse(MarketPlanogram::Serialize(Whole), Reparsed, ReparseErrors));
    TestEqual(TEXT("Extra blocks are never saved"), Reparsed.Placements.Num(), 2);

    // Wall shelf: single sided, 5 levels, 230 cm; a same-category product fills it without being placed there.
    FMarketPlanogram Wall;
    FPlanogramFixture Shelf; Shelf.Id = TEXT("duvar"); Shelf.EquipmentId = TEXT("wall_shelf_2400"); Shelf.Category = TEXT("cay");
    Wall.Fixtures.Add(Shelf);
    FMarketProduct Tea; Tea.Id = TEXT("cay_a"); Tea.Category = TEXT("cay"); Tea.WidthMm = 115; Tea.DepthMm = 65;
    TArray<FMarketProduct> TeaCatalog = { Tea };
    MarketPlanogram::FillToCapacity(Wall, TeaCatalog, true);
    TestEqual(TEXT("Wall shelf gets one block per level"), Wall.Placements.Num(), 5);
    TestFalse(TEXT("Wall shelf is single sided"), MarketPlanogram::IsDoubleSided(Shelf));
    for (const FPlanogramPlacement& Block : Wall.Placements)
        TestTrue(TEXT("Wall block fits 230 cm"), MarketPlanogram::LevelUsedWidthCm(Wall, TeaCatalog, Shelf.Id, Block.Face, Block.Level) <= 230.f);
    TestEqual(TEXT("Wall facings fill the width (13.5 cm each)"), Wall.Placements[0].Facings, 17);

    FMarketPlanogram Parsed; TArray<FString> Errors;
    TestTrue(TEXT("autoFill round trip parses"), MarketPlanogram::Parse(TEXT("{\"autoFill\":false,\"fixtures\":[]}"), Parsed, Errors));
    TestFalse(TEXT("autoFill false is read"), Parsed.bAutoFill);
    FMarketPlanogram Again; TArray<FString> Errors2;
    TestTrue(TEXT("Serialized autoFill parses"), MarketPlanogram::Parse(MarketPlanogram::Serialize(Parsed), Again, Errors2));
    TestFalse(TEXT("autoFill survives serialize"), Again.bAutoFill);
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
    TestEqual(TEXT("Restock moves the whole warehouse"), S.Restock(0), 16);
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
    S.Stock[0].Capacity = 0;
    TestFalse(TEXT("Zero capacity rejected"), S.IsStructurallyValid());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketInventoryTest, "MirasMarket.Economy.InventoryAndOrder", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketInventoryTest::RunTest(const FString& Parameters)
{
    auto Catalog = TestCatalog(); FMarketState S; S.Initialize(Catalog);
    TestEqual(TEXT("Shelf filled from warehouse"), S.Restock(0), 8);
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
    S.Stock[0].Price = 300;
    TestTrue(TEXT("Existing basket respects its 250 kurus quote"), S.Sell(0, 4, 250, Catalog));
    TestEqual(TEXT("Cash includes exact receipt"), S.Cash, int64(36000));
    TestEqual(TEXT("Sold inventory removed"), S.Stock[0].Shelf, 12);
    TestEqual(TEXT("Cost of goods recorded"), S.CostOfGoods, int64(680));
    S.CloseDay();
    TestEqual(TEXT("Profit is revenue minus cost and daily expenses"), S.LastProfit, int64(-1880));
    TestEqual(TEXT("Day cost removed from cash"), S.Cash, int64(33800));
    TestEqual(TEXT("Daily revenue reset"), S.Revenue, int64(0));
    TestEqual(TEXT("Previous revenue preserved"), S.LastRevenue, int64(1000));
    TestEqual(TEXT("Losing day gives no profitable-day unlock"), S.ProfitableDays, 0);
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
#endif

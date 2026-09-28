#include "MarketBranches.h"
#include "MarketLayout.h"
#include "MarketStaff.h"
#include "MarketGoods.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketBranchesTest
{
    FMarketProduct Make(const TCHAR* Id, const TCHAR* Category, const TCHAR* Brand, int64 Cost, int64 Price, int32 W, int32 D, int32 H, int32 Case = 12)
    {
        FMarketProduct P;
        P.Id = Id; P.RealName = Id; P.Category = Category; P.Brand = Brand; P.Cost = Cost; P.BasePrice = Price;
        P.WidthMm = W; P.DepthMm = D; P.HeightMm = H; P.CaseUnits = Case; P.PackageType = TEXT("kutu");
        return P;
    }

    TArray<FMarketProduct> Catalog()
    {
        return {
            Make(TEXT("sutas_sut"), TEXT("s\u00fct"), TEXT("S\u00fcta\u015f"), 170, 250, 70, 70, 200),
            Make(TEXT("pinar_sut"), TEXT("s\u00fct"), TEXT("P\u0131nar"), 170, 245, 70, 70, 200),
            Make(TEXT("cola"), TEXT("i\u00e7ecek"), TEXT("Coca-Cola"), 180, 275, 80, 80, 260),
            Make(TEXT("gazoz"), TEXT("i\u00e7ecek"), TEXT("Uluda\u011f"), 60, 100, 60, 60, 150, 24),
            Make(TEXT("cay"), TEXT("\u00e7ay-kahve"), TEXT("\u00c7aykur"), 480, 675, 120, 60, 180),
            Make(TEXT("biskuvi"), TEXT("bisk\u00fcvi-\u00e7ikolata"), TEXT("\u00dclker"), 100, 175, 150, 50, 40),
            Make(TEXT("makarna"), TEXT("makarna-bakliyat"), TEXT("Barilla"), 210, 325, 190, 40, 70),
            Make(TEXT("deterjan"), TEXT("temizlik"), TEXT("Ariel"), 1400, 1990, 250, 180, 300, 4),
        };
    }

    const FPlanogramPlacement* BlockOf(const FMarketPlanogram& Plan, const TCHAR* Id)
    {
        return Plan.Placements.FindByPredicate([Id](const FPlanogramPlacement& P) { return P.ProductId == Id; });
    }

    void Close(FMarketState& S, const TArray<FMarketProduct>& Products)
    {
        S.DayNews.Reset();
        S.CloseDay();
        MarketBranches::CloseDay(S, Products);
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketLayoutTest, "MirasMarket.Layout.AutomaticPlan", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketLayoutTest::RunTest(const FString& Parameters)
{
    using namespace MarketBranchesTest;
    const TArray<FMarketProduct> Products = Catalog();
    TestTrue(TEXT("Drinks before dairy on the customer path"), MarketLayout::AisleRank(TEXT("i\u00e7ecek")) < MarketLayout::AisleRank(TEXT("s\u00fct")));
    TestTrue(TEXT("Household last"), MarketLayout::AisleRank(TEXT("temizlik")) > MarketLayout::AisleRank(TEXT("s\u00fct")));

    FMarketPlanogram Plan = MarketLayout::Fixtures(TEXT("mahalle"));
    TestEqual(TEXT("The family shop's fixtures"), Plan.Fixtures.Num(), 10);
    const TArray<float> Demand = { 20.f, 12.f, 15.f, 8.f, 6.f, 10.f, 5.f, 2.f };
    const MarketLayout::FResult Result = MarketLayout::Plan(Plan, Products, Demand);
    TestEqual(TEXT("Everything found a place"), Result.Placed, Products.Num());
    TestEqual(TEXT("No room problems"), Result.NoRoom.Num(), 0);

    // Aisles along the path; cleaning away from food.
    auto FixtureOf = [&Plan](const TCHAR* Id) { const FPlanogramPlacement* B = BlockOf(Plan, Id); return B ? Plan.FindFixture(B->FixtureId) : nullptr; };
    const FPlanogramFixture* Drinks = FixtureOf(TEXT("cola"));
    const FPlanogramFixture* Dairy = FixtureOf(TEXT("sutas_sut"));
    const FPlanogramFixture* Cleaning = FixtureOf(TEXT("deterjan"));
    TestTrue(TEXT("Blocks exist"), Drinks && Dairy && Cleaning);
    if (!Drinks || !Dairy || !Cleaning) return false;
    TestTrue(TEXT("Drinks nearer the entrance than milk"), Drinks->Location.Y <= Dairy->Location.Y);
    TestTrue(TEXT("Cleaning on a non-food fixture"), MarketGoods::Classify(Cleaning->Category) == MarketGoods::EGroup::Household);
    TestTrue(TEXT("Cleaning not next to the milk"), Cleaning->Id != Dairy->Id);
    // Heavy detergent at the bottom; every product at least one case.
    TestEqual(TEXT("Heavy box on the bottom shelf"), BlockOf(Plan, TEXT("deterjan"))->Level, 0);
    const TArray<int32> Capacities = MarketLayout::Capacities(Plan, Products);
    for (int32 I = 0; I < Products.Num(); ++I)
        TestTrue(*FString::Printf(TEXT("One case of %s fits"), *Products[I].Id), Capacities[I] >= FMath::Min(12, Products[I].CaseUnits));
    // Busier milk gets at least as much room as the quieter one of the same aisle.
    TestTrue(TEXT("Space follows demand"), Capacities[0] >= Capacities[1]);
    // Brand blocks: the two milks are on the same fixture face.
    const FPlanogramPlacement* A = BlockOf(Plan, TEXT("sutas_sut"));
    const FPlanogramPlacement* B = BlockOf(Plan, TEXT("pinar_sut"));
    TestTrue(TEXT("Milks side by side"), A && B && A->FixtureId == B->FixtureId);
    // Every block obeys the hand-arranging rules.
    for (int32 I = 0; I < Plan.Placements.Num(); ++I) TestTrue(TEXT("Block fits"), MarketPlanogram::BlockFits(Plan, Products, I));

    // A saved layout copies to another shop with the same fixtures.
    FMarketPlanogram Other = MarketLayout::Fixtures(TEXT("mahalle"));
    TestEqual(TEXT("Template copied"), MarketLayout::CopyTemplate(Plan, Other, Products), Plan.Placements.Num());
    // A small shop has room for fewer blocks.
    FMarketPlanogram Small = MarketLayout::Fixtures(TEXT("kucuk"));
    MarketLayout::Plan(Small, Products, Demand);
    TestTrue(TEXT("Small format"), Small.Fixtures.Num() == 3 && Small.Placements.Num() > 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBranchesTest, "MirasMarket.Branches.OpenAndRun", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketBranchesTest::RunTest(const FString& Parameters)
{
    using namespace MarketBranches;
    using namespace MarketBranchesTest;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S; S.Initialize(Products); S.RivalSeed = 12; S.Cash = 5000000;
    S.InheritedDebt = 0; S.ProfitableDays = 5; S.MarketShare = 40.f;
    FString Message;

    TestFalse(TEXT("Not in the family shop's district"), CanOpen(S, Products, EDistrict::Istasyon, TEXT("mahalle"), Message));
    FMarketState Poor = S; Poor.InheritedDebt = 100;
    TestFalse(TEXT("The debt first"), CanOpen(Poor, Products, EDistrict::Carsi, TEXT("mahalle"), Message));
    const int64 Cost = OpeningCost(S, Products, EDistrict::Carsi, TEXT("mahalle"));
    TestTrue(TEXT("Opening costs deposit + fit-out + stock"), Cost > 2 * 90000 + 400000);
    TestTrue(TEXT("Open in \u00c7ar\u015f\u0131"), Open(S, Products, EDistrict::Carsi, TEXT("mahalle"), Message));
    TestTrue(TEXT("A branch exists"), S.Branches.Num() == 1 && S.bSecondStore && S.Branches[0].Stage == static_cast<uint8>(EStage::Renovation));
    TestFalse(TEXT("One shop per district"), CanOpen(S, Products, EDistrict::Carsi, TEXT("mahalle"), Message));
    TestTrue(TEXT("Shelves planned"), S.Branches[0].Items.ContainsByPredicate([](const FMarketBranchItem& I) { return I.Capacity > 0; }));
    TestTrue(TEXT("Neighbour branch takes a little from the family shop"), MainShopFactor(S) >= 1.f); // not open yet

    // Renovation (5) -> permits (3 + 3 without an accountant) -> hiring -> open.
    for (int32 D = 0; D < 20 && S.Branches[0].Stage != static_cast<uint8>(EStage::Open); ++D) Close(S, Products);
    TestEqual(TEXT("Open"), S.Branches[0].Stage, static_cast<uint8>(EStage::Open));
    TestFalse(TEXT("A manager was hired"), S.Branches[0].ManagerName.IsEmpty());
    TestTrue(TEXT("Stocked"), S.Branches[0].Items.ContainsByPredicate([](const FMarketBranchItem& I) { return I.Units > 0; }));
    TestTrue(TEXT("Now it takes a little from the family shop"), MainShopFactor(S) < 1.f);
    FMarketState Second = S;
    TestFalse(TEXT("Third shop needs HR"), [&] { Open(Second, Products, EDistrict::Kocasinan, TEXT("kucuk"), Message); return CanOpen(Second, Products, EDistrict::Sanayi, TEXT("kucuk"), Message); }());

    // A month of business.
    int64 MonthProfit = 0;
    for (int32 D = 0; D < 35; ++D) { Close(S, Products); MonthProfit += S.Branches[0].LastProfit; }
    const FMarketBranch& Branch = S.Branches[0];
    TestTrue(TEXT("Shoppers come"), Branch.LastShoppers > 20);
    TestTrue(TEXT("Revenue"), Branch.LastRevenue > 0);
    TestTrue(TEXT("Habit built in a month"), Branch.Maturity >= 1.f);
    TestTrue(TEXT("The manager keeps it stocked"), Branch.Items.ContainsByPredicate([](const FMarketBranchItem& I) { return I.Capacity > 0 && I.Units > 0; }));
    TestTrue(TEXT("Branch result reaches the day's net"), S.LastBranchProfit == Branch.LastProfit);
    TestFalse(TEXT("Summary"), Summary(S, 0, Products).IsEmpty());

    // Promote a worker, then close the branch.
    FMarketEmployee Worker; Worker.Id = 77; Worker.Name = TEXT("Selin Kaya"); Worker.Role = static_cast<uint8>(MarketStaff::ERole::Cashier); Worker.Skill = 70; Worker.DailyWage = 2200; Worker.HiredDay = 1;
    S.Staff.Add(Worker);
    TestTrue(TEXT("Promote"), Promote(S, 77, 0, Message));
    TestTrue(TEXT("Now the manager"), S.Branches[0].ManagerName == TEXT("Selin Kaya") && !MarketStaff::FindEmployee(S, 77));
    const int64 CashBefore = S.Cash;
    TestTrue(TEXT("Close"), Close(S, 0, Message));
    TestTrue(TEXT("Deposit back"), S.Cash > CashBefore && !S.bSecondStore);

    // Old saves with the aggregate second shop become a mature branch.
    FMarketState Old; Old.Initialize(Products); Old.bSecondStore = true; Old.Cash = 1000000;
    Close(Old, Products);
    TestTrue(TEXT("Migrated"), Old.Branches.Num() == 1 && Old.Branches[0].Stage == static_cast<uint8>(EStage::Open) && Old.Branches[0].Maturity >= 1.f);
    return true;
}

#endif

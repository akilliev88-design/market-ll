#include "MarketBranches.h"
#include "MarketLayout.h"
#include "MarketManagers.h"
#include "MarketStaff.h"
#include "MarketGoods.h"
#include "MarketFinance.h"
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
    // G-086: branches in provinces (no districts), four market types.
    using namespace MarketBranches;
    using namespace MarketBranchesTest;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S; S.Initialize(Products); S.RivalSeed = 12; S.Cash = 5000000;
    S.InheritedDebt = 0; S.ProfitableDays = 5; S.MarketShare = 40.f;
    S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
    FString Message;

    TestEqual(TEXT("Four market types"), FormatIds().Num(), 4);
    TestEqual(TEXT("kucuk is the discounter"), FString(FormatInfo(TEXT("kucuk")).Short), FString(TEXT("Ucuzcu")));
    const FSite Home = SiteOf(S, TEXT("tr"), TEXT("kirklareli"));
    TestTrue(TEXT("Home province"), Home.bValid && Home.bHome && !Home.bAbroad);
    TestTrue(TEXT("Room from the population"), Room(Home) == FMath::Max(2, Home.PopulationK / PeoplePerStoreK));
    FMarketState Poor = S; Poor.InheritedDebt = 100;
    TestFalse(TEXT("The debt first"), CanOpen(Poor, Products, TEXT("tr"), TEXT("kirklareli"), TEXT("mahalle"), Message));
    TestFalse(TEXT("Beyond the home province: HR and an accountant"), CanOpen(S, Products, TEXT("tr"), TEXT("tekirdag"), TEXT("mahalle"), Message));
    TestFalse(TEXT("Hypermarket waits for its chapter"), CanOpen(S, Products, TEXT("tr"), TEXT("kirklareli"), TEXT("hiper"), Message));
    TestFalse(TEXT("Unknown province"), CanOpen(S, Products, TEXT("tr"), TEXT("atlantis"), TEXT("mahalle"), Message));
    const int64 Cost = OpeningCost(S, Products, TEXT("tr"), TEXT("kirklareli"), TEXT("mahalle"));
    TestTrue(TEXT("Opening cost is positive"), Cost > 0);
    TestTrue(TEXT("Open in the home province"), Open(S, Products, TEXT("tr"), TEXT("kirklareli"), TEXT("mahalle"), Message));
    TestTrue(TEXT("A branch exists"), S.Branches.Num() == 1 && S.bSecondStore && S.Branches[0].Stage == static_cast<uint8>(EStage::Renovation));
    int64 OpeningStock = 0;
    for (const FMarketBranchItem& Item : S.Branches[0].Items)
        for (const FMarketProduct& Product : Products) if (Product.Id == Item.ProductId) OpeningStock += static_cast<int64>(Item.Capacity) * Product.Cost;
    TestEqual(TEXT("View-specific opening cost is deposit + fit-out + stock"), Cost, 2 * S.Branches[0].Rent + S.OtherCosts + OpeningStock);
    TestEqual(TEXT("In its province"), S.Branches[0].Province, FString(TEXT("kirklareli")));
    TestTrue(TEXT("Named after the province and type"), S.Branches[0].Name.Contains(TEXT("Mahalle 1")));
    TestEqual(TEXT("Family shop + branch"), ShopsIn(S, TEXT("tr"), TEXT("kirklareli")), 2);
    TestTrue(TEXT("Shelves planned"), S.Branches[0].Items.ContainsByPredicate([](const FMarketBranchItem& I) { return I.Capacity > 0; }));
    TestTrue(TEXT("Not open yet: no effect on the family shop"), MainShopFactor(S) >= 1.f);
    {
        FString Country, Province, Format;
        const int32 Arg = EncodeSite(TEXT("tr"), TEXT("van"), TEXT("buyuk"));
        TestTrue(TEXT("Site round trip"), DecodeSite(Arg, Country, Province, Format) && Country == TEXT("tr") && Province == TEXT("van") && Format == TEXT("buyuk"));
    }

    // Renovation (5) -> permits (3 + 3 without an accountant) -> hiring -> open.
    for (int32 D = 0; D < 20 && S.Branches[0].Stage != static_cast<uint8>(EStage::Open); ++D) Close(S, Products);
    TestEqual(TEXT("Open"), S.Branches[0].Stage, static_cast<uint8>(EStage::Open));
    TestFalse(TEXT("A manager was hired"), S.Branches[0].ManagerName.IsEmpty());
    TestTrue(TEXT("Stocked"), S.Branches[0].Items.ContainsByPredicate([](const FMarketBranchItem& I) { return I.Units > 0; }));
    TestTrue(TEXT("Now it takes a little from the family shop"), MainShopFactor(S) < 1.f);
    FMarketState Second = S;
    TestFalse(TEXT("Third shop needs HR"), [&] { Open(Second, Products, TEXT("tr"), TEXT("kirklareli"), TEXT("kucuk"), Message); return CanOpen(Second, Products, TEXT("tr"), TEXT("kirklareli"), TEXT("kucuk"), Message); }());

    // A month of business.
    for (int32 D = 0; D < 35; ++D) Close(S, Products);
    const FMarketBranch& Branch = S.Branches[0];
    TestTrue(TEXT("Shoppers come"), Branch.LastShoppers > 20);
    TestTrue(TEXT("Revenue"), Branch.LastRevenue > 0);
    TestTrue(TEXT("Habit built in a month"), Branch.Maturity >= 1.f);
    TestTrue(TEXT("The manager keeps it stocked"), Branch.Items.ContainsByPredicate([](const FMarketBranchItem& I) { return I.Capacity > 0 && I.Units > 0; }));
    TestTrue(TEXT("Branch result reaches the day's net"), S.LastBranchProfit == Branch.LastProfit);
    TestFalse(TEXT("Summary"), Summary(S, 0, Products).IsEmpty());
    TestTrue(TEXT("A weekly mark"), Grade(S, 0) != TEXT("-"));

    // G-086b: the manager's style decides the waste. The same day with a careful and a generous manager: the
    // generous one throws more away, and what is thrown away (already paid) comes off the branch's result exactly.
    {
        FMarketState CarefulDay = S;
        FMarketState GenerousDay = S;
        CarefulDay.Branches[0].ManagerStyle = static_cast<uint8>(MarketManagers::EStyle::Careful);
        GenerousDay.Branches[0].ManagerStyle = static_cast<uint8>(MarketManagers::EStyle::Generous);
        MarketBranches::CloseDay(CarefulDay, Products);
        MarketBranches::CloseDay(GenerousDay, Products);
        const TArray<FMarketBranchItem>& KeptItems = CarefulDay.Branches[0].Items;
        const TArray<FMarketBranchItem>& WastedItems = GenerousDay.Branches[0].Items;
        int32 MoreThrown = 0;
        int64 ThrownCost = 0;
        for (int32 K = 0; K < KeptItems.Num() && K < WastedItems.Num(); ++K)
        {
            const FString& ItemId = KeptItems[K].ProductId;
            const FMarketProduct* Goods = Products.FindByPredicate([&ItemId](const FMarketProduct& X) { return X.Id == ItemId; });
            const int32 Diff = KeptItems[K].Units - WastedItems[K].Units;
            MoreThrown += Diff;
            if (Goods) ThrownCost += Goods->Cost * Diff;
        }
        TestTrue(TEXT("A generous manager throws more away"), MoreThrown >= 0);
        TestEqual(TEXT("Waste costs the goods thrown away"), CarefulDay.Branches[0].LastProfit - GenerousDay.Branches[0].LastProfit, ThrownCost);
    }

    // Promote a worker, then close the branch.
    FMarketEmployee Worker; Worker.Id = 77; Worker.Name = TEXT("Selin Kaya"); Worker.Role = static_cast<uint8>(MarketStaff::ERole::Cashier); Worker.Skill = 70; Worker.DailyWage = 2200; Worker.HiredDay = 1;
    S.Staff.Add(Worker);
    TestTrue(TEXT("Promote"), Promote(S, 77, 0, Message));
    TestTrue(TEXT("Now the manager"), S.Branches[0].ManagerName == TEXT("Selin Kaya") && !MarketStaff::FindEmployee(S, 77));
    const int64 CashBefore = S.Cash;
    TestTrue(TEXT("Close"), MarketBranches::Close(S, Products, 0, Message));
    TestTrue(TEXT("Deposit back"), S.Cash > CashBefore && !S.bSecondStore);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketRescueTest, "MirasMarket.Finance.RescuePlan", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketRescueTest::RunTest(const FString& Parameters)
{
    // M31: sixty days in the red and the bank's plan closes the losing branch and turns the hole into a long loan.
    using namespace MarketBranchesTest;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S; S.Initialize(Products); S.RivalSeed = 12; S.Cash = 5000000;
    S.InheritedDebt = 0; S.ProfitableDays = 5; S.MarketShare = 40.f;
    S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
    FString Message;
    TestTrue(TEXT("Open a branch"), MarketBranches::Open(S, Products, TEXT("tr"), TEXT("kirklareli"), TEXT("mahalle"), Message));
    for (int32 D = 0; D < 20 && S.Branches[0].Stage != static_cast<uint8>(MarketBranches::EStage::Open); ++D) Close(S, Products);
    TestEqual(TEXT("Open"), S.Branches[0].Stage, static_cast<uint8>(MarketBranches::EStage::Open));

    S.Branches[0].Last30Profit = -100000;
    S.Cash = -800000;
    S.NegativeCashDays = MarketFinance::RescueDays - 1;
    S.TroubleStage = 5;
    const int32 LoansBefore = S.Loans.Num();
    S.DayNews.Reset();
    MarketFinance::CloseDay(S, Products);
    TestEqual(TEXT("One rescue"), S.Rescues, 1);
    TestEqual(TEXT("The losing branch closed"), S.Branches[0].Stage, static_cast<uint8>(MarketBranches::EStage::Closed));
    TestTrue(TEXT("A rescue loan"), S.Loans.Num() == LoansBefore + 1 && S.Loans.Last().Principal > 600000);
    TestTrue(TEXT("Working capital"), S.Cash >= MarketFinance::RescueWorkingCapital);
    TestTrue(TEXT("Money to fill the shelves"), S.Cash > 0);
    TestTrue(TEXT("The ladder starts over"), S.NegativeCashDays == 0 && S.TroubleStage == 0);
    TestTrue(TEXT("Told"), S.DayNews.ContainsByPredicate([](const FString& Line) { return Line.Contains(TEXT("kurtarma")); }));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketRescueOnePlanTest, "MirasMarket.Finance.RescueOnePlan", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketRescueOnePlanTest::RunTest(const FString& Parameters)
{
    // C7 (Codex C4/C5: 321 plans, 500 million of debt): a missed installment costs its fee once a month, and the
    // plan folds every loan into one the shop can carry, writes off the rest and closes the doors for two years.
    using namespace MarketBranchesTest;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S; S.Initialize(Products); S.RivalSeed = 12; S.Day = 400;
    S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
    FMarketLoan Loan; Loan.Principal = Loan.Remaining = 1000000; Loan.MonthlyRate = 0.02f; Loan.Installment = 100000; Loan.NextDueDay = S.Day - 1;
    S.Loans.Add(Loan);
    S.Cash = -50000;
    for (int32 D = 0; D < 30; ++D)
    {
        S.NegativeCashDays = 0; S.TroubleStage = 0; S.Decisions.Reset();
        MarketFinance::CloseDay(S, Products);
        ++S.Day;
    }
    TestTrue(TEXT("Late fee once or twice a month, not every day"), S.Loans.Num() == 1 && S.Loans[0].Remaining <= 1000000 + 2 * 3000);

    // A pile of debt and a shop that sold little: one plan, most of it written off.
    for (int32 I = 0; I < 3; ++I) { FMarketLoan Old = Loan; Old.Remaining = 50000000; Old.NextDueDay = S.Day + 5; S.Loans.Add(Old); }
    FMarketCorpLoan Corp; Corp.Balance = 20000000; Corp.NextDueDay = S.Day + 5; S.Banking.Loans.Add(Corp);
    S.Banking.bLine = true; S.Banking.LineDrawn = 3000000;
    S.Cash = -1000000;
    const TArray<FString> Lines = MarketFinance::Rescue(S, Products);
    TestEqual(TEXT("One loan left"), S.Loans.Num(), 1);
    TestTrue(TEXT("Company loans and the line folded in"), S.Banking.Loans.Num() == 0 && S.Banking.LineDrawn == 0 && !S.Banking.bLine);
    TestTrue(TEXT("Most of it written off"), MarketFinance::Debt(S) < 20000000);
    TestTrue(TEXT("Money for a month and the shelves"), S.Cash > 0);
    TestTrue(TEXT("Half a year before the first installment"), S.Loans[0].NextDueDay >= S.Day + MarketFinance::RescueGraceDays);
    TestEqual(TEXT("No new loan under the plan"), MarketFinance::LoanLimit(S), int64(0));
    FString Why;
    TestFalse(TEXT("No new branch under the plan"), MarketBranches::CanOpen(S, Products, TEXT("tr"), TEXT("kirklareli"), TEXT("mahalle"), Why));
    TestTrue(TEXT("Told what was written off"), Lines.ContainsByPredicate([](const FString& Line) { return Line.Contains(TEXT("silindi")); }));
    return true;
}

#endif

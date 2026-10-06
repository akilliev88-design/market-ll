#include "MarketFirstStore.h"
#include "MarketStoreVisit.h"
#include "MarketBranches.h"
#include "MarketCompany.h"
#include "MarketEconomy.h"
#include "MarketFinance.h"
#include "MarketStaff.h"
#include "MarketStart.h"
#include "MarketStoreDemand.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// G-110 (M70): closing the first store and entering stores from the map.
namespace MarketFirstStoreTest
{
    FMarketProduct Make(const TCHAR* Id, const TCHAR* Category, int64 Cost)
    {
        FMarketProduct P;
        P.Id = Id; P.RealName = Id; P.FictionalName = Id; P.Category = Category; P.Cost = Cost; P.BasePrice = Cost * 3 / 2; P.CaseUnits = 12; P.bActive = true;
        return P;
    }

    // Detergent keeps, milk spoils.
    TArray<FMarketProduct> Catalog() { return { Make(TEXT("deterjan"), TEXT("temizlik"), 1400), Make(TEXT("sut"), TEXT("s\u00fct"), 170) }; }

    FMarketEmployee Person(int32 Id, MarketStaff::ERole Role)
    {
        FMarketEmployee E;
        E.Id = Id; E.Name = FString::Printf(TEXT("Ki\u015fi %d"), Id); E.Role = static_cast<uint8>(Role); E.DailyWage = 2000; E.HiredDay = 1;
        return E;
    }

    FMarketState Shop(const TArray<FMarketProduct>& Products)
    {
        FMarketState S;
        S.Initialize(Products); // 32 units of each in the depot
        S.CountryId = TEXT("tr");
        S.CityId = MarketStart::FallbackProvince(TEXT("tr"));
        S.Day = 200;
        S.Cash = 5000000;
        S.Staff.Add(Person(1, MarketStaff::ERole::Cashier));
        S.Staff.Add(Person(2, MarketStaff::ERole::Stocker));
        S.Staff.Add(Person(3, MarketStaff::ERole::HrManager));
        return S;
    }

    int32 AddBranch(FMarketState& S, const TArray<FMarketProduct>& Products, const FString& Province, const FString& Format)
    {
        FMarketBranch& B = S.Branches.AddDefaulted_GetRef();
        B.Country = S.CountryId; B.Province = Province; B.Format = Format;
        B.Name = FString::Printf(TEXT("Test %d"), S.Branches.Num());
        B.Stage = static_cast<uint8>(MarketBranches::EStage::Open);
        B.OpenedDay = 50; B.Satisfaction = 70.f; B.Maturity = 1.f; B.Rent = 90000;
        for (const FMarketProduct& P : Products)
        {
            FMarketStock Item; Item.Id = P.Id; Item.Capacity = 40; Item.Shelf = 10; Item.Warehouse = 0;
            B.Items.Add(Item);
        }
        return S.Branches.Num() - 1;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketFirstStoreSellTest, "MarketSim.FirstStore.CloseAndSell", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketFirstStoreSellTest::RunTest(const FString& Parameters)
{
    using namespace MarketFirstStoreTest;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S = Shop(Products);
    const FString Home = MarketStart::HomeProvince(S);
    const int32 Branch = AddBranch(S, Products, Home, TEXT("mahalle"));
    const int64 Building = MarketFinance::BuildingValue(S);
    const int64 CashBefore = S.Cash;
    TestTrue(TEXT("The building is worth something"), Building > 0);
    TestEqual(TEXT("Two stores before"), MarketCompany::TotalStores(S), 2);

    FString Message;
    TestTrue(TEXT("It closes"), MarketFirstStore::Close(S, Products, MarketFirstStore::EBuilding::Sell, Message));
    TestTrue(TEXT("Sold"), MarketFirstStore::StatusOf(S) == MarketFirstStore::EStatus::Sold);
    TestFalse(TEXT("Not open"), MarketFirstStore::IsOpen(S));
    TestTrue(TEXT("The building's money came in (less the severance)"), S.Cash > CashBefore + Building / 2);
    TestEqual(TEXT("A sold building leaves the balance sheet"), MarketFinance::BuildingValue(S), static_cast<int64>(0));
    int32 Units = 0;
    for (const FMarketStock& Row : S.Stock) Units += Row.Shelf + Row.Warehouse + Row.Dock + Row.Incoming;
    TestEqual(TEXT("The shop is empty"), Units, 0);
    TestEqual(TEXT("Only the head office's HR manager stays"), S.Staff.Num(), 1);
    TestEqual(TEXT("The detergent went to the branch"), S.Branches[Branch].Items[0].Incoming, 32);
    TestEqual(TEXT("The milk did not travel"), S.Branches[Branch].Items[1].Incoming, 0);
    TestEqual(TEXT("One store left"), MarketCompany::TotalStores(S), 1);
    TestEqual(TEXT("No shoppers at a closed shop"), MarketStoreDemand::FirstStoreShoppers(S, Products, S.Day), 0);
    TestEqual(TEXT("No running costs for a sold building"), S.FirstStoreRunning(), 0.0);
    TArray<int32> Cases; Cases.Init(1, Products.Num());
    TestFalse(TEXT("A closed shop orders nothing"), S.SubmitOrder(Cases, Products));
    FString Why;
    TestFalse(TEXT("A sold building never reopens"), MarketFirstStore::CanReopen(S, Why));
    TestFalse(TEXT("It cannot close twice"), MarketFirstStore::Close(S, Products, MarketFirstStore::EBuilding::Keep, Message));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketFirstStoreLeaseTest, "MarketSim.FirstStore.LeaseAndReopen", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketFirstStoreLeaseTest::RunTest(const FString& Parameters)
{
    using namespace MarketFirstStoreTest;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S = Shop(Products);
    const int64 Building = MarketFinance::BuildingValue(S);
    FString Message;
    TestTrue(TEXT("It closes"), MarketFirstStore::Close(S, Products, MarketFirstStore::EBuilding::Lease, Message));
    TestEqual(TEXT("A leased building stays ours"), MarketFinance::BuildingValue(S), Building);
    const int64 Rent = MarketFirstStore::MonthlyLease(S);
    TestTrue(TEXT("It brings a rent"), Rent > 0);
    const int64 Before = S.Cash;
    MarketFirstStore::CloseDay(S);
    TestEqual(TEXT("A day's rent"), S.Cash - Before, Rent / 30);
    S.Candidates.Reset();
    S.Candidates.Add(Person(7, MarketStaff::ERole::Cashier));
    TestFalse(TEXT("No cashier for a closed shop"), MarketStaff::Hire(S, 0, Message));
    TestEqual(TEXT("Nobody was hired"), S.Staff.Num(), 1);

    const int64 BeforeReopen = S.Cash;
    TestTrue(TEXT("It reopens"), MarketFirstStore::Reopen(S, Message));
    TestTrue(TEXT("Open again"), MarketFirstStore::IsOpen(S));
    TestEqual(TEXT("The tenant got a month's rent"), BeforeReopen - S.Cash, Rent * MarketFirstStore::LeaseNoticeMonths);
    TestEqual(TEXT("Full running costs again"), S.FirstStoreRunning(), 1.0);

    // Keep it empty: a quarter of the upkeep, it reopens for free.
    TestTrue(TEXT("Closes again"), MarketFirstStore::Close(S, Products, MarketFirstStore::EBuilding::Keep, Message));
    TestEqual(TEXT("An empty building's upkeep"), S.FirstStoreRunning(), 0.25);
    const int64 Free = S.Cash;
    TestTrue(TEXT("Reopens"), MarketFirstStore::Reopen(S, Message));
    TestEqual(TEXT("An empty building reopens for free"), S.Cash, Free);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketFirstStoreNoStoreTest, "MarketSim.FirstStore.CompanyGoesOn", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketFirstStoreNoStoreTest::RunTest(const FString& Parameters)
{
    using namespace MarketFirstStoreTest;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S = Shop(Products);
    S.ProfitableDays = 0; // the first-branch rule would ask for profitable days
    FString Message;
    TestTrue(TEXT("The only store closes"), MarketFirstStore::Close(S, Products, MarketFirstStore::EBuilding::Keep, Message));
    TestEqual(TEXT("No store"), MarketCompany::TotalStores(S), 0);
    TestEqual(TEXT("The home province has no shop of ours"), MarketBranches::ShopsIn(S, S.CountryId, MarketStart::HomeProvince(S)), 0);
    TestEqual(TEXT("Nothing to enter"), MarketStoreVisit::OnlyStore(S), MarketStoreVisit::None);
    FString Why;
    const bool bCan = MarketBranches::CanOpen(S, Products, S.CountryId, MarketStart::HomeProvince(S), TEXT("mahalle"), Why);
    TestFalse(TEXT("The first-branch rule does not apply any more"), Why.Contains(TEXT("k\u00e2rl\u0131 g\u00fcn")));
    (void)bCan;
    // A closed branch's goods find another store, not the closed first store.
    const int32 A = AddBranch(S, Products, MarketStart::HomeProvince(S), TEXT("mahalle"));
    const int32 B = AddBranch(S, Products, MarketStart::HomeProvince(S), TEXT("mahalle"));
    TestTrue(TEXT("A branch closes"), MarketBranches::Close(S, Products, A, Message));
    TestEqual(TEXT("The detergent went to the other branch"), S.Branches[B].Items[0].Incoming, 10);
    int32 FirstStoreUnits = 0;
    for (const FMarketStock& Row : S.Stock) FirstStoreUnits += Row.Warehouse;
    TestEqual(TEXT("Nothing went to the closed first store"), FirstStoreUnits, 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStoreVisitTest, "MarketSim.StoreVisit.PickAndNext", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStoreVisitTest::RunTest(const FString& Parameters)
{
    using namespace MarketFirstStoreTest;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S = Shop(Products);
    const FString Home = MarketStart::HomeProvince(S);
    TestTrue(TEXT("One store: the game opens inside it"), MarketStoreVisit::StartsInStore(S));
    TestEqual(TEXT("The only store is the first store"), MarketStoreVisit::OnlyStore(S), MarketStoreVisit::FirstStore);

    const int32 A = AddBranch(S, Products, Home, TEXT("mahalle"));
    const int32 B = AddBranch(S, Products, Home, TEXT("mahalle"));
    const int32 Big = AddBranch(S, Products, Home, TEXT("buyuk"));
    TestFalse(TEXT("More stores: the game opens on the map"), MarketStoreVisit::StartsInStore(S));

    const TArray<MarketStoreVisit::FTypeCount> Types = MarketStoreVisit::TypesIn(S, S.CountryId, Home);
    const MarketStoreVisit::FTypeCount* Hood = Types.FindByPredicate([](const MarketStoreVisit::FTypeCount& T) { return T.Format == TEXT("mahalle"); });
    TestTrue(TEXT("Neighbourhood stores counted"), Hood && Hood->Count == 3);
    const MarketStoreVisit::FTypeCount* Super = Types.FindByPredicate([](const MarketStoreVisit::FTypeCount& T) { return T.Format == TEXT("buyuk"); });
    TestTrue(TEXT("The supermarket counted"), Super && Super->Count == 1);

    // A store with empty shelves and a queue is entered far more often.
    for (FMarketStock& Item : S.Branches[B].Items) { Item.Yesterday.Sold = 2; Item.Yesterday.Empty = 18; }
    S.Branches[B].LastQueueLost = 6;
    int32 HitsB = 0, Total = 0;
    for (int32 Seed = 0; Seed < 400; ++Seed)
    {
        const int32 Store = MarketStoreVisit::Pick(S, S.CountryId, Home, TEXT("mahalle"), Seed);
        TestTrue(TEXT("A neighbourhood store of ours"), Store == MarketStoreVisit::FirstStore || Store == A || Store == B);
        HitsB += Store == B ? 1 : 0;
        ++Total;
    }
    TestTrue(TEXT("The troubled store comes up most"), HitsB * 2 > Total);
    TestEqual(TEXT("The same seed, the same store"), MarketStoreVisit::Pick(S, S.CountryId, Home, TEXT("mahalle"), 7), MarketStoreVisit::Pick(S, S.CountryId, Home, TEXT("mahalle"), 7));
    TestEqual(TEXT("The only supermarket"), MarketStoreVisit::Pick(S, S.CountryId, Home, TEXT("buyuk"), 3), Big);
    TestEqual(TEXT("No hypermarket there"), MarketStoreVisit::Pick(S, S.CountryId, Home, TEXT("hiper"), 3), MarketStoreVisit::None);

    // Tab walks the same type in order and comes back.
    TestEqual(TEXT("First store -> A"), MarketStoreVisit::Next(S, MarketStoreVisit::FirstStore), A);
    TestEqual(TEXT("A -> B"), MarketStoreVisit::Next(S, A), B);
    TestEqual(TEXT("B -> first store"), MarketStoreVisit::Next(S, B), MarketStoreVisit::FirstStore);
    TestEqual(TEXT("The only supermarket stays"), MarketStoreVisit::Next(S, Big), Big);
    TestTrue(TEXT("Header counts the type"), MarketStoreVisit::Header(S, A).EndsWith(TEXT("2/3")));

    // The boss's visit: the people's morale rises once a day.
    S.Branches[A].Staff.Add(Person(9, MarketStaff::ERole::Cashier));
    const float Morale = S.Branches[A].Staff[0].Morale;
    FString Message;
    TestTrue(TEXT("Visit"), MarketBranches::Visit(S, Products, A, Message));
    TestTrue(TEXT("Visit again the same day"), MarketBranches::Visit(S, Products, A, Message));
    TestEqual(TEXT("Morale rose once"), S.Branches[A].Staff[0].Morale, FMath::Min(100.f, Morale + MarketBranches::BossMorale));

    // With the first store closed, the type list no longer has it.
    TestTrue(TEXT("Close"), MarketFirstStore::Close(S, Products, MarketFirstStore::EBuilding::Keep, Message));
    TestEqual(TEXT("First store gone from the walk"), MarketStoreVisit::StoresOf(S, S.CountryId, Home, TEXT("mahalle")).Num(), 2);
    return true;
}

#endif

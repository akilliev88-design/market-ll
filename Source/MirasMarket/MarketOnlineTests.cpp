#include "MarketOnline.h"
#include "MarketEvents.h"
#include "MarketBranches.h"
#include "MarketManagers.h"
#include "MarketCalendar.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketOnlineTest
{
    FMarketProduct Make(const TCHAR* Id, const TCHAR* Category, int64 Cost, int64 Price)
    {
        FMarketProduct P;
        P.Id = Id; P.RealName = Id; P.Category = Category; P.Cost = Cost; P.BasePrice = Price; P.CaseUnits = 12;
        return P;
    }

    TArray<FMarketProduct> Catalog()
    {
        return {
            Make(TEXT("sut"), TEXT("s\u00fct"), 170, 250),
            Make(TEXT("makarna"), TEXT("makarna-bakliyat"), 210, 325),
            Make(TEXT("cay"), TEXT("\u00e7ay-kahve"), 480, 675),
            Make(TEXT("deterjan"), TEXT("temizlik"), 1400, 1990),
        };
    }

    FMarketState Start(const TArray<FMarketProduct>& Products, int32 Day)
    {
        FMarketState S; S.Initialize(Products);
        S.RivalSeed = 7; S.Cash = 500000000;
        S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
        S.ApplyShelfCapacities({ 30, 30, 30, 30 });
        for (FMarketStock& Row : S.Stock) { Row.Warehouse = 100; Row.Shelf = 30; }
        S.Day = Day;
        return S;
    }

    void AddBranch(FMarketState& S, const TArray<FMarketProduct>& Products, const TCHAR* Province)
    {
        FMarketBranch B;
        B.Country = TEXT("tr"); B.Province = Province; B.Format = TEXT("mahalle"); B.Name = Province;
        B.Stage = static_cast<uint8>(MarketBranches::EStage::Open);
        B.Workers = 4; B.LastShoppers = 300; B.PriceIndex = 1.f;
        for (const FMarketProduct& P : Products) { FMarketBranchItem Item; Item.ProductId = P.Id; Item.Capacity = 40; Item.Units = 40; B.Items.Add(Item); }
        S.Branches.Add(B);
    }

    int32 Units(const FMarketState& S)
    {
        int32 Sum = 0;
        for (const FMarketStock& Row : S.Stock) Sum += Row.Warehouse + Row.Shelf;
        for (const FMarketBranch& B : S.Branches) for (const FMarketBranchItem& Item : B.Items) Sum += Item.Units;
        return Sum;
    }

    // One closed day; true when the online money is exactly what reached the till.
    bool Close(FMarketState& S, const TArray<FMarketProduct>& Products, int32 Served = 300)
    {
        S.Served = Served;
        S.DayNews.Reset();
        S.CloseDay();
        const int64 CashBefore = S.Cash;
        MarketOnline::CloseDay(S, Products);
        return S.Cash - CashBefore == S.Online.LastRevenue - S.Online.LastCosts;
    }

    int32 AreaOf(const FMarketState& S, const TCHAR* Province)
    {
        return S.Online.Areas.IndexOfByPredicate([Province](const FMarketOnlineArea& A) { return A.Province == Province; });
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketOnlineTimelineTest, "MirasMarket.Online.TimelineAndShare", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketOnlineTimelineTest::RunTest(const FString& Parameters)
{
    // M32: online selling follows the campaign's epidemic, not a calendar year, and is hidden at the start.
    using namespace MarketOnlineTest;
    using MarketOnline::EChannel;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S = Start(Products, 2);
    const int32 Web = MarketOnline::OpenDay(S, EChannel::Web), Platform = MarketOnline::OpenDay(S, EChannel::Platform);
    const int32 App = MarketOnline::OpenDay(S, EChannel::App), Quick = MarketOnline::OpenDay(S, EChannel::Quick);
    const int32 Sick = MarketOnline::PandemicStart(S), Well = MarketOnline::PandemicEnd(S);
    TestTrue(TEXT("Order: web, platform, app, epidemic, quick"), Web < Platform && Platform < App && App < Sick && Sick < Quick);
    TestTrue(TEXT("The app a year and a half before the epidemic"), Sick - App > 365 && Sick - App < 730);
    TestFalse(TEXT("Nothing shows at the start"), MarketOnline::Visible(S));
    FString Reason;
    TestFalse(TEXT("No web shop at the start"), MarketOnline::CanOpen(S, EChannel::Web, Reason));
    TestEqual(TEXT("Nobody shops online at the start"), MarketOnline::OnlineShare(S, 2), 0.f);
    S.Day = Web - MarketOnline::RumourDays + 1;
    TestTrue(TEXT("The rumour opens the page"), MarketOnline::Visible(S));

    const float Before = MarketOnline::OnlineShare(S, Sick - 1);
    TestTrue(TEXT("Grows before the epidemic"), MarketOnline::OnlineShare(S, Web + 30) < Before);
    TestTrue(TEXT("Jumps in the epidemic"), MarketOnline::OnlineShare(S, Sick + 5) > 2.f * Before);
    TestTrue(TEXT("Keeps moving online after it"), MarketOnline::OnlineShare(S, Well + 700) > MarketOnline::OnlineShare(S, Well + 5));
    const float Settled = MarketOnline::OnlineShare(S, Well + 4 * 365 + 100);
    TestTrue(TEXT("Settles near the country's plateau"), Settled > 0.07f && Settled < 0.13f);
    TestTrue(TEXT("The shops' own trade grows back after the epidemic"), MarketOnline::StoreTrafficFactorOn(S, Well + 365) > 1.f - MarketOnline::OnlineShare(S, Well + 365));
    TestTrue(TEXT("Closure days exist"), [&S, Sick, Well] { for (int32 D = Sick; D < Well; ++D) if (MarketOnline::IsCurfew(S, D)) return true; return false; }());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketOnlineFamilyTest, "MirasMarket.Online.FamilyShopOnThePlatform", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketOnlineFamilyTest::RunTest(const FString& Parameters)
{
    // The family shop has no couriers: alone it can only join the platform.
    using namespace MarketOnlineTest;
    using MarketOnline::EChannel;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S = Start(Products, 2);
    S.Day = MarketOnline::PandemicStart(S) + 20;
    S.Online.Reputation = 90.f;
    FString Message;
    TestFalse(TEXT("No own web shop with one shop"), MarketOnline::CanOpen(S, EChannel::Web, Message));
    TestTrue(TEXT("The platform"), MarketOnline::Open(S, EChannel::Platform, Message));
    const int32 UnitsBefore = Units(S);
    int32 Orders = 0;
    bool bMoney = true;
    for (int32 D = 0; D < 10; ++D) { bMoney &= Close(S, Products); Orders += S.Online.LastOrders; }
    TestTrue(TEXT("Money kept"), bMoney);
    TestTrue(TEXT("Orders came"), Orders > 0);
    TestTrue(TEXT("Picked from the stock"), Units(S) < UnitsBefore);
    TestEqual(TEXT("One area: the home province"), S.Online.Areas.Num(), 1);
    TestTrue(TEXT("The platform's month counts"), S.Online.MonthOrders.IsValidIndex(static_cast<int32>(EChannel::Platform)));
    TestFalse(TEXT("The assistant heard of it"), S.Online.Hint.IsEmpty());
    TestFalse(TEXT("Summary"), MarketOnline::Summary(S).IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketOnlineCompanyTest, "MirasMarket.Online.CompanyAndProvinces", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketOnlineCompanyTest::RunTest(const FString& Parameters)
{
    // Web, the app from a software house, our fast delivery from a dark store, the province decides.
    using namespace MarketOnlineTest;
    using MarketOnline::EChannel;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S = Start(Products, 2);
    for (int32 I = 0; I < 7; ++I) AddBranch(S, Products, I < 4 ? TEXT("kirklareli") : TEXT("tekirdag"));
    S.Day = MarketOnline::OpenDay(S, EChannel::App) + 10;
    FString Message;
    TestTrue(TEXT("Web with branches"), MarketOnline::Open(S, EChannel::Web, Message));
    TestTrue(TEXT("The app asks for a software house"), MarketOnline::Open(S, EChannel::App, Message) && S.Decisions.ContainsByPredicate([](const FMarketDecision& D) { return D.Id == TEXT("online.app"); }));
    const int32 Card = S.Decisions.IndexOfByPredicate([](const FMarketDecision& D) { return D.Id == TEXT("online.app"); });
    TestTrue(TEXT("The solid one"), MarketOnline::Resolve(S, Products, S.Decisions[Card], 1, Message) && S.Online.bApp && S.Online.AppTier == 1);
    TestTrue(TEXT("A day in"), Close(S, Products));
    TestEqual(TEXT("Two areas"), S.Online.Areas.Num(), 2);

    S.Day = MarketOnline::OpenDay(S, EChannel::Quick) + 5;
    TestTrue(TEXT("Fast delivery"), MarketOnline::Open(S, EChannel::Quick, Message));
    const int32 Home = AreaOf(S, TEXT("kirklareli")), Other = AreaOf(S, TEXT("tekirdag"));
    TestTrue(TEXT("Both areas"), Home != INDEX_NONE && Other != INDEX_NONE);
    if (Home == INDEX_NONE || Other == INDEX_NONE) return false;
    TestTrue(TEXT("Dark store with five shops"), MarketOnline::BuildDarkStore(S, Home, Message));
    TestFalse(TEXT("No dark store with three"), MarketOnline::CanBuildDarkStore(S, Other, Message));
    TestTrue(TEXT("The player sets the province"), MarketOnline::SetArea(S, Home, 7, Message) && S.Online.Areas[Home].bPlayerSet);
    TestFalse(TEXT("No fast delivery without a dark store"), MarketOnline::SetArea(S, Other, 4, Message));

    const int32 UnitsBefore = Units(S);
    int32 Orders = 0;
    bool bMoney = true;
    for (int32 D = 0; D < 10; ++D) { bMoney &= Close(S, Products); Orders += S.Online.LastOrders; }
    TestTrue(TEXT("Money kept"), bMoney);
    TestTrue(TEXT("Orders from the company"), Orders > 0);
    TestTrue(TEXT("Picked from the shops"), Units(S) < UnitsBefore);
    TestTrue(TEXT("Fast orders counted"), S.Online.MonthOrders[static_cast<int32>(EChannel::Quick)] > 0 || S.Online.PrevOrders[static_cast<int32>(EChannel::Quick)] > 0);
    TestFalse(TEXT("A province line"), MarketOnline::AreaLine(S, Home).IsEmpty());

    // Without a province manager the company's rule decides again.
    S.Online.bDefaultPlatform = false;
    TestTrue(TEXT("Give it back"), MarketOnline::ReturnArea(S, Home, Message) && !S.Online.Areas[Home].bPlayerSet);
    TestTrue(TEXT("The company's rule"), MarketOnline::AreaDecider(S, Home).IsEmpty() && !S.Online.Areas[Home].bPlatform);

    // A province manager does not flip channels: after three losing months he proposes, we approve.
    TestTrue(TEXT("The platform for the company"), MarketOnline::Open(S, EChannel::Platform, Message));
    FMarketManager Boss;
    Boss.Level = static_cast<uint8>(MarketManagers::ELevel::Province); Boss.Country = TEXT("tr"); Boss.Area = TEXT("tekirdag");
    Boss.Name = TEXT("Deneme M\u00fcd\u00fcr"); Boss.Skill = 70; Boss.AppointedDay = 1;
    S.Management.Managers.Add(Boss);
    FMarketOnlineArea& Far = S.Online.Areas[Other];
    Far.bPlatform = true; Far.PlatformLossMonths = 2; Far.MonthPlatformProfit = -100;
    const MarketCalendar::FDate Now = MarketCalendar::DateOf(S.Day);
    S.Day = MarketCalendar::GameDayOf(Now.Month == 12 ? Now.Year + 1 : Now.Year, Now.Month == 12 ? 1 : Now.Month + 1, 1) - 1;
    Close(S, Products);
    const int32 Proposal = S.Decisions.IndexOfByPredicate([](const FMarketDecision& D) { return D.Id == TEXT("online.area:tr|tekirdag"); });
    TestTrue(TEXT("A proposal, not a change"), Proposal != INDEX_NONE && S.Online.Areas[AreaOf(S, TEXT("tekirdag"))].bPlatform);
    if (Proposal != INDEX_NONE)
        TestTrue(TEXT("Approved: the platform leaves the province"), MarketOnline::Resolve(S, Products, S.Decisions[Proposal], 0, Message) && !S.Online.Areas[AreaOf(S, TEXT("tekirdag"))].bPlatform);

    // The e-commerce manager.
    TestTrue(TEXT("Hire"), MarketOnline::HireManager(S, Message) && !S.Online.ManagerName.IsEmpty());
    TestTrue(TEXT("He keeps the policy"), MarketOnline::SetAutoPolicy(S, true, Message));
    TestTrue(TEXT("Closing the web closes the app"), MarketOnline::Close(S, EChannel::Web, Message) && !S.Online.bApp && !S.Online.bQuick);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketOnlineRivalsTest, "MirasMarket.Online.RivalsAndCards", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketOnlineRivalsTest::RunTest(const FString& Parameters)
{
    // Rivals go online and the assistant tells; the platform asks for more commission.
    using namespace MarketOnlineTest;
    using MarketOnline::EChannel;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S = Start(Products, 2);
    FMarketChain Chain;
    Chain.Id = TEXT("bin"); Chain.Name = TEXT("B\u0130N"); Chain.Country = TEXT("tr");
    FMarketChainSpot Spot; Spot.Province = TEXT("istanbul"); Spot.Stores = 800; Chain.Spots.Add(Spot);
    S.Rivals.Chains.Add(Chain);
    TestEqual(TEXT("Nobody online at the start"), MarketOnline::RivalOnline(S, TEXT("tr"), 2), 0.f);
    const int32 Late = MarketOnline::OpenDay(S, EChannel::Quick) + 800;
    TestTrue(TEXT("Rivals online later"), MarketOnline::RivalOnline(S, TEXT("tr"), Late) > 0.5f);

    S.Day = MarketOnline::PandemicStart(S) + MarketOnline::CommissionAfter + 1;
    FString Message;
    TestTrue(TEXT("On the platform"), MarketOnline::Open(S, EChannel::Platform, Message));
    Close(S, Products);
    TestTrue(TEXT("The rival's news"), S.Online.RivalsTold.Num() > 0);
    TestTrue(TEXT("RivalLines"), MarketOnline::RivalLines(S).Num() > 0);
    const int32 Card = S.Decisions.IndexOfByPredicate([](const FMarketDecision& D) { return D.Id == TEXT("online.commission"); });
    TestTrue(TEXT("Commission card"), Card != INDEX_NONE);
    if (Card == INDEX_NONE) return false;
    const float Before = S.Online.Commission;
    TestTrue(TEXT("Accepted"), MarketOnline::Resolve(S, Products, S.Decisions[Card], 0, Message) && S.Online.Commission > Before);
    TestTrue(TEXT("The epidemic card came too"), S.Decisions.ContainsByPredicate([](const FMarketDecision& D) { return D.Id == TEXT("online.pandemic"); }));
    return true;
}

#endif

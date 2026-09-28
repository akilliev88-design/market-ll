#include "MarketOnline.h"
#include "MarketPayments.h"
#include "MarketDirector.h"
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

    int32 Units(const FMarketState& S)
    {
        int32 Sum = 0;
        for (const FMarketStock& Row : S.Stock) Sum += Row.Warehouse + Row.Shelf;
        return Sum;
    }

    int32 SoldYesterday(const FMarketState& S)
    {
        int32 Sum = 0;
        for (const FMarketStock& Row : S.Stock) Sum += Row.Yesterday.Sold;
        return Sum;
    }

    void AddRegulars(FMarketState& S, int32 Count)
    {
        for (int32 I = 0; I < Count; ++I)
        {
            FMarketLoyalty L; L.CustomerId = I; L.Visits = 5; L.Satisfaction = 80.f;
            S.Loyalty.Add(L);
        }
    }

    // One closed day: the shop's close, then the online orders. True when picked units left the stock and counted
    // as sold, and the order money reached the till.
    bool Close(FMarketState& S, const TArray<FMarketProduct>& Products, int32 Served = 0)
    {
        S.Served = Served;
        S.DayNews.Reset();
        S.CloseDay();
        const int32 Before = Units(S);
        const int32 SoldBefore = SoldYesterday(S);
        const int64 CashBefore = S.Cash;
        MarketOnline::CloseDay(S, Products);
        const bool bStock = Before - Units(S) == SoldYesterday(S) - SoldBefore;
        const bool bMoney = S.Cash - CashBefore == S.Online.LastRevenue - S.Online.LastCosts;
        return bStock && bMoney;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketOnlineTest, "MirasMarket.Online.OrdersAndEras", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketOnlineTest::RunTest(const FString& Parameters)
{
    using namespace MarketOnlineTest;
    using MarketOnline::EChannel;
    const TArray<FMarketProduct> Products = Catalog();
    FMarketState S; S.Initialize(Products); S.RivalSeed = 7; S.Cash = 5000000;
    for (int32 I = 0; I < S.Stock.Num(); ++I) { S.Stock[I].Price = Products[I].BasePrice; S.Stock[I].Warehouse = 60; S.Stock[I].Shelf = 20; }
    FString Message;

    // 2011: only the phone.
    TestFalse(TEXT("No web shop in 2011"), MarketOnline::SetChannel(S, EChannel::Web, true, Message));
    TestFalse(TEXT("No platform in 2011"), MarketOnline::SetChannel(S, EChannel::Platform, true, Message));
    TestEqual(TEXT("Nobody shops online in 2011"), MarketOnline::DistrictOnlineShare(S, S.Day), 0.f);
    TestTrue(TEXT("Phone orders"), MarketOnline::SetChannel(S, EChannel::Phone, true, Message));
    AddRegulars(S, 30);
    int32 Orders = 0;
    for (int32 D = 0; D < 7; ++D) { TestTrue(TEXT("Stock and money kept"), Close(S, Products)); Orders += S.Online.LastOrders; }
    TestTrue(TEXT("Regulars ring"), Orders > 0);
    TestTrue(TEXT("Being served at home counts as a visit"), S.Loyalty[0].Visits >= 5);

    // Too many orders for the owner alone: late and cancelled ones hurt the reputation.
    FMarketState Busy = S;
    AddRegulars(Busy, 250);
    const float RepBefore = Busy.Online.Reputation;
    TestTrue(TEXT("Stock and money kept"), Close(Busy, Products));
    TestTrue(TEXT("Without a courier some orders are late or cancelled"), Busy.Online.LastLate + Busy.Online.LastCancelled > 0);
    TestTrue(TEXT("Reputation drops"), Busy.Online.Reputation < RepBefore);
    TestTrue(TEXT("A courier"), MarketOnline::HireCourier(Busy, Message));
    TestEqual(TEXT("A courier carries more"), MarketOnline::DeliveryCapacity(Busy), MarketOnline::OrdersPerCourier);

    // 2017: the district shops online; staying offline costs walk-ins.
    FMarketState Late = S;
    Late.Day = MarketCalendar::GameDayOf(2017, 6, 1);
    TestTrue(TEXT("Online share in 2017"), MarketOnline::DistrictOnlineShare(Late, Late.Day) > 0.01f);
    TestTrue(TEXT("Walk-ins go online"), MarketOnline::StoreTrafficFactor(Late) < 1.f);
    TestFalse(TEXT("Web needs a courier"), MarketOnline::SetChannel(Late, EChannel::Web, true, Message));
    MarketOnline::HireCourier(Late, Message);
    TestTrue(TEXT("Web shop"), MarketOnline::SetChannel(Late, EChannel::Web, true, Message));
    TestTrue(TEXT("Set-up is a cost of the day"), Late.OtherCosts > 0);
    TestTrue(TEXT("Platform"), MarketOnline::SetChannel(Late, EChannel::Platform, true, Message));

    // The 2020 profile: panic buying, curfews, online jump.
    FMarketState Pandemic = Late;
    Pandemic.Day = MarketCalendar::GameDayOf(2020, 3, 16);
    TestEqual(TEXT("Panic buying of staples"), MarketOnline::GroupFactor(Pandemic, MarketGoods::EGroup::Staples), 2.f);
    Pandemic.Day = MarketCalendar::GameDayOf(2020, 4, 18); // Saturday
    TestTrue(TEXT("Weekend curfew"), MarketOnline::IsCurfew(Pandemic, Pandemic.Day));
    TestTrue(TEXT("Few walk-ins on a curfew day"), MarketOnline::StoreTrafficFactor(Pandemic) < 0.5f);
    TestTrue(TEXT("Online jumps"), MarketOnline::DistrictOnlineShare(Pandemic, Pandemic.Day) > 2.f * MarketOnline::DistrictOnlineShare(Late, Late.Day));
    int32 Web = 0;
    for (int32 D = 0; D < 14; ++D) { TestTrue(TEXT("Stock and money kept"), Close(Pandemic, Products, 150)); Web += Pandemic.Online.LastOrders; }
    TestTrue(TEXT("Orders come from web and platform"), Web > 10);
    TestTrue(TEXT("Online revenue is in the day's revenue"), Pandemic.LastRevenue >= Pandemic.Online.LastRevenue);
    FMarketState Off = Pandemic;
    TestTrue(TEXT("The profile can be switched off"), MarketDirector::Command(Off, Products, TEXT("PandemicProfile"), 0, Message));
    TestFalse(TEXT("No curfew without the profile"), MarketOnline::IsCurfew(Off, MarketCalendar::GameDayOf(2020, 4, 18)));

    // Empty shelves: substitution or missing lines.
    FMarketState Empty = Pandemic;
    Empty.Stock[1].Warehouse = Empty.Stock[1].Shelf = 0;
    Empty.Stock[0].Warehouse = Empty.Stock[0].Shelf = 0;
    int32 Missing = 0;
    for (int32 D = 0; D < 7; ++D) { TestTrue(TEXT("Stock and money kept"), Close(Empty, Products, 150)); Missing += Empty.Online.LastMissing + Empty.Online.LastSubstituted; }
    TestTrue(TEXT("Missing items are noticed"), Missing > 0);
    TestFalse(TEXT("Summary"), MarketOnline::Summary(Empty).IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketPaymentsTest, "MirasMarket.Payments.CashAndCards", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketPaymentsTest::RunTest(const FString& Parameters)
{
    using MarketPayments::EMethod;
    using MarketCustomers::ESegment;
    const TArray<FMarketProduct> Products = MarketOnlineTest::Catalog();
    FMarketState S; S.Initialize(Products); S.RivalSeed = 3;
    FString Message;

    TestTrue(TEXT("Cards spread over the years"), MarketPayments::CardShare(1) < MarketPayments::CardShare(MarketCalendar::GameDayOf(2021, 1, 1)));
    TestTrue(TEXT("A child pays cash"), MarketPayments::Choose(S, ESegment::Child, 0.f) == EMethod::Cash);
    TestTrue(TEXT("An office worker wants a card, there is no POS"), MarketPayments::Choose(S, ESegment::Worker, 0.f) == EMethod::NoCard);
    TestTrue(TEXT("Some leave"), MarketPayments::LeavesWithoutCard(S, 0.1f));
    TestFalse(TEXT("Others pay cash"), MarketPayments::LeavesWithoutCard(S, 0.9f));
    TestFalse(TEXT("Meal cards need a POS"), MarketPayments::SetMealCard(S, true, Message));
    TestTrue(TEXT("POS"), MarketDirector::Command(S, Products, TEXT("Card"), 1, Message));
    TestTrue(TEXT("Meal cards"), MarketDirector::Command(S, Products, TEXT("MealCard"), 1, Message));
    TestTrue(TEXT("With a POS the card works"), MarketPayments::Choose(S, ESegment::Worker, 0.35f) == EMethod::Card);
    TestTrue(TEXT("Meal card first for office workers"), MarketPayments::Choose(S, ESegment::Worker, 0.f) == EMethod::MealCard);
    TestTrue(TEXT("Card shoppers spend a little more"), MarketPayments::BudgetFactor(S, ESegment::Worker) > 1.f);

    // A card basket: not in the drawer today, in the bank tomorrow, minus 1.8 %.
    S.Stock[0].Shelf = 10; S.Stock[0].Price = 10000;
    TArray<FMarketSaleLine> Lines; FMarketSaleLine Line; Line.Product = 0; Line.Quantity = 1; Line.QuotedPrice = 10000; Lines.Add(Line);
    const int64 Start = S.Cash;
    int64 Receipt = 0;
    TestTrue(TEXT("Sold"), S.SellBasket(Lines, Products, &Receipt));
    MarketDirector::OnCheckout(S, 999, Receipt, 0.99f, static_cast<uint8>(EMethod::Card));
    TestEqual(TEXT("Not in the drawer"), S.Cash, Start);
    TestEqual(TEXT("Commission"), S.Payments.Commission, static_cast<int64>(180));
    S.CloseDay();
    const int64 ProfitBefore = S.LastProfit;
    const int64 AfterShop = S.Cash;
    MarketPayments::CloseDay(S);
    TestTrue(TEXT("Commission and rent are costs"), S.LastProfit < ProfitBefore - 179);
    TestEqual(TEXT("Waits for tomorrow"), S.Payments.CardTomorrow, static_cast<int64>(9820));
    TestTrue(TEXT("Only the rent left the till"), S.Cash < AfterShop && S.Cash > AfterShop - 1000);
    const int64 Before = S.Cash;
    S.CloseDay();
    MarketPayments::CloseDay(S);
    TestTrue(TEXT("Card money arrived"), S.Cash - Before + S.LastOperatingCost >= 9820);
    TestEqual(TEXT("Nothing pending"), S.Payments.CardTomorrow, static_cast<int64>(0));
    TestFalse(TEXT("Summary"), MarketPayments::Summary(S).IsEmpty());
    return true;
}

#endif

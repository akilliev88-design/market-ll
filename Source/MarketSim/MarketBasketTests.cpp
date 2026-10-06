#include "MarketBasket.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBasketTest, "MarketSim.Customers.BasketAndLoyalty", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketBasketTest::RunTest(const FString& Parameters)
{
    FMarketProduct MilkA; MilkA.Id = TEXT("milk_a"); MilkA.Category = TEXT("milk"); MilkA.BasePrice = 250; MilkA.Cost = 150;
    FMarketProduct MilkB; MilkB.Id = TEXT("milk_b"); MilkB.Category = TEXT("milk"); MilkB.BasePrice = 240; MilkB.Cost = 140;
    FMarketProduct Cola; Cola.Id = TEXT("cola"); Cola.Category = TEXT("drink"); Cola.BasePrice = 300; Cola.Cost = 180;
    FMarketProduct Tea; Tea.Id = TEXT("tea"); Tea.Category = TEXT("tea"); Tea.BasePrice = 500; Tea.Cost = 300;
    const TArray<FMarketProduct> Products = { MilkA, MilkB, Cola, Tea };
    FMarketState State;
    State.Initialize(Products);
    State.ApplyShelfCapacities({ 12, 12, 12, 12 });
    for (FMarketStock& Item : State.Stock) Item.Shelf = 8;

    FRandomStream Random(53053);
    const TArray<int32> List = MarketBasket::BuildList(State, 4, Random);
    TSet<int32> Unique;
    for (const int32 Product : List) Unique.Add(Product);
    TestEqual(TEXT("Four requested lines"), List.Num(), 4);
    TestEqual(TEXT("Every requested product is distinct"), Unique.Num(), 4);

    TArray<int32> Available = { 0, 5, 8, 8 };
    TSet<int32> Excluded;
    TestEqual(TEXT("Same-category substitute is selected"), MarketBasket::FindSubstitute(State, Products, 0, Available, Excluded, 1.f), 1);
    Available[1] = 0;
    TestEqual(TEXT("Another category is never substituted"), MarketBasket::FindSubstitute(State, Products, 0, Available, Excluded, 1.f), INDEX_NONE);

    bool bReturning = false;
    const int32 CustomerId = MarketBasket::ChooseCustomer(State, 1.f, .2f, bReturning);
    TestFalse(TEXT("First visit is new"), bReturning);
    MarketBasket::RecordVisit(State, CustomerId, 4, 4, false);
    const FMarketLoyalty* Happy = MarketBasket::FindCustomer(State, CustomerId);
    TestTrue(TEXT("Full basket raises satisfaction"), Happy && Happy->Visits == 1 && Happy->Satisfaction > 50.f);
    const float LoyalShare = MarketBasket::EffectiveMarketShare(State, CustomerId);
    const int32 ReturningId = MarketBasket::ChooseCustomer(State, 0.f, .5f, bReturning);
    TestTrue(TEXT("A known shopper returns"), bReturning && ReturningId == CustomerId);
    TestTrue(TEXT("Satisfaction improves price tolerance"), LoyalShare > State.MarketShare);
    const float BeforeBadVisit = Happy ? Happy->Satisfaction : 0.f;
    MarketBasket::RecordVisit(State, CustomerId, 4, 0, true);
    Happy = MarketBasket::FindCustomer(State, CustomerId);
    TestTrue(TEXT("An empty delayed visit lowers satisfaction"), Happy && Happy->Satisfaction < BeforeBadVisit);

    State.Stock[0].Shelf = 4; State.Stock[2].Shelf = 3;
    const int64 CashBefore = State.Cash;
    const TArray<FMarketSaleLine> Basket = { { 0, 2, 250 }, { 2, 3, 300 } };
    int64 Receipt = 0; int32 Units = 0;
    TestTrue(TEXT("Multi-product basket sells atomically"), State.SellBasket(Basket, Products, &Receipt, &Units));
    TestTrue(TEXT("One basket is one served shopper"), State.Served == 1 && Units == 5 && Receipt == 1400 && State.Cash == CashBefore + 1400);
    const int32 ShelfBeforeFailure = State.Stock[0].Shelf;
    const int64 CashBeforeFailure = State.Cash;
    TestFalse(TEXT("Unavailable line rejects the whole basket"), State.SellBasket({ { 0, 1, 250 }, { 2, 1, 300 } }, Products));
    TestTrue(TEXT("Rejected basket changes nothing"), State.Stock[0].Shelf == ShelfBeforeFailure && State.Cash == CashBeforeFailure);
    return true;
}

#endif

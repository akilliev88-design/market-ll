#include "MarketEconomy.h"

void FMarketState::Initialize(const TArray<FMarketProduct>& Products)
{
    *this = FMarketState();
    for (const auto& Product : Products)
    {
        FMarketStock Item;
        Item.Id = Product.Id;
        Item.Price = Product.BasePrice;
        Stock.Add(Item);
    }
}

bool FMarketState::Order(int32 Index, const TArray<FMarketProduct>& Products)
{
    if (!Stock.IsValidIndex(Index) || !Products.IsValidIndex(Index)) return false;
    const int32 Units = FMath::Clamp(Products[Index].CaseUnits, 1, 48);
    const int64 Bill = Products[Index].Cost * Units;
    auto& Item = Stock[Index];
    if (Cash < Bill || Item.Incoming + Item.Warehouse + Units > StorageCapacity) return false;
    Cash -= Bill;
    Item.Incoming += Units;
    return true;
}

int32 FMarketState::Restock(int32 Index)
{
    if (!Stock.IsValidIndex(Index)) return 0;
    auto& Item = Stock[Index];
    const int32 Amount = FMath::Clamp(FMath::Min(Item.Capacity - Item.Shelf, Item.Warehouse), 0, MAX_int32);
    Item.Shelf += Amount;
    Item.Warehouse -= Amount;
    return Amount;
}

bool FMarketState::Sell(int32 Index, int32 Quantity, int64 QuotedPrice, const TArray<FMarketProduct>& Products)
{
    if (!Stock.IsValidIndex(Index) || !Products.IsValidIndex(Index) || Quantity <= 0 || QuotedPrice <= 0) return false;
    auto& Item = Stock[Index];
    if (Item.Shelf < Quantity) return false;
    Item.Shelf -= Quantity;
    const int64 Receipt = QuotedPrice * Quantity;
    Cash += Receipt;
    Revenue += Receipt;
    CostOfGoods += Products[Index].Cost * Quantity;
    ++Served;
    return true;
}

void FMarketState::CloseDay()
{
    LastOperatingCost = 2200 + (bCashier ? 2000 : 0);
    // The second branch is an explicit aggregate prototype: net daily contribution.
    LastBranchProfit = bSecondStore ? FMath::RoundToInt64(800 + MarketShare * 35) : 0;
    LastRevenue = Revenue;
    LastCostOfGoods = CostOfGoods;
    LastProfit = Revenue - CostOfGoods - LastOperatingCost + LastBranchProfit;
    Cash += LastBranchProfit - LastOperatingCost;
    if (LastProfit > 0) ++ProfitableDays;
    LastServed = Served;
    LastLost = Lost;
    if (Served + Lost > 0)
    {
        const float Satisfaction = static_cast<float>(Served) / (Served + Lost);
        MarketShare = FMath::Clamp(MarketShare * 0.8f + (12.f + 53.f * Satisfaction) * 0.2f, 5.f, 65.f);
    }
    for (auto& Item : Stock)
    {
        Item.Warehouse += Item.Incoming;
        Item.Incoming = 0;
    }
    ++Day;
    Revenue = CostOfGoods = 0;
    Served = Lost = 0;
}

int32 FMarketState::ApplyShelfCapacities(const TArray<int32>& Capacities)
{
    int32 Discarded = 0;
    for (int32 I = 0; I < Stock.Num(); ++I)
    {
        auto& Item = Stock[I];
        Item.Capacity = Capacities.IsValidIndex(I) ? FMath::Clamp(Capacities[I], 1, MaxShelfCapacity) : DefaultShelfCapacity;
        if (Item.Shelf <= Item.Capacity) continue;
        const int32 Excess = Item.Shelf - Item.Capacity;
        Item.Shelf = Item.Capacity;
        const int32 Room = FMath::Max(0, StorageCapacity - Item.Warehouse - Item.Incoming);
        const int32 Moved = FMath::Min(Excess, Room);
        Item.Warehouse += Moved;
        Discarded += Excess - Moved;
    }
    return Discarded;
}

int32 FMarketState::FillShelfFree(int32 Index)
{
    if (!Stock.IsValidIndex(Index)) return 0;
    auto& Item = Stock[Index];
    const int32 Added = FMath::Max(0, Item.Capacity - Item.Shelf);
    Item.Shelf += Added;
    return Added;
}

int32 FMarketState::ReceiveFree(int32 Index, int32 Units)
{
    if (!Stock.IsValidIndex(Index) || Units <= 0) return 0;
    auto& Item = Stock[Index];
    const int32 Added = FMath::Clamp(Units, 0, FMath::Max(0, StorageCapacity - Item.Warehouse - Item.Incoming));
    Item.Warehouse += Added;
    return Added;
}

bool FMarketState::IsStructurallyValid() const
{
    if (Version != 1 || Day < 1 || !FMath::IsFinite(MarketShare) || MarketShare < 5 || MarketShare > 65) return false;
    TSet<FString> Seen;
    for (const auto& Item : Stock)
    {
        bool bDuplicate = false;
        Seen.Add(Item.Id, &bDuplicate);
        if (bDuplicate || Item.Id.IsEmpty() || Item.Capacity < 1 || Item.Capacity > MaxShelfCapacity || Item.Shelf < 0 || Item.Shelf > Item.Capacity || Item.Warehouse < 0 || Item.Incoming < 0 ||
            Item.Warehouse + Item.Incoming > StorageCapacity || Item.Price < 10) return false;
    }
    return true;
}

bool FMarketState::IsValidFor(const TArray<FMarketProduct>& Products) const
{
    if (!IsStructurallyValid() || Stock.Num() != Products.Num()) return false;
    for (int32 I = 0; I < Stock.Num(); ++I)
    {
        if (Stock[I].Id != Products[I].Id || Stock[I].Price > Products[I].BasePrice * 3) return false;
    }
    return true;
}

int32 FMarketState::ReconcileWith(const TArray<FMarketProduct>& Products, TArray<FString>* OutAdded, TArray<FString>* OutRemoved)
{
    TMap<FString, FMarketStock> Previous;
    for (const auto& Item : Stock) Previous.Add(Item.Id, Item);
    TArray<FMarketStock> Aligned;
    int32 Changes = 0;
    for (const auto& Product : Products)
    {
        if (const FMarketStock* Found = Previous.Find(Product.Id))
        {
            FMarketStock Item = *Found;
            Item.Price = FMath::Clamp<int64>(Item.Price, 10, FMath::Max<int64>(10, Product.BasePrice * 3));
            Aligned.Add(Item);
            Previous.Remove(Product.Id);
        }
        else
        {
            FMarketStock Item;
            Item.Id = Product.Id;
            Item.Shelf = 0;
            Item.Warehouse = 0;
            Item.Price = Product.BasePrice;
            Aligned.Add(Item);
            ++Changes;
            if (OutAdded) OutAdded->Add(Product.Id);
        }
    }
    for (const auto& Pair : Previous)
    {
        ++Changes;
        if (OutRemoved) OutRemoved->Add(Pair.Key);
    }
    Stock = MoveTemp(Aligned);
    return Changes;
}

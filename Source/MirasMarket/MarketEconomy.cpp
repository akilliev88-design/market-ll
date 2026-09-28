#include "MarketEconomy.h"
#include "MarketPrices.h"
#include "MarketSuppliers.h"

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
    TArray<int32> Cases;
    Cases.Init(0, Products.Num());
    Cases[Index] = 1;
    return SubmitOrder(Cases, Products);
}

bool FMarketState::SubmitOrder(const TArray<int32>& Cases, const TArray<FMarketProduct>& Products, int64* OutBill, int32* OutUnits)
{
    if (Products.Num() != Stock.Num() || Cases.Num() != Products.Num()) return false;
    int64 Bill = 0;
    int32 TotalUnits = 0;
    for (int32 I = 0; I < Products.Num(); ++I)
    {
        if (Cases[I] < 0 || Cases[I] > 9) return false;
        const int32 CaseUnits = FMath::Clamp(Products[I].CaseUnits, 1, 48);
        const int32 Units = Cases[I] * CaseUnits;
        if (Units > 0 && Stock[I].Warehouse + Stock[I].Dock + Stock[I].Incoming + Units > StorageCapacity) return false;
        if (Products[I].Cost < 0 || (Units > 0 && Products[I].Cost > MAX_int64 / Units)) return false;
        const int64 LineBill = Products[I].Cost * Units;
        if (Bill > MAX_int64 - LineBill) return false;
        Bill += LineBill;
        TotalUnits += Units;
    }
    if (TotalUnits <= 0 || Cash < Bill) return false;
    Cash -= Bill;
    Purchases += Bill;
    for (int32 I = 0; I < Products.Num(); ++I)
        Stock[I].Incoming += Cases[I] * FMath::Clamp(Products[I].CaseUnits, 1, 48);
    if (OutBill) *OutBill = Bill;
    if (OutUnits) *OutUnits = TotalUnits;
    return true;
}

int32 FMarketState::ReceiveDelivery(int32 Index, int32 MaxUnits)
{
    if (!Stock.IsValidIndex(Index) || MaxUnits <= 0) return 0;
    FMarketStock& Item = Stock[Index];
    const int32 Amount = FMath::Clamp(FMath::Min(Item.Dock, MaxUnits), 0, FMath::Max(0, StorageCapacity - Item.Warehouse - Item.Incoming));
    Item.Dock -= Amount;
    Item.Warehouse += Amount;
    return Amount;
}

int32 FMarketState::Restock(int32 Index, int32 MaxUnits)
{
    if (!Stock.IsValidIndex(Index)) return 0;
    auto& Item = Stock[Index];
    const int32 Amount = FMath::Clamp(FMath::Min3(Item.Capacity - Item.Shelf, Item.Warehouse, MaxUnits), 0, MAX_int32);
    Item.Shelf += Amount;
    Item.Warehouse -= Amount;
    return Amount;
}

bool FMarketState::Sell(int32 Index, int32 Quantity, int64 QuotedPrice, const TArray<FMarketProduct>& Products)
{
    FMarketSaleLine Line;
    Line.Product = Index; Line.Quantity = Quantity; Line.QuotedPrice = QuotedPrice;
    return SellBasket({ Line }, Products);
}

bool FMarketState::SellBasket(const TArray<FMarketSaleLine>& Lines, const TArray<FMarketProduct>& Products, int64* OutReceipt, int32* OutUnits)
{
    if (Lines.Num() == 0 || Stock.Num() != Products.Num()) return false;
    TArray<int32> Required;
    Required.Init(0, Stock.Num());
    TArray<bool> HasLine;
    HasLine.Init(false, Stock.Num());
    int64 Receipt = 0;
    int32 Units = 0;
    for (const FMarketSaleLine& Line : Lines)
    {
        if (!Stock.IsValidIndex(Line.Product) || Line.Quantity <= 0 || Line.QuotedPrice <= 0) return false;
        if (Required[Line.Product] > MAX_int32 - Line.Quantity) return false;
        Required[Line.Product] += Line.Quantity;
        HasLine[Line.Product] = true;
        if (Line.QuotedPrice > MAX_int64 / Line.Quantity) return false;
        const int64 LineReceipt = Line.QuotedPrice * Line.Quantity;
        if (Receipt > MAX_int64 - LineReceipt || Units > MAX_int32 - Line.Quantity) return false;
        Receipt += LineReceipt;
        Units += Line.Quantity;
    }
    for (int32 I = 0; I < Required.Num(); ++I)
        if (Required[I] > Stock[I].Shelf) return false;

    for (const FMarketSaleLine& Line : Lines)
    {
        FMarketStock& Item = Stock[Line.Product];
        Item.Shelf -= Line.Quantity;
        Item.Today.Sold += Line.Quantity;
        CostOfGoods += Products[Line.Product].Cost * Line.Quantity;
    }
    for (int32 I = 0; I < HasLine.Num(); ++I) if (HasLine[I]) ++Stock[I].Today.Buyers;
    Cash += Receipt;
    Revenue += Receipt;
    ++Served;
    if (OutReceipt) *OutReceipt = Receipt;
    if (OutUnits) *OutUnits = Units;
    return true;
}

int64 FMarketState::DailyPayroll() const
{
    if (Staff.Num() == 0) return (bCashier ? 2000 : 0) + FMath::Clamp(Stockers, 0, MaxStockers) * StockerDailyWage;
    int64 Total = 0;
    for (const FMarketEmployee& Employee : Staff) Total += FMath::Max<int64>(0, Employee.DailyWage);
    return Total;
}

void FMarketState::CloseDay()
{
    // Rent-free family shop: electricity, water, bags and upkeep follow the monthly price list (MarketPrices).
    LastOperatingCost = FMath::RoundToInt64(2200.0 * MarketPrices::ListLevel(Day)) + DailyPayroll() + Marketing;
    Marketing = 0;
    // The second branch is an explicit aggregate prototype: net daily contribution.
    LastBranchProfit = bSecondStore ? FMath::RoundToInt64(800 + MarketShare * 35) : 0;
    LastRevenue = Revenue;
    LastCostOfGoods = CostOfGoods;
    LastProfit = Revenue - CostOfGoods - LastOperatingCost + LastBranchProfit;
    Cash += LastBranchProfit - LastOperatingCost;
    if (LastProfit > 0) ++ProfitableDays;
    LastServed = Served;
    LastLost = Lost;
    LastLostWaiting = LostWaiting;
    if (Served + Lost > 0)
    {
        const float Satisfaction = static_cast<float>(Served) / (Served + Lost);
        MarketShare = FMath::Clamp(MarketShare * 0.8f + (12.f + 53.f * Satisfaction) * 0.2f, 5.f, 65.f);
    }
    LastDeliveryMissing = 0;
    LastDeliveryDamaged = 0;
    for (int32 I = 0; I < Stock.Num(); ++I)
    {
        FMarketStock& Item = Stock[I];
        // Stable per day/product: supplier problems cannot be rerolled by reloading a save.
        uint32 Hash = 2166136261u;
        for (const TCHAR Character : Item.Id) { Hash ^= static_cast<uint32>(Character); Hash *= 16777619u; }
        Hash ^= static_cast<uint32>(Day * 7919 + I * 104729);
        // A cheap, careless wholesaler loses and breaks more (MarketSuppliers::Info().DeliveryRisk; 1 = v0.1 odds).
        const uint32 Risk = MarketSuppliers::Info(static_cast<MarketSuppliers::ESupplier>(Supplier)).DeliveryRisk;
        const int32 Missing = Item.Incoming >= 2 && Hash % 23u < Risk ? 1 : 0;
        const int32 Damaged = Item.Incoming - Missing >= 2 && (Hash / 23u) % 17u < Risk ? 1 : 0;
        LastDeliveryMissing += Missing;
        LastDeliveryDamaged += Damaged;
        Item.Dock += Item.Incoming - Missing - Damaged;
        Item.Incoming = 0;
        Item.Yesterday = Item.Today;
        Item.Today = FMarketDemandStats();
    }
    LastPurchases = Purchases;
    Purchases = 0;
    ++Day;
    Revenue = CostOfGoods = 0;
    Served = Lost = LostWaiting = 0;
}

int32 FMarketState::ApplyShelfCapacities(const TArray<int32>& Capacities)
{
    int32 Discarded = 0;
    for (int32 I = 0; I < Stock.Num(); ++I)
    {
        auto& Item = Stock[I];
        // 0 = the product is not on any shelf (planogram): all its units wait in the warehouse.
        Item.Capacity = Capacities.IsValidIndex(I) ? FMath::Clamp(Capacities[I], 0, MaxShelfCapacity) : DefaultShelfCapacity;
        if (Item.Shelf <= Item.Capacity) continue;
        const int32 Excess = Item.Shelf - Item.Capacity;
        Item.Shelf = Item.Capacity;
        const int32 Room = FMath::Max(0, StorageCapacity - Item.Warehouse - Item.Dock - Item.Incoming);
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
    const int32 Added = FMath::Clamp(Units, 0, FMath::Max(0, StorageCapacity - Item.Warehouse - Item.Dock - Item.Incoming));
    Item.Warehouse += Added;
    return Added;
}

int32 FMarketState::DeliveryUnits() const
{
    int32 Total = 0;
    for (const FMarketStock& Item : Stock) Total += Item.Dock;
    return Total;
}

bool FMarketState::IsStructurallyValid() const
{
    if (InheritedDebt < 0 || DebtClearedDay < 0 || WeekDebtPaid < 0 || LastWeekNumber < 0) return false;
    if (Version != 1 || Day < 1 || !FMath::IsFinite(MarketShare) || MarketShare < 5 || MarketShare > 65 || Stockers < 0 || Stockers > MaxStockers) return false;
    TSet<FString> Seen;
    for (const auto& Item : Stock)
    {
        bool bDuplicate = false;
        Seen.Add(Item.Id, &bDuplicate);
        if (bDuplicate || Item.Id.IsEmpty() || Item.Capacity < 0 || Item.Capacity > MaxShelfCapacity || Item.Shelf < 0 || Item.Shelf > Item.Capacity || Item.Warehouse < 0 || Item.Dock < 0 || Item.Incoming < 0 ||
            Item.Warehouse + Item.Dock + Item.Incoming > StorageCapacity || Item.Price < 10) return false;
    }
    if (Books.TaxDue < 0 || Books.VatCarry < 0 || Purchases < 0 || NextEmployeeId < 1) return false;
    if (Marketing < 0) return false;
    if (!FMath::IsFinite(ShelfPriceLevel) || ShelfPriceLevel <= 0.0 || Supplier >= static_cast<uint8>(MarketSuppliers::ESupplier::Count)) return false;
    for (const FMarketPayable& Bill : Payables) if (Bill.Amount < 0) return false;
    TSet<int32> EmployeeIds;
    for (const TArray<FMarketEmployee>* People : { &Staff, &Candidates })
        for (const FMarketEmployee& Employee : *People)
        {
            bool bDuplicate = false;
            if (People == &Staff) EmployeeIds.Add(Employee.Id, &bDuplicate);
            if (bDuplicate || Employee.Role > 3 || Employee.DailyWage < 0 || !FMath::IsFinite(Employee.Morale) || !FMath::IsFinite(Employee.Fatigue) ||
                Employee.Morale < 0.f || Employee.Morale > 100.f || Employee.Fatigue < 0.f || Employee.Fatigue > 100.f) return false;
        }
    TSet<int32> CustomerIds;
    for (const FMarketLoyalty& Customer : Loyalty)
    {
        bool bDuplicate = false;
        CustomerIds.Add(Customer.CustomerId, &bDuplicate);
        if (bDuplicate || Customer.CustomerId < 0 || Customer.CustomerId >= 32 || Customer.Visits < 0 ||
            !FMath::IsFinite(Customer.Satisfaction) || Customer.Satisfaction < 0.f || Customer.Satisfaction > 100.f) return false;
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

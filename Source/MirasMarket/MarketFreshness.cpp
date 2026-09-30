#include "MarketFreshness.h"
#include "MarketCountry.h"
#include "MarketGoods.h"

namespace MarketFreshness
{
    FString FreshTl(int64 Kurus)
    {
        return MarketCountry::Money(Kurus); // G-084: the active country\'s currency
    }

    int32 Held(const FMarketStock& Item) { return Item.Shelf + Item.Warehouse + Item.Dock; }

    // Takes units out of the stock, the shelf front first (old goods are in front), then the depot, then the door.
    int32 RemoveUnits(FMarketStock& Item, int32 Units)
    {
        int32 Left = Units;
        const int32 FromShelf = FMath::Min(Left, Item.Shelf); Item.Shelf -= FromShelf; Left -= FromShelf;
        const int32 FromDepot = FMath::Min(Left, Item.Warehouse); Item.Warehouse -= FromDepot; Left -= FromDepot;
        const int32 FromDoor = FMath::Min(Left, Item.Dock); Item.Dock -= FromDoor; Left -= FromDoor;
        return Units - Left;
    }

    FString Shown(const FMarketProduct& P) { return P.RealName.IsEmpty() ? P.Id : P.RealName; }
}

int32 MarketFreshness::LastDayUnits(const FMarketState& State, const FString& ProductId)
{
    int32 Units = 0;
    for (const FMarketBatch& B : State.Batches) if (B.ProductId == ProductId && B.ExpiresDay == State.Day) Units += B.Units;
    return Units;
}

int32 MarketFreshness::MarkdownUnitsLeft(const FMarketState& State, int32 Index)
{
    if (State.FreshPolicy != static_cast<uint8>(EPolicy::Markdown) || !State.Stock.IsValidIndex(Index)) return 0;
    return FMath::Max(0, LastDayUnits(State, State.Stock[Index].Id) - State.Stock[Index].Today.Sold);
}

float MarketFreshness::PriceFactor(const FMarketState& State, int32 Index)
{
    return MarkdownUnitsLeft(State, Index) > 0 ? 1.f - MarkdownPercent / 100.f : 1.f;
}

double MarketFreshness::PriceFactor(const FMarketState& State, int32 Index, int32 Quantity)
{
    const int32 Units = FMath::Max(1, Quantity);
    const int32 Marked = FMath::Min(Units, MarkdownUnitsLeft(State, Index));
    return (Marked * (1.0 - MarkdownPercent / 100.0) + (Units - Marked)) / Units;
}

int32 MarketFreshness::DaysLeft(const FMarketState& State, const FString& ProductId)
{
    int32 Oldest = MAX_int32;
    for (const FMarketBatch& B : State.Batches) if (B.ProductId == ProductId && B.Units > 0) Oldest = FMath::Min(Oldest, B.ExpiresDay);
    return Oldest == MAX_int32 ? INDEX_NONE : Oldest - State.Day;
}

FString MarketFreshness::PolicyName(EPolicy Policy)
{
    switch (Policy)
    {
    case EPolicy::Markdown: return TEXT("son g\u00fcn %30 indirim");
    case EPolicy::Donate: return TEXT("son g\u00fcn ihtiya\u00e7 sahiplerine ba\u011f\u0131\u015f");
    default: return TEXT("hi\u00e7bir \u015fey yapma");
    }
}

bool MarketFreshness::SetPolicy(FMarketState& State, EPolicy Policy, FString& OutMessage)
{
    if (State.FreshPolicy == static_cast<uint8>(Policy)) { OutMessage = TEXT("Zaten b\u00f6yle."); return false; }
    State.FreshPolicy = static_cast<uint8>(Policy);
    OutMessage = TEXT("Son kullanma g\u00fcn\u00fc gelen \u00fcr\u00fcnler i\u00e7in: ") + PolicyName(Policy) + TEXT(".");
    return true;
}

void MarketFreshness::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    State.LastWasteUnits = 0;
    State.LastWasteCost = 0;
    int32 Donated = 0;
    TArray<FString> Spoiled;
    for (int32 I = 0; I < State.Stock.Num() && I < Products.Num(); ++I)
    {
        FMarketStock& Item = State.Stock[I];
        const int32 Life = MarketGoods::ShelfLifeDays(Products[I]);
        if (Life <= 0) { Item.Received = 0; continue; }
        // Units that arrived since the last close form a new batch; what left the shop (sold, broken, thrown away)
        // left from the oldest batches. Counting arrivals separately matters: a shop that sells 12 and receives 12
        // every day holds the same number, but its milk is new every day.
        int32 InBatches = 0;
        for (const FMarketBatch& B : State.Batches) if (B.ProductId == Item.Id) InBatches += B.Units;
        const int32 Now = Held(Item);
        const int32 Arrived = FMath::Max(0, Item.Received);
        Item.Received = 0;
        const int32 Left = InBatches + Arrived - Now;
        const int32 Fresh = Arrived + FMath::Max(0, -Left); // units nobody recorded (older saves) count as new too
        if (Left > 0)
        {
            int32 Gone = FMath::Min(Left, InBatches);
            // Oldest first.
            State.Batches.Sort([](const FMarketBatch& A, const FMarketBatch& B) { return A.ExpiresDay < B.ExpiresDay; });
            for (FMarketBatch& B : State.Batches)
            {
                if (B.ProductId != Item.Id || Gone <= 0) continue;
                const int32 Take = FMath::Min(Gone, B.Units);
                B.Units -= Take;
                Gone -= Take;
            }
        }
        if (Fresh > 0)
        {
            FMarketBatch New;
            New.ProductId = Item.Id;
            New.Units = FMath::Min(Fresh, Now);
            New.ExpiresDay = State.Day + Life - 1; // arrived at this close: sellable for Life days from tomorrow
            if (New.Units > 0) State.Batches.Add(New);
        }
        // Expired batches become waste; last-day batches are given away under the donation policy.
        for (FMarketBatch& B : State.Batches)
        {
            if (B.ProductId != Item.Id || B.Units <= 0) continue;
            const bool bExpired = B.ExpiresDay < State.Day;
            const bool bDonate = !bExpired && State.FreshPolicy == static_cast<uint8>(EPolicy::Donate) && B.ExpiresDay == State.Day;
            if (!bExpired && !bDonate) continue;
            const int32 Removed = RemoveUnits(Item, B.Units);
            B.Units = 0;
            if (Removed <= 0) continue;
            State.LastWasteUnits += Removed;
            State.LastWasteCost += State.UnitCost(I, Products) * Removed;
            if (bDonate) Donated += Removed;
            else Spoiled.Add(FString::Printf(TEXT("%d %s"), Removed, *Shown(Products[I])));
        }
    }
    // Batches of products that left the catalog or stopped spoiling are dropped.
    State.Batches.RemoveAll([&State, &Products](const FMarketBatch& B)
    {
        if (B.Units <= 0) return true;
        const int32 Row = State.Stock.IndexOfByPredicate([&B](const FMarketStock& S) { return S.Id == B.ProductId; });
        return !Products.IsValidIndex(Row) || MarketGoods::ShelfLifeDays(Products[Row]) <= 0;
    });
    if (State.LastWasteCost > 0)
    {
        State.LastProfit -= State.LastWasteCost;
        if (Spoiled.Num() > 0)
            State.DayNews.Add(FString::Printf(TEXT("Fire: %s tarihi ge\u00e7ti ve at\u0131ld\u0131 (%s zarar). Az sipari\u015f ver ya da son g\u00fcn indirimi yap."),
                *FString::Join(Spoiled, TEXT(", ")), *FreshTl(State.LastWasteCost)));
    }
    if (Donated > 0)
    {
        for (FMarketLoyalty& L : State.Loyalty) L.Satisfaction = FMath::Min(100.f, L.Satisfaction + FMath::Min(3.f, Donated * 0.2f));
        State.DayNews.Add(FString::Printf(TEXT("Son g\u00fcn\u00fc gelen %d \u00fcr\u00fcn mahalledeki ihtiya\u00e7 sahiplerine verildi. Mahalle bunu konu\u015fuyor."), Donated));
    }
}

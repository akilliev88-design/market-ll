#include "MarketPromotions.h"
#include "MarketCountry.h"
#include "MarketGoods.h"
#include "MarketPrices.h"
#include "MarketSuppliers.h"
#include "MarketFreshness.h"
#include "MarketDemand.h"

namespace MarketPromotions
{
    FString PromoTl(int64 Kurus)
    {
        return MarketCountry::Money(Kurus); // G-084: the active country\'s currency
    }

    uint32 PromoMix(int32 Seed, int32 Day, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), static_cast<uint32>(Day), Salt };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }

    EKind KindOf(const FMarketPromotion& Promo)
    {
        return static_cast<EKind>(FMath::Min<uint8>(Promo.Kind, static_cast<uint8>(EKind::Count) - 1));
    }

    EScope ScopeOf(const FMarketPromotion& Promo) { return static_cast<EScope>(FMath::Min<uint8>(Promo.Scope, static_cast<uint8>(EScope::Count) - 1)); }
    EMechanic MechanicOf(const FMarketPromotion& Promo) { return static_cast<EMechanic>(FMath::Min<uint8>(Promo.Mechanic, static_cast<uint8>(EMechanic::Count) - 1)); }

    // G-078: does a scope reach product Index? Key = the brand / subcategory / aisle name of the scope.
    bool InScope(const TArray<FMarketProduct>& Products, int32 Index, EScope Scope, int32 Product, const FString& Key)
    {
        if (!Products.IsValidIndex(Index)) return false;
        const FMarketProduct& P = Products[Index];
        switch (Scope)
        {
        case EScope::Product: return Index == Product;
        case EScope::Brand: return !Key.IsEmpty() && MarketGoods::Fold(P.Brand) == MarketGoods::Fold(Key);
        case EScope::Subcategory: return !Key.IsEmpty() && MarketGoods::Fold(P.Subcategory.IsEmpty() ? P.Category : P.Subcategory) == MarketGoods::Fold(Key);
        case EScope::Category: return MarketGoods::Fold(P.Category) == MarketGoods::Fold(Key);
        default: return true;
        }
    }

    // G-078: 1 = a deal is news; down to 0.3 for a product on a deal most of the last month.
    float DealTrust(const FMarketStock& Item) { return FMath::Clamp(1.f - Item.PromoHeat / 20.f, 0.3f, 1.f); }

    // Price elasticity of a product (catalog value, else a middle value): how strongly a deal moves it.
    float ElasticityOf(const TArray<FMarketProduct>& Products, int32 Index)
    {
        return Products.IsValidIndex(Index) ? MarketDemand::ElasticityOf(Products[Index]) : MarketDemand::DefaultElasticity;
    }

    // Whether a promotion covers catalog product Index.
    bool Covers(const FMarketPromotion& Promo, const TArray<FMarketProduct>& Products, int32 Index)
    {
        if (!Products.IsValidIndex(Index)) return false;
        if (KindOf(Promo) == EKind::Scoped) return InScope(Products, Index, ScopeOf(Promo), Promo.Product, Promo.ScopeKey);
        if (KindOf(Promo) == EKind::AisleDiscount) return MarketGoods::Fold(Products[Index].Category) == MarketGoods::Fold(Promo.Category);
        if (KindOf(Promo) == EKind::Flyer) return false;
        return Promo.Product == Index;
    }

    int32 RunningCount(const FMarketState& State)
    {
        int32 Count = 0;
        for (const FMarketPromotion& P : State.Promotions)
            if (IsActive(P, State.Day) && KindOf(P) != EKind::Endcap) ++Count;
        return Count;
    }

    int32 DaysOf(EKind Kind)
    {
        switch (Kind)
        {
        case EKind::AisleDiscount: return DiscountDays;
        case EKind::MultiBuy: return MultiBuyDays;
        case EKind::Flyer: return FlyerDays;
        case EKind::SupplierDeal: return DealDays;
        default: return 100000; // endcap: until moved
        }
    }

    // Yesterday's units of everything the promotion covers (the "before" of the report). B1 (#30): shop sales only;
    // the online orders of the same day are not part of a promotion's "during" either.
    int32 CoveredSold(const FMarketState& State, const FMarketPromotion& Promo, const TArray<FMarketProduct>& Products)
    {
        int32 Units = 0;
        const TArray<int32>& Online = State.Ledger.OnlineSold;
        for (int32 I = 0; I < State.Stock.Num(); ++I)
            if (Covers(Promo, Products, I)) Units += FMath::Max(0, State.Stock[I].Yesterday.Sold - (Online.IsValidIndex(I) ? Online[I] : 0));
        return Units;
    }

    // x shelf price of the promotions running on Day for Quantity units (1 = none).
    double PromoFactor(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index, int32 Quantity, int32 Day)
    {
        double Factor = 1.0;
        for (const FMarketPromotion& P : State.Promotions)
        {
            if (!IsActive(P, Day) || !Covers(P, Products, Index)) continue;
            switch (KindOf(P))
            {
            case EKind::AisleDiscount: Factor = FMath::Min(Factor, 1.0 - FMath::Clamp(P.Percent, 0, 50) / 100.0); break;
            case EKind::SupplierDeal: Factor = FMath::Min(Factor, 1.0 - DealShelfCut / 100.0); break;
            case EKind::MultiBuy:
                // Three for two: every third unit is free.
                if (Quantity >= 3) Factor = FMath::Min(Factor, static_cast<double>(Quantity - Quantity / 3) / Quantity);
                break;
            case EKind::Scoped:
                switch (MechanicOf(P))
                {
                case EMechanic::Percent: Factor = FMath::Min(Factor, 1.0 - FMath::Clamp(P.Percent, 0, 50) / 100.0); break;
                case EMechanic::ThreeForTwo: if (Quantity >= 3) Factor = FMath::Min(Factor, static_cast<double>(Quantity - Quantity / 3) / Quantity); break;
                case EMechanic::TwoForOne: if (Quantity >= 2) Factor = FMath::Min(Factor, static_cast<double>(Quantity - Quantity / 2) / Quantity); break;
                case EMechanic::SecondHalf: if (Quantity >= 2) Factor = FMath::Min(Factor, (Quantity - 0.5 * (Quantity / 2)) / Quantity); break;
                default: break;
                }
                break;
            default: break;
            }
        }
        return Factor;
    }

    // B1 (#27): the tally of a running promotion (created on its first counted day).
    FMarketPromoTally& TallyOf(FMarketState& State, const FMarketPromotion& Promo)
    {
        TArray<FMarketPromoTally>& Tallies = State.Ledger.PromoTallies;
        for (FMarketPromoTally& T : Tallies)
            if (T.Kind == Promo.Kind && T.Product == Promo.Product && T.StartDay == Promo.StartDay) return T;
        FMarketPromoTally& New = Tallies.AddDefaulted_GetRef();
        New.Kind = Promo.Kind; New.Product = Promo.Product; New.StartDay = Promo.StartDay;
        return New;
    }
}

bool MarketPromotions::IsActive(const FMarketPromotion& Promo, int32 Day)
{
    return Promo.StartDay > 0 && Promo.StartDay <= Day && Day <= Promo.EndDay;
}

TArray<const FMarketPromotion*> MarketPromotions::Active(const FMarketState& State)
{
    TArray<const FMarketPromotion*> Result;
    for (const FMarketPromotion& P : State.Promotions) if (IsActive(P, State.Day)) Result.Add(&P);
    return Result;
}

int32 MarketPromotions::AdjustQuantity(const FMarketState& State, int32 Index, int32 Quantity)
{
    for (const FMarketPromotion& P : State.Promotions)
    {
        if (!IsActive(P, State.Day) || P.Product != Index) continue;
        if (KindOf(P) == EKind::MultiBuy && Quantity >= 2) return FMath::Max(3, Quantity);
        // G-078: a single-product multi-buy tops the basket up to the free unit. Wider scopes only change the price
        // (UnitPrice); the shopper who already takes enough gets the deal.
        if (KindOf(P) != EKind::Scoped || ScopeOf(P) != EScope::Product) continue;
        if (MechanicOf(P) == EMechanic::ThreeForTwo && Quantity >= 2) return FMath::Max(3, Quantity);
        if (MechanicOf(P) == EMechanic::TwoForOne && Quantity >= 1) return FMath::Max(2, Quantity);
    }
    return Quantity;
}

int64 MarketPromotions::UnitPrice(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index, int32 Quantity)
{
    if (!State.Stock.IsValidIndex(Index)) return 0;
    const int64 Shelf = State.Stock[Index].Price;
    // Last-day markdown of an old batch (B1 #43: only on the units of that batch still on the shelf).
    const double Fresh = MarketFreshness::PriceFactor(State, Index, Quantity);
    const double Factor = FMath::Min(Fresh, PromoFactor(State, Products, Index, Quantity, State.Day));
    return FMath::Max<int64>(1, FMath::RoundToInt64(Shelf * Factor));
}

int64 MarketPromotions::DealPrice(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index, int32 Quantity, int32 Day)
{
    if (!State.Stock.IsValidIndex(Index)) return 0;
    return FMath::Max<int64>(1, FMath::RoundToInt64(State.Stock[Index].Price * PromoFactor(State, Products, Index, FMath::Max(1, Quantity), Day)));
}

float MarketPromotions::Interest(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index)
{
    float Factor = 1.f;
    bool bPromoted = false;
    bool bFlyer = false;
    for (const FMarketPromotion& P : State.Promotions)
    {
        if (!IsActive(P, State.Day)) continue;
        if (KindOf(P) == EKind::Flyer) { bFlyer = true; continue; }
        if (!Covers(P, Products, Index)) continue;
        bPromoted = true;
        switch (KindOf(P))
        {
        case EKind::AisleDiscount: Factor *= 1.f + FMath::Clamp(P.Percent, 0, 50) / 50.f; break;
        case EKind::MultiBuy: Factor *= 1.35f; break;
        case EKind::Endcap: Factor *= 1.6f; break;
        case EKind::SupplierDeal: Factor *= 1.2f; break;
        case EKind::Scoped:
        {
            // G-078: the deeper the deal and the more price-sensitive the product, the more it is sought; a
            // store-wide sale lifts each product less (the shopper does not notice every price).
            const float Depth = MechanicOf(P) == EMechanic::Percent ? FMath::Clamp(P.Percent, 0, 50) / 100.f
                : MechanicOf(P) == EMechanic::TwoForOne ? 0.5f : MechanicOf(P) == EMechanic::ThreeForTwo ? 0.33f : 0.25f;
            const float Reach = ScopeOf(P) == EScope::Store ? 0.35f : ScopeOf(P) == EScope::Product ? 1.f : 0.75f;
            Factor *= 1.f + Depth * ElasticityOf(Products, Index) * 0.6f * Reach;
            break;
        }
        default: break;
        }
    }
    if (bFlyer && bPromoted) Factor *= 1.25f; // the flyer shows the promoted products
    if (State.Stock.IsValidIndex(Index))
    {
        // G-078: a product always on a deal stops exciting shoppers (its "real" price becomes the deal price), and
        // what they stocked up at home on the last deal keeps them away for a while.
        if (bPromoted) Factor = 1.f + (Factor - 1.f) * DealTrust(State.Stock[Index]);
        Factor *= FMath::Clamp(1.f - State.Stock[Index].Pantry * 0.02f, 0.6f, 1.f);
    }
    return FMath::Clamp(Factor, 0.5f, 3.f);
}

float MarketPromotions::TrafficFactor(const FMarketState& State)
{
    float Factor = 1.f;
    for (const FMarketPromotion& P : State.Promotions)
    {
        if (!IsActive(P, State.Day)) continue;
        if (KindOf(P) == EKind::Flyer) Factor = FMath::Max(Factor, 1.15f);
        // G-078: a store-wide sale is news in the neighbourhood.
        if (KindOf(P) == EKind::Scoped && ScopeOf(P) == EScope::Store && MechanicOf(P) == EMechanic::Percent)
            Factor *= 1.f + FMath::Clamp(P.Percent, 0, 50) / 200.f;
    }
    return FMath::Min(Factor, 1.35f);
}

float MarketPromotions::CostFactor(const FMarketState& State, int32 Index)
{
    for (const FMarketPromotion& P : State.Promotions)
        if (IsActive(P, State.Day) && KindOf(P) == EKind::SupplierDeal && P.Product == Index) return 1.f - DealCostCut / 100.f;
    return 1.f;
}

FString MarketPromotions::Badge(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index)
{
    if (MarketFreshness::PriceFactor(State, Index) < 1.f) return FString::Printf(TEXT("son g\u00fcn %%%d"), MarketFreshness::MarkdownPercent);
    for (const FMarketPromotion& P : State.Promotions)
    {
        if (!IsActive(P, State.Day) || !Covers(P, Products, Index)) continue;
        switch (KindOf(P))
        {
        case EKind::AisleDiscount: return FString::Printf(TEXT("%%%d indirim"), P.Percent);
        case EKind::MultiBuy: return TEXT("3 al 2 \u00f6de");
        case EKind::SupplierDeal: return FString::Printf(TEXT("%%%d indirim"), DealShelfCut);
        case EKind::Endcap: return TEXT("gondol ba\u015f\u0131");
        case EKind::Scoped: return MechanicName(MechanicOf(P), P.Percent);
        default: break;
        }
    }
    return FString();
}

FString MarketPromotions::Describe(const FMarketPromotion& Promo, const TArray<FMarketProduct>& Products)
{
    const FString Name = Products.IsValidIndex(Promo.Product)
        ? (Products[Promo.Product].RealName.IsEmpty() ? Products[Promo.Product].Id : Products[Promo.Product].RealName) : FString();
    switch (KindOf(Promo))
    {
    case EKind::AisleDiscount: return FString::Printf(TEXT("%s reyonunda %%%d indirim"), *Promo.Category, Promo.Percent);
    case EKind::MultiBuy: return FString::Printf(TEXT("%s: 3 al 2 \u00f6de"), *Name);
    case EKind::Flyer: return TEXT("Mahalleye bro\u015f\u00fcr");
    case EKind::Endcap: return FString::Printf(TEXT("%s gondol ba\u015f\u0131nda"), *Name);
    case EKind::Scoped:
    {
        const FString Where = ScopeOf(Promo) == EScope::Product ? Name : ScopeOf(Promo) == EScope::Store ? FString(TEXT("T\u00fcm ma\u011faza"))
            : FString::Printf(TEXT("%s (%s)"), *Promo.ScopeKey, *ScopeName(ScopeOf(Promo)).ToLower());
        return FString::Printf(TEXT("%s: %s"), *Where, *MechanicName(MechanicOf(Promo), Promo.Percent));
    }
    default: return FString::Printf(TEXT("%s: toptanc\u0131 destekli %%%d indirim"), *Name, DealShelfCut);
    }
}

bool MarketPromotions::Start(FMarketState& State, const TArray<FMarketProduct>& Products, EKind Kind, int32 Product, int32 Percent, FString& OutMessage)
{
    if (Kind != EKind::Flyer && !Products.IsValidIndex(Product)) { OutMessage = TEXT("\u00d6nce bir \u00fcr\u00fcn se\u00e7."); return false; }
    if (Kind != EKind::Endcap && RunningCount(State) >= MaxRunning)
    {
        OutMessage = FString::Printf(TEXT("Ayn\u0131 anda en \u00e7ok %d kampanya y\u00fcr\u00fcr; biri bitsin ya da durdur."), MaxRunning);
        return false;
    }
    FMarketPromotion Promo;
    Promo.Kind = static_cast<uint8>(Kind);
    Promo.Product = Kind == EKind::Flyer ? INDEX_NONE : Product;
    Promo.Category = Products.IsValidIndex(Product) ? Products[Product].Category : FString();
    Promo.Percent = Kind == EKind::AisleDiscount ? (Percent >= 20 ? 20 : 10) : 0;
    Promo.StartDay = State.Day;
    Promo.EndDay = State.Day + DaysOf(Kind) - 1;
    for (const FMarketPromotion& P : State.Promotions)
    {
        if (!IsActive(P, State.Day) || P.Kind != Promo.Kind) continue;
        const bool bSame = Kind == EKind::Flyer || (Kind == EKind::AisleDiscount ? MarketGoods::Fold(P.Category) == MarketGoods::Fold(Promo.Category) : P.Product == Promo.Product);
        if (bSame) { OutMessage = TEXT("Bu kampanya zaten s\u00fcr\u00fcyor."); return false; }
    }
    if (Kind == EKind::Flyer)
    {
        Promo.Cost = FMath::RoundToInt64(FlyerCost * MarketPrices::ListLevel(State.Day));
        if (State.Cash < Promo.Cost + State.Marketing) { OutMessage = FString::Printf(TEXT("Bro\u015f\u00fcr bask\u0131s\u0131 ve da\u011f\u0131t\u0131m\u0131 %s tutar; kasada yok."), *PromoTl(Promo.Cost)); return false; }
        State.Marketing += Promo.Cost;
    }
    if (Kind == EKind::Endcap)
        for (FMarketPromotion& P : State.Promotions)
            if (IsActive(P, State.Day) && KindOf(P) == EKind::Endcap) P.EndDay = State.Day - 1; // one gondola head
    Promo.Baseline = CoveredSold(State, Promo, Products) * FMath::Min(DaysOf(Kind), 30);
    State.Promotions.Add(Promo);
    switch (Kind)
    {
    case EKind::AisleDiscount:
        OutMessage = FString::Printf(TEXT("%s reyonunda %%%d indirim ba\u015flad\u0131 (%d g\u00fcn). Reyon daha \u00e7ok aran\u0131r, rakipler fark edebilir."), *Promo.Category, Promo.Percent, DiscountDays); break;
    case EKind::MultiBuy:
        OutMessage = FString::Printf(TEXT("\"3 al 2 \u00f6de\" ba\u015flad\u0131 (%d g\u00fcn). \u0130ki ve daha \u00e7ok alan \u00fc\u00e7 al\u0131r, birini bedava g\u00f6t\u00fcr\u00fcr."), MultiBuyDays); break;
    case EKind::Flyer:
        OutMessage = FString::Printf(TEXT("Bro\u015f\u00fcr mahalleye da\u011f\u0131t\u0131l\u0131yor (%s, %d g\u00fcn). Daha \u00e7ok m\u00fc\u015fteri gelir; kampanyal\u0131 \u00fcr\u00fcnler \u00f6ne \u00e7\u0131kar."), *PromoTl(Promo.Cost), FlyerDays); break;
    case EKind::Endcap:
        OutMessage = TEXT("\u00dcr\u00fcn gondol ba\u015f\u0131na kondu: giri\u015fte g\u00f6r\u00fcn\u00fcr, daha \u00e7ok al\u0131n\u0131r."); break;
    default:
        OutMessage = TEXT("Toptanc\u0131 destekli kampanya ba\u015flad\u0131."); break;
    }
    return true;
}

bool MarketPromotions::Stop(FMarketState& State, int32 PromotionIndex, FString& OutMessage)
{
    if (!State.Promotions.IsValidIndex(PromotionIndex) || !IsActive(State.Promotions[PromotionIndex], State.Day))
    {
        OutMessage = TEXT("Bu kampanya zaten bitmi\u015f.");
        return false;
    }
    State.Promotions[PromotionIndex].EndDay = State.Day - 1;
    OutMessage = TEXT("Kampanya durduruldu; sonucu g\u00fcn sonu raporunda.");
    return true;
}

bool MarketPromotions::AcceptOffer(FMarketState& State, const TArray<FMarketProduct>& Products, FString& OutMessage)
{
    if (State.Offer.Product == INDEX_NONE || State.Offer.StartDay != 0 || State.Offer.EndDay < State.Day)
    {
        OutMessage = TEXT("Bekleyen bir toptanc\u0131 teklifi yok.");
        return false;
    }
    if (RunningCount(State) >= MaxRunning) { OutMessage = TEXT("\u00d6nce y\u00fcr\u00fcyen kampanyalardan biri bitsin."); return false; }
    FMarketPromotion Deal = State.Offer;
    Deal.StartDay = State.Day;
    Deal.EndDay = State.Day + DealDays - 1;
    Deal.Baseline = CoveredSold(State, Deal, Products) * DealDays;
    State.Promotions.Add(Deal);
    State.Offer = FMarketPromotion();
    OutMessage = FString::Printf(TEXT("Teklif kabul: %d g\u00fcn boyunca al\u0131\u015f %%%d ucuz, rafta %%%d indirim."), DealDays, DealCostCut, DealShelfCut);
    return true;
}

void MarketPromotions::DeclineOffer(FMarketState& State)
{
    State.Offer = FMarketPromotion();
}

void MarketPromotions::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    const int32 Closed = State.Day - 1;
    if (Closed < 1) return;
    TArray<FString>& News = State.DayNews;
    // G-078 shopper memory: deal days and the stock shoppers took home.
    for (int32 Index = 0; Index < State.Stock.Num() && Index < Products.Num(); ++Index)
    {
        bool bOnDeal = false;
        for (const FMarketPromotion& P : State.Promotions)
            if (IsActive(P, Closed) && KindOf(P) != EKind::Flyer && KindOf(P) != EKind::Endcap && Covers(P, Products, Index)) { bOnDeal = true; break; }
        FMarketStock& Item = State.Stock[Index];
        Item.PromoHeat = Item.PromoHeat * 0.97f + (bOnDeal ? 1.f : 0.f);
        const float Stockpile = Products[Index].Stockpile >= 0.f ? Products[Index].Stockpile : 0.6f;
        Item.Pantry = Item.Pantry * 0.8f + (bOnDeal ? Item.Yesterday.Sold * Stockpile * 0.3f : 0.f);
    }
    for (int32 I = 0; I < State.Promotions.Num();)
    {
        FMarketPromotion& P = State.Promotions[I];
        if (IsActive(P, Closed))
        {
            const bool bTally = KindOf(P) != EKind::Flyer && KindOf(P) != EKind::Endcap;
            for (int32 Index = 0; Index < State.Stock.Num(); ++Index)
            {
                if (!Covers(P, Products, Index)) continue;
                const int32 Sold = State.Stock[Index].Yesterday.Sold;
                P.Sold += Sold;
                const int64 Price = State.Stock[Index].Price;
                const int64 LostBefore = P.MarginLost;
                if (KindOf(P) == EKind::AisleDiscount) P.MarginLost += Price * Sold * P.Percent / 100;
                else if (KindOf(P) == EKind::Scoped)
                {
                    // Estimate of the price given away (baskets are gone by now): % off on every unit; multi-buys on
                    // the free share of the units.
                    switch (MechanicOf(P))
                    {
                    case EMechanic::Percent: P.MarginLost += Price * Sold * FMath::Clamp(P.Percent, 0, 50) / 100; break;
                    case EMechanic::ThreeForTwo: P.MarginLost += Price * (Sold / 3); break;
                    case EMechanic::TwoForOne: P.MarginLost += Price * (Sold / 2); break;
                    case EMechanic::SecondHalf: P.MarginLost += Price * (Sold / 2) / 2; break;
                    default: break;
                    }
                }
                else if (KindOf(P) == EKind::SupplierDeal) P.MarginLost += Price * Sold * DealShelfCut / 100;
                else if (KindOf(P) == EKind::MultiBuy) P.MarginLost += Price * (Sold / 3);
                if (!bTally || Index >= Products.Num()) continue;
                // B1 (#27): what the covered units brought in and cost; the funded offer's support comes with the
                // units delivered while it runs (bought at the cut cost; Products hold the closed day's costs).
                FMarketPromoTally& Tally = TallyOf(State, P);
                Tally.Revenue += Price * Sold - (P.MarginLost - LostBefore);
                Tally.CostOfGoods += State.UnitCost(Index, Products) * Sold;
                if (KindOf(P) == EKind::SupplierDeal && Index == P.Product && State.Stock[Index].Received > 0)
                    Tally.Support += FMath::RoundToInt64(static_cast<double>(State.Stock[Index].Received) * Products[Index].Cost * DealCostCut / (100.0 - DealCostCut));
            }
        }
        if (P.EndDay < State.Day && P.StartDay > 0)
        {
            const int32 Days = FMath::Max(1, FMath::Min(P.EndDay, Closed) - P.StartDay + 1);
            FString Line = FString::Printf(TEXT("Kampanya bitti: %s. %d g\u00fcnde %d adet"), *Describe(P, Products), Days, P.Sold);
            if (KindOf(P) != EKind::Flyer)
            {
                const int32 Planned = KindOf(P) == EKind::Scoped ? FMath::Max(1, P.EndDay - P.StartDay + 1) : DaysOf(KindOf(P));
                const int32 Expected = P.Baseline * Days / FMath::Max(1, FMath::Min(Planned, 30));
                Line += FString::Printf(TEXT(" (\u00f6ncesine g\u00f6re %s%d)"), P.Sold >= Expected ? TEXT("+") : TEXT(""), P.Sold - Expected);
            }
            if (P.MarginLost > 0) Line += TEXT(", verilen indirim ") + PromoTl(P.MarginLost);
            if (P.Cost > 0) Line += TEXT(", maliyet ") + PromoTl(P.Cost);
            // B1 (#27): the real result: the gross profit of the covered sales, and the wholesaler's support.
            const int32 TallyAt = State.Ledger.PromoTallies.IndexOfByPredicate([&P](const FMarketPromoTally& T)
                { return T.Kind == P.Kind && T.Product == P.Product && T.StartDay == P.StartDay; });
            if (TallyAt != INDEX_NONE)
            {
                const FMarketPromoTally& Tally = State.Ledger.PromoTallies[TallyAt];
                if (KindOf(P) == EKind::SupplierDeal) Line += TEXT(", toptanc\u0131 deste\u011fi ") + PromoTl(Tally.Support);
                Line += TEXT(", sat\u0131\u015flar\u0131n br\u00fct k\u00e2r\u0131 ") + PromoTl(Tally.Revenue - Tally.CostOfGoods - P.Cost);
                State.Ledger.PromoTallies.RemoveAt(TallyAt);
            }
            News.Add(Line + TEXT("."));
            State.Promotions.RemoveAt(I);
            continue;
        }
        ++I;
    }

    // B1 (#27): tallies of promotions that no longer exist (stopped saves, removed rows) are dropped.
    State.Ledger.PromoTallies.RemoveAll([&State](const FMarketPromoTally& T)
    {
        return !State.Promotions.ContainsByPredicate([&T](const FMarketPromotion& P) { return P.Kind == T.Kind && P.Product == T.Product && P.StartDay == T.StartDay; });
    });

    // B1 (#27): units sold below their cost on the closed day (deepest deal price of the day; the last-day markdown
    // is left out: selling old milk cheap is its purpose).
    State.Ledger.LastBelowCostUnits = 0;
    State.Ledger.LastBelowCostLoss = 0;
    int32 WorstIndex = INDEX_NONE;
    int64 WorstLoss = 0;
    for (int32 Index = 0; Index < State.Stock.Num() && Index < Products.Num(); ++Index)
    {
        const int32 Sold = State.Stock[Index].Yesterday.Sold;
        if (Sold <= 0) continue;
        const int64 Gap = State.UnitCost(Index, Products) - DealPrice(State, Products, Index, 3, Closed);
        if (Gap <= 0) continue;
        State.Ledger.LastBelowCostUnits += Sold;
        State.Ledger.LastBelowCostLoss += Gap * Sold;
        if (Gap * Sold > WorstLoss) { WorstLoss = Gap * Sold; WorstIndex = Index; }
    }
    if (State.Ledger.LastBelowCostLoss > 0 && Products.IsValidIndex(WorstIndex))
        News.Add(FString::Printf(TEXT("Maliyetin alt\u0131nda sat\u0131\u015f: %d adet, %s zarar (en \u00e7ok %s)."), State.Ledger.LastBelowCostUnits,
            *PromoTl(State.Ledger.LastBelowCostLoss), *(Products[WorstIndex].RealName.IsEmpty() ? Products[WorstIndex].Id : Products[WorstIndex].RealName)));

    // The wholesaler's offer: expires after its deadline; a new one now and then when Selim trusts the shop.
    if (State.Offer.Product != INDEX_NONE && State.Offer.EndDay < State.Day) State.Offer = FMarketPromotion();
    const FMarketSupplierAccount* Selim = MarketSuppliers::FindAccount(State, MarketSuppliers::ESupplier::TrakyaGida);
    if (State.Offer.Product == INDEX_NONE && State.Supplier == 0 && Selim && Selim->Trust >= OfferTrust &&
        PromoMix(State.RivalSeed, Closed, 0x0FFE4u) % 10u == 0u)
    {
        TArray<int32> Carried;
        for (int32 I = 0; I < State.Stock.Num() && I < Products.Num(); ++I) if (State.Stock[I].Capacity > 0) Carried.Add(I);
        if (Carried.Num() > 0)
        {
            const int32 Product = Carried[PromoMix(State.RivalSeed, Closed, 0x0FFE5u) % static_cast<uint32>(Carried.Num())];
            State.Offer = FMarketPromotion();
            State.Offer.Kind = static_cast<uint8>(EKind::SupplierDeal);
            State.Offer.Product = Product;
            State.Offer.Category = Products[Product].Category;
            State.Offer.Percent = DealCostCut;
            State.Offer.EndDay = State.Day + 2; // answer within three days
            News.Add(FString::Printf(TEXT("Selim'in teklifi: %s i\u00e7in %d g\u00fcn al\u0131\u015fta %%%d destek; kar\u015f\u0131l\u0131\u011f\u0131nda rafta %%%d indirim. %d. g\u00fcne kadar men\u00fcden kabul et."),
                *(Products[Product].RealName.IsEmpty() ? Products[Product].Id : Products[Product].RealName), DealDays, DealCostCut, DealShelfCut, State.Offer.EndDay));
        }
    }
}

// ---- G-078: scoped campaigns (karar J07) -------------------------------------------------------------------------

FString MarketPromotions::ScopeName(EScope Scope)
{
    switch (Scope)
    {
    case EScope::Product: return TEXT("\u00dcr\u00fcn");
    case EScope::Brand: return TEXT("Marka");
    case EScope::Subcategory: return TEXT("Alt grup");
    case EScope::Category: return TEXT("Reyon");
    default: return TEXT("T\u00fcm ma\u011faza");
    }
}

FString MarketPromotions::MechanicName(EMechanic Mechanic, int32 Percent)
{
    switch (Mechanic)
    {
    case EMechanic::ThreeForTwo: return TEXT("3 al 2 \u00f6de");
    case EMechanic::TwoForOne: return TEXT("2 al 1 \u00f6de");
    case EMechanic::SecondHalf: return TEXT("2. \u00fcr\u00fcn %50");
    default: return FString::Printf(TEXT("%%%d indirim"), FMath::Clamp(Percent, 0, 50));
    }
}

FString MarketPromotions::ScopeKeyOf(const TArray<FMarketProduct>& Products, int32 Product, EScope Scope)
{
    if (!Products.IsValidIndex(Product)) return FString();
    const FMarketProduct& P = Products[Product];
    switch (Scope)
    {
    case EScope::Brand: return P.Brand;
    case EScope::Subcategory: return P.Subcategory.IsEmpty() ? P.Category : P.Subcategory;
    case EScope::Category: return P.Category;
    default: return FString();
    }
}

int32 MarketPromotions::ScopeSize(const TArray<FMarketProduct>& Products, int32 Product, EScope Scope)
{
    const FString Key = ScopeKeyOf(Products, Product, Scope);
    int32 Count = 0;
    for (int32 I = 0; I < Products.Num(); ++I) if (InScope(Products, I, Scope, Product, Key)) ++Count;
    return Count;
}

int32 MarketPromotions::PackArg(int32 Product, EScope Scope, EMechanic Mechanic, int32 Percent, int32 Days)
{
    return FMath::Clamp(Product, 0, 9999) + 10000 * static_cast<int32>(Scope) + 100000 * static_cast<int32>(Mechanic)
        + 1000000 * FMath::Clamp(Percent, 0, 99) + 100000000 * FMath::Clamp(Days, 1, MaxScopedDays);
}

bool MarketPromotions::StartScoped(FMarketState& State, const TArray<FMarketProduct>& Products, int32 PackedArg, FString& OutMessage)
{
    if (PackedArg < 0) { OutMessage = TEXT("Ge\u00e7ersiz kampanya."); return false; }
    const int32 Product = PackedArg % 10000;
    const EScope Scope = static_cast<EScope>(FMath::Min((PackedArg / 10000) % 10, static_cast<int32>(EScope::Count) - 1));
    const EMechanic Mechanic = static_cast<EMechanic>(FMath::Min((PackedArg / 100000) % 10, static_cast<int32>(EMechanic::Count) - 1));
    const int32 Percent = FMath::Clamp((PackedArg / 1000000) % 100, 5, 50);
    const int32 Days = FMath::Clamp(PackedArg / 100000000, 1, MaxScopedDays);
    if (!Products.IsValidIndex(Product)) { OutMessage = TEXT("\u00d6nce bir \u00fcr\u00fcn se\u00e7."); return false; }
    if (RunningCount(State) >= MaxRunning)
    {
        OutMessage = FString::Printf(TEXT("Ayn\u0131 anda en \u00e7ok %d kampanya y\u00fcr\u00fcr; biri bitsin ya da durdur."), MaxRunning);
        return false;
    }
    const FString Key = ScopeKeyOf(Products, Product, Scope);
    if ((Scope == EScope::Brand || Scope == EScope::Subcategory) && Key.IsEmpty())
    {
        OutMessage = FString::Printf(TEXT("Bu \u00fcr\u00fcn\u00fcn %s bilgisi yok; \u00fcr\u00fcn ya da reyon kapsam\u0131n\u0131 se\u00e7."), *ScopeName(Scope).ToLower());
        return false;
    }
    FMarketPromotion Promo;
    Promo.Kind = static_cast<uint8>(EKind::Scoped);
    Promo.Scope = static_cast<uint8>(Scope);
    Promo.Mechanic = static_cast<uint8>(Mechanic);
    Promo.ScopeKey = Key;
    Promo.Product = Product;
    Promo.Category = Products[Product].Category;
    Promo.Percent = Mechanic == EMechanic::Percent ? Percent : 0;
    Promo.StartDay = State.Day;
    Promo.EndDay = State.Day + Days - 1;
    for (const FMarketPromotion& P : State.Promotions)
    {
        if (!IsActive(P, State.Day) || KindOf(P) != EKind::Scoped || P.Scope != Promo.Scope) continue;
        const bool bSame = Scope == EScope::Store || (Scope == EScope::Product ? P.Product == Product : MarketGoods::Fold(P.ScopeKey) == MarketGoods::Fold(Key));
        if (bSame) { OutMessage = TEXT("Bu kapsamda bir kampanya zaten s\u00fcr\u00fcyor."); return false; }
    }
    Promo.Baseline = CoveredSold(State, Promo, Products) * FMath::Min(Days, 30);
    State.Promotions.Add(Promo);
    OutMessage = FString::Printf(TEXT("Kampanya ba\u015flad\u0131: %s, %d g\u00fcn (%d \u00fcr\u00fcn). Sonucu bitince g\u00fcn raporunda."),
        *Describe(Promo, Products), Days, ScopeSize(Products, Product, Scope));
    return true;
}

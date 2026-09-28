#include "MarketPromotions.h"
#include "MarketGoods.h"
#include "MarketPrices.h"
#include "MarketSuppliers.h"

namespace MarketPromotions
{
    FString PromoTl(int64 Kurus)
    {
        const int64 Abs = Kurus < 0 ? -Kurus : Kurus;
        return FString::Printf(TEXT("%s%lld,%02lld TL"), Kurus < 0 ? TEXT("-") : TEXT(""), static_cast<long long>(Abs / 100), static_cast<long long>(Abs % 100));
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

    // Whether a promotion covers catalog product Index.
    bool Covers(const FMarketPromotion& Promo, const TArray<FMarketProduct>& Products, int32 Index)
    {
        if (!Products.IsValidIndex(Index)) return false;
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

    // Yesterday's units of everything the promotion covers (the "before" of the report).
    int32 CoveredSold(const FMarketState& State, const FMarketPromotion& Promo, const TArray<FMarketProduct>& Products)
    {
        int32 Units = 0;
        for (int32 I = 0; I < State.Stock.Num(); ++I)
            if (Covers(Promo, Products, I)) Units += State.Stock[I].Yesterday.Sold;
        return Units;
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
        if (IsActive(P, State.Day) && KindOf(P) == EKind::MultiBuy && P.Product == Index && Quantity >= 2) return FMath::Max(3, Quantity);
    return Quantity;
}

int64 MarketPromotions::UnitPrice(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index, int32 Quantity)
{
    if (!State.Stock.IsValidIndex(Index)) return 0;
    const int64 Shelf = State.Stock[Index].Price;
    double Factor = 1.0;
    for (const FMarketPromotion& P : State.Promotions)
    {
        if (!IsActive(P, State.Day) || !Covers(P, Products, Index)) continue;
        switch (KindOf(P))
        {
        case EKind::AisleDiscount: Factor = FMath::Min(Factor, 1.0 - FMath::Clamp(P.Percent, 0, 50) / 100.0); break;
        case EKind::SupplierDeal: Factor = FMath::Min(Factor, 1.0 - DealShelfCut / 100.0); break;
        case EKind::MultiBuy:
            // Three for two: every third unit is free.
            if (Quantity >= 3) Factor = FMath::Min(Factor, static_cast<double>(Quantity - Quantity / 3) / Quantity);
            break;
        default: break;
        }
    }
    return FMath::Max<int64>(1, FMath::RoundToInt64(Shelf * Factor));
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
        default: break;
        }
    }
    if (bFlyer && bPromoted) Factor *= 1.25f; // the flyer shows the promoted products
    return FMath::Clamp(Factor, 0.5f, 3.f);
}

float MarketPromotions::TrafficFactor(const FMarketState& State)
{
    for (const FMarketPromotion& P : State.Promotions)
        if (IsActive(P, State.Day) && KindOf(P) == EKind::Flyer) return 1.15f;
    return 1.f;
}

float MarketPromotions::CostFactor(const FMarketState& State, int32 Index)
{
    for (const FMarketPromotion& P : State.Promotions)
        if (IsActive(P, State.Day) && KindOf(P) == EKind::SupplierDeal && P.Product == Index) return 1.f - DealCostCut / 100.f;
    return 1.f;
}

FString MarketPromotions::Badge(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index)
{
    for (const FMarketPromotion& P : State.Promotions)
    {
        if (!IsActive(P, State.Day) || !Covers(P, Products, Index)) continue;
        switch (KindOf(P))
        {
        case EKind::AisleDiscount: return FString::Printf(TEXT("%%%d indirim"), P.Percent);
        case EKind::MultiBuy: return TEXT("3 al 2 \u00f6de");
        case EKind::SupplierDeal: return FString::Printf(TEXT("%%%d indirim"), DealShelfCut);
        case EKind::Endcap: return TEXT("gondol ba\u015f\u0131");
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
    for (int32 I = 0; I < State.Promotions.Num();)
    {
        FMarketPromotion& P = State.Promotions[I];
        if (IsActive(P, Closed))
        {
            for (int32 Index = 0; Index < State.Stock.Num(); ++Index)
            {
                if (!Covers(P, Products, Index)) continue;
                const int32 Sold = State.Stock[Index].Yesterday.Sold;
                P.Sold += Sold;
                const int64 Price = State.Stock[Index].Price;
                if (KindOf(P) == EKind::AisleDiscount) P.MarginLost += Price * Sold * P.Percent / 100;
                else if (KindOf(P) == EKind::SupplierDeal) P.MarginLost += Price * Sold * DealShelfCut / 100;
                else if (KindOf(P) == EKind::MultiBuy) P.MarginLost += Price * (Sold / 3);
            }
        }
        if (P.EndDay < State.Day && P.StartDay > 0)
        {
            const int32 Days = FMath::Max(1, FMath::Min(P.EndDay, Closed) - P.StartDay + 1);
            FString Line = FString::Printf(TEXT("Kampanya bitti: %s. %d g\u00fcnde %d adet"), *Describe(P, Products), Days, P.Sold);
            if (KindOf(P) != EKind::Flyer)
            {
                const int32 Expected = P.Baseline * Days / FMath::Max(1, FMath::Min(DaysOf(KindOf(P)), 30));
                Line += FString::Printf(TEXT(" (\u00f6ncesine g\u00f6re %s%d)"), P.Sold >= Expected ? TEXT("+") : TEXT(""), P.Sold - Expected);
            }
            if (P.MarginLost > 0) Line += TEXT(", verilen indirim ") + PromoTl(P.MarginLost);
            if (P.Cost > 0) Line += TEXT(", maliyet ") + PromoTl(P.Cost);
            News.Add(Line + TEXT("."));
            State.Promotions.RemoveAt(I);
            continue;
        }
        ++I;
    }

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

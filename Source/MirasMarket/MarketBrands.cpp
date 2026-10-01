#include "MarketBrands.h"
#include "MarketBranches.h"
#include "MarketCompany.h"
#include "MarketCountry.h"
#include "MarketGoods.h"
#include "MarketPrices.h"
#include "MarketStoreAssign.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace MarketBrandsLocal
{
    struct FTableBox
    {
        bool bLoaded = false;
        TArray<MarketBrands::FBrandInfo> Rows;
    };

    FTableBox& Box()
    {
        static FTableBox Table;
        return Table;
    }

    float TierPower(const FString& Tier)
    {
        const FString Key = MarketGoods::Fold(Tier);
        if (Key == TEXT("lider")) return 1.f;
        if (Key == TEXT("guclu")) return 0.7f;
        if (Key == TEXT("orta")) return 0.45f;
        return 0.3f;
    }

    void Load(TArray<MarketBrands::FBrandInfo>& Out)
    {
        Out.Reset();
        FString Json;
        if (!FFileHelper::LoadFileToString(Json, *(FPaths::ProjectConfigDir() / TEXT("markalar.json")))) return;
        TSharedPtr<FJsonObject> Root;
        if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid()) return;
        const TArray<TSharedPtr<FJsonValue>>* Brands = nullptr;
        if (!Root->TryGetArrayField(TEXT("brands"), Brands)) return;
        for (const TSharedPtr<FJsonValue>& Value : *Brands)
        {
            const TSharedPtr<FJsonObject> Obj = Value.IsValid() ? Value->AsObject() : nullptr;
            if (!Obj.IsValid()) continue;
            MarketBrands::FBrandInfo Row;
            Obj->TryGetStringField(TEXT("real"), Row.Real);
            Obj->TryGetStringField(TEXT("fictional"), Row.Fictional);
            Obj->TryGetStringField(TEXT("category"), Row.Category);
            FString Tier;
            Obj->TryGetStringField(TEXT("tier"), Tier);
            Row.Power = TierPower(Tier);
            if (!Row.Real.IsEmpty()) Out.Add(Row);
        }
    }

    // Shelf capacity of every catalog product over the family shop and the live branches (branch items are in the
    // catalog's order; a changed catalog falls back to a search).
    TArray<int32> Capacities(const FMarketState& State, const TArray<FMarketProduct>& Products)
    {
        TArray<int32> Cap;
        Cap.Init(0, Products.Num());
        for (int32 I = 0; I < Products.Num(); ++I) if (State.Stock.IsValidIndex(I)) Cap[I] += FMath::Max(0, State.Stock[I].Capacity);
        for (const FMarketBranch& B : State.Branches)
        {
            if (B.Stage == static_cast<uint8>(MarketBranches::EStage::Closed)) continue;
            for (int32 I = 0; I < Products.Num(); ++I)
            {
                const FMarketBranchItem* Item = B.Items.IsValidIndex(I) && B.Items[I].ProductId == Products[I].Id ? &B.Items[I]
                    : B.Items.FindByPredicate([&Products, I](const FMarketBranchItem& It) { return It.ProductId == Products[I].Id; });
                if (Item) Cap[I] += FMath::Max(0, Item->Capacity);
            }
        }
        return Cap;
    }

    int32 Sum(const TArray<FMarketProduct>& Products, const TArray<int32>& Cap, const FString& Brand, const FString& Category)
    {
        int32 Total = 0;
        for (int32 I = 0; I < Products.Num(); ++I)
            if (Products[I].Category == Category && (Brand.IsEmpty() || Products[I].Brand == Brand)) Total += Cap[I];
        return Total;
    }

    FMarketBrandShare* ShareOf(FMarketState& State, const FString& Brand, const FString& Category)
    {
        return State.Brands.Shares.FindByPredicate([&](const FMarketBrandShare& S) { return S.Brand == Brand && S.Category == Category; });
    }

    float Roll(const FMarketState& State, uint32 A, uint32 B)
    {
        uint32 H = static_cast<uint32>(State.RivalSeed) * 0x9E3779B1u ^ (A + 0x632BE5ABu) * 0x85EBCA6Bu ^ (B + 0x2545F491u) * 0xC2B2AE35u;
        H ^= H >> 16; H *= 0x85EBCA6Bu; H ^= H >> 13;
        return static_cast<float>(H % 10000u) / 10000.f;
    }

    uint32 Hash(const FString& Text) { return MarketStoreAssign::StableHash(Text); }

    int64 AisleSales(const FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Category)
    {
        TSet<FString> Seen;
        int64 Total = 0;
        for (const FMarketProduct& P : Products)
        {
            if (P.Category != Category || Seen.Contains(P.Brand)) continue;
            Seen.Add(P.Brand);
            if (const int64* Sales = State.Brands.MonthSales.Find(P.Brand)) Total += *Sales;
        }
        return Total;
    }

    // A brand's share entries all hold the same trust: move them together.
    void AddTrust(FMarketState& State, const FString& Brand, float Delta)
    {
        for (FMarketBrandShare& S : State.Brands.Shares) if (S.Brand == Brand) S.Trust = FMath::Clamp(S.Trust + Delta, -100.f, 100.f);
    }

    FString Percent(float Share) { return FString::Printf(TEXT("%%%.0f"), Share * 100.f); }
}

const TArray<MarketBrands::FBrandInfo>& MarketBrands::Table()
{
    MarketBrandsLocal::FTableBox& Box = MarketBrandsLocal::Box();
    if (!Box.bLoaded) { MarketBrandsLocal::Load(Box.Rows); Box.bLoaded = true; }
    return Box.Rows;
}

void MarketBrands::SetTable(const TArray<FBrandInfo>& Brands)
{
    MarketBrandsLocal::FTableBox& Box = MarketBrandsLocal::Box();
    Box.Rows = Brands;
    Box.bLoaded = true;
}

void MarketBrands::ResetTable()
{
    MarketBrandsLocal::FTableBox& Box = MarketBrandsLocal::Box();
    Box.Rows.Reset();
    Box.bLoaded = false;
}

float MarketBrands::Power(const FString& Brand, const FString& Category)
{
    const FString Folded = MarketGoods::Fold(Category);
    for (const FBrandInfo& Row : Table())
        if (Row.Real == Brand && (Row.Category.IsEmpty() || MarketGoods::Fold(Row.Category) == Folded)) return Row.Power;
    for (const FBrandInfo& Row : Table()) if (Row.Real == Brand) return Row.Power;
    return 0.45f;
}

FString MarketBrands::NameOf(const FMarketState& State, const FString& Brand)
{
    if (State.bRealBrands) return Brand;
    for (const FBrandInfo& Row : Table()) if (Row.Real == Brand && !Row.Fictional.IsEmpty()) return Row.Fictional;
    return Brand;
}

int32 MarketBrands::ShelfUnits(const FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Brand, const FString& Category)
{
    return MarketBrandsLocal::Sum(Products, MarketBrandsLocal::Capacities(State, Products), Brand, Category);
}

int32 MarketBrands::AisleUnits(const FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Category)
{
    return MarketBrandsLocal::Sum(Products, MarketBrandsLocal::Capacities(State, Products), FString(), Category);
}

float MarketBrands::ShelfShare(const FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Brand, const FString& Category)
{
    const TArray<int32> Cap = MarketBrandsLocal::Capacities(State, Products);
    const int32 Aisle = MarketBrandsLocal::Sum(Products, Cap, FString(), Category);
    return Aisle > 0 ? static_cast<float>(MarketBrandsLocal::Sum(Products, Cap, Brand, Category)) / Aisle : 0.f;
}

float MarketBrands::NationalShare(const FMarketState& State, const FString& Brand, const FString& Category)
{
    const FMarketBrandShare* S = State.Brands.Shares.FindByPredicate([&](const FMarketBrandShare& R) { return R.Brand == Brand && R.Category == Category; });
    return S ? S->National : 0.f;
}

float MarketBrands::Trust(const FMarketState& State, const FString& Brand)
{
    const FMarketBrandShare* S = State.Brands.Shares.FindByPredicate([&Brand](const FMarketBrandShare& R) { return R.Brand == Brand; });
    return S ? S->Trust : 0.f;
}

float MarketBrands::CostFactor(const FMarketState& State, const FString& Brand)
{
    const float T = Trust(State, Brand);
    return T <= -30.f ? 1.03f : T >= 40.f ? 0.98f : 1.f;
}

TArray<FString> MarketBrands::Categories(const TArray<FMarketProduct>& Products)
{
    TArray<FString> List;
    for (const FMarketProduct& P : Products) if (!P.Category.IsEmpty() && !P.Brand.IsEmpty()) List.AddUnique(P.Category);
    return List;
}

void MarketBrands::Ensure(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    for (const FString& Category : Categories(Products))
    {
        TArray<FString> Brands;
        for (const FMarketProduct& P : Products) if (P.Category == Category && !P.Brand.IsEmpty()) Brands.AddUnique(P.Brand);
        bool bNew = false;
        for (const FString& Brand : Brands)
        {
            if (MarketBrandsLocal::ShareOf(State, Brand, Category)) continue;
            FMarketBrandShare& S = State.Brands.Shares.AddDefaulted_GetRef();
            S.Brand = Brand; S.Category = Category;
            S.Trust = MarketBrands::Trust(State, Brand); // a brand already known in another aisle keeps its mood
            bNew = true;
        }
        if (!bNew) continue;
        // Shares from the brands' standing (the aisle adds to 1).
        float Total = 0.f;
        for (const FString& Brand : Brands) Total += Power(Brand, Category);
        for (const FString& Brand : Brands)
            if (FMarketBrandShare* S = MarketBrandsLocal::ShareOf(State, Brand, Category)) S->National = Total > 0.f ? Power(Brand, Category) / Total : 0.f;
    }
    if (State.Brands.LastMonthDay == 0) State.Brands.LastMonthDay = State.Day;
}

bool MarketBrands::Accept(FMarketState& State, int32 OfferId, FString& OutMessage)
{
    const int32 Index = State.Brands.Offers.IndexOfByPredicate([OfferId](const FMarketBrandOffer& O) { return O.Id == OfferId; });
    if (Index == INDEX_NONE) { OutMessage = TEXT("Bu teklif art\u0131k yok."); return false; }
    const FMarketBrandOffer Offer = State.Brands.Offers[Index];
    State.Brands.Offers.RemoveAt(Index);
    FMarketBrandDeal& Deal = State.Brands.Deals.AddDefaulted_GetRef();
    Deal.Terms = Offer;
    Deal.StartDay = State.Day;
    Deal.UntilDay = Offer.Kind == static_cast<uint8>(EKind::Listing) ? State.Day + OfferDays : State.Day + FMath::Max(1, Offer.Months) * MonthDays;
    MarketBrandsLocal::AddTrust(State, Offer.Brand, 5.f);
    OutMessage = FString::Printf(TEXT("%s ile anla\u015ft\u0131n: %s"), *NameOf(State, Offer.Brand), *DescribeOffer(State, Offer));
    return true;
}

bool MarketBrands::Reject(FMarketState& State, int32 OfferId, FString& OutMessage)
{
    const int32 Index = State.Brands.Offers.IndexOfByPredicate([OfferId](const FMarketBrandOffer& O) { return O.Id == OfferId; });
    if (Index == INDEX_NONE) { OutMessage = TEXT("Bu teklif art\u0131k yok."); return false; }
    const FString Brand = State.Brands.Offers[Index].Brand;
    State.Brands.Offers.RemoveAt(Index);
    MarketBrandsLocal::AddTrust(State, Brand, -3.f);
    OutMessage = FString::Printf(TEXT("%s teklifini geri \u00e7evirdin."), *NameOf(State, Brand));
    return true;
}

FString MarketBrands::DescribeOffer(const FMarketState& State, const FMarketBrandOffer& Offer)
{
    const FString Aisle = MarketGoods::GroupName(MarketGoods::Classify(Offer.Category));
    switch (static_cast<EKind>(Offer.Kind))
    {
    case EKind::ShelfShare:
        return FString::Printf(TEXT("%s raf\u0131nda pay\u0131 %s olursa %d ay boyunca ayda %s \u00f6der."), *Offer.Category, *MarketBrandsLocal::Percent(Offer.Target), Offer.Months, *MarketCountry::Money(Offer.Amount));
    case EKind::Listing:
        return FString::Printf(TEXT("%s \u00fcr\u00fcn\u00fcn\u00fc %d g\u00fcn i\u00e7inde rafa koyarsan bir kerede %s \u00f6der."), *Offer.ProductName, OfferDays, *MarketCountry::Money(Offer.Amount));
    case EKind::Rebate:
        return FString::Printf(TEXT("\u00dcr\u00fcnlerinden ayda %s \u00fcst\u00fc satarsan sat\u0131\u015f\u0131n %s'\u00fc kadar prim \u00f6der (%d ay)."), *MarketCountry::Money(Offer.Amount), *MarketBrandsLocal::Percent(Offer.Target), Offer.Months);
    default:
        return Aisle;
    }
}

FString MarketBrands::DescribeDeal(const FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketBrandDeal& Deal)
{
    const FMarketBrandOffer& T = Deal.Terms;
    const int32 Left = FMath::Max(0, Deal.UntilDay - State.Day);
    if (T.Kind == static_cast<uint8>(EKind::ShelfShare))
        return FString::Printf(TEXT("%s \u00b7 %s raf\u0131: \u015fimdi %s / hedef %s \u00b7 ayda %s \u00b7 %d g\u00fcn kald\u0131%s"), *NameOf(State, T.Brand), *T.Category,
            *MarketBrandsLocal::Percent(ShelfShare(State, Products, T.Brand, T.Category)), *MarketBrandsLocal::Percent(T.Target), *MarketCountry::Money(T.Amount), Left,
            Deal.Strikes > 0 ? TEXT(" \u00b7 bir kez ka\u00e7\u0131rd\u0131n") : TEXT(""));
    if (T.Kind == static_cast<uint8>(EKind::Listing))
        return FString::Printf(TEXT("%s \u00b7 %s rafa gelince %s \u00b7 %d g\u00fcn kald\u0131"), *NameOf(State, T.Brand), *T.ProductName, *MarketCountry::Money(T.Amount), Left);
    const int64* Sales = State.Brands.MonthSales.Find(T.Brand);
    return FString::Printf(TEXT("%s \u00b7 ciro primi %s \u00b7 bu ay %s / taban %s \u00b7 %d g\u00fcn kald\u0131"), *NameOf(State, T.Brand), *MarketBrandsLocal::Percent(T.Target),
        *MarketCountry::Money(Sales ? *Sales : 0), *MarketCountry::Money(T.Amount), Left);
}

FString MarketBrands::AisleLine(const FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Category)
{
    const TArray<int32> Cap = MarketBrandsLocal::Capacities(State, Products);
    const int32 Aisle = MarketBrandsLocal::Sum(Products, Cap, FString(), Category);
    TArray<const FMarketBrandShare*> Rows;
    for (const FMarketBrandShare& S : State.Brands.Shares) if (S.Category == Category) Rows.Add(&S);
    Rows.Sort([](const FMarketBrandShare& A, const FMarketBrandShare& B) { return A.National > B.National; });
    TArray<FString> Parts;
    for (const FMarketBrandShare* S : Rows)
    {
        const int32 Ours = MarketBrandsLocal::Sum(Products, Cap, S->Brand, Category);
        Parts.Add(FString::Printf(TEXT("%s %s (rafta %s)"), *NameOf(State, S->Brand), *MarketBrandsLocal::Percent(S->National),
            *MarketBrandsLocal::Percent(Aisle > 0 ? static_cast<float>(Ours) / Aisle : 0.f)));
    }
    return FString::Printf(TEXT("%s: %s"), *Category, *FString::Join(Parts, TEXT(" \u00b7 ")));
}

void MarketBrands::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    using namespace MarketBrandsLocal;
    Ensure(State, Products);
    FMarketBrandsState& R = State.Brands;

    // The closed day's sales by brand (family shop at its labels, branches at their price index).
    for (int32 I = 0; I < Products.Num(); ++I)
    {
        const FMarketProduct& P = Products[I];
        if (P.Brand.IsEmpty()) continue;
        int64 Sales = State.Stock.IsValidIndex(I) ? static_cast<int64>(State.Stock[I].Yesterday.Sold) * State.Stock[I].Price : 0;
        for (const FMarketBranch& B : State.Branches)
        {
            if (B.Stage != static_cast<uint8>(MarketBranches::EStage::Open)) continue;
            const FMarketBranchItem* Item = B.Items.IsValidIndex(I) && B.Items[I].ProductId == P.Id ? &B.Items[I] : nullptr;
            if (Item) Sales += FMath::RoundToInt64(static_cast<double>(Item->LastSold) * P.BasePrice * B.PriceIndex);
        }
        if (Sales > 0) R.MonthSales.FindOrAdd(P.Brand) += Sales;
    }

    // Listing money: paid the day the product is on a shelf.
    {
        const TArray<int32> Cap = Capacities(State, Products);
        for (int32 D = R.Deals.Num() - 1; D >= 0; --D)
        {
            FMarketBrandDeal& Deal = R.Deals[D];
            if (Deal.Terms.Kind != static_cast<uint8>(EKind::Listing)) continue;
            const int32 Product = Products.IndexOfByPredicate([&Deal](const FMarketProduct& P) { return P.Id == Deal.Terms.ProductId; });
            if (Cap.IsValidIndex(Product) && Cap[Product] > 0)
            {
                State.Cash += Deal.Terms.Amount;
                MarketLedger::Post(State, MarketLedger::EAccount::BrandListing, Deal.Terms.Amount, true, MarketLedger::HeadOfficeStore); // C3 (B7.2)
                R.TotalReceived += Deal.Terms.Amount;
                AddTrust(State, Deal.Terms.Brand, 10.f);
                State.DayNews.Add(FString::Printf(TEXT("%s raf paras\u0131n\u0131 \u00f6dedi: %s."), *NameOf(State, Deal.Terms.Brand), *MarketCountry::Money(Deal.Terms.Amount)));
                R.Deals.RemoveAt(D);
            }
            else if (State.Day > Deal.UntilDay)
            {
                AddTrust(State, Deal.Terms.Brand, -10.f);
                State.DayNews.Add(FString::Printf(TEXT("%s: s\u00f6z verilen \u00fcr\u00fcn rafa gelmedi, anla\u015fma d\u00fc\u015ft\u00fc."), *NameOf(State, Deal.Terms.Brand)));
                R.Deals.RemoveAt(D);
            }
        }
    }
    R.Offers.RemoveAll([&State](const FMarketBrandOffer& O) { return State.Day > O.ExpireDay; });
    if (State.Day - R.LastMonthDay < MonthDays) return;
    R.LastMonthDay = State.Day;
    const double Level = MarketPrices::ListLevel(State.Day);

    // The month's deals.
    for (int32 D = R.Deals.Num() - 1; D >= 0; --D)
    {
        FMarketBrandDeal& Deal = R.Deals[D];
        const FMarketBrandOffer& T = Deal.Terms;
        const FString Name = NameOf(State, T.Brand);
        if (T.Kind == static_cast<uint8>(EKind::ShelfShare))
        {
            const float Share = MarketBrands::ShelfShare(State, Products, T.Brand, T.Category);
            if (Share + 0.005f >= T.Target)
            {
                State.Cash += T.Amount; Deal.Paid += T.Amount; R.TotalReceived += T.Amount;
                MarketLedger::Post(State, MarketLedger::EAccount::BrandShelfShare, T.Amount, true, MarketLedger::HeadOfficeStore);
                AddTrust(State, T.Brand, 5.f);
                State.DayNews.Add(FString::Printf(TEXT("%s ayl\u0131k raf \u00f6demesini yapt\u0131: %s (%s raf\u0131nda pay\u0131 %s)."), *Name, *MarketCountry::Money(T.Amount), *T.Category, *Percent(Share)));
            }
            else
            {
                ++Deal.Strikes;
                AddTrust(State, T.Brand, -10.f);
                State.DayNews.Add(FString::Printf(TEXT("%s \u00f6demedi: %s raf\u0131nda pay\u0131 %s, s\u00f6z %s idi."), *Name, *T.Category, *Percent(Share), *Percent(T.Target)));
            }
        }
        else if (T.Kind == static_cast<uint8>(EKind::Rebate))
        {
            const int64 Sales = R.MonthSales.FindRef(T.Brand);
            if (Sales >= T.Amount)
            {
                const int64 Prim = FMath::RoundToInt64(Sales * T.Target);
                State.Cash += Prim; Deal.Paid += Prim; R.TotalReceived += Prim;
                MarketLedger::Post(State, MarketLedger::EAccount::BrandRebate, Prim, true, MarketLedger::HeadOfficeStore);
                AddTrust(State, T.Brand, 5.f);
                State.DayNews.Add(FString::Printf(TEXT("%s ciro primi \u00f6dedi: %s."), *Name, *MarketCountry::Money(Prim)));
            }
        }
        if (Deal.Strikes >= 2)
        {
            AddTrust(State, T.Brand, -20.f);
            State.DayNews.Add(FString::Printf(TEXT("%s anla\u015fmay\u0131 bozdu: iki ay \u00fcst \u00fcste s\u00f6z tutulmad\u0131."), *Name));
            R.Deals.RemoveAt(D);
            continue;
        }
        if (State.Day >= Deal.UntilDay)
        {
            AddTrust(State, T.Brand, 10.f);
            State.DayNews.Add(FString::Printf(TEXT("%s ile anla\u015fma bitti. Toplam %s \u00f6dedi."), *Name, *MarketCountry::Money(Deal.Paid)));
            R.Deals.RemoveAt(D);
        }
    }

    // Trust from how we treat them, and the national shares drift with our shelves once we are big.
    const TArray<int32> Cap = Capacities(State, Products);
    const float Weight = FMath::Clamp(MarketCompany::NationalShare(State) / 100.f, 0.f, 0.3f);
    for (const FString& Category : Categories(Products))
    {
        const int32 Aisle = Sum(Products, Cap, FString(), Category);
        float Total = 0.f;
        for (FMarketBrandShare& S : R.Shares)
        {
            if (S.Category != Category) continue;
            const float Shelf = Aisle > 0 ? static_cast<float>(Sum(Products, Cap, S.Brand, Category)) / Aisle : 0.f;
            if (S.National >= 0.2f)
            {
                const float Before = S.Trust;
                S.Trust = FMath::Clamp(S.Trust + (Shelf < S.National * 0.5f ? -10.f : Shelf >= S.National ? 5.f : 0.f), -100.f, 100.f);
                if (Before > -30.f && S.Trust <= -30.f)
                    State.DayNews.Add(FString::Printf(TEXT("%s sana k\u00fcst\u00fc: %s raf\u0131nda yer bulamad\u0131, fiyatlar\u0131n\u0131 sana %%3 art\u0131rd\u0131."), *NameOf(State, S.Brand), *Category));
                if (Before < 40.f && S.Trust >= 40.f)
                    State.DayNews.Add(FString::Printf(TEXT("%s seni iyi m\u00fc\u015fteri say\u0131yor: fiyatlar\u0131na %%2 indirim."), *NameOf(State, S.Brand)));
            }
            const float Wobble = (Roll(State, Hash(S.Brand + Category), static_cast<uint32>(State.Day)) - 0.5f) * 0.01f;
            S.National = FMath::Max(0.01f, S.National + (Shelf - S.National) * Weight * 0.1f + Wobble);
            Total += S.National;
        }
        if (Total > 0.f) for (FMarketBrandShare& S : R.Shares) if (S.Category == Category) S.National /= Total;
    }

    // New offers: the brands short on our shelves come to us.
    TArray<FString> Aisles = Categories(Products);
    const uint32 Month = static_cast<uint32>(State.Day / MonthDays);
    for (int32 K = 0; K < Aisles.Num() && R.Offers.Num() < MaxOpenOffers; ++K)
    {
        const FString& Category = Aisles[(K + Month) % Aisles.Num()];
        const int64 Sales = AisleSales(State, Products, Category);
        const int32 Aisle = Sum(Products, Cap, FString(), Category);
        for (const FMarketBrandShare& S : R.Shares)
        {
            if (S.Category != Category || R.Offers.Num() >= MaxOpenOffers) continue;
            const bool bBusy = R.Deals.ContainsByPredicate([&S](const FMarketBrandDeal& D) { return D.Terms.Brand == S.Brand; })
                || R.Offers.ContainsByPredicate([&S](const FMarketBrandOffer& O) { return O.Brand == S.Brand; });
            if (bBusy || S.Trust <= -50.f) continue;
            const float Shelf = Aisle > 0 ? static_cast<float>(Sum(Products, Cap, S.Brand, Category)) / Aisle : 0.f;
            const float P = MarketBrands::Power(S.Brand, Category);
            const float Luck = Roll(State, Hash(S.Brand) ^ Month, 7u);
            FMarketBrandOffer Offer;
            Offer.Brand = S.Brand; Offer.Category = Category; Offer.ExpireDay = State.Day + OfferDays; Offer.Months = DealMonths;
            // A product of theirs we do not carry: listing money.
            int32 NotCarried = INDEX_NONE;
            for (int32 I = 0; I < Products.Num(); ++I) if (Products[I].Brand == S.Brand && Products[I].Category == Category && Cap[I] <= 0) { NotCarried = I; break; }
            if (NotCarried != INDEX_NONE && Luck < 0.5f)
            {
                Offer.Kind = static_cast<uint8>(EKind::Listing);
                Offer.ProductId = Products[NotCarried].Id;
                Offer.ProductName = State.bRealBrands ? Products[NotCarried].RealName : Products[NotCarried].FictionalName;
                Offer.Amount = FMath::Max(FMath::RoundToInt64(50000.0 * Level * P), Sales / 50);
            }
            else if (S.National >= 0.15f && Shelf < S.National + 0.05f && Luck < 0.6f + 0.3f * P)
            {
                Offer.Kind = static_cast<uint8>(EKind::ShelfShare);
                Offer.Target = FMath::Min(0.7f, FMath::Max(S.National + 0.1f, Shelf + 0.1f));
                Offer.Amount = FMath::Max(FMath::RoundToInt64(30000.0 * Level * P), FMath::RoundToInt64(Sales * 0.03 * P));
            }
            else if (P >= 0.7f && R.MonthSales.FindRef(S.Brand) > FMath::RoundToInt64(200000.0 * Level) && Luck < 0.4f)
            {
                Offer.Kind = static_cast<uint8>(EKind::Rebate);
                Offer.Target = 0.02f + 0.02f * P;
                Offer.Amount = R.MonthSales.FindRef(S.Brand) * 11 / 10;
            }
            else continue;
            Offer.Id = R.NextOfferId++;
            R.Offers.Add(Offer);
            const FString What = DescribeOffer(State, Offer);
            State.DayNews.Add(FString::Printf(TEXT("%s temsilcisi geldi: \"%s\" Kampanyalar \u203a Markalar."), *NameOf(State, S.Brand), *What));
        }
    }
    R.MonthSales.Reset();
}

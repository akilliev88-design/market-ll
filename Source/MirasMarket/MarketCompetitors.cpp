#include "MarketCompetitors.h"
#include "MarketCalendar.h"
#include "MarketGoods.h"
#include "MarketPromotions.h"
#include "MarketRivals.h"
#include "MarketStaff.h"

namespace MarketCompetitors
{
    enum ETold : int32 { ToldStruggling = 1, ToldRaised = 2, ToldSecondStore = 4, ToldOpened = 8 };

    uint32 RivalMix(int32 Seed, int32 Day, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), static_cast<uint32>(Day), Salt };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }

    FMarketCompetitor* FindMutable(FMarketState& State, ECompany Company)
    {
        return State.Competitors.FindByPredicate([Company](const FMarketCompetitor& C) { return C.Company == static_cast<uint8>(Company); });
    }

    float Attraction(float PriceIndex, float Service, float Proximity, int32 Stores)
    {
        return FMath::Exp(-(PriceIndex - 1.f) / PriceSensitivity) * Service * Proximity * FMath::Sqrt(static_cast<float>(FMath::Max(1, Stores)));
    }

    // A101 pushes hard in its first month in the district.
    float OpeningPush(const FMarketState& State, ECompany Company)
    {
        const int32 Open = Profile(Company).OpenDay;
        return Company == ECompany::A101 && State.Day >= Open && State.Day < Open + 30 ? 0.98f : 1.f;
    }
}

const MarketCompetitors::FProfile& MarketCompetitors::Profile(ECompany Company)
{
    static const FProfile Profiles[static_cast<int32>(ECompany::Count)] =
    {
        { TEXT("Bereket Market"), TEXT("mahalle marketi"), 1.00f, 0.90f, 1.30f, 1, 150000, INDEX_NONE },
        { TEXT("B\u0130M"), TEXT("indirim marketi"), 0.93f, 0.80f, 1.00f, 1, 50000000, 0 },
        { TEXT("Migros"), TEXT("s\u00fcpermarket"), 1.03f, 1.10f, 0.80f, 1, 50000000, 1 },
        { TEXT("A101"), TEXT("indirim marketi"), 0.94f, 0.85f, 1.00f, MarketRivals::ChainOpensDay, 50000000, 2 },
        { TEXT("\u015eok"), TEXT("indirim marketi"), 0.95f, 0.85f, 0.95f, 131, 50000000, INDEX_NONE }, // 15 July 2011
    };
    return Profiles[FMath::Clamp(static_cast<int32>(Company), 0, static_cast<int32>(ECompany::Count) - 1)];
}

void MarketCompetitors::Ensure(FMarketState& State)
{
    for (int32 C = 0; C < static_cast<int32>(ECompany::Count); ++C)
    {
        if (Find(State, static_cast<ECompany>(C))) continue;
        const FProfile& P = Profile(static_cast<ECompany>(C));
        FMarketCompetitor New;
        New.Company = static_cast<uint8>(C);
        New.Cash = P.StartCash;
        New.BaseIndex = P.BaseIndex;
        New.Service = P.Service;
        New.Stores = 1;
        State.Competitors.Add(New);
    }
}

const FMarketCompetitor* MarketCompetitors::Find(const FMarketState& State, ECompany Company)
{
    return State.Competitors.FindByPredicate([Company](const FMarketCompetitor& C) { return C.Company == static_cast<uint8>(Company); });
}

bool MarketCompetitors::IsOpen(const FMarketState& State, ECompany Company)
{
    return State.Day >= Profile(Company).OpenDay;
}

float MarketCompetitors::PriceIndex(const FMarketState& State, ECompany Company, const FString& Category, const TArray<FString>& Aisles)
{
    const FMarketCompetitor* C = Find(State, Company);
    float Index = (C ? C->BaseIndex : Profile(Company).BaseIndex) * OpeningPush(State, Company);
    if (C && !C->WarCategory.IsEmpty() && State.Day <= C->WarUntil && MarketGoods::Fold(C->WarCategory) == MarketGoods::Fold(Category))
        Index *= 1.f - WarPercent / 100.f;
    const int32 News = Profile(Company).NewsRival;
    if (News != INDEX_NONE && News < MarketRivals::RivalCount(State.Day))
    {
        bool bEmpty = false;
        const float Factor = MarketRivals::RivalFactor(State.Day, State.RivalSeed, Aisles, Category, News, &bEmpty);
        Index *= bEmpty ? 1.25f : Factor; // an empty shelf elsewhere sends the shopper to us
    }
    return FMath::Clamp(Index, 0.6f, 1.5f);
}

float MarketCompetitors::RivalPriceFactor(const FMarketState& State, const FString& Category, const TArray<FString>& Aisles)
{
    float Weighted = 0.f, Weights = 0.f;
    for (int32 C = 0; C < static_cast<int32>(ECompany::Count); ++C)
    {
        const ECompany Company = static_cast<ECompany>(C);
        if (!IsOpen(State, Company)) continue;
        const FMarketCompetitor* Found = Find(State, Company);
        // Before the first day close the shares are unknown: weigh by how attractive each rival is.
        const FProfile& P = Profile(Company);
        const float Weight = Found && Found->Share > 0.f ? Found->Share : Attraction(P.BaseIndex, P.Service, P.Proximity, 1);
        Weighted += Weight * PriceIndex(State, Company, Category, Aisles);
        Weights += Weight;
    }
    return Weights > 0.f ? FMath::Clamp(Weighted / Weights, 0.7f, 1.3f) : 1.f;
}

float MarketCompetitors::TrafficFactor(const FMarketState& State)
{
    return FMath::Clamp(FMath::Sqrt(FMath::Max(1.f, State.MarketShare) / 25.f), 0.6f, 1.5f);
}

float MarketCompetitors::OurAttraction(const FMarketState& State, const TArray<FMarketProduct>& Products)
{
    // Price level: what shoppers paid against the list, weighted by how much each product is asked for.
    double PriceSum = 0.0, WeightSum = 0.0;
    int32 Sold = 0, Empty = 0, Missing = 0;
    for (int32 I = 0; I < State.Stock.Num() && I < Products.Num(); ++I)
    {
        const FMarketStock& Item = State.Stock[I];
        const FMarketDemandStats& Y = Item.Yesterday;
        Sold += Y.Sold; Empty += Y.Empty; Missing += Y.NotCarried;
        if (Item.Capacity <= 0 || Products[I].BasePrice <= 0) continue;
        const double Weight = 1.0 + Y.Sold + Y.Empty + Y.Expensive;
        PriceSum += Weight * static_cast<double>(MarketPromotions::UnitPrice(State, Products, I, 1)) / Products[I].BasePrice;
        WeightSum += Weight;
    }
    const float OurIndex = WeightSum > 0.0 ? static_cast<float>(PriceSum / WeightSum) : 1.2f;
    const int32 Asked = Sold + Empty + Missing / 2;
    const float Availability = Asked > 0 ? FMath::Clamp(static_cast<float>(Sold) / Asked, 0.2f, 1.f) : (WeightSum > 0.0 ? 0.8f : 0.3f);
    const int32 Visitors = State.LastServed + State.LastLost;
    // Service: nobody gave up waiting, and most visitors left with what they came for.
    const float Served = Visitors > 0 ? static_cast<float>(State.LastServed) / Visitors : 0.9f;
    const float Service = (Visitors > 0 ? FMath::Clamp(1.f - 0.8f * State.LastLostWaiting / Visitors, 0.3f, 1.f) : 1.f) * (0.55f + 0.5f * Served);
    float Satisfaction = 50.f;
    if (State.Loyalty.Num() > 0)
    {
        Satisfaction = 0.f;
        for (const FMarketLoyalty& L : State.Loyalty) Satisfaction += L.Satisfaction;
        Satisfaction /= State.Loyalty.Num();
    }
    const float Loyalty = 0.8f + Satisfaction / 250.f;
    const float Promotions = FMath::Min(1.25f, 1.f + 0.08f * MarketPromotions::Active(State).Num()) * (MarketPromotions::TrafficFactor(State) > 1.f ? 1.1f : 1.f);
    return Attraction(OurIndex, Service, HomeAdvantage, 1) * FMath::Pow(Availability, 1.2f) * Loyalty * Promotions;
}

float MarketCompetitors::TargetShare(const FMarketState& State, const TArray<FMarketProduct>& Products, const TArray<FString>& Aisles)
{
    const float Ours = OurAttraction(State, Products);
    float Total = Ours;
    for (int32 C = 0; C < static_cast<int32>(ECompany::Count); ++C)
    {
        const ECompany Company = static_cast<ECompany>(C);
        if (!IsOpen(State, Company)) continue;
        const FMarketCompetitor* Found = Find(State, Company);
        const FProfile& P = Profile(Company);
        Total += Attraction(PriceIndex(State, Company, FString(), Aisles), Found ? Found->Service : P.Service, P.Proximity, Found ? Found->Stores : 1);
    }
    return Total > 0.f ? Ours / Total : 0.f;
}

void MarketCompetitors::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products, const TArray<FString>& Aisles)
{
    Ensure(State);
    const int32 Closed = State.Day - 1;
    if (Closed < 1) return;
    TArray<FString>& News = State.DayNews;

    // 1. Shares of the district.
    const float Ours = OurAttraction(State, Products);
    float Total = Ours;
    TArray<float> Attr;
    Attr.Init(0.f, static_cast<int32>(ECompany::Count));
    for (int32 C = 0; C < Attr.Num(); ++C)
    {
        const ECompany Company = static_cast<ECompany>(C);
        if (!IsOpen(State, Company)) continue;
        const FMarketCompetitor* Found = Find(State, Company);
        Attr[C] = Attraction(PriceIndex(State, Company, FString(), Aisles), Found->Service, Profile(Company).Proximity, Found->Stores);
        Total += Attr[C];
    }
    const float Target = Total > 0.f ? Ours / Total : 0.f;
    for (FMarketCompetitor& C : State.Competitors)
        C.Share = Total > 0.f && C.Company < Attr.Num() ? Attr[C.Company] / Total : 0.f;
    // Habits change slowly: the day's result moves the local share a little. This replaces the simple satisfaction
    // update of FMarketState::CloseDay (satisfaction is part of our attractiveness here). A day without visitors
    // (shop closed) keeps the share, as before.
    if (State.LastServed + State.LastLost > 0)
        State.MarketShare = FMath::Clamp(State.ShareBeforeClose * 0.85f + Target * 100.f * 0.15f, 5.f, 65.f);

    // 2. Bereket Market: pride, anger, price wars, money.
    if (FMarketCompetitor* B = FindMutable(State, ECompany::Bereket))
    {
        const float OurShare = State.MarketShare / 100.f;
        B->Anger = FMath::Clamp(B->Anger * 0.93f + FMath::Max(0.f, OurShare - B->Share) * 60.f, 0.f, 100.f);
        for (const FMarketPromotion* P : MarketPromotions::Active(State))
            if (P->Kind == static_cast<uint8>(MarketPromotions::EKind::AisleDiscount)) B->Anger = FMath::Min(100.f, B->Anger + 4.f);
        // His shop earns when it keeps its customers and bleeds when it loses them or fights.
        B->Cash += static_cast<int64>((B->Share - 0.15f) * 20000.f);
        const bool bAtWar = !B->WarCategory.IsEmpty() && State.Day <= B->WarUntil;
        if (bAtWar) B->Cash -= 3000;
        if (!bAtWar && !B->WarCategory.IsEmpty())
        {
            News.Add(FString::Printf(TEXT("Bereket Market %s reyonundaki indirimi bitirdi."), *B->WarCategory));
            B->WarCategory.Reset();
        }
        if (!bAtWar && B->Anger >= 40.f && B->Cash >= 20000 && State.Day >= B->WarCooldownUntil)
        {
            // He hits where we sell most.
            TMap<FString, int32> SoldBy;
            for (int32 I = 0; I < State.Stock.Num() && I < Products.Num(); ++I) SoldBy.FindOrAdd(Products[I].Category) += State.Stock[I].Yesterday.Sold;
            FString Best;
            int32 BestSold = -1;
            for (const TPair<FString, int32>& Pair : SoldBy) if (Pair.Value > BestSold || (Pair.Value == BestSold && Pair.Key < Best)) { Best = Pair.Key; BestSold = Pair.Value; }
            if (!Best.IsEmpty())
            {
                B->WarCategory = Best;
                B->WarUntil = State.Day + WarDays - 1;
                B->WarCooldownUntil = B->WarUntil + 10;
                B->Anger = FMath::Max(0.f, B->Anger - 30.f);
                News.Add(FString::Printf(TEXT("Bereket Market sana cevap verdi: %s reyonunda %%%d indirim (%d g\u00fcn). Kadir Bey mahallede \"yeni \u00e7ocuk fiyatlar\u0131 bozuyor\" diyor."),
                    *Best, WarPercent, WarDays));
            }
        }
        if (B->Cash < 0 && !(B->Told & ToldStruggling))
        {
            B->Told |= ToldStruggling;
            B->Service = FMath::Max(0.6f, B->Service - 0.15f);
            News.Add(TEXT("Mahallede konu\u015fuluyor: Bereket Market toptanc\u0131ya bor\u00e7lanm\u0131\u015f, raflar\u0131 seyreldi."));
        }
        if (B->Cash < 0 && !(B->Told & ToldRaised))
        {
            B->Told |= ToldRaised;
            B->BaseIndex = 1.05f;
            News.Add(TEXT("Bereket Market fiyatlar\u0131n\u0131 art\u0131rd\u0131; sava\u015fa dayanacak paras\u0131 kalmad\u0131."));
        }
    }

    // 3. Chains: openings and a second A101 when we are strong.
    for (int32 C = 1; C < static_cast<int32>(ECompany::Count); ++C)
    {
        const ECompany Company = static_cast<ECompany>(C);
        FMarketCompetitor* Chain = FindMutable(State, Company);
        if (!Chain) continue;
        if (Company == ECompany::Sok && State.Day == Profile(Company).OpenDay && !(Chain->Told & ToldOpened))
        {
            Chain->Told |= ToldOpened;
            News.Add(TEXT("\u015eok il\u00e7ede yeni bir ma\u011faza a\u00e7t\u0131. Bir indirim marketi daha: fiyat\u0131na dikkat."));
        }
        if (Company == ECompany::A101 && State.Day > Profile(Company).OpenDay + 45 && State.MarketShare >= 35.f && !(Chain->Told & ToldSecondStore))
        {
            Chain->Told |= ToldSecondStore;
            Chain->Stores = 2;
            News.Add(TEXT("A101 senin soka\u011f\u0131na yak\u0131n ikinci bir ma\u011faza a\u00e7t\u0131. Pay\u0131n b\u00fcy\u00fcd\u00fck\u00e7e zincirler de yakla\u015f\u0131yor."));
        }
    }

    // 4. Poaching: now and then a chain offers one of our good people a job.
    if (RivalMix(State.RivalSeed, Closed, 0x9A7Eu) % 40u == 0u)
    {
        FMarketEmployee* Target = nullptr;
        for (FMarketEmployee& E : State.Staff)
        {
            const MarketStaff::ERole Role = MarketStaff::RoleOf(E);
            if ((Role == MarketStaff::ERole::Cashier || Role == MarketStaff::ERole::Stocker) && E.Skill >= 65 && E.LeaveDay == 0 && (!Target || E.Skill > Target->Skill)) Target = &E;
        }
        if (Target)
        {
            const TCHAR* Chain = RivalMix(State.RivalSeed, Closed, 0x9A7Fu) % 2u == 0u ? TEXT("A101") : TEXT("B\u0130M");
            if (Target->Morale < 45.f) // below the level where MarketStaff lets a notice be withdrawn
            {
                Target->LeaveDay = State.Day + 1;
                News.Add(FString::Printf(TEXT("%s, %s'e daha iyi \u00fccret teklif etti. %s ayr\u0131lmak istiyor (%d. g\u00fcn\u00fcn sonunda). Zam yaparsan kalabilir."),
                    Chain, *Target->Name, *Target->Name, Target->LeaveDay));
            }
            else News.Add(FString::Printf(TEXT("%s, %s'e i\u015f teklif etti; %s burada mutlu, reddetti."), Chain, *Target->Name, *Target->Name));
        }
    }
}

FString MarketCompetitors::Describe(const FMarketState& State, ECompany Company)
{
    const FProfile& P = Profile(Company);
    if (!IsOpen(State, Company)) return FString::Printf(TEXT("%s \u00b7 hen\u00fcz il\u00e7ede yok"), P.Name);
    const FMarketCompetitor* C = Find(State, Company);
    FString Text = FString::Printf(TEXT("%s \u00b7 %s \u00b7 pay %%%.0f \u00b7 fiyat d\u00fczeyi %%%.0f"), P.Name, P.Format, C ? C->Share * 100.f : 0.f,
        (C ? C->BaseIndex : P.BaseIndex) * OpeningPush(State, Company) * 100.f);
    if (C && Company == ECompany::Bereket)
    {
        if (!C->WarCategory.IsEmpty() && State.Day <= C->WarUntil) Text += FString::Printf(TEXT(" \u00b7 %s reyonunda fiyat sava\u015f\u0131nda"), *C->WarCategory);
        else if (C->Anger >= 40.f) Text += TEXT(" \u00b7 sana k\u0131zg\u0131n");
        if (C->Cash < 0) Text += TEXT(" \u00b7 zor durumda");
    }
    return Text;
}

float MarketCompetitors::NewsRivalIndex(const FMarketState& State, int32 NewsRival)
{
    for (int32 C = 0; C < static_cast<int32>(ECompany::Count); ++C)
        if (Profile(static_cast<ECompany>(C)).NewsRival == NewsRival)
        {
            const FMarketCompetitor* Found = Find(State, static_cast<ECompany>(C));
            return (Found ? Found->BaseIndex : Profile(static_cast<ECompany>(C)).BaseIndex) * OpeningPush(State, static_cast<ECompany>(C));
        }
    return 1.f;
}

#include "MarketCompetitors.h"
#include "MarketStart.h"
#include "MarketChains.h"
#include "MarketCast.h"
#include "MarketCountry.h"
#include "MarketCalendar.h"
#include "MarketGoods.h"
#include "MarketPromotions.h"
#include "MarketRivals.h"
#include "MarketStaff.h"
#include "MarketEvents.h"
#include "MarketPrices.h"
#include "MarketStory.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace MarketCompetitors
{
    enum ETold : int32 { ToldStruggling = 1, ToldRaised = 2, ToldSecondStore = 4, ToldOpened = 8, ToldForSale = 16, ToldSold = 32, ToldCut = 64 };

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

    // G-077 (#14): a rival's price level for the whole store is the average of its aisle prices, so aisle price
    // wars, aisle sales, rises and empty shelves (MarketRivals news) reach the share model. Before, the share model
    // asked with an empty aisle name and only store-wide effects counted.
    float StoreIndex(const FMarketState& State, ECompany Company, const TArray<FString>& Aisles)
    {
        if (Aisles.Num() == 0) return PriceIndex(State, Company, FString(), Aisles);
        float Sum = 0.f;
        int32 Count = 0;
        for (const FString& Aisle : Aisles)
            if (Sells(Company, Aisle)) { Sum += PriceIndex(State, Company, Aisle, Aisles); ++Count; }
        return Count > 0 ? Sum / Count : 1.5f; // sells none of our aisles: no pull on price
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
        { TEXT("Mahalle marketi"), TEXT("mahalle marketi"), 1.00f, 0.90f, 1.30f, 1, 150000, INDEX_NONE },
        // C3 (L12, A5): the same fictional names as the national roster (MarketChains).
        { TEXT("B\u0130N"), TEXT("indirim marketi"), 0.93f, 0.80f, 1.00f, 1, 50000000, 0 },
        { TEXT("Migron"), TEXT("s\u00fcpermarket"), 1.03f, 1.10f, 0.80f, 1, 50000000, 1 },
        { TEXT("A110"), TEXT("indirim marketi"), 0.94f, 0.85f, 1.00f, MarketRivals::ChainOpensDay, 50000000, 2 },
        { TEXT("\u015eAK"), TEXT("indirim marketi"), 0.95f, 0.85f, 0.95f, 131, 50000000, INDEX_NONE }, // 15 July 2011
        // G-079: four corner grocers together; a little dearer, very close, credit and cigarettes, open late.
        { TEXT("Mahalle bakkallar\u0131"), TEXT("bakkal ve tekel"), 1.10f, 1.00f, 1.25f, 1, 3000000, INDEX_NONE, 4 },
        // G-079: the Tuesday street market: cheap and fresh, only dairy (and produce later), only on its day.
        { TEXT("Sal\u0131 Pazar\u0131"), TEXT("semt pazar\u0131"), 0.82f, 0.90f, 1.00f, 1, 0, INDEX_NONE, 1, 1, true },
    };
    return Profiles[FMath::Clamp(static_cast<int32>(Company), 0, static_cast<int32>(ECompany::Count) - 1)];
}

namespace MarketCompetitors
{
    const TCHAR* ChainIdOf(ECompany Company)
    {
        switch (Company)
        {
        case ECompany::Bim: return TEXT("bim");
        case ECompany::Migros: return TEXT("migros");
        case ECompany::A101: return TEXT("a101");
        case ECompany::Sok: return TEXT("sok");
        default: return TEXT("");
        }
    }
}

namespace MarketCompetitorsLocal
{
    // M35: the names bound for the campaign that is running (Activate), like the active country.
    TArray<FString>& StreetNames()
    {
        static TArray<FString> Names;
        if (Names.Num() != static_cast<int32>(MarketCompetitors::ECompany::Count)) Names.Init(FString(), static_cast<int32>(MarketCompetitors::ECompany::Count));
        return Names;
    }

    // Kinds of chain a street slot takes, in order of preference.
    TArray<MarketChains::EArchetype> Wanted(MarketCompetitors::ECompany Company)
    {
        using MarketChains::EArchetype;
        switch (Company)
        {
        case MarketCompetitors::ECompany::Bim: return { EArchetype::Discount, EArchetype::FastDiscount, EArchetype::Regional };
        case MarketCompetitors::ECompany::A101: return { EArchetype::FastDiscount, EArchetype::Discount, EArchetype::Regional };
        case MarketCompetitors::ECompany::Sok: return { EArchetype::Discount, EArchetype::Regional, EArchetype::FastDiscount, EArchetype::Family };
        case MarketCompetitors::ECompany::Migros: return { EArchetype::Super, EArchetype::Premium, EArchetype::Hyper, EArchetype::Regional };
        default: return {};
        }
    }
}

void MarketCompetitors::Activate(const FMarketState& State)
{
    TArray<FString>& Names = MarketCompetitorsLocal::StreetNames();
    for (FString& Name : Names) Name.Reset();
    for (const FMarketCompetitor& C : State.Competitors)
        if (Names.IsValidIndex(C.Company) && !C.Name.IsEmpty()) Names[C.Company] = C.Name;
}

void MarketCompetitors::Bind(FMarketState& State)
{
    const FString Home = MarketStart::HomeProvince(State);
    const ECompany Slots[4] = { ECompany::Bim, ECompany::A101, ECompany::Sok, ECompany::Migros };
    TArray<FString> Taken;
    for (const FMarketCompetitor& C : State.Competitors) if (!C.ChainId.IsEmpty() && C.ChainId != TEXT("#gone")) Taken.Add(C.ChainId);
    for (const ECompany Slot : Slots)
    {
        FMarketCompetitor* C = FindMutable(State, Slot);
        if (!C || C->ChainId == TEXT("#gone")) continue;
        if (!C->ChainId.IsEmpty())
        {
            const FMarketChain* Chain = State.Rivals.Chains.FindByPredicate([C, &State](const FMarketChain& X) { return X.Id == C->ChainId && X.Country == State.CountryId; });
            if (Chain && !Chain->bGone && !Chain->bOurs) { C->Name = Chain->Name; continue; }
            if (Chain)
            {
                // The chain left: its shop on our street closes.
                if (IsOpen(State, Slot)) State.DayNews.Add(FString::Printf(TEXT("%s sokakta\u015f\u0131 ma\u011fazas\u0131n\u0131 kapatt\u0131 (zincir %s). M\u00fc\u015fterilerinin bir k\u0131sm\u0131 bize gelecek."),
                    *C->Name, Chain->bOurs ? TEXT("art\u0131k bizim") : TEXT("piyasadan \u00e7ekildi")));
                C->ChainId = TEXT("#gone");
                C->Stores = 0;
                continue;
            }
            C->ChainId.Reset();   // a chain of another campaign: bind again
        }
        // The best fitting chain of the country: its kind, a shop in the home province, its size.
        const TArray<MarketChains::EArchetype> Kinds = MarketCompetitorsLocal::Wanted(Slot);
        const FMarketChain* Best = nullptr;
        int32 BestRank = MAX_int32;
        for (const FMarketChain& X : State.Rivals.Chains)
        {
            if (X.bGone || X.bOurs || X.Country != State.CountryId || Taken.Contains(X.Id)) continue;
            const int32 Kind = Kinds.IndexOfByKey(static_cast<MarketChains::EArchetype>(X.Archetype));
            if (Kind == INDEX_NONE) continue;
            const bool bHere = X.Spots.ContainsByPredicate([&Home](const FMarketChainSpot& S) { return S.Province == Home && S.Stores > 0; });
            const int32 Rank = (bHere ? 0 : 100000000) + Kind * 1000000 - FMath::Min(999999, MarketChains::TotalStores(X));
            if (Rank < BestRank) { BestRank = Rank; Best = &X; }
        }
        if (!Best) continue;
        C->ChainId = Best->Id;
        C->Name = Best->Name;
        Taken.Add(Best->Id);
    }
    Activate(State);
}

FString MarketCompetitors::DisplayName(ECompany Company)
{
    if (Company == ECompany::Bereket) return MarketCast::RivalShop(); // M30: the family market next door, named from the country
    const TArray<FString>& Bound = MarketCompetitorsLocal::StreetNames();
    if (Bound.IsValidIndex(static_cast<int32>(Company)) && !Bound[static_cast<int32>(Company)].IsEmpty()) return Bound[static_cast<int32>(Company)]; // M35
    // Config/zincirler.json is read once: {"useFictional": bool, "chains":[{"id","real","fictional"}]}.
    static TMap<FString, FString> Fictional;
    static bool bLoaded = false, bUseFictional = false;
    if (!bLoaded)
    {
        bLoaded = true;
        FString Json;
        if (FFileHelper::LoadFileToString(Json, *FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("zincirler.json"))))
        {
            TSharedPtr<FJsonObject> Root;
            const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
            if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
            {
                Root->TryGetBoolField(TEXT("useFictional"), bUseFictional);
                const TArray<TSharedPtr<FJsonValue>>* Chains = nullptr;
                if (Root->TryGetArrayField(TEXT("chains"), Chains))
                    for (const TSharedPtr<FJsonValue>& Value : *Chains)
                    {
                        const TSharedPtr<FJsonObject> Chain = Value.IsValid() ? Value->AsObject() : nullptr;
                        FString Id, Name;
                        if (Chain.IsValid() && Chain->TryGetStringField(TEXT("id"), Id) && Chain->TryGetStringField(TEXT("fictional"), Name)) Fictional.Add(Id, Name);
                    }
            }
        }
    }
    const FString Id = ChainIdOf(Company);
    // G-084, D3: the country pack names its own chains and traditional trade; a pack with realChainNames (Turkey,
    // karar A12) shows the real names until the brand switch asks for fictional ones.
    const bool bReal = MarketCountry::Active().bRealChainNames;
    if (!Id.IsEmpty() && (bUseFictional || !bReal))
    {
        const FString Local = MarketCountry::ChainName(Id);
        if (!Local.IsEmpty()) return Local;
    }
    if (Company == ECompany::Bakkal && !MarketCountry::Active().GrocerName.IsEmpty()) return MarketCountry::Active().GrocerName;
    if (Company == ECompany::Pazar && !MarketCountry::Active().MarketName.IsEmpty()) return MarketCountry::Active().MarketName;
    if (bUseFictional && !Id.IsEmpty())
        if (const FString* Name = Fictional.Find(Id)) return *Name;
    return Profile(Company).Name;
}

bool MarketCompetitors::Sells(ECompany Company, const FString& Category)
{
    if (!Profile(Company).bFreshOnly) return true;
    return !Category.IsEmpty() && MarketGoods::Classify(Category) == MarketGoods::EGroup::Dairy;
}

bool MarketCompetitors::IsOpenOn(const FMarketState& State, ECompany Company, int32 GameDay)
{
    const FProfile& P = Profile(Company);
    if (GameDay < P.OpenDay) return false;
    if (const FMarketCompetitor* C = Find(State, Company); C && ((C->Told & ToldSold) || C->ChainId == TEXT("#gone"))) return false; // bought and closed; M35: its chain left
    const int32 Weekday = Company == ECompany::Pazar ? MarketCountry::Active().MarketWeekday : P.Weekday; // G-084: the country's market day
    return Weekday < 0 || MarketCalendar::DateOf(GameDay).Weekday == Weekday;
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
        New.Stores = FMath::Max(1, P.StartStores);
        State.Competitors.Add(New);
    }
    Bind(State); // M35
}

const FMarketCompetitor* MarketCompetitors::Find(const FMarketState& State, ECompany Company)
{
    return State.Competitors.FindByPredicate([Company](const FMarketCompetitor& C) { return C.Company == static_cast<uint8>(Company); });
}

bool MarketCompetitors::IsOpen(const FMarketState& State, ECompany Company)
{
    return IsOpenOn(State, Company, State.Day);
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
        if (!IsOpen(State, Company) || !Sells(Company, Category)) continue;
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
    // #19: promotions pull by how deep and how wide they are, not by how many (ten tiny ones = one small one).
    float Pull = 0.f;
    for (const FMarketPromotion* Promo : MarketPromotions::Active(State))
    {
        const MarketPromotions::EMechanic Mechanic = static_cast<MarketPromotions::EMechanic>(Promo->Mechanic);
        const float Depth = Mechanic == MarketPromotions::EMechanic::ThreeForTwo ? 0.33f : Mechanic == MarketPromotions::EMechanic::TwoForOne ? 0.5f
            : Mechanic == MarketPromotions::EMechanic::SecondHalf ? 0.25f : FMath::Clamp(Promo->Percent / 100.f, 0.f, 0.5f);
        const bool bScoped = Promo->Kind == static_cast<uint8>(MarketPromotions::EKind::Scoped);
        const MarketPromotions::EScope Scope = static_cast<MarketPromotions::EScope>(Promo->Scope);
        const float Width = !bScoped ? 0.3f : Scope == MarketPromotions::EScope::Store ? 1.f : Scope == MarketPromotions::EScope::Category ? 0.6f
            : Scope == MarketPromotions::EScope::Subcategory ? 0.4f : Scope == MarketPromotions::EScope::Brand ? 0.3f : 0.15f;
        Pull += Depth * Width;
    }
    const float Promotions = (1.f + 0.25f * (1.f - FMath::Exp(-2.5f * Pull))) * (MarketPromotions::TrafficFactor(State) > 1.f ? 1.1f : 1.f);
    return Attraction(OurIndex, Service, HomeAdvantage, 1) * FMath::Pow(Availability, 1.2f) * Loyalty * Promotions;
}

float MarketCompetitors::TargetShare(const FMarketState& State, const TArray<FMarketProduct>& Products, const TArray<FString>& Aisles)
{
    const float Ours = OurAttraction(State, Products);
    const float Crowd = MarketCountry::CityCompetition(State.CountryId, State.CityId); // G-084: a crowded city pulls harder
    float Total = Ours;
    for (int32 C = 0; C < static_cast<int32>(ECompany::Count); ++C)
    {
        const ECompany Company = static_cast<ECompany>(C);
        if (!IsOpen(State, Company)) continue;
        const FMarketCompetitor* Found = Find(State, Company);
        const FProfile& P = Profile(Company);
        Total += Crowd * Attraction(StoreIndex(State, Company, Aisles), Found ? Found->Service : P.Service, P.Proximity, Found ? Found->Stores : 1);
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
    const float Crowd = MarketCountry::CityCompetition(State.CountryId, State.CityId); // G-084
    float Total = Ours;
    TArray<float> Attr;
    Attr.Init(0.f, static_cast<int32>(ECompany::Count));
    for (int32 C = 0; C < Attr.Num(); ++C)
    {
        const ECompany Company = static_cast<ECompany>(C);
        if (!IsOpenOn(State, Company, Closed)) continue; // G-079 (#9): the day that was played, not tomorrow
        const FMarketCompetitor* Found = Find(State, Company);
        Attr[C] = Crowd * Attraction(StoreIndex(State, Company, Aisles), Found->Service, Profile(Company).Proximity, Found->Stores);
        Total += Attr[C];
    }
    const float Target = Total > 0.f ? Ours / Total : 0.f;
    // Habits change slowly: the day's result moves the local share a little. This replaces the simple satisfaction
    // update of FMarketState::CloseDay (satisfaction is part of our attractiveness here). A day without visitors
    // (shop closed) keeps the share, as before.
    if (State.LastServed + State.LastLost > 0)
        State.MarketShare = FMath::Clamp(State.ShareBeforeClose * 0.85f + Target * 100.f * 0.15f, 5.f, 65.f);
    // G-077 (#15): the rivals split what we do not hold, in proportion to their attractiveness, so every share the
    // menu shows adds up to 100 %.
    const float RivalTotal = Total - Ours;
    const float RivalPart = 1.f - State.MarketShare / 100.f;
    for (FMarketCompetitor& C : State.Competitors)
        C.Share = RivalTotal > 0.f && C.Company < Attr.Num() ? RivalPart * Attr[C.Company] / RivalTotal : 0.f;

    // 2. The neighbour market (internal id Bereket): pride, anger, price wars, money.
    if (FMarketCompetitor* B = FindMutable(State, ECompany::Bereket); B && !(B->Told & ToldSold))
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
            News.Add(FString::Printf(TEXT("%s %s reyonundaki indirimi bitirdi."), *MarketCast::RivalShop(), *B->WarCategory));
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
                News.Add(FString::Printf(TEXT("%s sana cevap verdi: %s reyonunda %%%d indirim (%d g\u00fcn). %s mahallede \"yeni \u00e7ocuk fiyatlar\u0131 bozuyor\" diyor."),
                    *MarketCast::RivalShop(), *Best, WarPercent, WarDays, *MarketCast::RivalOwner()));
            }
        }
        if (B->Cash < 0 && !(B->Told & ToldStruggling))
        {
            B->Told |= ToldStruggling;
            B->Service = FMath::Max(0.6f, B->Service - 0.15f);
            News.Add(FString::Printf(TEXT("Mahallede konu\u015fuluyor: %s toptanc\u0131ya bor\u00e7lanm\u0131\u015f, raflar\u0131 seyreldi."), *MarketCast::RivalShop()));
        }
        if (B->Cash < 0 && !(B->Told & ToldRaised))
        {
            B->Told |= ToldRaised;
            B->BaseIndex = 1.05f;
            News.Add(FString::Printf(TEXT("%s fiyatlar\u0131n\u0131 art\u0131rd\u0131; sava\u015fa dayanacak paras\u0131 kalmad\u0131."), *MarketCast::RivalShop()));
        }
        // G-079 (karar E03): a month without money and its owner puts the shop up for sale. Recovering resets it.
        B->RedDays = B->Cash < 0 ? B->RedDays + 1 : 0;
        if (B->Cash >= 0 && (B->Told & ToldRaised) && B->Share > 0.2f) { B->BaseIndex = 1.f; B->Told &= ~ToldRaised; B->Service = FMath::Min(0.9f, B->Service + 0.05f); }
        if (B->RedDays >= SaleAfterRedDays && !(B->Told & ToldForSale) && !MarketStory::StoryClosed(State))
        {
            B->Told |= ToldForSale;
            const int64 Price = MarketPrices::Scaled(BereketPrice2011, Closed);
            FMarketDecision D;
            D.Id = TEXT("rival.bereket");
            D.Title = FString::Printf(TEXT("%s sat\u0131l\u0131k"), *MarketCast::RivalShop());
            D.Text = FString::Printf(TEXT("%s geldi: \"Yoruldum. D\u00fckk\u00e2n\u0131, mal\u0131yla raf\u0131yla sana verelim, %s.\" Al\u0131rsan %s kapan\u0131r, m\u00fc\u015fterileri sokakta sana kal\u0131r."),
                *MarketCast::RivalOwner(), *MarketCountry::Money(Price / 100 * 100), *MarketCast::RivalShop());
            D.Options = { FString::Printf(TEXT("Sat\u0131n al (%s)"), *MarketCountry::Money(Price)), FString(TEXT("Almayaca\u011f\u0131m")) };
            D.DefaultOption = 1;
            D.Deadline = State.Day + 6;
            D.Arg = static_cast<int32>(FMath::Min<int64>(Price, MAX_int32));
            MarketEvents::Offer(State, D);
        }
    }

    // 3. Chains: openings, a second A101 when we are strong, and (G-079) their answer to a rising shop. They watch
    // a slow average of our share; when we climb well above it the discount chains cut prices a little and the
    // supermarket improves service; when we fall back they drift home. Harder difficulty: they notice sooner and cut
    // deeper. Over the years they open more shops around us.
    const float Threshold = State.Difficulty == 0 ? 5.f : State.Difficulty == 2 ? 2.f : 3.f;
    const float Step = State.Difficulty == 0 ? 0.005f : State.Difficulty == 2 ? 0.015f : 0.01f;
    const int32 Year = MarketCalendar::DateOf(Closed).Year;
    for (int32 C = 1; C < static_cast<int32>(ECompany::Count); ++C)
    {
        const ECompany Company = static_cast<ECompany>(C);
        FMarketCompetitor* Chain = FindMutable(State, Company);
        if (!Chain) continue;
        const bool bChain = Company == ECompany::Bim || Company == ECompany::Migros || Company == ECompany::A101 || Company == ECompany::Sok;
        if (bChain && IsOpenOn(State, Company, Closed))
        {
            if (Chain->WatchShare <= 0.f) Chain->WatchShare = State.MarketShare;
            Chain->WatchShare = Chain->WatchShare * 0.96f + State.MarketShare * 0.04f;
            if (Closed % 7 == 0)
            {
                const FProfile& P = Profile(Company);
                const float Lead = State.MarketShare - Chain->WatchShare;
                if (Lead > Threshold)
                {
                    if (Company == ECompany::Migros) Chain->Service = FMath::Min(P.Service + 0.1f, Chain->Service + 0.02f);
                    else if (Chain->BaseIndex > P.BaseIndex - 0.04f)
                    {
                        Chain->BaseIndex = FMath::Max(P.BaseIndex - 0.04f, Chain->BaseIndex - Step);
                        if (!(Chain->Told & ToldCut)) { Chain->Told |= ToldCut; News.Add(FString::Printf(TEXT("%s mahalledeki fiyatlar\u0131n\u0131 k\u0131rd\u0131: m\u00fc\u015fterilerinin sana kayd\u0131\u011f\u0131n\u0131 fark ettiler."), *DisplayName(Company))); }
                    }
                }
                else if (Lead < 0.f)
                {
                    Chain->BaseIndex = FMath::Min(P.BaseIndex, Chain->BaseIndex + Step * 0.5f);
                    Chain->Service = FMath::Max(P.Service, Chain->Service - 0.01f);
                    if (FMath::IsNearlyEqual(Chain->BaseIndex, P.BaseIndex)) Chain->Told &= ~ToldCut;
                }
            }
            // More shops around us over the years (discount chains), one step every few years.
            if (Company != ECompany::Migros)
            {
                const int32 Grown = FMath::Clamp(1 + (Year - 2011) / 4, 1, 3);
                if (Grown > Chain->Stores)
                {
                    Chain->Stores = Grown;
                    News.Add(FString::Printf(TEXT("%s il\u00e7eye bir ma\u011faza daha a\u00e7t\u0131 (sokaklar\u0131m\u0131za %d ma\u011faza)."), *DisplayName(Company), Grown));
                }
            }
        }
        if (Company == ECompany::Sok && State.Day == Profile(Company).OpenDay && !(Chain->Told & ToldOpened))
        {
            Chain->Told |= ToldOpened;
            News.Add(TEXT("\u015eok il\u00e7ede yeni bir ma\u011faza a\u00e7t\u0131. Bir indirim marketi daha: fiyat\u0131na dikkat."));
        }
        if (Company == ECompany::A101 && State.Day > Profile(Company).OpenDay + 45 && State.MarketShare >= 35.f && !(Chain->Told & ToldSecondStore))
        {
            Chain->Told |= ToldSecondStore;
            Chain->Stores = 2;
            News.Add(TEXT("A110 senin soka\u011f\u0131na yak\u0131n ikinci bir ma\u011faza a\u00e7t\u0131. Pay\u0131n b\u00fcy\u00fcd\u00fck\u00e7e zincirler de yakla\u015f\u0131yor."));
        }
    }

    // 4. Poaching: now and then a chain offers one of our good people a job.
    if (RivalMix(State.RivalSeed, Closed, 0x9A7Eu) % 40u == 0u)
    {
        FMarketEmployee* PoachingTarget = nullptr;
        for (FMarketEmployee& E : State.Staff)
        {
            const MarketStaff::ERole Role = MarketStaff::RoleOf(E);
            if ((Role == MarketStaff::ERole::Cashier || Role == MarketStaff::ERole::Stocker) && E.Skill >= 65 && E.LeaveDay == 0 && (!PoachingTarget || E.Skill > PoachingTarget->Skill)) PoachingTarget = &E;
        }
        if (PoachingTarget)
        {
            // G-079 (#22): only a chain that is in the district makes an offer.
            TArray<ECompany> Hiring;
            for (const ECompany Candidate : { ECompany::Bim, ECompany::A101, ECompany::Sok, ECompany::Migros })
                if (IsOpenOn(State, Candidate, Closed)) Hiring.Add(Candidate);
            const FString ChainName = Hiring.Num() > 0 ? DisplayName(Hiring[RivalMix(State.RivalSeed, Closed, 0x9A7Fu) % static_cast<uint32>(Hiring.Num())]) : DisplayName(ECompany::Bim);
            const TCHAR* Chain = *ChainName;
            if (PoachingTarget->Morale < 45.f) // below the level where MarketStaff lets a notice be withdrawn
            {
                PoachingTarget->LeaveDay = State.Day + 1;
                News.Add(FString::Printf(TEXT("%s, %s'e daha iyi \u00fccret teklif etti. %s ayr\u0131lmak istiyor (%d. g\u00fcn\u00fcn sonunda). Zam yaparsan kalabilir."),
                    Chain, *PoachingTarget->Name, *PoachingTarget->Name, PoachingTarget->LeaveDay));
            }
            else News.Add(FString::Printf(TEXT("%s, %s'e i\u015f teklif etti; %s burada mutlu, reddetti."), Chain, *PoachingTarget->Name, *PoachingTarget->Name));
        }
    }
}

FString MarketCompetitors::Describe(const FMarketState& State, ECompany Company)
{
    const FProfile& P = Profile(Company);
    const FString Name = DisplayName(Company);
    const FMarketCompetitor* Sold = Find(State, Company);
    if (Sold && (Sold->Told & ToldSold)) return FString::Printf(TEXT("%s \u00b7 kapand\u0131 (sen sat\u0131n ald\u0131n)"), *Name);
    if (P.Weekday >= 0 && State.Day >= P.OpenDay && !IsOpen(State, Company))
        return FString::Printf(TEXT("%s \u00b7 %s \u00b7 haftada bir kurulur, taze \u00fcr\u00fcnde ucuz"), *Name, P.Format);
    if (!IsOpen(State, Company)) return FString::Printf(TEXT("%s \u00b7 hen\u00fcz il\u00e7ede yok"), *Name);
    const FMarketCompetitor* C = Find(State, Company);
    FString Text = FString::Printf(TEXT("%s \u00b7 %s \u00b7 pay %%%.0f \u00b7 fiyat d\u00fczeyi %%%.0f"), *Name, P.Format, C ? C->Share * 100.f : 0.f,
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

bool MarketCompetitors::Resolve(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& D, int32 Option, FString& OutMessage)
{
    if (D.Id != TEXT("rival.bereket")) { OutMessage = TEXT("Bu karar art\u0131k ge\u00e7erli de\u011fil."); return true; }
    FMarketCompetitor* B = FindMutable(State, ECompany::Bereket);
    if (Option != 0 || !B)
    {
        OutMessage = FString::Printf(TEXT("D\u00fckk\u00e2n\u0131 almad\u0131n. %s ba\u015fka bir al\u0131c\u0131 arayacak."), *MarketCast::RivalOwner());
        if (B) { B->Told &= ~ToldForSale; B->RedDays = 0; B->Cash = 0; } // someone keeps it going for a while
        return true;
    }
    const int64 Price = FMath::Max<int64>(0, D.Arg);
    if (State.Cash < Price) { OutMessage = TEXT("Kasada bu kadar para yok. Banka kredisiyle ya da biraz bekleyerek tekrar d\u00fc\u015f\u00fcn."); return false; }
    State.Cash -= Price;
    MarketLedger::Post(State, MarketLedger::EAccount::Investment, -Price, true, MarketLedger::HeadOfficeStore); // C3 (B2): Bereket bought
    B->Told |= ToldSold;
    B->Share = 0.f;
    MarketStory::AddMemory(State, FString::Printf(TEXT("%s sat\u0131n al\u0131nd\u0131; sokakta tek bakkal kald\u0131n"), *MarketCast::RivalShop()));
    OutMessage = FString::Printf(TEXT("%s art\u0131k kapal\u0131. M\u00fc\u015fterileri yava\u015f yava\u015f sana ge\u00e7ecek; %s sana elini uzatt\u0131."), *MarketCast::RivalShop(), *MarketCast::RivalOwner());
    return true;
}

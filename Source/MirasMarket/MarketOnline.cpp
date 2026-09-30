#include "MarketOnline.h"
#include "MarketLedger.h"
#include "MarketPromotions.h"
#include "MarketCountry.h"
#include "MarketBasket.h"
#include "MarketCalendar.h"
#include "MarketCustomers.h"
#include "MarketPrices.h"
#include "MarketStaff.h"

namespace MarketOnline
{
    uint32 OnlineMix(int32 Seed, int32 Day, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), static_cast<uint32>(Day), Salt };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }

    FString OnlineTl(int64 Kurus)
    {
        return MarketCountry::Money(Kurus); // G-084: the active country\'s currency
    }

    int32 DateDay(int32 Year, int32 Month, int32 Day) { return MarketCalendar::GameDayOf(Year, Month, Day); }

    bool Between(int32 GameDay, int32 FromYear, int32 FromMonth, int32 FromDay, int32 ToYear, int32 ToMonth, int32 ToDay)
    {
        return GameDay >= DateDay(FromYear, FromMonth, FromDay) && GameDay < DateDay(ToYear, ToMonth, ToDay);
    }

    // The 2020-2021 period is different in every campaign (karar D11, Mustafa 29.09.2026): when it starts, how
    // long the panic lasts, which weekends are closed and how long the spring closure is come from the seed.
    uint32 Roll(const FMarketState& State, uint32 Salt, int32 Extra = 0) { return OnlineMix(State.RivalSeed, Extra, Salt); }
    int32 PanicEnd(const FMarketState& State) { return PandemicStart(State) + 1 + 8 + static_cast<int32>(Roll(State, 0x9A01u) % 7u); }
    int32 ClosureStart(const FMarketState& State) { return DateDay(2021, 4, 15) + static_cast<int32>(Roll(State, 0x9A02u) % 21u); }
    int32 ClosureEnd(const FMarketState& State) { return ClosureStart(State) + 10 + static_cast<int32>(Roll(State, 0x9A03u) % 11u); }

    bool IsPanic(const FMarketState& State, int32 GameDay)
    {
        return State.Online.bPandemic && GameDay > PandemicStart(State) && GameDay < PanicEnd(State);
    }

    float StoreFactorOn(const FMarketState& State, int32 GameDay)
    {
        float Factor = 1.f - DistrictOnlineShare(State, GameDay);
        if (IsCurfew(State, GameDay)) Factor *= 0.4f;             // open only a few hours
        else if (IsPanic(State, GameDay)) Factor *= 1.25f;        // everyone stocks up
        else if (IsPandemic(State, GameDay)) Factor *= 0.85f;     // people go out less
        return Factor;
    }

    float RepFactor(const FMarketState& State) { return 0.5f + State.Online.Reputation / 100.f; }

    float StarFactor(const FMarketState& State) { return FMath::Clamp((Stars(State) - 3.f) / 1.2f, 0.15f, 1.3f); }

    // Walk-in shoppers of the closed day -> all grocery trips of the district that day.
    float DistrictTrips(const FMarketState& State, int32 GameDay)
    {
        const float Visitors = static_cast<float>(FMath::Max(40, State.LastServed + State.LastLost));
        // G-077 (#16): walk-ins grow with the square root of the share (MarketCompetitors::TrafficFactor), so the
        // district is walk-ins / sqrt(share / 25 %) / 25 %: its size no longer shrinks as our share grows.
        const float Reach = FMath::Clamp(FMath::Sqrt(FMath::Max(1.f, State.ShareBeforeClose) / 25.f), 0.6f, 1.5f);
        return Visitors / Reach / 0.25f / FMath::Max(0.2f, StoreFactorOn(State, GameDay));
    }

    float WebMaturity(const FMarketState& State, int32 GameDay)
    {
        return FMath::Clamp(static_cast<float>(GameDay - State.Online.WebOpenedDay + 1) / 60.f, 0.2f, 1.f);
    }

    float PhoneBias(MarketCustomers::ESegment Segment)
    {
        using MarketCustomers::ESegment;
        switch (Segment)
        {
        case ESegment::Retired: return 1.6f;
        case ESegment::Family: return 1.2f;
        case ESegment::Worker: return 0.8f;
        case ESegment::Student: return 0.5f;
        case ESegment::Trader: return 0.7f;
        default: return 0.f;
        }
    }

    float PhoneChance(const FMarketState& State, const FMarketLoyalty& L, int32 GameDay)
    {
        if (L.Visits < 3 || L.Satisfaction < 50.f) return 0.f;
        const MarketCustomers::ESegment Segment = MarketCustomers::SegmentOf(L.CustomerId, State.RivalSeed);
        float Chance = 0.05f * PhoneBias(Segment) * RepFactor(State) * (State.Online.bFreeDelivery ? 1.2f : 1.f);
        if (State.Online.bWeb) Chance *= 0.75f;                    // some of them order on the site now
        if (IsPandemic(State, GameDay)) Chance *= IsCurfew(State, GameDay) ? 3.f : 1.8f;
        return Chance;
    }

    float WebOrders(const FMarketState& State, int32 GameDay)
    {
        if (!State.Online.bWeb) return 0.f;
        return DistrictTrips(State, GameDay) * DistrictOnlineShare(State, GameDay) / 1.6f * 0.12f * RepFactor(State)
            * (State.Online.bFreeDelivery ? 1.25f : 1.f) * WebMaturity(State, GameDay)
            * (State.Company.bDarkStore ? 1.5f : 1.f);   // faster delivery from the dark store
    }

    float PlatformOrders(const FMarketState& State, int32 GameDay)
    {
        if (!State.Online.bPlatform) return 0.f;
        return DistrictTrips(State, GameDay) * DistrictOnlineShare(State, GameDay) / 1.6f * 0.25f * StarFactor(State);
    }

    int32 Count(float Expected, FRandomStream& Random)
    {
        const int32 Whole = FMath::FloorToInt(Expected);
        return Whole + (Random.FRand() < Expected - Whole ? 1 : 0);
    }

    struct FOrder
    {
        EChannel Channel = EChannel::Phone;
        int32 Customer = INDEX_NONE;
        int32 Lines = 3;
    };

    int32 Available(const FMarketState& State, int32 Index)
    {
        return State.Stock[Index].Warehouse + State.Stock[Index].Shelf;
    }

    void Take(FMarketState& State, int32 Index, int32 Units)
    {
        FMarketStock& S = State.Stock[Index];
        const int32 FromDepot = FMath::Min(S.Warehouse, Units);
        S.Warehouse -= FromDepot;
        S.Shelf -= FMath::Min(S.Shelf, Units - FromDepot);
    }

    int32 PickProduct(const TArray<float>& Weights, float Total, FRandomStream& Random)
    {
        float Roll = Random.FRand() * Total;
        for (int32 I = 0; I < Weights.Num(); ++I)
        {
            Roll -= Weights[I];
            if (Roll <= 0.f && Weights[I] > 0.f) return I;
        }
        for (int32 I = Weights.Num() - 1; I >= 0; --I) if (Weights[I] > 0.f) return I;
        return INDEX_NONE;
    }

    int32 FindSubstitute(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index, int32 Units)
    {
        for (int32 Step = 1; Step < Products.Num(); ++Step)
        {
            const int32 Other = (Index + Step) % Products.Num();
            if (Products[Other].Category == Products[Index].Category && Available(State, Other) >= Units) return Other;
        }
        return INDEX_NONE;
    }

    void News(FMarketState& State, int32 GameDay)
    {
        if (GameDay == OpenDay(EChannel::Web))
            State.DayNews.Add(TEXT("\u0130nternetten market al\u0131\u015fveri\u015fi yay\u0131l\u0131yor: art\u0131k kendi web sitemizi a\u00e7abiliriz (kurulum, bar\u0131nd\u0131rma, kurye)."));
        if (GameDay == OpenDay(EChannel::Platform))
            State.DayNews.Add(TEXT("H\u0131zl\u0131 teslimat platformu Getirsin il\u00e7eye geldi: %18 komisyonla sipari\u015fleri kendi kuryeleri ta\u015f\u0131yor. Yoksak, m\u00fc\u015fteri ba\u015fkas\u0131ndan s\u00f6yl\u00fcyor."));
        if (State.Online.bPandemic)
        {
            if (GameDay == PandemicStart(State))
                State.DayNews.Add(TEXT("Salg\u0131n d\u00f6nemi ba\u015flad\u0131: insanlar evde kal\u0131yor. Temel g\u0131da ve temizlik raflar\u0131 bo\u015fal\u0131yor, telefon ve internet sipari\u015fleri patl\u0131yor."));
            if (IsCurfew(State, GameDay + 1) && !IsCurfew(State, GameDay))
                State.DayNews.Add(TEXT("Yar\u0131n soka\u011fa \u00e7\u0131kma k\u0131s\u0131tlamas\u0131 var: market k\u0131sa saatlerle a\u00e7\u0131k, d\u00fckkana az ki\u015fi gelir; sipari\u015fler artar."));
            if (GameDay == PandemicEnd(State))
                State.DayNews.Add(TEXT("K\u0131s\u0131tlamalar kalkt\u0131. \u0130nternetten al\u0131\u015fveri\u015fe al\u0131\u015fan bir k\u0131s\u0131m m\u00fc\u015fteri geri d\u00f6nmeyecek."));
        }
        const MarketCalendar::FDate Date = MarketCalendar::DateOf(GameDay);
        if (Date.Day == 1 && !State.Online.bWeb && !State.Online.bPlatform && DistrictOnlineShare(State, GameDay) >= 0.01f)
            State.DayNews.Add(FString::Printf(TEXT("\u0130l\u00e7ede market al\u0131\u015fveri\u015finin %%%.1f'i internete kaym\u0131\u015f durumda; biz orada yokuz, bu m\u00fc\u015fteriler kap\u0131m\u0131za u\u011framaz."),
                DistrictOnlineShare(State, GameDay) * 100.f));
    }
}

FString MarketOnline::ChannelName(EChannel Channel)
{
    switch (Channel)
    {
    case EChannel::Phone: return TEXT("Telefon sipari\u015fi");
    case EChannel::Web: return TEXT("Web sitesi");
    case EChannel::Platform: return TEXT("Getirsin platformu");
    default: return TEXT("?");
    }
}

int32 MarketOnline::OpenDay(EChannel Channel)
{
    switch (Channel)
    {
    case EChannel::Web: return DateDay(2014, 1, 1);
    case EChannel::Platform: return DateDay(2016, 3, 1);
    default: return 1;
    }
}

bool MarketOnline::IsOn(const FMarketState& State, EChannel Channel)
{
    switch (Channel)
    {
    case EChannel::Phone: return State.Online.bPhone;
    case EChannel::Web: return State.Online.bWeb;
    case EChannel::Platform: return State.Online.bPlatform;
    default: return false;
    }
}

float MarketOnline::DistrictOnlineShare(const FMarketState& State, int32 GameDay)
{
    // Rough share of Turkish grocery shopping done online; the district follows the country.
    static const float Shares[] = { 0.005f, 0.008f, 0.012f, 0.016f, 0.021f, 0.027f, 0.032f, 0.038f, 0.045f, 0.05f, 0.055f };
    const int32 Year = MarketCalendar::DateOf(GameDay).Year;
    if (Year < 2014) return 0.f;
    const int32 Last = static_cast<int32>(UE_ARRAY_COUNT(Shares)) - 1;
    float Share = Year - 2014 <= Last ? Shares[Year - 2014] : FMath::Min(0.10f, Shares[Last] + 0.004f * (Year - 2014 - Last));
    if (State.Online.bPandemic)
    {
        const int32 Start = PandemicStart(State), End = PandemicEnd(State);
        if (GameDay >= Start && GameDay < Start + 110) Share *= 3.5f;
        else if (GameDay >= Start && GameDay < End) Share *= 2.5f;
        else if (GameDay >= End && GameDay < End + 550) Share *= 1.4f;   // some never come back
    }
    return FMath::Min(Share, 0.3f);
}

int32 MarketOnline::PandemicStart(const FMarketState& State)
{
    return DateDay(2020, 3, 1) + static_cast<int32>(Roll(State, 0x9A00u) % 21u);
}

int32 MarketOnline::PandemicEnd(const FMarketState& State)
{
    return DateDay(2021, 5, 20) + static_cast<int32>(Roll(State, 0x9A04u) % 50u);
}

bool MarketOnline::IsPandemic(const FMarketState& State, int32 GameDay)
{
    return State.Online.bPandemic && GameDay >= PandemicStart(State) && GameDay < PandemicEnd(State);
}

bool MarketOnline::IsCurfew(const FMarketState& State, int32 GameDay)
{
    if (!IsPandemic(State, GameDay)) return false;
    if (GameDay >= ClosureStart(State) && GameDay < ClosureEnd(State)) return true;   // the full closure in spring 2021
    if (!MarketCalendar::IsWeekend(GameDay)) return false;
    // Two waves of weekend curfews; about two weekends in three are closed, which ones differs per campaign.
    const int32 Start = PandemicStart(State);
    const bool bWave = (GameDay >= Start + 30 && GameDay < Start + 90) || (GameDay >= Start + 270 && GameDay < ClosureStart(State));
    const int32 Saturday = GameDay - (MarketCalendar::DateOf(GameDay).Weekday - 5);
    return bWave && Roll(State, 0x9A05u, Saturday) % 3u != 0u;
}

float MarketOnline::StoreTrafficFactor(const FMarketState& State)
{
    return StoreFactorOn(State, State.Day);
}

float MarketOnline::GroupFactor(const FMarketState& State, MarketGoods::EGroup Group)
{
    using MarketGoods::EGroup;
    if (IsPanic(State, State.Day))
        return Group == EGroup::Staples || Group == EGroup::Household || Group == EGroup::Paper || Group == EGroup::OilSauce ? 2.f : 1.f;
    if (IsPandemic(State, State.Day)) return Group == EGroup::Household ? 1.3f : 1.f;
    return 1.f;
}

float MarketOnline::Stars(const FMarketState& State)
{
    return FMath::Clamp(1.f + State.Online.Reputation / 25.f, 1.f, 5.f);
}

int32 MarketOnline::DeliveryCapacity(const FMarketState& State)
{
    // A dark store (MarketCompany) has its own delivery team.
    return (State.Online.Couriers > 0 ? State.Online.Couriers * OrdersPerCourier : OrdersWithoutCourier) + (State.Company.bDarkStore ? 60 : 0);
}

int32 MarketOnline::PickCapacity(const FMarketState& State)
{
    return 6 + FMath::Max(0, State.Stockers) * 18 + State.Online.Couriers * 4 + (State.Company.bDarkStore ? 80 : 0);
}

float MarketOnline::ExpectedOrders(const FMarketState& State, EChannel Channel)
{
    const int32 Day = State.Day;
    if (Channel == EChannel::Web) return WebOrders(State, Day);
    if (Channel == EChannel::Platform) return PlatformOrders(State, Day);
    if (!State.Online.bPhone) return 0.f;
    float Sum = 0.f;
    for (const FMarketLoyalty& L : State.Loyalty) Sum += PhoneChance(State, L, Day);
    return Sum;
}

bool MarketOnline::SetChannel(FMarketState& State, EChannel Channel, bool bOn, FString& OutMessage)
{
    FMarketOnline& O = State.Online;
    if (IsOn(State, Channel) == bOn) { OutMessage = ChannelName(Channel) + (bOn ? TEXT(" zaten a\u00e7\u0131k.") : TEXT(" zaten kapal\u0131.")); return false; }
    if (bOn && State.Day < OpenDay(Channel))
    {
        OutMessage = ChannelName(Channel) + (Channel == EChannel::Web ? TEXT(" i\u00e7in erken: mahallede internetten market al\u0131\u015fveri\u015fi hen\u00fcz yok (4. y\u0131l).")
                                                                   : TEXT(" hen\u00fcz il\u00e7ede yok (6. y\u0131l)."));
        return false;
    }
    if (!bOn)
    {
        if (Channel == EChannel::Phone) O.bPhone = false;
        if (Channel == EChannel::Web) O.bWeb = false;
        if (Channel == EChannel::Platform) O.bPlatform = false;
        OutMessage = ChannelName(Channel) + TEXT(" kapat\u0131ld\u0131.");
        return true;
    }
    if (Channel == EChannel::Phone)
    {
        O.bPhone = true;
        OutMessage = FString::Printf(TEXT("Telefonla sipari\u015f ba\u015flad\u0131: tan\u0131d\u0131k m\u00fc\u015fteriler aray\u0131p s\u00f6yler. Kurye yoksa g\u00fcnde %d sipari\u015fi kapan\u0131\u015ftan sonra sen g\u00f6t\u00fcr\u00fcrs\u00fcn."), OrdersWithoutCourier);
        return true;
    }
    if (Channel == EChannel::Web)
    {
        if (O.Couriers <= 0) { OutMessage = TEXT("Web sipari\u015fleri i\u00e7in \u00f6nce bir kurye gerekir."); return false; }
        const int64 Cost = FMath::RoundToInt64(WebSetupCost * MarketPrices::ListLevel(State.Day));
        if (State.Cash < Cost) { OutMessage = FString::Printf(TEXT("Site kurulumu %s; kasada yok."), *OnlineTl(Cost)); return false; }
        State.OtherCosts += Cost;             // paid with today's costs at the day close
        O.bWeb = true;
        O.WebOpenedDay = State.Day;
        OutMessage = FString::Printf(TEXT("Web sitesi a\u00e7\u0131ld\u0131 (%s kurulum). \u0130nsanlar\u0131n siteyi \u00f6\u011frenmesi iki ay s\u00fcrer; kartl\u0131 \u00f6demede %%1,8 komisyon."), *OnlineTl(Cost));
        return true;
    }
    const int64 Tablet = FMath::RoundToInt64(30000 * MarketPrices::ListLevel(State.Day));
    if (State.Cash < Tablet) { OutMessage = TEXT("Platform tableti i\u00e7in kasada para yok."); return false; }
    State.OtherCosts += Tablet;
    O.bPlatform = true;
    OutMessage = TEXT("Getirsin'e kat\u0131ld\u0131k: sipari\u015fleri onlar\u0131n kuryesi al\u0131r, %18 komisyon. Y\u0131ld\u0131z\u0131m\u0131z d\u00fc\u015ferse sipari\u015f azal\u0131r.");
    return true;
}

bool MarketOnline::HireCourier(FMarketState& State, FString& OutMessage)
{
    if (State.Online.Couriers >= MaxCouriers) { OutMessage = TEXT("Daha fazla kurye gerekmez."); return false; }
    ++State.Online.Couriers;
    OutMessage = FString::Printf(TEXT("Bisikletli kurye al\u0131nd\u0131 (%d kurye, g\u00fcnl\u00fck %s, g\u00fcnde %d teslimat)."), State.Online.Couriers,
        *OnlineTl(FMath::RoundToInt64(CourierDailyWage * MarketPrices::WageIndex(State.Day))), OrdersPerCourier);
    return true;
}

bool MarketOnline::FireCourier(FMarketState& State, FString& OutMessage)
{
    if (State.Online.Couriers <= 0) { OutMessage = TEXT("Kurye yok."); return false; }
    --State.Online.Couriers;
    OutMessage = FString::Printf(TEXT("Bir kurye ayr\u0131ld\u0131 (%d kald\u0131)."), State.Online.Couriers);
    return true;
}

bool MarketOnline::SetSubstitute(FMarketState& State, int32 Rule, FString& OutMessage)
{
    const uint8 Value = static_cast<uint8>(FMath::Clamp(Rule, 0, 2));
    if (State.Online.Substitute == Value) { OutMessage = TEXT("Eksik \u00fcr\u00fcn kural\u0131 zaten b\u00f6yle."); return false; }
    State.Online.Substitute = Value;
    static const TCHAR* Names[3] = { TEXT("m\u00fc\u015fteriyi aray\u0131p sor"), TEXT("ayn\u0131 reyondan benzerini koy"), TEXT("\u00e7\u0131kar, paray\u0131 alma") };
    OutMessage = FString::Printf(TEXT("Eksik \u00fcr\u00fcnde: %s."), Names[Value]);
    return true;
}

bool MarketOnline::SetFreeDelivery(FMarketState& State, bool bFree, FString& OutMessage)
{
    if (State.Online.bFreeDelivery == bFree) { OutMessage = TEXT("Teslimat \u00fccreti zaten b\u00f6yle."); return false; }
    State.Online.bFreeDelivery = bFree;
    OutMessage = bFree ? TEXT("Teslimat \u00fccretsiz: daha \u00e7ok sipari\u015f gelir, kurye masraf\u0131 bize kal\u0131r.")
                       : TEXT("K\u00fc\u00e7\u00fck sepetlere teslimat \u00fccreti: daha az sipari\u015f, masraf kar\u015f\u0131lan\u0131r.");
    return true;
}

FString MarketOnline::Summary(const FMarketState& State)
{
    const FMarketOnline& O = State.Online;
    TArray<FString> On;
    for (int32 C = 0; C < static_cast<int32>(EChannel::Count); ++C)
        if (IsOn(State, static_cast<EChannel>(C))) On.Add(ChannelName(static_cast<EChannel>(C)));
    FString Line = On.Num() ? FString::Join(On, TEXT(", ")) : FString(TEXT("Sipari\u015f kanal\u0131 yok"));
    Line += FString::Printf(TEXT("  \u00b7  kurye %d  \u00b7  itibar %.0f"), O.Couriers, O.Reputation);
    if (O.bPlatform) Line += FString::Printf(TEXT(" (%.1f y\u0131ld\u0131z)"), Stars(State));
    if (O.LastOrders > 0)
        Line += FString::Printf(TEXT("  \u00b7  d\u00fcn %d sipari\u015f, net %s"), O.LastOrders, *OnlineTl(O.LastProfit));
    const float Share = DistrictOnlineShare(State, State.Day);
    if (Share > 0.f) Line += FString::Printf(TEXT("  \u00b7  il\u00e7ede internet pay\u0131 %%%.1f"), Share * 100.f);
    return Line;
}

void MarketOnline::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    FMarketOnline& O = State.Online;
    const int32 Closed = State.Day - 1;
    News(State, Closed);
    O.LastOrders = O.LastLate = O.LastCancelled = O.LastMissing = O.LastSubstituted = 0;
    O.LastRevenue = O.LastCosts = O.LastProfit = 0;
    const double Level = MarketPrices::ListLevel(Closed);

    int64 Costs = FMath::RoundToInt64(CourierDailyWage * MarketPrices::WageIndex(Closed)) * O.Couriers;
    if (O.bWeb) Costs += FMath::RoundToInt64(WebMonthlyHosting * Level / 30.0);

    FRandomStream Random(static_cast<int32>(OnlineMix(State.RivalSeed, Closed, 0x0A11)));
    TArray<FOrder> Orders;
    if (O.bPhone)
        for (const FMarketLoyalty& L : State.Loyalty)
            if (Random.FRand() < PhoneChance(State, L, Closed))
            {
                FOrder Order; Order.Channel = EChannel::Phone; Order.Customer = L.CustomerId; Order.Lines = Random.RandRange(2, 5);
                Orders.Add(Order);
            }
    for (int32 N = Count(WebOrders(State, Closed), Random); N > 0; --N)
    {
        FOrder Order; Order.Channel = EChannel::Web; Order.Lines = Random.RandRange(4, 9);
        Orders.Add(Order);
    }
    for (int32 N = Count(PlatformOrders(State, Closed), Random); N > 0; --N)
    {
        FOrder Order; Order.Channel = EChannel::Platform; Order.Lines = Random.RandRange(2, 5);
        Orders.Add(Order);
    }

    const bool bStock = State.Stock.Num() == Products.Num() && Products.Num() > 0;
    TArray<float> Weights;
    float Total = 0.f;
    if (bStock)
        for (const FMarketProduct& P : Products)
        {
            // B1 (#30): only what the shop carries (a shelf plan block) is on the web list.
            const int32 Row = Weights.Num();
            const bool bCarried = State.Stock.IsValidIndex(Row) && State.Stock[Row].Capacity > 0;
            const float W = P.bActive && bCarried ? MarketCalendar::CategoryFactor(Closed, State.RivalSeed, P.Category) * GroupFactor(State, MarketGoods::Classify(P.Category)) : 0.f;
            Weights.Add(W);
            Total += W;
        }

    int64 Revenue = 0, Cogs = 0;
    int32 Units = 0, Picked = 0, Own = 0;
    // B1 (#30): online units by catalog row (a promotion's "before" counts the shop only).
    State.Ledger.OnlineSold.Init(0, State.Stock.Num());
    const int32 PickCap = PickCapacity(State), DeliverCap = DeliveryCapacity(State);
    for (const FOrder& Order : Orders)
    {
        const bool bOwn = Order.Channel != EChannel::Platform;
        bool bLate = Picked >= PickCap;
        if (bOwn && Own >= DeliverCap)
        {
            if (Random.FRand() < 0.4f) { ++O.LastCancelled; O.Reputation -= 2.5f; continue; }
            bLate = true;
        }
        ++Picked;
        if (bOwn) ++Own;
        int64 Value = 0;
        int32 Missing = 0, Delivered = 0;
        for (int32 Line = 0; Line < Order.Lines && bStock && Total > 0.f; ++Line)
        {
            int32 Index = PickProduct(Weights, Total, Random);
            if (Index == INDEX_NONE) break;
            const int32 Wanted = Random.RandRange(1, Order.Channel == EChannel::Web ? 3 : 2);
            if (Available(State, Index) < Wanted)
            {
                const int32 Other = O.Substitute == 2 ? INDEX_NONE : FindSubstitute(State, Products, Index, Wanted);
                const float Accept = O.Substitute == 0 ? 0.85f : 0.7f;
                if (Other != INDEX_NONE && Random.FRand() < Accept)
                {
                    Index = Other;
                    ++O.LastSubstituted;
                    O.Reputation -= O.Substitute == 0 ? 0.1f : 0.3f;
                }
                else
                {
                    ++Missing;
                    O.Reputation -= O.Substitute == 0 ? 0.5f : O.Substitute == 2 ? 1.f : 1.2f;
                    continue;
                }
            }
            Take(State, Index, Wanted);
            FMarketStock& Row = State.Stock[Index];
            Row.Yesterday.Sold += Wanted;
            ++Row.Yesterday.Buyers;
            Value += MarketPromotions::DealPrice(State, Products, Index, Wanted, Closed) * Wanted; // B1 (#30): the shelf deal
            State.Ledger.OnlineSold[Index] += Wanted;
            Cogs += State.UnitCost(Index, Products) * Wanted;
            Units += Wanted;
            ++Delivered;
        }
        O.LastMissing += Missing;
        if (!O.bFreeDelivery && bOwn && Value < FMath::RoundToInt64(3000 * Level)) Value += FMath::RoundToInt64(300 * Level);
        Revenue += Value;
        Costs += FMath::RoundToInt64(PackagingPerOrder * Level);
        if (Order.Channel == EChannel::Platform) Costs += FMath::RoundToInt64(Value * PlatformCommission);
        if (Order.Channel == EChannel::Web) Costs += FMath::RoundToInt64(Value * WebCardCommission);
        if (bLate) { ++O.LastLate; O.Reputation -= 1.5f; }
        else if (Missing == 0) O.Reputation += 0.25f;
        if (Order.Customer != INDEX_NONE)
            MarketBasket::RecordVisit(State, Order.Customer, Order.Lines, Delivered, bLate);
        ++O.LastOrders;
    }
    O.Reputation = FMath::Clamp(O.Reputation, 0.f, 100.f);

    // Picking tires the stocker on duty.
    if (Units > 0)
        for (const FMarketEmployee& E : State.Staff)
            if (E.Role == static_cast<uint8>(MarketStaff::ERole::Stocker) && E.OffDay != Closed)
            {
                MarketStaff::RecordWork(State, E.Id, Units / 2);
                break;
            }

    O.LastRevenue = Revenue;
    O.LastCosts = Costs;
    O.LastProfit = Revenue - Cogs - Costs;
    O.TotalOrders += O.LastOrders;
    O.WeekOrders += O.LastOrders;
    O.WeekProfit += O.LastProfit;
    State.LastRevenue += Revenue;
    State.LastCostOfGoods += Cogs;
    State.LastOperatingCost += Costs;
    State.LastProfit += O.LastProfit;
    State.Cash += Revenue - Costs;
    MarketLedger::Post(State, MarketLedger::EAccount::OnlineSales, Revenue); // B2
    MarketLedger::Post(State, MarketLedger::EAccount::CostOfGoods, -Cogs, false);
    MarketLedger::Post(State, MarketLedger::EAccount::OnlineCosts, -Costs);

    if (O.LastOrders > 0 || O.LastCancelled > 0)
    {
        FString Line = FString::Printf(TEXT("Sipari\u015fler: %d teslim (%d ge\u00e7)"), O.LastOrders, O.LastLate);
        if (O.LastCancelled) Line += FString::Printf(TEXT(", %d iptal \u2014 kurye yetmedi"), O.LastCancelled);
        if (O.LastMissing) Line += FString::Printf(TEXT(", %d eksik \u00fcr\u00fcn"), O.LastMissing);
        if (O.LastSubstituted) Line += FString::Printf(TEXT(", %d ikame"), O.LastSubstituted);
        Line += FString::Printf(TEXT("; ciro %s, net %s."), *OnlineTl(Revenue), *OnlineTl(O.LastProfit));
        if (O.bPlatform) Line += FString::Printf(TEXT(" Getirsin y\u0131ld\u0131z\u0131 %.1f."), Stars(State));
        State.DayNews.Add(Line);
    }
    else if (O.Couriers > 0 && !O.bPhone && !O.bWeb)
        State.DayNews.Add(TEXT("Kurye bo\u015fta bekledi: telefon ya da web sipari\u015fi a\u00e7\u0131k de\u011fil."));
    if (MarketCalendar::DateOf(Closed).Weekday == 6)
    {
        if (O.WeekOrders > 0)
            State.DayNews.Add(FString::Printf(TEXT("Haftan\u0131n sipari\u015fleri: %d, net %s."), O.WeekOrders, *OnlineTl(O.WeekProfit)));
        O.WeekOrders = 0;
        O.WeekProfit = 0;
    }
}

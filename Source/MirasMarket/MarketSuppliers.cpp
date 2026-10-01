#include "MarketSuppliers.h"
#include "MarketStart.h"
#include "MarketCountry.h"
#include "MarketCalendar.h"
#include "MarketPrices.h"
#include "MarketPromotions.h"
#include "MarketEvents.h"
#include "MarketGoods.h"

namespace MarketSuppliers
{
    FString SupplierTl(int64 Kurus)
    {
        return MarketCountry::Money(Kurus); // G-084: the active country\'s currency
    }
}

const MarketSuppliers::FInfo& MarketSuppliers::Info(ESupplier Supplier)
{
    static const FInfo Trakya = { TEXT("Trakya G\u0131da Da\u011f\u0131t\u0131m"), TEXT("Selim"), 0.f, 1u, 1, true };
    static const FInfo Ozdemir = { TEXT("\u00d6zdemir Toptan"), TEXT("\u00d6zdemir'in o\u011flu"), 0.04f, 3u, 14, false };
    return Supplier == ESupplier::Ozdemir ? Ozdemir : Trakya;
}

MarketSuppliers::ESupplier MarketSuppliers::Current(const FMarketState& State)
{
    return State.Supplier < static_cast<uint8>(ESupplier::Count) ? static_cast<ESupplier>(State.Supplier) : ESupplier::TrakyaGida;
}

const FMarketSupplierAccount* MarketSuppliers::FindAccount(const FMarketState& State, ESupplier Supplier)
{
    return State.SupplierAccounts.FindByPredicate([Supplier](const FMarketSupplierAccount& A) { return A.Supplier == static_cast<uint8>(Supplier); });
}

FMarketSupplierAccount& MarketSuppliers::Account(FMarketState& State, ESupplier Supplier)
{
    if (FMarketSupplierAccount* Found = State.SupplierAccounts.FindByPredicate([Supplier](const FMarketSupplierAccount& A) { return A.Supplier == static_cast<uint8>(Supplier); }))
        return *Found;
    FMarketSupplierAccount New;
    New.Supplier = static_cast<uint8>(Supplier);
    // The father paid Selim for twenty years: he starts a little warmer than a stranger.
    New.Trust = Supplier == ESupplier::TrakyaGida ? 45 : 30;
    State.SupplierAccounts.Add(New);
    return State.SupplierAccounts.Last();
}

bool MarketSuppliers::Available(const FMarketState& State, ESupplier Supplier)
{
    return State.Day >= Info(Supplier).UnlockDay;
}

float MarketSuppliers::Discount(const FMarketState& State, ESupplier Supplier)
{
    float Total = Info(Supplier).BaseDiscount;
    if (Supplier == ESupplier::TrakyaGida)
        if (const FMarketSupplierAccount* A = FindAccount(State, Supplier))
        {
            // Volume tiers follow the list level so that inflation does not hand out discounts by itself.
            const double Level = MarketPrices::ListLevel(State.Day);
            if (A->Volume30 >= static_cast<int64>(VolumeTier2 * Level)) Total += 0.05f;
            else if (A->Volume30 >= static_cast<int64>(VolumeTier1 * Level)) Total += 0.03f;
        }
    return Total;
}

int32 MarketSuppliers::TermsDays(const FMarketState& State, ESupplier Supplier)
{
    if (!Info(Supplier).bOffersTerms) return 0;
    const FMarketSupplierAccount* A = FindAccount(State, Supplier);
    if (!A) return 0;
    // An unpaid overdue bill closes the terms.
    for (const FMarketPayable& Bill : State.Payables)
        if (Bill.Supplier == static_cast<uint8>(Supplier) && Bill.DueDay < State.Day) return 0;
    return A->Trust >= LongTermsTrust ? 14 : A->Trust >= TermsTrust ? 7 : 0;
}

double MarketSuppliers::MonthSwing(const FMarketState& State)
{
    const MarketCalendar::FDate Date = MarketCalendar::DateOf(State.Day);
    if (Date.Year == MarketCalendar::StartYear && Date.Month == MarketCalendar::StartMonth) return 1.0;
    uint32 Hash = 2166136261u;
    const uint32 Parts[2] = { static_cast<uint32>(State.RivalSeed), static_cast<uint32>(Date.Year * 12 + Date.Month) };
    for (uint32 Part : Parts)
        for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
    return 1.0 + (static_cast<int32>(Hash % 41u) - 20) / 1000.0;
}

int64 MarketSuppliers::UnitCost(const FMarketState& State, const FMarketProduct& Base, int32 Index)
{
    const double Level = MarketPrices::ListLevel(State.Day) * WholesaleFactor * MonthSwing(State);
    const double Deal = Index != INDEX_NONE ? MarketPromotions::CostFactor(State, Index) : 1.0;
    const double Identity = MarketEvents::Factor(State, MarketEvents::EModifier::CostFactor, MarketGoods::Classify(Base.Category)); // quality/discount identity
    return FMath::Max<int64>(1, FMath::RoundToInt64(Base.Cost * Level * (1.0 - Discount(State, Current(State))) * Deal * Identity));
}

int64 MarketSuppliers::ListPrice(const FMarketState& State, const FMarketProduct& Base)
{
    // Retail prices end in 5 kurus.
    const double Price = Base.BasePrice * MarketPrices::ListLevel(State.Day);
    return FMath::Max<int64>(5, FMath::RoundToInt64(Price / 5.0) * 5);
}

void MarketSuppliers::ApplyPrices(const FMarketState& State, const TArray<FMarketProduct>& Base, TArray<FMarketProduct>& Out)
{
    for (int32 I = 0; I < Out.Num(); ++I)
    {
        const FMarketProduct* Source = Base.IsValidIndex(I) && Base[I].Id == Out[I].Id ? &Base[I]
            : Base.FindByPredicate([&Out, I](const FMarketProduct& P) { return P.Id == Out[I].Id; });
        if (!Source) continue;
        Out[I].Cost = UnitCost(State, *Source, I);
        Out[I].BasePrice = ListPrice(State, *Source);
    }
}

namespace
{
    // G-077 (#40): only a real bill (100 TL at today's prices or more) builds trust; splitting orders does not.
    bool BuildsTrust(const FMarketState& State, int64 Amount)
    {
        return Amount >= FMath::RoundToInt64(10000.0 * MarketPrices::ListLevel(State.Day));
    }
}

int64 MarketSuppliers::OrderAllowance(const FMarketState& State)
{
    const ESupplier Supplier = Current(State);
    if (TermsDays(State, Supplier) <= 0) return 0;
    const FMarketSupplierAccount* A = FindAccount(State, Supplier);
    const int64 Limit = FMath::RoundToInt64(50000.0 * MarketPrices::ListLevel(State.Day)) + (A ? A->Volume30 / 2 : 0);
    return FMath::Max<int64>(0, Limit - OpenBills(State));
}

FString MarketSuppliers::OnOrder(FMarketState& State, int64 Bill)
{
    if (Bill <= 0) return FString();
    const ESupplier Supplier = Current(State);
    FMarketSupplierAccount& A = Account(State, Supplier);
    A.Volume30 += Bill;
    const int32 Terms = TermsDays(State, Supplier);
    if (Terms <= 0) return FString();
    // Bought on terms: the cash SubmitOrder took goes back; the bill waits.
    State.Cash += Bill;
    MarketLedger::Post(State, MarketLedger::EAccount::SupplierCredit, Bill); // C3 (B2): bought on terms
    FMarketPayable Payable;
    Payable.Supplier = static_cast<uint8>(Supplier);
    Payable.Amount = Bill;
    Payable.DueDay = State.Day + Terms;
    State.Payables.Add(Payable);
    return FString::Printf(TEXT("%s vadeli yazd\u0131: %s, %d. g\u00fcn \u00f6denecek."), Info(Supplier).Contact, *SupplierTl(Bill), Payable.DueDay);
}

bool MarketSuppliers::Switch(FMarketState& State, ESupplier Supplier, FString& OutMessage)
{
    if (Supplier == Current(State)) { OutMessage = FString::Printf(TEXT("Zaten %s ile \u00e7al\u0131\u015f\u0131yorsun."), Info(Supplier).Name); return false; }
    if (!Available(State, Supplier)) { OutMessage = FString::Printf(TEXT("%s hen\u00fcz b\u00f6lgeye gelmedi."), Info(Supplier).Name); return false; }
    State.Supplier = static_cast<uint8>(Supplier);
    Account(State, Supplier);
    // Leaving the father's wholesaler is noticed.
    if (Supplier != ESupplier::TrakyaGida) Account(State, ESupplier::TrakyaGida).Trust = FMath::Max(0, Account(State, ESupplier::TrakyaGida).Trust - 10);
    OutMessage = Supplier == ESupplier::Ozdemir
        ? FString(TEXT("\u00d6zdemir Toptan'dan alacaks\u0131n: %4 ucuz, ama eksik ve k\u0131r\u0131k mal daha s\u0131k gelir, vade yok. Selim bunu duyunca bozuldu."))
        : FString(TEXT("Yine Selim'den alacaks\u0131n. Trakya G\u0131da eski d\u00fczeninle devam ediyor."));
    return true;
}

int64 MarketSuppliers::PayBills(FMarketState& State)
{
    int64 Paid = 0;
    for (int32 I = 0; I < State.Payables.Num();)
    {
        FMarketPayable& Bill = State.Payables[I];
        if (State.Cash < Bill.Amount) { ++I; continue; }
        State.Cash -= Bill.Amount;
        MarketLedger::Post(State, MarketLedger::EAccount::SupplierCredit, -Bill.Amount);
        Paid += Bill.Amount;
        FMarketSupplierAccount& A = Account(State, static_cast<ESupplier>(Bill.Supplier));
        if (Bill.LateSince == 0) { ++A.OnTime; if (BuildsTrust(State, Bill.Amount)) A.Trust = FMath::Min(100, A.Trust + 5); }
        State.Payables.RemoveAt(I);
    }
    return Paid;
}

int64 MarketSuppliers::OpenBills(const FMarketState& State)
{
    int64 Total = 0;
    for (const FMarketPayable& Bill : State.Payables) Total += Bill.Amount;
    return Total;
}

double MarketSuppliers::PriceGap(const FMarketState& State)
{
    return MarketPrices::ListLevel(State.Day) / FMath::Max(0.01, State.ShelfPriceLevel) - 1.0;
}

int32 MarketSuppliers::PassOnPriceRise(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    const double Factor = 1.0 + PriceGap(State);
    int32 Changed = 0;
    for (int32 I = 0; I < State.Stock.Num() && I < Products.Num(); ++I)
    {
        FMarketStock& Item = State.Stock[I];
        // Shops round a rise up to the next 5 kurus; a small monthly rise would otherwise never reach the label.
        const double Target = Item.Price * Factor;
        const int64 Rounded = Factor > 1.0 ? static_cast<int64>(FMath::CeilToDouble(Target / 5.0 - 1e-9)) * 5 : FMath::RoundToInt64(Target / 5.0) * 5;
        const int64 Raised = FMath::Clamp<int64>(Rounded, 10, FMath::Max<int64>(10, Products[I].BasePrice * 3));
        if (Raised != Item.Price) { Item.Price = Raised; ++Changed; }
    }
    State.ShelfPriceLevel = MarketPrices::ListLevel(State.Day);
    return Changed;
}

FString MarketSuppliers::Summary(const FMarketState& State)
{
    const ESupplier Supplier = Current(State);
    const FMarketSupplierAccount* A = FindAccount(State, Supplier);
    const int32 Terms = TermsDays(State, Supplier);
    FString Text = FString::Printf(TEXT("%s (%s) \u00b7 g\u00fcven %d \u00b7 %s \u00b7 iskonto %%%.0f"), Info(Supplier).Name, Info(Supplier).Contact, A ? A->Trust : 40,
        Terms > 0 ? *FString::Printf(TEXT("%d g\u00fcn vade"), Terms) : TEXT("pe\u015fin"), Discount(State, Supplier) * 100.f);
    if (OpenBills(State) > 0) Text += TEXT(" \u00b7 a\u00e7\u0131k fatura ") + SupplierTl(OpenBills(State));
    const double Gap = PriceGap(State);
    if (Gap >= 0.01) Text += FString::Printf(TEXT(" \u00b7 raf fiyatlar\u0131n listenin %%%.0f gerisinde"), Gap * 100.0);
    return Text;
}

void MarketSuppliers::CloseDay(FMarketState& State)
{
    const int32 Closed = State.Day - 1;
    if (Closed < 1) return;
    TArray<FString>& News = State.DayNews;
    const ESupplier Supplier = Current(State);
    Account(State, Supplier);

    // Relationships: volume fades; ordering keeps the relationship warm.
    for (FMarketSupplierAccount& A : State.SupplierAccounts)
    {
        A.Volume30 -= A.Volume30 / 30;
        if (A.Supplier == static_cast<uint8>(Supplier) && State.LastPurchases > 0) A.Trust = FMath::Min(100, A.Trust + 1);
    }

    // Bills due today are paid from the till automatically; when the cash is short the bill is late.
    for (int32 I = 0; I < State.Payables.Num();)
    {
        FMarketPayable& Bill = State.Payables[I];
        if (Bill.DueDay > Closed) { ++I; continue; }
        FMarketSupplierAccount& A = Account(State, static_cast<ESupplier>(Bill.Supplier));
        const FInfo& Who = Info(static_cast<ESupplier>(Bill.Supplier));
        if (State.Cash >= Bill.Amount)
        {
            State.Cash -= Bill.Amount;
            MarketLedger::Post(State, MarketLedger::EAccount::SupplierCredit, -Bill.Amount);
            if (Bill.LateSince == 0) { ++A.OnTime; if (BuildsTrust(State, Bill.Amount)) A.Trust = FMath::Min(100, A.Trust + 5); }
            News.Add(FString::Printf(TEXT("%s: vadeli fatura \u00f6dendi (%s)."), Who.Name, *SupplierTl(Bill.Amount)));
            State.Payables.RemoveAt(I);
            continue;
        }
        // The late fee grows every unpaid day for LateFeeDays (#41: capped); the lost trust and the late mark come
        // once per bill, a reminder every week after the fee stops.
        const bool bFirstDay = Bill.LateSince == 0;
        const bool bFeeOver = !bFirstDay && Closed - Bill.LateSince >= LateFeeDays;
        const int64 Fee = bFeeOver ? 0 : FMath::Max<int64>(100, FMath::RoundToInt64(Bill.Amount * LateFee));
        if (bFeeOver)
        {
            if ((Closed - Bill.LateSince) % 7 == 0)
            {
                A.Trust = FMath::Max(0, A.Trust - 5);
                News.Add(FString::Printf(TEXT("%s: fatura h\u00e2l\u00e2 \u00f6denmedi (%s). G\u00fcven azal\u0131yor."), Who.Name, *SupplierTl(Bill.Amount)));
            }
            ++I;
            continue;
        }
        Bill.Amount += Fee;
        MarketLedger::Post(State, MarketLedger::EAccount::Penalties, -Fee, false); // C3 (B2): the debt grows, no cash
        if (bFirstDay) { Bill.LateSince = Closed; ++A.Late; A.Trust = FMath::Max(0, A.Trust - 25); }
        State.LastProfit -= Fee;
        News.Add(FString::Printf(TEXT("%s: fatura \u00f6denemedi, %s gecikme fark\u0131 eklendi (bor\u00e7 %s). %s"), Who.Name, *SupplierTl(Fee), *SupplierTl(Bill.Amount),
            bFirstDay ? TEXT("Vade kapand\u0131; g\u00fcven d\u00fc\u015ft\u00fc.") : TEXT("")));
        ++I;
    }

    // The next day's price list. On the 1st the list moves; three days before, the rep warns.
    const MarketCalendar::FDate Tomorrow = MarketCalendar::DateOf(State.Day);
    const FInfo& Who = Info(Supplier);
    if (Tomorrow.Day == 1)
    {
        const double Rise = MarketPrices::ListLevel(State.Day) / MarketPrices::ListLevel(Closed) - 1.0;
        if (Rise > 0.0005)
            News.Add(FString::Printf(TEXT("%s: yeni ay, yeni liste. Al\u0131\u015f fiyatlar\u0131 ve rakip raf fiyatlar\u0131 %%%.1f artt\u0131. Raf fiyatlar\u0131n\u0131 g\u00fcncellemezsen k\u00e2r\u0131n erir."), Who.Contact, Rise * 100.0));
    }
    else if (Tomorrow.Day == MarketCalendar::DaysInMonth(Tomorrow.Year, Tomorrow.Month) - 2)
    {
        const int32 NextFirst = State.Day + 3;
        const double Rise = MarketPrices::ListLevel(NextFirst) / MarketPrices::ListLevel(State.Day) - 1.0;
        if (Rise > 0.0005)
            News.Add(FString::Printf(TEXT("%s: ay\u0131n 1'inde yakla\u015f\u0131k %%%.1f zam geliyor. Depon ve nakdin yetiyorsa \u015fimdi stok yap."), Who.Contact, Rise * 100.0));
    }
    const double Gap = PriceGap(State);
    if (Gap >= 0.03 && Tomorrow.Day == 2)
        News.Add(FString::Printf(TEXT("Raf fiyatlar\u0131n listenin %%%.0f gerisinde. Men\u00fcden \"zamm\u0131 yans\u0131t\" ile hepsini birden g\u00fcncelleyebilirsin."), Gap * 100.0));

    // The cheaper wholesaler arrives.
    if (State.Day == Info(ESupplier::Ozdemir).UnlockDay)
        News.Add(TEXT("\u00d6zdemir Toptan'dan biri u\u011frad\u0131: \"Selim'den %4 ucuza veririz.\" Ucuz ama mal eksik ve k\u0131r\u0131k gelebilir; vade de yok."));
    // Terms open up.
    const FMarketSupplierAccount& Main = Account(State, ESupplier::TrakyaGida);
    if (Supplier == ESupplier::TrakyaGida && Main.Trust >= TermsTrust && Main.Trust - 1 < TermsTrust && State.LastPurchases > 0)
        News.Add(FString::Printf(TEXT("Selim: \"%s gibi d\u00fczenli \u00e7al\u0131\u015f\u0131yorsun. Bundan sonra 7 g\u00fcn vadeli yazar\u0131m.\""), *MarketStart::Relative(State, MarketStart::ECase::Plain, true)));
}

#include "MarketSimulation.h"
#include "MarketBasket.h"
#include "MarketCampaign.h"
#include "MarketCustomers.h"
#include "MarketDemand.h"
#include "MarketDirector.h"
#include "MarketEvents.h"
#include "MarketOrderAdvice.h"
#include "MarketPromotions.h"
#include "MarketRivals.h"
#include "MarketStaff.h"
#include "MarketSuppliers.h"

namespace MarketSimulation
{
    uint32 SimMix(int32 Seed, int32 Day, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), static_cast<uint32>(Day), Salt };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }

    FString SimTl(int64 Kurus)
    {
        const int64 Abs = Kurus < 0 ? -Kurus : Kurus;
        return FString::Printf(TEXT("%s%lld,%02lld TL"), Kurus < 0 ? TEXT("-") : TEXT(""), static_cast<long long>(Abs / 100), static_cast<long long>(Abs % 100));
    }

    void FillShelves(FMarketState& State)
    {
        for (int32 I = 0; I < State.Stock.Num(); ++I) State.Restock(I);
    }

    // One shopper, as AMarketGameMode::SpawnCustomer + ResolveCustomerItem + Checkout do it in the world.
    void Shopper(FMarketState& State, const TArray<FMarketProduct>& Products, const TArray<FString>& Aisles, FRandomStream& Random)
    {
        bool bReturning = false;
        const int32 CustomerId = MarketBasket::ChooseCustomer(State, Random.FRand(), Random.FRand(), bReturning);
        const MarketCustomers::ESegment Segment = MarketCustomers::SegmentOf(CustomerId, State.RivalSeed);
        const uint8 SegmentId = static_cast<uint8>(Segment);
        int64 Budget = FMath::RoundToInt64(MarketCustomers::VisitBudget(Segment, State.Day) * MarketDirector::BudgetFactor(State, SegmentId));
        const TArray<int32> List = MarketCustomers::BuildList(State, Products, Segment, Random);
        if (List.Num() == 0) return;
        const float Share = MarketBasket::EffectiveMarketShare(State, CustomerId);
        const double BaseTolerance = MarketCustomers::Profile(Segment).PriceTolerance;
        TArray<FMarketSaleLine> Lines;
        TSet<int32> InBasket;
        int32 Fulfilled = 0;
        auto Rival = [&](int32 Index) { return MarketDirector::RivalPriceFactor(State, Aisles, Products[Index].Category); };
        auto Available = [&](int32 Index)
        {
            int32 Taken = 0;
            for (const FMarketSaleLine& L : Lines) if (L.Product == Index) Taken += L.Quantity;
            return FMath::Max(0, State.Stock[Index].Shelf - Taken);
        };
        auto TryBuy = [&](int32 Index, MarketDemand::FVisit& OutVisit) -> bool
        {
            const int32 Quantity = MarketPromotions::AdjustQuantity(State, Index, MarketCustomers::Quantity(Segment, Random));
            OutVisit = MarketDemand::Decide(State, Products, Index, Available(Index), Rival(Index), Quantity, Random.FRand(), Share,
                BaseTolerance + MarketDirector::ToleranceBonus(State, Products[Index]), MarketPromotions::UnitPrice(State, Products, Index, Quantity));
            if (OutVisit.Result != MarketDemand::EVisit::Buy) return false;
            int64 Unit = MarketPromotions::UnitPrice(State, Products, OutVisit.Product, OutVisit.Quantity);
            OutVisit.Quantity = MarketCustomers::Affordable(Budget, Unit, OutVisit.Quantity);
            if (OutVisit.Quantity <= 0) { OutVisit.Result = MarketDemand::EVisit::Expensive; return false; }
            Unit = MarketPromotions::UnitPrice(State, Products, OutVisit.Product, OutVisit.Quantity);
            Budget -= Unit * OutVisit.Quantity;
            FMarketSaleLine Line; Line.Product = OutVisit.Product; Line.Quantity = OutVisit.Quantity; Line.QuotedPrice = Unit;
            Lines.Add(Line);
            InBasket.Add(OutVisit.Product);
            return true;
        };
        for (int32 Wanted : List)
        {
            if (!Products.IsValidIndex(Wanted) || !State.Stock.IsValidIndex(Wanted)) continue;
            MarketDemand::FVisit Visit;
            if (TryBuy(Wanted, Visit)) { ++Fulfilled; continue; }
            TArray<int32> Free;
            for (int32 I = 0; I < State.Stock.Num(); ++I) Free.Add(Available(I));
            const int32 Substitute = MarketBasket::FindSubstitute(State, Products, Wanted, Free, InBasket, Rival(Wanted));
            MarketDemand::FVisit Second;
            if (Substitute != INDEX_NONE && TryBuy(Substitute, Second)) { ++Fulfilled; continue; }
            Visit.Product = Wanted;
            MarketDemand::RecordItemFailure(State, Visit);
        }
        if (Lines.Num() == 0)
        {
            ++State.Lost;
            MarketBasket::RecordVisit(State, CustomerId, List.Num(), 0, false);
            return;
        }
        const uint8 Method = MarketDirector::PaymentMethod(State, SegmentId, Random.FRand());
        if (Method == 3 && MarketDirector::LeavesWithoutCard(State, Random.FRand()))
        {
            ++State.Lost;
            MarketBasket::RecordVisit(State, CustomerId, List.Num(), 0, true);
            return;
        }
        int64 Receipt = 0;
        if (State.SellBasket(Lines, Products, &Receipt)) MarketDirector::OnCheckout(State, CustomerId, Receipt, Random.FRand(), Method);
        else ++State.Lost;
        MarketBasket::RecordVisit(State, CustomerId, List.Num(), Fulfilled, false);
    }
}

FString MarketSimulation::DifficultyName(EDifficulty Difficulty)
{
    switch (Difficulty)
    {
    case EDifficulty::Easy: return TEXT("Rahat");
    case EDifficulty::Hard: return TEXT("Zor");
    default: return TEXT("Normal");
    }
}

float MarketSimulation::TrafficFactor(const FMarketState& State)
{
    return State.Difficulty == 0 ? 1.1f : State.Difficulty == 2 ? 0.92f : 1.f;
}

double MarketSimulation::ToleranceBonus(const FMarketState& State)
{
    return State.Difficulty == 0 ? 0.05 : State.Difficulty == 2 ? -0.04 : 0.0;
}

bool MarketSimulation::SetDifficulty(FMarketState& State, int32 Difficulty, FString& OutMessage)
{
    const uint8 Value = static_cast<uint8>(FMath::Clamp(Difficulty, 0, 2));
    if (State.Difficulty == Value) { OutMessage = TEXT("Zorluk zaten b\u00f6yle."); return false; }
    State.Difficulty = Value;
    OutMessage = FString::Printf(TEXT("Zorluk: %s. %s"), *DifficultyName(static_cast<EDifficulty>(Value)),
        Value == 0 ? TEXT("Biraz daha \u00e7ok m\u00fc\u015fteri, fiyata daha ho\u015fg\u00f6r\u00fcl\u00fc.")
        : Value == 2 ? TEXT("Daha az m\u00fc\u015fteri, fiyata daha titiz.") : TEXT("Oyunun tasarland\u0131\u011f\u0131 denge."));
    return true;
}

MarketSimulation::FDay MarketSimulation::PlayDay(FMarketState& State, const TArray<FMarketProduct>& CatalogBase, TArray<FMarketProduct>& Products, bool bAutoOrder)
{
    FDay Day;
    if (State.Stock.Num() != Products.Num()) return Day;
    FRandomStream Random(static_cast<int32>(SimMix(State.RivalSeed, State.Day, 0x51A7u)));
    const TArray<FString> Aisles = MarketRivals::Aisles(Products);
    // Morning routine of the family: the month's price rise goes on the shelf tags, the declared tax is paid, and
    // a spare 50 TL goes to the father's debt when the till can bear it.
    if (MarketSuppliers::PriceGap(State) > 0.02) MarketSuppliers::PassOnPriceRise(State, Products);
    if (State.Books.TaxDue > 0) MarketStaff::PayTax(State);
    if (MarketCampaign::DebtOpen(State) && State.Cash > 4 * MarketCampaign::Installment + 20000) MarketCampaign::PayDebt(State);
    // Morning: the rear door is carried in and the shelves are filled.
    for (int32 I = 0; I < State.Stock.Num(); ++I) State.ReceiveDelivery(I);
    FillShelves(State);
    // The world's crowd limit: nine people inside turn the next one away at the door (about 1 in 25 on busy days).
    const float Traffic = MarketDirector::TrafficFactor(State, Aisles);
    Day.Shoppers = FMath::Max(0, FMath::RoundToInt(ShoppersPerDay * Traffic));
    const int32 Crowded = Traffic > 1.3f ? Day.Shoppers / 25 : 0;
    for (int32 N = 0; N < Crowded; ++N) MarketDemand::RecordWaitingLoss(State);
    for (int32 N = Crowded; N < Day.Shoppers; ++N)
    {
        Shopper(State, Products, Aisles, Random);
        if (N % 8 == 7) FillShelves(State);   // the family (and the stockers) refill between customers
    }
    Day.Served = State.Served;
    Day.Lost = State.Lost;
    Day.Revenue = State.Revenue;
    // Evening: the order the advice suggests, if it reaches the wholesaler's minimum.
    if (bAutoOrder)
    {
        TArray<int32> Draft;
        Draft.Init(0, Products.Num());
        const TArray<float> Scales = MarketDirector::OrderScales(State, Products);
        MarketOrderAdvice::FillSuggested(State, Products, Draft, &Scales);
        int64 Bill = 0;
        for (int32 I = 0; I < Draft.Num(); ++I) Bill += Draft[I] * MarketOrderAdvice::CaseUnits(Products[I]) * Products[I].Cost;
        if (Bill >= MarketOrderAdvice::MinimumOrder && Bill <= State.Cash && State.SubmitOrder(Draft, Products, &Bill))
        {
            MarketDirector::OnOrder(State, Bill);
            Day.Ordered = Bill;
        }
    }
    State.CloseDay();
    MarketDirector::CloseDay(State, Products);
    MarketDirector::ApplyPrices(State, CatalogBase, Products);
    MarketCampaign::CloseDay(State);
    Day.Profit = State.LastProfit;
    ++State.AdvancedDays;
    return Day;
}

int32 MarketSimulation::Advance(FMarketState& State, const TArray<FMarketProduct>& CatalogBase, TArray<FMarketProduct>& Products, int32 Days, FString& OutSummary)
{
    FDay Total;
    int32 Played = 0;
    FString Reason;
    for (; Played < FMath::Clamp(Days, 1, 31);)
    {
        if (MarketEvents::Pending(State)) { Reason = TEXT("bir karar seni bekliyor"); break; }
        const int32 WeekBefore = State.LastWeekNumber;
        const FDay Day = PlayDay(State, CatalogBase, Products);
        ++Played;
        Total.Shoppers += Day.Shoppers; Total.Served += Day.Served; Total.Lost += Day.Lost;
        Total.Revenue += Day.Revenue; Total.Profit += Day.Profit; Total.Ordered += Day.Ordered;
        if (State.Cash < 0) { Reason = TEXT("kasa eksiye d\u00fc\u015ft\u00fc"); break; }
        if (MarketEvents::Pending(State)) { Reason = TEXT("bir karar seni bekliyor"); break; }
        if (State.LastWeekNumber != WeekBefore && Played < Days) { Reason = TEXT("hafta bitti, raporuna bak"); break; }
    }
    OutSummary = FString::Printf(TEXT("%d g\u00fcn ilerledi: %d m\u00fc\u015fteri, %d sat\u0131\u015f, %d kay\u0131p; ciro %s, net %s, sipari\u015f %s."),
        Played, Total.Shoppers, Total.Served, Total.Lost, *SimTl(Total.Revenue), *SimTl(Total.Profit), *SimTl(Total.Ordered));
    if (!Reason.IsEmpty()) OutSummary += FString::Printf(TEXT(" Durdu: %s."), *Reason);
    return Played;
}

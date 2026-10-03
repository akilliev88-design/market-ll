#include "MarketSimulation.h"
#include "MarketCalendar.h"
#include "MarketCountry.h"
#include "MarketBasket.h"
#include "MarketCampaign.h"
#include "MarketCustomers.h"
#include "MarketDemand.h"
#include "MarketDirector.h"
#include "MarketEvents.h"
#include "MarketManagers.h"
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
        return MarketCountry::Money(Kurus); // G-084: the active country\'s currency
    }

    void FillShelves(FMarketState& State)
    {
        for (int32 I = 0; I < State.Stock.Num(); ++I) State.Restock(I);
    }

    // One shopper, as AMarketGameMode::SpawnCustomer + ResolveCustomerItem + Checkout do it in the world.
    void Shopper(FMarketState& State, const TArray<FMarketProduct>& Products, const TArray<FString>& Aisles, FRandomStream& Random, int32& AuditFailures)
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
        const int64 CheckoutCash = State.Cash;
        TArray<int32> ShelvesBefore;
        for (const FMarketStock& Item : State.Stock) ShelvesBefore.Add(Item.Shelf);
        int64 ExpectedReceipt = 0;
        for (const FMarketSaleLine& Line : Lines) ExpectedReceipt += Line.QuotedPrice * Line.Quantity;
        if (State.SellBasket(Lines, Products, &Receipt))
        {
            if (Receipt != ExpectedReceipt || State.Cash - CheckoutCash != ExpectedReceipt) ++AuditFailures;
            for (int32 Index = 0; Index < State.Stock.Num(); ++Index)
            {
                int32 ExpectedShelf = ShelvesBefore[Index];
                for (const FMarketSaleLine& Line : Lines) if (Line.Product == Index) ExpectedShelf -= Line.Quantity;
                if (State.Stock[Index].Shelf != ExpectedShelf) ++AuditFailures;
            }
            MarketDirector::OnCheckout(State, CustomerId, Receipt, Random.FRand(), Method);
        }
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

float MarketSimulation::CapitalFactor(const FMarketState& State)
{
    return State.Difficulty == 0 ? 0.75f : State.Difficulty >= 2 ? 1.15f : 1.f; // C12c: hard 1.3 -> 1.15
}

bool MarketSimulation::SetDifficulty(FMarketState& State, int32 Difficulty, FString& OutMessage)
{
    const uint8 Value = static_cast<uint8>(FMath::Clamp(Difficulty, 0, 2));
    if (State.Difficulty == Value) { OutMessage = TEXT("Zorluk zaten b\u00f6yle."); return false; }
    // G-076: chosen at the start of a campaign and then locked, so it cannot be switched just before an advance.
    if (State.Day > 1) { OutMessage = TEXT("Zorluk kampanyan\u0131n ba\u015f\u0131nda se\u00e7ilir; ilk g\u00fcnden sonra de\u011fi\u015fmez."); return false; }
    State.Difficulty = Value;
    OutMessage = FString::Printf(TEXT("Zorluk: %s. %s"), *DifficultyName(static_cast<EDifficulty>(Value)),
        Value == 0 ? TEXT("Biraz daha \u00e7ok m\u00fc\u015fteri, fiyata daha ho\u015fg\u00f6r\u00fcl\u00fc; yeni ma\u011faza ve kira daha ucuz, rakipler daha yava\u015f.")
        : Value == 2 ? TEXT("Daha az m\u00fc\u015fteri, fiyata daha titiz; yeni ma\u011faza ve kira daha pahal\u0131, rakipler daha sert.") : TEXT("Oyunun tasarland\u0131\u011f\u0131 denge."));
    return true;
}

MarketSimulation::FDay MarketSimulation::PlayDay(FMarketState& State, const TArray<FMarketProduct>& CatalogBase, TArray<FMarketProduct>& Products, bool bAutoOrder)
{
    FDay Day;
    if (State.Stock.Num() != Products.Num()) return Day;
    FRandomStream Random(static_cast<int32>(SimMix(State.RivalSeed, State.Day, 0x51A7u)));
    const TArray<FString> Aisles = MarketRivals::Aisles(Products);
    // Morning routine of the family: the month's price rise goes on the shelf tags, the declared tax is paid, and
    // a spare installment (a tenth of the starting debt, M37) goes to the father's debt when the till can bear it.
    // G-086b ek (M19): with a manager in the family shop his style and skill run the routine (bManaged false: as before).
    MarketManagers::FFamilyRule Family = MarketManagers::FamilyRule(State);
    Family.ForgetPermille = RoutineForgetPermille(State);
    Family.bManaged = true; // the family also makes occasional mistakes without a hired manager
    if (MarketSuppliers::PriceGap(State) > Family.PriceRiseGap && !DelayPriceRise(State)) MarketSuppliers::PassOnPriceRise(State, Products);
    if (State.Books.TaxDue > 0) MarketStaff::PayTax(State);
    if (MarketCampaign::DebtOpen(State) && State.Cash > 4 * MarketCampaign::InstallmentOf(State) + 20000) MarketCampaign::PayDebt(State);
    // Morning: the rear door is carried in and the shelves are filled.
    for (int32 I = 0; I < State.Stock.Num(); ++I)
    {
        const FMarketStock Before = State.Stock[I];
        State.ReceiveDelivery(I);
        const FMarketStock& After = State.Stock[I];
        if (Before.Dock + Before.Warehouse + Before.Shelf != After.Dock + After.Warehouse + After.Shelf) ++Day.AuditFailures;
    }
    FillShelves(State);
    // The world's crowd limit: nine people inside turn the next one away at the door (about 1 in 25 on busy days).
    const float Traffic = MarketDirector::TrafficFactor(State, Aisles, Products); // E2a: the one store formula when switched on
    // G-084: a day the law keeps shops shut (Germany: Sundays, holidays) has no shoppers; wages and rent still run.
    Day.Shoppers = MarketCalendar::ClosedByLaw(State.Day) ? 0 : FMath::Max(0, FMath::RoundToInt(ShoppersPerDay * Traffic));
    const int32 Crowded = Traffic > 1.3f ? Day.Shoppers / 25 : 0;
    for (int32 N = 0; N < Crowded; ++N) MarketDemand::RecordWaitingLoss(State);
    for (int32 N = Crowded; N < Day.Shoppers; ++N)
    {
        Shopper(State, Products, Aisles, Random, Day.AuditFailures);
        if (N % Family.RefillEvery == Family.RefillEvery - 1) FillShelves(State);   // the family (and the stockers) refill between customers
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
        MarketManagers::ShapeFamilyOrder(State, Family, Draft);
        for (int32& Cases : Draft) Cases = FMath::Clamp(Cases, 0, MarketOrderAdvice::MaxCases);
        int64 Bill = 0;
        for (int32 I = 0; I < Draft.Num(); ++I) Bill += Draft[I] * MarketOrderAdvice::CaseUnits(Products[I]) * Products[I].Cost;
        const int64 OrderCash = State.Cash;
        const int64 Allowance = MarketDirector::OrderAllowance(State);   // G-077 (#33): terms count
        if (Bill >= MarketOrderAdvice::MinimumOrderOn(State.Day) && Bill <= State.Cash + Allowance && State.SubmitOrder(Draft, Products, &Bill, nullptr, Allowance))
        {
            if (State.Cash != OrderCash - Bill) ++Day.AuditFailures;
            MarketDirector::OnOrder(State, Bill);
            Day.Ordered = Bill;
        }
    }
    const int64 CoreCash = State.Cash;
    State.CloseDay();
    if (State.Cash != CoreCash + State.LastBranchProfit - State.LastOperatingCost) ++Day.AuditFailures;
    Day.FamilyProfit = State.LastProfit;
    const int64 DirectorCash = State.Cash;
    MarketDirector::CloseDay(State, Products);
    Day.BackgroundCashDelta = State.Cash - DirectorCash;
    MarketDirector::ApplyPrices(State, CatalogBase, Products);
    MarketCampaign::CloseDay(State);
    Day.Profit = State.LastProfit;
    ++State.AdvancedDays;
    return Day;
}

bool MarketSimulation::AdjustPrice(FMarketState& State, const TArray<FMarketProduct>& Products, int32 Index, bool bUp)
{
    if (!State.Stock.IsValidIndex(Index) || !Products.IsValidIndex(Index)) return false;
    FMarketStock& Item = State.Stock[Index];
    const int64 Previous = Item.Price;
    const int64 Step = MarketDemand::PriceStep(Products[Index]);
    Item.Price = FMath::Clamp<int64>(Item.Price + (bUp ? Step : -Step), 10, Products[Index].BasePrice * 3);
    return Item.Price != Previous;
}
int32 MarketSimulation::RoutineSkill(const FMarketState& State)
{
    const MarketManagers::FFamilyRule Family = MarketManagers::FamilyRule(State);
    return Family.bManaged ? Family.Skill : 55; // familiar family routine, not perfect management
}
int32 MarketSimulation::RoutineForgetPermille(const FMarketState& State)
{
    return FMath::Clamp((85 - RoutineSkill(State)) * 4, 0, 340);
}
bool MarketSimulation::DelayPriceRise(const FMarketState& State)
{
    const int32 Chance = FMath::Clamp((80 - RoutineSkill(State)) * 6, 0, 480);
    return static_cast<int32>(SimMix(State.RivalSeed, State.Day, 0xA3F1u) % 1000u) < Chance;
}
int32 MarketSimulation::RequestedDays(const FMarketState& State, ETurn Turn)
{
    if (Turn == ETurn::Week) return 7;
    if (Turn == ETurn::Month)
    {
        const MarketCalendar::FDate Date = MarketCalendar::DateOf(State.Day);
        return MarketCalendar::DaysInMonth(Date.Year, Date.Month) - Date.Day + 1;
    }
    return 1;
}
FString MarketSimulation::StopMessage(EStop Reason, int32 Played)
{
    const TCHAR* Why = TEXT("tur tamamlandi");
    switch (Reason)
    {
    case EStop::Decision: Why = TEXT("bir karar seni bekliyor"); break;
    case EStop::NegativeCash: Why = TEXT("kasa eksiye dustu"); break;
    case EStop::WeekReport: Why = TEXT("hafta ozeti hazir"); break;
    case EStop::MonthReport: Why = TEXT("ay ozeti hazir"); break;
    case EStop::Chapter: Why = TEXT("yeni hikaye bolumu acildi"); break;
    case EStop::ImportantEvent: Why = TEXT("onemli bir gelisme var, kararlara bak"); break;
    case EStop::InvalidCatalog: Why = TEXT("stok kaydi katalogla uyusmuyor"); break;
    case EStop::CampaignOver: Why = TEXT("bu kampanya sona erdi"); break;
    default: break;
    }
    return FString::Printf(TEXT("%d gun ilerledi; %s."), Played, Why);
}
namespace MarketSimulation
{
    struct FSignals
    {
        int32 Chapter = 0;
        TArray<int32> Wars, Caught;
        TArray<FString> DepotCaught;
        explicit FSignals(const FMarketState& State)
        {
            Chapter = State.Story.Chapter;
            for (const FMarketCompetitor& Rival : State.Competitors) Wars.Add(Rival.WarUntil);
            for (const FMarketBranch& Branch : State.Branches) Caught.Add(Branch.ManagerCaughtDay);
            for (const FMarketDepot& Depot : State.Company.DepotSites) DepotCaught.Add(Depot.CaughtName);
        }
        bool Important(const FMarketState& State) const
        {
            for (int32 Index = 0; Index < State.Competitors.Num(); ++Index)
                if (State.Competitors[Index].WarUntil >= State.Day && State.Competitors[Index].WarUntil > (Wars.IsValidIndex(Index) ? Wars[Index] : 0)) return true;
            for (int32 Index = 0; Index < State.Branches.Num(); ++Index)
                if (State.Branches[Index].ManagerCaughtDay > (Caught.IsValidIndex(Index) ? Caught[Index] : 0)) return true;
            for (int32 Index = 0; Index < State.Company.DepotSites.Num(); ++Index)
                if (!State.Company.DepotSites[Index].CaughtName.IsEmpty() && State.Company.DepotSites[Index].CaughtName != (DepotCaught.IsValidIndex(Index) ? DepotCaught[Index] : FString())) return true;
            return false;
        }
    };
    EStop BeforeTurn(const FMarketState& State, const TArray<FMarketProduct>& Products)
    {
        if (State.Story.bCampaignOver) return EStop::CampaignOver;
        if (State.Stock.Num() != Products.Num() || Products.IsEmpty()) return EStop::InvalidCatalog;
        if (State.Cash < 0) return EStop::NegativeCash;
        if (MarketEvents::Pending(State)) return EStop::Decision;
        return EStop::None;
    }
    FTurn PlayTurn(FMarketState& State, const TArray<FMarketProduct>& Base, TArray<FMarketProduct>& Products, int32 Requested, ETurn Kind, const FHooks& Hooks)
    {
        FTurn Result; Result.Requested = FMath::Clamp(Requested, 1, FMath::Clamp(Hooks.MaxDays, 1, 31));
        const int32 First = State.Day;
        for (; Result.Played < Result.Requested;)
        {
            Result.Stop = BeforeTurn(State, Products);
            if (Result.Stop != EStop::None) break;
            if (Hooks.BeforeDay) Hooks.BeforeDay();
            // Policy commands can offer a new choice or exhaust the cash before a day starts.
            Result.Stop = BeforeTurn(State, Products);
            if (Result.Stop != EStop::None) break;
            const FSignals Before(State);
            const FDay Today = PlayDay(State, Base, Products, Hooks.bAutoOrder);
            ++Result.Played;
            Result.Total.Shoppers += Today.Shoppers; Result.Total.Served += Today.Served; Result.Total.Lost += Today.Lost;
            Result.Total.Revenue += Today.Revenue; Result.Total.Profit += Today.Profit; Result.Total.Ordered += Today.Ordered;
            Result.Total.AuditFailures += Today.AuditFailures;
            if (Hooks.AfterDay) Hooks.AfterDay(Today);
            Result.Stop = BeforeTurn(State, Products);
            if (Result.Stop != EStop::None) break;
            if (State.Story.Chapter != Before.Chapter) { Result.Stop = EStop::Chapter; break; }
            if (Before.Important(State)) { Result.Stop = EStop::ImportantEvent; break; }
            // A seven-day request crosses the intervening calendar week; it never ends after 1..6 days for a report.
        }
        if (Result.Stop == EStop::None)
        {
            if (Kind == ETurn::Week) Result.Stop = EStop::WeekReport;
            else if (Kind == ETurn::Month && MarketCalendar::DateOf(State.Day).Day == 1) Result.Stop = EStop::MonthReport;
            else if ((State.Day - 1) % 7 == 0 && Result.Played > 0) Result.Stop = EStop::WeekReport;
        }
        Result.Period = Summary(State, First, State.Day - 1);
        Result.Message = StopMessage(Result.Stop, Result.Played);
        return Result;
    }
}
MarketSimulation::FTurn MarketSimulation::AdvanceTurn(FMarketState& State, const TArray<FMarketProduct>& CatalogBase, TArray<FMarketProduct>& Products, ETurn Turn, const FHooks& Hooks)
{
    return PlayTurn(State, CatalogBase, Products, RequestedDays(State, Turn), Turn, Hooks);
}
int32 MarketSimulation::Advance(FMarketState& State, const TArray<FMarketProduct>& CatalogBase, TArray<FMarketProduct>& Products, int32 Days, FString& OutSummary)
{
    const ETurn Kind = Days == 7 ? ETurn::Week : ETurn::Day;
    const FTurn Result = PlayTurn(State, CatalogBase, Products, Days, Kind, FHooks());
    OutSummary = Result.Message;
    return Result.Played;
}
MarketSimulation::FPeriod MarketSimulation::Summary(const FMarketState& State, int32 FirstDay, int32 LastDay)
{
    FPeriod Result; Result.FirstDay = FMath::Max(1, FirstDay); Result.LastDay = FMath::Min(State.Day - 1, LastDay);
    if (Result.LastDay < Result.FirstDay) return Result;
    int32 LastSeen = 0;
    for (const FMarketDayRecord& Record : State.History)
    {
        if (Record.Day < Result.FirstDay || Record.Day > Result.LastDay) continue;
        ++Result.RecordedDays; Result.Revenue += Record.Revenue; Result.Profit += Record.Profit;
        Result.Served += Record.Served; Result.Lost += Record.Lost;
        if (Record.Day >= LastSeen) { LastSeen = Record.Day; Result.ClosingCash = Record.Cash; }
    }
    Result.MissingDays = FMath::Max(0, Result.LastDay - Result.FirstDay + 1 - Result.RecordedDays);
    return Result;
}
MarketSimulation::FPeriod MarketSimulation::WeekSummary(const FMarketState& State, int32 ClosedDay)
{
    const int32 Anchor = ClosedDay > 0 ? FMath::Min(State.Day - 1, ClosedDay) : State.Day - 1;
    if (Anchor < 1) return FPeriod();
    const int32 First = (Anchor - 1) / 7 * 7 + 1;
    return Summary(State, First, FMath::Min(Anchor, First + 6));
}
MarketSimulation::FPeriod MarketSimulation::MonthSummary(const FMarketState& State, int32 ClosedDay)
{
    const int32 Anchor = ClosedDay > 0 ? FMath::Min(State.Day - 1, ClosedDay) : State.Day - 1;
    if (Anchor < 1) return FPeriod();
    const MarketCalendar::FDate Date = MarketCalendar::DateOf(Anchor);
    return Summary(State, MarketCalendar::GameDayOf(Date.Year, Date.Month, 1), Anchor);
}
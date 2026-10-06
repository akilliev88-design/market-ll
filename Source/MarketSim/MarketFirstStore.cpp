#include "MarketFirstStore.h"
#include "MarketEconomy.h"
#include "MarketBranches.h"
#include "MarketCountry.h"
#include "MarketFinance.h"
#include "MarketGoods.h"
#include "MarketLedger.h"
#include "MarketStaff.h"
#include "MarketStart.h"

namespace MarketFirstStoreLocal
{
    FString Tl(int64 Kurus) { return MarketCountry::Money(Kurus); }

    bool IsShopRole(const FMarketEmployee& E)
    {
        const MarketStaff::ERole Role = MarketStaff::RoleOf(E);
        return Role == MarketStaff::ERole::Cashier || Role == MarketStaff::ERole::Stocker;
    }

    // Room a branch has for one more delivery of a product: one and a half shelves, less what is there and on the way.
    int32 RoomFor(const FMarketStock& Item)
    {
        return FMath::Max(0, Item.Capacity * 3 / 2 - Item.Shelf - Item.Incoming);
    }
}

MarketFirstStore::EStatus MarketFirstStore::StatusOf(const FMarketState& State)
{
    return State.FirstStoreStatus <= static_cast<uint8>(EStatus::Leased) ? static_cast<EStatus>(State.FirstStoreStatus) : EStatus::Open;
}

bool MarketFirstStore::IsOpen(const FMarketState& State)
{
    return StatusOf(State) == EStatus::Open;
}

double MarketFirstStore::RunningShare(const FMarketState& State)
{
    return State.FirstStoreRunning();
}

int64 MarketFirstStore::MonthlyLease(const FMarketState& State)
{
    return FMath::Max<int64>(0, MarketFinance::BuildingReferenceValue(State) / MarketFinance::BuildingRentMonths);
}

bool MarketFirstStore::CanClose(const FMarketState& State, FString& OutReason)
{
    if (!IsOpen(State)) { OutReason = TEXT("\u0130lk ma\u011faza zaten kapal\u0131."); return false; }
    return true;
}

int32 MarketFirstStore::SendToStores(FMarketState& State, const FString& Country, const FString& Province, const FString& ProductId, int32 Units, int32 SkipBranch)
{
    if (Units <= 0) return 0;
    int32 Placed = 0;
    // Three passes: the same province, the same country, anywhere.
    for (int32 Pass = 0; Pass < 3 && Placed < Units; ++Pass)
    {
        for (int32 I = 0; I < State.Branches.Num() && Placed < Units; ++I)
        {
            if (I == SkipBranch) continue;
            FMarketBranch& B = State.Branches[I];
            if (B.Stage != static_cast<uint8>(MarketBranches::EStage::Open)) continue;
            const FString BCountry = MarketBranches::CountryOf(State, B);
            const FString BProvince = B.Province.IsEmpty() ? MarketStart::HomeProvince(State) : B.Province;
            const bool bSameCountry = BCountry == Country;
            const bool bSameProvince = bSameCountry && BProvince == Province;
            if ((Pass == 0 && !bSameProvince) || (Pass == 1 && (!bSameCountry || bSameProvince)) || (Pass == 2 && bSameCountry)) continue;
            FMarketStock* Item = B.Items.FindByPredicate([&ProductId](const FMarketStock& S) { return S.Id == ProductId; });
            if (!Item || Item->Capacity <= 0) continue;
            const int32 Take = FMath::Min(Units - Placed, MarketFirstStoreLocal::RoomFor(*Item));
            if (Take <= 0) continue;
            Item->Incoming += Take; // our own truck brings it with the next delivery
            Placed += Take;
        }
    }
    return Placed;
}

bool MarketFirstStore::Close(FMarketState& State, const TArray<FMarketProduct>& Products, EBuilding Building, FString& OutMessage)
{
    if (!CanClose(State, OutMessage)) return false;
    const FString Name = MarketStart::FirstStoreName(State);
    const FString Home = MarketStart::HomeProvince(State);
    // The building's value before it changes hands (a sold building leaves the balance sheet).
    const int64 Value = MarketFinance::BuildingValue(State);

    // Goods: to our other stores as far as they have room; perishable goods and the rest to the wholesaler at half the cost.
    int32 Moved = 0, Sold = 0;
    int64 SoldValue = 0;
    for (FMarketStock& Row : State.Stock)
    {
        const int32 Units = Row.Shelf + Row.Warehouse + Row.Dock + Row.Incoming;
        Row.Shelf = Row.Warehouse = Row.Dock = Row.Incoming = 0;
        Row.Received = 0;
        if (Units <= 0) continue;
        const FMarketProduct* Product = Products.FindByPredicate([&Row](const FMarketProduct& P) { return P.Id == Row.Id; });
        const bool bPerishable = Product && MarketGoods::ShelfLifeDays(*Product) > 0;
        const int32 Placed = bPerishable ? 0 : SendToStores(State, State.CountryId, Home, Row.Id, Units, INDEX_NONE);
        Moved += Placed;
        const int32 Left = Units - Placed;
        if (Left <= 0) continue;
        const int64 UnitCost = Row.AvgCost > 0 ? Row.AvgCost : (Product ? Product->Cost : 0);
        const int64 Half = static_cast<int64>(Left) * UnitCost / 2;
        Sold += Left;
        SoldValue += Half;
        State.PendingLoss += Half; // the other half is lost
    }
    State.Batches.Reset();
    State.Cash += SoldValue;
    MarketLedger::Post(State, MarketLedger::EAccount::Divestment, SoldValue, true);
    MarketLedger::Post(State, MarketLedger::EAccount::Shrinkage, -SoldValue, false);

    // People: the shop's cashiers and stockers leave with their notice and seniority pay.
    int64 Severance = 0;
    int32 LetGo = 0;
    for (const FMarketEmployee& E : State.Staff)
        if (MarketFirstStoreLocal::IsShopRole(E)) { Severance += MarketStaff::SeverancePay(State, E); ++LetGo; }
    State.Staff.RemoveAll([](const FMarketEmployee& E) { return MarketFirstStoreLocal::IsShopRole(E); });
    State.Cash -= Severance;
    MarketLedger::Post(State, MarketLedger::EAccount::Severance, -Severance, true);

    // Its own campaigns end today.
    for (FMarketPromotion& P : State.Promotions)
        if (P.Store == MarketLedger::FirstStore && P.StartDay > 0 && P.EndDay >= State.Day) P.EndDay = FMath::Max(P.StartDay, State.Day - 1);

    // The building.
    FString Building_;
    switch (Building)
    {
    case EBuilding::Sell:
        State.Cash += Value;
        MarketLedger::Post(State, MarketLedger::EAccount::Divestment, Value, true);
        State.FirstStoreStatus = static_cast<uint8>(EStatus::Sold);
        Building_ = FString::Printf(TEXT("Bina sat\u0131ld\u0131: %s kasaya girdi."), *MarketFirstStoreLocal::Tl(Value));
        break;
    case EBuilding::Lease:
        State.FirstStoreStatus = static_cast<uint8>(EStatus::Leased);
        Building_ = FString::Printf(TEXT("Bina kiraya verildi: ayda %s kira gelir."), *MarketFirstStoreLocal::Tl(MonthlyLease(State)));
        break;
    default:
        State.FirstStoreStatus = static_cast<uint8>(EStatus::Empty);
        Building_ = TEXT("Bina bo\u015f duruyor; istedi\u011fin g\u00fcn yeniden a\u00e7abilirsin.");
        break;
    }
    State.FirstStoreClosedDay = State.Day;

    OutMessage = FString::Printf(TEXT("%s kapand\u0131. %s"), *Name, *Building_);
    if (Moved > 0) OutMessage += FString::Printf(TEXT(" %d \u00fcr\u00fcn di\u011fer ma\u011fazalar\u0131m\u0131za g\u00f6nderildi."), Moved);
    if (Sold > 0) OutMessage += FString::Printf(TEXT(" %d \u00fcr\u00fcn toptanc\u0131ya yar\u0131 fiyat\u0131na verildi (%s)."), Sold, *MarketFirstStoreLocal::Tl(SoldValue));
    if (LetGo > 0) OutMessage += FString::Printf(TEXT(" %d \u00e7al\u0131\u015fan\u0131n tazminat\u0131 \u00f6dendi (%s)."), LetGo, *MarketFirstStoreLocal::Tl(Severance));
    return true;
}

bool MarketFirstStore::CanReopen(const FMarketState& State, FString& OutReason)
{
    switch (StatusOf(State))
    {
    case EStatus::Open: OutReason = TEXT("\u0130lk ma\u011faza zaten a\u00e7\u0131k."); return false;
    case EStatus::Sold: OutReason = TEXT("Bina sat\u0131ld\u0131; ilk ma\u011faza yeniden a\u00e7\u0131lamaz."); return false;
    case EStatus::Leased:
        if (State.Cash < MonthlyLease(State) * LeaseNoticeMonths)
        {
            OutReason = FString::Printf(TEXT("Kirac\u0131n\u0131n \u00e7\u0131kmas\u0131 i\u00e7in kasada %s gerekiyor."), *MarketFirstStoreLocal::Tl(MonthlyLease(State) * LeaseNoticeMonths));
            return false;
        }
        return true;
    default: return true;
    }
}

bool MarketFirstStore::Reopen(FMarketState& State, FString& OutMessage)
{
    if (!CanReopen(State, OutMessage)) return false;
    FString Tenant;
    if (StatusOf(State) == EStatus::Leased)
    {
        const int64 Pay = MonthlyLease(State) * LeaseNoticeMonths;
        State.Cash -= Pay;
        MarketLedger::Post(State, MarketLedger::EAccount::Penalties, -Pay, true);
        Tenant = FString::Printf(TEXT(" Kirac\u0131ya \u00e7\u0131kmas\u0131 i\u00e7in %s \u00f6dendi."), *MarketFirstStoreLocal::Tl(Pay));
    }
    State.FirstStoreStatus = static_cast<uint8>(EStatus::Open);
    State.FirstStoreClosedDay = 0;
    OutMessage = FString::Printf(TEXT("%s yeniden a\u00e7\u0131ld\u0131.%s Raflar bo\u015f ve kasiyer yok: personel al, sipari\u015f ver."), *MarketStart::FirstStoreName(State), *Tenant);
    return true;
}

void MarketFirstStore::CloseDay(FMarketState& State)
{
    if (StatusOf(State) != EStatus::Leased) return;
    const int64 Rent = MonthlyLease(State) / 30;
    if (Rent <= 0) return;
    State.Cash += Rent;
    MarketLedger::Post(State, MarketLedger::EAccount::OtherIncome, Rent, true);
}

FString MarketFirstStore::StatusText(const FMarketState& State)
{
    switch (StatusOf(State))
    {
    case EStatus::Empty: return TEXT("kapal\u0131 \u00b7 bina bo\u015f");
    case EStatus::Sold: return TEXT("kapal\u0131 \u00b7 bina sat\u0131ld\u0131");
    case EStatus::Leased: return FString::Printf(TEXT("kapal\u0131 \u00b7 kirada, ayda %s"), *MarketFirstStoreLocal::Tl(MonthlyLease(State)));
    default: return TEXT("bizim \u00b7 kira yok");
    }
}

FString MarketFirstStore::CloseQuestion(const FMarketState& State, EBuilding Building)
{
    const FString Name = MarketStart::FirstStoreName(State);
    const FString Common = TEXT("Kasiyer ve reyon g\u00f6revlileri tazminatla ayr\u0131l\u0131r, mal di\u011fer ma\u011fazalar\u0131m\u0131za gider (s\u0131\u011fmayan ve bozulabilir olan toptanc\u0131ya yar\u0131 fiyat\u0131na).");
    switch (Building)
    {
    case EBuilding::Sell:
        return FString::Printf(TEXT("%s kapans\u0131n ve bina sat\u0131ls\u0131n m\u0131? %s Bina %s eder; sat\u0131l\u0131rsa ilk ma\u011faza bir daha a\u00e7\u0131lamaz."), *Name, *Common, *MarketFirstStoreLocal::Tl(MarketFinance::BuildingValue(State)));
    case EBuilding::Lease:
        return FString::Printf(TEXT("%s kapans\u0131n ve bina kiraya verilsin mi? %s Ayda %s kira gelir; yeniden a\u00e7mak i\u00e7in kirac\u0131ya bir ayl\u0131k kira \u00f6denir."), *Name, *Common, *MarketFirstStoreLocal::Tl(MonthlyLease(State)));
    default:
        return FString::Printf(TEXT("%s kapans\u0131n, bina bo\u015f dursun mu? %s Bo\u015f bina giderin d\u00f6rtte birini yer; istedi\u011fin g\u00fcn yeniden a\u00e7\u0131l\u0131r."), *Name, *Common);
    }
}

FString MarketFirstStore::ReopenQuestion(const FMarketState& State)
{
    const FString Name = MarketStart::FirstStoreName(State);
    if (StatusOf(State) == EStatus::Leased)
        return FString::Printf(TEXT("%s yeniden a\u00e7\u0131ls\u0131n m\u0131? Kirac\u0131ya %s \u00f6denir; raflar bo\u015f a\u00e7\u0131l\u0131r."), *Name, *MarketFirstStoreLocal::Tl(MonthlyLease(State) * LeaseNoticeMonths));
    return FString::Printf(TEXT("%s yeniden a\u00e7\u0131ls\u0131n m\u0131? Raflar bo\u015f a\u00e7\u0131l\u0131r; personel al ve sipari\u015f ver."), *Name);
}

#include "MarketBranches.h"
#include "MarketCalendar.h"
#include "MarketCampaign.h"
#include "MarketCompetitors.h"
#include "MarketCustomers.h"
#include "MarketGoods.h"
#include "MarketLayout.h"
#include "MarketPrices.h"
#include "MarketStaff.h"
#include "MarketSuppliers.h"

namespace MarketBranches
{
    constexpr uint32 Bit(EDistrict D) { return 1u << static_cast<uint32>(D); }

    uint32 BranchMix(int32 Seed, int32 Day, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), static_cast<uint32>(Day), Salt };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }

    FString BranchTl(int64 Kurus)
    {
        const int64 Abs = Kurus < 0 ? -Kurus : Kurus;
        return FString::Printf(TEXT("%s%lld,%02lld TL"), Kurus < 0 ? TEXT("-") : TEXT(""), static_cast<long long>(Abs / 100), static_cast<long long>(Abs % 100));
    }

    const TCHAR* ManagerNames[] = { TEXT("Levent \u00d6zt\u00fcrk"), TEXT("Sibel Kara"), TEXT("Orhan Yavuz"), TEXT("Nurcan Aksu"), TEXT("Tamer G\u00fcne\u015f"),
        TEXT("Filiz Er"), TEXT("G\u00f6khan Bulut"), TEXT("Aynur Tekin") };

    // Our shops in the district and next door take customers from each other.
    float Cannibalization(const FMarketState& State, int32 Self, EDistrict District)
    {
        const FDistrict& Here = DistrictInfo(District);
        float Factor = 1.f;
        auto Count = [&](EDistrict Other)
        {
            if (Other == District) Factor *= 0.6f;
            else if (Here.Neighbours & Bit(Other)) Factor *= 0.9f;
        };
        Count(EDistrict::Istasyon); // the family shop
        for (int32 I = 0; I < State.Branches.Num(); ++I)
            if (I != Self && State.Branches[I].Stage == static_cast<uint8>(EStage::Open)) Count(static_cast<EDistrict>(State.Branches[I].District));
        return Factor;
    }

    // Share of the district's wishes for each product (segment mix x taste x calendar).
    TArray<float> Wishes(const FMarketState& State, const TArray<FMarketProduct>& Products, const FDistrict& Where, int32 Day)
    {
        TArray<float> Weights;
        float Total = 0.f;
        for (const FMarketProduct& P : Products)
        {
            const MarketGoods::EGroup Group = MarketGoods::Classify(P.Category);
            float Taste = 0.f;
            for (int32 S = 0; S < 6; ++S) Taste += Where.Mix[S] / 100.f * MarketCustomers::Profile(static_cast<MarketCustomers::ESegment>(S)).Preference[static_cast<int32>(Group)];
            const float W = Taste * MarketCalendar::GroupFactor(Day, State.RivalSeed, Group);
            Weights.Add(W);
            Total += W;
        }
        for (float& W : Weights) W = Total > 0.f ? W / Total : 0.f;
        return Weights;
    }

    FMarketBranchItem* ItemOf(FMarketBranch& Branch, const FString& ProductId)
    {
        return Branch.Items.FindByPredicate([&ProductId](const FMarketBranchItem& I) { return I.ProductId == ProductId; });
    }

    void PlanShelves(const FMarketState& State, FMarketBranch& Branch, const TArray<FMarketProduct>& Products)
    {
        const FDistrict& Where = DistrictInfo(static_cast<EDistrict>(Branch.District));
        const TArray<float> Wish = Wishes(State, Products, Where, FMath::Max(1, State.Day));
        TArray<float> Demand;
        for (const float W : Wish) Demand.Add(W * Where.Shoppers * 0.3f * UnitsPerShopper);
        FMarketPlanogram Plan = MarketLayout::Fixtures(Branch.Format);
        MarketLayout::Plan(Plan, Products, Demand);
        const TArray<int32> Capacities = MarketLayout::Capacities(Plan, Products);
        Branch.Items.Reset();
        for (int32 I = 0; I < Products.Num(); ++I)
        {
            FMarketBranchItem Item;
            Item.ProductId = Products[I].Id;
            Item.Capacity = Capacities.IsValidIndex(I) ? Capacities[I] : 0;
            Branch.Items.Add(Item);
        }
    }

    int64 StockCost(const FMarketBranch& Branch, const TArray<FMarketProduct>& Products)
    {
        int64 Cost = 0;
        for (const FMarketBranchItem& Item : Branch.Items)
            for (const FMarketProduct& P : Products)
                if (P.Id == Item.ProductId) { Cost += static_cast<int64>(FMath::Max(0, Item.Capacity - Item.Units)) * P.Cost; break; }
        return Cost;
    }

    float MainPriceIndex(const FMarketState& State, const TArray<FMarketProduct>& Products)
    {
        double Sum = 0.0;
        int32 Count = 0;
        for (int32 I = 0; I < State.Stock.Num() && I < Products.Num(); ++I)
            if (Products[I].BasePrice > 0) { Sum += static_cast<double>(State.Stock[I].Price) / Products[I].BasePrice; ++Count; }
        return Count > 0 ? static_cast<float>(Sum / Count) : 1.f;
    }
}

const MarketBranches::FDistrict& MarketBranches::DistrictInfo(EDistrict District)
{
    using D = EDistrict;
    static const FDistrict Districts[static_cast<int32>(EDistrict::Count)] =
    {
        { TEXT("\u0130stasyon"), TEXT("baban\u0131n d\u00fckk\u00e2n\u0131; eski esnaf, emekliler"), 220, 0.95f, 0, { 22, 28, 24, 12, 6, 8 }, 3.0f, Bit(D::Carsi) | Bit(D::Sanayi) },
        { TEXT("\u00c7ar\u015f\u0131"), TEXT("yaya \u00e7ok, esnaf ve i\u015f \u00e7\u0131k\u0131\u015f\u0131; zincirler burada"), 320, 1.0f, 90000, { 15, 15, 35, 10, 20, 5 }, 4.2f, Bit(D::Istasyon) | Bit(D::Kocasinan) | Bit(D::Universite) },
        { TEXT("Kocasinan"), TEXT("aileler, haftal\u0131k al\u0131\u015fveri\u015f"), 280, 1.0f, 60000, { 18, 40, 22, 8, 4, 8 }, 3.2f, Bit(D::Carsi) | Bit(D::YeniMahalle) | Bit(D::Evrensekiz) },
        { TEXT("Yeni Mahalle"), TEXT("gen\u00e7 aileler, fiyata duyarl\u0131; y\u0131llar i\u00e7inde b\u00fcy\u00fcr"), 200, 0.85f, 35000, { 10, 45, 25, 5, 3, 12 }, 2.0f, Bit(D::Kocasinan) },
        { TEXT("\u00dcniversite yolu"), TEXT("\u00f6\u011frenciler; yaz\u0131n yar\u0131ya iner"), 180, 0.7f, 45000, { 5, 10, 15, 55, 5, 10 }, 2.0f, Bit(D::Carsi) },
        { TEXT("Sanayi"), TEXT("i\u015f\u00e7iler ve esnaf, sabah erken"), 150, 0.9f, 30000, { 5, 10, 55, 5, 20, 5 }, 1.5f, Bit(D::Istasyon) },
        { TEXT("Evrensekiz yolu"), TEXT("villalar; y\u00fcksek gelir, marka ve tazelik ister"), 120, 1.5f, 80000, { 20, 45, 20, 5, 5, 5 }, 1.8f, Bit(D::Kocasinan) },
    };
    return Districts[FMath::Clamp(static_cast<int32>(District), 0, static_cast<int32>(EDistrict::Count) - 1)];
}

const MarketBranches::FFormat& MarketBranches::FormatInfo(const FString& Id)
{
    static const FFormat Small = { TEXT("kucuk"), TEXT("k\u00fc\u00e7\u00fck market"), 250000, 2, 0.95f };
    static const FFormat Neighbourhood = { TEXT("mahalle"), TEXT("mahalle marketi"), 400000, 3, 1.0f };
    static const FFormat Big = { TEXT("buyuk"), TEXT("s\u00fcpermarket"), 900000, 6, 1.1f };
    return Id == TEXT("kucuk") ? Small : Id == TEXT("buyuk") ? Big : Neighbourhood;
}

FString MarketBranches::DistrictName(EDistrict District)
{
    return DistrictInfo(District).Name;
}

int64 MarketBranches::OpeningCost(const FMarketState& State, const TArray<FMarketProduct>& Products, EDistrict District, const FString& Format)
{
    const double Level = MarketPrices::ListLevel(State.Day);
    FMarketBranch Probe;
    Probe.District = static_cast<uint8>(District);
    Probe.Format = FormatInfo(Format).Id;
    PlanShelves(State, Probe, Products);
    return FMath::RoundToInt64((2 * DistrictInfo(District).Rent + FormatInfo(Format).FitOut) * Level) + StockCost(Probe, Products);
}

int32 MarketBranches::OpenCount(const FMarketState& State)
{
    int32 Count = 0;
    for (const FMarketBranch& B : State.Branches) if (B.Stage != static_cast<uint8>(EStage::Closed)) ++Count;
    return Count;
}

bool MarketBranches::CanOpen(const FMarketState& State, const TArray<FMarketProduct>& Products, EDistrict District, const FString& Format, FString& OutReason)
{
    if (District == EDistrict::Istasyon || District >= EDistrict::Count) { OutReason = TEXT("Baban\u0131n d\u00fckk\u00e2n\u0131 zaten \u0130stasyon'da."); return false; }
    for (const FMarketBranch& B : State.Branches)
        if (B.District == static_cast<uint8>(District) && B.Stage != static_cast<uint8>(EStage::Closed)) { OutReason = TEXT("Bu semtte zaten bir \u015fuben var."); return false; }
    if (OpenCount(State) == 0)
    {
        // The first branch keeps the chapter-2 goals (the money is checked below against the real opening cost).
        if (MarketCampaign::DebtOpen(State)) { OutReason = TEXT("\u00d6nce baban\u0131n borcunu kapat."); return false; }
        if (State.ProfitableDays < MarketCampaign::ExpandProfitableDays) { OutReason = FString::Printf(TEXT("\u00d6nce %d k\u00e2rl\u0131 g\u00fcn."), MarketCampaign::ExpandProfitableDays); return false; }
        if (State.MarketShare < MarketCampaign::ExpandShare) { OutReason = FString::Printf(TEXT("\u00d6nce mahalle pay\u0131 %%%.0f."), MarketCampaign::ExpandShare); return false; }
    }
    // One person cannot follow three shops: from the third shop on, an HR manager is needed.
    if (OpenCount(State) >= 2 && !MarketStaff::HasHr(State)) { OutReason = TEXT("\u00dc\u00e7\u00fcnc\u00fc \u015fube i\u00e7in \u00f6nce bir \u0130K m\u00fcd\u00fcr\u00fc i\u015fe al."); return false; }
    const int64 Cost = OpeningCost(State, Products, District, Format);
    if (State.Cash < Cost) { OutReason = FString::Printf(TEXT("A\u00e7\u0131l\u0131\u015f i\u00e7in %s gerekiyor (depozito, tadilat, a\u00e7\u0131l\u0131\u015f sto\u011fu)."), *BranchTl(Cost)); return false; }
    return true;
}

bool MarketBranches::Open(FMarketState& State, const TArray<FMarketProduct>& Products, EDistrict District, const FString& Format, FString& OutMessage)
{
    if (!CanOpen(State, Products, District, Format, OutMessage)) return false;
    const FDistrict& Where = DistrictInfo(District);
    const FFormat& Kind = FormatInfo(Format);
    const double Level = MarketPrices::ListLevel(State.Day);
    FMarketBranch Branch;
    Branch.Name = FString::Printf(TEXT("Miras Market %s"), Where.Name);
    Branch.District = static_cast<uint8>(District);
    Branch.Format = Kind.Id;
    Branch.Stage = static_cast<uint8>(EStage::Renovation);
    Branch.StageUntil = State.Day + RenovationDays - 1;
    Branch.Rent = FMath::RoundToInt64(Where.Rent * Level);
    Branch.PriceIndex = FMath::Clamp(MainPriceIndex(State, Products), 0.85f, 1.2f);
    PlanShelves(State, Branch, Products);
    // The deposit leaves the till now and comes back when the branch closes; the fit-out is an expense of today
    // (paid at the day close with the other costs).
    State.Cash -= FMath::RoundToInt64(2 * Where.Rent * Level);
    State.OtherCosts += FMath::RoundToInt64(Kind.FitOut * Level);
    State.Branches.Add(Branch);
    State.bSecondStore = true;
    OutMessage = FString::Printf(TEXT("%s: kira s\u00f6zle\u015fmesi imzaland\u0131 (depozito %s), tadilat ba\u015flad\u0131 (%d g\u00fcn). Raflar senin kurallar\u0131nla otomatik planland\u0131."),
        *Branch.Name, *BranchTl(FMath::RoundToInt64(2 * Where.Rent * Level)), RenovationDays);
    return true;
}

bool MarketBranches::Close(FMarketState& State, int32 BranchIndex, FString& OutMessage)
{
    if (!State.Branches.IsValidIndex(BranchIndex) || State.Branches[BranchIndex].Stage == static_cast<uint8>(EStage::Closed)) { OutMessage = TEXT("B\u00f6yle bir \u015fube yok."); return false; }
    FMarketBranch& B = State.Branches[BranchIndex];
    B.Stage = static_cast<uint8>(EStage::Closed);
    // The deposit comes back; what is left on the shelves goes to the family shop's depot.
    State.Cash += 2 * B.Rent;
    for (const FMarketBranchItem& Item : B.Items)
        if (FMarketStock* Stock = State.Stock.FindByPredicate([&Item](const FMarketStock& S) { return S.Id == Item.ProductId; }))
            Stock->Warehouse = FMath::Min(FMarketState::StorageCapacity - Stock->Dock - Stock->Incoming, Stock->Warehouse + Item.Units);
    B.Items.Reset();
    State.bSecondStore = OpenCount(State) > 0;
    OutMessage = FString::Printf(TEXT("%s kapand\u0131. Depozito geri al\u0131nd\u0131, raftaki mal ana depoya ta\u015f\u0131nd\u0131."), *B.Name);
    return true;
}

bool MarketBranches::Promote(FMarketState& State, int32 EmployeeId, int32 BranchIndex, FString& OutMessage)
{
    FMarketEmployee* E = MarketStaff::FindEmployee(State, EmployeeId);
    if (!E || !State.Branches.IsValidIndex(BranchIndex) || State.Branches[BranchIndex].Stage == static_cast<uint8>(EStage::Closed))
    {
        OutMessage = TEXT("Terfi i\u00e7in bir \u00e7al\u0131\u015fan ve a\u00e7\u0131k bir \u015fube se\u00e7.");
        return false;
    }
    const MarketStaff::ERole Role = MarketStaff::RoleOf(*E);
    if (Role != MarketStaff::ERole::Cashier && Role != MarketStaff::ERole::Stocker) { OutMessage = TEXT("Yaln\u0131zca kasiyer ya da reyon g\u00f6revlisi \u015fube m\u00fcd\u00fcr\u00fc olabilir."); return false; }
    FMarketBranch& B = State.Branches[BranchIndex];
    B.ManagerName = E->Name;
    B.ManagerSkill = FMath::Min(100, E->Skill + 5); // knows the family's way of working
    B.ManagerHonesty = E->Honesty;
    B.ManagerWage = E->DailyWage * 14 / 10;
    OutMessage = FString::Printf(TEXT("%s art\u0131k %s m\u00fcd\u00fcr\u00fc (g\u00fcnl\u00fck %s)."), *E->Name, *B.Name, *BranchTl(B.ManagerWage));
    State.Staff.RemoveAll([EmployeeId](const FMarketEmployee& X) { return X.Id == EmployeeId; });
    MarketStaff::SyncCounts(State);
    return true;
}

void MarketBranches::Migrate(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    if (!State.bSecondStore || State.Branches.Num() > 0) return;
    FMarketBranch Branch;
    Branch.Name = TEXT("Miras Market \u00c7ar\u015f\u0131");
    Branch.District = static_cast<uint8>(EDistrict::Carsi);
    Branch.Format = TEXT("mahalle");
    Branch.Stage = static_cast<uint8>(EStage::Open);
    Branch.OpenedDay = FMath::Max(1, State.Day - MaturityDays);
    Branch.Maturity = 1.f;
    Branch.Rent = FMath::RoundToInt64(DistrictInfo(EDistrict::Carsi).Rent * MarketPrices::ListLevel(State.Day));
    Branch.Workers = FormatInfo(Branch.Format).Workers;
    Branch.ManagerName = ManagerNames[0];
    Branch.ManagerSkill = 55;
    Branch.ManagerWage = MarketStaff::FairWage(MarketStaff::ERole::HrManager, 55, State.Day) * 9 / 10;
    PlanShelves(State, Branch, Products);
    for (FMarketBranchItem& Item : Branch.Items) Item.Units = Item.Capacity;
    State.Branches.Add(Branch);
}

float MarketBranches::MainShopFactor(const FMarketState& State)
{
    float Factor = 1.f;
    const FDistrict& Home = DistrictInfo(EDistrict::Istasyon);
    for (const FMarketBranch& B : State.Branches)
        if (B.Stage == static_cast<uint8>(EStage::Open) && (Home.Neighbours & Bit(static_cast<EDistrict>(B.District)))) Factor *= 0.95f;
    return Factor;
}

FString MarketBranches::Summary(const FMarketState& State, int32 BranchIndex, const TArray<FMarketProduct>& Products)
{
    if (!State.Branches.IsValidIndex(BranchIndex)) return FString();
    const FMarketBranch& B = State.Branches[BranchIndex];
    switch (static_cast<EStage>(B.Stage))
    {
    case EStage::Renovation: return FString::Printf(TEXT("%s \u00b7 tadilat (%d. g\u00fcne kadar)"), *B.Name, B.StageUntil);
    case EStage::Permits: return FString::Printf(TEXT("%s \u00b7 ruhsat bekleniyor (%d. g\u00fcne kadar)"), *B.Name, B.StageUntil);
    case EStage::Hiring: return FString::Printf(TEXT("%s \u00b7 i\u015fe al\u0131m ve a\u00e7\u0131l\u0131\u015f sto\u011fu"), *B.Name);
    case EStage::Closed: return FString::Printf(TEXT("%s \u00b7 kapal\u0131"), *B.Name);
    default: break;
    }
    int32 Capacity = 0, Units = 0, Carried = 0;
    for (const FMarketBranchItem& Item : B.Items) { Capacity += Item.Capacity; Units += FMath::Min(Item.Units, Item.Capacity); if (Item.Capacity > 0) ++Carried; }
    return FString::Printf(TEXT("%s \u00b7 %d. g\u00fcn \u00b7 d\u00fcn %d m\u00fc\u015fteri, ciro %s, net %s \u00b7 raf %%%d dolu, %d \u00fcr\u00fcn \u00b7 m\u00fcd\u00fcr %s \u00b7 al\u0131\u015fkanl\u0131k %%%.0f"),
        *B.Name, State.Day - B.OpenedDay, B.LastShoppers, *BranchTl(B.LastRevenue), *BranchTl(B.LastProfit),
        Capacity > 0 ? Units * 100 / Capacity : 0, Carried, B.ManagerName.IsEmpty() ? TEXT("yok (senin talimatlar\u0131n)") : *FString::Printf(TEXT("%s, beceri %d"), *B.ManagerName, B.ManagerSkill),
        B.Maturity * 100.f);
}

void MarketBranches::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    Migrate(State, Products);
    const int32 Closed = State.Day - 1;
    if (Closed < 1) return;
    TArray<FString>& News = State.DayNews;
    const double Level = MarketPrices::ListLevel(State.Day);
    for (int32 Index = 0; Index < State.Branches.Num(); ++Index)
    {
        FMarketBranch& B = State.Branches[Index];
        const EStage Stage = static_cast<EStage>(B.Stage);
        const FDistrict& Where = DistrictInfo(static_cast<EDistrict>(B.District));
        const FFormat& Kind = FormatInfo(B.Format);
        if (Stage == EStage::Closed) continue;
        // Opening steps.
        if (Stage == EStage::Renovation && Closed >= B.StageUntil)
        {
            B.Stage = static_cast<uint8>(EStage::Permits);
            B.StageUntil = State.Day + PermitDays - 1 + (MarketStaff::HasAccountant(State) ? 0 : 3);
            News.Add(FString::Printf(TEXT("%s: tadilat bitti. Ruhsat i\u00e7in belediyeye ba\u015fvuruldu (%d. g\u00fcn).%s"), *B.Name, B.StageUntil,
                MarketStaff::HasAccountant(State) ? TEXT(" Necati Bey evraklar\u0131 haz\u0131rlad\u0131.") : TEXT(" Evraklar eksik gidince i\u015f uzad\u0131.")));
            continue;
        }
        if (Stage == EStage::Permits && Closed >= B.StageUntil)
        {
            B.Stage = static_cast<uint8>(EStage::Hiring);
            B.Workers = Kind.Workers;
            State.OtherCosts += MarketStaff::HireCost * B.Workers;
            if (B.ManagerName.IsEmpty())
            {
                const uint32 Roll = BranchMix(State.RivalSeed, Closed, 0xB4A7u + Index);
                B.ManagerName = ManagerNames[Roll % UE_ARRAY_COUNT(ManagerNames)];
                B.ManagerSkill = 35 + static_cast<int32>((Roll >> 8) % 46u);
                B.ManagerHonesty = (Roll >> 16) % 100u < 12u ? 20 : 60 + static_cast<int32>((Roll >> 20) % 40u);
                B.ManagerWage = MarketStaff::FairWage(MarketStaff::ERole::HrManager, B.ManagerSkill, State.Day) * 9 / 10;
            }
            News.Add(FString::Printf(TEXT("%s: ruhsat \u00e7\u0131kt\u0131. %d \u00e7al\u0131\u015fan i\u015fe al\u0131nd\u0131; m\u00fcd\u00fcr %s (beceri %d)."), *B.Name, B.Workers, *B.ManagerName, B.ManagerSkill));
            continue;
        }
        if (Stage == EStage::Hiring)
        {
            const int64 Bill = StockCost(B, Products);
            State.Cash -= Bill;
            State.Purchases += Bill;
            for (FMarketBranchItem& Item : B.Items) Item.Units = FMath::Max(Item.Units, Item.Capacity);
            B.Stage = static_cast<uint8>(EStage::Open);
            B.OpenedDay = State.Day;
            News.Add(FString::Printf(TEXT("%s YARIN A\u00c7ILIYOR! A\u00e7\u0131l\u0131\u015f sto\u011fu raflarda (%s). \u0130lk hafta merak edenler gelir; kal\u0131c\u0131 m\u00fc\u015fteri bir ayda olu\u015fur."), *B.Name, *BranchTl(Bill)));
            continue;
        }
        if (Stage != EStage::Open || Closed < B.OpenedDay) continue;

        // The morning delivery ordered at the last close.
        for (FMarketBranchItem& Item : B.Items) { Item.Units += Item.Incoming; Item.Incoming = 0; }

        // Shoppers: the district's trips x our share x the calendar x habit x our shops nearby.
        int32 Sold = 0, Empty = 0;
        for (const FMarketBranchItem& Item : B.Items) { Sold += Item.LastSold; Empty += Item.LastEmpty; }
        const float Availability = Sold + Empty > 0 ? FMath::Clamp(static_cast<float>(Sold) / (Sold + Empty), 0.2f, 1.f) : 0.9f;
        const float Service = Kind.Service * (B.ManagerName.IsEmpty() ? 0.9f : 1.f);
        const float Pull = FMath::Exp(-(B.PriceIndex - 1.f) / MarketCompetitors::PriceSensitivity) * Availability * Service *
            (0.8f + B.Satisfaction / 250.f) * (0.9f + 0.4f * B.Maturity) * (State.Day - B.OpenedDay < 7 ? 1.3f : 1.f);
        const float Share = Pull / (Pull + Where.RivalAttraction);
        float Trips = Where.Shoppers * MarketCalendar::TrafficFactor(Closed, State.RivalSeed);
        const MarketCalendar::FDate Date = MarketCalendar::DateOf(Closed);
        if (static_cast<EDistrict>(B.District) == EDistrict::Universite && Date.Month * 100 + Date.Day >= 615 && Date.Month * 100 + Date.Day < 915) Trips *= 0.5f;
        if (static_cast<EDistrict>(B.District) == EDistrict::YeniMahalle) Trips *= 1.f + 0.03f * (Date.Year - MarketCalendar::StartYear);
        const int32 Shoppers = FMath::RoundToInt32(Trips * Share * (0.5f + 0.5f * B.Maturity) * Cannibalization(State, Index, static_cast<EDistrict>(B.District)));

        // What they want, what is on the shelf.
        const TArray<float> Wish = Wishes(State, Products, Where, Closed);
        int64 Revenue = 0, Cogs = 0;
        int32 DayEmpty = 0, DaySold = 0;
        for (int32 I = 0; I < Products.Num(); ++I)
        {
            FMarketBranchItem* Item = ItemOf(B, Products[I].Id);
            if (!Item) continue;
            const int32 Want = FMath::RoundToInt32(Shoppers * UnitsPerShopper * Wish[I] * Where.Income);
            const int32 Take = Item->Capacity > 0 ? FMath::Min(Want, Item->Units) : 0;
            Item->Units -= Take;
            Item->LastSold = Take;
            Item->LastEmpty = Want - Take;
            DaySold += Take;
            DayEmpty += Want - Take;
            const int64 Price = FMath::Max<int64>(5, FMath::RoundToInt64(Products[I].BasePrice * B.PriceIndex / 5.0) * 5);
            Revenue += Price * Take;
            Cogs += Products[I].Cost * Take;
        }
        // A dishonest manager keeps a little of the till.
        const int64 Skim = B.ManagerHonesty < 35 ? Revenue * 15 / 1000 : 0;
        const int64 Opex = FMath::RoundToInt64(B.Rent * MarketPrices::ListLevel(State.Day) / MarketPrices::ListLevel(FMath::Max(1, B.OpenedDay)) / 30.0)
            + B.Workers * MarketStaff::FairWage(MarketStaff::ERole::Cashier, 50, State.Day) + B.ManagerWage
            + FMath::RoundToInt64(1500 * Level * (B.Format == TEXT("buyuk") ? 2.0 : B.Format == TEXT("kucuk") ? 0.7 : 1.0));
        const int64 Profit = Revenue - Skim - Cogs - Opex;
        State.Cash += Revenue - Skim - Opex;     // goods were paid when ordered; the family shop's till stays separate
        State.LastBranchProfit += Profit;
        State.LastProfit += Profit;
        B.LastRevenue = Revenue;
        B.LastProfit = Profit;
        B.LastShoppers = Shoppers;
        B.WeekProfit += Profit;
        const float DayAvailability = DaySold + DayEmpty > 0 ? static_cast<float>(DaySold) / (DaySold + DayEmpty) : 1.f;
        B.Satisfaction = FMath::Clamp(B.Satisfaction + ((50.f + 40.f * DayAvailability - 100.f * (B.PriceIndex - 1.f)) - B.Satisfaction) * 0.1f, 0.f, 100.f);
        B.Maturity = FMath::Min(1.f, B.Maturity + 1.f / MaturityDays);

        // The manager's order for tomorrow (a skilled one is closer to the real demand) and prices.
        const float Error = (100 - FMath::Clamp(B.ManagerSkill, 0, 100)) / 100.f * 0.4f;
        const float Tomorrow = MarketCalendar::TrafficFactor(State.Day, State.RivalSeed) / FMath::Max(0.3f, MarketCalendar::TrafficFactor(Closed, State.RivalSeed));
        const bool bTight = State.Cash < Opex * 3; // short of money: the manager orders half
        int64 Bill = 0;
        for (int32 I = 0; I < Products.Num(); ++I)
        {
            FMarketBranchItem* Item = ItemOf(B, Products[I].Id);
            if (!Item || Item->Capacity <= 0) continue;
            const float Noise = (BranchMix(State.RivalSeed, Closed, 0x0DE7u + Index * 131u + I) % 2001u) / 1000.f - 1.f;
            const float Expected = (Item->LastSold + Item->LastEmpty) * Tomorrow * (1.f + Error * Noise);
            const int32 Target = FMath::Min(FMath::RoundToInt32(Item->Capacity * 1.5f), FMath::Max(Item->Capacity, FMath::RoundToInt32(Expected * 1.2f)));
            int32 Order = FMath::Max(0, Target - Item->Units);
            if (bTight) Order /= 2;
            Item->Incoming = Order;
            Bill += Products[I].Cost * Order;
        }
        State.Cash -= Bill;
        State.Purchases += Bill;
        MarketSuppliers::Account(State, MarketSuppliers::Current(State)).Volume30 += Bill; // more shops, better purchase terms
        if (B.ManagerSkill >= 60)
        {
            // A good manager keeps prices a little above the rivals' and away from price wars.
            const float Rival = MarketCompetitors::RivalPriceFactor(State, FString(), TArray<FString>());
            B.PriceIndex = FMath::Clamp(B.PriceIndex + (Rival * 1.02f - B.PriceIndex) * 0.2f, 0.85f, 1.2f);
        }
        else B.PriceIndex = FMath::Clamp(MainPriceIndex(State, Products), 0.85f, 1.2f); // copies the family shop's labels

        if (Closed % 7 == 0)
        {
            News.Add(FString::Printf(TEXT("%s haftas\u0131: net %s, d\u00fcn %d m\u00fc\u015fteri, raf dolulu\u011fu %%%.0f.%s"), *B.Name, *BranchTl(B.WeekProfit), Shoppers, DayAvailability * 100.f,
                Skim > 0 && MarketStaff::HasAccountant(State) ? TEXT(" Necati Bey: \"\u015eubenin kasas\u0131 sat\u0131\u015flarla tutmuyor.\"") : TEXT("")));
            B.WeekProfit = 0;
        }
    }
}

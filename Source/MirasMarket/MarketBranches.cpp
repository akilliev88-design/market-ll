#include "MarketBranches.h"
#include "MarketLedger.h"
#include "MarketCountry.h"
#include "MarketCalendar.h"
#include "MarketCampaign.h"
#include "MarketCompany.h"
#include "MarketCompetitors.h"
#include "MarketDepots.h"
#include "MarketCustomers.h"
#include "MarketGoods.h"
#include "MarketLayout.h"
#include "MarketManagers.h"
#include "MarketPrices.h"
#include "MarketStaff.h"
#include "MarketOnline.h"
#include "MarketAdvertising.h"
#include "MarketStart.h"
#include "MarketStory.h"
#include "MarketStoreAssign.h"
#include "MarketStoreViews.h"
#include "MarketChains.h"
#include "MarketSourcing.h"
#include "MarketDepartments.h"
#include "MarketSuppliers.h"
#include "MarketPromotions.h"
#include "MarketTuning.h"
#include "MarketSimulation.h"

namespace MarketBranches
{
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
        return MarketCountry::Money(Kurus); // G-084: the active country's currency
    }


    // Shoppers of a province by segment (retired, families, workers, students, tradesmen, singles): one mix for
    // every province; the market type decides who comes.
    const int32 DefaultMix[6] = { 15, 32, 25, 10, 10, 8 };

    bool IsOpenStage(const FMarketBranch& B) { return B.Stage == static_cast<uint8>(EStage::Open); }
    bool IsLive(const FMarketBranch& B) { return B.Stage != static_cast<uint8>(EStage::Closed); }

    bool SameSite(const FMarketState& State, const FMarketBranch& B, const FString& Country, const FString& Province)
    {
        return CountryOf(State, B) == Country && (B.Province.IsEmpty() ? MarketStart::HomeProvince(State) : B.Province) == Province;
    }

    // Our other open shops in the province take customers once the province is full.
    float Cannibalization(const FMarketState& State, int32 Self, const FSite& Site)
    {
        float Others = Site.bHome ? 1.f : 0.f; // the family shop
        for (int32 I = 0; I < State.Branches.Num(); ++I)
        {
            const FMarketBranch& B = State.Branches[I];
            if (I != Self && IsOpenStage(B) && SameSite(State, B, Site.Country, Site.Province)) Others += FormatInfo(B.Format).Weight;
        }
        const float Slots = FMath::Max(1.f, static_cast<float>(Site.PopulationK) / PeoplePerSlotK);
        return 1.f / (1.f + 0.6f * Others / Slots);
    }

    // Share of the province's wishes for each product (segment mix x taste x calendar).
    TArray<float> Wishes(const FMarketState& State, const TArray<FMarketProduct>& Products, int32 Day)
    {
        TArray<float> Weights;
        float Total = 0.f;
        for (const FMarketProduct& P : Products)
        {
            const MarketGoods::EGroup Group = MarketGoods::Classify(P.Category);
            float Taste = 0.f;
            for (int32 S = 0; S < 6; ++S) Taste += DefaultMix[S] / 100.f * MarketCustomers::Profile(static_cast<MarketCustomers::ESegment>(S)).Preference[static_cast<int32>(Group)];
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

    float TripsOf(const FSite& Site, const FFormat& Kind)
    {
        // Bigger provinces are denser: a little more traffic per shop.
        const float Size = FMath::Clamp(FMath::Pow(FMath::Max(1.f, static_cast<float>(Site.PopulationK)) / 341.f, 0.1f), 0.85f, 1.3f);
        return Kind.Trips * Size;
    }

    void PlanShelves(const FMarketState& State, FMarketBranch& Branch, const TArray<FMarketProduct>& Products)
    {
        const FSite Site = SiteOf(State, Branch);
        const FFormat& Kind = FormatInfo(Branch.Format);
        const TArray<float> Wish = Wishes(State, Products, FMath::Max(1, State.Day));
        TArray<float> Demand;
        for (const float W : Wish) Demand.Add(W * TripsOf(Site, Kind) * 0.3f * UnitsPerShopper);
        FMarketPlanogram Plan = MarketLayout::Fixtures(Branch.Format);
        MarketLayout::Plan(Plan, Products, Demand);
        const TArray<int32> Capacities = MarketLayout::Capacities(Plan, Products);
        const float Variety = MarketStoreAssign::VarietyFactor(MarketStoreViews::MeasuresOf(Branch), Branch.Format);
        Branch.Items.Reset();
        for (int32 I = 0; I < Products.Num(); ++I)
        {
            FMarketBranchItem Item;
            Item.ProductId = Products[I].Id;
            Item.Capacity = Capacities.IsValidIndex(I) ? Capacities[I] : 0;
            // G-088 C: the signed store's shelf front against the type's nominal store (0.6..1.5).
            if (Item.Capacity > 0) Item.Capacity = FMath::Max(1, FMath::RoundToInt32(Item.Capacity * Variety));
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

    int64 MonthlyRent(const FMarketState& State, const FSite& Site, const FFormat& Kind, double Level)
    {
        return FMath::RoundToInt64(Kind.Rent * Site.Rent * Level * MarketSimulation::CapitalFactor(State)); // C12: the difficulty
    }

    FString NameFor(const FMarketState& State, const FSite& Site, const FFormat& Kind)
    {
        int32 Same = 0;
        for (const FMarketBranch& B : State.Branches)
            if (SameSite(State, B, Site.Country, Site.Province) && B.Format == Kind.Id) ++Same;
        return FString::Printf(TEXT("%s \u00b7 %s %d"), *Site.Name, Kind.Short, Same + 1);
    }

}

const TArray<FString>& MarketBranches::FormatIds()
{
    static const TArray<FString> Ids = { TEXT("kucuk"), TEXT("mahalle"), TEXT("buyuk"), TEXT("hiper") };
    return Ids;
}

const MarketBranches::FFormat& MarketBranches::FormatInfo(const FString& Id)
{
    //                                 id               name                                     short                    fit-out  wk  service price trips rent    weight run   pop  depot chapter
    // C12 (M42, C11 bot: a supermarket paid its fit-out back in 1-2 months at a 16 % net margin, a hypermarket in 4;
    // money stopped being a limit after 15 shops): large stores' rent x1.5 and a fit-out of about half a year's
    // profit, the supermarket at the rivals' price level. C12b: the small shops keep their rent and fit-out (the
    // first try, rent x1.5 everywhere, left a neighbourhood shop ~1 % net and the early game stalled at 3 shops).
    // C12c: the hypermarket's fit-out 100 000 -> 70 000 TL (a year's payback held the balanced player back).
    static const FFormat Discount = { TEXT("kucuk"), TEXT("ucuzcu (indirim marketi)"), TEXT("Ucuzcu"), 250000, 3, 0.9f, 0.94f, 300, 45000, 1.f, 0.7f, 0, false, 0 };
    static const FFormat Neighbourhood = { TEXT("mahalle"), TEXT("mahalle marketi"), TEXT("Mahalle"), 400000, 3, 1.0f, 1.0f, 240, 60000, 1.f, 1.f, 0, false, 0 };
    static const FFormat Super = { TEXT("buyuk"), TEXT("s\u00fcpermarket"), TEXT("S\u00fcpermarket"), 3000000, 6, 1.1f, 1.0f, 520, 225000, 1.5f, 2.f, 0, false, 0 };
    static const FFormat Hyper = { TEXT("hiper"), TEXT("hipermarket"), TEXT("Hipermarket"), 7000000, 14, 1.15f, 0.98f, 1600, 750000, 3.f, 3.f, 500, true, 5 }; // C3 (A6): 20 -> 14 workers, running 5 -> 3
    return Id == TEXT("kucuk") ? Discount : Id == TEXT("buyuk") ? Super : Id == TEXT("hiper") ? Hyper : Neighbourhood;
}

FString MarketBranches::CountryOf(const FMarketState& State, const FMarketBranch& Branch)
{
    return Branch.Country.IsEmpty() ? State.CountryId : Branch.Country;
}

MarketBranches::FSite MarketBranches::SiteOf(const FMarketState& State, const FString& Country, const FString& Province)
{
    FSite Site;
    Site.Country = Country.IsEmpty() ? State.CountryId : Country;
    Site.Province = Province;
    Site.bAbroad = Site.Country != State.CountryId;
    Site.bHome = !Site.bAbroad && Province == MarketStart::HomeProvince(State);
    if (const MarketCountry::FCity* City = MarketCountry::FindCity(Site.Country, Province))
    {
        Site.bValid = true;
        Site.Name = City->Name;
        Site.SubRegion = City->SubRegion;
        Site.PopulationK = City->PopulationK;
        Site.Income = FMath::Clamp(City->Income, 0.5f, 2.f);
        Site.Rent = FMath::Clamp(City->Rent, 0.4f, 2.f);
        Site.Competition = FMath::Clamp(City->Competition, 0.5f, 2.f);
    }
    else
    {
        // Unknown province (a pack changed under an older save): the reference values.
        Site.Name = Province;
        Site.PopulationK = 341;
    }
    return Site;
}

MarketBranches::FSite MarketBranches::SiteOf(const FMarketState& State, const FMarketBranch& Branch)
{
    return SiteOf(State, CountryOf(State, Branch), Branch.Province.IsEmpty() ? MarketStart::HomeProvince(State) : Branch.Province);
}

int32 MarketBranches::Room(const FSite& Site)
{
    return FMath::Max(2, Site.PopulationK / PeoplePerStoreK);
}

int32 MarketBranches::ShopsIn(const FMarketState& State, const FString& Country, const FString& Province)
{
    const FString C = Country.IsEmpty() ? State.CountryId : Country;
    int32 Count = C == State.CountryId && Province == MarketStart::HomeProvince(State) ? 1 : 0;
    for (const FMarketBranch& B : State.Branches) if (IsLive(B) && SameSite(State, B, C, Province)) ++Count;
    return Count;
}

TArray<FString> MarketBranches::ProvincesWithShops(const FMarketState& State, const FString& Country)
{
    const FString C = Country.IsEmpty() ? State.CountryId : Country;
    TArray<FString> List;
    if (C == State.CountryId) List.Add(MarketStart::HomeProvince(State));
    for (const FMarketBranch& B : State.Branches)
        if (IsOpenStage(B) && CountryOf(State, B) == C) List.AddUnique(B.Province.IsEmpty() ? MarketStart::HomeProvince(State) : B.Province);
    return List;
}

int32 MarketBranches::EncodeSite(const FString& Country, const FString& Province, const FString& Format)
{
    const TArray<MarketCountry::FProfile>& All = MarketCountry::All();
    const int32 C = All.IndexOfByPredicate([&Country](const MarketCountry::FProfile& P) { return P.Id == Country; });
    if (C == INDEX_NONE) return INDEX_NONE;
    const int32 P = All[C].Cities.IndexOfByPredicate([&Province](const MarketCountry::FCity& City) { return City.Id == Province; });
    const int32 F = FormatIds().IndexOfByKey(Format);
    if (P == INDEX_NONE || P >= 1000 || F == INDEX_NONE) return INDEX_NONE;
    return (C * 1000 + P) * 10 + F;
}

bool MarketBranches::DecodeSite(int32 Arg, FString& OutCountry, FString& OutProvince, FString& OutFormat)
{
    if (Arg < 0) return false;
    const TArray<MarketCountry::FProfile>& All = MarketCountry::All();
    const int32 F = Arg % 10, P = (Arg / 10) % 1000, C = Arg / 10000;
    if (!All.IsValidIndex(C) || !All[C].Cities.IsValidIndex(P) || !FormatIds().IsValidIndex(F)) return false;
    OutCountry = All[C].Id;
    OutProvince = All[C].Cities[P].Id;
    OutFormat = FormatIds()[F];
    return true;
}

bool MarketBranches::IsFirstBranch(const FMarketState& State, const FSite& Site, const FFormat& Kind)
{
    return State.Branches.Num() == 0 && Site.bHome && FString(Kind.Id) == TEXT("mahalle");
}

int64 MarketBranches::FitOutCost(const FMarketState& State, const FSite& Site, const FFormat& Kind, float MeasureFactor)
{
    // C12 (M42): the difficulty scales the money a shop needs; the very first branch, a neighbourhood shop in the
    // home province, is a neighbour's empty shop that needs little work (the first milestone comes in months 4-8).
    const double First = IsFirstBranch(State, Site, Kind) ? FirstBranchFitOut : 1.0;
    return FMath::RoundToInt64(Kind.FitOut * MarketPrices::ListLevel(State.Day) * MeasureFactor * MarketSimulation::CapitalFactor(State) * First);
}

int64 MarketBranches::MonthlyFixedCost(const FMarketState& State, const FString& Country, const FString& Province, const FString& Format)
{
    const double Level = MarketPrices::ListLevel(State.Day);
    const FFormat& Kind = FormatInfo(Format);
    FMarketBranch Probe;
    Probe.Country = Country.IsEmpty() ? State.CountryId : Country;
    Probe.Province = Province;
    Probe.Format = Kind.Id;
    MarketStoreViews::PreviewTo(State, Probe);
    const FSite Site = SiteOf(State, Probe);
    const MarketStoreAssign::FStoreMeasures Measures = MarketStoreViews::MeasuresOf(Probe);
    const int64 Rent = FMath::RoundToInt64(MonthlyRent(State, Site, Kind, Level) * MarketStoreAssign::RentFactor(Measures, Kind.Id));
    // The same lines as the branch's day (CloseDay): a middling cashier per worker and a store manager.
    const int64 Wages = MarketStoreAssign::WorkersFor(Measures, Kind.Id) * MarketStaff::FairWage(MarketStaff::ERole::Cashier, 50, State.Day) + MarketStaff::FairWage(MarketStaff::ERole::HrManager, 55, State.Day) * 9 / 10;
    const int64 Running = FMath::RoundToInt64(1500 * Level * Kind.Running);
    return Rent + 30 * (Wages + MarketStaff::EmployerShare(Wages) + Running);
}

int64 MarketBranches::OpeningCost(const FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Country, const FString& Province, const FString& Format)
{
    const double Level = MarketPrices::ListLevel(State.Day);
    const FFormat& Kind = FormatInfo(Format);
    FMarketBranch Probe;
    Probe.Country = Country.IsEmpty() ? State.CountryId : Country;
    Probe.Province = Province;
    Probe.Format = Kind.Id;
    MarketStoreViews::PreviewTo(State, Probe); // G-088 C: the store this site would get
    PlanShelves(State, Probe, Products);
    const FSite Site = SiteOf(State, Probe);
    const MarketStoreAssign::FStoreMeasures Measures = MarketStoreViews::MeasuresOf(Probe);
    return 2 * FMath::RoundToInt64(MonthlyRent(State, Site, Kind, Level) * MarketStoreAssign::RentFactor(Measures, Kind.Id))
        + FitOutCost(State, Site, Kind, MarketStoreAssign::FitOutFactor(Measures, Kind.Id)) + StockCost(Probe, Products);
}

int32 MarketBranches::OpenCount(const FMarketState& State)
{
    int32 Count = 0;
    for (const FMarketBranch& B : State.Branches) if (IsLive(B)) ++Count;
    return Count;
}

bool MarketBranches::CanOpen(const FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Country, const FString& Province, const FString& Format, FString& OutReason)
{
    const FSite Site = SiteOf(State, Country, Province);
    const FFormat& Kind = FormatInfo(Format);
    if (!Site.bValid) { OutReason = TEXT("Bu il bilinmiyor."); return false; }
    if (State.Day < State.RescueUntil) { OutReason = FString::Printf(TEXT("Kurtarma plan\u0131 s\u00fcr\u00fcyor: %d g\u00fcn daha yeni \u015fube yok."), State.RescueUntil - State.Day); return false; } // C7
    if (ShopsIn(State, Site.Country, Site.Province) >= Room(Site)) { OutReason = FString::Printf(TEXT("%s'de yeni ma\u011faza i\u00e7in yer kalmad\u0131 (en \u00e7ok %d)."), *Site.Name, Room(Site)); return false; }
    if (OpenCount(State) == 0)
    {
        // The first branch keeps the chapter-2 goals (the money is checked below against the real opening cost).
        if (MarketCampaign::DebtOpen(State)) { OutReason = TEXT("\u00d6nce i\u015fletmenin borcunu kapat."); return false; }
        if (State.ProfitableDays < MarketCampaign::ExpandProfitableDays) { OutReason = FString::Printf(TEXT("\u00d6nce %d k\u00e2rl\u0131 g\u00fcn."), MarketCampaign::ExpandProfitableDays); return false; }
        if (State.MarketShare < MarketCampaign::ExpandShare) { OutReason = FString::Printf(TEXT("\u00d6nce yerel pay %%%.0f."), MarketCampaign::ExpandShare); return false; }
    }
    // One person cannot follow three shops: from the third shop on, an HR manager is needed.
    if (OpenCount(State) >= 2 && !MarketStaff::HasHr(State)) { OutReason = TEXT("\u00dc\u00e7\u00fcnc\u00fc ma\u011faza i\u00e7in \u00f6nce bir \u0130K m\u00fcd\u00fcr\u00fc i\u015fe al."); return false; }
    // Beyond the home province the shop is a company: people who run it (karar, 03_MAGAZA_AGI \u00a73).
    if (!Site.bHome && (!MarketStaff::HasHr(State) || !MarketStaff::HasAccountant(State)))
    {
        OutReason = TEXT("Ev ilinin d\u0131\u015f\u0131nda ma\u011faza i\u00e7in \u0130K m\u00fcd\u00fcr\u00fc ve mali m\u00fc\u015favir gerekir.");
        return false;
    }
    if (Site.bAbroad && !MarketCompany::ChapterOpen(State, 6)) { OutReason = FString::Printf(TEXT("Yurt d\u0131\u015f\u0131 i\u00e7in \"%s\" b\u00f6l\u00fcm\u00fc a\u00e7\u0131lmal\u0131."), *MarketStory::ChapterTitle(6)); return false; }
    if (Kind.Chapter > 0 && !MarketCompany::ChapterOpen(State, Kind.Chapter)) { OutReason = FString::Printf(TEXT("%s i\u00e7in \"%s\" b\u00f6l\u00fcm\u00fc a\u00e7\u0131lmal\u0131."), Kind.Short, *MarketStory::ChapterTitle(Kind.Chapter)); return false; }
    if (Site.PopulationK < Kind.MinPopulationK) { OutReason = FString::Printf(TEXT("%s yaln\u0131z n\u00fcfusu %d binin \u00fcst\u00fcndeki illere a\u00e7\u0131l\u0131r."), Kind.Short, Kind.MinPopulationK); return false; }
    float DepotKm = 0.f; // G-089: a depot of the country within range (the home province's short range does not apply)
    if (Kind.bNeedsDepot && MarketDepots::Nearest(State, Site.Country, Site.Province, false, DepotKm) == INDEX_NONE) { OutReason = FString::Printf(TEXT("%s i\u00e7in %d km i\u00e7inde bir depo gerekir."), Kind.Short, MarketDepots::RangeKm); return false; }
    const int64 Cost = OpeningCost(State, Products, Site.Country, Site.Province, Kind.Id);
    if (State.Cash < Cost) { OutReason = FString::Printf(TEXT("A\u00e7\u0131l\u0131\u015f i\u00e7in %s gerekiyor (depozito, tadilat, a\u00e7\u0131l\u0131\u015f sto\u011fu)."), *BranchTl(Cost)); return false; }
    return true;
}

bool MarketBranches::Open(FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Country, const FString& Province, const FString& Format, FString& OutMessage)
{
    if (!CanOpen(State, Products, Country, Province, Format, OutMessage)) return false;
    const FSite Site = SiteOf(State, Country, Province);
    const FFormat& Kind = FormatInfo(Format);
    const double Level = MarketPrices::ListLevel(State.Day);
    FMarketBranch Branch;
    Branch.Country = Site.Country;
    Branch.Province = Site.Province;
    Branch.Name = NameFor(State, Site, Kind);
    Branch.Format = Kind.Id;
    Branch.Stage = static_cast<uint8>(EStage::Renovation);
    Branch.StageUntil = State.Day + RenovationDays - 1;
    // G-088 C: the site's ready-made store (saved for the province and type); its size sets rent and fit-out.
    MarketStoreViews::AssignTo(State, Branch);
    const MarketStoreAssign::FStoreMeasures Measures = MarketStoreViews::MeasuresOf(Branch);
    Branch.Rent = FMath::RoundToInt64(MonthlyRent(State, Site, Kind, Level) * MarketStoreAssign::RentFactor(Measures, Kind.Id));
    Branch.PriceIndex = FMath::Clamp(Kind.PriceTarget, 0.85f, 1.2f);
    PlanShelves(State, Branch, Products);
    // The deposit leaves the till now and comes back when the branch closes; the fit-out is an expense of today
    // (paid at the day close with the other costs).
    State.Cash -= 2 * Branch.Rent;
    MarketLedger::Post(State, MarketLedger::EAccount::Investment, -2 * Branch.Rent, true, State.Branches.Num()); // C3: the deposit
    const bool bFirst = IsFirstBranch(State, Site, Kind);
    MarketLedger::AddStoreCost(State, FitOutCost(State, Site, Kind, MarketStoreAssign::FitOutFactor(Measures, Kind.Id)), State.Branches.Num()); // C10: the branch's own books
    State.Branches.Add(Branch);
    State.bSecondStore = true;
    OutMessage = FString::Printf(TEXT("%s: kira s\u00f6zle\u015fmesi imzaland\u0131 (depozito %s), tadilat ba\u015flad\u0131 (%d g\u00fcn). Raflar senin kurallar\u0131nla otomatik planland\u0131."),
        *Branch.Name, *BranchTl(2 * Branch.Rent), RenovationDays);
    if (bFirst) OutMessage += TEXT(" Kom\u015fu esnaf\u0131n bo\u015falan d\u00fckk\u00e2n\u0131: raflar\u0131 ve tezg\u00e2h\u0131 duruyor, tadilat ucuza geldi.");
    if (Site.bAbroad && !State.Branches.ContainsByPredicate([&State, &Site](const FMarketBranch& B) { return &B != &State.Branches.Last() && CountryOf(State, B) == Site.Country; }))
    {
        const MarketCountry::FProfile* Pack = MarketCountry::Find(Site.Country);
        MarketStory::AddMemory(State, FString::Printf(TEXT("%s: yurt d\u0131\u015f\u0131nda ilk ma\u011faza"), Pack ? *Pack->Name : *Site.Country));
    }
    return true;
}

int32 MarketBranches::AddAcquired(FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Country, const FString& Province, const FString& Format)
{
    const FSite Site = SiteOf(State, Country, Province);
    if (!Site.bValid || ShopsIn(State, Site.Country, Site.Province) >= Room(Site)) return INDEX_NONE;
    const FFormat& Kind = FormatInfo(Format);
    FMarketBranch Branch;
    Branch.Country = Site.Country;
    Branch.Province = Site.Province;
    Branch.Name = NameFor(State, Site, Kind);
    Branch.Format = Kind.Id;
    Branch.Stage = static_cast<uint8>(EStage::Open);
    Branch.StageUntil = State.Day;
    Branch.OpenedDay = State.Day;
    MarketStoreViews::AssignTo(State, Branch);
    const MarketStoreAssign::FStoreMeasures Measures = MarketStoreViews::MeasuresOf(Branch);
    Branch.Rent = FMath::RoundToInt64(MonthlyRent(State, Site, Kind, MarketPrices::ListLevel(State.Day)) * MarketStoreAssign::RentFactor(Measures, Kind.Id));
    Branch.PriceIndex = FMath::Clamp(Kind.PriceTarget, 0.85f, 1.2f);
    Branch.Workers = MarketStoreAssign::WorkersFor(Measures, Kind.Id);
    Branch.Maturity = 0.6f;       // the district already shops there
    Branch.Satisfaction = 60.f;
    PlanShelves(State, Branch, Products);
    for (FMarketBranchItem& Item : Branch.Items) Item.Units = Item.Capacity; // the goods came with the chain
    State.Branches.Add(Branch);
    State.bSecondStore = true;
    const int32 Index = State.Branches.Num() - 1;
    MarketManagers::HireStoreManager(State, Index);
    return Index;
}

bool MarketBranches::Close(FMarketState& State, const TArray<FMarketProduct>& Products, int32 BranchIndex, FString& OutMessage)
{
    if (!State.Branches.IsValidIndex(BranchIndex) || State.Branches[BranchIndex].Stage == static_cast<uint8>(EStage::Closed)) { OutMessage = TEXT("B\u00f6yle bir \u015fube yok."); return false; }
    FMarketBranch& B = State.Branches[BranchIndex];
    B.Stage = static_cast<uint8>(EStage::Closed);
    // The deposit comes back; what is left on the shelves goes to the family shop's depot.
    State.Cash += 2 * B.Rent;
    MarketLedger::Post(State, MarketLedger::EAccount::Divestment, 2 * B.Rent, true, BranchIndex); // C3: the deposit back
    // Shelf units and the paid delivery still on the way come to the family shop's depot as far as it has room;
    // the rest is sold to the wholesaler at half price.
    int32 Moved = 0, Sold = 0;
    int64 SoldValue = 0;
    for (const FMarketBranchItem& Item : B.Items)
    {
        const int32 Units = Item.Units + Item.Incoming;
        if (Units <= 0) continue;
        FMarketStock* Stock = State.Stock.FindByPredicate([&Item](const FMarketStock& S) { return S.Id == Item.ProductId; });
        const int32 Room = Stock ? FMath::Max(0, FMarketState::StorageCapacity - Stock->Warehouse - Stock->Dock - Stock->Incoming) : 0;
        const int32 Take = FMath::Min(Units, Room);
        const FMarketProduct* Product = Products.FindByPredicate([&Item](const FMarketProduct& P) { return P.Id == Item.ProductId; });
        if (Stock)
        {
            Stock->Warehouse += Take;
            // C3 (B #43): perishable goods come as an old batch (a third of their life left), not as a fresh delivery.
            const int32 Life = Product ? MarketGoods::ShelfLifeDays(*Product) : 0;
            if (Life > 0 && Take > 0) { FMarketBatch Old; Old.ProductId = Stock->Id; Old.Units = Take; Old.ExpiresDay = State.Day + FMath::Max(1, Life / 3); State.Batches.Add(Old); }
            else Stock->Received += Take;
        }
        Moved += Take;
        const int32 Left = Units - Take;
        if (Left <= 0) continue;
        const int64 Value = Product ? static_cast<int64>(Left) * Product->Cost / 2 : 0;
        Sold += Left;
        SoldValue += Value;
        State.PendingLoss += Value; // half of the cost is lost
    }
    State.Cash += SoldValue;
    MarketLedger::Post(State, MarketLedger::EAccount::Divestment, SoldValue, true, BranchIndex);
    MarketLedger::Post(State, MarketLedger::EAccount::Shrinkage, -SoldValue, false, BranchIndex); // the lost half (sold at half the cost)
    MarketDepartments::CloseAll(State, BranchIndex); // M26: its departments sell their stock off
    B.Items.Reset();
    State.bSecondStore = OpenCount(State) > 0;
    OutMessage = FString::Printf(TEXT("%s kapand\u0131. Depozito geri al\u0131nd\u0131, %d \u00fcr\u00fcn ana depoya ta\u015f\u0131nd\u0131."), *B.Name, Moved);
    if (Sold > 0) OutMessage += FString::Printf(TEXT(" Depoya s\u0131\u011fmayan %d \u00fcr\u00fcn toptanc\u0131ya yar\u0131 fiyat\u0131na verildi (%s)."), Sold, *BranchTl(SoldValue));
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
    // G-086b ek (M22): the manager he replaces and he himself are used names (never candidates later).
    if (!B.ManagerName.IsEmpty()) State.Management.UsedNames.AddUnique(B.ManagerName);
    State.Management.UsedNames.AddUnique(E->Name);
    B.ManagerName = E->Name;
    B.ManagerSkill = FMath::Min(100, E->Skill + 5); // knows the family's way of working
    B.ManagerHonesty = E->Honesty;
    B.ManagerWage = E->DailyWage * 14 / 10;
    MarketManagers::InitStoreManager(State, B, BranchIndex); // G-086b
    OutMessage = FString::Printf(TEXT("%s art\u0131k %s m\u00fcd\u00fcr\u00fc (ayda %s)."), *E->Name, *B.Name, *BranchTl(B.ManagerWage * 30));
    State.Staff.RemoveAll([EmployeeId](const FMarketEmployee& X) { return X.Id == EmployeeId; });
    MarketStaff::SyncCounts(State);
    return true;
}

float MarketBranches::MainShopFactor(const FMarketState& State)
{
    const FString Home = MarketStart::HomeProvince(State);
    const FSite Site = SiteOf(State, State.CountryId, Home);
    float Weight = 0.f;
    for (const FMarketBranch& B : State.Branches)
        if (IsOpenStage(B) && SameSite(State, B, State.CountryId, Home)) Weight += FormatInfo(B.Format).Weight;
    const float Slots = FMath::Max(1.f, static_cast<float>(Site.PopulationK) / PeoplePerSlotK);
    return 1.f / (1.f + 0.25f * Weight / Slots);
}

FString MarketBranches::Grade(const FMarketState& State, int32 BranchIndex)
{
    if (!State.Branches.IsValidIndex(BranchIndex)) return TEXT("-");
    const FMarketBranch& B = State.Branches[BranchIndex];
    if (!IsOpenStage(B) || State.Day - B.OpenedDay < 7) return TEXT("-");
    int32 Sold = 0, Empty = 0;
    for (const FMarketBranchItem& Item : B.Items) { Sold += Item.LastSold; Empty += Item.LastEmpty; }
    const float Availability = Sold + Empty > 0 ? static_cast<float>(Sold) / (Sold + Empty) : 1.f;
    const double DailyRent = static_cast<double>(FMath::Max<int64>(1, B.Rent)) / 30.0;
    const float Money = FMath::Clamp(0.5f + static_cast<float>(B.Last30Profit / (30.0 * DailyRent * 4.0)), 0.f, 1.f);
    // G-086b: a province manager's oversight lifts the marks a little.
    const float Score = 0.4f * Availability + 0.3f * B.Satisfaction / 100.f + 0.3f * Money + MarketManagers::GradeBonus(State, BranchIndex);
    return Score >= 0.8f ? TEXT("A") : Score >= 0.66f ? TEXT("B") : Score >= 0.52f ? TEXT("C") : TEXT("D");
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
    return FString::Printf(TEXT("%s \u00b7 karne %s \u00b7 %d. g\u00fcn \u00b7 d\u00fcn %d m\u00fc\u015fteri, ciro %s, net %s \u00b7 raf %%%d dolu, %d \u00fcr\u00fcn \u00b7 m\u00fcd\u00fcr %s \u00b7 al\u0131\u015fkanl\u0131k %%%.0f"),
        *B.Name, *Grade(State, BranchIndex), State.Day - B.OpenedDay, B.LastShoppers, *BranchTl(B.LastRevenue), *BranchTl(B.LastProfit),
        Capacity > 0 ? Units * 100 / Capacity : 0, Carried, B.ManagerName.IsEmpty() ? TEXT("yok (senin talimatlar\u0131n)")
            // The skill stays hidden without an HR manager or a province manager (MarketManagers::SkillVisible).
            : *(MarketManagers::SkillVisible(State, BranchIndex) ? FString::Printf(TEXT("%s, beceri %d"), *B.ManagerName, B.ManagerSkill) : B.ManagerName),
        B.Maturity * 100.f) + (B.Depts.Num() > 0 ? TEXT(" \u00b7 reyonlar: ") + MarketDepartments::BranchLine(B) : FString()); // M26
}

void MarketBranches::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    MarketManagers::Migrate(State); // seeds a new manager's hidden style, morale and ceiling once; keeps the used names
    MarketDepartments::CloseDay(State); // M26: departments reach new branches, masters learn
    const int32 Closed = State.Day - 1;
    if (Closed < 1) return;
    TArray<FString>& News = State.DayNews;
    const int32 Span = MarketManagers::SpanPenalty(State); // the player's span of control, once for the day
    const double Level = MarketPrices::ListLevel(State.Day);
    const TArray<float> WishToday = Wishes(State, Products, Closed);
    // G-089: where every branch gets its goods today (nearest depot, its efficiency, the trucks), once for the day.
    const TArray<MarketDepots::FLink> Links = MarketDepots::AllLinks(State);
    for (int32 Index = 0; Index < State.Branches.Num(); ++Index)
    {
        FMarketBranch& B = State.Branches[Index];
        const EStage Stage = static_cast<EStage>(B.Stage);
        const FFormat& Kind = FormatInfo(B.Format);
        if (Stage == EStage::Closed) continue;
        const FSite Where = SiteOf(State, B);
        // Opening steps.
        if (Stage == EStage::Renovation && Closed >= B.StageUntil)
        {
            B.Stage = static_cast<uint8>(EStage::Permits);
            B.StageUntil = State.Day + PermitDays - 1 + (MarketStaff::HasAccountant(State) ? 0 : 3);
            // G-086b: the province manager knows the town hall.
            B.StageUntil = FMath::Max(State.Day, B.StageUntil - MarketManagers::OpeningDaysSavedIn(State, Where.Country, Where.Province));
            News.Add(FString::Printf(TEXT("%s: tadilat bitti. Ruhsat i\u00e7in belediyeye ba\u015fvuruldu (%d. g\u00fcn).%s"), *B.Name, B.StageUntil,
                MarketStaff::HasAccountant(State) ? TEXT(" Mali m\u00fc\u015favir evraklar\u0131 haz\u0131rlad\u0131.") : TEXT(" Evraklar eksik gidince i\u015f uzad\u0131.")));
            continue;
        }
        if (Stage == EStage::Permits && Closed >= B.StageUntil)
        {
            B.Stage = static_cast<uint8>(EStage::Hiring);
            B.Workers = MarketStoreAssign::WorkersFor(MarketStoreViews::MeasuresOf(B), B.Format); // G-088 C: the store's size and tills
            MarketLedger::AddStoreCost(State, MarketStaff::HireCostOn(MarketStaff::ERole::Cashier, Closed) * B.Workers, Index); // C10
            if (B.ManagerName.IsEmpty()) MarketManagers::HireStoreManager(State, Index); // G-086b ek (M22): a name never used before
            News.Add(FString::Printf(TEXT("%s: ruhsat \u00e7\u0131kt\u0131. %d \u00e7al\u0131\u015fan i\u015fe al\u0131nd\u0131; m\u00fcd\u00fcr %s (beceri %d)."), *B.Name, B.Workers, *B.ManagerName, B.ManagerSkill));
            continue;
        }
        if (Stage == EStage::Hiring)
        {
            const int64 Bill = StockCost(B, Products);
            State.Cash -= Bill;
            State.Purchases += Bill;
            MarketLedger::Post(State, MarketLedger::EAccount::Purchases, -Bill, true, Index); // C3: bought during the close
            for (FMarketBranchItem& Item : B.Items) Item.Units = FMath::Max(Item.Units, Item.Capacity);
            B.Stage = static_cast<uint8>(EStage::Open);
            B.OpenedDay = State.Day;
            News.Add(FString::Printf(TEXT("%s YARIN A\u00c7ILIYOR! A\u00e7\u0131l\u0131\u015f sto\u011fu raflarda (%s). \u0130lk hafta merak edenler gelir; kal\u0131c\u0131 m\u00fc\u015fteri bir ayda olu\u015fur."), *B.Name, *BranchTl(Bill)));
            continue;
        }
        if (Stage != EStage::Open || Closed < B.OpenedDay) continue;

        // The morning delivery ordered at the last close. G-089: through a depot some of it comes short or broken
        // (a weak or missing depot manager, missing trucks) and a dishonest depot manager keeps a little; the goods
        // were paid when ordered.
        const MarketDepots::FLink Link = Links.IsValidIndex(Index) ? Links[Index] : MarketDepots::FLink();
        int64 DepotLoss = 0;
        if (Link.Depot != INDEX_NONE && (Link.ShortPermille > 0 || Link.SkimPermille > 0))
        {
            int64 ShortCost = 0, SkimCost = 0;
            for (int32 I = 0; I < Products.Num(); ++I)
            {
                FMarketBranchItem* Item = ItemOf(B, Products[I].Id);
                if (!Item || Item->Incoming <= 0) continue;
                const int32 Short = MarketDepots::LostUnits(Item->Incoming, Link.ShortPermille, BranchMix(State.RivalSeed, Closed, 0xD390u + Index * 131u + I));
                const int32 Taken = MarketDepots::LostUnits(Item->Incoming - Short, Link.SkimPermille, BranchMix(State.RivalSeed, Closed, 0xD391u + Index * 131u + I));
                Item->Incoming -= Short + Taken;
                ShortCost += Products[I].Cost * Short;
                SkimCost += Products[I].Cost * Taken;
            }
            MarketDepots::RecordLoss(State, Link.Depot, ShortCost, SkimCost);
            DepotLoss = ShortCost + SkimCost;
        }
        for (FMarketBranchItem& Item : B.Items) { Item.Units += Item.Incoming; Item.Incoming = 0; }
        // G-086b: what the manager brings today (effective skill, style, honesty, the hierarchy above).
        const MarketManagers::FBranchRule Rule = MarketManagers::RuleFor(State, Index, Span);

        // Shoppers: the catchment's trips x our share x the calendar x habit x our shops nearby. A day the law
        // keeps shops shut (G-084) has none; the costs still run.
        int32 Sold = 0, Empty = 0;
        for (const FMarketBranchItem& Item : B.Items) { Sold += Item.LastSold; Empty += Item.LastEmpty; }
        const float Availability = Sold + Empty > 0 ? FMath::Clamp(static_cast<float>(Sold) / (Sold + Empty), 0.2f, 1.f) : 0.9f;
        const float Service = Kind.Service * (B.ManagerName.IsEmpty() ? 0.9f : 1.f);
        const float Pull = FMath::Exp(-(B.PriceIndex - 1.f) / MarketCompetitors::PriceSensitivity) * Availability * Service *
            (0.8f + B.Satisfaction / 250.f) * (0.9f + 0.4f * B.Maturity) * (State.Day - B.OpenedDay < 7 ? 1.3f : 1.f) * MarketCompany::TrafficBonus(State)
            * MarketDepartments::PullFactor(B, Closed); // M26: fresh bread, a good butcher
        // Akis C2b: the province's chains against the start, a price war against us on top.
        const float Share = Pull / (Pull + MarketTuning::Get(TEXT("BranchCompetition"), 3.f) * Where.Competition * MarketChains::PressureFactor(State, Where.Country, Where.Province, Closed));
        const float Trips = MarketCalendar::ClosedByLaw(Closed) ? 0.f : TripsOf(Where, Kind) * MarketCalendar::TrafficFactor(Closed, State.RivalSeed)
            * MarketOnline::StoreTrafficFactorOn(State, Closed, Where.Country) // M32: trips gone online, the epidemic's closure days
            * MarketAdvertising::TrafficFactor(State, Where.Country); // M34: the company's ads
        const int32 Arrived = FMath::RoundToInt32(Trips * Share * (0.5f + 0.5f * B.Maturity) * Cannibalization(State, Index, Where));
        // G-088 C: the store's tills. Too few lanes lose shoppers in the queue; roomy ones keep a few more.
        const MarketStoreAssign::FStoreMeasures Measures = MarketStoreViews::MeasuresOf(B);
        const int32 Shoppers = FMath::RoundToInt32(Arrived * MarketStoreAssign::QueueFactor(Measures, B.Format, static_cast<float>(Arrived)));
        B.LastQueueLost = FMath::Max(0, Arrived - Shoppers);
        const float FreshDemand = MarketStoreAssign::FreshFactor(Measures, B.Format);
        const float FreshSpoil = MarketStoreAssign::SpoilFactor(Measures, B.Format);
        const float ColdChain = MarketSourcing::DairySpoilFactor(State); // G-083: a distributor keeps the cold chain

        // C10 knob (default off): a shopper's basket grows with the real wage (wages / prices), elasticity RealSpend.
        const float RealSpend = FMath::Pow(static_cast<float>(MarketPrices::WageIndex(Closed) / FMath::Max(0.01, MarketPrices::ListLevel(Closed))), MarketTuning::Get(TEXT("RealSpend"), 0.f));
        // What they want, what is on the shelf.
        int64 Revenue = 0, Cogs = 0, WasteCost = 0;
        int32 DayEmpty = 0, DaySold = 0;
        for (int32 I = 0; I < Products.Num(); ++I)
        {
            FMarketBranchItem* Item = ItemOf(B, Products[I].Id);
            if (!Item) continue;
            // G-088 C: dairy and ice cream follow the store's cold room (and spoil more when it is crowded).
            const MarketGoods::EGroup Group = MarketGoods::Classify(Products[I].Category);
            const bool bFresh = Group == MarketGoods::EGroup::Dairy || Group == MarketGoods::EGroup::IceCream;
            // M33: a week's clearance makes a slow item move (and earns less on each). M38: the store's campaigns (the
            // player's, the company's in every store) work the same way; a multi-buy or a gondola head adds wish.
            float PromoCut = 0.f, PromoPull = 1.f;
            MarketPromotions::StoreEffect(State, Products, Index, I, Closed, PromoCut, PromoPull);
            const float Cut = FMath::Max(Item->MarkdownUntil >= Closed ? Item->Markdown / 100.f : 0.f, PromoCut);
            const int32 Want = FMath::RoundToInt32(Shoppers * UnitsPerShopper * RealSpend * WishToday[I] * Where.Income * (bFresh ? FreshDemand : 1.f) * (1.f + 2.5f * Cut)
                * PromoPull * MarketStory::IdentityDemand(State, Group)); // M38: the company's identity
            const int32 Take = Item->Capacity > 0 ? FMath::Min(Want, Item->Units) : 0;
            Item->Units -= Take;
            Item->LastSold = Take;
            Item->LastEmpty = Want - Take;
            DaySold += Take;
            DayEmpty += Want - Take;
            const int64 Price = FMath::Max<int64>(5, FMath::RoundToInt64(Products[I].BasePrice * B.PriceIndex * (1.0 - Cut) / 5.0) * 5);
            Item->IdleDays = Take == 0 && Item->Units > 0 ? Item->IdleDays + 1 : 0;
            Revenue += Price * Take;
            Cogs += Products[I].Cost * Take;
            // G-086b: waste follows the manager's style (a generous one keeps more on hand and throws more away).
            const float SpoilExact = Item->Units * (Rule.WasteRate + Link.ExtraWaste) * (bFresh ? FreshSpoil : 1.f) * (Group == MarketGoods::EGroup::Dairy ? ColdChain : 1.f); // G-089: the depot's handling
            int32 Spoil = FMath::FloorToInt32(SpoilExact);
            if ((BranchMix(State.RivalSeed, Closed, 0x5F01u + Index * 131u + I) % 1000u) < static_cast<uint32>((SpoilExact - Spoil) * 1000.f)) ++Spoil;
            Spoil = FMath::Clamp(Spoil, 0, Item->Units);
            Item->Units -= Spoil;
            WasteCost += Products[I].Cost * Spoil;
        }
        // Logistics and buying power of the company (G-086: depots per sub-region, trucks, central buying, own
        // brand, a new country's first months).
        // Positive: freight, customs, a new country's learning; negative: rebates of depots and central buying.
        const int64 Logistics = FMath::RoundToInt64(Cogs * (MarketCompany::CostFactor(State, B, Link) - 1.f));
        // A dishonest manager keeps a little of the till.
        const int64 Skim = Revenue * Rule.SkimPermille / 1000;
        const int64 RentDay = FMath::RoundToInt64(B.Rent * MarketPrices::ListLevel(State.Day) / MarketPrices::ListLevel(FMath::Max(1, B.OpenedDay)) / 30.0);
        const int64 WagesDay = B.Workers * MarketStaff::FairWage(MarketStaff::ERole::Cashier, 50, State.Day) + B.ManagerWage;
        const int64 SocialDay = MarketStaff::EmployerShare(WagesDay); // C3 (B3): the employer's social security share
        const int64 RunningDay = FMath::RoundToInt64(1500 * Level * Kind.Running);
        const int64 Opex = RentDay + WagesDay + SocialDay + RunningDay;
        // M26: the branch's departments (they book their own lines; their goods are bought and paid the same day).
        const MarketDepartments::FDay Dept = MarketDepartments::Day(State, Index, Shoppers, Where.Income, Closed);
        const int64 Profit = Revenue - Skim - Cogs - Logistics - Opex - WasteCost - DepotLoss + Dept.Profit; // waste, depot losses: goods already paid
        State.Cash += Revenue - Skim - Logistics - Opex + Dept.Cash; // goods were paid when ordered; the family shop's till stays separate
        State.Books.PeriodPurchases += Dept.Purchases; // VAT paid on the department goods
        // C3 (B2): the branch's day line by line (Store = the branch).
        {
            using MarketLedger::EAccount;
            MarketLedger::Post(State, EAccount::Sales, Revenue, true, Index);
            MarketLedger::Post(State, EAccount::Shrinkage, -Skim, true, Index);
            MarketLedger::Post(State, EAccount::Logistics, -Logistics, true, Index);
            MarketLedger::Post(State, EAccount::Rent, -RentDay, true, Index);
            MarketLedger::Post(State, EAccount::Wages, -WagesDay, true, Index);
            MarketLedger::Post(State, EAccount::SocialSecurity, -SocialDay, true, Index);
            MarketLedger::Post(State, EAccount::Utilities, -RunningDay, true, Index);
            MarketLedger::Post(State, EAccount::CostOfGoods, -Cogs, false, Index);
            MarketLedger::Post(State, EAccount::Waste, -WasteCost, false, Index);
            MarketLedger::Post(State, EAccount::Shrinkage, -DepotLoss, false, Index);
        }
        State.LastBranchProfit += Profit;
        State.LastProfit += Profit;
        // Branch sales carry VAT like the family shop's (their purchases already count in State.Purchases).
        State.Books.PeriodSales += Revenue + Dept.Revenue;
        B.LastRevenue = Revenue + Dept.Revenue;
        B.LastProfit = Profit;
        B.LastShoppers = Shoppers;
        B.WeekProfit += Profit;
        B.Last30Profit = B.Last30Profit * 29 / 30 + Profit;
        B.Last30Revenue = B.Last30Revenue * 29 / 30 + Revenue + Dept.Revenue;
        const float DayAvailability = DaySold + DayEmpty > 0 ? static_cast<float>(DaySold) / (DaySold + DayEmpty) : 1.f;
        B.Satisfaction = FMath::Clamp(B.Satisfaction + ((50.f + 40.f * DayAvailability - 100.f * (B.PriceIndex - 1.f)) - B.Satisfaction) * 0.1f, 0.f, 100.f);
        B.Maturity = FMath::Min(1.f, B.Maturity + 1.f / MaturityDays);

        // The manager's order for tomorrow (a skilled one is closer to the real demand) and prices.
        const float Error = (100 - FMath::Clamp(Rule.Skill, 0, 100)) / 100.f * 0.4f * Rule.ErrorFactor;
        const float Tomorrow = MarketCalendar::TrafficFactor(State.Day, State.RivalSeed) / FMath::Max(0.3f, MarketCalendar::TrafficFactor(Closed, State.RivalSeed));
        const bool bTight = State.Cash < Opex * 3; // short of money: the manager orders half
        // Never more than the till holds; nothing when the company is already in the red.
        const int64 Budget = FMath::Max<int64>(0, State.Cash);
        // G-088 C: what fits behind the shop (half a shelf more at the nominal stock room).
        const float StockRoom = 1.f + 0.5f * MarketStoreAssign::BackroomFactor(Measures, B.Format);
        int64 Bill = 0;
        for (int32 I = 0; I < Products.Num(); ++I)
        {
            FMarketBranchItem* Item = ItemOf(B, Products[I].Id);
            if (!Item || Item->Capacity <= 0) continue;
            const float Noise = (BranchMix(State.RivalSeed, Closed, 0x0DE7u + Index * 131u + I) % 2001u) / 1000.f - 1.f;
            const float Expected = (Item->LastSold + Item->LastEmpty) * Tomorrow * (1.f + Error * Noise);
            const int32 Target = FMath::RoundToInt32(Rule.OrderFactor * FMath::Min(Item->Capacity * StockRoom, FMath::Max(static_cast<float>(Item->Capacity), Expected * 1.2f)));
            int32 Order = FMath::Max(0, Target - Item->Units);
            if (bTight) Order /= 2;
            if (Products[I].Cost > 0) Order = static_cast<int32>(FMath::Min<int64>(Order, FMath::Max<int64>(0, Budget - Bill) / Products[I].Cost));
            Order = FMath::Max(0, Order);
            Item->Incoming = Order;
            Bill += Products[I].Cost * Order;
            MarketSourcing::RecordPurchase(State, Products[I].Category, Products[I].Cost * Order); // G-083: the line's monthly minimum
        }
        State.Cash -= Bill;
        State.Purchases += Bill;
        MarketLedger::Post(State, MarketLedger::EAccount::Purchases, -Bill, true, Index); // C3: the manager's order, bought during the close
        MarketSuppliers::Account(State, MarketSuppliers::Current(State)).Volume30 += Bill; // more shops, better purchase terms
        if (Rule.bFollowsRivals)
        {
            // A good (or price-minded) manager keeps prices at the market type's place against the rivals; the
            // price-minded one a little under it (G-086b style).
            const float Rival = MarketCompetitors::RivalPriceFactor(State, FString(), TArray<FString>());
            B.PriceIndex = FMath::Clamp(B.PriceIndex + (Rival * (Kind.PriceTarget + 0.02f + Rule.PriceBias) - B.PriceIndex) * 0.2f, 0.85f, 1.2f);
        }
        else B.PriceIndex = FMath::Clamp(MainPriceIndex(State, Products) * (Kind.PriceTarget + Rule.PriceBias), 0.85f, 1.2f); // copies the family shop's labels

        if (Closed % 7 == 0)
        {
            const FString Cleared = Clearance(State, Index, Products); // M33: the store manager's own clearance
            News.Add(FString::Printf(TEXT("%s haftas\u0131: net %s, karne %s, d\u00fcn %d m\u00fc\u015fteri, raf dolulu\u011fu %%%.0f.%s%s%s"), *B.Name, *BranchTl(B.WeekProfit), *Grade(State, Index), Shoppers, DayAvailability * 100.f,
                Skim > 0 && MarketStaff::HasAccountant(State) && !Rule.bSkimHidden ? TEXT(" Mali m\u00fc\u015favir: \"\u015eubenin kasas\u0131 sat\u0131\u015flarla tutmuyor.\"") : TEXT(""),
                *MarketStoreViews::WeeklyHint(B, Arrived), *Cleared)); // G-088 C: tills or cold room too small
            B.WeekProfit = 0;
        }
    }
}

FString MarketBranches::Clearance(FMarketState& State, int32 BranchIndex, const TArray<FMarketProduct>& Products)
{
    // M33 (Mustafa 01.10.2026: "stok azaltmak i\u00e7in kampanya yaps\u0131nlar; kritik \u015feylerde bir \u00fcst\u00fcnde kim varsa ona sorsunlar").
    // Up to three slow items a week (ten idle days, at least half a shelf) get a week's markdown: 15 % for a careful
    // manager, 30 % for a price-minded one, 20 % otherwise. More than 20 % needs the province manager; without one the
    // store manager stays at 20 %. A weak store manager now and then marks down a selling item by mistake; a capable
    // province manager (skill 50+) takes it back. Nothing of this comes to the player: it shows in the weekly line.
    if (!State.Branches.IsValidIndex(BranchIndex)) return FString();
    FMarketBranch& B = State.Branches[BranchIndex];
    if (B.ManagerName.IsEmpty()) return FString();
    const int32 Day = State.Day - 1;
    const MarketManagers::FBranchRule Rule = MarketManagers::RuleFor(State, BranchIndex);
    const int32 Boss = MarketManagers::FindManager(State, MarketManagers::ELevel::Province, CountryOf(State, B), B.Province);
    const int32 BossSkill = Boss == INDEX_NONE ? 0 : MarketManagers::EffectiveManagerSkill(State, Boss);
    const int32 Wanted = B.ManagerStyle == static_cast<uint8>(MarketManagers::EStyle::PriceMinded) ? 30 : B.ManagerStyle == static_cast<uint8>(MarketManagers::EStyle::Careful) ? 15 : 20;
    int32 Marked = 0, Approved = 0, TakenBack = 0;
    TArray<FString> Goods;   // C8 (Codex C6): the line names the goods and the cut
    auto NameOf = [&Products, &State](const FString& Id) -> FString
    {
        const FMarketProduct* P = Products.FindByPredicate([&Id](const FMarketProduct& X) { return X.Id == Id; });
        return P ? (State.bRealBrands ? P->RealName : P->FictionalName) : Id;
    };
    for (int32 I = 0; I < B.Items.Num() && Marked < 3; ++I)
    {
        FMarketBranchItem& Item = B.Items[I];
        if (Item.MarkdownUntil >= Day || Item.IdleDays < 10 || Item.Units < FMath::Max(3, Item.Capacity / 2)) continue;
        int32 Cut = Wanted;
        if (Cut > 20) { if (Boss != INDEX_NONE) ++Approved; else Cut = 20; }
        Item.Markdown = static_cast<uint8>(Cut);
        Item.MarkdownUntil = Day + 7;
        Item.IdleDays = 0;
        Goods.Add(FString::Printf(TEXT("%s %%%d"), *NameOf(Item.ProductId), Cut));
        ++Marked;
    }
    // A mistake: a weak manager marks down an item that sells.
    if (Rule.Skill < 45 && (BranchMix(State.RivalSeed, Day, 0xC1EAu + BranchIndex * 131u) % 100u) < static_cast<uint32>((45 - Rule.Skill) * 2))
    {
        int32 Best = INDEX_NONE;
        for (int32 I = 0; I < B.Items.Num(); ++I)
            if (B.Items[I].MarkdownUntil < Day && (Best == INDEX_NONE || B.Items[I].LastSold > B.Items[Best].LastSold)) Best = I;
        if (Best != INDEX_NONE && B.Items[Best].LastSold > 0)
        {
            if (Boss != INDEX_NONE && BossSkill >= 50) ++TakenBack;
            else { B.Items[Best].Markdown = 20; B.Items[Best].MarkdownUntil = Day + 7; Goods.Add(FString::Printf(TEXT("%s %%20"), *NameOf(B.Items[Best].ProductId))); ++Marked; }
        }
    }
    if (Marked == 0 && TakenBack == 0) return FString();
    FString Line = Marked > 0 ? FString::Printf(TEXT(" Stok eritme: bir haftal\u0131k indirim (%s)"), *FString::Join(Goods, TEXT(", "))) : FString(TEXT(" Stok eritme yok"));
    if (Approved > 0) Line += FString::Printf(TEXT(" (%%%d'luk indirimi il m\u00fcd\u00fcr\u00fc %s onaylad\u0131)"), Wanted, *State.Management.Managers[Boss].Name);
    if (TakenBack > 0) Line += FString::Printf(TEXT("; il m\u00fcd\u00fcr\u00fc %s iyi satan bir \u00fcr\u00fcndeki yanl\u0131\u015f indirimi geri ald\u0131"), *State.Management.Managers[Boss].Name);
    return Line + TEXT(".");
}

bool MarketBranches::RecentlyVisited(const FMarketState& State, int32 BranchIndex)
{
    return State.Branches.IsValidIndex(BranchIndex) && State.Branches[BranchIndex].VisitedDay > 0 && State.Day - State.Branches[BranchIndex].VisitedDay <= VisitSeenDays;
}

bool MarketBranches::Visit(FMarketState& State, const TArray<FMarketProduct>& Products, int32 BranchIndex, FString& OutMessage)
{
    if (!State.Branches.IsValidIndex(BranchIndex) || State.Branches[BranchIndex].Stage != static_cast<uint8>(EStage::Open))
    {
        OutMessage = TEXT("Bu \u015fube \u015fu an gezilemez.");
        return false;
    }
    FMarketBranch& B = State.Branches[BranchIndex];
    const bool bFresh = B.VisitedDay == 0 || State.Day - B.VisitedDay >= 30;
    B.VisitedDay = State.Day;
    TArray<FString> Parts;
    Parts.Add(FString::Printf(TEXT("%s: karne %s."), *B.Name, *Grade(State, BranchIndex)));
    if (!B.ManagerName.IsEmpty())
    {
        Parts.Add(FString::Printf(TEXT("M\u00fcd\u00fcr %s i\u015fini %s (beceri %d)."), *B.ManagerName,
            B.ManagerSkill >= 75 ? TEXT("\u00e7ok iyi biliyor") : B.ManagerSkill >= 55 ? TEXT("biliyor") : TEXT("zor y\u00fcr\u00fct\u00fcyor"), B.ManagerSkill));
        if (bFresh && B.ManagerMorale >= 0.f)
        {
            B.ManagerMorale = FMath::Min(100.f, B.ManagerMorale + 5.f);
            Parts.Add(TEXT("Seni g\u00f6r\u00fcnce ekip toparland\u0131."));
        }
        if (B.ManagerHonesty < 50) Parts.Add(TEXT("Kasa defteri biraz kar\u0131\u015f\u0131k duruyor."));
    }
    // The emptiest shelf and the queue: what the daily line does not say.
    int32 Worst = INDEX_NONE;
    float WorstFill = 1.f;
    for (int32 I = 0; I < Products.Num(); ++I)
    {
        const FMarketBranchItem* Item = B.Items.FindByPredicate([&Products, I](const FMarketBranchItem& It) { return It.ProductId == Products[I].Id; });
        if (!Item || Item->Capacity <= 0) continue;
        const float Fill = static_cast<float>(Item->Units) / Item->Capacity;
        if (Fill < WorstFill) { WorstFill = Fill; Worst = I; }
    }
    if (Worst != INDEX_NONE && WorstFill < 0.3f) Parts.Add(FString::Printf(TEXT("%s raf\u0131 neredeyse bo\u015f."), *(State.bRealBrands ? Products[Worst].RealName : Products[Worst].FictionalName)));
    if (B.LastQueueLost > 0) Parts.Add(FString::Printf(TEXT("D\u00fcn kasa kuyru\u011fundan %d m\u00fc\u015fteri vazge\u00e7mi\u015f."), B.LastQueueLost));
    OutMessage = FString::Join(Parts, TEXT(" "));
    return true;
}

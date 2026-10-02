#include "MarketCompany.h"
#include "MarketLedger.h"
#include "MarketCountry.h"
#include "MarketBranches.h"
#include "MarketCalendar.h"
#include "MarketDepots.h"
#include "MarketManagers.h"
#include "MarketPrices.h"
#include "MarketStaff.h"
#include "MarketStart.h"
#include "MarketStory.h"

namespace MarketCompany
{
    FString CompanyTl(int64 Kurus)
    {
        return MarketCountry::Money(Kurus); // G-084: the active country's currency
    }

    int64 Scaled(const FMarketState& State, int64 Kurus) { return FMath::RoundToInt64(Kurus * MarketPrices::ListLevel(State.Day)); }

    bool IsOpen(const FMarketBranch& B) { return B.Stage == static_cast<uint8>(MarketBranches::EStage::Open); }

    // First day of the company's first open shop in a country (0 = none).
    int32 CountryFirstDay(const FMarketState& State, const FString& Country)
    {
        int32 First = 0;
        for (const FMarketBranch& B : State.Branches)
            if (IsOpen(B) && MarketBranches::CountryOf(State, B) == Country && (First == 0 || B.OpenedDay < First)) First = B.OpenedDay;
        return First;
    }
}

int32 MarketCompany::TotalStores(const FMarketState& State)
{
    int32 Count = 1;
    for (const FMarketBranch& B : State.Branches) if (IsOpen(B)) ++Count;
    return Count;
}

int32 MarketCompany::CountryStores(const FMarketState& State, const FString& Country)
{
    int32 Count = Country == State.CountryId ? 1 : 0;
    for (const FMarketBranch& B : State.Branches) if (IsOpen(B) && MarketBranches::CountryOf(State, B) == Country) ++Count;
    return Count;
}

int32 MarketCompany::Provinces(const FMarketState& State)
{
    return MarketBranches::ProvincesWithShops(State).Num();
}

int32 MarketCompany::ForeignCountries(const FMarketState& State)
{
    TArray<FString> Seen;
    for (const FMarketBranch& B : State.Branches)
    {
        const FString Country = MarketBranches::CountryOf(State, B);
        if (IsOpen(B) && Country != State.CountryId) Seen.AddUnique(Country);
    }
    return Seen.Num();
}

int64 MarketCompany::CountryMarketDay(const FMarketState& State)
{
    const int64 People = static_cast<int64>(FMath::Max(1000, MarketCountry::PopulationK(State.CountryId))) * 1000;
    const MarketCountry::FProfile* Pack = MarketCountry::Find(State.CountryId);
    const double PerPerson = Pack && Pack->GroceryPerPersonDay > 0.0 ? Pack->GroceryPerPersonDay : 65.0 * (Pack ? Pack->WageFactor : 1.f);
    return FMath::Max<int64>(1, FMath::RoundToInt64(People * PerPerson * MarketPrices::ListLevel(FMath::Max(1, State.Day - 1))));
}

int64 MarketCompany::CountryRevenueToday(const FMarketState& State)
{
    int64 Revenue = FMath::Max<int64>(0, State.LastRevenue);
    for (const FMarketBranch& B : State.Branches)
        if (IsOpen(B) && MarketBranches::CountryOf(State, B) == State.CountryId) Revenue += FMath::Max<int64>(0, B.LastRevenue);
    return Revenue;
}

void MarketCompany::TrackNationalRevenue(FMarketState& State)
{
    int64& Smooth = State.Ledger.CountryRevenueDay;
    const int64 Today = CountryRevenueToday(State);
    Smooth = Smooth <= 0 ? Today : Smooth + (Today - Smooth) / 30;
}

float MarketCompany::NationalShare(const FMarketState& State)
{
    const int64 Revenue = State.Ledger.CountryRevenueDay > 0 ? State.Ledger.CountryRevenueDay : CountryRevenueToday(State);
    return static_cast<float>(100.0 * static_cast<double>(Revenue) / static_cast<double>(CountryMarketDay(State)));
}

bool MarketCompany::ChapterOpen(const FMarketState& State, int32 Chapter)
{
    return State.Story.Chapter >= Chapter;   // chapters only grow (1..7); the finale keeps what was open
}

bool MarketCompany::HasDepot(const FMarketState& State, const FString& Country, const FString& SubRegion)
{
    return MarketDepots::HasDepotInSubRegion(State, Country, SubRegion);
}

int32 MarketCompany::DepotCount(const FMarketState& State)
{
    return MarketDepots::Count(State);
}

int32 MarketCompany::FarStores(const FMarketState& State)
{
    int32 Far = 0;
    for (const FMarketBranch& B : State.Branches)
        if (IsOpen(B) && !MarketBranches::SiteOf(State, B).bHome) ++Far;
    return Far;
}

float MarketCompany::CostFactor(const FMarketState& State, const FMarketBranch& Branch)
{
    return CostFactor(State, Branch, MarketDepots::LinkOf(State, Branch));
}

float MarketCompany::CostFactor(const FMarketState& State, const FMarketBranch& Branch, const MarketDepots::FLink& Link)
{
    const FMarketCompany& C = State.Company;
    const MarketBranches::FSite Site = MarketBranches::SiteOf(State, Branch);
    float Factor = 1.f;
    if (C.bCentralBuying) Factor -= 0.02f;
    if (C.bPrivateLabel) Factor -= 0.015f;
    // G-089: the nearest depot within range (its rebate x efficiency, the road, missing trucks); else the van.
    if (Link.Depot != INDEX_NONE) Factor += Link.CostAdd;
    else if (!Site.bHome) Factor += MarketDepots::WholesalerVan; // the wholesaler's van for every far shop
    if (Site.bAbroad)
    {
        Factor += 0.01f;                           // customs and paperwork
        const int32 First = CountryFirstDay(State, Site.Country);
        if (First > 0 && State.Day - First < LearningDays) Factor += 0.03f; // other suppliers, other habits
    }
    // G-086b: a sub-region manager cuts the losses on the road, a director plans the depots together (+0.5 %),
    // a country manager abroad knows the wholesalers.
    Factor += MarketManagers::CostAdjust(State, Branch);
    return Factor;
}

float MarketCompany::TrafficBonus(const FMarketState& State)
{
    return State.Company.bPrivateLabel ? 1.03f : 1.f;
}

int64 MarketCompany::DepotCost(const FMarketState& State)
{
    return MarketDepots::BuildCost(State, State.CountryId, MarketStart::HomeProvince(State));
}

bool MarketCompany::BuildDepot(FMarketState& State, const FString& Country, const FString& SubRegion, FString& OutMessage)
{
    const FString C = Country.IsEmpty() ? State.CountryId : Country;
    const MarketCountry::FProfile* Pack = MarketCountry::Find(C);
    const MarketCountry::FRegion* Sub = Pack ? Pack->SubRegions.FindByPredicate([&SubRegion](const MarketCountry::FRegion& R) { return R.Id == SubRegion; }) : nullptr;
    if (!Sub) { OutMessage = TEXT("B\u00f6yle bir b\u00f6lge yok."); return false; }
    if (HasDepot(State, C, SubRegion)) { OutMessage = FString::Printf(TEXT("%s b\u00f6lgesinde zaten depo var."), *Sub->Name); return false; }
    if (TotalStores(State) < MarketDepots::MinStores) { OutMessage = FString::Printf(TEXT("Depo i\u00e7in en az %d ma\u011faza gerekir."), MarketDepots::MinStores); return false; }
    bool bShopThere = false;
    for (const FString& Province : MarketBranches::ProvincesWithShops(State, C)) if (Sub->Provinces.Contains(Province)) bShopThere = true;
    if (!bShopThere) { OutMessage = FString::Printf(TEXT("%s b\u00f6lgesinde hen\u00fcz ma\u011fazan yok."), *Sub->Name); return false; }
    // G-089: the depot stands in the sub-region's province nearest to the branches.
    const MarketDepots::FAdvice Advice = MarketDepots::SuggestDepotProvince(State, C, SubRegion);
    if (Advice.Province.IsEmpty()) { OutMessage = Advice.Text; return false; }
    return MarketDepots::Build(State, C, Advice.Province, OutMessage);
}

bool MarketCompany::Build(FMarketState& State, int32 What, FString& OutMessage)
{
    FMarketCompany& C = State.Company;
    const int32 Stores = TotalStores(State);
    auto Pay = [&](int64 Cost2011, const TCHAR* Name) -> bool
    {
        const int64 Cost = Scaled(State, Cost2011);
        if (State.Cash < Cost) { OutMessage = FString::Printf(TEXT("%s %s; kasada yok."), Name, *CompanyTl(Cost)); return false; }
        State.Cash -= Cost;
        MarketLedger::Post(State, MarketLedger::EAccount::Investment, -Cost, true, MarketLedger::HeadOfficeStore); // B2
        OutMessage = FString::Printf(TEXT("%s tamam (%s)."), Name, *CompanyTl(Cost));
        return true;
    };
    switch (What)
    {
    case 0:
    {
        const MarketCountry::FCity* Home = MarketCountry::FindCity(State.CountryId, MarketStart::HomeProvince(State));
        return BuildDepot(State, State.CountryId, Home ? Home->SubRegion : FString(), OutMessage);
    }
    case 1:
        if (MarketDepots::Count(State) == 0) { OutMessage = TEXT("Kamyon i\u00e7in \u00f6nce bir depo."); return false; }
        if (!Pay(1200000, TEXT("Kamyon"))) return false;
        if (++C.Trucks == 1) MarketStory::AddMemory(State, TEXT("\u0130lk kamyon yola \u00e7\u0131kt\u0131"));
        return true;
    case 2:
        if (C.bCentralBuying) { OutMessage = TEXT("Merkezi sat\u0131n alma zaten var."); return false; }
        if (MarketDepots::Count(State) == 0 || Stores < 8) { OutMessage = TEXT("Merkezi sat\u0131n alma i\u00e7in bir depo ve en az 8 ma\u011faza gerekir."); return false; }
        if (!Pay(500000, TEXT("Merkezi sat\u0131n alma"))) return false;
        C.bCentralBuying = true;
        return true;
    case 3:
        if (C.bPrivateLabel) { OutMessage = TEXT("\"Miras\" markas\u0131 zaten raflarda."); return false; }
        if (Stores < 20) { OutMessage = TEXT("\u00d6zel marka i\u00e7in en az 20 ma\u011faza gerekir."); return false; }
        if (!Pay(2000000, TEXT("\"Miras\" \u00f6zel markas\u0131"))) return false;
        C.bPrivateLabel = true;
        MarketStory::AddMemory(State, TEXT("\"Miras\" markal\u0131 ilk \u00fcr\u00fcn rafta"));
        return true;
    default:
        OutMessage = TEXT("Bilinmeyen yat\u0131r\u0131m.");
        return false;
    }
}

bool MarketCompany::LeadsToday(const FMarketState& State)
{
    float Satisfaction = 0.f;
    for (const FMarketLoyalty& L : State.Loyalty) Satisfaction += L.Satisfaction;
    const float Average = State.Loyalty.Num() > 0 ? Satisfaction / State.Loyalty.Num() : 0.f;
    return State.MarketShare >= 40.f && TotalStores(State) >= 60 && State.LastProfit > 0 && Average >= 60.f;
}

FString MarketCompany::Summary(const FMarketState& State)
{
    const FMarketCompany& C = State.Company;
    FString Line = FString::Printf(TEXT("\u015eirket: %d ma\u011faza, %d il, ulusal pay %%%.2f"), TotalStores(State), Provinces(State), NationalShare(State));
    if (ForeignCountries(State) > 0) Line += FString::Printf(TEXT(", %d yabanc\u0131 \u00fclke"), ForeignCountries(State));
    TArray<FString> Built;
    if (MarketDepots::Count(State) > 0) Built.Add(FString::Printf(TEXT("%d depo"), MarketDepots::Count(State)));
    if (C.Trucks > 0) Built.Add(FString::Printf(TEXT("%d kamyon"), C.Trucks));
    if (C.bCentralBuying) Built.Add(TEXT("merkezi al\u0131m"));
    if (C.bPrivateLabel) Built.Add(TEXT("Miras markas\u0131"));
    if (Built.Num()) Line += TEXT(" \u00b7 ") + FString::Join(Built, TEXT(", "));
    if (State.Story.Chapter == 7 && !MarketStory::StoryClosed(State)) Line += FString::Printf(TEXT("\nLiderlik: %d / %d g\u00fcn"), C.LeadershipDays, LeadershipGoalDays);
    return Line;
}

int64 MarketCompany::DailyOfficeCost(const FMarketState& State, int32 GameDay)
{
    // G-089: each depot's rent (its province, the list level); a truck's running cost.
    return MarketDepots::DailyRent(State, GameDay) + FMath::RoundToInt64(State.Company.Trucks * 6000 * MarketPrices::ListLevel(GameDay));
}

void MarketCompany::CloseDay(FMarketState& State)
{
    FMarketCompany& C = State.Company;
    const int32 Closed = State.Day - 1;
    if (Closed < 1) return;
    // Head office: the depots and trucks (dark stores: MarketOnline). The managers above the shops are named people since G-086b
    // and are paid by MarketManagers::CloseDay (no anonymous area managers here any more).
    const int32 Stores = TotalStores(State);
    const int64 Office = DailyOfficeCost(State, Closed);
    const int64 Total = -Office;
    C.LastProfit = Total;
    C.WeekProfit += Total;
    State.LastBranchProfit += Total;
    State.LastProfit += Total;
    State.Cash += Total;
    MarketLedger::Post(State, MarketLedger::EAccount::HeadOffice, Total, true, MarketLedger::HeadOfficeStore); // B2: depots' rent, trucks

    // Chapter 7: a year of leading on every measure brings the one finale (karar J02; the measure becomes the
    // global retail league in G-082). After the finale the game goes on without new story content.
    if (State.Story.Chapter == 7 && !MarketStory::StoryClosed(State))
    {
        if (LeadsToday(State)) ++C.LeadershipDays;
        else C.LeadershipDays = FMath::Max(0, C.LeadershipDays - 3);
        if (C.LeadershipDays >= LeadershipGoalDays) MarketStory::ReachFinale(State, MarketStory::EEnding::Legacy);
    }
    if (MarketCalendar::DateOf(Closed).Weekday == 6)
    {
        if (MarketDepots::Count(State) > 0 || Office > 0)
            State.DayNews.Add(FString::Printf(TEXT("\u015eirket haftas\u0131: %d ma\u011faza, merkez gideri %s."), Stores, *CompanyTl(-C.WeekProfit)));
        C.WeekProfit = 0;
    }
}

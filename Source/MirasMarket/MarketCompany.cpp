#include "MarketCompany.h"
#include "MarketBranches.h"
#include "MarketCalendar.h"
#include "MarketPrices.h"
#include "MarketStaff.h"
#include "MarketStory.h"

namespace MarketCompany
{
    FString CompanyTl(int64 Kurus)
    {
        const int64 Abs = Kurus < 0 ? -Kurus : Kurus;
        return FString::Printf(TEXT("%s%lld,%02lld TL"), Kurus < 0 ? TEXT("-") : TEXT(""), static_cast<long long>(Abs / 100), static_cast<long long>(Abs % 100));
    }

    const FCity Cities[static_cast<int32>(ECity::Count)] = {
        { TEXT("Babaeski"), TEXT("K\u0131rklareli"), TEXT("T\u00fcrkiye"), 4, 0.95f, 0.9f, 120000, 3 },
        { TEXT("K\u0131rklareli"), TEXT("K\u0131rklareli"), TEXT("T\u00fcrkiye"), 4, 1.f, 1.f, 150000, 4 },
        { TEXT("\u00c7orlu"), TEXT("Tekirda\u011f"), TEXT("T\u00fcrkiye"), 4, 1.1f, 1.15f, 220000, 6 },
        { TEXT("Tekirda\u011f"), TEXT("Tekirda\u011f"), TEXT("T\u00fcrkiye"), 4, 1.05f, 1.1f, 200000, 5 },
        { TEXT("Edirne"), TEXT("Edirne"), TEXT("T\u00fcrkiye"), 4, 1.f, 1.f, 170000, 5 },
        { TEXT("Ke\u015fan"), TEXT("Edirne"), TEXT("T\u00fcrkiye"), 4, 0.9f, 0.85f, 110000, 3 },
        { TEXT("\u0130stanbul Avrupa"), TEXT("\u0130stanbul"), TEXT("T\u00fcrkiye"), 5, 1.3f, 1.35f, 500000, 30 },
        { TEXT("\u0130stanbul Anadolu"), TEXT("\u0130stanbul"), TEXT("T\u00fcrkiye"), 5, 1.3f, 1.3f, 480000, 25 },
        { TEXT("Bursa"), TEXT("Bursa"), TEXT("T\u00fcrkiye"), 5, 1.1f, 1.15f, 260000, 12 },
        { TEXT("\u0130zmir"), TEXT("\u0130zmir"), TEXT("T\u00fcrkiye"), 5, 1.15f, 1.2f, 320000, 15 },
        { TEXT("Ankara"), TEXT("Ankara"), TEXT("T\u00fcrkiye"), 5, 1.15f, 1.2f, 300000, 15 },
        { TEXT("Kocaeli"), TEXT("Kocaeli"), TEXT("T\u00fcrkiye"), 5, 1.1f, 1.1f, 250000, 10 },
        { TEXT("K\u0131rcaali"), TEXT("K\u0131rcaali"), TEXT("Bulgaristan"), 6, 0.85f, 0.8f, 140000, 4 },
        { TEXT("Filibe"), TEXT("Filibe"), TEXT("Bulgaristan"), 6, 1.f, 1.1f, 220000, 6 },
        { TEXT("K\u00f6stence"), TEXT("K\u00f6stence"), TEXT("Romanya"), 6, 1.f, 1.1f, 230000, 6 },
    };

    bool IsHomeProvince(ECity City) { return FString(CityInfo(City).Province) == TEXT("K\u0131rklareli"); }
    bool IsAbroad(ECity City) { return FString(CityInfo(City).Country) != TEXT("T\u00fcrkiye"); }

    int32 FarStores(const FMarketState& State)
    {
        int32 Far = 0;
        for (const FMarketCityStores& C : State.Company.Cities) if (!IsHomeProvince(static_cast<ECity>(C.City))) Far += C.Stores;
        return Far;
    }

    int32 CountryFirstDay(const FMarketState& State, const TCHAR* Country)
    {
        int32 First = 0;
        for (const FMarketCityStores& C : State.Company.Cities)
            if (C.Stores > 0 && FString(CityInfo(static_cast<ECity>(C.City)).Country) == Country && (First == 0 || C.FirstDay < First)) First = C.FirstDay;
        return First;
    }

    int64 Scaled(const FMarketState& State, int64 Kurus) { return FMath::RoundToInt64(Kurus * MarketPrices::ListLevel(State.Day)); }

    FMarketCityStores& Row(FMarketState& State, ECity City)
    {
        if (FMarketCityStores* Found = State.Company.Cities.FindByPredicate([City](const FMarketCityStores& C) { return C.City == static_cast<uint8>(City); })) return *Found;
        FMarketCityStores New;
        New.City = static_cast<uint8>(City);
        State.Company.Cities.Add(New);
        return State.Company.Cities.Last();
    }
}

const MarketCompany::FCity& MarketCompany::CityInfo(ECity City)
{
    return Cities[FMath::Clamp(static_cast<int32>(City), 0, static_cast<int32>(ECity::Count) - 1)];
}

const FMarketCityStores* MarketCompany::Find(const FMarketState& State, ECity City)
{
    return State.Company.Cities.FindByPredicate([City](const FMarketCityStores& C) { return C.City == static_cast<uint8>(City); });
}

int32 MarketCompany::TotalStores(const FMarketState& State)
{
    int32 Count = 1;
    for (const FMarketBranch& B : State.Branches) if (B.Stage == static_cast<uint8>(MarketBranches::EStage::Open)) ++Count;
    for (const FMarketCityStores& C : State.Company.Cities) Count += C.Stores;
    return Count;
}

int32 MarketCompany::CountryStores(const FMarketState& State, const TCHAR* Country)
{
    int32 Count = 0;
    for (const FMarketCityStores& C : State.Company.Cities)
        if (FString(CityInfo(static_cast<ECity>(C.City)).Country) == Country) Count += C.Stores;
    return Count;
}

int32 MarketCompany::Provinces(const FMarketState& State)
{
    TArray<FString> Seen = { FString(TEXT("K\u0131rklareli")) };
    for (const FMarketCityStores& C : State.Company.Cities)
    {
        const FCity& Info = CityInfo(static_cast<ECity>(C.City));
        if (C.Stores > 0 && !IsAbroad(static_cast<ECity>(C.City)) && !Seen.Contains(FString(Info.Province))) Seen.Add(Info.Province);
    }
    return Seen.Num();
}

float MarketCompany::NationalShare(const FMarketState& State)
{
    return (TotalStores(State) - CountryStores(State, TEXT("Bulgaristan")) - CountryStores(State, TEXT("Romanya"))) * 0.04f;
}

bool MarketCompany::ChapterOpen(const FMarketState& State, int32 Chapter)
{
    return State.Story.Chapter >= Chapter;   // free play after the sale ending (99) keeps everything open
}

float MarketCompany::Margin(const FMarketState& State, ECity City)
{
    const FMarketCompany& C = State.Company;
    float M = 0.20f + (C.bDepot ? 0.015f : 0.f) + (C.bCentralBuying ? 0.02f : 0.f) + (C.bPrivateLabel ? 0.015f : 0.f);
    if (IsAbroad(City))
    {
        const int32 First = CountryFirstDay(State, CityInfo(City).Country);
        if (First > 0 && State.Day - First < LearningDays) M -= 0.03f;   // other suppliers, other habits
    }
    return M;
}

float MarketCompany::LogisticsLoss(const FMarketState& State, ECity City)
{
    if (IsHomeProvince(City)) return 0.f;
    const FMarketCompany& C = State.Company;
    float Loss = IsAbroad(City) ? 0.01f : 0.f;   // customs and paperwork
    if (!C.bDepot) Loss += 0.03f;                // the wholesaler's van for every far store
    else if (C.Trucks * StoresPerTruck < FarStores(State)) Loss += 0.015f;
    return Loss;
}

int64 MarketCompany::StoreDayProfit(const FMarketState& State, ECity City, float Maturity, int32 GameDay)
{
    const FCity& Info = CityInfo(City);
    const double Level = MarketPrices::ListLevel(GameDay);
    const double Revenue = StoreDayRevenue * Info.Income / Info.Competition * FMath::Clamp(Maturity, 0.2f, 1.f)
        * MarketCalendar::TrafficFactor(GameDay, State.RivalSeed) * Level * (State.Company.bPrivateLabel ? 1.03 : 1.0);
    const double Gross = Revenue * (Margin(State, City) - LogisticsLoss(State, City));
    const double Costs = Info.Rent * Level / 30.0 + StoreStaff * 4000.0 * MarketPrices::WageIndex(GameDay) + 5000.0 * Level;
    return FMath::RoundToInt64(Gross - Costs);
}

int64 MarketCompany::StoreCost(const FMarketState& State, ECity City)
{
    // Fit-out, two months' deposit and the opening stock.
    return Scaled(State, 800000 + 2 * CityInfo(City).Rent + 1000000);
}

bool MarketCompany::CanOpenStore(const FMarketState& State, ECity City, FString& OutReason)
{
    const FCity& Info = CityInfo(City);
    if (!ChapterOpen(State, Info.Chapter))
    {
        OutReason = FString::Printf(TEXT("%s i\u00e7in erken: \"%s\" b\u00f6l\u00fcm\u00fc a\u00e7\u0131lmal\u0131."), Info.Name, *MarketStory::ChapterTitle(Info.Chapter));
        return false;
    }
    if (!MarketStaff::HasHr(State) || !MarketStaff::HasAccountant(State)) { OutReason = TEXT("Ba\u015fka \u015fehirde ma\u011faza i\u00e7in \u0130K m\u00fcd\u00fcr\u00fc ve mali m\u00fc\u015favir gerekir."); return false; }
    const FMarketCityStores* Row = Find(State, City);
    if (Row && Row->Stores >= Info.MaxStores) { OutReason = FString::Printf(TEXT("%s'de yeni ma\u011faza i\u00e7in yer kalmad\u0131."), Info.Name); return false; }
    if (State.Cash < StoreCost(State, City)) { OutReason = FString::Printf(TEXT("Ma\u011faza %s; kasada yok."), *CompanyTl(StoreCost(State, City))); return false; }
    return true;
}

bool MarketCompany::OpenStore(FMarketState& State, ECity City, FString& OutMessage)
{
    if (!CanOpenStore(State, City, OutMessage)) return false;
    const int64 Cost = StoreCost(State, City);
    State.Cash -= Cost;   // an investment: it does not count against the day's net
    FMarketCityStores& Row = MarketCompany::Row(State, City);
    if (Row.Stores == 0) Row.FirstDay = State.Day;
    // A new store dilutes the city's habit: its own shoppers still have to learn the way.
    Row.Maturity = (Row.Maturity * Row.Stores + 0.3f) / (Row.Stores + 1);
    ++Row.Stores;
    OutMessage = FString::Printf(TEXT("%s'de ma\u011faza a\u00e7\u0131ld\u0131 (%s). \u015eehirde %d ma\u011faza, \u015firkette toplam %d."),
        CityInfo(City).Name, *CompanyTl(Cost), Row.Stores, TotalStores(State));
    if (Row.Stores == 1 && IsAbroad(City) && CountryStores(State, CityInfo(City).Country) == 1)
        MarketStory::AddMemory(State, FString::Printf(TEXT("%s'da ilk ma\u011faza"), CityInfo(City).Country));
    return true;
}

bool MarketCompany::CloseStore(FMarketState& State, ECity City, FString& OutMessage)
{
    FMarketCityStores* Row = State.Company.Cities.FindByPredicate([City](const FMarketCityStores& C) { return C.City == static_cast<uint8>(City); });
    if (!Row || Row->Stores <= 0) { OutMessage = TEXT("Orada ma\u011faza yok."); return false; }
    --Row->Stores;
    const int64 Deposit = Scaled(State, 2 * CityInfo(City).Rent);
    State.Cash += Deposit;
    OutMessage = FString::Printf(TEXT("%s'de bir ma\u011faza kapand\u0131; depozito geri geldi (%s)."), CityInfo(City).Name, *CompanyTl(Deposit));
    return true;
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
        OutMessage = FString::Printf(TEXT("%s tamam (%s)."), Name, *CompanyTl(Cost));
        return true;
    };
    switch (What)
    {
    case 0:
        if (C.bDepot) { OutMessage = TEXT("B\u00f6lge deposu zaten var."); return false; }
        if (!ChapterOpen(State, 4) || Stores < 4) { OutMessage = TEXT("B\u00f6lge deposu i\u00e7in Trakya b\u00f6l\u00fcm\u00fc ve en az 4 ma\u011faza gerekir."); return false; }
        if (!Pay(3000000, TEXT("B\u00f6lge deposu"))) return false;
        C.bDepot = true;
        MarketStory::AddMemory(State, TEXT("B\u00f6lge deposu a\u00e7\u0131ld\u0131"));
        return true;
    case 1:
        if (!C.bDepot) { OutMessage = TEXT("Kamyon i\u00e7in \u00f6nce b\u00f6lge deposu."); return false; }
        if (!Pay(1200000, TEXT("Kamyon"))) return false;
        if (++C.Trucks == 1) MarketStory::AddMemory(State, TEXT("\u0130lk kamyon yola \u00e7\u0131kt\u0131"));
        return true;
    case 2:
        if (C.bCentralBuying) { OutMessage = TEXT("Merkezi sat\u0131n alma zaten var."); return false; }
        if (!C.bDepot || Stores < 8) { OutMessage = TEXT("Merkezi sat\u0131n alma i\u00e7in depo ve en az 8 ma\u011faza gerekir."); return false; }
        if (!Pay(500000, TEXT("Merkezi sat\u0131n alma"))) return false;
        C.bCentralBuying = true;
        return true;
    case 3:
        if (C.bPrivateLabel) { OutMessage = TEXT("\"Miras\" markas\u0131 zaten raflarda."); return false; }
        if (!ChapterOpen(State, 5) || Stores < 20) { OutMessage = TEXT("\u00d6zel marka i\u00e7in T\u00fcrkiye b\u00f6l\u00fcm\u00fc ve en az 20 ma\u011faza gerekir."); return false; }
        if (!Pay(2000000, TEXT("\"Miras\" \u00f6zel markas\u0131"))) return false;
        C.bPrivateLabel = true;
        MarketStory::AddMemory(State, TEXT("\"Miras\" markal\u0131 ilk \u00fcr\u00fcn rafta"));
        return true;
    case 4:
        if (C.bDarkStore) { OutMessage = TEXT("Karanl\u0131k ma\u011faza zaten var."); return false; }
        if (!State.Online.bWeb || !ChapterOpen(State, 5) || Stores < 20) { OutMessage = TEXT("Karanl\u0131k ma\u011faza i\u00e7in web sitesi, T\u00fcrkiye b\u00f6l\u00fcm\u00fc ve en az 20 ma\u011faza gerekir."); return false; }
        if (!Pay(1500000, TEXT("Karanl\u0131k ma\u011faza (yaln\u0131z sipari\u015f toplayan depo)"))) return false;
        C.bDarkStore = true;
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
    TArray<FString> Built;
    if (C.bDepot) Built.Add(TEXT("depo"));
    if (C.Trucks > 0) Built.Add(FString::Printf(TEXT("%d kamyon"), C.Trucks));
    if (C.bCentralBuying) Built.Add(TEXT("merkezi al\u0131m"));
    if (C.bPrivateLabel) Built.Add(TEXT("Miras markas\u0131"));
    if (C.bDarkStore) Built.Add(TEXT("karanl\u0131k ma\u011faza"));
    if (Built.Num()) Line += TEXT(" \u00b7 ") + FString::Join(Built, TEXT(", "));
    if (C.Cities.Num()) Line += FString::Printf(TEXT(" \u00b7 d\u00fcn \u015fehirler %s"), *CompanyTl(C.LastProfit));
    for (const FMarketCityStores& Row : C.Cities)
        if (Row.Stores > 0)
            Line += FString::Printf(TEXT("\n  %s: %d ma\u011faza, al\u0131\u015fkanl\u0131k %%%.0f, d\u00fcn %s"), CityInfo(static_cast<ECity>(Row.City)).Name, Row.Stores, Row.Maturity * 100.f, *CompanyTl(Row.LastProfit));
    if (State.Story.Chapter == 7) Line += FString::Printf(TEXT("\nLiderlik: %d / %d g\u00fcn"), C.LeadershipDays, LeadershipGoalDays);
    return Line;
}

void MarketCompany::CloseDay(FMarketState& State)
{
    FMarketCompany& C = State.Company;
    const int32 Closed = State.Day - 1;
    if (Closed < 1) return;
    int64 Total = 0;
    for (FMarketCityStores& Row : C.Cities)
    {
        Row.LastProfit = 0;
        if (Row.Stores <= 0) continue;
        Row.LastProfit = StoreDayProfit(State, static_cast<ECity>(Row.City), Row.Maturity, Closed) * Row.Stores;
        Row.Last30Profit = Row.Last30Profit * 29 / 30 + Row.LastProfit;
        Row.Maturity = FMath::Min(1.f, Row.Maturity + 1.f / MaturityDays);
        Total += Row.LastProfit;
    }
    // Head office: regional managers, the depot, trucks, the dark store.
    const double Level = MarketPrices::ListLevel(Closed);
    const int32 Stores = TotalStores(State);
    int64 Office = Stores > 8 ? FMath::RoundToInt64((Stores / 10) * 5000.0 * MarketPrices::WageIndex(Closed)) : 0;
    Office += C.bDepot ? FMath::RoundToInt64(20000 * Level) : 0;
    Office += FMath::RoundToInt64(C.Trucks * 6000 * Level);
    Office += C.bDarkStore ? FMath::RoundToInt64(30000 * Level) : 0;
    Total -= Office;
    C.LastProfit = Total;
    C.WeekProfit += Total;
    State.LastBranchProfit += Total;
    State.LastProfit += Total;
    State.Cash += Total;

    // Chapter 7: a year of leading on every measure.
    if (State.Story.Chapter == 7)
    {
        if (LeadsToday(State)) ++C.LeadershipDays;
        else C.LeadershipDays = FMath::Max(0, C.LeadershipDays - 3);
        if (C.LeadershipDays >= LeadershipGoalDays && State.Story.Ending != static_cast<uint8>(MarketStory::EEnding::Legacy))
        {
            State.Story.Ending = static_cast<uint8>(MarketStory::EEnding::Legacy);
            MarketStory::AddMemory(State, TEXT("Miras: babadan kalan d\u00fckk\u00e2n, herkesin bildi\u011fi bir tabela oldu"));
            State.DayNews.Add(TEXT("Bir y\u0131ld\u0131r her \u00f6l\u00e7\u00fcde \u00f6ndesin. Nermin teyze tabelaya bak\u0131p g\u00fcl\u00fcmsedi: \"Baban g\u00f6rseydi...\" Oyun serbest devam ediyor."));
        }
    }
    if (MarketCalendar::DateOf(Closed).Weekday == 6)
    {
        if (C.Cities.Num() > 0 || C.bDepot)
            State.DayNews.Add(FString::Printf(TEXT("\u015eirket haftas\u0131: %d ma\u011faza, \u015fehir ma\u011fazalar\u0131 ve merkez net %s."), Stores, *CompanyTl(C.WeekProfit)));
        C.WeekProfit = 0;
    }
}

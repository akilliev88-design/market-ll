#include "MarketLeague.h"
#include "MarketEconomy.h"
#include "MarketBranches.h"
#include "MarketCalendar.h"
#include "MarketCountry.h"
#include "MarketEras.h"
#include "MarketFinance.h"
#include "MarketLedger.h"
#include "MarketPrices.h"
#include "MarketStory.h"
#include "MarketSuppliers.h"

namespace MarketLeague
{
    uint32 LeagueMix(int32 Seed, uint32 A, uint32 B)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), A, B };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }

    // -1..1, a little bell-shaped (average of two rolls).
    double Wobble(int32 Seed, uint32 A, uint32 B)
    {
        const uint32 H = LeagueMix(Seed, A, B);
        return ((H & 0xFFFFu) / 65535.0 + (H >> 16) / 65535.0) - 1.0;
    }

    uint32 CountryKey(const FString& Id)
    {
        uint32 Hash = 5381u;
        for (const TCHAR C : Id) Hash = Hash * 33u + static_cast<uint32>(C);
        return Hash;
    }

    // The giants (fictional names close to the real ones, karar L12): revenue of the first league year in billions
    // (the real world's scale) and their trend a year. Research: MarketRetail::Global (Rakipler page).
    struct FGiant { const TCHAR* Name; const TCHAR* Home; double StartBillions; double Growth; };
    const FGiant Giants[] =
    {
        { TEXT("Walmer"), TEXT("ABD \u00b7 hipermarket"), 447.0, 0.025 },
        { TEXT("Carrefort"), TEXT("Fransa \u00b7 hipermarket"), 113.0, -0.01 },
        { TEXT("Tesca"), TEXT("\u0130ngiltere \u00b7 s\u00fcpermarket"), 102.0, -0.005 },
        { TEXT("Metra"), TEXT("Almanya \u00b7 toptan"), 93.0, -0.02 },
        { TEXT("Krogen"), TEXT("ABD \u00b7 s\u00fcpermarket"), 90.0, 0.025 },
        { TEXT("Costa Depo"), TEXT("ABD \u00b7 \u00fcyelikli depo"), 89.0, 0.055 },
        { TEXT("Schwarzmann"), TEXT("Almanya \u00b7 indirim ve hipermarket"), 87.0, 0.045 },
        { TEXT("Aldino"), TEXT("Almanya \u00b7 indirim marketi"), 73.0, 0.03 },
    };

    double WalkSize(MarketCountry::ECharacter Character)
    {
        switch (Character)
        {
        case MarketCountry::ECharacter::Stable: return 0.03;
        case MarketCountry::ECharacter::Volatile: return 0.08;
        default: return 0.06;
        }
    }
}

double MarketLeague::FxRate(const FMarketState& State, const FString& CountryId, int32 GameDay)
{
    const FString Id = CountryId.IsEmpty() ? State.CountryId : CountryId;
    const MarketCountry::FProfile* Pack = MarketCountry::Find(Id);
    const MarketCountry::FProfile& P = Pack ? *Pack : MarketCountry::Active();
    const bool bOwn = Id == State.CountryId;
    double Log = FMath::Loge(FMath::Max(0.0001, P.FxPerWorld));
    const int32 Day = FMath::Max(1, GameDay);
    const int32 FirstYear = MarketCalendar::DateOf(1).Year;
    const int32 LastYearOfDay = MarketCalendar::DateOf(Day).Year;
    for (int32 Year = FirstYear; Year <= LastYearOfDay; ++Year)
    {
        const int32 From = FMath::Max(1, MarketCalendar::GameDayOf(Year, 1, 1));
        const int32 To = FMath::Min(Day, MarketCalendar::GameDayOf(Year + 1, 1, 1));
        if (To <= From) continue;
        const double Part = (To - From) / 365.0;
        // Own country: the campaign's price curve (eras included); others: their pack's average.
        const double Inflation = bOwn ? MarketPrices::YearlyInflation(Year) : P.InflationMean;
        Log += (FMath::Loge(1.0 + Inflation) - FMath::Loge(1.0 + WorldInflation)) * Part;
        Log += WalkSize(P.Character) * Wobble(State.RivalSeed, CountryKey(Id), static_cast<uint32>(Year)) * Part;
    }
    if (bOwn)
        for (const MarketEras::FEra& E : MarketEras::PlanOf(State))
            if (E.Kind == MarketEras::EKind::CurrencyShock && Day > E.StartDay)
                Log += 0.25 * E.Strength * FMath::Min(1.0, (Day - E.StartDay) / 10.0); // the jump takes about ten days
    return FMath::Exp(Log);
}

int64 MarketLeague::ToWorld(const FMarketState& State, const FString& CountryId, int64 Internal, int32 GameDay)
{
    const FString Id = CountryId.IsEmpty() ? State.CountryId : CountryId;
    const MarketCountry::FProfile* Pack = MarketCountry::Find(Id);
    const double Scale = Pack ? Pack->DisplayScale : 1.0;
    return FMath::RoundToInt64(static_cast<double>(Internal) * Scale / FMath::Max(0.0001, FxRate(State, Id, GameDay)));
}

int32 MarketLeague::GiantCount()
{
    return static_cast<int32>(UE_ARRAY_COUNT(Giants));
}

int64 MarketLeague::GiantRevenue(const FMarketState& State, int32 Giant, int32 LeagueYear)
{
    if (Giant < 0 || Giant >= GiantCount()) return 0;
    const FGiant& G = Giants[Giant];
    // One giant stumbles for five years somewhere in the campaign (which one and when: the seed).
    const int32 Stumbler = static_cast<int32>(LeagueMix(State.RivalSeed, 0x6A17u, 1u) % static_cast<uint32>(GiantCount()));
    const int32 StumbleFrom = 6 + static_cast<int32>(LeagueMix(State.RivalSeed, 0x6A17u, 2u) % 12u);
    double Revenue = G.StartBillions;
    for (int32 Year = 2; Year <= LeagueYear; ++Year)
    {
        double Growth = G.Growth + 0.015 * Wobble(State.RivalSeed, 0x61A0u + static_cast<uint32>(Giant), static_cast<uint32>(Year));
        if (Giant == Stumbler && Year >= StumbleFrom && Year < StumbleFrom + 5) Growth -= 0.04;
        Revenue *= 1.0 + Growth;
    }
    return FMath::RoundToInt64(Revenue * 1.0e9 * 100.0 * WorldScale);
}

int32 MarketLeague::LeagueYearOf(int32 GameDay)
{
    return (FMath::Max(1, GameDay) - 1) / YearDays + 1;
}

TArray<MarketLeague::FEntry> MarketLeague::Table(const FMarketState& State, int32 LeagueYear, int64 OurRevenue)
{
    TArray<FEntry> Rows;
    for (int32 I = 0; I < GiantCount(); ++I)
    {
        FEntry E;
        E.Name = Giants[I].Name;
        E.Home = Giants[I].Home;
        E.Revenue = GiantRevenue(State, I, LeagueYear);
        Rows.Add(E);
    }
    FEntry Us;
    Us.Name = TEXT("Miras Market");
    Us.Home = MarketCountry::Find(State.CountryId) ? MarketCountry::Find(State.CountryId)->Name : FString();
    Us.Revenue = OurRevenue;
    Us.bPlayer = true;
    Rows.Add(Us);
    // Ties: the giant stays in front.
    Rows.StableSort([](const FEntry& A, const FEntry& B) { return A.Revenue != B.Revenue ? A.Revenue > B.Revenue : !A.bPlayer && B.bPlayer; });
    return Rows;
}

TArray<MarketLeague::FEntry> MarketLeague::CurrentTable(const FMarketState& State)
{
    const FMarketLeague& L = State.League;
    if (L.LastRank > 0) return Table(State, L.LastYear, L.LastYearRevenue);
    const int64 Scaled = L.YearDays > 0 ? L.YearRevenue * YearDays / L.YearDays : 0;
    return Table(State, FMath::Max(1, L.LeagueYear), Scaled);
}

int32 MarketLeague::CurrentRank(const FMarketState& State)
{
    const TArray<FEntry> Rows = CurrentTable(State);
    const int32 At = Rows.IndexOfByPredicate([](const FEntry& E) { return E.bPlayer; });
    return At + 1;
}

bool MarketLeague::MoneyOk(const FMarketState& State, int64 YearEbitda)
{
    if (YearEbitda <= 0) return false;
    const int64 Debt = MarketFinance::Debt(State) + MarketSuppliers::OpenBills(State);
    return static_cast<double>(Debt) < DebtToEbitda * static_cast<double>(YearEbitda);
}

FString MarketLeague::Summary(const FMarketState& State)
{
    const TArray<FEntry> Rows = CurrentTable(State);
    const int32 At = Rows.IndexOfByPredicate([](const FEntry& E) { return E.bPlayer; });
    if (At == INDEX_NONE || Rows.Num() == 0) return FString();
    if (At == 0) return FString::Printf(TEXT("D\u00fcnya ligi: 1. s\u0131ra; ikincinin %.1f kat\u0131 ciro."), Rows.Num() > 1 && Rows[1].Revenue > 0 ? static_cast<double>(Rows[0].Revenue) / Rows[1].Revenue : 1.0);
    const double Part = Rows[0].Revenue > 0 ? 100.0 * static_cast<double>(Rows[At].Revenue) / Rows[0].Revenue : 0.0;
    return FString::Printf(TEXT("D\u00fcnya ligi: %d. s\u0131ra; birincinin %%%.1f'i kadar ciro."), At + 1, Part);
}

void MarketLeague::CloseDay(FMarketState& State)
{
    const int32 Closed = State.Day - 1;
    if (Closed < 1) return;
    FMarketLeague& L = State.League;
    const int32 Year = LeagueYearOf(Closed);
    if (L.LeagueYear == 0) L.LeagueYear = Year; // an older save joins the league now (its first year counts from here)
    // The day in world units: the family shop (with its online orders) and every open branch in its own money.
    int64 Revenue = ToWorld(State, State.CountryId, FMath::Max<int64>(0, State.LastRevenue), Closed);
    for (const FMarketBranch& B : State.Branches)
        if (B.Stage == static_cast<uint8>(MarketBranches::EStage::Open))
            Revenue += ToWorld(State, MarketBranches::CountryOf(State, B), FMath::Max<int64>(0, B.LastRevenue), Closed);
    // Operating result of the day from the books: net profit before interest and tax.
    const MarketLedger::FStatement Day = MarketLedger::DayStatement(State, Closed);
    L.YearRevenue += Revenue;
    L.YearEbitda += Day.NetProfit - Day.At(MarketLedger::EAccount::Interest) - Day.At(MarketLedger::EAccount::Tax);
    ++L.YearDays;
    if (Closed % YearDays != 0) return;

    // The league year is over.
    const bool bFullYear = L.YearDays >= YearDays - 5;
    const int64 Ours = L.YearDays > 0 ? L.YearRevenue * YearDays / L.YearDays : 0;
    const TArray<FEntry> Rows = Table(State, Year, Ours);
    const int32 Rank = Rows.IndexOfByPredicate([](const FEntry& E) { return E.bPlayer; }) + 1;
    const bool bMoney = MoneyOk(State, L.YearEbitda);
    L.FirstPlaceYears = Rank == 1 && bMoney && bFullYear ? L.FirstPlaceYears + 1 : 0;
    L.LastYear = Year;
    L.LastYearRevenue = Ours;
    L.LastRank = Rank;
    L.BestRank = L.BestRank == 0 ? Rank : FMath::Min(L.BestRank, Rank);
    const double Part = Rows[0].Revenue > 0 ? 100.0 * static_cast<double>(Ours) / Rows[0].Revenue : 0.0;
    if (Rank == 1)
        State.DayNews.Add(FString::Printf(TEXT("%d. y\u0131l d\u00fcnya ligi: 1. s\u0131radas\u0131n.%s"), Year,
            bMoney ? TEXT("") : TEXT(" Ama bor\u00e7 k\u00e2r\u0131n \u00fc\u00e7 kat\u0131n\u0131 a\u015f\u0131yor; birincilik say\u0131lmad\u0131.")));
    else
        State.DayNews.Add(FString::Printf(TEXT("%d. y\u0131l d\u00fcnya ligi: %d \u015firket i\u00e7inde %d. s\u0131radas\u0131n; birincinin %%%.1f'i kadar ciro."), Year, Rows.Num(), Rank, Part));
    L.YearRevenue = 0;
    L.YearEbitda = 0;
    L.YearDays = 0;
    L.LeagueYear = Year + 1;
    // Karar J02: two league years in a row as the first, in the last chapter, bring the "Miras" finale.
    if (L.FirstPlaceYears >= FinaleYears && State.Story.Chapter >= 7 && !MarketStory::StoryClosed(State))
        MarketStory::ReachFinale(State, MarketStory::EEnding::Legacy);
}

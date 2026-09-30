#include "MarketDepots.h"
#include "MarketBranches.h"
#include "MarketCompany.h"
#include "MarketCountry.h"
#include "MarketManagers.h"
#include "MarketPrices.h"
#include "MarketStaff.h"
#include "MarketStart.h"
#include "MarketStory.h"

namespace MarketDepots
{
    constexpr double DepotGoodsShare = 0.8;      // goods at cost of a branch's revenue (the advice's estimate)
    constexpr float DepotAdviceEfficiency = 0.8f; // an ordinary depot manager (the advice's estimate)

    uint32 DepotMix(int32 Seed, int32 Day, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), static_cast<uint32>(Day), Salt };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }

    FString DepotCountry(const FMarketState& State, const FString& Country)
    {
        return Country.IsEmpty() ? State.CountryId : Country;
    }

    bool DepotBranchOpen(const FMarketBranch& B) { return B.Stage == static_cast<uint8>(MarketBranches::EStage::Open); }

    FString DepotBranchProvince(const FMarketState& State, const FMarketBranch& B)
    {
        return B.Province.IsEmpty() ? MarketStart::HomeProvince(State) : B.Province;
    }

    bool IsHomePlace(const FMarketState& State, const FString& Country, const FString& Province)
    {
        return Country == State.CountryId && Province == MarketStart::HomeProvince(State);
    }

    float DepotRentFactor(const FMarketState& State, const FString& Country, const FString& Province)
    {
        return MarketBranches::SiteOf(State, Country, Province).Rent;
    }

    FString DepotMoney(int64 Kurus) { return MarketCountry::Money(Kurus); }

    // Does the depot's manager take goods now (dishonest, not deterred by a recent warning)?
    bool DepotSkims(const FMarketState& State, int32 DepotIndex)
    {
        const int32 M = ManagerOf(State, DepotIndex);
        if (M == INDEX_NONE) return false;
        const FMarketManager& Man = State.Management.Managers[M];
        const bool bDeterred = Man.WarnedDay > 0 && State.Day - Man.WarnedDay < MarketManagers::WarnDeterDays;
        return Man.Honesty < 35 && !bDeterred;
    }

    float EfficiencyWith(const FMarketState& State, int32 DepotIndex, int32 ServedNow)
    {
        if (!State.Company.DepotSites.IsValidIndex(DepotIndex)) return 0.f;
        const FMarketDepot& Depot = State.Company.DepotSites[DepotIndex];
        float Value = UnmanagedEfficiency;
        const int32 M = ManagerOf(State, DepotIndex);
        if (M != INDEX_NONE)
        {
            const FMarketManager& Man = State.Management.Managers[M];
            int32 Skill = MarketManagers::EffectiveManagerSkill(State, M);
            if (Man.AppointedDay > 0 && State.Day - Man.AppointedDay < MarketManagers::SettleDays) Skill -= MarketManagers::SettlePenalty;
            Value = 0.6f + 0.4f * FMath::Clamp((Skill - 20) / 60.f, 0.f, 1.f);
        }
        const int32 Cap = FMath::Max(1, Depot.Capacity);
        if (ServedNow > Cap) Value *= static_cast<float>(Cap) / static_cast<float>(ServedNow);
        return FMath::Clamp(Value, 0.2f, 1.f);
    }

    FLink MakeLink(const FMarketState& State, int32 DepotIndex, float Km, float Eff, float Shortage)
    {
        FLink Link;
        Link.Depot = DepotIndex;
        Link.Km = Km;
        Link.Efficiency = Eff;
        Link.CostAdd = -Rebate * Eff + DistanceCost(Km) + TruckShortCost * Shortage;
        Link.ShortPermille = FMath::RoundToInt32(MaxShortPermille * (1.f - Eff) + TruckShortPermille * Shortage);
        Link.ExtraWaste = MaxExtraWaste * (1.f - Eff);
        Link.SkimPermille = DepotSkims(State, DepotIndex) ? SkimPermille : 0;
        return Link;
    }

    // Nearest depot of every open branch (INDEX_NONE: none) and its distance.
    void NearestAll(const FMarketState& State, TArray<int32>& OutDepot, TArray<float>& OutKm)
    {
        OutDepot.Init(INDEX_NONE, State.Branches.Num());
        OutKm.Init(0.f, State.Branches.Num());
        if (State.Company.DepotSites.Num() == 0) return;
        for (int32 I = 0; I < State.Branches.Num(); ++I)
        {
            const FMarketBranch& B = State.Branches[I];
            if (!DepotBranchOpen(B)) continue;
            const FString Country = MarketBranches::CountryOf(State, B);
            const FString Province = DepotBranchProvince(State, B);
            float Km = 0.f;
            OutDepot[I] = Nearest(State, Country, Province, IsHomePlace(State, Country, Province), Km);
            OutKm[I] = Km;
        }
    }

    float LoadsOf(const TArray<int32>& Depot, const TArray<float>& Km)
    {
        float Loads = 0.f;
        for (int32 I = 0; I < Depot.Num(); ++I)
            if (Depot[I] != INDEX_NONE) Loads += 1.f + Km[I] / static_cast<float>(LoadKm);
        return Loads;
    }

    int32 TrucksFor(float Loads)
    {
        if (Loads <= 0.f) return 0;
        int32 Needed = FMath::FloorToInt32(Loads / static_cast<float>(LoadsPerTruck));
        if (static_cast<float>(Needed * LoadsPerTruck) < Loads) ++Needed;
        return Needed;
    }

    float ShortageFor(const FMarketState& State, int32 Needed)
    {
        if (Needed <= 0) return 0.f;
        return FMath::Clamp(1.f - static_cast<float>(State.Company.Trucks) / static_cast<float>(Needed), 0.f, 1.f);
    }

    // The province of a sub-region an older depot moves to: most of our shops, else the most people.
    FString MigrationProvince(const FMarketState& State, const FString& Country, const MarketCountry::FRegion& Sub)
    {
        FString Best;
        int32 BestShops = -1, BestPeople = -1;
        for (const FString& Province : Sub.Provinces)
        {
            const MarketCountry::FCity* City = MarketCountry::FindCity(Country, Province);
            if (!City) continue;
            const int32 Shops = MarketBranches::ShopsIn(State, Country, Province);
            if (Shops > BestShops || (Shops == BestShops && City->PopulationK > BestPeople))
            {
                Best = Province;
                BestShops = Shops;
                BestPeople = City->PopulationK;
            }
        }
        return Best;
    }

    FMarketDepot NewDepot(const FMarketState& State, const FString& Country, const FString& Province)
    {
        FMarketDepot Depot;
        Depot.Country = Country;
        Depot.Province = Province;
        Depot.OpenedDay = FMath::Max(1, State.Day);
        Depot.Rent = FMath::RoundToInt64(static_cast<double>(MonthlyRent2011) * DepotRentFactor(State, Country, Province));
        Depot.Capacity = DefaultCapacity;
        return Depot;
    }
}

float MarketDepots::DistanceKm(const FString& Country, const FString& FromProvince, const FString& ToProvince)
{
    if (FromProvince == ToProvince) return 0.f;
    const MarketCountry::FProfile* Pack = MarketCountry::Find(Country);
    const MarketCountry::FCity* From = MarketCountry::FindCity(Country, FromProvince);
    const MarketCountry::FCity* To = MarketCountry::FindCity(Country, ToProvince);
    if (!Pack || !From || !To) return static_cast<float>(OtherRegionKm);
    if (From->bOnMap && To->bOnMap)
    {
        const float Dx = From->MapX - To->MapX;
        const float Dy = From->MapY - To->MapY;
        return FMath::Sqrt(Dx * Dx + Dy * Dy) * Pack->MapKm;
    }
    if (!From->SubRegion.IsEmpty() && From->SubRegion == To->SubRegion) return static_cast<float>(SameSubRegionKm);
    if (!From->Region.IsEmpty() && From->Region == To->Region) return static_cast<float>(SameRegionKm);
    return static_cast<float>(OtherRegionKm);
}

float MarketDepots::DistanceCost(float Km)
{
    return CostPer100Km * FMath::Max(0.f, Km - static_cast<float>(FreeKm)) / 100.f;
}

int32 MarketDepots::Count(const FMarketState& State)
{
    return State.Company.DepotSites.Num() + State.Company.Depots.Num();
}

int32 MarketDepots::Find(const FMarketState& State, const FString& Country, const FString& Province)
{
    const FString C = DepotCountry(State, Country);
    return State.Company.DepotSites.IndexOfByPredicate([&C, &Province](const FMarketDepot& D) { return D.Country == C && D.Province == Province; });
}

bool MarketDepots::HasDepotIn(const FMarketState& State, const FString& Country, const FString& Province)
{
    return Find(State, Country, Province) != INDEX_NONE;
}

bool MarketDepots::HasDepotInSubRegion(const FMarketState& State, const FString& Country, const FString& SubRegion)
{
    if (SubRegion.IsEmpty()) return false;
    const FString C = DepotCountry(State, Country);
    if (State.Company.Depots.Contains(C + TEXT(":") + SubRegion)) return true; // an older save before Migrate
    for (const FMarketDepot& D : State.Company.DepotSites)
    {
        if (D.Country != C) continue;
        const MarketCountry::FCity* City = MarketCountry::FindCity(C, D.Province);
        if (City && City->SubRegion == SubRegion) return true;
    }
    return false;
}

FString MarketDepots::DepotName(const FMarketState& State, int32 DepotIndex)
{
    if (!State.Company.DepotSites.IsValidIndex(DepotIndex)) return FString();
    const FMarketDepot& D = State.Company.DepotSites[DepotIndex];
    const MarketCountry::FCity* City = MarketCountry::FindCity(D.Country, D.Province);
    return (City ? City->Name : D.Province) + TEXT(" deposu");
}

int32 MarketDepots::ManagerOf(const FMarketState& State, int32 DepotIndex)
{
    if (!State.Company.DepotSites.IsValidIndex(DepotIndex)) return INDEX_NONE;
    const FMarketDepot& D = State.Company.DepotSites[DepotIndex];
    return MarketManagers::FindManager(State, MarketManagers::ELevel::Depot, D.Country, D.Province);
}

int32 MarketDepots::Nearest(const FMarketState& State, const FString& Country, const FString& Province, bool bHome, float& OutKm)
{
    const FString C = DepotCountry(State, Country);
    const float Limit = static_cast<float>(bHome ? HomeRangeKm : RangeKm);
    int32 Best = INDEX_NONE;
    OutKm = 0.f;
    const TArray<FMarketDepot>& Sites = State.Company.DepotSites;
    for (int32 D = 0; D < Sites.Num(); ++D)
    {
        if (Sites[D].Country != C) continue;
        const float Km = DistanceKm(C, Province, Sites[D].Province);
        if (Km > Limit) continue;
        if (Best == INDEX_NONE || Km < OutKm) { Best = D; OutKm = Km; }
    }
    return Best;
}

TArray<MarketDepots::FLink> MarketDepots::AllLinks(const FMarketState& State)
{
    TArray<FLink> Links;
    Links.Init(FLink(), State.Branches.Num());
    const int32 Depots = State.Company.DepotSites.Num();
    if (Depots == 0) return Links;
    TArray<int32> Depot;
    TArray<float> Km;
    NearestAll(State, Depot, Km);
    TArray<int32> ServedCount;
    ServedCount.Init(0, Depots);
    for (int32 I = 0; I < Depot.Num(); ++I) if (Depot[I] != INDEX_NONE) ++ServedCount[Depot[I]];
    TArray<float> Eff;
    Eff.Init(0.f, Depots);
    for (int32 D = 0; D < Depots; ++D) Eff[D] = EfficiencyWith(State, D, ServedCount[D]);
    const float Shortage = ShortageFor(State, TrucksFor(LoadsOf(Depot, Km)));
    for (int32 I = 0; I < Depot.Num(); ++I)
        if (Depot[I] != INDEX_NONE) Links[I] = MakeLink(State, Depot[I], Km[I], Eff[Depot[I]], Shortage);
    return Links;
}

MarketDepots::FLink MarketDepots::LinkFor(const FMarketState& State, const FString& Country, const FString& Province)
{
    const FString C = DepotCountry(State, Country);
    float Km = 0.f;
    const int32 D = Nearest(State, C, Province, IsHomePlace(State, C, Province), Km);
    if (D == INDEX_NONE) return FLink();
    return MakeLink(State, D, Km, Efficiency(State, D), TruckShortage(State));
}

MarketDepots::FLink MarketDepots::LinkOf(const FMarketState& State, const FMarketBranch& Branch)
{
    return LinkFor(State, MarketBranches::CountryOf(State, Branch), DepotBranchProvince(State, Branch));
}

TArray<int32> MarketDepots::ServedBranches(const FMarketState& State, int32 DepotIndex)
{
    TArray<int32> List;
    if (!State.Company.DepotSites.IsValidIndex(DepotIndex)) return List;
    TArray<int32> Depot;
    TArray<float> Km;
    NearestAll(State, Depot, Km);
    for (int32 I = 0; I < Depot.Num(); ++I) if (Depot[I] == DepotIndex) List.Add(I);
    return List;
}

int32 MarketDepots::Served(const FMarketState& State, int32 DepotIndex)
{
    return ServedBranches(State, DepotIndex).Num();
}

float MarketDepots::Efficiency(const FMarketState& State, int32 DepotIndex)
{
    return EfficiencyWith(State, DepotIndex, Served(State, DepotIndex));
}

float MarketDepots::TruckLoads(const FMarketState& State)
{
    TArray<int32> Depot;
    TArray<float> Km;
    NearestAll(State, Depot, Km);
    return LoadsOf(Depot, Km);
}

int32 MarketDepots::TrucksNeeded(const FMarketState& State)
{
    return TrucksFor(TruckLoads(State));
}

float MarketDepots::TruckShortage(const FMarketState& State)
{
    return ShortageFor(State, TrucksNeeded(State));
}

int64 MarketDepots::BuildCost(const FMarketState& State, const FString& Country, const FString& Province)
{
    const FString C = DepotCountry(State, Country);
    return FMath::RoundToInt64(static_cast<double>(BuildCost2011) * DepotRentFactor(State, C, Province) * MarketPrices::ListLevel(State.Day));
}

int64 MarketDepots::MonthlyRent(const FMarketState& State, const FString& Country, const FString& Province)
{
    const FString C = DepotCountry(State, Country);
    return FMath::RoundToInt64(static_cast<double>(MonthlyRent2011) * DepotRentFactor(State, C, Province) * MarketPrices::ListLevel(State.Day));
}

bool MarketDepots::CanBuild(const FMarketState& State, const FString& Country, const FString& Province, FString& OutReason)
{
    const FString C = DepotCountry(State, Country);
    const MarketCountry::FCity* City = MarketCountry::FindCity(C, Province);
    if (!City) { OutReason = TEXT("B\u00f6yle bir il yok."); return false; }
    if (HasDepotIn(State, C, Province)) { OutReason = FString::Printf(TEXT("%s ilinde zaten depo var."), *City->Name); return false; }
    if (MarketCompany::TotalStores(State) < MinStores) { OutReason = FString::Printf(TEXT("Depo i\u00e7in en az %d ma\u011faza gerekir."), MinStores); return false; }
    bool bNear = false;
    for (const FMarketBranch& B : State.Branches)
        if (DepotBranchOpen(B) && MarketBranches::CountryOf(State, B) == C && DistanceKm(C, DepotBranchProvince(State, B), Province) <= static_cast<float>(RangeKm)) { bNear = true; break; }
    if (!bNear) { OutReason = FString::Printf(TEXT("%s ilinin %d km \u00e7evresinde a\u00e7\u0131k \u015fuben yok; depo bo\u015f kal\u0131r."), *City->Name, RangeKm); return false; }
    const int64 Cost = BuildCost(State, C, Province);
    if (State.Cash < Cost) { OutReason = FString::Printf(TEXT("%s deposu %s; kasada yok."), *City->Name, *DepotMoney(Cost)); return false; }
    return true;
}

bool MarketDepots::Build(FMarketState& State, const FString& Country, const FString& Province, FString& OutMessage)
{
    const FString C = DepotCountry(State, Country);
    if (!CanBuild(State, C, Province, OutMessage)) return false;
    const int64 Cost = BuildCost(State, C, Province);
    State.Cash -= Cost;
    State.Company.DepotSites.Add(NewDepot(State, C, Province));
    const int32 Index = State.Company.DepotSites.Num() - 1;
    const FString Name = DepotName(State, Index);
    OutMessage = FString::Printf(TEXT("%s a\u00e7\u0131ld\u0131 (%s, ayl\u0131k kira %s). %d \u015fube mal\u0131 buradan alacak. Depoya m\u00fcd\u00fcr ata: m\u00fcd\u00fcrs\u00fcz depo yar\u0131 verimle \u00e7al\u0131\u015f\u0131r."),
        *Name, *DepotMoney(Cost), *DepotMoney(MonthlyRent(State, C, Province)), Served(State, Index));
    if (State.Company.DepotSites.Num() == 1) MarketStory::AddMemory(State, FString::Printf(TEXT("%s a\u00e7\u0131ld\u0131"), *Name));
    return true;
}

MarketDepots::FAdvice MarketDepots::SuggestDepotProvince(const FMarketState& State, const FString& Country, const FString& WithinSubRegion)
{
    FAdvice Advice;
    const FString C = DepotCountry(State, Country);
    Advice.Country = C;
    const MarketCountry::FProfile* Pack = MarketCountry::Find(C);
    if (!Pack) { Advice.Text = TEXT("B\u00f6yle bir \u00fclke yok."); return Advice; }

    // The branches of the country: revenue weight, where they are, how they get goods today.
    struct FSpot
    {
        FString Province;
        double Weight = 1.0;
        bool bHome = false;
        float NowKm = 0.f;        // to the depot they use now (RangeKm + FreeKm when none)
        float NowCost = 0.f;      // share of the goods they pay on top now
    };
    TArray<FSpot> Spots;
    const double Floor = 300000.0 * MarketPrices::ListLevel(State.Day); // a new branch weighs like a small day
    for (const FMarketBranch& B : State.Branches)
    {
        if (!DepotBranchOpen(B) || MarketBranches::CountryOf(State, B) != C) continue;
        FSpot Spot;
        Spot.Province = DepotBranchProvince(State, B);
        Spot.Weight = FMath::Max(static_cast<double>(B.LastRevenue), Floor);
        Spot.bHome = IsHomePlace(State, C, Spot.Province);
        float Km = 0.f;
        const int32 D = Nearest(State, C, Spot.Province, Spot.bHome, Km);
        Spot.NowKm = D != INDEX_NONE ? Km : static_cast<float>(RangeKm + FreeKm);
        Spot.NowCost = D != INDEX_NONE ? -Rebate * DepotAdviceEfficiency + DistanceCost(Km) : (Spot.bHome ? 0.f : WholesalerVan);
        Spots.Add(Spot);
    }
    if (Spots.Num() == 0) { Advice.Text = FString::Printf(TEXT("%s \u015fuben yok; depo i\u00e7in \u00f6nce \u015fube a\u00e7."), *Pack->Name); return Advice; }

    // The province with the smallest revenue-weighted distance of every branch to its nearest depot.
    double BestScore = 0.0;
    int32 BestPeople = -1;
    for (const MarketCountry::FCity& City : Pack->Cities)
    {
        if (!WithinSubRegion.IsEmpty() && City.SubRegion != WithinSubRegion) continue;
        if (HasDepotIn(State, C, City.Id)) continue;
        double Score = 0.0;
        for (const FSpot& Spot : Spots)
        {
            const float Km = DistanceKm(C, Spot.Province, City.Id);
            const float Limit = static_cast<float>(Spot.bHome ? HomeRangeKm : RangeKm);
            Score += Spot.Weight * (Km <= Limit ? FMath::Min(Km, Spot.NowKm) : Spot.NowKm);
        }
        if (Advice.Province.IsEmpty() || Score < BestScore - 0.5 || (Score <= BestScore + 0.5 && City.PopulationK > BestPeople))
        {
            Advice.Province = City.Id;
            BestScore = Score;
            BestPeople = City.PopulationK;
        }
    }
    if (Advice.Province.IsEmpty()) { Advice.Text = TEXT("Depo kurulacak uygun il kalmad\u0131."); return Advice; }

    // What it would bring: the branches that would switch to it, their distance and the cheaper goods a month.
    double Weight = 0.0, WeightedKm = 0.0, Gain = 0.0;
    for (const FSpot& Spot : Spots)
    {
        const float Km = DistanceKm(C, Spot.Province, Advice.Province);
        const float Limit = static_cast<float>(Spot.bHome ? HomeRangeKm : RangeKm);
        if (Km > Limit || Km >= Spot.NowKm) continue;
        const float NewCost = -Rebate * DepotAdviceEfficiency + DistanceCost(Km);
        if (NewCost >= Spot.NowCost) continue;
        ++Advice.Branches;
        Weight += Spot.Weight;
        WeightedKm += Spot.Weight * Km;
        Gain += Spot.Weight * DepotGoodsShare * 30.0 * (Spot.NowCost - NewCost);
    }
    Advice.AverageKm = Weight > 0.0 ? static_cast<float>(WeightedKm / Weight) : 0.f;
    const int64 Wage = MarketPrices::WageScaled(MarketManagers::BaseWageFor(MarketManagers::ELevel::Depot, 60, C), State.Day) * 30;
    Advice.MonthlyGain = FMath::RoundToInt64(Gain) - MonthlyRent(State, C, Advice.Province) - Wage;
    const MarketCountry::FCity* Best = MarketCountry::FindCity(C, Advice.Province);
    const FString Name = Best ? Best->Name : Advice.Province;
    Advice.Text = Advice.MonthlyGain >= 0
        ? FString::Printf(TEXT("\u00d6nerilen il: %s \u00b7 %d \u015fubeye ciro a\u011f\u0131rl\u0131kl\u0131 ortalama %.0f km \u00b7 tahmini ayl\u0131k kazan\u00e7 %s (kira ve depo m\u00fcd\u00fcr\u00fc d\u00fc\u015f\u00fcld\u00fc)."),
            *Name, Advice.Branches, Advice.AverageKm, *DepotMoney(Advice.MonthlyGain))
        : FString::Printf(TEXT("\u00d6nerilen il: %s \u00b7 %d \u015fubeye ciro a\u011f\u0131rl\u0131kl\u0131 ortalama %.0f km \u00b7 \u015fimdilik ayda %s zarar (kira ve depo m\u00fcd\u00fcr\u00fc); \u015fube artt\u0131k\u00e7a kazand\u0131r\u0131r."),
            *Name, Advice.Branches, Advice.AverageKm, *DepotMoney(-Advice.MonthlyGain));
    return Advice;
}

FString MarketDepots::Describe(const FMarketState& State, int32 DepotIndex)
{
    if (!State.Company.DepotSites.IsValidIndex(DepotIndex)) return FString();
    const FMarketDepot& D = State.Company.DepotSites[DepotIndex];
    TArray<FString> Parts;
    Parts.Add(DepotName(State, DepotIndex));
    const int32 M = ManagerOf(State, DepotIndex);
    if (M == INDEX_NONE) Parts.Add(TEXT("m\u00fcd\u00fcr yok (yar\u0131 verim)"));
    else
    {
        const FMarketManager& Man = State.Management.Managers[M];
        Parts.Add(FString::Printf(TEXT("m\u00fcd\u00fcr %s"), *Man.Name));
        if (!D.CaughtName.IsEmpty() && D.CaughtName == Man.Name) Parts.Add(TEXT("MALDAN KA\u00c7IRIYOR"));
    }
    const int32 ServedNow = Served(State, DepotIndex);
    Parts.Add(FString::Printf(TEXT("%d / %d \u015fube"), ServedNow, D.Capacity));
    Parts.Add(FString::Printf(TEXT("verim %%%.0f"), 100.f * EfficiencyWith(State, DepotIndex, ServedNow)));
    Parts.Add(FString::Printf(TEXT("ayl\u0131k kira %s"), *DepotMoney(FMath::RoundToInt64(D.Rent * MarketPrices::ListLevel(State.Day)))));
    if (D.WeekLoss > 0) Parts.Add(FString::Printf(TEXT("bu hafta yolda eksik/k\u0131r\u0131k %s"), *DepotMoney(D.WeekLoss)));
    return FString::Join(Parts, TEXT(" \u00b7 "));
}

FString MarketDepots::DescribeLink(const FMarketState& State, int32 BranchIndex)
{
    if (!State.Branches.IsValidIndex(BranchIndex)) return FString();
    const FMarketBranch& B = State.Branches[BranchIndex];
    const FString C = MarketBranches::CountryOf(State, B);
    const FString Province = DepotBranchProvince(State, B);
    const FLink Link = LinkFor(State, C, Province);
    if (Link.Depot == INDEX_NONE)
        return IsHomePlace(State, C, Province) ? FString(TEXT("Mal\u0131 ev ilinin toptanc\u0131s\u0131ndan al\u0131r."))
            : FString::Printf(TEXT("Yak\u0131nda depo yok (%d km): mal\u0131 toptanc\u0131dan al\u0131r (+%%%.0f)."), RangeKm, WholesalerVan * 100.f);
    return FString::Printf(TEXT("%s, %.0f km \u00b7 yol +%%%.1f \u00b7 depo verimi %%%.0f"), *DepotName(State, Link.Depot), Link.Km, DistanceCost(Link.Km) * 100.f, Link.Efficiency * 100.f);
}

int32 MarketDepots::LostUnits(int32 Units, int32 Permille, uint32 Roll)
{
    if (Units <= 0 || Permille <= 0) return 0;
    const double Exact = static_cast<double>(Units) * Permille / 1000.0;
    int32 Lost = FMath::FloorToInt32(static_cast<float>(Exact));
    if (static_cast<double>(Roll % 1000u) < (Exact - Lost) * 1000.0) ++Lost;
    return FMath::Clamp(Lost, 0, Units);
}

void MarketDepots::RecordLoss(FMarketState& State, int32 DepotIndex, int64 ShortCost, int64 SkimCost)
{
    if (!State.Company.DepotSites.IsValidIndex(DepotIndex)) return;
    FMarketDepot& D = State.Company.DepotSites[DepotIndex];
    D.WeekLoss += ShortCost;
    D.WeekSkim += SkimCost;
}

void MarketDepots::Migrate(FMarketState& State)
{
    FMarketCompany& Co = State.Company;
    if (Co.Depots.Num() == 0) return;
    TArray<FString> Moved;
    for (const FString& Key : Co.Depots)
    {
        FString Country, Sub;
        if (!Key.Split(TEXT(":"), &Country, &Sub)) { Country = State.CountryId; Sub = Key; }
        const MarketCountry::FProfile* Pack = MarketCountry::Find(Country);
        const MarketCountry::FRegion* Row = Pack ? Pack->SubRegions.FindByPredicate([&Sub](const MarketCountry::FRegion& R) { return R.Id == Sub; }) : nullptr;
        if (!Row) continue;
        const FString Province = MigrationProvince(State, Country, *Row);
        if (Province.IsEmpty() || HasDepotIn(State, Country, Province)) continue;
        Co.DepotSites.Add(NewDepot(State, Country, Province));
        Moved.Add(FString::Printf(TEXT("%s \u2192 %s"), *Row->Name, *DepotName(State, Co.DepotSites.Num() - 1)));
    }
    Co.Depots.Reset();
    // A depot manager appointed over an older sub-region depot (G-086b) follows it to its province.
    for (FMarketManager& M : State.Management.Managers)
    {
        if (M.Level != static_cast<uint8>(MarketManagers::ELevel::Depot) || HasDepotIn(State, M.Country, M.Area)) continue;
        const MarketCountry::FCity* Was = MarketCountry::FindCity(M.Country, M.Area);
        for (int32 D = 0; D < Co.DepotSites.Num(); ++D)
        {
            const MarketCountry::FCity* Site = MarketCountry::FindCity(Co.DepotSites[D].Country, Co.DepotSites[D].Province);
            if (Was && Site && Co.DepotSites[D].Country == M.Country && Site->SubRegion == Was->SubRegion && ManagerOf(State, D) == INDEX_NONE)
            {
                M.Area = Co.DepotSites[D].Province;
                break;
            }
        }
    }
    if (Moved.Num() > 0)
        State.DayNews.Add(FString::Printf(TEXT("B\u00f6lge depolar\u0131 illere ta\u015f\u0131nd\u0131 (%s). Depolar art\u0131k 600 km \u00e7evresindeki \u015fubelere hizmet veriyor. Depona m\u00fcd\u00fcr ata: m\u00fcd\u00fcrs\u00fcz depo yar\u0131 verimle \u00e7al\u0131\u015f\u0131r."),
            *FString::Join(Moved, TEXT(", "))));
}

int64 MarketDepots::DailyRent(const FMarketState& State, int32 Day)
{
    const double Level = MarketPrices::ListLevel(FMath::Max(1, Day));
    int64 Sum = 0;
    for (const FMarketDepot& D : State.Company.DepotSites) Sum += FMath::RoundToInt64(static_cast<double>(D.Rent) * Level / 30.0);
    return Sum;
}

void MarketDepots::CloseDay(FMarketState& State)
{
    Migrate(State);
    const int32 Closed = State.Day - 1;
    if (Closed < 1 || Closed % 7 != 0) return;
    TArray<FString>& News = State.DayNews;
    for (int32 Index = 0; Index < State.Company.DepotSites.Num(); ++Index)
    {
        const int32 M = ManagerOf(State, Index);
        const FString Name = DepotName(State, Index);
        FMarketDepot& D = State.Company.DepotSites[Index];
        if (M == INDEX_NONE)
        {
            D.CaughtName.Reset();
            News.Add(FString::Printf(TEXT("%s: m\u00fcd\u00fcr yok, depo yar\u0131 verimle \u00e7al\u0131\u015f\u0131yor (fire ve eksik teslimat art\u0131yor). Depo m\u00fcd\u00fcr\u00fc ata."), *Name));
        }
        else
        {
            const FMarketManager& Man = State.Management.Managers[M];
            if (!D.CaughtName.IsEmpty() && D.CaughtName != Man.Name) D.CaughtName.Reset();
            // A dishonest depot manager: only the country manager or the player can see it (not a province or
            // sub-region manager); a player with too many people sees nothing.
            if (D.WeekSkim > 0 && D.CaughtName.IsEmpty())
            {
                const int32 Head = MarketManagers::FindManager(State, MarketManagers::ELevel::Country, D.Country, D.Country);
                const float Chance = Head != INDEX_NONE ? 70.f * MarketManagers::Strength(State, Head)
                    : MarketManagers::OverLimit(State) > 0 ? 0.f : (MarketStaff::HasAccountant(State) ? 35.f : 15.f);
                if (static_cast<float>(DepotMix(State.RivalSeed, Closed, 0xDE90u + static_cast<uint32>(Index) * 31u) % 100u) < Chance)
                {
                    D.CaughtName = Man.Name;
                    const FString Who = Head != INDEX_NONE ? FString::Printf(TEXT("%s (%s)"), *State.Management.Managers[Head].Name, *MarketManagers::LevelName(MarketManagers::ELevel::Country)) : FString(TEXT("Sen"));
                    News.Add(FString::Printf(TEXT("%s fark etti: %s m\u00fcd\u00fcr\u00fc %s maldan ka\u00e7\u0131r\u0131yor (bu hafta %s). \u00d6neri: depo m\u00fcd\u00fcr\u00fcn\u00fc de\u011fi\u015ftir."),
                        *Who, *Name, *Man.Name, *DepotMoney(D.WeekSkim)));
                }
            }
        }
        if (D.WeekLoss > 0)
            News.Add(FString::Printf(TEXT("%s haftas\u0131: %d \u015fube, verim %%%.0f, yolda eksik ya da k\u0131r\u0131k gelen mal %s."), *Name, Served(State, Index), 100.f * Efficiency(State, Index), *DepotMoney(D.WeekLoss)));
        D.WeekLoss = 0;
        D.WeekSkim = 0;
    }
}

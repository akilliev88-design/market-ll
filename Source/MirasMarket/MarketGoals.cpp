#include "MarketGoals.h"
#include "MarketEconomy.h"
#include "MarketBranches.h"
#include "MarketCompany.h"
#include "MarketCountry.h"
#include "MarketDepots.h"
#include "MarketEvents.h"
#include "MarketFinance.h"
#include "MarketBanking.h"
#include "MarketLedger.h"
#include "MarketStaff.h"
#include "MarketStory.h"
#include "MarketSuppliers.h"

namespace MarketGoals
{
    constexpr int32 KeepDays = 30;
    constexpr int32 KeepCelebrations = 40;
    constexpr int32 RecentMemory = 4;        // goal kinds not given again right away
    constexpr int32 RecordGapDays = 7;
    constexpr int32 RecordAfterDays = 14;    // no records in the first two weeks (every day would be one)

    uint32 GoalMix(int32 Seed, int32 Day, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), static_cast<uint32>(Day), Salt };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }

    FString GoalMoney(int64 Kurus) { return MarketCountry::Money(Kurus); }

    // A round target: up to 10 money units under 1.000, 50 up to 10.000, 100 above.
    int64 RoundUp(int64 Kurus)
    {
        const int64 Step = Kurus < 100000 ? 1000 : Kurus < 1000000 ? 5000 : 10000;
        return FMath::Max<int64>(Step, (Kurus + Step - 1) / Step * Step);
    }

    EScale ScaleOf(EGoal Kind)
    {
        return static_cast<uint8>(Kind) <= static_cast<uint8>(EGoal::Service) ? EScale::Short
            : static_cast<uint8>(Kind) <= static_cast<uint8>(EGoal::FirstDepot) ? EScale::Medium : EScale::Long;
    }

    int32 DurationOf(EScale Scale) { return Scale == EScale::Short ? 7 : Scale == EScale::Medium ? 30 : 365; }

    int64 CompanyRevenue(const FMarketState& State)
    {
        int64 Revenue = FMath::Max<int64>(0, State.LastRevenue);
        for (const FMarketBranch& B : State.Branches)
            if (B.Stage == static_cast<uint8>(MarketBranches::EStage::Open)) Revenue += FMath::Max<int64>(0, B.LastRevenue);
        return Revenue;
    }

    // The visited store's shelves at the close, per mille of the planned capacity (-1: no shelf plan).
    int32 ShelfFill(const FMarketState& State)
    {
        int64 Held = 0, Room = 0;
        for (const FMarketStock& Row : State.Stock)
            if (Row.Capacity > 0) { Room += Row.Capacity; Held += FMath::Clamp(Row.Shelf, 0, Row.Capacity); }
        return Room > 0 ? static_cast<int32>(Held * 1000 / Room) : -1;
    }

    template <typename T> T AverageOfLast(const TArray<T>& Values, int32 Days)
    {
        const int32 N = FMath::Min(Days, Values.Num());
        if (N <= 0) return 0;
        T Sum = 0;
        for (int32 I = Values.Num() - N; I < Values.Num(); ++I) Sum += Values[I];
        return Sum / N;
    }

    int32 ServedAverage(const FMarketState& State)
    {
        int32 Sum = 0, N = 0;
        for (int32 I = State.History.Num() - 1; I >= 0 && N < 7; --I, ++N) Sum += State.History[I].Served;
        return N > 0 ? Sum / N : 0;
    }

    int32 NationalMilli(const FMarketState& State) { return FMath::FloorToInt32(MarketCompany::NationalShare(State) * 1000.f); }

    // The measure a goal follows, read at a close.
    int64 Measure(const FMarketState& State, const FMarketGoal& G, int64 DayRevenue, int64 DayProfit)
    {
        switch (static_cast<EGoal>(G.Kind))
        {
        case EGoal::DayRevenue: return FMath::Max(G.Value, DayRevenue);
        case EGoal::WeekProfit: case EGoal::MonthProfit: case EGoal::YearProfit: return G.Value + DayProfit;
        case EGoal::ShelvesFull: return ShelfFill(State) >= G.Base ? G.Value + 1 : G.Value; // Base = the fill asked (per mille)
        case EGoal::Service: return G.Value + State.LastServed;
        case EGoal::MoreStores: return MarketCompany::TotalStores(State);
        case EGoal::LocalShare: return FMath::FloorToInt32(State.MarketShare * 10.f);
        case EGoal::FirstDepot: return MarketDepots::Count(State);
        case EGoal::Provinces: return MarketCompany::Provinces(State);
        case EGoal::NationalShare: return NationalMilli(State);
        case EGoal::Abroad: return MarketCompany::ForeignCountries(State);
        default: return G.Value;
        }
    }

    // Kinds counted from zero (sums, streaks, amounts paid); the others start from where the player stood.
    bool FromZero(EGoal Kind)
    {
        switch (Kind)
        {
        case EGoal::DayRevenue: case EGoal::WeekProfit: case EGoal::MonthProfit: case EGoal::YearProfit: case EGoal::ShelvesFull:
        case EGoal::Service: return true;
        default: return false;
        }
    }

    float ProgressOf(const FMarketGoal& G)
    {
        const EGoal Kind = static_cast<EGoal>(G.Kind);
        const int64 From = FromZero(Kind) || Kind == EGoal::ShelvesFull ? 0 : G.Base;
        const int64 Span = G.Target - From;
        if (Span <= 0) return 1.f;
        return FMath::Clamp(static_cast<float>(static_cast<double>(G.Value - From) / static_cast<double>(Span)), 0.f, 1.f);
    }

    // Builds a goal of a kind for the player now; false when its door is shut or it would be as good as done.
    bool Make(const FMarketState& State, EGoal Kind, int32 Day, FMarketGoal& Out)
    {
        const FMarketGoals& S = State.Goals;
        Out = FMarketGoal();
        Out.Kind = static_cast<uint8>(Kind);
        Out.Scale = static_cast<uint8>(ScaleOf(Kind));
        Out.StartDay = Day;
        Out.DueDay = Day + DurationOf(ScaleOf(Kind)) - 1;
        const int64 Revenue7 = AverageOfLast(S.RecentRevenue, 7);
        const int64 Profit14 = AverageOfLast(S.RecentProfit, 14);
        const int64 Profit30 = AverageOfLast(S.RecentProfit, 30);
        const int32 Stores = MarketCompany::TotalStores(State);
        const int32 Stage = MarketGoals::Stage(State);
        switch (Kind)
        {
        case EGoal::DayRevenue:
        {
            if (S.DaysCounted < 7 || Revenue7 <= 0) return false;
            int64 Target = S.BestDayRevenue > 0 ? FMath::Min<int64>(S.BestDayRevenue * 102 / 100, Revenue7 * 125 / 100) : Revenue7 * 120 / 100;
            Target = FMath::Max<int64>(Target, Revenue7 * 108 / 100);
            Out.Target = RoundUp(Target);
            return true;
        }
        case EGoal::WeekProfit:
            if (S.DaysCounted < 14 || Profit14 <= 0) return false;
            Out.Target = RoundUp(Profit14 * 7 * 104 / 100);
            return true;
        case EGoal::ShelvesFull:
        {
            const int32 Fill = static_cast<int32>(AverageOfLast(S.RecentFill, 7));
            if (ShelfFill(State) < 0) return false;
            // A little above the shop's usual evening shelves, in steps of 5 %.
            int32 Ask = FMath::Clamp((Fill + 40) / 50 * 50, 700, 900);
            // Not as good as done: when the last four closes already reached it, this goal waits (another comes).
            bool bAlready = S.RecentFill.Num() >= 4;
            for (int32 I = FMath::Max(0, S.RecentFill.Num() - 4); I < S.RecentFill.Num(); ++I) bAlready &= S.RecentFill[I] >= Ask;
            if (bAlready) return false;
            Out.Base = Ask;
            Out.Target = 4;
            return true;
        }
        case EGoal::Service:
            Out.Target = FMath::Max(7 * 20, ServedAverage(State) * 7);
            return true;
        case EGoal::MoreStores:
        {
            // M69: nothing locks the first branch; the goal comes once the first month is behind.
            const bool bDoorOpen = Stage >= 1 || S.DaysCounted >= 30;
            if (!bDoorOpen) return false;
            Out.Base = Stores;
            Out.Target = Stores + (Stores < 5 ? 1 : Stores < 20 ? 2 : FMath::Max(3, Stores / 10));
            return true;
        }
        case EGoal::MonthProfit:
            if (S.DaysCounted < 14 || Profit14 <= 0) return false;
            Out.Target = RoundUp(Profit14 * 30 * 104 / 100);
            return true;
        case EGoal::LocalShare:
        {
            if (State.MarketShare >= 60.f) return false;
            Out.Base = FMath::FloorToInt32(State.MarketShare * 10.f);
            Out.Target = FMath::Min(650, (FMath::FloorToInt32(State.MarketShare) + 3) * 10);
            return true;
        }
        case EGoal::FirstDepot:
            if (MarketDepots::Count(State) > 0 || Stores < MarketDepots::MinStores) return false;
            Out.Base = 0;
            Out.Target = 1;
            return true;
        case EGoal::Provinces:
        {
            if (Stage < 1) return false;
            const int32 Now = MarketCompany::Provinces(State);
            const int32 Steps[] = { 2, 5, 10, 20, 40, 81 };
            for (int32 Next : Steps) if (Next > Now) { Out.Base = Now; Out.Target = Next; return true; }
            return false;
        }
        case EGoal::NationalShare:
        {
            if (Stage < 2) return false;
            const int32 Now = NationalMilli(State);
            const int32 Steps[] = { 10, 50, 100, 250, 500, 1000, 2500, 5000, 10000 };
            for (int32 Next : Steps) if (Next > Now) { Out.Base = Now; Out.Target = Next; return true; }
            return false;
        }
        case EGoal::Abroad:
            if (Stage < 3 || MarketCompany::ForeignCountries(State) > 0) return false;
            if (!MarketCompany::AbroadOpen(State)) return false;
            Out.Base = 0;
            Out.Target = 1;
            return true;
        case EGoal::YearProfit:
            if (S.DaysCounted < 30 || Profit30 <= 0) return false;
            Out.Target = RoundUp(Profit30 * 365 * 105 / 100);
            return true;
        default: return false;
        }
    }

    // bMilestone: a first or a record (the rhythm guard counts those as something happening; a finished goal not).
    void Celebrate(FMarketState& State, int32 Day, const FString& Title, const FString& Text, uint8 Importance, bool bMilestone = true)
    {
        FMarketGoals& S = State.Goals;
        FMarketCelebration C;
        C.Day = Day; C.Title = Title; C.Text = Text; C.Importance = FMath::Min<uint8>(Importance, 2);
        S.Celebrations.Add(C);
        if (S.Celebrations.Num() > KeepCelebrations) S.Celebrations.RemoveAt(0, S.Celebrations.Num() - KeepCelebrations);
        if (bMilestone) S.LastLivelyDay = FMath::Max(S.LastLivelyDay, Day);
        State.DayNews.Add(FString::Printf(TEXT("Kutlama: %s %s"), *Title, *Text));
    }

    void CheerTeam(FMarketState& State, float Morale)
    {
        for (FMarketEmployee& E : State.Staff)
            if (MarketStaff::RoleOf(E) != MarketStaff::ERole::Accountant) E.Morale = FMath::Min(100.f, E.Morale + Morale);
    }

    FString RewardText(EScale Scale)
    {
        switch (Scale)
        {
        case EScale::Short: return TEXT("Ekibin morali y\u00fckselir (+3).");
        case EScale::Medium: return TEXT("Ekip ve toptanc\u0131 memnun (+5 moral, +3 g\u00fcven).");
        default: return TEXT("Bir hat\u0131ra ve ekibe +8 moral.");
        }
    }

    FString TitleOf(const FMarketState& State, const FMarketGoal& G)
    {
        switch (static_cast<EGoal>(G.Kind))
        {
        case EGoal::DayRevenue: return FString::Printf(TEXT("Bir g\u00fcnde %s ciro yap"), *GoalMoney(G.Target));
        case EGoal::WeekProfit: return FString::Printf(TEXT("Bu hafta %s net k\u00e2r"), *GoalMoney(G.Target));
        case EGoal::ShelvesFull: return FString::Printf(TEXT("Bu hafta 4 ak\u015fam raflar\u0131 %%%d dolu kapat"), static_cast<int32>(G.Base / 10));
        case EGoal::Service: return FString::Printf(TEXT("Bu hafta %lld m\u00fc\u015fteriye hizmet et"), static_cast<long long>(G.Target));
        case EGoal::MoreStores: return FString::Printf(TEXT("%lld ma\u011fazaya ula\u015f"), static_cast<long long>(G.Target));
        case EGoal::MonthProfit: return FString::Printf(TEXT("Bu ay %s net k\u00e2r"), *GoalMoney(G.Target));
        case EGoal::LocalShare: return FString::Printf(TEXT("Mahallede pay %%%lld olsun"), static_cast<long long>(G.Target / 10));
        case EGoal::FirstDepot: return TEXT("\u0130lk depoyu kur");
        case EGoal::Provinces: return FString::Printf(TEXT("%lld ilde ma\u011faza"), static_cast<long long>(G.Target));
        case EGoal::NationalShare: return FString::Printf(TEXT("Ulusal pay %%%s olsun"), *FString::SanitizeFloat(G.Target / 1000.0, 0));
        case EGoal::Abroad: return TEXT("Yurt d\u0131\u015f\u0131nda ilk ma\u011faza");
        case EGoal::YearProfit: return FString::Printf(TEXT("Bu y\u0131l %s net k\u00e2r"), *GoalMoney(G.Target));
        default: return FString();
        }
    }

    FString WhyOf(const FMarketState& State, const FMarketGoal& G)
    {
        switch (static_cast<EGoal>(G.Kind))
        {
        case EGoal::DayRevenue: return TEXT("Rekor g\u00fcnler m\u00fc\u015fterinin seni se\u00e7ti\u011fini g\u00f6sterir.");
        case EGoal::WeekProfit: return TEXT("Sonraki ad\u0131m\u0131n paras\u0131 haftal\u0131k k\u00e2rdan \u00e7\u0131kar.");
        case EGoal::ShelvesFull: return TEXT("Bo\u015f raf m\u00fc\u015fteri kaybettirir, dolu raf sadakat getirir.");
        case EGoal::Service: return TEXT("Her memnun m\u00fc\u015fteri bir sonraki ziyaretin tohumu.");
        case EGoal::MoreStores: return TEXT("Ma\u011faza say\u0131s\u0131 toptanc\u0131yla pazarl\u0131k g\u00fcc\u00fcn\u00fc art\u0131r\u0131r.");
        case EGoal::MonthProfit: return TEXT("\u0130stikrarl\u0131 ay k\u00e2r\u0131 bankan\u0131n ve toptanc\u0131n\u0131n g\u00fcvenini getirir.");
        case EGoal::LocalShare: return TEXT("Mahallede b\u00fcy\u00fck pay, rakibin fiyat sava\u015f\u0131n\u0131 bo\u015fa \u00e7\u0131kar\u0131r.");
        case EGoal::FirstDepot: return TEXT("Depo uzak ma\u011fazalar\u0131n mal\u0131n\u0131 ucuzlat\u0131r.");
        case EGoal::Provinces: return TEXT("Yeni iller yeni m\u00fc\u015fteri ve daha g\u00fc\u00e7l\u00fc bir marka demek.");
        case EGoal::NationalShare: return TEXT("Ulusal pay b\u00fcy\u00fcd\u00fck\u00e7e markalar ve toptanc\u0131lar kap\u0131n\u0131 \u00e7alar.");
        case EGoal::Abroad: return TEXT("D\u00fcnya ligine giden yol s\u0131n\u0131r\u0131n \u00f6tesinden ge\u00e7er.");
        case EGoal::YearProfit: return TEXT("G\u00fc\u00e7l\u00fc bir y\u0131l b\u00fcy\u00fck yat\u0131r\u0131mlar\u0131n temelidir.");
        default: return FString();
        }
    }

    FGoalView ViewOf(const FMarketState& State, const FMarketGoal& G)
    {
        FGoalView V;
        V.Kind = static_cast<EGoal>(G.Kind);
        V.Scale = static_cast<EScale>(G.Scale);
        V.Title = TitleOf(State, G);
        V.Why = WhyOf(State, G);
        V.Reward = RewardText(V.Scale);
        V.Progress = ProgressOf(G);
        V.DaysLeft = FMath::Max(0, G.DueDay - (State.Day - 1));
        return V;
    }

    void Remember(FMarketGoals& S, uint8 Kind)
    {
        S.RecentKinds.Add(Kind);
        if (S.RecentKinds.Num() > RecentMemory * 3) S.RecentKinds.RemoveAt(0, S.RecentKinds.Num() - RecentMemory * 3);
    }

    bool Recent(const FMarketGoals& S, EGoal Kind)
    {
        // The last few finished or dropped goals of the same scale.
        int32 SameScale = 0;
        for (int32 I = S.RecentKinds.Num() - 1; I >= 0 && SameScale < 2; --I)
        {
            if (ScaleOf(static_cast<EGoal>(S.RecentKinds[I])) != ScaleOf(Kind)) continue;
            if (S.RecentKinds[I] == static_cast<uint8>(Kind)) return true;
            ++SameScale;
        }
        return false;
    }

    void FillSlots(FMarketState& State, int32 Day)
    {
        FMarketGoals& S = State.Goals;
        for (int32 Scale = 0; Scale < static_cast<int32>(EScale::Count); ++Scale)
        {
            if (S.Goals.ContainsByPredicate([Scale](const FMarketGoal& G) { return G.Scale == Scale; })) continue;
            TArray<EGoal> Kinds;
            for (int32 K = 0; K < static_cast<int32>(EGoal::Count); ++K)
                if (static_cast<int32>(ScaleOf(static_cast<EGoal>(K))) == Scale) Kinds.Add(static_cast<EGoal>(K));
            // Seeded order: every campaign and day tries them differently.
            const int32 Offset = Kinds.Num() > 0 ? static_cast<int32>(GoalMix(State.RivalSeed, Day, 0x60A1u + Scale) % static_cast<uint32>(Kinds.Num())) : 0;
            bool bFound = false;
            for (int32 Pass = 0; Pass < 2 && !bFound; ++Pass)
                for (int32 I = 0; I < Kinds.Num() && !bFound; ++I)
                {
                    const EGoal Kind = Kinds[(I + Offset) % Kinds.Num()];
                    if (Pass == 0 && Recent(S, Kind)) continue;
                    FMarketGoal G;
                    if (!Make(State, Kind, Day, G)) continue;
                    if (ProgressOf(G) >= 0.95f) continue; // as good as done
                    S.Goals.Add(G);
                    bFound = true;
                }
            // A short goal is always there.
            if (!bFound && Scale == static_cast<int32>(EScale::Short))
            {
                FMarketGoal G;
                Make(State, EGoal::Service, Day, G);
                S.Goals.Add(G);
            }
        }
    }

    struct FFirstInfo { EFirst First; const TCHAR* Title; const TCHAR* Text; uint8 Importance; };
    const FFirstInfo FirstInfos[] =
    {
        { EFirst::ProfitDay, TEXT("\u0130lk k\u00e2rl\u0131 g\u00fcn!"), TEXT("G\u00fcn sonunda kasa art\u0131da; d\u00fckk\u00e2n nefes almaya ba\u015flad\u0131."), 1 },
        { EFirst::FirstBranch, TEXT("\u0130lk \u015fube!"), TEXT("\u0130lk ma\u011fazan\u0131n ilk karde\u015fi kap\u0131lar\u0131n\u0131 a\u00e7t\u0131."), 2 },
        { EFirst::Stores5, TEXT("5 ma\u011faza!"), TEXT("Be\u015f tabelan var; art\u0131k k\u00fc\u00e7\u00fck bir zincirsin."), 1 },
        { EFirst::Stores10, TEXT("10 ma\u011faza!"), TEXT("On ma\u011fazayla toptanc\u0131lar seni ciddiye al\u0131yor."), 1 },
        { EFirst::Stores25, TEXT("25 ma\u011faza!"), TEXT("Yirmi be\u015f ma\u011fazayla b\u00f6lgenin tan\u0131nan zincirlerindensin."), 1 },
        { EFirst::Stores50, TEXT("50 ma\u011faza!"), TEXT("Elli ma\u011faza: \u00fclke \u00e7ap\u0131nda konu\u015fulan bir isimsin."), 2 },
        { EFirst::Stores100, TEXT("100 ma\u011faza!"), TEXT("Y\u00fcz\u00fcnc\u00fc tabela as\u0131ld\u0131; devrald\u0131\u011f\u0131n k\u00fc\u00e7\u00fck market bir zincirin ilk halkas\u0131 oldu."), 2 },
        { EFirst::Stores250, TEXT("250 ma\u011faza!"), TEXT("\u0130ki y\u00fcz elli ma\u011fazayla b\u00fcy\u00fck zincirlerin aras\u0131ndas\u0131n."), 2 },
        { EFirst::Stores500, TEXT("500 ma\u011faza!"), TEXT("Be\u015f y\u00fcz ma\u011faza: \u00fclkenin her yerinde senin tabelan var."), 2 },
        { EFirst::Stores1000, TEXT("1000 ma\u011faza!"), TEXT("Bininci ma\u011faza a\u00e7\u0131ld\u0131; d\u00fcnya ligi seni g\u00f6r\u00fcyor."), 2 },
        { EFirst::Provinces2, TEXT("\u0130kinci il!"), TEXT("Art\u0131k ba\u015fka bir ilde de tabelan var."), 1 },
        { EFirst::Provinces5, TEXT("5 ilde vars\u0131n!"), TEXT("Be\u015f ilde ma\u011faza: b\u00f6lgesel bir zincirsin."), 1 },
        { EFirst::Provinces10, TEXT("10 ilde vars\u0131n!"), TEXT("On ilde ma\u011fazayla \u00fclke haritas\u0131nda yerin belli."), 1 },
        { EFirst::Provinces20, TEXT("20 ilde vars\u0131n!"), TEXT("Yirmi ilde ma\u011faza: ulusal bir zincirsin."), 2 },
        { EFirst::FirstDepot, TEXT("\u0130lk depo!"), TEXT("Mallar art\u0131k kendi deponuzdan \u00e7\u0131k\u0131yor."), 1 },
        { EFirst::Abroad, TEXT("S\u0131n\u0131r\u0131n \u00f6tesinde!"), TEXT("\u0130lk yurt d\u0131\u015f\u0131 ma\u011fazan a\u00e7\u0131ld\u0131."), 2 },
        { EFirst::OnlineOrder, TEXT("\u0130lk sipari\u015f!"), TEXT("Bir m\u00fc\u015fteri al\u0131\u015fveri\u015fini evden verdi; kap\u0131ya kadar g\u00f6t\u00fcrd\u00fcn\u00fcz."), 0 },
        { EFirst::Share01, TEXT("Ulusal pay %0,1!"), TEXT("\u00dclkenin market al\u0131\u015fveri\u015finin binde biri art\u0131k senden."), 1 },
        { EFirst::Share1, TEXT("Ulusal pay %1!"), TEXT("\u00dclkedeki her y\u00fcz liral\u0131k market al\u0131\u015fveri\u015finin biri sende."), 2 },
        { EFirst::League10, TEXT("D\u00fcnya ilk 10'u!"), TEXT("D\u00fcnya perakende liginde ilk ona girdin."), 2 },
        { EFirst::League3, TEXT("D\u00fcnya ilk 3'\u00fc!"), TEXT("D\u00fcnyan\u0131n en b\u00fcy\u00fck \u00fc\u00e7 perakendecisinden birisin."), 2 },
        { EFirst::League1, TEXT("D\u00fcnya birincisi!"), TEXT("D\u00fcnya perakende liginin zirvesindesin."), 2 },
    };

    bool Reached(const FMarketState& State, EFirst First)
    {
        const int32 Stores = MarketCompany::TotalStores(State);
        const int32 Provinces = MarketCompany::Provinces(State);
        const int32 Rank = State.Goals.LastLeagueRank;
        switch (First)
        {
        case EFirst::ProfitDay: return State.ProfitableDays > 0;
        case EFirst::FirstBranch: return Stores >= 2;
        case EFirst::Stores5: return Stores >= 5;
        case EFirst::Stores10: return Stores >= 10;
        case EFirst::Stores25: return Stores >= 25;
        case EFirst::Stores50: return Stores >= 50;
        case EFirst::Stores100: return Stores >= 100;
        case EFirst::Stores250: return Stores >= 250;
        case EFirst::Stores500: return Stores >= 500;
        case EFirst::Stores1000: return Stores >= 1000;
        case EFirst::Provinces2: return Provinces >= 2;
        case EFirst::Provinces5: return Provinces >= 5;
        case EFirst::Provinces10: return Provinces >= 10;
        case EFirst::Provinces20: return Provinces >= 20;
        case EFirst::FirstDepot: return MarketDepots::Count(State) > 0;
        case EFirst::Abroad: return MarketCompany::ForeignCountries(State) > 0;
        case EFirst::OnlineOrder: return State.Online.TotalOrders > 0;
        case EFirst::Share01: return MarketCompany::NationalShare(State) >= 0.1f;
        case EFirst::Share1: return MarketCompany::NationalShare(State) >= 1.f;
        case EFirst::League10: return Rank > 0 && Rank <= 10;
        case EFirst::League3: return Rank > 0 && Rank <= 3;
        case EFirst::League1: return Rank == 1;
        default: return false;
        }
    }

    int64 Bit(EFirst First) { return static_cast<int64>(1) << static_cast<int32>(First); }

    // Firsts not yet written: celebrate.
    void CheckFirsts(FMarketState& State, int32 Day)
    {
        FMarketGoals& S = State.Goals;
        for (const FFirstInfo& Info : FirstInfos)
        {
            if ((S.Firsts & Bit(Info.First)) || !Reached(State, Info.First)) continue;
            S.Firsts |= Bit(Info.First);
            Celebrate(State, Day, Info.Title, Info.Text, Info.Importance);
            CheerTeam(State, Info.Importance >= 2 ? 5.f : 2.f);
        }
    }
}

int32 MarketGoals::Stage(const FMarketState& State)
{
    if (MarketCompany::ForeignCountries(State) > 0) return 4;
    const int32 Provinces = MarketCompany::Provinces(State);
    if (Provinces >= 5) return 3;
    if (Provinces >= 2) return 2;
    return MarketCompany::TotalStores(State) >= 2 ? 1 : 0;
}

TArray<MarketGoals::FGoalView> MarketGoals::Goals(const FMarketState& State)
{
    TArray<FGoalView> Views;
    for (const FMarketGoal& G : State.Goals.Goals) Views.Add(ViewOf(State, G));
    Views.StableSort([](const FGoalView& A, const FGoalView& B) { return static_cast<uint8>(A.Scale) < static_cast<uint8>(B.Scale); });
    return Views;
}

bool MarketGoals::NextGoal(const FMarketState& State, FGoalView& Out)
{
    bool bAny = false;
    for (const FGoalView& V : Goals(State))
    {
        // Closest to done first; among equals the shorter one.
        if (!bAny || V.Progress > Out.Progress + 0.001f) { Out = V; bAny = true; }
    }
    return bAny;
}

FString MarketGoals::StripText(const FMarketState& State)
{
    FGoalView V;
    if (!NextGoal(State, V)) return FString();
    const TCHAR* When = V.Scale == EScale::Short ? TEXT("Bu hafta") : V.Scale == EScale::Medium ? TEXT("Bu ay") : TEXT("Bu y\u0131l");
    return FString::Printf(TEXT("%s: %s (%%%d, %d g\u00fcn)"), When, *V.Title, FMath::RoundToInt32(V.Progress * 100.f), V.DaysLeft);
}

TArray<FMarketCelebration> MarketGoals::CelebrationsOn(const FMarketState& State, int32 Day)
{
    return State.Goals.Celebrations.FilterByPredicate([Day](const FMarketCelebration& C) { return C.Day == Day; });
}

TArray<FMarketCelebration> MarketGoals::RecentCelebrations(const FMarketState& State, int32 MaxCount)
{
    TArray<FMarketCelebration> Out;
    for (int32 I = State.Goals.Celebrations.Num() - 1; I >= 0 && Out.Num() < MaxCount; --I) Out.Add(State.Goals.Celebrations[I]);
    return Out;
}

TArray<MarketGoals::FRecordView> MarketGoals::Records(const FMarketState& State)
{
    const FMarketGoals& S = State.Goals;
    TArray<FRecordView> Rows;
    auto Add = [&Rows](const TCHAR* Name, const FString& Value) { FRecordView R; R.Name = Name; R.Value = Value; Rows.Add(R); };
    Add(TEXT("En iyi g\u00fcn (ciro)"), GoalMoney(S.BestDayRevenue));
    Add(TEXT("En iyi hafta (ciro)"), GoalMoney(S.BestWeekRevenue));
    Add(TEXT("En k\u00e2rl\u0131 30 g\u00fcn"), GoalMoney(S.BestMonthProfit));
    Add(TEXT("En \u00e7ok ma\u011faza"), FString::Printf(TEXT("%d"), S.MostStores));
    if (S.LastLeagueRank > 0) Add(TEXT("D\u00fcnya ligi (son y\u0131l)"), FString::Printf(TEXT("%d. s\u0131ra"), S.LastLeagueRank));
    return Rows;
}

bool MarketGoals::IsBadEvent(const FString& EventId)
{
    return EventId == TEXT("event.fridge") || EventId == TEXT("event.power") || EventId == TEXT("event.inspection") || EventId == TEXT("event.complaint")
        || EventId == TEXT("event.roadworks") || EventId == TEXT("event.truck");
}

int32 MarketGoals::QuietDays(const FMarketState& State)
{
    return State.Difficulty == 0 ? 15 : State.Difficulty >= 2 ? 25 : 20;
}

int32 MarketGoals::BadLimit(const FMarketState& State)
{
    return State.Difficulty == 0 ? 2 : State.Difficulty >= 2 ? 4 : 3;
}

bool MarketGoals::HoldBadEvent(const FMarketState& State)
{
    const int32 Closed = State.Day - 1;
    int32 Recent = 0;
    for (int32 Day : State.Goals.BadEventDays) if (Day > Closed - 7) ++Recent;
    return Recent >= BadLimit(State);
}

void MarketGoals::OnLeagueYear(FMarketState& State, int32 Rank, bool bFullYear)
{
    // M69: the league year only brings its firsts (top 10, top 3, first in the world); the game never ends here.
    (void)bFullYear;
    FMarketGoals& S = State.Goals;
    S.LastLeagueRank = FMath::Max(0, Rank);
    if (S.bStarted) CheckFirsts(State, FMath::Max(1, State.Day - 1));
}

void MarketGoals::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    FMarketGoals& S = State.Goals;
    const int32 Closed = State.Day - 1;
    if (Closed < 1 || Closed == S.LastClosedDay) return; // the same close twice changes nothing
    S.LastClosedDay = Closed;
    const int64 DayRevenue = CompanyRevenue(State);
    const int64 DayProfit = State.LastProfit;
    const int32 Fill = ShelfFill(State);

    if (!S.bStarted)
    {
        // The campaign's first close: the rhythm starts counting, the first day is the first record.
        S.bStarted = true;
        S.LastLivelyDay = Closed;
        S.BestDayRevenue = DayRevenue;
        S.MostStores = MarketCompany::TotalStores(State);
    }

    // The day into the rolling window.
    S.RecentRevenue.Add(DayRevenue);
    S.RecentProfit.Add(DayProfit);
    S.RecentFill.Add(FMath::Max(0, Fill));
    for (TArray<int64>* A : { &S.RecentRevenue, &S.RecentProfit }) if (A->Num() > KeepDays) A->RemoveAt(0, A->Num() - KeepDays);
    if (S.RecentFill.Num() > KeepDays) S.RecentFill.RemoveAt(0, S.RecentFill.Num() - KeepDays);
    ++S.DaysCounted;

    // Firsts.
    CheckFirsts(State, Closed);

    // Records (from the third week on, at most one told a week; they are always kept).
    const bool bTell = S.DaysCounted > RecordAfterDays && Closed - S.RecordDay >= RecordGapDays;
    if (DayRevenue > S.BestDayRevenue)
    {
        if (bTell && S.BestDayRevenue > 0 && DayRevenue * 100 >= S.BestDayRevenue * 102)
        {
            Celebrate(State, Closed, TEXT("Ciro rekoru!"), FString::Printf(TEXT("Bug\u00fcn %s ciro yapt\u0131n; en iyi g\u00fcn\u00fc ge\u00e7tin."), *GoalMoney(DayRevenue)), 0);
            S.RecordDay = Closed;
        }
        S.BestDayRevenue = DayRevenue;
    }
    if (S.DaysCounted % 7 == 0 && S.RecentRevenue.Num() >= 7)
    {
        int64 Week = 0;
        for (int32 I = S.RecentRevenue.Num() - 7; I < S.RecentRevenue.Num(); ++I) Week += S.RecentRevenue[I];
        if (Week > S.BestWeekRevenue)
        {
            if (S.DaysCounted > RecordAfterDays && S.BestWeekRevenue > 0 && Closed - S.RecordDay >= RecordGapDays)
            {
                Celebrate(State, Closed, TEXT("En iyi hafta!"), FString::Printf(TEXT("Son yedi g\u00fcnde %s ciro; \u015fimdiye kadarki en iyi haftan."), *GoalMoney(Week)), 0);
                S.RecordDay = Closed;
            }
            S.BestWeekRevenue = Week;
        }
    }
    if (S.DaysCounted % 30 == 0 && S.RecentProfit.Num() >= 30)
    {
        int64 Month = 0;
        for (const int64 P : S.RecentProfit) Month += P;
        if (Month > S.BestMonthProfit)
        {
            if (S.BestMonthProfit > 0 && Closed - S.RecordDay >= RecordGapDays)
            {
                Celebrate(State, Closed, TEXT("En k\u00e2rl\u0131 ay!"), FString::Printf(TEXT("Son otuz g\u00fcn\u00fcn net k\u00e2r\u0131 %s; en iyi ay\u0131n."), *GoalMoney(Month)), 1);
                S.RecordDay = Closed;
            }
            S.BestMonthProfit = Month;
        }
    }
    S.MostStores = FMath::Max(S.MostStores, MarketCompany::TotalStores(State));

    // Goals: follow, finish, drop, refill.
    for (int32 I = 0; I < S.Goals.Num();)
    {
        FMarketGoal& G = S.Goals[I];
        G.Value = Measure(State, G, DayRevenue, DayProfit);
        const float Progress = ProgressOf(G);
        if (Progress >= 1.f)
        {
            const FString Title = TitleOf(State, G);
            const EScale Scale = static_cast<EScale>(G.Scale);
            Celebrate(State, Closed, TEXT("Hedef tamam!"), Title + TEXT(". ") + RewardText(Scale), static_cast<uint8>(G.Scale), false);
            CheerTeam(State, Scale == EScale::Short ? 3.f : Scale == EScale::Medium ? 5.f : 8.f);
            if (Scale == EScale::Medium)
            {
                FMarketSupplierAccount& A = MarketSuppliers::Account(State, MarketSuppliers::Current(State));
                A.Trust = FMath::Min(100, A.Trust + 3);
            }
            if (Scale == EScale::Long) MarketStory::AddMemory(State, Title);
            Remember(S, G.Kind);
            S.Goals.RemoveAt(I);
            continue;
        }
        if (Closed >= G.DueDay)
        {
            // Not a punishment: a missed week just makes room for the next goal; a month or a year gets one line.
            if (G.Scale != static_cast<uint8>(EScale::Short))
                State.DayNews.Add(FString::Printf(TEXT("Hedefin s\u00fcresi doldu: %s (%%%d tamamland\u0131). Yeni hedef geldi."), *TitleOf(State, G), FMath::RoundToInt32(Progress * 100.f)));
            Remember(S, G.Kind);
            S.Goals.RemoveAt(I);
            continue;
        }
        ++I;
    }
    FillSlots(State, Closed + 1);

    // Rhythm guard: a quiet stretch brings a pleasant or interesting event.
    if (State.Decisions.Num() > 0) S.LastLivelyDay = FMath::Max(S.LastLivelyDay, Closed);
    S.BadEventDays.RemoveAll([Closed](int32 Day) { return Day <= Closed - 7; });
    if (Closed - S.LastLivelyDay >= QuietDays(State) && !State.Story.bCampaignOver) // the rhythm runs as long as the company exists
    {
        const TCHAR* Pleasant[] = { TEXT("event.fair"), TEXT("event.newbuilding"), TEXT("event.wedding"), TEXT("event.derby") };
        const int32 Count = static_cast<int32>(UE_ARRAY_COUNT(Pleasant));
        const int32 Offset = static_cast<int32>(GoalMix(State.RivalSeed, Closed, 0x7117u) % static_cast<uint32>(Count));
        for (int32 I = 0; I < Count; ++I)
        {
            if (!MarketEvents::Trigger(State, Products, Pleasant[(I + Offset) % Count])) continue;
            ++S.QuietEvents;
            S.LastLivelyDay = Closed;
            break;
        }
    }
}

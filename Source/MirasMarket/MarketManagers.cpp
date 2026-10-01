#include "MarketManagers.h"
#include "MarketBranches.h"
#include "MarketCountry.h"
#include "MarketDepots.h"
#include "MarketPrices.h"
#include "MarketStaff.h"
#include "MarketStart.h"
#include "MarketStory.h"

namespace MarketManagers
{
    uint32 ManagerMix(int32 Seed, int32 Day, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), static_cast<uint32>(Day), Salt };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }

    FString ManagerTl(int64 Kurus)
    {
        return MarketCountry::Money(Kurus);
    }

    const TCHAR* TurkishFirst[] = { TEXT("Serkan"), TEXT("H\u00fclya"), TEXT("Volkan"), TEXT("G\u00fclay"), TEXT("Erdem"), TEXT("Nesrin"), TEXT("Kaan"),
        TEXT("Meltem"), TEXT("Tuncay"), TEXT("\u00d6zlem"), TEXT("Bar\u0131\u015f"), TEXT("Sevgi"), TEXT("Cengiz"), TEXT("P\u0131nar"),
        // G-086b ek (M22): enough names for a long campaign without repeats.
        TEXT("Ahmet"), TEXT("Ay\u015fe"), TEXT("Mehmet"), TEXT("Fatma"), TEXT("Mustafa"), TEXT("Emine"), TEXT("H\u00fcseyin"), TEXT("Hatice"),
        TEXT("\u0130brahim"), TEXT("Zeynep"), TEXT("Hasan"), TEXT("Elif"), TEXT("Osman"), TEXT("Merve"), TEXT("Yusuf"), TEXT("Esra"),
        TEXT("Burak"), TEXT("Seda"), TEXT("Emre"), TEXT("Gamze"), TEXT("Onur"), TEXT("Dilek"), TEXT("Sinan"), TEXT("Tu\u011fba"),
        TEXT("Halil"), TEXT("Song\u00fcl"), TEXT("Ercan"), TEXT("Yasemin"), TEXT("Levent"), TEXT("Serap"), TEXT("B\u00fclent"), TEXT("Arzu"),
        TEXT("Kadir"), TEXT("Esin"), TEXT("Tolga"), TEXT("Ebru"), TEXT("Ufuk"), TEXT("Derya"), TEXT("Alper"), TEXT("Nazl\u0131") };
    const TCHAR* TurkishLast[] = { TEXT("Y\u0131ld\u0131r\u0131m"), TEXT("Arslan"), TEXT("Do\u011fan"), TEXT("\u00c7etin"), TEXT("Ko\u00e7"), TEXT("Kaplan"), TEXT("\u00d6zdemir"),
        TEXT("Ta\u015f"), TEXT("U\u00e7ar"), TEXT("Polat"), TEXT("Erdo\u011fan"), TEXT("\u015eahin"),
        TEXT("Y\u0131lmaz"), TEXT("Kaya"), TEXT("Demir"), TEXT("\u00c7elik"), TEXT("Y\u0131ld\u0131z"), TEXT("Ayd\u0131n"), TEXT("\u00d6zt\u00fcrk"), TEXT("Kurt"),
        TEXT("\u00d6zkan"), TEXT("\u015eim\u015fek"), TEXT("Akta\u015f"), TEXT("Korkmaz"), TEXT("Karaca"), TEXT("Bulut"), TEXT("Tekin"), TEXT("Aksoy"),
        TEXT("G\u00fcler"), TEXT("Kara"), TEXT("Duman"), TEXT("Sar\u0131"), TEXT("Akg\u00fcl"), TEXT("Er"), TEXT("Kocaba\u015f"), TEXT("Tun\u00e7"),
        TEXT("Bozkurt"), TEXT("Yavuz"), TEXT("G\u00fcne\u015f"), TEXT("Soylu"), TEXT("Uysal"), TEXT("Kalkan"), TEXT("Avc\u0131"), TEXT("Turan") };
    // Names added to a foreign pack's own (Config/ulkeler.json keeps a few per country).
    const TCHAR* GermanFirst[] = { TEXT("Lukas"), TEXT("Anna"), TEXT("Jonas"), TEXT("Lena"), TEXT("Felix"), TEXT("Laura"), TEXT("Tobias"), TEXT("Julia"),
        TEXT("Stefan"), TEXT("Sabine"), TEXT("Markus"), TEXT("Katrin"), TEXT("Florian"), TEXT("Nina"), TEXT("Andreas"), TEXT("Claudia"), TEXT("Jan"), TEXT("Petra") };
    const TCHAR* GermanLast[] = { TEXT("M\u00fcller"), TEXT("Schmidt"), TEXT("Schneider"), TEXT("Fischer"), TEXT("Weber"), TEXT("Meyer"), TEXT("Wagner"),
        TEXT("Becker"), TEXT("Schulz"), TEXT("Hoffmann"), TEXT("Koch"), TEXT("Richter"), TEXT("Klein"), TEXT("Wolf"), TEXT("Neumann"), TEXT("Braun") };
    const TCHAR* EnglishFirst[] = { TEXT("James"), TEXT("Emma"), TEXT("Oliver"), TEXT("Sophie"), TEXT("Daniel"), TEXT("Grace"), TEXT("Thomas"), TEXT("Emily"),
        TEXT("Michael"), TEXT("Sarah"), TEXT("David"), TEXT("Hannah"), TEXT("Robert"), TEXT("Lucy"), TEXT("Matthew"), TEXT("Chloe"), TEXT("Andrew"), TEXT("Megan") };
    const TCHAR* EnglishLast[] = { TEXT("Smith"), TEXT("Jones"), TEXT("Taylor"), TEXT("Brown"), TEXT("Williams"), TEXT("Wilson"), TEXT("Johnson"),
        TEXT("Davies"), TEXT("Evans"), TEXT("Walker"), TEXT("Wright"), TEXT("Thompson"), TEXT("Harris"), TEXT("Clarke"), TEXT("Baker"), TEXT("Miller") };

    // First and last names of a country's pool: the pack's own plus the built-in ones (Turkish for Turkey and for a
    // pack without names).
    void NamePool(const FString& Country, TArray<FString>& OutFirst, TArray<FString>& OutLast)
    {
        const MarketCountry::FProfile* Pack = MarketCountry::Find(Country);
        const FString Id = Pack ? Pack->Id : FString(TEXT("tr"));
        auto AddAll = [](TArray<FString>& Into, const TCHAR* const* List, int32 Count) { for (int32 N = 0; N < Count; ++N) Into.AddUnique(FString(List[N])); };
        if (Pack && Id != TEXT("tr")) { for (const FString& X : Pack->FirstNames) OutFirst.AddUnique(X); for (const FString& X : Pack->LastNames) OutLast.AddUnique(X); }
        if (Id == TEXT("de")) { AddAll(OutFirst, GermanFirst, static_cast<int32>(UE_ARRAY_COUNT(GermanFirst))); AddAll(OutLast, GermanLast, static_cast<int32>(UE_ARRAY_COUNT(GermanLast))); }
        else if (Id == TEXT("gb") || Id == TEXT("us")) { AddAll(OutFirst, EnglishFirst, static_cast<int32>(UE_ARRAY_COUNT(EnglishFirst))); AddAll(OutLast, EnglishLast, static_cast<int32>(UE_ARRAY_COUNT(EnglishLast))); }
        if (OutFirst.Num() == 0 || OutLast.Num() == 0 || Id == TEXT("tr"))
        {
            AddAll(OutFirst, TurkishFirst, static_cast<int32>(UE_ARRAY_COUNT(TurkishFirst)));
            AddAll(OutLast, TurkishLast, static_cast<int32>(UE_ARRAY_COUNT(TurkishLast)));
            if (Pack) { for (const FString& X : Pack->FirstNames) OutFirst.AddUnique(X); for (const FString& X : Pack->LastNames) OutLast.AddUnique(X); }
        }
    }

    // Everyone whose name a new candidate must not take: used and turned-down names, and everybody working for us.
    TSet<FString> TakenNames(const FMarketState& State)
    {
        TSet<FString> Taken;
        for (const FString& Used : State.Management.UsedNames) Taken.Add(Used);
        for (const FMarketBranch& B : State.Branches) if (!B.ManagerName.IsEmpty()) Taken.Add(B.ManagerName);
        for (const FMarketManager& M : State.Management.Managers) Taken.Add(M.Name);
        for (const FMarketEmployee& E : State.Staff) Taken.Add(E.Name);
        return Taken;
    }

    // A name nobody has had: walks the first x last combinations from a seeded start; when all are taken, a double
    // surname (never a letter or a number added to a name).
    FString FreshName(const TArray<FString>& First, const TArray<FString>& Last, uint32 Roll, TSet<FString>& Taken)
    {
        const int32 F = First.Num(), L = Last.Num();
        if (F == 0 || L == 0) return FString();
        const int32 Pairs = F * L;
        const int32 Start = static_cast<int32>(Roll % static_cast<uint32>(Pairs));
        for (int32 K = 0; K < Pairs; ++K)
        {
            const int32 P = (Start + K) % Pairs;
            const FString Name = First[P % F] + TEXT(" ") + Last[P / F];
            if (!Taken.Contains(Name)) { Taken.Add(Name); return Name; }
        }
        const int64 Triples = static_cast<int64>(Pairs) * L;
        for (int64 K = 0; K < Triples; ++K)
        {
            const int64 P = (Start + K) % Triples;
            const int32 A = static_cast<int32>(P % F), B1 = static_cast<int32>((P / F) % L), B2 = static_cast<int32>(P / Pairs);
            if (B1 == B2) continue;
            const FString Name = First[A] + TEXT(" ") + Last[B1] + TEXT(" ") + Last[B2];
            if (!Taken.Contains(Name)) { Taken.Add(Name); return Name; }
        }
        return First[Start % F] + TEXT(" ") + Last[Start / F];
    }

    // The area an appointment really means (a country's own id, the family shop's home province).
    FString ResolveArea(const FMarketState& State, ELevel Level, const FString& Country, const FString& Area)
    {
        if (!Area.IsEmpty()) return Area;
        if (Level == ELevel::Country) return Country.IsEmpty() ? State.CountryId : Country;
        if (Level == ELevel::FamilyShop) return MarketStart::HomeProvince(State);
        return Area;
    }

    // M21: an older save's (or a promoted employee's) ceiling: the skill + 5..20, at most 95.
    int32 DerivedPotential(int32 Skill, uint32 Roll)
    {
        return FMath::Min(SkillTop, Skill + 5 + static_cast<int32>(Roll % 16u));
    }

    // "Tekirda\u011f'da", "K\u0131rklareli'de", "Sivas'ta": the locative of a place name.
    FString Locative(const FString& Name)
    {
        if (Name.IsEmpty()) return Name;
        const FString Back = TEXT("a\u0131ouAIOU");
        const FString Front = TEXT("ei\u00f6\u00fcE\u0130\u00d6\u00dc");
        const FString Hard = TEXT("fstk\u00e7\u015fhpFSTK\u00c7\u015eHP");
        TCHAR Vowel = TEXT('e');
        for (int32 I = Name.Len() - 1; I >= 0; --I)
        {
            int32 Found = INDEX_NONE;
            if (Back.FindChar(Name[I], Found)) { Vowel = TEXT('a'); break; }
            if (Front.FindChar(Name[I], Found)) { Vowel = TEXT('e'); break; }
        }
        int32 HardAt = INDEX_NONE;
        const TCHAR First = Hard.FindChar(Name[Name.Len() - 1], HardAt) ? TEXT('t') : TEXT('d');
        return Name + TEXT("'") + FString::Chr(First) + FString::Chr(Vowel);
    }

    FString CountryOr(const FMarketState& State, const FString& Country)
    {
        return Country.IsEmpty() ? State.CountryId : Country;
    }

    FString ProvinceOfBranch(const FMarketState& State, const FMarketBranch& Branch)
    {
        return Branch.Province.IsEmpty() ? MarketStart::HomeProvince(State) : Branch.Province;
    }

    bool IsOpenBranch(const FMarketBranch& B) { return B.Stage == static_cast<uint8>(MarketBranches::EStage::Open); }
    bool IsLiveBranch(const FMarketBranch& B) { return B.Stage != static_cast<uint8>(MarketBranches::EStage::Closed); }
    bool HasManager(const FMarketBranch& B) { return IsLiveBranch(B) && !B.ManagerName.IsEmpty(); }

    // Store, family shop and depot managers are the bottom rank; the rest are ranked by their level.
    int32 Rank(ELevel Level)
    {
        return Level == ELevel::FamilyShop || Level == ELevel::Store || Level == ELevel::Depot ? 0 : static_cast<int32>(Level);
    }

    // Where someone stands: the areas of every level above.
    struct FChain
    {
        FString Country;
        FString Province;
        FString Sub;
        FString Region;
    };

    FChain ChainOfProvince(const FString& Country, const FString& Province)
    {
        FChain Chain;
        Chain.Country = Country;
        Chain.Province = Province;
        if (const MarketCountry::FCity* City = MarketCountry::FindCity(Country, Province)) { Chain.Sub = City->SubRegion; Chain.Region = City->Region; }
        return Chain;
    }

    FString ParentOfSub(const FString& Country, const FString& Sub)
    {
        const MarketCountry::FProfile* Pack = MarketCountry::Find(Country);
        const MarketCountry::FRegion* Row = Pack ? Pack->SubRegions.FindByPredicate([&Sub](const MarketCountry::FRegion& R) { return R.Id == Sub; }) : nullptr;
        return Row ? Row->Parent : FString();
    }

    FChain ChainOfManager(const FMarketManager& M)
    {
        switch (static_cast<ELevel>(M.Level))
        {
        case ELevel::SubRegion: { FChain Chain; Chain.Country = M.Country; Chain.Sub = M.Area; Chain.Region = ParentOfSub(M.Country, M.Area); return Chain; }
        case ELevel::Region: { FChain Chain; Chain.Country = M.Country; Chain.Region = M.Area; return Chain; }
        case ELevel::Country: { FChain Chain; Chain.Country = M.Country; return Chain; }
        default: return ChainOfProvince(M.Country, M.Area);
        }
    }

    FChain ChainOfBranch(const FMarketState& State, const FMarketBranch& B)
    {
        return ChainOfProvince(MarketBranches::CountryOf(State, B), ProvinceOfBranch(State, B));
    }

    FString AreaIn(const FChain& Chain, ELevel Level)
    {
        switch (Level)
        {
        case ELevel::Province: return Chain.Province;
        case ELevel::SubRegion: return Chain.Sub;
        case ELevel::Region: return Chain.Region;
        case ELevel::Country: return Chain.Country;
        default: return Chain.Province;
        }
    }

    // The nearest appointed manager above a rank (INDEX_NONE = the player).
    int32 BossIn(const FMarketState& State, int32 FromRank, const FChain& Chain)
    {
        for (int32 L = FMath::Max(1, FromRank + 1); L <= static_cast<int32>(ELevel::Country); ++L)
        {
            const FString Area = AreaIn(Chain, static_cast<ELevel>(L));
            if (Area.IsEmpty()) continue;
            const int32 Found = FindManager(State, static_cast<ELevel>(L), Chain.Country, Area);
            if (Found != INDEX_NONE) return Found;
        }
        return INDEX_NONE;
    }

    TArray<FString> CountriesWithShops(const FMarketState& State)
    {
        TArray<FString> List;
        List.Add(State.CountryId); // the family shop
        for (const FMarketBranch& B : State.Branches) if (IsLiveBranch(B)) List.AddUnique(MarketBranches::CountryOf(State, B));
        return List;
    }

    // The boss of an appointed manager. G-089 (M23): a depot manager answers to his country's manager only (a
    // province or sub-region manager does not run a depot), else to the player.
    int32 BossOfManager(const FMarketState& State, const FMarketManager& M)
    {
        if (M.Level == static_cast<uint8>(ELevel::Depot)) return FindManager(State, ELevel::Country, M.Country, M.Country);
        return BossIn(State, Rank(static_cast<ELevel>(M.Level)), ChainOfManager(M));
    }

    int32 ProvinceManagersIn(const FMarketState& State, const FString& Country, const FString& Sub)
    {
        int32 Count = 0;
        for (const FMarketManager& M : State.Management.Managers)
            if (M.Level == static_cast<uint8>(ELevel::Province) && M.Country == Country && ChainOfManager(M).Sub == Sub) ++Count;
        return Count;
    }

    int32 SubManagersIn(const FMarketState& State, const FString& Country, const FString& Region)
    {
        int32 Count = 0;
        for (const FMarketManager& M : State.Management.Managers)
            if (M.Level == static_cast<uint8>(ELevel::SubRegion) && M.Country == Country && ParentOfSub(M.Country, M.Area) == Region) ++Count;
        return Count;
    }

    bool CountryMissing(const FMarketState& State, const FString& Country)
    {
        return CountryManagerRequired(State) && FindManager(State, ELevel::Country, Country, Country) == INDEX_NONE;
    }

    int32 MoraleSkill(float Morale)
    {
        return Morale < 0.f ? 0 : FMath::RoundToInt32((Morale - 60.f) / 4.f);
    }

    int32 EffManager(const FMarketState& State, int32 ManagerIndex, int32 Span);

    float StrengthOf(const FMarketState& State, int32 ManagerIndex, int32 Span)
    {
        if (!State.Management.Managers.IsValidIndex(ManagerIndex)) return 0.f;
        const FMarketManager& M = State.Management.Managers[ManagerIndex];
        if (M.Level == static_cast<uint8>(ELevel::FamilyShop)) return 1.f;
        const int32 Need = RequiredSkill(State, ManagerIndex);
        float Value = FMath::Clamp((EffManager(State, ManagerIndex, Span) - (Need - 30)) / 30.f, 0.f, 1.f);
        if (M.Honesty < 35) Value *= 0.5f; // busy with his own business
        return Value;
    }

    int32 EffManager(const FMarketState& State, int32 ManagerIndex, int32 Span)
    {
        if (!State.Management.Managers.IsValidIndex(ManagerIndex)) return 0;
        const FMarketManager& M = State.Management.Managers[ManagerIndex];
        int32 Skill = M.Skill + MoraleSkill(M.Morale);
        const int32 Boss = BossOfManager(State, M);
        if (Boss == INDEX_NONE) Skill -= Span;
        else Skill += FMath::RoundToInt32(5.f * StrengthOf(State, Boss, Span));
        if (M.Level != static_cast<uint8>(ELevel::Country) && CountryMissing(State, M.Country)) Skill -= MissingCountryPenalty;
        return FMath::Clamp(Skill, 0, 100);
    }

    float OversightOf(const FMarketState& State, int32 BranchIndex, int32 Span)
    {
        if (!State.Branches.IsValidIndex(BranchIndex)) return 0.f;
        const FChain Chain = ChainOfBranch(State, State.Branches[BranchIndex]);
        const int32 Province = FindManager(State, ELevel::Province, Chain.Country, Chain.Province);
        if (Province != INDEX_NONE) return StrengthOf(State, Province, Span);
        const int32 Boss = BossIn(State, 0, Chain);
        return Boss != INDEX_NONE ? 0.5f * StrengthOf(State, Boss, Span) : 0.f;
    }

    // A candidate becomes a branch's store manager (a settling-in week); his name is used from now on.
    void HireFrom(FMarketState& State, int32 BranchIndex, const FCandidate& Who)
    {
        FMarketBranch& B = State.Branches[BranchIndex];
        B.ManagerName = Who.Name;
        B.ManagerSkill = Who.Skill;
        B.ManagerHonesty = Who.Honesty;
        B.ManagerWage = Who.Wage;
        ++State.Management.Hires;
        State.Management.UsedNames.AddUnique(Who.Name);
        InitStoreManager(State, B, BranchIndex);
        if (Who.Style != 0) B.ManagerStyle = Who.Style;
        B.ManagerPotential = Who.Potential;
    }

    // The candidates the player looked at are not offered again.
    void MarkSeen(FMarketState& State, const TArray<FCandidate>& Pool)
    {
        for (const FCandidate& Seen : Pool) State.Management.UsedNames.AddUnique(Seen.Name);
    }

    FMarketManager ManagerFrom(const FMarketState& State, const FCandidate& Who)
    {
        FMarketManager M;
        M.Level = static_cast<uint8>(Who.Level);
        M.Country = Who.Country;
        M.Area = Who.Area;
        M.Name = Who.Name;
        M.Skill = Who.Skill;
        M.Honesty = Who.Honesty;
        M.Style = Who.Style;
        M.Potential = Who.Potential;
        M.BaseWage = Who.BaseWage;
        M.Morale = 70.f;
        M.AppointedDay = State.Day;
        return M;
    }

    void ClearStoreManager(FMarketBranch& B)
    {
        B.ManagerName.Reset();
        B.ManagerSkill = 0;
        B.ManagerHonesty = 70;
        B.ManagerWage = 0;
        B.ManagerStyle = 0;
        B.ManagerMorale = -1.f;
        B.ManagerWarnings = 0;
        B.ManagerSince = 0;
        B.ManagerBadWeeks = 0;
        B.ManagerGoodWeeks = 0;
        B.ManagerBonusDay = 0;
        B.ManagerWarnedDay = 0;
        B.ManagerCaughtDay = 0;
        B.ManagerPotential = 0;
    }

    FString TitleOf(const FMarketManager& M)
    {
        const ELevel Level = static_cast<ELevel>(M.Level);
        if (Level == ELevel::FamilyShop) return LevelName(Level);
        return AreaName(Level, M.Country, M.Area) + TEXT(" ") + LevelName(Level);
    }

    FString BossText(const FMarketState& State, int32 Boss)
    {
        return State.Management.Managers.IsValidIndex(Boss) ? FString::Printf(TEXT("%s (%s)"), *State.Management.Managers[Boss].Name, *TitleOf(State.Management.Managers[Boss])) : FString(TEXT("sana"));
    }
}

FString MarketManagers::LevelName(ELevel Level)
{
    switch (Level)
    {
    case ELevel::Province: return TEXT("il m\u00fcd\u00fcr\u00fc");
    case ELevel::SubRegion: return TEXT("b\u00f6lge m\u00fcd\u00fcr\u00fc");
    case ELevel::Region: return TEXT("b\u00f6lge direkt\u00f6r\u00fc");
    case ELevel::Country: return TEXT("\u00fclke m\u00fcd\u00fcr\u00fc");
    case ELevel::FamilyShop: return TEXT("aile d\u00fckk\u00e2n\u0131 m\u00fcd\u00fcr\u00fc");
    case ELevel::Depot: return TEXT("depo m\u00fcd\u00fcr\u00fc");
    default: return TEXT("ma\u011faza m\u00fcd\u00fcr\u00fc");
    }
}

FString MarketManagers::StyleName(uint8 Style)
{
    switch (static_cast<EStyle>(Style))
    {
    case EStyle::Careful: return TEXT("Temkinli");
    case EStyle::Generous: return TEXT("C\u00f6mert");
    case EStyle::PriceMinded: return TEXT("Fiyat\u00e7\u0131");
    default: return TEXT("?");
    }
}

const TArray<MarketManagers::ELevel>& MarketManagers::Tiers()
{
    static const TArray<ELevel> List = { ELevel::Country, ELevel::Region, ELevel::SubRegion, ELevel::Province };
    return List;
}

bool MarketManagers::IsTierVisible(const FMarketState& State, ELevel Level, const FString& Country, const FString& Area)
{
    const FString C = CountryOr(State, Country);
    const FString A = ResolveArea(State, Level, C, Area);
    if (FindManager(State, Level, C, A) != INDEX_NONE) return true;
    FString Reason;
    return CanAppoint(State, Level, C, A, INDEX_NONE, Reason);
}

TArray<MarketManagers::ELevel> MarketManagers::VisibleTiers(const FMarketState& State, const FString& Country)
{
    TArray<ELevel> List;
    const FString C = CountryOr(State, Country);
    for (const ELevel Level : Tiers())
        if (Level != ELevel::Country || IsTierVisible(State, ELevel::Country, C, C)) List.Add(Level);
    return List;
}

int32 MarketManagers::ProvincesWithShops(const FMarketState& State, const FString& Country)
{
    const FString C = CountryOr(State, Country);
    TArray<FString> Provinces;
    if (C == State.CountryId) Provinces.Add(MarketStart::HomeProvince(State)); // the family shop
    for (const FMarketBranch& B : State.Branches)
        if (IsOpenBranch(B) && MarketBranches::CountryOf(State, B) == C) Provinces.AddUnique(ProvinceOfBranch(State, B));
    return Provinces.Num();
}

FString MarketManagers::AreaName(ELevel Level, const FString& Country, const FString& Area)
{
    const MarketCountry::FProfile* Pack = MarketCountry::Find(Country);
    switch (Level)
    {
    case ELevel::Country: return Pack ? Pack->Name : Country;
    case ELevel::Region:
    {
        const MarketCountry::FRegion* Row = Pack ? Pack->Regions.FindByPredicate([&Area](const MarketCountry::FRegion& R) { return R.Id == Area; }) : nullptr;
        return Row ? Row->Name : Area;
    }
    case ELevel::SubRegion:
    {
        const MarketCountry::FRegion* Row = Pack ? Pack->SubRegions.FindByPredicate([&Area](const MarketCountry::FRegion& R) { return R.Id == Area; }) : nullptr;
        return Row ? Row->Name : Area;
    }
    default:
    {
        const MarketCountry::FCity* City = MarketCountry::FindCity(Country, Area);
        return City ? City->Name : Area;
    }
    }
}

int32 MarketManagers::FindManager(const FMarketState& State, ELevel Level, const FString& Country, const FString& Area)
{
    const FString C = CountryOr(State, Country);
    const FString A = Level == ELevel::Country && Area.IsEmpty() ? C : Level == ELevel::FamilyShop && Area.IsEmpty() ? MarketStart::HomeProvince(State) : Area;
    const TArray<FMarketManager>& List = State.Management.Managers;
    for (int32 I = 0; I < List.Num(); ++I)
        if (List[I].Level == static_cast<uint8>(Level) && List[I].Country == C && List[I].Area == A) return I;
    return INDEX_NONE;
}

const FMarketManager* MarketManagers::ManagerOf(const FMarketState& State, ELevel Level, const FString& Country, const FString& Area)
{
    const int32 Index = FindManager(State, Level, Country, Area);
    return Index != INDEX_NONE ? &State.Management.Managers[Index] : nullptr;
}

const FMarketManager* MarketManagers::ProvinceManager(const FMarketState& State, const FString& Country, const FString& Province) { return ManagerOf(State, ELevel::Province, Country, Province); }
const FMarketManager* MarketManagers::SubRegionManager(const FMarketState& State, const FString& Country, const FString& SubRegion) { return ManagerOf(State, ELevel::SubRegion, Country, SubRegion); }
const FMarketManager* MarketManagers::RegionManager(const FMarketState& State, const FString& Country, const FString& Region) { return ManagerOf(State, ELevel::Region, Country, Region); }
const FMarketManager* MarketManagers::CountryManager(const FMarketState& State, const FString& Country) { return ManagerOf(State, ELevel::Country, Country, CountryOr(State, Country)); }

TArray<MarketManagers::FPerson> MarketManagers::People(const FMarketState& State, ELevel Level)
{
    TArray<FPerson> List;
    if (Level == ELevel::Store)
    {
        for (int32 I = 0; I < State.Branches.Num(); ++I)
        {
            const FMarketBranch& B = State.Branches[I];
            if (!HasManager(B)) continue;
            FPerson Person;
            Person.Level = ELevel::Store;
            Person.Branch = I;
            Person.Name = B.ManagerName;
            Person.Title = B.Name + TEXT(" m\u00fcd\u00fcr\u00fc");
            List.Add(Person);
        }
        return List;
    }
    const TArray<FMarketManager>& Managers = State.Management.Managers;
    for (int32 I = 0; I < Managers.Num(); ++I)
    {
        if (Managers[I].Level != static_cast<uint8>(Level)) continue;
        FPerson Person;
        Person.Level = Level;
        Person.Manager = I;
        Person.Name = Managers[I].Name;
        Person.Title = TitleOf(Managers[I]);
        List.Add(Person);
    }
    return List;
}

int32 MarketManagers::BossOf(const FMarketState& State, const FPerson& Person)
{
    if (Person.Level == ELevel::Store)
        return State.Branches.IsValidIndex(Person.Branch) ? BossIn(State, 0, ChainOfBranch(State, State.Branches[Person.Branch])) : INDEX_NONE;
    if (!State.Management.Managers.IsValidIndex(Person.Manager)) return INDEX_NONE;
    return BossOfManager(State, State.Management.Managers[Person.Manager]);
}

TArray<MarketManagers::FPerson> MarketManagers::DirectReports(const FMarketState& State)
{
    TArray<FPerson> List;
    const ELevel Order[7] = { ELevel::Country, ELevel::Region, ELevel::SubRegion, ELevel::Province, ELevel::Depot, ELevel::FamilyShop, ELevel::Store };
    for (const ELevel Level : Order)
        for (const FPerson& Person : People(State, Level))
            if (BossOf(State, Person) == INDEX_NONE) List.Add(Person);
    return List;
}

int32 MarketManagers::DirectCount(const FMarketState& State)
{
    return DirectReports(State).Num();
}

int32 MarketManagers::OverLimit(const FMarketState& State)
{
    return FMath::Max(0, DirectCount(State) - SpanLimit);
}

int32 MarketManagers::SpanPenalty(const FMarketState& State)
{
    return FMath::Min(SpanPenaltyMax, OverLimit(State) * SpanPenaltyPerPerson);
}

bool MarketManagers::ReportsToPlayer(const FMarketState& State, int32 BranchIndex)
{
    return State.Branches.IsValidIndex(BranchIndex) && BossIn(State, 0, ChainOfBranch(State, State.Branches[BranchIndex])) == INDEX_NONE;
}

int32 MarketManagers::ProvinceBranches(const FMarketState& State, const FString& Country, const FString& Province)
{
    const FString C = CountryOr(State, Country);
    int32 Count = 0;
    for (const FMarketBranch& B : State.Branches)
        if (IsOpenBranch(B) && MarketBranches::CountryOf(State, B) == C && ProvinceOfBranch(State, B) == Province) ++Count;
    return Count;
}

int32 MarketManagers::ProvinceRequiredSkill(int32 Shops)
{
    return 40 + FMath::Max(0, Shops) / 3;
}

int32 MarketManagers::RequiredSkill(const FMarketState& State, int32 ManagerIndex)
{
    if (!State.Management.Managers.IsValidIndex(ManagerIndex)) return 0;
    const FMarketManager& M = State.Management.Managers[ManagerIndex];
    switch (static_cast<ELevel>(M.Level))
    {
    case ELevel::Province: return ProvinceRequiredSkill(ProvinceBranches(State, M.Country, M.Area));
    case ELevel::SubRegion: return 45 + 2 * ProvinceManagersIn(State, M.Country, M.Area);
    case ELevel::Region: return 50 + 3 * SubManagersIn(State, M.Country, M.Area);
    case ELevel::Country: return 55;
    case ELevel::Depot: // G-089: a busier depot needs a better manager
    {
        const int32 Depot = MarketDepots::Find(State, M.Country, M.Area);
        return 40 + FMath::Min(30, (Depot != INDEX_NONE ? MarketDepots::Served(State, Depot) : 0) / 3);
    }
    default: return 30;
    }
}

int32 MarketManagers::EffectiveManagerSkill(const FMarketState& State, int32 ManagerIndex)
{
    return EffManager(State, ManagerIndex, SpanPenalty(State));
}

float MarketManagers::Strength(const FMarketState& State, int32 ManagerIndex)
{
    return StrengthOf(State, ManagerIndex, SpanPenalty(State));
}

float MarketManagers::Oversight(const FMarketState& State, int32 BranchIndex)
{
    return OversightOf(State, BranchIndex, SpanPenalty(State));
}

MarketManagers::FBranchRule MarketManagers::RuleFor(const FMarketState& State, int32 BranchIndex, int32 Span)
{
    FBranchRule Rule;
    Rule.WasteRate = 0.006f;
    if (!State.Branches.IsValidIndex(BranchIndex)) return Rule;
    const FMarketBranch& B = State.Branches[BranchIndex];
    if (B.ManagerName.IsEmpty()) return Rule; // the player's standing orders
    const int32 SpanNow = Span >= 0 ? Span : SpanPenalty(State);
    const bool bDirect = ReportsToPlayer(State, BranchIndex);
    const float Watch = OversightOf(State, BranchIndex, SpanNow);
    int32 Skill = B.ManagerSkill + MoraleSkill(B.ManagerMorale) + FMath::RoundToInt32(10.f * Watch);
    if (B.ManagerSince > 0 && State.Day - B.ManagerSince < SettleDays) Skill -= SettlePenalty;
    if (bDirect) Skill -= SpanNow;
    if (CountryMissing(State, MarketBranches::CountryOf(State, B))) Skill -= MissingCountryPenalty;
    Rule.Skill = FMath::Clamp(Skill, 0, 100);
    Rule.ErrorFactor = 1.f - 0.4f * Watch;
    switch (static_cast<EStyle>(B.ManagerStyle))
    {
    case EStyle::Careful: Rule.OrderFactor = 0.85f; Rule.WasteRate = 0.003f; Rule.PriceBias = 0.01f; break;
    case EStyle::Generous: Rule.OrderFactor = 1.25f; Rule.WasteRate = 0.012f; break;
    case EStyle::PriceMinded: Rule.PriceBias = -0.03f; break;
    default: break;
    }
    Rule.bFollowsRivals = Rule.Skill >= 60 || B.ManagerStyle == static_cast<uint8>(EStyle::PriceMinded);
    const bool bDeterred = B.ManagerWarnedDay > 0 && State.Day - B.ManagerWarnedDay < WarnDeterDays;
    if (B.ManagerHonesty < 35 && !bDeterred) Rule.SkimPermille = 15;
    else if (B.ManagerHonesty < 50 && B.ManagerMorale >= 0.f && B.ManagerMorale < 25.f) Rule.SkimPermille = 5; // an unhappy one starts
    Rule.bSkimHidden = bDirect && SpanNow > 0;
    return Rule;
}

int32 MarketManagers::EffectiveSkill(const FMarketState& State, int32 BranchIndex)
{
    return RuleFor(State, BranchIndex).Skill;
}

float MarketManagers::GradeBonus(const FMarketState& State, int32 BranchIndex)
{
    return 0.05f * Oversight(State, BranchIndex);
}

int32 MarketManagers::OpeningDaysSavedIn(const FMarketState& State, const FString& Country, const FString& Province)
{
    const int32 Index = FindManager(State, ELevel::Province, Country, Province);
    return Index != INDEX_NONE ? FMath::RoundToInt32(OpeningDaysSaved * Strength(State, Index)) : 0;
}

float MarketManagers::CostAdjust(const FMarketState& State, const FMarketBranch& Branch)
{
    if (State.Management.Managers.Num() == 0) return 0.f;
    const MarketBranches::FSite Site = MarketBranches::SiteOf(State, Branch);
    const FChain Chain = ChainOfProvince(Site.Country, Site.Province);
    const int32 Span = SpanPenalty(State);
    float Adjust = 0.f;
    const int32 Sub = Chain.Sub.IsEmpty() ? INDEX_NONE : FindManager(State, ELevel::SubRegion, Chain.Country, Chain.Sub);
    if (Sub != INDEX_NONE && !Site.bHome) Adjust -= 0.01f * StrengthOf(State, Sub, Span);   // fewer losses on the road
    const int32 Director = Chain.Region.IsEmpty() ? INDEX_NONE : FindManager(State, ELevel::Region, Chain.Country, Chain.Region);
    if (Director != INDEX_NONE) Adjust -= 0.005f * StrengthOf(State, Director, Span);       // depots planned together
    const int32 Head = FindManager(State, ELevel::Country, Chain.Country, Chain.Country);
    if (Head != INDEX_NONE && Site.bAbroad) Adjust -= 0.01f * StrengthOf(State, Head, Span); // knows the wholesalers
    return Adjust;
}

bool MarketManagers::CountryManagerRequired(const FMarketState& State)
{
    return CountriesWithShops(State).Num() >= 2;
}

TArray<FString> MarketManagers::CountriesMissingManager(const FMarketState& State)
{
    TArray<FString> Missing;
    const TArray<FString> Countries = CountriesWithShops(State);
    if (Countries.Num() < 2) return Missing;
    for (const FString& C : Countries)
        if (FindManager(State, ELevel::Country, C, C) == INDEX_NONE) Missing.Add(C);
    return Missing;
}

int64 MarketManagers::BaseWageFor(ELevel Level, int32 Skill, const FString& Country)
{
    const double T = FMath::Clamp((Skill - 30) / 60.0, 0.0, 1.0);
    double Wage = 3000.0 + 1500.0 * T;
    switch (Level)
    {
    case ELevel::Province: Wage = 6000.0 + 2000.0 * T; break;
    case ELevel::SubRegion: Wage = 12000.0 + 3000.0 * T; break;
    case ELevel::Region: Wage = 18000.0 + 4000.0 * T; break;
    case ELevel::Depot: Wage = 8000.0 + 4000.0 * T; break; // G-089: 80-120 TL a day at the start level
    case ELevel::Country:
    {
        const MarketCountry::FProfile* Pack = MarketCountry::Find(Country);
        Wage = 25000.0 * (0.9 + 0.2 * T) * FMath::Max(0.5, Pack ? static_cast<double>(Pack->WageFactor) : 1.0);
        break;
    }
    default: break;
    }
    return FMath::RoundToInt64(Wage / 50.0) * 50;
}

int64 MarketManagers::DailyWage(const FMarketState& State, const FMarketManager& Manager)
{
    return MarketPrices::WageScaled(Manager.BaseWage, State.Day);
}

int64 MarketManagers::DailyWages(const FMarketState& State)
{
    int64 Sum = 0;
    for (const FMarketManager& M : State.Management.Managers) Sum += DailyWage(State, M);
    return Sum;
}

bool MarketManagers::SkillVisible(const FMarketState& State, int32 BranchIndex)
{
    if (MarketStaff::HasHr(State)) return true;
    if (!State.Branches.IsValidIndex(BranchIndex)) return false;
    if (MarketBranches::RecentlyVisited(State, BranchIndex)) return true; // C3: the player saw the manager at work
    const FChain Chain = ChainOfBranch(State, State.Branches[BranchIndex]);
    return FindManager(State, ELevel::Province, Chain.Country, Chain.Province) != INDEX_NONE;
}

int32 MarketManagers::CandidateWeek(const FMarketState& State)
{
    return FMath::Max(0, State.Day - 1) / 7;
}

TArray<MarketManagers::FCandidate> MarketManagers::Candidates(const FMarketState& State, ELevel Level, const FString& Country, const FString& Area)
{
    TArray<FCandidate> Pool;
    const FString C = CountryOr(State, Country);
    const FString A = ResolveArea(State, Level, C, Area);
    // One pool per level and area; it changes with the week and after every hire (not with saving and loading).
    const uint32 Key = GetTypeHash(FString::Printf(TEXT("%d/%s/%s"), static_cast<int32>(Level), *C, *A));
    const uint32 Base = ManagerMix(State.RivalSeed, CandidateWeek(State), 0xCA4Du ^ Key);
    const uint32 Seed = ManagerMix(static_cast<int32>(Base), State.Management.Hires, 0x0B11u);
    TArray<FString> First, Last;
    NamePool(C, First, Last);
    TSet<FString> Taken = TakenNames(State);
    for (int32 Slot = 0; Slot < CandidateCount; ++Slot)
    {
        const uint32 NameRoll = ManagerMix(static_cast<int32>(Seed), Slot, 0xC0DEu);
        const uint32 Roll = ManagerMix(static_cast<int32>(NameRoll), Slot, 0xBEEFu);
        FCandidate Who;
        Who.Level = Level;
        Who.Country = C;
        Who.Area = A;
        Who.Name = FreshName(First, Last, NameRoll, Taken);
        if (Level == ELevel::Store) Who.Skill = 35 + static_cast<int32>(Roll % 46u);
        else Who.Skill = 40 + static_cast<int32>(Roll % 41u) + (Rank(Level) >= 2 ? 5 : 0);
        Who.Honesty = (Roll >> 8) % 100u < 11u ? 20 + static_cast<int32>((Roll >> 12) % 10u) : 60 + static_cast<int32>((Roll >> 16) % 40u);
        Who.Style = static_cast<uint8>(1u + (Roll >> 24) % 3u);
        // A good candidate usually has room to grow too.
        const int32 Room = 5 + static_cast<int32>((NameRoll >> 20) % 21u) + (Who.Skill >= 65 ? 5 : 0);
        Who.Potential = FMath::Clamp(Who.Skill + Room, PotentialMin, SkillTop);
        const int64 Ask = 100 + static_cast<int64>((NameRoll >> 12) % 9u); // 100..108 % of the band
        if (Level == ELevel::Store)
        {
            Who.Wage = MarketStaff::FairWage(MarketStaff::ERole::HrManager, Who.Skill, State.Day) * 9 / 10 * Ask / 100;
            Who.BaseWage = MarketStaff::FairWage(MarketStaff::ERole::HrManager, Who.Skill, 1) * 9 / 10 * Ask / 100;
        }
        else
        {
            Who.BaseWage = FMath::RoundToInt64(static_cast<double>(BaseWageFor(Level, Who.Skill, C) * Ask) / 5000.0) * 50;
            Who.Wage = MarketPrices::WageScaled(Who.BaseWage, State.Day);
        }
        Pool.Add(Who);
    }
    return Pool;
}

TArray<MarketManagers::FCandidate> MarketManagers::BranchCandidates(const FMarketState& State, int32 BranchIndex)
{
    if (!State.Branches.IsValidIndex(BranchIndex)) return TArray<FCandidate>();
    const FMarketBranch& B = State.Branches[BranchIndex];
    return Candidates(State, ELevel::Store, MarketBranches::CountryOf(State, B), ProvinceOfBranch(State, B));
}

void MarketManagers::SkillRange(const FMarketState& State, const FCandidate& Who, int32& OutLow, int32& OutHigh)
{
    if (MarketStaff::HasHr(State)) { OutLow = Who.Skill; OutHigh = Who.Skill; return; }
    // A seeded guess within 6 points, shown +-12: the range always holds the real skill.
    const int32 Guess = Who.Skill + static_cast<int32>(ManagerMix(State.RivalSeed, Who.Skill, GetTypeHash(Who.Name)) % 13u) - 6;
    OutLow = FMath::Max(0, Guess - CandidateRangeHalf);
    OutHigh = FMath::Min(100, Guess + CandidateRangeHalf);
}

bool MarketManagers::HonestyVisible(const FMarketState& State)
{
    return MarketStaff::HasHr(State);
}

FString MarketManagers::PotentialHint(int32 Skill, int32 Potential)
{
    const int32 Gap = FMath::Min(Potential, SkillTop) - Skill;
    if (Gap >= 15) return TEXT("geli\u015fmeye \u00e7ok a\u00e7\u0131k");
    if (Gap >= 6) return TEXT("biraz geli\u015fir");
    return TEXT("olgun, pek geli\u015fmez");
}

FString MarketManagers::DescribeCandidate(const FMarketState& State, const FCandidate& Who)
{
    TArray<FString> Parts;
    Parts.Add(Who.Name);
    int32 Low = 0, High = 0;
    SkillRange(State, Who, Low, High);
    Parts.Add(Low == High ? FString::Printf(TEXT("beceri %d"), Low) : FString::Printf(TEXT("beceri %d-%d"), Low, High));
    if (HonestyVisible(State)) Parts.Add(FString::Printf(TEXT("d\u00fcr\u00fcstl\u00fck %d"), Who.Honesty));
    Parts.Add(FString::Printf(TEXT("tarz %s"), *StyleName(Who.Style)));
    Parts.Add(PotentialHint(Who.Skill, Who.Potential));
    Parts.Add(FString::Printf(TEXT("ayda %s"), *ManagerTl(Who.Wage * 30)));
    return FString::Join(Parts, TEXT(" \u00b7 "));
}

FMarketManager MarketManagers::Candidate(const FMarketState& State, ELevel Level, const FString& Country, const FString& Area)
{
    const TArray<FCandidate> Pool = Candidates(State, Level, Country, Area);
    return Pool.Num() > 0 ? ManagerFrom(State, Pool[0]) : FMarketManager();
}

FString MarketManagers::AreaOfBranch(const FMarketState& State, int32 BranchIndex, ELevel Level)
{
    if (!State.Branches.IsValidIndex(BranchIndex)) return FString();
    return AreaIn(ChainOfBranch(State, State.Branches[BranchIndex]), Level);
}

bool MarketManagers::CanAppoint(const FMarketState& State, ELevel Level, const FString& Country, const FString& Area, int32 FromBranch, FString& OutReason)
{
    const FString C = CountryOr(State, Country);
    const MarketCountry::FProfile* Pack = MarketCountry::Find(C);
    if (!Pack) { OutReason = TEXT("B\u00f6yle bir \u00fclke yok."); return false; }
    const FString A = Level == ELevel::Country && Area.IsEmpty() ? C : Level == ELevel::FamilyShop && Area.IsEmpty() ? MarketStart::HomeProvince(State) : Area;
    const FString Where = AreaName(Level, C, A);
    if (const FMarketManager* Existing = ManagerOf(State, Level, C, A))
    {
        OutReason = FString::Printf(TEXT("%s i\u00e7in zaten %s var: %s."), *Where, *LevelName(Level), *Existing->Name);
        return false;
    }
    switch (Level)
    {
    case ELevel::Province:
    {
        if (!MarketCountry::FindCity(C, A)) { OutReason = TEXT("B\u00f6yle bir il yok."); return false; }
        const int32 Shops = ProvinceBranches(State, C, A);
        if (Shops < ProvinceShops)
        {
            OutReason = FString::Printf(TEXT("%s %d ma\u011faza var; il m\u00fcd\u00fcr\u00fc i\u00e7in en az %d ma\u011faza gerekir (aile d\u00fckk\u00e2n\u0131 say\u0131lmaz)."), *Locative(Where), Shops, ProvinceShops);
            return false;
        }
        break;
    }
    case ELevel::SubRegion:
    {
        if (!Pack->SubRegions.ContainsByPredicate([&A](const MarketCountry::FRegion& R) { return R.Id == A; })) { OutReason = TEXT("B\u00f6yle bir b\u00f6lge yok."); return false; }
        const int32 Count = ProvinceManagersIn(State, C, A);
        if (Count < SubRegionManagers)
        {
            OutReason = FString::Printf(TEXT("%s %d il m\u00fcd\u00fcr\u00fc var; b\u00f6lge m\u00fcd\u00fcr\u00fc i\u00e7in en az %d gerekir."), *Locative(Where), Count, SubRegionManagers);
            return false;
        }
        break;
    }
    case ELevel::Region:
    {
        if (!Pack->Regions.ContainsByPredicate([&A](const MarketCountry::FRegion& R) { return R.Id == A; })) { OutReason = TEXT("B\u00f6yle bir b\u00f6lge yok."); return false; }
        const int32 Count = SubManagersIn(State, C, A);
        if (Count < RegionManagers)
        {
            OutReason = FString::Printf(TEXT("%s %d b\u00f6lge m\u00fcd\u00fcr\u00fc var; b\u00f6lge direkt\u00f6r\u00fc i\u00e7in en az %d gerekir."), *Locative(Where), Count, RegionManagers);
            return false;
        }
        break;
    }
    case ELevel::Country:
    {
        if (!CountriesWithShops(State).Contains(C)) { OutReason = FString::Printf(TEXT("%s ma\u011fazan yok."), *Locative(Where)); return false; }
        // M20: 5 provinces of the country first; in two countries a country manager is required everywhere.
        const int32 Provinces = ProvincesWithShops(State, C);
        if (!CountryManagerRequired(State) && Provinces < CountryProvinces)
        {
            OutReason = FString::Printf(TEXT("%s %d ilde ma\u011fazan var; \u00fclke m\u00fcd\u00fcr\u00fc i\u00e7in en az %d il gerekir (aile d\u00fckk\u00e2n\u0131n\u0131n ili dahil)."), *Locative(Where), Provinces, CountryProvinces);
            return false;
        }
        break;
    }
    case ELevel::FamilyShop:
        if (C != State.CountryId || A != MarketStart::HomeProvince(State)) { OutReason = TEXT("Aile d\u00fckk\u00e2n\u0131 yaln\u0131z ev ilinde."); return false; }
        // M19: only once a branch outside the family shop is open.
        if (!State.Branches.ContainsByPredicate([](const FMarketBranch& B) { return IsOpenBranch(B); })) { OutReason = TEXT("\u00d6nce ikinci ma\u011fazan\u0131 a\u00e7."); return false; }
        if (FromBranch != INDEX_NONE) { OutReason = TEXT("Aile d\u00fckk\u00e2n\u0131na d\u0131\u015far\u0131dan bir m\u00fcd\u00fcr gelir."); return false; }
        break;
    case ELevel::Depot:
    {
        // M23 (G-089): a depot of ours in the province.
        const MarketCountry::FCity* City = MarketCountry::FindCity(C, A);
        if (!City) { OutReason = TEXT("B\u00f6yle bir il yok."); return false; }
        if (!MarketDepots::HasDepotIn(State, C, A))
        {
            OutReason = FString::Printf(TEXT("Depo m\u00fcd\u00fcr\u00fc i\u00e7in \u00f6nce %s bir depo kur."), *Locative(Where));
            return false;
        }
        break;
    }
    default:
        OutReason = TEXT("Ma\u011faza m\u00fcd\u00fcr\u00fcn\u00fc \u015fubenin kendisinden de\u011fi\u015ftir.");
        return false;
    }
    if (FromBranch != INDEX_NONE)
    {
        if (!State.Branches.IsValidIndex(FromBranch) || !IsOpenBranch(State.Branches[FromBranch]) || State.Branches[FromBranch].ManagerName.IsEmpty())
        {
            OutReason = TEXT("Terfi i\u00e7in m\u00fcd\u00fcr\u00fc olan a\u00e7\u0131k bir \u015fube se\u00e7.");
            return false;
        }
        if (MarketBranches::CountryOf(State, State.Branches[FromBranch]) != C || AreaOfBranch(State, FromBranch, Level) != A)
        {
            OutReason = FString::Printf(TEXT("Terfi eden m\u00fcd\u00fcr\u00fcn ma\u011fazas\u0131 %s i\u00e7inde olmal\u0131."), *Where);
            return false;
        }
        if (State.Branches[FromBranch].ManagerCaughtDay > 0) { OutReason = TEXT("Kasadan ald\u0131\u011f\u0131 bilinen biri terfi edemez."); return false; }
    }
    return true;
}

bool MarketManagers::Appoint(FMarketState& State, ELevel Level, const FString& Country, const FString& Area, int32 FromBranch, FString& OutMessage)
{
    if (FromBranch == INDEX_NONE) return AppointCandidate(State, Level, Country, Area, 0, OutMessage); // M22: the pool's first
    if (!CanAppoint(State, Level, Country, Area, FromBranch, OutMessage)) return false;
    const bool bFirstProvince = Level == ELevel::Province && !State.Management.Managers.ContainsByPredicate([](const FMarketManager& X) { return X.Level == static_cast<uint8>(ELevel::Province); });
    const FString C = CountryOr(State, Country);
    const FMarketBranch& From = State.Branches[FromBranch];
    FMarketManager M;
    M.Level = static_cast<uint8>(Level);
    M.Country = C;
    M.Area = ResolveArea(State, Level, C, Area);
    M.Name = From.ManagerName;
    M.Skill = From.ManagerSkill;
    M.Honesty = From.ManagerHonesty;
    M.Style = From.ManagerStyle;
    M.Potential = PotentialOf(From);
    M.Morale = FMath::Min(100.f, FMath::Max(50.f, From.ManagerMorale) + 10.f);
    M.BaseWage = BaseWageFor(Level, M.Skill, M.Country);
    M.AppointedDay = State.Day;
    M.bPromoted = true;
    HireStoreManager(State, FromBranch);
    const FMarketBranch& After = State.Branches[FromBranch];
    const FString BranchNote = FString::Printf(TEXT(" %s: yeni m\u00fcd\u00fcr %s, bir hafta al\u0131\u015facak."), *After.Name, *After.ManagerName);
    State.Management.Managers.Add(M);
    OutMessage = FString::Printf(TEXT("%s art\u0131k %s (ayda %s).%s Sana do\u011frudan ba\u011fl\u0131: %d ki\u015fi."), *M.Name, *TitleOf(M), *ManagerTl(DailyWage(State, M) * 30), *BranchNote, DirectCount(State));
    if (bFirstProvince) MarketStory::AddMemory(State, FString::Printf(TEXT("\u0130lk il m\u00fcd\u00fcr\u00fc: %s (%s)"), *M.Name, *AreaName(Level, M.Country, M.Area)));
    return true;
}

bool MarketManagers::AppointCandidate(FMarketState& State, ELevel Level, const FString& Country, const FString& Area, int32 CandidateIndex, FString& OutMessage)
{
    if (Level == ELevel::Store) { OutMessage = TEXT("Ma\u011faza m\u00fcd\u00fcr\u00fcn\u00fc \u015fubenin kendisinden de\u011fi\u015ftir."); return false; }
    if (!CanAppoint(State, Level, Country, Area, INDEX_NONE, OutMessage)) return false;
    const TArray<FCandidate> Pool = Candidates(State, Level, Country, Area);
    if (!Pool.IsValidIndex(CandidateIndex)) { OutMessage = TEXT("B\u00f6yle bir aday yok."); return false; }
    const bool bFirstProvince = Level == ELevel::Province && !State.Management.Managers.ContainsByPredicate([](const FMarketManager& X) { return X.Level == static_cast<uint8>(ELevel::Province); });
    const FMarketManager M = ManagerFrom(State, Pool[CandidateIndex]);
    MarkSeen(State, Pool);
    ++State.Management.Hires;
    State.Management.Managers.Add(M);
    OutMessage = FString::Printf(TEXT("%s art\u0131k %s (ayda %s). Sana do\u011frudan ba\u011fl\u0131: %d ki\u015fi."), *M.Name, *TitleOf(M), *ManagerTl(DailyWage(State, M) * 30), DirectCount(State));
    if (bFirstProvince) MarketStory::AddMemory(State, FString::Printf(TEXT("\u0130lk il m\u00fcd\u00fcr\u00fc: %s (%s)"), *M.Name, *AreaName(Level, M.Country, M.Area)));
    return true;
}

bool MarketManagers::Dismiss(FMarketState& State, int32 ManagerIndex, FString& OutMessage)
{
    if (!State.Management.Managers.IsValidIndex(ManagerIndex)) { OutMessage = TEXT("B\u00f6yle bir y\u00f6netici yok."); return false; }
    const FMarketManager Leaving = State.Management.Managers[ManagerIndex];
    // C3 (B3): notice pay and seniority pay after the first full year.
    const int64 Severance = DailyWage(State, Leaving) * SeveranceDays + MarketStaff::SeniorityPay(DailyWage(State, Leaving), Leaving.AppointedDay, State.Day);
    if (State.Cash < Severance + State.OtherCosts) { OutMessage = FString::Printf(TEXT("Tazminat i\u00e7in kasada %s gerekiyor."), *ManagerTl(Severance)); return false; }
    State.OtherCosts += Severance;
    State.Management.UsedNames.AddUnique(Leaving.Name); // M22: he does not come back as a candidate
    State.Management.Managers.RemoveAt(ManagerIndex);
    const int32 Boss = BossOfManager(State, Leaving);
    OutMessage = FString::Printf(TEXT("%s g\u00f6revden al\u0131nd\u0131 (%s), tazminat %s. Alt\u0131ndakiler art\u0131k %s ba\u011fl\u0131. Sana do\u011frudan ba\u011fl\u0131: %d ki\u015fi."),
        *Leaving.Name, *TitleOf(Leaving), *ManagerTl(Severance), *BossText(State, Boss), DirectCount(State));
    return true;
}

bool MarketManagers::BonusManager(FMarketState& State, int32 ManagerIndex, FString& OutMessage)
{
    if (!State.Management.Managers.IsValidIndex(ManagerIndex)) { OutMessage = TEXT("B\u00f6yle bir y\u00f6netici yok."); return false; }
    FMarketManager& M = State.Management.Managers[ManagerIndex];
    if (M.BonusDay > 0 && State.Day - M.BonusDay < BonusCooldown) { OutMessage = FString::Printf(TEXT("%s yak\u0131n zamanda prim ald\u0131."), *M.Name); return false; }
    const int64 Cost = DailyWage(State, M) * BonusDays;
    if (State.Cash < Cost + State.OtherCosts) { OutMessage = FString::Printf(TEXT("Prim i\u00e7in kasada %s gerekiyor."), *ManagerTl(Cost)); return false; }
    State.OtherCosts += Cost;
    M.Morale = FMath::Min(100.f, M.Morale + 15.f);
    if (M.Skill < FMath::Min(SkillTop, PotentialOf(M))) ++M.Skill; // M21: never past the ceiling
    M.BonusDay = State.Day;
    OutMessage = FString::Printf(TEXT("%s (%s) prim ald\u0131: %s. Morali y\u00fckseldi."), *M.Name, *TitleOf(M), *ManagerTl(Cost));
    return true;
}

bool MarketManagers::WarnManager(FMarketState& State, int32 ManagerIndex, FString& OutMessage)
{
    if (!State.Management.Managers.IsValidIndex(ManagerIndex)) { OutMessage = TEXT("B\u00f6yle bir y\u00f6netici yok."); return false; }
    FMarketManager& M = State.Management.Managers[ManagerIndex];
    if (M.WarnedDay > 0 && State.Day - M.WarnedDay < 7) { OutMessage = FString::Printf(TEXT("%s bu hafta zaten uyar\u0131ld\u0131."), *M.Name); return false; }
    const bool bDeserved = Strength(State, ManagerIndex) < 0.7f || M.Honesty < 35;
    ++M.Warnings;
    M.WarnedDay = State.Day;
    M.Morale = FMath::Max(0.f, M.Morale - (bDeserved ? 4.f : 12.f));
    OutMessage = bDeserved ? FString::Printf(TEXT("%s uyar\u0131ld\u0131; i\u015fine daha s\u0131k\u0131 bakacak."), *M.Name)
                           : FString::Printf(TEXT("%s uyar\u0131y\u0131 haks\u0131z buldu; morali d\u00fc\u015ft\u00fc."), *M.Name);
    return true;
}

bool MarketManagers::Bonus(FMarketState& State, int32 BranchIndex, FString& OutMessage)
{
    if (!State.Branches.IsValidIndex(BranchIndex) || !HasManager(State.Branches[BranchIndex])) { OutMessage = TEXT("Bu \u015fubenin m\u00fcd\u00fcr\u00fc yok."); return false; }
    FMarketBranch& B = State.Branches[BranchIndex];
    if (B.ManagerBonusDay > 0 && State.Day - B.ManagerBonusDay < BonusCooldown) { OutMessage = FString::Printf(TEXT("%s yak\u0131n zamanda prim ald\u0131."), *B.ManagerName); return false; }
    const int64 Cost = B.ManagerWage * BonusDays;
    if (State.Cash < Cost + State.OtherCosts) { OutMessage = FString::Printf(TEXT("Prim i\u00e7in kasada %s gerekiyor."), *ManagerTl(Cost)); return false; }
    State.OtherCosts += Cost;
    B.ManagerMorale = FMath::Min(100.f, FMath::Max(0.f, B.ManagerMorale) + 15.f);
    if (B.ManagerSkill < FMath::Min(SkillTop, PotentialOf(B))) ++B.ManagerSkill; // M21: never past the ceiling
    B.ManagerBonusDay = State.Day;
    OutMessage = FString::Printf(TEXT("%s (%s) prim ald\u0131: %s. Morali y\u00fckseldi."), *B.ManagerName, *B.Name, *ManagerTl(Cost));
    return true;
}

bool MarketManagers::Warn(FMarketState& State, int32 BranchIndex, FString& OutMessage)
{
    if (!State.Branches.IsValidIndex(BranchIndex) || !HasManager(State.Branches[BranchIndex])) { OutMessage = TEXT("Bu \u015fubenin m\u00fcd\u00fcr\u00fc yok."); return false; }
    FMarketBranch& B = State.Branches[BranchIndex];
    if (B.ManagerWarnedDay > 0 && State.Day - B.ManagerWarnedDay < 7) { OutMessage = FString::Printf(TEXT("%s bu hafta zaten uyar\u0131ld\u0131."), *B.ManagerName); return false; }
    const FString Mark = MarketBranches::Grade(State, BranchIndex);
    const bool bDishonest = B.ManagerHonesty < 35;
    const bool bDeserved = bDishonest || B.ManagerBadWeeks > 0 || Mark == TEXT("C") || Mark == TEXT("D");
    ++B.ManagerWarnings;
    B.ManagerWarnedDay = State.Day;
    const float Base = FMath::Max(0.f, B.ManagerMorale);
    if (bDishonest) B.ManagerMorale = FMath::Max(0.f, Base - 3.f); // keeps his hands off the till for a while
    else if (bDeserved) { B.ManagerMorale = FMath::Max(0.f, Base - 4.f); B.ManagerBadWeeks = FMath::Max(0, B.ManagerBadWeeks - 1); }
    else B.ManagerMorale = FMath::Max(0.f, Base - 12.f);
    OutMessage = bDeserved ? FString::Printf(TEXT("%s (%s) uyar\u0131ld\u0131; toparlanmaya \u00e7al\u0131\u015facak."), *B.ManagerName, *B.Name)
                           : FString::Printf(TEXT("%s (%s) uyar\u0131y\u0131 haks\u0131z buldu; morali d\u00fc\u015ft\u00fc."), *B.ManagerName, *B.Name);
    return true;
}

bool MarketManagers::Replace(FMarketState& State, int32 BranchIndex, FString& OutMessage)
{
    return ReplaceWithCandidate(State, BranchIndex, 0, OutMessage); // M22: the pool's first
}

bool MarketManagers::ReplaceWithCandidate(FMarketState& State, int32 BranchIndex, int32 CandidateIndex, FString& OutMessage)
{
    if (!State.Branches.IsValidIndex(BranchIndex) || !IsLiveBranch(State.Branches[BranchIndex])) { OutMessage = TEXT("B\u00f6yle bir \u015fube yok."); return false; }
    const FMarketBranch Before = State.Branches[BranchIndex];
    if (Before.ManagerName.IsEmpty() && !IsOpenBranch(Before)) { OutMessage = TEXT("M\u00fcd\u00fcr i\u015fe al\u0131m a\u015famas\u0131nda gelir."); return false; }
    const TArray<FCandidate> Pool = BranchCandidates(State, BranchIndex);
    if (!Pool.IsValidIndex(CandidateIndex)) { OutMessage = TEXT("B\u00f6yle bir aday yok."); return false; }
    const int64 Severance = Before.ManagerName.IsEmpty() ? 0 : Before.ManagerWage * SeveranceDays + MarketStaff::SeniorityPay(Before.ManagerWage, Before.ManagerSince, State.Day); // C3 (B3)
    if (Severance > 0 && State.Cash < Severance + State.OtherCosts) { OutMessage = FString::Printf(TEXT("Tazminat i\u00e7in kasada %s gerekiyor."), *ManagerTl(Severance)); return false; }
    State.OtherCosts += Severance;
    if (!Before.ManagerName.IsEmpty()) State.Management.UsedNames.AddUnique(Before.ManagerName);
    HireFrom(State, BranchIndex, Pool[CandidateIndex]);
    MarkSeen(State, Pool);
    const FMarketBranch& After = State.Branches[BranchIndex];
    const FString Skill = SkillVisible(State, BranchIndex) ? FString::Printf(TEXT(" (beceri %d)"), After.ManagerSkill) : FString();
    OutMessage = Before.ManagerName.IsEmpty()
        ? FString::Printf(TEXT("%s: yeni m\u00fcd\u00fcr %s%s, bir hafta al\u0131\u015facak."), *After.Name, *After.ManagerName, *Skill)
        : FString::Printf(TEXT("%s ayr\u0131ld\u0131 (tazminat %s). %s: yeni m\u00fcd\u00fcr %s%s, bir hafta al\u0131\u015facak."), *Before.ManagerName, *ManagerTl(Severance), *After.Name, *After.ManagerName, *Skill);
    return true;
}

bool MarketManagers::HireCandidateForBranch(FMarketState& State, int32 BranchIndex, int32 CandidateIndex, FString& OutMessage)
{
    if (!State.Branches.IsValidIndex(BranchIndex) || !IsLiveBranch(State.Branches[BranchIndex])) { OutMessage = TEXT("B\u00f6yle bir \u015fube yok."); return false; }
    const FMarketBranch& B = State.Branches[BranchIndex];
    if (!B.ManagerName.IsEmpty()) { OutMessage = FString::Printf(TEXT("%s \u015fubesinin m\u00fcd\u00fcr\u00fc var (%s); yenisi i\u00e7in de\u011fi\u015ftir."), *B.Name, *B.ManagerName); return false; }
    return ReplaceWithCandidate(State, BranchIndex, CandidateIndex, OutMessage);
}

void MarketManagers::HireStoreManager(FMarketState& State, int32 BranchIndex)
{
    if (!State.Branches.IsValidIndex(BranchIndex)) return;
    const TArray<FCandidate> Pool = BranchCandidates(State, BranchIndex);
    if (Pool.Num() > 0) HireFrom(State, BranchIndex, Pool[0]);
}

bool MarketManagers::PromoteToProvince(FMarketState& State, int32 BranchIndex, FString& OutMessage)
{
    if (!State.Branches.IsValidIndex(BranchIndex)) { OutMessage = TEXT("B\u00f6yle bir \u015fube yok."); return false; }
    return Appoint(State, ELevel::Province, MarketBranches::CountryOf(State, State.Branches[BranchIndex]), AreaOfBranch(State, BranchIndex, ELevel::Province), BranchIndex, OutMessage);
}

void MarketManagers::InitStoreManager(const FMarketState& State, FMarketBranch& Branch, int32 BranchIndex)
{
    const uint32 Roll = ManagerMix(State.RivalSeed, State.Day, 0x5717u + static_cast<uint32>(BranchIndex) * 13u + GetTypeHash(Branch.ManagerName));
    Branch.ManagerStyle = static_cast<uint8>(1u + Roll % 3u);
    Branch.ManagerPotential = DerivedPotential(Branch.ManagerSkill, Roll >> 8); // a candidate brings his own (HireFrom)
    Branch.ManagerMorale = 70.f;
    Branch.ManagerWarnings = 0;
    Branch.ManagerSince = FMath::Max(1, State.Day);
    Branch.ManagerBadWeeks = 0;
    Branch.ManagerGoodWeeks = 0;
    Branch.ManagerBonusDay = 0;
    Branch.ManagerWarnedDay = 0;
    Branch.ManagerCaughtDay = 0;
}

FString MarketManagers::DescribeBranchManager(const FMarketState& State, int32 BranchIndex)
{
    if (!State.Branches.IsValidIndex(BranchIndex)) return FString();
    const FMarketBranch& B = State.Branches[BranchIndex];
    if (B.ManagerName.IsEmpty()) return FString::Printf(TEXT("%s: m\u00fcd\u00fcr yok (senin talimatlar\u0131n)"), *B.Name);
    TArray<FString> Parts;
    Parts.Add(FString::Printf(TEXT("%s \u00b7 %s m\u00fcd\u00fcr\u00fc"), *B.ManagerName, *B.Name));
    const bool bVisible = SkillVisible(State, BranchIndex);
    if (bVisible) Parts.Add(FString::Printf(TEXT("beceri %d (etkin %d), d\u00fcr\u00fcstl\u00fck %d"), B.ManagerSkill, EffectiveSkill(State, BranchIndex), B.ManagerHonesty));
    else Parts.Add(TEXT("beceri ? (\u0130K m\u00fcd\u00fcr\u00fc ya da il m\u00fcd\u00fcr\u00fc g\u00f6r\u00fcr)"));
    if (bVisible || B.ManagerSince == 0 || State.Day - B.ManagerSince >= 28) Parts.Add(FString::Printf(TEXT("tarz %s"), *StyleName(B.ManagerStyle)));
    Parts.Add(FString::Printf(TEXT("moral %.0f"), FMath::Max(0.f, B.ManagerMorale)));
    if (B.ManagerWarnings > 0) Parts.Add(FString::Printf(TEXT("%d uyar\u0131"), B.ManagerWarnings));
    if (B.ManagerSince > 0 && State.Day - B.ManagerSince < SettleDays) Parts.Add(FString::Printf(TEXT("al\u0131\u015f\u0131yor (%d g\u00fcn)"), SettleDays - (State.Day - B.ManagerSince)));
    if (B.ManagerBadWeeks > 0) Parts.Add(FString::Printf(TEXT("%d hafta k\u00f6t\u00fc karne"), B.ManagerBadWeeks));
    if (B.ManagerCaughtDay > 0) Parts.Add(TEXT("KASADAN ALIYOR"));
    Parts.Add(FString::Printf(TEXT("ba\u011fl\u0131: %s"), *BossText(State, BossIn(State, 0, ChainOfBranch(State, B)))));
    return FString::Join(Parts, TEXT(" \u00b7 "));
}

FString MarketManagers::DescribeManager(const FMarketState& State, int32 ManagerIndex)
{
    if (!State.Management.Managers.IsValidIndex(ManagerIndex)) return FString();
    const FMarketManager& M = State.Management.Managers[ManagerIndex];
    TArray<FString> Parts;
    Parts.Add(FString::Printf(TEXT("%s \u00b7 %s"), *M.Name, *TitleOf(M)));
    Parts.Add(FString::Printf(TEXT("beceri %d (etkin %d, gereken %d)"), M.Skill, EffectiveManagerSkill(State, ManagerIndex), RequiredSkill(State, ManagerIndex)));
    if (M.Level != static_cast<uint8>(ELevel::FamilyShop)) Parts.Add(FString::Printf(TEXT("denetim %%%.0f"), 100.f * Strength(State, ManagerIndex)));
    if (MarketStaff::HasHr(State)) Parts.Add(FString::Printf(TEXT("d\u00fcr\u00fcstl\u00fck %d"), M.Honesty));
    Parts.Add(FString::Printf(TEXT("ayda %s"), *ManagerTl(DailyWage(State, M) * 30)));
    Parts.Add(FString::Printf(TEXT("moral %.0f"), M.Morale));
    if (M.Warnings > 0) Parts.Add(FString::Printf(TEXT("%d uyar\u0131"), M.Warnings));
    Parts.Add(FString::Printf(TEXT("%d. g\u00fcn"), FMath::Max(0, State.Day - M.AppointedDay)));
    Parts.Add(FString::Printf(TEXT("ba\u011fl\u0131: %s"), *BossText(State, BossOfManager(State, M))));
    return FString::Join(Parts, TEXT(" \u00b7 "));
}

FString MarketManagers::Describe(const FMarketState& State, const FPerson& Person)
{
    return Person.Level == ELevel::Store ? DescribeBranchManager(State, Person.Branch) : DescribeManager(State, Person.Manager);
}

FString MarketManagers::SpanText(const FMarketState& State)
{
    const int32 Direct = DirectCount(State);
    FString Line = FString::Printf(TEXT("Sana do\u011frudan %d ki\u015fi ba\u011fl\u0131 (s\u0131n\u0131r %d)."), Direct, SpanLimit);
    const int32 Over = FMath::Max(0, Direct - SpanLimit);
    if (Over > 0)
        Line += FString::Printf(TEXT(" %d ki\u015fi fazla: do\u011frudan ba\u011fl\u0131lar\u0131n becerisi -%d, kasadan \u00e7alan g\u00f6r\u00fcnmez, memnuniyet yava\u015f\u00e7a d\u00fc\u015fer."), Over, FMath::Min(SpanPenaltyMax, Over * SpanPenaltyPerPerson));
    return Line;
}

TArray<FString> MarketManagers::Suggestions(const FMarketState& State)
{
    TArray<FString> Urgent, Later;
    for (const FString& C : CountriesMissingManager(State))
        Urgent.Add(FString::Printf(TEXT("\u015eirket birden \u00e7ok \u00fclkede: %s i\u00e7in \u00fclke m\u00fcd\u00fcr\u00fc ata (zorunlu)."), *AreaName(ELevel::Country, C, C)));
    const int32 Direct = DirectCount(State);
    if (Direct > SpanLimit) Urgent.Add(SpanText(State) + TEXT(" Bir \u00fcst kademe ata."));
    for (const FMarketBranch& B : State.Branches)
    {
        if (!IsOpenBranch(B)) continue;
        if (B.ManagerName.IsEmpty()) Urgent.Add(FString::Printf(TEXT("%s: m\u00fcd\u00fcr yok; yeni m\u00fcd\u00fcr al."), *B.Name));
        else if (B.ManagerCaughtDay > 0) Urgent.Add(FString::Printf(TEXT("%s: m\u00fcd\u00fcr %s kasadan al\u0131yor; de\u011fi\u015ftir."), *B.Name, *B.ManagerName));
        else if (B.ManagerBadWeeks >= 2) Later.Add(FString::Printf(TEXT("%s: %d haftad\u0131r k\u00f6t\u00fc karne; uyar ya da de\u011fi\u015ftir."), *B.Name, B.ManagerBadWeeks));
    }
    // G-089: a depot without a manager works at half its efficiency; a depot manager caught taking goods goes.
    for (int32 D = 0; D < State.Company.DepotSites.Num(); ++D)
    {
        const int32 Keeper = MarketDepots::ManagerOf(State, D);
        const FString& Caught = State.Company.DepotSites[D].CaughtName;
        if (Keeper == INDEX_NONE)
            Urgent.Add(FString::Printf(TEXT("%s: m\u00fcd\u00fcr yok (yar\u0131 verim); depo m\u00fcd\u00fcr\u00fc ata."), *MarketDepots::DepotName(State, D)));
        else if (!Caught.IsEmpty() && Caught == State.Management.Managers[Keeper].Name)
            Urgent.Add(FString::Printf(TEXT("%s: m\u00fcd\u00fcr %s maldan ka\u00e7\u0131r\u0131yor; de\u011fi\u015ftir."), *MarketDepots::DepotName(State, D), *Caught));
    }
    // Levels that can be appointed now.
    TArray<FString> Seen;
    for (const FMarketBranch& B : State.Branches)
    {
        if (!IsOpenBranch(B)) continue;
        const FChain Chain = ChainOfBranch(State, B);
        const FString Key = Chain.Country + TEXT(":") + Chain.Province;
        if (Seen.Contains(Key)) continue;
        Seen.Add(Key);
        const int32 Shops = ProvinceBranches(State, Chain.Country, Chain.Province);
        if (Shops >= ProvinceShops && FindManager(State, ELevel::Province, Chain.Country, Chain.Province) == INDEX_NONE)
            Urgent.Add(FString::Printf(TEXT("%s %d ma\u011faza var: il m\u00fcd\u00fcr\u00fc atayabilirsin."), *Locative(AreaName(ELevel::Province, Chain.Country, Chain.Province)), Shops));
    }
    for (const FMarketManager& M : State.Management.Managers)
    {
        const FChain Chain = ChainOfManager(M);
        if (M.Level == static_cast<uint8>(ELevel::Province) && !Chain.Sub.IsEmpty() && !Seen.Contains(TEXT("sub:") + Chain.Country + Chain.Sub))
        {
            Seen.Add(TEXT("sub:") + Chain.Country + Chain.Sub);
            const int32 Count = ProvinceManagersIn(State, Chain.Country, Chain.Sub);
            if (Count >= SubRegionManagers && FindManager(State, ELevel::SubRegion, Chain.Country, Chain.Sub) == INDEX_NONE)
                Later.Add(FString::Printf(TEXT("%s %d il m\u00fcd\u00fcr\u00fc var: b\u00f6lge m\u00fcd\u00fcr\u00fc atayabilirsin."), *Locative(AreaName(ELevel::SubRegion, Chain.Country, Chain.Sub)), Count));
        }
        if (M.Level == static_cast<uint8>(ELevel::SubRegion) && !Chain.Region.IsEmpty() && !Seen.Contains(TEXT("reg:") + Chain.Country + Chain.Region))
        {
            Seen.Add(TEXT("reg:") + Chain.Country + Chain.Region);
            const int32 Count = SubManagersIn(State, Chain.Country, Chain.Region);
            if (Count >= RegionManagers && FindManager(State, ELevel::Region, Chain.Country, Chain.Region) == INDEX_NONE)
                Later.Add(FString::Printf(TEXT("%s %d b\u00f6lge m\u00fcd\u00fcr\u00fc var: b\u00f6lge direkt\u00f6r\u00fc atayabilirsin."), *Locative(AreaName(ELevel::Region, Chain.Country, Chain.Region)), Count));
        }
    }
    // M20: a country manager can come once 5 provinces have our shops.
    if (!CountryManagerRequired(State))
        for (const FString& C : CountriesWithShops(State))
        {
            const int32 Provinces = ProvincesWithShops(State, C);
            if (Provinces >= CountryProvinces && FindManager(State, ELevel::Country, C, C) == INDEX_NONE)
                Later.Add(FString::Printf(TEXT("%s %d ilde ma\u011fazan var: \u00fclke m\u00fcd\u00fcr\u00fc atayabilirsin."), *Locative(AreaName(ELevel::Country, C, C)), Provinces));
        }
    // M20: the country manager is named only once that level can be seen (5 provinces or two countries).
    if (Direct == SpanLimit)
    {
        const bool bCountryOpen = FindManager(State, ELevel::Country, State.CountryId, State.CountryId) == INDEX_NONE
            && IsTierVisible(State, ELevel::Country, State.CountryId, State.CountryId);
        Later.Add(bCountryOpen
            ? FString::Printf(TEXT("S\u0131n\u0131rdas\u0131n (%d ki\u015fi): yeni ma\u011fazadan \u00f6nce il m\u00fcd\u00fcr\u00fc ya da \u00fclke m\u00fcd\u00fcr\u00fc d\u00fc\u015f\u00fcn."), SpanLimit)
            : FString::Printf(TEXT("S\u0131n\u0131rdas\u0131n (%d ki\u015fi): yeni ma\u011fazadan \u00f6nce bir il m\u00fcd\u00fcr\u00fc d\u00fc\u015f\u00fcn."), SpanLimit));
    }
    Urgent.Append(Later);
    return Urgent;
}

FString MarketManagers::NextStep(const FMarketState& State)
{
    const TArray<FString> List = Suggestions(State);
    return List.Num() > 0 ? List[0] : FString();
}

int32 MarketManagers::EncodeArea(ELevel Level, const FString& Country, const FString& Area)
{
    const TArray<MarketCountry::FProfile>& All = MarketCountry::All();
    const int32 C = All.IndexOfByPredicate([&Country](const MarketCountry::FProfile& P) { return P.Id == Country; });
    if (C == INDEX_NONE || C >= 100 || Level == ELevel::Store) return INDEX_NONE;
    int32 A = 0;
    switch (Level)
    {
    case ELevel::Province:
    case ELevel::Depot: A = All[C].Cities.IndexOfByPredicate([&Area](const MarketCountry::FCity& X) { return X.Id == Area; }); break;
    case ELevel::SubRegion: A = All[C].SubRegions.IndexOfByPredicate([&Area](const MarketCountry::FRegion& X) { return X.Id == Area; }); break;
    case ELevel::Region: A = All[C].Regions.IndexOfByPredicate([&Area](const MarketCountry::FRegion& X) { return X.Id == Area; }); break;
    default: break;
    }
    if (A == INDEX_NONE || A >= 1000) return INDEX_NONE;
    return (static_cast<int32>(Level) * 100 + C) * 1000 + A;
}

bool MarketManagers::DecodeArea(int32 Arg, ELevel& OutLevel, FString& OutCountry, FString& OutArea)
{
    if (Arg < 0) return false;
    const TArray<MarketCountry::FProfile>& All = MarketCountry::All();
    const int32 A = Arg % 1000, C = (Arg / 1000) % 100, L = Arg / 100000;
    if (L < static_cast<int32>(ELevel::Province) || L > static_cast<int32>(ELevel::Depot) || !All.IsValidIndex(C)) return false;
    OutLevel = static_cast<ELevel>(L);
    OutCountry = All[C].Id;
    switch (OutLevel)
    {
    case ELevel::Province:
    case ELevel::Depot: if (!All[C].Cities.IsValidIndex(A)) return false; OutArea = All[C].Cities[A].Id; break;
    case ELevel::SubRegion: if (!All[C].SubRegions.IsValidIndex(A)) return false; OutArea = All[C].SubRegions[A].Id; break;
    case ELevel::Region: if (!All[C].Regions.IsValidIndex(A)) return false; OutArea = All[C].Regions[A].Id; break;
    case ELevel::Country: OutArea = All[C].Id; break;
    default: OutArea.Reset(); break; // family shop: the home province
    }
    return true;
}

void MarketManagers::Migrate(FMarketState& State)
{
    for (int32 I = 0; I < State.Branches.Num(); ++I)
    {
        FMarketBranch& B = State.Branches[I];
        if (B.ManagerName.IsEmpty()) continue;
        const uint32 Roll = ManagerMix(State.RivalSeed, I, 0x0A1Du + GetTypeHash(B.ManagerName));
        if (B.ManagerStyle == 0) B.ManagerStyle = static_cast<uint8>(1u + Roll % 3u);
        if (B.ManagerMorale < 0.f) B.ManagerMorale = 55.f + static_cast<float>((Roll >> 8) % 21u);
        if (B.ManagerPotential <= 0) B.ManagerPotential = PotentialOf(B); // G-086b ek (M21): the ceiling, once
    }
    for (int32 I = 0; I < State.Management.Managers.Num(); ++I)
    {
        FMarketManager& M = State.Management.Managers[I];
        if (M.Style == 0) M.Style = static_cast<uint8>(1u + ManagerMix(State.RivalSeed, I, 0x5791u + GetTypeHash(M.Name)) % 3u);
        if (M.Potential <= 0) M.Potential = PotentialOf(M);
    }
    // M22: everyone who works for us (older saves, promoted employees, the legacy branches) is a used name, so
    // nobody's name comes back after he leaves. Idempotent: only names not listed yet are added.
    TArray<FString>& Used = State.Management.UsedNames;
    TSet<FString> Known;
    for (const FString& Name : Used) Known.Add(Name);
    auto Remember = [&Used, &Known](const FString& Name)
    {
        if (Name.IsEmpty() || Known.Contains(Name)) return;
        Known.Add(Name);
        Used.Add(Name);
    };
    for (const FMarketBranch& B : State.Branches) Remember(B.ManagerName);
    for (const FMarketManager& M : State.Management.Managers) Remember(M.Name);
}

int32 MarketManagers::PotentialOf(const FMarketBranch& Branch)
{
    return Branch.ManagerPotential > 0 ? Branch.ManagerPotential : DerivedPotential(Branch.ManagerSkill, ManagerMix(0, Branch.ManagerSkill, 0x9071u + GetTypeHash(Branch.ManagerName)));
}

int32 MarketManagers::PotentialOf(const FMarketManager& Manager)
{
    return Manager.Potential > 0 ? Manager.Potential : DerivedPotential(Manager.Skill, ManagerMix(0, Manager.Skill, 0x9071u + GetTypeHash(Manager.Name)));
}

float MarketManagers::GrowthChance(int32 Skill, int32 Potential, int32 TenureWeeks)
{
    const int32 Gap = FMath::Min(Potential, SkillTop) - Skill;
    if (Gap <= 0) return 0.f;
    const float Seniority = FMath::Clamp(static_cast<float>(TenureWeeks) / 52.f, 0.f, 1.f);
    return FMath::Min(0.6f, static_cast<float>(Gap) / 40.f * (1.f + 0.5f * Seniority));
}

int32 MarketManagers::GrowSkill(int32 Skill, int32 Potential, int32 TenureWeeks, uint32 Roll, float Pace)
{
    if (Skill >= FMath::Min(Potential, SkillTop)) return Skill;
    const float Chance = GrowthChance(Skill, Potential, TenureWeeks) * Pace;
    return static_cast<float>(Roll % 100000u) < Chance * 100000.f ? Skill + 1 : Skill;
}

MarketManagers::FFamilyRule MarketManagers::FamilyRule(const FMarketState& State)
{
    FFamilyRule Rule;
    const int32 Index = FindManager(State, ELevel::FamilyShop, State.CountryId, FString());
    if (Index == INDEX_NONE) return Rule;
    const FMarketManager& M = State.Management.Managers[Index];
    Rule.bManaged = true;
    int32 Skill = EffectiveManagerSkill(State, Index);
    if (M.AppointedDay > 0 && State.Day - M.AppointedDay < SettleDays) Skill -= SettlePenalty; // learning the family's way
    Rule.Skill = FMath::Clamp(Skill, 0, 100);
    switch (static_cast<EStyle>(M.Style))
    {
    case EStyle::Careful: Rule.OrderFactor = 0.85f; Rule.PriceRiseGap = 0.01; break;   // small stock, protects the margin
    case EStyle::Generous: Rule.OrderFactor = 1.2f; Rule.PriceRiseGap = 0.03; break;   // full shelves
    case EStyle::PriceMinded: Rule.OrderFactor = 1.f; Rule.PriceRiseGap = 0.05; break; // holds the old prices longer
    default: break;
    }
    Rule.RefillEvery = Rule.Skill >= 70 ? 6 : Rule.Skill >= 45 ? 8 : 12;
    Rule.ForgetPermille = Rule.Skill < 45 ? (45 - Rule.Skill) * 8 : 0;
    return Rule;
}

void MarketManagers::ShapeFamilyOrder(const FMarketState& State, const FFamilyRule& Rule, TArray<int32>& Draft)
{
    if (!Rule.bManaged) return;
    for (int32 I = 0; I < Draft.Num(); ++I)
    {
        if (Draft[I] <= 0) continue;
        if (Rule.ForgetPermille > 0 && static_cast<int32>(ManagerMix(State.RivalSeed, State.Day, 0xFA31u + static_cast<uint32>(I) * 7919u) % 1000u) < Rule.ForgetPermille)
        {
            Draft[I] = 0;
            continue;
        }
        Draft[I] = FMath::Max(1, FMath::RoundToInt32(Draft[I] * Rule.OrderFactor));
    }
}

void MarketManagers::CloseDay(FMarketState& State)
{
    Migrate(State);
    const int32 Closed = State.Day - 1;
    if (Closed < 1) return;
    FMarketManagement& Team = State.Management;
    TArray<FString>& News = State.DayNews;

    // Wages of the appointed managers: the same way as the branches' costs (store managers are paid there).
    const int64 Wages = DailyWages(State);
    const int64 Social = MarketStaff::EmployerShare(Wages); // C3 (B3): the employer's share
    State.Cash -= Wages + Social;
    State.LastBranchProfit -= Wages + Social;
    State.LastProfit -= Wages + Social;
    MarketLedger::Post(State, MarketLedger::EAccount::Wages, -Wages, true, MarketLedger::HeadOfficeStore);
    MarketLedger::Post(State, MarketLedger::EAccount::SocialSecurity, -Social, true, MarketLedger::HeadOfficeStore);
    Team.LastWages = Wages;
    Team.WeekWages += Wages;

    const int32 Span = SpanPenalty(State);
    const int32 Over = OverLimit(State);
    const TArray<FString> Missing = CountriesMissingManager(State);
    if (Missing.Num() > 0)
    {
        ++Team.MissingCountryDays;
        if (Team.MissingCountryDays == 1 || Closed % 7 == 0)
        {
            TArray<FString> Names;
            for (const FString& C : Missing) Names.Add(AreaName(ELevel::Country, C, C));
            News.Add(FString::Printf(TEXT("\u015eirket birden \u00e7ok \u00fclkede: her \u00fclkeye bir \u00fclke m\u00fcd\u00fcr\u00fc gerekir (eksik: %s). Atanana kadar oradaki ma\u011fazalar zay\u0131f y\u00f6netilir."), *FString::Join(Names, TEXT(", "))));
        }
    }
    else Team.MissingCountryDays = 0;

    // Store managers: morale drifts to what their situation gives; the player's span and a missing country manager
    // cost satisfaction slowly.
    for (int32 I = 0; I < State.Branches.Num(); ++I)
    {
        FMarketBranch& B = State.Branches[I];
        if (!IsOpenBranch(B) || B.ManagerName.IsEmpty()) continue;
        const FChain Chain = ChainOfBranch(State, B);
        const bool bDirect = BossIn(State, 0, Chain) == INDEX_NONE;
        const int32 Province = FindManager(State, ELevel::Province, Chain.Country, Chain.Province);
        const bool bBadBoss = Province != INDEX_NONE && (StrengthOf(State, Province, Span) < 0.3f || Team.Managers[Province].Honesty < 35);
        const float Aim = 65.f - 4.f * FMath::Min(B.ManagerWarnings, 3) - (bBadBoss ? 10.f : 0.f) - (bDirect && Over > 0 ? 5.f : 0.f);
        if (B.ManagerMorale < 0.f) B.ManagerMorale = 60.f;
        B.ManagerMorale = FMath::Clamp(B.ManagerMorale + (Aim - B.ManagerMorale) * 0.03f, 0.f, 100.f);
        if (bDirect && Over > 0) B.Satisfaction = FMath::Max(0.f, B.Satisfaction - 0.1f * FMath::Min(Over, 5));
        if (Missing.Contains(Chain.Country)) B.Satisfaction = FMath::Max(0.f, B.Satisfaction - 0.2f);
    }
    for (FMarketManager& M : Team.Managers)
    {
        const float Aim = 65.f - 4.f * FMath::Min(M.Warnings, 3);
        M.Morale = FMath::Clamp(M.Morale + (Aim - M.Morale) * 0.03f, 0.f, 100.f);
    }

    if (Closed % 7 != 0) return;

    // Weekly marks: growth, tiredness, leaving; the province manager warns, proposes and catches a skimmer.
    for (int32 I = 0; I < State.Branches.Num(); ++I)
    {
        FMarketBranch& B = State.Branches[I];
        if (!IsOpenBranch(B) || B.ManagerName.IsEmpty()) continue;
        const FString Mark = MarketBranches::Grade(State, I);
        if (Mark == TEXT("-")) continue;
        if (Mark == TEXT("A") || Mark == TEXT("B"))
        {
            ++B.ManagerGoodWeeks;
            B.ManagerBadWeeks = 0;
            B.ManagerMorale = FMath::Min(100.f, B.ManagerMorale + 3.f);
            // M21: a well-run shop teaches, less and less near the manager's ceiling.
            if (B.ManagerMorale >= 50.f)
            {
                const int32 Tenure = B.ManagerSince > 0 ? (State.Day - B.ManagerSince) / 7 : 52;
                B.ManagerSkill = GrowSkill(B.ManagerSkill, PotentialOf(B), Tenure, ManagerMix(State.RivalSeed, Closed, 0x6E0Du + static_cast<uint32>(I) * 131u));
            }
        }
        else if (Mark == TEXT("C")) { B.ManagerGoodWeeks = 0; B.ManagerMorale = FMath::Max(0.f, B.ManagerMorale - 1.f); }
        else { B.ManagerGoodWeeks = 0; ++B.ManagerBadWeeks; B.ManagerMorale = FMath::Max(0.f, B.ManagerMorale - 6.f); }

        const FChain Chain = ChainOfBranch(State, B);
        const int32 Province = FindManager(State, ELevel::Province, Chain.Country, Chain.Province);
        const int32 Watcher = Province != INDEX_NONE ? Province : BossIn(State, 0, Chain);
        const float Watch = OversightOf(State, I, Span);
        if (Mark == TEXT("D") && Province != INDEX_NONE && Watch >= 0.5f)
        {
            const FMarketManager& Boss = Team.Managers[Province];
            if (B.ManagerWarnings == 0)
            {
                B.ManagerWarnings = 1;
                B.ManagerWarnedDay = Closed;
                B.ManagerMorale = FMath::Max(0.f, B.ManagerMorale - 3.f);
                News.Add(FString::Printf(TEXT("%s (%s) %s m\u00fcd\u00fcr\u00fcn\u00fc uyard\u0131."), *Boss.Name, *TitleOf(Boss), *B.Name));
            }
            else if (B.ManagerBadWeeks >= 2)
                News.Add(FString::Printf(TEXT("%s (%s): \"%s m\u00fcd\u00fcr\u00fc %s de\u011fi\u015ftirilmeli.\""), *Boss.Name, *TitleOf(Boss), *B.Name, *B.ManagerName));
        }
        const FBranchRule Rule = RuleFor(State, I, Span);
        if (Rule.SkimPermille > 0 && B.ManagerCaughtDay == 0 && !Rule.bSkimHidden && Watcher != INDEX_NONE && Watch > 0.f)
        {
            const uint32 Roll = ManagerMix(State.RivalSeed, Closed, 0xCA7Cu + static_cast<uint32>(I) * 31u);
            if (static_cast<float>(Roll % 100u) < Watch * 70.f)
            {
                B.ManagerCaughtDay = Closed;
                const FMarketManager& Boss = Team.Managers[Watcher];
                News.Add(FString::Printf(TEXT("%s (%s): %s m\u00fcd\u00fcr\u00fc %s kasadan al\u0131yordu. \u00d6neri: m\u00fcd\u00fcr\u00fc de\u011fi\u015ftir."), *Boss.Name, *TitleOf(Boss), *B.Name, *B.ManagerName));
            }
        }
        if ((B.ManagerBadWeeks >= 3 && B.ManagerMorale < 30.f) || B.ManagerMorale < 12.f)
        {
            News.Add(FString::Printf(TEXT("%s (%s) istifa etti: \u00fcst \u00fcste k\u00f6t\u00fc karne, moral kalmad\u0131. \u015eube senin talimatlar\u0131nla d\u00f6n\u00fcyor; yeni m\u00fcd\u00fcr al."), *B.ManagerName, *B.Name));
            Team.UsedNames.AddUnique(B.ManagerName); // M22: never a candidate again
            ClearStoreManager(B);
        }
    }
    for (int32 I = Team.Managers.Num() - 1; I >= 0; --I)
    {
        FMarketManager& M = Team.Managers[I];
        const int32 Weeks = (State.Day - M.AppointedDay) / 7;
        // M21: a good week (strong, content) may teach, at half a store manager's pace, never past the ceiling.
        if (M.Morale >= 60.f && StrengthOf(State, I, Span) >= 0.7f)
            M.Skill = GrowSkill(M.Skill, PotentialOf(M), Weeks, ManagerMix(State.RivalSeed, Closed, 0x6E1Du + static_cast<uint32>(I) * 137u), 0.5f);
        if (M.Morale < 12.f)
        {
            News.Add(FString::Printf(TEXT("%s (%s) istifa etti; alt\u0131ndakiler bir \u00fcste ba\u011fland\u0131."), *M.Name, *TitleOf(M)));
            Team.UsedNames.AddUnique(M.Name); // M22: never a candidate again
            Team.Managers.RemoveAt(I);
        }
    }
    const int32 Direct = DirectCount(State);
    if (Team.Managers.Num() > 0 || Direct > SpanLimit)
        News.Add(FString::Printf(TEXT("Y\u00f6netim haftas\u0131: %d y\u00f6netici, \u00fccretleri %s. %s"), Team.Managers.Num(), *ManagerTl(Team.WeekWages), *SpanText(State)));
    Team.WeekWages = 0;
}

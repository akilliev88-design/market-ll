#include "MarketResponse.h"
#include "MarketEconomy.h"
#include "MarketBranches.h"
#include "MarketChains.h"
#include "MarketCountry.h"
#include "MarketEras.h"
#include "MarketEvents.h"
#include "MarketLedger.h"
#include "MarketManagers.h"
#include "MarketPrices.h"
#include "MarketStrategy.h"

namespace MarketResponseLocal
{
    using MarketResponse::EKind;

    FString CountryOr(const FMarketState& State, const FString& Country) { return Country.IsEmpty() ? State.CountryId : Country; }
    FString Key(const FString& Country, const FString& Province) { return Country + TEXT("|") + Province; }

    bool Active(const FMarketResponse& R, int32 Day) { return Day >= R.Day && Day <= R.EndDay; }
    // A crisis answer is company-wide; a rival answer works in its province.
    bool Covers(const FMarketResponse& R, const FString& Country, const FString& Province)
    {
        return R.Province.IsEmpty() || (R.Country == Country && R.Province == Province);
    }
    EKind KindOf(const FMarketResponse& R) { return static_cast<EKind>(R.Kind); }

    uint32 Mix(uint32 A, uint32 B)
    {
        uint32 H = 2166136261u;
        const uint32 Parts[2] = { A, B };
        for (uint32 P : Parts) for (int32 Byte = 0; Byte < 4; ++Byte) { H ^= (P >> (Byte * 8)) & 0xFFu; H *= 16777619u; }
        return H;
    }

    // A manager whose style was never seeded still has one (from the name).
    uint8 StyleOf(uint8 Style, const FString& Name)
    {
        return Style >= 1 && Style <= 3 ? Style : static_cast<uint8>(1 + GetTypeHash(Name) % 3);
    }

    int64 DailyPerShop(EKind Kind, int32 Option)
    {
        if (Kind == EKind::War && Option == 1) return MarketResponse::WarDailyPerShopStart;
        if (Kind == EKind::Opening && Option == 0) return MarketResponse::OpeningDailyPerShopStart;
        return 0;
    }

    FString Where(const FMarketResponse& R)
    {
        return R.Province.IsEmpty() ? FString(TEXT("\u015eirket geneli")) : MarketChains::ProvinceName(R.Country, R.Province);
    }

    FString MoveText(const FMarketResponse& R)
    {
        switch (KindOf(R))
        {
        case EKind::War: return FString::Printf(TEXT("%s fiyat sava\u015f\u0131 a\u00e7t\u0131"), *R.Rival);
        case EKind::Opening: return FString::Printf(TEXT("%s yeni ma\u011faza a\u00e7t\u0131"), *R.Rival);
        default: return FString::Printf(TEXT("%s ba\u015flad\u0131"), *R.Rival);
        }
    }

    // "response.<kind>|<country>|<province>|<end day>|<rival>"
    FString CardId(EKind Kind, const FString& Country, const FString& Province, int32 EndDay, const FString& Rival)
    {
        return FString::Printf(TEXT("response.%d|%s|%s|%d|%s"), static_cast<int32>(Kind), *Country, *Province, EndDay, *Rival);
    }

    bool ParseCard(const FString& Id, EKind& OutKind, FString& OutCountry, FString& OutProvince, int32& OutEnd, FString& OutRival)
    {
        if (!Id.StartsWith(TEXT("response."))) return false;
        TArray<FString> Parts;
        Id.RightChop(9).ParseIntoArray(Parts, TEXT("|"), false);
        if (Parts.Num() < 5) return false;
        const int32 K = FCString::Atoi(*Parts[0]);
        if (K < 0 || K >= static_cast<int32>(EKind::Count)) return false;
        OutKind = static_cast<EKind>(K);
        OutCountry = Parts[1];
        OutProvince = Parts[2];
        OutEnd = FCString::Atoi(*Parts[3]);
        OutRival = Parts[4];
        for (int32 I = 5; I < Parts.Num(); ++I) OutRival += TEXT("|") + Parts[I];
        return true;
    }

    bool DeskFull(const FMarketState& State)
    {
        return State.Decisions.ContainsByPredicate([](const FMarketDecision& D) { return D.Id.StartsWith(TEXT("response.")); })
            || State.Decisions.Num() >= MarketEvents::MaxPending;
    }
}

FString MarketResponse::KindName(EKind Kind)
{
    switch (Kind)
    {
    case EKind::War: return TEXT("fiyat sava\u015f\u0131");
    case EKind::Opening: return TEXT("rakip a\u00e7\u0131l\u0131\u015f\u0131");
    default: return TEXT("kriz");
    }
}

FString MarketResponse::OptionName(EKind Kind, int32 Option)
{
    static const TCHAR* Names[3][3] = {
        { TEXT("fiyatla kar\u015f\u0131l\u0131k"), TEXT("hizmet ve kampanya"), TEXT("bekle, fiyat\u0131 koru") },
        { TEXT("a\u00e7\u0131l\u0131\u015f haftas\u0131 kampanyas\u0131"), TEXT("k\u00fc\u00e7\u00fck fiyat indirimi"), TEXT("bekle") },
        { TEXT("kemer s\u0131k"), TEXT("devam et"), TEXT("f\u0131rsat kolla") } };
    const int32 K = FMath::Clamp(static_cast<int32>(Kind), 0, 2);
    return Names[K][FMath::Clamp(Option, 0, 2)];
}

TArray<FString> MarketResponse::Options(const FMarketState& State, EKind Kind)
{
    const FString War = MarketCountry::Money(MarketPrices::Scaled(WarDailyPerShopStart, State.Day));
    const FString Open = MarketCountry::Money(MarketPrices::Scaled(OpeningDailyPerShopStart, State.Day));
    switch (Kind)
    {
    case EKind::War:
        return { FString(TEXT("Fiyatla kar\u015f\u0131l\u0131k ver: oradaki ma\u011fazalar\u0131n raf fiyat\u0131 %4 iner, sava\u015f bitene kadar. Pay korunur, marj d\u00fc\u015fer.")),
            FString::Printf(TEXT("Hizmet ve kampanyayla ayr\u0131\u015f: ma\u011faza ba\u015f\u0131na g\u00fcnde %s, m\u00fc\u015fteri %%6 fazla gelir; fiyat ayn\u0131 kal\u0131r."), *War),
            FString(TEXT("Bekle, fiyat\u0131 koru: masraf yok; sava\u015f s\u00fcresince pay biraz d\u00fc\u015fer, rakip sava\u015f\u0131 kazanm\u0131\u015f say\u0131l\u0131r.")) };
    case EKind::Opening:
        return { FString::Printf(TEXT("A\u00e7\u0131l\u0131\u015f haftas\u0131na kampanya: 30 g\u00fcn ma\u011faza ba\u015f\u0131na g\u00fcnde %s, m\u00fc\u015fteri %%5 fazla."), *Open),
            FString(TEXT("Fiyatlar\u0131 biraz indir: 30 g\u00fcn raf fiyat\u0131 %2 a\u015fa\u011f\u0131; marj biraz d\u00fc\u015fer.")),
            FString(TEXT("Bekle: masraf yok; yeni rakip m\u00fc\u015fterinin bir k\u0131sm\u0131n\u0131 al\u0131r.")) };
    default:
        return { FString(TEXT("Kemer s\u0131k: ma\u011fazalar\u0131n i\u015fletme giderleri %30, sipari\u015fler %10 azal\u0131r; hizmet biraz d\u00fc\u015fer (m\u00fc\u015fteri %2 az).")),
            FString(TEXT("Devam et: hi\u00e7bir \u015feyi de\u011fi\u015ftirme.")),
            FString(TEXT("F\u0131rsat kolla: kriz s\u00fcresince yeni kiralar %15, tadilatlar %10 ucuza gelir; b\u00fct\u00fcn raflarda fiyat %2 a\u015fa\u011f\u0131 (fiyata bakan m\u00fc\u015fteri gelir, marj d\u00fc\u015fer).")) };
    }
}

int32 MarketResponse::HoldOption(EKind Kind)
{
    return Kind == EKind::Crisis ? 1 : 2;
}

MarketResponse::FDecider MarketResponse::DeciderFor(const FMarketState& State, const FString& InCountry, const FString& Province)
{
    using MarketManagers::ELevel;
    using namespace MarketResponseLocal;
    FDecider Out;
    const FString Country = CountryOr(State, InCountry);
    auto Take = [&State, &Out](int32 Index, const FString& Title) -> bool
    {
        if (Index == INDEX_NONE) return false;
        const FMarketManager& M = State.Management.Managers[Index];
        Out.bPlayer = false;
        Out.Name = M.Name;
        Out.Title = Title;
        Out.Style = StyleOf(M.Style, M.Name);
        Out.Skill = M.Skill;
        return true;
    };
    if (Province.IsEmpty())
    {
        // A crisis: the top of the company.
        if (Take(MarketManagers::FindManager(State, ELevel::Chief, State.CountryId, TEXT("merkez")), TEXT("Genel m\u00fcd\u00fcr"))) return Out;
        const FString Continent = MarketCountry::ContinentOf(State.CountryId);
        if (!Continent.IsEmpty() && Take(MarketManagers::FindManager(State, ELevel::Continent, State.CountryId, Continent), TEXT("K\u0131ta direkt\u00f6r\u00fc"))) return Out;
        Take(MarketManagers::FindManager(State, ELevel::Country, State.CountryId, State.CountryId), TEXT("\u00dclke m\u00fcd\u00fcr\u00fc"));
        return Out;
    }
    const FString Place = MarketChains::ProvinceName(Country, Province);
    if (Take(MarketManagers::FindManager(State, ELevel::Province, Country, Province), Place + TEXT(" il m\u00fcd\u00fcr\u00fc"))) return Out;
    if (const MarketCountry::FCity* City = MarketCountry::FindCity(Country, Province))
    {
        if (Take(MarketManagers::FindManager(State, ELevel::SubRegion, Country, City->SubRegion), TEXT("B\u00f6lge m\u00fcd\u00fcr\u00fc"))) return Out;
        if (Take(MarketManagers::FindManager(State, ELevel::Region, Country, City->Region), TEXT("B\u00f6lge direkt\u00f6r\u00fc"))) return Out;
    }
    if (Take(MarketManagers::FindManager(State, ELevel::Country, Country, Country), TEXT("\u00dclke m\u00fcd\u00fcr\u00fc"))) return Out;
    // Nobody above the shops: the manager of our biggest store there.
    int32 Best = INDEX_NONE;
    for (int32 I = 0; I < State.Branches.Num(); ++I)
    {
        const FMarketBranch& B = State.Branches[I];
        if (B.Stage != static_cast<uint8>(MarketBranches::EStage::Open) || B.ManagerName.IsEmpty() || MarketBranches::CountryOf(State, B) != Country || B.Province != Province) continue;
        if (Best == INDEX_NONE || B.Last30Revenue > State.Branches[Best].Last30Revenue) Best = I;
    }
    if (Best != INDEX_NONE)
    {
        const FMarketBranch& B = State.Branches[Best];
        Out.bPlayer = false;
        Out.Name = B.ManagerName;
        Out.Title = B.Name + TEXT(" m\u00fcd\u00fcr\u00fc");
        Out.Style = StyleOf(B.ManagerStyle, B.ManagerName);
        Out.Skill = B.ManagerSkill;
    }
    return Out;
}

int32 MarketResponse::Proposal(EKind Kind, const FDecider& Decider)
{
    if (Decider.bPlayer) return HoldOption(Kind);
    using MarketManagers::EStyle;
    const EStyle Style = static_cast<EStyle>(Decider.Style);
    // Careful waits (or tightens the belt), generous spends on service (or carries on), price-minded cuts prices
    // (or goes for the chances).
    switch (Kind)
    {
    case EKind::War: return Style == EStyle::PriceMinded ? 0 : Style == EStyle::Generous ? 1 : 2;
    case EKind::Opening: return Style == EStyle::PriceMinded ? 1 : Style == EStyle::Generous ? 0 : 2;
    default: return Style == EStyle::Careful ? 0 : Style == EStyle::PriceMinded ? 2 : 1;
    }
}

float MarketResponse::Impact(const FMarketState& State, const FString& InCountry, const FString& Province)
{
    const FString Country = MarketResponseLocal::CountryOr(State, InCountry);
    double All = 0.0, Here = 0.0;
    for (const FMarketBranch& B : State.Branches)
    {
        if (B.Stage != static_cast<uint8>(MarketBranches::EStage::Open)) continue;
        const double R = static_cast<double>(FMath::Max<int64>(0, B.Last30Revenue));
        All += R;
        if (MarketBranches::CountryOf(State, B) == Country && B.Province == Province) Here += R;
    }
    return All > 0.0 ? static_cast<float>(Here / All) : 1.f;
}

void MarketResponse::Answer(FMarketState& State, EKind Kind, const FString& InCountry, const FString& Province, const FString& Rival, int32 Option,
    const FString& Decider, bool bPlayer, bool bProposed, int32 EndDay)
{
    FMarketResponse R;
    R.Kind = static_cast<uint8>(Kind);
    R.Country = MarketResponseLocal::CountryOr(State, InCountry);
    R.Province = Province;
    R.Rival = Rival;
    R.Option = static_cast<uint8>(FMath::Clamp(Option, 0, 2));
    R.Decider = Decider;
    R.bPlayer = bPlayer;
    R.bProposed = bProposed;
    R.Day = State.Day;
    R.EndDay = FMath::Max(State.Day, EndDay);
    FMarketResponses& Book = State.Responses;
    Book.Log.Add(R);
    if (Book.Log.Num() > LogSize) Book.Log.RemoveAt(0, Book.Log.Num() - LogSize);
    ++Book.Answered;
    if (bPlayer) ++Book.ByPlayer;
}

bool MarketResponse::Raise(FMarketState& State, EKind Kind, const FString& InCountry, const FString& Province, const FString& Rival, int32 EndDay)
{
    using namespace MarketResponseLocal;
    const FString Country = CountryOr(State, InCountry);
    if (!Province.IsEmpty() && MarketBranches::ShopsIn(State, Country, Province) <= 0) return false;
    const FDecider Who = DeciderFor(State, Country, Province);
    const int32 Proposed = Proposal(Kind, Who);
    const bool bBig = Kind == EKind::Crisis || Impact(State, Country, Province) >= BigShare;
    const FString WhoText = Who.bPlayer ? FString() : FString::Printf(TEXT("%s %s"), *Who.Title, *Who.Name);
    if ((Who.bPlayer || bBig) && !DeskFull(State))
    {
        // The player's last word: the manager's proposal is the default.
        FMarketDecision D;
        D.Id = CardId(Kind, Country, Province, EndDay, Rival);
        const FString Place = Province.IsEmpty() ? FString(TEXT("Ekonomi")) : MarketChains::ProvinceName(Country, Province);
        FString Move;
        if (Kind == EKind::War) Move = FString::Printf(TEXT("%s, %s'da fiyatlar\u0131n\u0131 %%8 k\u0131rd\u0131: hedefi biziz (sava\u015f %d. g\u00fcne kadar)."), *Rival, *Place, EndDay);
        else if (Kind == EKind::Opening) Move = FString::Printf(TEXT("%s ba\u015fta olmak \u00fczere rakipler %s'da yeni ma\u011fazalar a\u00e7t\u0131."), *Rival, *Place);
        else Move = FString::Printf(TEXT("%s ba\u015flad\u0131; b\u00fct\u00fcn ma\u011fazalar\u0131 etkiler."), *Rival);
        D.Title = Province.IsEmpty() ? FString::Printf(TEXT("Kriz: %s"), *Rival) : FString::Printf(TEXT("%s: %s"), *Place, *KindName(Kind));
        FString Text = Move;
        if (!Province.IsEmpty())
            Text += FString::Printf(TEXT(" Orada %d ma\u011fazam\u0131z var (cironun %%%.0f'i)."), MarketBranches::ShopsIn(State, Country, Province), Impact(State, Country, Province) * 100.f);
        Text += Who.bPlayer ? FString(TEXT(" Bu konuda karar verecek bir y\u00f6neticin yok: karar senin."))
            : FString::Printf(TEXT(" %s \u00f6neriyor: %s. Son onay senin."), *WhoText, *OptionName(Kind, Proposed));
        D.Text = Text;
        D.Options = Options(State, Kind);
        D.DefaultOption = Proposed;
        D.Deadline = State.Day + DecisionDays;
        MarketEvents::Offer(State, D);
        return true;
    }
    if (Who.bPlayer)
    {
        // Nobody to answer and the desk is full: the player's standing order.
        Answer(State, Kind, Country, Province, Rival, HoldOption(Kind), TEXT("kendi talimat\u0131n (masan doluydu)"), true, false, EndDay);
        return true;
    }
    Answer(State, Kind, Country, Province, Rival, Proposed, WhoText, false, false, EndDay);
    return true;
}

bool MarketResponse::Resolve(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& Decision, int32 Option, FString& OutMessage)
{
    using namespace MarketResponseLocal;
    EKind Kind = EKind::War;
    FString Country, Province, Rival;
    int32 EndDay = 0;
    if (!ParseCard(Decision.Id, Kind, Country, Province, EndDay, Rival)) { OutMessage = TEXT("Bu karar art\u0131k ge\u00e7erli de\u011fil."); return true; }
    const FDecider Who = DeciderFor(State, Country, Province);
    if (State.Day > Decision.Deadline)
    {
        // Nobody answered in time: the proposal (or the player's standing order) stands.
        Answer(State, Kind, Country, Province, Rival, Decision.DefaultOption, Who.bPlayer ? FString(TEXT("kendi talimat\u0131n (s\u00fcre doldu)"))
            : FString::Printf(TEXT("%s %s (s\u00fcre doldu, \u00f6nerisi uyguland\u0131)"), *Who.Title, *Who.Name), Who.bPlayer, false, EndDay);
        OutMessage = FString::Printf(TEXT("cevap verilmedi, %s uyguland\u0131."), *OptionName(Kind, Decision.DefaultOption));
        return true;
    }
    const bool bAsProposed = !Who.bPlayer && Option == Decision.DefaultOption;
    const FString Text = Who.bPlayer ? FString(TEXT("sen"))
        : bAsProposed ? FString::Printf(TEXT("%s %s \u00f6nerdi, sen onaylad\u0131n"), *Who.Title, *Who.Name)
        : FString::Printf(TEXT("sen (%s %s'\u0131n \u00f6nerisi: %s)"), *Who.Title, *Who.Name, *OptionName(Kind, Decision.DefaultOption));
    Answer(State, Kind, Country, Province, Rival, Option, Text, true, !Who.bPlayer, EndDay);
    OutMessage = FString::Printf(TEXT("%s: %s. Ma\u011fazalar \u203a M\u00fcdahaleler'de izleyebilirsin."), Province.IsEmpty() ? *Rival : *MarketChains::ProvinceName(Country, Province), *OptionName(Kind, Option));
    return true;
}

float MarketResponse::PullFactor(const FMarketState& State, const FString& InCountry, const FString& Province, int32 Day)
{
    using namespace MarketResponseLocal;
    if (State.Responses.Log.Num() == 0) return 1.f;
    const FString Country = CountryOr(State, InCountry);
    float F = 1.f;
    for (const FMarketResponse& R : State.Responses.Log)
    {
        if (!Active(R, Day) || !Covers(R, Country, Province)) continue;
        const EKind Kind = KindOf(R);
        if (Kind == EKind::War && R.Option == 1) F *= WarPull;
        else if (Kind == EKind::Opening && R.Option == 0) F *= OpeningPull;
        else if (Kind == EKind::Crisis && R.Option == 0) F *= CutPull;
    }
    return F;
}

float MarketResponse::PriceFactor(const FMarketState& State, const FString& InCountry, const FString& Province, int32 Day)
{
    using namespace MarketResponseLocal;
    if (State.Responses.Log.Num() == 0) return 1.f;
    const FString Country = CountryOr(State, InCountry);
    float F = 1.f;
    for (const FMarketResponse& R : State.Responses.Log)
    {
        if (!Active(R, Day) || !Covers(R, Country, Province)) continue;
        const EKind Kind = KindOf(R);
        if (Kind == EKind::War && R.Option == 0) F *= WarPrice;
        else if (Kind == EKind::Opening && R.Option == 1) F *= OpeningPrice;
        else if (Kind == EKind::Crisis && R.Option == 2) F *= ChancePrice;
    }
    return FMath::Max(0.85f, F);
}

namespace MarketResponseLocal
{
    bool CrisisAnswer(const FMarketState& State, int32 Day, int32 Option)
    {
        for (const FMarketResponse& R : State.Responses.Log)
            if (KindOf(R) == EKind::Crisis && R.Option == Option && Active(R, Day)) return true;
        return false;
    }
}

float MarketResponse::RunningFactor(const FMarketState& State, int32 Day) { return MarketResponseLocal::CrisisAnswer(State, Day, 0) ? CutRunning : 1.f; }
float MarketResponse::OrderFactor(const FMarketState& State, int32 Day) { return MarketResponseLocal::CrisisAnswer(State, Day, 0) ? CutOrder : 1.f; }
float MarketResponse::LeaseFactor(const FMarketState& State) { return MarketResponseLocal::CrisisAnswer(State, State.Day, 2) ? ChanceLease : 1.f; }
float MarketResponse::FitOutFactor(const FMarketState& State) { return MarketResponseLocal::CrisisAnswer(State, State.Day, 2) ? ChanceFitOut : 1.f; }

bool MarketResponse::Fought(const FMarketState& State, const FString& InCountry, const FString& Province, int32 WarUntil)
{
    const FString Country = MarketResponseLocal::CountryOr(State, InCountry);
    for (const FMarketResponse& R : State.Responses.Log)
        if (R.Kind == static_cast<uint8>(EKind::War) && R.Option <= 1 && R.Country == Country && R.Province == Province && R.EndDay == WarUntil) return true;
    return false;
}

int32 MarketResponse::LogCount(const FMarketState& State)
{
    return State.Responses.Log.Num();
}

bool MarketResponse::LogActive(const FMarketState& State, int32 Index)
{
    const int32 I = State.Responses.Log.Num() - 1 - Index;
    return State.Responses.Log.IsValidIndex(I) && MarketResponseLocal::Active(State.Responses.Log[I], State.Day);
}

FString MarketResponse::LogLine(const FMarketState& State, int32 Index)
{
    using namespace MarketResponseLocal;
    const int32 I = State.Responses.Log.Num() - 1 - Index;
    if (!State.Responses.Log.IsValidIndex(I)) return FString();
    const FMarketResponse& R = State.Responses.Log[I];
    FString Line = FString::Printf(TEXT("%d. g\u00fcn \u00b7 %s \u00b7 %s \u2192 %s \u00b7 karar: %s"), R.Day, *Where(R), *MoveText(R), *OptionName(KindOf(R), R.Option), *R.Decider);
    if (R.Spent > 0) Line += FString::Printf(TEXT(" \u00b7 harcanan %s"), *MarketCountry::Money(R.Spent));
    Line += Active(R, State.Day) ? FString::Printf(TEXT(" \u00b7 %d. g\u00fcne kadar s\u00fcr\u00fcyor"), R.EndDay) : FString(TEXT(" \u00b7 bitti"));
    return Line;
}

FString MarketResponse::Summary(const FMarketState& State)
{
    const FMarketResponses& L = State.Responses;
    if (L.Answered == 0) return TEXT("Hen\u00fcz bir rakip hamlesine ya da krize cevap verilmedi. Fiyat sava\u015f\u0131, yeni rakip ma\u011fazas\u0131 ya da kriz olunca ilgili y\u00f6netici karar verir; ciroda b\u00fcy\u00fck pay\u0131 olan illerde ve krizlerde son onay sana gelir.");
    int32 Active = 0;
    int64 Spent = 0;
    for (const FMarketResponse& R : L.Log) { if (MarketResponseLocal::Active(R, State.Day)) ++Active; Spent += R.Spent; }
    return FString::Printf(TEXT("%d m\u00fcdahale (%d'i senin karar\u0131n ya da onay\u0131n, kalan\u0131 y\u00f6neticilerin) \u00b7 \u015fu an %d tanesi s\u00fcr\u00fcyor \u00b7 listedeki cevaplar\u0131n masraf\u0131 %s. Ciroda %%%.0f ve \u00fczeri pay\u0131 olan illerde ve krizlerde son onay sana gelir."),
        L.Answered, L.ByPlayer, Active, *MarketCountry::Money(Spent), BigShare * 100.f);
}

void MarketResponse::CloseDay(FMarketState& State)
{
    using namespace MarketResponseLocal;
    if (State.Day <= 1) return;
    const int32 Day = State.Day;

    // 1. The running answers' costs (service, an opening's campaign): every shop of ours there, a day.
    for (FMarketResponse& R : State.Responses.Log)
    {
        if (!Active(R, Day - 1) || R.Province.IsEmpty()) continue;
        const int64 PerShop = DailyPerShop(KindOf(R), R.Option);
        if (PerShop <= 0) continue;
        const int64 Cost = MarketBranches::InHome(MarketBranches::MoneyOf(State, R.Country, Day), MarketPrices::Scaled(PerShop, Day)) * MarketBranches::ShopsIn(State, R.Country, R.Province);
        if (Cost <= 0) continue;
        State.Cash -= Cost;
        State.LastProfit -= Cost;
        R.Spent += Cost;
        MarketLedger::Post(State, MarketLedger::EAccount::Marketing, -Cost, true, MarketLedger::HeadOfficeStore);
    }

    // 2. Price wars against us (a chain's war that was not answered yet).
    for (const FMarketChain& C : State.Rivals.Chains)
    {
        if (C.bGone || C.WarProvince.IsEmpty() || Day > C.WarUntil) continue;
        const FString Key = C.Id + TEXT("|") + C.WarProvince;
        const int32* Seen = State.Responses.WarSeen.Find(Key);
        if (Seen && *Seen == C.WarUntil) continue;
        State.Responses.WarSeen.Add(Key, C.WarUntil);
        Raise(State, EKind::War, C.Country, C.WarProvince, C.Name, C.WarUntil);
    }

    // 3. Rival openings where we are (weekly).
    if (Day % LookEvery == 0)
    {
        for (const FString& Place : MarketStrategy::OurProvinces(State, 100000))
        {
            FString Country, Province;
            if (!Place.Split(TEXT("|"), &Country, &Province)) continue;
            const int32 Now = MarketChains::StoresIn(State, Country, Province);
            const int32* Seen = State.Responses.RivalSeen.Find(Place);
            const int32 Before = Seen ? *Seen : Now;
            State.Responses.RivalSeen.Add(Place, Now);
            if (Now - Before < FMath::Max(1, FMath::RoundToInt32(Before * OpeningGrowth))) continue;
            // One answer at a time in a province.
            const bool bRunning = State.Responses.Log.ContainsByPredicate([&Country, &Province, Day](const FMarketResponse& R)
                { return R.Kind == static_cast<uint8>(EKind::Opening) && R.Country == Country && R.Province == Province && Active(R, Day); });
            if (bRunning) continue;
            const TArray<int32> Rivals = MarketChains::RivalsIn(State, Country, Province, 1);
            const FString Who = Rivals.Num() > 0 ? State.Rivals.Chains[Rivals[0]].Name : FString(TEXT("Rakipler"));
            Raise(State, EKind::Opening, Country, Province, Who, Day + OpeningDays);
        }
    }

    // 4. A crisis of the campaign's economy.
    MarketEras::FEra Era;
    if (MarketEras::Current(State, Day, Era) && Era.Kind != MarketEras::EKind::Recovery)
    {
        const int32 Code = static_cast<int32>(Era.Kind) * 10 + Era.Wave;
        if (Code != State.Responses.EraSeen)
        {
            State.Responses.EraSeen = Code;
            const int32 End = Era.Kind == MarketEras::EKind::Pandemic || Era.EndDay < Day ? Day + CrisisMaxDays : FMath::Min(Era.EndDay, Day + CrisisMaxDays);
            Raise(State, EKind::Crisis, State.CountryId, FString(), MarketEras::Name(Era.Kind), End);
        }
    }
}

#include "MarketStory.h"
#include "MarketGoals.h"
#include "MarketDepots.h"
#include "MarketStart.h"
#include "MarketCountry.h"
#include "MarketCalendar.h"
#include "MarketCampaign.h"
#include "MarketEvents.h"
#include "MarketGoods.h"
#include "MarketStaff.h"
#include "MarketCompany.h"
#include "MarketBranches.h"

namespace MarketStory
{
    enum EBeat : int64
    {
        // M57: bits 0-3, 5 and 13 were the removed character scenes (welcome, apprentice, salesman, neighbour, the
        // neighbour's offer); they stay unused.
        BIdentityOffered = 1ll << 4,
        BIdentity = 1ll << 6, BFirstOrder = 1ll << 7, BFirstProfit = 1ll << 8, BDebt = 1ll << 9, BShare35 = 1ll << 10,
        BFirstEmployee = 1ll << 11, BSecondStore = 1ll << 12, BFirstWeek = 1ll << 14,
    };

    FString StoryTl(int64 Kurus)
    {
        return MarketCountry::Money(Kurus); // G-084: the active country\'s currency
    }

    bool Has(const FMarketState& State, int64 Beat) { return (State.Story.Beats & Beat) != 0; }
    void Mark(FMarketState& State, int64 Beat) { State.Story.Beats |= Beat; }

    int32 Regulars(const FMarketState& State)
    {
        int32 Count = 0;
        for (const FMarketLoyalty& L : State.Loyalty) if (L.Visits >= 3 && L.Satisfaction >= 60.f) ++Count;
        return Count;
    }

    // E3a: a branch signed, being fitted out or open (the v0.1 "second store" flag is gone).
    bool HasBranch(const FMarketState& State)
    {
        return State.Branches.ContainsByPredicate([](const FMarketBranch& B) { return B.Stage != static_cast<uint8>(MarketBranches::EStage::Closed); });
    }

    FMarketDecision StoryDecision(const FMarketState& State, const TCHAR* Id, const FString& Title, const FString& Text, std::initializer_list<FString> Options, int32 Default, int32 Days, int32 Arg = 0)
    {
        FMarketDecision D;
        D.Id = Id; D.Title = Title; D.Text = Text;
        for (const FString& O : Options) D.Options.Add(O);
        D.DefaultOption = Default;
        D.Deadline = State.Day + Days - 1;
        D.Arg = Arg;
        return D;
    }

    void OfferIdentity(FMarketState& State)
    {
        MarketEvents::Offer(State, StoryDecision(State, TEXT("story.identity"), TEXT("Marketin kimli\u011fi"),
            FString(TEXT("D\u00fckk\u00e2n ayakta. \u015eimdi marketinin ne olaca\u011f\u0131na karar ver; a\u00e7aca\u011f\u0131n b\u00fct\u00fcn ma\u011fazalar da bu kimli\u011fi ta\u015f\u0131r. Hi\u00e7biri her ko\u015fulda \u00fcst\u00fcn de\u011fil.")),
            { FString(TEXT("Mahalle Marketi: g\u00fcler y\u00fcz ve sad\u0131k m\u00fc\u015fteri")), FString(TEXT("Kaliteli Market: iyi mal, iyi fiyat")), FString(TEXT("H\u0131zl\u0131 \u0130ndirim: ucuz al, ucuz sat, \u00e7ok sat")) },
            0, 3));
    }
}

FString MarketStory::ChapterTitle(int32 Chapter)
{
    switch (Chapter)
    {
    case 1: return TEXT("Defter");
    case 2: return TEXT("K\u00f6k Salmak"); // M57: the neighbour's chapter became the shop's own
    case 3: return TEXT("\u0130kinci Tabela");
    case 4: return TEXT("\u0130ller");
    case 5: return TEXT("\u00dclke \u00c7ap\u0131nda");
    case 6: return TEXT("S\u0131n\u0131r\u0131n \u00d6tesi");
    case 7: return TEXT("Miras");
    default: return TEXT("Serbest oyun");
    }
}

float MarketStory::IdentityDemand(const FMarketState& State, MarketGoods::EGroup Group)
{
    switch (static_cast<EIdentity>(State.Story.Identity))
    {
    case EIdentity::Bakkal: return Group == MarketGoods::EGroup::Staples || Group == MarketGoods::EGroup::Dairy ? 1.1f : 1.02f;
    case EIdentity::Kaliteli: return 1.035f;
    case EIdentity::Indirim: return 1.035f;
    default: return 1.f;
    }
}

FString MarketStory::IdentityName(EIdentity Identity)
{
    switch (Identity)
    {
    case EIdentity::Bakkal: return TEXT("Mahalle Marketi");
    case EIdentity::Kaliteli: return TEXT("Kaliteli Market");
    case EIdentity::Indirim: return TEXT("H\u0131zl\u0131 \u0130ndirim");
    default: return TEXT("se\u00e7ilmedi");
    }
}

TArray<MarketStory::FObjective> MarketStory::Objectives(const FMarketState& State)
{
    TArray<FObjective> List;
    auto Add = [&List](const FString& Text, bool bDone, bool bLater = false) { FObjective O; O.Text = Text; O.bDone = bDone; O.bLater = bLater; List.Add(O); };
    if (StoryClosed(State)) return List;   // free play after the finale: no goals
    switch (State.Story.Chapter)
    {
    case 1:
        Add(TEXT("\u0130lk sipari\u015fi ver"), Has(State, BFirstOrder));
        Add(TEXT("\u0130lk k\u00e2rl\u0131 g\u00fcn"), State.ProfitableDays >= 1);
        Add(TEXT("\u0130lk haftay\u0131 bitir"), State.Day > 7);
        break;
    case 2:
        Add(TEXT("\u0130\u015fletmenin borcunu kapat"), !MarketCampaign::DebtOpen(State));
        Add(FString::Printf(TEXT("Mahalle pay\u0131n\u0131 %%%.0f'e \u00e7\u0131kar (\u015fu an %%%.0f)"), MarketCampaign::ShareGoal(State), State.MarketShare), Has(State, BShare35));
        Add(FString::Printf(TEXT("15 m\u00fcdavim kazan (\u015fu an %d)"), Regulars(State)), Regulars(State) >= 15);
        Add(TEXT("Marketin kimli\u011fini se\u00e7"), Has(State, BIdentity));
        break;
    case 3:
        Add(TEXT("\u0130kinci \u015fubeyi a\u00e7"), HasBranch(State));
        Add(TEXT("Bir \u0130K m\u00fcd\u00fcr\u00fc i\u015fe al"), MarketStaff::HasHr(State));
        Add(TEXT("Defterleri mali m\u00fc\u015favire devret"), MarketStaff::HasAccountant(State));
        break;
    case 4:
    {
        const int32 Stores = MarketCompany::TotalStores(State), Provinces = MarketCompany::Provinces(State);
        Add(FString::Printf(TEXT("\u0130ki ilde 8 ma\u011faza (\u015fu an %d ma\u011faza, %d il)"), Stores, Provinces), Stores >= 8 && Provinces >= 2);
        Add(TEXT("\u0130lk depo"), MarketDepots::Count(State) > 0); // G-089: depots in provinces
        Add(TEXT("\u0130lk kamyon"), State.Company.Trucks > 0);
        break;
    }
    case 5:
    {
        const int32 Stores = MarketCompany::TotalStores(State);
        Add(FString::Printf(TEXT("50 ma\u011faza (\u015fu an %d)"), Stores), Stores >= 50);
        Add(TEXT("\u00d6zel marka \"Miras\""), State.Company.bPrivateLabel);
        // B1 (#45): the share follows revenue now; 0.1 % is about fifty stores of a discounter's size.
        Add(FString::Printf(TEXT("Ulusal pay %%%.1f (\u015fu an %%%.2f)"), NationalShareGoal, MarketCompany::NationalShare(State)), MarketCompany::NationalShare(State) >= NationalShareGoal);
        break;
    }
    case 6:
    {
        // G-086: any country pack; the first foreign shop must earn within its first month.
        bool bPilot = false;
        for (const FMarketBranch& B : State.Branches)
            if (B.Stage == static_cast<uint8>(MarketBranches::EStage::Open) && MarketBranches::CountryOf(State, B) != State.CountryId && State.Day - B.OpenedDay >= 30 && B.Last30Profit > 0) bPilot = true;
        Add(TEXT("Yurt d\u0131\u015f\u0131ndaki ilk ma\u011faza 30 g\u00fcnde k\u00e2rl\u0131"), bPilot);
        Add(FString::Printf(TEXT("\u0130kinci yabanc\u0131 \u00fclke (\u015fu an %d)"), MarketCompany::ForeignCountries(State)), MarketCompany::ForeignCountries(State) >= 2);
        break;
    }
    case 7:
        // B6 (karar J02): the world league (C's MarketChains calls MarketGoals::OnLeagueYear); the older measure
        // below stays until Mustafa decides (Akis B notu).
        Add(State.Goals.LastLeagueRank > 0
                ? FString::Printf(TEXT("D\u00fcnya liginde %d y\u0131l \u00fcst \u00fcste 1. (son y\u0131l %d. s\u0131ra)"), MarketGoals::FinaleYears, State.Goals.LastLeagueRank)
                : FString::Printf(TEXT("D\u00fcnya liginde %d y\u0131l \u00fcst \u00fcste 1."), MarketGoals::FinaleYears),
            State.Goals.LeagueFirstYears >= MarketGoals::FinaleYears);
        Add(FString::Printf(TEXT("Bir y\u0131l her \u00f6l\u00e7\u00fctte \u00f6nde: pay %%40, 60 ma\u011faza, k\u00e2r, memnun m\u00fc\u015fteri (%d / %d g\u00fcn)"),
            State.Company.LeadershipDays, MarketCompany::LeadershipGoalDays), State.Company.LeadershipDays >= MarketCompany::LeadershipGoalDays);
        break;
    default: break;
    }
    return List;
}

void MarketStory::AddMemory(FMarketState& State, const FString& Text)
{
    const FString Date = MarketCalendar::DateText(FMath::Max(1, State.Day - 1));
    State.Story.Memories.Add(Date + TEXT(": ") + Text);
    State.DayNews.Add(TEXT("Hat\u0131ra: ") + Text + TEXT("."));
}

bool MarketStory::StoryClosed(const FMarketState& State)
{
    return State.Story.bEnded || State.Story.bCampaignOver;
}

bool MarketStory::ReachFinale(FMarketState& State, EEnding Ending)
{
    if (State.Story.bEnded) return false;
    State.Story.bEnded = true;
    State.Story.Ending = static_cast<uint8>(Ending);
    const bool bLegacy = Ending == EEnding::Legacy;
    AddMemory(State, bLegacy ? TEXT("Miras: babandan devrald\u0131\u011f\u0131n d\u00fckk\u00e2n, herkesin bildi\u011fi bir tabela oldu") : TEXT("kampanyan\u0131n son g\u00fcn\u00fc"));
    // The finale card (karar J02): a short summary, one choice, shown once.
    const FString Summary = FString::Printf(TEXT("%d ma\u011faza \u00b7 %d il \u00b7 kasa %s \u00b7 %d hat\u0131ra."),
        MarketCompany::TotalStores(State), MarketCompany::Provinces(State), *StoryTl(State.Cash), State.Story.Memories.Num());
    const FString Text = bLegacy
        ? FString::Printf(TEXT("%s eski tabelan\u0131n \u00f6n\u00fcnde durup g\u00fcl\u00fcmsedi. Devrald\u0131\u011f\u0131n d\u00fckk\u00e2n art\u0131k herkesin bildi\u011fi bir isim. "), *MarketStart::Relative(State, MarketStart::ECase::Plain, true)) + Summary
        : TEXT("Y\u0131llar ge\u00e7ti; defterin son sayfas\u0131na geldin. ") + Summary;
    MarketEvents::Offer(State, StoryDecision(State, TEXT("story.finale"), bLegacy ? TEXT("Son: Miras") : TEXT("Son: Defterin son sayfas\u0131"), Text,
        { FString(TEXT("Oynamaya devam et")) }, 0, 7));
    return true;
}

void MarketStory::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    (void)Products;
    const int32 Closed = State.Day - 1;
    if (Closed < 1 || StoryClosed(State)) return;
    // Karar J02: the campaign's last day brings the finale if the Legacy one has not come.
    if (MarketCalendar::DateOf(Closed).Year > FinalYear) { ReachFinale(State, EEnding::TimeUp); return; }
    // Milestones.
    if (State.LastPurchases > 0 && !Has(State, BFirstOrder)) { Mark(State, BFirstOrder); AddMemory(State, TEXT("ilk sipari\u015f")); }
    if (State.LastProfit > 0 && !Has(State, BFirstProfit)) { Mark(State, BFirstProfit); AddMemory(State, TEXT("ilk k\u00e2rl\u0131 g\u00fcn")); }
    if (!MarketCampaign::DebtOpen(State) && !Has(State, BDebt)) { Mark(State, BDebt); AddMemory(State, TEXT("i\u015fletmenin borcu kapand\u0131")); }
    if (State.MarketShare >= MarketCampaign::ShareGoal(State) && !Has(State, BShare35)) { Mark(State, BShare35); AddMemory(State, TEXT("mahallede say\u0131lan bir d\u00fckk\u00e2n olduk")); }
    if (MarketStaff::Count(State, MarketStaff::ERole::Cashier) + MarketStaff::Count(State, MarketStaff::ERole::Stocker) > 0 && !Has(State, BFirstEmployee))
    {
        Mark(State, BFirstEmployee);
        AddMemory(State, TEXT("ilk \u00e7al\u0131\u015fan"));
    }
    if (HasBranch(State) && !Has(State, BSecondStore)) { Mark(State, BSecondStore); AddMemory(State, TEXT("ikinci tabela as\u0131ld\u0131")); }
    if (Closed >= 7 && !Has(State, BFirstWeek)) { Mark(State, BFirstWeek); AddMemory(State, TEXT("ilk hafta bitti")); }

    // M57: in the second chapter the shop chooses what it wants to be (once; the default keeps the corner shop).
    if (State.Story.Chapter >= 2 && !Has(State, BIdentityOffered) && !Has(State, BIdentity) && State.Day >= 10)
    {
        Mark(State, BIdentityOffered);
        OfferIdentity(State);
    }

    // Next chapter when every goal of this one is reached.
    const TArray<FObjective> Goals = Objectives(State);
    bool bAll = Goals.Num() > 0;
    for (const FObjective& G : Goals) if (!G.bDone || G.bLater) bAll = false;
    if (bAll && State.Story.Chapter < 7)
    {
        ++State.Story.Chapter;
        AddMemory(State, FString::Printf(TEXT("yeni b\u00f6l\u00fcm: %s"), *ChapterTitle(State.Story.Chapter)));
    }
}

bool MarketStory::Resolve(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& D, int32 Option, FString& OutMessage)
{
    if (D.Id == TEXT("story.finale"))
    {
        OutMessage = TEXT("Oyun serbest devam ediyor. Yeni b\u00f6l\u00fcm ya da hikaye gelmeyecek; d\u00fckk\u00e2n, \u015firket ve rakipler i\u015flemeye devam eder.");
        return true;
    }
    if (D.Id == TEXT("story.identity"))
    {
        using MarketEvents::EModifier;
        constexpr int32 Forever = MAX_int32 / 2;
        const EIdentity Identity = static_cast<EIdentity>(FMath::Clamp(Option, 0, 2) + 1);
        State.Story.Identity = static_cast<uint8>(Identity);
        Mark(State, BIdentity);
        const FString Source = TEXT("story.identity");
        State.Modifiers.RemoveAll([&Source](const FMarketModifier& M) { return M.Source == Source; });
        switch (Identity)
        {
        case EIdentity::Bakkal:
            MarketEvents::AddModifier(State, EModifier::PriceTolerance, MarketEvents::AllGroups, 0.04f, State.Day, Forever, Source);
            MarketEvents::AddModifier(State, EModifier::Interest, static_cast<uint8>(MarketGoods::EGroup::Staples), 1.1f, State.Day, Forever, Source);
            MarketEvents::AddModifier(State, EModifier::Interest, static_cast<uint8>(MarketGoods::EGroup::Dairy), 1.1f, State.Day, Forever, Source);
            OutMessage = TEXT("Mahalle Marketi: m\u00fcdavimler biraz daha ho\u015fg\u00f6r\u00fcl\u00fc, s\u00fct ve temel g\u0131da biraz daha \u00e7ok satar. B\u00fct\u00fcn ma\u011fazalar\u0131n bu kimli\u011fi ta\u015f\u0131r.");
            break;
        case EIdentity::Kaliteli:
            MarketEvents::AddModifier(State, EModifier::PriceTolerance, MarketEvents::AllGroups, 0.07f, State.Day, Forever, Source);
            MarketEvents::AddModifier(State, EModifier::CostFactor, MarketEvents::AllGroups, 1.04f, State.Day, Forever, Source);
            OutMessage = TEXT("Kaliteli Market: m\u00fc\u015fteri daha y\u00fcksek fiyat\u0131 kabul eder; iyi mal %4 daha pahal\u0131ya gelir. B\u00fct\u00fcn ma\u011fazalar\u0131n bu kimli\u011fi ta\u015f\u0131r.");
            break;
        default:
            MarketEvents::AddModifier(State, EModifier::PriceTolerance, MarketEvents::AllGroups, -0.03f, State.Day, Forever, Source);
            MarketEvents::AddModifier(State, EModifier::CostFactor, MarketEvents::AllGroups, 0.96f, State.Day, Forever, Source);
            MarketEvents::AddModifier(State, EModifier::Traffic, MarketEvents::AllGroups, 1.05f, State.Day, Forever, Source);
            OutMessage = TEXT("H\u0131zl\u0131 \u0130ndirim: al\u0131\u015flar %4 ucuz, biraz daha \u00e7ok m\u00fc\u015fteri; ama gelen m\u00fc\u015fteri fiyat avc\u0131s\u0131. B\u00fct\u00fcn ma\u011fazalar\u0131n bu kimli\u011fi ta\u015f\u0131r.");
            break;
        }
        AddMemory(State, FString::Printf(TEXT("d\u00fckk\u00e2n\u0131n kimli\u011fi: %s"), *IdentityName(Identity)));
        return true;
    }
    OutMessage = TEXT("Bu karar art\u0131k ge\u00e7erli de\u011fil.");
    return true;
}

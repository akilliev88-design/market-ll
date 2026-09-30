#include "MarketStory.h"
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
        BWelcome = 1ll << 0, BCem = 1ll << 1, BSelim = 1ll << 2, BKadir = 1ll << 3, BSellOffered = 1ll << 4, BSellAnswered = 1ll << 5,
        BIdentity = 1ll << 6, BFirstOrder = 1ll << 7, BFirstProfit = 1ll << 8, BDebt = 1ll << 9, BShare35 = 1ll << 10,
        BFirstEmployee = 1ll << 11, BSecondStore = 1ll << 12, BCemGone = 1ll << 13, BFirstWeek = 1ll << 14,
    };
    constexpr int32 CemCandidateId = 900001;   // fixed id: the story recognises Cem in the pool and on the payroll

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

    FMarketLoyalty& Nermin(FMarketState& State)
    {
        if (FMarketLoyalty* Found = State.Loyalty.FindByPredicate([](const FMarketLoyalty& L) { return L.CustomerId == 0; })) return *Found;
        FMarketLoyalty Entry;
        Entry.CustomerId = 0;
        Entry.Satisfaction = 60.f; // she liked the father
        State.Loyalty.Add(Entry);
        return State.Loyalty.Last();
    }

    bool CemOnStaff(const FMarketState& State)
    {
        return State.Staff.ContainsByPredicate([](const FMarketEmployee& E) { return E.Id == CemCandidateId; });
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
        MarketEvents::Offer(State, StoryDecision(State, TEXT("story.identity"), TEXT("D\u00fckk\u00e2n\u0131n kimli\u011fi"),
            TEXT("Kadir Bey'e hay\u0131r dedin. \u015eimdi d\u00fckk\u00e2n\u0131n ne olaca\u011f\u0131na karar ver. Hi\u00e7biri her ko\u015fulda \u00fcst\u00fcn de\u011fil; rakipler de buna g\u00f6re davranacak."),
            { FString(TEXT("Mahallenin Bakkal\u0131: samimiyet ve sadakat")), FString(TEXT("Kaliteli Yerel: iyi mal, iyi fiyat")), FString(TEXT("H\u0131zl\u0131 \u0130ndirim: ucuz al, ucuz sat, \u00e7ok sat")) },
            0, 3));
    }
}

FString MarketStory::ChapterTitle(int32 Chapter)
{
    switch (Chapter)
    {
    case 1: return TEXT("Defter");
    case 2: return TEXT("Kar\u015f\u0131 D\u00fckk\u00e2n");
    case 3: return TEXT("\u0130kinci Tabela");
    case 4: return TEXT("\u0130ller");
    case 5: return TEXT("\u00dclke \u00c7ap\u0131nda");
    case 6: return TEXT("S\u0131n\u0131r\u0131n \u00d6tesi");
    case 7: return TEXT("Miras");
    default: return TEXT("Serbest oyun");
    }
}

FString MarketStory::IdentityName(EIdentity Identity)
{
    switch (Identity)
    {
    case EIdentity::Bakkal: return TEXT("Mahallenin Bakkal\u0131");
    case EIdentity::Kaliteli: return TEXT("Kaliteli Yerel");
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
        Add(TEXT("Yerel pay\u0131 %35'e \u00e7\u0131kar"), Has(State, BShare35));
        Add(FString::Printf(TEXT("15 m\u00fcdavim kazan (\u015fu an %d)"), Regulars(State)), Regulars(State) >= 15);
        Add(TEXT("Kadir Bey'in teklifine cevap ver"), Has(State, BSellAnswered));
        break;
    case 3:
        Add(TEXT("\u0130kinci \u015fubeyi a\u00e7"), State.bSecondStore);
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
        Add(FString::Printf(TEXT("Ulusal pay %%2 (\u015fu an %%%.2f)"), MarketCompany::NationalShare(State)), MarketCompany::NationalShare(State) >= 2.f);
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

int64 MarketStory::SaleOffer(const FMarketState& State, const TArray<FMarketProduct>& Products)
{
    int64 Goods = 0;
    for (int32 I = 0; I < State.Stock.Num() && I < Products.Num(); ++I)
        Goods += static_cast<int64>(State.Stock[I].Shelf + State.Stock[I].Warehouse) * Products[I].Cost;
    int64 Profit = 0;
    int32 Days = 0;
    for (int32 I = State.History.Num() - 1; I >= 0 && Days < 7; --I, ++Days) Profit += State.History[I].Profit;
    const int64 DailyProfit = Days > 0 ? FMath::Max<int64>(0, Profit / Days) : 0;
    // Name and customers, the goods, two months of profit; he also takes over the father's debt.
    const int64 Offer = 30000 + Goods + DailyProfit * 60 + static_cast<int64>(State.MarketShare * 1000.f) - State.InheritedDebt;
    return FMath::Max<int64>(20000, Offer / 500 * 500);
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
    AddMemory(State, bLegacy ? TEXT("Miras: aileden kalan d\u00fckk\u00e2n, herkesin bildi\u011fi bir tabela oldu") : TEXT("kampanyan\u0131n son g\u00fcn\u00fc"));
    // The finale card (karar J02): a short summary, one choice, shown once.
    const FString Summary = FString::Printf(TEXT("%d ma\u011faza \u00b7 %d il \u00b7 kasa %s \u00b7 %d hat\u0131ra."),
        MarketCompany::TotalStores(State), MarketCompany::Provinces(State), *StoryTl(State.Cash), State.Story.Memories.Num());
    const FString Text = bLegacy
        ? FString::Printf(TEXT("Nermin teyze tabelaya bak\u0131p g\u00fcl\u00fcmsedi: \"%s g\u00f6rseydi...\" Aileden kalan d\u00fckk\u00e2n art\u0131k herkesin bildi\u011fi bir isim. "), *MarketStart::Relative(State, MarketStart::ECase::Plain, true)) + Summary
        : TEXT("Y\u0131llar ge\u00e7ti; defterin son sayfas\u0131na geldin. ") + Summary;
    MarketEvents::Offer(State, StoryDecision(State, TEXT("story.finale"), bLegacy ? TEXT("Son: Miras") : TEXT("Son: Defterin son sayfas\u0131"), Text,
        { FString(TEXT("Oynamaya devam et")) }, 0, 7));
    return true;
}

void MarketStory::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    const int32 Closed = State.Day - 1;
    if (Closed < 1 || StoryClosed(State)) return;
    // Karar J02: the campaign's last day brings the finale if the Legacy one has not come.
    if (MarketCalendar::DateOf(Closed).Year > FinalYear) { ReachFinale(State, EEnding::TimeUp); return; }
    TArray<FString>& News = State.DayNews;

    // Scenes.
    if (Closed >= 1 && !Has(State, BWelcome))
    {
        Mark(State, BWelcome);
        Nermin(State);
        News.Add(FString::Printf(TEXT("Nermin teyze u\u011frad\u0131: \"%s her sabah bana bir \u015fi\u015fe s\u00fct ay\u0131r\u0131rd\u0131, evlad\u0131m. Sen de ay\u0131r\u0131rs\u0131n, de\u011fil mi?\" S\u00fct raf\u0131n\u0131 bo\u015f b\u0131rakma; mahalle ona bak\u0131yor."), *MarketStart::Relative(State, MarketStart::ECase::Plain, true)));
    }
    if (Closed >= 1 && !Has(State, BCem))
    {
        Mark(State, BCem);
        FMarketEmployee Cem;
        Cem.Id = CemCandidateId;
        Cem.Name = TEXT("Cem Aksoy");
        Cem.Role = static_cast<uint8>(MarketStaff::ERole::Stocker);
        Cem.Skill = 62; Cem.Speed = 45; Cem.Stamina = 75; Cem.Honesty = 95;
        Cem.DailyWage = FMath::Max<int64>(1000, MarketStaff::FairWage(MarketStaff::ERole::Stocker, 62, State.Day) * 95 / 100 / 50 * 50);
        Cem.Morale = 80.f;
        State.Candidates.Insert(Cem, 0);
        News.Add(FString::Printf(TEXT("Cem kap\u0131ya geldi: \"%s yan\u0131nda dokuz y\u0131l \u00e7al\u0131\u015ft\u0131m abi. Depoyu, raf\u0131, m\u00fc\u015fteriyi bilirim.\" Aday listesinde; ilk hafta boyunca bekler."), *MarketStart::Relative(State, MarketStart::ECase::Genitive, true)));
    }
    if (Closed >= 3 && !Has(State, BSelim))
    {
        Mark(State, BSelim);
        News.Add(FString::Printf(TEXT("Selim (Trakya G\u0131da) u\u011frad\u0131: \"%s yirmi y\u0131l \u00e7al\u0131\u015ft\u0131k. D\u00fczenli al, zaman\u0131nda \u00f6de; vadeyi de iskontoyu da a\u00e7ar\u0131m.\""), *MarketStart::Relative(State, MarketStart::ECase::With, true)));
    }
    if (Closed >= 5 && !Has(State, BKadir))
    {
        Mark(State, BKadir);
        News.Add(TEXT("Kadir Bereketo\u011flu kap\u0131n\u0131n \u00f6n\u00fcnden ge\u00e7erken durdu: \"Bu sokak iki bakkala dar gelir, delikanl\u0131. G\u00f6r\u00fcr\u00fcz.\""));
    }
    if (Closed >= 7 && Has(State, BCem) && !Has(State, BCemGone) && !CemOnStaff(State) &&
        !State.Candidates.ContainsByPredicate([](const FMarketEmployee& E) { return E.Id == CemCandidateId; }))
    {
        Mark(State, BCemGone);
        News.Add(FString::Printf(TEXT("Cem Babaeski'de bir f\u0131r\u0131nda i\u015fe ba\u015flam\u0131\u015f. \"%s olsa beni al\u0131rd\u0131\" demi\u015f."), *MarketStart::Relative(State, MarketStart::ECase::Plain, true)));
    }
    // Nermin teyze notices an empty milk shelf (at most every ten days).
    bool bNoMilk = false;
    for (int32 I = 0; I < State.Stock.Num() && I < Products.Num(); ++I)
        if (MarketGoods::Classify(Products[I].Category) == MarketGoods::EGroup::Dairy && State.Stock[I].Yesterday.Empty > 0) bNoMilk = true;
    if (bNoMilk && !MarketEvents::Happened(State, TEXT("story.nermin"), 10))
    {
        MarketEvents::Log(State, TEXT("story.nermin"));
        FMarketLoyalty& N = Nermin(State);
        N.Satisfaction = FMath::Max(0.f, N.Satisfaction - 8.f);
        News.Add(FString::Printf(TEXT("Nermin teyze eli bo\u015f d\u00f6nd\u00fc: \"%s zaman\u0131nda s\u00fct hi\u00e7 bitmezdi.\" Bunu yar\u0131n b\u00fct\u00fcn sokak duyar."), *MarketStart::Relative(State, MarketStart::ECase::Genitive, true)));
    }

    // Milestones.
    if (State.LastPurchases > 0 && !Has(State, BFirstOrder)) { Mark(State, BFirstOrder); AddMemory(State, TEXT("ilk sipari\u015f")); }
    if (State.LastProfit > 0 && !Has(State, BFirstProfit)) { Mark(State, BFirstProfit); AddMemory(State, TEXT("ilk k\u00e2rl\u0131 g\u00fcn")); }
    if (!MarketCampaign::DebtOpen(State) && !Has(State, BDebt)) { Mark(State, BDebt); AddMemory(State, TEXT("i\u015fletmenin borcu kapand\u0131")); }
    if (State.MarketShare >= 35.f && !Has(State, BShare35)) { Mark(State, BShare35); AddMemory(State, TEXT("mahallenin \u00fc\u00e7te biri art\u0131k bizden al\u0131\u015fveri\u015f yap\u0131yor")); }
    if (MarketStaff::Count(State, MarketStaff::ERole::Cashier) + MarketStaff::Count(State, MarketStaff::ERole::Stocker) > 0 && !Has(State, BFirstEmployee))
    {
        Mark(State, BFirstEmployee);
        AddMemory(State, CemOnStaff(State) ? TEXT("Cem yeniden d\u00fckk\u00e2nda") : TEXT("ilk \u00e7al\u0131\u015fan"));
    }
    if (State.bSecondStore && !Has(State, BSecondStore)) { Mark(State, BSecondStore); AddMemory(State, TEXT("ikinci tabela as\u0131ld\u0131")); }
    if (Closed >= 7 && !Has(State, BFirstWeek)) { Mark(State, BFirstWeek); AddMemory(State, TEXT("ilk hafta bitti")); }

    // Kadir Bey wants the shop.
    if (State.Story.Chapter >= 2 && !Has(State, BSellOffered) && (State.Day >= 10 || !MarketCampaign::DebtOpen(State)))
    {
        Mark(State, BSellOffered);
        const int64 Offer = SaleOffer(State, Products);
        MarketEvents::Offer(State, StoryDecision(State, TEXT("story.sell"), TEXT("Kadir Bey'in teklifi"),
            FString::Printf(TEXT("Kadir Bereketo\u011flu \u00e7ay\u0131n\u0131 kar\u0131\u015ft\u0131r\u0131yor: \"D\u00fckk\u00e2n\u0131 bana devret. %s veririm, d\u00fckk\u00e2n\u0131n toptanc\u0131 borcunu da ben kapat\u0131r\u0131m. Bina senin kals\u0131n, kiras\u0131n\u0131 \u00f6derim.\""), *StoryTl(Offer)),
            { FString::Printf(TEXT("Satm\u0131yorum. Bu d\u00fckk\u00e2n %s."), *MarketStart::Relative(State, MarketStart::ECase::Mine)), FString::Printf(TEXT("Sat (%s)"), *StoryTl(Offer)) }, 0, 3, static_cast<int32>(FMath::Min<int64>(Offer, MAX_int32))));
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
    if (D.Id == TEXT("story.sell"))
    {
        Mark(State, BSellAnswered);
        if (Option == 0)
        {
            OutMessage = FString::Printf(TEXT("\"Bu d\u00fckk\u00e2n %s.\" Kadir Bey bir \u015fey demeden kalkt\u0131. Sokakta r\u00fczg\u00e2r de\u011fi\u015fti."), *MarketStart::Relative(State, MarketStart::ECase::Mine));
            AddMemory(State, TEXT("Kadir Bey'e hay\u0131r dendi"));
            OfferIdentity(State);
            // A proud neighbour does not forget a no.
            if (FMarketCompetitor* B = State.Competitors.FindByPredicate([](const FMarketCompetitor& C) { return C.Company == 0; })) B->Anger = FMath::Min(100.f, B->Anger + 15.f);
        }
        else
        {
            MarketEvents::Offer(State, StoryDecision(State, TEXT("story.aftersale"), TEXT("\u0130mza"),
                TEXT("Noterde k\u00e2\u011f\u0131tlar haz\u0131r. \u0130mzalarsan d\u00fckk\u00e2n Bereket Market'in olur; sen de parayla yeni bir hayata ba\u015flars\u0131n. Bu oyunun sonlar\u0131ndan biri. \u0130stersen son anda vazge\u00e7ebilirsin."),
                { FString(TEXT("Son anda vazge\u00e7, d\u00fckk\u00e2na d\u00f6n")), FString(TEXT("\u0130mzala: bu son olsun")) }, 0, 1, D.Arg));
            OutMessage = TEXT("Kadir Bey'in eli uzand\u0131. Noter yar\u0131n.");
        }
        return true;
    }
    if (D.Id == TEXT("story.aftersale"))
    {
        if (Option == 0)
        {
            OutMessage = TEXT("Kalemi b\u0131rakt\u0131n. \"Olmaz, Kadir Bey.\" D\u00fckk\u00e2na d\u00f6nd\u00fcn.");
            AddMemory(State, TEXT("noterden d\u00f6n\u00fcld\u00fc"));
            OfferIdentity(State);
            return true;
        }
        // The ending is seen and remembered; then the player chooses: it was a dream (back to the shop, the
        // money never came) or this is where the story ends (free play).
        // The sale money is not paid into the till yet (karar J03): it cannot be spent before the choice below,
        // so "Ruyaymis" can undo the sale completely.
        State.Story.Ending = static_cast<uint8>(EEnding::Sold);
        AddMemory(State, TEXT("d\u00fckk\u00e2n sat\u0131ld\u0131 (son: Satt\u0131n)"));
        OutMessage = FString::Printf(TEXT("Satt\u0131n. %s ile \u0130stanbul'a gittin; y\u0131llar sonra \u0130stasyon Caddesi'nden ge\u00e7erken tabelada ba\u015fka bir isim vard\u0131. (Son: Satt\u0131n.)"),
            *StoryTl(D.Arg));
        MarketEvents::Offer(State, StoryDecision(State, TEXT("story.dream"), TEXT("Son: Satt\u0131n"),
            TEXT("Bu hikayenin sonlar\u0131ndan biriydi. Bir sabah d\u00fckk\u00e2n\u0131n kepengini a\u00e7ma sesiyle uyan\u0131rsan r\u00fcyaym\u0131\u015f ve oyun kald\u0131\u011f\u0131 yerden s\u00fcrer; ya da burada bitsin: bu kampanya kapan\u0131r, yeni oyun a\u00e7\u0131l\u0131r."),
            { FString(TEXT("R\u00fcyaym\u0131\u015f: d\u00fckk\u00e2na d\u00f6n")), FString(TEXT("Burada bitsin")) }, 0, 3, D.Arg));
        return true;
    }
    if (D.Id == TEXT("story.dream"))
    {
        if (Option == 0)
        {
            // The sale money never reached the till, so there is nothing to take back (karar J03).
            OutMessage = FString::Printf(TEXT("Kepengin sesiyle uyand\u0131n. Tezgah, defter, %s \u00e7ay barda\u011f\u0131... Hepsi yerinde. R\u00fcyaym\u0131\u015f."), *MarketStart::Relative(State, MarketStart::ECase::Mine));
            AddMemory(State, TEXT("sat\u0131\u015f r\u00fcyaym\u0131\u015f; d\u00fckk\u00e2na d\u00f6n\u00fcld\u00fc"));
            OfferIdentity(State);
            return true;
        }
        State.Story.bCampaignOver = true;
        State.Story.bEnded = true;
        OutMessage = TEXT("Hikaye burada bitti. Bu kampanya kapand\u0131; yeni oyun i\u00e7in F6'ya iki kez bas.");
        return true;
    }
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
            OutMessage = TEXT("Mahallenin Bakkal\u0131: m\u00fcdavimler biraz daha ho\u015fg\u00f6r\u00fcl\u00fc, s\u00fct ve temel g\u0131da biraz daha \u00e7ok satar. Veresiye defteri a\u00e7\u0131l\u0131r.");
            break;
        case EIdentity::Kaliteli:
            MarketEvents::AddModifier(State, EModifier::PriceTolerance, MarketEvents::AllGroups, 0.07f, State.Day, Forever, Source);
            MarketEvents::AddModifier(State, EModifier::CostFactor, MarketEvents::AllGroups, 1.04f, State.Day, Forever, Source);
            OutMessage = TEXT("Kaliteli Yerel: m\u00fc\u015fteri daha y\u00fcksek fiyat\u0131 kabul eder; iyi mal %4 daha pahal\u0131ya gelir.");
            break;
        default:
            MarketEvents::AddModifier(State, EModifier::PriceTolerance, MarketEvents::AllGroups, -0.03f, State.Day, Forever, Source);
            MarketEvents::AddModifier(State, EModifier::CostFactor, MarketEvents::AllGroups, 0.96f, State.Day, Forever, Source);
            MarketEvents::AddModifier(State, EModifier::Traffic, MarketEvents::AllGroups, 1.05f, State.Day, Forever, Source);
            if (FMarketCompetitor* B = State.Competitors.FindByPredicate([](const FMarketCompetitor& C) { return C.Company == 0; })) B->Anger = FMath::Min(100.f, B->Anger + 25.f);
            OutMessage = TEXT("H\u0131zl\u0131 \u0130ndirim: al\u0131\u015flar %4 ucuz, biraz daha \u00e7ok m\u00fc\u015fteri; ama gelen m\u00fc\u015fteri fiyat avc\u0131s\u0131. Kadir Bey \u00e7ok sinirlendi.");
            break;
        }
        AddMemory(State, FString::Printf(TEXT("d\u00fckk\u00e2n\u0131n kimli\u011fi: %s"), *IdentityName(Identity)));
        return true;
    }
    OutMessage = TEXT("Bu karar art\u0131k ge\u00e7erli de\u011fil.");
    return true;
}

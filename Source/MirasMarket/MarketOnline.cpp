#include "MarketOnline.h"
#include "MarketEras.h"
#include "MarketLedger.h"
#include "MarketPromotions.h"
#include "MarketCountry.h"
#include "MarketCalendar.h"
#include "MarketPrices.h"
#include "MarketStaff.h"
#include "MarketCast.h"
#include "MarketBranches.h"
#include "MarketChains.h"
#include "MarketManagers.h"
#include "MarketStart.h"
#include "MarketEvents.h"
#include "MarketStoreAssign.h"

namespace MarketOnlineLocal
{
    using MarketOnline::EChannel;

    enum : int32
    {
        TWebRumour = 1 << 0, TWeb = 1 << 1, TPlatformRumour = 1 << 2, TPlatform = 1 << 3, TAppRumour = 1 << 4, TApp = 1 << 5,
        TPandemic = 1 << 6, TQuick = 1 << 7, TPlatformMarket = 1 << 8, TPandemicEnd = 1 << 9
    };

    uint32 Mix(int32 Seed, int32 Day, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), static_cast<uint32>(Day), Salt };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }

    FString Tl(int64 Kurus) { return MarketCountry::Money(Kurus); }
    int32 DateDay(int32 Year, int32 Month, int32 Day) { return MarketCalendar::GameDayOf(Year, Month, Day); }
    double LevelOn(int32 Day) { return MarketPrices::ListLevel(FMath::Max(1, Day)); }
    int64 AtLevel(int64 Kurus, int32 Day) { return FMath::RoundToInt64(Kurus * LevelOn(Day)); }
    int64 WageAt(int64 Kurus, int32 Day) { return FMath::RoundToInt64(Kurus * MarketPrices::WageIndex(FMath::Max(1, Day))); }

    // The 2020-2021 period is different in every campaign: when it starts, how long the panic lasts, which closure
    // days come and how long the full closure is come from the seed.
    uint32 Roll(const FMarketState& State, uint32 Salt, int32 Extra = 0) { return Mix(State.RivalSeed, Extra, Salt); }
    int32 PanicEnd(const FMarketState& State) { return MarketOnline::PandemicStart(State) + 1 + 8 + static_cast<int32>(Roll(State, 0x9A01u) % 7u); }
    int32 ClosureStart(const FMarketState& State) { return DateDay(2021, 4, 15) + MarketEras::PandemicShiftDays(State) + static_cast<int32>(Roll(State, 0x9A02u) % 21u); }
    int32 ClosureEnd(const FMarketState& State) { return ClosureStart(State) + 10 + static_cast<int32>(Roll(State, 0x9A03u) % 11u); }
    bool IsPanic(const FMarketState& State, int32 GameDay) { return GameDay > MarketOnline::PandemicStart(State) && GameDay < PanicEnd(State); }

    FString CountryOr(const FMarketState& State, const FString& Country) { return Country.IsEmpty() ? State.CountryId : Country; }

    float Plateau(const FMarketState& State, const FString& Country)
    {
        const MarketCountry::FProfile* Pack = MarketCountry::Find(CountryOr(State, Country));
        return Pack ? Pack->OnlinePlateau : 0.08f;
    }

    void Tell(FMarketState& State, const FString& Text, bool bHint = true)
    {
        State.DayNews.Add(Text);
        if (bHint) { State.Online.Hint = Text; State.Online.HintDay = State.Day; }
    }

    bool IsOpenBranch(const FMarketBranch& B) { return B.Stage == static_cast<uint8>(MarketBranches::EStage::Open); }

    // What a closed day sold online in a shop; Branch INDEX_NONE = the family shop.
    struct FShop
    {
        int32 Branch = INDEX_NONE;
        int32 Area = INDEX_NONE;
        int32 Walkins = 0;
        int32 Pick = 0;
    };

    // Channel weights of an area on a day (0 when the channel does not work there).
    struct FPull
    {
        float Web = 0.f, App = 0.f, Platform = 0.f;
        bool bQuick = false;
        float Own() const { return Web + App; }
        float All() const { return Web + App + Platform; }
    };

    bool OwnWorks(const FMarketOnline& O, const FMarketOnlineArea& A) { return O.bWeb && A.bOwn; }
    bool QuickWorks(const FMarketOnline& O, const FMarketOnlineArea& A) { return O.bWeb && O.bApp && O.bQuick && A.bOwn && A.bQuick && A.DarkStoreDay > 0; }
    bool PlatformWorks(const FMarketOnline& O, const FMarketOnlineArea& A) { return O.bPlatform && A.bPlatform; }

    float PolicyFactor(const FMarketOnline& O)
    {
        static const float Fee[3] = { 1.2f, 1.f, 0.85f };
        static const float Min[3] = { 1.f, 0.92f, 0.8f };
        static const float Gap[3] = { 1.f, 0.9f, 0.78f };
        return Fee[FMath::Min<int32>(O.Fee, 2)] * Min[FMath::Min<int32>(O.MinBasket, 2)] * Gap[FMath::Min<int32>(O.PriceGap, 2)] * (1.f + 0.12f * FMath::Min<int32>(O.Ads, 3));
    }

    FPull PullOf(const FMarketState& State, const FMarketOnlineArea& A, int32 Day)
    {
        const FMarketOnline& O = State.Online;
        FPull P;
        const float Rep = 0.5f + O.Reputation / 100.f;
        if (OwnWorks(O, A))
        {
            const float Policy = PolicyFactor(O) * Rep;
            P.Web = 0.45f * FMath::Clamp(static_cast<float>(Day - O.WebDay + 1) / 60.f, 0.2f, 1.f) * Policy;
            if (O.bApp) P.App = 0.9f * MarketOnline::AppQuality[FMath::Min<int32>(O.AppTier, 2)] * FMath::Clamp(static_cast<float>(Day - O.AppDay + 1) / 30.f, 0.3f, 1.f) * Policy;
            P.bQuick = QuickWorks(O, A);
            if (P.bQuick) { P.Web *= 1.5f; P.App *= 1.5f; }
        }
        if (PlatformWorks(O, A)) P.Platform = 0.9f * FMath::Clamp((MarketOnline::Stars(State) - 3.f) / 1.2f, 0.15f, 1.3f);
        return P;
    }

    // Orders a shop gets on a day before the dice (and how they split).
    float Expected(const FMarketState& State, const FMarketOnlineArea& A, int32 Walkins, int32 Day, FPull& OutPull)
    {
        OutPull = PullOf(State, A, Day);
        const float All = OutPull.All();
        if (All <= 0.f || Walkins <= 0) return 0.f;
        const float Share = MarketOnline::OnlineShare(State, Day, A.Country);
        const float Pool = static_cast<float>(Walkins) / FMath::Max(0.3f, MarketOnline::StoreTrafficFactorOn(State, Day, A.Country)) * Share;
        return Pool * All / (All + MarketOnline::RivalOnline(State, A.Country, Day) + 0.4f);
    }

    int32 Count(float Mean, FRandomStream& Random)
    {
        const int32 Whole = FMath::FloorToInt(Mean);
        return Whole + (Random.FRand() < Mean - Whole ? 1 : 0);
    }

    int32 PickProduct(const TArray<float>& Weights, float Total, FRandomStream& Random)
    {
        float Left = Random.FRand() * Total;
        for (int32 I = 0; I < Weights.Num(); ++I)
        {
            Left -= Weights[I];
            if (Left <= 0.f && Weights[I] > 0.f) return I;
        }
        for (int32 I = Weights.Num() - 1; I >= 0; --I) if (Weights[I] > 0.f) return I;
        return INDEX_NONE;
    }

    FMarketBranchItem* ItemAt(FMarketBranch& B, const TArray<FMarketProduct>& Products, int32 Index)
    {
        if (B.Items.IsValidIndex(Index) && B.Items[Index].ProductId == Products[Index].Id) return &B.Items[Index];
        return B.Items.FindByPredicate([&Products, Index](const FMarketBranchItem& It) { return It.ProductId == Products[Index].Id; });
    }

    int32 RivalStage(const FMarketState& State, const FMarketChain& C, int32 Day)
    {
        const uint32 H = MarketStoreAssign::StableHash(C.Id);
        const int32 Stores = MarketChains::TotalStores(C);
        const int32 Web = MarketOnline::OpenDay(State, EChannel::Web) + static_cast<int32>(H % 1100u) - 200;
        const int32 App = MarketOnline::OpenDay(State, EChannel::App) + static_cast<int32>((H >> 8) % 500u) - 120;
        const int32 Quick = MarketOnline::OpenDay(State, EChannel::Quick) + static_cast<int32>((H >> 16) % 700u);
        if (Stores >= 300 && Day >= Quick) return 3;
        if (Day >= App) return 2;
        return Day >= Web ? 1 : 0;
    }

    FString FlagText(int32 Flags)
    {
        TArray<FString> Parts;
        if (Flags & 1) Parts.Add(MarketCast::Platform());
        if (Flags & 2) Parts.Add(TEXT("kendi teslimat\u0131m\u0131z"));
        if (Flags & 4) Parts.Add(TEXT("h\u0131zl\u0131 teslimat"));
        return Parts.Num() ? FString::Join(Parts, TEXT(" + ")) : FString(TEXT("kapal\u0131"));
    }

    FMarketDecision Card(const FMarketState& State, const TCHAR* Id, const FString& Title, const FString& Text, const TArray<FString>& Options, int32 Default, int32 Days)
    {
        FMarketDecision D;
        D.Id = Id; D.Title = Title; D.Text = Text; D.Options = Options;
        D.DefaultOption = Default;
        D.Deadline = State.Day + Days - 1;
        return D;
    }

    bool HasCard(const FMarketState& State, const TCHAR* Id)
    {
        return State.Decisions.ContainsByPredicate([Id](const FMarketDecision& D) { return D.Id == Id; });
    }

    void OfferApp(FMarketState& State)
    {
        if (HasCard(State, TEXT("online.app"))) return;
        const int32 Day = State.Day;
        TArray<FString> Options;
        static const TCHAR* Names[3] = { TEXT("Ucuz firma"), TEXT("Sa\u011flam firma"), TEXT("Se\u00e7kin firma") };
        for (int32 T = 0; T < 3; ++T) Options.Add(FString::Printf(TEXT("%s (%s)"), Names[T], *Tl(AtLevel(MarketOnline::AppCosts[T], Day))));
        Options.Add(TEXT("\u015eimdilik yapt\u0131rma"));
        MarketEvents::Offer(State, Card(State, TEXT("online.app"), TEXT("Telefon uygulamas\u0131"),
            TEXT("\u00dc\u00e7 yaz\u0131l\u0131m firmas\u0131 teklif verdi. Ucuzu \u00e7abuk yapar ama s\u0131k bozulur (y\u0131ld\u0131zlar d\u00fc\u015fer); se\u00e7kini pahal\u0131d\u0131r ama m\u00fc\u015fteri sever. Her ay bak\u0131m\u0131 maliyetinin %2'si."),
            Options, 3, 7));
    }

    // The company's rule for a province (a new province; provinces without a manager follow it at once).
    void Decide(FMarketState& State, int32 AreaIndex, bool bQuiet)
    {
        FMarketOnline& O = State.Online;
        FMarketOnlineArea& A = O.Areas[AreaIndex];
        if (A.bPlayerSet) return;
        A.bPlatform = O.bDefaultPlatform;
        A.bOwn = O.bDefaultOwn;
        A.bQuick = O.bDefaultOwn && O.bDefaultQuick && A.DarkStoreDay > 0;
        (void)bQuiet;
    }

    // Who brings a province manager's proposal to us: the country manager, else the region's, else the sub-region's.
    FString Forwarder(const FMarketState& State, const FMarketOnlineArea& A)
    {
        using MarketManagers::ELevel;
        const MarketCountry::FCity* City = MarketCountry::FindCity(A.Country, A.Province);
        int32 Who = MarketManagers::FindManager(State, ELevel::Country, A.Country, A.Country);
        FString Title = TEXT("\u00fclke m\u00fcd\u00fcr\u00fc");
        if (Who == INDEX_NONE && City) { Who = MarketManagers::FindManager(State, ELevel::Region, A.Country, City->Region); Title = TEXT("b\u00f6lge direkt\u00f6r\u00fc"); }
        if (Who == INDEX_NONE && City) { Who = MarketManagers::FindManager(State, ELevel::SubRegion, A.Country, City->SubRegion); Title = TEXT("b\u00f6lge m\u00fcd\u00fcr\u00fc"); }
        return Who == INDEX_NONE ? FString() : FString::Printf(TEXT("%s %s"), *Title, *State.Management.Managers[Who].Name);
    }

    FString AreaKey(const FMarketOnlineArea& A) { return A.Country + TEXT("|") + A.Province; }

    // Month end: a province manager looks at the last months and, when something has to change, sends a proposal
    // up the line (decision card "online.area:<country>|<province>", Arg = new flags, +8 = build the dark store).
    // Nothing changes until we approve.
    void Review(FMarketState& State, int32 AreaIndex)
    {
        FMarketOnline& O = State.Online;
        FMarketOnlineArea& A = O.Areas[AreaIndex];
        const bool bPlatformOn = PlatformWorks(O, A), bOwnOn = OwnWorks(O, A);
        A.PlatformLossMonths = bPlatformOn && A.MonthPlatformProfit < 0 ? A.PlatformLossMonths + 1 : 0;
        A.OwnLossMonths = bOwnOn && A.MonthOwnProfit < 0 ? A.OwnLossMonths + 1 : 0;
        A.MonthPlatformProfit = 0;
        A.MonthOwnProfit = 0;
        if (A.bPlayerSet || State.Day < A.QuietUntil) return;
        const int32 Boss = MarketManagers::FindManager(State, MarketManagers::ELevel::Province, A.Country, A.Province);
        if (Boss == INDEX_NONE) return;
        const FString Id = TEXT("online.area:") + AreaKey(A);
        if (State.Decisions.ContainsByPredicate([&Id](const FMarketDecision& D) { return D.Id == Id; })) return;
        const int32 Skill = MarketManagers::EffectiveManagerSkill(State, Boss);
        const int32 Shops = MarketOnline::ShopsIn(State, A.Country, A.Province);
        int32 Flags = MarketOnline::AreaFlags(A);
        FString Why;
        // A channel that lost money three months running: he wants it closed here.
        if (A.PlatformLossMonths >= 3) { Flags &= ~1; Why = FString::Printf(TEXT("%s \u00fc\u00e7 ayd\u0131r bu ilde zarar ettiriyor; komisyon k\u00e2r\u0131 yiyor."), *MarketCast::Platform()); }
        else if (A.OwnLossMonths >= 3) { Flags &= ~6; Why = TEXT("Kendi teslimat\u0131m\u0131z \u00fc\u00e7 ayd\u0131r bu ilde zarar ediyor; sipari\u015f kurye masraf\u0131n\u0131 kar\u015f\u0131lam\u0131yor."); }
        // A channel the company has but this province does not use: a good manager sees the chance.
        else if (O.bPlatform && !(Flags & 1) && Skill >= 45) { Flags |= 1; Why = FString::Printf(TEXT("Bu ilde %s'dan sipari\u015f gelmiyor; rakipler orada."), *MarketCast::Platform()); }
        else if (O.bWeb && !(Flags & 2) && Skill >= 50) { Flags |= 2; Why = TEXT("Sitemiz ve uygulamam\u0131z bu ilde \u00e7al\u0131\u015fm\u0131yor; m\u00fc\u015fteri soruyor."); }
        else if (O.bQuick && O.bApp && (Flags & 2) && !(Flags & 4) && Skill >= 55 && Shops >= MarketOnline::DarkStoreShops)
        {
            FString Reason;
            if (A.DarkStoreDay > 0) { Flags |= 4; Why = TEXT("Karanl\u0131k depo haz\u0131r ama h\u0131zl\u0131 teslimat kapal\u0131."); }
            else if (MarketOnline::CanBuildDarkStore(State, AreaIndex, Reason) && State.Cash > 3 * MarketOnline::DarkStorePrice(State, AreaIndex))
            { Flags |= 4 | 8; Why = FString::Printf(TEXT("%d ma\u011fazam\u0131zla bir karanl\u0131k depo (%s) 30 dakikada teslimat\u0131 m\u00fcmk\u00fcn k\u0131lar."), Shops, *Tl(MarketOnline::DarkStorePrice(State, AreaIndex))); }
        }
        // A skilled manager with our own fast delivery leaves the platform to save its commission.
        else if ((Flags & 1) && (Flags & 4) && Skill >= 75 && A.PlatformLossMonths >= 1) { Flags &= ~1; Why = TEXT("H\u0131zl\u0131 teslimat\u0131m\u0131z yetiyor; platformun komisyonundan kurtulal\u0131m."); }
        if (Why.IsEmpty() || ((Flags & 7) == MarketOnline::AreaFlags(A) && !(Flags & 8))) return;
        const FString Name = State.Management.Managers[Boss].Name;
        const FString Via = Forwarder(State, A);
        const FString Text = FString::Printf(TEXT("%s il m\u00fcd\u00fcr\u00fc %s: \"%s\" \u00d6nerisi: internette %s (\u015fimdi %s).%s"),
            *MarketOnline::AreaName(A), *Name, *Why, *FlagText(Flags & 7), *FlagText(MarketOnline::AreaFlags(A)),
            Via.IsEmpty() ? TEXT(" Karar senin.") : *FString::Printf(TEXT(" %s inceledi, onay\u0131na sunuyor."), *Via));
        FMarketDecision D = Card(State, *Id, FString::Printf(TEXT("%s: internet \u00f6nerisi"), *MarketOnline::AreaName(A)), Text,
            { FString(TEXT("Onayla")), FString(TEXT("Reddet")) }, 1, 10);
        D.Arg = Flags;
        MarketEvents::Offer(State, D);
    }

    // Provinces with our shops are the areas; a province without shops loses its area (and its dark store).
    void KeepAreas(FMarketState& State, bool bMonth)
    {
        FMarketOnline& O = State.Online;
        TArray<TPair<FString, FString>> Places;
        Places.Add(TPair<FString, FString>(State.CountryId, MarketStart::HomeProvince(State)));
        for (const FMarketBranch& B : State.Branches)
        {
            if (!IsOpenBranch(B)) continue;
            const TPair<FString, FString> Place(MarketBranches::CountryOf(State, B), B.Province);
            if (!Places.Contains(Place)) Places.Add(Place);
        }
        O.Areas.RemoveAll([&Places](const FMarketOnlineArea& A) { return !Places.Contains(TPair<FString, FString>(A.Country, A.Province)); });
        for (const TPair<FString, FString>& Place : Places)
        {
            if (O.Areas.ContainsByPredicate([&Place](const FMarketOnlineArea& A) { return A.Country == Place.Key && A.Province == Place.Value; })) continue;
            FMarketOnlineArea A;
            A.Country = Place.Key;
            A.Province = Place.Value;
            const int32 Added = O.Areas.Add(A);
            Decide(State, Added, true);
        }
        if (bMonth) for (int32 I = 0; I < O.Areas.Num(); ++I) Review(State, I);
    }

    void Timeline(FMarketState& State, const TArray<FMarketProduct>& Products, int32 Day)
    {
        FMarketOnline& O = State.Online;
        auto Once = [&O](int32 Bit) { if (O.Told & Bit) return false; O.Told |= Bit; return true; };
        const int32 Web = MarketOnline::OpenDay(State, EChannel::Web);
        const int32 Platform = MarketOnline::OpenDay(State, EChannel::Platform);
        const int32 App = MarketOnline::OpenDay(State, EChannel::App);
        const int32 Quick = MarketOnline::OpenDay(State, EChannel::Quick);
        const int32 Start = MarketOnline::PandemicStart(State);
        if (Day >= Web - MarketOnline::RumourDays && Once(TWebRumour))
            Tell(State, TEXT("Kulak misafiri oldum: insanlar internetten market al\u0131\u015fveri\u015fini konu\u015fmaya ba\u015flad\u0131. Birka\u00e7 ay i\u00e7inde web sitesinden sipari\u015f almak m\u00fcmk\u00fcn olacak."));
        if (Day >= Web && Once(TWeb))
            Tell(State, FString::Printf(TEXT("\u0130nternetten market al\u0131\u015fveri\u015fi ba\u015flad\u0131. En az %d ma\u011fazam\u0131z olunca web sitesi kurabiliriz (kurulum %s); sipari\u015fleri ma\u011fazalar toplar, \u015firketin kuryeleri g\u00f6t\u00fcr\u00fcr."),
                MarketOnline::WebShops, *Tl(AtLevel(MarketOnline::WebSetupCost, Day))));
        if (Day >= Platform - 60 && Once(TPlatformRumour))
            Tell(State, FString::Printf(TEXT("%s ad\u0131nda bir h\u0131zl\u0131 teslimat platformu \u00fclkeye gelmeye haz\u0131rlan\u0131yor."), *MarketCast::Platform()));
        if (Day >= Platform && Once(TPlatform))
            Tell(State, FString::Printf(TEXT("%s geldi: sipari\u015fleri kendi kuryeleri ta\u015f\u0131yor, %%%.0f komisyon al\u0131yor. Aile d\u00fckk\u00e2n\u0131 bile kat\u0131labilir; girmezsek m\u00fc\u015fteri ba\u015fkas\u0131ndan s\u00f6yl\u00fcyor."),
                *MarketCast::Platform(), O.Commission * 100.f));
        if (Day >= App - MarketOnline::RumourDays && Once(TAppRumour))
            Tell(State, TEXT("Telefon uygulamalar\u0131yla market sipari\u015fi yay\u0131lmaya ba\u015fl\u0131yor; b\u00fcy\u00fck zincirler haz\u0131rlan\u0131yor. Uygulama i\u00e7in web sitesi ve en az 8 ma\u011faza gerekecek."));
        if (Day >= App && !(O.Told & TApp))
        {
            FString Reason;
            if (MarketOnline::CanOpen(State, EChannel::App, Reason)) { O.Told |= TApp; OfferApp(State); Tell(State, TEXT("Yaz\u0131l\u0131m firmalar\u0131 kap\u0131da: telefon uygulamam\u0131z i\u00e7in teklif verdiler.")); }
            else if (Day == App) Tell(State, FString::Printf(TEXT("Market uygulamalar\u0131 \u00e7\u0131kt\u0131. Bizim i\u00e7in de m\u00fcmk\u00fcn: %s"), *Reason));
        }
        if (Day >= Start && Once(TPandemic))
        {
            Tell(State, TEXT("Salg\u0131n d\u00f6nemi ba\u015flad\u0131: insanlar evde kal\u0131yor. Temel g\u0131da ve temizlik raflar\u0131 bo\u015fal\u0131yor, internet sipari\u015fleri patl\u0131yor."));
            MarketEvents::Offer(State, Card(State, TEXT("online.pandemic"), TEXT("Salg\u0131n: internete ge\u00e7elim mi?"),
                TEXT("Ma\u011fazalara gelen azal\u0131yor, sipari\u015f katlan\u0131yor. Her ilde platforma girebiliriz (komisyonlu ama hemen), kendi teslimat\u0131m\u0131z\u0131 d\u00f6rt ay i\u00e7in iki kat\u0131na \u00e7\u0131karabiliriz (kurye %30 pahal\u0131) ya da bekleriz."),
                { FString(TEXT("Her ilde platforma gir")), FString(TEXT("Kendi teslimat\u0131m\u0131z\u0131 g\u00fc\u00e7lendir")), FString(TEXT("Bekle")) }, 2, 5));
        }
        if (MarketOnline::IsCurfew(State, Day + 1) && !MarketOnline::IsCurfew(State, Day))
            State.DayNews.Add(TEXT("Yar\u0131n k\u0131s\u0131tlama g\u00fcn\u00fc: marketler k\u0131sa saatlerle a\u00e7\u0131k, ma\u011fazaya az ki\u015fi gelir; sipari\u015fler artar."));
        if (Day >= MarketOnline::PandemicEnd(State) && Once(TPandemicEnd))
            Tell(State, TEXT("K\u0131s\u0131tlamalar kalkt\u0131. \u0130nternetten al\u0131\u015fveri\u015fe al\u0131\u015fan m\u00fc\u015fterilerin bir k\u0131sm\u0131 geri d\u00f6nmeyecek; ma\u011fazalar\u0131n kendi i\u015fi de yava\u015f yava\u015f b\u00fcy\u00fcyecek."));
        if (Day >= Quick && Once(TQuick))
            Tell(State, FString::Printf(TEXT("30 dakikada teslimat d\u00f6nemi: uygulamas\u0131 olanlar il il karanl\u0131k depo kuruyor (bir ilde en az %d ma\u011faza)."), MarketOnline::DarkStoreShops));
        if (Day >= MarketOnline::PlatformMarketDay(State) && Once(TPlatformMarket))
        {
            Tell(State, FString::Printf(TEXT("%s kendi marketini a\u00e7t\u0131: art\u0131k hem kuryemiz hem rakibimiz."), *MarketCast::Platform()));
            if (O.bPlatform)
                MarketEvents::Offer(State, Card(State, TEXT("online.platformmarket"), FString::Printf(TEXT("%s rakip oldu"), *MarketCast::Platform()),
                    TEXT("Platform kendi marketini a\u00e7t\u0131. Devam edebiliriz, \u00e7\u0131kabiliriz ya da iki y\u0131ll\u0131k \u00f6zel anla\u015fma yapar\u0131z: komisyon 3 puan d\u00fc\u015fer ama iki y\u0131l ayr\u0131lamay\u0131z."),
                    { FString(TEXT("Devam et")), FString(TEXT("Platformdan \u00e7\u0131k")), FString(TEXT("\u00d6zel anla\u015fma")) }, 0, 7));
        }
        if (O.NextCommissionDay == 0) O.NextCommissionDay = Start + MarketOnline::CommissionAfter;
        if (Day >= O.NextCommissionDay)
        {
            O.NextCommissionDay = Day + MarketOnline::CommissionEvery;
            if (O.bPlatform && Day >= O.DealUntil)
                MarketEvents::Offer(State, Card(State, TEXT("online.commission"), TEXT("Komisyon art\u0131yor"),
                    FString::Printf(TEXT("%s komisyonu %%%.0f'den %%%.0f'e \u00e7\u0131karmak istiyor. Kabul edebilir, pazarl\u0131k edebilir (tutmazsa daha da artar) ya da platformdan \u00e7\u0131kabiliriz."),
                        *MarketCast::Platform(), O.Commission * 100.f, (O.Commission + 0.03f) * 100.f),
                    { FString(TEXT("Kabul et")), FString(TEXT("Pazarl\u0131k et")), FString(TEXT("Platformdan \u00e7\u0131k")) }, 0, 7));
        }
        // Rivals going online (the bigger ones; the assistant tells).
        for (const FMarketChain& C : State.Rivals.Chains)
        {
            if (C.bGone || C.bOurs || C.Country != State.CountryId || MarketChains::TotalStores(C) < 100) continue;
            const int32 Stage = RivalStage(State, C, Day);
            if (Stage == 0) continue;
            const FString Key = FString::Printf(TEXT("%s|%d"), *C.Id, Stage);
            if (O.RivalsTold.Contains(Key)) continue;
            O.RivalsTold.Add(Key);
            Tell(State, Stage == 1 ? FString::Printf(TEXT("%s art\u0131k web sitesinden sipari\u015f al\u0131yor."), *C.Name)
                : Stage == 2 ? FString::Printf(TEXT("%s telefon uygulamas\u0131n\u0131 \u00e7\u0131kard\u0131; m\u00fc\u015fteriler cepten sipari\u015f veriyor."), *C.Name)
                : FString::Printf(TEXT("%s 30 dakikada teslimat ba\u015flatt\u0131."), *C.Name));
        }
        // Once a month: what staying offline costs.
        const float Share = MarketOnline::OnlineShare(State, Day);
        if (MarketCalendar::DateOf(Day).Day == 1 && !O.bWeb && !O.bPlatform && Share >= 0.01f)
            Tell(State, FString::Printf(TEXT("\u00dclkede market al\u0131\u015fveri\u015finin %%%.1f'i internete kaym\u0131\u015f; biz orada yokuz, bu m\u00fc\u015fteriler kap\u0131m\u0131za u\u011framaz."), Share * 100.f));
        (void)Products;
    }

    void AutoPolicy(FMarketState& State)
    {
        FMarketOnline& O = State.Online;
        if (!O.bAutoPolicy || O.ManagerName.IsEmpty()) return;
        int32 OwnOrders = 0;
        int64 OwnProfit = 0;
        for (int32 C = 0; C < MarketOnline::ChannelCount; ++C)
            if (C != static_cast<int32>(EChannel::Platform) && O.PrevOrders.IsValidIndex(C)) { OwnOrders += O.PrevOrders[C]; OwnProfit += O.PrevProfit[C]; }
        O.Substitute = O.Reputation < 50.f ? 0 : 1;
        O.Fee = OwnOrders > 0 && OwnProfit < 0 ? 2 : 1;
        O.MinBasket = 1;
        O.PriceGap = O.ManagerSkill >= 60 ? 1 : 0;
        const int64 PerOrder = OwnOrders > 0 ? OwnProfit / OwnOrders : 0;
        const int64 Acquire = O.PrevNew > 0 ? O.PrevAds / O.PrevNew : MAX_int64;
        const int32 Top = FMath::Clamp(O.ManagerSkill / 30, 1, 3);
        if (O.PrevNew > 0 && PerOrder > 0 && Acquire < 4 * PerOrder) O.Ads = static_cast<uint8>(FMath::Min<int32>(O.Ads + 1, Top));
        else if (O.Ads > 0) O.Ads = static_cast<uint8>(O.Ads - 1);
        State.DayNews.Add(FString::Printf(TEXT("E-ticaret m\u00fcd\u00fcr\u00fc %s ay\u0131n politikas\u0131n\u0131 ayarlad\u0131 (reklam %d, teslimat \u00fccreti %d)."), *O.ManagerName, O.Ads, O.Fee));
    }

    void Grow(TArray<int32>& A) { if (A.Num() != MarketOnline::ChannelCount) A.Init(0, MarketOnline::ChannelCount); }
    void Grow(TArray<int64>& A) { if (A.Num() != MarketOnline::ChannelCount) A.Init(0, MarketOnline::ChannelCount); }
}

// ---------------------------------------------------------------------------------------------------------------
// Timeline and demand

FString MarketOnline::ChannelName(EChannel Channel)
{
    switch (Channel)
    {
    case EChannel::Web: return TEXT("Web sitesi");
    case EChannel::App: return TEXT("Telefon uygulamas\u0131");
    case EChannel::Platform: return MarketCast::Platform();
    case EChannel::Quick: return TEXT("H\u0131zl\u0131 teslimat");
    default: return TEXT("?");
    }
}

int32 MarketOnline::OpenDay(const FMarketState& State, EChannel Channel)
{
    const int32 Start = PandemicStart(State);
    switch (Channel)
    {
    case EChannel::Web: return FMath::Max(2, Start - WebBefore);
    case EChannel::Platform: return FMath::Max(2, Start - PlatformBefore);
    case EChannel::App: return FMath::Max(2, Start - AppBefore);
    case EChannel::Quick: return Start + QuickAfter;
    default: return MAX_int32;
    }
}

int32 MarketOnline::PlatformMarketDay(const FMarketState& State) { return PandemicStart(State) + PlatformMarketAfter; }

bool MarketOnline::Visible(const FMarketState& State)
{
    return State.Day >= OpenDay(State, EChannel::Web) - RumourDays || State.Online.bWeb || State.Online.bPlatform;
}

bool MarketOnline::IsOn(const FMarketState& State, EChannel Channel)
{
    const FMarketOnline& O = State.Online;
    switch (Channel)
    {
    case EChannel::Web: return O.bWeb;
    case EChannel::App: return O.bApp;
    case EChannel::Platform: return O.bPlatform;
    case EChannel::Quick: return O.bQuick;
    default: return false;
    }
}

float MarketOnline::OnlineShare(const FMarketState& State, int32 GameDay, const FString& Country)
{
    const float Top = MarketOnlineLocal::Plateau(State, Country);
    const int32 Start = PandemicStart(State), End = PandemicEnd(State);
    const int32 Web = Start - WebBefore;
    if (GameDay < Web) return 0.f;
    auto Before = [Top, Start, Web](int32 Day)
    {
        const float T = FMath::Clamp(static_cast<float>(Day - Web) / FMath::Max(1, Start - Web), 0.f, 1.f);
        float Share = 0.004f + (0.22f * Top - 0.004f) * T;
        const int32 App = Start - AppBefore;
        if (Day >= App) Share += 0.05f * Top * FMath::Clamp(static_cast<float>(Day - App) / 365.f, 0.f, 1.f);
        return Share;
    };
    float Share;
    if (GameDay < Start) Share = Before(GameDay);
    else if (GameDay < Start + 110) Share = Before(Start) * 3.5f;          // everyone stays home
    else if (GameDay < End) Share = Before(Start) * 2.5f;
    else
    {
        const float Left = Before(Start) * 2.5f;
        const float Years = static_cast<float>(GameDay - End) / 730.f;      // two-year steps
        if (Years <= 1.f) Share = Left + (1.15f * Top - Left) * Years;     // demand keeps moving online
        else if (Years <= 2.f) Share = 1.15f * Top - 0.15f * Top * (Years - 1.f); // the shops win some back: a balance
        else Share = Top * (1.f + 0.06f * FMath::Min(5.f, Years - 2.f));   // and slow growth
    }
    return FMath::Clamp(Share, 0.f, 0.4f);
}

int32 MarketOnline::PandemicStart(const FMarketState& State)
{
    // B4: the epidemic comes when the campaign's eras say (MarketEras; no shift without MarketEras::Setup).
    return MarketOnlineLocal::DateDay(2020, 3, 1) + MarketEras::PandemicShiftDays(State) + static_cast<int32>(MarketOnlineLocal::Roll(State, 0x9A00u) % 21u);
}

int32 MarketOnline::PandemicEnd(const FMarketState& State)
{
    return MarketOnlineLocal::DateDay(2021, 5, 20) + MarketEras::PandemicShiftDays(State) + static_cast<int32>(MarketOnlineLocal::Roll(State, 0x9A04u) % 50u);
}

bool MarketOnline::IsPandemic(const FMarketState& State, int32 GameDay)
{
    return GameDay >= PandemicStart(State) && GameDay < PandemicEnd(State);
}

bool MarketOnline::IsCurfew(const FMarketState& State, int32 GameDay)
{
    using namespace MarketOnlineLocal;
    if (!IsPandemic(State, GameDay)) return false;
    if (GameDay >= ClosureStart(State) && GameDay < ClosureEnd(State)) return true;   // the full closure
    if (!MarketCalendar::IsWeekend(GameDay)) return false;
    // Two waves of closure days; about two weekends in three, which ones differs per campaign.
    const int32 Start = PandemicStart(State);
    const bool bWave = (GameDay >= Start + 30 && GameDay < Start + 90) || (GameDay >= Start + 270 && GameDay < ClosureStart(State));
    const int32 Saturday = GameDay - (MarketCalendar::DateOf(GameDay).Weekday - 5);
    return bWave && Roll(State, 0x9A05u, Saturday) % 3u != 0u;
}

float MarketOnline::StoreTrafficFactor(const FMarketState& State) { return StoreTrafficFactorOn(State, State.Day); }

float MarketOnline::StoreTrafficFactorOn(const FMarketState& State, int32 GameDay, const FString& Country)
{
    float Factor = 1.f - OnlineShare(State, GameDay, Country);
    const int32 End = PandemicEnd(State);
    if (IsCurfew(State, GameDay)) Factor *= 0.4f;                          // open only a few hours
    else if (MarketOnlineLocal::IsPanic(State, GameDay)) Factor *= 1.25f;   // everyone stocks up
    else if (IsPandemic(State, GameDay)) Factor *= 0.85f;                  // people go out less
    else if (GameDay >= End)
    {
        // The shops' own trade grows back after the epidemic, then settles.
        const float Years = static_cast<float>(GameDay - End) / 365.f;
        const float Growth = Years < 1.f ? Years : FMath::Max(0.3f, 1.f - (Years - 1.f) * 0.35f);
        Factor *= 1.f + 0.06f * Growth;
    }
    return Factor;
}

float MarketOnline::GroupFactor(const FMarketState& State, MarketGoods::EGroup Group)
{
    using MarketGoods::EGroup;
    if (MarketOnlineLocal::IsPanic(State, State.Day))
        return Group == EGroup::Staples || Group == EGroup::Household || Group == EGroup::Paper || Group == EGroup::OilSauce ? 2.f : 1.f;
    if (IsPandemic(State, State.Day)) return Group == EGroup::Household ? 1.3f : 1.f;
    return 1.f;
}

float MarketOnline::Stars(const FMarketState& State) { return FMath::Clamp(1.f + State.Online.Reputation / 25.f, 1.f, 5.f); }

float MarketOnline::AppStars(const FMarketState& State)
{
    const float Quality = AppQuality[FMath::Min<int32>(State.Online.AppTier, 2)];
    return FMath::Clamp(1.f + State.Online.Reputation / 25.f * (0.7f + 0.3f * Quality), 1.f, 5.f);
}

int32 MarketOnline::ShopsIn(const FMarketState& State, const FString& Country, const FString& Province)
{
    int32 Shops = Country == State.CountryId && Province == MarketStart::HomeProvince(State) ? 1 : 0;
    for (const FMarketBranch& B : State.Branches)
        if (MarketOnlineLocal::IsOpenBranch(B) && B.Province == Province && MarketBranches::CountryOf(State, B) == Country) ++Shops;
    return Shops;
}

int32 MarketOnline::TotalShops(const FMarketState& State)
{
    int32 Shops = 1;
    for (const FMarketBranch& B : State.Branches) if (MarketOnlineLocal::IsOpenBranch(B)) ++Shops;
    return Shops;
}

// ---------------------------------------------------------------------------------------------------------------
// Channels

bool MarketOnline::CanOpen(const FMarketState& State, EChannel Channel, FString& OutReason)
{
    const FMarketOnline& O = State.Online;
    if (IsOn(State, Channel)) { OutReason = ChannelName(Channel) + TEXT(" zaten a\u00e7\u0131k."); return false; }
    if (State.Day < OpenDay(State, Channel))
    {
        OutReason = Channel == EChannel::Web ? FString(TEXT("Hen\u00fcz erken: insanlar internetten market al\u0131\u015fveri\u015fine al\u0131\u015fmad\u0131."))
            : Channel == EChannel::Platform ? FString::Printf(TEXT("%s hen\u00fcz bu \u00fclkede yok."), *MarketCast::Platform())
            : Channel == EChannel::App ? FString(TEXT("Telefon uygulamalar\u0131 hen\u00fcz market al\u0131\u015fveri\u015fine gelmedi."))
            : FString(TEXT("30 dakikada teslimat hen\u00fcz kimsede yok."));
        return false;
    }
    switch (Channel)
    {
    case EChannel::Web:
        if (TotalShops(State) < WebShops) { OutReason = FString::Printf(TEXT("Web sitesi i\u00e7in en az %d ma\u011faza gerekir (toplama ve kurye \u015firketin i\u015fi)."), WebShops); return false; }
        if (State.Cash < AtLevel(WebSetupCost, State.Day) + State.OtherCosts) { OutReason = FString::Printf(TEXT("Kurulum i\u00e7in kasada %s gerekiyor."), *MarketOnlineLocal::Tl(MarketOnlineLocal::AtLevel(WebSetupCost, State.Day))); return false; }
        return true;
    case EChannel::App:
        if (!O.bWeb) { OutReason = TEXT("\u00d6nce web sitesi gerekir."); return false; }
        if (TotalShops(State) < AppShops) { OutReason = FString::Printf(TEXT("Uygulama i\u00e7in en az %d ma\u011faza gerekir."), AppShops); return false; }
        return true;
    case EChannel::Quick:
        if (!O.bApp) { OutReason = TEXT("H\u0131zl\u0131 teslimat i\u00e7in \u00f6nce uygulama gerekir."); return false; }
        return true;
    default:
        return true;
    }
}

bool MarketOnline::Open(FMarketState& State, EChannel Channel, FString& OutMessage)
{
    using namespace MarketOnlineLocal;
    if (!CanOpen(State, Channel, OutMessage)) return false;
    FMarketOnline& O = State.Online;
    const int32 Day = State.Day;
    switch (Channel)
    {
    case EChannel::Web:
    {
        const int64 Cost = AtLevel(WebSetupCost, Day);
        State.OtherCosts += Cost;              // paid with today's costs at the day close
        O.bWeb = true;
        O.WebDay = Day;
        OutMessage = FString::Printf(TEXT("Web sitesi a\u00e7\u0131ld\u0131 (%s kurulum). Hangi illerde \u00e7al\u0131\u015faca\u011f\u0131na il m\u00fcd\u00fcrleri karar verir; insanlar\u0131n siteyi \u00f6\u011frenmesi iki ay s\u00fcrer."), *Tl(Cost));
        return true;
    }
    case EChannel::App:
        OfferApp(State);
        O.Told |= TApp;
        OutMessage = TEXT("Yaz\u0131l\u0131m firmalar\u0131ndan teklif istendi; kararlardan birini se\u00e7.");
        return true;
    case EChannel::Platform:
    {
        const int64 Cost = AtLevel(PlatformJoinCost, Day) * FMath::Max(1, O.Areas.Num());
        State.OtherCosts += Cost;
        O.bPlatform = true;
        OutMessage = FString::Printf(TEXT("%s ile anla\u015ft\u0131k (%s tablet ve listeleme). Sipari\u015fleri onlar\u0131n kuryesi al\u0131r, %%%.0f komisyon; y\u0131ld\u0131z\u0131m\u0131z d\u00fc\u015ferse sipari\u015f azal\u0131r."),
            *MarketCast::Platform(), *Tl(Cost), O.Commission * 100.f);
        return true;
    }
    case EChannel::Quick:
        O.bQuick = true;
        OutMessage = FString::Printf(TEXT("H\u0131zl\u0131 teslimat program\u0131 ba\u015flad\u0131: en az %d ma\u011fazam\u0131z olan illerde karanl\u0131k depo kurulunca 30 dakikada teslim ederiz."), DarkStoreShops);
        return true;
    default:
        return false;
    }
}

bool MarketOnline::Close(FMarketState& State, EChannel Channel, FString& OutMessage)
{
    FMarketOnline& O = State.Online;
    if (!IsOn(State, Channel)) { OutMessage = ChannelName(Channel) + TEXT(" zaten kapal\u0131."); return false; }
    if (Channel == EChannel::Platform && State.Day < O.DealUntil)
    {
        OutMessage = FString::Printf(TEXT("%s ile \u00f6zel anla\u015fma s\u00fcr\u00fcyor; %d g\u00fcn daha ayr\u0131lamay\u0131z."), *MarketCast::Platform(), O.DealUntil - State.Day);
        return false;
    }
    if (Channel == EChannel::Web) { O.bWeb = O.bApp = O.bQuick = false; }
    if (Channel == EChannel::App) { O.bApp = O.bQuick = false; }
    if (Channel == EChannel::Platform) O.bPlatform = false;
    if (Channel == EChannel::Quick) O.bQuick = false;
    OutMessage = ChannelName(Channel) + TEXT(" kapat\u0131ld\u0131. Bu kanal\u0131n m\u00fc\u015fterileri ba\u015fka yere al\u0131\u015f\u0131r.");
    return true;
}

bool MarketOnline::OpenApp(FMarketState& State, int32 Tier, FString& OutMessage)
{
    using namespace MarketOnlineLocal;
    FString Reason;
    if (!CanOpen(State, EChannel::App, Reason)) { OutMessage = Reason; return false; }
    const int32 T = FMath::Clamp(Tier, 0, 2);
    const int64 Cost = AtLevel(AppCosts[T], State.Day);
    if (State.Cash < Cost + State.OtherCosts) { OutMessage = FString::Printf(TEXT("Uygulama i\u00e7in kasada %s gerekiyor."), *Tl(Cost)); return false; }
    FMarketOnline& O = State.Online;
    State.OtherCosts += Cost;
    O.bApp = true;
    O.AppDay = State.Day;
    O.AppTier = static_cast<uint8>(T);
    O.AppCost = Cost;
    OutMessage = FString::Printf(TEXT("Telefon uygulamam\u0131z yay\u0131nda (%s). \u0130lk ay m\u00fc\u015fteriler indiriyor; her ay bak\u0131m\u0131 %s."), *Tl(Cost), *Tl(FMath::RoundToInt64(Cost * AppUpkeepMonthly)));
    return true;
}

// ---------------------------------------------------------------------------------------------------------------
// Areas

int32 MarketOnline::AreaFlags(const FMarketOnlineArea& Area) { return (Area.bPlatform ? 1 : 0) | (Area.bOwn ? 2 : 0) | (Area.bQuick ? 4 : 0); }

FString MarketOnline::AreaName(const FMarketOnlineArea& Area)
{
    const MarketCountry::FCity* City = MarketCountry::FindCity(Area.Country, Area.Province);
    return City ? City->Name : Area.Province;
}

FString MarketOnline::AreaDecider(const FMarketState& State, int32 AreaIndex)
{
    if (!State.Online.Areas.IsValidIndex(AreaIndex)) return FString();
    const FMarketOnlineArea& A = State.Online.Areas[AreaIndex];
    if (A.bPlayerSet) return TEXT("sen");
    const int32 Boss = MarketManagers::FindManager(State, MarketManagers::ELevel::Province, A.Country, A.Province);
    return Boss == INDEX_NONE ? FString() : State.Management.Managers[Boss].Name;
}

int32 MarketOnline::EncodeArea(int32 AreaIndex, int32 Flags) { return AreaIndex * 10 + FMath::Clamp(Flags, 0, 7); }

bool MarketOnline::DecodeArea(int32 Arg, int32& OutArea, int32& OutFlags)
{
    if (Arg < 0) return false;
    OutArea = Arg / 10;
    OutFlags = Arg % 10;
    return OutFlags <= 7;
}

bool MarketOnline::SetArea(FMarketState& State, int32 AreaIndex, int32 Flags, FString& OutMessage)
{
    if (!State.Online.Areas.IsValidIndex(AreaIndex)) { OutMessage = TEXT("B\u00f6yle bir il yok."); return false; }
    FMarketOnlineArea& A = State.Online.Areas[AreaIndex];
    if ((Flags & 4) && A.DarkStoreDay == 0) { OutMessage = TEXT("H\u0131zl\u0131 teslimat i\u00e7in bu ilde \u00f6nce karanl\u0131k depo gerekir."); return false; }
    A.bPlatform = (Flags & 1) != 0;
    A.bOwn = (Flags & 2) != 0;
    A.bQuick = A.bOwn && (Flags & 4) != 0;
    A.bPlayerSet = true;
    OutMessage = FString::Printf(TEXT("%s: internette %s. Bu ilde art\u0131k sen karar veriyorsun."), *AreaName(A), *MarketOnlineLocal::FlagText(AreaFlags(A)));
    return true;
}

bool MarketOnline::ReturnArea(FMarketState& State, int32 AreaIndex, FString& OutMessage)
{
    if (!State.Online.Areas.IsValidIndex(AreaIndex) || !State.Online.Areas[AreaIndex].bPlayerSet) { OutMessage = TEXT("Bu ilde zaten il m\u00fcd\u00fcr\u00fc ya da \u015firket kural\u0131 karar veriyor."); return false; }
    State.Online.Areas[AreaIndex].bPlayerSet = false;
    MarketOnlineLocal::Decide(State, AreaIndex, true);
    const FString Who = AreaDecider(State, AreaIndex);
    OutMessage = FString::Printf(TEXT("%s: karar yeniden %s."), *AreaName(State.Online.Areas[AreaIndex]), Who.IsEmpty() ? TEXT("\u015firket kural\u0131nda") : *(Who + TEXT(" adl\u0131 il m\u00fcd\u00fcr\u00fcnde")));
    return true;
}

bool MarketOnline::SetDefault(FMarketState& State, int32 Flags, FString& OutMessage)
{
    FMarketOnline& O = State.Online;
    O.bDefaultPlatform = (Flags & 1) != 0;
    O.bDefaultOwn = (Flags & 2) != 0;
    O.bDefaultQuick = (Flags & 4) != 0;
    // Only the provinces without a manager follow the rule at once (a manager decides at the month's start).
    for (int32 I = 0; I < O.Areas.Num(); ++I)
        if (MarketManagers::FindManager(State, MarketManagers::ELevel::Province, O.Areas[I].Country, O.Areas[I].Province) == INDEX_NONE) MarketOnlineLocal::Decide(State, I, true);
    OutMessage = FString::Printf(TEXT("\u0130l m\u00fcd\u00fcr\u00fc olmayan illerde kural: %s."), *MarketOnlineLocal::FlagText((O.bDefaultPlatform ? 1 : 0) | (O.bDefaultOwn ? 2 : 0) | (O.bDefaultQuick ? 4 : 0)));
    return true;
}

int64 MarketOnline::DarkStorePrice(const FMarketState& State, int32 AreaIndex)
{
    if (!State.Online.Areas.IsValidIndex(AreaIndex)) return 0;
    const FMarketOnlineArea& A = State.Online.Areas[AreaIndex];
    const MarketCountry::FCity* City = MarketCountry::FindCity(A.Country, A.Province);
    return FMath::RoundToInt64(DarkStoreCost * MarketOnlineLocal::LevelOn(State.Day) * (City ? FMath::Clamp(City->Rent, 0.4f, 2.5f) : 1.f));
}

bool MarketOnline::CanBuildDarkStore(const FMarketState& State, int32 AreaIndex, FString& OutReason)
{
    if (!State.Online.Areas.IsValidIndex(AreaIndex)) { OutReason = TEXT("B\u00f6yle bir il yok."); return false; }
    const FMarketOnlineArea& A = State.Online.Areas[AreaIndex];
    if (A.DarkStoreDay > 0) { OutReason = TEXT("Bu ilde karanl\u0131k depo zaten var."); return false; }
    if (!State.Online.bQuick) { OutReason = TEXT("\u00d6nce h\u0131zl\u0131 teslimat program\u0131 a\u00e7\u0131lmal\u0131 (uygulama gerekir)."); return false; }
    if (ShopsIn(State, A.Country, A.Province) < DarkStoreShops) { OutReason = FString::Printf(TEXT("Karanl\u0131k depo i\u00e7in bu ilde en az %d ma\u011fazam\u0131z olmal\u0131."), DarkStoreShops); return false; }
    const int64 Price = DarkStorePrice(State, AreaIndex);
    if (State.Cash < Price + State.OtherCosts) { OutReason = FString::Printf(TEXT("Karanl\u0131k depo i\u00e7in kasada %s gerekiyor."), *MarketOnlineLocal::Tl(Price)); return false; }
    return true;
}

bool MarketOnline::BuildDarkStore(FMarketState& State, int32 AreaIndex, FString& OutMessage)
{
    if (!CanBuildDarkStore(State, AreaIndex, OutMessage)) return false;
    const int64 Price = DarkStorePrice(State, AreaIndex);
    FMarketOnlineArea& A = State.Online.Areas[AreaIndex];
    State.Cash -= Price;
    MarketLedger::Post(State, MarketLedger::EAccount::Investment, -Price, true, MarketLedger::HeadOfficeStore);
    A.DarkStoreDay = State.Day;
    if (!A.bPlayerSet) MarketOnlineLocal::Decide(State, AreaIndex, true);   // the province's rule sees the new dark store
    OutMessage = FString::Printf(TEXT("%s karanl\u0131k deposu kuruldu (%s): yaln\u0131z sipari\u015f toplar, 30 dakikada teslim eder."), *AreaName(A), *MarketOnlineLocal::Tl(Price));
    return true;
}

// ---------------------------------------------------------------------------------------------------------------
// Policy and the e-commerce manager

bool MarketOnline::SetFee(FMarketState& State, int32 Level, FString& OutMessage)
{
    State.Online.Fee = static_cast<uint8>(FMath::Clamp(Level, 0, 2));
    static const TCHAR* Text[3] = { TEXT("\u00fccretsiz (daha \u00e7ok sipari\u015f, kurye bize kal\u0131r)"), TEXT("k\u00fc\u00e7\u00fck sepete \u00fccret"), TEXT("her sipari\u015fe \u00fccret (daha az sipari\u015f)") };
    OutMessage = FString::Printf(TEXT("Teslimat: %s."), Text[State.Online.Fee]);
    return true;
}

bool MarketOnline::SetMinBasket(FMarketState& State, int32 Level, FString& OutMessage)
{
    State.Online.MinBasket = static_cast<uint8>(FMath::Clamp(Level, 0, 2));
    static const TCHAR* Text[3] = { TEXT("yok"), TEXT("k\u00fc\u00e7\u00fck"), TEXT("b\u00fcy\u00fck (az ama dolu sepet)") };
    OutMessage = FString::Printf(TEXT("En az sepet: %s."), Text[State.Online.MinBasket]);
    return true;
}

bool MarketOnline::SetPriceGap(FMarketState& State, int32 Level, FString& OutMessage)
{
    State.Online.PriceGap = static_cast<uint8>(FMath::Clamp(Level, 0, 2));
    static const TCHAR* Text[3] = { TEXT("rafla ayn\u0131"), TEXT("%5 fazla"), TEXT("%10 fazla") };
    OutMessage = FString::Printf(TEXT("\u0130nternet fiyatlar\u0131: %s."), Text[State.Online.PriceGap]);
    return true;
}

bool MarketOnline::SetAds(FMarketState& State, int32 Level, FString& OutMessage)
{
    State.Online.Ads = static_cast<uint8>(FMath::Clamp(Level, 0, 3));
    OutMessage = FString::Printf(TEXT("\u0130nternet reklam\u0131: ayda %s."), *MarketOnlineLocal::Tl(MarketOnlineLocal::AtLevel(AdsMonthly[State.Online.Ads], State.Day)));
    return true;
}

bool MarketOnline::SetSubstitute(FMarketState& State, int32 Rule, FString& OutMessage)
{
    State.Online.Substitute = static_cast<uint8>(FMath::Clamp(Rule, 0, 2));
    static const TCHAR* Names[3] = { TEXT("m\u00fc\u015fteriyi aray\u0131p sor"), TEXT("ayn\u0131 reyondan benzerini koy"), TEXT("\u00e7\u0131kar, paray\u0131 alma") };
    OutMessage = FString::Printf(TEXT("Eksik \u00fcr\u00fcnde: %s."), Names[State.Online.Substitute]);
    return true;
}

bool MarketOnline::SetAutoPolicy(FMarketState& State, bool bAuto, FString& OutMessage)
{
    if (bAuto && State.Online.ManagerName.IsEmpty()) { OutMessage = TEXT("Politikay\u0131 b\u0131rakmak i\u00e7in \u00f6nce e-ticaret m\u00fcd\u00fcr\u00fc gerekir."); return false; }
    State.Online.bAutoPolicy = bAuto;
    OutMessage = bAuto ? FString::Printf(TEXT("\u0130nternet politikas\u0131n\u0131 her ay %s ayarlar."), *State.Online.ManagerName) : FString(TEXT("\u0130nternet politikas\u0131n\u0131 sen ayarl\u0131yorsun."));
    return true;
}

void MarketOnline::Candidate(const FMarketState& State, FString& OutName, int32& OutSkill, int64& OutWage)
{
    const int32 Week = (State.Day - 1) / 7;
    const uint32 H = MarketOnlineLocal::Mix(State.RivalSeed, Week, 0xEC0Bu);
    const MarketCountry::FProfile& Pack = MarketCountry::Active();
    const FString First = Pack.FirstNames.Num() ? Pack.FirstNames[H % static_cast<uint32>(Pack.FirstNames.Num())] : FString(TEXT("Deniz"));
    const FString Last = Pack.LastNames.Num() ? Pack.LastNames[(H >> 8) % static_cast<uint32>(Pack.LastNames.Num())] : FString(TEXT("Y\u0131lmaz"));
    OutName = First + TEXT(" ") + Last;
    OutSkill = 45 + static_cast<int32>((H >> 16) % 41u);
    OutWage = MarketStaff::FairWage(MarketStaff::ERole::HrManager, OutSkill, State.Day) * 13 / 10;
}

bool MarketOnline::CanHireManager(const FMarketState& State, FString& OutReason)
{
    if (!State.Online.ManagerName.IsEmpty()) { OutReason = TEXT("E-ticaret m\u00fcd\u00fcr\u00fc zaten var."); return false; }
    if (!State.Online.bWeb && !State.Online.bPlatform) { OutReason = TEXT("\u00d6nce internetten sat\u0131\u015f ba\u015flamal\u0131."); return false; }
    if (TotalShops(State) < ManagerShops && !State.Online.bApp) { OutReason = FString::Printf(TEXT("E-ticaret m\u00fcd\u00fcr\u00fc i\u00e7in en az %d ma\u011faza ya da uygulama gerekir."), ManagerShops); return false; }
    return true;
}

bool MarketOnline::HireManager(FMarketState& State, FString& OutMessage)
{
    if (!CanHireManager(State, OutMessage)) return false;
    FMarketOnline& O = State.Online;
    Candidate(State, O.ManagerName, O.ManagerSkill, O.ManagerWage);
    O.ManagerSince = State.Day;
    OutMessage = FString::Printf(TEXT("E-ticaret m\u00fcd\u00fcr\u00fc %s i\u015fe ba\u015flad\u0131 (beceri %d, g\u00fcnl\u00fck %s): y\u0131ld\u0131zlar\u0131 ve toplamay\u0131 iyile\u015ftirir, istersen politikay\u0131 o ayarlar."),
        *O.ManagerName, O.ManagerSkill, *MarketOnlineLocal::Tl(O.ManagerWage));
    return true;
}

bool MarketOnline::FireManager(FMarketState& State, FString& OutMessage)
{
    FMarketOnline& O = State.Online;
    if (O.ManagerName.IsEmpty()) { OutMessage = TEXT("E-ticaret m\u00fcd\u00fcr\u00fc yok."); return false; }
    const int64 Severance = O.ManagerWage * 10 + MarketStaff::SeniorityPay(O.ManagerWage, O.ManagerSince, State.Day);
    if (State.Cash < Severance + State.OtherCosts) { OutMessage = FString::Printf(TEXT("Tazminat i\u00e7in kasada %s gerekiyor."), *MarketOnlineLocal::Tl(Severance)); return false; }
    State.OtherCosts += Severance;
    OutMessage = FString::Printf(TEXT("%s ayr\u0131ld\u0131, tazminat %s."), *O.ManagerName, *MarketOnlineLocal::Tl(Severance));
    O.ManagerName.Reset();
    O.ManagerSkill = 0;
    O.ManagerWage = 0;
    O.bAutoPolicy = false;
    return true;
}

// ---------------------------------------------------------------------------------------------------------------
// Rivals and menu lines

float MarketOnline::RivalOnline(const FMarketState& State, const FString& Country, int32 GameDay)
{
    const FString Where = MarketOnlineLocal::CountryOr(State, Country);
    float Sum = 0.f;
    for (const FMarketChain& C : State.Rivals.Chains)
    {
        if (C.bGone || C.bOurs || C.Country != Where) continue;
        const int32 Stores = MarketChains::TotalStores(C);
        if (Stores < 20) continue;
        const int32 Stage = MarketOnlineLocal::RivalStage(State, C, GameDay);
        static const float Weight[4] = { 0.f, 0.35f, 0.7f, 1.f };
        Sum += Weight[Stage] * FMath::Clamp(Stores / 600.f, 0.15f, 1.f);
    }
    if (GameDay >= PlatformMarketDay(State)) Sum += 0.5f;   // the platform's own market
    return FMath::Min(Sum, 3.f);
}

TArray<FString> MarketOnline::RivalLines(const FMarketState& State)
{
    TArray<FString> Lines;
    static const TCHAR* Stages[4] = { TEXT(""), TEXT("web"), TEXT("web + uygulama"), TEXT("web + uygulama + h\u0131zl\u0131 teslimat") };
    for (const FMarketChain& C : State.Rivals.Chains)
    {
        if (C.bGone || C.bOurs || C.Country != State.CountryId || MarketChains::TotalStores(C) < 20) continue;
        const int32 Stage = MarketOnlineLocal::RivalStage(State, C, State.Day);
        if (Stage > 0) Lines.Add(FString::Printf(TEXT("%s: %s"), *C.Name, Stages[Stage]));
    }
    if (State.Day >= PlatformMarketDay(State)) Lines.Add(FString::Printf(TEXT("%s: kendi marketi"), *MarketCast::Platform()));
    return Lines;
}

FString MarketOnline::Summary(const FMarketState& State)
{
    const FMarketOnline& O = State.Online;
    TArray<FString> On;
    if (O.bWeb) On.Add(ChannelName(EChannel::Web));
    if (O.bApp) On.Add(FString::Printf(TEXT("uygulama (%.1f y\u0131ld\u0131z)"), AppStars(State)));
    if (O.bPlatform) On.Add(FString::Printf(TEXT("%s (%%%.0f, %.1f y\u0131ld\u0131z)"), *MarketCast::Platform(), O.Commission * 100.f, Stars(State)));
    if (O.bQuick) On.Add(ChannelName(EChannel::Quick));
    FString Line = On.Num() ? FString::Join(On, TEXT(", ")) : FString(TEXT("\u0130nternette yokuz"));
    if (O.LastOrders > 0 || O.LastCancelled > 0) Line += FString::Printf(TEXT("  \u00b7  d\u00fcn %d sipari\u015f (%d ge\u00e7, %d iptal), net %s"), O.LastOrders, O.LastLate, O.LastCancelled, *MarketOnlineLocal::Tl(O.LastProfit));
    Line += FString::Printf(TEXT("  \u00b7  \u00fclkede internet pay\u0131 %%%.1f"), OnlineShare(State, State.Day) * 100.f);
    return Line;
}

FString MarketOnline::ChannelLine(const FMarketState& State, EChannel Channel)
{
    using namespace MarketOnlineLocal;
    const FMarketOnline& O = State.Online;
    const int32 Day = State.Day;
    if (IsOn(State, Channel))
    {
        switch (Channel)
        {
        case EChannel::Web: return FString::Printf(TEXT("A\u00e7\u0131k \u00b7 ayda %s bar\u0131nd\u0131rma \u00b7 kartla \u00f6demede %%%.1f"), *Tl(AtLevel(WebMonthly, Day)), CardCommission * 100.f);
        case EChannel::App:
        {
            static const TCHAR* Names[3] = { TEXT("ucuz firma"), TEXT("sa\u011flam firma"), TEXT("se\u00e7kin firma") };
            return FString::Printf(TEXT("A\u00e7\u0131k \u00b7 %s \u00b7 %.1f y\u0131ld\u0131z \u00b7 ayda %s bak\u0131m"), Names[FMath::Min<int32>(O.AppTier, 2)], AppStars(State), *Tl(FMath::RoundToInt64(O.AppCost * AppUpkeepMonthly)));
        }
        case EChannel::Platform:
            return FString::Printf(TEXT("A\u00e7\u0131k \u00b7 komisyon %%%.0f%s \u00b7 %.1f y\u0131ld\u0131z"), (O.Commission - (Day < O.DealUntil ? 0.03f : 0.f)) * 100.f,
                Day < O.DealUntil ? *FString::Printf(TEXT(" (\u00f6zel anla\u015fma, %d g\u00fcn)"), O.DealUntil - Day) : TEXT(""), Stars(State));
        case EChannel::Quick:
        {
            int32 Stores = 0;
            for (const FMarketOnlineArea& A : O.Areas) if (A.DarkStoreDay > 0) ++Stores;
            return FString::Printf(TEXT("A\u00e7\u0131k \u00b7 %d karanl\u0131k depo"), Stores);
        }
        default: return FString();
        }
    }
    FString Reason;
    if (!CanOpen(State, Channel, Reason)) return Reason;
    switch (Channel)
    {
    case EChannel::Web: return FString::Printf(TEXT("Kurulabilir: %s kurulum."), *Tl(AtLevel(WebSetupCost, Day)));
    case EChannel::App: return FString::Printf(TEXT("Yapt\u0131r\u0131labilir: %s ile %s aras\u0131."), *Tl(AtLevel(AppCosts[0], Day)), *Tl(AtLevel(AppCosts[2], Day)));
    case EChannel::Platform: return FString::Printf(TEXT("Kat\u0131labilirsin: il ba\u015f\u0131na %s, %%%.0f komisyon."), *Tl(AtLevel(PlatformJoinCost, Day)), O.Commission * 100.f);
    case EChannel::Quick: return FString::Printf(TEXT("Ba\u015flat\u0131labilir: il ba\u015f\u0131na karanl\u0131k depo (en az %d ma\u011faza)."), DarkStoreShops);
    default: return FString();
    }
}

FString MarketOnline::ChannelStats(const FMarketState& State, EChannel Channel)
{
    const FMarketOnline& O = State.Online;
    const int32 C = static_cast<int32>(Channel);
    if (!O.PrevOrders.IsValidIndex(C) || O.PrevOrders[C] <= 0) return FString::Printf(TEXT("%s: ge\u00e7en ay sipari\u015f yok."), *ChannelName(Channel));
    return FString::Printf(TEXT("%s: ge\u00e7en ay %d sipari\u015f, ciro %s, k\u00e2r %s (sipari\u015f ba\u015f\u0131 %s)."), *ChannelName(Channel), O.PrevOrders[C],
        *MarketOnlineLocal::Tl(O.PrevRevenue[C]), *MarketOnlineLocal::Tl(O.PrevProfit[C]), *MarketOnlineLocal::Tl(O.PrevProfit[C] / O.PrevOrders[C]));
}

FString MarketOnline::AreaLine(const FMarketState& State, int32 AreaIndex)
{
    if (!State.Online.Areas.IsValidIndex(AreaIndex)) return FString();
    const FMarketOnlineArea& A = State.Online.Areas[AreaIndex];
    const FString Who = AreaDecider(State, AreaIndex);
    return FString::Printf(TEXT("%s \u00b7 %d ma\u011faza \u00b7 %s%s \u00b7 karar: %s \u00b7 30 g\u00fcn ~%d sipari\u015f, k\u00e2r %s"), *AreaName(A), ShopsIn(State, A.Country, A.Province),
        *MarketOnlineLocal::FlagText(AreaFlags(A)), A.DarkStoreDay > 0 ? TEXT(" \u00b7 karanl\u0131k depo") : TEXT(""),
        Who.IsEmpty() ? TEXT("\u015firket kural\u0131") : *Who, A.Orders30, *MarketOnlineLocal::Tl(A.Profit30));
}

FString MarketOnline::AdsLine(const FMarketState& State)
{
    const FMarketOnline& O = State.Online;
    if (O.PrevAds <= 0 && O.PrevNew <= 0) return TEXT("Ge\u00e7en ay reklam yok.");
    return FString::Printf(TEXT("Ge\u00e7en ay reklam %s, yeni m\u00fc\u015fteri %d%s."), *MarketOnlineLocal::Tl(O.PrevAds), O.PrevNew,
        O.PrevNew > 0 && O.PrevAds > 0 ? *FString::Printf(TEXT(", birinin maliyeti %s"), *MarketOnlineLocal::Tl(O.PrevAds / O.PrevNew)) : TEXT(""));
}

FString MarketOnline::Hint(const FMarketState& State)
{
    return State.Online.Hint.IsEmpty() || State.Day - State.Online.HintDay > 21 ? FString() : State.Online.Hint;
}

float MarketOnline::ExpectedOrders(const FMarketState& State)
{
    float Sum = 0.f;
    const FString Home = MarketStart::HomeProvince(State);
    for (const FMarketOnlineArea& A : State.Online.Areas)
    {
        MarketOnlineLocal::FPull Pull;
        if (A.Country == State.CountryId && A.Province == Home) Sum += MarketOnlineLocal::Expected(State, A, State.LastServed + State.LastLost, State.Day, Pull);
        for (const FMarketBranch& B : State.Branches)
            if (MarketOnlineLocal::IsOpenBranch(B) && B.Province == A.Province && MarketBranches::CountryOf(State, B) == A.Country)
                Sum += MarketOnlineLocal::Expected(State, A, B.LastShoppers, State.Day, Pull);
    }
    return Sum;
}

// ---------------------------------------------------------------------------------------------------------------
// Decisions

bool MarketOnline::Resolve(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& D, int32 Option, FString& OutMessage)
{
    FMarketOnline& O = State.Online;
    if (D.Id == TEXT("online.app"))
    {
        if (Option >= 0 && Option <= 2) { OpenApp(State, Option, OutMessage); return true; }
        OutMessage = TEXT("Uygulamay\u0131 sonraya b\u0131rakt\u0131k; Sat\u0131\u015f kanallar\u0131ndan istedi\u011fin zaman yeniden teklif isteyebilirsin. Rakipler beklemeyecek.");
        return true;
    }
    if (D.Id.StartsWith(TEXT("online.area:")))
    {
        const FString Key = D.Id.RightChop(12);
        const int32 Index = O.Areas.IndexOfByPredicate([&Key](const FMarketOnlineArea& A) { return MarketOnlineLocal::AreaKey(A) == Key; });
        if (Index == INDEX_NONE) { OutMessage = TEXT("O ilde art\u0131k ma\u011fazam\u0131z yok."); return true; }
        FMarketOnlineArea& A = O.Areas[Index];
        if (Option != 0)
        {
            A.QuietUntil = State.Day + 90;
            OutMessage = FString::Printf(TEXT("%s: \u00f6neri reddedildi; il m\u00fcd\u00fcr\u00fc \u00fc\u00e7 ay bu konuyu a\u00e7maz."), *AreaName(A));
            return true;
        }
        if ((D.Arg & 8) && A.DarkStoreDay == 0)
        {
            FString Built;
            if (!BuildDarkStore(State, Index, Built)) { OutMessage = Built; return true; }
        }
        FMarketOnlineArea& Same = O.Areas[Index];
        Same.bPlatform = (D.Arg & 1) != 0;
        Same.bOwn = (D.Arg & 2) != 0;
        Same.bQuick = Same.bOwn && (D.Arg & 4) != 0 && Same.DarkStoreDay > 0;
        Same.PlatformLossMonths = Same.OwnLossMonths = 0;
        OutMessage = FString::Printf(TEXT("%s: onayland\u0131, internette %s."), *AreaName(Same), *MarketOnlineLocal::FlagText(AreaFlags(Same)));
        return true;
    }
    if (D.Id == TEXT("online.pandemic"))
    {
        if (Option == 0)
        {
            if (!O.bPlatform && State.Day >= OpenDay(State, EChannel::Platform)) { FString Ignored; Open(State, EChannel::Platform, Ignored); }
            O.bDefaultPlatform = true;
            for (FMarketOnlineArea& A : O.Areas) if (!A.bPlayerSet) A.bPlatform = true;
            OutMessage = O.bPlatform ? FString::Printf(TEXT("B\u00fct\u00fcn illerde %s'dan sipari\u015f al\u0131yoruz."), *MarketCast::Platform()) : FString(TEXT("Platform bu \u00fclkede hen\u00fcz yok."));
        }
        else if (Option == 1)
        {
            if (!O.bWeb) OutMessage = TEXT("Kendi teslimat\u0131m\u0131z yok (web sitesi gerekir); bekliyoruz.");
            else { O.SurgeUntil = State.Day + 120; OutMessage = TEXT("D\u00f6rt ay boyunca kurye ve toplay\u0131c\u0131 iki kat\u0131 (kurye %30 pahal\u0131)."); }
        }
        else OutMessage = TEXT("Bekliyoruz.");
        return true;
    }
    if (D.Id == TEXT("online.platformmarket"))
    {
        if (Option == 1) { O.bPlatform = false; OutMessage = FString::Printf(TEXT("%s'dan \u00e7\u0131kt\u0131k."), *MarketCast::Platform()); }
        else if (Option == 2) { O.DealUntil = State.Day + 730; OutMessage = TEXT("\u00d6zel anla\u015fma: iki y\u0131l komisyon 3 puan d\u00fc\u015f\u00fck, ayr\u0131lamay\u0131z."); }
        else OutMessage = TEXT("Platformda devam.");
        return true;
    }
    if (D.Id == TEXT("online.commission"))
    {
        if (Option == 0) { O.Commission = FMath::Min(0.35f, O.Commission + 0.03f); OutMessage = FString::Printf(TEXT("Komisyon %%%.0f oldu."), O.Commission * 100.f); }
        else if (Option == 1)
        {
            const bool bWon = MarketOnlineLocal::Mix(State.RivalSeed, State.Day, 0xC0DEu) % 2u == 0u;
            if (!bWon) O.Commission = FMath::Min(0.35f, O.Commission + 0.05f);
            OutMessage = bWon ? FString(TEXT("Pazarl\u0131k tuttu: komisyon ayn\u0131 kald\u0131.")) : FString::Printf(TEXT("Pazarl\u0131k tutmad\u0131: komisyon %%%.0f."), O.Commission * 100.f);
        }
        else { O.bPlatform = false; OutMessage = FString::Printf(TEXT("%s'dan \u00e7\u0131kt\u0131k."), *MarketCast::Platform()); }
        return true;
    }
    (void)Products;
    OutMessage = TEXT("Bu karar art\u0131k ge\u00e7erli de\u011fil.");
    return true;
}

// ---------------------------------------------------------------------------------------------------------------
// The day

void MarketOnline::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    using namespace MarketOnlineLocal;
    FMarketOnline& O = State.Online;
    const int32 Closed = State.Day - 1;
    if (Closed < 1) return;
    Grow(O.MonthOrders); Grow(O.MonthRevenue); Grow(O.MonthProfit); Grow(O.PrevOrders); Grow(O.PrevRevenue); Grow(O.PrevProfit);
    const bool bNewMonth = MarketCalendar::DateOf(State.Day).Day == 1;
    Timeline(State, Products, Closed);
    KeepAreas(State, bNewMonth);

    O.LastOrders = O.LastLate = O.LastCancelled = O.LastMissing = O.LastSubstituted = 0;
    O.LastRevenue = O.LastCosts = O.LastProfit = O.LastBranchProfit = 0;
    for (FMarketOnlineArea& A : O.Areas) { A.Revenue30 = A.Revenue30 * 29 / 30; A.Profit30 = A.Profit30 * 29 / 30; A.LastOrders = 0; }
    const double Level = LevelOn(Closed);

    // Fixed costs of the head office.
    int64 Fixed = 0;
    if (O.bWeb) Fixed += FMath::RoundToInt64(WebMonthly * Level / 30.0);
    if (O.bApp) Fixed += FMath::RoundToInt64(O.AppCost * AppUpkeepMonthly / 30.0);
    for (const FMarketOnlineArea& A : O.Areas)
        if (A.DarkStoreDay > 0)
        {
            const MarketCountry::FCity* City = MarketCountry::FindCity(A.Country, A.Province);
            Fixed += FMath::RoundToInt64(DarkStoreMonthly * Level * (City ? FMath::Clamp(City->Rent, 0.4f, 2.5f) : 1.f) / 30.0);
        }
    const int64 Ads = (O.bWeb || O.bPlatform) ? FMath::RoundToInt64(AdsMonthly[FMath::Min<int32>(O.Ads, 3)] * Level / 30.0) : 0;
    Fixed += Ads;
    O.MonthAds += Ads;
    if (!O.ManagerName.IsEmpty()) Fixed += MarketStaff::EmployerCost(O.ManagerWage);
    // The manager's touch: the stars drift to his skill, picking is quicker.
    if (!O.ManagerName.IsEmpty()) O.Reputation += (O.ManagerSkill - 50) / 500.f;
    const float Hands = (O.ManagerName.IsEmpty() ? 1.f : 1.f + O.ManagerSkill / 200.f) * (Closed < O.SurgeUntil ? 2.f : 1.f);

    const int64 Courier = FMath::RoundToInt64(MarketStaff::EmployerCost(WageAt(CourierDailyWage, Closed)) / static_cast<double>(OrdersPerCourier) * (Closed < O.SurgeUntil ? 1.3 : 1.0));
    const int64 Packaging = FMath::RoundToInt64(PackagingPerOrder * Level);
    const float Commission = O.Commission - (Closed < O.DealUntil ? 0.03f : 0.f);
    static const float Gap[3] = { 1.f, 1.05f, 1.1f };
    const float PriceGap = Gap[FMath::Min<int32>(O.PriceGap, 2)];

    // The shops of every area.
    const FString Home = MarketStart::HomeProvince(State);
    TArray<FShop> Shops;
    for (int32 AreaIndex = 0; AreaIndex < O.Areas.Num(); ++AreaIndex)
    {
        const FMarketOnlineArea& A = O.Areas[AreaIndex];
        if (A.Country == State.CountryId && A.Province == Home)
        {
            int32 Workers = 0;
            for (const FMarketEmployee& E : State.Staff) if (MarketStaff::RoleOf(E) != MarketStaff::ERole::Accountant) ++Workers;
            FShop Shop; Shop.Area = AreaIndex; Shop.Walkins = State.LastServed + State.LastLost; Shop.Pick = FMath::RoundToInt32((6 + 3 * Workers) * Hands);
            Shops.Add(Shop);
        }
        for (int32 I = 0; I < State.Branches.Num(); ++I)
        {
            const FMarketBranch& B = State.Branches[I];
            if (!IsOpenBranch(B) || B.Province != A.Province || MarketBranches::CountryOf(State, B) != A.Country) continue;
            FShop Shop; Shop.Branch = I; Shop.Area = AreaIndex; Shop.Walkins = B.LastShoppers; Shop.Pick = FMath::RoundToInt32((8 + 3 * B.Workers) * Hands);
            Shops.Add(Shop);
        }
    }

    const bool bStock = State.Stock.Num() == Products.Num() && Products.Num() > 0;
    State.Ledger.OnlineSold.Init(0, State.Stock.Num());   // B1 (#30): the family shop's online units by catalog row
    TArray<int32> QuickPicked;
    QuickPicked.Init(0, O.Areas.Num());
    int64 FamilyRevenue = 0, FamilyCogs = 0, FamilyCosts = 0, Units = 0;

    for (int32 ShopIndex = 0; ShopIndex < Shops.Num(); ++ShopIndex)
    {
        const FShop& Shop = Shops[ShopIndex];
        FMarketOnlineArea& A = O.Areas[Shop.Area];
        FPull Pull;
        const float Want = Expected(State, A, Shop.Walkins, Closed, Pull);
        if (Want <= 0.f || !bStock) continue;
        FRandomStream Random(static_cast<int32>(Mix(State.RivalSeed, Closed, 0x0A11u + static_cast<uint32>(Shop.Branch + 2) * 7919u)));
        FMarketBranch* Branch = Shop.Branch == INDEX_NONE ? nullptr : &State.Branches[Shop.Branch];

        // What this shop can pick from.
        TArray<float> Weights;
        Weights.Init(0.f, Products.Num());
        float Total = 0.f;
        for (int32 I = 0; I < Products.Num(); ++I)
        {
            bool bCarried;
            if (Branch) { const FMarketBranchItem* Item = ItemAt(*Branch, Products, I); bCarried = Item && Item->Capacity > 0; }
            else bCarried = State.Stock.IsValidIndex(I) && State.Stock[I].Capacity > 0;   // B1 (#30): only what the shop carries
            if (!Products[I].bActive || !bCarried) continue;
            Weights[I] = MarketCalendar::CategoryFactor(Closed, State.RivalSeed, Products[I].Category) * GroupFactor(State, MarketGoods::Classify(Products[I].Category));
            Total += Weights[I];
        }
        if (Total <= 0.f) continue;
        auto Available = [&State, Branch, &Products](int32 I) -> int32
        {
            if (Branch) { const FMarketBranchItem* Item = ItemAt(*Branch, Products, I); return Item ? Item->Units : 0; }
            return State.Stock[I].Warehouse + State.Stock[I].Shelf;
        };

        int32 OwnDone = 0;
        const int32 Orders = Count(Want, Random);
        for (int32 N = 0; N < Orders; ++N)
        {
            // Which channel this order came through.
            const float Draw = Random.FRand() * Pull.All();
            EChannel Channel = Draw < Pull.Platform ? EChannel::Platform : Pull.bQuick ? EChannel::Quick : Draw < Pull.Platform + Pull.Web ? EChannel::Web : EChannel::App;
            const bool bOwn = Channel != EChannel::Platform;
            bool bLate = false;
            if (bOwn)
            {
                if (Channel == EChannel::Quick)
                {
                    if (QuickPicked[Shop.Area] >= FMath::RoundToInt32(DarkStorePicks * Hands)) bLate = true;
                    ++QuickPicked[Shop.Area];
                }
                else if (OwnDone >= Shop.Pick)
                {
                    if (Random.FRand() < 0.4f) { ++O.LastCancelled; O.Reputation -= 2.5f; continue; }
                    bLate = true;
                }
                ++OwnDone;
            }
            const int32 MinLines = Channel == EChannel::Web || Channel == EChannel::App ? 4 : 2;
            const int32 MaxLines = Channel == EChannel::Web ? 9 : Channel == EChannel::App ? 8 : Channel == EChannel::Platform ? 5 : 4;
            const int32 Lines = Random.RandRange(MinLines, MaxLines) + (bOwn ? O.MinBasket : 0);
            int64 Value = 0, Cogs = 0;
            int32 Missing = 0;
            for (int32 Line = 0; Line < Lines; ++Line)
            {
                int32 Index = PickProduct(Weights, Total, Random);
                if (Index == INDEX_NONE) break;
                const int32 Wanted = Random.RandRange(1, Channel == EChannel::Web || Channel == EChannel::App ? 3 : 2);
                if (Available(Index) < Wanted)
                {
                    int32 Other = INDEX_NONE;
                    if (O.Substitute != 2)
                        for (int32 Step = 1; Step < Products.Num() && Other == INDEX_NONE; ++Step)
                        {
                            const int32 Try = (Index + Step) % Products.Num();
                            if (Weights[Try] > 0.f && Products[Try].Category == Products[Index].Category && Available(Try) >= Wanted) Other = Try;
                        }
                    if (Other != INDEX_NONE && Random.FRand() < (O.Substitute == 0 ? 0.85f : 0.7f))
                    {
                        Index = Other;
                        ++O.LastSubstituted;
                        O.Reputation -= O.Substitute == 0 ? 0.1f : 0.3f;
                    }
                    else
                    {
                        ++Missing;
                        O.Reputation -= O.Substitute == 0 ? 0.5f : O.Substitute == 2 ? 1.f : 1.2f;
                        continue;
                    }
                }
                int64 Price, Cost;
                if (Branch)
                {
                    FMarketBranchItem* Item = ItemAt(*Branch, Products, Index);
                    Item->Units -= Wanted;
                    Item->LastSold += Wanted;
                    Price = FMath::Max<int64>(5, FMath::RoundToInt64(Products[Index].BasePrice * Branch->PriceIndex / 5.0) * 5);
                    Cost = Products[Index].Cost;
                }
                else
                {
                    FMarketStock& Row = State.Stock[Index];
                    const int32 FromDepot = FMath::Min(Row.Warehouse, Wanted);
                    Row.Warehouse -= FromDepot;
                    Row.Shelf -= FMath::Min(Row.Shelf, Wanted - FromDepot);
                    Row.Yesterday.Sold += Wanted;
                    ++Row.Yesterday.Buyers;
                    State.Ledger.OnlineSold[Index] += Wanted;
                    Price = MarketPromotions::DealPrice(State, Products, Index, Wanted, Closed);   // B1 (#30): the shelf deal
                    Cost = State.UnitCost(Index, Products);
                }
                Value += FMath::RoundToInt64(static_cast<double>(Price) * Wanted * static_cast<double>(PriceGap));
                Cogs += Cost * Wanted;
                Units += Wanted;
            }
            O.LastMissing += Missing;
            if (bOwn && (O.Fee == 2 || (O.Fee == 1 && Value < FMath::RoundToInt64(3000 * Level)))) Value += FMath::RoundToInt64(300 * Level);
            int64 Costs = Packaging;
            if (bOwn) Costs += FMath::RoundToInt64(Courier * (Channel == EChannel::Quick ? QuickCourierFactor : 1.f)) + FMath::RoundToInt64(Value * CardCommission);
            else Costs += FMath::RoundToInt64(Value * Commission);
            if (bLate) { ++O.LastLate; O.Reputation -= 1.5f; }
            else if (Missing == 0) O.Reputation += 0.25f;
            if (bOwn && Random.FRand() < 0.03f + 0.04f * O.Ads) ++O.MonthNew;
            const int64 Profit = Value - Cogs - Costs;
            const int32 C = static_cast<int32>(Channel);
            ++O.MonthOrders[C]; O.MonthRevenue[C] += Value; O.MonthProfit[C] += Profit;
            ++O.LastOrders;
            O.LastRevenue += Value;
            O.LastCosts += Costs;
            O.LastProfit += Profit;
            ++A.LastOrders;
            (bOwn ? A.MonthOwnProfit : A.MonthPlatformProfit) += Profit;
            A.Revenue30 += Value;
            A.Profit30 += Profit;
            // The books of the shop that picked it.
            State.Cash += Value - Costs;
            const int32 Store = Branch ? Shop.Branch : MarketLedger::FamilyShop;
            MarketLedger::Post(State, MarketLedger::EAccount::OnlineSales, Value, true, Store);
            MarketLedger::Post(State, MarketLedger::EAccount::CostOfGoods, -Cogs, false, Store);
            MarketLedger::Post(State, MarketLedger::EAccount::OnlineCosts, -Costs, true, Store);
            if (Branch)
            {
                Branch->LastRevenue += Value;
                Branch->LastProfit += Profit;
                Branch->Last30Revenue += Value;
                Branch->Last30Profit += Profit;
                Branch->WeekProfit += Profit;
                State.Books.PeriodSales += Value;
                State.LastBranchProfit += Profit;
                O.LastBranchProfit += Profit;
            }
            else { FamilyRevenue += Value; FamilyCogs += Cogs; FamilyCosts += Costs; }
            State.LastProfit += Profit;
        }
    }
    O.Reputation = FMath::Clamp(O.Reputation, 0.f, 100.f);

    // The family shop's part goes into its day (the tax books see it); the head office pays the fixed part.
    State.LastRevenue += FamilyRevenue;
    State.LastCostOfGoods += FamilyCogs;
    State.LastOperatingCost += FamilyCosts;
    if (Fixed > 0)
    {
        State.Cash -= Fixed;
        State.LastProfit -= Fixed;
        State.LastBranchProfit -= Fixed;
        O.LastCosts += Fixed;
        O.LastProfit -= Fixed;
        MarketLedger::Post(State, MarketLedger::EAccount::OnlineCosts, -Fixed, true, MarketLedger::HeadOfficeStore);
    }
    // Picking tires the family shop's stocker on duty.
    if (Units > 0)
        for (const FMarketEmployee& E : State.Staff)
            if (E.Role == static_cast<uint8>(MarketStaff::ERole::Stocker) && E.OffDay != Closed) { MarketStaff::RecordWork(State, E.Id, static_cast<int32>(FMath::Min<int64>(Units, 400)) / 4); break; }

    for (FMarketOnlineArea& A : O.Areas)
    {
        A.Orders30 = A.Orders30 * 29 / 30 + A.LastOrders;
        A.LastOrders = 0;
    }
    O.TotalOrders += O.LastOrders;
    O.WeekOrders += O.LastOrders;
    O.WeekProfit += O.LastProfit;

    if (O.LastOrders > 0 || O.LastCancelled > 0)
    {
        FString Line = FString::Printf(TEXT("\u0130nternet: %d sipari\u015f (%d ge\u00e7)"), O.LastOrders, O.LastLate);
        if (O.LastCancelled) Line += FString::Printf(TEXT(", %d iptal \u2014 toplay\u0131c\u0131 yetmedi"), O.LastCancelled);
        if (O.LastMissing) Line += FString::Printf(TEXT(", %d eksik \u00fcr\u00fcn"), O.LastMissing);
        if (O.LastSubstituted) Line += FString::Printf(TEXT(", %d ikame"), O.LastSubstituted);
        Line += FString::Printf(TEXT("; ciro %s, net %s."), *Tl(O.LastRevenue), *Tl(O.LastProfit));
        State.DayNews.Add(Line);
    }
    if (MarketCalendar::DateOf(Closed).Weekday == 6)
    {
        if (O.WeekOrders > 0) State.DayNews.Add(FString::Printf(TEXT("Haftan\u0131n internet sipari\u015fleri: %d, net %s."), O.WeekOrders, *Tl(O.WeekProfit)));
        O.WeekOrders = 0;
        O.WeekProfit = 0;
    }
    if (bNewMonth)
    {
        O.PrevOrders = O.MonthOrders; O.PrevRevenue = O.MonthRevenue; O.PrevProfit = O.MonthProfit;
        O.PrevAds = O.MonthAds; O.PrevNew = O.MonthNew;
        O.MonthOrders.Init(0, ChannelCount); O.MonthRevenue.Init(0, ChannelCount); O.MonthProfit.Init(0, ChannelCount);
        O.MonthAds = 0; O.MonthNew = 0;
        AutoPolicy(State);
    }
}

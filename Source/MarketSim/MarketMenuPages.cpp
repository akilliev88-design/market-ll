// Management menu pages (G-074): products and prices, promotions, rivals, staff, finance, sales channels and
// branches. The frame, the building blocks and the summary / orders / reports pages are in MarketMenuWidget.cpp;
// shared helpers in MarketMenuInternal.h. Every decision goes through AMarketGameMode::MenuCommand / StaffCommand.
#include "MarketMenuWidget.h"
#include "MarketCast.h"
#include "MarketMenuInternal.h"
#include "CoreGlobals.h"

#include "MarketGame.h"
#include "ProductCatalog.h"
#include "MarketSuppliers.h"
#include "MarketPromotions.h"
#include "MarketFinance.h"
#include "MarketOnline.h"
#include "MarketAdvertising.h"
#include "MarketPayments.h"
#include "MarketFreshness.h"
#include "MarketBranches.h"
#include "MarketCompany.h"
#include "MarketCampaign.h"
#include "MarketCalendar.h"
#include "MarketStaff.h"
#include "MarketPrices.h"
#include "MarketMap.h"
#include "MarketTheme.h"
#include "MarketCountry.h"
#include "MarketStart.h"
#include "MarketStory.h"
#include "MarketManagers.h"
#include "MarketDepots.h"
#include "MarketStoreViews.h"
#include "MarketChains.h"
#include "MarketBrands.h"
#include "MarketSourcing.h"
#include "MarketDepartments.h"
#include "MarketBanking.h"
#include "MarketLedger.h"
#include "MarketOwner.h"
#include "MarketRumors.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "MarketSubsidiaries.h"
#include "MarketResearch.h"
#include "MarketFranchise.h"
#include "MarketWorldMap.h"
#include "MarketStrategy.h"
#include "MarketPortfolio.h"
#include "MarketResponse.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

// Named namespace (unity build): helpers of the pages only.
namespace MarketMenuPagesUi
{
    // Game year with the month as a fraction (market data is by year).
    float YearOf(const FMarketState& State)
    {
        const MarketCalendar::FDate Date = MarketCalendar::DateOf(State.Day);
        return static_cast<float>(Date.Year) + static_cast<float>(Date.Month - 1) / 12.f;
    }

    FLinearColor ChainColor(const FString& Key)
    {
        using MarketMenuUi::Hex;
        if (Key == TEXT("bim")) return Hex(TEXT("C8102E"));
        if (Key == TEXT("migros")) return Hex(TEXT("F28C00"));
        if (Key == TEXT("a101")) return Hex(TEXT("0068A8"));
        if (Key == TEXT("sok")) return Hex(TEXT("D9A400"));
        if (Key == TEXT("onur")) return Hex(TEXT("7A3E9D"));
        if (Key == TEXT("carrefoursa")) return Hex(TEXT("1E4FA0"));
        if (Key == TEXT("kipa")) return Hex(TEXT("B5121B"));
        if (Key == TEXT("metro")) return Hex(TEXT("0B2D6B"));
        if (Key == TEXT("miras")) return Hex(TEXT("16775F"));
        return Hex(TEXT("5F6670"));
    }

    // Map layers of the Subeler page: 0 = our stores, then the rival chains of Config/iller.json.
    const TCHAR* LayerKey(int32 Layer)
    {
        static const TCHAR* Keys[6] = { TEXT("miras"), TEXT("bim"), TEXT("a101"), TEXT("sok"), TEXT("migros"), TEXT("onur") };
        return Keys[FMath::Clamp(Layer, 0, 5)];
    }

    // Stores of a chain per 100.000 people in a province, and the highest value in the country (cached per chain).
    float Density(const MarketMapData::FProvince& Province, const FString& Key)
    {
        const int32* Count = Province.Stores.Find(Key);
        return Count ? static_cast<float>(*Count) * 100.f / static_cast<float>(FMath::Max(1, Province.PopulationK)) : 0.f;
    }

    float MaxDensity(const FString& Key)
    {
        static TMap<FString, float> Cache;
        if (const float* Found = Cache.Find(Key)) return *Found;
        float Most = 0.f;
        for (const MarketMapData::FProvince& Province : MarketMapData::Get().Provinces) Most = FMath::Max(Most, Density(Province, Key));
        Cache.Add(Key, Most);
        return Most;
    }

    // G-086: map index of a province id of the map's country (INDEX_NONE without the map). Looked up once per id.
    int32 MapIndexOf(const FString& ProvinceId)
    {
        static TMap<FString, int32> Cache;
        if (const int32* Found = Cache.Find(ProvinceId)) return *Found;
        const MarketMapData::FData& Data = MarketMapData::Get();
        const int32 Index = Data.Provinces.IndexOfByPredicate([&ProvinceId](const MarketMapData::FProvince& P) { return P.Id == ProvinceId; });
        Cache.Add(ProvinceId, Index);
        return Index;
    }

    // Our shops in a province (the first store counts in the home province).
    int32 OurShops(const FMarketState& State, const FString& Country, const FString& Province)
    {
        return MarketBranches::ShopsIn(State, Country, Province);
    }

    // Branch indexes in a province, in the order they opened.
    TArray<int32> BranchesIn(const FMarketState& State, const FString& Country, const FString& Province)
    {
        TArray<int32> List;
        for (int32 I = 0; I < State.Branches.Num(); ++I)
        {
            const FMarketBranch& B = State.Branches[I];
            if (B.Stage == static_cast<uint8>(MarketBranches::EStage::Closed)) continue;
            if (MarketBranches::CountryOf(State, B) == Country && B.Province == Province) List.Add(I);
        }
        return List;
    }

    // The worst weekly mark of our branches in a province ("" = none marked yet).
    FString WorstGrade(const FMarketState& State, const FString& Country, const FString& Province)
    {
        FString Worst;
        for (const int32 I : BranchesIn(State, Country, Province))
        {
            const FString Grade = MarketBranches::Grade(State, I);
            if (Grade != TEXT("-") && (Worst.IsEmpty() || Grade[0] > Worst[0])) Worst = Grade;
        }
        return Worst;
    }

    SMarketMenu::ERole GradeRole(const FString& Grade)
    {
        using ERole = SMarketMenu::ERole;
        return Grade == TEXT("A") || Grade == TEXT("B") ? ERole::Good : Grade == TEXT("C") ? ERole::Warn : Grade == TEXT("D") ? ERole::Bad : ERole::Muted;
    }

    // "orta (x1,05)" for a province value around 1.
    FString Level(float Value, const TCHAR* Low, const TCHAR* Mid, const TCHAR* High)
    {
        const TCHAR* Word = Value < 0.9f ? Low : Value > 1.12f ? High : Mid;
        return FString::Printf(TEXT("%s (\u00d7%s)"), Word, *FString::SanitizeFloat(FMath::RoundToFloat(Value * 100.f) / 100.f).Replace(TEXT("."), TEXT(",")));
    }

    FString PeopleText(int32 PopulationK)
    {
        return PopulationK >= 1000 ? FString::Printf(TEXT("%s milyon"), *FString::SanitizeFloat(FMath::RoundToFloat(PopulationK / 100.f) / 10.f).Replace(TEXT("."), TEXT(",")))
            : FString::Printf(TEXT("%d bin"), PopulationK);
    }

    // What to expect in a province (new game and the province card).
    FString Expectation(const MarketCountry::FCity& City)
    {
        TArray<FString> Lines;
        if (City.Competition < 0.9f) Lines.Add(TEXT("Az rakip: ilk aylar rahat ge\u00e7er."));
        else if (City.Competition > 1.2f) Lines.Add(TEXT("Zincirler kalabal\u0131k: pay kazanmak zor, fiyat ve raf \u00f6nemli."));
        if (City.Rent < 0.85f) Lines.Add(TEXT("Kira ucuz; ma\u011faza a\u00e7mak az tutar."));
        else if (City.Rent > 1.3f) Lines.Add(TEXT("Kira pahal\u0131; yeni ma\u011faza \u00e7abuk k\u00e2ra ge\u00e7meli."));
        if (City.Income < 0.85f) Lines.Add(TEXT("M\u00fc\u015fteri fiyata \u00e7ok duyarl\u0131; ucuzcu iyi gider, s\u00fcpermarket zorlan\u0131r."));
        else if (City.Income > 1.15f) Lines.Add(TEXT("Al\u0131m g\u00fcc\u00fc y\u00fcksek; s\u00fcpermarket ve marka \u00fcr\u00fcn iyi gider."));
        if (City.PopulationK >= 2000) Lines.Add(TEXT("B\u00fcy\u00fck il: \u00e7ok m\u00fc\u015fteri, \u00e7ok ma\u011faza yeri."));
        else if (City.PopulationK < 300) Lines.Add(TEXT("K\u00fc\u00e7\u00fck il: az ma\u011faza yeri; b\u00fcy\u00fcmek i\u00e7in ba\u015fka illere gerekecek."));
        if (Lines.Num() == 0) Lines.Add(TEXT("Dengeli bir il: ne \u00e7ok zor ne \u00e7ok kolay."));
        return TEXT("\u2022 ") + FString::Join(Lines, TEXT("\n\u2022 "));
    }

    // One rotating card of the main screen.
    struct FRotCard
    {
        FString Title;
        FString Headline;
        FString Body;
        int32 Page = INDEX_NONE;
    };

    TArray<FRotCard> RotCards(const AMarketGameMode& G)
    {
        const FMarketState& S = G.State;
        TArray<FRotCard> Cards;
        // The best and the most worrying branch of the last month.
        int32 Best = INDEX_NONE, Worst = INDEX_NONE;
        for (int32 I = 0; I < S.Branches.Num(); ++I)
        {
            if (MarketBranches::Grade(S, I) == TEXT("-")) continue;
            if (Best == INDEX_NONE || S.Branches[I].Last30Profit > S.Branches[Best].Last30Profit) Best = I;
            if (Worst == INDEX_NONE || S.Branches[I].Last30Profit < S.Branches[Worst].Last30Profit) Worst = I;
        }
        if (Best != INDEX_NONE)
        {
            const FMarketBranch& B = S.Branches[Best];
            Cards.Add({ TEXT("AYIN MA\u011eAZASI"), B.Name, FString::Printf(TEXT("Karne %s \u00b7 son 30 g\u00fcn %s \u00b7 m\u00fcd\u00fcr %s (beceri %d). Raf dolulu\u011fu ve memnuniyet onu \u00f6ne \u00e7\u0131kar\u0131yor."),
                *MarketBranches::Grade(S, Best), *MarketMenuUi::Tl(B.Last30Profit), B.ManagerName.IsEmpty() ? TEXT("yok") : *B.ManagerName, B.ManagerSkill), SMarketMenu::Branches });
        }
        const FString WorstMark = Worst != INDEX_NONE ? MarketBranches::Grade(S, Worst) : FString();
        if (Worst != INDEX_NONE && Worst != Best && (WorstMark == TEXT("C") || WorstMark == TEXT("D")))
        {
            const FMarketBranch& B = S.Branches[Worst];
            Cards.Add({ TEXT("D\u0130KKAT \u0130STEYEN MA\u011eAZA"), B.Name, FString::Printf(TEXT("Karne %s \u00b7 son 30 g\u00fcn %s. M\u00fcd\u00fcr\u00fc de\u011fi\u015ftirmeyi ya da fiyat\u0131 d\u00fczeltmeyi d\u00fc\u015f\u00fcn."),
                *MarketBranches::Grade(S, Worst), *MarketMenuUi::Tl(B.Last30Profit)), SMarketMenu::Branches });
        }
        // The next special day within two weeks.
        for (int32 Ahead = 1; Ahead <= 14; ++Ahead)
        {
            const MarketCalendar::FDayInfo Day = MarketCalendar::Info(S.Day + Ahead, S.RivalSeed);
            bool bSpecial = false;
            for (const MarketCalendar::ETag Tag : Day.Tags)
                if (Tag != MarketCalendar::ETag::Payday && Tag != MarketCalendar::ETag::MonthEnd) bSpecial = true;
            if (!bSpecial) continue;
            FString Line = MarketCalendar::Forecast(S.Day + Ahead, S.RivalSeed);
            Line.RemoveFromStart(TEXT("Yar\u0131n "));
            Cards.Add({ TEXT("YAKLA\u015eAN \u00d6ZEL G\u00dcN"), Ahead == 1 ? FString(TEXT("Yar\u0131n")) : FString::Printf(TEXT("%d g\u00fcn sonra"), Ahead), Line, SMarketMenu::Orders });
            break;
        }
        // Rivals and the till (M69: no chapters; the inherited debt waits on the Finance page, it locks nothing).
        // E2: a chain's price war against us in the home province is the rivals' move worth a card.
        const int32 War = MarketChains::WarIn(S, S.CountryId, MarketStart::HomeProvince(S), S.Day);
        if (War != INDEX_NONE) Cards.Add({ TEXT("RAK\u0130PLERDE BUG\u00dcN"), TEXT("Fiyat sava\u015f\u0131"), FString::Printf(TEXT("%s ilde fiyatlar\u0131n\u0131 k\u0131rd\u0131 (%d. g\u00fcne kadar). M\u00fc\u015fterinin akl\u0131ndaki rakip fiyat\u0131 d\u00fc\u015ft\u00fc."), *S.Rivals.Chains[War].Name, S.Rivals.Chains[War].WarUntil), SMarketMenu::Rivals });
        if (S.Cash < 0) Cards.Add({ TEXT("KASA"), TEXT("Kasa eksi"), TEXT("Sipari\u015fleri k\u0131s, kredi al ya da zarar eden ma\u011fazay\u0131 kapat."), SMarketMenu::Finance });
        if (Cards.Num() == 0) Cards.Add({ TEXT("SAK\u0130N B\u0130R G\u00dcN"), TEXT("Her \u015fey yolunda"), TEXT("Raflar dolu, kasa yolunda. Haritadan yeni bir il se\u00e7ip ma\u011faza a\u00e7may\u0131 d\u00fc\u015f\u00fcnebilirsin."), INDEX_NONE });
        return Cards;
    }

    // K-th running promotion (index into State.Promotions), INDEX_NONE when fewer run.
    int32 RunningPromotion(const FMarketState& State, int32 Slot)
    {
        int32 Seen = 0;
        for (int32 I = 0; I < State.Promotions.Num(); ++I)
            if (MarketPromotions::IsActive(State.Promotions[I], State.Day) && Seen++ == Slot) return I;
        return INDEX_NONE;
    }

    bool OfferWaiting(const FMarketState& State)
    {
        return State.Offer.Product != INDEX_NONE && State.Offer.EndDay >= State.Day;
    }

    // Today's money for an amount at the game-start price level (loans, flyers, web shop set-up), rounded to whole
    // lira (M24/Y1: "Start" names the start level in code only).
    int64 Today(const FMarketState& State, int64 KurusStart)
    {
        return FMath::RoundToInt64(static_cast<double>(KurusStart) * MarketPrices::ListLevel(State.Day) / 100.0) * 100;
    }

    // G-086b: a branch's day in one line without its manager (the manager has a line of his own; his skill stays
    // hidden until an HR manager or the province manager sees it).
    FString BranchLine(const FMarketState& State, int32 BranchIndex, const TArray<FMarketProduct>& Products)
    {
        if (!State.Branches.IsValidIndex(BranchIndex)) return FString();
        const FMarketBranch& B = State.Branches[BranchIndex];
        if (B.Stage != static_cast<uint8>(MarketBranches::EStage::Open)) return MarketBranches::Summary(State, BranchIndex, Products);
        int32 Capacity = 0, Units = 0;
        for (const FMarketStock& Item : B.Items) { Capacity += Item.Capacity; Units += FMath::Min(Item.Shelf, Item.Capacity); }
        return FString::Printf(TEXT("%s \u00b7 karne %s \u00b7 %d. g\u00fcn \u00b7 d\u00fcn %d m\u00fc\u015fteri, net %s \u00b7 raf %%%d dolu"), *B.Name, *MarketBranches::Grade(State, BranchIndex),
            State.Day - B.OpenedDay, B.LastShoppers, *MarketMenuUi::Tl(B.LastProfit), Capacity > 0 ? Units * 100 / Capacity : 0);
    }

    // G-086b: the store manager of a branch in one line: name, skill ("?" while hidden), morale, the first week.
    FString ManagerLine(const FMarketState& State, int32 BranchIndex)
    {
        if (!State.Branches.IsValidIndex(BranchIndex)) return FString();
        const FMarketBranch& B = State.Branches[BranchIndex];
        if (B.ManagerName.IsEmpty())
            return B.Stage == static_cast<uint8>(MarketBranches::EStage::Open) ? FString(TEXT("M\u00fcd\u00fcr yok: senin talimatlar\u0131n i\u015fliyor")) : FString(TEXT("M\u00fcd\u00fcr i\u015fe al\u0131m a\u015famas\u0131nda gelir"));
        TArray<FString> Parts;
        Parts.Add(TEXT("M\u00fcd\u00fcr ") + B.ManagerName);
        Parts.Add(MarketManagers::SkillVisible(State, BranchIndex)
            ? FString::Printf(TEXT("beceri %d (etkin %d)"), B.ManagerSkill, MarketManagers::EffectiveSkill(State, BranchIndex))
            : FString(TEXT("beceri ?")));
        Parts.Add(FString::Printf(TEXT("moral %.0f"), FMath::Max(0.f, B.ManagerMorale)));
        if (B.ManagerSince > 0 && State.Day - B.ManagerSince < MarketManagers::SettleDays)
            Parts.Add(FString::Printf(TEXT("al\u0131\u015f\u0131yor (%d g\u00fcn)"), MarketManagers::SettleDays - (State.Day - B.ManagerSince)));
        if (B.ManagerWarnings > 0) Parts.Add(FString::Printf(TEXT("%d uyar\u0131"), B.ManagerWarnings));
        if (B.ManagerBadWeeks > 0) Parts.Add(FString::Printf(TEXT("%d hafta k\u00f6t\u00fc karne"), B.ManagerBadWeeks));
        if (B.ManagerCaughtDay > 0) Parts.Add(TEXT("kasadan al\u0131yor"));
        return FString::Join(Parts, TEXT(" \u00b7 "));
    }

    SMarketMenu::ERole ManagerRole(const FMarketState& State, int32 BranchIndex)
    {
        using ERole = SMarketMenu::ERole;
        if (!State.Branches.IsValidIndex(BranchIndex)) return ERole::Muted;
        const FMarketBranch& B = State.Branches[BranchIndex];
        if (B.ManagerCaughtDay > 0) return ERole::Bad;
        if (B.ManagerName.IsEmpty() && B.Stage == static_cast<uint8>(MarketBranches::EStage::Open)) return ERole::Warn;
        if (!B.ManagerName.IsEmpty() && ((B.ManagerMorale >= 0.f && B.ManagerMorale < 30.f) || B.ManagerBadWeeks >= 2)) return ERole::Warn;
        return ERole::Muted;
    }

    // The province of a branch that has room for a province manager and none yet (the "Il muduru yap" button).
    bool ProvinceWantsManager(const FMarketState& State, int32 BranchIndex)
    {
        if (!State.Branches.IsValidIndex(BranchIndex)) return false;
        const FString Country = MarketBranches::CountryOf(State, State.Branches[BranchIndex]);
        const FString Province = MarketManagers::AreaOfBranch(State, BranchIndex, MarketManagers::ELevel::Province);
        return MarketManagers::ProvinceBranches(State, Country, Province) >= MarketManagers::ProvinceShops
            && MarketManagers::FindManager(State, MarketManagers::ELevel::Province, Country, Province) == INDEX_NONE;
    }

    // One line of the tree of levels (Magazalar > Yonetim): an appointed manager, an empty level that can be
    // appointed now, or the store managers of a province.
    struct FTierRow
    {
        MarketManagers::ELevel Tier = MarketManagers::ELevel::Store;
        FString Country;
        FString Area;
        int32 Manager = INDEX_NONE;   // State.Management.Managers
        int32 Depth = 0;              // indent: country 0, main region 1, sub-region 2, province 3, stores 4
        int32 Stores = 0;             // Store rows: open branches of the province
        int32 FirstBranch = INDEX_NONE;
        bool bRequired = false;       // an empty country level the company must fill
        FString Lock;                 // G-086b ek (M19): why the first store's level is still closed ("" = open)
    };

    // The tree of levels in the order of the network: every country with a shop, its main regions, sub-regions and
    // provinces. A level shows when it has a manager or can get one now; provinces with shops show their stores.
    TArray<FTierRow> TierRows(const FMarketState& State)
    {
        using MarketManagers::ELevel;
        TArray<FTierRow> Rows;
        TArray<FString> Countries;
        Countries.Add(State.CountryId);
        for (const FMarketBranch& B : State.Branches)
            if (B.Stage != static_cast<uint8>(MarketBranches::EStage::Closed)) Countries.AddUnique(MarketBranches::CountryOf(State, B));
        const bool bRequired = MarketManagers::CountryManagerRequired(State);
        auto CanFill = [&State](ELevel Tier, const FString& Country, const FString& Area)
        {
            FString Reason;
            return MarketManagers::CanAppoint(State, Tier, Country, Area, INDEX_NONE, Reason);
        };
        auto AddLevel = [&Rows, &State, &CanFill](ELevel Tier, const FString& Country, const FString& Area, int32 Depth)
        {
            FTierRow Row;
            Row.Tier = Tier; Row.Country = Country; Row.Area = Area; Row.Depth = Depth;
            Row.Manager = MarketManagers::FindManager(State, Tier, Country, Area);
            if (Row.Manager == INDEX_NONE && !CanFill(Tier, Country, Area)) return;
            Rows.Add(Row);
        };
        auto AddProvince = [&Rows, &State, &AddLevel](const FString& Country, const FString& Province)
        {
            AddLevel(ELevel::Province, Country, Province, 3);
            FTierRow Row;
            Row.Tier = ELevel::Store; Row.Country = Country; Row.Area = Province; Row.Depth = 4;
            for (int32 I = 0; I < State.Branches.Num(); ++I)
            {
                const FMarketBranch& B = State.Branches[I];
                if (B.Stage != static_cast<uint8>(MarketBranches::EStage::Open) || MarketBranches::CountryOf(State, B) != Country
                    || MarketManagers::AreaOfBranch(State, I, ELevel::Province) != Province) continue;
                if (Row.FirstBranch == INDEX_NONE) Row.FirstBranch = I;
                ++Row.Stores;
            }
            if (Row.Stores > 0) Rows.Add(Row);
        };
        // M19: the first store on top; locked (with its reason) until a branch outside it is open.
        {
            FTierRow Family;
            Family.Tier = ELevel::FirstStore; Family.Country = State.CountryId; Family.Area = MarketStart::HomeProvince(State); Family.Depth = 0;
            Family.Manager = MarketManagers::FindManager(State, ELevel::FirstStore, State.CountryId, FString());
            if (Family.Manager == INDEX_NONE)
            {
                FString Reason;
                if (!MarketManagers::CanAppoint(State, ELevel::FirstStore, State.CountryId, FString(), INDEX_NONE, Reason)) Family.Lock = Reason.IsEmpty() ? FString(TEXT("Kilitli.")) : Reason;
            }
            Rows.Add(Family);
        }
        // D4: the optional general manager and the continent directors above the countries.
        AddLevel(ELevel::Chief, State.CountryId, FString(), 0);
        {
            const TArray<FString> Missing = MarketManagers::ContinentsMissingDirector(State);
            for (const FString& Continent : MarketCountry::Continents())
            {
                if (MarketManagers::CountriesOn(State, Continent) == 0) continue;
                FTierRow Row;
                Row.Tier = ELevel::Continent; Row.Country = State.CountryId; Row.Area = Continent; Row.Depth = 0;
                Row.Manager = MarketManagers::FindManager(State, ELevel::Continent, State.CountryId, Continent);
                Row.bRequired = Row.Manager == INDEX_NONE && Missing.Contains(Continent);
                if (Row.Manager != INDEX_NONE || Row.bRequired || CanFill(ELevel::Continent, State.CountryId, Continent)) Rows.Add(Row);
            }
        }
        for (const FString& Country : Countries)
        {
            const MarketCountry::FProfile* Pack = MarketCountry::Find(Country);
            if (!Pack) continue;
            // M20: the country level shows once 5 provinces have our shops (or it is required: then red).
            FTierRow Head;
            Head.Tier = ELevel::Country; Head.Country = Country; Head.Area = Country; Head.Depth = 0;
            Head.Manager = MarketManagers::FindManager(State, ELevel::Country, Country, Country);
            Head.bRequired = bRequired && Head.Manager == INDEX_NONE;
            if (Head.Manager != INDEX_NONE || Head.bRequired || MarketManagers::IsTierVisible(State, ELevel::Country, Country, Country)) Rows.Add(Head);
            // M23: the depots of the country (their managers answer to the country manager, else to the player).
            for (const FMarketDepot& Site : State.Company.DepotSites)
            {
                if (Site.Country != Country) continue;
                FTierRow Depot;
                Depot.Tier = ELevel::Depot; Depot.Country = Country; Depot.Area = Site.Province; Depot.Depth = 1;
                Depot.Manager = MarketManagers::FindManager(State, ELevel::Depot, Country, Site.Province);
                Rows.Add(Depot);
            }
            TArray<FString> Seen;
            for (const MarketCountry::FRegion& Main : Pack->Regions)
            {
                AddLevel(ELevel::Region, Country, Main.Id, 1);
                for (const MarketCountry::FRegion& Sub : Pack->SubRegions)
                {
                    if (Sub.Parent != Main.Id) continue;
                    AddLevel(ELevel::SubRegion, Country, Sub.Id, 2);
                    for (const MarketCountry::FCity& City : Pack->Cities)
                    {
                        if (City.SubRegion != Sub.Id) continue;
                        Seen.Add(City.Id);
                        AddProvince(Country, City.Id);
                    }
                }
            }
            // Provinces of a pack without (complete) regions.
            for (const MarketCountry::FCity& City : Pack->Cities)
                if (!Seen.Contains(City.Id)) AddProvince(Country, City.Id);
        }
        return Rows;
    }

    // Store managers who could be promoted to a level of an area (their branch lies in it).
    TArray<int32> PromotionCandidates(const FMarketState& State, MarketManagers::ELevel Tier, const FString& Country, const FString& Area)
    {
        TArray<int32> List;
        if (Tier == MarketManagers::ELevel::FirstStore || Tier == MarketManagers::ELevel::Store) return List;
        for (int32 I = 0; I < State.Branches.Num(); ++I)
        {
            const FMarketBranch& B = State.Branches[I];
            if (B.Stage != static_cast<uint8>(MarketBranches::EStage::Open) || B.ManagerName.IsEmpty()) continue;
            if (MarketBranches::CountryOf(State, B) == Country && MarketManagers::AreaOfBranch(State, I, Tier) == Area) List.Add(I);
        }
        return List;
    }

    // "Il muduru" with a capital first letter (the tree's level column).
    FString LevelTitle(MarketManagers::ELevel Tier)
    {
        if (Tier == MarketManagers::ELevel::Store) return TEXT("Ma\u011fazalar");
        if (Tier == MarketManagers::ELevel::FirstStore) return TEXT("\u0130lk ma\u011faza");
        return MarketMenuUi::Title(MarketManagers::LevelName(Tier));
    }

    // Name of a province of a country ("Tekirda\u011f"), the id when unknown.
    FString CityName(const FString& Country, const FString& Province)
    {
        const MarketCountry::FCity* City = MarketCountry::FindCity(Country, Province);
        return City ? City->Name : Province;
    }

    // G-089: a branch's supply in a few words for its row ("Depo: Tekirda\u011f \u00b7 120 km" / "Toptanc\u0131dan").
    FString SupplyShort(const FMarketState& State, int32 BranchIndex, const TArray<MarketDepots::FLink>& Links)
    {
        if (!State.Branches.IsValidIndex(BranchIndex)) return FString();
        if (Links.IsValidIndex(BranchIndex) && State.Company.DepotSites.IsValidIndex(Links[BranchIndex].Depot))
        {
            const FMarketDepot& Site = State.Company.DepotSites[Links[BranchIndex].Depot];
            return FString::Printf(TEXT("Depo: %s \u00b7 %.0f km"), *CityName(Site.Country, Site.Province), Links[BranchIndex].Km);
        }
        const FString Country = MarketBranches::CountryOf(State, State.Branches[BranchIndex]);
        const FString Province = MarketManagers::AreaOfBranch(State, BranchIndex, MarketManagers::ELevel::Province);
        const bool bHome = Country == State.CountryId && Province == MarketStart::HomeProvince(State);
        return bHome ? FString(TEXT("Toptanc\u0131dan (ev ili)")) : FString(TEXT("Toptanc\u0131dan \u00b7 depo uzak"));
    }

    // G-089: open branches of the country outside the home province that buy from the wholesaler (no depot in range).
    int32 BranchesFarFromDepot(const FMarketState& State, const TArray<MarketDepots::FLink>& Links)
    {
        int32 Far = 0;
        for (int32 I = 0; I < State.Branches.Num(); ++I)
        {
            if (State.Branches[I].Stage != static_cast<uint8>(MarketBranches::EStage::Open)) continue;
            if (Links.IsValidIndex(I) && Links[I].Depot != INDEX_NONE) continue;
            const FString Country = MarketBranches::CountryOf(State, State.Branches[I]);
            const FString Province = MarketManagers::AreaOfBranch(State, I, MarketManagers::ELevel::Province);
            if (Country == State.CountryId && Province == MarketStart::HomeProvince(State)) continue;
            ++Far;
        }
        return Far;
    }

    // G-089: the province panel's depot line: this province's own depot, the nearest one within range or the
    // nearest one out of range (the goods then come from the wholesaler).
    FString NearestDepotLine(const FMarketState& State, const FString& Country, const FString& Province)
    {
        const int32 Here = MarketDepots::Find(State, Country, Province);
        if (Here != INDEX_NONE)
            return FString::Printf(TEXT("Depo burada: %d ma\u011fazaya mal g\u00f6nderiyor \u00b7 verim %%%.0f"), MarketDepots::Served(State, Here), 100.f * MarketDepots::Efficiency(State, Here));
        int32 Best = INDEX_NONE;
        float BestKm = 0.f;
        const TArray<FMarketDepot>& Sites = State.Company.DepotSites;
        for (int32 D = 0; D < Sites.Num(); ++D)
        {
            if (Sites[D].Country != Country) continue;
            const float Km = MarketDepots::DistanceKm(Country, Province, Sites[D].Province);
            if (Best == INDEX_NONE || Km < BestKm) { Best = D; BestKm = Km; }
        }
        if (Best == INDEX_NONE) return FString(TEXT("Bu \u00fclkede depon yok: mal toptanc\u0131dan gelir."));
        const bool bHome = Country == State.CountryId && Province == MarketStart::HomeProvince(State);
        const float Limit = static_cast<float>(bHome ? MarketDepots::HomeRangeKm : MarketDepots::RangeKm);
        const FString Name = CityName(Sites[Best].Country, Sites[Best].Province);
        if (BestKm <= Limit) return FString::Printf(TEXT("En yak\u0131n depo: %s \u00b7 %.0f km"), *Name, BestKm);
        return FString::Printf(TEXT("En yak\u0131n depo: %s \u00b7 %.0f km (menzil d\u0131\u015f\u0131: mal toptanc\u0131dan)"), *Name, BestKm);
    }

    // G-089 (M23): a province the depot builder offers: the suggested one of each country first, then the ones with
    // most of our open branches within the depot's range.
    struct FDepotChoice
    {
        FString Country;
        FString Province;
        int32 Reach = 0;              // open branches of the country within MarketDepots::RangeKm
        bool bSuggested = false;
        FString Advice;               // the suggestion's line (suggested rows only)
    };

    TArray<FDepotChoice> DepotChoices(const FMarketState& State, int32 Most)
    {
        TArray<FString> Countries;
        Countries.Add(State.CountryId);
        TArray<FString> Where;        // "country|province" of every open branch
        for (int32 I = 0; I < State.Branches.Num(); ++I)
        {
            if (State.Branches[I].Stage != static_cast<uint8>(MarketBranches::EStage::Open)) continue;
            const FString Country = MarketBranches::CountryOf(State, State.Branches[I]);
            Countries.AddUnique(Country);
            Where.Add(Country + TEXT("|") + MarketManagers::AreaOfBranch(State, I, MarketManagers::ELevel::Province));
        }
        TArray<FDepotChoice> List;
        for (const FString& Country : Countries)
        {
            const MarketCountry::FProfile* Pack = MarketCountry::Find(Country);
            if (!Pack) continue;
            const MarketDepots::FAdvice Advice = MarketDepots::SuggestDepotProvince(State, Country);
            for (const MarketCountry::FCity& City : Pack->Cities)
            {
                if (MarketDepots::HasDepotIn(State, Country, City.Id)) continue;
                FDepotChoice Choice;
                Choice.Country = Country;
                Choice.Province = City.Id;
                for (const FString& Spot : Where)
                {
                    FString SpotCountry, SpotProvince;
                    if (!Spot.Split(TEXT("|"), &SpotCountry, &SpotProvince) || SpotCountry != Country) continue;
                    if (MarketDepots::DistanceKm(Country, SpotProvince, City.Id) <= static_cast<float>(MarketDepots::RangeKm)) ++Choice.Reach;
                }
                Choice.bSuggested = !Advice.Province.IsEmpty() && Advice.Province == City.Id;
                if (Choice.bSuggested) Choice.Advice = Advice.Text;
                if (Choice.Reach == 0 && !Choice.bSuggested) continue;
                List.Add(Choice);
            }
        }
        List.StableSort([](const FDepotChoice& A, const FDepotChoice& B)
        {
            if (A.bSuggested != B.bSuggested) return A.bSuggested;
            return A.Reach > B.Reach;
        });
        if (List.Num() > Most) List.SetNum(Most);
        return List;
    }
}

// Named namespace (unity build): C5 menu simplification helpers (A's list, Docs/Surec/akislar/A.md).
namespace MarketMenuSimplifyUi
{
    // Our row of a national / world table has a revenue (on day one we are listed with nothing).
    bool C5UsRanked(const TArray<MarketChains::FStanding>& Table)
    {
        for (const MarketChains::FStanding& Row : Table) if (Row.bUs) return Row.Revenue > 0.0;
        return false;
    }

    // What we could put on the table for a chain: the till and the most one bank lends for its takeover.
    int64 C5BidReach(const FMarketState& State, int32 ChainIndex)
    {
        const int64 TargetEbitda = MarketChains::YearProfit(State, ChainIndex);
        int64 Room = 0;
        for (int32 BankIndex = 0; BankIndex < MarketBanking::BankCount; ++BankIndex)
            Room = FMath::Max(Room, MarketBanking::AcquisitionRoom(State, BankIndex, TargetEbitda));
        return FMath::Max<int64>(0, State.Cash) + Room;
    }

    // A bid is worth showing once the reach is at least a third of the price.
    bool C5BidPlausible(const FMarketState& State, int32 ChainIndex)
    {
        return C5BidReach(State, ChainIndex) * 3 >= MarketChains::BidPrice(State, ChainIndex);
    }

    // The first store's loans: the nearest MaxRows installments and one total line (the rest in the tooltip).
    FString C5FamilyLoans(const FMarketState& State, int32 MaxRows)
    {
        const TArray<FMarketLoan>& All = State.Loans;
        if (All.Num() == 0) return FString();
        TArray<int32> Order;
        int64 Left = 0, Monthly = 0;
        for (int32 I = 0; I < All.Num(); ++I) { Order.Add(I); Left += All[I].Remaining; Monthly += All[I].Installment; }
        Order.Sort([&All](int32 A, int32 B) { return All[A].NextDueDay < All[B].NextDueDay; });
        FString Text;
        const int32 Rows = MaxRows > 0 ? FMath::Min(MaxRows, Order.Num()) : Order.Num();
        for (int32 I = 0; I < Rows; ++I)
        {
            const FMarketLoan& Item = All[Order[I]];
            Text += FString::Printf(TEXT("\n\u2022 %d. g\u00fcn taksit %s \u00b7 kalan %s"), Item.NextDueDay, *MarketMenuUi::Tl(Item.Installment), *MarketMenuUi::Tl(Item.Remaining));
        }
        if (Order.Num() > Rows) Text += FString::Printf(TEXT("\n\u2022 %d kredi daha (ayr\u0131nt\u0131 \u00fczerinde)"), Order.Num() - Rows);
        Text += FString::Printf(TEXT("\nToplam %d kredi \u00b7 kalan %s \u00b7 taksitler %s"), All.Num(), *MarketMenuUi::Tl(Left), *MarketMenuUi::Tl(Monthly));
        return Text;
    }

    // One reason when no company bank lends and they all give the same one (empty otherwise).
    FString C5CommonBankLock(const FMarketState& State)
    {
        FString Common;
        for (int32 BankIndex = 0; BankIndex <= MarketBanking::BondBank; ++BankIndex)
        {
            FString Why;
            if (MarketBanking::CanBorrow(State, BankIndex, Why)) return FString();
            if (BankIndex == 0) Common = Why;
            else if (Why != Common) return FString();
        }
        return Common.IsEmpty() ? FString(TEXT("\u015eirket kredisi \u015fimdilik kapal\u0131.")) : Common;
    }

    // C7 (A menu comparison): nothing to manage yet (day one): no branch ever, no manager, no subsidiary.
    bool C7NoNetworkYet(const FMarketState& State)
    {
        if (State.Branches.Num() > 0 || State.Management.Managers.Num() > 0) return false;
        for (const FMarketChain& Chain : State.Rivals.Chains) if (Chain.bOurs && !Chain.bGone) return false;
        return true;
    }

    // M37: a salary step as a chip ("1\u00d7", "1,5\u00d7", "40\u00d7"; tenths of the minimum wage).
    FString M37SalaryChip(int32 StepX10)
    {
        return StepX10 % 10 == 0 ? FString::Printf(TEXT("%d\u00d7"), StepX10 / 10) : FString::Printf(TEXT("%d,%d\u00d7"), StepX10 / 10, StepX10 % 10);
    }

    // M37: the dividend of one step, rounded as MarketOwner::PayDividend rounds it.
    int64 M37DividendGross(const FMarketState& State, int32 Step)
    {
        return FMath::Max<int64>(100, MarketOwner::DividendRoom(State) * MarketOwner::DividendPercents[Step] / 100 / 100 * 100);
    }

    // M37: personal money a capital step puts into the company (as MarketOwner::PutCapital).
    int64 M37CapitalAmount(const FMarketState& State, int32 Step)
    {
        return State.Owner.Wealth * MarketOwner::CapitalPercents[Step] / 100 / 100 * 100;
    }

    // M37: why no dividend can be paid now (empty = it can).
    FString M37DividendWhy(const FMarketState& State)
    {
        if (MarketOwner::DividendRoom(State) > 0) return FString();
        if (State.Day < State.RescueUntil) return FString(TEXT("Kurtarma plan\u0131 s\u00fcrerken k\u00e2r pay\u0131 da\u011f\u0131t\u0131lamaz."));
        if (State.TroubleStage > 0) return FString(TEXT("Kasa eksideyken k\u00e2r pay\u0131 da\u011f\u0131t\u0131lamaz."));
        if (MarketOwner::Distributable(State) <= 0) return FString(TEXT("Da\u011f\u0131t\u0131lacak k\u00e2r yok: ge\u00e7en y\u0131l\u0131n net k\u00e2r\u0131 ya yok ya da da\u011f\u0131t\u0131ld\u0131."));
        return FString::Printf(TEXT("Kasada \u015firketin bir ayl\u0131k sabit gideri (%s) kalmal\u0131."), *MarketMenuUi::Tl(MarketFinance::CompanyMonthCost(State)));
    }

    // M38: where a campaign runs, for the running list.
    FString M38StoreName(const FMarketState& State, int32 Store)
    {
        if (Store == MarketLedger::AllStores) return FString(TEXT("b\u00fct\u00fcn ma\u011fazalar"));
        if (Store == MarketLedger::FirstStore) return FString(TEXT("ilk d\u00fckk\u00e2n"));
        return State.Branches.IsValidIndex(Store) ? State.Branches[Store].Name : FString(TEXT("kapanan ma\u011faza"));
    }

    // M38: K-th campaign running in any store (index into State.Promotions), INDEX_NONE when fewer run.
    int32 M38RunningPromotion(const FMarketState& State, int32 Slot)
    {
        int32 Seen = 0;
        for (int32 I = 0; I < State.Promotions.Num(); ++I)
            if (MarketPromotions::IsRunning(State.Promotions[I], State.Day) && Seen++ == Slot) return I;
        return INDEX_NONE;
    }
}

// ---------------------------------------------------------------------------------------------------------------
// 3 Urunler ve fiyat

TSharedRef<SWidget> SMarketMenu::PricesPage()
{
    auto G = [this] { return Game.Get(); };
    auto Current = [G] { return G() && G()->Products.IsValidIndex(G()->MenuProduct) ? G()->MenuProduct : 0; };
    auto Valid = [G, Current] { return G() && G()->Products.IsValidIndex(Current()) && G()->State.Stock.IsValidIndex(Current()); };

    TSharedRef<SScrollBox> List = SNew(SScrollBox);
    const int32 Count = G() ? G()->Products.Num() : 0;
    for (int32 I = 0; I < Count; ++I)
    {
        auto Has = [G, I] { return G() && G()->Products.IsValidIndex(I) && G()->State.Stock.IsValidIndex(I); };
        List->AddSlot().Padding(0.f, 3.f)
        [
            SNew(SBox)
            .Visibility_Lambda([this, G, Has, I] { return Has() && (PriceCategory.IsEmpty() || G()->Products[I].Category == PriceCategory) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SButton).ButtonStyle(&RowStyle).IsFocusable(false).ContentPadding(FMargin(10.f, 6.f))
                .ButtonColorAndOpacity_Lambda([this, Current, I] { return FSlateColor(Color(Current() == I ? ERole::Inset : ERole::Panel)); })
                .OnClicked_Lambda([G, I] { if (AMarketGameMode* Mode = G(); Mode && Mode->Products.IsValidIndex(I)) Mode->MenuProduct = I; return FReply::Handled(); })
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)[ ProductPicture(I, 40.f) ]
                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()[ Label([G, Has, I] { return Has() ? G()->ProductName(I) : FString(); }, 11, ERole::Text, true) ]
                        + SVerticalBox::Slot().AutoHeight()
                        [ Label([G, Has, I]
                        {
                            if (!Has()) return FString();
                            const FString Promo = MarketPromotions::Badge(G()->State, G()->Products, I);
                            return MarketMenuUi::Title(G()->Products[I].Category) + (Promo.IsEmpty() ? FString() : TEXT(" \u00b7 ") + Promo);
                        }, 9, ERole::Muted) ]
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)[ Label([G, Has, I] { return Has() ? MarketMenuUi::Tl(G()->State.Stock[I].Price) : FString(); }, 12, ERole::Text, true) ]
                        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
                        [ Label([G, Has, I]
                        {
                            int64 Price = 0;
                            return Has() && MarketMenuUi::CheapestRival(*G(), I, Price) != INDEX_NONE ? FString::Printf(TEXT("rakip en ucuz %s"), *MarketMenuUi::Tl(Price)) : FString(TEXT("rakiplerde yok"));
                        }, 9, ERole::Muted) ]
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [
                        SNew(SBox).WidthOverride(58.f).HAlign(HAlign_Right)
                        [
                            LabelBy([G, Has, I]
                            {
                                int64 Price = 0;
                                if (!Has() || MarketMenuUi::CheapestRival(*G(), I, Price) == INDEX_NONE) return FString(TEXT("tek biz"));
                                const int64 Ours = G()->State.Stock[I].Price;
                                return Ours < Price ? FString(TEXT("ucuz")) : Ours > Price ? FString(TEXT("pahal\u0131")) : FString(TEXT("ayn\u0131"));
                            }, 10, [G, Has, I]
                            {
                                int64 Price = 0;
                                if (!Has() || MarketMenuUi::CheapestRival(*G(), I, Price) == INDEX_NONE) return ERole::Good;
                                const int64 Ours = G()->State.Stock[I].Price;
                                return Ours < Price ? ERole::Good : Ours > Price ? ERole::Bad : ERole::Muted;
                            }, true)
                        ]
                    ]
                ]
            ]
        ];
    }

    TSharedRef<SVerticalBox> RivalRows = SNew(SVerticalBox);
    for (int32 Rival = 0; Rival < 3; ++Rival)
    {
        RivalRows->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 8.f))
            .Visibility_Lambda([G, Rival] { return Rival < MarketMenuUi::RivalCount(G()) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)
                [ Badge(MarketMenuUi::RivalLogoKey(G(), Rival), MarketMenuUi::Initials(MarketMenuUi::RivalName(G(), Rival)), MarketMenuUi::RivalColor(Rival), 34.f) ]
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Label([G, Rival] { return MarketMenuUi::RivalName(G(), Rival); }, 12, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        Label([G, Valid, Current, Rival]
                        {
                            if (!Valid()) return FString();
                            int64 Price = 0;
                            const FString Kind = MarketMenuUi::RivalKind(G(), Rival);
                            if (!MarketMenuUi::RivalShelfPrice(*G(), Current(), Rival, Price) || G()->Products[Current()].BasePrice <= 0) return Kind;
                            const float Factor = static_cast<float>(Price) / static_cast<float>(G()->Products[Current()].BasePrice);
                            if (Factor < 0.995f) return Kind + FString::Printf(TEXT(" \u00b7 listenin %%%d alt\u0131nda"), FMath::RoundToInt32((1.f - Factor) * 100.f));
                            if (Factor > 1.005f) return Kind + FString::Printf(TEXT(" \u00b7 listenin %%%d \u00fcst\u00fcnde"), FMath::RoundToInt32((Factor - 1.f) * 100.f));
                            return Kind;
                        }, 9, ERole::Muted)
                    ]
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f)
                [
                    Label([G, Valid, Current, Rival]
                    {
                        int64 Price = 0;
                        return Valid() && MarketMenuUi::RivalShelfPrice(*G(), Current(), Rival, Price) ? MarketMenuUi::Tl(Price) : FString(TEXT("Rafta yok"));
                    }, 14, ERole::Text, true)
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [
                    SNew(SBox).WidthOverride(70.f).HAlign(HAlign_Right)
                    [
                        LabelBy([G, Valid, Current, Rival]
                        {
                            int64 Price = 0;
                            if (!Valid() || !MarketMenuUi::RivalShelfPrice(*G(), Current(), Rival, Price) || Price <= 0) return FString();
                            const int32 Diff = FMath::RoundToInt32((static_cast<double>(G()->State.Stock[Current()].Price) / Price - 1.0) * 100.0);
                            return Diff == 0 ? FString(TEXT("ayn\u0131")) : FString::Printf(TEXT("biz %s%%%d"), Diff > 0 ? TEXT("+") : TEXT("-"), FMath::Abs(Diff));
                        }, 10, [G, Valid, Current, Rival]
                        {
                            int64 Price = 0;
                            if (!Valid() || !MarketMenuUi::RivalShelfPrice(*G(), Current(), Rival, Price)) return ERole::Muted;
                            return G()->State.Stock[Current()].Price > Price ? ERole::Bad : ERole::Good;
                        }, true)
                    ]
                ]
            ]
        ];
    }

    // The label picture of the selected product (or a coloured initial when the studio has none).
    TSharedRef<SWidget> Picture = SNew(SBox).WidthOverride(92.f).HeightOverride(92.f)
    [
        SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(4.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
        [
            SNew(SOverlay)
            + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
            [
                SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
                .Visibility_Lambda([this, Current] { return PictureBrush(Current()) ? EVisibility::Visible : EVisibility::Collapsed; })
                [ SNew(SImage).Image_Lambda([this, Current] { return PictureBrush(Current()); }) ]
            ]
            + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
            [
                SNew(STextBlock).Font(MarketMenuUi::MenuFont(true, 26))
                .Visibility_Lambda([this, Current] { return PictureBrush(Current()) ? EVisibility::Collapsed : EVisibility::Visible; })
                .ColorAndOpacity_Lambda([G, Valid, Current] { return FSlateColor(Valid() ? FLinearColor(G()->Products[Current()].Color) : FLinearColor::Gray); })
                .Text_Lambda([G, Valid, Current] { return FText::FromString(Valid() ? MarketMenuUi::Initials(G()->ProductName(Current())) : FString()); })
            ]
        ]
    ];

    TSharedRef<SWidget> Detail = Card(
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)[ Picture ]
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ Label([G, Valid, Current] { return Valid() ? G()->ProductName(Current()) : FString(TEXT("\u00dcr\u00fcn yok")); }, 20, ERole::Text, true, true) ]
                + SVerticalBox::Slot().AutoHeight()
                [ Label([G, Valid, Current] { return Valid() ? FString::Printf(TEXT("%s \u00b7 %s"), *MarketMenuUi::Title(G()->Products[Current()].Category), *G()->Products[Current()].Brand) : FString(); }, 10, ERole::Muted) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
                [ Label([G, Valid, Current]
                {
                    if (!Valid()) return FString();
                    const FMarketStock& S = G()->State.Stock[Current()];
                    return S.Capacity > 0 ? FString::Printf(TEXT("Rafta %d / %d \u00b7 depoda %d"), S.Shelf, S.Capacity, S.Warehouse) : FString(TEXT("Rafta de\u011fil (R ile reyona koy)"));
                }, 10, ERole::Muted) ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("B\u0130Z\u0130M F\u0130YAT")) ]
                + SVerticalBox::Slot().AutoHeight()[ Label([G, Valid, Current] { return Valid() ? MarketMenuUi::Tl(G()->State.Stock[Current()].Price) : FString(); }, 28, ERole::Text, true) ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(18.f, 0.f, 6.f, 0.f)
            [ Button([] { return FString(TEXT("-")); }, [this, Current] { Do(TEXT("PriceDown"), Current()); }, false, Valid) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ Button([] { return FString(TEXT("+")); }, [this, Current] { Do(TEXT("PriceUp"), Current()); }, false, Valid) ]
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).HAlign(HAlign_Right)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)[ Section(TEXT("ALAN M\u00dc\u015eTER\u0130")) ]
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
                [ LabelBy([G, Valid, Current] { return Valid() ? FString::Printf(TEXT("~%%%d"), FMath::RoundToInt32(MarketMenuUi::BuyChanceOf(*G(), Current()) * 100.0)) : FString(); }, 24,
                    [G, Valid, Current] { const double Chance = Valid() ? MarketMenuUi::BuyChanceOf(*G(), Current()) : 1.0; return Chance >= 0.6 ? ERole::Good : Chance >= 0.35 ? ERole::Warn : ERole::Bad; }, true) ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [
            Label([G, Valid, Current]
            {
                if (!Valid()) return FString();
                const FMarketProduct& P = G()->Products[Current()];
                const int64 Ours = G()->State.Stock[Current()].Price;
                const int32 Margin = Ours > 0 ? FMath::RoundToInt32(static_cast<double>(Ours - P.Cost) / Ours * 100.0) : 0;
                return FString::Printf(TEXT("Al\u0131\u015f %s \u00b7 liste fiyat\u0131 %s \u00b7 k\u00e2r marj\u0131 %%%d"), *MarketMenuUi::Tl(P.Cost), *MarketMenuUi::Tl(P.BasePrice), Margin);
            }, 10, ERole::Muted)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 6.f)
        [ Label([G] { return G() ? FString(TEXT("RAK\u0130P F\u0130YATLARI \u00b7 ")) + MarketStart::PlaceText(G()->State) : FString(TEXT("RAK\u0130P F\u0130YATLARI")); }, 10, ERole::Muted, true) ] // C3 (A5): the chosen province
        + SVerticalBox::Slot().AutoHeight()[ RivalRows ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
        [
            Label([G, Valid, Current]
            {
                if (!Valid()) return FString();
                const int64 Theirs = MarketDemand::RivalPrice(G()->Products[Current()], G()->RivalPriceFactor(Current()));
                return FString::Printf(TEXT("M\u00fc\u015fterinin akl\u0131ndaki rakip fiyat\u0131: %s. Pahal\u0131 bulan m\u00fc\u015fteri \u00fcr\u00fcn\u00fc almaz; sad\u0131k m\u00fc\u015fteri biraz daha ho\u015fg\u00f6r\u00fcl\u00fcd\u00fcr."), *MarketMenuUi::Tl(Theirs));
            }, 10, ERole::Muted, false, true)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [ Label([G, Valid, Current] { return Valid() ? G()->OrderAdvice(Current()) : FString(); }, 10, ERole::Muted, false, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [ Label([G, Valid, Current]
            {
                if (!Valid()) return FString();
                const FString Promo = MarketPromotions::Badge(G()->State, G()->Products, Current());
                return Promo.IsEmpty() ? FString(TEXT("Bu \u00fcr\u00fcnde kampanya yok.")) : TEXT("Kampanyada: ") + Promo;
            }, 10, ERole::Text) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ Button([] { return FString(TEXT("Kampanyalar  \u203a")); }, [this] { Go(Promotions); }) ]
        ],
        ERole::Panel, FMargin(22.f, 20.f));

    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(0.9f).Padding(0.f, 0.f, 16.f, 0.f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[ CategoryChips(&PriceCategory) ]
            + SVerticalBox::Slot().FillHeight(1.f)[ List ]
        ]
        + SHorizontalBox::Slot().FillWidth(1.f)
        [ SNew(SScrollBox) + SScrollBox::Slot()[ Detail ] ];
}

// ---------------------------------------------------------------------------------------------------------------
// 4 Kampanyalar

TSharedRef<SWidget> SMarketMenu::PromotionsPage()
{
    // G-064 promotions on one page: start one for the selected product, the neighbourhood flyer, the wholesaler's
    // funded offer and every running promotion with its own stop button.
    auto G = [this] { return Game.Get(); };
    auto Current = [G] { return G() && G()->Products.IsValidIndex(G()->MenuProduct) ? G()->MenuProduct : 0; };
    auto Valid = [G, Current] { return G() && G()->Products.IsValidIndex(Current()) && G()->State.Stock.IsValidIndex(Current()); };
    auto PickedName = [G, Valid, Current] { return Valid() ? G()->ProductName(Current()) : FString(); };
    auto Aisle = [G, Valid, Current] { return Valid() ? MarketMenuUi::Title(G()->Products[Current()].Category) : FString(); };

    // Product picker (same filter chips as the price page).
    TSharedRef<SScrollBox> Picker = SNew(SScrollBox);
    const int32 Count = G() ? G()->Products.Num() : 0;
    for (int32 I = 0; I < Count; ++I)
    {
        auto Has = [G, I] { return G() && G()->Products.IsValidIndex(I); };
        Picker->AddSlot().Padding(0.f, 2.f)
        [
            SNew(SBox)
            .Visibility_Lambda([this, G, Has, I] { return Has() && (PriceCategory.IsEmpty() || G()->Products[I].Category == PriceCategory) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SButton).ButtonStyle(&RowStyle).IsFocusable(false).ContentPadding(FMargin(10.f, 5.f))
                .ButtonColorAndOpacity_Lambda([this, Current, I] { return FSlateColor(Color(Current() == I ? ERole::Inset : ERole::Panel)); })
                .OnClicked_Lambda([G, I] { if (AMarketGameMode* Mode = G(); Mode && Mode->Products.IsValidIndex(I)) Mode->MenuProduct = I; return FReply::Handled(); })
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)[ ProductPicture(I, 28.f) ]
                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Label([G, Has, I] { return Has() ? G()->ProductName(I) : FString(); }, 11, ERole::Text, true) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [ Label([G, Has, I] { return Has() ? MarketPromotions::Badge(G()->State, G()->Products, I) : FString(); }, 9, ERole::Accent, true) ]
                ]
            ]
        ];
    }

    // M38: every store's campaigns (the first store, each branch, the company's in every store); only the family
    // shop's and the company's can be stopped here.
    TSharedRef<SVerticalBox> Running = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 40; ++Slot)
    {
        auto Index = [G, Slot] { return G() ? MarketMenuSimplifyUi::M38RunningPromotion(G()->State, Slot) : INDEX_NONE; };
        Running->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 8.f))
            .Visibility_Lambda([Index] { return Index() != INDEX_NONE ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [ Label([G, Index]
                {
                    const int32 At = Index();
                    if (At == INDEX_NONE || !G()) return FString();
                    const FMarketPromotion& P = G()->State.Promotions[At];
                    const FString Where = MarketMenuSimplifyUi::M38StoreName(G()->State, P.Store) + (P.bManager ? FString(TEXT(" (m\u00fcd\u00fcr)")) : FString());
                    if (P.Store >= 0) return FString::Printf(TEXT("%s \u00b7 %s \u00b7 %d. g\u00fcne kadar"), *MarketPromotions::Describe(P, G()->Products), *Where, P.EndDay); // a branch's sales are in its weekly line
                    return FString::Printf(TEXT("%s \u00b7 %s \u00b7 %d. g\u00fcne kadar \u00b7 \u015fimdiye dek %d adet"), *MarketPromotions::Describe(P, G()->Products), *Where, P.EndDay, P.Sold);
                }, 11, ERole::Text, false, true) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
                [ Button([] { return FString(TEXT("Durdur")); }, [this, Index] { const int32 At = Index(); if (At != INDEX_NONE) Manage(TEXT("StopPromotion"), At); }, false,
                    [G, Index] { const int32 At = Index(); return G() && G()->State.Promotions.IsValidIndex(At) && MarketPromotions::IsActive(G()->State.Promotions[At], G()->State.Day); }) ]
            ]
        ];
    }

    // G-078 campaign builder rows.
    auto ChipRow = [this](int32 Count, TFunction<FString(int32)> Text, TFunction<bool(int32)> Selected, TFunction<void(int32)> Pick) -> TSharedRef<SWidget>
    {
        TSharedRef<SWrapBox> Row = SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(4.f, 4.f));
        for (int32 I = 0; I < Count; ++I)
            Row->AddSlot()[ Choice(Text(I), [Selected, I] { return Selected(I); }, [Pick, I] { Pick(I); }) ];
        return Row;
    };
    static const int32 Percents[6] = { 5, 10, 15, 20, 25, 30 };
    static const int32 DayChoices[4] = { 3, 7, 10, 14 };
    TSharedRef<SWidget> PromoScopeRow = ChipRow(static_cast<int32>(MarketPromotions::EScope::Count),
        [](int32 I) { return MarketPromotions::ScopeName(static_cast<MarketPromotions::EScope>(I)); },
        [this](int32 I) { return PromoScope == I; }, [this](int32 I) { PromoScope = I; });
    TSharedRef<SWidget> PromoMechanicRow = ChipRow(static_cast<int32>(MarketPromotions::EMechanic::Count),
        [](int32 I) { return I == 0 ? FString(TEXT("% indirim")) : MarketPromotions::MechanicName(static_cast<MarketPromotions::EMechanic>(I), 0); },
        [this](int32 I) { return PromoMechanic == I; }, [this](int32 I) { PromoMechanic = I; });
    TSharedRef<SWidget> PromoPercentRow = ChipRow(6, [](int32 I) { return FString::Printf(TEXT("%%%d"), Percents[I]); },
        [this](int32 I) { return PromoPercent == Percents[I]; }, [this](int32 I) { PromoPercent = Percents[I]; PromoMechanic = 0; });
    TSharedRef<SWidget> PromoDaysRow = ChipRow(4, [](int32 I) { return FString::Printf(TEXT("%d g\u00fcn"), DayChoices[I]); },
        [this](int32 I) { return PromoDays == DayChoices[I]; }, [this](int32 I) { PromoDays = DayChoices[I]; });
    // M38: where the next campaign runs (State.PromoStore): the first shop, every store, or a branch chosen from
    // Ma\u011fazalar (its chip is shown while it is the choice).
    auto PromoBranchChosen = [G] { return G() && G()->State.Branches.IsValidIndex(G()->State.PromoStore); };
    TSharedRef<SWidget> PromoWhereRow = SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(4.f, 4.f))
        + SWrapBox::Slot()[ Choice(TEXT("\u0130lk d\u00fckk\u00e2n"), [G] { return G() && G()->State.PromoStore == MarketLedger::FirstStore; },
            [this] { Manage(TEXT("PromoStore"), MarketLedger::FirstStore); }) ]
        + SWrapBox::Slot()[ Choice(TEXT("B\u00fct\u00fcn ma\u011fazalar"), [G] { return G() && G()->State.PromoStore == MarketLedger::AllStores; },
            [this] { Manage(TEXT("PromoStore"), MarketLedger::AllStores); }) ]
        + SWrapBox::Slot()
        [
            SNew(SBox).Visibility_Lambda([PromoBranchChosen] { return PromoBranchChosen() ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SButton).ButtonStyle(&PillStyle).IsFocusable(false).ContentPadding(FMargin(12.f, 5.f))
                .ButtonColorAndOpacity_Lambda([this] { return FSlateColor(Color(ERole::Primary)); })
                [
                    SNew(STextBlock).Font(MarketMenuUi::MenuFont(true, 10))
                    .Text_Lambda([G, PromoBranchChosen] { return FText::FromString(PromoBranchChosen() ? G()->State.Branches[G()->State.PromoStore].Name : FString()); })
                    .ColorAndOpacity_Lambda([this] { return FSlateColor(Color(ERole::PrimaryText)); })
                ]
            ]
        ];

    TSharedRef<SWidget> Start = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("YEN\u0130 KAMPANYA \u00b7 SE\u00c7\u0130L\u0130 \u00dcR\u00dcN")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 2.f)[ Label(PickedName, 16, ERole::Text, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
        [ Label([G, Valid, Current, Aisle]
        {
            if (!Valid()) return FString();
            const FString Promo = MarketPromotions::Badge(G()->State, G()->Products, Current());
            return FString::Printf(TEXT("%s reyonu \u00b7 rafta %s%s"), *Aisle(), *MarketMenuUi::Tl(G()->State.Stock[Current()].Price), Promo.IsEmpty() ? TEXT("") : *(TEXT(" \u00b7 \u015fu an ") + Promo));
        }, 10, ERole::Muted) ]
        // G-078 (karar J07): scope x mechanic x percent x days, from the selected product.
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[ Fixed(TEXT("Nerede?"), 10, ERole::Muted) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[ PromoWhereRow ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[ Fixed(TEXT("Kapsam"), 10, ERole::Muted) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[ PromoScopeRow ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[ Fixed(TEXT("Kampanya t\u00fcr\u00fc"), 10, ERole::Muted) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[ PromoMechanicRow ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[ Fixed(TEXT("\u0130ndirim ve s\u00fcre"), 10, ERole::Muted) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[ PromoPercentRow ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[ PromoDaysRow ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
        [ Label([this, G, Valid, Current]
        {
            if (!Valid()) return FString();
            const auto Scope = static_cast<MarketPromotions::EScope>(PromoScope);
            const FString Key = MarketPromotions::ScopeKeyOf(G()->Products, Current(), Scope);
            return FString::Printf(TEXT("%s \u00b7 %s \u00b7 %d g\u00fcn \u00b7 %d \u00fcr\u00fcn"),
                *(Scope == MarketPromotions::EScope::Product ? G()->ProductName(Current()) : Scope == MarketPromotions::EScope::Store ? FString(TEXT("t\u00fcm ma\u011faza")) : Key),
                *MarketPromotions::MechanicName(static_cast<MarketPromotions::EMechanic>(PromoMechanic), PromoPercent), PromoDays,
                MarketPromotions::ScopeSize(G()->Products, Current(), Scope));
        }, 11, ERole::Text, true, true) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 6.f))
            + SWrapBox::Slot()[ RiskyButton([] { return FString(TEXT("Kampanyay\u0131 ba\u015flat")); },
                [] { return FString(TEXT("Kampanya ba\u015flas\u0131n m\u0131? Kapsamdaki \u00fcr\u00fcnler daha \u00e7ok aran\u0131r ama her sat\u0131\u015fta k\u00e2r azal\u0131r; rakipler fark edebilir.")); },
                [this, Current] { Manage(TEXT("PromoScoped"), MarketPromotions::PackArg(Current(), static_cast<MarketPromotions::EScope>(PromoScope), static_cast<MarketPromotions::EMechanic>(PromoMechanic), PromoPercent, PromoDays)); }, Valid) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Gondol ba\u015f\u0131na koy")); }, [this, Current] { Manage(TEXT("Endcap"), Current()); }, false, Valid) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [ More([] { return FString(TEXT("Ayn\u0131 anda en \u00e7ok 3 kampanya y\u00fcr\u00fcr; gondol ba\u015f\u0131 bunlara say\u0131lmaz. Tek \u00fcr\u00fcn en g\u00fc\u00e7l\u00fc etkiyi verir; t\u00fcm ma\u011faza indirimi m\u00fc\u015fteri getirir ama her \u00fcr\u00fcn\u00fc az art\u0131r\u0131r.")); }) ]
        // M36: the neighbourhood flyer is gone; flyers are the company's ads (\u015eirket \u203a Reklam) for every shop.
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [ Label([] { return FString(TEXT("Reklam (bro\u015f\u00fcr, sosyal medya, TV...) \u015eirket \u203a Reklam'da: b\u00fct\u00fcn ma\u011fazalar\u0131 birlikte etkiler.")); }, 10, ERole::Muted, false, true) ]);

    TSharedRef<SWidget> Offer = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("TOPTANCI TEKL\u0130F\u0130")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 8.f)
        [ Label([G]
        {
            if (!G()) return FString();
            if (!MarketMenuPagesUi::OfferWaiting(G()->State))
                return FString::Printf(TEXT("\u015eu an teklif yok. Toptanc\u0131n\u0131n g\u00fcveni %d'i ge\u00e7ince ara s\u0131ra destekli teklif getirir: rafta %%%d indirim yaparsan al\u0131\u015fta %%%d indirim."),
                    MarketPromotions::OfferTrust, MarketPromotions::DealShelfCut, MarketPromotions::DealCostCut);
            return MarketPromotions::Describe(G()->State.Offer, G()->Products) + FString::Printf(TEXT(" \u00b7 teklif %d. g\u00fcne kadar ge\u00e7erli."), G()->State.Offer.EndDay);
        }, 11, ERole::Text, false, true) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)
            [ Button([] { return FString(TEXT("Kabul et")); }, [this] { Manage(TEXT("AcceptOffer"), INDEX_NONE); }, true, [G] { return G() && MarketMenuPagesUi::OfferWaiting(G()->State); }) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ Button([] { return FString(TEXT("Geri \u00e7evir")); }, [this] { Manage(TEXT("DeclineOffer"), INDEX_NONE); }, false, [G] { return G() && MarketMenuPagesUi::OfferWaiting(G()->State); }) ]
        ]);

    TSharedRef<SWidget> Now = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Section(TEXT("Y\u00dcR\u00dcYEN KAMPANYALAR")) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SBox).Visibility_Lambda([G] { return G() && MarketMenuSimplifyUi::M38RunningPromotion(G()->State, 0) == INDEX_NONE ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Fixed(TEXT("Y\u00fcr\u00fcyen kampanya yok. Biten her kampanyan\u0131n sonucu g\u00fcn raporunda yazar."), 11, ERole::Muted) ]
        ]
        + SVerticalBox::Slot().AutoHeight()[ Running ]);

    // Karar M25: brands' offers for room on our shelves, the deals running, the chosen aisle's brand shares.
    TSharedRef<SVerticalBox> BrandOffers = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < MarketBrands::MaxOpenOffers; ++Slot)
    {
        auto OfferAt = [G, Slot]() -> const FMarketBrandOffer* { return G() && G()->State.Brands.Offers.IsValidIndex(Slot) ? &G()->State.Brands.Offers[Slot] : nullptr; };
        BrandOffers->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 6.f))
            .Visibility_Lambda([OfferAt] { return OfferAt() ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [ Label([G, OfferAt] { const FMarketBrandOffer* O = OfferAt(); return O && G() ? MarketBrands::NameOf(G()->State, O->Brand) + TEXT(": ") + MarketBrands::DescribeOffer(G()->State, *O)
                    + FString::Printf(TEXT(" (%d g\u00fcn i\u00e7inde)"), FMath::Max(0, O->ExpireDay - G()->State.Day)) : FString(); }, 10, ERole::Text, false, true) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                [ Button([] { return FString(TEXT("Kabul et")); }, [this, OfferAt] { if (const FMarketBrandOffer* O = OfferAt()) Manage(TEXT("AcceptBrandOffer"), O->Id); }, true) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 0.f, 0.f)
                [ Button([] { return FString(TEXT("Geri \u00e7evir")); }, [this, OfferAt] { if (const FMarketBrandOffer* O = OfferAt()) Manage(TEXT("RejectBrandOffer"), O->Id); }) ]
            ]
        ];
    }
    TSharedRef<SWidget> BrandsCard = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Section(TEXT("MARKALAR")) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ More([] { return FString(TEXT("Markalar raflar\u0131nda daha \u00e7ok yer ister: pay\u0131 tutarsan her ay \u00f6der, rafa yeni \u00fcr\u00fcn\u00fcn\u00fc koyarsan bir kerede \u00f6der, \u00e7ok satarsan prim verir. Rafta \u00fclkedeki pay\u0131n\u0131n yar\u0131s\u0131n\u0131 bile bulamayan g\u00fc\u00e7l\u00fc marka k\u00fcser ve fiyat\u0131n\u0131 art\u0131r\u0131r; iyi dost indirim yapar.")); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([G] { return G() && G()->State.Brands.Offers.Num() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Fixed(TEXT("\u015eimdilik teklif yok. Markalar ay ba\u015f\u0131nda gelir."), 10, ERole::Muted) ]
        ]
        + SVerticalBox::Slot().AutoHeight()[ BrandOffers ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [ Label([G]
        {
            if (!G()) return FString();
            TArray<FString> Lines;
            for (const FMarketBrandDeal& Deal : G()->State.Brands.Deals) Lines.Add(TEXT("\u2022 ") + MarketBrands::DescribeDeal(G()->State, G()->Products, Deal));
            return Lines.Num() > 0 ? TEXT("S\u00dcREN ANLA\u015eMALAR\n") + FString::Join(Lines, TEXT("\n")) : FString();
        }, 10, ERole::Text, false, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [ Label([this, G]
        {
            if (!G()) return FString();
            if (PriceCategory.IsEmpty()) return FString(TEXT("Soldan bir reyon se\u00e7: markalar\u0131n \u00fclkedeki ve senin raf\u0131ndaki pay\u0131 burada."));
            return MarketBrands::AisleLine(G()->State, G()->Products, PriceCategory);
        }, 10, ERole::Muted, false, true) ]);

    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(0.8f).Padding(0.f, 0.f, 16.f, 0.f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[ CategoryChips(&PriceCategory) ]
            + SVerticalBox::Slot().FillHeight(1.f)[ Picker ]
        ]
        + SHorizontalBox::Slot().FillWidth(1.f)
        [
            SNew(SScrollBox)
            + SScrollBox::Slot()
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[ Start ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[ Now ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[ Offer ]
                + SVerticalBox::Slot().AutoHeight()[ BrandsCard ]
            ]
        ];
}

// ---------------------------------------------------------------------------------------------------------------
// 5 Rakipler

TSharedRef<SWidget> SMarketMenu::RivalsPage()
{
    auto G = [this] { return Game.Get(); };

    // Local: the district's share model and the chains' news.
    TSharedRef<SHorizontalBox> NewsCards = SNew(SHorizontalBox);
    for (int32 Rival = 0; Rival < 3; ++Rival)
    {
        NewsCards->AddSlot().FillWidth(1.f).Padding(0.f, 0.f, Rival < 2 ? 12.f : 0.f, 0.f)
        [
            SNew(SBox)
            .Visibility_Lambda([G, Rival] { return Rival < MarketMenuUi::RivalCount(G()) ? EVisibility::Visible : EVisibility::Hidden; })
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)
                        [ Badge(MarketMenuUi::RivalLogoKey(G(), Rival), MarketMenuUi::Initials(MarketMenuUi::RivalName(G(), Rival)), MarketMenuUi::RivalColor(Rival), 40.f) ]
                        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot().AutoHeight()[ Label([G, Rival] { return MarketMenuUi::RivalName(G(), Rival); }, 15, ERole::Text, true) ]
                            + SVerticalBox::Slot().AutoHeight()[ Label([G, Rival] { return MarketMenuUi::RivalKind(G(), Rival); }, 10, ERole::Muted) ]
                        ]
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 6.f)[ Section(TEXT("BUG\u00dcN")) ]
                    + SVerticalBox::Slot().AutoHeight()[ Label([G, Rival] { return G() ? MarketMenuUi::RivalsToday(*G(), Rival) : FString(); }, 11, ERole::Text, false, true) ])
            ]
        ];
    }
    TSharedRef<SWidget> Local = SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            Card(SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ Label([G] { return G() ? FString(TEXT("YEREL PAZAR \u00b7 ")) + MarketMenuPagesUi::CityName(G()->State.CountryId, MarketStart::HomeProvince(G()->State)).ToUpper() : FString(); }, 9, ERole::Muted, true) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 8.f)
                [ Label([G] { return G() ? FString::Printf(TEXT("\u00c7evre pay\u0131 %%%.0f  \u00b7  d\u00fckk\u00e2n\u0131n \u00e7evresindeki al\u0131\u015fveri\u015fin bu kadar\u0131 bizden"), G()->State.MarketShare) : FString(); }, 16, ERole::Text, true) ]
                + SVerticalBox::Slot().AutoHeight()[ Bar([G] { return G() ? G()->State.MarketShare / 100.f : 0.f; }, ERole::Accent) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
                [ Label([G]
                {
                    if (!G()) return FString();
                    const FString Home = MarketStart::HomeProvince(G()->State);
                    // M61b: the whole city is another figure: a corner shop is a fraction of a percent of it.
                    return FString::Printf(TEXT("\u015eehirdeki pay (%s) %%%.2f  \u00b7  ilin b\u00fct\u00fcn market al\u0131\u015fveri\u015finin bu kadar\u0131 bizden"),
                        *MarketMenuPagesUi::CityName(G()->State.CountryId, Home), MarketCompany::ProvinceShare(G()->State, G()->State.CountryId, Home));
                }, 12, ERole::Text) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
                [ More([] { return FString(TEXT("\u00c7evre pay\u0131 yaln\u0131z d\u00fckk\u00e2n\u0131n y\u00fcr\u00fcme mesafesindeki m\u00fc\u015fterileri sayar; b\u00fcy\u00fck \u015fehirde rakip \u00e7ok oldu\u011fu i\u00e7in do\u011fal olarak daha d\u00fc\u015f\u00fckt\u00fcr. \u015eehirdeki pay b\u00fct\u00fcn ilin market al\u0131\u015fveri\u015fine bakar; tek d\u00fckk\u00e2nla k\u00fc\u00e7\u00fck kal\u0131r, \u015fubelerle b\u00fcy\u00fcr. \u00c7evre pay\u0131 her g\u00fcn sonunda de\u011fi\u015fir: fiyat, dolu raf, bekleme, m\u00fc\u015fteri memnuniyeti, ildeki zincirler ve fiyat sava\u015flar\u0131.")); }) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
                [ Label([G]
                {
                    if (!G()) return FString();
                    return MarketMenuUi::RivalsToday(*G()); // E2: the home province's chains
                }, 10, ERole::Text, false, true) ])
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[ NewsCards ]
    ];

    // Akis C2b: the country's retailers and the world league come from MarketChains (tables built once a frame).
    struct FTableCache
    {
        uint64 Frame = 0;
        bool bFilled = false;
        TArray<MarketChains::FStanding> National;      // M53: the rows shown (the list's first N)
        TArray<MarketChains::FStanding> World;
        TArray<MarketChains::FStanding> NationalFull;  // every retailer, us included
        TArray<MarketChains::FStanding> WorldFull;
    };
    TSharedRef<FTableCache> Tables = MakeShared<FTableCache>();
    auto Fill = [G, Tables]()
    {
        if (Tables->bFilled && Tables->Frame == GFrameCounter) return;
        Tables->bFilled = true;
        Tables->Frame = GFrameCounter;
        Tables->NationalFull = G() ? MarketChains::NationalTable(G()->State, G()->State.CountryId) : TArray<MarketChains::FStanding>();
        Tables->WorldFull = G() ? MarketChains::WorldTable(G()->State) : TArray<MarketChains::FStanding>();
        Tables->National = G() ? MarketChains::Listed(Tables->NationalFull, MarketChains::NationalListSize(G()->State.CountryId)) : TArray<MarketChains::FStanding>();
        Tables->World = MarketChains::Listed(Tables->WorldFull, MarketChains::WorldListSize);
    };
    auto RowOf = [Tables, Fill](bool bWorld, int32 Slot) -> const MarketChains::FStanding*
    {
        Fill();
        const TArray<MarketChains::FStanding>& List = bWorld ? Tables->World : Tables->National;
        return List.IsValidIndex(Slot) ? &List[Slot] : nullptr;
    };
    // One table row: rank, badge, name and kind, a bar against the leader, the yearly revenue; for-sale chains
    // get a "Sat\u0131n al" button (asks first).
    auto TableRows = [this, G, RowOf](bool bWorld, int32 Rows) -> TSharedRef<SWidget>
    {
        TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
        for (int32 Slot = 0; Slot < Rows; ++Slot)
        {
            auto Row = [RowOf, bWorld, Slot] { return RowOf(bWorld, Slot); };
            auto ForSale = [G, Row]
            {
                const MarketChains::FStanding* R = Row();
                return R && G() && G()->State.Rivals.Chains.IsValidIndex(R->Chain) && G()->State.Rivals.Chains[R->Chain].bForSale && !G()->State.Rivals.Chains[R->Chain].bGone;
            };
            Box->AddSlot().AutoHeight().Padding(0.f, 3.f)
            [
                SNew(SBox).HeightOverride(46.f)
                // C7 (A menu comparison): our own row stays hidden while we have no revenue ("our brand, 0" on day one).
                .Visibility_Lambda([Row] { const MarketChains::FStanding* R = Row(); return R && !(R->bUs && R->Revenue <= 0.0) ? EVisibility::Visible : EVisibility::Collapsed; })
                .ToolTip(Tip([G, Row] { const MarketChains::FStanding* R = Row(); return R && G() && R->Chain != INDEX_NONE ? MarketChains::Describe(G()->State, R->Chain) : R ? R->Detail : FString(); }))
                [
                    SNew(SBorder).BorderImage(&SmallBrush).Padding(FMargin(12.f, 4.f)).VAlign(VAlign_Center)
                    .BorderBackgroundColor(ColBy([Row] { const MarketChains::FStanding* R = Row(); return R && R->bUs ? ERole::AccentSoft : R && R->bNemesis ? ERole::WarnSoft : ERole::Inset; }))
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
                        [ SNew(SBox).WidthOverride(28.f)[ Mono([Slot] { return FString::Printf(TEXT("%d."), Slot + 1); }, 12.f, [] { return ERole::Muted; }) ] ]
                        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot().AutoHeight()[ TextPx([Row] { const MarketChains::FStanding* R = Row(); return R ? (R->bNemesis ? R->Name + TEXT(" \u00b7 ezeli rakip") : R->Name) : FString(); }, 13.f,
                                [Row] { const MarketChains::FStanding* R = Row(); return R && R->bUs ? ERole::Accent : ERole::Text; }, true) ]
                            // M65: a country's own table shows the registered name before the kind (the world league only the brand).
                            + SVerticalBox::Slot().AutoHeight()[ TextPx([Row] { const MarketChains::FStanding* R = Row(); return R ? (R->LegalName.IsEmpty() ? R->Detail : R->LegalName + TEXT(" \u00b7 ") + R->Detail) : FString(); }, 10.f, [] { return ERole::Muted; }) ]
                        ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f)
                        [
                            SNew(SBox).WidthOverride(140.f)
                            [ Bar([RowOf, Row, bWorld] { const MarketChains::FStanding* R = Row(); const MarketChains::FStanding* Top = RowOf(bWorld, 0);
                                return R && Top && Top->Revenue > 0.0 ? static_cast<float>(R->Revenue / Top->Revenue) : 0.f; }, bWorld ? ERole::Info : ERole::Accent) ]
                        ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                        [
                            SNew(SBox).WidthOverride(130.f).HAlign(HAlign_Right)
                            [ Mono([Row, bWorld] { const MarketChains::FStanding* R = Row(); if (!R) return FString();
                                if (bWorld) return FString::Printf(TEXT("%.2f milyar"), R->Revenue / 1.0e9);
                                // C3 (A5): big revenues in short form so the column never cuts them (the full sum is in the tooltip).
                                const double Lira = R->Revenue / 100.0;
                                return Lira >= 1.0e9 ? FString::Printf(TEXT("%.2f milyar"), Lira / 1.0e9) : Lira >= 1.0e6 ? FString::Printf(TEXT("%.1f milyon"), Lira / 1.0e6)
                                    : MarketMenuUi::Tl(static_cast<int64>(R->Revenue)); }, 12.f, [] { return ERole::Text; }) ]
                        ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                        [
                            SNew(SBox).Visibility_Lambda([ForSale] { return ForSale() ? EVisibility::Visible : EVisibility::Collapsed; })
                            [ Button([] { return FString(TEXT("Sat\u0131n al")); }, [this, G, Row]
                            {
                                const MarketChains::FStanding* R = Row();
                                if (!R || !G()) return;
                                const int32 Chain = R->Chain;
                                FString Why;
                                if (!MarketChains::CanBuy(G()->State, Chain, Why, false)) { Ask(Why, [] {}); return; }
                                // M29: what the till lacks comes from a bank (an acquisition loan).
                                const int64 Cost = MarketChains::Price(G()->State, Chain);
                                const bool bLoan = G()->State.Cash < Cost + Cost / 20;
                                Ask(FString::Printf(TEXT("%s sat\u0131n al\u0131ns\u0131n m\u0131? Fiyat\u0131 %s. Ma\u011fazalar\u0131, illerde yer oldu\u011fu kadar \u015fubemiz olur; gerisi sat\u0131l\u0131r.%s"),
                                    *G()->State.Rivals.Chains[Chain].Name, *MarketMenuUi::Tl(Cost), bLoan ? TEXT(" Kasa yetmedi\u011fi i\u00e7in bankalara sat\u0131n alma kredisi ba\u015fvurusu yap\u0131l\u0131r; cevaplar birka\u00e7 g\u00fcnde gelir (Finans).") : TEXT("")),
                                    [this, Chain] { Manage(TEXT("DealFinance"), Chain); });
                            }, true) ]
                        ]
                        // M29: a takeover bid for a chain that is not for sale (local, regional and national ones).
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                        [
                            SNew(SBox).Visibility_Lambda([G, Row, ForSale]
                            {
                                const MarketChains::FStanding* R = Row();
                                FString Why;
                                // C5 (A menu list): only a bid we could plausibly fund (till + a bank's takeover loan >= a third of the price).
                                return R && G() && !R->bUs && R->Chain != INDEX_NONE && !ForSale() && MarketChains::CanBid(G()->State, R->Chain, Why, false)
                                    && MarketMenuSimplifyUi::C5BidPlausible(G()->State, R->Chain) ? EVisibility::Visible : EVisibility::Collapsed;
                            })
                            [ Button([] { return FString(TEXT("Teklif ver")); }, [this, G, Row]
                            {
                                const MarketChains::FStanding* R = Row();
                                if (!R || !G()) return;
                                const int32 Chain = R->Chain;
                                const float Chance = MarketChains::AcceptChance(G()->State, Chain);
                                const int64 Cost = MarketChains::BidPrice(G()->State, Chain);
                                Ask(FString::Printf(TEXT("%s i\u00e7in %s teklif edilsin mi? Kabul etme ihtimali %s. Y\u00f6netim kurulu 2-4 g\u00fcnde cevap verir. Reddederse dan\u0131\u015fmanlar %%0,5 al\u0131r ve zincir sana daha \u00e7ok k\u0131zar; kabul ederse 14 g\u00fcn i\u00e7inde \u00f6dersin (kasa yetmezse bankalara ba\u015fvurulur)."),
                                    *G()->State.Rivals.Chains[Chain].Name, *MarketMenuUi::Tl(Cost), Chance >= 0.5f ? TEXT("y\u00fcksek") : Chance >= 0.25f ? TEXT("orta") : TEXT("d\u00fc\u015f\u00fck")),
                                    [this, Chain] { Manage(TEXT("BidChainAsk"), Chain); });
                            }) ]
                        ]
                    ]
                ]
            ];
        }
        return Box;
    };
    auto OurLine = [G, Tables, Fill](bool bWorld) -> FString
    {
        if (!G()) return FString();
        Fill();
        const int32 Rank = MarketChains::OurRank(bWorld ? Tables->World : Tables->National);
        // M53: off the list, how far its last row is.
        if (!MarketMenuSimplifyUi::C5UsRanked(bWorld ? Tables->World : Tables->National))
        {
            const FString Outside = MarketChains::OutsideText(bWorld ? Tables->WorldFull : Tables->NationalFull,
                bWorld ? MarketChains::WorldListSize : MarketChains::NationalListSize(G()->State.CountryId), bWorld);
            return Outside.IsEmpty() ? FString(TEXT("Hen\u00fcz s\u0131ralamaya girmedin: ciro olu\u015ftuk\u00e7a listede yerini al\u0131rs\u0131n.")) : Outside;
        }
        const FMarketChainsState& R = G()->State.Rivals;
        const int32 Best = bWorld ? R.BestLeagueRank : R.BestNationalRank;
        return Rank > 0 ? FString::Printf(TEXT("%d. s\u0131radas\u0131n%s"), Rank, Best > 0 && Best < Rank ? *FString::Printf(TEXT(" \u00b7 en iyi %d."), Best) : TEXT("")) : FString(TEXT("Hen\u00fcz s\u0131ralamada de\u011filsin."));
    };

    // C7 (A menu comparison): one line instead of our empty row while we have no revenue.
    auto RankNote = [this, G, Tables, Fill](bool bWorld) -> TSharedRef<SWidget>
    {
        return SNew(SBox).Visibility_Lambda([G, Tables, Fill, bWorld]
        {
            if (!G()) return EVisibility::Collapsed;
            Fill();
            return MarketMenuSimplifyUi::C5UsRanked(bWorld ? Tables->World : Tables->National) ? EVisibility::Collapsed : EVisibility::Visible;
        })
        [ Label([] { return FString(TEXT("Sen hen\u00fcz bu listede yoksun: listenin sonuncusunu ciroda ge\u00e7ince girersin.")); }, 10, ERole::Muted, false, true) ];
    };

    TSharedRef<SWidget> Nationwide = SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            Card(SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 14.f, 0.f)[ Badge(TEXT("miras"), TEXT("MM"), Color(ERole::Accent), 40.f) ]
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("\u00dcLKEDEK\u0130 PERAKENDEC\u0130LER \u00b7 YILLIK C\u0130RO")) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)[ Label([OurLine] { return OurLine(false); }, 16, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
                    [ Label([G] { return G() ? MarketChains::NemesisLine(G()->State) : FString(); }, 10, ERole::Warn, false, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
                    [ More([] { return FString(TEXT("Zincirler her ay karar verir: bo\u015f ve k\u00e2rl\u0131 illere a\u00e7\u0131l\u0131r, kanayan ma\u011fazay\u0131 kapat\u0131r, h\u0131zl\u0131 b\u00fcy\u00fcd\u00fc\u011f\u00fcn ilde fiyat sava\u015f\u0131 a\u00e7abilir. Paras\u0131 biten zincir sat\u0131l\u0131\u011fa \u00e7\u0131kar: \u00f6nce sen alabilirsin, sonra b\u00fcy\u00fckler al\u0131r.")); }) ]
                ])
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [ Card(SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Section(TEXT("ZINC\u0130RLER VE B\u0130Z")) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[ RankNote(false) ]
            + SVerticalBox::Slot().AutoHeight()[ TableRows(false, 30) ]) ]
    ];

    TSharedRef<SWidget> World = SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [ Card(SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("D\u00dcNYA PERAKENDE L\u0130G\u0130")) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)[ Label([OurLine] { return OurLine(true); }, 16, ERole::Text, true) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
            [ Label([G]
            {
                if (!G()) return FString();
                TArray<FString> Parts;
                for (const MarketCountry::FProfile& Pack : MarketCountry::All())
                    if (Pack.Id != G()->State.CountryId) Parts.Add(FString::Printf(TEXT("%s %d"), *Pack.Name, MarketCompany::CountryStores(G()->State, Pack.Id)));
                return FString::Printf(TEXT("Yurt d\u0131\u015f\u0131ndaki ma\u011fazalar\u0131m\u0131z: %s. Yurt d\u0131\u015f\u0131 ana \u00fclkede %d ma\u011faza ve %d ille a\u00e7\u0131l\u0131r; yeni \u00fclkede ilk %d g\u00fcn marj d\u00fc\u015f\u00fck kal\u0131r."),
                    *FString::Join(Parts, TEXT(", ")), MarketCompany::AbroadHomeShops, MarketManagers::CountryProvinces, MarketCompany::LearningDays);
            }, 10, ERole::Muted, false, true) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
            [ More([] { return FString(TEXT("Lig y\u0131ll\u0131k ciroyla, t\u00fcm \u00fclkeler i\u00e7in ortak birimde s\u0131ralan\u0131r (enflasyondan ar\u0131nd\u0131r\u0131lm\u0131\u015f). D\u00fcnya devleri her y\u0131l b\u00fcy\u00fcr ve zaman zaman oldu\u011fun \u00fclkelere girer.")); }) ]) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [ Card(SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Section(TEXT("SIRALAMA")) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[ RankNote(true) ]
            + SVerticalBox::Slot().AutoHeight()[ TableRows(true, MarketChains::WorldListSize) ]) ]
    ];

    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)[ Choice(TEXT("Yerel"), [this] { return RivalScope == 0; }, [this] { RivalScope = 0; }) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)[ Choice(TEXT("Ulusal"), [this] { return RivalScope == 1; }, [this] { RivalScope = 1; }) ]
            + SHorizontalBox::Slot().AutoWidth()[ Choice(TEXT("Uluslararas\u0131"), [this] { return RivalScope == 2; }, [this] { RivalScope = 2; }) ]
        ]
        + SVerticalBox::Slot().FillHeight(1.f)
        [
            SNew(SWidgetSwitcher).WidgetIndex_Lambda([this] { return FMath::Clamp(RivalScope, 0, 2); })
            + SWidgetSwitcher::Slot()[ Local ]
            + SWidgetSwitcher::Slot()[ Nationwide ]
            + SWidgetSwitcher::Slot()[ World ]
        ];
}

// ---------------------------------------------------------------------------------------------------------------
// 6 Personel

TSharedRef<SWidget> SMarketMenu::StaffPage()
{
    // G-060: people are persons (MarketStaff). Rows are fixed slots shown while the roster/pool has that many
    // entries; every button reads the person's id at click time, so a changed roster never hits the wrong person.
    // Taxes and the accountant are on the Finans page.
    auto G = [this] { return Game.Get(); };
    auto PersonAt = [G](int32 Slot) -> const FMarketEmployee* { return G() && G()->State.Staff.IsValidIndex(Slot) ? &G()->State.Staff[Slot] : nullptr; };
    auto CandidateAt = [G](int32 Slot) -> const FMarketEmployee* { return G() && G()->State.Candidates.IsValidIndex(Slot) ? &G()->State.Candidates[Slot] : nullptr; };
    auto RoleAt = [PersonAt](int32 Slot) { const FMarketEmployee* E = PersonAt(Slot); return E ? MarketStaff::RoleOf(*E) : MarketStaff::ERole::Accountant; };

    TSharedRef<SVerticalBox> People = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 10; ++Slot)
    {
        People->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 6.f))
            .Visibility_Lambda([PersonAt, Slot] { return PersonAt(Slot) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
                [ Label([G, PersonAt, Slot] { const FMarketEmployee* E = PersonAt(Slot); return E ? MarketStaff::DescribeEmployee(G()->State, *E) : FString(); }, 11, ERole::Text, false, true) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f)
                [ Button([] { return FString(TEXT("Zam %10")); }, [this, PersonAt, Slot] { if (const FMarketEmployee* E = PersonAt(Slot)) Manage(TEXT("Raise"), E->Id); }, false,
                    [RoleAt, PersonAt, Slot] { return PersonAt(Slot) && RoleAt(Slot) != MarketStaff::ERole::Accountant; }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f)
                [ Button([] { return FString(TEXT("\u0130zin ver")); }, [this, PersonAt, Slot] { if (const FMarketEmployee* E = PersonAt(Slot)) Manage(TEXT("DayOff"), E->Id); }, false,
                    [RoleAt, PersonAt, Slot] { return PersonAt(Slot) && RoleAt(Slot) != MarketStaff::ERole::Accountant; }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f)
                [ Button([] { return FString(TEXT("Uyar")); }, [this, PersonAt, Slot] { if (const FMarketEmployee* E = PersonAt(Slot)) Manage(TEXT("Warn"), E->Id); }, false,
                    [RoleAt, PersonAt, Slot] { return PersonAt(Slot) && RoleAt(Slot) == MarketStaff::ERole::Cashier; }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f)
                [ RiskyButton([RoleAt, Slot] { return FString(RoleAt(Slot) == MarketStaff::ERole::Accountant ? TEXT("S\u00f6zle\u015fmeyi bitir") : TEXT("Kov")); },
                    [G, PersonAt, RoleAt, Slot]
                    {
                        const FMarketEmployee* E = PersonAt(Slot);
                        if (!E || !G()) return FString();
                        if (RoleAt(Slot) == MarketStaff::ERole::Accountant)
                            return FString::Printf(TEXT("%s ile s\u00f6zle\u015fme bitsin mi? Vergiyi bundan sonra sen \u00f6dersin; defter tutulmazsa inceleme gelebilir."), *E->Name);
                        // C3 (B3): notice and seniority pay as they will be paid.
                        return FString::Printf(TEXT("%s i\u015ften \u00e7\u0131kar\u0131ls\u0131n m\u0131? \u0130hbar ve k\u0131dem: %s. Ekibin morali biraz d\u00fc\u015fer."),
                            *E->Name, *MarketMenuUi::Tl(MarketStaff::SeverancePay(G()->State, *E)));
                    },
                    [this, PersonAt, Slot] { if (const FMarketEmployee* E = PersonAt(Slot)) Manage(TEXT("Fire"), E->Id); },
                    [PersonAt, Slot] { return PersonAt(Slot) != nullptr; }) ]
            ]
        ];
    }

    TSharedRef<SVerticalBox> Pool = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < MarketStaff::HrPoolSize; ++Slot)
    {
        Pool->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 6.f))
            .Visibility_Lambda([CandidateAt, Slot] { return CandidateAt(Slot) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
                [ Label([G, CandidateAt, Slot] { const FMarketEmployee* C = CandidateAt(Slot); return C ? MarketStaff::DescribeCandidate(G()->State, *C) : FString(); }, 11, ERole::Text, false, true) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f)
                [ Button([] { return FString(TEXT("\u0130\u015fe al")); }, [this, CandidateAt, Slot] { if (const FMarketEmployee* C = CandidateAt(Slot)) Manage(TEXT("HireCandidate"), C->Id); }, true,
                    [CandidateAt, Slot] { return CandidateAt(Slot) != nullptr; }) ]
            ]
        ];
    }

    return SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("EK\u0130P")) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 4.f)
                    [ Label([G] { return G() ? FString::Printf(TEXT("%d ki\u015fi \u00b7 ayda %s"), G()->State.Staff.Num(), *MarketMenuUi::Tl(G()->State.DailyPayroll() * 30)) : FString(); }, 18, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ Label([G] { return G() ? FString::Printf(TEXT("Bug\u00fcn kasada: %s \u00b7 reyonda %d g\u00f6revli"), MarketStaff::CashierOnDuty(G()->State) ? TEXT("kasiyer") : TEXT("sen (E)"), MarketStaff::StockersOnDuty(G()->State)) : FString(); }, 10, ERole::Muted, false, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
                    [ Label([G] { return G() ? G()->WorkerSummary() : FString(); }, 10, ERole::Muted, false, true) ])
            ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("\u0130NSAN KAYNAKLARI")) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 4.f)
                    [ Label([G]
                    {
                        if (!G()) return FString();
                        if (MarketStaff::HasHr(G()->State)) return FString(TEXT("\u0130K m\u00fcd\u00fcr\u00fc \u00e7al\u0131\u015f\u0131yor"));
                        if (!MarketStaff::HrUnlocked(G()->State)) return FString::Printf(TEXT("%d \u00e7al\u0131\u015fandan sonra"), MarketStaff::HrUnlockStaff);
                        // C5 (A menu list): say it only when the list really has one.
                        const bool bHrInPool = G()->State.Candidates.ContainsByPredicate([](const FMarketEmployee& C) { return MarketStaff::RoleOf(C) == MarketStaff::ERole::HrManager; });
                        return bHrInPool ? FString(TEXT("Aday listesinde \u0130K m\u00fcd\u00fcr\u00fc var")) : FString(TEXT("Yeni aday listesini bekle"));
                    }, 18, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ More([] { return FString(TEXT("Her g\u00fcn en mutsuz ki\u015fiyle konu\u015fur, yorgunlara izin ayarlar, ayr\u0131lan\u0131n yerine aday bulur, \u00fccret pazarl\u0131\u011f\u0131 yapar ve adaylar\u0131n ger\u00e7ek de\u011ferlerini g\u00f6sterir.")); }) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f).HAlign(HAlign_Left)
                    [ Button([G] { return FString(G() && G()->State.bHrAutoReplace ? TEXT("Ayr\u0131lan\u0131n yerine al: a\u00e7\u0131k") : TEXT("Ayr\u0131lan\u0131n yerine al: kapal\u0131")); },
                        [this] { Manage(TEXT("HrAutoReplace"), INDEX_NONE); }, false, [G] { return G() && MarketStaff::HasHr(G()->State); }) ])
            ]
            + SHorizontalBox::Slot().FillWidth(1.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("MUHASEBE")) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 4.f)
                    [ LabelBy([G] { return G() && G()->State.Books.TaxDue > 0 ? FString::Printf(TEXT("Vergi %s"), *MarketMenuUi::Tl(G()->State.Books.TaxDue)) : FString(TEXT("Vergi borcu yok")); }, 18,
                        [G] { return G() && G()->State.Books.TaxDue > 0 ? ERole::Warn : ERole::Good; }, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ Label([G] { return G() && MarketStaff::HasAccountant(G()->State) ? FString(TEXT("Mali m\u00fc\u015favir defterleri tutuyor.")) : FString(TEXT("Mali m\u00fc\u015favir yok: vergiyi sen \u00f6dersin.")); }, 10, ERole::Muted, false, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f).HAlign(HAlign_Left)
                    [ Button([] { return FString(TEXT("Vergi ve m\u00fc\u015favir  \u203a")); }, [this] { Go(Finance); }) ])
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            Card(SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Section(TEXT("\u00c7ALI\u015eANLAR")) ]
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(SBox).Visibility_Lambda([G] { return G() && G()->State.Staff.Num() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
                    [ Fixed(TEXT("Kimse yok. Kasay\u0131 ve raflar\u0131 sen yap\u0131yorsun."), 11, ERole::Muted) ]
                ]
                + SVerticalBox::Slot().AutoHeight()[ People ])
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            Card(SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Section(TEXT("\u0130\u015e BA\u015eVURULARI")) ]
                + SVerticalBox::Slot().AutoHeight()
                [ Label([G] { return G() ? FString::Printf(TEXT("\u0130\u015fe alma %s (\u0130K m\u00fcd\u00fcr\u00fc %s). Liste %s yenilenir; son yenileme %d. g\u00fcn."),
                    *MarketMenuUi::Tl(MarketStaff::HireCostOn(MarketStaff::ERole::Cashier, G()->State.Day)), *MarketMenuUi::Tl(MarketStaff::HireCostOn(MarketStaff::ERole::HrManager, G()->State.Day)),
                    MarketStaff::HasHr(G()->State) ? TEXT("3 g\u00fcnde bir") : TEXT("haftada bir"), G()->State.CandidatesDay) : FString(); }, 10, ERole::Muted, false, true) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
                [
                    SNew(SBox).Visibility_Lambda([G] { return G() && G()->State.Candidates.Num() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
                    [ Fixed(TEXT("\u015eu an ba\u015fvuru yok; yeni liste gelince burada g\u00f6r\u00fcn\u00fcr."), 11, ERole::Muted) ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[ Pool ])
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(4.f, 14.f, 0.f, 0.f)
        [ More([] { return FString(TEXT("Moral \u00fccrete, yorgunlu\u011fa ve ilgiye g\u00f6re de\u011fi\u015fir. \u00dc\u00e7 g\u00fcn \u00e7ok mutsuz olan istifa dilek\u00e7esi verir ve iki g\u00fcn sonra ayr\u0131l\u0131r; zam veya izin fikrini de\u011fi\u015ftirebilir.")); }) ]
    ];
}

// ---------------------------------------------------------------------------------------------------------------
// 7 Finans

TSharedRef<SWidget> SMarketMenu::FinancePage()
{
    // G-067 / G-060 / G-071: the till beyond the day, bank loans, taxes, perishables (M36: no credit book), the money
    // trouble ladder and the family's household money.
    auto G = [this] { return Game.Get(); };

    auto LoanButton = [this, G](int32 Step) -> TSharedRef<SWidget>
    {
        return RiskyButton([G, Step] { return G() ? FString::Printf(TEXT("Kredi %s"), *MarketMenuUi::Tl(MarketMenuPagesUi::Today(G()->State, MarketFinance::LoanSteps[Step]))) : FString(); },
            [G, Step]
            {
                if (!G()) return FString();
                return FString::Printf(TEXT("%s: %s kredi al\u0131ns\u0131n m\u0131? %d ay boyunca her 30 g\u00fcnde bir taksit kasadan \u00f6denir; faiz y\u0131l\u0131n oran\u0131n\u0131 izler. Erken kapatma %%1 \u00fccretlidir."),
                    *MarketCast::Bank(0), *MarketMenuUi::Tl(MarketMenuPagesUi::Today(G()->State, MarketFinance::LoanSteps[Step])), MarketFinance::LoanMonths);
            },
            [this, Step] { Manage(TEXT("TakeLoan"), Step); },
            [G, Step] { return G() && MarketMenuPagesUi::Today(G()->State, MarketFinance::LoanSteps[Step]) <= MarketFinance::LoanLimit(G()->State); });
    };
    auto FreshChoice = [this, G](int32 Policy) -> TSharedRef<SWidget>
    {
        return Choice(MarketMenuUi::Title(MarketFreshness::PolicyName(static_cast<MarketFreshness::EPolicy>(Policy))),
            [G, Policy] { return G() && G()->State.FreshPolicy == Policy; }, [this, Policy] { Manage(TEXT("FreshPolicy"), Policy); });
    };

    TSharedRef<SWidget> Top = SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
        [ Stat(TEXT("KASA"), [G] { return G() ? MarketMenuUi::Tl(G()->State.Cash) : FString(); },
               [G] { return !G() ? FString() : G()->State.TroubleStage > 0 ? FString::Printf(TEXT("%d g\u00fcnd\u00fcr eksi"), G()->State.NegativeCashDays) : FString(TEXT("nakit")); },
               [G] { return G() && G()->State.Cash < 0 ? ERole::Bad : ERole::Text; }) ]
        + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
        [ Stat(TEXT("BOR\u00c7LAR"), [G] { return G() ? MarketMenuUi::Tl(MarketFinance::Debt(G()->State) + MarketBanking::Debt(G()->State) + MarketSuppliers::OpenBills(G()->State) + G()->State.Books.TaxDue) : FString(); },
               // C7 (A menu comparison): what this sum covers; the inherited debt has its own card.
               [G] { return G() ? FString::Printf(TEXT("banka %s \u00b7 toptanc\u0131 %s \u00b7 vergi %s%s"), *MarketMenuUi::Tl(MarketFinance::Debt(G()->State) + MarketBanking::Debt(G()->State)), *MarketMenuUi::Tl(MarketSuppliers::OpenBills(G()->State)), *MarketMenuUi::Tl(G()->State.Books.TaxDue),
                   G()->State.InheritedDebt > 0 ? *FString::Printf(TEXT(" \u00b7 devral\u0131nan bor\u00e7 %s hari\u00e7"), *MarketMenuUi::Tl(G()->State.InheritedDebt)) : TEXT("")) : FString(); },
               [G] { return G() && MarketFinance::Debt(G()->State) + MarketBanking::Debt(G()->State) + MarketSuppliers::OpenBills(G()->State) + G()->State.Books.TaxDue > 0 ? ERole::Warn : ERole::Good; }) ]
        // M69: the first store's building is ours: no rent, an asset.
        + SHorizontalBox::Slot().FillWidth(1.f)
        [ Stat(TEXT("B\u0130NA \u00b7 \u0130LK MA\u011eAZA"), [G] { return G() ? MarketMenuUi::Tl(MarketFinance::BuildingValue(G()->State)) : FString(); },
               [] { return FString(TEXT("bizim \u00b7 kira yok")); }) ];

    // M37: our salary and our own money (MarketOwner). The company's till is not ours.
    auto SalaryChip = [this, G](int32 Step) -> TSharedRef<SWidget>
    {
        const int32 X10 = MarketOwner::SalarySteps[Step];
        const FString Chip = MarketMenuSimplifyUi::M37SalaryChip(X10) + (Step == 0 ? FString(TEXT(" asgari")) : FString());
        return Choice(Chip, [G, X10] { return G() && G()->State.Owner.SalaryX10 == X10; }, [this, Step] { Manage(TEXT("OwnerSalary"), Step); },
            [G, X10] { return G() && (G()->State.Day >= G()->State.RescueUntil || X10 <= 10); });
    };
    auto DividendButton = [this, G](int32 Step) -> TSharedRef<SWidget>
    {
        const int32 Share = MarketOwner::DividendPercents[Step];
        return RiskyButton([Share] { return FString::Printf(TEXT("K\u00e2r pay\u0131 %%%d"), Share); },
            [G, Step, Share]
            {
                if (!G()) return FString();
                const int64 Gross = MarketMenuSimplifyUi::M37DividendGross(G()->State, Step);
                const int64 Withheld = FMath::RoundToInt64(static_cast<double>(Gross) * static_cast<double>(MarketOwner::DividendWithholding));
                return FString::Printf(TEXT("Ge\u00e7en y\u0131l\u0131n k\u00e2r\u0131ndan %%%d k\u00e2r pay\u0131 al\u0131ns\u0131n m\u0131? \u015eirketten %s \u00e7\u0131kar, %%%d stopaj (%s) kesilir, servetine %s eklenir. Bu para art\u0131k \u015firketin de\u011fil."),
                    Share, *MarketMenuUi::Tl(Gross), FMath::RoundToInt(MarketOwner::DividendWithholding * 100.f), *MarketMenuUi::Tl(Withheld), *MarketMenuUi::Tl(Gross - Withheld));
            },
            [this, Step] { Manage(TEXT("Dividend"), Step); },
            [G] { return G() && MarketOwner::DividendRoom(G()->State) > 0; });
    };
    auto CapitalButton = [this, G](int32 Step) -> TSharedRef<SWidget>
    {
        const int32 Share = MarketOwner::CapitalPercents[Step];
        return RiskyButton([Share] { return FString::Printf(TEXT("Sermaye koy %%%d"), Share); },
            [G, Step, Share]
            {
                if (!G()) return FString();
                return FString::Printf(TEXT("Servetinin %%%d'i (%s) \u015firkete sermaye olarak konsun mu? Para kasaya girer; geri alman\u0131n yolu ancak k\u00e2r pay\u0131d\u0131r."),
                    Share, *MarketMenuUi::Tl(MarketMenuSimplifyUi::M37CapitalAmount(G()->State, Step)));
            },
            [this, Step] { Manage(TEXT("OwnerCapital"), Step); },
            [G, Step] { return G() && MarketMenuSimplifyUi::M37CapitalAmount(G()->State, Step) > 0; });
    };
    TSharedRef<SWrapBox> SalaryRow = SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(4.f, 4.f));
    for (int32 Step = 0; Step < MarketOwner::StepCount; ++Step) SalaryRow->AddSlot()[ SalaryChip(Step) ];
    TSharedRef<SWrapBox> DividendRow = SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 6.f));
    for (int32 Step = 0; Step < MarketOwner::DividendSteps; ++Step) DividendRow->AddSlot()[ DividendButton(Step) ];
    TSharedRef<SWrapBox> CapitalRow = SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 6.f));
    for (int32 Step = 0; Step < MarketOwner::DividendSteps; ++Step) CapitalRow->AddSlot()[ CapitalButton(Step) ];

    TSharedRef<SWidget> OwnerCard = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Section(TEXT("SEN \u00b7 MAA\u015e VE SERVET")) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ More([] { return FString(TEXT("Maa\u015f\u0131n \u015firketin gideridir: ay ba\u015f\u0131nda kasadan \u00f6denir, gelir vergisi ve sigortan kesilir, kalan\u0131 servetine ge\u00e7er. Kasa yetmezse o ay maa\u015f yok. K\u00e2r pay\u0131 ge\u00e7en y\u0131l\u0131n net k\u00e2r\u0131ndan al\u0131n\u0131r; %15 stopaj kesilir. Kasada \u015firketin bir ayl\u0131k sabit gideri kalmal\u0131; kurtarma plan\u0131nda k\u00e2r pay\u0131 yok. Ailenin ge\u00e7imi her ay ki\u015fisel servetinden \u00e7\u0131kar. Servet senin paran, \u015firketin de\u011fil; istersen sermaye olarak \u015firkete koyabilirsin.")); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 2.f)
        [ LabelBy([G] { return G() ? FString::Printf(TEXT("Servet %s"), *MarketMenuUi::Tl(G()->State.Owner.Wealth)) : FString(); }, 18,
            [G] { return G() && G()->State.Owner.Wealth < 0 ? ERole::Bad : ERole::Text; }, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
        [ Label([G]
        {
            if (!G()) return FString();
            const FMarketState& S = G()->State;
            return FString::Printf(TEXT("Maa\u015f\u0131n ayda %s br\u00fct \u00b7 eline %s ge\u00e7er \u00b7 \u015firkete maliyeti %s\nEvin ge\u00e7imi ayda %s \u00b7 bu y\u0131l maa\u015f %s, k\u00e2r pay\u0131 %s"),
                *MarketMenuUi::Tl(MarketOwner::GrossSalary(S)), *MarketMenuUi::Tl(MarketOwner::NetSalary(S)), *MarketMenuUi::Tl(MarketOwner::CompanyCost(S)),
                *MarketMenuUi::Tl(MarketOwner::Living(S)), *MarketMenuUi::Tl(S.Owner.YearSalary), *MarketMenuUi::Tl(S.Owner.YearDividends));
        }, 10, ERole::Text, false, true) ]
        + SVerticalBox::Slot().AutoHeight()
        [ Why([G] { return G() && G()->State.Owner.MissedSalaries > 0 ? FString::Printf(TEXT("Kasa %d ay maa\u015f\u0131n\u0131 \u00f6deyemedi."), G()->State.Owner.MissedSalaries) : FString(); }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 4.f)[ Fixed(TEXT("Ayl\u0131k maa\u015f (asgari \u00fccretin kat\u0131)"), 10, ERole::Muted) ]
        + SVerticalBox::Slot().AutoHeight()[ SalaryRow ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 4.f)
        [ Label([G] { return G() ? FString::Printf(TEXT("K\u00e2r pay\u0131 \u00b7 ge\u00e7en y\u0131ldan da\u011f\u0131t\u0131labilir %s \u00b7 \u015fimdi en \u00e7ok %s"),
            *MarketMenuUi::Tl(MarketOwner::Distributable(G()->State)), *MarketMenuUi::Tl(MarketOwner::DividendRoom(G()->State))) : FString(); }, 10, ERole::Muted, false, true) ]
        + SVerticalBox::Slot().AutoHeight()[ DividendRow ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([G] { return G() && !MarketMenuSimplifyUi::M37DividendWhy(G()->State).IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Label([G] { return G() ? MarketMenuSimplifyUi::M37DividendWhy(G()->State) : FString(); }, 9, ERole::Muted, false, true) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 4.f)[ Fixed(TEXT("Kendi paranla \u015firkete sermaye koy"), 10, ERole::Muted) ]
        + SVerticalBox::Slot().AutoHeight()[ CapitalRow ]);

    TSharedRef<SWidget> Bank = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("BANKA KRED\u0130S\u0130 \u00b7 A\u0130LE D\u00dcKK\u00c2NI")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 8.f)[ Label([G] { return G() ? MarketFinance::Summary(G()->State) : FString(); }, 11, ERole::Text, false, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
        [
            // C5 (A istek 3): the nearest three installments and a total; every loan in the tooltip.
            SNew(SBox).ToolTip(Tip([G] { return G() && G()->State.Loans.Num() > 0 ? FString(TEXT("\u015eirketin b\u00fct\u00fcn kredileri:")) + MarketMenuSimplifyUi::C5FamilyLoans(G()->State, 0) : FString(TEXT("\u0130lk ma\u011fazan\u0131n kredisi yok.")); }))
            [ Label([G]
            {
                if (!G()) return FString();
                return FString::Printf(TEXT("Bankan\u0131n verebilece\u011fi: %s."), *MarketMenuUi::Tl(FMath::Max<int64>(0, MarketFinance::LoanLimit(G()->State))))
                    + MarketMenuSimplifyUi::C5FamilyLoans(G()->State, 3);
            }, 10, ERole::Muted, false, true) ]
        ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 6.f))
            + SWrapBox::Slot()[ LoanButton(0) ]
            + SWrapBox::Slot()[ LoanButton(1) ]
            + SWrapBox::Slot()[ LoanButton(2) ]
            + SWrapBox::Slot()[ RiskyButton([] { return FString(TEXT("Krediyi kapat")); },
                [G] { return G() ? FString::Printf(TEXT("B\u00fct\u00fcn kredi borcu (%s) \u015fimdi kasadan \u00f6densin mi? Erken kapatma %%1 \u00fccretlidir."), *MarketMenuUi::Tl(MarketFinance::Debt(G()->State))) : FString(); },
                [this] { Manage(TEXT("RepayLoan"), 0); }, [G] { return G() && G()->State.Loans.Num() > 0; }) ]
        ]);

    TSharedRef<SWidget> Tax = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("VERG\u0130 \u00b7 MAL\u0130 M\u00dc\u015eAV\u0130R")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 4.f)
        [ LabelBy([G] { return G() && G()->State.Books.TaxDue > 0 ? FString::Printf(TEXT("%s \u00b7 son g\u00fcn %d"), *MarketMenuUi::Tl(G()->State.Books.TaxDue), G()->State.Books.TaxDueDay)
            : FString(TEXT("\u00d6denecek vergi yok")); }, 18, [G] { return G() && G()->State.Books.TaxDue > 0 ? ERole::Warn : ERole::Good; }, true) ]
        + SVerticalBox::Slot().AutoHeight()
        [ More([G] { return G() && MarketStaff::HasAccountant(G()->State)
            ? FString::Printf(TEXT("Mali m\u00fc\u015favir %s defterleri tutuyor: haftal\u0131k vergiyi zaman\u0131nda \u00f6der, belgeli giderleri d\u00fc\u015fer, kasa farklar\u0131n\u0131 izler."), *MarketCast::Accountant())
            : FString(TEXT("Vergi her 7. g\u00fcn\u00fcn sonunda \u00e7\u0131kar; 3 g\u00fcn i\u00e7inde \u00f6denmezse ceza i\u015fler. Defter tutulmazsa inceleme gelebilir.")); }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
        [ Label([G] { return G() ? FString::Printf(TEXT("\u015eimdiye dek \u00f6denen vergi %s \u00b7 ceza %s \u00b7 inceleme %d"), *MarketMenuUi::Tl(G()->State.Books.TotalTaxPaid), *MarketMenuUi::Tl(G()->State.Books.TotalPenalties), G()->State.Books.Audits) : FString(); }, 10, ERole::Muted, false, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f).HAlign(HAlign_Left)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
            [ Button([] { return FString(TEXT("Vergiyi \u00f6de")); }, [this] { Manage(TEXT("PayTax"), INDEX_NONE); }, true, [G] { return G() && G()->State.Books.TaxDue > 0; }) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ Button([G] { return FString::Printf(TEXT("M\u00fc\u015favirle anla\u015f (ayda %s)"), *MarketMenuUi::Tl(MarketStaff::FairWage(MarketStaff::ERole::Accountant, 80, G() ? G()->State.Day : 1) * 30)); },
                [this] { Manage(TEXT("HireAccountant"), INDEX_NONE); }, false, [G] { return G() && !MarketStaff::HasAccountant(G()->State); }) ]
        ]);

    TSharedRef<SWidget> Fresh = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("TAZE \u00dcR\u00dcN \u00b7 SON G\u00dcN")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 8.f)
        [ Label([G] { return G() ? FString::Printf(TEXT("Son g\u00fcn\u00fcne gelen s\u00fct, yo\u011furt ve benzerleri: %%%d indirimle sat\u0131l\u0131r, ihtiyac\u0131 olana ba\u011f\u0131\u015flan\u0131r ya da bir \u015fey yap\u0131lmaz. D\u00fcn fire: %d adet (%s)."),
            MarketFreshness::MarkdownPercent, G()->State.LastWasteUnits, *MarketMenuUi::Tl(G()->State.LastWasteCost)) : FString(); }, 10, ERole::Text, false, true) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)[ FreshChoice(1) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)[ FreshChoice(2) ]
            + SHorizontalBox::Slot().AutoWidth()[ FreshChoice(0) ]
        ]);

    TSharedRef<SWidget> Trouble = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("NAK\u0130T SIKINTISI")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
        [ LabelBy([G]
        {
            if (!G()) return FString();
            if (G()->State.TroubleStage <= 0) return FString(TEXT("Yok. Kasa art\u0131da."));
            return FString::Printf(TEXT("Kasa %d g\u00fcnd\u00fcr eksi (a\u015fama %d)."), G()->State.NegativeCashDays, G()->State.TroubleStage);
        }, 14, [G] { return G() && G()->State.TroubleStage > 0 ? ERole::Bad : ERole::Good; }, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
        // C7 (A menu comparison): one "Nas\u0131l i\u015fler?" for the steps and the household money.
        [ More([G] { return FString(TEXT("Oyun bitmez, ama her ad\u0131m \u00f6nceden s\u00f6ylenir: 1 g\u00fcn uyar\u0131, 3 g\u00fcn toptanc\u0131 vadeyi kapat\u0131r, 7 g\u00fcn se\u00e7im (acil kredi ya da depoyu yar\u0131 fiyat\u0131na satmak), 14 g\u00fcn depo yar\u0131 fiyat\u0131na gider, 30 g\u00fcn banka uyar\u0131r, 60 g\u00fcn kurtarma plan\u0131 uygulan\u0131r. Kasa art\u0131ya ge\u00e7ince biter."))
            + FString(TEXT("\n\nBina: ilk ma\u011fazan\u0131n binas\u0131 marketle birlikte devrald\u0131\u011f\u0131n senin m\u00fclk\u00fcn; kira \u00f6demez, bilan\u00e7oda varl\u0131k olarak durur. \u015eubeler kirac\u0131d\u0131r.")); }) ]);

    // M65: our companies: the brand everybody says, the parent company and a subsidiary in every country entered.
    // The registered names can be written freely; "Ba\u015fka \u00f6neri" walks through the country's usual forms.
    TSharedRef<SVerticalBox> CompanyRows = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 12; ++Slot)
    {
        auto Sub = [G, Slot]() -> const FMarketSubsidiary* { return G() && G()->State.Company.Subsidiaries.IsValidIndex(Slot) ? &G()->State.Company.Subsidiaries[Slot] : nullptr; };
        CompanyRows->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([Sub] { return Sub() ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
                    [
                        SNew(SBox).WidthOverride(120.f)
                        [ Label([G, Sub] { const FMarketSubsidiary* S = Sub(); if (!S || !G()) return FString();
                            const MarketCountry::FProfile* P = MarketCountry::Find(S->Country);
                            return (P ? P->Name : S->Country) + (S->Country == G()->State.CountryId ? FString(TEXT(" \u00b7 ana \u015firket")) : FString()); }, 11, ERole::Muted) ]
                    ]
                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                    [
                        SNew(SEditableTextBox).Font(MarketMenuUi::MenuFont(false, 11))
                        .Text_Lambda([Sub] { const FMarketSubsidiary* S = Sub(); return FText::FromString(S ? S->LegalName : FString()); })
                        .OnTextCommitted_Lambda([G, Slot](const FText& Text, ETextCommit::Type)
                        {
                            if (!G() || !G()->State.Company.Subsidiaries.IsValidIndex(Slot)) return;
                            FString Message;
                            if (MarketSubsidiaries::SetLegalName(G()->State, G()->State.Company.Subsidiaries[Slot].Country, Text.ToString(), Message)) G()->Notify(Message);
                        })
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                    [ Button([] { return FString(TEXT("Ba\u015fka \u00f6neri")); }, [G, Slot]
                    {
                        if (!G() || !G()->State.Company.Subsidiaries.IsValidIndex(Slot)) return;
                        const FString Country = G()->State.Company.Subsidiaries[Slot].Country;
                        FString Message;
                        if (MarketSubsidiaries::SetLegalName(G()->State, Country, MarketSubsidiaries::NextSuggestion(G()->State, Country), Message)) G()->Notify(Message);
                    }) ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(128.f, 2.f, 0.f, 0.f)
                [ Label([G, Sub]
                {
                    const FMarketSubsidiary* S = Sub();
                    if (!S || !G()) return FString();
                    if (S->Country == G()->State.CountryId) return FString::Printf(TEXT("Yurt d\u0131\u015f\u0131ndaki \u015firketlerin k\u00e2r\u0131 buraya gelir. Toplam gelen %s."),
                        *MarketMenuUi::Tl([&]{ int64 Sum = 0; for (const FMarketSubsidiary& X : G()->State.Company.Subsidiaries) Sum += X.TotalTransferred; return Sum; }()));
                    return FString::Printf(TEXT("%d. g\u00fcn kuruldu \u00b7 ge\u00e7en ay %s \u00b7 ana \u015firkete %s \u00b7 stopaj %s"), S->FoundedDay,
                        *MarketMenuUi::Tl(S->LastMonthProfit), *MarketMenuUi::Tl(S->LastTransfer), *MarketMenuUi::Tl(S->LastWithheld));
                }, 10, ERole::Muted) ]
            ]
        ];
    }
    TSharedRef<SWidget> Companies = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("\u015e\u0130RKETLER")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
            [ SNew(SBox).WidthOverride(120.f)[ Fixed(TEXT("Marka"), 11, ERole::Muted) ] ]
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [
                SNew(SEditableTextBox).Font(MarketMenuUi::MenuFont(true, 12))
                .Text_Lambda([G] { return FText::FromString(G() ? MarketSubsidiaries::Brand(G()->State) : FString()); })
                .OnTextCommitted_Lambda([G](const FText& Text, ETextCommit::Type)
                {
                    FString Message;
                    if (G() && MarketSubsidiaries::SetBrand(G()->State, Text.ToString(), Message)) G()->Notify(Message);
                })
            ]
        ]
        + SVerticalBox::Slot().AutoHeight()[ CompanyRows ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [ More([] { return FString(TEXT("Herkes markay\u0131 s\u00f6yler: tabelada, haberde, d\u00fcnya s\u0131ralamas\u0131nda. Tescilli ad, bir \u00fclkenin kendi perakende listesinde ve defterde g\u00f6r\u00fcn\u00fcr. Yeni bir \u00fclkeye ilk ma\u011faza a\u00e7\u0131l\u0131nca orada bir \u015firket kurulur (kurulu\u015f masraf\u0131 a\u00e7\u0131l\u0131\u015f maliyetine eklenir). Her ay ba\u015f\u0131nda o \u015firketin k\u00e2r\u0131 ana \u015firkete aktar\u0131l\u0131r; o \u00fclke bir stopaj vergisi al\u0131r.")); }) ]);

    return SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Top ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[ OwnerCard ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[ Companies ]
        // C5 (A istek 3): the company's rating, banks and limit first; the first store's loan after it.
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[ BankingCard() ]
        // M36: no credit book: the first store's loan sits next to the tax.
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)[ Bank ]
            + SHorizontalBox::Slot().FillWidth(1.f)[ Tax ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[ Fresh ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[ Trouble ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[ LedgerCards() ]
    ];
}

TSharedRef<SWidget> SMarketMenu::BankingCard()
{
    // M28: the company's banks. The rating and why, one row per bank (what it lends now, its rate), the loan's
    // shape (share of the offer, months, grace), the running loans and the credit line.
    auto G = [this] { return Game.Get(); };
    // C13 (M43): the loan is an application; the fifth step asks for half again more than the bank offers today.
    static const TCHAR* StepNames[5] = { TEXT("%25"), TEXT("%50"), TEXT("%75"), TEXT("Tam\u0131"), TEXT("Fazlas\u0131n\u0131 iste") };
    TSharedRef<SHorizontalBox> Shape = SNew(SHorizontalBox);
    for (int32 S = 0; S < 5; ++S)
        Shape->AddSlot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)[ Choice(StepNames[S], [this, S] { return LoanStep == S; }, [this, S] { LoanStep = S; }) ];
    Shape->AddSlot().AutoWidth().Padding(8.f, 0.f, 4.f, 0.f)[ SNew(SBox).WidthOverride(1.f).HeightOverride(20.f)[ SNew(SBorder).BorderImage(&FlatBrush).BorderBackgroundColor(Col(ERole::Line)) ] ];
    for (int32 T = 0; T < MarketBanking::TenorCount; ++T)
        Shape->AddSlot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)[ Choice(FString::Printf(TEXT("%d ay"), MarketBanking::Tenors[T]), [this, T] { return LoanTenor == T; }, [this, T] { LoanTenor = T; }) ];
    Shape->AddSlot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)[ Choice(TEXT("6 ay yaln\u0131z faiz"), [this] { return bLoanGrace; }, [this] { bLoanGrace = !bLoanGrace; }) ];

    TSharedRef<SVerticalBox> Banks = SNew(SVerticalBox);
    for (int32 B = 0; B <= MarketBanking::BondBank; ++B)
    {
        auto Line = [G, B]
        {
            if (!G()) return FString();
            const MarketBanking::FBank Lender = MarketBanking::Bank(G()->State, B);
            FString Why;
            if (!MarketBanking::CanBorrow(G()->State, B, Why)) return FString::Printf(TEXT("%s \u00b7 %s"), *Lender.Name, *Why);
            return FString::Printf(TEXT("%s \u00b7 en \u00e7ok %s \u00b7 y\u0131ll\u0131k %%%.1f \u00b7 en uzun %d ay%s"), *Lender.Name, *MarketMenuUi::Tl(MarketBanking::Offer(G()->State, B)),
                MarketBanking::YearRate(G()->State, B) * 100.0, B == MarketBanking::BondBank ? 60 : Lender.MaxTenor, Lender.bInvestmentOnly ? TEXT(" \u00b7 y\u0131lda bir") : B == MarketBanking::BondBank ? TEXT(" \u00b7 ana para sonda") : TEXT(""));
        };
        auto Amount = [this, G, B] { return G() ? MarketBanking::AskedFor(G()->State, B, LoanStep) : 0; };
        Banks->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 6.f))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Label(Line, 10, ERole::Text, false, true) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                [ RiskyButton([] { return FString(TEXT("Ba\u015fvur")); },
                    [this, G, B, Amount]
                    {
                        if (!G()) return FString();
                        const int32 Months = B == MarketBanking::BondBank ? 60 : FMath::Min(MarketBanking::Tenors[LoanTenor], MarketBanking::Bank(G()->State, B).MaxTenor);
                        return FString::Printf(TEXT("%s'a %s i\u00e7in ba\u015fvurulsun mu (%d ay%s)? Kredi komitesi birka\u00e7 g\u00fcnde cevap verir: tam\u0131n\u0131, bir k\u0131sm\u0131n\u0131 ya da ret. Bug\u00fcnk\u00fc faiz y\u0131ll\u0131k %%%.1f; cevap g\u00fcn\u00fc biraz farkl\u0131 olabilir. Teklif bir hafta ge\u00e7erli."),
                            *MarketBanking::Bank(G()->State, B).Name, *MarketMenuUi::Tl(Amount()), Months,
                            bLoanGrace && B != MarketBanking::BondBank ? TEXT(", ilk 6 ay yaln\u0131z faiz") : TEXT(""), MarketBanking::YearRate(G()->State, B) * 100.0);
                    },
                    [this, B] { Manage(TEXT("LoanApply"), MarketBanking::EncodeLoan(B, LoanStep, LoanTenor, bLoanGrace)); },
                    [G, B] { FString Why; return G() && MarketBanking::CanApply(G()->State, B, Why); }) ]
            ]
        ];
    }

    // C13 (M43): applications waiting, offers to accept, refusals of the week.
    static constexpr int32 AppRows = 6;
    TSharedRef<SVerticalBox> Apps = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < AppRows; ++Slot)
    {
        auto AppAt = [G, Slot]() -> int32 { if (!G()) return INDEX_NONE; const TArray<int32> List = MarketBanking::OpenApps(G()->State); return List.IsValidIndex(Slot) ? List[Slot] : INDEX_NONE; };
        auto IsOffer = [G, AppAt] { const int32 I = AppAt(); return I != INDEX_NONE && G()->State.Banking.Apps[I].Status == static_cast<uint8>(MarketBanking::EAppStatus::Offered); };
        auto IsOpen = [G, AppAt] { const int32 I = AppAt(); return I != INDEX_NONE && G()->State.Banking.Apps[I].Status <= static_cast<uint8>(MarketBanking::EAppStatus::Offered); };
        Apps->AddSlot().AutoHeight().Padding(0.f, 2.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).Padding(FMargin(12.f, 5.f))
            .BorderBackgroundColor(ColBy([IsOffer] { return IsOffer() ? ERole::AccentSoft : ERole::Inset; }))
            .Visibility_Lambda([AppAt] { return AppAt() != INDEX_NONE ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Label([G, AppAt] { const int32 I = AppAt(); return I != INDEX_NONE ? MarketBanking::DescribeApp(G()->State, I) : FString(); }, 10, ERole::Text, false, true) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                [ SNew(SBox).Visibility_Lambda([IsOffer] { return IsOffer() ? EVisibility::Visible : EVisibility::Collapsed; })
                    [ RiskyButton([] { return FString(TEXT("Kabul et")); },
                        [G, AppAt] { const int32 I = AppAt(); if (I == INDEX_NONE) return FString(); const FMarketLoanApp& A = G()->State.Banking.Apps[I];
                            return FString::Printf(TEXT("%s'\u0131n teklifi kabul edilsin mi? %s, y\u0131ll\u0131k %%%.1f, %d ay. Taksit her ay kasadan \u00f6denir; \u00f6denmezse not d\u00fc\u015fer.%s"),
                                *MarketBanking::Bank(G()->State, A.Bank).Name, *MarketMenuUi::Tl(A.Offered), A.YearRate * 100.f, A.Months,
                                A.Purpose == static_cast<uint8>(MarketBanking::EPurpose::Acquisition) ? TEXT(" Para gelince sat\u0131n alma tamamlan\u0131r.") : TEXT("")); },
                        [this, G, AppAt] { const int32 I = AppAt(); if (I != INDEX_NONE) Manage(TEXT("LoanAccept"), G()->State.Banking.Apps[I].Id); }) ] ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f, 0.f, 0.f)
                [ SNew(SBox).Visibility_Lambda([IsOpen] { return IsOpen() ? EVisibility::Visible : EVisibility::Collapsed; })
                    [ Button([IsOffer] { return FString(IsOffer() ? TEXT("Reddet") : TEXT("Geri \u00e7ek")); },
                        [this, G, AppAt] { const int32 I = AppAt(); if (I != INDEX_NONE) Manage(TEXT("LoanDecline"), G()->State.Banking.Apps[I].Id); }) ] ]
            ]
        ];
    }

    static constexpr int32 LoanRows = 6;
    TSharedRef<SVerticalBox> Loans = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < LoanRows; ++Slot)
    {
        Loans->AddSlot().AutoHeight().Padding(0.f, 2.f)
        [
            SNew(SHorizontalBox).Visibility_Lambda([G, Slot] { return G() && G()->State.Banking.Loans.IsValidIndex(Slot) ? EVisibility::Visible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Label([G, Slot] { return G() ? MarketBanking::Describe(G()->State, Slot) : FString(); }, 10, ERole::Text, false, true) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
            [ RiskyButton([] { return FString(TEXT("Kapat")); },
                [G, Slot] { return G() && G()->State.Banking.Loans.IsValidIndex(Slot) ? FString::Printf(TEXT("Kalan %s \u015fimdi \u00f6densin mi? Erken kapama %%1."), *MarketMenuUi::Tl(MarketBanking::BalanceHome(G()->State, G()->State.Banking.Loans[Slot]))) : FString(); },
                [this, Slot] { Manage(TEXT("RepayCorpLoan"), Slot); },
                [G, Slot] { return G() && G()->State.Banking.Loans.IsValidIndex(Slot) && G()->State.Cash >= MarketBanking::BalanceHome(G()->State, G()->State.Banking.Loans[Slot]) * 101 / 100; }) ]
        ];
    }

    // E4b: the banks of the countries where we have a company lend in their own money (the commercial bank, the
    // whole offer, 36 months; asks first). The payments follow the exchange rate.
    TSharedRef<SVerticalBox> Abroad = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 8; ++Slot)
    {
        auto Country = [G, Slot]() -> FString
        {
            if (!G()) return FString();
            int32 Seen = 0;
            for (const FMarketSubsidiary& S : G()->State.Company.Subsidiaries)
                if (S.Country != G()->State.CountryId && Seen++ == Slot) return S.Country;
            return FString();
        };
        Abroad->AddSlot().AutoHeight().Padding(0.f, 2.f)
        [
            SNew(SHorizontalBox).Visibility_Lambda([Country] { return Country().IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible; })
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [ Label([G, Country]
            {
                const FString C = Country();
                if (!G() || C.IsEmpty()) return FString();
                FString Why;
                if (!MarketBanking::CanBorrowIn(G()->State, C, 1, Why)) return FString::Printf(TEXT("%s: %s"), *MarketCountry::FindOrDefault(C).Name, *Why);
                return FString::Printf(TEXT("%s \u00b7 %s: %s'e kadar, y\u0131ll\u0131k %%%.1f (o \u00fclkenin faizi), \u00f6deme o paran\u0131n kuruyla"), *MarketCountry::FindOrDefault(C).Name,
                    *MarketBanking::BankNameIn(G()->State, C, 1), *MarketMenuUi::Tl(MarketBanking::OfferIn(G()->State, C, 1)), MarketBanking::YearRateIn(G()->State, C, 1) * 100.0);
            }, 10, ERole::Text, false, true) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
            [ RiskyButton([] { return FString(TEXT("Kredi al")); },
                [G, Country] { const FString C = Country(); return G() && !C.IsEmpty() ? FString::Printf(TEXT("%s: %s kredi al\u0131ns\u0131n m\u0131? 36 ay, taksitler o \u00fclkenin paras\u0131yla; kur d\u00fc\u015ferse taksit hafifler, y\u00fckselirse a\u011f\u0131rla\u015f\u0131r."),
                    *MarketBanking::BankNameIn(G()->State, C, 1), *MarketMenuUi::Tl(MarketBanking::OfferIn(G()->State, C, 1))) : FString(); },
                [G, Country] { FString Message; if (G() && MarketBanking::BorrowIn(G()->State, Country(), 1, 3, 1, false, Message)) G()->Notify(Message); else if (G()) G()->Notify(Message); },
                [G, Country] { return G() && !Country().IsEmpty() && MarketBanking::OfferIn(G()->State, Country(), 1) > 0; }) ]
        ];
    }

    return Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Section(TEXT("\u015e\u0130RKET F\u0130NANSI")) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
            [ Mono([G] { return G() ? FString::Printf(TEXT("not %s"), *MarketBanking::RatingName(MarketBanking::Rating(G()->State))) : FString(); }, 14.f,
                [G] { const int32 R = G() ? static_cast<int32>(MarketBanking::Rating(G()->State)) : 0; return R >= 4 ? ERole::Good : R >= 2 ? ERole::Accent : ERole::Bad; }) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ More([G] { return G() ? FString(TEXT("Kredi notu her ay \u015funlardan:\n")) + FString::Join(MarketBanking::RatingLines(G()->State), TEXT("\n"))
                + TEXT("\n\nBor\u00e7 FAV\u00d6K'\u00fcn 3,5 kat\u0131n\u0131 ge\u00e7erse bor\u00e7 s\u0131n\u0131r\u0131 a\u015f\u0131l\u0131r: faiz 2 puan artar, yeni kredi kesilir; \u00fc\u00e7 ay s\u00fcrerse en b\u00fcy\u00fck banka borcun d\u00f6rtte birini geri ister. Enflasyon eski borcu sessizce k\u00fc\u00e7\u00fclt\u00fcr.") : FString(); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
        [ LabelBy([G] { return G() ? MarketBanking::Summary(G()->State) : FString(); }, 11, [G] { return G() && MarketBanking::CovenantBroken(G()->State) ? ERole::Bad : ERole::Text; }) ]
        // C5 (A menu list): while no bank lends for the same reason (day one: no branch yet), one lock line instead
        // of the loan shape and five rows repeating it.
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        // C7 (A menu comparison): not twice when the summary above already says it (day one).
        [ Why([G] { if (!G()) return FString(); const FString Lock = MarketMenuSimplifyUi::C5CommonBankLock(G()->State); return Lock == MarketBanking::Summary(G()->State) ? FString() : Lock; }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 4.f)
        [ SNew(SBox).Visibility_Lambda([G] { return G() && MarketMenuSimplifyUi::C5CommonBankLock(G()->State).IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed; })[ Shape ] ]
        + SVerticalBox::Slot().AutoHeight()
        [ SNew(SBox).Visibility_Lambda([G] { return G() && MarketMenuSimplifyUi::C5CommonBankLock(G()->State).IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed; })[ Banks ] ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[ Abroad ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[ Apps ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[ Loans ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            .Visibility_Lambda([G] { return G() && (MarketBanking::IsOpen(G()->State) || G()->State.Banking.bLine) ? EVisibility::Visible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [ Label([G] { return !G() ? FString() : G()->State.Banking.bLine ? FString::Printf(TEXT("Kredi limiti %s \u00b7 kullan\u0131lan %s"), *MarketMenuUi::Tl(G()->State.Banking.LineLimit), *MarketMenuUi::Tl(G()->State.Banking.LineDrawn))
                : FString::Printf(TEXT("Kredi limiti kapal\u0131 (a\u00e7\u0131labilir: %s)"), *MarketMenuUi::Tl(MarketBanking::LineLimitFor(G()->State))); }, 10, ERole::Text) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f, 0.f, 0.f)
            [ Button([] { return FString(TEXT("Limit a\u00e7")); }, [this] { Manage(TEXT("OpenLine"), 0); }, false, [G] { return G() && !G()->State.Banking.bLine && MarketBanking::LineLimitFor(G()->State) > 0; }) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f, 0.f, 0.f)
            [ Choice(TEXT("Eksi kasay\u0131 kapats\u0131n"), [G] { return G() && G()->State.Banking.bLineAuto; }, [this, G] { if (G()) Manage(TEXT("LineAuto"), G()->State.Banking.bLineAuto ? 0 : 1); }, [G] { return G() && G()->State.Banking.bLine; }) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f, 0.f, 0.f)
            [ Button([] { return FString(TEXT("Limiti \u00f6de")); }, [this] { Manage(TEXT("RepayLine"), 0); }, false, [G] { return G() && G()->State.Banking.LineDrawn > 0; }) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f, 0.f, 0.f)
            [ RiskyButton([] { return FString(TEXT("Yap\u0131land\u0131r")); },
                [G] { return FString::Printf(TEXT("B\u00fct\u00fcn krediler tek uzun krediye toplans\u0131n m\u0131 (%s)? Taksit d\u00fc\u015fer; %%2 masraf borca eklenir, not alt\u0131 ay bir basamak a\u015fa\u011f\u0131da kal\u0131r."), G() ? *MarketBanking::Bank(G()->State, 1).Name : TEXT("ticari banka")); },
                [this] { Manage(TEXT("Restructure"), 1); }, [G] { return G() && G()->State.Banking.Loans.Num() > 0; }) ]
        ]);
}

TSharedRef<SWidget> SMarketMenu::LedgerCards()
{
    // C3 (B2, B4): the company's books. Income statement for a period, the balance sheet, the audit line and the
    // era the economy is in.
    auto G = [this] { return Game.Get(); };
    auto Period = [this, G]() -> MarketLedger::FStatement
    {
        if (!G()) return MarketLedger::FStatement();
        const FMarketState& S = G()->State;
        const int32 Closed = FMath::Max(1, S.Day - 1);
        const MarketCalendar::FDate Date = MarketCalendar::DateOf(Closed);
        switch (LedgerPeriod)
        {
        case 0: return MarketLedger::DayStatement(S, Closed);
        case 2: return MarketLedger::MonthStatement(S, Date.Year, Date.Month);
        case 3: return MarketLedger::YearStatement(S, Date.Year);
        default: return MarketLedger::WeekStatement(S, (Closed - 1) / 7 + 1);
        }
    };
    static const TCHAR* PeriodNames[4] = { TEXT("D\u00fcn"), TEXT("Bu hafta"), TEXT("Bu ay"), TEXT("Bu y\u0131l") };
    TSharedRef<SHorizontalBox> Periods = SNew(SHorizontalBox);
    for (int32 P = 0; P < 4; ++P)
        Periods->AddSlot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)[ Choice(PeriodNames[P], [this, P] { return LedgerPeriod == P; }, [this, P] { LedgerPeriod = P; }) ];
    auto Line = [Period](int32 Which)
    {
        const MarketLedger::FStatement S = Period();
        const int64 Value = Which == 0 ? S.Revenue : Which == 1 ? S.GrossProfit : Which == 2 ? S.Expenses : S.NetProfit;
        return MarketMenuUi::TlShort(Value);
    };
    // The full sum under a shortened one (C3, A istek 1).
    auto Full = [Period](int32 Which)
    {
        const MarketLedger::FStatement S = Period();
        const int64 Value = Which == 0 ? S.Revenue : Which == 1 ? S.GrossProfit : Which == 2 ? S.Expenses : S.NetProfit;
        return MarketMenuUi::TlShort(Value) == MarketMenuUi::Tl(Value) ? FString() : MarketMenuUi::Tl(Value);
    };
    TSharedRef<SWidget> Income = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Section(TEXT("GEL\u0130R TABLOSU")) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Periods ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [
            // C5 (A istek 2): two by two, so big amounts (and their full sum under them) fit in half the page.
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 8.f, 0.f)[ Stat(TEXT("Ciro"), [Line] { return Line(0); }, [Full] { return Full(0); }, [] { return ERole::Text; }) ]
                + SHorizontalBox::Slot().FillWidth(1.f)[ Stat(TEXT("Br\u00fct k\u00e2r"), [Line] { return Line(1); }, [Full] { return Full(1); }, [] { return ERole::Text; }) ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 8.f, 0.f)[ Stat(TEXT("Giderler"), [Line] { return Line(2); }, [Full] { return Full(2); }, [] { return ERole::Muted; }) ]
                + SHorizontalBox::Slot().FillWidth(1.f)[ Stat(TEXT("Net"), [Line] { return Line(3); }, [Full] { return Full(3); }, [Period] { return Period().NetProfit >= 0 ? ERole::Good : ERole::Bad; }) ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
        [ More([Period]
        {
            const MarketLedger::FStatement S = Period();
            TArray<FString> Lines;
            for (int32 A = 0; A < static_cast<int32>(MarketLedger::EAccount::Count); ++A)
            {
                const MarketLedger::EAccount Account = static_cast<MarketLedger::EAccount>(A);
                if (!MarketLedger::IsIncomeStatement(Account) || S.At(Account) == 0) continue;
                Lines.Add(FString::Printf(TEXT("%s: %s"), *MarketLedger::AccountName(Account), *MarketMenuUi::Tl(S.At(Account))));
            }
            return MarketLedger::StatementText(S) + (Lines.Num() > 0 ? TEXT("\n\n") + FString::Join(Lines, TEXT("\n")) : FString());
        }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
        [ LabelBy([G] { return G() ? MarketLedger::AuditText(G()->State) : FString(); }, 10, [G] { return G() && MarketLedger::AuditOk(G()->State) ? ERole::Good : ERole::Warn; }) ]);

    auto Bal = [G]() -> MarketLedger::FBalance { return G() ? MarketLedger::Balance(G()->State, G()->Products) : MarketLedger::FBalance(); };
    TSharedRef<SWidget> Sheet = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("B\u0130LAN\u00c7O")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f)[ Stat(TEXT("Varl\u0131klar"), [Bal] { return MarketMenuUi::TlShort(Bal().Assets()); }, [Bal] { const int64 V = Bal().Assets(); return MarketMenuUi::TlShort(V) == MarketMenuUi::Tl(V) ? FString() : MarketMenuUi::Tl(V); }, [] { return ERole::Text; }) ]
            + SHorizontalBox::Slot().FillWidth(1.f)[ Stat(TEXT("Bor\u00e7lar (miras dahil)"), [Bal] { return MarketMenuUi::TlShort(Bal().Liabilities()); }, [Bal] { const int64 V = Bal().Liabilities(); return MarketMenuUi::TlShort(V) == MarketMenuUi::Tl(V) ? FString() : MarketMenuUi::Tl(V); }, [] { return ERole::Muted; }) ]
            + SHorizontalBox::Slot().FillWidth(1.f)[ Stat(TEXT("\u00d6zkaynak"), [Bal] { return MarketMenuUi::TlShort(Bal().Equity()); }, [Bal] { const int64 V = Bal().Equity(); return MarketMenuUi::TlShort(V) == MarketMenuUi::Tl(V) ? FString() : MarketMenuUi::Tl(V); }, [Bal] { return Bal().Equity() >= 0 ? ERole::Good : ERole::Bad; }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
        [ More([Bal]
        {
            const MarketLedger::FBalance B = Bal();
            return FString::Printf(TEXT("Kasa %s \u00b7 d\u00fckk\u00e2n sto\u011fu %s \u00b7 \u015fube sto\u011fu %s \u00b7 reyon sto\u011fu %s \u00b7 kart alaca\u011f\u0131 %s \u00b7 depozitolar %s \u00b7 bina %s\nToptanc\u0131 %s \u00b7 banka %s \u00b7 vergi %s \u00b7 devral\u0131nan bor\u00e7 %s"),
                *MarketMenuUi::Tl(B.Cash), *MarketMenuUi::Tl(B.Stock), *MarketMenuUi::Tl(B.BranchStock), *MarketMenuUi::Tl(B.DepartmentStock), *MarketMenuUi::Tl(B.CardReceivable),
                *MarketMenuUi::Tl(B.Deposits), *MarketMenuUi::Tl(B.Building), *MarketMenuUi::Tl(B.Payables), *MarketMenuUi::Tl(B.Loans), *MarketMenuUi::Tl(B.TaxDue), *MarketMenuUi::Tl(B.InheritedDebt));
        }) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [ LabelBy([G] { const FString Era = G() ? MarketEras::Summary(G()->State) : FString(); return Era.IsEmpty() ? FString(TEXT("Ekonomi sakin.")) : Era; }, 10,
            [G] { return G() && !MarketEras::Summary(G()->State).IsEmpty() ? ERole::Warn : ERole::Muted; }) ]
        + SVerticalBox::Slot().AutoHeight()
        [ More([] { return FString(TEXT("Ekonomide d\u00f6nemler s\u0131rayla gelir: kur \u015foku, durgunluk, salg\u0131n, y\u00fcksek enflasyon, toparlanma. Zamanlar\u0131 her oyunda farkl\u0131d\u0131r.")); }) ]);

    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)[ Income ]
        + SHorizontalBox::Slot().FillWidth(1.f)[ Sheet ];
}

// ---------------------------------------------------------------------------------------------------------------
// 8 Satis kanallari

TSharedRef<SWidget> SMarketMenu::ChannelsPage()
{
    // M32: online selling of the company (channels, provinces, policy, numbers) and the payment methods. Nothing of
    // online selling shows before its first rumour.
    using MarketOnline::EChannel;
    auto G = [this] { return Game.Get(); };
    auto Shown = [G] { return G() && MarketOnline::Visible(G()->State) ? EVisibility::Visible : EVisibility::Collapsed; };
    auto ChannelRow = [this, G](EChannel Channel, const FString& Note) -> TSharedRef<SWidget>
    {
        const int32 Code = static_cast<int32>(Channel);
        auto IsOn = [G, Channel] { return G() && MarketOnline::IsOn(G()->State, Channel); };
        auto Seen = [G, Channel] { return G() && (MarketOnline::IsOn(G()->State, Channel) || G()->State.Day >= MarketOnline::OpenDay(G()->State, Channel) - MarketOnline::RumourDays); };
        auto Ready = [G, Channel, IsOn] { FString Reason; return IsOn() || (G() && MarketOnline::CanOpen(G()->State, Channel, Reason)); };
        return SNew(SBox).Visibility_Lambda([Seen] { return Seen() ? EVisibility::Visible : EVisibility::Collapsed; })
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 8.f))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [ LabelBy([Channel, IsOn] { return FString::Printf(TEXT("%s \u00b7 %s"), *MarketOnline::ChannelName(Channel), IsOn() ? TEXT("a\u00e7\u0131k") : TEXT("kapal\u0131")); }, 12,
                        [IsOn] { return IsOn() ? ERole::Accent : ERole::Text; }, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ Label([Note] { return Note; }, 10, ERole::Muted, false, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ Label([G, Channel] { return G() ? MarketOnline::ChannelLine(G()->State, Channel) : FString(); }, 10, ERole::Text, false, true) ]
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
                [
                    RiskyButton([IsOn] { return FString(IsOn() ? TEXT("Kapat") : TEXT("A\u00e7")); },
                        [G, Channel, IsOn]
                        {
                            if (!G()) return FString();
                            if (IsOn()) return FString::Printf(TEXT("%s kapans\u0131n m\u0131? Bu kanal\u0131n m\u00fc\u015fterileri ba\u015fka yere al\u0131\u015f\u0131r."), *MarketOnline::ChannelName(Channel));
                            return FString::Printf(TEXT("%s a\u00e7\u0131ls\u0131n m\u0131? %s"), *MarketOnline::ChannelName(Channel), *MarketOnline::ChannelLine(G()->State, Channel));
                        },
                        [this, Code, IsOn] { Manage(IsOn() ? TEXT("OnlineClose") : TEXT("OnlineOpen"), Code); },
                        Ready)
                ]
            ]
        ];
    };

    TSharedRef<SWidget> ChannelCard = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("\u0130NTERNETTEN SATI\u015e")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 8.f)[ Label([G] { return G() ? MarketOnline::Summary(G()->State) : FString(); }, 11, ERole::Text, false, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)[ ChannelRow(EChannel::Web, TEXT("Kendi sitemiz: b\u00fcy\u00fck sepetler, yava\u015f ba\u015flang\u0131\u00e7. Sipari\u015fi ma\u011fazalar toplar, \u015firketin kuryeleri g\u00f6t\u00fcr\u00fcr.")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)[ ChannelRow(EChannel::App, TEXT("Telefon uygulamas\u0131: en b\u00fcy\u00fck ve en sad\u0131k sepetler, teslimat\u0131 en pahal\u0131. Yaz\u0131l\u0131m firmas\u0131n\u0131 sen se\u00e7ersin.")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)[ ChannelRow(EChannel::Platform, TEXT("H\u0131zl\u0131 teslimat platformu: onlar\u0131n kuryesi, \u00e7ok sipari\u015f, komisyon; ilk ma\u011faza da kat\u0131labilir.")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)[ ChannelRow(EChannel::Quick, TEXT("Kendi 30 dakika teslimat\u0131m\u0131z: ilde karanl\u0131k depo ister, kuryesi pahal\u0131d\u0131r.")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [ More([G] { if (!G()) return FString(); const TArray<FString> Lines = MarketOnline::RivalLines(G()->State);
            return Lines.Num() ? TEXT("Rakipler internette:\n") + FString::Join(Lines, TEXT("\n")) : FString(TEXT("Rakiplerden hen\u00fcz internete \u00e7\u0131kan yok.")); }) ]);

    // The provinces: who decides, what runs there, the dark store, the last month.
    TSharedRef<SVerticalBox> AreaRows = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 30; ++Slot)
    {
        auto Has = [G, Slot] { return G() && G()->State.Online.Areas.IsValidIndex(Slot); };
        auto Flags = [G, Has, Slot] { return Has() ? MarketOnline::AreaFlags(G()->State.Online.Areas[Slot]) : 0; };
        auto Toggle = [this, Flags, Slot](int32 Bit) { Manage(TEXT("OnlineArea"), MarketOnline::EncodeArea(Slot, Flags() ^ Bit)); };
        AreaRows->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBox).Visibility_Lambda([Has] { return Has() ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ Label([G, Has, Slot] { return Has() ? MarketOnline::AreaLine(G()->State, Slot) : FString(); }, 10, ERole::Text, false, true) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
                [
                    SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 4.f))
                    + SWrapBox::Slot()[ Choice(TEXT("Platform"), [Flags] { return (Flags() & 1) != 0; }, [Toggle] { Toggle(1); }) ]
                    + SWrapBox::Slot()[ Choice(TEXT("Kendi teslimat\u0131m\u0131z"), [Flags] { return (Flags() & 2) != 0; }, [Toggle] { Toggle(2); }) ]
                    + SWrapBox::Slot()[ Choice(TEXT("H\u0131zl\u0131"), [Flags] { return (Flags() & 4) != 0; }, [Toggle] { Toggle(4); },
                        [G, Has, Slot] { return Has() && G()->State.Online.Areas[Slot].DarkStoreDay > 0; }) ]
                    + SWrapBox::Slot()[ RiskyButton([] { return FString(TEXT("Karanl\u0131k depo")); },
                        [G, Slot] { return G() ? FString::Printf(TEXT("Bu ilde karanl\u0131k depo kurulsun mu? %s; her ay kira."), *MarketMenuUi::Tl(MarketOnline::DarkStorePrice(G()->State, Slot))) : FString(); },
                        [this, Slot] { Manage(TEXT("DarkStore"), Slot); },
                        [G, Slot] { FString Reason; return G() && MarketOnline::CanBuildDarkStore(G()->State, Slot, Reason); }) ]
                    + SWrapBox::Slot()[ Button([] { return FString(TEXT("Karar\u0131 b\u0131rak")); }, [this, Slot] { Manage(TEXT("OnlineAreaReturn"), Slot); }, false,
                        [G, Has, Slot] { return Has() && G()->State.Online.Areas[Slot].bPlayerSet; }) ]
                ]
            ]
        ];
    }
    auto DefaultFlags = [G] { return G() ? (G()->State.Online.bDefaultPlatform ? 1 : 0) | (G()->State.Online.bDefaultOwn ? 2 : 0) | (G()->State.Online.bDefaultQuick ? 4 : 0) : 0; };
    TSharedRef<SWidget> AreaCard = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("\u0130LLER")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 6.f)
        [ Label([] { return FString(TEXT("\u0130l m\u00fcd\u00fcr\u00fc olan ilde m\u00fcd\u00fcr birka\u00e7 ay\u0131n sonucuna bak\u0131p de\u011fi\u015fiklik \u00f6nerir; \u00f6neri \u00fclke (ya da b\u00f6lge) m\u00fcd\u00fcr\u00fcnden ge\u00e7ip onay\u0131na gelir. Bir se\u00e7ene\u011fe bas\u0131nca karar senin olur. M\u00fcd\u00fcr\u00fc olmayan illerde \u015firket kural\u0131 \u00e7al\u0131\u015f\u0131r:")); }, 10, ERole::Muted, false, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
        [
            SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 4.f))
            + SWrapBox::Slot()[ Choice(TEXT("Platform"), [DefaultFlags] { return (DefaultFlags() & 1) != 0; }, [this, DefaultFlags] { Manage(TEXT("OnlineDefault"), DefaultFlags() ^ 1); }) ]
            + SWrapBox::Slot()[ Choice(TEXT("Kendi teslimat\u0131m\u0131z"), [DefaultFlags] { return (DefaultFlags() & 2) != 0; }, [this, DefaultFlags] { Manage(TEXT("OnlineDefault"), DefaultFlags() ^ 2); }) ]
            + SWrapBox::Slot()[ Choice(TEXT("Karanl\u0131k deposu olan ilde h\u0131zl\u0131"), [DefaultFlags] { return (DefaultFlags() & 4) != 0; }, [this, DefaultFlags] { Manage(TEXT("OnlineDefault"), DefaultFlags() ^ 4); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight()[ AreaRows ]);

    // Policy, the e-commerce manager, the numbers.
    auto Level = [this, G](const FString& Text, const TCHAR* Action, int32 Value, uint8 FMarketOnline::* Field) -> TSharedRef<SWidget>
    {
        return Choice(Text, [G, Value, Field] { return G() && G()->State.Online.*Field == Value; }, [this, Action, Value] { Manage(Action, Value); });
    };
    TSharedRef<SWidget> PolicyCard = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("POL\u0130T\u0130KA")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 2.f)[ Fixed(TEXT("Teslimat \u00fccreti"), 10, ERole::Muted) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 4.f))
            + SWrapBox::Slot()[ Level(TEXT("\u00dccretsiz"), TEXT("OnlineFee"), 0, &FMarketOnline::Fee) ]
            + SWrapBox::Slot()[ Level(TEXT("K\u00fc\u00e7\u00fck sepete"), TEXT("OnlineFee"), 1, &FMarketOnline::Fee) ]
            + SWrapBox::Slot()[ Level(TEXT("Her sipari\u015fe"), TEXT("OnlineFee"), 2, &FMarketOnline::Fee) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 2.f)[ Fixed(TEXT("En az sepet"), 10, ERole::Muted) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 4.f))
            + SWrapBox::Slot()[ Level(TEXT("Yok"), TEXT("OnlineMinBasket"), 0, &FMarketOnline::MinBasket) ]
            + SWrapBox::Slot()[ Level(TEXT("K\u00fc\u00e7\u00fck"), TEXT("OnlineMinBasket"), 1, &FMarketOnline::MinBasket) ]
            + SWrapBox::Slot()[ Level(TEXT("B\u00fcy\u00fck"), TEXT("OnlineMinBasket"), 2, &FMarketOnline::MinBasket) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 2.f)[ Fixed(TEXT("\u0130nternet fiyat\u0131"), 10, ERole::Muted) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 4.f))
            + SWrapBox::Slot()[ Level(TEXT("Rafla ayn\u0131"), TEXT("OnlinePriceGap"), 0, &FMarketOnline::PriceGap) ]
            + SWrapBox::Slot()[ Level(TEXT("+%5"), TEXT("OnlinePriceGap"), 1, &FMarketOnline::PriceGap) ]
            + SWrapBox::Slot()[ Level(TEXT("+%10"), TEXT("OnlinePriceGap"), 2, &FMarketOnline::PriceGap) ]
        ]
        // C7 (A menu comparison): the internet ads live under the advertising manager; a link instead of a pointer.
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Fixed(TEXT("\u0130nternet reklam\u0131 (sosyal medya, arama) reklam kart\u0131nda."), 10, ERole::Muted) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f, 0.f, 0.f)
            [ Button([] { return FString(TEXT("\u015eirket \u203a Reklam")); }, [this] { BranchTab = 1; Go(Branches); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 2.f)[ Fixed(TEXT("\u00dcr\u00fcn eksikse"), 10, ERole::Muted) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 4.f))
            + SWrapBox::Slot()[ Level(TEXT("Aray\u0131p sor"), TEXT("Substitute"), 0, &FMarketOnline::Substitute) ]
            + SWrapBox::Slot()[ Level(TEXT("Benzerini koy"), TEXT("Substitute"), 1, &FMarketOnline::Substitute) ]
            + SWrapBox::Slot()[ Level(TEXT("\u00c7\u0131kar"), TEXT("Substitute"), 2, &FMarketOnline::Substitute) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 4.f)[ Section(TEXT("E-T\u0130CARET M\u00dcD\u00dcR\u00dc")) ]
        + SVerticalBox::Slot().AutoHeight()
        [ Label([G]
        {
            if (!G()) return FString();
            const FMarketOnline& O = G()->State.Online;
            if (!O.ManagerName.IsEmpty()) return FString::Printf(TEXT("%s \u00b7 beceri %d \u00b7 ayda %s%s"), *O.ManagerName, O.ManagerSkill, *MarketMenuUi::Tl(O.ManagerWage * 30), O.bAutoPolicy ? TEXT(" \u00b7 politikay\u0131 o ayarl\u0131yor") : TEXT(""));
            FString Name; int32 Skill = 0; int64 Wage = 0;
            MarketOnline::Candidate(G()->State, Name, Skill, Wage);
            FString Reason;
            return MarketOnline::CanHireManager(G()->State, Reason) ? FString::Printf(TEXT("Bu haftan\u0131n aday\u0131: %s, beceri %d, ayda %s."), *Name, Skill, *MarketMenuUi::Tl(Wage * 30)) : Reason;
        }, 10, ERole::Text, false, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
        [
            SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 4.f))
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("\u0130\u015fe al")); }, [this] { Manage(TEXT("OnlineHire"), 0); }, false, [G] { FString Reason; return G() && MarketOnline::CanHireManager(G()->State, Reason); }) ]
            + SWrapBox::Slot()[ RiskyButton([] { return FString(TEXT("Ayr\u0131lmas\u0131n\u0131 iste")); }, [] { return FString(TEXT("E-ticaret m\u00fcd\u00fcr\u00fc ayr\u0131ls\u0131n m\u0131? On g\u00fcnl\u00fck \u00fccret ve k\u0131dem tazminat\u0131 \u00f6denir.")); },
                [this] { Manage(TEXT("OnlineFire"), 0); }, [G] { return G() && !G()->State.Online.ManagerName.IsEmpty(); }) ]
            + SWrapBox::Slot()[ Choice(TEXT("Politika onda"), [G] { return G() && G()->State.Online.bAutoPolicy; }, [this, G] { Manage(TEXT("OnlineAutoPolicy"), G() && G()->State.Online.bAutoPolicy ? 0 : 1); },
                [G] { return G() && !G()->State.Online.ManagerName.IsEmpty(); }) ]
        ]);

    TSharedRef<SWidget> StatsCard = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("KANALLARIN GE\u00c7EN AYI")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
        [ Label([G]
        {
            if (!G()) return FString();
            TArray<FString> Lines;
            for (int32 C = 0; C < MarketOnline::ChannelCount; ++C) Lines.Add(MarketOnline::ChannelStats(G()->State, static_cast<EChannel>(C)));
            Lines.Add(MarketOnline::AdsLine(G()->State));
            Lines.Add(FString::Printf(TEXT("Y\u0131ld\u0131zlar: platform %.1f, uygulama %.1f \u00b7 itibar %.0f"), MarketOnline::Stars(G()->State), MarketOnline::AppStars(G()->State), G()->State.Online.Reputation));
            return FString::Join(Lines, TEXT("\n"));
        }, 10, ERole::Text, false, true) ]);

    TSharedRef<SWidget> Pay = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("\u00d6DEME")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 8.f)[ Label([G] { return G() ? MarketPayments::Summary(G()->State) : FString(); }, 11, ERole::Text, false, true) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 6.f))
            + SWrapBox::Slot()[ Choice(TEXT("POS var"), [G] { return G() && G()->State.Payments.bCard; }, [this] { Manage(TEXT("Card"), 1); }) ]
            + SWrapBox::Slot()[ Choice(TEXT("POS yok"), [G] { return G() && !G()->State.Payments.bCard; }, [this] { Manage(TEXT("Card"), 0); }) ]
            + SWrapBox::Slot()[ Choice(TEXT("Yemek kart\u0131 al\u0131n\u0131r"), [G] { return G() && G()->State.Payments.bMealCard; }, [this] { Manage(TEXT("MealCard"), 1); }) ]
            + SWrapBox::Slot()[ Choice(TEXT("Yemek kart\u0131 yok"), [G] { return G() && !G()->State.Payments.bMealCard; }, [this] { Manage(TEXT("MealCard"), 0); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [ More([] { return FString::Printf(TEXT("POS: ayl\u0131k kira ve %%%.1f komisyon, para ertesi g\u00fcn gelir; POS yoksa kartla \u00f6demek isteyenlerin bir k\u0131sm\u0131 sepeti b\u0131rak\u0131r. Yemek kart\u0131: %%%.0f komisyon, \u00f6\u011fle aras\u0131 \u00e7al\u0131\u015fanlar\u0131 getirir."),
            MarketPayments::CardCommission * 100.0, MarketPayments::MealCommission * 100.0); }) ]);

    return SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ SNew(SBox).Visibility_Lambda(Shown)[ ChannelCard ] ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda(Shown)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)[ AreaCard ]
                + SHorizontalBox::Slot().FillWidth(1.f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ PolicyCard ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[ StatsCard ]
                ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[ Pay ]
    ];
}

// ---------------------------------------------------------------------------------------------------------------
// 9 Subeler

// ---------------------------------------------------------------------------------------------------------------
// G-086 sade ana ekran (Docs/Kurgu/03_MAGAZA_AGI.md \u00a712): the map is the stage. Region chips above it zoom in;
// a click on a province opens one panel on the right (its numbers, our shops, the four market types to open); one
// assistant line tells the most important thing today. Numbers sit in the top pills, actions in the dock.

FString SMarketMenu::ShownCountry() const
{
    const AMarketGameMode* G = Game.Get();
    if (!MapCountry.IsEmpty() && MarketCountry::Find(MapCountry)) return MapCountry;
    return G ? G->State.CountryId : MarketCountry::DefaultId();
}

FString SMarketMenu::ShownProvince() const
{
    const AMarketGameMode* G = Game.Get();
    const FString Country = ShownCountry();
    if (!MapProvinceId.IsEmpty() && MarketCountry::FindCity(Country, MapProvinceId)) return MapProvinceId;
    if (G && Country == G->State.CountryId) return MarketStart::HomeProvince(G->State);
    const MarketCountry::FProfile* Pack = MarketCountry::Find(Country);
    return Pack && Pack->Cities.Num() > 0 ? Pack->Cities[0].Id : FString();
}

TSharedRef<SWidget> SMarketMenu::HomePage()
{
    // G-086d, design boards 4 (no province chosen) and 5 (the panel open).
    auto HasMap = [this] { return MarketCountry::HasMap(ShownCountry()); };
    auto Chip = [this](const FString& Name, TFunction<bool()> On, TFunction<void()> Click) -> TSharedRef<SWidget>
    {
        return SNew(SBox).HeightOverride(28.f)
        [
            SNew(SButton).ButtonStyle(&RoundStyle).IsFocusable(false).ContentPadding(FMargin(12.f, 0.f)).VAlign(VAlign_Center)
            .ButtonColorAndOpacity_Lambda([this, On] { return FSlateColor(On() ? Color(ERole::Inset) : FLinearColor::Transparent); })
            .OnClicked_Lambda([Click] { Click(); return FReply::Handled(); })
            [
                SNew(STextBlock).Text(FText::FromString(Name))
                .Font_Lambda([On] { return MarketTheme::Font(On() ? MarketTheme::EFace::Semi : MarketTheme::EFace::Medium, 12.f); })
                .ColorAndOpacity_Lambda([this, On] { return FSlateColor(Color(On() ? ERole::Text : ERole::Muted)); })
            ]
        ];
    };
    auto Tray = [this](const TSharedRef<SWidget>& Row) -> TSharedRef<SWidget>
    {
        return Raised(SNew(SBorder).BorderImage(&RoundBrush).BorderBackgroundColor(Col(ERole::Solid)).Padding(FMargin(4.f))[ Row ]);
    };

    // What the map colours (board 4: our shops, the rivals, the opportunities).
    TSharedRef<SHorizontalBox> LayerRow = SNew(SHorizontalBox);
    const TCHAR* LayerNames[3] = { TEXT("Ma\u011fazalar\u0131m\u0131z"), TEXT("Rakipler"), TEXT("F\u0131rsatlar") };
    for (int32 Layer = 0; Layer < 3; ++Layer)
        LayerRow->AddSlot().AutoWidth().Padding(Layer > 0 ? 4.f : 0.f, 0.f, 0.f, 0.f)
        [ Chip(LayerNames[Layer], [this, Layer] { return MapLayer == Layer; }, [this, Layer] { MapLayer = Layer; }) ];

    // C5 (A menu list): the colour key of the rivals / opportunities layers, under the chips while that layer is on.
    // The swatches use the same blends as HomeMap's Fill (Land -> Warn for rivals, Land -> Ours for opportunities).
    auto LegendSwatch = [this](float Amount) -> TSharedRef<SWidget>
    {
        return SNew(SBox).WidthOverride(12.f).HeightOverride(12.f).VAlign(VAlign_Center)
        [
            SNew(SBorder).BorderImage(&BadgeBrush).Padding(0.f)
            .BorderBackgroundColor_Lambda([this, Amount]
            {
                const FLinearColor Land = Color(ERole::Land);
                return FSlateColor(MapLayer == 2 ? FLinearColor::LerpUsingHSV(Land, Color(ERole::Ours), Amount)
                    : FLinearColor::LerpUsingHSV(Land, Color(ERole::Warn), 0.06f + 0.6f * Amount));
            })
        ];
    };
    auto LegendWord = [this](bool bHigh) -> TSharedRef<SWidget>
    {
        return TextPx([this, bHigh]
        {
            if (MapLayer == 2) return FString(bHigh ? TEXT("iyi f\u0131rsat (ma\u011fazam\u0131z yok)") : TEXT("f\u0131rsat az / ma\u011fazam\u0131z var"));
            return FString(bHigh ? TEXT("rakip yo\u011fun") : TEXT("rakip az"));
        }, 11.f, [] { return ERole::Muted; });
    };
    TSharedRef<SWidget> Legend = SNew(SBox).Padding(FMargin(12.f, 2.f, 12.f, 4.f))
        .Visibility_Lambda([this] { return MapLayer == 1 || MapLayer == 2 ? EVisibility::Visible : EVisibility::Collapsed; })
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 6.f, 0.f)[ LegendSwatch(0.f) ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 14.f, 0.f)[ LegendWord(false) ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 2.f, 0.f)[ LegendSwatch(0.5f) ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 6.f, 0.f)[ LegendSwatch(1.f) ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ LegendWord(true) ]
    ];
    TSharedRef<SWidget> LayerTray = SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[ LayerRow ]
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[ Legend ];
    // C5 (A istek 1): the dock (BottomNav) is 58 px items + 2 x 8 px padding, 24 px from the bottom; the tray keeps
    // 14 px clear above it (the same line as the dock's "Diger" card), at every resolution (the menu is scaled).
    const float LayerTrayBottom = 24.f + 58.f + 2.f * 8.f + 14.f;

    // G-086e: the stage never changes its layout. The province panel floats on the right and glides in; the map
    // glides left to make room for it (SMarketMap RightInset).
    auto Ease = [this] { const float T = PanelAnim; return T * T * (3.f - 2.f * T); };
    return SNew(SOverlay)
        + SOverlay::Slot()[ SNew(SBorder).BorderImage(&FlatBrush).BorderBackgroundColor(Col(ERole::Stage)) ]
        + SOverlay::Slot().Padding(FMargin(70.f, 130.f, 70.f, 160.f))[ HomeMap() ] // C5: clear of the layer tray
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, LayerTrayBottom) // C3 (A istek 2) / C5: above the dock
        [ SNew(SBox).Visibility_Lambda([HasMap] { return HasMap() ? EVisibility::Visible : EVisibility::Hidden; })[ Tray(LayerTray) ] ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0.f, 84.f, 0.f, 0.f)[ RegionChips() ]
        // D7: the world card under the chips.
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0.f, 128.f, 0.f, 0.f)
        [ SNew(SBox).WidthOverride(620.f).Visibility_Lambda([this] { return bWorldOpen ? EVisibility::Visible : EVisibility::Collapsed; })[ WorldCard() ] ]
        + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(24.f, 0.f, 0.f, 28.f)[ Assistant() ]
        // C3 (B6): the goals at three scales, always in sight on the main screen.
        + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(24.f, 128.f, 0.f, 0.f)
        [ SNew(SBox).WidthOverride(300.f).Visibility_Lambda([this] { return Game.Get() && Game.Get()->State.Day > 1 ? EVisibility::Visible : EVisibility::Collapsed; })[ GoalsCard() ] ]
        + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Fill).Padding(0.f, 128.f, 24.f, 112.f)
        [
            SNew(SBox).WidthOverride(PanelWidth)
            .Visibility_Lambda([this] { return PanelAnim > 0.005f ? (PanelOpen() ? EVisibility::Visible : EVisibility::HitTestInvisible) : EVisibility::Collapsed; })
            .RenderTransform_Lambda([this, Ease] { return TOptional<FSlateRenderTransform>(FSlateRenderTransform(FVector2f((1.f - Ease()) * (PanelWidth + 40.f), 0.f))); })
            [ ProvinceCard() ]
        ];
}

TSharedRef<SWidget> SMarketMenu::HomeMap()
{
    auto G = [this] { return Game.Get(); };
    auto HasMap = [this] { return MarketCountry::HasMap(ShownCountry()); };
    auto IdOf = [](int32 Index) -> FString
    {
        const MarketMapData::FData& Data = MarketMapData::Get();
        return Data.Provinces.IsValidIndex(Index) ? Data.Provinces[Index].Id : FString();
    };
    // Our shops on the map count only in the map's country (D3: the pack drawn from iller.json).
    auto Shops = [G, IdOf](int32 Index) { return G() ? MarketMenuPagesUi::OurShops(G()->State, MarketCountry::MapCountry(), IdOf(Index)) : 0; };
    auto IsHome = [G, IdOf](int32 Index) { return G() && MarketStart::HomeProvince(G()->State) == IdOf(Index); };
    auto InRegion = [this, IdOf](int32 Index)
    {
        if (MapRegion.IsEmpty()) return true;
        const MarketCountry::FCity* City = MarketCountry::FindCity(MarketCountry::MapCountry(), IdOf(Index));
        return City && City->Region == MapRegion;
    };
    auto Fill = [this, G, IdOf, Shops, IsHome, InRegion](int32 Index) -> FLinearColor
    {
        const FLinearColor Land = Color(ERole::Land);
        const MarketCountry::FCity* City = MarketCountry::FindCity(MarketCountry::MapCountry(), IdOf(Index));
        if (!G() || !City) return Land;
        FLinearColor Out = Land;
        if (MapLayer == 1)
        {
            // Rivals: warmer where the chains crowd.
            const float Heat = FMath::Clamp((City->Competition - 0.6f) / 0.9f, 0.f, 1.f);
            Out = FLinearColor::LerpUsingHSV(Land, Color(ERole::Warn), 0.06f + 0.6f * Heat);
        }
        else if (MapLayer == 2)
        {
            // Opportunities: provinces without a shop of ours, buying power against rivals and rent.
            const float Score = City->Income / FMath::Max(0.3f, City->Competition * FMath::Sqrt(FMath::Max(0.2f, City->Rent)));
            const float Pull = Shops(Index) > 0 ? 0.f : FMath::Clamp((Score - 0.75f) / 0.5f, 0.f, 1.f);
            Out = FLinearColor::LerpUsingHSV(Land, Color(ERole::Ours), Pull);
        }
        else if (Shops(Index) > 0) Out = Color(IsHome(Index) ? ERole::Home : ERole::Ours);
        // Outside the chosen region the provinces fade into the paper.
        return InRegion(Index) ? Out : FMath::Lerp(Out, Color(ERole::Stage), 0.6f);
    };
    auto Worrying = [G, IdOf](int32 Index) { return G() && MarketMenuPagesUi::WorstGrade(G()->State, MarketCountry::MapCountry(), IdOf(Index)) == TEXT("D"); };
    TSharedRef<SWidget> Map = SNew(SMarketMap)
        .FillOf(Fill)
        .LineColor([this] { return Color(ERole::Stage); })
        .Selected([this] { return PanelOpen() && ShownCountry() == MarketCountry::MapCountry() ? MarketMenuPagesUi::MapIndexOf(MapProvinceId) : INDEX_NONE; })
        .InView([this, InRegion](int32 Index) { return !MapRegion.IsEmpty() && InRegion(Index); })
        .HasPin([this, Shops](int32 Index) { return MapLayer == 0 && Shops(Index) > 0; })
        .PinCount([Shops](int32 Index) { return Shops(Index); })
        .PinColor([this, IsHome, Worrying](int32 Index) { return Color(Worrying(Index) ? ERole::Warn : IsHome(Index) ? ERole::Text : ERole::Accent); })
        .PinTextOf([this, IsHome, Worrying](int32 Index) { return Color(!Worrying(Index) && IsHome(Index) ? ERole::PrimaryText : ERole::OnAccent); })
        .WarnOf([this, Worrying](int32 Index) { return MapLayer == 0 && Worrying(Index); })
        .WarnColor([this] { return Color(ERole::Warn); })
        .StrongColor([this] { return Color(ERole::Text); })
        .LabelOf([this, Shops](int32 Index) { return MapLayer == 0 && Shops(Index) > 0; })
        .LabelColor([this] { return Color(ERole::MapLabel); })
        .RightInset([this] { const float T = PanelAnim; return T * T * (3.f - 2.f * T) * (PanelWidth + 24.f - 70.f + 16.f); })
        // G-089: our depots (a square "D" beside the pin) and the range of the chosen depot (else the open province's).
        .DepotOf([this, G, IdOf](int32 Index) { return MapLayer == 0 && G() && MarketDepots::HasDepotIn(G()->State, MarketCountry::MapCountry(), IdOf(Index)); })
        .DepotColor([this] { return Color(ERole::Info); })
        .DepotTextColor([this] { return Color(ERole::Solid); })
        .RingOf([this, G]() -> int32
        {
            if (MapLayer != 0 || !G()) return INDEX_NONE;
            const TArray<FMarketDepot>& Sites = G()->State.Company.DepotSites;
            int32 Depot = Sites.IsValidIndex(DepotSel) ? DepotSel : INDEX_NONE;
            if (Depot == INDEX_NONE && PanelOpen() && ShownCountry() == MarketCountry::MapCountry()) Depot = MarketDepots::Find(G()->State, MarketCountry::MapCountry(), MapProvinceId);
            if (!Sites.IsValidIndex(Depot) || Sites[Depot].Country != MarketCountry::MapCountry()) return INDEX_NONE;
            return MarketMenuPagesUi::MapIndexOf(Sites[Depot].Province);
        })
        .RingRadius([]
        {
            const MarketCountry::FProfile* Pack = MarketCountry::Find(MarketCountry::MapCountry());
            return Pack && Pack->MapKm > 0.f ? static_cast<float>(MarketDepots::RangeKm) / Pack->MapKm : 0.f;
        })
        .RingColor([this] { return Color(ERole::Info); })
        .OnPick([this, G, IdOf](int32 Index)
        {
            const FString Id = IdOf(Index);
            // A second click on the open province closes its panel.
            if (PanelOpen() && ShownCountry() == MarketCountry::MapCountry() && MapProvinceId == Id) { MapProvinceId.Reset(); DepotSel = INDEX_NONE; return; }
            MapCountry = MarketCountry::MapCountry();
            MapProvinceId = Id;
            // A depot's range follows the province clicked (its own depot, none otherwise).
            DepotSel = G() ? MarketDepots::Find(G()->State, MarketCountry::MapCountry(), Id) : INDEX_NONE;
            if (G()) G()->MapProvince = Index;
        });

    // Packs without a map: the provinces of the chosen region as pills (with our shops).
    TSharedRef<SWrapBox> List = SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 6.f));
    for (const MarketCountry::FProfile& Pack : MarketCountry::All())
    {
        const FString Country = Pack.Id;
        for (const MarketCountry::FCity& City : Pack.Cities)
        {
            const FString Id = City.Id, Name = City.Name, Region = City.Region;
            auto IsSel = [this, Country, Id] { return PanelOpen() && ShownCountry() == Country && MapProvinceId == Id; };
            List->AddSlot()
            [
                SNew(SBox).Visibility_Lambda([this, Country, Region, HasMap]
                {
                    return ShownCountry() == Country && !HasMap() && (MapRegion.IsEmpty() || MapRegion == Region) ? EVisibility::Visible : EVisibility::Collapsed;
                })
                [
                    SNew(SBox).HeightOverride(32.f)
                    [
                        SNew(SButton).ButtonStyle(&RoundStyle).IsFocusable(false).ContentPadding(FMargin(14.f, 0.f)).VAlign(VAlign_Center)
                        .ButtonColorAndOpacity_Lambda([this, G, Country, Id, IsSel]
                        {
                            const bool bOurs = G() && MarketMenuPagesUi::OurShops(G()->State, Country, Id) > 0;
                            return FSlateColor(Color(IsSel() ? ERole::Text : bOurs ? ERole::Ours : ERole::Land));
                        })
                        .OnClicked_Lambda([this, Country, Id, IsSel]
                        {
                            if (IsSel()) MapProvinceId.Reset();
                            else { MapCountry = Country; MapProvinceId = Id; }
                            return FReply::Handled();
                        })
                        [
                            SNew(STextBlock).Font(MarketTheme::Font(MarketTheme::EFace::Semi, 12.f))
                            .Text_Lambda([G, Country, Id, Name]
                            {
                                const int32 N = G() ? MarketMenuPagesUi::OurShops(G()->State, Country, Id) : 0;
                                return FText::FromString(N > 0 ? FString::Printf(TEXT("%s \u00b7 %d"), *Name, N) : Name);
                            })
                            .ColorAndOpacity_Lambda([this, IsSel] { return FSlateColor(Color(IsSel() ? ERole::PrimaryText : ERole::Text)); })
                        ]
                    ]
                ]
            ];
        }
    }
    return SNew(SWidgetSwitcher).WidgetIndex_Lambda([HasMap] { return HasMap() ? 0 : 1; })
        + SWidgetSwitcher::Slot()[ Map ]
        + SWidgetSwitcher::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [
            SNew(SBox).MaxDesiredWidth(900.f).MaxDesiredHeight(460.f)
            [ SNew(SScrollBox) + SScrollBox::Slot()[ List ] ]
        ];
}

TSharedRef<SWidget> SMarketMenu::RegionChips()
{
    // The country and its main regions; a click zooms the map to the region (again: back to the whole country).
    auto G = [this] { return Game.Get(); };
    auto Chip = [this](const FString& Name, TFunction<bool()> On, TFunction<void()> Click) -> TSharedRef<SWidget>
    {
        return SNew(SBox).HeightOverride(28.f)
        [
            SNew(SButton).ButtonStyle(&RoundStyle).IsFocusable(false).ContentPadding(FMargin(12.f, 0.f)).VAlign(VAlign_Center)
            .ButtonColorAndOpacity_Lambda([this, On] { return FSlateColor(On() ? Color(ERole::Inset) : FLinearColor::Transparent); })
            .OnClicked_Lambda([Click] { Click(); return FReply::Handled(); })
            [
                SNew(STextBlock).Text(FText::FromString(Name))
                .Font_Lambda([On] { return MarketTheme::Font(On() ? MarketTheme::EFace::Semi : MarketTheme::EFace::Medium, 12.f); })
                .ColorAndOpacity_Lambda([this, On] { return FSlateColor(Color(On() ? ERole::Text : ERole::Muted)); })
            ]
        ];
    };
    TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
    // D7: the world card, once the way abroad is open.
    Row->AddSlot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)
    [
        SNew(SBox).Visibility_Lambda([G] { return G() && MarketCompany::AbroadOpen(G()->State) ? EVisibility::Visible : EVisibility::Collapsed; })
        [ Chip(TEXT("D\u00fcnya"), [this] { return bWorldOpen; }, [this] { bWorldOpen = !bWorldOpen; }) ]
    ];
    for (const MarketCountry::FProfile& Pack : MarketCountry::All())
    {
        const FString PackId = Pack.Id;
        auto Mine = [this, PackId] { return ShownCountry() == PackId ? EVisibility::Visible : EVisibility::Collapsed; };
        Row->AddSlot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)
        [ SNew(SBox).Visibility_Lambda(Mine)[ Chip(Pack.Name, [this] { return MapRegion.IsEmpty(); }, [this] { MapRegion.Reset(); }) ] ];
        for (const MarketCountry::FRegion& Region : Pack.Regions)
        {
            const FString RegionId = Region.Id;
            Row->AddSlot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)
            [ SNew(SBox).Visibility_Lambda(Mine)
                [ Chip(Region.Name, [this, RegionId] { return MapRegion == RegionId; }, [this, RegionId] { MapRegion = MapRegion == RegionId ? FString() : RegionId; }) ] ];
        }
    }
    // Other countries: D6 (M67) once the company is big enough at home or is already abroad.
    auto Abroad = [G] { return G() && MarketCompany::AbroadOpen(G()->State); };
    Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 8.f, 0.f)
    [
        SNew(SBox).WidthOverride(1.f).HeightOverride(18.f).Visibility_Lambda([Abroad] { return Abroad() ? EVisibility::Visible : EVisibility::Collapsed; })
        [ SNew(SBorder).BorderImage(&FlatBrush).BorderBackgroundColor(Col(ERole::Line)) ]
    ];
    for (const MarketCountry::FProfile& Pack : MarketCountry::All())
    {
        const FString PackId = Pack.Id;
        Row->AddSlot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([this, Abroad, PackId] { return Abroad() && ShownCountry() != PackId ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Chip(Pack.Name, [] { return false; }, [this, PackId] { MapCountry = PackId; MapRegion.Reset(); MapProvinceId.Reset(); bWorldOpen = false; }) ]
        ];
    }
    return Raised(SNew(SBorder).BorderImage(&RoundBrush).BorderBackgroundColor(Col(ERole::Solid)).Padding(FMargin(4.f))[ Row ]);
}

TSharedRef<SWidget> SMarketMenu::WorldCard()
{
    // D7: the countries of the packs by continent; ours first in mind: our shops, a partner, a study, or why closed.
    // The lines are built once a frame (the country tables are not free).
    auto G = [this] { return Game.Get(); };
    struct FWorldCache
    {
        uint64 Frame = 0;
        bool bFilled = false;
        TMap<FString, FString> Lines;
    };
    TSharedRef<FWorldCache> Cache = MakeShared<FWorldCache>();
    auto LineOf = [G, Cache](const FString& Country) -> FString
    {
        if (!G()) return FString();
        if (!Cache->bFilled || Cache->Frame != GFrameCounter)
        {
            Cache->bFilled = true;
            Cache->Frame = GFrameCounter;
            Cache->Lines.Reset();
            for (const MarketCountry::FProfile& P : MarketCountry::All()) Cache->Lines.Add(P.Id, MarketCompany::CountryStatus(G()->State, P.Id));
        }
        const FString* Line = Cache->Lines.Find(Country);
        return Line ? *Line : FString();
    };
    TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
    TArray<FString> Continents = MarketCountry::Continents();
    Continents.Add(FString()); // packs without a continent
    for (const FString& Continent : Continents)
    {
        TArray<const MarketCountry::FProfile*> Packs;
        for (const MarketCountry::FProfile& P : MarketCountry::All()) if (P.Continent == Continent && P.Cities.Num() > 0) Packs.Add(&P);
        if (Packs.Num() == 0) continue;
        Rows->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 2.f)
        [ Label([Continent] { return Continent.IsEmpty() ? FString(TEXT("Di\u011fer")) : MarketCountry::ContinentName(Continent); }, 10, ERole::Muted, true) ];
        for (const MarketCountry::FProfile* P : Packs)
        {
            const FString Id = P->Id;
            const FString Name = P->Name;
            Rows->AddSlot().AutoHeight().Padding(0.f, 2.f)
            [
                SNew(SButton).ButtonStyle(&RoundStyle).IsFocusable(false).ContentPadding(FMargin(10.f, 5.f))
                .ButtonColorAndOpacity_Lambda([this, Id] { return FSlateColor(ShownCountry() == Id ? Color(ERole::Inset) : FLinearColor::Transparent); })
                .OnClicked_Lambda([this, Id] { MapCountry = Id; MapRegion.Reset(); MapProvinceId.Reset(); bWorldOpen = false; return FReply::Handled(); })
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [ SNew(SBox).WidthOverride(150.f)[ Label([Name] { return Name; }, 11, ERole::Text, true) ] ]
                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                    [ Label([LineOf, Id] { return LineOf(Id); }, 10, ERole::Muted, false, true) ]
                ]
            ];
        }
    }
    // D7: the schematic map above the list: a dot a country, its colour our standing there.
    auto DotRole = [G](const FString& Country) -> ERole
    {
        if (!G()) return ERole::Line;
        const FMarketState& S = G()->State;
        if (Country == S.CountryId) return ERole::Primary;
        if (MarketCompany::CountryStores(S, Country) > 0) return ERole::Good;
        if (MarketFranchise::Find(S, Country)) return ERole::Info;
        const MarketResearch::EStatus Study = MarketResearch::Status(S, Country);
        if (Study == MarketResearch::EStatus::Running || Study == MarketResearch::EStatus::Ready) return ERole::Warn;
        return ERole::Line;
    };
    TSharedPtr<SMarketWorldMap> WorldMap;
    TSharedRef<SWidget> MapView = SNew(SBox).HeightOverride(280.f)
    [
        SAssignNew(WorldMap, SMarketWorldMap)
        .DotColor([this, DotRole](const FString& Country) { return Color(DotRole(Country)); })
        .IsSelected([this](const FString& Country) { return ShownCountry() == Country; })
        .OnPick([this](const FString& Country) { MapCountry = Country; MapRegion.Reset(); MapProvinceId.Reset(); bWorldOpen = false; })
        .AreaColor([this] { return Color(ERole::Inset); })
        .TextColor([this] { return Color(ERole::Text); })
        .MutedColor([this] { return Color(ERole::Muted); })
    ];
    TWeakPtr<SMarketWorldMap> WeakMap = WorldMap;
    auto HoverLine = [LineOf, WeakMap]() -> FString
    {
        const TSharedPtr<SMarketWorldMap> Map = WeakMap.Pin();
        if (!Map.IsValid() || Map->GetHovered().IsEmpty())
            return FString(TEXT("Renkler: koyu ana \u00fclke \u00b7 ye\u015fil ma\u011fazam\u0131z var \u00b7 mavi ortakl\u0131k \u00b7 sar\u0131 pazar ara\u015ft\u0131rmas\u0131 \u00b7 gri girilmedi."));
        return MarketCountry::FindOrDefault(Map->GetHovered()).Name + TEXT(": ") + LineOf(Map->GetHovered());
    };
    return Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 2.f)[ Section(TEXT("D\u00dcNYA")) ]
        + SVerticalBox::Slot().AutoHeight()[ Label([] { return FString(TEXT("Bir \u00fclkeye t\u0131kla: il haritas\u0131 a\u00e7\u0131l\u0131r; ara\u015ft\u0131rma, ma\u011faza ve ortakl\u0131k oradan.")); }, 9, ERole::Muted, false, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[ MapView ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 4.f)[ Label(HoverLine, 10, ERole::Text, false, true) ]
        + SVerticalBox::Slot().AutoHeight()[ SNew(SBox).MaxDesiredHeight(260.f)[ SNew(SScrollBox) + SScrollBox::Slot()[ Rows ] ] ]);
}

TSharedRef<SWidget> SMarketMenu::Assistant()
{
    // Board 4, bottom left: the one thing that matters most today (the most urgent to-do, else a note of the day
    // that changes daily). "Bak" opens the page that solves it, or the decisions.
    struct FLine { FString Title; FString Body; int32 Page = INDEX_NONE; int32 Product = INDEX_NONE; bool bWarn = false; };
    auto G = [this] { return Game.Get(); };
    auto Pick = [G]() -> FLine
    {
        FLine Line;
        if (!G()) return Line;
        const FMarketTodo* Best = nullptr;
        for (const FMarketTodo& Todo : G()->Todos()) if (!Best || Todo.Severity > Best->Severity) Best = &Todo;
        if (Best && Best->Severity >= 1)
        {
            Line.Title = Best->Title;
            Line.Body = Best->Text;
            Line.Page = Best->Page;
            Line.Product = Best->Product;
            Line.bWarn = true;
            return Line;
        }
        // M32: what the assistant heard about online selling (rumours, rivals going online) comes before the daily notes.
        const FString Heard = MarketOnline::Hint(G()->State);
        if (!Heard.IsEmpty())
        {
            Line.Title = TEXT("Kulak misafiri");
            Line.Body = Heard;
            Line.Page = SMarketMenu::Channels;
            return Line;
        }
        const TArray<MarketMenuPagesUi::FRotCard> Cards = MarketMenuPagesUi::RotCards(*G());
        if (Cards.Num() == 0) return Line;
        const MarketMenuPagesUi::FRotCard& Card = Cards[G()->State.Day % Cards.Num()];
        Line.Title = Card.Headline;
        Line.Body = Card.Body;
        Line.Page = Card.Page;
        Line.bWarn = Card.Title.StartsWith(TEXT("D\u0130KKAT")) || Card.Title == TEXT("KASA");
        return Line;
    };
    return Raised(SNew(SBox).WidthOverride(300.f)
    [
        SNew(SBorder).BorderImage(&CardBrush).BorderBackgroundColor(Col(ERole::Solid)).Padding(FMargin(16.f, 14.f))
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 14.f, 0.f)
            [
                SNew(SBox).WidthOverride(40.f).HeightOverride(40.f)
                [
                    SNew(SBorder).BorderImage(&CircleBrush).HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.f)
                    .BorderBackgroundColor_Lambda([this, Pick] { return FSlateColor(Color(Pick().bWarn ? ERole::WarnSoft : ERole::AccentSoft)); })
                    [
                        SNew(STextBlock).Font(MarketTheme::Font(MarketTheme::EFace::DisplayBold, 18.f))
                        .Text_Lambda([Pick] { return FText::FromString(Pick().bWarn ? TEXT("!") : TEXT("i")); })
                        .ColorAndOpacity_Lambda([this, Pick] { return FSlateColor(Color(Pick().bWarn ? ERole::Warn : ERole::Accent)); })
                    ]
                ]
            ]
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ TextPx([Pick] { return Pick().Title; }, 14.f, [] { return ERole::Text; }, true, true) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)[ TextPx([Pick] { return Pick().Body; }, 12.f, [] { return ERole::Muted; }, false, true) ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(14.f, 0.f, 0.f, 0.f)
            [
                SNew(SBox).HeightOverride(36.f)
                [
                    SNew(SButton).ButtonStyle(&RowStyle).IsFocusable(false).ContentPadding(FMargin(14.f, 0.f)).VAlign(VAlign_Center)
                    .ButtonColorAndOpacity(Col(ERole::Primary))
                    .OnClicked_Lambda([this, G, Pick]
                    {
                        const FLine Line = Pick();
                        if (Line.Page != INDEX_NONE && Line.Page != Summary)
                        {
                            if (G() && G()->Products.IsValidIndex(Line.Product)) G()->MenuProduct = Line.Product;
                            Go(Line.Page);
                        }
                        else { bDecisionsOpen = true; bSettingsOpen = false; }
                        return FReply::Handled();
                    })
                    [ TextPx([] { return FString(TEXT("Bak")); }, 13.f, [] { return ERole::PrimaryText; }, true) ]
                ]
            ]
        ]
    ]);
}

TSharedRef<SWidget> SMarketMenu::ProvinceCard()
{
    // Board 5, the panel on the right: where, three numbers, our shops, the province manager, the four types.
    auto G = [this] { return Game.Get(); };
    auto City = [this]() -> const MarketCountry::FCity* { return MarketCountry::FindCity(ShownCountry(), PanelId); };
    auto Site = [this, G]() -> MarketBranches::FSite { return G() ? MarketBranches::SiteOf(G()->State, ShownCountry(), PanelId) : MarketBranches::FSite(); };
    auto Ours = [this, G] { return G() ? MarketMenuPagesUi::OurShops(G()->State, ShownCountry(), PanelId) : 0; };
    auto Short = [](int64 Kurus) { FString Money = MarketMenuUi::Tl(Kurus); Money.ReplaceInline(TEXT(",00"), TEXT("")); return Money; };
    auto Signed = [Short](int64 Kurus) { return (Kurus > 0 ? TEXT("+") : TEXT("")) + Short(Kurus); };
    auto Muted = [] { return ERole::Muted; };
    auto Plain = [] { return ERole::Text; };
    auto Hair = [this]() -> TSharedRef<SWidget> { return SNew(SBox).HeightOverride(1.f)[ SNew(SBorder).BorderImage(&FlatBrush).BorderBackgroundColor(Col(ERole::Hairline)) ]; };

    auto Tile = [this](const FString& Heading, TFunction<FString()> Value, TFunction<ERole()> Role, bool bMono) -> TSharedRef<SWidget>
    {
        TSharedRef<SWidget> Number = bMono ? Mono(Value, 17.f, Role) : TextPx(Value, 17.f, Role, true);
        return SNew(SBorder).BorderImage(&TileBrush).BorderBackgroundColor(Col(ERole::Panel)).Padding(FMargin(12.f))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ TextPx([Heading] { return Heading; }, 11.f, [] { return ERole::Muted; }) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)[ Number ]
        ];
    };
    TSharedRef<SHorizontalBox> Tiles = SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 10.f, 0.f)
        [ Tile(TEXT("N\u00fcfus"), [City]
        {
            const MarketCountry::FCity* C = City();
            if (!C) return FString();
            return C->PopulationK >= 1000 ? FString::Printf(TEXT("%.2f M"), C->PopulationK / 1000.f).Replace(TEXT("."), TEXT(",")) : FString::Printf(TEXT("%d bin"), C->PopulationK);
        }, Plain, true) ]
        + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 10.f, 0.f)
        [ Tile(TEXT("Kira"), [City] { const MarketCountry::FCity* C = City(); return C ? FString::Printf(TEXT("\u00d7%.2f"), C->Rent).Replace(TEXT("."), TEXT(",")) : FString(); },
            [City] { const MarketCountry::FCity* C = City(); return C && C->Rent < 0.9f ? ERole::Good : ERole::Text; }, true) ]
        + SHorizontalBox::Slot().FillWidth(1.f)
        [ Tile(TEXT("Rekabet"), [City] { const MarketCountry::FCity* C = City(); return !C ? FString() : C->Competition < 0.9f ? FString(TEXT("Az")) : C->Competition > 1.12f ? FString(TEXT("Y\u00fcksek")) : FString(TEXT("Orta")); },
            [City] { const MarketCountry::FCity* C = City(); return C && C->Competition < 0.9f ? ERole::Good : C && C->Competition > 1.12f ? ERole::Warn : ERole::Text; }, false) ];

    // Our shops: a grade square, the name, the manager and the last thirty days.
    auto GradeFill = [this](const FString& Grade) -> FLinearColor
    {
        using MarketMenuUi::Hex;
        const bool bLight = IsLight();
        if (Grade == TEXT("A")) return bLight ? Hex(TEXT("D9EFE6")) : Hex(TEXT("1F3A33"));
        if (Grade == TEXT("B")) return bLight ? Hex(TEXT("E6F0DA")) : Hex(TEXT("2A3522"));
        if (Grade == TEXT("C")) return bLight ? Hex(TEXT("F6ECD6")) : Hex(TEXT("3A2E17"));
        if (Grade == TEXT("D")) return bLight ? Hex(TEXT("F8E1DC")) : Hex(TEXT("3D2220"));
        return Color(ERole::Inset);
    };
    auto GradeInk = [this](const FString& Grade) -> FLinearColor
    {
        using MarketMenuUi::Hex;
        const bool bLight = IsLight();
        if (Grade == TEXT("A")) return bLight ? Hex(TEXT("1F6B55")) : Hex(TEXT("71C6AC"));
        if (Grade == TEXT("B")) return bLight ? Hex(TEXT("4E6B1F")) : Hex(TEXT("A8C98A"));
        if (Grade == TEXT("C")) return bLight ? Hex(TEXT("8A5A10")) : Hex(TEXT("E8A94E"));
        if (Grade == TEXT("D")) return bLight ? Hex(TEXT("A33B2B")) : Hex(TEXT("F07F6E"));
        return Color(ERole::Muted);
    };
    auto Row = [this, Hair](TFunction<FString()> Mark, TFunction<FLinearColor()> Fill, TFunction<FLinearColor()> Ink, TFunction<FString()> Name, TFunction<FString()> Note) -> TSharedRef<SWidget>
    {
        return SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ Hair() ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)
                [
                    SNew(SBox).WidthOverride(30.f).HeightOverride(30.f)
                    [
                        SNew(SBorder).BorderImage(&SmallBrush).HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.f)
                        .BorderBackgroundColor_Lambda([Fill] { return FSlateColor(Fill()); })
                        [
                            SNew(STextBlock).Font(MarketTheme::Font(MarketTheme::EFace::DisplayBold, 15.f))
                            .Text_Lambda([Mark] { return FText::FromString(Mark()); })
                            .ColorAndOpacity_Lambda([Ink] { return FSlateColor(Ink()); })
                        ]
                    ]
                ]
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ TextPx(Name, 14.f, [] { return ERole::Text; }, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ TextPx(Note, 12.f, [] { return ERole::Muted; }) ]
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [
                    SNew(SBox).ToolTip(Tip([] { return FString(TEXT("Ma\u011fazay\u0131 birinci \u015fah\u0131sla gezmek yak\u0131nda (G-087).")); }))
                    [ TextPx([] { return FString(TEXT("Gez")); }, 13.f, [] { return ERole::Muted; }, true) ]
                ]
            ];
    };
    TSharedRef<SVerticalBox> Shops = SNew(SVerticalBox);
    Shops->AddSlot().AutoHeight()
    [
        SNew(SBox).Visibility_Lambda([Site] { return Site().bHome ? EVisibility::Visible : EVisibility::Collapsed; })
        [ Row([] { return FString(TEXT("EV")); }, [this] { return Color(ERole::AccentSoft); }, [this] { return Color(ERole::Accent); },
            [G] { return G() ? MarketStart::FirstStoreName(G()->State) : FString(); }, [] { return FString(TEXT("\u0130lk \u015fuben \u00b7 buradas\u0131n")); }) ]
    ];
    for (int32 Slot = 0; Slot < 8; ++Slot)
    {
        auto Index = [this, G, Slot]() -> int32
        {
            if (!G()) return INDEX_NONE;
            const TArray<int32> List = MarketMenuPagesUi::BranchesIn(G()->State, ShownCountry(), PanelId);
            return List.IsValidIndex(Slot) ? List[Slot] : INDEX_NONE;
        };
        auto Grade = [G, Index] { const int32 I = Index(); return G() && I != INDEX_NONE ? MarketBranches::Grade(G()->State, I) : FString(); };
        Shops->AddSlot().AutoHeight()
        [
            SNew(SBox).Visibility_Lambda([Index] { return Index() != INDEX_NONE ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Row(Grade, [Grade, GradeFill] { return GradeFill(Grade()); }, [Grade, GradeInk] { return GradeInk(Grade()); },
                [G, Index] { const int32 I = Index(); return G() && I != INDEX_NONE ? G()->State.Branches[I].Name : FString(); },
                [G, Index, Signed]
                {
                    const int32 I = Index();
                    if (!G() || I == INDEX_NONE) return FString();
                    const FMarketBranch& B = G()->State.Branches[I];
                    const FString Boss = B.ManagerName.IsEmpty() ? FString(TEXT("M\u00fcd\u00fcr yok")) : TEXT("M\u00fcd\u00fcr: ") + B.ManagerName;
                    return FString::Printf(TEXT("%s \u00b7 %s"), *Boss, *Signed(B.Last30Profit));
                }) ]
        ];
    }
    // G-086b: the province manager (INDEX_NONE = none), our open branches without the first store, and whether the
    // manager's skill covers them.
    auto Chief = [this, G] { return G() ? MarketManagers::FindManager(G()->State, MarketManagers::ELevel::Province, ShownCountry(), PanelId) : INDEX_NONE; };
    auto Stores = [this, G] { return G() ? MarketManagers::ProvinceBranches(G()->State, ShownCountry(), PanelId) : 0; };
    auto Enough = [G, Chief]
    {
        const int32 Index = Chief();
        return G() && Index != INDEX_NONE && MarketManagers::EffectiveManagerSkill(G()->State, Index) >= MarketManagers::RequiredSkill(G()->State, Index);
    };
    auto Month = [this, G]() -> int64
    {
        int64 Sum = 0;
        if (G()) for (const int32 I : MarketMenuPagesUi::BranchesIn(G()->State, ShownCountry(), PanelId)) Sum += G()->State.Branches[I].Last30Profit;
        return Sum;
    };

    // The four market types; the one that suits the province is framed and marked "\u00f6neri".
    auto CanDo = [this, G](const FString& Format) { FString Reason; return G() && MarketBranches::CanOpen(G()->State, G()->Products, ShownCountry(), PanelId, Format, Reason); };
    auto TypeCard = [this, G, City, CanDo](const FString& Format, const FString& Tagline) -> TSharedRef<SWidget>
    {
        auto Suits = [City, CanDo, Format]
        {
            const MarketCountry::FCity* C = City();
            if (!C || !CanDo(Format)) return false;
            const TCHAR* Best = C->Income < 0.9f ? TEXT("kucuk") : C->Income > 1.15f && C->PopulationK >= 300 ? TEXT("buyuk") : TEXT("mahalle");
            return Format == Best;
        };
        auto Note = [this, G, Format, Tagline]
        {
            FString Reason;
            if (G() && !MarketBranches::CanOpen(G()->State, G()->Products, ShownCountry(), PanelId, Format, Reason)) return Reason;
            return FString::Printf(TEXT("%d ki\u015fi \u00b7 %s"), MarketBranches::FormatInfo(Format).Workers, *Tagline);
        };
        return SNew(SBorder).BorderImage(&TileBrush)
            .Padding_Lambda([Suits] { return FMargin(Suits() ? 2.f : 1.f); })
            .BorderBackgroundColor_Lambda([this, Suits] { return FSlateColor(Color(Suits() ? ERole::Accent : ERole::Line)); })
            .ColorAndOpacity_Lambda([CanDo, Format] { return FLinearColor(1.f, 1.f, 1.f, CanDo(Format) ? 1.f : 0.5f); })
        [
            SNew(SButton).ButtonStyle(&RowStyle).IsFocusable(false).ContentPadding(FMargin(12.f))
            .ButtonColorAndOpacity(Col(ERole::Panel))
            .IsEnabled_Lambda([CanDo, Format] { return CanDo(Format); })
            .OnClicked_Lambda([this, G, Format]
            {
                if (!G()) return FReply::Handled();
                const MarketBranches::FFormat& K = MarketBranches::FormatInfo(Format);
                const MarketBranches::FSite S = MarketBranches::SiteOf(G()->State, ShownCountry(), PanelId);
                const FString Country = ShownCountry(), Province = PanelId;
                Ask(FString::Printf(TEXT("%s'de %s a\u00e7\u0131ls\u0131n m\u0131? \u015eimdi %s \u00e7\u0131kar (depozito, tadilat, a\u00e7\u0131l\u0131\u015f sto\u011fu); %d \u00e7al\u0131\u015fanla ve bir m\u00fcd\u00fcrle a\u00e7\u0131l\u0131r. Tadilat %d g\u00fcn, ruhsat %d g\u00fcn s\u00fcrer. Sat\u0131\u015ftan \u00f6nce ayl\u0131k gideri yakla\u015f\u0131k %s (kira, \u00fccret, SGK, i\u015fletme): a\u00e7t\u0131ktan sonra kasada en az bu kadar kals\u0131n."),
                    *S.Name, K.Name, *MarketMenuUi::Tl(MarketBranches::OpeningCost(G()->State, G()->Products, Country, Province, Format)),
                    K.Workers, MarketBranches::RenovationDays, MarketBranches::PermitDays, *MarketMenuUi::Tl(MarketBranches::MonthlyFixedCost(G()->State, Country, Province, Format))),
                    [this, Country, Province, Format] { Manage(TEXT("OpenBranch"), MarketBranches::EncodeSite(Country, Province, Format)); });
                return FReply::Handled();
            })
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                    [ TextPx([Format] { return FString(MarketBranches::FormatInfo(Format).Short); }, 14.f, [] { return ERole::Text; }, true) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [
                        SNew(SBorder).BorderImage(&BadgeBrush).BorderBackgroundColor(Col(ERole::AccentSoft)).Padding(FMargin(7.f, 2.f))
                        .Visibility_Lambda([Suits] { return Suits() ? EVisibility::Visible : EVisibility::Collapsed; })
                        [ TextPx([] { return FString(TEXT("\u00f6neri")); }, 10.f, [] { return ERole::Accent; }, true) ]
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
                [ Mono([this, G, Format] { return G() ? MarketMenuUi::Tl(MarketBranches::OpeningCost(G()->State, G()->Products, ShownCountry(), PanelId, Format)) : FString(); }, 13.f, [] { return ERole::Text; }) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)[ TextPx(Note, 11.f, [] { return ERole::Muted; }, false, true) ]
            ]
        ];
    };
    TSharedRef<SUniformGridPanel> Types = SNew(SUniformGridPanel).SlotPadding(FMargin(5.f));
    Types->AddSlot(0, 0)[ TypeCard(TEXT("kucuk"), TEXT("dar raf, ucuz")) ];
    Types->AddSlot(1, 0)[ TypeCard(TEXT("mahalle"), TEXT("dengeli")) ];
    Types->AddSlot(0, 1)[ TypeCard(TEXT("buyuk"), TEXT("geni\u015f \u00fcr\u00fcn")) ];
    Types->AddSlot(1, 1)[ TypeCard(TEXT("hiper"), TEXT("b\u00fcy\u00fck il")) ];
    // M54: the country's own store types (convenience stores, cash-and-carry) where its market has them.
    Types->AddSlot(0, 2)[ SNew(SBox).Visibility_Lambda([this] { return MarketBranches::FormatsIn(ShownCountry()).Contains(TEXT("yakin")) ? EVisibility::Visible : EVisibility::Collapsed; })
        [ TypeCard(TEXT("yakin"), TEXT("k\u00fc\u00e7\u00fck, pahal\u0131, \u00e7ok \u015fube")) ] ];
    Types->AddSlot(1, 2)[ SNew(SBox).Visibility_Lambda([this] { return MarketBranches::FormatsIn(ShownCountry()).Contains(TEXT("toptan")) ? EVisibility::Visible : EVisibility::Collapsed; })
        [ TypeCard(TEXT("toptan"), TEXT("koli sat\u0131\u015f\u0131, b\u00fcy\u00fck il")) ] ];

    TSharedRef<SWidget> Body = SNew(SVerticalBox)
        // Where, and the way out.
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [ TextPx([this]
                {
                    const MarketCountry::FRegion* Sub = MarketCountry::SubRegionOf(ShownCountry(), PanelId);
                    const MarketCountry::FRegion* Main = MarketCountry::RegionOf(ShownCountry(), PanelId);
                    return Sub ? (Main && Main->Id != Sub->Id ? Sub->Name + TEXT(" \u00b7 ") + Main->Name : Sub->Name) : FString();
                }, 12.f, Muted) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)[ Display([City] { const MarketCountry::FCity* C = City(); return C ? C->Name : FString(); }, 30.f) ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)
            [
                SNew(SBox).WidthOverride(36.f).HeightOverride(36.f).ToolTip(Tip([] { return FString(TEXT("Kapat (Esc)")); }))
                [
                    SNew(SButton).ButtonStyle(&RoundStyle).IsFocusable(false).ContentPadding(FMargin(0.f)).HAlign(HAlign_Center).VAlign(VAlign_Center)
                    .ButtonColorAndOpacity(Col(ERole::Inset))
                    .OnClicked_Lambda([this] { MapProvinceId.Reset(); return FReply::Handled(); })
                    [ IconImage(TEXT("close"), 16.f, [] { return ERole::Text; }) ]
                ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 20.f, 0.f, 0.f)[ Tiles ]
        // M58: a country not entered yet needs a market study first (its status, the report, the button).
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            .Visibility_Lambda([this, G] { return G() && MarketResearch::Status(G()->State, ShownCountry()) != MarketResearch::EStatus::NotNeeded ? EVisibility::Visible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [ TextPx([this, G] { return G() ? MarketResearch::StatusText(G()->State, ShownCountry()) : FString(); }, 12.f, [] { return ERole::Text; }, false, true) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
            [ Button([] { return FString(TEXT("Pazar ara\u015ft\u0131rmas\u0131")); }, [this, G]
            {
                if (!G()) return;
                const FString Country = ShownCountry();
                Ask(FString::Printf(TEXT("%s i\u00e7in pazar ara\u015ft\u0131rmas\u0131 yapt\u0131r\u0131ls\u0131n m\u0131? %s, %d g\u00fcn s\u00fcrer; rapor bir y\u0131l ge\u00e7erli."),
                    *MarketCountry::FindOrDefault(Country).Name, *MarketMenuUi::Tl(MarketResearch::Cost(G()->State, Country)), MarketResearch::Days(G()->State, Country)),
                    [this, G, Country] { FString Message; if (G()) { MarketResearch::Start(G()->State, Country, Message); G()->Notify(Message); } });
            }, false, [this, G] { FString Why; return G() && MarketResearch::CanStart(G()->State, ShownCountry(), Why); }) ]
        ]
        // D6 (M67): a partner under our brand, the second way into a country (status and the two buttons).
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            .Visibility_Lambda([this, G] { return G() && ShownCountry() != G()->State.CountryId && MarketCompany::AbroadOpen(G()->State) ? EVisibility::Visible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [ TextPx([this, G] { return G() ? MarketFranchise::StatusText(G()->State, ShownCountry()) : FString(); }, 12.f, [] { return ERole::Text; }, false, true) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
            [
                SNew(SBox).Visibility_Lambda([this, G] { return G() && !MarketFranchise::Find(G()->State, ShownCountry()) ? EVisibility::Visible : EVisibility::Collapsed; })
                .ToolTip(Tip([this, G] { FString Why; return G() && !MarketFranchise::CanSign(G()->State, ShownCountry(), Why) ? Why : FString(TEXT("Yerel bir ortak ma\u011fazalar\u0131 bizim ad\u0131m\u0131zla a\u00e7ar; risk ve kazan\u00e7 k\u00fc\u00e7\u00fckt\u00fcr.")); }))
                [ Button([] { return FString(TEXT("Ortakl\u0131kla gir")); }, [this, G]
                {
                    if (!G()) return;
                    const FString Country = ShownCountry();
                    Ask(FString::Printf(TEXT("%s i\u00e7in yerel bir ortakla s\u00f6zle\u015fme imzalans\u0131n m\u0131? S\u00f6zle\u015fme ve marka tescili %s. Ortak ad\u0131m\u0131zla %d ma\u011fazayla ba\u015flar, sat\u0131\u015f\u0131n %%%.0f'\u00fc bize gelir; s\u00fcrerken orada kendi ma\u011fazan olmaz."),
                        *MarketCountry::FindOrDefault(Country).Name, *MarketMenuUi::Tl(MarketFranchise::Fee(G()->State, Country)), MarketFranchise::StartStores, 100.f * MarketFranchise::RoyaltyRate),
                        [this, G, Country] { FString Message; if (G()) { MarketFranchise::Sign(G()->State, Country, Message); G()->Notify(Message); } });
                }, false, [this, G] { FString Why; return G() && MarketFranchise::CanSign(G()->State, ShownCountry(), Why); }) ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
            [
                SNew(SBox).Visibility_Lambda([this, G] { return G() && MarketFranchise::Find(G()->State, ShownCountry()) ? EVisibility::Visible : EVisibility::Collapsed; })
                [ Button([] { return FString(TEXT("Ortakl\u0131\u011f\u0131 bitir")); }, [this, G]
                {
                    if (!G()) return;
                    const FString Country = ShownCountry();
                    Ask(FString::Printf(TEXT("Ortakl\u0131k bitsin mi? Tazminat %s (bir y\u0131ll\u0131k k\u00e2r pay\u0131). Ortak ma\u011fazalar\u0131n\u0131 kendi ad\u0131na \u00e7evirir; sonra orada kendi ma\u011fazan\u0131 a\u00e7abilirsin."),
                        *MarketMenuUi::Tl(MarketFranchise::EndCost(G()->State, Country))),
                        [this, G, Country] { FString Message; if (G()) { MarketFranchise::End(G()->State, Country, Message); G()->Notify(Message); } });
                }) ]
            ]
        ]
        // Our shops.
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 20.f, 0.f, 6.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Bottom)[ TextPx([Ours] { return FString::Printf(TEXT("Ma\u011fazalar\u0131m\u0131z \u00b7 %d"), Ours()); }, 13.f, Muted, true) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom)
            [
                SNew(SHorizontalBox).Visibility_Lambda([Month] { return Month() != 0 ? EVisibility::Visible : EVisibility::Collapsed; })
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ TextPx([] { return FString(TEXT("Son 30 g\u00fcn ")); }, 12.f, Muted) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Mono([Month, Signed] { return Signed(Month()); }, 12.f, [Month] { return Month() < 0 ? ERole::Bad : ERole::Accent; }) ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight()[ Shops ]
        + SVerticalBox::Slot().AutoHeight()
        [ SNew(SBox).Visibility_Lambda([Ours] { return Ours() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
            [ TextPx([] { return FString(TEXT("Bu ilde hen\u00fcz ma\u011fazan yok.")); }, 13.f, Muted) ] ]
        // Karar M05 / G-086b: the province manager strip. With a manager: who and whether the skill covers the
        // province; without one and with three shops (the first store does not count): "Ata" opens the appointment
        // in Magazalar > Yonetim. Fixed height (M15).
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 20.f, 0.f, 0.f)
        [
            SNew(SBox).HeightOverride(64.f).Visibility_Lambda([Chief, Stores] { return Chief() != INDEX_NONE || Stores() >= MarketManagers::ProvinceShops ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SBorder).BorderImage(&TileBrush).Padding(FMargin(14.f, 8.f)).VAlign(VAlign_Center)
                .BorderBackgroundColor_Lambda([this, Chief, Enough] { return FSlateColor(Color(Chief() != INDEX_NONE && Enough() ? ERole::AccentSoft : ERole::WarnSoft)); })
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()
                        [ TextPx([G, Chief, Stores]
                        {
                            const int32 Index = Chief();
                            if (G() && Index != INDEX_NONE) return FString::Printf(TEXT("\u0130l m\u00fcd\u00fcr\u00fc: %s"), *G()->State.Management.Managers[Index].Name);
                            return FString::Printf(TEXT("%d ma\u011faza oldu: bu il bir il m\u00fcd\u00fcr\u00fc istiyor."), Stores());
                        }, 13.f, Plain, true) ]
                        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
                        [ TextPx([G, Chief, Stores]
                        {
                            const int32 Index = Chief();
                            if (!G()) return FString();
                            const FMarketState& S = G()->State;
                            if (Index != INDEX_NONE)
                                return FString::Printf(TEXT("Beceri %d / gereken %d \u00b7 denetim %%%.0f \u00b7 ayda %s"), MarketManagers::EffectiveManagerSkill(S, Index),
                                    MarketManagers::RequiredSkill(S, Index), 100.f * MarketManagers::Strength(S, Index), *MarketMenuUi::Tl(MarketManagers::DailyWage(S, S.Management.Managers[Index]) * 30));
                            return FString::Printf(TEXT("Gereken beceri %d \u00b7 terfiyle ya da d\u0131\u015far\u0131dan"), MarketManagers::ProvinceRequiredSkill(Stores()));
                        }, 12.f, [Chief, Enough] { return Chief() != INDEX_NONE && !Enough() ? ERole::Warn : ERole::Muted; }) ]
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12.f, 0.f, 0.f, 0.f)
                    [
                        SNew(SBox).HeightOverride(32.f).ToolTip(Tip([G, Chief] { return G() && Chief() != INDEX_NONE ? MarketManagers::DescribeManager(G()->State, Chief())
                            : FString(TEXT("\u0130l m\u00fcd\u00fcr\u00fc ildeki b\u00fct\u00fcn ma\u011fazalara bakar: sipari\u015f hatalar\u0131 azal\u0131r, karneler y\u00fckselir, kasadan alan\u0131 yakalar, a\u00e7\u0131l\u0131\u015flar 2 g\u00fcn k\u0131sal\u0131r.")); }))
                        [
                            SNew(SButton).ButtonStyle(&RowStyle).IsFocusable(false).ContentPadding(FMargin(12.f, 0.f)).VAlign(VAlign_Center)
                            .ButtonColorAndOpacity(Col(ERole::Primary))
                            .OnClicked_Lambda([this, Chief]
                            {
                                if (Chief() != INDEX_NONE) { BranchTab = 2; AppointArea = INDEX_NONE; Go(Branches); }
                                else OpenAppointment(MarketManagers::EncodeArea(MarketManagers::ELevel::Province, ShownCountry(), PanelId));
                                return FReply::Handled();
                            })
                            [ TextPx([Chief] { return FString(Chief() != INDEX_NONE ? TEXT("Y\u00f6netim") : TEXT("\u0130l m\u00fcd\u00fcr\u00fc ata")); }, 13.f, [] { return ERole::PrimaryText; }, true) ]
                        ]
                    ]
                ]
            ]
        ]
        // G-089: the nearest depot of the country (a fixed line once the company has a depot).
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
        [
            SNew(SBox).HeightOverride(22.f).Visibility_Lambda([G] { return G() && G()->State.Company.DepotSites.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed; })
            .ToolTip(Tip([] { return FString(TEXT("Ma\u011faza mal\u0131n\u0131 \u00fclkesindeki en yak\u0131n depodan al\u0131r: 600 km'den uzaksa (ev ilinde 200 km) toptanc\u0131dan. \u0130lk 100 km bedava, sonra her 100 km +%0,6.")); }))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
                [
                    SNew(SBox).WidthOverride(16.f).HeightOverride(16.f)
                    [
                        SNew(SBorder).BorderImage(&BadgeBrush).HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.f).BorderBackgroundColor(Col(ERole::Info))
                        [ SNew(STextBlock).Font(MarketTheme::Font(MarketTheme::EFace::Bold, 10.f)).Text(FText::FromString(TEXT("D"))).ColorAndOpacity(Col(ERole::Solid)) ]
                    ]
                ]
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ TextPx([this, G] { return G() ? MarketMenuPagesUi::NearestDepotLine(G()->State, ShownCountry(), PanelId) : FString(); }, 12.f, Muted) ]
            ]
        ]
        // Open a shop.
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 20.f, 0.f, 5.f)
        [ TextPx([Site, Ours]
        {
            const int32 Free = FMath::Max(0, MarketBranches::Room(Site()) - Ours());
            return Free > 0 ? FString::Printf(TEXT("Yeni ma\u011faza a\u00e7 \u00b7 %d yer daha var"), Free) : FString(TEXT("Bu ilde yer kalmad\u0131"));
        }, 13.f, Muted, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(-5.f, 0.f)[ Types ];

    return SNew(SBorder).BorderImage(&CardBrush).BorderBackgroundColor(Col(ERole::Sheet)).Padding(FMargin(26.f, 24.f, 26.f, 22.f))
    [ SNew(SScrollBox) + SScrollBox::Slot()[ Body ] ];
}

// ---------------------------------------------------------------------------------------------------------------
// Magazalar: every shop by province and the company.

TSharedRef<SWidget> SMarketMenu::BranchesPage()
{
    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)[ Choice(TEXT("Ma\u011fazalar"), [this] { return BranchTab == 0; }, [this] { BranchTab = 0; }) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)[ Choice(TEXT("Y\u00f6netim"), [this] { return BranchTab == 2; }, [this] { BranchTab = 2; }) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)[ Choice(TEXT("\u015eirket"), [this] { return BranchTab == 1; }, [this] { BranchTab = 1; }) ]
            + SHorizontalBox::Slot().AutoWidth()[ Choice(TEXT("M\u00fcdahaleler"), [this] { return BranchTab == 3; }, [this] { BranchTab = 3; }) ]
        ]
        + SVerticalBox::Slot().FillHeight(1.f)
        [
            SNew(SWidgetSwitcher).WidgetIndex_Lambda([this] { return FMath::Clamp(BranchTab, 0, 3); })
            + SWidgetSwitcher::Slot()[ ShopsTab() ]
            + SWidgetSwitcher::Slot()[ CompanyTab() ]
            + SWidgetSwitcher::Slot()[ ManagementTab() ]
            + SWidgetSwitcher::Slot()[ ResponsesTab() ]
        ];
}

TSharedRef<SWidget> SMarketMenu::ShopsTab()
{
    auto G = [this] { return Game.Get(); };
    auto BranchOpen = [G](int32 Slot) { return G() && G()->State.Branches.IsValidIndex(Slot) && G()->State.Branches[Slot].Stage != static_cast<uint8>(MarketBranches::EStage::Closed); };

    auto HasBoss = [G](int32 Slot) { return G() && G()->State.Branches.IsValidIndex(Slot) && !G()->State.Branches[Slot].ManagerName.IsEmpty(); };
    auto ShopOpen = [G](int32 Slot) { return G() && G()->State.Branches.IsValidIndex(Slot) && G()->State.Branches[Slot].Stage == static_cast<uint8>(MarketBranches::EStage::Open); };
    // A button with a tooltip that says why it is off (or what it does).
    auto Hinted = [this](const TSharedRef<SWidget>& Inner, TFunction<FString()> Hint) -> TSharedRef<SWidget>
    {
        return SNew(SBox).VAlign(VAlign_Center).ToolTip(Tip(Hint))[ Inner ];
    };

    // G-089: every branch's depot link, once a frame for all the rows.
    struct FLinkCache
    {
        uint64 Frame = 0;
        bool bFilled = false;
        TArray<MarketDepots::FLink> Links;
    };
    TSharedRef<FLinkCache> LinkCache = MakeShared<FLinkCache>();
    auto Supply = [G, LinkCache](int32 Slot) -> FString
    {
        if (!G()) return FString();
        if (!LinkCache->bFilled || LinkCache->Frame != GFrameCounter)
        {
            LinkCache->bFilled = true;
            LinkCache->Frame = GFrameCounter;
            LinkCache->Links = MarketDepots::AllLinks(G()->State);
        }
        return MarketMenuPagesUi::SupplyShort(G()->State, Slot, LinkCache->Links);
    };

    // D9b (M46): why a store's works cannot start ("" they can) and the question with their price.
    auto CanWorks = [G](int32 Slot, MarketPortfolio::EWorks Kind, const FString& Format) -> FString
    {
        if (!G()) return FString();
        if (Kind == MarketPortfolio::EWorks::Reformat && Format.IsEmpty()) return FString(TEXT("Bu y\u00f6nde ba\u015fka t\u00fcr yok."));
        FString Reason;
        return MarketPortfolio::CanStart(G()->State, G()->Products, Slot, Kind, Format, Reason) ? FString() : Reason;
    };
    auto Works = [G](int32 Slot, MarketPortfolio::EWorks Kind, const FString& Format, const TCHAR* Ask) -> FString
    {
        if (!G() || !G()->State.Branches.IsValidIndex(Slot)) return FString();
        const FMarketBranch& B = G()->State.Branches[Slot];
        const FString Into = Kind == MarketPortfolio::EWorks::Reformat && !Format.IsEmpty() ? FString::Printf(TEXT(" (%s)"), MarketBranches::FormatInfo(Format).Name) : FString();
        return FString::Printf(TEXT("%s%s %s? Masraf %s (kapal\u0131 g\u00fcnlerin kiras\u0131 ve maa\u015flar\u0131 dahil), %d g\u00fcn kapal\u0131 kal\u0131r."), *B.Name, *Into, Ask,
            *MarketMenuUi::Tl(MarketPortfolio::WorksCost(G()->State, G()->Products, Slot, Kind, Format)),
            MarketPortfolio::WorksDays(Kind == MarketPortfolio::EWorks::Reformat && !Format.IsEmpty() ? Format : B.Format));
    };
    TSharedRef<SVerticalBox> Shops = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 80; ++Slot)
    {
        auto Grade = [G, Slot] { return G() && G()->State.Branches.IsValidIndex(Slot) ? MarketBranches::Grade(G()->State, Slot) : FString(); };
        auto BranchName = [G, Slot] { return G() && G()->State.Branches.IsValidIndex(Slot) ? G()->State.Branches[Slot].Name : FString(); };
        auto BossName = [G, Slot] { return G() && G()->State.Branches.IsValidIndex(Slot) ? G()->State.Branches[Slot].ManagerName : FString(); };
        auto Wage = [G, Slot]() -> int64 { return G() && G()->State.Branches.IsValidIndex(Slot) ? G()->State.Branches[Slot].ManagerWage : 0; };
        // G-086b decisions on the store manager (MarketDirector: ManagerBonus / ManagerWarn / ManagerReplace / PromoteToProvince).
        auto BonusReady = [G, Slot, HasBoss, ShopOpen]
        {
            if (!HasBoss(Slot) || !ShopOpen(Slot)) return false;
            const FMarketBranch& B = G()->State.Branches[Slot];
            return B.ManagerBonusDay <= 0 || G()->State.Day - B.ManagerBonusDay >= MarketManagers::BonusCooldown;
        };
        auto WarnReady = [G, Slot, HasBoss, ShopOpen]
        {
            if (!HasBoss(Slot) || !ShopOpen(Slot)) return false;
            const FMarketBranch& B = G()->State.Branches[Slot];
            return B.ManagerWarnedDay <= 0 || G()->State.Day - B.ManagerWarnedDay >= 7;
        };
        auto PromoteWhy = [G, Slot]() -> FString
        {
            if (!G() || !G()->State.Branches.IsValidIndex(Slot)) return FString();
            const FMarketState& S = G()->State;
            FString Reason;
            MarketManagers::CanAppoint(S, MarketManagers::ELevel::Province, MarketBranches::CountryOf(S, S.Branches[Slot]),
                MarketManagers::AreaOfBranch(S, Slot, MarketManagers::ELevel::Province), Slot, Reason);
            return Reason;
        };
        Shops->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(ColBy([this, Slot] { return PromoteBranch == Slot || PickBranch == Slot ? ERole::Line : ERole::Inset; })).Padding(FMargin(12.f, 8.f))
            .Visibility_Lambda([BranchOpen, Slot] { return BranchOpen(Slot) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
                    [
                        SNew(SBox).WidthOverride(26.f).HeightOverride(26.f)
                        [
                            SNew(SBorder).BorderImage(&BadgeBrush).HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.f)
                            .BorderBackgroundColor_Lambda([this, Grade] { return FSlateColor(Color(MarketMenuPagesUi::GradeRole(Grade())) * FLinearColor(1.f, 1.f, 1.f, 0.25f)); })
                            [ LabelBy(Grade, 11, [Grade] { return MarketMenuPagesUi::GradeRole(Grade()); }, true) ]
                        ]
                    ]
                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                    [ Label([G, Slot] { return G() && G()->State.Branches.IsValidIndex(Slot) ? MarketMenuPagesUi::BranchLine(G()->State, Slot, G()->Products) : FString(); }, 10, ERole::Text, false, true) ]
                    // G-089: where the goods come from ("Depo: Tekirda\u011f \u00b7 120 km"); the details in the tooltip.
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                    [
                        SNew(SBox).Visibility_Lambda([ShopOpen, Slot] { return ShopOpen(Slot) ? EVisibility::Visible : EVisibility::Collapsed; })
                        .ToolTip(Tip([G, Slot] { return G() && G()->State.Branches.IsValidIndex(Slot) ? MarketStoreViews::Describe(G()->State.Branches[Slot]) + TEXT("\n") + MarketDepots::DescribeLink(G()->State, Slot) : FString(); }))
                        [ Mono([Supply, Slot] { return Supply(Slot); }, 11.f, [] { return ERole::Muted; }) ]
                    ]
                    // C3 (A4): walk through the branch; what you see comes from its numbers, the economy waits.
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                    [ Button([] { return FString(TEXT("Gez")); }, [G, Slot] { if (G()) G()->StartBranchVisit(Slot); }, false, [ShopOpen, Slot] { return ShopOpen(Slot); }) ]
                    // M38: this store's own campaign: the campaign page with the branch chosen.
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                    [ Button([] { return FString(TEXT("Kampanya")); }, [this, Slot] { Manage(TEXT("PromoStore"), Slot); Go(SMarketMenu::Promotions); }, false, [ShopOpen, Slot] { return ShopOpen(Slot); }) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                    [ RiskyButton([] { return FString(TEXT("Kapat")); },
                        [G, Slot] { return G() && G()->State.Branches.IsValidIndex(Slot)
                            ? FString::Printf(TEXT("%s kapans\u0131n m\u0131? Raftaki mal ilk ma\u011fazan\u0131n deposuna ta\u015f\u0131n\u0131r, s\u0131\u011fmayan yar\u0131 fiyat\u0131na sat\u0131l\u0131r; \u00e7al\u0131\u015fanlar ayr\u0131l\u0131r."), *G()->State.Branches[Slot].Name) : FString(); },
                        [this, Slot] { if (PromoteBranch == Slot) PromoteBranch = INDEX_NONE; Manage(TEXT("CloseBranch"), Slot); },
                        [BranchOpen, Slot] { return BranchOpen(Slot); }) ]
                ]
                // The store manager (G-086b): a line of fixed height, so the row never jumps.
                + SVerticalBox::Slot().AutoHeight().Padding(36.f, 6.f, 0.f, 0.f)
                [
                    // C3 (A istek 3): the manager's line on a row of its own, so a long karne note is never cut by the buttons.
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        SNew(SBox).ToolTip(Tip([G, Slot] { return G() ? MarketManagers::DescribeBranchManager(G()->State, Slot) : FString(); }))
                        [ LabelBy([G, Slot] { return G() ? MarketMenuPagesUi::ManagerLine(G()->State, Slot) : FString(); }, 10,
                            [G, Slot] { return G() ? MarketMenuPagesUi::ManagerRole(G()->State, Slot) : ERole::Muted; }) ]
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
                    [
                    SNew(SBox).HeightOverride(30.f)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().FillWidth(1.f)[ SNew(SSpacer) ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f, 0.f, 0.f)
                        [ Hinted(Button([] { return FString(TEXT("D\u00fckk\u00e2ndan")); }, [this, Slot] { PickBranch = INDEX_NONE; PromoteBranch = PromoteBranch == Slot ? INDEX_NONE : Slot; }, false, [BranchOpen, Slot] { return BranchOpen(Slot); }),
                            [] { return FString(TEXT("\u0130lk ma\u011fazadan bir kasiyeri ya da reyon g\u00f6revlisini bu \u015fubeye m\u00fcd\u00fcr yap.")); }) ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 0.f, 0.f)
                        [ Hinted(RiskyButton([] { return FString(TEXT("Prim")); },
                            [BossName, Wage] { return FString::Printf(TEXT("%s prim als\u0131n m\u0131? Bir haftal\u0131k \u00fccret: %s. Morali y\u00fckselir, becerisi biraz artar. %d g\u00fcnde bir verilir."),
                                *BossName(), *MarketMenuUi::Tl(Wage() * MarketManagers::BonusDays), MarketManagers::BonusCooldown); },
                            [this, Slot] { Manage(TEXT("ManagerBonus"), Slot); }, BonusReady),
                            [BonusReady, HasBoss, Slot] { return BonusReady() ? FString(TEXT("Bir haftal\u0131k \u00fccret kadar prim: moral +15, beceri +1.")) : HasBoss(Slot) ? FString(TEXT("Yak\u0131n zamanda prim ald\u0131 (14 g\u00fcnde bir).")) : FString(TEXT("\u015eubenin m\u00fcd\u00fcr\u00fc yok.")); }) ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 0.f, 0.f)
                        [ Hinted(RiskyButton([] { return FString(TEXT("Uyar")); },
                            [BossName] { return FString::Printf(TEXT("%s uyar\u0131ls\u0131n m\u0131? Hak ettiyse toparlan\u0131r ya da kasadan elini \u00e7eker; hak etmediyse morali d\u00fc\u015fer."), *BossName()); },
                            [this, Slot] { Manage(TEXT("ManagerWarn"), Slot); }, WarnReady),
                            [WarnReady, HasBoss, Slot] { return WarnReady() ? FString(TEXT("K\u00f6t\u00fc karne ya da kasa a\u00e7\u0131\u011f\u0131 varsa uyar\u0131 i\u015fe yarar; haks\u0131z uyar\u0131 d\u00fcr\u00fcst m\u00fcd\u00fcr\u00fc k\u0131rar.")) : HasBoss(Slot) ? FString(TEXT("Bu hafta zaten uyar\u0131ld\u0131.")) : FString(TEXT("\u015eubenin m\u00fcd\u00fcr\u00fc yok.")); }) ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 0.f, 0.f)
                        [ Hinted(Button([HasBoss, Slot] { return FString(HasBoss(Slot) ? TEXT("De\u011fi\u015ftir") : TEXT("M\u00fcd\u00fcr al")); },
                            [this, Slot] { PromoteBranch = INDEX_NONE; PickBranch = PickBranch == Slot ? INDEX_NONE : Slot; }, false, [ShopOpen, Slot] { return ShopOpen(Slot); }),
                            [] { return FString(TEXT("3 d\u0131\u015f aday aras\u0131ndan se\u00e7 (adaylar her hafta yenilenir). \u0130K m\u00fcd\u00fcr\u00fc varsa becerilerini tam, d\u00fcr\u00fcstl\u00fcklerini ve tarzlar\u0131n\u0131 g\u00f6r\u00fcrs\u00fcn.")); }) ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 0.f, 0.f)
                        [
                            SNew(SBox).Visibility_Lambda([G, Slot] { return G() && MarketMenuPagesUi::ProvinceWantsManager(G()->State, Slot) ? EVisibility::Visible : EVisibility::Collapsed; })
                            [ Hinted(RiskyButton([] { return FString(TEXT("\u0130l m\u00fcd\u00fcr\u00fc yap")); },
                                [BossName, BranchName] { return FString::Printf(TEXT("%s il m\u00fcd\u00fcr\u00fc olsun mu? \u0130ldeki b\u00fct\u00fcn ma\u011fazalara bakar ve il m\u00fcd\u00fcr\u00fc \u00fccreti al\u0131r; %s \u015fubesine d\u0131\u015far\u0131dan yeni bir m\u00fcd\u00fcr gelir (bir hafta al\u0131\u015f\u0131r)."),
                                    *BossName(), *BranchName()); },
                                [this, Slot] { if (PromoteBranch == Slot) PromoteBranch = INDEX_NONE; Manage(TEXT("PromoteToProvince"), Slot); },
                                [PromoteWhy] { return PromoteWhy().IsEmpty(); }),
                                [PromoteWhy] { const FString Reason = PromoteWhy(); return Reason.IsEmpty() ? FString(TEXT("\u0130yi bir ma\u011faza m\u00fcd\u00fcr\u00fc ili tan\u0131r: terfi eden ailenin g\u00fcvendi\u011fi biridir.")) : Reason; }) ]
                        ]
                    ]
                ]
                    ]
                // D9b (M46): the store's age, its yearly card and the works (renew, relocate, a larger or smaller type).
                + SVerticalBox::Slot().AutoHeight().Padding(36.f, 4.f, 0.f, 0.f)
                [
                    SNew(SBox).HeightOverride(30.f).Visibility_Lambda([BranchOpen, Slot] { return BranchOpen(Slot) ? EVisibility::Visible : EVisibility::Collapsed; })
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                        [ LabelBy([G, Slot] { return G() ? MarketPortfolio::Line(G()->State, Slot) : FString(); }, 10,
                            [G, Slot] { return G() && G()->State.Branches.IsValidIndex(Slot) && !MarketPortfolio::Advice(G()->State, Slot).IsEmpty() ? ERole::Warn : ERole::Muted; }) ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 0.f, 0.f)
                        [ Hinted(RiskyButton([] { return FString(TEXT("Yenile")); },
                            [Works, Slot] { return Works(Slot, MarketPortfolio::EWorks::Renovate, FString(), TEXT("yenilensin mi")); },
                            [this, Slot] { Manage(TEXT("RenovateBranch"), Slot); },
                            [CanWorks, Slot] { return CanWorks(Slot, MarketPortfolio::EWorks::Renovate, FString()).IsEmpty(); }),
                            [CanWorks, Slot] { const FString Why = CanWorks(Slot, MarketPortfolio::EWorks::Renovate, FString()); return Why.IsEmpty() ? FString(TEXT("Ya\u015f\u0131n\u0131 s\u0131f\u0131rlar, ilk y\u0131l m\u00fc\u015fteri %4 fazla gelir. 2-3 hafta kapal\u0131 kal\u0131r.")) : Why; }) ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 0.f, 0.f)
                        [ Hinted(RiskyButton([] { return FString(TEXT("Ta\u015f\u0131")); },
                            [Works, Slot] { return Works(Slot, MarketPortfolio::EWorks::Relocate, FString(), TEXT("ayn\u0131 ilde ba\u015fka bir yere ta\u015f\u0131ns\u0131n m\u0131")); },
                            [this, Slot] { Manage(TEXT("RelocateBranch"), Slot); },
                            [CanWorks, Slot] { return CanWorks(Slot, MarketPortfolio::EWorks::Relocate, FString()).IsEmpty(); }),
                            [CanWorks, Slot] { const FString Why = CanWorks(Slot, MarketPortfolio::EWorks::Relocate, FString()); return Why.IsEmpty() ? FString(TEXT("Ayn\u0131 ilde daha iyi bir yere: aceleyle se\u00e7ilmi\u015f zay\u0131f yer d\u00fczelir, ya\u015f\u0131 s\u0131f\u0131rlan\u0131r; m\u00fc\u015fterinin bir k\u0131sm\u0131 yeni yeri sonradan \u00f6\u011frenir.")) : Why; }) ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 0.f, 0.f)
                        [ Hinted(RiskyButton([] { return FString(TEXT("B\u00fcy\u00fct")); },
                            [G, Works, Slot] { return G() ? Works(Slot, MarketPortfolio::EWorks::Reformat, MarketPortfolio::Larger(G()->State, Slot), TEXT("b\u00fcy\u00fcs\u00fcn m\u00fc")) : FString(); },
                            [this, G, Slot] { if (G()) Manage(TEXT("ReformatBranch"), MarketPortfolio::EncodeFormat(Slot, MarketPortfolio::Larger(G()->State, Slot))); },
                            [G, CanWorks, Slot] { return G() && CanWorks(Slot, MarketPortfolio::EWorks::Reformat, MarketPortfolio::Larger(G()->State, Slot)).IsEmpty(); }),
                            [G, CanWorks, Slot] { if (!G()) return FString(); const FString Why = CanWorks(Slot, MarketPortfolio::EWorks::Reformat, MarketPortfolio::Larger(G()->State, Slot)); return Why.IsEmpty() ? FString(TEXT("Bir b\u00fcy\u00fck t\u00fcre \u00e7evir: yeni raflar, daha \u00e7ok \u00e7al\u0131\u015fan, yeni kira.")) : Why; }) ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 0.f, 0.f)
                        [ Hinted(RiskyButton([] { return FString(TEXT("K\u00fc\u00e7\u00fclt")); },
                            [G, Works, Slot] { return G() ? Works(Slot, MarketPortfolio::EWorks::Reformat, MarketPortfolio::Smaller(G()->State, Slot), TEXT("k\u00fc\u00e7\u00fcls\u00fcn m\u00fc")) : FString(); },
                            [this, G, Slot] { if (G()) Manage(TEXT("ReformatBranch"), MarketPortfolio::EncodeFormat(Slot, MarketPortfolio::Smaller(G()->State, Slot))); },
                            [G, CanWorks, Slot] { return G() && CanWorks(Slot, MarketPortfolio::EWorks::Reformat, MarketPortfolio::Smaller(G()->State, Slot)).IsEmpty(); }),
                            [G, CanWorks, Slot] { if (!G()) return FString(); const FString Why = CanWorks(Slot, MarketPortfolio::EWorks::Reformat, MarketPortfolio::Smaller(G()->State, Slot)); return Why.IsEmpty() ? FString(TEXT("Bir k\u00fc\u00e7\u00fck t\u00fcre \u00e7evir: kira ve \u00e7al\u0131\u015fan azal\u0131r; zay\u0131f bir ilde zarar\u0131 durdurabilir.")) : Why; }) ]
                    ]
                ]
            ]
        ];
    }

    // Manager picker for the branch chosen above: cashiers and stockers of the first store.
    auto PersonAt = [G](int32 Slot) -> const FMarketEmployee*
    {
        if (!G() || !G()->State.Staff.IsValidIndex(Slot)) return nullptr;
        const MarketStaff::ERole Role = MarketStaff::RoleOf(G()->State.Staff[Slot]);
        return Role == MarketStaff::ERole::Cashier || Role == MarketStaff::ERole::Stocker ? &G()->State.Staff[Slot] : nullptr;
    };
    TSharedRef<SVerticalBox> Candidates = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 10; ++Slot)
    {
        Candidates->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SHorizontalBox).Visibility_Lambda([PersonAt, Slot] { return PersonAt(Slot) ? EVisibility::Visible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [ Label([G, PersonAt, Slot] { const FMarketEmployee* E = PersonAt(Slot); return E && G() ? MarketStaff::DescribeEmployee(G()->State, *E) : FString(); }, 10, ERole::Text, false, true) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
            [ RiskyButton([] { return FString(TEXT("M\u00fcd\u00fcr yap")); },
                [G, PersonAt, Slot, this]
                {
                    const FMarketEmployee* E = PersonAt(Slot);
                    if (!E || !G() || !G()->State.Branches.IsValidIndex(PromoteBranch)) return FString();
                    return FString::Printf(TEXT("%s, %s m\u00fcd\u00fcr\u00fc olsun mu? D\u00fckk\u00e2ndan ayr\u0131l\u0131r; \u00fccreti %%40 artar. Becerisi sipari\u015fleri, d\u00fcr\u00fcstl\u00fc\u011f\u00fc kasay\u0131 belirler."), *E->Name, *G()->State.Branches[PromoteBranch].Name);
                },
                [this, PersonAt, Slot]
                {
                    const FMarketEmployee* E = PersonAt(Slot);
                    if (!E || PromoteBranch == INDEX_NONE) return;
                    const int32 Arg = PromoteBranch * 1000000 + E->Id;
                    PromoteBranch = INDEX_NONE;
                    Manage(TEXT("PromoteTo"), Arg);
                },
                [PersonAt, Slot] { return PersonAt(Slot) != nullptr; }) ]
        ];
    }
    TSharedRef<SWidget> Picker = SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [ Label([G, this] { return G() && G()->State.Branches.IsValidIndex(PromoteBranch) ? FString::Printf(TEXT("%s \u0130\u00c7\u0130N M\u00dcD\u00dcR \u00b7 A\u0130LE D\u00dcKK\u00c2NINDAN"), *G()->State.Branches[PromoteBranch].Name.ToUpper()) : FString(); }, 9, ERole::Muted, true) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)
                [ Button([] { return FString(TEXT("D\u0131\u015f adaylar \u203a")); }, [this] { PickBranch = PromoteBranch; PromoteBranch = INDEX_NONE; }) ]
                + SHorizontalBox::Slot().AutoWidth()[ Button([] { return FString(TEXT("Vazge\u00e7")); }, [this] { PromoteBranch = INDEX_NONE; }) ]
            ]
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SBox).Visibility_Lambda([PersonAt] { for (int32 K = 0; K < 10; ++K) if (PersonAt(K)) return EVisibility::Collapsed; return EVisibility::Visible; })
                [ Fixed(TEXT("D\u00fckk\u00e2nda kasiyer ya da reyon g\u00f6revlisi yok. \u00d6nce Personel sayfas\u0131ndan birini i\u015fe al."), 10, ERole::Warn) ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[ Candidates ];

    // G-086b ek (M22): the store manager of a branch from 3 outside candidates ("De\u011fi\u015ftir" / "M\u00fcd\u00fcr al").
    TSharedRef<SWidget> BranchPicker = SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [ Label([G, this] { return G() && G()->State.Branches.IsValidIndex(PickBranch) ? FString::Printf(TEXT("%s \u0130\u00c7\u0130N MA\u011eAZA M\u00dcD\u00dcR\u00dc"), *G()->State.Branches[PickBranch].Name.ToUpper()) : FString(); }, 9, ERole::Muted, true) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)
            [ Button([] { return FString(TEXT("D\u00fckk\u00e2ndan biri \u203a")); }, [this] { PromoteBranch = PickBranch; PickBranch = INDEX_NONE; }) ]
            + SHorizontalBox::Slot().AutoWidth()[ Button([] { return FString(TEXT("Vazge\u00e7")); }, [this] { PickBranch = INDEX_NONE; }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 10.f)
        [ Label([G, this]
        {
            if (!G() || !G()->State.Branches.IsValidIndex(PickBranch)) return FString();
            const FMarketBranch& B = G()->State.Branches[PickBranch];
            return B.ManagerName.IsEmpty() ? FString(TEXT("\u015eubenin m\u00fcd\u00fcr\u00fc yok. Se\u00e7ti\u011fin aday ilk hafta al\u0131\u015f\u0131r."))
                : FString::Printf(TEXT("\u015eimdiki: %s. De\u011fi\u015firse %d g\u00fcnl\u00fck \u00fccret tazminat al\u0131r (%s)."), *MarketMenuPagesUi::ManagerLine(G()->State, PickBranch),
                    MarketManagers::SeveranceDays, *MarketMenuUi::Tl(B.ManagerWage * MarketManagers::SeveranceDays));
        }, 10, ERole::Text, false, true) ]
        + SVerticalBox::Slot().AutoHeight()[ CandidateCards([this] { return PickBranch != INDEX_NONE ? -2 - PickBranch : INDEX_NONE; }, [this] { PickBranch = INDEX_NONE; }) ];

    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1.4f).Padding(0.f, 0.f, 12.f, 0.f)
        [
            SNew(SScrollBox)
            + SScrollBox::Slot()
            [
                // One card, three layers (M15): the shops, a branch's candidates, the first store's people.
                SNew(SWidgetSwitcher).WidgetIndex_Lambda([this, BranchOpen] { return BranchOpen(PickBranch) ? 1 : BranchOpen(PromoteBranch) ? 2 : 0; })
                + SWidgetSwitcher::Slot()[ Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("MA\u011eAZALARIN")) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 4.f)
                    [
                        SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 10.f))
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot().AutoHeight()[ Label([G] { return G() ? MarketStart::PlaceText(G()->State) : FString(); }, 13, ERole::Text, true) ]
                            + SVerticalBox::Slot().AutoHeight()[ Label([G] { return G() ? MarketStart::FirstStoreName(G()->State) + TEXT(" \u00b7 ilk \u015fuben \u00b7 buradas\u0131n") : FString(); }, 10, ERole::Muted) ]
                        ]
                    ]
                    + SVerticalBox::Slot().AutoHeight()[ Shops ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
                    [ Label([G] { return G() && G()->State.Branches.Num() > 0 ? FString::Printf(TEXT("\u015eubelerin ve merkezin d\u00fcnk\u00fc toplam katk\u0131s\u0131 %s. Yeni ma\u011faza: Harita'da bir il se\u00e7."), *MarketMenuUi::Tl(G()->State.LastBranchProfit))
                        : FString(TEXT("Hen\u00fcz \u015fube yok. Yeni ma\u011faza: Harita'da bir il se\u00e7.")); }, 10, ERole::Muted, false, true) ]) ]
                + SWidgetSwitcher::Slot()[ LayerFade(Card(BranchPicker, ERole::Solid)) ]
                + SWidgetSwitcher::Slot()[ LayerFade(Card(Picker, ERole::Solid)) ]
            ]
        ]
        + SHorizontalBox::Slot().FillWidth(1.f)
        [
            SNew(SScrollBox)
            + SScrollBox::Slot().Padding(0.f, 0.f, 0.f, 12.f)[ SpanCounter(true) ]
            + SScrollBox::Slot()
            [
                // C5 (A menu list): the first branch's conditions only until a branch is open (as in the decisions layer).
                SNew(SBox).Padding(FMargin(0.f, 0.f, 0.f, 12.f))
                .Visibility_Lambda([G] { return G() && MarketBranches::OpenCount(G()->State) == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
                [
                    Card(SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[ Section(TEXT("\u0130LK \u015eUBE \u0130\u00c7\u0130N")) ]
                        + SVerticalBox::Slot().AutoHeight()[ GoalList() ])
                ]
            ]
            + SScrollBox::Slot()
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Section(TEXT("KARNE NASIL VER\u0130L\u0130R")) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ Label([] { return FString(TEXT("Her hafta: raf dolulu\u011fu (%40), m\u00fc\u015fteri memnuniyeti (%30) ve son 30 g\u00fcn\u00fcn k\u00e2r\u0131 (%30). A en iyi, D en k\u00f6t\u00fc. \u0130lk hafta karne yok.")); }, 10, ERole::Text, false, true) ])
            ]
        ];
}

// ---------------------------------------------------------------------------------------------------------------
// G-086b Magazalar > Yonetim: the player's span of control, the tree of levels and the appointments
// (Docs/Kurgu/03_MAGAZA_AGI.md \u00a74, \u00a710). Decisions: MarketDirector AppointOutside / AppointPromote /
// BonusManager / WarnManager / DismissManager.

void SMarketMenu::OpenAppointment(int32 Area)
{
    AppointArea = Area;
    BranchTab = 2;
    Go(Branches);
}

TSharedRef<SWidget> SMarketMenu::SpanCounter(bool bOpensManagement)
{
    auto G = [this] { return Game.Get(); };
    auto Direct = [G] { return G() ? MarketManagers::DirectCount(G()->State) : 0; };
    auto Role = [Direct]
    {
        const int32 N = Direct();
        return N > MarketManagers::SpanLimit ? ERole::Bad : N == MarketManagers::SpanLimit ? ERole::Warn : ERole::Text;
    };
    auto Status = [G, Direct]() -> FString
    {
        const int32 N = Direct();
        if (N > MarketManagers::SpanLimit)
            return FString::Printf(TEXT("%d ki\u015fi fazla: sana ba\u011fl\u0131 herkesin becerisi -%d, kasadan \u00e7alan g\u00f6r\u00fcnmez"), N - MarketManagers::SpanLimit, G() ? MarketManagers::SpanPenalty(G()->State) : 0);
        if (N == MarketManagers::SpanLimit) return TEXT("S\u0131n\u0131rdas\u0131n: yeni ma\u011fazadan \u00f6nce bir \u00fcst kademe d\u00fc\u015f\u00fcn");
        return FString::Printf(TEXT("%d ki\u015filik yerin var"), MarketManagers::SpanLimit - N);
    };
    auto StatusRole = [Direct]
    {
        const int32 N = Direct();
        return N > MarketManagers::SpanLimit ? ERole::Bad : N == MarketManagers::SpanLimit ? ERole::Warn : ERole::Muted;
    };
    auto Rules = [G]() -> FString
    {
        FString Text;
        if (G())
        {
            Text = MarketManagers::SpanText(G()->State);
            TArray<FString> Names;
            for (const MarketManagers::FPerson& Person : MarketManagers::DirectReports(G()->State)) Names.Add(FString::Printf(TEXT("%s (%s)"), *Person.Name, *Person.Title));
            if (Names.Num() > 0) Text += TEXT("\n\nSana ba\u011fl\u0131: ") + FString::Join(Names, TEXT(", ")) + TEXT(".");
            Text += TEXT("\n\n");
        }
        Text += TEXT("En \u00e7ok 5 ki\u015fiyle do\u011frudan ilgilenebilirsin; \u0130K m\u00fcd\u00fcr\u00fc bu say\u0131y\u0131 art\u0131rmaz. \u00dcst kademesi atanmam\u0131\u015f herkes sana ba\u011fl\u0131d\u0131r: il m\u00fcd\u00fcr\u00fc olmayan ildeki ma\u011faza m\u00fcd\u00fcrleri, b\u00f6lge m\u00fcd\u00fcr\u00fc olmayan il m\u00fcd\u00fcrleri, \u00fclke m\u00fcd\u00fcr\u00fc yoksa b\u00f6lge direkt\u00f6rleri. \u0130lk ma\u011faza m\u00fcd\u00fcr atanmad\u0131k\u00e7a say\u0131lmaz; depo m\u00fcd\u00fcrleri \u00fclke m\u00fcd\u00fcr\u00fc yoksa sana ba\u011fl\u0131d\u0131r.")
            TEXT("\n\n5'i a\u015f\u0131nca fazladan her ki\u015fi, sana do\u011frudan ba\u011fl\u0131 herkesin becerisini 4 puan d\u00fc\u015f\u00fcr\u00fcr (en \u00e7ok 25). Kasadan \u00e7alan g\u00f6r\u00fcnmez olur, memnuniyet yava\u015f yava\u015f d\u00fc\u015fer.")
            TEXT("\n\n\u00c7\u00f6z\u00fcm: il m\u00fcd\u00fcr\u00fc (ilde 3 ma\u011faza), b\u00f6lge m\u00fcd\u00fcr\u00fc (alt b\u00f6lgede 2 il m\u00fcd\u00fcr\u00fc), b\u00f6lge direkt\u00f6r\u00fc (ana b\u00f6lgede 2 b\u00f6lge m\u00fcd\u00fcr\u00fc) ya da \u00fclke m\u00fcd\u00fcr\u00fc ata.");
        return Text;
    };
    TSharedRef<SHorizontalBox> Line = SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 14.f, 0.f)
        [ Mono([Direct] { return FString::Printf(TEXT("%d / %d"), Direct(), MarketManagers::SpanLimit); }, 26.f, Role) ]
        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
        [ TextPx(Status, 12.f, StatusRole, true, true) ];
    if (bOpensManagement)
        Line->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
        [ Button([] { return FString(TEXT("Y\u00f6netim \u203a")); }, [this] { BranchTab = 2; }) ];
    return Card(SNew(SBox).HeightOverride(70.f)
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Section(TEXT("SANA DO\u011eRUDAN BA\u011eLI")) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ More(Rules) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[ Line ]
    ]);
}

TSharedRef<SWidget> SMarketMenu::CandidateCards(TFunction<int32()> Target, TFunction<void()> OnDone)
{
    // G-086b ek (M22): three outside candidates side by side, cards of one fixed height (M15). Without an HR manager
    // the skill is a range and honesty and style stay unknown; the potential shows as a hint.
    using MarketManagers::ELevel;
    using MarketManagers::FCandidate;
    auto G = [this] { return Game.Get(); };
    struct FPoolCache
    {
        uint64 Frame = 0;
        bool bFilled = false;
        int32 Key = INDEX_NONE;
        TArray<FCandidate> Pool;
    };
    TSharedRef<FPoolCache> Cache = MakeShared<FPoolCache>();
    auto Pool = [G, Target, Cache]() -> const TArray<FCandidate>&
    {
        const int32 Key = Target ? Target() : INDEX_NONE;
        if (!Cache->bFilled || Cache->Frame != GFrameCounter || Cache->Key != Key)
        {
            Cache->bFilled = true;
            Cache->Frame = GFrameCounter;
            Cache->Key = Key;
            Cache->Pool.Reset();
            if (G() && Key >= 0)
            {
                ELevel Level = ELevel::Province;
                FString Country, Area;
                if (MarketManagers::DecodeArea(Key, Level, Country, Area)) Cache->Pool = MarketManagers::Candidates(G()->State, Level, Country, Area);
            }
            else if (G() && Key <= -2) Cache->Pool = MarketManagers::BranchCandidates(G()->State, -2 - Key);
        }
        return Cache->Pool;
    };
    auto Blocked = [G, Target]() -> FString
    {
        const int32 Key = Target ? Target() : INDEX_NONE;
        if (!G()) return FString();
        const FMarketState& S = G()->State;
        if (Key >= 0)
        {
            ELevel Level = ELevel::Province;
            FString Country, Area, Reason;
            if (!MarketManagers::DecodeArea(Key, Level, Country, Area)) return FString(TEXT("B\u00f6yle bir kademe yok."));
            MarketManagers::CanAppoint(S, Level, Country, Area, INDEX_NONE, Reason);
            return Reason;
        }
        if (Key <= -2)
        {
            const int32 Branch = -2 - Key;
            if (!S.Branches.IsValidIndex(Branch) || S.Branches[Branch].Stage != static_cast<uint8>(MarketBranches::EStage::Open))
                return FString(TEXT("\u015eube hen\u00fcz a\u00e7\u0131k de\u011fil: m\u00fcd\u00fcr a\u00e7\u0131l\u0131\u015fta gelir."));
        }
        return FString();
    };

    TSharedRef<SUniformGridPanel> Row = SNew(SUniformGridPanel).SlotPadding(FMargin(5.f, 0.f));
    for (int32 K = 0; K < MarketManagers::CandidateCount; ++K)
    {
        auto Who = [Pool, K]() -> const FCandidate*
        {
            const TArray<FCandidate>& List = Pool();
            return List.IsValidIndex(K) ? &List[K] : nullptr;
        };
        auto SkillText = [G, Who]() -> FString
        {
            const FCandidate* C = Who();
            if (!C || !G()) return FString();
            int32 Low = 0, High = 0;
            MarketManagers::SkillRange(G()->State, *C, Low, High);
            return Low == High ? FString::Printf(TEXT("beceri %d"), Low) : FString::Printf(TEXT("beceri %d\u2013%d"), Low, High);
        };
        auto Character = [G, Who]() -> FString
        {
            const FCandidate* C = Who();
            if (!C || !G()) return FString();
            if (!MarketManagers::HonestyVisible(G()->State)) return FString(TEXT("d\u00fcr\u00fcstl\u00fck ve tarz: \u0130K g\u00f6r\u00fcr"));
            return FString::Printf(TEXT("d\u00fcr\u00fcstl\u00fck %d \u00b7 %s"), C->Honesty, *MarketManagers::StyleName(C->Style));
        };
        auto CharacterRole = [G, Who]
        {
            const FCandidate* C = Who();
            if (!C || !G() || !MarketManagers::HonestyVisible(G()->State)) return ERole::Muted;
            return C->Honesty < 50 ? ERole::Bad : ERole::Text;
        };
        auto Growth = [Who]() -> FString
        {
            const FCandidate* C = Who();
            return C ? MarketManagers::PotentialHint(C->Skill, C->Potential) : FString();
        };
        auto GrowthRole = [Who]
        {
            const FCandidate* C = Who();
            return C && FMath::Min(C->Potential, MarketManagers::SkillTop) - C->Skill >= 15 ? ERole::Accent : ERole::Muted;
        };
        auto Pick = [this, G, Target, Who, K, OnDone]
        {
            const FCandidate* C = Who();
            const int32 Key = Target ? Target() : INDEX_NONE;
            if (!C || !G() || Key == INDEX_NONE) return;
            const FMarketState& S = G()->State;
            FString Question;
            FName Action;
            int32 Arg = INDEX_NONE;
            if (Key >= 0)
            {
                ELevel Level = ELevel::Province;
                FString Country, Area;
                if (!MarketManagers::DecodeArea(Key, Level, Country, Area)) return;
                const FString Post = Level == ELevel::FirstStore || Level == ELevel::Chief ? MarketManagers::LevelName(Level)
                    : Level == ELevel::Depot ? FString::Printf(TEXT("%s deposunun m\u00fcd\u00fcr\u00fc"), *MarketManagers::AreaName(ELevel::Province, Country, Area))
                    : MarketManagers::AreaName(Level, Country, Area) + TEXT(" ") + MarketManagers::LevelName(Level);
                Question = FString::Printf(TEXT("%s, %s olsun mu? Ayl\u0131k \u00fccreti %s; g\u00f6revden almak %d g\u00fcnl\u00fck tazminat ister."),
                    *C->Name, *Post, *MarketMenuUi::Tl(C->Wage * 30), MarketManagers::SeveranceDays);
                Action = TEXT("AppointCandidate");
                Arg = Key * 10 + K;
            }
            else
            {
                const int32 Branch = -2 - Key;
                if (!S.Branches.IsValidIndex(Branch)) return;
                const FMarketBranch& B = S.Branches[Branch];
                if (B.ManagerName.IsEmpty())
                {
                    Question = FString::Printf(TEXT("%s, %s m\u00fcd\u00fcr\u00fc olsun mu? Ayl\u0131k \u00fccreti %s; ilk hafta al\u0131\u015f\u0131r, becerisi tam i\u015flemez."), *C->Name, *B.Name, *MarketMenuUi::Tl(C->Wage * 30));
                    Action = TEXT("ManagerHireFor");
                }
                else
                {
                    Question = FString::Printf(TEXT("%s gitsin, yerine %s gelsin mi? Tazminat %s (%d g\u00fcnl\u00fck \u00fccret); yeni m\u00fcd\u00fcr\u00fcn ayl\u0131\u011f\u0131 %s, bir hafta al\u0131\u015f\u0131r."),
                        *B.ManagerName, *C->Name, *MarketMenuUi::Tl(B.ManagerWage * MarketManagers::SeveranceDays), MarketManagers::SeveranceDays, *MarketMenuUi::Tl(C->Wage * 30));
                    Action = TEXT("ManagerReplaceWith");
                }
                Arg = Branch * 10 + K;
            }
            Ask(Question, [this, Action, Arg, OnDone] { if (OnDone) OnDone(); Manage(Action, Arg); });
        };
        Row->AddSlot(K, 0)
        [
            SNew(SBox).HeightOverride(150.f).Visibility_Lambda([Who] { return Who() ? EVisibility::Visible : EVisibility::Hidden; })
            [
                SNew(SBorder).BorderImage(&TileBrush).BorderBackgroundColor(Col(ERole::Panel)).Padding(FMargin(12.f, 10.f))
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ TextPx([Who] { const FCandidate* C = Who(); return C ? C->Name : FString(); }, 14.f, [] { return ERole::Text; }, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)[ Mono(SkillText, 13.f, [] { return ERole::Text; }) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)[ TextPx(Character, 11.f, CharacterRole) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)[ TextPx(Growth, 11.f, GrowthRole) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
                    [ Mono([Who] { const FCandidate* C = Who(); return C ? FString::Printf(TEXT("ayda %s"), *MarketMenuUi::Tl(C->Wage * 30)) : FString(); }, 12.f, [] { return ERole::Muted; }) ]
                    + SVerticalBox::Slot().FillHeight(1.f)[ SNew(SSpacer) ]
                    + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left)
                    [
                        SNew(SBox).ToolTip(Tip([Blocked] { const FString Reason = Blocked(); return Reason.IsEmpty() ? FString(TEXT("Bu aday\u0131 se\u00e7 (\u00f6nce sorar).")) : Reason; }))
                        [ Button([] { return FString(TEXT("Se\u00e7")); }, Pick, true, [Who, Blocked] { return Who() != nullptr && Blocked().IsEmpty(); }) ]
                    ]
                ]
            ]
        ];
    }
    auto Note = [G]() -> FString
    {
        if (!G()) return FString();
        const int32 Left = 7 - FMath::Max(0, G()->State.Day - 1) % 7;
        FString Text = FString::Printf(TEXT("Adaylar her hafta yenilenir (yeni adaylar %d g\u00fcn sonra)."), Left);
        if (!MarketManagers::HonestyVisible(G()->State)) Text += TEXT(" \u0130K m\u00fcd\u00fcr\u00fcn yok: beceri aral\u0131k olarak g\u00f6r\u00fcn\u00fcr, d\u00fcr\u00fcstl\u00fck ve tarz bilinmez.");
        return Text;
    };
    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(-5.f, 0.f)[ Row ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[ SNew(SBox).HeightOverride(30.f).VAlign(VAlign_Top)[ Label(Note, 9, ERole::Muted, false, true) ] ]
        + SVerticalBox::Slot().AutoHeight()[ Why(Blocked) ];
}

// D9b (M47): what the managers and the player answered to rivals' moves and crises, province by province.
TSharedRef<SWidget> SMarketMenu::ResponsesTab()
{
    auto G = [this] { return Game.Get(); };
    TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < MarketResponse::LogSize; ++Slot)
    {
        Rows->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 7.f))
            .Visibility_Lambda([G, Slot] { return G() && Slot < MarketResponse::LogCount(G()->State) ? EVisibility::Visible : EVisibility::Collapsed; })
            [ LabelBy([G, Slot] { return G() ? MarketResponse::LogLine(G()->State, Slot) : FString(); }, 10,
                [G, Slot] { return G() && MarketResponse::LogActive(G()->State, Slot) ? ERole::Text : ERole::Muted; }) ]
        ];
    }
    TSharedRef<SVerticalBox> Body = SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Label([] { return FString(TEXT("\u0130L KARARLARI VE M\u00dcDAHALELER")); }, 9, ERole::Muted, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 10.f)
        [ Label([G] { return G() ? MarketResponse::Summary(G()->State) : FString(); }, 10, ERole::Text, false, true) ]
        + SVerticalBox::Slot().AutoHeight()[ Rows ];
    return SNew(SBorder).BorderImage(&CardBrush).BorderBackgroundColor(Col(ERole::Sheet)).Padding(FMargin(26.f, 24.f, 26.f, 22.f))
    [ SNew(SScrollBox) + SScrollBox::Slot()[ Body ] ];
}

TSharedRef<SWidget> SMarketMenu::ManagementTab()
{
    using MarketManagers::ELevel;
    using MarketMenuPagesUi::FTierRow;
    auto G = [this] { return Game.Get(); };

    // The tree is built once a frame (every row's lambdas read it).
    struct FTreeCache
    {
        uint64 Frame = 0;
        bool bFilled = false;
        TArray<FTierRow> Rows;
    };
    TSharedRef<FTreeCache> Cache = MakeShared<FTreeCache>();
    auto RowAt = [G, Cache](int32 Index) -> const FTierRow*
    {
        if (!Cache->bFilled || Cache->Frame != GFrameCounter)
        {
            Cache->bFilled = true;
            Cache->Frame = GFrameCounter;
            Cache->Rows = G() ? MarketMenuPagesUi::TierRows(G()->State) : TArray<FTierRow>();
        }
        return Cache->Rows.IsValidIndex(Index) ? &Cache->Rows[Index] : nullptr;
    };
    auto Blurb = [](ELevel Tier) -> FString
    {
        switch (Tier)
        {
        case ELevel::Province: return TEXT("\u0130l m\u00fcd\u00fcr\u00fc ildeki b\u00fct\u00fcn ma\u011fazalara bakar: sipari\u015f hatalar\u0131 azal\u0131r, karneler y\u00fckselir, kasadan alan\u0131 yakalar, o ilde a\u00e7\u0131l\u0131\u015f 2 g\u00fcn k\u0131sal\u0131r. Gereken beceri 40 + ma\u011faza say\u0131s\u0131 / 3; yetmezse yar\u0131m \u00e7al\u0131\u015f\u0131r.");
        case ELevel::SubRegion: return TEXT("B\u00f6lge m\u00fcd\u00fcr\u00fc alt b\u00f6lgedeki il m\u00fcd\u00fcrlerini denetler; uzak \u015fubelerde lojistik kayb\u0131n\u0131 %1'e kadar azalt\u0131r.");
        case ELevel::Region: return TEXT("B\u00f6lge direkt\u00f6r\u00fc ana b\u00f6lgedeki b\u00f6lge m\u00fcd\u00fcrlerini denetler; depolar\u0131n ortak plan\u0131yla +%0,5 marj getirir.");
        case ELevel::Country: return TEXT("\u00dclke m\u00fcd\u00fcr\u00fc a\u011f\u0131n tepesinde durur: o \u00fclkede sana ba\u011fl\u0131 ki\u015fi say\u0131s\u0131n\u0131 tek ba\u015f\u0131na 1'e indirir, ama pahal\u0131d\u0131r; maa\u015f\u0131 a\u011f b\u00fcy\u00fcd\u00fck\u00e7e artar (az ma\u011fazada band\u0131n %30'u, 30 ma\u011fazada tam\u0131). \u00dclkede 5 ilde ma\u011fazan olunca atanabilir (ilk ma\u011fazan\u0131n ili dahil). \u015eirket ikinci \u00fclkeye girince her \u00fclkede zorunlu; yurt d\u0131\u015f\u0131nda maliyeti %1 d\u00fc\u015f\u00fcr\u00fcr.");
        case ELevel::FirstStore: return TEXT("\u0130lk ma\u011fazan\u0131n m\u00fcd\u00fcr\u00fc senin yerine sipari\u015f verir, zamm\u0131 rafa yans\u0131t\u0131r, raflar\u0131 doldurtur. Tarz\u0131 ve becerisi dolulu\u011fu ve fireyi belirler. Sana ba\u011fl\u0131 5 ki\u015fiden biri say\u0131l\u0131r.");
        case ELevel::Continent: return TEXT("K\u0131ta direkt\u00f6r\u00fc k\u0131tadaki \u00fclke m\u00fcd\u00fcrlerini y\u00f6netir: onlar art\u0131k sana de\u011fil ona ba\u011fl\u0131d\u0131r, i\u015fleri onun g\u00fcc\u00fc kadar iyi gider; k\u0131tadaki ma\u011fazalar\u0131n maliyeti %0,5'e kadar d\u00fc\u015fer (ortak al\u0131m). \u015eirket 3 \u00fclkeye girince atanabilir; birden \u00e7ok \u00fclken olan k\u0131tada zorunludur (yokken o \u00fclkelerin m\u00fcd\u00fcrleri 8 beceri kaybeder). \u0130lk iki \u00fclkenin m\u00fcd\u00fcrleri sana ba\u011fl\u0131 kalabilir.");
        case ELevel::Chief: return TEXT("Genel m\u00fcd\u00fcr iste\u011fe ba\u011fl\u0131d\u0131r: k\u0131ta direkt\u00f6rleri ona ba\u011flan\u0131r, sana do\u011frudan ba\u011fl\u0131 ki\u015fi say\u0131s\u0131 d\u00fc\u015fer, b\u00fct\u00fcn ma\u011fazalar\u0131n maliyeti %0,3'e kadar azal\u0131r. En pahal\u0131 y\u00f6neticidir. 2 k\u0131ta direkt\u00f6r\u00fc atand\u0131ktan sonra gelebilir.");
        case ELevel::Depot: return TEXT("Depo m\u00fcd\u00fcr\u00fc deponun verimini belirler: becerisi fireyi, eksik/k\u0131r\u0131k teslimat\u0131 ve raf bulunurlu\u011funu etkiler. M\u00fcd\u00fcrs\u00fcz depo yar\u0131 verimle \u00e7al\u0131\u015f\u0131r. \u00dclke m\u00fcd\u00fcr\u00fcne, o yoksa sana ba\u011fl\u0131d\u0131r.");
        default: return FString();
        }
    };

    // ---- The tree of levels.
    TSharedRef<SVerticalBox> Tree = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 64; ++Slot)
    {
        auto Row = [RowAt, Slot] { return RowAt(Slot); };
        // 0 = an appointed manager, 1 = an empty level that can be filled, 2 = the store managers of a province,
        // 3 = the first store while it is locked (M19).
        auto Kind = [Row]() -> int32
        {
            const FTierRow* R = Row();
            if (!R) return INDEX_NONE;
            return R->Tier == ELevel::Store ? 2 : R->Manager != INDEX_NONE ? 0 : !R->Lock.IsEmpty() ? 3 : 1;
        };
        auto Person = [G, Row]() -> const FMarketManager*
        {
            const FTierRow* R = Row();
            return R && G() && G()->State.Management.Managers.IsValidIndex(R->Manager) ? &G()->State.Management.Managers[R->Manager] : nullptr;
        };
        auto Where = [Row]() -> FString
        {
            const FTierRow* R = Row();
            if (!R) return FString();
            if (R->Tier == ELevel::FirstStore) return FString::Printf(TEXT("\u0130lk ma\u011faza (%s)"), *MarketManagers::AreaName(ELevel::Province, R->Country, R->Area));
            if (R->Tier == ELevel::Depot) return MarketManagers::AreaName(ELevel::Province, R->Country, R->Area) + TEXT(" deposu");
            if (R->Tier == ELevel::Chief) return TEXT("Genel m\u00fcd\u00fcr");
            if (R->Tier == ELevel::Continent) return MarketManagers::AreaName(R->Tier, R->Country, R->Area) + TEXT(" k\u0131ta direkt\u00f6r\u00fc");
            return MarketManagers::AreaName(R->Tier == ELevel::Store ? ELevel::Province : R->Tier, R->Country, R->Area);
        };
        auto MainText = [G, Row, Kind, Person, Where]() -> FString
        {
            const FTierRow* R = Row();
            if (!R || !G()) return FString();
            const int32 What = Kind();
            if (What == 0) return Person() ? FString::Printf(TEXT("%s \u00b7 %s"), *Person()->Name, *Where()) : FString();
            if (What == 1) return R->bRequired ? FString::Printf(TEXT("%s \u00b7 bo\u015f (zorunlu)"), *Where())
                : R->Tier == ELevel::Depot ? FString::Printf(TEXT("%s \u00b7 bo\u015f (yar\u0131 verim)"), *Where()) : FString::Printf(TEXT("%s \u00b7 bo\u015f"), *Where());
            if (What == 3) return FString::Printf(TEXT("%s \u00b7 kilitli"), *Where());
            MarketManagers::FPerson Shop;
            Shop.Level = ELevel::Store;
            Shop.Branch = R->FirstBranch;
            const int32 Boss = MarketManagers::BossOf(G()->State, Shop);
            const FString Above = G()->State.Management.Managers.IsValidIndex(Boss) ? G()->State.Management.Managers[Boss].Name : FString(TEXT("sana"));
            return FString::Printf(TEXT("%s \u00b7 %d ma\u011faza m\u00fcd\u00fcr\u00fc \u00b7 ba\u011fl\u0131: %s"), *Where(), R->Stores, *Above);
        };
        auto MainRole = [G, Row, Kind]
        {
            const FTierRow* R = Row();
            if (!R || !G()) return ERole::Muted;
            if (Kind() == 0) return MarketManagers::Strength(G()->State, R->Manager) < 0.5f ? ERole::Warn : ERole::Text;
            if (Kind() == 1) return R->bRequired ? ERole::Bad : R->Tier == ELevel::Depot ? ERole::Warn : ERole::Muted;
            return ERole::Muted;
        };
        auto Numbers = [G, Row, Kind, Person]() -> FString
        {
            const FTierRow* R = Row();
            if (!R || !G()) return FString();
            const FMarketState& S = G()->State;
            if (Kind() == 0 && Person())
                return FString::Printf(TEXT("beceri %d/%d \u00b7 ayda %s"), MarketManagers::EffectiveManagerSkill(S, R->Manager), MarketManagers::RequiredSkill(S, R->Manager),
                    *MarketMenuUi::Tl(MarketManagers::DailyWage(S, *Person()) * 30));
            if (Kind() == 1 && R->Tier == ELevel::Province)
                return FString::Printf(TEXT("gereken %d"), MarketManagers::ProvinceRequiredSkill(MarketManagers::ProvinceBranches(S, R->Country, R->Area)));
            return FString();
        };
        auto Hint = [G, Row, Kind, Blurb]() -> FString
        {
            const FTierRow* R = Row();
            if (!R || !G()) return FString();
            if (Kind() == 0) return MarketManagers::DescribeManager(G()->State, R->Manager);
            if (Kind() == 3) return R->Lock + TEXT("\n\n") + Blurb(R->Tier);
            if (Kind() == 1 && (R->Tier == ELevel::Continent || R->Tier == ELevel::Chief)) return TEXT("Ata: 3 d\u0131\u015f aday aras\u0131ndan se\u00e7.\n\n") + Blurb(R->Tier);
            if (Kind() == 1 && R->Tier == ELevel::FirstStore) return TEXT("M\u00fcd\u00fcr ata: 3 d\u0131\u015f aday aras\u0131ndan se\u00e7.\n\n") + Blurb(R->Tier);
            if (Kind() == 1) return TEXT("Ata: 3 d\u0131\u015f aday aras\u0131ndan se\u00e7 ya da bir ma\u011faza m\u00fcd\u00fcr\u00fcn\u00fc terfi ettir.\n\n") + Blurb(R->Tier);
            return TEXT("Ma\u011faza m\u00fcd\u00fcrlerine Ma\u011fazalar sekmesinden prim ver, uyar ya da de\u011fi\u015ftir.");
        };
        auto BonusOk = [G, Person]
        {
            const FMarketManager* M = Person();
            return M && G() && (M->BonusDay <= 0 || G()->State.Day - M->BonusDay >= MarketManagers::BonusCooldown);
        };
        auto WarnOk = [G, Person]
        {
            const FMarketManager* M = Person();
            return M && G() && (M->WarnedDay <= 0 || G()->State.Day - M->WarnedDay >= 7);
        };
        // The manager index is taken when the button is pressed; the question comes after.
        auto AskOn = [this, G, Row](FName Action, TFunction<FString(const FMarketManager&, int64)> Question, int32 Days)
        {
            const FTierRow* R = Row();
            if (!R || !G() || !G()->State.Management.Managers.IsValidIndex(R->Manager)) return;
            const int32 Index = R->Manager;
            const FMarketManager& M = G()->State.Management.Managers[Index];
            Ask(Question(M, MarketManagers::DailyWage(G()->State, M) * Days), [this, Action, Index] { Manage(Action, Index); });
        };

        TSharedRef<SHorizontalBox> Buttons = SNew(SHorizontalBox);
        Buttons->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 0.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([Kind] { return Kind() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
            .ToolTip(Tip([BonusOk] { return BonusOk() ? FString(TEXT("Bir haftal\u0131k \u00fccret kadar prim: moral +15, beceri +1.")) : FString(TEXT("Yak\u0131n zamanda prim ald\u0131 (14 g\u00fcnde bir).")); }))
            [ Button([] { return FString(TEXT("Prim")); }, [AskOn]
                {
                    AskOn(TEXT("BonusManager"), [](const FMarketManager& M, int64 Cost)
                        { return FString::Printf(TEXT("%s prim als\u0131n m\u0131? Bir haftal\u0131k \u00fccret: %s. Morali y\u00fckselir, becerisi biraz artar."), *M.Name, *MarketMenuUi::Tl(Cost)); }, MarketManagers::BonusDays);
                }, false, BonusOk) ]
        ];
        Buttons->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 0.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([Kind] { return Kind() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
            .ToolTip(Tip([WarnOk] { return WarnOk() ? FString(TEXT("Denetimi zay\u0131fsa ya da d\u00fcr\u00fcst de\u011filse uyar\u0131 i\u015fe yarar; haks\u0131z uyar\u0131 morali d\u00fc\u015f\u00fcr\u00fcr.")) : FString(TEXT("Bu hafta zaten uyar\u0131ld\u0131.")); }))
            [ Button([] { return FString(TEXT("Uyar")); }, [AskOn]
                {
                    AskOn(TEXT("WarnManager"), [](const FMarketManager& M, int64)
                        { return FString::Printf(TEXT("%s uyar\u0131ls\u0131n m\u0131? Hak ettiyse i\u015fine daha s\u0131k\u0131 bakar; hak etmediyse morali d\u00fc\u015fer."), *M.Name); }, 0);
                }, false, WarnOk) ]
        ];
        Buttons->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 0.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([Kind] { return Kind() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Button([] { return FString(TEXT("G\u00f6revden al")); }, [AskOn]
                {
                    AskOn(TEXT("DismissManager"), [](const FMarketManager& M, int64 Cost)
                        { return FString::Printf(TEXT("%s g\u00f6revden al\u0131ns\u0131n m\u0131? Tazminat %s (%d g\u00fcnl\u00fck \u00fccret). Alt\u0131ndakiler bir \u00fcstteki ki\u015fiye, o da yoksa sana ba\u011flan\u0131r."), *M.Name, *MarketMenuUi::Tl(Cost), MarketManagers::SeveranceDays); },
                        MarketManagers::SeveranceDays);
                }) ]
        ];
        Buttons->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 0.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([Kind] { return Kind() == 1 || Kind() == 3 ? EVisibility::Visible : EVisibility::Collapsed; })
            .ToolTip(Tip(Hint))
            [ Button([Row] { const FTierRow* R = Row(); return FString(R && R->Tier == ELevel::FirstStore ? TEXT("M\u00fcd\u00fcr ata") : TEXT("Ata")); }, [this, Row]
                {
                    if (const FTierRow* R = Row()) AppointArea = MarketManagers::EncodeArea(R->Tier, R->Country, R->Area);
                }, true, [Kind] { return Kind() == 1; }) ]
        ];
        Buttons->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 0.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([Kind] { return Kind() == 2 ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Button([] { return FString(TEXT("Ma\u011fazalar")); }, [this] { BranchTab = 0; }) ]
        ];

        Tree->AddSlot().AutoHeight()
        [
            SNew(SBox).HeightOverride(40.f).Visibility_Lambda([Row] { return Row() ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().FillHeight(1.f)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth()
                    [ SNew(SBox).WidthOverride_Lambda([Row]() -> FOptionalSize { const FTierRow* R = Row(); return FOptionalSize(R ? 18.f * R->Depth : 0.f); }) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [ SNew(SBox).WidthOverride(118.f)[ Label([Row] { const FTierRow* R = Row(); return R ? MarketMenuPagesUi::LevelTitle(R->Tier) : FString(); }, 9, ERole::Muted, true) ] ]
                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                    [ SNew(SBox).ToolTip(Tip(Hint))[ LabelBy(MainText, 11, MainRole, true) ] ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                    [ Mono(Numbers, 12.f, MainRole) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f, 0.f, 0.f)
                    [ Buttons ]
                ]
                + SVerticalBox::Slot().AutoHeight()
                [ SNew(SBox).HeightOverride(1.f)[ SNew(SBorder).BorderImage(&FlatBrush).BorderBackgroundColor(Col(ERole::Hairline)) ] ]
            ]
        ];
    }

    // ---- The appointment of one area: the outside candidate and the store managers who could be promoted.
    struct FTarget
    {
        bool bValid = false;
        ELevel Tier = ELevel::Province;
        FString Country;
        FString Area;
    };
    auto Target = [this]() -> FTarget
    {
        FTarget T;
        if (AppointArea != INDEX_NONE) T.bValid = MarketManagers::DecodeArea(AppointArea, T.Tier, T.Country, T.Area);
        return T;
    };
    TSharedRef<SVerticalBox> Promotable = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 12; ++Slot)
    {
        auto BranchAt = [G, Target, Slot]() -> int32
        {
            const FTarget T = Target();
            if (!T.bValid || !G()) return INDEX_NONE;
            const TArray<int32> List = MarketMenuPagesUi::PromotionCandidates(G()->State, T.Tier, T.Country, T.Area);
            return List.IsValidIndex(Slot) ? List[Slot] : INDEX_NONE;
        };
        auto Blocked = [G, Target, BranchAt]() -> FString
        {
            const FTarget T = Target();
            const int32 Branch = BranchAt();
            if (!T.bValid || !G() || Branch == INDEX_NONE) return FString();
            FString Reason;
            MarketManagers::CanAppoint(G()->State, T.Tier, T.Country, T.Area, Branch, Reason);
            return Reason;
        };
        Promotable->AddSlot().AutoHeight()
        [
            SNew(SBox).HeightOverride(36.f).Visibility_Lambda([BranchAt] { return BranchAt() != INDEX_NONE ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [
                    SNew(SBox).ToolTip(Tip([G, BranchAt] { const int32 Branch = BranchAt(); return G() && Branch != INDEX_NONE ? MarketManagers::DescribeBranchManager(G()->State, Branch) : FString(); }))
                    [ Label([G, BranchAt]
                    {
                        const int32 Branch = BranchAt();
                        if (!G() || Branch == INDEX_NONE) return FString();
                        const FMarketBranch& B = G()->State.Branches[Branch];
                        return FString::Printf(TEXT("%s \u00b7 %s \u00b7 karne %s"), *MarketMenuPagesUi::ManagerLine(G()->State, Branch), *B.Name, *MarketBranches::Grade(G()->State, Branch));
                    }, 10, ERole::Text) ]
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                [
                    SNew(SBox).ToolTip(Tip([Blocked] { const FString Reason = Blocked(); return Reason.IsEmpty() ? FString(TEXT("Terfi eden m\u00fcd\u00fcr a\u011f\u0131 tan\u0131r; \u015fubesine d\u0131\u015far\u0131dan yeni bir m\u00fcd\u00fcr gelir.")) : Reason; }))
                    [ Button([] { return FString(TEXT("Terfi ettir")); }, [this, G, Target, BranchAt]
                        {
                            const FTarget T = Target();
                            const int32 Branch = BranchAt();
                            if (!T.bValid || !G() || Branch == INDEX_NONE) return;
                            const FMarketBranch& B = G()->State.Branches[Branch];
                            const int32 Arg = Branch * 10 + static_cast<int32>(T.Tier);
                            Ask(FString::Printf(TEXT("%s, %s %s olsun mu? \u00dccreti yeni kademeye \u00e7\u0131kar; %s \u015fubesine d\u0131\u015far\u0131dan bir m\u00fcd\u00fcr gelir (bir hafta al\u0131\u015f\u0131r)."),
                                *B.ManagerName, *MarketManagers::AreaName(T.Tier, T.Country, T.Area), *MarketManagers::LevelName(T.Tier), *B.Name),
                                [this, Arg] { AppointArea = INDEX_NONE; Manage(TEXT("AppointPromote"), Arg); });
                        }, false, [Blocked] { return Blocked().IsEmpty(); }) ]
                ]
            ]
        ];
    }
    auto NoPromotion = [G, Target]
    {
        const FTarget T = Target();
        return !T.bValid || !G() || MarketMenuPagesUi::PromotionCandidates(G()->State, T.Tier, T.Country, T.Area).Num() == 0;
    };
    // The first store's manager always comes from outside (M19).
    auto PromotionShown = [Target] { const FTarget T = Target(); return T.bValid && T.Tier != ELevel::FirstStore ? EVisibility::Visible : EVisibility::Collapsed; };
    TSharedRef<SWidget> Picker = SNew(SScrollBox)
        + SScrollBox::Slot()
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [ Label([Target]
                {
                    const FTarget T = Target();
                    if (!T.bValid) return FString();
                    if (T.Tier == ELevel::FirstStore) return FString(TEXT("A\u0130LE D\u00dcKK\u00c2NI \u0130\u00c7\u0130N M\u00dcD\u00dcR"));
                    return FString::Printf(TEXT("%s \u0130\u00c7\u0130N %s"), *MarketManagers::AreaName(T.Tier == ELevel::Depot ? ELevel::Province : T.Tier, T.Country, T.Area).ToUpper(), *MarketManagers::LevelName(T.Tier).ToUpper());
                }, 11, ERole::Text, true) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Button([] { return FString(TEXT("Vazge\u00e7")); }, [this] { AppointArea = INDEX_NONE; }) ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
            [ Label([G, Target, Blurb]
            {
                const FTarget T = Target();
                if (!T.bValid) return FString();
                FString Text = Blurb(T.Tier);
                if (T.Tier == ELevel::Province && G())
                {
                    const int32 Shops = MarketManagers::ProvinceBranches(G()->State, T.Country, T.Area);
                    Text += FString::Printf(TEXT(" Burada %d ma\u011faza var: gereken beceri %d."), Shops, MarketManagers::ProvinceRequiredSkill(Shops));
                }
                return Text;
            }, 10, ERole::Muted, false, true) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 6.f)[ Section(TEXT("DI\u015eARIDAN \u00b7 3 ADAY")) ]
            + SVerticalBox::Slot().AutoHeight()[ CandidateCards([this] { return AppointArea; }, [this] { AppointArea = INDEX_NONE; }) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 4.f)[ SNew(SBox).Visibility_Lambda(PromotionShown)[ Section(TEXT("TERF\u0130")) ] ]
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SBox).Visibility_Lambda([NoPromotion, PromotionShown] { return NoPromotion() && PromotionShown() == EVisibility::Visible ? EVisibility::Visible : EVisibility::Collapsed; })
                [ Fixed(TEXT("Bu b\u00f6lgede terfi edecek ma\u011faza m\u00fcd\u00fcr\u00fc yok."), 10, ERole::Muted) ]
            ]
            + SVerticalBox::Slot().AutoHeight()[ Promotable ]
        ];

    // ---- Suggestions (MarketManagers::Suggestions, most urgent first).
    TSharedRef<SVerticalBox> Advice = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 4; ++Slot)
    {
        auto Text = [G, Slot]() -> FString
        {
            if (!G()) return FString();
            const TArray<FString> List = MarketManagers::Suggestions(G()->State);
            return List.IsValidIndex(Slot) ? List[Slot] : FString();
        };
        Advice->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [ SNew(SBox).Visibility_Lambda([Text] { return Text().IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible; })[ Label(Text, 10, ERole::Text, false, true) ] ];
    }
    Advice->AddSlot().AutoHeight()
    [
        SNew(SBox).Visibility_Lambda([G] { return G() && MarketManagers::Suggestions(G()->State).Num() > 0 ? EVisibility::Collapsed : EVisibility::Visible; })
        [ Fixed(TEXT("\u015eimdilik yap\u0131lacak bir \u015fey yok."), 10, ERole::Muted) ]
    ];

    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1.5f).Padding(0.f, 0.f, 12.f, 0.f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ SpanCounter(false) ]
            + SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 12.f, 0.f, 0.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Section(TEXT("KADEMELER")) ]
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                        [ More([] { return FString(TEXT("\u00dclke m\u00fcd\u00fcr\u00fc \u2192 b\u00f6lge direkt\u00f6r\u00fc \u2192 b\u00f6lge m\u00fcd\u00fcr\u00fc \u2192 il m\u00fcd\u00fcr\u00fc \u2192 ma\u011faza m\u00fcd\u00fcr\u00fc. Hi\u00e7bir kademe kendili\u011finden atanmaz; bo\u015f kademenin ki\u015fileri bir \u00fcsttekine, o da yoksa sana ba\u011flan\u0131r.\n\n\u0130l m\u00fcd\u00fcr\u00fc: ilde 3 ma\u011faza (ilk ma\u011faza say\u0131lmaz). B\u00f6lge m\u00fcd\u00fcr\u00fc: alt b\u00f6lgede 2 il m\u00fcd\u00fcr\u00fc. B\u00f6lge direkt\u00f6r\u00fc: ana b\u00f6lgede 2 b\u00f6lge m\u00fcd\u00fcr\u00fc. \u00dclke m\u00fcd\u00fcr\u00fc: \u00fclkede 5 ilde ma\u011fazan olunca (ilk ma\u011fazan\u0131n ili dahil) g\u00f6r\u00fcn\u00fcr ve atanabilir; \u015firket ikinci \u00fclkeye girince zorunlu, yoksa o \u00fclkedeki m\u00fcd\u00fcrlerin becerisi 10 puan d\u00fc\u015fer.\n\n\u0130lk ma\u011faza: ikinci ma\u011fazan a\u00e7\u0131l\u0131nca m\u00fcd\u00fcr atanabilir. Depo m\u00fcd\u00fcr\u00fc: her depoya bir tane (\u015eirket \u203a Depolar).\n\nHer atamada 3 d\u0131\u015f aday g\u00f6r\u00fcrs\u00fcn; adaylar her hafta yenilenir, ayn\u0131 ad bir daha gelmez. Beceri deneyimle artar ama herkesin bir tavan\u0131 var (en \u00e7ok 95).\n\nAtama terfiyle ya da d\u0131\u015far\u0131dan olur; g\u00f6revden almak 10 g\u00fcnl\u00fck tazminat ister. \u00dccretler g\u00fcn kapan\u0131\u015f\u0131nda kasadan \u00e7\u0131kar.")); }) ]
                    ]
                    // C7 (A menu comparison): one guiding line while there is nobody to manage yet.
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
                    [
                        SNew(SBox).Visibility_Lambda([G] { return G() && MarketMenuSimplifyUi::C7NoNetworkYet(G()->State) ? EVisibility::Visible : EVisibility::Collapsed; })
                        [ Label([] { return FString(TEXT("Kademeler ilk \u015fubeyle dolmaya ba\u015flar: \u00f6nce Ma\u011fazalar sekmesinden ikinci ma\u011fazan\u0131 a\u00e7.")); }, 10, ERole::Muted, false, true) ]
                    ]
                    + SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 8.f, 0.f, 0.f)
                    [
                        SNew(SWidgetSwitcher).WidgetIndex_Lambda([this] { return AppointArea != INDEX_NONE ? 1 : 0; })
                        + SWidgetSwitcher::Slot()[ SNew(SScrollBox) + SScrollBox::Slot()[ Tree ] ]
                        + SWidgetSwitcher::Slot()[ LayerFade(Picker) ]
                    ])
            ]
        ]
        + SHorizontalBox::Slot().FillWidth(1.f)
        [
            SNew(SScrollBox)
            + SScrollBox::Slot()
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("Y\u00d6NET\u0130M G\u0130DER\u0130")) ]
                    // C7 (A menu comparison): no "0 TL / g\u00fcn" while nobody is paid (the line below says so).
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
                    [ SNew(SBox).Visibility_Lambda([G] { return G() && (MarketManagers::DailyWages(G()->State) != 0 || G()->State.Management.LastWages != 0 || G()->State.Management.WeekWages != 0) ? EVisibility::Visible : EVisibility::Collapsed; })
                    [ Mono([G] { return G() ? FString::Printf(TEXT("%s / ay"), *MarketMenuUi::Tl(MarketManagers::DailyWages(G()->State) * 30)) : FString(); }, 20.f, [] { return ERole::Text; }) ] ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
                    [ Label([G]
                    {
                        if (!G()) return FString();
                        // C5 (A menu list): one short line while nobody is paid yet.
                        if (MarketManagers::DailyWages(G()->State) == 0 && G()->State.Management.LastWages == 0 && G()->State.Management.WeekWages == 0)
                            return FString(TEXT("Hen\u00fcz y\u00f6netici yok."));
                        return FString::Printf(TEXT("D\u00fcn \u00f6denen %s \u00b7 bu hafta %s. Ma\u011faza m\u00fcd\u00fcrlerinin \u00fccreti \u015fube giderinde."),
                            *MarketMenuUi::Tl(G()->State.Management.LastWages), *MarketMenuUi::Tl(G()->State.Management.WeekWages));
                    }, 10, ERole::Muted, false, true) ])
            ]
            + SScrollBox::Slot().Padding(0.f, 12.f, 0.f, 0.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Section(TEXT("\u00d6NER\u0130LER")) ]
                    + SVerticalBox::Slot().AutoHeight()[ SNew(SBox).VAlign(VAlign_Top)
                        // C5: the fixed height only while there are suggestions (else one short line).
                        .HeightOverride_Lambda([G]() -> FOptionalSize { return G() && MarketManagers::Suggestions(G()->State).Num() > 0 ? FOptionalSize(150.f) : FOptionalSize(); })[ Advice ] ])
            ]
        ];
}

TSharedRef<SWidget> SMarketMenu::CompanyTab()
{
    // G-086 / G-089: depots in provinces (DepotCard), trucks, central buying, own brand, dark store.
    auto G = [this] { return Game.Get(); };
    static const TCHAR* BuildNames[3] = { TEXT("Kamyon"), TEXT("Merkezi sat\u0131n alma"), TEXT("\u00d6zel marka") }; // M32: dark stores are per province (Sat\u0131\u015f kanallar\u0131)
    static const TCHAR* BuildNotes[3] = {
        TEXT("Depodan \u015fubelere: kamyon g\u00fcnde 8 y\u00fck ta\u015f\u0131r; eksikse %1,5'e kadar kay\u0131p ve ge\u00e7 teslimat."), TEXT("+%2 marj, bir depo ve 8 ma\u011fazadan sonra."),
        TEXT("+%1,5 marj ve biraz daha m\u00fc\u015fteri, 20 ma\u011fazadan sonra.") };
    auto Built = [G](int32 What)
    {
        if (!G()) return false;
        const FMarketCompany& C = G()->State.Company;
        return What == 2 ? C.bCentralBuying : What == 3 ? C.bPrivateLabel : false;
    };
    TSharedRef<SVerticalBox> Builds = SNew(SVerticalBox);
    for (int32 What = 1; What <= 3; ++What)
    {
        Builds->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [ LabelBy([G, Built, What]
                {
                    if (What == 1) return FString::Printf(TEXT("%s \u00b7 %d"), BuildNames[0], G() ? G()->State.Company.Trucks : 0);
                    return FString::Printf(TEXT("%s \u00b7 %s"), BuildNames[What - 1], Built(What) ? TEXT("var") : TEXT("yok"));
                }, 11, [Built, What] { return Built(What) ? ERole::Accent : ERole::Text; }, true) ]
                + SVerticalBox::Slot().AutoHeight()[ Fixed(BuildNotes[What - 1], 9, ERole::Muted) ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
            [ RiskyButton([What] { return FString(What == 1 ? TEXT("Kamyon al") : TEXT("Kur")); },
                [What] { return FString::Printf(TEXT("%s i\u00e7in yat\u0131r\u0131m yap\u0131ls\u0131n m\u0131? Para kasadan \u00e7\u0131kar."), BuildNames[What - 1]); },
                [this, What] { Manage(TEXT("Build"), What); },
                [Built, What] { return !Built(What); }) ]
        ];
    }

    // G-083: the supply lines (tier, discount, the month's minimum) with a step up / down.
    TSharedRef<SVerticalBox> Supply = SNew(SVerticalBox);
    for (int32 L = 0; L < MarketSourcing::LineCount; ++L)
    {
        const MarketSourcing::ELine Line = static_cast<MarketSourcing::ELine>(L);
        auto Tier = [G, Line] { return G() ? static_cast<int32>(MarketSourcing::TierOf(G()->State, Line)) : 0; };
        auto UpOk = [G, Line, Tier] { FString Why; return G() && Tier() + 1 < MarketSourcing::TierCount && MarketSourcing::CanSet(G()->State, Line, static_cast<MarketSourcing::ETier>(Tier() + 1), Why); };
        Supply->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 6.f))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Label([G, Line] { return G() ? MarketSourcing::Describe(G()->State, Line) : FString(); }, 11, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ Label([G, Line] { return G() ? MarketSourcing::NextStep(G()->State, Line) : FString(); }, 9, ERole::Muted, false, true) ]
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                [ RiskyButton([] { return FString(TEXT("Y\u00fckselt")); },
                    [G, Line, Tier] { const MarketSourcing::ETier Next = static_cast<MarketSourcing::ETier>(FMath::Min(Tier() + 1, MarketSourcing::TierCount - 1));
                        return FString::Printf(TEXT("%s art\u0131k %s ile mi \u00e7al\u0131\u015fs\u0131n? Daha ucuz, ama her ay en az %s al\u0131m ister; iki ay alt\u0131nda kal\u0131rsan seni b\u0131rak\u0131r."),
                            *MarketSourcing::LineName(Line), *MarketSourcing::TierName(Next), *MarketCountry::Money(G() ? FMath::RoundToInt64(MarketSourcing::TierMinimum(Next) * MarketPrices::ListLevel(G()->State.Day)) : 0)); },
                    [this, Line, Tier] { Manage(TEXT("SetSourcing"), MarketSourcing::Encode(Line, static_cast<MarketSourcing::ETier>(FMath::Min(Tier() + 1, MarketSourcing::TierCount - 1)))); },
                    UpOk) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 0.f, 0.f)
                [ Button([] { return FString(TEXT("D\u00fc\u015f\u00fcr")); }, [this, Line, Tier] { if (Tier() > 0) Manage(TEXT("SetSourcing"), MarketSourcing::Encode(Line, static_cast<MarketSourcing::ETier>(Tier() - 1))); },
                    false, [Tier] { return Tier() > 0; }) ]
            ]
        ];
    }

    // M26: departments per store type, price stance, masters.
    static const TCHAR* FormatShorts[4] = { TEXT("Ucuzcu"), TEXT("Mahalle"), TEXT("S\u00fcper"), TEXT("Hiper") };
    static const TCHAR* StanceShorts[3] = { TEXT("Ucuz"), TEXT("Normal"), TEXT("Pahal\u0131") };
    TSharedRef<SVerticalBox> Depts = SNew(SVerticalBox);
    for (int32 Dx = 0; Dx < MarketDepartments::DeptCount; ++Dx)
    {
        const MarketDepartments::EDept Dept = static_cast<MarketDepartments::EDept>(Dx);
        const MarketDepartments::FInfo& Info = MarketDepartments::Info(Dept);
        if (Dx == 0 || Dx == static_cast<int32>(MarketDepartments::EDept::Electronics))
            Depts->AddSlot().AutoHeight().Padding(0.f, Dx == 0 ? 0.f : 8.f, 0.f, 2.f)[ Fixed(Dx == 0 ? TEXT("Taze reyonlar") : TEXT("G\u0131da d\u0131\u015f\u0131"), 10, ERole::Muted, true) ];
        TSharedRef<SHorizontalBox> Formats = SNew(SHorizontalBox);
        for (int32 F = Info.MinFormat; F < MarketDepartments::FormatCount; ++F)
        {
            Formats->AddSlot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)
            [ Choice(FormatShorts[F],
                [G, Dept, F] { return G() && MarketDepartments::IsOn(G()->State, Dept, F); },
                [this, G, Dept, F] { if (G()) Manage(TEXT("SetDepartment"), MarketDepartments::EncodeSet(Dept, F, !MarketDepartments::IsOn(G()->State, Dept, F))); },
                [G, Dept, F] { FString Why; return G() && (MarketDepartments::IsOn(G()->State, Dept, F) || MarketDepartments::CanSet(G()->State, Dept, F, true, Why)); }) ];
        }
        TSharedRef<SHorizontalBox> Stances = SNew(SHorizontalBox);
        for (int32 S = 0; S < 3; ++S)
        {
            Stances->AddSlot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)
            [ Choice(StanceShorts[S], [G, Dept, S] { return G() && MarketDepartments::Stance(G()->State, Dept) == S; },
                [this, Dx, S] { Manage(TEXT("SetDeptStance"), Dx * 10 + S); }) ];
        }
        TSharedRef<SVerticalBox> Right = SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)[ Formats ]
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 3.f, 0.f, 0.f)[ Stances ];
        if (Info.bMaster)
            Right->AddSlot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 3.f, 0.f, 0.f)
            [ Button([G, Dept] { return FString::Printf(TEXT("Zay\u0131f ustalar\u0131 de\u011fi\u015ftir (%d)"), G() ? MarketDepartments::WeakMasters(G()->State, Dept) : 0); },
                [this, Dx] { Manage(TEXT("ReplaceMasters"), Dx); }, false, [G, Dept] { return G() && MarketDepartments::WeakMasters(G()->State, Dept) > 0; }) ];
        Depts->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 6.f))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(Info.Name, 11, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(MarketDepartments::Describe(Dept), 9, ERole::Muted) ]
                    + SVerticalBox::Slot().AutoHeight()[ Label([G, Dept] { return G() ? MarketDepartments::Results(G()->State, Dept) : FString(); }, 9, ERole::Text) ]
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)[ Right ]
            ]
        ];
    }

    // M29 / M30: chains for sale anywhere (a giant leaving a country too) and our subsidiaries.
    static constexpr int32 DealRows = 8;
    auto ForSaleAt = [G](int32 Slot) -> int32
    {
        if (!G()) return INDEX_NONE;
        int32 Seen = 0;
        const TArray<FMarketChain>& Chains = G()->State.Rivals.Chains;
        for (int32 I = 0; I < Chains.Num(); ++I)
            if (!Chains[I].bGone && !Chains[I].bOurs && Chains[I].bForSale && Seen++ == Slot) return I;
        return INDEX_NONE;
    };
    auto OursAt = [G](int32 Slot) -> int32
    {
        if (!G()) return INDEX_NONE;
        int32 Seen = 0;
        const TArray<FMarketChain>& Chains = G()->State.Rivals.Chains;
        for (int32 I = 0; I < Chains.Num(); ++I)
            if (!Chains[I].bGone && Chains[I].bOurs && Seen++ == Slot) return I;
        return INDEX_NONE;
    };
    TSharedRef<SVerticalBox> Deals = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < DealRows; ++Slot)
    {
        Deals->AddSlot().AutoHeight().Padding(0.f, 2.f)
        [
            SNew(SHorizontalBox).Visibility_Lambda([ForSaleAt, Slot] { return ForSaleAt(Slot) != INDEX_NONE ? EVisibility::Visible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [ Label([G, ForSaleAt, Slot]
            {
                const int32 I = ForSaleAt(Slot);
                if (I == INDEX_NONE) return FString();
                const MarketCountry::FProfile* Pack = MarketCountry::Find(G()->State.Rivals.Chains[I].Country);
                return FString::Printf(TEXT("%s \u00b7 %s"), *MarketChains::Describe(G()->State, I), Pack ? *Pack->Name : TEXT(""));
            }, 10, ERole::Text, false, true) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
            [ RiskyButton([] { return FString(TEXT("Sat\u0131n al")); },
                [G, ForSaleAt, Slot] { const int32 I = ForSaleAt(Slot); return I != INDEX_NONE ? FString::Printf(TEXT("%s %s kar\u015f\u0131l\u0131\u011f\u0131nda al\u0131ns\u0131n m\u0131? Kasa yetiyorsa hemen; yetmiyorsa bankalara sat\u0131n alma kredisi ba\u015fvurusu yap\u0131l\u0131r, cevaplar birka\u00e7 g\u00fcnde gelir (Finans). B\u00fcy\u00fck zincir kendi ad\u0131yla ba\u011fl\u0131 \u015firketimiz olur."),
                    *G()->State.Rivals.Chains[I].Name, *MarketCountry::Money(MarketChains::Price(G()->State, I))) : FString(); },
                [this, ForSaleAt, Slot] { const int32 I = ForSaleAt(Slot); if (I != INDEX_NONE) Manage(TEXT("DealFinance"), I); },
                [G, ForSaleAt, Slot] { FString Why; const int32 I = ForSaleAt(Slot); return I != INDEX_NONE && MarketChains::CanBuy(G()->State, I, Why, false); }) ]
        ];
    }
    TSharedRef<SVerticalBox> Owned = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < DealRows; ++Slot)
    {
        Owned->AddSlot().AutoHeight().Padding(0.f, 2.f)
        [
            SNew(SHorizontalBox).Visibility_Lambda([OursAt, Slot] { return OursAt(Slot) != INDEX_NONE ? EVisibility::Visible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [ Label([G, OursAt, Slot] { const int32 I = OursAt(Slot); return I != INDEX_NONE ? MarketChains::Describe(G()->State, I) : FString(); }, 10, ERole::Text, false, true) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
            [ RiskyButton([G, OursAt, Slot] { const int32 I = OursAt(Slot); return FString::Printf(TEXT("Bizim ad\u0131m\u0131za \u00e7evir (%d)"), I != INDEX_NONE ? MarketChains::ConvertRoom(G()->State, I) : 0); },
                [G, OursAt, Slot] { const int32 I = OursAt(Slot); if (I == INDEX_NONE) return FString(); const int32 N = MarketChains::ConvertRoom(G()->State, I);
                    return FString::Printf(TEXT("%d ma\u011faza tabelas\u0131n\u0131 de\u011fi\u015ftirip \u015fubemiz olsun mu? Maliyet %s. Ayda en \u00e7ok %d ma\u011faza \u00e7evrilir; ilde yer olmal\u0131."), N, *MarketCountry::Money(MarketChains::ConvertCost(G()->State, I, N)), MarketChains::ConvertPerMonth); },
                [this, OursAt, Slot, G] { const int32 I = OursAt(Slot); if (I != INDEX_NONE) Manage(TEXT("ConvertStores"), MarketChains::EncodeConvert(I, MarketChains::ConvertRoom(G()->State, I))); },
                [G, OursAt, Slot] { const int32 I = OursAt(Slot); return I != INDEX_NONE && MarketChains::ConvertRoom(G()->State, I) > 0; }) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f, 0.f, 0.f)
            [ RiskyButton([] { return FString(TEXT("Sat")); },
                [G, OursAt, Slot] { const int32 I = OursAt(Slot); return I != INDEX_NONE ? FString::Printf(TEXT("%s %s kar\u015f\u0131l\u0131\u011f\u0131nda sat\u0131ls\u0131n m\u0131? Yeniden rakibimiz olur."), *G()->State.Rivals.Chains[I].Name, *MarketCountry::Money(MarketChains::SalePrice(G()->State, I))) : FString(); },
                [this, OursAt, Slot] { const int32 I = OursAt(Slot); if (I != INDEX_NONE) Manage(TEXT("SellSubsidiary"), I); },
                [OursAt, Slot] { return OursAt(Slot) != INDEX_NONE; }) ]
        ];
    }

    // D9 (M48-M50): the strategic forks, the paths and our province markets with the push.
    auto ProvinceAt = [G](int32 Slot) -> FString
    {
        if (!G()) return FString();
        const TArray<FString> List = MarketStrategy::OurProvinces(G()->State, 6);
        return List.IsValidIndex(Slot) ? List[Slot] : FString();
    };
    TSharedRef<SVerticalBox> PathRows = SNew(SVerticalBox);
    for (int32 P = 0; P < static_cast<int32>(MarketStrategy::EPath::Count); ++P)
        PathRows->AddSlot().AutoHeight().Padding(0.f, 2.f)
        [ Label([G, P] { return G() ? MarketStrategy::PathLine(G()->State, static_cast<MarketStrategy::EPath>(P)) : FString(); }, 10, ERole::Text, false, true) ];
    TSharedRef<SVerticalBox> ProvinceRows = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 6; ++Slot)
    {
        ProvinceRows->AddSlot().AutoHeight().Padding(0.f, 2.f)
        [
            SNew(SHorizontalBox).Visibility_Lambda([ProvinceAt, Slot] { return ProvinceAt(Slot).IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible; })
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [ Label([G, ProvinceAt, Slot] { FString C, P; return ProvinceAt(Slot).Split(TEXT("|"), &C, &P) ? MarketStrategy::ProvinceLine(G()->State, C, P) : FString(); }, 10, ERole::Text, false, true) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
            [ RiskyButton([] { return FString(TEXT("\u0130l ata\u011f\u0131")); },
                [G, ProvinceAt, Slot] { FString C, P; if (!ProvinceAt(Slot).Split(TEXT("|"), &C, &P)) return FString();
                    return FString::Printf(TEXT("%s: %d g\u00fcn yerel kampanya: g\u00fcnde %s (toplam yakla\u015f\u0131k %s). \u015eubelerimize daha \u00e7ok m\u00fc\u015fteri gelir; rakip fiyat k\u0131rabilir; sonunda k\u00fc\u00e7\u00fck rakipler bir ma\u011fazas\u0131n\u0131 kapatabilir. Ba\u015flas\u0131n m\u0131?"),
                        *MarketChains::ProvinceName(C, P), MarketStrategy::PushDays, *MarketCountry::Money(MarketStrategy::PushDailyCost(G()->State, C, P)),
                        *MarketCountry::Money(MarketStrategy::PushDailyCost(G()->State, C, P) * MarketStrategy::PushDays)); },
                [this, ProvinceAt, Slot] { FString C, P; if (ProvinceAt(Slot).Split(TEXT("|"), &C, &P)) Manage(TEXT("ProvincePush"), MarketBranches::EncodeSite(C, P, TEXT("mahalle"))); },
                [G, ProvinceAt, Slot] { FString C, P, Why; return G() && ProvinceAt(Slot).Split(TEXT("|"), &C, &P) && MarketStrategy::CanPush(G()->State, C, P, Why); }) ]
        ];
    }
    TSharedRef<SWidget> StrategyCard = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Section(TEXT("STRATEJ\u0130, YOLLAR VE \u0130LLER")) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ More([] { return FString(TEXT("Yol ayr\u0131mlar\u0131: 5, 25 ve 100 \u015fubede \u015firketin y\u00f6n\u00fc sorulur (odak, b\u00fcy\u00fcme modeli, b\u00fcy\u00fck yat\u0131r\u0131m). Se\u00e7im kal\u0131c\u0131d\u0131r.\n\nYollar: b\u00fcy\u00fckl\u00fckten ba\u015fka d\u00f6rt iyi olma yolu. Her ay \u00f6l\u00e7\u00fcl\u00fcr, \u00fc\u00e7 basama\u011f\u0131 var; basamak d\u00fc\u015febilir. Her basamak k\u00fc\u00e7\u00fck bir kazan\u0131m getirir.\n\n\u0130ller: bir ilde her rakip zincirden \u00e7ok ma\u011fazan varsa (en az 2) il \u015fampiyonusun. \u0130l ata\u011f\u0131 60 g\u00fcn yerel kampanyad\u0131r: \u015fubelere daha \u00e7ok m\u00fc\u015fteri gelir, rakip fiyat k\u0131rarak cevap verebilir, sonunda k\u00fc\u00e7\u00fck rakipler oradaki bir ma\u011fazas\u0131n\u0131 kapatabilir. Ayn\u0131 il alt\u0131 ay dinlenir.")); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
        [ Label([G] { if (!G()) return FString(); const FString Line = MarketStrategy::StrategyLine(G()->State);
            return Line.IsEmpty() ? FString(TEXT("Hen\u00fcz yol ayr\u0131m\u0131 se\u00e7ilmedi: ilki 5 \u015fubede gelir.")) : Line; }, 10, ERole::Muted, false, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[ PathRows ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[ ProvinceRows ]);

    // C13 (M43): our bids waiting for an answer or accepted, and the market rumours.
    auto BidAt = [G](int32 Slot) -> int32
    {
        if (!G()) return INDEX_NONE;
        int32 Seen = 0;
        const FMarketState& St = G()->State;
        for (int32 I = 0; I < St.Rivals.Chains.Num(); ++I)
        {
            const FMarketChain& C = St.Rivals.Chains[I];
            if (!C.bGone && (C.BidAnswerDay >= St.Day || C.BidAcceptedUntil >= St.Day) && Seen++ == Slot) return I;
        }
        return INDEX_NONE;
    };
    TSharedRef<SVerticalBox> BidRows = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 4; ++Slot)
    {
        BidRows->AddSlot().AutoHeight().Padding(0.f, 2.f)
        [
            SNew(SHorizontalBox).Visibility_Lambda([BidAt, Slot] { return BidAt(Slot) != INDEX_NONE ? EVisibility::Visible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [ Label([G, BidAt, Slot]
            {
                const int32 I = BidAt(Slot);
                if (I == INDEX_NONE) return FString();
                const FMarketState& St = G()->State;
                const FMarketChain& C = St.Rivals.Chains[I];
                if (C.BidAcceptedUntil >= St.Day)
                    return FString::Printf(TEXT("%s teklifimizi KABUL ETT\u0130: %s \u00b7 %d g\u00fcn i\u00e7inde \u00f6denmeli"), *C.Name, *MarketCountry::Money(C.BidAgreed), C.BidAcceptedUntil - St.Day + 1);
                return FString::Printf(TEXT("%s teklifimizi (%s) de\u011ferlendiriyor \u00b7 cevap %d g\u00fcn i\u00e7inde"), *C.Name, *MarketCountry::Money(C.BidAgreed), FMath::Max(1, C.BidAnswerDay - St.Day + 1));
            }, 10, ERole::Text, false, true) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
            [ RiskyButton([] { return FString(TEXT("Anla\u015fmay\u0131 tamamla")); },
                [G, BidAt, Slot] { const int32 I = BidAt(Slot); if (I == INDEX_NONE) return FString(); const int64 DealCost = MarketChains::DealPrice(G()->State, I);
                    return G()->State.Cash >= DealCost ? FString::Printf(TEXT("%s kasadan \u00f6densin ve %s bizim olsun mu?"), *MarketCountry::Money(DealCost), *G()->State.Rivals.Chains[I].Name)
                        : FString::Printf(TEXT("Kasada %s eksik. Bankalara sat\u0131n alma kredisi ba\u015fvurusu yap\u0131ls\u0131n m\u0131? Cevaplar birka\u00e7 g\u00fcnde gelir; bir teklifi kabul edince anla\u015fma kendili\u011finden tamamlan\u0131r."),
                            *MarketCountry::Money(MarketBanking::AcquisitionNeed(G()->State, DealCost))); },
                [this, BidAt, Slot] { const int32 I = BidAt(Slot); if (I != INDEX_NONE) Manage(TEXT("DealFinance"), I); },
                [G, BidAt, Slot] { const int32 I = BidAt(Slot); return I != INDEX_NONE && G()->State.Rivals.Chains[I].BidAcceptedUntil >= G()->State.Day; }) ]
        ];
    }
    TSharedRef<SVerticalBox> RumorRows = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < MarketRumors::MaxActive; ++Slot)
        RumorRows->AddSlot().AutoHeight().Padding(0.f, 2.f)
        [ SNew(SBox).Visibility_Lambda([G, Slot] { return G() && G()->State.Rumors.Active.IsValidIndex(Slot) ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Label([G, Slot] { return G() && G()->State.Rumors.Active.IsValidIndex(Slot) ? MarketRumors::Describe(G()->State, G()->State.Rumors.Active[Slot]) : FString(); }, 10, ERole::Text, false, true) ] ];
    for (int32 Slot = 0; Slot < 3; ++Slot)
        RumorRows->AddSlot().AutoHeight().Padding(0.f, 2.f)
        [ SNew(SBox).Visibility_Lambda([G, Slot] { return G() && G()->State.Rumors.Past.Num() > Slot ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Label([G, Slot] { if (!G() || G()->State.Rumors.Past.Num() <= Slot) return FString(); const TArray<FMarketRumor>& Past = G()->State.Rumors.Past;
                return MarketRumors::DescribePast(G()->State, Past[Past.Num() - 1 - Slot]); }, 9, ERole::Muted, false, true) ] ];
    TSharedRef<SWidget> TalkCard = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Section(TEXT("KUL\u0130S VE TEKL\u0130FLER")) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ More([] { return FString(TEXT("Piyasada rakipler hakk\u0131nda s\u00f6ylentiler dola\u015f\u0131r: sat\u0131\u015fa \u00e7\u0131kmak, bir ile girmek, ba\u015fka bir zinciri almak, fiyat k\u0131rmak. Kaynaklar geldik\u00e7e asistan s\u00f6ylentinin g\u00fcc\u00fcn\u00fc s\u00f6yler. G\u00fcvenilir kaynak \u00e7o\u011fu zaman hakl\u0131d\u0131r ama her zaman de\u011fil; g\u00fcvensiz kaynak da bazen do\u011fru \u00e7\u0131kar. \u00dclke m\u00fcd\u00fcr\u00fc ve mali m\u00fc\u015favir daha g\u00fcvenilir kaynak getirir.\n\nSat\u0131n alma teklifi zincirin y\u00f6netim kuruluna gider, cevap 2-4 g\u00fcnde gelir. Kabul ederse 14 g\u00fcn i\u00e7inde \u00f6deme yap\u0131lmal\u0131; kasa yetmezse bankalara ba\u015fvurulur.")); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight()
        [ SNew(SBox).Visibility_Lambda([G, BidAt] { return G() && BidAt(0) == INDEX_NONE && G()->State.Rumors.Active.Num() == 0 && G()->State.Rumors.Past.Num() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Fixed(TEXT("Kulis sessiz: \u015fimdilik dola\u015fan bir s\u00f6ylenti ya da bekleyen teklif yok."), 10, ERole::Muted) ] ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)[ BidRows ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)[ RumorRows ]);

    // M34: the company's advertising, one country at a time.
    auto AdWhere = [this, G]() -> FString
    {
        if (!G()) return FString();
        const TArray<FString> List = MarketAdvertising::Countries(G()->State);
        return List.IsValidIndex(AdCountry) ? List[AdCountry] : G()->State.CountryId;
    };
    TSharedRef<SVerticalBox> AdRows = SNew(SVerticalBox);
    for (int32 Ch = 0; Ch < MarketAdvertising::ChannelCount; ++Ch)
    {
        const MarketAdvertising::EChannel Channel = static_cast<MarketAdvertising::EChannel>(Ch);
        auto Level = [G, AdWhere, Channel] { return G() ? MarketAdvertising::LevelOf(G()->State, AdWhere(), Channel) : 0; };
        auto There = [G, Channel] { return G() && MarketAdvertising::EraFactor(G()->State, Channel, G()->State.Day) > 0.f; };
        TSharedRef<SHorizontalBox> Steps = SNew(SHorizontalBox);
        static const TCHAR* StepNames[4] = { TEXT("Yok"), TEXT("Az"), TEXT("Orta"), TEXT("\u00c7ok") };
        for (int32 L = 0; L <= MarketAdvertising::MaxLevel; ++L)
            Steps->AddSlot().AutoWidth().Padding(L > 0 ? 4.f : 0.f, 0.f, 0.f, 0.f)
            [ Choice(StepNames[L], [Level, L] { return Level() == L; }, [this, G, Channel, L] { const int32 Index = G() && MarketAdvertising::Countries(G()->State).IsValidIndex(AdCountry) ? AdCountry : 0; Manage(TEXT("AdLevel"), MarketAdvertising::Encode(Index, Channel, L)); }, [There, L] { return L == 0 || There(); }) ];
        AdRows->AddSlot().AutoHeight().Padding(0.f, 4.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ Fixed(MarketAdvertising::ChannelName(Channel), 11, ERole::Text, true) ]
                + SVerticalBox::Slot().AutoHeight()[ Fixed(MarketAdvertising::ChannelNote(Channel), 9, ERole::Muted) ]
                + SVerticalBox::Slot().AutoHeight()[ Label([G, AdWhere, Channel] { return G() ? MarketAdvertising::ChannelLine(G()->State, AdWhere(), Channel) : FString(); }, 9, ERole::Muted, false, true) ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)[ Steps ]
        ];
    }
    TSharedRef<SHorizontalBox> AdCountries = SNew(SHorizontalBox);
    for (int32 Slot = 0; Slot < 6; ++Slot)
        AdCountries->AddSlot().AutoWidth().Padding(Slot > 0 ? 4.f : 0.f, 0.f, 0.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([G, Slot] { return G() && MarketAdvertising::Countries(G()->State).Num() > 1 && MarketAdvertising::Countries(G()->State).IsValidIndex(Slot) ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Button([G, Slot] { if (!G()) return FString(); const TArray<FString> List = MarketAdvertising::Countries(G()->State);
                const MarketCountry::FProfile* Pack = List.IsValidIndex(Slot) ? MarketCountry::Find(List[Slot]) : nullptr; return Pack ? Pack->Name : FString(); },
                [this, Slot] { AdCountry = Slot; }, false) ]
        ];
    static const int32 Budgets[4] = { 10, 20, 30, 50 };
    TSharedRef<SHorizontalBox> BudgetRow = SNew(SHorizontalBox);
    for (int32 B = 0; B < 4; ++B)
        BudgetRow->AddSlot().AutoWidth().Padding(B > 0 ? 4.f : 0.f, 0.f, 0.f, 0.f)
        [ Choice(FString::Printf(TEXT("%%%d"), Budgets[B] / 10), [G, B] { return G() && G()->State.Advertising.BudgetPermille == Budgets[B]; }, [this, B] { Manage(TEXT("AdBudget"), Budgets[B]); }) ];
    TSharedRef<SWidget> AdCard = Card(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Section(TEXT("REKLAM")) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ More([] { return FString(TEXT("Reklam b\u00fct\u00fcn ma\u011fazalar\u0131n m\u00fc\u015fterisini (en \u00e7ok %12) ve internet sipari\u015fini (en \u00e7ok %25) art\u0131r\u0131r; ilk lira en \u00e7ok i\u015fe yarar. Her kanal\u0131n ak\u0131llarda kalan\u0131 kendi h\u0131z\u0131yla s\u00f6ner: televizyon yava\u015f, bro\u015f\u00fcr ve arama \u00e7abuk. \u00dc\u00e7 ya da daha \u00e7ok kanal birlikte %20, be\u015fi birden %30 daha iyi \u00e7al\u0131\u015f\u0131r.")); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)[ AdCountries ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 4.f)[ Label([G, AdWhere] { return G() ? MarketAdvertising::CountryLine(G()->State, AdWhere()) : FString(); }, 10, ERole::Text, false, true) ]
        // C7 (A menu comparison): the summary, the manager and the budget first, the channel rows under them.
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 2.f)
        [ Label([G]
        {
            if (!G()) return FString();
            const FMarketAdvertising& A = G()->State.Advertising;
            if (!A.ManagerName.IsEmpty()) return FString::Printf(TEXT("Reklam m\u00fcd\u00fcr\u00fc %s \u00b7 beceri %d \u00b7 ayda %s%s"), *A.ManagerName, A.ManagerSkill, *MarketMenuUi::Tl(A.ManagerWage * 30),
                A.bAuto ? *FString::Printf(TEXT(" \u00b7 kar\u0131\u015f\u0131m\u0131 o ayarl\u0131yor (cironun %%%.0f'i)"), A.BudgetPermille / 10.f) : TEXT(""));
            FString Reason;
            if (!MarketAdvertising::CanHireManager(G()->State, Reason)) return Reason;
            FString Name; int32 Skill = 0; int64 Wage = 0;
            MarketAdvertising::Candidate(G()->State, Name, Skill, Wage);
            return FString::Printf(TEXT("Bu haftan\u0131n reklam m\u00fcd\u00fcr\u00fc aday\u0131: %s, beceri %d, ayda %s."), *Name, Skill, *MarketMenuUi::Tl(Wage * 30));
        }, 10, ERole::Text, false, true) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 4.f))
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("\u0130\u015fe al")); }, [this] { Manage(TEXT("AdHire"), 0); }, false, [G] { FString Reason; return G() && MarketAdvertising::CanHireManager(G()->State, Reason); }) ]
            + SWrapBox::Slot()[ RiskyButton([] { return FString(TEXT("Ayr\u0131lmas\u0131n\u0131 iste")); }, [] { return FString(TEXT("Reklam m\u00fcd\u00fcr\u00fc ayr\u0131ls\u0131n m\u0131? On g\u00fcnl\u00fck \u00fccret ve k\u0131dem tazminat\u0131 \u00f6denir.")); },
                [this] { Manage(TEXT("AdFire"), 0); }, [G] { return G() && !G()->State.Advertising.ManagerName.IsEmpty(); }) ]
            + SWrapBox::Slot()[ Choice(TEXT("Kar\u0131\u015f\u0131m onda"), [G] { return G() && G()->State.Advertising.bAuto; }, [this, G] { Manage(TEXT("AdAuto"), G() && G()->State.Advertising.bAuto ? 0 : 1); },
                [G] { return G() && !G()->State.Advertising.ManagerName.IsEmpty(); }) ]
            + SWrapBox::Slot()[ BudgetRow ]
        ]
        // C7 (A menu comparison): a manual level switches the manager's mix off (MarketAdvertising::SetLevelArg); say so next to the buttons.
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([G] { return G() && G()->State.Advertising.bAuto && !G()->State.Advertising.ManagerName.IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Label([] { return FString(TEXT("Elle de\u011fi\u015ftirirsen m\u00fcd\u00fcr\u00fcn otomatik kar\u0131\u015f\u0131m\u0131 kapan\u0131r.")); }, 9, ERole::Warn, false, true) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[ AdRows ]);

    return SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        // C7 (A menu comparison): day one, one guiding line above the cards.
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
        [
            SNew(SBox).Visibility_Lambda([G] { return G() && MarketMenuSimplifyUi::C7NoNetworkYet(G()->State) ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Label([] { return FString(TEXT("\u015eirket kararlar\u0131 \u015fubelerle anlam kazan\u0131r: depo, tedarik ve reyonlar ma\u011faza say\u0131s\u0131 artt\u0131k\u00e7a a\u00e7\u0131l\u0131r.")); }, 11, ERole::Muted, false, true) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[ StrategyCard ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[ TalkCard ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
        [ Card(SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                // C7 (A menu comparison): nothing for sale and no subsidiary: only the one line below.
                SNew(SHorizontalBox).Visibility_Lambda([ForSaleAt, OursAt] { return ForSaleAt(0) == INDEX_NONE && OursAt(0) == INDEX_NONE ? EVisibility::Collapsed : EVisibility::Visible; })
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Section(TEXT("SATIN ALMA VE BA\u011eLI \u015e\u0130RKETLER")) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [ More([] { return FString(TEXT("Sat\u0131l\u0131k zincirler burada (bir \u00fclkeden \u00e7ekilen dev de, hen\u00fcz girmedi\u011fin \u00fclkede bile). Sat\u0131l\u0131k olmayana Rakipler sayfas\u0131ndan teklif verilir. Ald\u0131\u011f\u0131n zincirin hepsi senin: k\u00fc\u00e7\u00fc\u011f\u00fc (20 ma\u011fazaya kadar) hemen \u015fube olur, b\u00fcy\u00fc\u011f\u00fc kendi ad\u0131yla ba\u011fl\u0131 \u015firketin olarak \u00e7al\u0131\u015f\u0131r; ayda 20 ma\u011fazas\u0131n\u0131 bizim ad\u0131m\u0131za \u00e7evirebilir ya da \u015firketi satabilirsin.")); }) ]
            ]
            + SVerticalBox::Slot().AutoHeight()
            [ SNew(SBox).Visibility_Lambda([ForSaleAt, OursAt] { return ForSaleAt(0) == INDEX_NONE && OursAt(0) == INDEX_NONE ? EVisibility::Visible : EVisibility::Collapsed; })
                [ Fixed(TEXT("Sat\u0131n alma: \u015fu an sat\u0131l\u0131k zincir ya da ba\u011fl\u0131 \u015firket yok."), 10, ERole::Muted) ] ]
            + SVerticalBox::Slot().AutoHeight()[ Deals ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[ Owned ]) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[ AdCard ]
        + SVerticalBox::Slot().AutoHeight()
        [
            Card(SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ Section(TEXT("\u015e\u0130RKET")) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[ Label([G] { return G() ? MarketCompany::Summary(G()->State) : FString(); }, 11, ERole::Text, false, true) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
                [ More([] { return FString(TEXT("B\u00fct\u00fcn \u00fclke ba\u015ftan a\u00e7\u0131k. Ev ilinin d\u0131\u015f\u0131ndaki ma\u011faza \u0130K m\u00fcd\u00fcr\u00fc ve mali m\u00fc\u015favir ister; 600 km i\u00e7inde deposu olmayan ma\u011fazalar mal\u0131 toptanc\u0131dan al\u0131r (+%3). Yurt d\u0131\u015f\u0131 ana \u00fclkede 25 ma\u011faza ve 5 ille a\u00e7\u0131l\u0131r.")); }) ])
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.6f).Padding(0.f, 0.f, 12.f, 0.f)
            [ DepotCard() ]
            + SHorizontalBox::Slot().FillWidth(1.f)
            [ Card(SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Section(TEXT("YATIRIMLAR")) ]
                + SVerticalBox::Slot().AutoHeight()[ Builds ]) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [ Card(SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Section(TEXT("TEDAR\u0130K")) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
                [ Mono([G] { return G() ? FString::Printf(TEXT("al\u0131m g\u00fcc\u00fc %%%.1f"), 100.f * MarketSourcing::VolumeDiscount(G()->State)) : FString(); }, 12.f, [] { return ERole::Accent; }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [ More([] { return FString(TEXT("Her mal grubu bir kaynaktan al\u0131n\u0131r: yerel toptanc\u0131, b\u00f6lge distrib\u00fct\u00f6r\u00fc, ulusal distrib\u00fct\u00f6r ya da \u00fcretici. \u00dcst kaynak ucuzdur ama b\u00fcy\u00fckl\u00fck (ma\u011faza, depo, merkezi sat\u0131n alma) ve ayl\u0131k asgari al\u0131m ister. Al\u0131m g\u00fcc\u00fc son 30 g\u00fcn\u00fcn al\u0131m\u0131yla b\u00fcy\u00fcr: her iki kat\u0131nda %3, en \u00e7ok %12.")); }) ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[ Supply ]) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [ Card(SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Section(TEXT("REYONLAR")) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
                [ Mono([G] { return G() ? FString::Printf(TEXT("alan: mahalle %%%d/%d \u00b7 s\u00fcper %%%d/%d \u00b7 hiper %%%d/%d"),
                    MarketDepartments::SpaceUsed(G()->State, 1), MarketDepartments::SpaceCap(1), MarketDepartments::SpaceUsed(G()->State, 2), MarketDepartments::SpaceCap(2),
                    MarketDepartments::SpaceUsed(G()->State, 3), MarketDepartments::SpaceCap(3)) : FString(); }, 12.f, [] { return ERole::Accent; }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [ More([] { return FString(TEXT("Paketli raflar her ma\u011fazada var; reyonlar onlar\u0131n \u00fcst\u00fcne ayr\u0131 birer i\u015ftir. Hangi ma\u011faza t\u00fcr\u00fcnde hangi reyon olaca\u011f\u0131n\u0131 se\u00e7ersin: o t\u00fcrdeki b\u00fct\u00fcn \u015fubelerde a\u00e7\u0131l\u0131r (tadilat + stok \u015fimdi \u00f6denir), yeni \u015fubeler onunla a\u00e7\u0131l\u0131r. Taze reyonlar m\u00fc\u015fteri \u00e7eker ama fire verir; kasap, f\u0131r\u0131n ve bal\u0131k usta ister. G\u0131da d\u0131\u015f\u0131 aylarca stok ba\u011flar, mevsime g\u00f6re sat\u0131l\u0131r. Alan s\u0131n\u0131rl\u0131: s\u00fcpermarkette hepsi s\u0131\u011fmaz. Kapat\u0131rsan stok %60'\u0131na elden \u00e7\u0131kar.")); }) ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[ Depts ]) ]
    ];
}

// ---------------------------------------------------------------------------------------------------------------
// G-089 (M23) Sirket > Depolar: every depot (range, branches served / capacity, manager, efficiency), building one
// in a province (the suggested province first, the gain and the cost) and a depot manager from 3 candidates. One
// card of fixed height with three layers (M15).

TSharedRef<SWidget> SMarketMenu::DepotCard()
{
    using MarketMenuPagesUi::FDepotChoice;
    auto G = [this] { return Game.Get(); };
    auto SiteAt = [G](int32 Depot) -> const FMarketDepot*
    {
        return G() && G()->State.Company.DepotSites.IsValidIndex(Depot) ? &G()->State.Company.DepotSites[Depot] : nullptr;
    };
    auto Sites = [G] { return G() ? G()->State.Company.DepotSites.Num() : 0; };

    // ---- Layer 0: the depots.
    TSharedRef<SVerticalBox> List = SNew(SVerticalBox);
    for (int32 Depot = 0; Depot < 16; ++Depot)
    {
        auto Boss = [G, Depot] { return G() ? MarketDepots::ManagerOf(G()->State, Depot) : INDEX_NONE; };
        auto Line = [G, SiteAt, Boss, Depot]() -> FString
        {
            const FMarketDepot* Site = SiteAt(Depot);
            if (!Site || !G()) return FString();
            const FMarketState& S = G()->State;
            const int32 Man = Boss();
            FString Who = Man != INDEX_NONE && S.Management.Managers.IsValidIndex(Man) ? FString::Printf(TEXT("m\u00fcd\u00fcr %s"), *S.Management.Managers[Man].Name) : FString(TEXT("m\u00fcd\u00fcr yok (yar\u0131 verim)"));
            if (Man != INDEX_NONE && S.Management.Managers.IsValidIndex(Man) && !Site->CaughtName.IsEmpty() && Site->CaughtName == S.Management.Managers[Man].Name)
                Who += TEXT(" \u00b7 MALDAN KA\u00c7IRIYOR");
            return FString::Printf(TEXT("%d km menzil \u00b7 %d / %d ma\u011faza \u00b7 verim %%%.0f \u00b7 %s"), MarketDepots::RangeKm, MarketDepots::Served(S, Depot), Site->Capacity,
                100.f * MarketDepots::Efficiency(S, Depot), *Who);
        };
        auto LineRole = [G, SiteAt, Boss, Depot]
        {
            const FMarketDepot* Site = SiteAt(Depot);
            if (!Site || !G()) return ERole::Muted;
            const int32 Man = Boss();
            const TArray<FMarketManager>& People = G()->State.Management.Managers;
            if (!Site->CaughtName.IsEmpty() && People.IsValidIndex(Man) && Site->CaughtName == People[Man].Name) return ERole::Bad;
            if (Man == INDEX_NONE || MarketDepots::Served(G()->State, Depot) > Site->Capacity) return ERole::Warn;
            return ERole::Muted;
        };
        List->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBox).HeightOverride(58.f).Visibility_Lambda([SiteAt, Depot] { return SiteAt(Depot) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SBorder).BorderImage(&SmallBrush).Padding(FMargin(12.f, 6.f)).VAlign(VAlign_Center)
                .BorderBackgroundColor(ColBy([this, Depot] { return DepotSel == Depot ? ERole::Line : ERole::Inset; }))
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                    [
                        SNew(SVerticalBox).ToolTip(Tip([G, Depot] { return G() ? MarketDepots::Describe(G()->State, Depot) : FString(); }))
                        + SVerticalBox::Slot().AutoHeight()[ TextPx([G, Depot] { return G() ? MarketDepots::DepotName(G()->State, Depot) : FString(); }, 13.f, [] { return ERole::Text; }, true) ]
                        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)[ TextPx(Line, 11.f, LineRole) ]
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                    [
                        SNew(SBox).ToolTip(Tip([] { return FString(TEXT("Ana ekranda deponun 600 km menzilini g\u00f6ster.")); }))
                        [ Button([] { return FString(TEXT("Haritada")); }, [this, G, SiteAt, Depot]
                        {
                            const FMarketDepot* Site = SiteAt(Depot);
                            if (!Site) return;
                            DepotSel = Depot;
                            MapLayer = 0;
                            MapCountry = Site->Country;
                            MapRegion.Reset();
                            MapProvinceId = Site->Province;
                            if (G() && Site->Country == MarketCountry::MapCountry()) G()->MapProvince = MarketMenuPagesUi::MapIndexOf(Site->Province);
                            Go(Summary);
                        }) ]
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f, 0.f, 0.f)
                    [
                        SNew(SBox).ToolTip(Tip([G, Boss] { return G() && Boss() != INDEX_NONE ? MarketManagers::DescribeManager(G()->State, Boss())
                            : FString(TEXT("3 d\u0131\u015f aday aras\u0131ndan bir depo m\u00fcd\u00fcr\u00fc se\u00e7. Becerisi fireyi, eksik ve k\u0131r\u0131k teslimat\u0131 azalt\u0131r.")); }))
                        [ Button([Boss] { return FString(Boss() == INDEX_NONE ? TEXT("M\u00fcd\u00fcr ata") : TEXT("Y\u00f6netim")); }, [this, Boss, Depot]
                        {
                            if (Boss() == INDEX_NONE) { DepotSel = Depot; DepotLayer = 2; }
                            else { BranchTab = 2; AppointArea = INDEX_NONE; }
                        }) ]
                    ]
                ]
            ]
        ];
    }
    auto Suggestion = [G]() -> MarketDepots::FAdvice
    {
        return G() ? MarketDepots::SuggestDepotProvince(G()->State, G()->State.CountryId) : MarketDepots::FAdvice();
    };
    auto FirstWhy = [G, Sites, Suggestion]() -> FString
    {
        if (!G()) return FString();
        if (Sites() > 0)
        {
            // Open branches outside every depot's range (the home province's own wholesaler aside).
            const int32 Far = MarketMenuPagesUi::BranchesFarFromDepot(G()->State, MarketDepots::AllLinks(G()->State));
            return Far > 0 ? FString::Printf(TEXT("%d ma\u011fazan depoya uzak: mal\u0131 toptanc\u0131dan al\u0131yor (+%%3)."), Far) : FString();
        }
        const MarketDepots::FAdvice Advice = Suggestion();
        if (Advice.Province.IsEmpty()) return Advice.Text;
        FString Reason;
        MarketDepots::CanBuild(G()->State, Advice.Country, Advice.Province, Reason);
        return Reason;
    };
    TSharedRef<SWidget> Depots = SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Section(TEXT("DEPOLAR")) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
            [
                SNew(SBox).ToolTip(Tip([] { return FString(TEXT("Her depoya giden ma\u011faza bir y\u00fck, her 300 km bir y\u00fck daha; bir kamyon g\u00fcnde 8 y\u00fck ta\u015f\u0131r. Kamyon eksikse teslimat gecikir ve eksik gelir.")); }))
                [ Mono([G] { return G() ? FString::Printf(TEXT("Kamyon %d / gereken %d"), G()->State.Company.Trucks, MarketDepots::TrucksNeeded(G()->State)) : FString(); }, 12.f,
                    [G] { return G() && G()->State.Company.Trucks < MarketDepots::TrucksNeeded(G()->State) ? ERole::Warn : ERole::Muted; }) ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ More([] { return FString(TEXT("B\u00fcy\u00fck depo bir ile kurulur ve \u00e7evresindeki illere hizmet verir. Her ma\u011faza kendi \u00fclkesindeki en yak\u0131n depodan mal\u0131n\u0131 al\u0131r: 600 km'den uzaksa (ev ilinde 200 km) depo hizmet vermez, mal toptanc\u0131dan gelir (ev ilinin d\u0131\u015f\u0131nda +%3).\n\nDepo zincir indirimi getirir (tam verimde mal\u0131n %1,5'i). Yol: ilk 100 km bedava, sonra her 100 km +%0,6.\n\nDepo m\u00fcd\u00fcr\u00fc verimi belirler: becerisi fireyi, eksik/k\u0131r\u0131k teslimat\u0131 ve raf bulunurlu\u011funu etkiler. M\u00fcd\u00fcrs\u00fcz depo yar\u0131 verimle \u00e7al\u0131\u015f\u0131r. D\u00fcr\u00fcst olmayan m\u00fcd\u00fcr maldan ka\u00e7\u0131r\u0131r; \u00fclke m\u00fcd\u00fcr\u00fc ya da sen yakalayabilirsin.\n\nBir depo 60 ma\u011fazaya rahat bakar; fazlas\u0131 onu yava\u015flat\u0131r. Kira her ay kasadan \u00e7\u0131kar.")); }) ]
        ]
        + SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 8.f, 0.f, 0.f)
        [
            SNew(SScrollBox)
            + SScrollBox::Slot()[ List ]
            + SScrollBox::Slot()
            [
                SNew(SBox).Visibility_Lambda([Sites] { return Sites() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
                [ Fixed(TEXT("Hen\u00fcz depo yok: ma\u011fazalar mal\u0131 toptanc\u0131dan al\u0131r. En az 4 ma\u011fazadan sonra bir ile depo kurabilirsin; \u00e7evresindeki 600 km'ye hizmet verir."), 10, ERole::Muted) ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Why(FirstWhy) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
            [ Button([] { return FString(TEXT("Depo kur \u203a")); }, [this] { DepotLayer = 1; }, true) ]
        ];

    // ---- Layer 1: the province of a new depot (the suggested one first).
    struct FChoiceCache
    {
        uint64 Frame = 0;
        bool bFilled = false;
        TArray<FDepotChoice> Choices;
    };
    TSharedRef<FChoiceCache> Cache = MakeShared<FChoiceCache>();
    auto ChoiceAt = [this, G, Cache](int32 Slot) -> const FDepotChoice*
    {
        if (DepotLayer != 1 || !G()) return nullptr;
        if (!Cache->bFilled || Cache->Frame != GFrameCounter)
        {
            Cache->bFilled = true;
            Cache->Frame = GFrameCounter;
            Cache->Choices = MarketMenuPagesUi::DepotChoices(G()->State, 16);
        }
        return Cache->Choices.IsValidIndex(Slot) ? &Cache->Choices[Slot] : nullptr;
    };
    TSharedRef<SVerticalBox> Provinces = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 16; ++Slot)
    {
        auto Option = [ChoiceAt, Slot] { return ChoiceAt(Slot); };
        auto Reason = [G, Option]() -> FString
        {
            const FDepotChoice* C = Option();
            FString Text;
            if (C && G()) MarketDepots::CanBuild(G()->State, C->Country, C->Province, Text);
            return Text;
        };
        auto Sub = [G, Option]() -> FString
        {
            const FDepotChoice* C = Option();
            if (!C || !G()) return FString();
            FString Text = FString::Printf(TEXT("%d ma\u011faza 600 km i\u00e7inde \u00b7 kurulum %s \u00b7 kira %s/ay"), C->Reach,
                *MarketMenuUi::Tl(MarketDepots::BuildCost(G()->State, C->Country, C->Province)), *MarketMenuUi::Tl(MarketDepots::MonthlyRent(G()->State, C->Country, C->Province)));
            return Text;
        };
        Provinces->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBox).HeightOverride(56.f).Visibility_Lambda([Option] { return Option() ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SBorder).BorderImage(&SmallBrush).Padding(FMargin(12.f, 6.f)).VAlign(VAlign_Center)
                .BorderBackgroundColor(ColBy([Option] { const FDepotChoice* C = Option(); return C && C->bSuggested ? ERole::AccentSoft : ERole::Inset; }))
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                    [
                        SNew(SVerticalBox).ToolTip(Tip([Option] { const FDepotChoice* C = Option(); return C && C->bSuggested ? C->Advice : FString(TEXT("Depo bu ile kurulursa 600 km \u00e7evresindeki ma\u011fazalar mal\u0131 buradan al\u0131r.")); }))
                        + SVerticalBox::Slot().AutoHeight()
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                            [ TextPx([Option] { const FDepotChoice* C = Option(); return C ? MarketMenuPagesUi::CityName(C->Country, C->Province) : FString(); }, 13.f, [] { return ERole::Text; }, true) ]
                            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                            [
                                SNew(SBorder).BorderImage(&BadgeBrush).BorderBackgroundColor(Col(ERole::Solid)).Padding(FMargin(7.f, 1.f))
                                .Visibility_Lambda([Option] { const FDepotChoice* C = Option(); return C && C->bSuggested ? EVisibility::Visible : EVisibility::Collapsed; })
                                [ TextPx([] { return FString(TEXT("\u00f6neri")); }, 10.f, [] { return ERole::Accent; }, true) ]
                            ]
                            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                            [ Mono([G, Option]
                            {
                                const FDepotChoice* C = Option();
                                if (!C || !C->bSuggested || !G()) return FString();
                                const MarketDepots::FAdvice Advice = MarketDepots::SuggestDepotProvince(G()->State, C->Country);
                                return Advice.MonthlyGain != 0 ? FString::Printf(TEXT("tahmini %s%s/ay"), Advice.MonthlyGain > 0 ? TEXT("+") : TEXT(""), *MarketMenuUi::Tl(Advice.MonthlyGain)) : FString();
                            }, 12.f, [] { return ERole::Accent; }) ]
                        ]
                        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)[ TextPx(Sub, 11.f, [] { return ERole::Muted; }) ]
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                    [
                        SNew(SBox).ToolTip(Tip([Reason] { const FString Text = Reason(); return Text.IsEmpty() ? FString(TEXT("Depoyu bu ile kur (\u00f6nce sorar).")) : Text; }))
                        [ Button([] { return FString(TEXT("Kur")); }, [this, G, Option]
                        {
                            const FDepotChoice* C = Option();
                            if (!C || !G()) return;
                            const FString Country = C->Country, Province = C->Province;
                            const FMarketState& S = G()->State;
                            Ask(FString::Printf(TEXT("Depo %s iline kurulsun mu? Kurulum %s kasadan \u00e7\u0131kar, ayl\u0131k kira %s. 600 km \u00e7evresindeki %d ma\u011faza mal\u0131 buradan al\u0131r. M\u00fcd\u00fcrs\u00fcz depo yar\u0131 verimle \u00e7al\u0131\u015f\u0131r: kurunca bir depo m\u00fcd\u00fcr\u00fc se\u00e7."),
                                *MarketMenuPagesUi::CityName(Country, Province), *MarketMenuUi::Tl(MarketDepots::BuildCost(S, Country, Province)),
                                *MarketMenuUi::Tl(MarketDepots::MonthlyRent(S, Country, Province)), C->Reach),
                                [this, G, Country, Province]
                                {
                                    Manage(TEXT("BuildDepotIn"), MarketManagers::EncodeArea(MarketManagers::ELevel::Depot, Country, Province));
                                    const int32 Built = G() ? MarketDepots::Find(G()->State, Country, Province) : INDEX_NONE;
                                    // Built: straight to its manager's candidates.
                                    if (Built != INDEX_NONE) { DepotSel = Built; DepotLayer = 2; }
                                    else DepotLayer = 0;
                                });
                        }, true, [Reason] { return Reason().IsEmpty(); }) ]
                    ]
                ]
            ]
        ];
    }
    TSharedRef<SWidget> Builder = SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[ Section(TEXT("DEPO KUR \u00b7 \u0130L SE\u00c7")) ]
            + SHorizontalBox::Slot().AutoWidth()[ Button([] { return FString(TEXT("Vazge\u00e7")); }, [this] { DepotLayer = 0; }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 6.f)
        [ SNew(SBox).HeightOverride(34.f).VAlign(VAlign_Top)
            [ Fixed(TEXT("\u00d6nerilen il, ma\u011fazalar\u0131na cirolar\u0131na g\u00f6re en yak\u0131n yer. Depo 600 km \u00e7evresine hizmet verir; ilk 100 km bedava, sonra her 100 km +%0,6."), 10, ERole::Muted) ] ]
        + SVerticalBox::Slot().FillHeight(1.f)
        [
            SNew(SScrollBox)
            + SScrollBox::Slot()[ Provinces ]
            + SScrollBox::Slot()
            [
                SNew(SBox).Visibility_Lambda([ChoiceAt] { return ChoiceAt(0) ? EVisibility::Collapsed : EVisibility::Visible; })
                [ Fixed(TEXT("Depo kurulacak il yok: \u00f6nce \u015fube a\u00e7."), 10, ERole::Muted) ]
            ]
        ];

    // ---- Layer 2: the manager of the chosen depot.
    auto DepotArea = [this, SiteAt]() -> int32
    {
        const FMarketDepot* Site = SiteAt(DepotSel);
        return Site ? MarketManagers::EncodeArea(MarketManagers::ELevel::Depot, Site->Country, Site->Province) : INDEX_NONE;
    };
    TSharedRef<SWidget> Hiring = SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [ Label([this, G] { return G() && G()->State.Company.DepotSites.IsValidIndex(DepotSel) ? FString::Printf(TEXT("%s \u0130\u00c7\u0130N DEPO M\u00dcD\u00dcR\u00dc"), *MarketDepots::DepotName(G()->State, DepotSel).ToUpper()) : FString(); }, 9, ERole::Muted, true) ]
            + SHorizontalBox::Slot().AutoWidth()[ Button([] { return FString(TEXT("Vazge\u00e7")); }, [this] { DepotLayer = 0; }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 10.f)
        [ Label([this, G]() -> FString
        {
            if (!G() || !G()->State.Company.DepotSites.IsValidIndex(DepotSel)) return FString();
            const int32 Served = MarketDepots::Served(G()->State, DepotSel);
            return FString::Printf(TEXT("Depo %d ma\u011fazaya mal g\u00f6nderiyor; iyi \u00e7al\u0131\u015fmas\u0131 i\u00e7in beceri %d gerekir. Becerisi fireyi, eksik/k\u0131r\u0131k teslimat\u0131 ve raf bulunurlu\u011funu belirler. \u00dclke m\u00fcd\u00fcr\u00fcne, o yoksa sana ba\u011fl\u0131d\u0131r."),
                Served, 40 + FMath::Min(30, Served / 3));
        }, 10, ERole::Text, false, true) ]
        + SVerticalBox::Slot().AutoHeight()[ CandidateCards(DepotArea, [this] { DepotLayer = 0; }) ];

    return Card(SNew(SBox).HeightOverride(440.f)
    [
        SNew(SWidgetSwitcher).WidgetIndex_Lambda([this, SiteAt] { return DepotLayer == 2 && SiteAt(DepotSel) ? 2 : DepotLayer == 1 ? 1 : 0; })
        + SWidgetSwitcher::Slot()[ Depots ]
        + SWidgetSwitcher::Slot()[ LayerFade(Builder) ]
        + SWidgetSwitcher::Slot()[ LayerFade(Hiring) ]
    ]);
}

// ---------------------------------------------------------------------------------------------------------------
// G-086 new game: country and province (no default, Mustafa 29.09.2026).

TSharedRef<SWidget> SMarketMenu::NewGameLayer()
{
    auto G = [this] { return Game.Get(); };
    auto Country = [this, G]() -> FString
    {
        if (!NewCountry.IsEmpty() && MarketCountry::Find(NewCountry)) return NewCountry;
        return G() ? G()->State.CountryId : MarketCountry::DefaultId();
    };
    auto Chosen = [this, Country]() -> const MarketCountry::FCity* { return NewCity.IsEmpty() ? nullptr : MarketCountry::FindCity(Country(), NewCity); };
    auto HasMap = [Country] { return MarketCountry::HasMap(Country()); };
    auto Forced = [G] { return G() && G()->bNeedStart; };

    // Countries.
    TSharedRef<SVerticalBox> Countries = SNew(SVerticalBox);
    for (const MarketCountry::FProfile& Pack : MarketCountry::All())
    {
        const FString Id = Pack.Id;
        const FString Line = FString::Printf(TEXT("%s \u00b7 %s \u00b7 %d il%s"), *Pack.CurrencySymbol,
            Pack.Character == MarketCountry::ECharacter::Stable ? TEXT("istikrarl\u0131") : Pack.Character == MarketCountry::ECharacter::Volatile ? TEXT("oynak") : TEXT("y\u00fcksek enflasyon"),
            Pack.Cities.Num(), Pack.bSundayClosed ? TEXT(" \u00b7 pazar kapal\u0131") : TEXT(""));
        const FString Name = Pack.Name;
        Countries->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
        [
            SNew(SButton).ButtonStyle(&RowStyle).IsFocusable(false).ContentPadding(FMargin(14.f, 10.f))
            .ButtonColorAndOpacity_Lambda([this, Country, Id] { return FSlateColor(Color(Country() == Id ? ERole::Inset : ERole::Panel)); })
            .OnClicked_Lambda([this, Id] { NewCountry = Id; NewCity.Reset(); NewSearch.Reset(); return FReply::Handled(); })
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ LabelBy([Name] { return Name; }, 14, [Country, Id] { return Country() == Id ? ERole::Accent : ERole::Text; }, true) ]
                + SVerticalBox::Slot().AutoHeight()[ Fixed(Line, 9, ERole::Muted) ]
            ]
        ];
    }

    // The map (the map's country, D3) or the provinces as buttons, and the search.
    auto Fill = [this](int32 Index) -> FLinearColor
    {
        const MarketMapData::FData& Data = MarketMapData::Get();
        const FLinearColor Base = Color(ERole::Inset);
        const MarketCountry::FCity* City = Data.Provinces.IsValidIndex(Index) ? MarketCountry::FindCity(MarketCountry::MapCountry(), Data.Provinces[Index].Id) : nullptr;
        if (!City) return Base;
        if (NewLayer == 1) return FLinearColor::LerpUsingHSV(Base, Color(ERole::Good), FMath::Clamp((City->Income - 0.65f) / 0.8f, 0.f, 1.f) * 0.8f);
        if (NewLayer == 2) return FLinearColor::LerpUsingHSV(Base, Color(ERole::Warn), FMath::Clamp((City->Rent - 0.5f) / 1.4f, 0.f, 1.f) * 0.8f);
        return FLinearColor::LerpUsingHSV(Base, Color(ERole::Bad), 0.1f + 0.7f * FMath::Clamp((City->Competition - 0.6f) / 0.9f, 0.f, 1.f));
    };
    TSharedRef<SWidget> Map = SNew(SMarketMap)
        .FillOf(Fill)
        .LineColor([this] { return Color(ERole::Solid); })
        .Selected([this, HasMap] { return HasMap() && !NewCity.IsEmpty() ? MarketMenuPagesUi::MapIndexOf(NewCity) : INDEX_NONE; })
        .HasPin([this](int32 Index) { const MarketMapData::FData& Data = MarketMapData::Get(); return Data.Provinces.IsValidIndex(Index) && Data.Provinces[Index].Id == NewCity; })
        .PinColor([this](int32) { return Color(ERole::Accent); })
        .StrongColor([this] { return Color(ERole::Text); })
        .OnPick([this](int32 Index) { const MarketMapData::FData& Data = MarketMapData::Get(); if (Data.Provinces.IsValidIndex(Index)) { NewCountry = MarketCountry::MapCountry(); NewCity = Data.Provinces[Index].Id; } });
    TSharedRef<SWrapBox> List = SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 6.f));
    for (const MarketCountry::FProfile& Pack : MarketCountry::All())
    {
        const FString PackId = Pack.Id;
        for (const MarketCountry::FCity& City : Pack.Cities)
        {
            const FString Id = City.Id, Name = City.Name;
            List->AddSlot()
            [
                SNew(SBox).Visibility_Lambda([this, Country, HasMap, PackId, Name]
                {
                    if (Country() != PackId) return EVisibility::Collapsed;
                    if (!NewSearch.IsEmpty()) return Name.Contains(NewSearch) ? EVisibility::Visible : EVisibility::Collapsed;
                    return HasMap() ? EVisibility::Collapsed : EVisibility::Visible;
                })
                [ Choice(Name, [this, Id] { return NewCity == Id; }, [this, PackId, Id] { NewCountry = PackId; NewCity = Id; }) ]
            ];
        }
    }

    auto StatLine = [this](const FString& Name, TFunction<FString()> Value) -> TSharedRef<SWidget>
    {
        return SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f)[ Fixed(Name, 11, ERole::Muted) ]
            + SHorizontalBox::Slot().AutoWidth()[ Label(Value, 11, ERole::Text, true) ];
    };

    return SNew(SOverlay)
        .Visibility_Lambda([G] { return G() && (G()->bNeedStart || G()->bNewGameAsk) ? EVisibility::Visible : EVisibility::Collapsed; })
        + SOverlay::Slot()[ SNew(SBorder).BorderImage(&FlatBrush).BorderBackgroundColor(Col(ERole::Solid)) ]
        + SOverlay::Slot().Padding(FMargin(40.f, 32.f))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 20.f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("YEN\u0130 OYUN"), 10, ERole::Accent, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)[ Fixed(TEXT("Marketin nerede?"), 30, ERole::Text, true) ]
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 6.f, 0.f)[ Choice(TEXT("1 \u00b7 \u00dclke"), [] { return true; }, [] {}) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 6.f, 0.f)[ Choice(TEXT("2 \u00b7 \u0130l"), [Chosen] { return Chosen() != nullptr; }, [] {}) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 6.f, 0.f)[ Choice(TEXT("3 \u00b7 Market"), [this] { return !NewMarketName.TrimStartAndEnd().IsEmpty(); }, [] {}) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Choice(TEXT("4 \u00b7 Ba\u015fla"), [] { return false; }, [] {}) ]
            ]
            + SVerticalBox::Slot().FillHeight(1.f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth()
                [
                    SNew(SBox).WidthOverride(270.f)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[ Section(TEXT("\u00dcLKE")) ]
                        + SVerticalBox::Slot().AutoHeight()[ Countries ]
                        + SVerticalBox::Slot().FillHeight(1.f)[ SNew(SSpacer) ]
                        + SVerticalBox::Slot().AutoHeight()
                        [ Fixed(TEXT("Oyunun kendi ekonomisi var: kurlar, enflasyon ve markalar kurgudur. \u00dclke; para birimini, \u00f6zel g\u00fcnleri, yasalar\u0131 ve rakipleri belirler."), 9, ERole::Muted) ]
                    ]
                ]
                + SHorizontalBox::Slot().FillWidth(1.f).Padding(20.f, 0.f)
                [
                    SNew(SBorder).BorderImage(&CardBrush).BorderBackgroundColor(Col(ERole::Panel)).Padding(FMargin(16.f, 14.f))
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)[ Fixed(TEXT("\u0130l ara"), 10, ERole::Muted) ]
                            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                            [
                                SNew(SBox).WidthOverride(220.f)
                                [
                                    SNew(SEditableTextBox).Font(MarketMenuUi::MenuFont(false, 11))
                                    .HintText(FText::FromString(TEXT("\u00f6r. Van")))
                                    .Text_Lambda([this] { return FText::FromString(NewSearch); })
                                    .OnTextChanged_Lambda([this](const FText& Text) { NewSearch = Text.ToString(); })
                                ]
                            ]
                            + SHorizontalBox::Slot().FillWidth(1.f)[ SNew(SSpacer) ]
                            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 6.f, 0.f)
                            [ SNew(SBox).Visibility_Lambda([HasMap] { return HasMap() ? EVisibility::Visible : EVisibility::Collapsed; })[ Fixed(TEXT("Renk:"), 10, ERole::Muted) ] ]
                            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)
                            [ SNew(SBox).Visibility_Lambda([HasMap] { return HasMap() ? EVisibility::Visible : EVisibility::Collapsed; })[ Choice(TEXT("Rekabet"), [this] { return NewLayer == 0; }, [this] { NewLayer = 0; }) ] ]
                            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)
                            [ SNew(SBox).Visibility_Lambda([HasMap] { return HasMap() ? EVisibility::Visible : EVisibility::Collapsed; })[ Choice(TEXT("Al\u0131m g\u00fcc\u00fc"), [this] { return NewLayer == 1; }, [this] { NewLayer = 1; }) ] ]
                            + SHorizontalBox::Slot().AutoWidth()
                            [ SNew(SBox).Visibility_Lambda([HasMap] { return HasMap() ? EVisibility::Visible : EVisibility::Collapsed; })[ Choice(TEXT("Kira"), [this] { return NewLayer == 2; }, [this] { NewLayer = 2; }) ] ]
                        ]
                        + SVerticalBox::Slot().FillHeight(1.f)
                        [
                            SNew(SWidgetSwitcher).WidgetIndex_Lambda([HasMap] { return HasMap() ? 0 : 1; })
                            + SWidgetSwitcher::Slot()[ Map ]
                            + SWidgetSwitcher::Slot()[ SNew(SSpacer) ]
                        ]
                        + SVerticalBox::Slot().AutoHeight().MaxHeight(260.f).Padding(0.f, 8.f, 0.f, 0.f)[ SNew(SScrollBox) + SScrollBox::Slot()[ List ] ]
                        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
                        [ Fixed(TEXT("Varsay\u0131lan il yok: haritada bir ile t\u0131kla ya da ara. K\u00fc\u00e7\u00fck il sakin ba\u015flar ama m\u00fc\u015fteri fiyata duyarl\u0131d\u0131r; b\u00fcy\u00fck ilde kira ve rakip \u00e7oktur."), 10, ERole::Muted) ]
                    ]
                ]
                + SHorizontalBox::Slot().AutoWidth()
                [
                    SNew(SBox).WidthOverride(340.f)
                    [
                        SNew(SBorder).BorderImage(&CardBrush).BorderBackgroundColor(Col(ERole::Panel)).Padding(FMargin(20.f, 18.f))
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot().AutoHeight()[ Label([Chosen] { const MarketCountry::FCity* C = Chosen(); return C ? C->Name : FString(TEXT("\u0130l se\u00e7ilmedi")); }, 26, ERole::Text, true) ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 12.f)
                            [ Label([this, Country, Chosen]
                            {
                                const MarketCountry::FCity* C = Chosen();
                                if (!C) return FString(TEXT("Haritadan ya da listeden bir il se\u00e7."));
                                const MarketCountry::FRegion* Sub = MarketCountry::SubRegionOf(Country(), C->Id);
                                const MarketCountry::FRegion* Main = MarketCountry::RegionOf(Country(), C->Id);
                                return Sub ? (Main && Main->Id != Sub->Id ? Sub->Name + TEXT(" \u00b7 ") + Main->Name : Sub->Name) : FString();
                            }, 10, ERole::Muted, false, true) ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)[ StatLine(TEXT("N\u00fcfus"), [Chosen] { const MarketCountry::FCity* C = Chosen(); return C ? MarketMenuPagesUi::PeopleText(C->PopulationK) : FString(TEXT("\u2014")); }) ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)[ StatLine(TEXT("Al\u0131m g\u00fcc\u00fc"), [Chosen] { const MarketCountry::FCity* C = Chosen(); return C ? MarketMenuPagesUi::Level(C->Income, TEXT("d\u00fc\u015f\u00fck"), TEXT("orta"), TEXT("y\u00fcksek")) : FString(TEXT("\u2014")); }) ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)[ StatLine(TEXT("Kira"), [Chosen] { const MarketCountry::FCity* C = Chosen(); return C ? MarketMenuPagesUi::Level(C->Rent, TEXT("ucuz"), TEXT("orta"), TEXT("pahal\u0131")) : FString(TEXT("\u2014")); }) ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)[ StatLine(TEXT("Rekabet"), [Chosen] { const MarketCountry::FCity* C = Chosen(); return C ? MarketMenuPagesUi::Level(C->Competition, TEXT("az"), TEXT("orta"), TEXT("yo\u011fun")) : FString(TEXT("\u2014")); }) ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)
                            [ StatLine(TEXT("Ma\u011faza yeri"), [this, G, Country, Chosen] { const MarketCountry::FCity* C = Chosen(); return C && G() ? FString::FromInt(MarketBranches::Room(MarketBranches::SiteOf(G()->State, Country(), C->Id))) : FString(TEXT("\u2014")); }) ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 6.f)[ Section(TEXT("NE BEKLEMEL\u0130")) ]
                            + SVerticalBox::Slot().AutoHeight()[ Label([Chosen] { const MarketCountry::FCity* C = Chosen(); return C ? MarketMenuPagesUi::Expectation(*C) : FString(); }, 11, ERole::Text, false, true) ]
                            + SVerticalBox::Slot().FillHeight(1.f)[ SNew(SSpacer) ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Section(TEXT("MARKET\u0130N ADI")) ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
                            [
                                SNew(SEditableTextBox).Font(MarketMenuUi::MenuFont(false, 12))
                                .HintText(FText::FromString(TEXT("\u00f6r. Y\u0131ld\u0131z Market")))
                                .Text_Lambda([this] { return FText::FromString(NewMarketName); })
                                .OnTextChanged_Lambda([this](const FText& Text) { NewMarketName = Text.ToString().Left(24); })
                            ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Section(TEXT("SEN\u0130N ADIN")) ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
                            [
                                SNew(SEditableTextBox).Font(MarketMenuUi::MenuFont(false, 12))
                                .HintText(FText::FromString(TEXT("iste\u011fe ba\u011fl\u0131")))
                                .Text_Lambda([this] { return FText::FromString(NewPlayerName); })
                                .OnTextChanged_Lambda([this](const FText& Text) { NewPlayerName = Text.ToString().Left(MarketStart::MaxPlayerNameLength); })
                            ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Section(TEXT("ZORLUK")) ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
                            [
                                SNew(SHorizontalBox)
                                + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)[ Choice(TEXT("Rahat"), [this] { return NewDifficulty == 0; }, [this] { NewDifficulty = 0; }) ]
                                + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)[ Choice(TEXT("Normal"), [this] { return NewDifficulty == 1; }, [this] { NewDifficulty = 1; }) ]
                                + SHorizontalBox::Slot().AutoWidth()[ Choice(TEXT("Zor"), [this] { return NewDifficulty == 2; }, [this] { NewDifficulty = 2; }) ]
                            ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
                            [
                                SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 10.f))
                                [ Fixed(TEXT("\u0130l\u00e7edeki tek \u015fubeli k\u00fc\u00e7\u00fck bir market. Onu i\u015fleten aile yoruldu ve i\u015fi sana devretti: bina, 1 kasiyer, 2 reyon g\u00f6revlisi, toptanc\u0131ya hafif bir bor\u00e7 ve yar\u0131 dolu raflar. Sonras\u0131 senin hikayen."), 10, ERole::Text) ]
                            ]
                            + SVerticalBox::Slot().AutoHeight()
                            [
                                RiskyButton([this, Chosen] { const MarketCountry::FCity* C = Chosen(); return !C ? FString(TEXT("\u00d6nce bir il se\u00e7")) : NewMarketName.TrimStartAndEnd().IsEmpty() ? FString(TEXT("Marketine bir ad ver")) : FString::Printf(TEXT("%s'de ba\u015fla"), *C->Name); },
                                    [G, Forced, Chosen]
                                    {
                                        const MarketCountry::FCity* C = Chosen();
                                        if (!C || !G()) return FString();
                                        return Forced() ? FString::Printf(TEXT("Oyun %s'de ba\u015flas\u0131n m\u0131?"), *C->Name)
                                            : FString::Printf(TEXT("%d. yuvadaki kampanya silinir ve %s'de yeni oyun ba\u015flar. Emin misin?"), G()->ActiveSlot, *C->Name);
                                    },
                                    [this, G, Country, Chosen] { const MarketCountry::FCity* C = Chosen(); if (G() && C) G()->StartNewCampaign(Country(), C->Id, NewMarketName, NewPlayerName, NewDifficulty); },
                                    [this, Chosen] { return Chosen() != nullptr && !NewMarketName.TrimStartAndEnd().IsEmpty(); })
                            ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
                            [
                                SNew(SBox).Visibility_Lambda([Forced] { return Forced() ? EVisibility::Collapsed : EVisibility::Visible; })
                                [ Button([] { return FString(TEXT("Vazge\u00e7  (Esc)")); }, [G] { if (G()) G()->bNewGameAsk = false; }) ]
                            ]
                        ]
                    ]
                ]
            ]
        ];
}

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
        // Milestones already written as memories (bit flags; free bits are simply unused).
        BIdentityOffered = 1ll << 4,
        BIdentity = 1ll << 6, BFirstOrder = 1ll << 7, BFirstProfit = 1ll << 8, BDebt = 1ll << 9, BShare35 = 1ll << 10,
        BFirstEmployee = 1ll << 11, BSecondStore = 1ll << 12, BFirstWeek = 1ll << 14,
    };

    bool Has(const FMarketState& State, int64 Beat) { return (State.Story.Beats & Beat) != 0; }
    void Mark(FMarketState& State, int64 Beat) { State.Story.Beats |= Beat; }

    // A branch signed, being fitted out or open.
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
            FString(TEXT("Market ayakta. \u015eimdi ne olaca\u011f\u0131na karar ver; a\u00e7aca\u011f\u0131n b\u00fct\u00fcn ma\u011fazalar da bu kimli\u011fi ta\u015f\u0131r. Hi\u00e7biri her ko\u015fulda \u00fcst\u00fcn de\u011fil.")),
            { FString(TEXT("Mahalle Marketi: g\u00fcler y\u00fcz ve sad\u0131k m\u00fc\u015fteri")), FString(TEXT("Kaliteli Market: iyi mal, iyi fiyat")), FString(TEXT("H\u0131zl\u0131 \u0130ndirim: ucuz al, ucuz sat, \u00e7ok sat")) },
            0, 3));
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

void MarketStory::AddMemory(FMarketState& State, const FString& Text)
{
    const FString Date = MarketCalendar::DateText(FMath::Max(1, State.Day - 1));
    State.Story.Memories.Add(Date + TEXT(": ") + Text);
    State.DayNews.Add(TEXT("Hat\u0131ra: ") + Text + TEXT("."));
}

bool MarketStory::StoryClosed(const FMarketState& State)
{
    return State.Story.bCampaignOver;
}

void MarketStory::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    (void)Products;
    const int32 Closed = State.Day - 1;
    if (Closed < 1 || StoryClosed(State)) return;
    // Milestones (M69: the company's own history; nothing here unlocks anything).
    if (State.LastPurchases > 0 && !Has(State, BFirstOrder)) { Mark(State, BFirstOrder); AddMemory(State, TEXT("ilk sipari\u015f")); }
    if (State.LastProfit > 0 && !Has(State, BFirstProfit)) { Mark(State, BFirstProfit); AddMemory(State, TEXT("ilk k\u00e2rl\u0131 g\u00fcn")); }
    if (!MarketCampaign::DebtOpen(State) && !Has(State, BDebt)) { Mark(State, BDebt); AddMemory(State, TEXT("devral\u0131nan bor\u00e7 kapand\u0131")); }
    if (State.MarketShare >= MarketCampaign::ShareGoal(State) && !Has(State, BShare35)) { Mark(State, BShare35); AddMemory(State, TEXT("\u00e7evrede say\u0131lan bir market olduk")); }
    if (MarketStaff::Count(State, MarketStaff::ERole::Cashier) + MarketStaff::Count(State, MarketStaff::ERole::Stocker) > 0 && !Has(State, BFirstEmployee))
    {
        Mark(State, BFirstEmployee);
        AddMemory(State, TEXT("ilk \u00e7al\u0131\u015fan"));
    }
    if (HasBranch(State) && !Has(State, BSecondStore)) { Mark(State, BSecondStore); AddMemory(State, TEXT("ikinci tabela as\u0131ld\u0131")); }
    if (Closed >= 7 && !Has(State, BFirstWeek)) { Mark(State, BFirstWeek); AddMemory(State, TEXT("ilk hafta bitti")); }

    // In the first weeks the market chooses what it wants to be (once; the default keeps the neighbourhood market).
    if (State.Story.Identity == 0 && !Has(State, BIdentityOffered) && !Has(State, BIdentity) && State.Day >= IdentityDay)
    {
        Mark(State, BIdentityOffered);
        OfferIdentity(State);
    }
}

bool MarketStory::Resolve(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& D, int32 Option, FString& OutMessage)
{
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
        AddMemory(State, FString::Printf(TEXT("marketin kimli\u011fi: %s"), *IdentityName(Identity)));
        return true;
    }
    OutMessage = TEXT("Bu karar art\u0131k ge\u00e7erli de\u011fil.");
    return true;
}

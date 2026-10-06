#include "MarketResponse.h"
#include "MarketBranches.h"
#include "MarketCountry.h"
#include "MarketEconomy.h"
#include "MarketEras.h"
#include "MarketEvents.h"
#include "MarketManagers.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketResponseTest
{
    using MarketResponse::EKind;

    FMarketState Start()
    {
        FMarketState S; S.Initialize(TArray<FMarketProduct>());
        S.RivalSeed = 7; S.Day = 900; S.Cash = 500000000;
        S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
        S.Decisions.Reset();
        // No crisis card unless a test asks for one.
        MarketEras::FEra Era;
        if (MarketEras::Current(S, S.Day, Era)) S.Responses.EraSeen = static_cast<int32>(Era.Kind) * 10 + Era.Wave;
        return S;
    }
    TArray<FString> Towns()
    {
        TArray<FString> Out;
        if (const MarketCountry::FProfile* Pack = MarketCountry::Find(TEXT("tr")))
            for (const MarketCountry::FCity& City : Pack->Cities)
                if (City.Id != TEXT("kirklareli") && Out.Num() < 2) Out.Add(City.Id);
        return Out;
    }
    void AddShops(FMarketState& S, const FString& Province, int32 Count, int64 Revenue, const FString& Manager = FString(), uint8 Style = 0)
    {
        for (int32 I = 0; I < Count; ++I)
        {
            FMarketBranch& B = S.Branches.AddDefaulted_GetRef();
            B.Country = TEXT("tr"); B.Province = Province; B.Format = TEXT("mahalle");
            B.Name = FString::Printf(TEXT("Test %d"), S.Branches.Num());
            B.Stage = static_cast<uint8>(MarketBranches::EStage::Open);
            B.OpenedDay = 100; B.Last30Revenue = Revenue;
            B.ManagerName = Manager; B.ManagerStyle = Style; B.ManagerSkill = Manager.IsEmpty() ? 0 : 60;
        }
    }
    const FMarketDecision* Card(const FMarketState& S)
    {
        return S.Decisions.FindByPredicate([](const FMarketDecision& D) { return D.Id.StartsWith(TEXT("response.")); });
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketResponseLastWordTest, "MirasMarket.Response.ManagersAndLastWord", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketResponseLastWordTest::RunTest(const FString& Parameters)
{
    using namespace MarketResponseTest;
    const TArray<FString> T = Towns();
    if (!TestEqual(TEXT("Two towns in the pack"), T.Num(), 2)) return false;
    const FString Small = T[0], Big = T[1];
    FMarketState S = Start();
    AddShops(S, Small, 1, 50000);                                    // ~5 % of the revenue, nobody manages it
    AddShops(S, Big, 9, 100000, TEXT("Kaan Test"), static_cast<uint8>(MarketManagers::EStyle::Generous));
    TestTrue(TEXT("The small province's share"), MarketResponse::Impact(S, TEXT("tr"), Small) < MarketResponse::BigShare);
    TestTrue(TEXT("The big province's share"), MarketResponse::Impact(S, TEXT("tr"), Big) >= MarketResponse::BigShare);

    // Nobody to answer: the player decides, holding is the standing order.
    TestTrue(TEXT("Raised"), MarketResponse::Raise(S, EKind::War, TEXT("tr"), Small, TEXT("Rakip A"), S.Day + 20));
    const FMarketDecision* D = Card(S);
    TestTrue(TEXT("A card for the player"), D != nullptr && D->DefaultOption == MarketResponse::HoldOption(EKind::War) && D->Options.Num() == 3);
    FString Message;
    TestTrue(TEXT("Decided"), MarketEvents::Decide(S, TArray<FMarketProduct>(), 0, Message));
    TestEqual(TEXT("Logged"), MarketResponse::LogCount(S), 1);
    TestTrue(TEXT("By the player"), S.Responses.Log[0].bPlayer && S.Responses.ByPlayer == 1);
    TestEqual(TEXT("Our shelves cheaper there"), MarketResponse::PriceFactor(S, TEXT("tr"), Small, S.Day), MarketResponse::WarPrice);
    TestEqual(TEXT("Nothing elsewhere"), MarketResponse::PriceFactor(S, TEXT("tr"), Big, S.Day), 1.f);
    TestTrue(TEXT("The war counts as fought"), MarketResponse::Fought(S, TEXT("tr"), Small, S.Day + 20));
    TestEqual(TEXT("Over after the war"), MarketResponse::PriceFactor(S, TEXT("tr"), Small, S.Day + 21), 1.f);

    // A big province: the store manager proposes by his style (generous: service), the player has the last word.
    TestTrue(TEXT("Raised again"), MarketResponse::Raise(S, EKind::War, TEXT("tr"), Big, TEXT("Rakip B"), S.Day + 20));
    D = Card(S);
    TestTrue(TEXT("The proposal is the default"), D != nullptr && D->DefaultOption == 1);
    // The deadline passes: the proposal stands, as the manager's.
    S.Day = D->Deadline + 1;
    MarketEvents::Decide(S, TArray<FMarketProduct>(), D->DefaultOption, Message);
    TestEqual(TEXT("Applied when nobody answered"), MarketResponse::LogCount(S), 2);
    TestTrue(TEXT("As the manager's"), !S.Responses.Log.Last().bPlayer && S.Responses.Log.Last().Option == 1);
    TestTrue(TEXT("More shoppers there"), MarketResponse::PullFactor(S, TEXT("tr"), Big, S.Day) > 1.f);
    const int64 Cash = S.Cash;
    ++S.Day;
    MarketResponse::CloseDay(S);
    TestTrue(TEXT("Service costs every day"), S.Cash < Cash && S.Responses.Log.Last().Spent == Cash - S.Cash);

    // A small province with a store manager: settled below, no card.
    S.Branches[0].ManagerName = TEXT("Elif Test");
    S.Branches[0].ManagerStyle = static_cast<uint8>(MarketManagers::EStyle::PriceMinded);
    TestTrue(TEXT("Raised a third time"), MarketResponse::Raise(S, EKind::Opening, TEXT("tr"), Small, TEXT("Rakip C"), S.Day + 30));
    TestTrue(TEXT("No card"), Card(S) == nullptr);
    TestTrue(TEXT("The manager cut prices a little"), S.Responses.Log.Last().Option == 1 && S.Responses.Log.Last().Decider.Contains(TEXT("Elif Test")));
    TestTrue(TEXT("The log tells it"), MarketResponse::LogLine(S, 0).Contains(TEXT("Rakip C")) && MarketResponse::LogActive(S, 0));

    // A province manager comes before the store managers.
    FMarketManager& M = S.Management.Managers.AddDefaulted_GetRef();
    M.Level = static_cast<uint8>(MarketManagers::ELevel::Province); M.Country = TEXT("tr"); M.Area = Small; M.Name = TEXT("Ay\u015fe Test");
    M.Style = static_cast<uint8>(MarketManagers::EStyle::Careful);
    const MarketResponse::FDecider Who = MarketResponse::DeciderFor(S, TEXT("tr"), Small);
    TestTrue(TEXT("The province manager answers"), !Who.bPlayer && Who.Name == TEXT("Ay\u015fe Test") && MarketResponse::Proposal(EKind::War, Who) == 2);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketResponseMovesTest, "MirasMarket.Response.CrisisAndMoves", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketResponseMovesTest::RunTest(const FString& Parameters)
{
    using namespace MarketResponseTest;
    const TArray<FString> T = Towns();
    if (!TestEqual(TEXT("Two towns in the pack"), T.Num(), 2)) return false;
    FMarketState S = Start();
    AddShops(S, T[0], 3, 100000, TEXT("Kaan Test"), static_cast<uint8>(MarketManagers::EStyle::Careful));

    // A crisis always reaches the player; without a country manager the default is to carry on.
    TestTrue(TEXT("Crisis raised"), MarketResponse::Raise(S, EKind::Crisis, TEXT("tr"), FString(), TEXT("durgunluk"), S.Day + 200));
    const FMarketDecision* D = Card(S);
    TestTrue(TEXT("A crisis card"), D != nullptr && D->DefaultOption == 1);
    FString Message;
    MarketEvents::Decide(S, TArray<FMarketProduct>(), 0, Message);
    TestEqual(TEXT("Belt tightened: running costs"), MarketResponse::RunningFactor(S, S.Day), MarketResponse::CutRunning);
    TestEqual(TEXT("Smaller orders"), MarketResponse::OrderFactor(S, S.Day), MarketResponse::CutOrder);
    TestEqual(TEXT("A little less service everywhere"), MarketResponse::PullFactor(S, TEXT("tr"), T[1], S.Day), MarketResponse::CutPull);
    TestEqual(TEXT("No cheap leases"), MarketResponse::LeaseFactor(S), 1.f);
    MarketResponse::Answer(S, EKind::Crisis, TEXT("tr"), FString(), TEXT("durgunluk"), 2, TEXT("sen"), true, false, S.Day + 200);
    TestTrue(TEXT("Chances: cheaper leases and fit-outs"), MarketResponse::LeaseFactor(S) == MarketResponse::ChanceLease && MarketResponse::FitOutFactor(S) == MarketResponse::ChanceFitOut);

    // A price war against us is noticed at the day close, once.
    FMarketChain& C = S.Rivals.Chains.AddDefaulted_GetRef();
    C.Id = TEXT("test.war"); C.Name = TEXT("Rakip Zincir"); C.Country = TEXT("tr");
    C.Spots.AddDefaulted_GetRef().Province = T[0];
    C.Spots[0].Stores = 10;
    C.WarProvince = T[0]; C.WarUntil = S.Day + 15;
    const int32 Before = MarketResponse::LogCount(S);
    ++S.Day;
    MarketResponse::CloseDay(S);
    MarketResponse::CloseDay(S);
    TestTrue(TEXT("The war answered once (card or manager)"), MarketResponse::LogCount(S) + (Card(S) ? 1 : 0) == Before + 1);
    S.Decisions.Reset();

    // Rival openings where we are: looked at weekly against the last look.
    while (S.Day % MarketResponse::LookEvery != 0) ++S.Day;
    MarketResponse::CloseDay(S);
    const int32 Seen = MarketResponse::LogCount(S);
    TestTrue(TEXT("The first look only remembers"), S.Responses.RivalSeen.Contains(FString(TEXT("tr|")) + T[0]));
    S.Rivals.Chains[0].Spots[0].Stores = 14;
    S.Rivals.Chains[0].WarProvince.Reset();
    S.Day += MarketResponse::LookEvery;
    MarketResponse::CloseDay(S);
    TestTrue(TEXT("New rival stores answered"), MarketResponse::LogCount(S) + (Card(S) ? 1 : 0) > Seen);
    TestFalse(TEXT("The summary counts them"), MarketResponse::Summary(S).IsEmpty());
    return true;
}

#endif

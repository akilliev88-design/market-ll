#include "MarketRumors.h"
#include "MarketChains.h"
#include "MarketBanking.h"
#include "MarketBranches.h"
#include "MarketDepots.h"
#include "MarketCalendar.h"
#include "MarketEconomy.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketRumorsTest
{
    FMarketState Start(int32 Seed = 9)
    {
        FMarketState S; S.Initialize(TArray<FMarketProduct>());
        S.RivalSeed = Seed; S.Day = 100; S.Cash = 100000000;
        S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
        FMarketBranch& B = S.Branches.AddDefaulted_GetRef();
        B.Country = TEXT("tr"); B.Province = TEXT("kirklareli"); B.Format = TEXT("mahalle"); B.Name = TEXT("Test");
        B.Stage = static_cast<uint8>(MarketBranches::EStage::Open); B.OpenedDay = 1;
        MarketChains::Ensure(S);
        return S;
    }
    int32 National(const FMarketState& S)
    {
        return S.Rivals.Chains.IndexOfByPredicate([](const FMarketChain& C) { return C.Scope == static_cast<uint8>(MarketChains::EScope::National) && !C.bGone && C.Country == TEXT("tr"); });
    }
    void Days(FMarketState& S, int32 N)
    {
        for (int32 D = 0; D < N; ++D) { ++S.Day; S.DayNews.Reset(); MarketRumors::CloseDay(S); }
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketRumorsTest, "MirasMarket.Rumors.SourcesAndTruth", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketRumorsTest::RunTest(const FString& Parameters)
{
    using namespace MarketRumorsTest;
    // The assistant's reading: market talk alone is uncertain, three reliable confirmations are strong, a reliable
    // denial pulls it down.
    FMarketRumor R;
    R.Sources = { 2 };
    TestEqual(TEXT("Talk alone: uncertain"), MarketRumors::BeliefName(MarketRumors::Belief(R)), FString(TEXT("belirsiz")));
    R.Sources = { 2, 3, 3, 3 };
    TestTrue(TEXT("Three reliable confirmations: very strong"), MarketRumors::Belief(R) >= 0.85f);
    R.Sources = { 2, 3, 1 };
    TestTrue(TEXT("A reliable denial cancels a reliable confirmation"), FMath::Abs(MarketRumors::Belief(R) - MarketRumors::Belief(FMarketRumor{})) < 0.2f);
    TestTrue(TEXT("Sources text"), MarketRumors::SourcesText(R).Contains(TEXT("g\u00fcvenilir kaynak yalanl\u0131yor")));

    // Sources follow the hidden truth with the stated accuracy (many rumours: reliable ones right about 74 %).
    FMarketState S = Start();
    int32 Right = 0, Total = 0, WrongStrong = 0, StrongCount = 0, TalkTrue = 0, TalkCount = 0;
    for (int32 K = 0; K < 600; ++K)
    {
        FMarketRumor T; T.Id = 1000 + K; T.bTrue = K % 2 == 0; T.Country = TEXT("tr"); T.Sources = { 2 };
        for (int32 N = 0; N < 4; ++N) MarketRumors::AddSource(S, T);
        for (int32 N = 1; N < T.Sources.Num(); ++N)
            if (T.Sources[N] & 1u) { ++Total; Right += (((T.Sources[N] & 2u) != 0) == T.bTrue) ? 1 : 0; }
        int32 RC = 0, RD = 0;
        for (int32 N = 1; N < T.Sources.Num(); ++N) if (T.Sources[N] & 1u) ((T.Sources[N] & 2u) ? RC : RD)++;
        if (RC >= 3 && RD == 0) { ++StrongCount; WrongStrong += T.bTrue ? 0 : 1; }
        if (RC == 0 && RD == 0) { ++TalkCount; TalkTrue += T.bTrue ? 1 : 0; }
    }
    const float Share = Total > 0 ? static_cast<float>(Right) / Total : 0.f;
    TestTrue(TEXT("Reliable sources are right most of the time"), Share > 0.66f && Share < 0.82f);
    TestTrue(TEXT("Three reliable confirmations are seldom wrong, but can be"), StrongCount > 0 && WrongStrong * 5 < StrongCount);
    TestTrue(TEXT("Talk of unreliable people alone comes true now and then"), TalkCount == 0 || (TalkTrue > 0 && TalkTrue < TalkCount));

    // A true rumour happens on its day; a false one is denied.
    const int32 Chain = National(S);
    TestTrue(TEXT("A national chain"), Chain != INDEX_NONE);
    if (Chain == INDEX_NONE) return false;
    TestTrue(TEXT("Started"), MarketRumors::Start(S, MarketRumors::EKind::ForSale, Chain, INDEX_NONE, FString(), true, S.Day + 10) != INDEX_NONE);
    Days(S, 12);
    TestTrue(TEXT("It went for sale"), S.Rivals.Chains[Chain].bForSale);
    TestTrue(TEXT("Remembered as true"), S.Rumors.Past.Num() == 1 && S.Rumors.Past[0].Outcome == 1 && S.Rumors.Active.Num() == 0);
    TestTrue(TEXT("Sources came in over the days"), S.Rumors.Past[0].Sources.Num() >= 2);
    const int32 Other = S.Rivals.Chains.IndexOfByPredicate([&S, Chain](const FMarketChain& C) { return &C != &S.Rivals.Chains[Chain] && C.Scope == static_cast<uint8>(MarketChains::EScope::National) && !C.bGone && !C.bForSale && C.Country == TEXT("tr"); });
    TestTrue(TEXT("Another chain"), Other != INDEX_NONE);
    if (Other == INDEX_NONE) return false;
    MarketRumors::Start(S, MarketRumors::EKind::ForSale, Other, INDEX_NONE, FString(), false, S.Day + 5);
    Days(S, 7);
    TestFalse(TEXT("A false one does not happen"), S.Rivals.Chains[Other].bForSale);
    TestEqual(TEXT("Denied"), S.Rumors.Past.Last().Outcome, static_cast<uint8>(2));

    // New rumours start by themselves once the company has a branch (at most three open).
    FMarketState Fresh = Start(21);
    Days(Fresh, 700);
    TestTrue(TEXT("Rumours start by themselves"), Fresh.Rumors.Started >= 5 && Fresh.Rumors.Active.Num() <= MarketRumors::MaxActive);
    TestTrue(TEXT("Some come true, not all"), Fresh.Rumors.CameTrue < Fresh.Rumors.Started);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketLoanAppTest, "MirasMarket.Banking.ApplicationsAndBids", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketLoanAppTest::RunTest(const FString& Parameters)
{
    using namespace MarketRumorsTest;
    FString Message;
    // A company with a year of books (10 000 TL sales, 3 000 TL operating result a month).
    FMarketState S = Start(4);
    const int32 Year = MarketCalendar::DateOf(1).Year + 1;
    for (int32 Month = 1; Month <= 12; ++Month)
    {
        S.Day = MarketCalendar::GameDayOf(Year, Month, 15);
        MarketLedger::Post(S, MarketLedger::EAccount::Sales, 1000000, false);
        MarketLedger::Post(S, MarketLedger::EAccount::CostOfGoods, -700000, false);
    }
    S.Day = MarketCalendar::GameDayOf(Year + 1, 1, 2);
    S.Cash = 100000;
    const FMarketState Fresh = S; // the same books with no debt, for the second application
    FString Why;
    TestTrue(TEXT("The commercial bank takes applications"), MarketBanking::CanApply(S, 1, Why));
    const int64 Cash = S.Cash;
    TestTrue(TEXT("Applied"), MarketBanking::Apply(S, 1, 3, 1, false, Message));
    TestEqual(TEXT("No money yet"), S.Cash, Cash);
    TestFalse(TEXT("One open application a bank"), MarketBanking::CanApply(S, 1, Why));
    for (int32 D = 0; D < 6; ++D) { ++S.Day; S.DayNews.Reset(); MarketBanking::CloseApps(S); }
    const int32 App = S.Banking.Apps.Num() - 1;
    TestEqual(TEXT("An offer came"), S.Banking.Apps[App].Status, static_cast<uint8>(MarketBanking::EAppStatus::Offered));
    TestTrue(TEXT("The assistant told"), S.DayNews.Num() > 0 || S.Banking.Apps[App].Offered > 0);
    const int32 Loans = S.Banking.Loans.Num();
    TestTrue(TEXT("Accepted"), MarketBanking::AcceptApp(S, TArray<FMarketProduct>(), S.Banking.Apps[App].Id, Message));
    TestTrue(TEXT("The loan is booked and the money in"), S.Banking.Loans.Num() == Loans + 1 && S.Cash == Cash + S.Banking.Apps[App].Offered);

    // The first loan used the room: the local bank does not take a new application.
    TestFalse(TEXT("No room, no application"), MarketBanking::CanApply(S, 0, Why));
    // Asking for more than the bank can give: a smaller offer and why.
    S = Fresh;
    TestTrue(TEXT("Applied for more"), MarketBanking::Apply(S, 0, MarketBanking::MaxAsk, 0, false, Message));
    for (int32 D = 0; D < 6; ++D) { ++S.Day; S.DayNews.Reset(); MarketBanking::CloseApps(S); }
    const FMarketLoanApp& More = S.Banking.Apps.Last();
    TestTrue(TEXT("Answered"), More.Status == static_cast<uint8>(MarketBanking::EAppStatus::Offered) || More.Status == static_cast<uint8>(MarketBanking::EAppStatus::Refused));
    if (More.Status == static_cast<uint8>(MarketBanking::EAppStatus::Offered)) TestTrue(TEXT("Part of it, with a reason"), More.Offered <= More.Asked);
    else TestFalse(TEXT("A refusal says why"), More.Reason.IsEmpty());
    // An offer not taken runs out after a week.
    for (int32 D = 0; D < MarketBanking::AppValidDays + 2; ++D) { ++S.Day; S.DayNews.Reset(); MarketBanking::CloseApps(S); }
    TestTrue(TEXT("Expired or refused"), S.Banking.Apps.Last().Status != static_cast<uint8>(MarketBanking::EAppStatus::Offered));

    // A bid goes to the owner and comes back in days.
    FMarketState B = Start(13);
    const int32 Local = B.Rivals.Chains.IndexOfByPredicate([](const FMarketChain& C) { return C.Id.StartsWith(TEXT("yerel.kirklareli")); });
    TestTrue(TEXT("A local chain"), Local != INDEX_NONE);
    if (Local == INDEX_NONE) return false;
    B.Rivals.Chains[Local].Cash = -1000; B.Rivals.Chains[Local].RedTurns = 2; B.Rivals.Chains[Local].Rivalry = 0.f;
    TestTrue(TEXT("Bid sent"), MarketChains::OfferBid(B, Local, Message));
    TestFalse(TEXT("Not bought yet"), B.Rivals.Chains[Local].bGone || B.Rivals.Chains[Local].bOurs);
    for (int32 D = 0; D < 6 && B.Rivals.Chains[Local].BidAnswerDay > 0; ++D) { ++B.Day; B.DayNews.Reset(); MarketChains::CloseBids(B); }
    TestEqual(TEXT("Answered"), B.Rivals.Chains[Local].BidAnswerDay, 0);
    if (B.Rivals.Chains[Local].BidAcceptedUntil >= B.Day)
    {
        TestTrue(TEXT("A deal at the agreed price"), MarketChains::DealPrice(B, Local) == B.Rivals.Chains[Local].BidAgreed);
        B.Cash = B.Rivals.Chains[Local].BidAgreed + 1000000;
        TestTrue(TEXT("Completed"), MarketChains::CompleteDeal(B, TArray<FMarketProduct>(), Local, Message));
        TestTrue(TEXT("Ours"), B.Rivals.Chains[Local].bGone || B.Rivals.Chains[Local].bOurs);
    }
    else TestTrue(TEXT("Refused: it waits"), B.Rivals.Chains[Local].BidDay > 0);

    // C13: a depot starts small (30 % of its rent and manager with few branches) and grows with what it serves.
    TestEqual(TEXT("Few branches"), MarketDepots::ScaleFor(2), MarketDepots::ScaleMin);
    TestEqual(TEXT("Full at twenty"), MarketDepots::ScaleFor(20), 1.f);
    TestTrue(TEXT("Grows"), MarketDepots::ScaleFor(12) > MarketDepots::ScaleFor(6));
    return true;
}

#endif

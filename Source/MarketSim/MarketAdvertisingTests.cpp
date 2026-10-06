#include "MarketAdvertising.h"
#include "MarketOnline.h"
#include "MarketBranches.h"
#include "MarketLedger.h"
#include "MarketPrices.h"
#include "MarketCalendar.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketAdvertisingTest
{
    FMarketState Start(int32 Branches)
    {
        FMarketState S;
        S.RivalSeed = 9; S.Cash = 2000000000; S.Day = 2;
        S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
        for (int32 I = 0; I < Branches; ++I)
        {
            FMarketBranch B;
            B.Country = TEXT("tr"); B.Province = I % 2 ? TEXT("tekirdag") : TEXT("kirklareli"); B.Name = B.Province;
            B.Stage = static_cast<uint8>(MarketBranches::EStage::Open); B.LastRevenue = 100000;
            S.Branches.Add(B);
        }
        return S;
    }

    void Days(FMarketState& S, int32 Count)
    {
        for (int32 D = 0; D < Count; ++D) { ++S.Day; S.DayNews.Reset(); MarketAdvertising::CloseDay(S); }
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketAdvertisingTest, "MarketSim.Advertising.ChannelsAndManager", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketAdvertisingTest::RunTest(const FString& Parameters)
{
    // M34: TV, radio, outdoor, print, social, search; a stock that fades; all together works better; a manager.
    using namespace MarketAdvertising;
    using namespace MarketAdvertisingTest;
    FMarketState S = Start(5);
    FString Message;
    TestEqual(TEXT("No ads, no lift"), TrafficFactor(S), 1.f);
    TestEqual(TEXT("No social media at the start"), EraFactor(S, EChannel::Social, S.Day), 0.f);
    TestFalse(TEXT("No search ads before the internet"), SetLevel(S, TEXT("tr"), EChannel::Search, 1, Message));
    TestEqual(TEXT("Print costs per shop"), MonthCost(S, TEXT("tr"), EChannel::Print, 1), FMath::RoundToInt64(10000.0 * 6 * MarketPrices::ListLevel(S.Day)));
    TestEqual(TEXT("Outdoor costs per province"), MonthCost(S, TEXT("tr"), EChannel::Outdoor, 1), FMath::RoundToInt64(40000.0 * 2 * MarketPrices::ListLevel(S.Day)));
    TestTrue(TEXT("TV"), SetLevel(S, TEXT("tr"), EChannel::TV, 1, Message));
    const int64 Cash = S.Cash;
    Days(S, 30);
    TestTrue(TEXT("TV lifts the shops"), TrafficFactor(S) > 1.02f && TrafficFactor(S) <= 1.f + MaxTraffic);
    TestTrue(TEXT("and costs"), S.Cash < Cash && S.Ledger.Entries.ContainsByPredicate([](const FMarketLedgerEntry& E) { return E.Account == static_cast<uint8>(MarketLedger::EAccount::Marketing); }));
    TestEqual(TEXT("One channel: no together bonus"), Together(S, TEXT("tr")), 1.f);
    TestTrue(TEXT("Radio"), SetLevel(S, TEXT("tr"), EChannel::Radio, 1, Message));
    TestTrue(TEXT("Print"), SetLevel(S, TEXT("tr"), EChannel::Print, 1, Message));
    TestEqual(TEXT("Three together"), Together(S, TEXT("tr")), 1.2f);
    const float Lifted = TrafficFactor(S);
    for (int32 C = 0; C < ChannelCount; ++C) SetLevel(S, TEXT("tr"), static_cast<EChannel>(C), 0, Message);
    Days(S, 300);
    TestTrue(TEXT("The ads fade without money"), TrafficFactor(S) < Lifted);

    // The internet's channels come with it.
    S.Day = MarketOnline::OpenDay(S, MarketOnline::EChannel::Web) + 30;
    TestTrue(TEXT("Search with the internet"), SetLevel(S, TEXT("tr"), EChannel::Search, 2, Message));
    Days(S, 40);
    TestTrue(TEXT("Search lifts online orders"), OnlineFactor(S) > 1.05f && OnlineFactor(S) <= 1.f + MaxOnline);

    // The manager from 20 shops keeps the mix inside his budget.
    TestFalse(TEXT("No manager for six shops"), HireManager(S, Message));
    FMarketState Big = Start(25);
    Big.Day = S.Day;
    TestTrue(TEXT("A manager for a big chain"), HireManager(Big, Message) && SetAuto(Big, true, Message) && SetBudget(Big, 20, Message));
    const MarketCalendar::FDate Now = MarketCalendar::DateOf(Big.Day);
    const int32 MonthStart = MarketCalendar::GameDayOf(Now.Month == 12 ? Now.Year + 1 : Now.Year, Now.Month == 12 ? 1 : Now.Month + 1, 1);
    Days(Big, MonthStart - Big.Day + 32);
    int64 Month = 0;
    for (int32 C = 0; C < ChannelCount; ++C) Month += MonthCost(Big, TEXT("tr"), static_cast<EChannel>(C), LevelOf(Big, TEXT("tr"), static_cast<EChannel>(C)));
    const FMarketAdCountry* Tr = Big.Advertising.Countries.FindByPredicate([](const FMarketAdCountry& C) { return C.Country == TEXT("tr"); });
    TestTrue(TEXT("He spends"), Month > 0);
    TestTrue(TEXT("inside the budget"), Tr && Month <= Tr->PrevRevenue * 20 / 1000 * 3 / 2 * 11 / 10 + 1);
    TestFalse(TEXT("A line for the menu"), CountryLine(Big, TEXT("tr")).IsEmpty());
    return true;
}

#endif

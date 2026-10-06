#include "MarketBanking.h"
#include "MarketBranches.h"
#include "MarketCalendar.h"
#include "MarketEconomy.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketBankingTest
{
    // A company with one open branch and a year of books: 10 000 TL sales and 3 000 TL operating result a month.
    FMarketState Start()
    {
        FMarketState S; S.Initialize(TArray<FMarketProduct>());
        S.RivalSeed = 4; S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli"); S.Cash = 100000;
        FMarketBranch& B = S.Branches.AddDefaulted_GetRef();
        B.Name = TEXT("Test"); B.Format = TEXT("mahalle"); B.Stage = static_cast<uint8>(MarketBranches::EStage::Open); B.OpenedDay = 1;
        const int32 Year = MarketCalendar::DateOf(1).Year + 1;
        for (int32 Month = 1; Month <= 12; ++Month)
        {
            S.Day = MarketCalendar::GameDayOf(Year, Month, 15);
            MarketLedger::Post(S, MarketLedger::EAccount::Sales, 1000000, false);
            MarketLedger::Post(S, MarketLedger::EAccount::CostOfGoods, -700000, false);
        }
        S.Day = MarketCalendar::GameDayOf(Year + 1, 1, 2);
        return S;
    }

    void Days(FMarketState& S, int32 Count)
    {
        for (int32 D = 0; D < Count; ++D) { ++S.Day; S.DayNews.Reset(); MarketBanking::CloseDay(S); }
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBankingLoansTest, "MarketSim.Banking.RatingLoansAndCovenant", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketBankingLoansTest::RunTest(const FString& Parameters)
{
    using namespace MarketBankingTest;
    FString Message;
    FMarketState Shop; Shop.Initialize(TArray<FMarketProduct>()); Shop.Day = 40;
    TestFalse(TEXT("No company loan before the first branch"), MarketBanking::CanBorrow(Shop, 1, Message));

    FMarketState S = Start();
    const MarketBanking::FPicture P = MarketBanking::Picture(S);
    TestEqual(TEXT("A year of revenue"), P.Revenue, static_cast<int64>(12000000));
    TestEqual(TEXT("Operating result"), P.Ebitda, static_cast<int64>(3600000));
    TestTrue(TEXT("No debt: a good rating"), static_cast<int32>(MarketBanking::Rating(S)) >= static_cast<int32>(MarketBanking::ERating::BPlus));
    TestTrue(TEXT("Rating explained"), MarketBanking::RatingLines(S).Num() >= 3);
    TestFalse(TEXT("A bond needs rating A and size"), MarketBanking::CanBorrow(S, MarketBanking::BondBank, Message));

    // Borrow three years of operating result at the commercial bank.
    const int64 Offer = MarketBanking::Offer(S, 1);
    TestTrue(TEXT("The bank lends"), Offer > 0 && Offer <= 3 * P.Ebitda);
    int32 Bank = 0, Step = 0, Tenor = 0; bool bGrace = true;
    TestTrue(TEXT("Argument"), MarketBanking::DecodeLoan(MarketBanking::EncodeLoan(1, 3, 0, false), Bank, Step, Tenor, bGrace) && Bank == 1 && Step == 3 && Tenor == 0 && !bGrace);
    const int64 Cash = S.Cash;
    TestTrue(TEXT("Borrowed"), MarketBanking::Borrow(S, 1, 3, 0, false, Message));
    TestEqual(TEXT("In the till"), S.Cash - Cash, MarketBanking::Debt(S));
    TestEqual(TEXT("No room left at the local bank"), MarketBanking::Offer(S, 0), static_cast<int64>(0));

    // The first installment a month later.
    const int64 Balance = S.Banking.Loans[0].Balance;
    const int64 Before = S.Cash;
    Days(S, MarketBanking::MonthDays + 1);
    TestEqual(TEXT("One month paid"), S.Banking.Loans[0].PaidMonths, 1);
    TestTrue(TEXT("Principal went down"), S.Banking.Loans[0].Balance < Balance);
    TestTrue(TEXT("Paid from the till"), S.Cash < Before);
    TestTrue(TEXT("Interest counted"), S.Banking.InterestPaid > 0);

    // A missed installment: the fee joins the debt and the rating remembers it.
    S.Cash = 0;
    const int64 Owed = S.Banking.Loans[0].Balance;
    Days(S, MarketBanking::MonthDays);
    TestTrue(TEXT("Late"), S.Banking.Loans[0].LateSince > 0 && S.Banking.LateDays.Num() == 1);
    TestTrue(TEXT("Late fee"), S.Banking.Loans[0].Balance > Owed);

    // Debt far above the operating result breaks the covenant: dearer loans, closed doors.
    S.Cash = 1000000000;
    S.Banking.Loans[0].Balance = 50000000;
    const float Rate = S.Banking.Loans[0].YearRate;
    S.Banking.LastRatingDay = S.Day - MarketBanking::MonthDays - 1;
    S.Banking.BreachMonths = 0; // the penalty comes with the first month of the breach
    Days(S, 1);
    TestTrue(TEXT("Covenant broken"), MarketBanking::CovenantBroken(S));
    TestTrue(TEXT("Dearer"), S.Banking.Loans[0].YearRate > Rate);
    TestFalse(TEXT("No new loan"), MarketBanking::CanBorrow(S, 1, Message));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketBankingLineTest, "MarketSim.Banking.LineRestructureAndAcquisition", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketBankingLineTest::RunTest(const FString& Parameters)
{
    using namespace MarketBankingTest;
    FString Message;
    FMarketState S = Start();
    TestTrue(TEXT("Line opened"), MarketBanking::OpenLine(S, Message));
    TestEqual(TEXT("15 % of a year's revenue"), S.Banking.LineLimit, static_cast<int64>(1800000));
    // A negative till is covered by the line, and paid back when the till is comfortable.
    S.Cash = -500000;
    Days(S, 1);
    TestEqual(TEXT("Covered"), S.Cash, static_cast<int64>(0));
    TestEqual(TEXT("Drawn"), S.Banking.LineDrawn, static_cast<int64>(500000));
    S.Cash = 5000000;
    Days(S, 1);
    TestEqual(TEXT("Paid back"), S.Banking.LineDrawn, static_cast<int64>(0));

    // Two loans into one longer loan.
    TestTrue(TEXT("Loan 1"), MarketBanking::Borrow(S, 1, 1, 0, false, Message));
    TestTrue(TEXT("Loan 2"), MarketBanking::Borrow(S, 0, 3, 0, true, Message));
    const int64 Total = MarketBanking::Debt(S);
    TestTrue(TEXT("Restructured"), MarketBanking::Restructure(S, 1, Message));
    TestEqual(TEXT("One loan"), S.Banking.Loans.Num(), 1);
    TestEqual(TEXT("Fee joins the debt"), S.Banking.Loans[0].Balance, Total + FMath::RoundToInt64(Total * MarketBanking::RestructureFee));
    TestTrue(TEXT("The rating waits"), S.Banking.RestructuredUntil > S.Day);

    // An acquisition: the target's operating result makes room the company alone does not have.
    FMarketState T = Start();
    const int64 Alone = MarketBanking::AcquisitionRoom(T, 1, 0);
    const int64 WithTarget = MarketBanking::AcquisitionRoom(T, 1, 4000000);
    TestTrue(TEXT("The target's result counts"), WithTarget > Alone);
    // 20 000 TL: more than any bank lends on the company alone (the investment bank: 1.6 x 10 800).
    TestFalse(TEXT("Too big without it"), MarketBanking::FinanceAcquisition(T, 2000000000 / 100, 0, Message));
    const int64 Cash = T.Cash;
    TestTrue(TEXT("Financed with it"), MarketBanking::FinanceAcquisition(T, 2000000000 / 100, 4000000, Message));
    TestTrue(TEXT("Money in"), T.Cash - Cash >= 20000000 && T.Banking.Loans.Num() == 1);
    return true;
}

#endif

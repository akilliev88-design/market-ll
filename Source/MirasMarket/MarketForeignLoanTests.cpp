#include "MarketBanking.h"
#include "MarketBranches.h"
#include "MarketCountry.h"
#include "MarketPrices.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// E4b: a loan from a bank abroad is owed in that country's money and paid at the day's exchange rate.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketForeignLoanTest, "MirasMarket.Banking.ForeignLoan", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketForeignLoanTest::RunTest(const FString& Parameters)
{
    FMarketState S; S.RivalSeed = 5; S.Day = 800; S.Cash = 100000000; S.CountryId = MarketCountry::DefaultId();
    FString Abroad;
    for (const MarketCountry::FProfile& P : MarketCountry::All()) if (P.Id != S.CountryId && P.Cities.Num() > 0) { Abroad = P.Id; break; }
    if (!TestFalse(TEXT("A foreign pack"), Abroad.IsEmpty())) return true;

    // Without a company there, no bank of that country lends; the development bank and bonds stay at home.
    FString Why;
    TestFalse(TEXT("No company, no loan"), MarketBanking::CanBorrowIn(S, Abroad, 1, Why));
    FMarketSubsidiary Company; Company.Country = Abroad; Company.FoundedDay = 1; S.Company.Subsidiaries.Add(Company);
    FMarketBranch Store; Store.Country = Abroad; Store.Stage = static_cast<uint8>(MarketBranches::EStage::Open); S.Branches.Add(Store);
    TestFalse(TEXT("No development bank abroad"), MarketBanking::CanBorrowIn(S, Abroad, 3, Why));
    TestTrue(TEXT("Never more than the whole company's offer"), MarketBanking::OfferIn(S, Abroad, 1) <= MarketBanking::Offer(S, 1));
    const MarketCountry::FProfile& Pack = MarketCountry::FindOrDefault(Abroad);
    if (Pack.Banks.IsValidIndex(1)) TestEqual(TEXT("The country's own bank"), MarketBanking::BankNameIn(S, Abroad, 1), Pack.Banks[1]);
    TestTrue(TEXT("That country's rate"), FMath::IsNearlyEqual(MarketBanking::YearRateIn(S, Abroad, 1) - MarketBanking::YearRate(S, 1),
        MarketPrices::LoanRate(Abroad, S.Day) - MarketPrices::LoanRate(S.Day), 1e-6) || MarketBanking::YearRateIn(S, Abroad, 1) == 0.01);

    // A running loan abroad: its balance and every installment at the day's rate.
    const double Fx = MarketPrices::ToHome(S, Abroad, S.Day);
    FMarketCorpLoan Loan; Loan.Id = 1; Loan.Bank = 1; Loan.Country = Abroad; Loan.Principal = Loan.Balance = 1200000; Loan.YearRate = 0.12f;
    Loan.Months = 12; Loan.StartDay = S.Day - 31; Loan.NextDueDay = S.Day - 1; Loan.Installment = MarketBanking::InstallmentOf(1200000, 0.12, 12, false);
    S.Banking.Loans.Add(Loan);
    TestEqual(TEXT("Balance in our money"), MarketBanking::BalanceHome(S, S.Banking.Loans[0]), FMath::RoundToInt64(1200000.0 * Fx));
    TestEqual(TEXT("The company's bank debt counts it at the rate"), MarketBanking::Debt(S), FMath::RoundToInt64(1200000.0 * Fx));
    const int64 Interest = FMath::RoundToInt64(1200000.0 * 0.12f / 12.0);
    const int64 DueLocal = Interest + FMath::Clamp<int64>(Loan.Installment - Interest, 0, 1200000);
    const int64 Cash = S.Cash;
    MarketBanking::CloseDay(S);
    TestEqual(TEXT("Installment paid at the day's rate"), Cash - S.Cash, FMath::RoundToInt64(static_cast<double>(DueLocal) * Fx));
    TestTrue(TEXT("Owed in its own money"), S.Banking.Loans.Num() == 1 && S.Banking.Loans[0].Balance == 1200000 - (DueLocal - Interest) && S.Banking.Loans[0].PaidMonths == 1);
    AddInfo(FString::Printf(TEXT("OLCUM: kur %.4f, yerel taksit %lld, bizim paramizla %lld"), Fx, DueLocal, Cash - S.Cash));
    return true;
}

#endif

#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// Karar M28 (Mustafa 01.10.2026): company finance. Independent of the world, tested (MarketSim.Banking.*).
// The first store keeps its small loan from the country's local bank (MarketFinance); once the company grows it deals with banks
// as a company:
//  - A credit rating (A+, A, B+, B, C, D) every month from what any bank sees: debt against the year's operating
//    result (EBITDA from the books), interest cover, size, age, late payments and money trouble. Every part is
//    told in one line, so the player knows which lever moves it.
//  - Banks of the country, each with a character: the local bank (small, warm, dear), a commercial bank, an
//    investment bank (big, cheap, choosy) and the development bank (cheapest, long, investment only, a yearly
//    quota). With rating A and a big enough company: a bond (interest only, the principal at the end).
//  - Investment loans: 24, 36 or 60 months, an optional six-month grace (interest only), the rate = the year's
//    loan rate + the bank's spread + the rating's spread. Paid monthly from the till; early repayment 1 %.
//  - A revolving credit line (from rating B): a limit from the year's revenue; it can cover a negative till at
//    the day close by itself (and pays itself back when the till is comfortable); interest monthly on what is
//    drawn, a small fee on the rest.
//  - Covenants: debt above 3.5 x EBITDA breaks them. A breach makes every loan 2 points dearer and closes the
//    banks' doors; three months in a row and the biggest lender calls back a quarter of its loan.
//  - Restructuring: every loan into one longer loan at one bank (2 % fee, rating one step down for six months).
//  - A missed installment: a late fee, the rating falls; the trouble ladder (MarketFinance) still applies.
//  - Fun (06 section 2b): borrowing is a real trade (grow now against interest and risk); the rating is a goal of
//    its own; inflation quietly shrinks old debt (an era to use).
namespace MarketBanking
{
    enum class ERating : uint8 { D = 0, C, B, BPlus, A, APlus, Count };
    enum class EKind : uint8 { Investment = 0, Bond, Restructured, Called, Acquisition };
    constexpr int32 BankCount = 4;               // local, commercial, investment, development (the bond: index 4)
    constexpr int32 BondBank = 4;
    constexpr int32 TenorCount = 3;
    const int32 Tenors[TenorCount] = { 24, 36, 60 };
    constexpr int32 GraceMonths = 6;
    constexpr int32 MonthDays = 30;
    constexpr float EarlyRepayFee = 0.01f;
    constexpr float RestructureFee = 0.02f;
    constexpr float LateFee = 0.03f;
    constexpr float CovenantLeverage = 3.5f;
    constexpr float BreachPenalty = 0.02f;       // yearly, on every loan while the covenant is broken
    constexpr int32 CallAfterMonths = 3;
    constexpr int32 MinCompanyStores = 1;        // company finance opens with the first branch

    struct FBank
    {
        FString Name;
        float Spread;           // yearly, on top of the year's loan rate
        float SizeFactor;       // how big a loan it gives (x the company's borrowing room)
        ERating MinRating;
        int32 MaxTenor;         // months
        bool bInvestmentOnly;   // the development bank: one loan a year
    };
    // The banks of the active country (MarketCast::Bank).
    FBank Bank(const FMarketState& State, int32 Index);
    FString RatingName(ERating Rating);
    // Yearly spread of a rating (A+ 1 point .. C 8 points; D: no lending).
    float RatingSpread(ERating Rating);

    // What the banks see (trailing 12 closed months from the books; a young company: what it has, annualised).
    struct FPicture
    {
        int64 Revenue = 0;
        int64 Ebitda = 0;       // net profit + interest + tax
        int64 Interest = 0;     // interest paid (positive)
        int64 Debt = 0;         // every bank debt (family loans, company loans, the line)
        int32 Months = 0;       // months of books behind it
        float Leverage = 0.f;   // Debt / Ebitda (99 when Ebitda <= 0 and there is debt)
        float Cover = 99.f;     // Ebitda / Interest
    };
    FPicture Picture(const FMarketState& State);
    ERating Rating(const FMarketState& State);
    // The rating's parts, one line each ("Bor\u00e7 / FAV\u00d6K 2,4: +1"...).
    TArray<FString> RatingLines(const FMarketState& State);

    // Company finance is open (a branch exists, or loans are running).
    bool IsOpen(const FMarketState& State);
    // Every bank debt of the company (not the first store's small loan: MarketFinance::Debt).
    int64 Debt(const FMarketState& State);
    bool CovenantBroken(const FMarketState& State);

    // The most a bank lends now (0 = it does not), the yearly rate it asks, a monthly installment.
    int64 Offer(const FMarketState& State, int32 BankIndex);
    double YearRate(const FMarketState& State, int32 BankIndex);
    int64 InstallmentOf(int64 Principal, double Rate, int32 Months, bool bBond);
    // Menu argument: bank x 1000 + amount step (0..3 = 25/50/75/100 % of the offer) x 100 + tenor x 10 + grace.
    int32 EncodeLoan(int32 BankIndex, int32 Step, int32 Tenor, bool bGrace);
    bool DecodeLoan(int32 Arg, int32& OutBank, int32& OutStep, int32& OutTenor, bool& bOutGrace);
    bool CanBorrow(const FMarketState& State, int32 BankIndex, FString& OutReason);
    bool Borrow(FMarketState& State, int32 BankIndex, int32 Step, int32 Tenor, bool bGrace, FString& OutMessage);
    bool Repay(FMarketState& State, int32 LoanIndex, FString& OutMessage);
    bool Restructure(FMarketState& State, int32 BankIndex, FString& OutMessage);
    // Karar M29: an acquisition loan. The bank also leans on the target's operating result (TargetEbitda), so a
    // healthy chain can be bought mostly with borrowed money; the cheapest bank that covers the shortfall lends it
    // for its longest tenor. False (and why) when no bank covers it.
    int64 AcquisitionRoom(const FMarketState& State, int32 BankIndex, int64 TargetEbitda);
    bool FinanceAcquisition(FMarketState& State, int64 Amount, int64 TargetEbitda, FString& OutMessage);

    // E4b (11_TEK_EKONOMI \u00a7E4.4): a loan from a bank of a country where we have a company (M65). It is owed in
    // that country's money at that country's loan rate (+ the bank's and the rating's spread); every payment goes
    // out at the day's exchange rate, so a falling currency makes the debt lighter and a rising one heavier. The
    // country's banks lend against our stores there (their share of our stores, at least a quarter of the offer).
    // Banks 0..2 (no development bank, no bond abroad).
    bool CanBorrowIn(const FMarketState& State, const FString& Country, int32 BankIndex, FString& OutReason);
    int64 OfferIn(const FMarketState& State, const FString& Country, int32 BankIndex);   // our money today
    double YearRateIn(const FMarketState& State, const FString& Country, int32 BankIndex);
    FString BankNameIn(const FMarketState& State, const FString& Country, int32 BankIndex);
    bool BorrowIn(FMarketState& State, const FString& Country, int32 BankIndex, int32 Step, int32 Tenor, bool bGrace, FString& OutMessage);
    // A loan's balance in our money today (abroad: at the day's rate).
    int64 BalanceHome(const FMarketState& State, const FMarketCorpLoan& Loan);

    // The credit line.
    int64 LineLimitFor(const FMarketState& State);
    bool OpenLine(FMarketState& State, FString& OutMessage);
    bool SetLineAuto(FMarketState& State, bool bAuto, FString& OutMessage);
    bool RepayLine(FMarketState& State, FString& OutMessage);

    // Menu lines.
    FString Describe(const FMarketState& State, int32 LoanIndex);
    FString Summary(const FMarketState& State);
    // Due in the next 30 days (installments + line interest).
    int64 DueSoon(const FMarketState& State);

    // C13 (karar M43, Mustafa 02.10.2026: "bankaya gidip kredi \u00e7ekebilelim, asistan teklifimize cevap geldi diye
    // sunsun"): the player's loans go through an application. The bank answers in 2-4 days (a bond in 5) with what
    // it sees on that day: the full amount, a smaller one (and why), or no (and why); its rate is the day's rate
    // with the bank's mood (-0.5 .. +1 point). An offer stays open for a week; accepting it books the loan. An
    // acquisition application (a chain we agreed to buy, or one for sale) asks for the shortfall and, accepted,
    // closes the deal. Several banks can be asked at once (one open application a bank). The automatic player and
    // older commands keep the instant Borrow / FinanceAcquisition.
    enum class EPurpose : uint8 { Investment = 0, Acquisition };
    enum class EAppStatus : uint8 { Pending = 0, Offered, Refused, Accepted, Declined, Expired };
    constexpr int32 AppValidDays = 7;
    constexpr int32 MaxAsk = 4;                 // menu steps 0..3 = 25..100 % of today's offer, 4 = half again more
    // Menu argument for an application: the same as EncodeLoan, step 0..4.
    bool DecodeApp(int32 Arg, int32& OutBank, int32& OutStep, int32& OutTenor, bool& bOutGrace);
    int64 AskedFor(const FMarketState& State, int32 BankIndex, int32 Step);
    bool CanApply(const FMarketState& State, int32 BankIndex, FString& OutReason);
    bool Apply(FMarketState& State, int32 BankIndex, int32 Step, int32 Tenor, bool bGrace, FString& OutMessage);
    // An acquisition: the shortfall of the price (+5 % to keep the stores running) from every bank that might lend.
    int64 AcquisitionNeed(const FMarketState& State, int64 Price);
    bool ApplyAcquisition(FMarketState& State, int32 ChainIndex, int64 Price, FString& OutMessage);
    int32 FindApp(const FMarketState& State, int32 AppId);
    bool AcceptApp(FMarketState& State, const TArray<FMarketProduct>& Products, int32 AppId, FString& OutMessage);
    bool DeclineApp(FMarketState& State, int32 AppId, FString& OutMessage);
    // Applications still waiting or offers open (the menu's list), newest first.
    TArray<int32> OpenApps(const FMarketState& State);
    FString DescribeApp(const FMarketState& State, int32 AppIndex);
    // Day close (after CloseDay): answers due today, offers that ran out.
    void CloseApps(FMarketState& State);

    // Day close (before MarketFinance: the line can cover a negative till first): installments, the line, the
    // monthly rating, covenants, news.
    void CloseDay(FMarketState& State);
}

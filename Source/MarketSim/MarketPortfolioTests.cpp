#include "MarketPortfolio.h"
#include "MarketBranches.h"
#include "MarketCalendar.h"
#include "MarketCommand.h"
#include "MarketCountry.h"
#include "MarketEconomy.h"
#include "MarketManagers.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketPortfolioTest
{
    FMarketState Start(int32 Day)
    {
        FMarketState S; S.Initialize(TArray<FMarketProduct>());
        S.RivalSeed = 7; S.Day = Day; S.Cash = 900000000;
        S.CountryId = TEXT("tr"); S.CityId = TEXT("kirklareli");
        S.Decisions.Reset();
        return S;
    }
    FString AnyCity(bool bBig)
    {
        if (const MarketCountry::FProfile* Pack = MarketCountry::Find(TEXT("tr")))
            for (const MarketCountry::FCity& City : Pack->Cities)
                if (City.Id != TEXT("kirklareli") && (bBig ? City.PopulationK >= 1000 : City.PopulationK < 300)) return City.Id;
        return FString();
    }
    int32 AddShop(FMarketState& S, const FString& Province, int32 OpenedDay)
    {
        FMarketBranch& B = S.Branches.AddDefaulted_GetRef();
        B.Country = TEXT("tr"); B.Province = Province; B.Format = TEXT("mahalle");
        B.Name = TEXT("Test \u00b7 Mahalle 1");
        B.Stage = static_cast<uint8>(MarketBranches::EStage::Open);
        B.OpenedDay = OpenedDay; B.Satisfaction = 60.f; B.Maturity = 1.f;
        B.Rent = 90000; B.Workers = 3;
        return S.Branches.Num() - 1;
    }
    constexpr int32 Year = MarketPortfolio::YearDays;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketPortfolioAgeTest, "MarketSim.Portfolio.AgeAndWorks", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketPortfolioAgeTest::RunTest(const FString& Parameters)
{
    using namespace MarketPortfolioTest;
    const FString Town = AnyCity(false);
    TestFalse(TEXT("A small town in the pack"), Town.IsEmpty());
    FMarketState S = Start(100 + 5 * Year);
    const int32 I = AddShop(S, Town, 100);
    TestEqual(TEXT("Five years: still new"), MarketPortfolio::AgeFactor(S, I), 1.f);
    TestEqual(TEXT("The first store never ages here"), MarketPortfolio::AgeFactor(S, -1), 1.f);
    S.Day = 100 + 9 * Year;
    TestTrue(TEXT("Nine years: half the loss"), FMath::IsNearlyEqual(MarketPortfolio::AgeFactor(S, I), 1.f - 0.5f * MarketPortfolio::AgeLoss, 0.001f));
    S.Day = 100 + 15 * Year;
    TestTrue(TEXT("Past twelve years: at most 15 %"), FMath::IsNearlyEqual(MarketPortfolio::AgeFactor(S, I), 1.f - MarketPortfolio::AgeLoss, 0.001f));
    TestTrue(TEXT("The card advises renewing"), MarketPortfolio::Advice(S, I).Contains(TEXT("yenileme")));

    // Renovation: closed for two weeks, paid with the day's costs, the age starts again with a fresh first year.
    const int64 Costs = S.OtherCosts;
    const int64 Price = MarketPortfolio::WorksCost(S, TArray<FMarketProduct>(), I, MarketPortfolio::EWorks::Renovate);
    TestTrue(TEXT("Renovation has a price"), Price > 0);
    FString Message;
    TestTrue(TEXT("Renovation starts"), MarketPortfolio::Renovate(S, TArray<FMarketProduct>(), I, Message));
    TestEqual(TEXT("Closed for the works"), S.Branches[I].Stage, static_cast<uint8>(MarketBranches::EStage::Renovation));
    TestEqual(TEXT("Small store: two weeks"), S.Branches[I].StageUntil, S.Day + MarketPortfolio::SmallWorksDays - 1);
    TestEqual(TEXT("Paid at the day close"), S.OtherCosts - Costs, Price);
    TestFalse(TEXT("No second works while one runs"), MarketPortfolio::Relocate(S, TArray<FMarketProduct>(), I, Message));
    MarketPortfolio::Finish(S, TArray<FMarketProduct>(), I);
    TestEqual(TEXT("Open again"), S.Branches[I].Stage, static_cast<uint8>(MarketBranches::EStage::Open));
    TestTrue(TEXT("Renewed: the first year draws more"), FMath::IsNearlyEqual(MarketPortfolio::AgeFactor(S, I), MarketPortfolio::FreshPull, 0.001f));
    S.Day += MarketPortfolio::FreshDays;
    TestEqual(TEXT("After a year: plain new"), MarketPortfolio::AgeFactor(S, I), 1.f);
    TestFalse(TEXT("Too young to renew again"), MarketPortfolio::Renovate(S, TArray<FMarketProduct>(), I, Message));

    // Relocation: a hasty site is left behind, part of the habit stays, a new lease.
    S.Branches[I].bHasty = 1;
    TestTrue(TEXT("The card points at the weak site"), MarketPortfolio::Advice(S, I).Contains(TEXT("ta\u015f\u0131n")));
    TestTrue(TEXT("Relocation starts"), MarketPortfolio::Relocate(S, TArray<FMarketProduct>(), I, Message));
    MarketPortfolio::Finish(S, TArray<FMarketProduct>(), I);
    TestEqual(TEXT("The weak site is gone"), S.Branches[I].bHasty, static_cast<uint8>(0));
    TestTrue(TEXT("60 % of the habit stays"), FMath::IsNearlyEqual(S.Branches[I].Maturity, MarketPortfolio::RelocateHabit, 0.001f));
    TestEqual(TEXT("A new lease from today"), S.Branches[I].OpenedDay, S.Day);
    TestTrue(TEXT("Its rent is today's"), S.Branches[I].Rent == MarketBranches::SignedRent(S, S.Branches[I]));

    // Change of type: the next sizes; a hypermarket needs a big province.
    TestEqual(TEXT("Larger: supermarket"), MarketPortfolio::Larger(S, I), FString(TEXT("buyuk")));
    TestEqual(TEXT("Smaller: discounter"), MarketPortfolio::Smaller(S, I), FString(TEXT("kucuk")));
    int32 Branch = INDEX_NONE;
    FString Format;
    TestTrue(TEXT("Argument round trip"), MarketPortfolio::DecodeFormat(MarketPortfolio::EncodeFormat(I, TEXT("buyuk")), Branch, Format) && Branch == I && Format == TEXT("buyuk"));
    S.Day += Year;
    TestFalse(TEXT("No hypermarket in a small town"), MarketPortfolio::CanStart(S, TArray<FMarketProduct>(), I, MarketPortfolio::EWorks::Reformat, TEXT("hiper"), Message));
    TestTrue(TEXT("Change of type starts"), MarketPortfolio::Reformat(S, TArray<FMarketProduct>(), I, TEXT("buyuk"), Message));
    TestEqual(TEXT("Big type: three weeks"), S.Branches[I].StageUntil, S.Day + MarketPortfolio::BigWorksDays - 1);
    MarketPortfolio::Finish(S, TArray<FMarketProduct>(), I);
    const FMarketBranch& After = S.Branches[I];
    TestEqual(TEXT("Now a supermarket"), After.Format, FString(TEXT("buyuk")));
    TestTrue(TEXT("Its name follows"), After.Name.Contains(MarketBranches::FormatInfo(TEXT("buyuk")).Short));
    TestTrue(TEXT("More people for the bigger store"), After.Workers > 3);
    TestEqual(TEXT("The works are done"), After.Works, static_cast<uint8>(0));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketPortfolioCardTest, "MarketSim.Portfolio.YearlyCard", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketPortfolioCardTest::RunTest(const FString& Parameters)
{
    using namespace MarketPortfolioTest;
    TestEqual(TEXT("A good margin, liked, new: A"), MarketPortfolio::CardOf(0.08f, 80.f, 1.f), static_cast<uint8>(1));
    TestEqual(TEXT("In the red, disliked, worn: E"), MarketPortfolio::CardOf(-0.05f, 30.f, 0.85f), static_cast<uint8>(5));
    TestTrue(TEXT("Age lowers the card"), MarketPortfolio::CardOf(0.03f, 60.f, 0.85f) > MarketPortfolio::CardOf(0.03f, 60.f, 1.f));
    TestEqual(TEXT("Letters"), MarketPortfolio::CardLetter(3), FString(TEXT("C")));

    const FString Town = AnyCity(false);
    const int32 NewYear = MarketCalendar::GameDayOfCampaign(3, 1, 1);
    FMarketState S = Start(NewYear - 1);
    S.DayNews.Reset();
    const int32 Good = AddShop(S, Town, 10);
    const int32 Bad = AddShop(S, Town, 10);
    const int32 Young = AddShop(S, Town, NewYear - 30);
    S.Branches[Good].YearDays = 365; S.Branches[Good].YearRevenue = 10000000; S.Branches[Good].YearProfit = 800000;
    S.Branches[Bad].YearDays = 365; S.Branches[Bad].YearRevenue = 10000000; S.Branches[Bad].YearProfit = -500000; S.Branches[Bad].Satisfaction = 30.f;
    S.Branches[Young].YearDays = 30; S.Branches[Young].YearRevenue = 900000;
    MarketPortfolio::CloseDay(S);
    TestEqual(TEXT("Not at the year's end: no card"), S.Branches[Good].Card, static_cast<uint8>(0));
    S.Day = NewYear;
    MarketPortfolio::CloseDay(S);
    TestTrue(TEXT("The good store's card is better"), S.Branches[Good].Card > 0 && S.Branches[Good].Card < S.Branches[Bad].Card);
    TestEqual(TEXT("For the year that ended"), S.Branches[Good].CardYear, 2);
    TestEqual(TEXT("A store open a month gets none"), S.Branches[Young].Card, static_cast<uint8>(0));
    TestTrue(TEXT("The year's sums start again"), S.Branches[Good].YearDays == 0 && S.Branches[Good].YearRevenue == 0 && S.Branches[Young].YearRevenue == 0);
    TestTrue(TEXT("A news line"), S.DayNews.ContainsByPredicate([](const FString& N) { return N.Contains(TEXT("karne")); }));
    TestTrue(TEXT("The losing store is told to shrink or close"), MarketPortfolio::Advice(S, Bad).Contains(TEXT("zararda")));
    TestTrue(TEXT("The menu line shows the card"), MarketPortfolio::Line(S, Good).Contains(MarketPortfolio::CardLetter(S.Branches[Good].Card)));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketPortfolioCommandTest, "MarketSim.Portfolio.ManagerProposesRenewal", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketPortfolioCommandTest::RunTest(const FString& Parameters)
{
    using namespace MarketPortfolioTest;
    const FString Town = AnyCity(false);
    int32 Day = 100 + 10 * Year;
    while (MarketCalendar::DateOf(Day).Day != 1) ++Day;
    FMarketState S = Start(Day);
    const int32 Old = AddShop(S, Town, 100);
    AddShop(S, Town, Day - 2 * Year);
    MarketCommand::CloseDay(S, TArray<FMarketProduct>());
    TestFalse(TEXT("Without a province manager nobody proposes"), S.Decisions.ContainsByPredicate([](const FMarketDecision& D) { return D.Id.StartsWith(TEXT("command.renew:")); }));
    FMarketManager& M = S.Management.Managers.AddDefaulted_GetRef();
    M.Level = static_cast<uint8>(MarketManagers::ELevel::Province); M.Country = TEXT("tr"); M.Area = Town; M.Name = TEXT("Ay\u015fe Test");
    MarketCommand::CloseDay(S, TArray<FMarketProduct>());
    const FMarketDecision* Card = S.Decisions.FindByPredicate([](const FMarketDecision& D) { return D.Id.StartsWith(TEXT("command.renew:")); });
    TestTrue(TEXT("The province manager proposes renewing the oldest store"), Card && Card->Arg == Old);
    MarketCommand::CloseDay(S, TArray<FMarketProduct>());
    int32 Cards = 0;
    for (const FMarketDecision& D : S.Decisions) if (D.Id.StartsWith(TEXT("command.renew:"))) ++Cards;
    TestEqual(TEXT("Once in half a year"), Cards, 1);
    return true;
}

#endif

#include "MarketGame.h"
#include "MarketStaff.h"
#include "MarketLedger.h"
#include "MarketPrices.h"
#include "MarketCalendar.h"
#include "MarketCountry.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketStaffTest
{
    // One played day: sales, then the world's day close order (economy, then people and books).
    void PlayDay(FMarketState& S, int64 Revenue, int32 Served, int64 CostOfGoods = 0, int64 Purchases = 0)
    {
        S.Revenue = Revenue; S.CostOfGoods = CostOfGoods; S.Purchases = Purchases; S.Served = Served;
        S.CloseDay();
        MarketStaff::CloseDay(S);
    }

    FMarketEmployee Person(FMarketState& S, MarketStaff::ERole Role, int32 Skill, int32 Honesty, int64 Wage)
    {
        FMarketEmployee E;
        E.Id = S.NextEmployeeId++;
        E.Name = FString::Printf(TEXT("Test %d"), E.Id);
        E.Role = static_cast<uint8>(Role);
        E.Skill = Skill; E.Speed = 50; E.Stamina = 90; E.Honesty = Honesty;
        E.DailyWage = Wage; E.Morale = 70.f; E.HiredDay = 1;
        return E;
    }

    bool AnyNews(const FMarketState& S, const TCHAR* Start)
    {
        for (const FString& Line : S.StaffNews) if (Line.StartsWith(Start)) return true;
        return false;
    }

    TArray<FMarketProduct> Catalog()
    {
        FMarketProduct Milk; Milk.Id = TEXT("milk"); Milk.Category = TEXT("s\u00fct"); Milk.Cost = 170; Milk.BasePrice = 250;
        return { Milk };
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStaffPeopleTest, "MirasMarket.Staff.PeopleAndMorale", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStaffPeopleTest::RunTest(const FString& Parameters)
{
    using namespace MarketStaffTest;
    using MarketStaff::ERole;
    FMarketState S; S.Initialize(Catalog()); S.RivalSeed = 1234; S.Cash = 100000;
    FString Message;

    // Hiring pool: always a cashier and a stocker.
    MarketStaff::EnsureCandidates(S);
    TestEqual(TEXT("Pool without HR"), S.Candidates.Num(), MarketStaff::PoolSize);
    TestTrue(TEXT("First candidate is a cashier"), MarketStaff::RoleOf(S.Candidates[0]) == ERole::Cashier);
    TestTrue(TEXT("Second candidate is a stocker"), MarketStaff::RoleOf(S.Candidates[1]) == ERole::Stocker);
    const int64 CashBefore = S.Cash;
    TestTrue(TEXT("H hires the best cashier"), MarketStaff::HireBest(S, ERole::Cashier, Message));
    TestEqual(TEXT("Hiring costs 120 TL as before"), S.Cash, CashBefore - MarketStaff::HireCost);
    TestTrue(TEXT("The till has a cashier today"), S.bCashier);
    TestEqual(TEXT("The candidate left the pool"), S.Candidates.Num(), MarketStaff::PoolSize - 1);
    const int32 CashierId = S.Staff[0].Id;
    TestEqual(TEXT("Payroll is the person's wage"), S.DailyPayroll(), S.Staff[0].DailyWage);

    // Pace: a quick, skilled, rested person is faster; a big basket takes longer.
    FMarketEmployee Quick; Quick.Speed = 90; Quick.Skill = 80;
    FMarketEmployee Slow; Slow.Speed = 20; Slow.Skill = 20; Slow.Fatigue = 90.f;
    TestTrue(TEXT("Quick cashier is faster"), MarketStaff::CheckoutSeconds(Quick, 6) < MarketStaff::CheckoutSeconds(Slow, 6));
    TestTrue(TEXT("Bigger basket takes longer"), MarketStaff::CheckoutSeconds(Quick, 1) < MarketStaff::CheckoutSeconds(Quick, 12));
    TestTrue(TEXT("Skilled stocker carries more"), MarketStaff::CarryUnits(Quick) > MarketStaff::CarryUnits(Slow));
    TestTrue(TEXT("Experienced stocker may re-plan"), MarketStaff::MayEditPlan(Quick) && !MarketStaff::MayEditPlan(Slow));
    TestTrue(TEXT("Fatigue slows down"), MarketStaff::WorkSpeed(Slow) < MarketStaff::WorkSpeed(Quick));

    // A day off: the player works the till, the person rests and comes back.
    TestTrue(TEXT("Day off given"), MarketStaff::GiveDayOff(S, CashierId, false, Message));
    TestFalse(TEXT("Nobody at the till on the day off"), S.bCashier);
    PlayDay(S, 20000, 40);
    TestTrue(TEXT("Back at the till the next day"), S.bCashier);
    TestEqual(TEXT("Nothing worked, nothing learned"), S.Staff[0].DaysWorked, 0);

    // Busy days make a cashier tired.
    for (int32 Day = 0; Day < 3; ++Day) PlayDay(S, 30000, 80);
    TestTrue(TEXT("Heavy days build fatigue"), S.Staff[0].Fatigue > 30.f);
    TestEqual(TEXT("Worked days are counted"), S.Staff[0].DaysWorked, 3);

    // An underpaid, exhausted stocker: unhappy for three days -> notice -> leaves two days later.
    // B3 (#39): the lowest legal wage; a very skilled stocker is worth much more.
    FMarketEmployee Under = Person(S, ERole::Stocker, 100, 80, MarketStaff::MinimumDailyWage(S.Day));
    Under.Morale = 28.f; Under.Fatigue = 90.f; Under.Stamina = 0;
    const int32 UnderId = Under.Id;
    S.Staff.Add(Under);
    for (int32 Day = 0; Day < 3; ++Day) PlayDay(S, 20000, 20);
    const FMarketEmployee* Notice = MarketStaff::FindEmployee(S, UnderId);
    TestTrue(TEXT("Three unhappy days bring a notice"), Notice && Notice->LeaveDay == S.Day + MarketStaff::NoticeDays - 1);
    for (int32 Day = 0; Day < MarketStaff::NoticeDays; ++Day) PlayDay(S, 20000, 20);
    TestNull(TEXT("Leaves when the notice runs out"), MarketStaff::FindEmployee(S, UnderId));

    // A raise keeps someone who is only a little unhappy.
    FMarketEmployee Doubtful = Person(S, ERole::Stocker, 50, 80, 2000);
    Doubtful.Morale = 40.f; Doubtful.LeaveDay = S.Day + 1;
    const int32 DoubtfulId = Doubtful.Id;
    S.Staff.Add(Doubtful);
    TestTrue(TEXT("Raise"), MarketStaff::Raise(S, DoubtfulId, Message));
    TestEqual(TEXT("Notice withdrawn"), MarketStaff::FindEmployee(S, DoubtfulId)->LeaveDay, 0);
    TestEqual(TEXT("Ten percent more"), MarketStaff::FindEmployee(S, DoubtfulId)->DailyWage, int64(2200));

    // Firing pays three days' wage.
    const int64 BeforeFire = S.Cash;
    TestTrue(TEXT("Fire"), MarketStaff::Fire(S, DoubtfulId, Message));
    TestEqual(TEXT("Severance"), S.Cash, BeforeFire - 2200 * MarketStaff::SeveranceDays);
    TestFalse(TEXT("Unknown person"), MarketStaff::Fire(S, 99999, Message));

    // Limits.
    FMarketState Full; Full.Initialize(Catalog()); Full.Cash = 1000000;
    for (int32 I = 0; I < MarketStaff::MaxCashiers; ++I) Full.Staff.Add(Person(Full, ERole::Cashier, 50, 80, 2000));
    MarketStaff::EnsureCandidates(Full);
    TestFalse(TEXT("At most two cashiers"), MarketStaff::HireBest(Full, ERole::Cashier, Message));

    // The roster and the books survive a save.
    S.Staff.Add(Person(S, ERole::Cashier, 55, 80, 2100)); // someone is surely on the roster for the save check
    auto* Save = NewObject<UMarketSave>();
    Save->State = S;
    Save->State.Books.TaxDue = 1234;
    TArray<uint8> Bytes;
    TestTrue(TEXT("Serialize"), UGameplayStatics::SaveGameToMemory(Save, Bytes));
    auto* Restored = Cast<UMarketSave>(UGameplayStatics::LoadGameFromMemory(Bytes));
    if (!TestNotNull(TEXT("Restore"), Restored)) return false;
    TestEqual(TEXT("Roster survives"), Restored->State.Staff.Num(), S.Staff.Num());
    TestEqual(TEXT("Wage survives"), Restored->State.Staff[0].DailyWage, S.Staff[0].DailyWage);
    TestEqual(TEXT("Tax survives"), Restored->State.Books.TaxDue, int64(1234));
    TestTrue(TEXT("Valid save"), Restored->State.IsStructurallyValid());
    Restored->State.Staff[0].Morale = 150.f;
    TestFalse(TEXT("Broken morale rejected"), Restored->State.IsStructurallyValid());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStaffTaxTest, "MirasMarket.Staff.TaxAndAccountant", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStaffTaxTest::RunTest(const FString& Parameters)
{
    using namespace MarketStaffTest;
    const auto Round = [](double Value) { return static_cast<int64>(FMath::RoundToDouble(Value)); };
    // G-077 (#34): the VAT inside a VAT-inclusive margin, and income tax on the profit without it.
    const double VatShare = static_cast<double>(MarketStaff::VatRate) / (1.0 + static_cast<double>(MarketStaff::VatRate));
    const int64 Vat = Round(7 * (50000 - 20000) * VatShare);

    // Without an accountant: the player pays; late means a penalty.
    FMarketState S; S.Initialize(Catalog()); S.RivalSeed = 42; S.Cash = 500000;
    for (int32 Day = 1; Day <= 6; ++Day) PlayDay(S, 50000, 20, 30000, 20000);
    TestEqual(TEXT("No tax before the week ends"), S.Books.TaxDue, int64(0));
    PlayDay(S, 50000, 20, 30000, 20000);
    const int64 Income = Round((7 * (50000 - 30000 - 2200) - Vat) * static_cast<double>(MarketStaff::IncomeTaxRate));
    const int64 Tax = Vat + Income;
    const int64 Audit = S.Books.Audits > 0 ? FMath::Max<int64>(MarketStaff::AuditPenaltyMin, static_cast<int64>(Tax * MarketStaff::AuditPenaltyRate)) : 0;
    TestEqual(TEXT("Week declared: VAT + income tax (+ audit fine)"), S.Books.TaxDue, Tax + Audit);
    TestEqual(TEXT("Three days to pay"), S.Books.TaxDueDay, 7 + MarketStaff::TaxPayDays);
    TestEqual(TEXT("Books start a new week"), S.Books.PeriodSales, int64(0));
    PlayDay(S, 50000, 20); PlayDay(S, 50000, 20);
    TestEqual(TEXT("No penalty before the due day"), S.Books.TotalPenalties, Audit);
    const int64 BeforeLate = S.Books.TaxDue;
    PlayDay(S, 50000, 20);
    TestTrue(TEXT("Late: a penalty is added"), S.Books.TaxDue > BeforeLate && S.LastPenalty > 0);
    const int64 Owed = S.Books.TaxDue;
    const int64 Cash = S.Cash;
    TestEqual(TEXT("Player pays everything"), MarketStaff::PayTax(S), Owed);
    TestEqual(TEXT("Cash goes down"), S.Cash, Cash - Owed);
    TestEqual(TEXT("Nothing left"), S.Books.TaxDue, int64(0));
    TestEqual(TEXT("Nothing to pay twice"), MarketStaff::PayTax(S), int64(0));

    // With the accountant: 10 % lower declaration, paid on time, never audited.
    FMarketState A; A.Initialize(Catalog()); A.RivalSeed = 42; A.Cash = 500000;
    FString Message;
    TestTrue(TEXT("Accountant hired"), MarketStaff::HireAccountant(A, Message));
    TestFalse(TEXT("Only one accountant"), MarketStaff::HireAccountant(A, Message));
    for (int32 Day = 1; Day <= 7; ++Day) PlayDay(A, 50000, 20, 30000, 20000);
    const int64 IncomeA = Round((7 * (50000 - 30000 - 2200 - MarketStaff::AccountantDailyFee) - Vat) * static_cast<double>(MarketStaff::IncomeTaxRate));
    const int64 TaxA = Round((Vat + IncomeA) * static_cast<double>(MarketStaff::AccountantDeduction));
    TestEqual(TEXT("Accountant's declaration"), A.Books.TaxDue, TaxA);
    PlayDay(A, 50000, 20); PlayDay(A, 50000, 20);
    TestEqual(TEXT("Paid the day before the deadline"), A.Books.TaxDue, int64(0));
    TestEqual(TEXT("Paid amount"), A.Books.TotalTaxPaid, TaxA);
    TestEqual(TEXT("No penalty"), A.Books.TotalPenalties, int64(0));

    // A loss week: no income tax, extra purchases carry VAT into the next week.
    FMarketState L; L.Initialize(Catalog()); L.Cash = 500000;
    for (int32 Day = 1; Day <= 7; ++Day) PlayDay(L, 1000, 1, 600, 20000);
    TestTrue(TEXT("Loss week: nothing due but VAT carried"), L.Books.VatCarry > 0 && L.Books.TaxDue <= MarketStaff::AuditPenaltyMin);

    // Audits: about one week in six without books, never with the accountant.
    FMarketState Long; Long.Initialize(Catalog()); Long.RivalSeed = 7; Long.Cash = 100000000;
    FMarketState Kept; Kept.Initialize(Catalog()); Kept.RivalSeed = 7; Kept.Cash = 100000000;
    MarketStaff::HireAccountant(Kept, Message);
    for (int32 Day = 1; Day <= 7 * 60; ++Day)
    {
        PlayDay(Long, 20000, 10, 12000, 10000); MarketStaff::PayTax(Long);
        PlayDay(Kept, 20000, 10, 12000, 10000);
    }
    TestTrue(TEXT("Audits happen without an accountant"), Long.Books.Audits >= 3 && Long.Books.Audits <= 25);
    TestEqual(TEXT("No audits with the accountant"), Kept.Books.Audits, 0);
    TestEqual(TEXT("Accountant never late"), Kept.Books.TotalPenalties, int64(0));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStaffTillHrTest, "MirasMarket.Staff.TillAndHr", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStaffTillHrTest::RunTest(const FString& Parameters)
{
    using namespace MarketStaffTest;
    using MarketStaff::ERole;
    FString Message;

    // Same seed, same person id: the honest and the dishonest cashier make the same honest mistakes; only the
    // dishonest one has small shortages on top. The accountant points at the pattern, never at the honest one.
    FMarketState Honest; Honest.Initialize(Catalog()); Honest.RivalSeed = 99; Honest.Cash = 1000000;
    FMarketState Dishonest; Dishonest.Initialize(Catalog()); Dishonest.RivalSeed = 99; Dishonest.Cash = 1000000;
    Honest.Staff.Add(Person(Honest, ERole::Cashier, 60, 90, 2000));
    Dishonest.Staff.Add(Person(Dishonest, ERole::Cashier, 60, 0, 2000));
    MarketStaff::HireAccountant(Honest, Message);
    MarketStaff::HireAccountant(Dishonest, Message);
    int64 HonestTill = 0, DishonestTill = 0;
    for (int32 Day = 0; Day < 42; ++Day)
    {
        PlayDay(Honest, 200000, 40); HonestTill += Honest.LastTillDifference; MarketStaff::PayTax(Honest);
        PlayDay(Dishonest, 200000, 40); DishonestTill += Dishonest.LastTillDifference; MarketStaff::PayTax(Dishonest);
    }
    TestTrue(TEXT("Shortages show in the till"), DishonestTill < HonestTill - 10000);
    TestTrue(TEXT("Honest mistakes stay small"), FMath::Abs(HonestTill) < 10000);
    TestEqual(TEXT("The honest cashier is never pointed at"), Honest.Staff[0].FlaggedWeek, 0);
    TestTrue(TEXT("The accountant notices the pattern"), Dishonest.Staff[0].FlaggedWeek > 0);
    TestTrue(TEXT("Only the cashier can be warned"), MarketStaff::Warn(Dishonest, Dishonest.Staff[0].Id, Message) && !MarketStaff::Warn(Dishonest, Dishonest.Staff[1].Id, Message));

    // HR manager: unlocked by three shop employees, found in the pool, then shows six candidates.
    FMarketState S; S.Initialize(Catalog()); S.RivalSeed = 5; S.Cash = 1000000;
    S.Staff.Add(Person(S, ERole::Stocker, 60, 80, 2000));
    S.Staff.Add(Person(S, ERole::Stocker, 60, 80, 5000));
    TestFalse(TEXT("HR locked with two people"), MarketStaff::HrUnlocked(S));
    S.Staff.Add(Person(S, ERole::Cashier, 60, 80, 2000));
    MarketStaff::SyncCounts(S);
    TestTrue(TEXT("HR unlocked with three"), MarketStaff::HrUnlocked(S));
    MarketStaff::EnsureCandidates(S);
    TestTrue(TEXT("HR manager offered"), MarketStaff::RoleOf(S.Candidates[2]) == ERole::HrManager);
    const int64 BeforeHr = S.Cash;
    TestTrue(TEXT("HR hired"), MarketStaff::Hire(S, 2, Message));
    TestEqual(TEXT("HR hire cost"), S.Cash, BeforeHr - MarketStaff::HrHireCost);
    S.Candidates.Reset();
    MarketStaff::EnsureCandidates(S);
    TestEqual(TEXT("HR finds six candidates"), S.Candidates.Num(), MarketStaff::HrPoolSize);
    TestFalse(TEXT("Only one HR manager"), S.Candidates.ContainsByPredicate([](const FMarketEmployee& C) { return MarketStaff::RoleOf(C) == ERole::HrManager; }));

    // A tired stocker gets tomorrow off (a colleague covers); a leaver is replaced from the pool.
    S.Staff[0].Fatigue = 95.f;
    S.Staff[1].LeaveDay = S.Day;
    FMarketEmployee Good = Person(S, ERole::Stocker, 95, 90, 3000); // better than any generated candidate (20..80)
    S.Candidates.Insert(Good, 0);
    const int32 TiredId = S.Staff[0].Id;
    const int32 GoodId = Good.Id;
    const int32 LeaverId = S.Staff[1].Id;
    PlayDay(S, 20000, 10);
    TestEqual(TEXT("HR gives the tired stocker tomorrow off"), MarketStaff::FindEmployee(S, TiredId)->OffDay, S.Day);
    TestNull(TEXT("The leaver is gone"), MarketStaff::FindEmployee(S, LeaverId));
    TestNotNull(TEXT("HR hired the replacement"), MarketStaff::FindEmployee(S, GoodId));
    TestEqual(TEXT("Still two stockers"), MarketStaff::Count(S, ERole::Stocker), 2);
    TestEqual(TEXT("One of them works tomorrow"), S.Stockers, 1);
    TestTrue(TEXT("HR reports"), AnyNews(S, TEXT("\u0130K:")));
    TestTrue(TEXT("Negotiated wage"), MarketStaff::FindEmployee(S, GoodId)->DailyWage < 3000);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketStaffWagesTest, "MirasMarket.Staff.WagesAndSocialSecurity", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketStaffWagesTest::RunTest(const FString& Parameters)
{
    // B3 (#39): the minimum wage floor, the employer's social security and the seniority pay.
    using namespace MarketStaffTest;
    using MarketStaff::ERole;
    MarketCountry::SetActiveProfile(MarketCountry::FProfile(), 1);
    const int32 July = MarketCalendar::GameDayOf(2011, 7, 1);
    TestEqual(TEXT("Minimum a day at the start (658,95 / 30, up to 50 kurus)"), MarketStaff::MinimumDailyWage(1), int64(2200));
    TestTrue(TEXT("It rises in July"), MarketStaff::MinimumDailyWage(July) > MarketStaff::MinimumDailyWage(July - 1));
    TestTrue(TEXT("The lowest fair wage is the minimum"), MarketStaff::FairWage(ERole::Cashier, 0, 1) == MarketStaff::MinimumDailyWage(1));
    TestEqual(TEXT("The accountant is a fee"), MarketStaff::FairWage(ERole::Accountant, 80, 1), MarketPrices::WageScaled(MarketStaff::AccountantDailyFee, 1));

    FMarketState S; S.Initialize(Catalog()); S.RivalSeed = 5; S.Cash = 1000000;
    MarketStaff::EnsureCandidates(S);
    bool bLegal = S.Candidates.Num() > 0;
    for (const FMarketEmployee& C : S.Candidates) bLegal &= C.DailyWage >= MarketStaff::MinimumDailyWage(S.Day);
    TestTrue(TEXT("Candidates ask at least the minimum"), bLegal);

    // Social security on the wages paid at the close (not on the accountant's fee), in the books.
    S.Candidates.Reset();
    S.Staff.Add(Person(S, ERole::Cashier, 50, 90, 3000));
    FString Message;
    TestTrue(TEXT("Accountant"), MarketStaff::HireAccountant(S, Message));
    TestEqual(TEXT("22.5 % of the wages"), MarketStaff::DailySocialSecurity(S), int64(675));
    const int64 Before = S.Cash;
    S.Revenue = 0; S.DayNews.Reset(); S.CloseDay();
    const int64 AfterEconomy = S.Cash;
    MarketStaff::CloseDay(S);
    TestEqual(TEXT("Paid at the close"), AfterEconomy - S.Cash + S.LastTillDifference, int64(675));
    TestTrue(TEXT("A cost of the day"), S.LastOperatingCost >= 675 && Before > S.Cash);
    TestEqual(TEXT("Booked"), MarketLedger::Statement(S, 1, S.Day).At(MarketLedger::EAccount::SocialSecurity), int64(-675));

    // A wage below the new minimum is raised at the close.
    FMarketState Low = S;
    Low.Staff[0].DailyWage = 1500;
    Low.Day = July; Low.DayNews.Reset(); Low.CloseDay(); MarketStaff::CloseDay(Low);
    TestEqual(TEXT("Raised to the minimum"), Low.Staff[0].DailyWage, MarketStaff::MinimumDailyWage(July));
    TestTrue(TEXT("Told"), AnyNews(Low, TEXT("Asgari \u00fccret artt\u0131")));

    // Seniority pay: nothing in the first year, 30 days' wage a year after it (pro rata).
    TestEqual(TEXT("Nothing before a year"), MarketStaff::SeniorityPay(3000, 1, 300), int64(0));
    TestEqual(TEXT("A year: 30 days' wage"), MarketStaff::SeniorityPay(3000, 1, 366), int64(90000));
    TestEqual(TEXT("A year and a half"), MarketStaff::SeniorityPay(3000, 1, 1 + 365 + 182), FMath::RoundToInt64(3000.0 * 30 * 547 / 365.0));
    FMarketState Old = S;
    Old.Day = 1 + 2 * 365;
    const int32 Id = Old.Staff[0].Id;
    const int64 Expected = Old.Staff[0].DailyWage * MarketStaff::SeveranceDays + MarketStaff::SeniorityPay(Old.Staff[0].DailyWage, Old.Staff[0].HiredDay, Old.Day);
    TestEqual(TEXT("Notice + seniority"), MarketStaff::SeverancePay(Old, Old.Staff[0]), Expected);
    const int64 CashOld = Old.Cash;
    TestTrue(TEXT("Fire after two years"), MarketStaff::Fire(Old, Id, Message));
    TestEqual(TEXT("Paid in full"), CashOld - Old.Cash, Expected);
    TestTrue(TEXT("The message says it"), Message.Contains(TEXT("k\u0131dem")));
    TestEqual(TEXT("Booked as severance"), MarketLedger::DayStatement(Old, Old.Day).At(MarketLedger::EAccount::Severance), -Expected);
    FMarketState Poor = S; Poor.Day = 1 + 2 * 365; Poor.Cash = 100;
    TestFalse(TEXT("Cannot fire without the money"), MarketStaff::Fire(Poor, Id, Message));

    // Another country's rules (pack values).
    MarketCountry::FProfile Us; Us.Id = TEXT("us"); Us.EmployerSocialRate = 0.1f; Us.SeveranceDaysPerYear = 0; Us.WageFactor = 2.f;
    MarketCountry::SetActiveProfile(Us, 1);
    TestEqual(TEXT("Lower share"), MarketStaff::EmployerShare(3000), int64(300));
    TestEqual(TEXT("No seniority pay"), MarketStaff::SeniorityPay(3000, 1, 1000), int64(0));
    TestTrue(TEXT("Minimum follows the wage level"), MarketStaff::MinimumDailyWage(1) >= 4350);
    MarketCountry::SetActiveProfile(MarketCountry::FProfile(), 1);
    return true;
}

#endif

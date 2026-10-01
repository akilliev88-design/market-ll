#include "MarketDirector.h"
#include "MarketCountry.h"
#include "MarketCalendar.h"
#include "MarketRivals.h"
#include "MarketStaff.h"
#include "MarketSuppliers.h"
#include "MarketPromotions.h"
#include "MarketCompetitors.h"
#include "MarketEvents.h"
#include "MarketStory.h"
#include "MarketGoods.h"
#include "MarketFreshness.h"
#include "MarketCredit.h"
#include "MarketFinance.h"
#include "MarketBranches.h"
#include "MarketOnline.h"
#include "MarketPayments.h"
#include "MarketSimulation.h"
#include "MarketCompany.h"
#include "MarketManagers.h"
#include "MarketDepots.h"
#include "MarketChains.h"
#include "MarketBrands.h"
#include "MarketSourcing.h"
#include "MarketDepartments.h"

float MarketDirector::TrafficFactor(const FMarketState& State, const TArray<FString>& Aisles)
{
    return MarketCalendar::TrafficFactor(State.Day, State.RivalSeed) * MarketRivals::TrafficFactor(State.Day, State.RivalSeed, Aisles)
        * MarketPromotions::TrafficFactor(State) * MarketCompetitors::TrafficFactor(State)
        * MarketEvents::Factor(State, MarketEvents::EModifier::Traffic) * MarketBranches::MainShopFactor(State)
        * MarketOnline::StoreTrafficFactor(State) * MarketPayments::TrafficFactor(State) * MarketSimulation::TrafficFactor(State);
}

double MarketDirector::ToleranceBonus(const FMarketState& State, const FMarketProduct& Product)
{
    // G-084: the city's purchasing power (Istanbul 1.25 -> +5 points, Van 0.75 -> -5 points; Lueleburgaz 0).
    const double Income = 0.2 * (MarketCountry::CityIncome(State.CountryId, State.CityId) - 1.0);
    return MarketEvents::Tolerance(State, MarketGoods::Classify(Product.Category)) + MarketSimulation::ToleranceBonus(State) + Income;
}

float MarketDirector::RivalPriceFactor(const FMarketState& State, const TArray<FString>& Aisles, const FString& Category)
{
    return MarketCompetitors::RivalPriceFactor(State, Category, Aisles);
}

float MarketDirector::DemandWeight(const FMarketState& State, const FMarketProduct& Product)
{
    return MarketCalendar::CategoryFactor(State.Day, State.RivalSeed, Product.Category) * MarketOnline::GroupFactor(State, MarketGoods::Classify(Product.Category));
}

float MarketDirector::OrderScale(const FMarketState& State, const FMarketProduct& Product)
{
    // The order arrives at the next day close and is sold the day after the running day.
    const float Then = MarketCalendar::CategoryFactor(State.Day + 1, State.RivalSeed, Product.Category);
    const float Before = FMath::Max(0.2f, MarketCalendar::CategoryFactor(FMath::Max(1, State.Day - 1), State.RivalSeed, Product.Category));
    return FMath::Clamp(Then / Before, 0.3f, 3.f);
}

TArray<float> MarketDirector::OrderScales(const FMarketState& State, const TArray<FMarketProduct>& Products)
{
    TArray<float> Scales;
    Scales.Init(1.f, Products.Num());
    for (int32 I = 0; I < Products.Num(); ++I) Scales[I] = OrderScale(State, Products[I]);
    return Scales;
}

FString MarketDirector::DateText(const FMarketState& State)
{
    return MarketCalendar::DateText(State.Day);
}

FString MarketDirector::TodayText(const FMarketState& State)
{
    return MarketCalendar::Describe(State.Day, State.RivalSeed);
}

FString MarketDirector::TomorrowText(const FMarketState& State)
{
    return MarketCalendar::Forecast(State.Day, State.RivalSeed);
}

void MarketDirector::ApplyPrices(const FMarketState& State, const TArray<FMarketProduct>& CatalogBase, TArray<FMarketProduct>& Products)
{
    MarketSuppliers::ApplyPrices(State, CatalogBase, Products);
    // Karar M25: a brand we starve charges more, a friend less.
    for (FMarketProduct& Product : Products)
    {
        // G-083: the supply line's tier and the company's buying power.
        const float Factor = MarketBrands::CostFactor(State, Product.Brand) * MarketSourcing::CostFactor(State, Product.Category);
        if (!FMath::IsNearlyEqual(Factor, 1.f)) Product.Cost = FMath::Max<int64>(1, FMath::RoundToInt64(static_cast<double>(Product.Cost) * Factor));
    }
}

uint8 MarketDirector::PaymentMethod(const FMarketState& State, uint8 Segment, float Roll)
{
    return static_cast<uint8>(MarketPayments::Choose(State, static_cast<MarketCustomers::ESegment>(Segment), Roll));
}

bool MarketDirector::LeavesWithoutCard(FMarketState& State, float Roll)
{
    return MarketPayments::LeavesWithoutCard(State, Roll);
}

float MarketDirector::BudgetFactor(const FMarketState& State, uint8 Segment)
{
    return MarketPayments::BudgetFactor(State, static_cast<MarketCustomers::ESegment>(Segment));
}

FString MarketDirector::OnCheckout(FMarketState& State, int32 CustomerId, int64 Receipt, float Roll, uint8 Method)
{
    // A basket written in the credit book is not paid now; everything else settles by its payment method.
    const int64 CashBefore = State.Cash;
    const FString Credit = MarketCredit::OnCheckout(State, CustomerId, Receipt, Roll);
    if (State.Cash != CashBefore) return Credit;
    const FString Paid = MarketPayments::Settle(State, static_cast<MarketPayments::EMethod>(FMath::Min<uint8>(Method, 3)), Receipt);
    return Credit.IsEmpty() ? Paid : Paid.IsEmpty() ? Credit : Credit + TEXT(" ") + Paid;
}

int64 MarketDirector::OrderAllowance(const FMarketState& State)
{
    return MarketSuppliers::OrderAllowance(State);
}

FString MarketDirector::OnOrder(FMarketState& State, int64 Bill)
{
    return MarketSuppliers::OnOrder(State, Bill);
}

bool MarketDirector::Command(FMarketState& State, const TArray<FMarketProduct>& Products, FName Action, int32 Arg, FString& OutMessage)
{
    if (Action == TEXT("Supplier"))
        return MarketSuppliers::Switch(State, static_cast<MarketSuppliers::ESupplier>(FMath::Clamp(Arg, 0, static_cast<int32>(MarketSuppliers::ESupplier::Count) - 1)), OutMessage);
    if (Action == TEXT("PayBills"))
    {
        const int64 Paid = MarketSuppliers::PayBills(State);
        OutMessage = Paid > 0 ? FString::Printf(TEXT("Toptanc\u0131 faturalar\u0131 \u00f6dendi: %s."), *MarketCountry::Money(Paid))
            : MarketSuppliers::OpenBills(State) > 0 ? FString(TEXT("Faturalar i\u00e7in kasada yeterli para yok.")) : FString(TEXT("A\u00e7\u0131k fatura yok."));
        return Paid > 0;
    }
    if (Action == TEXT("PassOnPriceRise"))
    {
        const double Gap = MarketSuppliers::PriceGap(State);
        const int32 Changed = MarketSuppliers::PassOnPriceRise(State, Products);
        OutMessage = Changed > 0 ? FString::Printf(TEXT("Zam raflara yans\u0131t\u0131ld\u0131: %d \u00fcr\u00fcnde fiyat %%%.1f artt\u0131."), Changed, Gap * 100.0)
            : FString(TEXT("Raf fiyatlar\u0131 zaten g\u00fcncel liste d\u00fczeyinde."));
        return Changed > 0;
    }
    using MarketPromotions::EKind;
    if (Action == TEXT("Discount10")) return MarketPromotions::Start(State, Products, EKind::AisleDiscount, Arg, 10, OutMessage);
    if (Action == TEXT("Discount20")) return MarketPromotions::Start(State, Products, EKind::AisleDiscount, Arg, 20, OutMessage);
    if (Action == TEXT("MultiBuy")) return MarketPromotions::Start(State, Products, EKind::MultiBuy, Arg, 0, OutMessage);
    if (Action == TEXT("Endcap")) return MarketPromotions::Start(State, Products, EKind::Endcap, Arg, 0, OutMessage);
    if (Action == TEXT("Flyer")) return MarketPromotions::Start(State, Products, EKind::Flyer, INDEX_NONE, 0, OutMessage);
    if (Action == TEXT("PromoScoped")) return MarketPromotions::StartScoped(State, Products, Arg, OutMessage); // G-078 (J07)
    if (Action == TEXT("StopPromotion")) return MarketPromotions::Stop(State, Arg, OutMessage);
    if (Action == TEXT("AcceptOffer")) return MarketPromotions::AcceptOffer(State, Products, OutMessage);
    if (Action == TEXT("Decide")) return MarketEvents::Decide(State, Products, Arg, OutMessage);
    if (Action == TEXT("FreshPolicy")) return MarketFreshness::SetPolicy(State, static_cast<MarketFreshness::EPolicy>(FMath::Clamp(Arg, 0, 2)), OutMessage);
    if (Action == TEXT("CreditLimit")) return MarketCredit::SetLimit(State, Arg, OutMessage);
    if (Action == TEXT("CollectCredit")) return MarketCredit::CollectAll(State, OutMessage) > 0;
    if (Action == TEXT("TakeLoan")) return MarketFinance::TakeLoan(State, Arg, OutMessage);
    if (Action == TEXT("RepayLoan")) return MarketFinance::RepayAll(State, OutMessage);
    if (Action == TEXT("OpenBranch"))
    {
        FString Country, Province, Format;
        if (!MarketBranches::DecodeSite(Arg, Country, Province, Format)) { OutMessage = TEXT("Bilinmeyen il ya da ma\u011faza t\u00fcr\u00fc."); return false; }
        return MarketBranches::Open(State, Products, Country, Province, Format, OutMessage);
    }
    if (Action == TEXT("CloseBranch")) return MarketBranches::Close(State, Products, Arg, OutMessage);
    if (Action == TEXT("Promote"))
    {
        int32 Newest = INDEX_NONE;
        for (int32 I = 0; I < State.Branches.Num(); ++I) if (State.Branches[I].Stage != static_cast<uint8>(MarketBranches::EStage::Closed)) Newest = I;
        return MarketBranches::Promote(State, Arg, Newest, OutMessage);
    }
    if (Action == TEXT("PromoteTo")) // G-074 menu: Arg = branch index * 1000000 + employee id
        return Arg >= 0 && MarketBranches::Promote(State, Arg % 1000000, Arg / 1000000, OutMessage);
    // G-086b: the management hierarchy (MarketManagers.h).
    if (Action == TEXT("ManagerBonus")) return MarketManagers::Bonus(State, Arg, OutMessage);
    if (Action == TEXT("ManagerWarn")) return MarketManagers::Warn(State, Arg, OutMessage);
    if (Action == TEXT("ManagerReplace")) return MarketManagers::Replace(State, Arg, OutMessage);
    if (Action == TEXT("PromoteToProvince")) return MarketManagers::PromoteToProvince(State, Arg, OutMessage);
    if (Action == TEXT("AppointOutside"))
    {
        MarketManagers::ELevel Level = MarketManagers::ELevel::Province;
        FString Country, Area;
        if (!MarketManagers::DecodeArea(Arg, Level, Country, Area)) { OutMessage = TEXT("B\u00f6yle bir kademe ya da b\u00f6lge yok."); return false; }
        return MarketManagers::Appoint(State, Level, Country, Area, INDEX_NONE, OutMessage);
    }
    if (Action == TEXT("AppointPromote")) // Arg = branch index * 10 + level (the branch's own area)
    {
        const int32 Branch = Arg / 10;
        const MarketManagers::ELevel Level = static_cast<MarketManagers::ELevel>(FMath::Clamp(Arg % 10, 0, static_cast<int32>(MarketManagers::ELevel::Depot)));
        if (Arg < 0 || !State.Branches.IsValidIndex(Branch)) { OutMessage = TEXT("B\u00f6yle bir \u015fube yok."); return false; }
        return MarketManagers::Appoint(State, Level, MarketBranches::CountryOf(State, State.Branches[Branch]), MarketManagers::AreaOfBranch(State, Branch, Level), Branch, OutMessage);
    }
    if (Action == TEXT("AppointCandidate")) // G-086b ek (M22): Arg = MarketManagers::EncodeArea x 10 + candidate (0..2)
    {
        MarketManagers::ELevel Level = MarketManagers::ELevel::Province;
        FString Country, Area;
        if (Arg < 0 || !MarketManagers::DecodeArea(Arg / 10, Level, Country, Area)) { OutMessage = TEXT("B\u00f6yle bir kademe ya da b\u00f6lge yok."); return false; }
        return MarketManagers::AppointCandidate(State, Level, Country, Area, Arg % 10, OutMessage);
    }
    if (Action == TEXT("ManagerReplaceWith")) // Arg = branch index x 10 + candidate (0..2)
        return Arg >= 0 && MarketManagers::ReplaceWithCandidate(State, Arg / 10, Arg % 10, OutMessage);
    if (Action == TEXT("ManagerHireFor")) // Arg = branch index x 10 + candidate (0..2): a branch without a manager
        return Arg >= 0 && MarketManagers::HireCandidateForBranch(State, Arg / 10, Arg % 10, OutMessage);
    if (Action == TEXT("DismissManager")) return MarketManagers::Dismiss(State, Arg, OutMessage);
    if (Action == TEXT("BonusManager")) return MarketManagers::BonusManager(State, Arg, OutMessage);
    if (Action == TEXT("WarnManager")) return MarketManagers::WarnManager(State, Arg, OutMessage);
    if (Action == TEXT("OnlineChannel"))
        return MarketOnline::SetChannel(State, static_cast<MarketOnline::EChannel>(FMath::Clamp(Arg / 10, 0, 2)), Arg % 10 != 0, OutMessage);
    if (Action == TEXT("HireCourier")) return MarketOnline::HireCourier(State, OutMessage);
    if (Action == TEXT("FireCourier")) return MarketOnline::FireCourier(State, OutMessage);
    if (Action == TEXT("Substitute")) return MarketOnline::SetSubstitute(State, Arg, OutMessage);
    if (Action == TEXT("FreeDelivery")) return MarketOnline::SetFreeDelivery(State, Arg != 0, OutMessage);
    if (Action == TEXT("Card")) return MarketPayments::SetCard(State, Arg != 0, OutMessage);
    if (Action == TEXT("MealCard")) return MarketPayments::SetMealCard(State, Arg != 0, OutMessage);
    if (Action == TEXT("Build")) return MarketCompany::Build(State, Arg, OutMessage);
    if (Action == TEXT("BuildDepot"))
    {
        // G-086: Arg = country index * 100 + sub-region index (MarketCountry::All(), FProfile::SubRegions).
        const TArray<MarketCountry::FProfile>& All = MarketCountry::All();
        const int32 C = Arg / 100, R = Arg % 100;
        if (Arg < 0 || !All.IsValidIndex(C) || !All[C].SubRegions.IsValidIndex(R)) { OutMessage = TEXT("B\u00f6yle bir b\u00f6lge yok."); return false; }
        return MarketCompany::BuildDepot(State, All[C].Id, All[C].SubRegions[R].Id, OutMessage);
    }
    if (Action == TEXT("BuildDepotIn")) // G-089: Arg = MarketManagers::EncodeArea(ELevel::Depot, country, province)
    {
        MarketManagers::ELevel Level = MarketManagers::ELevel::Depot;
        FString Country, Province;
        if (!MarketManagers::DecodeArea(Arg, Level, Country, Province) || Level != MarketManagers::ELevel::Depot) { OutMessage = TEXT("B\u00f6yle bir il yok."); return false; }
        return MarketDepots::Build(State, Country, Province, OutMessage);
    }
    if (Action == TEXT("AcceptBrandOffer")) return MarketBrands::Accept(State, Arg, OutMessage); // Arg = offer id (M25)
    if (Action == TEXT("RejectBrandOffer")) return MarketBrands::Reject(State, Arg, OutMessage);
    if (Action == TEXT("SetSourcing")) // G-083: Arg = line x 10 + tier
    {
        MarketSourcing::ELine Line = MarketSourcing::ELine::Drinks;
        MarketSourcing::ETier Tier = MarketSourcing::ETier::Local;
        if (!MarketSourcing::Decode(Arg, Line, Tier)) { OutMessage = TEXT("B\u00f6yle bir se\u00e7enek yok."); return false; }
        return MarketSourcing::Set(State, Line, Tier, OutMessage);
    }
    if (Action == TEXT("SetDepartment")) // M26: Arg = department x 100 + store type x 10 + on
    {
        MarketDepartments::EDept Dept = MarketDepartments::EDept::Produce;
        int32 Format = 0;
        bool bOn = false;
        if (!MarketDepartments::DecodeSet(Arg, Dept, Format, bOn)) { OutMessage = TEXT("B\u00f6yle bir se\u00e7enek yok."); return false; }
        return MarketDepartments::Set(State, Dept, Format, bOn, OutMessage);
    }
    if (Action == TEXT("SetDeptStance")) // M26: Arg = department x 10 + stance (0 cheap, 1 normal, 2 dear)
        return MarketDepartments::SetStance(State, static_cast<MarketDepartments::EDept>(FMath::Clamp(Arg / 10, 0, MarketDepartments::DeptCount)), Arg % 10, OutMessage);
    if (Action == TEXT("ReplaceMasters")) // M26: Arg = department
        return MarketDepartments::ReplaceWeakMasters(State, static_cast<MarketDepartments::EDept>(FMath::Clamp(Arg, 0, MarketDepartments::DeptCount)), OutMessage);
    if (Action == TEXT("BuyChain")) return MarketChains::Buy(State, Products, Arg, OutMessage); // Arg = State.Rivals.Chains index (Akis C2b)
    if (Action == TEXT("VisitBranch")) // Codex A4 calls it when a branch visit starts (what a visit reveals: later, C)
    {
        if (!State.Branches.IsValidIndex(Arg)) { OutMessage = TEXT("B\u00f6yle bir \u015fube yok."); return false; }
        OutMessage = FString::Printf(TEXT("%s ziyaret ediliyor."), *State.Branches[Arg].Name);
        return true;
    }
    if (Action == TEXT("Difficulty")) return MarketSimulation::SetDifficulty(State, Arg, OutMessage);
    if (Action == TEXT("PandemicProfile"))
    {
        if (State.Online.bPandemic == (Arg != 0)) { OutMessage = TEXT("Salg\u0131n d\u00f6nemi ayar\u0131 zaten b\u00f6yle."); return false; }
        State.Online.bPandemic = Arg != 0;
        OutMessage = State.Online.bPandemic ? TEXT("Salg\u0131n d\u00f6nemi (10. ve 11. y\u0131l) oyunda olacak.") : TEXT("Salg\u0131n d\u00f6nemi (10. ve 11. y\u0131l) oyunda olmayacak.");
        return true;
    }
    if (Action == TEXT("DeclineOffer")) { MarketPromotions::DeclineOffer(State); OutMessage = TEXT("Selim'in teklifi geri \u00e7evrildi."); return true; }
    OutMessage = FString::Printf(TEXT("Bilinmeyen karar: %s"), *Action.ToString());
    return false;
}

FString MarketDirector::ReportText(const FMarketState& State)
{
    TArray<FString> Lines = State.DayNews;
    const FString Staff = MarketStaff::ReportText(State);
    if (!Staff.IsEmpty()) Lines.Add(Staff);
    return FString::Join(Lines, TEXT("\n"));
}

void MarketDirector::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    State.DayNews.Reset();
    MarketPromotions::CloseDay(State, Products); // running promotions, results, funded offers (G-064)
    MarketFreshness::CloseDay(State, Products);  // batches, waste, donations (G-067) - before the books
    MarketCredit::CloseDay(State);               // paydays of the credit book (G-067)
    MarketCompetitors::CloseDay(State, Products, MarketRivals::Aisles(Products)); // shares, rivals' moves, poaching (G-065)
    MarketBranches::CloseDay(State, Products);   // opening steps and the simulated day of every branch (G-068)
    MarketManagers::CloseDay(State);             // managers' wages, morale, weekly marks, the player's span (G-086b)
    MarketDepots::CloseDay(State);               // depots: a caught depot manager, missing managers, losses (G-089)
    MarketChains::CloseDay(State);               // rival chains of our countries and the world giants (Akis C2b)
    MarketBrands::CloseDay(State, Products);     // brands: sales, deals, trust, offers (karar M25)
    MarketSourcing::CloseDay(State, Products);   // supply lines: the month's minimums (G-083)
    MarketCompany::CloseDay(State);              // stores in other cities, depot, trucks, leadership (G-072)
    MarketPayments::CloseDay(State);             // card money arrives, commissions and POS rent (G-069)
    MarketOnline::CloseDay(State, Products);     // phone, web and platform orders picked from our stock (G-069)
    // Bills are paid after the day's money is in (card payout, branches, cities, online), before the books.
    MarketSuppliers::CloseDay(State); // price list, payment terms, bills due, wholesaler news (G-063)
    MarketStaff::CloseDay(State);     // till, fatigue, morale, notices, HR, accountant and the weekly tax (G-060)
    MarketEvents::CloseDay(State, Products); // decisions past their day, modifiers, snow, a new neighbourhood event (G-066)
    MarketStory::CloseDay(State, Products);  // scenes, milestones, chapters (G-066)
    MarketFinance::CloseDay(State, Products); // loans, the money trouble ladder, month-end report (G-067)
}

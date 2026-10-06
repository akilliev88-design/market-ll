#include "MarketDirector.h"
#include "MarketProductDemand.h"
#include "MarketSubsidiaries.h"
#include "MarketCountry.h"
#include "MarketCalendar.h"
#include "MarketStaff.h"
#include "MarketSuppliers.h"
#include "MarketPromotions.h"
#include "MarketEvents.h"
#include "MarketStory.h"
#include "MarketGoods.h"
#include "MarketFreshness.h"
#include "MarketOwner.h"
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
#include "MarketBanking.h"
#include "MarketAdvertising.h"
#include "MarketCommand.h"
#include "MarketRumors.h"
#include "MarketStoreDemand.h"
#include "MarketStart.h"
#include "MarketResearch.h"
#include "MarketFranchise.h"
#include "MarketStrategy.h"
#include "MarketPortfolio.h"
#include "MarketResponse.h"

float MarketDirector::TrafficFactor(const FMarketState& State, const TArray<FMarketProduct>& Products)
{
    return static_cast<float>(MarketStoreDemand::FirstStoreShoppers(State, Products, State.Day)) / static_cast<float>(MarketSimulation::ShoppersPerDay);
}

double MarketDirector::ToleranceBonus(const FMarketState& State, const FMarketProduct& Product)
{
    // G-084: the city's purchasing power (income 1.25 -> +5 points, 0.75 -> -5 points), as for every store (E3b).
    const double Income = MarketProductDemand::IncomeTolerance(MarketCountry::CityIncome(State.CountryId, State.CityId));
    return MarketEvents::Tolerance(State, MarketGoods::Classify(Product.Category)) + MarketSimulation::ToleranceBonus(State) + Income;
}

float MarketDirector::RivalPriceFactor(const FMarketState& State, const FString& Category)
{
    (void)Category;
    return MarketChains::RivalPriceFactor(State, State.CountryId, MarketStart::HomeProvince(State), State.Day);
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
    // B4: a recession or high inflation makes the basket smaller, the recovery larger.
    return MarketPayments::BudgetFactor(State, static_cast<MarketCustomers::ESegment>(Segment)) * MarketEras::BudgetFactor(State);
}

FString MarketDirector::OnCheckout(FMarketState& State, int32 CustomerId, int64 Receipt, float Roll, uint8 Method)
{
    // M36 (Mustafa 02.10.2026): no credit book; every basket settles by its payment method.
    (void)CustomerId; (void)Roll;
    return MarketPayments::Settle(State, static_cast<MarketPayments::EMethod>(FMath::Min<uint8>(Method, 3)), Receipt);
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
    if (Action == TEXT("PromoScoped")) return MarketPromotions::StartScoped(State, Products, Arg, OutMessage); // G-078 (J07)
    if (Action == TEXT("StopPromotion")) return MarketPromotions::Stop(State, Arg, OutMessage);
    if (Action == TEXT("AcceptOffer")) return MarketPromotions::AcceptOffer(State, Products, OutMessage);
    if (Action == TEXT("Decide")) return MarketEvents::Decide(State, Products, Arg, OutMessage);
    if (Action == TEXT("FreshPolicy")) return MarketFreshness::SetPolicy(State, static_cast<MarketFreshness::EPolicy>(FMath::Clamp(Arg, 0, 2)), OutMessage);
    if (Action == TEXT("TakeLoan")) return MarketFinance::TakeLoan(State, Arg, OutMessage);
    if (Action == TEXT("PromoStore")) return MarketPromotions::SetStore(State, Arg, OutMessage); // M38: -1 first store, -1000 all, a branch
    // M37: our salary (step), a dividend (step 0..2 = 25/50/100 % of what may be paid), personal money into the company.
    if (Action == TEXT("OwnerSalary")) return MarketOwner::SetSalary(State, Arg, OutMessage);
    if (Action == TEXT("Dividend")) return MarketOwner::PayDividend(State, Arg, OutMessage);
    if (Action == TEXT("OwnerCapital")) return MarketOwner::PutCapital(State, Arg, OutMessage);
    if (Action == TEXT("RepayLoan")) return MarketFinance::RepayAll(State, OutMessage);
    if (Action == TEXT("OpenBranch"))
    {
        FString Country, Province, Format;
        if (!MarketBranches::DecodeSite(Arg, Country, Province, Format)) { OutMessage = TEXT("Bilinmeyen il ya da ma\u011faza t\u00fcr\u00fc."); return false; }
        return MarketBranches::Open(State, Products, Country, Province, Format, OutMessage);
    }
    if (Action == TEXT("CloseBranch")) return MarketBranches::Close(State, Products, Arg, OutMessage);
    if (Action == TEXT("RenovateBranch")) return MarketPortfolio::Renovate(State, Products, Arg, OutMessage);   // D9b (M46): Arg = branch
    if (Action == TEXT("RelocateBranch")) return MarketPortfolio::Relocate(State, Products, Arg, OutMessage);
    if (Action == TEXT("ReformatBranch")) // D9b (M46): Arg = MarketPortfolio::EncodeFormat
    {
        int32 Branch = INDEX_NONE;
        FString Format;
        if (!MarketPortfolio::DecodeFormat(Arg, Branch, Format)) { OutMessage = TEXT("B\u00f6yle bir t\u00fcr yok."); return false; }
        return MarketPortfolio::Reformat(State, Products, Branch, Format, OutMessage);
    }
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
    // M32: online selling (Arg channel = MarketOnline::EChannel; area = MarketOnline::EncodeArea; levels 0..2/3).
    if (Action == TEXT("OnlineOpen")) return MarketOnline::Open(State, static_cast<MarketOnline::EChannel>(FMath::Clamp(Arg, 0, MarketOnline::ChannelCount - 1)), OutMessage);
    if (Action == TEXT("OnlineClose")) return MarketOnline::Close(State, static_cast<MarketOnline::EChannel>(FMath::Clamp(Arg, 0, MarketOnline::ChannelCount - 1)), OutMessage);
    if (Action == TEXT("OnlineArea")) { int32 Area = 0, Flags = 0; return MarketOnline::DecodeArea(Arg, Area, Flags) && MarketOnline::SetArea(State, Area, Flags, OutMessage); }
    if (Action == TEXT("OnlineAreaReturn")) return MarketOnline::ReturnArea(State, Arg, OutMessage);
    if (Action == TEXT("OnlineDefault")) return MarketOnline::SetDefault(State, Arg, OutMessage);
    if (Action == TEXT("DarkStore")) return MarketOnline::BuildDarkStore(State, Arg, OutMessage);
    if (Action == TEXT("OnlineFee")) return MarketOnline::SetFee(State, Arg, OutMessage);
    if (Action == TEXT("OnlineMinBasket")) return MarketOnline::SetMinBasket(State, Arg, OutMessage);
    if (Action == TEXT("OnlinePriceGap")) return MarketOnline::SetPriceGap(State, Arg, OutMessage);
    if (Action == TEXT("OnlineAds")) return MarketOnline::SetAds(State, Arg, OutMessage);
    if (Action == TEXT("Substitute")) return MarketOnline::SetSubstitute(State, Arg, OutMessage);
    if (Action == TEXT("OnlineAutoPolicy")) return MarketOnline::SetAutoPolicy(State, Arg != 0, OutMessage);
    if (Action == TEXT("OnlineHire")) return MarketOnline::HireManager(State, OutMessage);
    if (Action == TEXT("OnlineFire")) return MarketOnline::FireManager(State, OutMessage);
    // M34: advertising (AdLevel Arg = MarketAdvertising::Encode: country index x 100 + channel x 10 + level).
    if (Action == TEXT("AdLevel")) return MarketAdvertising::SetLevelArg(State, Arg, OutMessage);
    if (Action == TEXT("AdHire")) return MarketAdvertising::HireManager(State, OutMessage);
    if (Action == TEXT("AdFire")) return MarketAdvertising::FireManager(State, OutMessage);
    if (Action == TEXT("AdAuto")) return MarketAdvertising::SetAuto(State, Arg != 0, OutMessage);
    if (Action == TEXT("AdBudget")) return MarketAdvertising::SetBudget(State, Arg, OutMessage);
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
    if (Action == TEXT("ProvincePush")) // D9 (M48): Arg = MarketBranches::EncodeSite (the format is ignored)
    {
        FString Country, Province, Format;
        if (!MarketBranches::DecodeSite(Arg, Country, Province, Format)) { OutMessage = TEXT("B\u00f6yle bir il yok."); return false; }
        return MarketStrategy::StartPush(State, Country, Province, OutMessage);
    }
    if (Action == TEXT("BidChain")) return MarketChains::Bid(State, Products, Arg, OutMessage); // M29: a takeover bid
    if (Action == TEXT("ConvertStores")) return MarketChains::Convert(State, Products, Arg / 100, Arg % 100, OutMessage); // M30: chain x 100 + count
    if (Action == TEXT("SellSubsidiary")) return MarketChains::SellSubsidiary(State, Arg, OutMessage);
    if (Action == TEXT("BuyChainFinanced") || Action == TEXT("BidChainFinanced")) // M29: the shortfall from a bank, then the deal
    {
        const bool bBid = Action == TEXT("BidChainFinanced");
        if (bBid ? !MarketChains::CanBid(State, Arg, OutMessage, false) : !MarketChains::CanBuy(State, Arg, OutMessage, false)) return false;
        const int64 Cost = bBid ? MarketChains::BidPrice(State, Arg) : MarketChains::Price(State, Arg);
        FString Loan;
        // A refused bid needs no loan; a deal that goes through needs the cost and a twentieth to keep the stores running.
        if ((!bBid || MarketChains::WouldAccept(State, Arg)) && State.Cash < Cost + Cost / 20
            && !MarketBanking::FinanceAcquisition(State, Cost + Cost / 20 - State.Cash, MarketChains::YearProfit(State, Arg), Loan))
        {
            OutMessage = Loan;
            return false;
        }
        const bool bDone = bBid ? MarketChains::Bid(State, Products, Arg, OutMessage) : MarketChains::Buy(State, Products, Arg, OutMessage);
        if (!Loan.IsEmpty()) OutMessage = Loan + TEXT(" ") + OutMessage;
        return bDone;
    }
    if (Action == TEXT("CorpLoan")) // M28: Arg = MarketBanking::EncodeLoan
    {
        int32 BankIndex = 0, Step = 0, Tenor = 0;
        bool bGrace = false;
        if (!MarketBanking::DecodeLoan(Arg, BankIndex, Step, Tenor, bGrace)) { OutMessage = TEXT("B\u00f6yle bir se\u00e7enek yok."); return false; }
        return MarketBanking::Borrow(State, BankIndex, Step, Tenor, bGrace, OutMessage);
    }
    if (Action == TEXT("RepayCorpLoan")) return MarketBanking::Repay(State, Arg, OutMessage);
    // C13 (M43): the player's way to a loan and a deal: an application, the bank's answer in days, the offer.
    if (Action == TEXT("LoanApply")) // Arg = MarketBanking::EncodeLoan with step 0..4
    {
        int32 BankIndex = 0, Step = 0, Tenor = 0;
        bool bGrace = false;
        if (!MarketBanking::DecodeApp(Arg, BankIndex, Step, Tenor, bGrace)) { OutMessage = TEXT("B\u00f6yle bir se\u00e7enek yok."); return false; }
        return MarketBanking::Apply(State, BankIndex, Step, Tenor, bGrace, OutMessage);
    }
    if (Action == TEXT("LoanAccept")) return MarketBanking::AcceptApp(State, Products, Arg, OutMessage); // Arg = application id
    if (Action == TEXT("LoanDecline")) return MarketBanking::DeclineApp(State, Arg, OutMessage);
    if (Action == TEXT("BidChainAsk")) return MarketChains::OfferBid(State, Arg, OutMessage); // Arg = chain index
    if (Action == TEXT("DealComplete") || Action == TEXT("DealFinance")) // Arg = chain index: pay from the till, or ask the banks for the shortfall
    {
        const int64 Price = MarketChains::DealPrice(State, Arg);
        if (Price <= 0) { OutMessage = TEXT("Tamamlanacak bir anla\u015fma yok."); return false; }
        if (State.Cash >= Price) return MarketChains::CompleteDeal(State, Products, Arg, OutMessage);
        return MarketBanking::ApplyAcquisition(State, Arg, Price, OutMessage);
    }
    if (Action == TEXT("Restructure")) return MarketBanking::Restructure(State, Arg, OutMessage);
    if (Action == TEXT("OpenLine")) return MarketBanking::OpenLine(State, OutMessage);
    if (Action == TEXT("LineAuto")) return MarketBanking::SetLineAuto(State, Arg != 0, OutMessage);
    if (Action == TEXT("RepayLine")) return MarketBanking::RepayLine(State, OutMessage);
    if (Action == TEXT("VisitBranch")) return MarketBranches::Visit(State, Products, Arg, OutMessage); // A4 calls it when a visit starts (C3)
    if (Action == TEXT("Difficulty")) return MarketSimulation::SetDifficulty(State, Arg, OutMessage);
    if (Action == TEXT("DeclineOffer")) { MarketPromotions::DeclineOffer(State); OutMessage = TEXT("Toptanc\u0131n\u0131n teklifi geri \u00e7evrildi."); return true; }
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
    // C3 order: the books open first (the first store's day from FMarketState::CloseDay's counters), the eras set
    // the day's economy, then every system; the books close and the goals look at the finished day last.
    MarketLedger::BeginClose(State, Products); // B2
    MarketEras::CloseDay(State);               // B4: this campaign's eras (price curve, effects, news)
    MarketPromotions::CloseDay(State, Products); // running promotions, results, funded offers (G-064)
    if (State.Day % 7 == 1) // M38: the first store's manager clears slow goods once a week, like every store manager
    {
        const FString Cleared = MarketPromotions::ManagerClearance(State, Products);
        if (!Cleared.IsEmpty()) State.DayNews.Add(Cleared);
    }
    MarketFreshness::CloseDay(State, Products);  // batches, waste, donations (G-067) - before the books
    MarketStoreDemand::CloseDay(State, Products); // E2: the first store's share of its province (one store formula)
    MarketChains::Poach(State, State.Day - 1);    // a chain of the home province offers one of our people a job
    MarketBranches::CloseDay(State, Products);   // opening steps and the simulated day of every branch (G-068)
    MarketManagers::CloseDay(State);             // managers' wages, morale, weekly marks, the player's span (G-086b)
    MarketCommand::CloseDay(State, Products);    // M33: province managers propose opening or closing a branch (up the line)
    MarketDepots::CloseDay(State);               // depots: a caught depot manager, missing managers, losses (G-089)
    MarketChains::CloseDay(State);               // rival chains of our countries and the world giants (Akis C2b)
    MarketRumors::CloseDay(State);               // C13 (M43): market rumours, their sources and their day
    MarketBrands::CloseDay(State, Products);     // brands: sales, deals, trust, offers (karar M25)
    MarketSourcing::CloseDay(State, Products);   // supply lines: the month's minimums (G-083)
    MarketCompany::CloseDay(State);              // stores in other cities, depot, trucks, leadership (G-072)
    MarketSubsidiaries::CloseDay(State);         // M65: our companies abroad send the month's profit to the parent
    MarketResearch::CloseDay(State);             // M58: market studies that are ready today
    MarketFranchise::CloseDay(State);            // D6 (M67): partner stores under our brand, the month's royalty
    MarketStrategy::CloseDay(State, Products);   // D9 (M48-M50): strategic forks, province pushes, paths
    MarketPortfolio::CloseDay(State);            // D9b (M46): the stores' yearly report cards
    MarketResponse::CloseDay(State);             // D9b (M47): answers to rivals' moves and crises (managers, the player's last word)
    MarketPayments::CloseDay(State);             // card money arrives, commissions and POS rent (G-069)
    MarketOnline::CloseDay(State, Products);     // M32: online orders of every shop, per province (after the branches)
    MarketAdvertising::CloseDay(State);          // M34: the company's ads: their cost, what stays in minds, the month's mix
    // Bills are paid after the day's money is in (card payout, branches, cities, online), before the books.
    MarketSuppliers::CloseDay(State); // price list, payment terms, bills due, wholesaler news (G-063)
    MarketStaff::CloseDay(State);     // till, fatigue, morale, notices, HR, accountant and the weekly tax (G-060)
    MarketEvents::CloseDay(State, Products); // decisions past their day, modifiers, snow, a new neighbourhood event (G-066)
    MarketStory::CloseDay(State, Products);  // milestones and the identity choice (M69)
    MarketBanking::CloseDay(State);           // M28: company loans, the credit line (covers a negative till first), rating, covenants
    MarketBanking::CloseApps(State);          // C13 (M43): the banks' answers to our applications
    MarketFinance::CloseDay(State, Products); // loans, the money trouble ladder, month-end report (G-067)
    MarketCompany::TrackNationalRevenue(State); // B1 (#45): national share by revenue, after every revenue is in
    MarketLedger::EndClose(State);              // B2: the audit (till change = cash entries)
    MarketGoals::CloseDay(State, Products);     // B6: goals, firsts, records, celebrations, the rhythm guard
}

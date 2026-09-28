#include "MarketDirector.h"
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

float MarketDirector::TrafficFactor(const FMarketState& State, const TArray<FString>& Aisles)
{
    return MarketCalendar::TrafficFactor(State.Day, State.RivalSeed) * MarketRivals::TrafficFactor(State.Day, State.RivalSeed, Aisles)
        * MarketPromotions::TrafficFactor(State) * MarketCompetitors::TrafficFactor(State)
        * MarketEvents::Factor(State, MarketEvents::EModifier::Traffic) * MarketBranches::MainShopFactor(State);
}

double MarketDirector::ToleranceBonus(const FMarketState& State, const FMarketProduct& Product)
{
    return MarketEvents::Tolerance(State, MarketGoods::Classify(Product.Category));
}

float MarketDirector::RivalPriceFactor(const FMarketState& State, const TArray<FString>& Aisles, const FString& Category)
{
    return MarketCompetitors::RivalPriceFactor(State, Category, Aisles);
}

float MarketDirector::DemandWeight(const FMarketState& State, const FMarketProduct& Product)
{
    return MarketCalendar::CategoryFactor(State.Day, State.RivalSeed, Product.Category);
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
}

FString MarketDirector::OnCheckout(FMarketState& State, int32 CustomerId, int64 Receipt, float Roll)
{
    return MarketCredit::OnCheckout(State, CustomerId, Receipt, Roll);
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
        OutMessage = Paid > 0 ? FString::Printf(TEXT("Toptanc\u0131 faturalar\u0131 \u00f6dendi: %s TL."), *FString::Printf(TEXT("%lld,%02lld"), static_cast<long long>(Paid / 100), static_cast<long long>(Paid % 100)))
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
        static const TCHAR* Formats[3] = { TEXT("kucuk"), TEXT("mahalle"), TEXT("buyuk") };
        return MarketBranches::Open(State, Products, static_cast<MarketBranches::EDistrict>(FMath::Clamp(Arg / 10, 0, static_cast<int32>(MarketBranches::EDistrict::Count) - 1)),
            Formats[FMath::Clamp(Arg % 10, 0, 2)], OutMessage);
    }
    if (Action == TEXT("CloseBranch")) return MarketBranches::Close(State, Arg, OutMessage);
    if (Action == TEXT("Promote"))
    {
        int32 Newest = INDEX_NONE;
        for (int32 I = 0; I < State.Branches.Num(); ++I) if (State.Branches[I].Stage != static_cast<uint8>(MarketBranches::EStage::Closed)) Newest = I;
        return MarketBranches::Promote(State, Arg, Newest, OutMessage);
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
    MarketSuppliers::CloseDay(State); // price list, payment terms, bills due, wholesaler news (G-063)
    MarketPromotions::CloseDay(State, Products); // running promotions, results, funded offers (G-064)
    MarketFreshness::CloseDay(State, Products);  // batches, waste, donations (G-067) - before the books
    MarketCredit::CloseDay(State);               // paydays of the credit book (G-067)
    MarketCompetitors::CloseDay(State, Products, MarketRivals::Aisles(Products)); // shares, rivals' moves, poaching (G-065)
    MarketBranches::CloseDay(State, Products);   // opening steps and the simulated day of every branch (G-068)
    MarketStaff::CloseDay(State);     // till, fatigue, morale, notices, HR, accountant and the weekly tax (G-060)
    MarketEvents::CloseDay(State, Products); // decisions past their day, modifiers, snow, a new neighbourhood event (G-066)
    MarketStory::CloseDay(State, Products);  // scenes, milestones, chapters (G-066)
    MarketFinance::CloseDay(State, Products); // loans, the money trouble ladder, month-end report (G-067)
}

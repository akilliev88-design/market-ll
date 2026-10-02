#include "MarketPayments.h"
#include "MarketLedger.h"
#include "MarketCountry.h"
#include "MarketCalendar.h"
#include "MarketPrices.h"

namespace MarketPayments
{
    FString PayTl(int64 Kurus)
    {
        return MarketCountry::Money(Kurus); // G-084: the active country\'s currency
    }

    int64 DailyFee(const FMarketState& State, int64 Monthly)
    {
        return FMath::RoundToInt64(Monthly * MarketPrices::ListLevel(State.Day) / 30.0);
    }
}

float MarketPayments::CardShare(int32 GameDay)
{
    // Rough Turkish grocery card use by year; contactless payments jump in 2020.
    static const float Shares[] = { 0.25f, 0.28f, 0.32f, 0.36f, 0.40f, 0.45f, 0.50f, 0.55f, 0.60f, 0.72f, 0.75f, 0.78f };
    const int32 Year = MarketCalendar::DateOf(GameDay).Year;
    const int32 Index = FMath::Clamp(Year - 2011, 0, static_cast<int32>(UE_ARRAY_COUNT(Shares)) - 1);
    // G-084: another country starts from its own card habit (pack cardShare) and follows the same trend.
    const MarketCountry::FProfile& Country = MarketCountry::Active();
    if (Country.Id != TEXT("tr")) return FMath::Clamp(Country.CardShare + (Shares[Index] - Shares[0]) * 0.6f, 0.05f, 0.95f);
    return Shares[Index];
}

float MarketPayments::SegmentCardFactor(MarketCustomers::ESegment Segment)
{
    using MarketCustomers::ESegment;
    switch (Segment)
    {
    case ESegment::Retired: return 0.45f;
    case ESegment::Family: return 1.f;
    case ESegment::Worker: return 1.35f;
    case ESegment::Student: return 1.15f;
    case ESegment::Trader: return 0.6f;
    default: return 0.f;
    }
}

float MarketPayments::MealCardShare(MarketCustomers::ESegment Segment)
{
    return Segment == MarketCustomers::ESegment::Worker ? 0.3f : Segment == MarketCustomers::ESegment::Trader ? 0.1f : 0.f;
}

MarketPayments::EMethod MarketPayments::Choose(const FMarketState& State, MarketCustomers::ESegment Segment, float Roll)
{
    const float Meal = State.Payments.bMealCard ? MealCardShare(Segment) : 0.f;
    const float Card = FMath::Clamp(CardShare(State.Day) * SegmentCardFactor(Segment), 0.f, 0.95f);
    if (Roll < Meal) return EMethod::MealCard;
    if (Roll < Meal + Card) return State.Payments.bCard ? EMethod::Card : EMethod::NoCard;
    return EMethod::Cash;
}

bool MarketPayments::LeavesWithoutCard(FMarketState& State, float Roll)
{
    ++State.Payments.NoCard;
    if (Roll >= NoCardLeaveChance) return false;
    ++State.Payments.NoCardLost;
    return true;
}

FString MarketPayments::Settle(FMarketState& State, EMethod Method, int64 Receipt)
{
    FMarketPayments& P = State.Payments;
    if (Receipt <= 0) return FString();
    if (Method == EMethod::Card || Method == EMethod::MealCard)
    {
        const bool bMeal = Method == EMethod::MealCard;
        const int64 Fee = FMath::RoundToInt64(Receipt * (bMeal ? MealCommission : CardCommission));
        State.Cash -= Receipt;              // not in the drawer: the bank pays it in tomorrow
        MarketLedger::Post(State, MarketLedger::EAccount::CardTransfer, -Receipt); // B2
        MarketLedger::Post(State, MarketLedger::EAccount::BankFees, -Fee, false);  // the bank keeps it from the payout
        P.CardToday += Receipt - Fee;
        P.Commission += Fee;
        ++(bMeal ? P.Meal : P.Card);
        return bMeal ? FString(TEXT("Yemek kart\u0131yla \u00f6dendi.")) : FString(TEXT("Kartla \u00f6dendi."));
    }
    ++P.Cash;
    return Method == EMethod::NoCard ? FString(TEXT("\"Kart ge\u00e7miyor mu?\" diye s\u00f6ylendi, nakit \u00f6dedi.")) : FString();
}

float MarketPayments::BudgetFactor(const FMarketState& State, MarketCustomers::ESegment Segment)
{
    if (!State.Payments.bCard) return 1.f;
    return 1.f + 0.10f * FMath::Clamp(CardShare(State.Day) * SegmentCardFactor(Segment), 0.f, 0.95f);
}

float MarketPayments::TrafficFactor(const FMarketState& State)
{
    return State.Payments.bMealCard ? 1.03f : 1.f;
}

bool MarketPayments::SetCard(FMarketState& State, bool bOn, FString& OutMessage)
{
    if (State.Payments.bCard == bOn) { OutMessage = bOn ? TEXT("POS cihaz\u0131 zaten var.") : TEXT("Zaten kart al\u0131nm\u0131yor."); return false; }
    State.Payments.bCard = bOn;
    if (!bOn) State.Payments.bMealCard = false;
    OutMessage = bOn ? FString::Printf(TEXT("Bankadan POS cihaz\u0131 geldi: ayl\u0131k %s kira, kartl\u0131 sat\u0131\u015flardan %%1,8 komisyon; para ertesi g\u00fcn hesapta."),
                           *PayTl(FMath::RoundToInt64(PosMonthlyRent * MarketPrices::ListLevel(State.Day))))
                     : FString(TEXT("POS cihaz\u0131 iade edildi; art\u0131k yaln\u0131z nakit."));
    return true;
}

bool MarketPayments::SetMealCard(FMarketState& State, bool bOn, FString& OutMessage)
{
    if (bOn && !State.Payments.bCard) { OutMessage = TEXT("Yemek kart\u0131 i\u00e7in \u00f6nce POS cihaz\u0131 gerekir."); return false; }
    if (State.Payments.bMealCard == bOn) { OutMessage = bOn ? TEXT("Yemek kart\u0131 zaten al\u0131n\u0131yor.") : TEXT("Yemek kart\u0131 zaten al\u0131nm\u0131yor."); return false; }
    State.Payments.bMealCard = bOn;
    OutMessage = bOn ? TEXT("Yemek kart\u0131 anla\u015fmas\u0131 yap\u0131ld\u0131: %6 komisyon, ayl\u0131k aidat; \u00f6\u011flen i\u015f\u00e7iler u\u011frar.")
                     : TEXT("Yemek kart\u0131 anla\u015fmas\u0131 bitti.");
    return true;
}

FString MarketPayments::Summary(const FMarketState& State)
{
    const FMarketPayments& P = State.Payments;
    const int32 Total = P.LastCash + P.LastCard + P.LastMeal;
    FString Line = FString::Printf(TEXT("\u00d6deme: %s%s"), P.bCard ? TEXT("POS var") : TEXT("yaln\u0131z nakit"), P.bMealCard ? TEXT(", yemek kart\u0131") : TEXT(""));
    if (Total > 0)
        Line += FString::Printf(TEXT("  \u00b7  d\u00fcn nakit %d, kart %d, yemek kart\u0131 %d"), P.LastCash, P.LastCard, P.LastMeal);
    if (P.LastNoCard > 0)
        Line += FString::Printf(TEXT("  \u00b7  kart isteyen %d (%d'i b\u0131rakt\u0131)"), P.LastNoCard, P.LastNoCardLost);
    if (P.CardTomorrow > 0) Line += FString::Printf(TEXT("  \u00b7  bankadan gelecek %s"), *PayTl(P.CardTomorrow));
    return Line;
}

int64 MarketPayments::DailyFees(const FMarketState& State)
{
    const FMarketPayments& P = State.Payments;
    return (P.bCard ? DailyFee(State, PosMonthlyRent) : 0) + (P.bMealCard ? DailyFee(State, MealMonthlyFee) : 0);
}

void MarketPayments::CloseDay(FMarketState& State)
{
    FMarketPayments& P = State.Payments;
    // Yesterday's card money arrives; today's waits for tomorrow's close.
    State.Cash += P.CardTomorrow;
    MarketLedger::Post(State, MarketLedger::EAccount::CardTransfer, P.CardTomorrow); // B2
    P.CardTomorrow = P.CardToday;
    P.CardToday = 0;
    // Commission and fees are costs of the closed day.
    const int64 Fees = DailyFees(State);
    P.LastCommission = P.Commission;
    State.LastOperatingCost += P.Commission + Fees;
    State.LastProfit -= P.Commission + Fees;
    State.Cash -= Fees;
    MarketLedger::Post(State, MarketLedger::EAccount::BankFees, -Fees); // B2
    P.Commission = 0;
    P.LastCash = P.Cash; P.LastCard = P.Card; P.LastMeal = P.Meal; P.LastNoCard = P.NoCard; P.LastNoCardLost = P.NoCardLost;
    P.Cash = P.Card = P.Meal = P.NoCard = P.NoCardLost = 0;
    if (!P.bCard && P.LastNoCard >= 3)
        State.DayNews.Add(FString::Printf(TEXT("Bug\u00fcn %d m\u00fc\u015fteri kartla \u00f6demek istedi%s. POS cihaz\u0131 d\u00fc\u015f\u00fcn\u00fclebilir."),
            P.LastNoCard, P.LastNoCardLost > 0 ? *FString::Printf(TEXT("; %d'i sepetini b\u0131rak\u0131p gitti"), P.LastNoCardLost) : TEXT("")));
}

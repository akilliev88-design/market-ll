#include "MarketCredit.h"
#include "MarketCalendar.h"
#include "MarketPrices.h"
#include "MarketStory.h"

namespace MarketCredit
{
    uint32 CreditMix(int32 Seed, int32 Day, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), static_cast<uint32>(Day), Salt };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }

    FString CreditTl(int64 Kurus)
    {
        const int64 Abs = Kurus < 0 ? -Kurus : Kurus;
        return FString::Printf(TEXT("%s%lld,%02lld TL"), Kurus < 0 ? TEXT("-") : TEXT(""), static_cast<long long>(Abs / 100), static_cast<long long>(Abs % 100));
    }

    FMarketLoyalty* Customer(FMarketState& State, int32 CustomerId)
    {
        return State.Loyalty.FindByPredicate([CustomerId](const FMarketLoyalty& L) { return L.CustomerId == CustomerId; });
    }

    int64 LimitToday(const FMarketState& State)
    {
        return FMath::RoundToInt64(State.CreditLimit * MarketPrices::ListLevel(State.Day));
    }
}

int64 MarketCredit::Outstanding(const FMarketState& State)
{
    int64 Total = 0;
    for (const FMarketCreditAccount& A : State.Credit) Total += A.Balance;
    return Total;
}

const FMarketCreditAccount* MarketCredit::Find(const FMarketState& State, int32 CustomerId)
{
    return State.Credit.FindByPredicate([CustomerId](const FMarketCreditAccount& A) { return A.CustomerId == CustomerId; });
}

FString MarketCredit::OnCheckout(FMarketState& State, int32 CustomerId, int64 Receipt, float Roll)
{
    FMarketLoyalty* L = Customer(State, CustomerId);
    if (!L || L->Visits < MinVisits || L->Satisfaction < MinSatisfaction || Receipt <= 0) return FString();
    const float Chance = State.Story.Identity == static_cast<uint8>(MarketStory::EIdentity::Bakkal) ? BakkalAskChance : AskChance;
    if (Roll >= Chance) return FString();
    FMarketCreditAccount* A = State.Credit.FindByPredicate([CustomerId](const FMarketCreditAccount& X) { return X.CustomerId == CustomerId; });
    const int64 Owed = A ? A->Balance : 0;
    if (State.CreditLimit <= 0 || Owed + Receipt > LimitToday(State))
    {
        L->Satisfaction = FMath::Max(0.f, L->Satisfaction - 5.f);
        return State.CreditLimit <= 0 ? FString(TEXT("\"Deftere yazsan olmaz m\u0131?\" dedi; veresiye yok. Biraz k\u0131r\u0131ld\u0131."))
                                      : FString(TEXT("\"Deftere yazar m\u0131s\u0131n?\" Limit dolu dendi; biraz k\u0131r\u0131ld\u0131."));
    }
    // The sale stays a sale; the money waits in the book.
    State.Cash -= Receipt;
    if (!A)
    {
        FMarketCreditAccount New;
        New.CustomerId = CustomerId;
        New.SinceDay = State.Day;
        State.Credit.Add(New);
        A = &State.Credit.Last();
    }
    if (A->Balance == 0) A->SinceDay = State.Day;
    A->Balance += Receipt;
    L->Satisfaction = FMath::Min(100.f, L->Satisfaction + 4.f);
    return FString::Printf(TEXT("Veresiye deftere yaz\u0131ld\u0131 (%s; toplam %s)."), *CreditTl(Receipt), *CreditTl(A->Balance));
}

bool MarketCredit::SetLimit(FMarketState& State, int32 Step, FString& OutMessage)
{
    const int64 Limit = Limits[FMath::Clamp(Step, 0, 3)];
    if (Limit == State.CreditLimit) { OutMessage = TEXT("Veresiye limiti zaten b\u00f6yle."); return false; }
    State.CreditLimit = Limit;
    OutMessage = Limit <= 0 ? FString(TEXT("Veresiye kapand\u0131: yeni bor\u00e7 yaz\u0131lmaz, eski bor\u00e7lar \u00f6deme g\u00fcnlerinde gelir."))
                            : FString::Printf(TEXT("Veresiye limiti ki\u015fi ba\u015f\u0131 %s. Tan\u0131d\u0131k m\u00fc\u015fteriler bazen deftere yazd\u0131r\u0131r."), *CreditTl(LimitToday(State)));
    return true;
}

int64 MarketCredit::CollectAll(FMarketState& State, FString& OutMessage)
{
    int64 Paid = 0;
    int32 Index = 0;
    for (FMarketCreditAccount& A : State.Credit)
    {
        if (A.Balance <= 0) continue;
        // Half of them can pay at once.
        if (CreditMix(State.RivalSeed, State.Day, 0xC011u + static_cast<uint32>(Index++)) % 2u == 0u) { Paid += A.Balance; A.Balance = 0; }
        if (FMarketLoyalty* L = Customer(State, A.CustomerId)) L->Satisfaction = FMath::Max(0.f, L->Satisfaction - 6.f);
    }
    State.Cash += Paid;
    State.Credit.RemoveAll([](const FMarketCreditAccount& A) { return A.Balance <= 0; });
    OutMessage = FString::Printf(TEXT("Bor\u00e7lar istendi: %s tahsil edildi, kalan %s. Mahallede \"s\u0131k\u0131\u015ft\u0131r\u0131yor\" dediler."), *CreditTl(Paid), *CreditTl(Outstanding(State)));
    return Paid;
}

void MarketCredit::CloseDay(FMarketState& State)
{
    if (State.Credit.Num() == 0) return;
    const int32 Closed = State.Day - 1;
    const MarketCalendar::FDate Date = MarketCalendar::DateOf(Closed);
    const bool bPayday = Date.Day == 1 || Date.Day == 15;
    int64 Collected = 0, Lost = 0;
    for (FMarketCreditAccount& A : State.Credit)
    {
        const uint32 Roll = CreditMix(State.RivalSeed, Closed, 0xCED1u + static_cast<uint32>(A.CustomerId)) % 100u;
        if (A.Balance <= 0) continue;
        if (bPayday)
        {
            // Most pay everything, some half, a few nothing this time.
            const int64 Pay = Roll < 70u ? A.Balance : Roll < 90u ? A.Balance / 2 : 0;
            A.Balance -= Pay;
            Collected += Pay;
            if (A.Balance == 0) A.SinceDay = 0;
        }
        if (A.Balance > 0 && A.SinceDay > 0 && Closed - A.SinceDay >= LostAfterDays && Roll < 30u)
        {
            Lost += A.Balance;
            A.Balance = 0;
        }
    }
    State.Cash += Collected;
    State.LastProfit -= Lost;
    State.Credit.RemoveAll([](const FMarketCreditAccount& A) { return A.Balance <= 0; });
    if (Collected > 0) State.DayNews.Add(FString::Printf(TEXT("Maa\u015f g\u00fcn\u00fc: veresiye defterinden %s tahsil edildi (kalan %s)."), *CreditTl(Collected), *CreditTl(Outstanding(State))));
    if (Lost > 0) State.DayNews.Add(FString::Printf(TEXT("Veresiye defterinde %s batt\u0131: bor\u00e7lu mahalleden ta\u015f\u0131nd\u0131."), *CreditTl(Lost)));
}

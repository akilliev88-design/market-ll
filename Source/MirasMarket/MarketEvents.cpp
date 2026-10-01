#include "MarketEvents.h"
#include "MarketCast.h"
#include "MarketGoals.h"
#include "MarketEras.h"
#include "MarketCountry.h"
#include "MarketCalendar.h"
#include "MarketPrices.h"
#include "MarketStaff.h"
#include "MarketStory.h"
#include "MarketFinance.h"
#include "MarketCompetitors.h"

namespace MarketEvents
{
    using MarketGoods::EGroup;

    uint32 EventMix(int32 Seed, int32 Day, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), static_cast<uint32>(Day), Salt };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }

    FString EventTl(int64 Kurus)
    {
        return MarketCountry::Money(Kurus); // G-084: the active country\'s currency
    }

    // Today's price of an amount at the start price level (M24).
    int64 Priced(const FMarketState& State, int64 Kurus2011)
    {
        return FMath::RoundToInt64(Kurus2011 * MarketPrices::ListLevel(State.Day) / 50.0) * 50;
    }

    void RemoveModifiers(FMarketState& State, const FString& Source)
    {
        State.Modifiers.RemoveAll([&Source](const FMarketModifier& M) { return M.Source == Source; });
    }

    void ChangeSatisfaction(FMarketState& State, float Delta, int32 OnlyCustomer = INDEX_NONE)
    {
        for (FMarketLoyalty& L : State.Loyalty)
            if (OnlyCustomer == INDEX_NONE || L.CustomerId == OnlyCustomer) L.Satisfaction = FMath::Clamp(L.Satisfaction + Delta, 0.f, 100.f);
    }

    bool Carries(const FMarketState& State, const TArray<FMarketProduct>& Products, EGroup Group, int32* OutProduct = nullptr, int32 MinWarehouse = 0)
    {
        for (int32 I = 0; I < State.Stock.Num() && I < Products.Num(); ++I)
            if (MarketGoods::Classify(Products[I].Category) == Group && State.Stock[I].Capacity > 0 && State.Stock[I].Warehouse >= MinWarehouse)
            {
                if (OutProduct) *OutProduct = I;
                return true;
            }
        return false;
    }

    FMarketDecision MakeDecision(const FMarketState& State, const TCHAR* Id, const FString& Title, const FString& Text,
        std::initializer_list<FString> Options, int32 Default, int32 Arg = 0)
    {
        FMarketDecision D;
        D.Id = Id; D.Title = Title; D.Text = Text;
        for (const FString& O : Options) D.Options.Add(O);
        D.DefaultOption = Default;
        D.Deadline = State.Day; // decide during the coming day; at its close the default stands
        D.Arg = Arg;
        return D;
    }

    // Resolution of the neighbourhood events. False = nothing happened (e.g. no cash); the decision stays.
    bool ResolveEvent(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& D, int32 Option, FString& Out)
    {
        const int64 Cost = D.Arg;
        if (D.Id == TEXT("event.fridge"))
        {
            if (Option == 0)
            {
                if (State.Cash < Cost + State.OtherCosts) { Out = TEXT("Tamirci i\u00e7in kasada para yok."); return false; }
                State.OtherCosts += Cost;
                RemoveModifiers(State, D.Id);
                Out = FString::Printf(TEXT("Tamirci dolab\u0131 onard\u0131 (%s). S\u00fctler yine so\u011fuk."), *EventTl(Cost));
            }
            else Out = TEXT("Dolap tamir edilmedi: s\u00fct \u00fcr\u00fcnleri \u00fc\u00e7 g\u00fcn az satacak, m\u00fc\u015fteriler s\u00f6yleniyor.");
            return true;
        }
        if (D.Id == TEXT("event.power"))
        {
            if (Option == 0)
            {
                if (State.Cash < Cost + State.OtherCosts) { Out = TEXT("Jenerat\u00f6r i\u00e7in kasada para yok."); return false; }
                State.OtherCosts += Cost;
                RemoveModifiers(State, D.Id);
                Out = FString::Printf(TEXT("Jenerat\u00f6r kiraland\u0131 (%s); d\u00fckk\u00e2n ayd\u0131nl\u0131k kalacak."), *EventTl(Cost));
            }
            else Out = TEXT("Kesinti boyunca d\u00fckk\u00e2n lo\u015f kalacak; m\u00fc\u015fteri yar\u0131ya iner.");
            return true;
        }
        if (D.Id == TEXT("event.inspection"))
        {
            if (Option == 0)
            {
                AddModifier(State, EModifier::Traffic, AllGroups, 0.9f, State.Day, State.Day, D.Id);
                Out = TEXT("Evraklar topland\u0131, zab\u0131ta memnun ayr\u0131ld\u0131. G\u00fcn\u00fcn bir k\u0131sm\u0131 bununla ge\u00e7ti.");
            }
            else if (EventMix(State.RivalSeed, State.Day, 0x1A5Fu) % 2u == 0u)
            {
                State.OtherCosts += Cost;
                Out = FString::Printf(TEXT("Zab\u0131ta etiketlerde eksik buldu: %s ceza."), *EventTl(Cost));
            }
            else Out = TEXT("Zab\u0131ta bir g\u00f6z att\u0131 ve gitti. Bu sefer \u015fansl\u0131yd\u0131n.");
            return true;
        }
        if (D.Id == TEXT("event.wedding"))
        {
            if (Option != 0) { Out = TEXT("D\u00fc\u011f\u00fcn sipari\u015fi geri \u00e7evrildi."); return true; }
            const int32 P = D.Arg;
            constexpr int32 Units = 24;
            if (!State.Stock.IsValidIndex(P) || !Products.IsValidIndex(P) || State.Stock[P].Warehouse < Units) { Out = TEXT("Depoda o kadar mal kalmad\u0131."); return false; }
            const int64 Bill = Products[P].BasePrice * Units;
            State.Stock[P].Warehouse -= Units;
            State.Cash += Bill;
            State.Revenue += Bill;
            State.CostOfGoods += State.UnitCost(P, Products) * Units;
            ChangeSatisfaction(State, 2.f);
            Out = FString::Printf(TEXT("D\u00fc\u011f\u00fcn sipari\u015fi teslim edildi: %d adet, %s. D\u00fc\u011f\u00fcn evi herkese seni anlatt\u0131."), Units, *EventTl(Bill));
            return true;
        }
        if (D.Id == TEXT("event.complaint"))
        {
            if (Option == 0)
            {
                if (State.Cash < Cost + State.OtherCosts) { Out = TEXT("\u0130ade i\u00e7in kasada para yok."); return false; }
                State.OtherCosts += Cost;
                if (State.Loyalty.Num() > 0) ChangeSatisfaction(State, 15.f, State.Loyalty[EventMix(State.RivalSeed, State.Day, 0xC0Au) % static_cast<uint32>(State.Loyalty.Num())].CustomerId);
                Out = TEXT("Paras\u0131 iade edildi, \u00f6z\u00fcr dilendi. M\u00fc\u015fteri g\u00f6nl\u00fc al\u0131nm\u0131\u015f olarak ayr\u0131ld\u0131.");
            }
            else
            {
                ChangeSatisfaction(State, -4.f);
                AddModifier(State, EModifier::Interest, static_cast<uint8>(EGroup::Dairy), 0.8f, State.Day, State.Day + 2, D.Id);
                Out = TEXT("\u015eik\u00e2yet kabul edilmedi. Mahallede \"s\u00fct\u00fc bozuk\" laf\u0131 dola\u015f\u0131yor.");
            }
            return true;
        }
        if (D.Id == TEXT("event.condolence"))
        {
            if (Option == 0)
            {
                if (State.Cash < Cost + State.OtherCosts) { Out = TEXT("Kasada para yok."); return false; }
                State.OtherCosts += Cost;
                ChangeSatisfaction(State, 3.f);
                ChangeSatisfaction(State, 10.f, 0); // Nermin teyze notices first
                Out = TEXT("Taziye evine \u00e7ay ve \u015feker g\u00f6nderildi. Mahalle bunu unutmaz.");
            }
            else Out = TEXT("Ba\u015fsa\u011fl\u0131\u011f\u0131 dilendi.");
            return true;
        }
        Out = TEXT("Bu karar art\u0131k ge\u00e7erli de\u011fil.");
        return true;
    }
}

void MarketEvents::AddModifier(FMarketState& State, EModifier Kind, uint8 Group, float Value, int32 FirstDay, int32 LastDay, const FString& Source)
{
    FMarketModifier M;
    M.Kind = static_cast<uint8>(Kind); M.Group = Group; M.Value = Value; M.FirstDay = FirstDay; M.LastDay = LastDay; M.Source = Source;
    State.Modifiers.Add(M);
}

float MarketEvents::Factor(const FMarketState& State, EModifier Kind, MarketGoods::EGroup Group)
{
    float Result = 1.f;
    for (const FMarketModifier& M : State.Modifiers)
    {
        if (M.Kind != static_cast<uint8>(Kind) || State.Day < M.FirstDay || State.Day > M.LastDay) continue;
        if (Kind != EModifier::Traffic && M.Group != AllGroups && M.Group != static_cast<uint8>(Group)) continue;
        Result *= M.Value;
    }
    // B7: a currency shock's purchase prices (up over two weeks, slowly down after it; MarketEras).
    if (Kind == EModifier::CostFactor) Result *= MarketEras::ImportCostFactor(State, MarketEras::GroupImportShare(static_cast<uint8>(Group)), State.Day);
    return FMath::Clamp(Result, 0.1f, 5.f);
}

double MarketEvents::Tolerance(const FMarketState& State, MarketGoods::EGroup Group)
{
    double Sum = 0.0;
    for (const FMarketModifier& M : State.Modifiers)
        if (M.Kind == static_cast<uint8>(EModifier::PriceTolerance) && State.Day >= M.FirstDay && State.Day <= M.LastDay &&
            (M.Group == AllGroups || M.Group == static_cast<uint8>(Group))) Sum += M.Value;
    return FMath::Clamp(Sum, -0.3, 0.3);
}

void MarketEvents::Offer(FMarketState& State, const FMarketDecision& Decision)
{
    State.Decisions.Add(Decision);
}

const FMarketDecision* MarketEvents::Pending(const FMarketState& State)
{
    return State.Decisions.Num() > 0 ? &State.Decisions[0] : nullptr;
}

bool MarketEvents::Decide(FMarketState& State, const TArray<FMarketProduct>& Products, int32 Option, FString& OutMessage)
{
    if (State.Decisions.Num() == 0) { OutMessage = TEXT("Bekleyen bir karar yok."); return false; }
    const FMarketDecision D = State.Decisions[0];
    if (!D.Options.IsValidIndex(Option)) { OutMessage = TEXT("B\u00f6yle bir se\u00e7enek yok."); return false; }
    const bool bDone = D.Id.StartsWith(TEXT("story.")) ? MarketStory::Resolve(State, Products, D, Option, OutMessage)
        : D.Id.StartsWith(TEXT("finance.")) ? MarketFinance::Resolve(State, Products, D, Option, OutMessage)
        : D.Id.StartsWith(TEXT("rival.")) ? MarketCompetitors::Resolve(State, Products, D, Option, OutMessage)
        : ResolveEvent(State, Products, D, Option, OutMessage);
    if (bDone && State.Decisions.Num() > 0 && State.Decisions[0].Id == D.Id) State.Decisions.RemoveAt(0);
    return bDone;
}

bool MarketEvents::Happened(const FMarketState& State, const FString& Id, int32 WithinDays)
{
    for (const FString& Entry : State.EventLog)
    {
        int32 At = INDEX_NONE;
        if (!Entry.FindLastChar(TEXT('@'), At) || Entry.Left(At) != Id) continue;
        const int32 Day = FCString::Atoi(*Entry.Mid(At + 1));
        if (State.Day - Day <= WithinDays) return true;
    }
    return false;
}

void MarketEvents::Log(FMarketState& State, const FString& Id)
{
    State.EventLog.Add(FString::Printf(TEXT("%s@%d"), *Id, State.Day));
    if (State.EventLog.Num() > 200) State.EventLog.RemoveAt(0, State.EventLog.Num() - 200);
}

bool MarketEvents::Trigger(FMarketState& State, const TArray<FMarketProduct>& Products, const FString& Id)
{
    TArray<FString>& News = State.DayNews;
    const int32 Tomorrow = State.Day;
    if (Id == TEXT("event.fridge"))
    {
        if (!Carries(State, Products, EGroup::Dairy)) return false;
        const int64 Cost = Priced(State, 6000);
        Offer(State, MakeDecision(State, TEXT("event.fridge"), TEXT("S\u00fct dolab\u0131 ar\u0131zaland\u0131"),
            FString::Printf(TEXT("Dolab\u0131n motoru \u00f6tt\u00fc, s\u00fctler \u0131l\u0131n\u0131yor. Tamirci yar\u0131n gelebilir: %s. Beklersen s\u00fct \u00fcr\u00fcnleri \u00fc\u00e7 g\u00fcn az satar."), *EventTl(Cost)),
            { FString::Printf(TEXT("Tamirciyi \u00e7a\u011f\u0131r (%s)"), *EventTl(Cost)), FString(TEXT("Bekle, idare ederiz")) }, 1, static_cast<int32>(Cost)));
        AddModifier(State, EModifier::Interest, static_cast<uint8>(EGroup::Dairy), 0.4f, Tomorrow, Tomorrow + 2, Id);
    }
    else if (Id == TEXT("event.power"))
    {
        const int64 Cost = Priced(State, 3000);
        Offer(State, MakeDecision(State, TEXT("event.power"), TEXT("Yar\u0131n elektrik kesintisi"),
            FString::Printf(TEXT("TEDA\u015e yar\u0131n 10.00-15.00 aras\u0131 kesinti duyurdu. Jenerat\u00f6r kiralarsan d\u00fckk\u00e2n a\u00e7\u0131k ve ayd\u0131nl\u0131k kal\u0131r (%s)."), *EventTl(Cost)),
            { FString::Printf(TEXT("Jenerat\u00f6r kirala (%s)"), *EventTl(Cost)), FString(TEXT("Karanl\u0131kta idare et")) }, 1, static_cast<int32>(Cost)));
        AddModifier(State, EModifier::Traffic, AllGroups, 0.55f, Tomorrow, Tomorrow, Id);
    }
    else if (Id == TEXT("event.inspection"))
    {
        if (State.Day < 6) return false;
        if (MarketStaff::HasAccountant(State))
        {
            News.Add(TEXT("Zab\u0131ta denetime geldi; Necati Bey'in haz\u0131rlad\u0131\u011f\u0131 evraklar eksiksizdi. Sorun yok."));
        }
        else
        {
            const int64 Fine = Priced(State, 5000);
            Offer(State, MakeDecision(State, TEXT("event.inspection"), TEXT("Zab\u0131ta denetimi"),
                FString::Printf(TEXT("Yar\u0131n zab\u0131ta etiket ve ruhsat denetimine gelecek. Evraklar\u0131 toplamak biraz vakit al\u0131r; toplamazsan %s ceza riski var."), *EventTl(Fine)),
                { FString(TEXT("Evraklar\u0131 hemen topla")), FString(TEXT("\u015eans\u0131n\u0131 dene")) }, 1, static_cast<int32>(Fine)));
        }
    }
    else if (Id == TEXT("event.wedding"))
    {
        int32 Product = INDEX_NONE;
        if (!Carries(State, Products, EGroup::Drinks, &Product, 24)) return false;
        const int64 Bill = Products[Product].BasePrice * 24;
        Offer(State, MakeDecision(State, TEXT("event.wedding"), TEXT("Mahallede d\u00fc\u011f\u00fcn"),
            FString::Printf(TEXT("Kar\u015f\u0131 apartmanda d\u00fc\u011f\u00fcn var. Damad\u0131n babas\u0131 24 adet %s istiyor, liste fiyat\u0131ndan (%s), pe\u015fin."),
                *(Products[Product].RealName.IsEmpty() ? Products[Product].Id : Products[Product].RealName), *EventTl(Bill)),
            { FString(TEXT("Kabul et, depodan ver")), FString(TEXT("Geri \u00e7evir")) }, 1, Product));
    }
    else if (Id == TEXT("event.complaint"))
    {
        if (!Carries(State, Products, EGroup::Dairy)) return false;
        const int64 Cost = Priced(State, 300);
        Offer(State, MakeDecision(State, TEXT("event.complaint"), TEXT("Bozuk s\u00fct \u015fik\u00e2yeti"),
            FString::Printf(TEXT("Bir m\u00fc\u015fteri d\u00fcn ald\u0131\u011f\u0131 s\u00fct\u00fcn ek\u015fi \u00e7\u0131kt\u0131\u011f\u0131n\u0131 s\u00f6yl\u00fcyor ve paras\u0131n\u0131 istiyor (%s)."), *EventTl(Cost)),
            { FString(TEXT("Paras\u0131n\u0131 iade et, \u00f6z\u00fcr dile")), FString(TEXT("Kabul etme")) }, 1, static_cast<int32>(Cost)));
    }
    else if (Id == TEXT("event.condolence"))
    {
        const int64 Cost = Priced(State, 1500);
        Offer(State, MakeDecision(State, TEXT("event.condolence"), TEXT("Taziye evi"),
            FString::Printf(TEXT("Soka\u011f\u0131n eski esnaflar\u0131ndan H\u00fcseyin Usta vefat etti. Mahallede adettir: taziye evine \u00e7ay ve \u015feker g\u00f6nderilir (%s)."), *EventTl(Cost)),
            { FString(TEXT("\u00c7ay ve \u015feker g\u00f6nder")), FString(TEXT("Ba\u015fsa\u011fl\u0131\u011f\u0131 dile")) }, 1, static_cast<int32>(Cost)));
    }
    else if (Id == TEXT("event.roadworks"))
    {
        AddModifier(State, EModifier::Traffic, AllGroups, 0.8f, Tomorrow, Tomorrow + 4, Id);
        News.Add(TEXT("Belediye soka\u011f\u0131n kald\u0131r\u0131m\u0131n\u0131 s\u00f6kt\u00fc: be\u015f g\u00fcn boyunca d\u00fckk\u00e2na gelmek zor, m\u00fc\u015fteri azalacak."));
    }
    else if (Id == TEXT("event.derby"))
    {
        AddModifier(State, EModifier::Interest, static_cast<uint8>(EGroup::Drinks), 1.5f, Tomorrow, Tomorrow, Id);
        AddModifier(State, EModifier::Interest, static_cast<uint8>(EGroup::Snacks), 1.6f, Tomorrow, Tomorrow, Id);
        AddModifier(State, EModifier::Interest, static_cast<uint8>(EGroup::Sweets), 1.2f, Tomorrow, Tomorrow, Id);
        News.Add(TEXT("Yar\u0131n ak\u015fam b\u00fcy\u00fck derbi var: i\u00e7ecek ve \u00e7erez \u00e7ok aranacak. Raflar dolu olsun."));
    }
    else if (Id == TEXT("event.fair"))
    {
        // B6 rhythm guard: a pleasant day after a quiet stretch.
        AddModifier(State, EModifier::Traffic, AllGroups, 1.25f, Tomorrow, Tomorrow, Id);
        AddModifier(State, EModifier::Interest, static_cast<uint8>(EGroup::Drinks), 1.3f, Tomorrow, Tomorrow, Id);
        News.Add(TEXT("Yar\u0131n mahallede \u015fenlik var: sokak kalabal\u0131k olacak, %25 daha \u00e7ok m\u00fc\u015fteri beklenir. \u0130\u00e7ecek raf\u0131n\u0131 doldur."));
    }
    else if (Id == TEXT("event.newbuilding"))
    {
        if (Happened(State, Id, 180)) return false;
        AddModifier(State, EModifier::Traffic, AllGroups, 1.05f, Tomorrow, Tomorrow + 29, Id);
        News.Add(TEXT("Soka\u011f\u0131n ba\u015f\u0131ndaki yeni apartmana ta\u015f\u0131nmalar ba\u015flad\u0131: bir ay boyunca %5 daha \u00e7ok m\u00fc\u015fteri gelir."));
    }
    else if (Id == TEXT("event.truck"))
    {
        // Orders placed during the coming day arrive one day later than usual.
        State.DeliveryDelayDay = Tomorrow;
        News.Add(FString::Printf(TEXT("%s arad\u0131 (%s): kamyon ar\u0131zaland\u0131. Yar\u0131n verece\u011fin sipari\u015f bir g\u00fcn ge\u00e7 gelir; depoyu ona g\u00f6re planla."), *MarketCast::Salesman(), *MarketCast::Wholesaler()));
    }
    else return false;
    Log(State, Id);
    // B6 rhythm guard: something happened today; bad ones are counted for the next seven days.
    const int32 Closed = FMath::Max(1, State.Day - 1);
    State.Goals.LastLivelyDay = FMath::Max(State.Goals.LastLivelyDay, Closed);
    if (MarketGoals::IsBadEvent(Id)) State.Goals.BadEventDays.Add(Closed);
    return true;
}

void MarketEvents::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    const int32 Closed = State.Day - 1;
    if (Closed < 1) return;
    TArray<FString>& News = State.DayNews;

    State.Modifiers.RemoveAll([&State](const FMarketModifier& M) { return M.LastDay < State.Day; });

    // Decisions whose day has passed take their default.
    for (int32 I = 0; I < State.Decisions.Num();)
    {
        if (State.Decisions[I].Deadline >= State.Day) { ++I; continue; }
        const FMarketDecision D = State.Decisions[I];
        State.Decisions.RemoveAt(I);
        State.Decisions.Insert(D, 0);
        FString Message;
        const bool bResolved = Decide(State, Products, FMath::Clamp(D.DefaultOption, 0, FMath::Max(0, D.Options.Num() - 1)), Message);
        if (!bResolved && State.Decisions.Num() > 0 && State.Decisions[0].Id == D.Id) State.Decisions.RemoveAt(0);
        if (!Message.IsEmpty()) News.Add(D.Title + TEXT(": ") + Message);
        I = 0;
    }

    // Snow closes the road for the wholesaler's truck.
    if (MarketCalendar::Info(State.Day, State.RivalSeed).Weather == MarketCalendar::EWeather::Snow && State.DeliveryDelayDay != State.Day)
    {
        State.DeliveryDelayDay = State.Day;
        News.Add(TEXT("Yar\u0131n kar bekleniyor, yollar kapanabilir: yar\u0131n verece\u011fin sipari\u015f bir g\u00fcn ge\u00e7 gelir."));
    }

    // A new event now and then.
    if (Closed < 3 || State.Decisions.Num() >= MaxPending) return;
    if (static_cast<int32>(EventMix(State.RivalSeed, Closed, 0xE7E0u) % 100u) >= EventChancePercent) return;
    struct FCandidate { const TCHAR* Id; int32 Weight; };
    const MarketCalendar::FDayInfo Day = MarketCalendar::Info(State.Day, State.RivalSeed);
    const bool bSummer = Day.Season == MarketCalendar::ESeason::Summer;
    const FCandidate Candidates[] =
    {
        { TEXT("event.fridge"), bSummer ? 4 : 2 }, { TEXT("event.power"), 2 }, { TEXT("event.inspection"), 2 },
        { TEXT("event.wedding"), MarketCalendar::IsWeekend(State.Day) || bSummer ? 3 : 1 }, { TEXT("event.complaint"), 2 },
        { TEXT("event.roadworks"), 1 }, { TEXT("event.derby"), 1 }, { TEXT("event.condolence"), 1 }, { TEXT("event.truck"), 1 },
    };
    int32 Total = 0;
    for (const FCandidate& C : Candidates) if (!Happened(State, C.Id, CooldownDays)) Total += C.Weight;
    if (Total <= 0) return;
    int32 Roll = static_cast<int32>(EventMix(State.RivalSeed, Closed, 0xE7E1u) % static_cast<uint32>(Total));
    for (const FCandidate& C : Candidates)
    {
        if (Happened(State, C.Id, CooldownDays)) continue;
        Roll -= C.Weight;
        if (Roll < 0)
        {
            // B6 rhythm guard: after a pile of bad luck the next bad event waits.
            if (MarketGoals::IsBadEvent(C.Id) && MarketGoals::HoldBadEvent(State)) { ++State.Goals.HeldBadEvents; break; }
            if (!Trigger(State, Products, C.Id)) Log(State, C.Id); // conditions not met: skip it for a while
            break;
        }
    }
}

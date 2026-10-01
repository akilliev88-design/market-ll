#include "MarketCommand.h"
#include "MarketBranches.h"
#include "MarketManagers.h"
#include "MarketCountry.h"
#include "MarketCalendar.h"
#include "MarketEvents.h"

namespace MarketCommandLocal
{
    bool IsOpen(const FMarketBranch& B) { return B.Stage == static_cast<uint8>(MarketBranches::EStage::Open); }
    FString Tl(int64 Kurus) { return MarketCountry::Money(Kurus); }

    FMarketDecision Card(const FMarketState& State, const FString& Id, const FString& Title, const FString& Text, int32 Arg)
    {
        FMarketDecision D;
        D.Id = Id; D.Title = Title; D.Text = Text;
        D.Options = { FString(TEXT("Onayla")), FString(TEXT("Reddet")) };
        D.DefaultOption = 1;
        D.Deadline = State.Day + 9;
        D.Arg = Arg;
        return D;
    }

    bool Pending(const FMarketState& State, const FString& Id)
    {
        return State.Decisions.ContainsByPredicate([&Id](const FMarketDecision& D) { return D.Id == Id; });
    }

    FString Via(const FMarketState& State, const FString& Country, const FString& Province)
    {
        const FString Who = MarketCommand::Forwarder(State, Country, Province);
        return Who.IsEmpty() ? FString(TEXT(" Karar senin.")) : FString::Printf(TEXT(" %s inceledi, onay\u0131na sunuyor."), *Who);
    }
}

FString MarketCommand::Forwarder(const FMarketState& State, const FString& Country, const FString& Province)
{
    using MarketManagers::ELevel;
    const MarketCountry::FCity* City = MarketCountry::FindCity(Country, Province);
    int32 Who = MarketManagers::FindManager(State, ELevel::Country, Country, Country);
    FString Title = TEXT("\u00dclke m\u00fcd\u00fcr\u00fc");
    if (Who == INDEX_NONE && City) { Who = MarketManagers::FindManager(State, ELevel::Region, Country, City->Region); Title = TEXT("B\u00f6lge direkt\u00f6r\u00fc"); }
    if (Who == INDEX_NONE && City) { Who = MarketManagers::FindManager(State, ELevel::SubRegion, Country, City->SubRegion); Title = TEXT("B\u00f6lge m\u00fcd\u00fcr\u00fc"); }
    return Who == INDEX_NONE ? FString() : FString::Printf(TEXT("%s %s"), *Title, *State.Management.Managers[Who].Name);
}

void MarketCommand::CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products)
{
    using namespace MarketCommandLocal;
    if (State.Day <= 1 || MarketCalendar::DateOf(State.Day).Day != 1) return;   // the month that just ended

    // 1. Months in the red; a province manager proposes closing.
    for (int32 I = 0; I < State.Branches.Num(); ++I)
    {
        FMarketBranch& B = State.Branches[I];
        if (!IsOpen(B) || State.Day - B.OpenedDay < MatureDays) { B.LossMonths = 0; continue; }
        B.LossMonths = B.Last30Profit < 0 ? B.LossMonths + 1 : 0;
        if (B.LossMonths < LossMonthsToClose || State.Day < B.QuietUntil) continue;
        const FString Country = MarketBranches::CountryOf(State, B);
        const int32 Boss = MarketManagers::FindManager(State, MarketManagers::ELevel::Province, Country, B.Province);
        const FString Id = FString::Printf(TEXT("command.close:%d"), I);
        if (Boss == INDEX_NONE || Pending(State, Id)) continue;
        const FString Text = FString::Printf(TEXT("%s il m\u00fcd\u00fcr\u00fc %s: \"%s %d ayd\u0131r zararda (son 30 g\u00fcn %s). D\u00fczelmiyor; kapatal\u0131m, mal\u0131 di\u011fer ma\u011fazalara alal\u0131m.\"%s"),
            *MarketBranches::SiteOf(State, B).Name, *State.Management.Managers[Boss].Name, *B.Name, B.LossMonths, *Tl(B.Last30Profit), *Via(State, Country, B.Province));
        MarketEvents::Offer(State, Card(State, Id, FString::Printf(TEXT("%s kapans\u0131n m\u0131?"), *B.Name), Text, I));
    }

    // 2. A province manager whose shops all earn proposes one more.
    for (const FMarketManager& M : State.Management.Managers)
    {
        if (M.Level != static_cast<uint8>(MarketManagers::ELevel::Province)) continue;
        const FString Key = M.Country + TEXT("|") + M.Area;
        if (State.Day < State.ProposalQuiet.FindRef(Key)) continue;
        const FString Id = TEXT("command.open:") + Key;
        if (Pending(State, Id)) continue;
        int32 Shops = 0;
        int64 Profit = 0;
        bool bAllEarn = true;
        for (const FMarketBranch& B : State.Branches)
        {
            if (!IsOpen(B) || B.Province != M.Area || MarketBranches::CountryOf(State, B) != M.Country) continue;
            ++Shops;
            Profit += B.Last30Profit;
            if (State.Day - B.OpenedDay >= MatureDays && B.Last30Profit <= 0) bAllEarn = false;
        }
        if (Shops < 2 || !bAllEarn || Profit <= 0) continue;
        const MarketBranches::FSite Site = MarketBranches::SiteOf(State, M.Country, M.Area);
        if (!Site.bValid || MarketBranches::Room(Site) <= MarketBranches::ShopsIn(State, M.Country, M.Area)) continue;
        FString Reason;
        const FString Format = TEXT("mahalle");
        if (!MarketBranches::CanOpen(State, Products, M.Country, M.Area, Format, Reason)) continue;
        const int64 Cost = MarketBranches::OpeningCost(State, Products, M.Country, M.Area, Format);
        // C8 (Codex C6: 300 of 401 proposals turned down): he asks only when the till keeps the opening and three
        // months of the whole network's fixed costs with the new shop.
        int64 Network = MarketBranches::MonthlyFixedCost(State, M.Country, M.Area, Format);
        for (const FMarketBranch& Other : State.Branches)
            if (IsOpen(Other)) Network += MarketBranches::MonthlyFixedCost(State, MarketBranches::CountryOf(State, Other), Other.Province, Other.Format);
        if (State.Cash < Cost + 3 * Network) continue;
        const int32 FormatIndex = MarketBranches::FormatIds().IndexOfByKey(Format);
        const FString Text = FString::Printf(TEXT("%s il m\u00fcd\u00fcr\u00fc %s: \"%d ma\u011fazam\u0131z\u0131n hepsi k\u00e2rda (son 30 g\u00fcn toplam %s), ilde yer var. Bir mahalle marketi daha a\u00e7al\u0131m: a\u00e7\u0131l\u0131\u015f %s, ayl\u0131k sabit gider %s.\"%s"),
            *Site.Name, *M.Name, Shops, *Tl(Profit), *Tl(Cost), *Tl(MarketBranches::MonthlyFixedCost(State, M.Country, M.Area, Format)), *Via(State, M.Country, M.Area));
        MarketEvents::Offer(State, Card(State, Id, FString::Printf(TEXT("%s: yeni ma\u011faza \u00f6nerisi"), *Site.Name), Text, FormatIndex));
        State.ProposalQuiet.Add(Key, State.Day + OpenQuietDays);
    }
}

bool MarketCommand::Resolve(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& D, int32 Option, FString& OutMessage)
{
    if (D.Id.StartsWith(TEXT("command.close:")))
    {
        const int32 Index = D.Arg;
        if (!State.Branches.IsValidIndex(Index) || State.Branches[Index].Stage == static_cast<uint8>(MarketBranches::EStage::Closed)) { OutMessage = TEXT("Bu \u015fube zaten kapal\u0131."); return true; }
        if (Option != 0)
        {
            State.Branches[Index].QuietUntil = State.Day + CloseQuietDays;
            State.Branches[Index].LossMonths = 0;
            OutMessage = FString::Printf(TEXT("%s a\u00e7\u0131k kal\u0131yor; il m\u00fcd\u00fcr\u00fc \u00fc\u00e7 ay bu konuyu a\u00e7maz."), *State.Branches[Index].Name);
            return true;
        }
        MarketBranches::Close(State, Products, Index, OutMessage);
        return true;
    }
    if (D.Id.StartsWith(TEXT("command.open:")))
    {
        if (Option != 0) { OutMessage = TEXT("Yeni ma\u011faza \u00f6nerisi reddedildi; il m\u00fcd\u00fcr\u00fc alt\u0131 ay sonra yeniden bakar."); return true; }
        FString Key = D.Id.RightChop(13), Country, Province;
        Key.Split(TEXT("|"), &Country, &Province);
        const TArray<FString>& Formats = MarketBranches::FormatIds();
        const FString Format = Formats.IsValidIndex(D.Arg) ? Formats[D.Arg] : FString(TEXT("mahalle"));
        if (!MarketBranches::Open(State, Products, Country, Province, Format, OutMessage)) OutMessage = TEXT("A\u00e7\u0131lamad\u0131: ") + OutMessage;
        return true;
    }
    OutMessage = TEXT("Bu karar art\u0131k ge\u00e7erli de\u011fil.");
    return true;
}

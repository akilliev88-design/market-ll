#include "MarketSubsidiaries.h"
#include "MarketBranches.h"
#include "MarketCalendar.h"
#include "MarketCountry.h"
#include "MarketLedger.h"
#include "MarketPrices.h"

namespace MarketSubsidiariesLocal
{
    // A pack without legal forms takes the default country's (M52: the forms live in the packs only).
    const TArray<FString>& FormsOf(const FString& Country)
    {
        const MarketCountry::FProfile& Pack = MarketCountry::FindOrDefault(Country);
        return Pack.LegalForms.Num() > 0 ? Pack.LegalForms : MarketCountry::Default().LegalForms;
    }

    FString Join(const FString& Name, const FString& Form)
    {
        return Form.IsEmpty() ? Name : Name + TEXT(" ") + Form;
    }

    uint32 Hash(const FString& Text)
    {
        uint32 H = 2166136261u;
        for (const TCHAR C : Text) { H ^= static_cast<uint32>(C); H *= 16777619u; }
        return H;
    }

    FString CountryOfStore(const FMarketState& State, int32 Store)
    {
        if (Store == MarketLedger::FamilyShop) return State.CountryId;
        return State.Branches.IsValidIndex(Store) ? MarketBranches::CountryOf(State, State.Branches[Store]) : FString();
    }

    FString Clean(const FString& Name)
    {
        FString Out = Name.TrimStartAndEnd();
        while (Out.Contains(TEXT("  "))) Out = Out.Replace(TEXT("  "), TEXT(" "));
        return Out;
    }
}

FString MarketSubsidiaries::Brand(const FMarketState& State)
{
    return State.Company.BrandName.IsEmpty() ? FString(TEXT("Miras")) : State.Company.BrandName;
}

bool MarketSubsidiaries::SetBrand(FMarketState& State, const FString& Name, FString& OutMessage)
{
    const FString Clean = MarketSubsidiariesLocal::Clean(Name);
    if (Clean.IsEmpty() || Clean.Len() > 24) { OutMessage = TEXT("Marka ad\u0131 1-24 harf olmal\u0131."); return false; }
    if (Clean == Brand(State)) { OutMessage = TEXT("Marka zaten bu."); return false; }
    const FString Old = Brand(State);
    State.Company.BrandName = Clean;
    // Registered names that still carry the old brand follow it (a written name stays as the player wrote it).
    for (FMarketSubsidiary& S : State.Company.Subsidiaries)
        if (S.LegalName.StartsWith(Old + TEXT(" ")) || S.LegalName == Old) S.LegalName = Clean + S.LegalName.Mid(Old.Len());
    OutMessage = FString::Printf(TEXT("Marka art\u0131k \"%s\". Tabelalarda, haberlerde ve listelerde bu ad ge\u00e7er."), *Clean);
    return true;
}

TArray<FString> MarketSubsidiaries::SuggestedNames(const FMarketState& State, const FString& Country)
{
    TArray<FString> Names;
    for (const FString& Form : MarketSubsidiariesLocal::FormsOf(Country)) Names.Add(MarketSubsidiariesLocal::Join(Brand(State), Form));
    if (Names.Num() == 0) Names.Add(Brand(State));
    return Names;
}

FString MarketSubsidiaries::DefaultLegalName(const FMarketState& State, const FString& Country)
{
    return SuggestedNames(State, Country)[0];
}

const FMarketSubsidiary* MarketSubsidiaries::Find(const FMarketState& State, const FString& Country)
{
    const FString Id = Country.IsEmpty() ? State.CountryId : Country;
    return State.Company.Subsidiaries.FindByPredicate([&Id](const FMarketSubsidiary& S) { return S.Country == Id; });
}

FString MarketSubsidiaries::LegalName(const FMarketState& State, const FString& Country)
{
    const FMarketSubsidiary* S = Find(State, Country);
    return S && !S->LegalName.IsEmpty() ? S->LegalName : DefaultLegalName(State, Country.IsEmpty() ? State.CountryId : Country);
}

bool MarketSubsidiaries::SetLegalName(FMarketState& State, const FString& Country, const FString& Name, FString& OutMessage)
{
    const FString Id = Country.IsEmpty() ? State.CountryId : Country;
    FMarketSubsidiary* S = State.Company.Subsidiaries.FindByPredicate([&Id](const FMarketSubsidiary& X) { return X.Country == Id; });
    if (!S) { OutMessage = TEXT("O \u00fclkede \u015firketimiz yok."); return false; }
    const FString Clean = MarketSubsidiariesLocal::Clean(Name);
    if (Clean.IsEmpty() || Clean.Len() > MaxNameLength) { OutMessage = FString::Printf(TEXT("\u015eirket ad\u0131 1-%d harf olmal\u0131."), MaxNameLength); return false; }
    if (Clean == S->LegalName) { OutMessage = TEXT("\u015eirketin ad\u0131 zaten bu."); return false; }
    S->LegalName = Clean;
    OutMessage = FString::Printf(TEXT("Tescilli ad: %s. Halk yine \"%s\" der."), *Clean, *Brand(State));
    return true;
}

FString MarketSubsidiaries::NextSuggestion(const FMarketState& State, const FString& Country)
{
    const TArray<FString> Names = SuggestedNames(State, Country);
    const int32 At = Names.IndexOfByKey(LegalName(State, Country));
    return Names[(At + 1) % Names.Num()];
}

int64 MarketSubsidiaries::SetupCost(const FMarketState& State, const FString& Country)
{
    const FString Id = Country.IsEmpty() ? State.CountryId : Country;
    if (Id == State.CountryId || Find(State, Id)) return 0;
    const MarketCountry::FProfile& Pack = MarketCountry::FindOrDefault(Id);
    const double Start = Pack.SetupCost > 0.0 ? Pack.SetupCost : MarketCountry::Default().SetupCost;
    const int64 HomeLevel = FMath::RoundToInt64(Start * MarketPrices::ListLevel(FMath::Max(1, State.Day)));
    return MarketBranches::InHome(MarketBranches::MoneyOf(State, Id, State.Day), HomeLevel); // E4: in the country's money
}

bool MarketSubsidiaries::Ensure(FMarketState& State, const FString& Country, bool bPay)
{
    const FString Id = Country.IsEmpty() ? State.CountryId : Country;
    if (Id.IsEmpty() || Find(State, Id)) return false;
    const int64 Cost = bPay ? SetupCost(State, Id) : 0;
    FMarketSubsidiary S;
    S.Country = Id;
    S.LegalName = DefaultLegalName(State, Id);
    S.FoundedDay = State.Day;
    State.Company.Subsidiaries.Add(S);
    if (Id == State.CountryId) return true; // the parent company: no news, no cost
    if (Cost > 0) MarketLedger::AddStoreCost(State, Cost, MarketLedger::HeadOfficeStore); // lawyers, registry, notary
    const MarketCountry::FProfile& Pack = MarketCountry::FindOrDefault(Id);
    State.DayNews.Add(FString::Printf(TEXT("%s kuruldu (%s). Ana \u015firket %s; k\u00e2r\u0131 her ay ana \u015firkete aktar\u0131l\u0131r.%s Ad\u0131n\u0131 Finans sayfas\u0131ndan de\u011fi\u015ftirebilirsin."),
        *S.LegalName, *Pack.Name, *LegalName(State, State.CountryId),
        Cost > 0 ? *FString::Printf(TEXT(" Kurulu\u015f masraf\u0131 %s."), *MarketCountry::Money(Cost)) : TEXT("")));
    return true;
}

FString MarketSubsidiaries::ChainLegalName(const FMarketChain& Chain)
{
    const TArray<FString>& Forms = MarketSubsidiariesLocal::FormsOf(Chain.Country);
    if (Forms.Num() == 0) return Chain.Name;
    return MarketSubsidiariesLocal::Join(Chain.Name, Forms[MarketSubsidiariesLocal::Hash(Chain.Id) % static_cast<uint32>(Forms.Num())]);
}

void MarketSubsidiaries::CloseDay(FMarketState& State)
{
    Ensure(State, State.CountryId, false); // the parent company always exists
    const MarketCalendar::FDate Today = MarketCalendar::DateOf(State.Day);
    if (Today.Day != 1 || State.Day <= 1) return;
    const int32 Closed = State.Day - 1;
    const MarketCalendar::FDate Last = MarketCalendar::DateOf(Closed);
    const int32 From = FMath::Max(1, MarketCalendar::GameDayOf(Last.Year, Last.Month, 1));
    for (FMarketSubsidiary& S : State.Company.Subsidiaries)
    {
        if (S.Country == State.CountryId) continue;
        int64 Profit = 0;
        for (int32 Store = 0; Store < State.Branches.Num(); ++Store)
            if (MarketSubsidiariesLocal::CountryOfStore(State, Store) == S.Country)
                Profit += MarketLedger::Statement(State, From, Closed, Store).NetProfit;
        const float Rate = MarketCountry::FindOrDefault(S.Country).DividendWithholding;
        const int64 Withheld = Profit > 0 ? FMath::RoundToInt64(Profit * static_cast<double>(Rate)) : 0;
        S.LastMonthProfit = Profit;
        S.LastWithheld = Withheld;
        S.LastTransfer = FMath::Max<int64>(0, Profit - Withheld);
        S.TotalTransferred += S.LastTransfer;
        if (Withheld > 0)
        {
            State.Cash -= Withheld;
            State.LastProfit -= Withheld;
            MarketLedger::Post(State, MarketLedger::EAccount::Withholding, -Withheld, true, MarketLedger::HeadOfficeStore);
        }
        if (Profit != 0)
            State.DayNews.Add(Profit > 0
                ? FString::Printf(TEXT("%s: ge\u00e7en ay %s k\u00e2r; %s ana \u015firkete aktar\u0131ld\u0131%s."), *S.LegalName, *MarketCountry::Money(Profit), *MarketCountry::Money(S.LastTransfer),
                    Withheld > 0 ? *FString::Printf(TEXT(", %s stopaj \u00f6dendi"), *MarketCountry::Money(Withheld)) : TEXT(""))
                : FString::Printf(TEXT("%s: ge\u00e7en ay %s zarar; ana \u015firket kar\u015f\u0131lad\u0131."), *S.LegalName, *MarketCountry::Money(-Profit)));
    }
}

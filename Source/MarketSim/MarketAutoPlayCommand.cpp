#include "MarketAutoPlayCommand.h"
#include "MarketPortfolio.h"
#include "MarketAdvertising.h"
#include "MarketAutoPlayFinance.h"
#include "MarketBranches.h"
#include "MarketCalendar.h"
#include "MarketChains.h"
#include "MarketStart.h"
#include "MarketDirector.h"
#include "MarketOnline.h"
#include "MarketStaff.h"
#include "MarketCast.h"
#include "MarketAutoPlayRescue.h"
namespace MarketAutoPlayCommand
{
    FString Quote(const FString& Value){return TEXT("\"")+Value.Replace(TEXT("\""),TEXT("\"\""))+TEXT("\"");}
    void Event(FStats& Stats,int32 Day,const TCHAR* Kind,const FString& Id,int32 Value=-1)
    {Stats.Events.Add(FString::Printf(TEXT("%d,%s,%s,%d\n"),Day,Kind,*Quote(Id),Value));}
    int32 Desired(int32 Style,int32 Shops,int32 Channel,bool Online)
    {
        if(Style==0)return 0;
        if(Style==1 && Shops>=10)
        {if(Channel==3 || Channel==4)return 1;if(Channel==5 && Online)return 1;}
        if(Style==2 && Shops>=30)
        {if(Channel==0 || Channel==1)return 1;if(Channel==4)return 2;}
        return 0;
    }
    int32 Choice(const FMarketState& State,const TArray<FMarketProduct>& Products,const FMarketDecision& Card,double ExpansionBuffer)
    {
        if(Card.Id.StartsWith(TEXT("command.close:")))
            return State.Branches.IsValidIndex(Card.Arg) && State.Branches[Card.Arg].LossMonths>3 && State.Branches[Card.Arg].Last30Profit<0?0:1;
        if(Card.Id.StartsWith(TEXT("command.open:")))
        {
            if(MarketAutoPlayRescue::Blocked(State))return 1;
            FString Country,Province;Card.Id.RightChop(13).Split(TEXT("|"),&Country,&Province);
            const auto& Formats=MarketBranches::FormatIds();
            if(!Formats.IsValidIndex(Card.Arg))return 1;
            const FString& Format=Formats[Card.Arg];FString Reason;
            if(!MarketBranches::CanOpen(State,Products,Country,Province,Format,Reason))return 1;
            return MarketAutoPlayFinance::CanExpand(State,MarketBranches::OpeningCost(State,Products,Country,Province,Format),MarketBranches::MonthlyFixedCost(State,Country,Province,Format),ExpansionBuffer)?0:1;
        }
        if(Card.Id.StartsWith(TEXT("command.renew:"))) // D9b (M46): renew when the till keeps three times the works above the reserve
        {
            if(MarketAutoPlayRescue::Blocked(State))return 1;
            const int64 Cost=MarketPortfolio::WorksCost(State,Products,Card.Arg,MarketPortfolio::EWorks::Renovate);
            return Cost>0 && State.Cash>=MarketAutoPlayFinance::NetworkReserve(State)+3*Cost?0:1;
        }
        return Card.DefaultOption;
    }
    void RecordChoice(const FMarketState& State,const FMarketDecision& Card,int32 Option,FStats& Stats)
    {
        const bool Open=Card.Id.StartsWith(TEXT("command.open:"));
        if(Card.Id.StartsWith(TEXT("command.renew:"))){Event(Stats,State.Day,TEXT("kart_secimi"),Card.Id,Option);return;}
        if(Open){if(Option==0)++Stats.OpenAccepted;else ++Stats.OpenRejected;}
        else{if(Option==0)++Stats.CloseAccepted;else ++Stats.CloseRejected;}
        Event(Stats,State.Day,TEXT("kart_secimi"),Card.Id,Option);
    }
    bool Send(FMarketState& State,const TArray<FMarketProduct>& Products,const TCHAR* Name,int32 Arg,FStats& Stats)
    {FString Message;if(!MarketDirector::Command(State,Products,FName(Name),Arg,Message))return false;Event(Stats,State.Day,TEXT("komut"),Name,Arg);return true;}
    void Decide(FMarketState& State,const TArray<FMarketProduct>& Products,int32 Style,FStats& Stats)
    {
        if(State.Day%7!=1 || Stats.LastDecision==State.Day)return;Stats.LastDecision=State.Day;
        const int32 Shops=MarketOnline::TotalShops(State);
        if(Style!=0 && Shops>=20 && State.Advertising.ManagerName.IsEmpty())
        {
            FString Name;int32 Skill=0;int64 Wage=0;MarketAdvertising::Candidate(State,Name,Skill,Wage);
            if(State.Cash>=State.OtherCosts+MarketAutoPlayFinance::NetworkReserve(State)+30*MarketStaff::EmployerCost(Wage))Send(State,Products,TEXT("AdHire"),0,Stats);
        }
        if(Style!=0 && !State.Advertising.ManagerName.IsEmpty())
        {
            const int32 Budget=Style==1?20:30;
            if(State.Advertising.BudgetPermille!=Budget)Send(State,Products,TEXT("AdBudget"),Budget,Stats);
            if(!State.Advertising.bAuto)Send(State,Products,TEXT("AdAuto"),1,Stats);
            return;
        }
        if(Style==0 && State.Advertising.bAuto)Send(State,Products,TEXT("AdAuto"),0,Stats);
        const auto Countries=MarketAdvertising::Countries(State);
        for(int32 CountryIndex=0;CountryIndex<Countries.Num();++CountryIndex)
            for(int32 Channel=0;Channel<6;++Channel)
            {
                const auto Kind=static_cast<MarketAdvertising::EChannel>(Channel);
                const int32 Level=Desired(Style,Shops,Channel,State.Online.bWeb||State.Online.bApp||State.Online.bPlatform||State.Online.bQuick);
                if(Level>0 && MarketAdvertising::EraFactor(State,Kind,State.Day)<=0.f)continue;
                if(MarketAdvertising::LevelOf(State,Countries[CountryIndex],Kind)!=Level)Send(State,Products,TEXT("AdLevel"),MarketAdvertising::Encode(CountryIndex,Kind,Level),Stats);
            }
    }
    void Track(const FMarketState& State,FStats& Stats)
    {
        for(const auto& Card:State.Decisions)if(Card.Id.StartsWith(TEXT("command.")))
        {
            const FString Key=Card.Id+TEXT("|")+FString::FromInt(Card.Deadline);
            if(Stats.SeenCards.Contains(Key))continue;Stats.SeenCards.Add(Key);
            if(Card.Id.StartsWith(TEXT("command.open:")))++Stats.OpenProposals;else ++Stats.CloseProposals;
            Event(Stats,State.Day,TEXT("kart_onerisi"),Card.Id,Card.Arg);
        }
        // E2: a price war of the home province's chains against us (the street rivals left with MarketCompetitors).
        {
            const int32 War=MarketChains::WarIn(State,State.CountryId,MarketStart::HomeProvince(State),State.Day);
            const bool On=War!=INDEX_NONE;
            const bool First=!Stats.StreetOpen.Contains(0);
            if(!First && Stats.StreetOpen[0]!=On)Event(Stats,State.Day,On?TEXT("yerel_fiyat_savasi"):TEXT("yerel_fiyat_savasi_bitti"),On?State.Rivals.Chains[War].Name:FString(),0);
            Stats.StreetOpen.Add(0,On);
        }
    }
    void BeginDay(const FMarketState& State,FStats& Stats)
    {
        if(!Stats.NamesLogged)
        {
            Stats.NamesLogged=true;
            Event(Stats,State.Day,TEXT("ulke_adi"),TEXT("Muhasebeci: ")+MarketCast::Accountant());
            Event(Stats,State.Day,TEXT("ulke_adi"),TEXT("Toptanci: ")+MarketCast::Wholesaler());
            Event(Stats,State.Day,TEXT("ulke_adi"),TEXT("Ucuz toptanci: ")+MarketCast::CashCarry());
            Event(Stats,State.Day,TEXT("ulke_adi"),TEXT("Platform: ")+MarketCast::Platform());
            for(int32 BankIndex=0;BankIndex<4;++BankIndex)Event(Stats,State.Day,TEXT("ulke_adi"),MarketCast::Bank(BankIndex));
        }
        Stats.BeforeAds.Reset();for(const auto& Country:State.Advertising.Countries)Stats.BeforeAds.Add(Country.Country,Country);
        Stats.BeforeMarkdown.Reset();for(int32 BranchIndex=0;BranchIndex<State.Branches.Num();++BranchIndex)
            for(const auto& Item:State.Branches[BranchIndex].Items)Stats.BeforeMarkdown.Add(FString::FromInt(BranchIndex)+TEXT("|")+Item.Id,Item.MarkdownUntil);
        Track(State,Stats);
    }
    void Observe(const FMarketState& State,int32 Year,FStats& Stats)
    {
        const int32 Day=State.Day-1;if(Day<=Stats.LastDay)return;Stats.LastDay=Day;
        (void)Year;
        const bool Rolled=MarketCalendar::DateOf(State.Day).Day==1;
        for(const auto& Country:State.Advertising.Countries)
        {
            const int32 ActualYear=MarketCalendar::DateOf(Day).Year-MarketCalendar::StartYear+1;
            FYear* Row=Stats.Years.FindByPredicate([&](const FYear& R){return R.Year==ActualYear && R.Country==Country.Country;});
            if(!Row){FYear Fresh;Fresh.Year=ActualYear;Fresh.Country=Country.Country;Row=&Stats.Years[Stats.Years.Add(Fresh)];}Row->Day=Day;
            const FMarketAdCountry* Before=Stats.BeforeAds.Find(Country.Country);
            const auto& Spent=Rolled?Country.PrevChannelSpend:Country.MonthChannelSpend;
            for(int32 Channel=0;Channel<6;++Channel)Row->Spend[Channel]+=(Spent.IsValidIndex(Channel)?Spent[Channel]:0)-(Before && Before->MonthChannelSpend.IsValidIndex(Channel)?Before->MonthChannelSpend[Channel]:0);
            const int64 Uplift=(Rolled?Country.PrevUplift:Country.MonthUplift)-(Before?Before->MonthUplift:0);
            double BrandStock=0;int32 LastBrand=-1;
            for(int32 Channel=0;Channel<5;++Channel)if(Country.Stock.IsValidIndex(Channel) && Country.Stock[Channel]>0){BrandStock+=Country.Stock[Channel];LastBrand=Channel;}
            int64 Allocated=0;
            for(int32 Channel=0;Channel<5;++Channel)if(Country.Stock.IsValidIndex(Channel) && Country.Stock[Channel]>0)
            {const int64 Part=Channel==LastBrand?Uplift-Allocated:FMath::RoundToInt64(Uplift*Country.Stock[Channel]/BrandStock);Row->Estimate[Channel]+=Part;Allocated+=Part;}
        }

        if(!State.Advertising.ManagerName.IsEmpty())Stats.ManagerCosts+=MarketStaff::EmployerCost(State.Advertising.ManagerWage);
        bool Cleared=false;
        for(int32 BranchIndex=0;BranchIndex<State.Branches.Num();++BranchIndex)
            for(const auto& Item:State.Branches[BranchIndex].Items)
            {const FString Key=FString::FromInt(BranchIndex)+TEXT("|")+Item.Id;
                if(Item.Markdown>0 && Item.MarkdownUntil>Stats.BeforeMarkdown.FindRef(Key) && Item.MarkdownUntil==Day+7)
                {++Stats.ClearanceItems;Cleared=true;Event(Stats,Day,TEXT("stok_eritme"),Key,Item.Markdown);}}
        if(Cleared)++Stats.ClearanceWeeks;
        for(const FString& News:State.DayNews)if(News.Contains(TEXT("Stok eritme:")))Event(Stats,Day,TEXT("haftalik_satir"),News);
        Track(State,Stats);
        if(Day<=365)
        {
            const auto Info=MarketCalendar::Info(Day,State.RivalSeed);
            Stats.Weather.Add(FString::Printf(TEXT("%d,%d,%d,%d,%s\n"),Day,static_cast<int32>(Info.Weather),Info.TemperatureC,Info.bClosedByLaw?1:0,*Quote(Info.HolidayName)));
        }
    }
    FString Report(const FStats& Stats)
    {
        FString Out=FString::Printf(TEXT("\n#### C6: reklam ve komuta\n\nA\u00e7ma \u00f6nerisi %d (onay %d / ret %d), kapama %d (onay %d / ret %d). Stok eritme: %d \u00fcr\u00fcn i\u015flemi, %d ayr\u0131 g\u00fcn. Reklam m\u00fcd\u00fcr\u00fc gideri %.2f TL (i\u00e7 birim).\n\n| Y\u0131l | \u00dclke | Kanal | Harcama TL | Tahmini ek ciro TL |\n|---|---|---|---:|---:|\n"),Stats.OpenProposals,Stats.OpenAccepted,Stats.OpenRejected,Stats.CloseProposals,Stats.CloseAccepted,Stats.CloseRejected,Stats.ClearanceItems,Stats.ClearanceWeeks,Stats.ManagerCosts/100.);
        for(const auto& Row:Stats.Years)for(int32 Channel=0;Channel<6;++Channel)
            Out+=FString::Printf(TEXT("| %d | %s | %s | %.2f | %.2f |\n"),Row.Year,*Row.Country,*MarketAdvertising::ChannelName(static_cast<MarketAdvertising::EChannel>(Channel)),Row.Spend[Channel]/100.,Row.Estimate[Channel]/100.);
        Out+=TEXT("\nTahmin k\u00e2r de\u011fildir: C'nin ma\u011faza ciro ek tahmini (internet cirosu dahil) be\u015f marka kanal\u0131n\u0131n ak\u0131lda kalan pay\u0131yla da\u011f\u0131t\u0131l\u0131r (yuvarlama korunur). Arama trafik \u00e7arpan\u0131na etki etmez; internet ek cirosu C taraf\u0131ndan ayr\u0131 tahmin edilmedi\u011fi i\u00e7in araman\u0131n getirisi burada 0/\u00f6l\u00e7\u00fclmedi, etkisiz demek de\u011fil. Takvim y\u0131llar\u0131, ilk/son y\u0131l k\u0131smi; m\u00fcd\u00fcr \u00fccreti kanal harcamas\u0131na da\u011f\u0131t\u0131lmaz. CSV para i\u00e7 kuru\u015f; Almanya g\u00f6sterim \u00f6l\u00e7e\u011fi ayr\u0131d\u0131r. Sokak adlar\u0131/kapan\u0131\u015flar komuta_olaylar.csv; ilk 365 g\u00fcn hava ilk_yil_hava.csv (0 g\u00fcne\u015f, 1 bulut, 2 ya\u011fmur, 3 kar, 4 s\u0131cak).\n");
        for(const auto& Pair:Stats.StreetNames)Out+=TEXT("- Sokak: ")+Pair.Value+TEXT("\n");return Out;
    }
    FString YearsCsv(const FStats& Stats,const FString& Style,int32 Seed)
    {FString Out;for(const auto& R:Stats.Years)for(int32 Channel=0;Channel<6;++Channel)Out+=FString::Printf(TEXT("%s,%d,%d,%s,%d,%d,%lld,%lld\n"),*Style,Seed,R.Year,*R.Country,R.Day,Channel,R.Spend[Channel],R.Estimate[Channel]);return Out;}
    FString EventsCsv(const FStats& Stats,const FString& Style,int32 Seed)
    {FString Out;for(const auto& E:Stats.Events)Out+=Style+TEXT(",")+FString::FromInt(Seed)+TEXT(",")+E;return Out;}
    FString WeatherCsv(const FStats& Stats,const FString& Style,int32 Seed)
    {FString Out;for(const auto& W:Stats.Weather)Out+=Style+TEXT(",")+FString::FromInt(Seed)+TEXT(",")+W;return Out;}
}

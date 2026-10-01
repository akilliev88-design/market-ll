#include "MarketAutoPlayOnline.h"
#include "MarketOnline.h"
#include "MarketDirector.h"
#include "MarketAutoPlayFinance.h"
#include "MarketBranches.h"
#include "MarketCalendar.h"
#include "MarketPrices.h"
#include "MarketCountry.h"
#include "MarketStaff.h"
#include "MarketEvents.h"

namespace MarketAutoPlayOnline
{
    int32 Choice(const FMarketState& State,const FMarketDecision& Card,int32 Style,bool Offline)
    {
        if(Card.Id==TEXT("online.app"))
        {
            const int32 Tier=FMath::Clamp(Style,0,2);
            return Offline || State.Cash-State.OtherCosts<MarketPrices::Scaled(MarketOnline::AppCosts[Tier],State.Day)?3:Tier;
        }
        if(Card.Id==TEXT("online.pandemic"))return Offline || Style==0?2:0;
        if(Card.Id==TEXT("online.platformmarket"))return Offline?1:0;
        if(Card.Id==TEXT("online.commission"))return Offline || State.Online.Commission+0.03f>0.24001f?2:0;
        if(Card.Id.StartsWith(TEXT("online.area:")))
        {
            if(Offline)return 1;
            if(!(Card.Arg&8))return 0;
            const FString Key=Card.Id.RightChop(12);
            const int32 Area=State.Online.Areas.IndexOfByPredicate([&Key](const FMarketOnlineArea& Row){return Row.Country+TEXT("|")+Row.Province==Key;});
            return Area==INDEX_NONE || State.Cash<MarketOnline::DarkStorePrice(State,Area)*5?1:0;
        }
        return Card.DefaultOption;
    }
    void RecordChoice(const FMarketState& State,const FMarketDecision& Card,int32 Option,FStats& Stats)
    {
        Stats.Events.Add({State.Day,Option,Card.Arg,TEXT("kart_secimi"),Card.Id});
        if(Card.Id.StartsWith(TEXT("online.area:"))) { if(Option==0)++Stats.Accepted;else ++Stats.Rejected; }
        if(Card.Id==TEXT("online.commission") && Option==2)Stats.PlatformLeft=true;
    }
    bool Send(FMarketState& State,const TArray<FMarketProduct>& Products,const TCHAR* Name,int32 Arg,FStats& Stats)
    {
        FString Message;
        if(!MarketDirector::Command(State,Products,FName(Name),Arg,Message))return false;
        ++Stats.Commands.FindOrAdd(Name);
        Stats.Events.Add({State.Day,-1,Arg,TEXT("komut"),FString(Name)});
        return true;
    }
    void Decide(FMarketState& State,const TArray<FMarketProduct>& Products,int32 Style,FStats& Stats)
    {
        if(Stats.Offline || State.Day%7!=1 || Stats.LastDecisionDay==State.Day)return;
        Stats.LastDecisionDay=State.Day;
        const int64 Reserve=MarketAutoPlayFinance::NetworkReserve(State);
        FString Reason;
        for(int32 Channel=0;Channel<MarketOnline::ChannelCount;++Channel)
        {
            const auto Kind=static_cast<MarketOnline::EChannel>(Channel);
            if(Channel==2 && (Stats.PlatformLeft || State.Online.Commission>0.24001f))continue;
            if(State.Day<MarketOnline::OpenDay(State,Kind)+(Style==0?90:Style==1?14:0) || !MarketOnline::CanOpen(State,Kind,Reason))continue;
            if(Channel==1 && State.Decisions.ContainsByPredicate([](const FMarketDecision& Card){return Card.Id==TEXT("online.app");}))continue;
            int64 Cost=0;
            if(Channel==0)Cost=MarketPrices::Scaled(MarketOnline::WebSetupCost,State.Day);
            if(Channel==1)Cost=MarketPrices::Scaled(MarketOnline::AppCosts[FMath::Clamp(Style,0,2)],State.Day);
            if(Channel==2)Cost=MarketPrices::Scaled(MarketOnline::PlatformJoinCost,State.Day)*FMath::Max(1,State.Online.Areas.Num());
            if(State.Cash>=State.OtherCosts+Cost+Reserve)Send(State,Products,TEXT("OnlineOpen"),Channel,Stats);
        }
        // Existing loss-making company channels can close, but province flags are changed only by their cards.
        if(MarketCalendar::DateOf(State.Day).Day<=7 && State.Cash<Reserve)
            for(int32 Channel=0;Channel<4;++Channel)
                if(MarketOnline::IsOn(State,static_cast<MarketOnline::EChannel>(Channel)) && State.Online.PrevProfit.IsValidIndex(Channel) && State.Online.PrevProfit[Channel]<0)
                    Send(State,Products,TEXT("OnlineClose"),Channel,Stats);
        if(!State.Online.bWeb && !State.Online.bPlatform)return;
        const int32 Ads=State.Cash>Reserve*3?FMath::Clamp(Style,0,2):0;
        if(!State.Online.bAutoPolicy && State.Online.Ads!=Ads)Send(State,Products,TEXT("OnlineAds"),Ads,Stats);
        if(!State.Online.bAutoPolicy && State.Online.Fee!=1)Send(State,Products,TEXT("OnlineFee"),1,Stats);
        if(State.Online.ManagerName.IsEmpty() && MarketOnline::CanHireManager(State,Reason))
        {
            FString Name;int32 Skill=0;int64 Wage=0; MarketOnline::Candidate(State,Name,Skill,Wage);
            if(State.Cash>=Reserve+30*MarketStaff::EmployerCost(Wage))Send(State,Products,TEXT("OnlineHire"),0,Stats);
        }
        if(!State.Online.ManagerName.IsEmpty() && !State.Online.bAutoPolicy)Send(State,Products,TEXT("OnlineAutoPolicy"),1,Stats);
        // With a province manager, wait for his proposal. Without one the company rule needs a physical depot.
        if(State.Online.bQuick && State.Online.bApp)
            for(int32 Area=0;Area<State.Online.Areas.Num();++Area)
                if(MarketOnline::AreaDecider(State,Area).IsEmpty() && MarketOnline::CanBuildDarkStore(State,Area,Reason) &&
                    State.Cash>=MarketOnline::DarkStorePrice(State,Area)*5+Reserve)
                    Send(State,Products,TEXT("DarkStore"),Area,Stats);
    }
    void Track(const FMarketState& State,int32 Day,FStats& Stats)
    {
        for(int32 Channel=0;Channel<4;++Channel)
        {
            const bool On=MarketOnline::IsOn(State,static_cast<MarketOnline::EChannel>(Channel));
            if(On!=Stats.LastOn[Channel])
            {
                Stats.Events.Add({Day,-1,Channel,On?TEXT("acilis"):TEXT("kapanis"),MarketOnline::ChannelName(static_cast<MarketOnline::EChannel>(Channel))});
                if(On && Stats.FirstOpen[Channel]==0)Stats.FirstOpen[Channel]=Day;
                Stats.LastOn[Channel]=On;
            }
        }
        for(const auto& Card:State.Decisions)
        {
            if(!Card.Id.StartsWith(TEXT("online.")))continue;
            const FString Key=Card.Id+TEXT("|")+FString::FromInt(Card.Deadline);
            if(Stats.SeenCards.Contains(Key))continue;
            Stats.SeenCards.Add(Key);Stats.Events.Add({Day,-1,Card.Arg,TEXT("kart_onerisi"),Card.Id});
            if(Card.Id.StartsWith(TEXT("online.area:")))++Stats.Proposals;
        }
    }
    void BeginDay(const FMarketState& State,FStats& Stats)
    {
        Stats.BeforeOrders=State.Online.MonthOrders;Stats.BeforeRevenue=State.Online.MonthRevenue;Stats.BeforeProfit=State.Online.MonthProfit;
        Stats.PandemicStart=MarketOnline::PandemicStart(State);Stats.PandemicEnd=MarketOnline::PandemicEnd(State);
        Track(State,State.Day,Stats);
    }
    template<typename T> T Value(const TArray<T>& Values,int32 Index) {return Values.IsValidIndex(Index)?Values[Index]:T(0);}
    void Observe(const FMarketState& State,int32 Year,int32 FamilyShoppers,FStats& Stats)
    {
        const int32 Day=State.Day-1;if(Day<=Stats.LastDay)return;Stats.LastDay=Day;
        const bool Rolled=MarketCalendar::DateOf(State.Day).Day==1;
        const auto& O=State.Online;auto& Row=Stats.Current;
        int64 Variable=0;
        for(int32 Channel=0;Channel<4;++Channel)
        {
            Row.Orders[Channel]+=Value(Rolled?O.PrevOrders:O.MonthOrders,Channel)-Value(Stats.BeforeOrders,Channel);
            Row.Revenue[Channel]+=Value(Rolled?O.PrevRevenue:O.MonthRevenue,Channel)-Value(Stats.BeforeRevenue,Channel);
            const int64 Profit=Value(Rolled?O.PrevProfit:O.MonthProfit,Channel)-Value(Stats.BeforeProfit,Channel);
            Row.Contribution[Channel]+=Profit;Variable+=Profit;
        }
        ++Row.Days;Row.TotalProfit+=O.LastProfit;Row.CommonCosts+=Variable-O.LastProfit;
        Row.StoreRevenue+=State.LastRevenue;int64 Shoppers=FamilyShoppers;
        for(const auto& Branch:State.Branches)if(Branch.Stage==static_cast<uint8>(MarketBranches::EStage::Open)) {Row.StoreRevenue+=Branch.LastRevenue;Shoppers+=Branch.LastShoppers;}
        Row.Shoppers+=Shoppers;Row.Traffic+=MarketOnline::StoreTrafficFactorOn(State,Day);Row.CountryShare=MarketOnline::OnlineShare(State,Day);
        Row.DarkStores=0;for(const auto& Area:O.Areas)Row.DarkStores+=Area.DarkStoreDay>0?1:0;
        if(MarketOnline::IsPandemic(State,Day)){++Row.PandemicDays;Row.PandemicOrders+=O.LastOrders;Row.PandemicShoppers+=Shoppers;}
        Track(State,Day,Stats);
        for(const auto& Key:O.RivalsTold)if(!Stats.Rivals.Contains(Key)){Stats.Rivals.Add(Key);Stats.Events.Add({Day,-1,0,TEXT("rakip_haberi"),Key});}
        if(Year>Stats.Years.Num()) {Row.Year=Year;Row.Day=Day;Stats.Years.Add(Row);Row=FYear();}
    }
    FString Report(const FStats& Stats)
    {
        FString Out=FString::Printf(TEXT("\n#### C5: internet\n\nSalg\u0131n %d..%d. \u0130l \u00f6nerileri %d, onay %d, ret %d. %s\n"),Stats.PandemicStart,Stats.PandemicEnd,Stats.Proposals,Stats.Accepted,Stats.Rejected,Stats.Offline?TEXT("Bu temkinli deneme internete hi\u00e7 \u00e7\u0131kmaz."):TEXT("Kanallar normal oyuncu komutlar\u0131yla a\u00e7\u0131l\u0131r."));
        for(int32 Channel=0;Channel<4;++Channel)Out+=FString::Printf(TEXT("- %s ilk a\u00e7\u0131l\u0131\u015f: %d; salg\u0131na uzakl\u0131k: %d g\u00fcn.\n"),*MarketOnline::ChannelName(static_cast<MarketOnline::EChannel>(Channel)),Stats.FirstOpen[Channel],Stats.FirstOpen[Channel]?Stats.FirstOpen[Channel]-Stats.PandemicStart:0);
        Out+=TEXT("\n| Y\u0131l | \u00dclke internet % | Bizim ciroda internet % | Sipari\u015f | Net TL | Depo | Salg\u0131n g\u00fcn | Salg\u0131n sipari\u015f | Salg\u0131n m\u00fc\u015fteri | Trafik \u00e7arpan\u0131 ort. |\n|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|\n");
        for(const auto& Row:Stats.Years)
        {int64 Orders=0,Revenue=0;for(int32 C=0;C<4;++C){Orders+=Row.Orders[C];Revenue+=Row.Revenue[C];}
            Out+=FString::Printf(TEXT("| %d | %.2f | %.2f | %lld | %.2f | %d | %d | %lld | %lld | %.3f |\n"),Row.Year,Row.CountryShare*100.,Row.StoreRevenue>0?100.*Revenue/Row.StoreRevenue:0.,Orders,Row.TotalProfit/100.,Row.DarkStores,Row.PandemicDays,Row.PandemicOrders,Row.PandemicShoppers,Row.Days?Row.Traffic/Row.Days:0.);}
        Out+=TEXT("\nKanal k\u00e2r\u0131 mal ve sipari\u015f masraf\u0131 sonras\u0131 katk\u0131d\u0131r; ortak web/uygulama/depo/reklam/m\u00fcd\u00fcr gideri ayr\u0131 s\u00fctunda, toplam netten \u00e7\u0131kar. Kanal\u0131 olmayan g\u00fcnde de ortak gider kaybolmaz. Ciro pay\u0131 fiziksel aile+\u015fubelerin y\u0131ll\u0131k cirosundand\u0131r; ba\u011fl\u0131 \u015firket agregas\u0131 hari\u00e7. \u00dclke pay\u0131 y\u0131l sonu; trafik \u00e7arpan\u0131 g\u00fcn ortalamas\u0131. Kurulum yat\u0131r\u0131m\u0131 faaliyet netine eklenmez. A\u00e7\u0131l\u0131\u015f 0 = hi\u00e7 a\u00e7\u0131lmad\u0131. Ayr\u0131nt\u0131 internet.csv ve internet_olaylar.csv.\n");return Out;
    }
    FString YearsCsv(const FStats& Stats,const FString& Style,int32 Seed)
    {
        FString Out;auto Rows=Stats.Years;if(Stats.Current.Days>0){auto Partial=Stats.Current;Partial.Year=Rows.Num()+1;Partial.Day=Stats.LastDay;Rows.Add(Partial);}
        for(const auto& Row:Rows)for(int32 C=0;C<4;++C)
            Out+=FString::Printf(TEXT("%s,%d,%d,%d,%d,%d,%lld,%lld,%lld,%.4f,%lld,%lld,%lld,%.8f,%d,%d,%lld,%lld,%.8f\n"),*Style,Seed,Row.Year,Row.Day,Row.Days,C,Row.Orders[C],Row.Revenue[C],Row.Contribution[C],Row.Orders[C]?Row.Contribution[C]/(100.*Row.Orders[C]):0.,Row.CommonCosts,Row.TotalProfit,Row.StoreRevenue,Row.CountryShare,Row.DarkStores,Row.PandemicDays,Row.PandemicOrders,Row.PandemicShoppers,Row.Days?Row.Traffic/Row.Days:0.);
        return Out;
    }
    FString EventsCsv(const FStats& Stats,const FString& Style,int32 Seed)
    {FString Out;for(const auto& E:Stats.Events)Out+=FString::Printf(TEXT("%s,%d,%d,%s,\"%s\",%d,%d\n"),*Style,Seed,E.Day,*E.Kind,*E.Id.Replace(TEXT("\""),TEXT("\"\"")),E.Option,E.Arg);return Out;}
}

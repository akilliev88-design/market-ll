#include "MarketAutoPlayFinance.h"
#include "MarketBanking.h"
#include "MarketBranches.h"
#include "MarketChains.h"
#include "MarketDirector.h"
#include "MarketLedger.h"
#include "MarketPrices.h"
#include "MarketCountry.h"
#include "MarketManagers.h"
#include "MarketStaff.h"
#include "MarketDepots.h"

namespace MarketAutoPlayFinance
{
    int64 NetworkReserve(const FMarketState& State)
    {
        const int64 Wages=State.DailyPayroll()+MarketManagers::DailyWages(State);
        int64 Total=30*(Wages+MarketStaff::EmployerShare(Wages)+MarketPrices::Scaled(2200,State.Day));
        // Head-office rent, trucks and a dark store also have to be paid while a new shop matures.
        Total+=30*MarketDepots::DailyRent(State,State.Day);
        Total+=30*MarketPrices::Scaled(State.Company.Trucks*6000,State.Day);
        for(const auto& Branch:State.Branches)
            if(Branch.Stage!=static_cast<uint8>(MarketBranches::EStage::Closed))
                Total+=MarketBranches::MonthlyFixedCost(State,Branch.Country,Branch.Province,Branch.Format);
        return Total;
    }
    bool CanExpand(const FMarketState& State,int64 Opening,int64 NewMonthly,double Buffer)
    { return State.Cash>=FMath::RoundToInt64(Opening*Buffer)+NetworkReserve(State)+NewMonthly; }
    bool LosingMonth(const FMarketBranch& Branch,int32 Day,int32& RedMonths)
    {
        if(Branch.Stage!=static_cast<uint8>(MarketBranches::EStage::Open) || Day-Branch.OpenedDay<=90)
        { RedMonths=0; return false; }
        RedMonths=Branch.Last30Profit<0?RedMonths+1:0;
        return RedMonths>=2;
    }
    bool Send(FMarketState& State,const TArray<FMarketProduct>& Products,FName Action,int32 Arg,FStats& Stats)
    {
        FString Message; ++Stats.Attempts.FindOrAdd(Action.ToString());
        const bool Done=MarketDirector::Command(State,Products,Action,Arg,Message);
        if(Done)++Stats.Commands.FindOrAdd(Action.ToString());
        return Done;
    }
    void Decide(FMarketState& State,const TArray<FMarketProduct>& Products,bool Aggressive,bool Careful,FStats& Stats)
    {
        const int32 Turn=(State.Day-1)/30;
        if(Stats.LastTurn==Turn+1)return;
        Stats.LastTurn=Turn+1;
        for(int32 Index=0;Index<State.Branches.Num();++Index)
            if(LosingMonth(State.Branches[Index],State.Day,Stats.RedMonths.FindOrAdd(Index)))
                Send(State,Products,TEXT("CloseBranch"),Index,Stats);
        const int64 Reserve=NetworkReserve(State);
        if(Careful)return;
        if(!State.Banking.bLine && MarketBanking::LineLimitFor(State)>0 && Send(State,Products,TEXT("OpenLine"),0,Stats))
            Send(State,Products,TEXT("LineAuto"),1,Stats);
        if(State.Banking.LineDrawn>0 && State.Cash>=Reserve*3+State.Banking.LineDrawn)
            Send(State,Products,TEXT("RepayLine"),0,Stats);
        if(State.Banking.Loans.Num()>0 && State.Day>=State.Banking.RestructuredUntil &&
            MarketBanking::DueSoon(State)>FMath::Max<int64>(0,State.Cash-Reserve) && State.Cash<Reserve)
            Send(State,Products,TEXT("Restructure"),static_cast<int32>(MarketBanking::Rating(State))<2?0:1,Stats);
        // Investment loans fund the next visible home-site opening, never a gift or an emergency cash injection.
        const FString Format=MarketBranches::OpenCount(State)>= (Aggressive?3:6)?TEXT("buyuk"):TEXT("mahalle");
        FString Reason;
        if(MarketBranches::CanOpen(State,Products,State.CountryId,State.CityId,Format,Reason))
        {
            const int64 Cost=MarketBranches::OpeningCost(State,Products,State.CountryId,State.CityId,Format);
            const int64 Future=Reserve+MarketBranches::MonthlyFixedCost(State,State.CountryId,State.CityId,Format);
            const int64 Needed=Cost+Future*3-State.Cash;
            if(Needed>0 && State.Cash>=Reserve && State.Banking.Loans.Num()<(Aggressive?3:1))
            {
                int32 Chosen=INDEX_NONE, Step=0; double Rate=MAX_dbl;
                for(int32 Bank=0;Bank<MarketBanking::BankCount;++Bank)
                {
                    const int64 Offer=MarketBanking::Offer(State,Bank);
                    for(int32 Part=0;Part<4;++Part)
                        if(Offer*(Part+1)/4/10000*10000>=Needed && MarketBanking::YearRate(State,Bank)<Rate)
                        { Chosen=Bank; Step=Part; Rate=MarketBanking::YearRate(State,Bank); break; }
                }
                if(Chosen!=INDEX_NONE)Send(State,Products,TEXT("CorpLoan"),MarketBanking::EncodeLoan(Chosen,Step,2,Aggressive),Stats);
            }
        }
        for(int32 Index=0;Index<State.Rivals.Chains.Num();++Index)
        {
            const auto& Chain=State.Rivals.Chains[Index];
            if(Chain.bOurs && !Chain.bGone)
            {
                if(Chain.MonthProfit<0 && State.Cash<Reserve)
                    Send(State,Products,TEXT("SellSubsidiary"),Index,Stats);
                else
                {
                    const int32 Count=FMath::Min(Aggressive?20:5,MarketChains::ConvertRoom(State,Index));
                    if(Count>0 && State.Cash>=MarketChains::ConvertCost(State,Index,Count)+Reserve*3)
                    {
                        const int32 Before=State.Rivals.ConvertedInMonth;
                        if(Send(State,Products,TEXT("ConvertStores"),MarketChains::EncodeConvert(Index,Count),Stats))
                            Stats.Converted+=State.Rivals.ConvertedInMonth-Before;
                    }
                }
                continue;
            }
            const bool Bid=!Chain.bForSale;
            if(Chain.bGone || MarketChains::YearProfit(State,Index)<=0 ||
                (Bid && (State.Day%(Aggressive?180:360)>30 || MarketChains::AcceptChance(State,Index)<(Aggressive?.25f:.6f))))continue;
            if(Bid?!MarketChains::CanBid(State,Index,Reason,false):!MarketChains::CanBuy(State,Index,Reason,false))continue;
            const int64 Price=Bid?MarketChains::BidPrice(State,Index):MarketChains::Price(State,Index);
            // The player's financed command leaves 5% working cash; require that to cover our whole network.
            if(FMath::Max<int64>(State.Cash-Price,Price/20)<Reserve || (Bid && State.Cash<Reserve+Price/200))continue;
            const int64 Shortfall=FMath::Max<int64>(0,Price+Price/20-State.Cash);
            int64 Room=0;
            for(int32 Bank=0;Bank<MarketBanking::BankCount;++Bank)
                Room=FMath::Max(Room,MarketBanking::AcquisitionRoom(State,Bank,MarketChains::YearProfit(State,Index)));
            if(Shortfall>Room)continue; // visible bank terms, not the hidden acceptance draw
            const int32 BeforeLoans=State.Banking.Loans.Num(), BeforeBuys=State.Rivals.OurBuys;
            const FString Id=Chain.Id;
            const int32 BeforeBid=Chain.BidDay;
            if(Bid)++Stats.Bids;
            Send(State,Products,Bid?TEXT("BidChainFinanced"):TEXT("BuyChainFinanced"),Index,Stats);
            if(State.Rivals.OurBuys>BeforeBuys) { if(Bid)++Stats.Accepted; if(State.Banking.Loans.Num()>BeforeLoans)++Stats.Financed; }
            else if(Bid && State.Rivals.Chains[Index].Id==Id && State.Rivals.Chains[Index].BidDay!=BeforeBid)++Stats.Refused;
            break; // at most one deal per monthly review
        }
    }
    void Observe(const FMarketState& State,int32 Year,FStats& Stats)
    {
        const int32 Day=State.Day-1;
        if(Day<=Stats.ObservedDay)return;
        Stats.ObservedDay=Day;
        if(State.Rescues>Stats.Rescues) { Stats.RescueDays.Add(Day); Stats.Rescues=State.Rescues; }
        Stats.Interest=State.Banking.InterestPaid;
        Stats.PeakLine=FMath::Max(Stats.PeakLine,State.Banking.LineDrawn);
        if(State.Banking.LastRatingDay!=Stats.LastRatingDay)
        { Stats.LastRatingDay=State.Banking.LastRatingDay; if(State.Banking.BreachMonths>0)++Stats.BreachMonths; }
        Stats.Closed=0;
        for(int32 Index=0;Index<State.Branches.Num();++Index)
        {
            const auto& Branch=State.Branches[Index];
            auto& Result=Stats.Branches.FindOrAdd(Index);
            Result.Name=Branch.Name; Result.Format=Branch.Format; Result.Opened=Branch.OpenedDay;
            if(Branch.Stage==static_cast<uint8>(MarketBranches::EStage::Closed)) { ++Stats.Closed; if(Result.Closed==0)Result.Closed=Day; }
            if(Branch.OpenedDay<=0 || Day<Branch.OpenedDay || Day>=Branch.OpenedDay+180 || (Result.Closed>0 && Day>Result.Closed))continue;
            if(Branch.Stage==static_cast<uint8>(MarketBranches::EStage::Open))++Result.Days;
        }
        // One scan of the retained ledger, not one full scan per branch. Account classification is the books' own.
        for(const auto& Entry:State.Ledger.Entries)
        {
            if(Entry.Day!=Day || Entry.Store<0)continue;
            auto* Result=Stats.Branches.Find(Entry.Store);
            if(!Result || Result->Opened<=0 || Day<Result->Opened || Day>=Result->Opened+180 || (Result->Closed>0 && Day>Result->Closed))continue;
            const auto Account=static_cast<MarketLedger::EAccount>(Entry.Account);
            if(MarketLedger::IsIncomeStatement(Account))Result->Net+=Entry.Amount;
            if(MarketLedger::IsRevenue(Account)) { Result->Revenue+=Entry.Amount; Result->Gross+=Entry.Amount; }
            else if(MarketLedger::IsGoodsCost(Account))Result->Gross+=Entry.Amount;
            if(Account==MarketLedger::EAccount::Rent)Result->Rent-=Entry.Amount;
            if(Account==MarketLedger::EAccount::Wages)Result->Wages-=Entry.Amount;
            if(Account==MarketLedger::EAccount::SocialSecurity)Result->Sgk-=Entry.Amount;
            if(Account==MarketLedger::EAccount::Utilities)Result->Running-=Entry.Amount;
            if(Account==MarketLedger::EAccount::Logistics)Result->Logistics-=Entry.Amount;
            if(Account==MarketLedger::EAccount::Waste || Account==MarketLedger::EAccount::DepartmentWaste)Result->Waste-=Entry.Amount;
        }
        for(const auto& Chain:State.Rivals.Chains)
            if(Chain.bExitSale) { Stats.Exits.Add(Chain.Id); if(Chain.Country!=State.CountryId)Stats.Gates.Add(Chain.Id); }
        if(Year>Stats.Years.Num())
        {
            const auto Picture=MarketBanking::Picture(State);
            FBankYear Row; Row.Year=Year; Row.Day=Day; Row.Rating=static_cast<int32>(MarketBanking::Rating(State));
            Row.Debt=MarketBanking::Debt(State); Row.Ebitda=Picture.Ebitda;
            Row.Leverage=Row.Debt<=0?0.f:Row.Ebitda>0?static_cast<float>(static_cast<double>(Row.Debt)/Row.Ebitda):99.f;
            Row.Interest=Stats.Interest; Row.Line=State.Banking.LineDrawn; Row.Limit=State.Banking.LineLimit;
            Row.Subsidiaries=MarketChains::Subsidiaries(State); Row.Stores=MarketChains::SubsidiaryStores(State);
            Stats.Years.Add(Row);
        }
    }
    FString Report(const FStats& Stats)
    {
        FString Text=TEXT("\n#### C4: banka ve ilk 180 g\u00fcn\n\n");
        Text+=FString::Printf(TEXT("Kurtarma %d; kapanan \u015fube %d; bor\u00e7 s\u0131n\u0131r\u0131 ihlali %d ay; \u015firket faizi %s; en y\u00fcksek limit kullan\u0131m\u0131 %s.\nTeklif %d, kabul %d, ret %d, krediyle al\u0131m %d, \u00e7evrilen ma\u011faza %d, devlerin sat\u0131l\u0131k kolu %d, yabanc\u0131 kap\u0131 kolu %d.\n"),Stats.Rescues,Stats.Closed,Stats.BreachMonths,*MarketCountry::Money(Stats.Interest),*MarketCountry::Money(Stats.PeakLine),Stats.Bids,Stats.Accepted,Stats.Refused,Stats.Financed,Stats.Converted,Stats.Exits.Num(),Stats.Gates.Num());
        Text+=TEXT("Kurtarma g\u00fcnleri:"); for(int32 Day:Stats.RescueDays)Text+=FString::Printf(TEXT(" %d"),Day);
        Text+=TEXT("\n\n\u015eube d\u00f6k\u00fcm\u00fc sube180.csv; y\u0131ll\u0131k banka durumu banka.csv. Eksik 180 g\u00fcnler kapanma/ko\u015fu sonu nedeniyle ayr\u0131 okunmal\u0131; yat\u0131r\u0131m ve stok al\u0131m\u0131 net faaliyet k\u00e2r\u0131na dahil de\u011fil. Faiz \u015firket bankas\u0131n\u0131n toplam\u0131d\u0131r; aile kredisi ayr\u0131d\u0131r.\n");
        TArray<FString> Names; Stats.Attempts.GetKeys(Names); Names.Sort();
        for(const auto& Name:Names)Text+=FString::Printf(TEXT("- %s: %d deneme / %d ba\u015far\u0131.\n"),*Name,Stats.Attempts[Name],Stats.Commands.FindRef(Name));
        return Text;
    }
    FString BranchCsv(const FStats& Stats,const FString& Style,int32 Seed)
    {
        FString Text; TArray<int32> Indices; Stats.Branches.GetKeys(Indices); Indices.Sort();
        for(int32 Index:Indices)
        { const auto& R=Stats.Branches[Index]; Text+=FString::Printf(TEXT("%s,%d,%d,%s,%d,%d,%d,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld\n"),*Style,Seed,Index,*R.Format,R.Opened,R.Days,R.Closed,R.Revenue,R.Gross,R.Rent,R.Wages,R.Sgk,R.Running,R.Logistics,R.Waste,R.Net); }
        return Text;
    }
    FString BankCsv(const FStats& Stats,const FString& Style,int32 Seed)
    {
        FString Text;
        for(const auto& R:Stats.Years)Text+=FString::Printf(TEXT("%s,%d,%d,%d,%s,%lld,%lld,%.4f,%lld,%lld,%lld,%d,%d\n"),*Style,Seed,R.Year,R.Day,*MarketBanking::RatingName(static_cast<MarketBanking::ERating>(R.Rating)),R.Debt,R.Ebitda,R.Leverage,R.Interest,R.Line,R.Limit,R.Subsidiaries,R.Stores);
        return Text;
    }
    FString SummaryCsv(const FStats& Stats,const FString& Style,int32 Seed)
    { return FString::Printf(TEXT("%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%lld,%lld\n"),*Style,Seed,Stats.Rescues,Stats.Closed,Stats.BreachMonths,Stats.Bids,Stats.Accepted,Stats.Refused,Stats.Financed,Stats.Converted,Stats.Exits.Num(),Stats.Gates.Num(),Stats.Interest,Stats.PeakLine); }
}

#include "MarketAutoPlayC.h"
#include "MarketDirector.h"
#include "MarketDepartments.h"
#include "MarketSourcing.h"
#include "MarketBrands.h"
#include "MarketChains.h"
#include "MarketBranches.h"
#include "MarketCalendar.h"
#include "MarketPrices.h"
#include "MarketCompany.h"
#include "MarketDepots.h"
#include "MarketLedger.h"
#include "MarketEras.h"
#include "MarketGoals.h"
#include "MarketStrategy.h"
#include "MarketPortfolio.h"
namespace MarketAutoPlayC
{
    FString Key(int32 Format,int32 Dept) { return FString::Printf(TEXT("%d|%d"),Format,Dept); }
    bool Send(FMarketState& State,const TArray<FMarketProduct>& Products,FName Action,int32 Arg,FStats& Stats)
    {
        FString Message;
        if(!MarketDirector::Command(State,Products,Action,Arg,Message)) { ++Stats.Rejected; return false; }
        ++Stats.Commands.FindOrAdd(Action.ToString()); return true;
    }
    bool SiteSuitable(const FMarketState& State,const FString& Country,const FString& Province,const FString& Format)
    {
        const auto Site=MarketBranches::SiteOf(State,Country,Province);
        const auto& Kind=MarketBranches::FormatInfo(Format);
        if(!Site.bValid || Site.PopulationK<Kind.MinPopulationK)return false;
        if(Kind.Chapter>0 && !MarketCompany::ChapterOpen(State,Kind.Chapter))return false;
        float Km=0.f;
        return !Kind.bNeedsDepot || MarketDepots::Nearest(State,Country,Province,false,Km)!=INDEX_NONE;
    }
    bool BrandWorth(const FMarketState& State,const TArray<FMarketProduct>& Products,const FMarketBrandOffer& Offer,const FPolicy& Policy)
    {
        if(Offer.Kind==static_cast<uint8>(MarketBrands::EKind::ShelfShare))
            return MarketBrands::ShelfShare(State,Products,Offer.Brand,Offer.Category)+.0001f >= Offer.Target*Policy.BrandCover;
        if(Offer.Kind==static_cast<uint8>(MarketBrands::EKind::Rebate))
            return State.Brands.MonthSales.FindRef(Offer.Brand) >= Offer.Amount*Policy.BrandCover;
        const int32 Index=Products.IndexOfByPredicate([&](const FMarketProduct& Product){return Product.Id==Offer.ProductId;});
        return State.Stock.IsValidIndex(Index) && State.Stock[Index].Capacity>0;
    }
    double PurchaseForecast(const FMarketState& State,int32 Line,const FStats& Stats)
    {
        const int32 Age=State.Day-State.Sourcing.LastMonthDay;
        if(Age>=7 && State.Sourcing.MonthBuy.IsValidIndex(Line)) return State.Sourcing.MonthBuy[Line]*30.0/Age;
        if(!Stats.DailyBuy.IsValidIndex(Line) || Stats.DailyBuy[Line].Num()<7) return 0;
        int64 Sum=0; for(int64 Amount:Stats.DailyBuy[Line])Sum+=Amount;
        // Month-close resets erase the last day's line total. Leaving it out is conservative, never invents buying.
        return static_cast<double>(Sum)*30/Stats.DailyBuy[Line].Num();
    }
    void Decide(FMarketState& State,const TArray<FMarketProduct>& Products,const FPolicy& Policy,int64 Reserve,FStats& Stats)
    {
        const TArray<FMarketBrandOffer> Offers=State.Brands.Offers;
        for(const auto& Offer:Offers) Send(State,Products,BrandWorth(State,Products,Offer,Policy)?TEXT("AcceptBrandOffer"):TEXT("RejectBrandOffer"),Offer.Id,Stats);
        if(State.Day<Policy.StartDay || (State.Day-Policy.StartDay)%Policy.Interval!=0)return;
        for(int32 Line=0;Line<MarketSourcing::LineCount;++Line)
        {
            const auto L=static_cast<MarketSourcing::ELine>(Line);
            const int32 Current=static_cast<int32>(MarketSourcing::TierOf(State,L));
            const double Forecast=PurchaseForecast(State,Line,Stats);
            const auto Next=static_cast<MarketSourcing::ETier>(FMath::Min(Current+1,MarketSourcing::TierCount-1));
            const double Minimum=MarketSourcing::TierMinimum(Next)*MarketPrices::ListLevel(State.Day);
            FString Reason;
            if(Current>0 && Forecast < MarketSourcing::TierMinimum(static_cast<MarketSourcing::ETier>(Current))*MarketPrices::ListLevel(State.Day)*.8)
                Send(State,Products,TEXT("SetSourcing"),Line*10+Current-1,Stats);
            else if(static_cast<int32>(Next)>Current && Forecast>=Minimum*Policy.MinimumCover && MarketSourcing::CanSet(State,L,Next,Reason))
                Send(State,Products,TEXT("SetSourcing"),Line*10+static_cast<int32>(Next),Stats);
            else if(static_cast<int32>(Next)>Current && MarketSourcing::CanSet(State,L,Next,Reason)) ++Stats.Blocked.FindOrAdd(TEXT("Tedarik asgarisi"));
        }
        for(int32 Format=1;Format<MarketDepartments::FormatCount;++Format)
        {
            int32 Count=0;
            for(const auto& Branch:State.Branches) if(Branch.Stage==static_cast<uint8>(MarketBranches::EStage::Open) && MarketDepartments::FormatIndex(Branch.Format)==Format)++Count;
            if(Count==0)continue;
            int32 Opened=0;
            struct FCandidate { int32 Dept; double Score; };
            TArray<FCandidate> Candidates;
            for(int32 Dept=0;Dept<MarketDepartments::DeptCount;++Dept)
            {
                const auto D=static_cast<MarketDepartments::EDept>(Dept);
                const auto& Info=MarketDepartments::Info(D);
                int64 Profit=0; int32 Mature=0;
                for(const auto& Branch:State.Branches) if(MarketDepartments::FormatIndex(Branch.Format)==Format && Branch.Stage==static_cast<uint8>(MarketBranches::EStage::Open))
                    for(const auto& Row:Branch.Depts) if(Row.Dept==Dept && State.Day-Row.OpenedDay>=30){Profit+=Row.Last30Profit;++Mature;}
                if(MarketDepartments::IsOn(State,D,Format))
                {
                    if(Mature>0 && Profit<0)
                    {
                        if(Send(State,Products,TEXT("SetDepartment"),MarketDepartments::EncodeSet(D,Format,false),Stats)) Stats.DeptCooldown.Add(Key(Format,Dept),State.Day+Policy.Interval*3);
                    }
                    else
                    {
                        if(MarketDepartments::Stance(State,D)!=Policy.Stance)Send(State,Products,TEXT("SetDeptStance"),Dept*10+Policy.Stance,Stats);
                        if(MarketDepartments::WeakMasters(State,D)>0 && State.Cash>Reserve*2)Send(State,Products,TEXT("ReplaceMasters"),Dept,Stats);
                    }
                    continue;
                }
                if(Format<Info.MinFormat || Stats.DeptCooldown.FindRef(Key(Format,Dept))>State.Day)continue;
                const int64* Known=Stats.LastDept.Find(Key(Format,Dept));
                const double Score=Known?static_cast<double>(*Known)/FMath::Max(1,Count):Info.Ratio*(Info.Margin-Info.Waste-Info.Shrink)*100000;
                Candidates.Add({Dept,Score});
            }
            Candidates.Sort([](const FCandidate& Left,const FCandidate& Right){return Left.Score==Right.Score?Left.Dept<Right.Dept:Left.Score>Right.Score;});
            for(const auto& Candidate:Candidates)
            {
                if(Opened>=Policy.OpensPerTurn)break;
                const auto D=static_cast<MarketDepartments::EDept>(Candidate.Dept); const auto& Info=MarketDepartments::Info(D);
                if(MarketDepartments::SpaceUsed(State,Format)+Info.Space>FMath::FloorToInt(MarketDepartments::SpaceCap(Format)*Policy.SpaceFraction))continue;
                int64 Estimate=0;
                for(const auto& Branch:State.Branches) if(Branch.Stage==static_cast<uint8>(MarketBranches::EStage::Open) && MarketDepartments::FormatIndex(Branch.Format)==Format)
                    Estimate+=MarketDepartments::FitOutCost(D,Format,State.Day)+FMath::RoundToInt64(FMath::Max(30,Branch.LastShoppers>0?Branch.LastShoppers:MarketBranches::FormatInfo(Branch.Format).Trips/5)*MarketDepartments::TicketStart*MarketPrices::ListLevel(State.Day)*Info.Ratio*(1.f-Info.Margin)*Info.StockDays);
                FString Reason;
                if(State.Cash>=Estimate+Reserve && MarketDepartments::CanSet(State,D,Format,true,Reason) && Send(State,Products,TEXT("SetDepartment"),MarketDepartments::EncodeSet(D,Format,true),Stats))
                { ++Opened; if(MarketDepartments::Stance(State,D)!=Policy.Stance)Send(State,Products,TEXT("SetDeptStance"),Candidate.Dept*10+Policy.Stance,Stats); }
            }
        }
    }
    bool NewKey(TSet<FString>& Seen,const FString& Value) { if(Seen.Contains(Value))return false;Seen.Add(Value);return true; }
    void Quiet(bool Interesting,int32& Days,int32& Periods) { Days=Interesting?0:Days+1;if(Days==31)++Periods; }
    void Pile(TArray<int32>& Days,int32 Day,bool& Active,int32& Count)
    { Days.RemoveAll([Day](int32 Earlier){return Earlier<Day-6;}); const bool Now=Days.Num()>3; if(Now&&!Active)++Count; Active=Now; }
    void Observe(const FMarketState& State,FStats& Stats)
    {
        const int32 Day=State.Day-1; bool BaseEvent=false,Extra=false;
        // A closed day is measured once; reports must never change the campaign.
        if(Day<=Stats.ObservedDay)return;
        Stats.ObservedDay=Day;
        Stats.GapDays=State.Ledger.GapDays; Stats.GapTotal=State.Ledger.TotalGap;
        Stats.GapAbsolute+=FMath::Abs(State.Ledger.LastGap);
        Stats.Closures=State.Rivals.Closures; Stats.Takeovers=State.Rivals.Takeovers; Stats.OurBuys=State.Rivals.OurBuys;
        Stats.Purchases=Stats.OurBuys;
        Stats.QuietEvents=State.Goals.QuietEvents; Stats.HeldBadEvents=State.Goals.HeldBadEvents;
        const int32 QuietDays=FMath::Max(0,Day-State.Goals.LastLivelyDay);
        Stats.RhythmLongest=FMath::Max(Stats.RhythmLongest,QuietDays);
        const bool Boring=State.Goals.bStarted && QuietDays>=MarketGoals::QuietDays(State);
        if(Boring && !Stats.bRhythmBoring)++Stats.RhythmBoring;
        Stats.bRhythmBoring=Boring;
        for(const auto& Goal:State.Goals.Goals)
            if(NewKey(Stats.SeenGoals,FString::Printf(TEXT("%d|%d|%d|%d"),Goal.Kind,Goal.Scale,Goal.StartDay,Goal.DueDay)))++Stats.GoalsSeen;
        for(const auto& Celebration:MarketGoals::CelebrationsOn(State,Day))
        { ++Stats.Celebrations; if(Celebration.Title==TEXT("Hedef tamam!"))++Stats.GoalsCompleted; }
        if(Stats.Eras.IsEmpty())for(const auto& Era:MarketEras::PlanOf(State))
        { FEraResult Row;Row.Kind=static_cast<int32>(Era.Kind);Row.Wave=Era.Wave;Row.Start=Era.StartDay;Row.End=Era.EndDay;Stats.Eras.Add(Row); }
        const int64 DayProfit=MarketLedger::DayStatement(State,Day).NetProfit;
        for(auto& Era:Stats.Eras)if(Day>=Era.Start && Day<=Era.End)
        {
            if(Era.Days==0){Era.FirstCash=State.Cash;Era.LowestCash=State.Cash;}
            ++Era.Days;Era.LastCash=State.Cash;Era.LowestCash=FMath::Min(Era.LowestCash,State.Cash);Era.Profit+=DayProfit;
        }
        for(const auto& Event:State.EventLog) if(NewKey(Stats.SeenEvents,Event))
        {
            BaseEvent=true; FString Id,Date; Event.Split(TEXT("@"),&Id,&Date);
            if(Id==TEXT("event.fridge")||Id==TEXT("event.power")||Id==TEXT("event.inspection")||Id==TEXT("event.complaint")||Id==TEXT("event.roadworks")||Id==TEXT("event.truck")||Id==TEXT("event.snow"))
            {Stats.BadDays.Add(Day);Stats.BaseBadDays.Add(Day);}
        }
        for(const auto& Decision:State.Decisions) BaseEvent|=NewKey(Stats.SeenDecisions,Decision.Id+TEXT("@")+FString::FromInt(Decision.Deadline));
        for(const auto& Offer:State.Brands.Offers)Extra|=NewKey(Stats.SeenOffers,FString::FromInt(Offer.Id));
        int32 Alive=0;
        for(const auto& Chain:State.Rivals.Chains)
        {
            if(!Chain.bGone)++Alive;
            if(Chain.bGone && NewKey(Stats.SeenGone,Chain.Id)){++Stats.Gone;Extra=true;}
            if(Chain.bForSale && NewKey(Stats.SeenSale,Chain.Id)){++Stats.Sale;Extra=true;}
            if(Chain.WarUntil>=Day && !Chain.WarProvince.IsEmpty() && NewKey(Stats.SeenWars,Chain.Id+TEXT("@")+FString::FromInt(Chain.WarUntil)))
            {++Stats.Wars;Stats.BadDays.Add(Day);Extra=true;}
        }
        for(const auto& News:State.DayNews) if(News.Contains(TEXT("kepenk indirdi:")))++Stats.BankruptcyNews;
        Stats.ChainPeak=FMath::Max(Stats.ChainPeak,Alive); Stats.Nemesis=MarketChains::NemesisLine(State);
        {
            const FMarketStrategyState& Sg=State.Strategy;
            int32 Ended=0,Gave=0; for(const FMarketPush& P:Sg.Pushes){ if(P.bEnded)++Ended; Gave+=P.Withdrawn; }
            FString Tiers; for(int32 I=0;I<Sg.Tiers.Num();++I)Tiers+=FString::Printf(TEXT("%d"),static_cast<int32>(Sg.Tiers[I]));
            const FString Line=MarketStrategy::StrategyLine(State);
            Stats.Strategy=FString::Printf(TEXT("%s; il ata\u011f\u0131 %d (biten %d, rakibin kapatt\u0131\u011f\u0131 ma\u011faza %d, harcanan %.0f TL); \u015fampiyon il %d; yollar (\u015fampiyon/verimli/insan/halk) %s; sadakat \u00f6demesi %.0f TL"),
                Line.IsEmpty()?TEXT("yol ayr\u0131m\u0131 se\u00e7ilmedi"):*Line,Sg.Pushes.Num(),Ended,Gave,Sg.PushPaid/100.,Sg.Champions.Num(),Tiers.IsEmpty()?TEXT("-"):*Tiers,Sg.LoyaltyPaid/100.);
        }
        {
            int32 Cards[6]={0,0,0,0,0,0},Renewed=0,Worn=0,Works=0;
            for(const FMarketBranch& B:State.Branches)
            {
                if(B.Stage==static_cast<uint8>(MarketBranches::EStage::Closed))continue;
                if(B.Card<=5)++Cards[B.Card];
                if(B.RenewedDay>0)++Renewed;
                if(B.Works!=0)++Works;
                else if(MarketPortfolio::IsAgeing(State,B))++Worn;
            }
            const FMarketResponses& R=State.Responses;
            int32 Kinds[3]={0,0,0}; for(const FMarketResponse& A:R.Log)if(A.Kind<3)++Kinds[A.Kind];
            Stats.Portfolio=FString::Printf(TEXT("yenilenmi\u015f/ta\u015f\u0131nm\u0131\u015f %d, eski %d, i\u015fte %d; son karneler A%d B%d C%d D%d E%d; m\u00fcdahale %d (senin %d; son 80: sava\u015f %d, a\u00e7\u0131l\u0131\u015f %d, kriz %d)"),
                Renewed,Worn,Works,Cards[1],Cards[2],Cards[3],Cards[4],Cards[5],R.Answered,R.ByPlayer,Kinds[0],Kinds[1],Kinds[2]);
        }
        Stats.BrandMoney=State.Brands.TotalReceived;
        for(const auto& Share:State.Brands.Shares)if(MarketBrands::CostFactor(State,Share.Brand)>1.f)Stats.CoolBrands.Add(Share.Brand);
        if(Stats.Tiers.Num()!=MarketSourcing::LineCount)Stats.Tiers.Init(0,MarketSourcing::LineCount);
        if(Stats.DailyBuy.Num()!=MarketSourcing::LineCount)Stats.DailyBuy.SetNum(MarketSourcing::LineCount);
        for(int32 Line=0;Line<MarketSourcing::LineCount;++Line)
        {
            const int32 Tier=static_cast<int32>(MarketSourcing::TierOf(State,static_cast<MarketSourcing::ELine>(Line)));
            if(Tier!=Stats.Tiers[Line]){Stats.Sourcing.Add({Day,Line,Stats.Tiers[Line],Tier});Stats.Tiers[Line]=Tier;Extra=true;}
            const FString Id=FString::FromInt(Line);const int64 Current=State.Sourcing.MonthBuy.IsValidIndex(Line)?State.Sourcing.MonthBuy[Line]:0;
            Stats.DailyBuy[Line].Add(FMath::Max<int64>(0,Current-Stats.PreviousBuy.FindRef(Id)));Stats.PreviousBuy.Add(Id,Current);
            if(Stats.DailyBuy[Line].Num()>30)Stats.DailyBuy[Line].RemoveAt(0);
        }
        int32 Stores=1;
        TMap<FString,FDept> Groups;
        for(const auto& Branch:State.Branches)if(Branch.Stage==static_cast<uint8>(MarketBranches::EStage::Open))
        {
            ++Stores;const int32 Format=MarketDepartments::FormatIndex(Branch.Format);
            for(const auto& Dept:Branch.Depts)
            {auto& Row=Groups.FindOrAdd(Key(Format,Dept.Dept));Row.Day=Day;Row.Format=Format;Row.Dept=Dept.Dept;++Row.Branches;Row.Profit+=Dept.Last30Profit;Row.Revenue+=Dept.Last30Revenue;}
        }
        for(const auto& Pair:Groups)
        {
            const auto& Row=Pair.Value; Stats.LastDept.Add(Pair.Key,Row.Profit);
            if(!Stats.WorstDept.Contains(Pair.Key)) {Stats.WorstDept.Add(Pair.Key,Row.Profit);Stats.BestDept.Add(Pair.Key,Row.Profit);}
            else {Stats.WorstDept[Pair.Key]=FMath::Min(Stats.WorstDept[Pair.Key],Row.Profit);Stats.BestDept[Pair.Key]=FMath::Max(Stats.BestDept[Pair.Key],Row.Profit);}
            if(Day%30==0)Stats.Departments.Add(Row);
        }
        BaseEvent|=Stores!=Stats.LastStores;Stats.LastStores=Stores;
        Extra|=State.Rivals.NationalRank!=Stats.LastNational||State.Rivals.LeagueRank!=Stats.LastWorld;
        Stats.LastNational=State.Rivals.NationalRank;Stats.LastWorld=State.Rivals.LeagueRank;
        Quiet(BaseEvent,Stats.QuietBase,Stats.BoringBase);Quiet(BaseEvent||Extra,Stats.QuietAll,Stats.BoringAll);
        Pile(Stats.BaseBadDays,Day,Stats.bPileBase,Stats.PilesBase);Pile(Stats.BadDays,Day,Stats.bPileAll,Stats.PilesAll);
        const int32 Year=Stats.Years.Num()+1;
        if(Day==MarketCalendar::GameDayOf(MarketCalendar::StartYear+Year,MarketCalendar::StartMonth,MarketCalendar::StartDayOfMonth)-1)
        {
            const auto World=MarketChains::WorldTable(State); const auto National=MarketChains::NationalTable(State,State.CountryId);
            FYear Row;Row.Year=Year;Row.Day=Day;Row.World=MarketChains::ListedRank(World,MarketChains::WorldListSize);Row.National=MarketChains::ListedRank(National,MarketChains::NationalListSize(State.CountryId));Row.Stores=Stores;
            if(!World.IsEmpty())Row.LeaderWorld=World[0].Revenue;
            for(const auto& Entry:World)if(Entry.bUs)Row.OurWorld=Entry.Revenue;
            Stats.Years.Add(Row);
        }
    }
    FString DeptCsv(const FStats& Stats,const FString& Style,int32 Seed)
    {
        FString Out;
        for(const auto& Row:Stats.Departments)Out+=FString::Printf(TEXT("%s,%d,%d,%d,%d,%d,%lld,%lld\n"),*Style,Seed,Row.Day,Row.Format,Row.Dept,Row.Branches,Row.Profit,Row.Revenue);
        return Out;
    }
    FString Report(const FStats& Stats)
    {
        FString Out=TEXT("\n#### C: yeni sistemlerin sonucu\n\n| Y\u0131l | Ulusal s\u0131ra | D\u00fcnya s\u0131ras\u0131 | Ma\u011faza | Bizim / liderin ortak cirosu |\n|---:|---:|---:|---:|---:|\n");
        for(const auto& Row:Stats.Years)Out+=FString::Printf(TEXT("| %d | %d | %d | %d | %.0f / %.0f |\n"),Row.Year,Row.National,Row.World,Row.Stores,Row.OurWorld,Row.LeaderWorld);
        Out+=FString::Printf(TEXT("\nRakipler: en \u00e7ok %d etkin zincir; %d farkl\u0131 sat\u0131l\u0131k zincir; %d piyasadan \u00e7ekilme (iflas veya sat\u0131n al\u0131nma); %d g\u00f6r\u00fcn\u00fcr iflas haberi; bizim %d sat\u0131n almam\u0131z; %d fiyat sava\u015f\u0131.\nEzeli rakip: %s.\nMarkalardan toplam %.2f TL; k\u00fcsen farkl\u0131 marka %d.\n"),Stats.ChainPeak,Stats.Sale,Stats.Gone,Stats.BankruptcyNews,Stats.Purchases,Stats.Wars,Stats.Nemesis.IsEmpty()?TEXT("yok"):*Stats.Nemesis,Stats.BrandMoney/100.,Stats.CoolBrands.Num());
        Out+=FString::Printf(TEXT("\nStrateji (D9): %s.\n"),Stats.Strategy.IsEmpty()?TEXT("-"):*Stats.Strategy);
        Out+=FString::Printf(TEXT("Portf\u00f6y ve m\u00fcdahaleler (D9b): %s.\n"),Stats.Portfolio.IsEmpty()?TEXT("-"):*Stats.Portfolio);
        Out+=FString::Printf(TEXT("\nC3 kapanma nedenleri: %d iflas/kapanma, %d rakip taraf\u0131ndan al\u0131nma, %d bizim al\u0131m\u0131m\u0131z. Haber say\u0131s\u0131 alt s\u0131n\u0131rd\u0131r; nedenler do\u011frudan sistem saya\u00e7lar\u0131ndan gelir.\n"),Stats.Closures,Stats.Takeovers,Stats.OurBuys);
        Out+=FString::Printf(TEXT("\nDefter denetimi: a\u00e7\u0131klanamayan fark %d g\u00fcn, toplam %.2f TL; mutlak fark toplam\u0131 %.2f TL.\nHedefler: g\u00f6zlenen %d, tamamlanan %d; kutlama %d. Ritim koruyucusu: %d sakin d\u00f6nem olay\u0131, %d ertelenen k\u00f6t\u00fc olay; e\u015fi\u011fi a\u015fan %d s\u0131k\u0131c\u0131 d\u00f6nem, en uzun sessizlik %d g\u00fcn.\n"),Stats.GapDays,Stats.GapTotal/100.,Stats.GapAbsolute/100.,Stats.GoalsSeen,Stats.GoalsCompleted,Stats.Celebrations,Stats.QuietEvents,Stats.HeldBadEvents,Stats.RhythmBoring,Stats.RhythmLongest);
        Out+=TEXT("\nD\u00f6nemler (g\u00fcnler kampanya ba\u015flang\u0131c\u0131ndan; k\u00e2r defterden; kasalar ilk/son g\u00fcn kapan\u0131\u015f\u0131, TL):\n\n| D\u00f6nem / dalga | Planlanan g\u00fcnler | Oynanan g\u00fcn | \u0130lk kasa | Son kasa | En az kasa | Net k\u00e2r |\n|---|---|---:|---:|---:|---:|---:|\n");
        for(const auto& Era:Stats.Eras)Out+=FString::Printf(TEXT("| %s / %d | %d\u2013%d | %d | %.2f | %.2f | %.2f | %.2f |\n"),*MarketEras::Name(static_cast<MarketEras::EKind>(Era.Kind)),Era.Wave,Era.Start,Era.End,Era.Days,Era.FirstCash/100.,Era.LastCash/100.,Era.LowestCash/100.,Era.Profit/100.);
        Out+=TEXT("\nTedarik kademe de\u011fi\u015fimleri (hat, g\u00fcn, \u00f6nce, sonra):\n");
        for(const auto& Row:Stats.Sourcing)Out+=FString::Printf(TEXT("- %s: %d. g\u00fcn %d -> %d.\n"),*MarketSourcing::LineName(static_cast<MarketSourcing::ELine>(Row.Line)),Row.Day,Row.From,Row.To);
        if(Stats.Sourcing.IsEmpty())Out+=TEXT("- Kademe de\u011fi\u015fmedi.\n");
        Out+=TEXT("\nReyonlar: her t\u00fcr/ma\u011faza t\u00fcr\u00fc i\u00e7in g\u00f6r\u00fclen en d\u00fc\u015f\u00fck / en y\u00fcksek 30 g\u00fcnl\u00fck k\u00e2r (TL, t\u00fcm o t\u00fcr \u015fubelerin toplam\u0131):\n");
        TArray<FString> Keys;Stats.WorstDept.GetKeys(Keys);Keys.Sort();
        for(const auto& Id:Keys){FString F,D;Id.Split(TEXT("|"),&F,&D);Out+=FString::Printf(TEXT("- T\u00fcr %s, %s: %.2f / %.2f%s.\n"),*F,*MarketDepartments::Name(static_cast<MarketDepartments::EDept>(FCString::Atoi(*D))),Stats.WorstDept[Id]/100.,Stats.BestDept[Id]/100.,Stats.WorstDept[Id]<0?TEXT(" (zarar g\u00f6r\u00fcld\u00fc)"):TEXT(""));}
        if(Keys.IsEmpty())Out+=TEXT("- Reyon i\u015fletilmedi; k\u00e2r s\u0131ralamas\u0131 i\u00e7in veri yok.\n");
        Out+=FString::Printf(TEXT("\nAk\u0131\u015f: mahalle karar/olay/ma\u011faza e\u015fi\u011fi say\u0131m\u0131yla %d s\u0131k\u0131c\u0131 d\u00f6nem; C'nin teklif/s\u0131ra/sava\u015f/kademe hareketi de say\u0131l\u0131nca %d. Felaket y\u0131\u011f\u0131lmas\u0131 %d / %d. Bu ayn\u0131 kampanyan\u0131n iki g\u00f6zlemidir, eski s\u00fcr\u00fcmle yeniden oynama de\u011fildir.\n"),Stats.BoringBase,Stats.BoringAll,Stats.PilesBase,Stats.PilesAll);
        TArray<FString> Actions;Stats.Commands.GetKeys(Actions);Actions.Sort();Out+=TEXT("\nBa\u015far\u0131l\u0131 yeni oyuncu komutlar\u0131:\n");
        for(const auto& Action:Actions)Out+=FString::Printf(TEXT("- %s: %d.\n"),*Action,Stats.Commands[Action]);
        Out+=FString::Printf(TEXT("- Asgari al\u0131m\u0131 kar\u015f\u0131layamad\u0131\u011f\u0131 i\u00e7in ertelenen tedarik karar\u0131: %d.\n"),Stats.Blocked.FindRef(TEXT("Tedarik asgarisi")));
        TArray<FString> Reasons;Stats.Blocked.GetKeys(Reasons);Reasons.Sort();
        Out+=TEXT("\nErtelenen kararlar (oyuncuya d\u00f6nen neden, tekrar say\u0131s\u0131):\n");
        for(const auto& Reason:Reasons)Out+=FString::Printf(TEXT("- %s: %d.\n"),*Reason,Stats.Blocked[Reason]);
        Out+=TEXT("\nMa\u011faza t\u00fcr\u00fc: 1 mahalle, 2 s\u00fcpermarket, 3 hipermarket. CSV reyon numaralar\u0131 MarketDepartments::EDept s\u0131ras\u0131d\u0131r.\n");
        return Out;
    }
    FString EraCsv(const FStats& Stats,const FString& Style,int32 Seed)
    {
        FString Out;
        for(const auto& Era:Stats.Eras)Out+=FString::Printf(TEXT("%s,%d,%d,%d,%d,%d,%d,%lld,%lld,%lld,%lld\n"),*Style,Seed,Era.Kind,Era.Wave,Era.Start,Era.End,Era.Days,Era.FirstCash,Era.LastCash,Era.LowestCash,Era.Profit);
        return Out;
    }
}

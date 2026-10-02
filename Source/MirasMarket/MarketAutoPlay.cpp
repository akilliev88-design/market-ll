#include "MarketAutoPlay.h"
#include "MarketTuning.h"
#include "MarketSimulation.h"
#include "MarketOrderAdvice.h"
#include "MarketStart.h"
#include "MarketDirector.h"
#include "MarketCountry.h"
#include "MarketBranches.h"
#include "MarketCompany.h"
#include "MarketDepots.h"
#include "MarketManagers.h"
#include "MarketStaff.h"
#include "MarketEvents.h"
#include "MarketFinance.h"
#include "MarketBanking.h"
#include "MarketSuppliers.h"
#include "MarketPrices.h"
#include "MarketCalendar.h"
#include "MarketLedger.h"
#include "MarketCompetitors.h"
#include "MarketRivals.h"
#include "ProductCatalog.h"
#include "Planogram.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Misc/Parse.h"
#include "Misc/CommandLine.h"

namespace MarketAutoPlay
{
    const TArray<FProfile>& Profiles()
    {
        static const TArray<FProfile> Values = {
            { EStyle::Careful, TEXT("Temkinli"), 30, 2.5, false, 30, 1.05, 12, {365,60,1,2,.8f,1.4f,1.15f,false,2.0}, .88,365,8,30 },
            { EStyle::Balanced, TEXT("Dengeli"), 14, 1.5, false, 14, 1.0, 8, {180,30,2,1,.95f,1.2f,1.05f,true,1.5}, .88,90,6,30 }, // C12c: hypermarkets from 30 shops (was 20)
            { EStyle::Bold, TEXT("Atak"), 7, 1.1, true, 7, 0.95, 4, {60,15,3,0,1.f,1.f,1.f,true,1.1}, .82,60,3,12 }
        };
        return Values;
    }
    int64 Buffer(const FMarketState& State, const FProfile& Profile)
    {
        return Profile.BufferDays * (State.DailyPayroll() + MarketPrices::Scaled(2200, State.Day));
    }
    bool Command(FMarketState& State, const TArray<FMarketProduct>& Products, FName Action, int32 Arg, FRun& Run)
    {
        FString Message;
        const bool Done = MarketDirector::Command(State, Products, Action, Arg, Message);
        if (!Done) ++Run.RejectedDecisions;
        return Done;
    }
    void ResolveChoices(FMarketState& State, const TArray<FMarketProduct>& Products, const FProfile& Profile, FRun& Run)
    {
        for (int32 Count = 0; Count < 8; ++Count)
        {
            const FMarketDecision* Pending = MarketEvents::Pending(State);
            if (!Pending) break;
            const FMarketDecision Choice = *Pending;
            // Never sell the campaign or accept a gift of emergency debt for the debt-free profile.
            int32 Option = Choice.DefaultOption;
            if (Choice.Id.StartsWith(TEXT("story."))) Option = 0;
            if (Choice.Id == TEXT("story.identity")) Option = static_cast<int32>(Profile.Style);
            if (Choice.Id.StartsWith(TEXT("finance."))) Option = Profile.bBorrow && !MarketAutoPlayRescue::Blocked(State) ? 0 : FMath::Min(1, Choice.Options.Num() - 1);
            if(Choice.Id.StartsWith(TEXT("online.")))Option=MarketAutoPlayOnline::Choice(State,Choice,static_cast<int32>(Profile.Style),Run.Online.Offline);
            if(Choice.Id.StartsWith(TEXT("command.")))Option=MarketAutoPlayCommand::Choice(State,Products,Choice,Profile.ExpansionBuffer);
            if(Run.bNoGrowth && (Choice.Id.StartsWith(TEXT("command.open")) || Choice.Id.StartsWith(TEXT("finance.")))) Option=FMath::Min(1, Choice.Options.Num()-1);
            Option = FMath::Clamp(Option, 0, FMath::Max(0, Choice.Options.Num() - 1));
            const bool Chosen=Command(State, Products, TEXT("Decide"), Option, Run);
            if(Chosen && Choice.Id.StartsWith(TEXT("online.")))MarketAutoPlayOnline::RecordChoice(State,Choice,Option,Run.Online);
            if(Chosen && Choice.Id.StartsWith(TEXT("command.")))MarketAutoPlayCommand::RecordChoice(State,Choice,Option,Run.CommandStats);
            if (!Chosen)
            {
                bool Resolved = false;
                for (int32 Alternative = 0; Alternative < Choice.Options.Num(); ++Alternative)
                    if (Alternative != Option && Command(State, Products, TEXT("Decide"), Alternative, Run)) { Resolved = true; break; }
                if (!Resolved) break; // existing deadline machinery will apply the default
            }
        }
    }
    void Manage(FMarketState& State, const TArray<FMarketProduct>& Products, const FProfile& Profile, FRun& Run)
    {
        if (State.Day % 7 != 1) return;
        FString Message;
        const int64 Reserve = FMath::Max(Buffer(State, Profile), MarketAutoPlayFinance::NetworkReserve(State));
        // Estimate recoverable gross profit from visible service losses, never total gross profit.
        const auto Family=MarketLedger::Statement(State,FMath::Max(1,State.Day-30),State.Day-1,MarketLedger::FamilyShop);
        int64 Sold=0; int32 Days=0;
        for(const auto& Day:State.History)if(Day.Day>=State.Day-30 && Day.Day<State.Day){Sold+=Day.Served;++Days;}
        const int64 GrossPerBasket=FMath::Max<int64>(0,Family.GrossProfit)/FMath::Max<int64>(1,Sold);
        const int64 QueueBenefit=State.LastLostWaiting*GrossPerBasket*30;
        int64 ShelfBenefit=0;
        for(int32 I=0;I<State.Stock.Num();++I)
            if(State.Stock[I].Warehouse>0)ShelfBenefit+=State.Stock[I].Yesterday.Empty*FMath::Max<int64>(0,State.Stock[I].Price-Products[I].Cost)*30;
        auto Hire=[&](MarketStaff::ERole Role,int64 Benefit)
        {
            MarketStaff::EnsureCandidates(State);
            for(int32 I=0;I<State.Candidates.Num();++I)
                if(MarketStaff::RoleOf(State.Candidates[I])==Role && MarketAutoPlayFinance::WorthHiring(State,Benefit,State.Candidates[I].DailyWage,MarketStaff::HireCostOn(Role,State.Day)))
                {MarketStaff::Hire(State,I,Message);break;}
        };
        if(Days>=7 && (!State.bCashier || State.LastLostWaiting>0))Hire(MarketStaff::ERole::Cashier,QueueBenefit);
        if(Days>=7 && ShelfBenefit>0)Hire(MarketStaff::ERole::Stocker,ShelfBenefit);
        if (State.Cash > Reserve * 2 && !MarketStaff::HasAccountant(State)) MarketStaff::HireAccountant(State, Message);
        for (const FMarketEmployee& Employee : State.Staff)
            if (Employee.Fatigue > 65.f) MarketStaff::GiveDayOff(State, Employee.Id, false, Message);
        if(MarketStaff::HrUnlocked(State) && !MarketStaff::HasHr(State))
        {
            // Visible negotiated wages (8%) and actual replacement fees are HR's potential savings.
            const auto Books=MarketLedger::Statement(State,FMath::Max(1,State.Day-30),State.Day-1);
            int64 Benefit=FMath::RoundToInt64(30*State.DailyPayroll()*.08)+FMath::Max<int64>(0,-Books.At(MarketLedger::EAccount::Severance));
            int64 Commitment=0;
            if(!Run.bNoGrowth && !MarketAutoPlayRescue::Blocked(State) && MarketBranches::OpenCount(State)>=2)
            {
                // HR enables another store. Forecast its incremental contribution from a mature visible
                // store of the same format, with no bonus for a richer destination or hidden future sales.
                for(const auto& City:MarketCountry::Active().Cities)
                {
                    const auto Next=MarketBranches::SiteOf(State,State.CountryId,City.Id);
                    if(MarketBranches::ShopsIn(State,State.CountryId,City.Id)>=MarketBranches::Room(Next) ||
                        !MarketAutoPlayC::SiteSuitable(State,State.CountryId,City.Id,TEXT("mahalle")))continue;
                    const int64 Fixed=MarketBranches::MonthlyFixedCost(State,State.CountryId,City.Id,TEXT("mahalle"));
                    int64 Opening=-1; // expensive shelf planning, once per promising site
                    for(const auto& Branch:State.Branches)
                    {
                        if(Branch.Stage!=static_cast<uint8>(MarketBranches::EStage::Open) || Branch.Format!=TEXT("mahalle") || State.Day-Branch.OpenedDay<90)continue;
                        const auto From=MarketBranches::SiteOf(State,Branch.Country,Branch.Province);
                        const int64 OldFixed=MarketBranches::MonthlyFixedCost(State,Branch.Country,Branch.Province,Branch.Format);
                        const int64 Contribution=FMath::RoundToInt64((Branch.Last30Profit+OldFixed)*FMath::Min(1.f,Next.Income/FMath::Max(.1f,From.Income)))-Fixed;
                        if(Contribution>Benefit)
                        {
                            if(Opening<0)Opening=FMath::RoundToInt64(MarketBranches::OpeningCost(State,Products,State.CountryId,City.Id,TEXT("mahalle"))*Profile.ExpansionBuffer);
                            if(State.Cash>=Reserve+Opening+Fixed){Benefit=Contribution;Commitment=Opening+Fixed;}
                        }
                    }
                }
            }
            MarketStaff::EnsureCandidates(State);
            for(int32 I=0;I<State.Candidates.Num();++I)
                if(MarketStaff::RoleOf(State.Candidates[I])==MarketStaff::ERole::HrManager &&
                    MarketAutoPlayFinance::WorthHiring(State,Benefit,State.Candidates[I].DailyWage,Commitment+MarketStaff::HireCostOn(MarketStaff::ERole::HrManager,State.Day)))
                {MarketStaff::Hire(State,I,Message);break;}
        }
        if (State.Cash <= Reserve) return;
        // Outside candidates are selected by visible list order, never by hidden honesty/potential.
        auto Appoint = [&](MarketManagers::ELevel Level, const FString& Country, const FString& Area)
        {
            FString Reason;
            if (MarketManagers::FindManager(State, Level, Country, Area) == INDEX_NONE &&
                MarketManagers::CanAppoint(State, Level, Country, Area, INDEX_NONE, Reason))
            {
                const auto Candidates=MarketManagers::Candidates(State,Level,Country,Area);
                if(!Candidates.IsEmpty() && (Level!=MarketManagers::ELevel::FamilyShop ||
                    MarketAutoPlayFinance::WorthHiring(State,(QueueBenefit+ShelfBenefit)/2,Candidates[0].Wage)))
                    Command(State, Products, TEXT("AppointCandidate"), MarketManagers::EncodeArea(Level, Country, Area) * 10, Run);
            }
        };
        Appoint(MarketManagers::ELevel::FamilyShop, State.CountryId, FString());
        for (const MarketCountry::FProfile& Country : MarketCountry::All())
        {
            for (const MarketCountry::FCity& Province : Country.Cities) Appoint(MarketManagers::ELevel::Province, Country.Id, Province.Id);
            for (const MarketCountry::FRegion& Region : Country.SubRegions) Appoint(MarketManagers::ELevel::SubRegion, Country.Id, Region.Id);
            for (const MarketCountry::FRegion& Region : Country.Regions) Appoint(MarketManagers::ELevel::Region, Country.Id, Region.Id);
            Appoint(MarketManagers::ELevel::Country, Country.Id, Country.Id);
        }
        for (int32 Index = 0; Index < State.Company.DepotSites.Num(); ++Index)
        {
            const FMarketDepot& Depot = State.Company.DepotSites[Index];
            Appoint(MarketManagers::ELevel::Depot, Depot.Country, Depot.Province);
        }
        for (int32 Index = 0; Index < State.Branches.Num(); ++Index)
            if (State.Branches[Index].Stage == static_cast<uint8>(MarketBranches::EStage::Open) && State.Branches[Index].ManagerName.IsEmpty())
                Command(State, Products, TEXT("ManagerHireFor"), Index * 10, Run);
    }
    void Grow(FMarketState& State, const TArray<FMarketProduct>& Products, const FProfile& Profile, FRun& Run)
    {
        if (MarketAutoPlayRescue::Blocked(State) || (State.Day - 1) % Profile.GrowthInterval != 0) return;
        const int64 Reserve = FMath::Max(Buffer(State, Profile), MarketAutoPlayFinance::NetworkReserve(State));
        if (Profile.bBorrow && State.Day % 30 == 1 && MarketFinance::Debt(State) == 0)
            Command(State, Products, TEXT("TakeLoan"), 2, Run);
        if (State.Cash <= Reserve) return;
        const int32 Stores = MarketCompany::TotalStores(State);
        if (Stores >= Profile.DepotAt && MarketDepots::Count(State) == 0)
        {
            const MarketDepots::FAdvice Advice = MarketDepots::SuggestDepotProvince(State, State.CountryId);
            if (!Advice.Province.IsEmpty() && State.Cash > MarketDepots::BuildCost(State, State.CountryId, Advice.Province) + Reserve)
                Command(State, Products, TEXT("BuildDepotIn"), MarketManagers::EncodeArea(MarketManagers::ELevel::Depot, State.CountryId, Advice.Province), Run);
        }
        if (Stores >= 8) Command(State, Products, TEXT("Build"), 2, Run);
        if (MarketDepots::Count(State) > 0 && State.Company.Trucks < MarketDepots::TrucksNeeded(State)) Command(State, Products, TEXT("Build"), 1, Run);
        if (Stores >= 20) Command(State, Products, TEXT("Build"), 3, Run);
        FString Format = Stores >= Profile.HyperAt ? TEXT("hiper") : Stores >= Profile.SuperAt ? TEXT("buyuk") : TEXT("mahalle");
        if(Format==TEXT("hiper") && !MarketCompany::ChapterOpen(State,MarketBranches::FormatInfo(Format).Chapter))Format=TEXT("buyuk");
        TArray<MarketBranches::FSite> Sites;
        for (const MarketCountry::FProfile& Country : MarketCountry::All())
        {
            if (Country.Id != State.CountryId && !MarketCompany::ChapterOpen(State, 6)) continue;
            for (const MarketCountry::FCity& Province : Country.Cities)
            {
                const auto Site = MarketBranches::SiteOf(State, Country.Id, Province.Id);
                if (!Site.bHome && (!MarketStaff::HasHr(State) || !MarketStaff::HasAccountant(State))) continue;
                if (MarketBranches::ShopsIn(State, Country.Id, Province.Id) >= MarketBranches::Room(Site)) continue;
                if(!MarketAutoPlayC::SiteSuitable(State,Country.Id,Province.Id,Format))continue;
                Sites.Add(Site);
            }
        }
        Sites.Sort([&](const MarketBranches::FSite& Left, const MarketBranches::FSite& Right)
        {
            const float A = Left.Rent / FMath::Max(0.1f, Left.Income) + MarketBranches::ShopsIn(State, Left.Country, Left.Province) * 0.25f;
            const float B = Right.Rent / FMath::Max(0.1f, Right.Income) + MarketBranches::ShopsIn(State, Right.Country, Right.Province) * 0.25f;
            return A == B ? Left.Country + Left.Province < Right.Country + Right.Province : A < B;
        });
        if (Sites.IsEmpty()) return;
        // Investigate one visible site per growth turn. OpeningCost plans shelves and is deliberately not run
        // for every province every week; map rent/income and our crowding choose which site to investigate.
        const auto& Site = Sites[0];

        FString Reason;
        if (!MarketBranches::CanOpen(State, Products, Site.Country, Site.Province, Format, Reason)) { ++Run.C.Blocked.FindOrAdd(Reason); return; }
        const int64 Cost = MarketBranches::OpeningCost(State, Products, Site.Country, Site.Province, Format);
        if (MarketAutoPlayFinance::CanExpand(State,Cost,MarketBranches::MonthlyFixedCost(State,Site.Country,Site.Province,Format),Profile.ExpansionBuffer))
            Command(State, Products, TEXT("OpenBranch"), MarketBranches::EncodeSite(Site.Country, Site.Province, Format), Run);
    }
    int64 PlaceOrder(FMarketState& State, const TArray<FMarketProduct>& Products, const FProfile& Profile, FRun& Trial)
    {
        TArray<int32> Suggested, Draft;
        const TArray<float> Scales = MarketDirector::OrderScales(State, Products);
        MarketOrderAdvice::FillSuggested(State, Products, Suggested, &Scales);
        Draft.Init(0, Products.Num());
        const int64 Allowance = MarketDirector::OrderAllowance(State);
        // Goods money is protected from investment and payroll raises, but is available to buy goods.
        const int64 Spendable = FMath::Max<int64>(0, State.Cash - FMath::Min(Buffer(State, Profile), FMath::Max<int64>(0, State.Cash / 2))) + Allowance;
        int64 Bill = 0;
        // Buy one case per useful line per pass: a large suggestion must not block every small order.
        for (int32 Pass = 0; Pass < MarketOrderAdvice::MaxCases; ++Pass)
            for (int32 Index = 0; Index < Products.Num(); ++Index)
            {
                if (Draft[Index] >= Suggested[Index]) continue;
                const int64 CaseCost = Products[Index].Cost * MarketOrderAdvice::CaseUnits(Products[Index]);
                if (CaseCost <= 0 || Bill + CaseCost > Spendable) continue;
                ++Draft[Index]; Bill += CaseCost;
            }
        if (Bill < MarketOrderAdvice::MinimumOrderOn(State.Day)) return 0;
        const int64 Before = State.Cash;
        if (!State.SubmitOrder(Draft, Products, &Bill, nullptr, Allowance)) { ++Trial.RejectedDecisions; return 0; }
        FString OrderLines;
        for(int32 I=0;I<Draft.Num();++I)if(Draft[I]>0)OrderLines+=FString::Printf(TEXT("%s:%d|"),*Products[I].Id,Draft[I]);
        Trial.Diagnosis.Events.Add(FString::Printf(TEXT("%d,order,%lld,%s\n"),State.Day,Bill,*OrderLines));
        if (State.Cash != Before - Bill) { ++Trial.AuditFailures; Trial.Issues.AddUnique(TEXT("Siparis bedeli kasayla uyusmuyor.")); }
        MarketDirector::OnOrder(State, Bill);
        if (Profile.Style == EStyle::Careful && State.Cash-MarketSuppliers::OpenBills(State)>=MarketAutoPlayFinance::NetworkReserve(State)) Command(State, Products, TEXT("PayBills"), 0, Trial);
        return Bill;
    }
    int64 PriceTarget(const FMarketState& State,const TArray<FMarketProduct>& Products,int32 Index,const FProfile& Profile)
    {
        const auto& Product=Products[Index];
        const bool Growing=State.Day>Profile.GrowthPriceAt && State.MarketShare<40.f && MarketBranches::OpenCount(State)<2;
        const double Rival=MarketCompetitors::RivalPriceFactor(State,Product.Category,MarketRivals::Aisles(Products));
        const double Factor=FMath::Min(Growing?Profile.GrowthPriceFactor:Profile.PriceFactor,Rival*(State.MarketShare<40.f?1.0:1.03));
        const int64 Desired=FMath::Max(FMath::RoundToInt64(Product.Cost*1.05),FMath::RoundToInt64(Product.BasePrice*Factor));
        // An opening or share threshold must not cause an abrupt family-shop price jump.
        return State.Stock[Index].Price>0?FMath::Min(Desired,FMath::RoundToInt64(State.Stock[Index].Price*1.03)):Desired;
    }
    void SetPrices(FMarketState& State, const TArray<FMarketProduct>& Products, const FProfile& Profile)
    {
        if (State.Day % 7 != 1) return;
        for (int32 Index = 0; Index < Products.Num(); ++Index)
        {
            const int64 Target = PriceTarget(State,Products,Index,Profile);
            for (int32 Step = 0; Step < 20; ++Step)
            {
                const int64 Previous = State.Stock[Index].Price;
                if (Previous == Target) break;
                if (!MarketSimulation::AdjustPrice(State, Products, Index, Previous < Target)) break;
                if (FMath::Abs(State.Stock[Index].Price - Target) >= FMath::Abs(Previous - Target))
                { MarketSimulation::AdjustPrice(State, Products, Index, Previous >= Target); break; }
            }
        }
    }
    TArray<FString> Validate(const FMarketState& State, const TArray<FMarketProduct>& Products)
    {
        TArray<FString> Errors;
        auto Check = [&](bool Value, const TCHAR* Text) { if (!Value) Errors.AddUnique(Text); };
        Check(State.IsValidFor(Products), TEXT("Kayit yapisi veya katalog eslesmesi gecersiz."));
        Check(State.Stock.Num() == Products.Num(), TEXT("Stok ve katalog satirlari uyusmuyor."));
        Check(FMath::IsFinite(State.MarketShare) && State.MarketShare >= 0 && State.MarketShare <= 100, TEXT("Pazar payi sinir disinda."));
        Check(FMath::IsFinite(State.ShelfPriceLevel) && State.ShelfPriceLevel > 0, TEXT("Fiyat duzeyi gecersiz."));
        for (const FMarketStock& Stock : State.Stock)
        {
            Check(Stock.Shelf >= 0 && Stock.Warehouse >= 0 && Stock.Dock >= 0 && Stock.Incoming >= 0, TEXT("Negatif urun adedi var."));
            Check(Stock.Shelf <= Stock.Capacity && Stock.Capacity >= 0 && Stock.Capacity <= FMarketState::MaxShelfCapacity, TEXT("Raf kapasitesi asildi."));
            Check(Stock.Price >= 0 && Stock.AvgCost >= 0, TEXT("Negatif fiyat veya maliyet var."));
            Check(FMath::IsFinite(Stock.PromoHeat) && FMath::IsFinite(Stock.Pantry), TEXT("Urun ilgisi gecersiz."));
        }
        for (const FMarketBranch& Branch : State.Branches)
        {
            Check(FMath::IsFinite(Branch.Satisfaction) && Branch.Satisfaction >= 0 && Branch.Satisfaction <= 100, TEXT("Sube memnuniyeti gecersiz."));
            for (const FMarketBranchItem& Item : Branch.Items) Check(Item.Units >= 0 && Item.Incoming >= 0, TEXT("Subede negatif stok var."));
        }
        for (const FMarketEmployee& Person : State.Staff)
            Check(FMath::IsFinite(Person.Morale) && FMath::IsFinite(Person.Fatigue) && Person.Morale >= 0 && Person.Morale <= 100 && Person.Fatigue >= 0 && Person.Fatigue <= 100, TEXT("Calisan durumu gecersiz."));
        for (const FMarketCompetitor& Rival : State.Competitors)
            Check(FMath::IsFinite(Rival.Share) && Rival.Share >= 0 && Rival.Share <= 1, TEXT("Rakip payi gecersiz."));
        for (const FMarketLoan& Loan : State.Loans)
            Check(Loan.Remaining >= 0 && FMath::IsFinite(Loan.MonthlyRate) && Loan.MonthlyRate >= 0, TEXT("Kredi kaydi gecersiz."));
        return Errors;
    }
    bool LoadInputs(TArray<FMarketProduct>& OutBase, TArray<int32>& OutCapacities, TArray<FString>& OutErrors)
    {
        if (!MarketCatalog::LoadFile(MarketCatalog::DefaultPath(), OutBase, OutErrors)) return false;
        OutBase.RemoveAll([](const FMarketProduct& Product) { return !Product.bActive; });
        FMarketPlanogram Plan;
        if (!MarketPlanogram::LoadFile(MarketPlanogram::DefaultPath(), Plan, OutErrors)) return false;
        MarketPlanogram::FitDepth(Plan, OutBase);
        OutCapacities.Reset();
        for (const FMarketProduct& Product : OutBase) OutCapacities.Add(MarketPlanogram::ProductCapacity(Plan, OutBase, Product.Id));
        return !OutBase.IsEmpty();
    }
    FRow Row(const FMarketState& State)
    {
        FRow Value;
        Value.Day = State.Day - 1; Value.Cash = State.Cash;
        Value.Debt = State.InheritedDebt + MarketFinance::Debt(State) + MarketBanking::Debt(State) + MarketSuppliers::OpenBills(State) + State.Books.TaxDue; // M28
        Value.Profit = State.LastProfit; Value.Revenue = State.LastRevenue;
        Value.Stores = MarketCompany::TotalStores(State); Value.Provinces = MarketCompany::Provinces(State);
        Value.Share = MarketCompany::NationalShare(State);
        Value.NationalRank=State.Rivals.NationalRank; Value.WorldRank=State.Rivals.LeagueRank;
        Value.Workers = State.Staff.Num() + State.Management.Managers.Num();
        for (const FMarketBranch& Branch : State.Branches)
            if (Branch.Stage == static_cast<uint8>(MarketBranches::EStage::Open)) Value.Workers += Branch.Workers + (!Branch.ManagerName.IsEmpty() ? 1 : 0);
        return Value;
    }
    void Milestones(const FMarketState& State, FRun& Run)
    {
        auto Mark = [&](const TCHAR* Name, bool Reached) { if (Reached && !Run.Milestones.Contains(Name)) Run.Milestones.Add(Name, State.Day - 1); };
        Mark(TEXT("Ilk sube"), MarketCompany::TotalStores(State) >= 2);
        Mark(TEXT("5 magaza"), MarketCompany::TotalStores(State) >= 5);
        Mark(TEXT("Ilk depo"), MarketDepots::Count(State) > 0);
        bool HasProvinceManager = false;
        for (const FMarketManager& Manager : State.Management.Managers) HasProvinceManager |= Manager.Level == static_cast<uint8>(MarketManagers::ELevel::Province);
        Mark(TEXT("Ilk il muduru"), HasProvinceManager);
        Mark(TEXT("5 il"), MarketCompany::Provinces(State) >= 5);
        Mark(TEXT("Ikinci ulke"), MarketCompany::ForeignCountries(State) > 0);
    }
    FReport Run(const FOptions& Options, const TArray<FMarketProduct>& Base, const TArray<int32>& Capacities, int32 RestoreSeed)
    {
        FReport Report; Report.Options = Options; Report.Tuning = MarketTuning::Describe();
        FParse::Value(FCommandLine::Get(),TEXT("MirasAutoPlayStyle="),Report.Options.StyleIndex);
        Report.Options.bOfflineCareful |= FParse::Param(FCommandLine::Get(),TEXT("MirasNoInternet"));
        Report.Options.bNoGrowth |= FParse::Param(FCommandLine::Get(),TEXT("MirasNoGrowth"));
        const double Started = FPlatformTime::Seconds();
        if (Base.IsEmpty() || Capacities.Num() != Base.Num() || !MarketCountry::FindCity(Options.Country, Options.Province) || Options.Days < 1 || Options.Days > 10958 || Options.Seeds < 1 || Options.Seeds > 100)
        { Report.Errors.Add(TEXT("Katalog, raf plani, ulke, il ya da kosu suresi gecersiz.")); return Report; }
        const MarketCountry::FProfile PreviousCountry = MarketCountry::Active();
        for (const FProfile& Profile : TunedProfiles()) for (int32 SeedIndex = 0; SeedIndex < Options.Seeds; ++SeedIndex)
        {
            if(((Report.Options.bOfflineCareful || Report.Options.bNoGrowth) && Profile.Style!=EStyle::Careful) || (Report.Options.StyleIndex>=0 && static_cast<int32>(Profile.Style)!=Report.Options.StyleIndex))continue;
            FRun Trial; Trial.Profile = Profile.Name; Trial.Seed = Options.FirstSeed + SeedIndex;
            Trial.bNoGrowth=Report.Options.bNoGrowth;
            if(Trial.bNoGrowth)Trial.Profile+=TEXT(" (buyume kapali)");
            Trial.Online.Offline=Report.Options.bOfflineCareful;
            if(Trial.Online.Offline)Trial.Profile+=TEXT(" (internetsiz)");
            FMarketState State; State.Initialize(Base);
            MarketStart::Setup(State, Options.Country, Options.Province, Trial.Seed);
            State.Difficulty = static_cast<uint8>(FMath::Clamp(Options.Difficulty, 0, 2)); // C12: chosen on day 1
            MarketCountry::SetActive(State.CountryId, State.RivalSeed); MarketEras::Activate(State);
            TArray<FMarketProduct> Products = Base;
            MarketDirector::ApplyPrices(State, Base, Products);
            State.ApplyShelfCapacities(Capacities);
            MarketStart::StockShelvesPartly(State, Trial.Seed);
            FRow Week;
            int32 DayIndex = 0;
            while (DayIndex < Options.Days)
            {
                ResolveChoices(State, Products, Profile, Trial);
                int64 Payroll = 0, TaxBefore = 0, Ordered = 0;
                bool BeforeCalled = false;
                MarketSimulation::FHooks Hooks;
                Hooks.bAutoOrder = false;
                Hooks.MaxDays = FMath::Min(31, Options.Days - DayIndex);
                Hooks.BeforeDay = [&]()
                {
                    BeforeCalled = true;
                    MarketAutoPlayFinance::Decide(State,Products,Profile.Style==EStyle::Bold,Profile.Style==EStyle::Careful,Trial.Finance);
                    SetPrices(State, Products, Profile);
                    Manage(State, Products, Profile, Trial);
                    if(!Trial.bNoGrowth)Grow(State, Products, Profile, Trial);
                    MarketAutoPlayC::Decide(State,Products,Profile.C,FMath::Max(Buffer(State,Profile),MarketAutoPlayFinance::NetworkReserve(State)),Trial.C);
                    MarketDirector::ApplyPrices(State,Base,Products);
                    MarketAutoPlayDiagnosis::Policy(State,Products,static_cast<int32>(Profile.Style),Trial.Diagnosis);
                    MarketAutoPlayDiagnosis::BeginDay(State,Products,Profile.PriceFactor,Trial.Diagnosis);
                    Payroll = State.DailyPayroll(); TaxBefore = State.Books.TotalTaxPaid;
                    Ordered = PlaceOrder(State, Products, Profile, Trial);
                    MarketAutoPlayOnline::Decide(State,Products,static_cast<int32>(Profile.Style),Trial.Online);
                    MarketAutoPlayCommand::Decide(State,Products,static_cast<int32>(Profile.Style),Trial.CommandStats);
                    MarketAutoPlayOnline::BeginDay(State,Trial.Online);
                    MarketAutoPlayCommand::BeginDay(State,Trial.CommandStats);
                    MarketAutoPlayRescue::BeginDay(State,Trial.RescueStats);
                };
                Hooks.AfterDay = [&](const MarketSimulation::FDay& Raw)
                {
                    MarketSimulation::FDay DayResult = Raw; DayResult.Ordered = Ordered;
                Trial.AuditFailures += DayResult.AuditFailures;
                MarketAutoPlayDiagnosis::Observe(State,DayResult,Ordered,Trial.Diagnosis);
                if(Trial.bNoGrowth && MarketCompany::TotalStores(State)!=1)
                {++Trial.AuditFailures;Trial.Issues.AddUnique(TEXT("Buyume kapali denemede magaza acildi."));}
                Trial.BackgroundNet += DayResult.BackgroundCashDelta;
                const TArray<FString> Problems = Validate(State, Products);
                Trial.AuditFailures += Problems.Num();
                for (const FString& Problem : Problems) Trial.Issues.AddUnique(Problem);
                if (DayResult.AuditFailures) Trial.Issues.AddUnique(TEXT("Satis, siparis veya gun kapanisinda kasa uyusmazligi."));
                Trial.Expenses.FindOrAdd(TEXT("Aile dukkaninin ucretleri")) += Payroll;
                Trial.Expenses.FindOrAdd(TEXT("Mal alimi (stok yatirimi)")) += DayResult.Ordered;
                Trial.Expenses.FindOrAdd(TEXT("Isletme giderleri (ucret haric)")) += FMath::Max<int64>(0, State.LastOperatingCost - Payroll);
                Trial.Expenses.FindOrAdd(TEXT("Vergi odemeleri")) += State.Books.TotalTaxPaid - TaxBefore;
                Trial.Expenses.FindOrAdd(TEXT("Fire (nakit cikisi degil)")) += State.LastWasteCost;
                Trial.Expenses.FindOrAdd(TEXT("Ust yonetim ucretleri")) += State.Management.LastWages;
                int64 BranchNet = 0;
                for (const FMarketBranch& Branch : State.Branches)
                    if (Branch.Stage == static_cast<uint8>(MarketBranches::EStage::Open))
                    {
                        BranchNet += Branch.LastProfit;
                        Trial.Expenses.FindOrAdd(TEXT("Zararli subeler (net zarar)")) += FMath::Max<int64>(0, -Branch.LastProfit);
                    }
                Trial.Expenses.FindOrAdd(TEXT("Depo ve merkez giderleri")) += FMath::Max<int64>(0, -State.Company.LastProfit);
                Trial.Results.FindOrAdd(TEXT("Aile dukkani")) += DayResult.FamilyProfit;
                Trial.Results.FindOrAdd(TEXT("Subeler")) += BranchNet - State.Online.LastBranchProfit; // M32: their online orders count under Internet
                Trial.Results.FindOrAdd(TEXT("Internet satisi")) += State.Online.LastProfit;
                Trial.Results.FindOrAdd(TEXT("Depo ve merkez")) += State.Company.LastProfit;
                Trial.Results.FindOrAdd(TEXT("Diger (yonetim, banka, fire, kasa farki)")) += DayResult.Profit - DayResult.FamilyProfit - (BranchNet - State.Online.LastBranchProfit) - State.Online.LastProfit - State.Company.LastProfit;
                if (State.Cash < 0) ++Trial.NegativeDays;
                if (State.TroubleStage > 0) ++Trial.TroubleDays;
                const FRow Today = Row(State); Trial.Daily.Add(Today);
                Week.Day = Today.Day; Week.Cash = Today.Cash; Week.Debt = Today.Debt;
                Week.Profit += Today.Profit; Week.Revenue += Today.Revenue;
                Week.Stores = Today.Stores; Week.Provinces = Today.Provinces; Week.Workers = Today.Workers; Week.Share = Today.Share; Week.NationalRank=Today.NationalRank; Week.WorldRank=Today.WorldRank;
                if (State.Day % 7 == 1 || DayIndex + 1 == Options.Days) { Trial.Weekly.Add(Week); Week = FRow(); }
                Milestones(State, Trial);
                MarketAutoPlayC::Observe(State,Trial.C);
                if(Trial.C.Years.Num()>Trial.Finance.Years.Num())
                    UE_LOG(LogTemp,Display,TEXT("AutoPlay progress %s seed %d year %d: %d stores, %d branches, %d ledger entries"),*Trial.Profile,Trial.Seed,Trial.C.Years.Num(),Today.Stores,State.Branches.Num(),State.Ledger.Entries.Num());
                MarketAutoPlayFinance::Observe(State,Trial.C.Years.Num(),Trial.Finance);
                MarketAutoPlayOnline::Observe(State,Trial.C.Years.Num(),Raw.Shoppers,Trial.Online);
                MarketAutoPlayCommand::Observe(State,Trial.C.Years.Num(),Trial.CommandStats);
                MarketAutoPlayRescue::Observe(State,Trial.C.Years.Num(),Trial.RescueStats);
                if(Trial.Online.Offline && (State.Online.bWeb || State.Online.bApp || State.Online.bPlatform || State.Online.bQuick))
                {++Trial.AuditFailures;Trial.Issues.AddUnique(TEXT("Internetsiz denemede kanal acildi."));}
                    ++DayIndex;
                };
                const MarketSimulation::FTurn Turn = MarketSimulation::AdvanceTurn(State, Base, Products, MarketSimulation::ETurn::Month, Hooks);
                if (Turn.Played == 0)
                {
                    // The normal UI stops on cash trouble. The bot keeps observing recovery/late bills, one
                    // ordinary PlayDay at a time, instead of freezing the ten-year report at that first stop.
                    if (!BeforeCalled) Hooks.BeforeDay();
                    Hooks.AfterDay(MarketSimulation::PlayDay(State, Base, Products, false));
                }
            }
            UE_LOG(LogTemp, Display, TEXT("AutoPlay %s seed %d: %d days, cash %lld, stores %d, audit failures %d"), *Trial.Profile, Trial.Seed, Trial.Daily.Num(), Trial.Daily.Last().Cash, Trial.Daily.Last().Stores, Trial.AuditFailures);
            Trial.RejectedDecisions+=Trial.C.Rejected;
            if (Options.bKeepFinalStates) Report.FinalStates.Add(State);
            Report.Runs.Add(MoveTemp(Trial));
        }
        MarketCountry::SetActiveProfile(PreviousCountry, RestoreSeed);
        Report.Seconds = FPlatformTime::Seconds() - Started;
        return Report;
    }
    FString Csv(const TArray<FRun>& Runs, bool bWeekly)
    {
        FString Output = TEXT("tarz,tohum,gun,kasa_kurus,borc_kurus,net_kar_kurus,ciro_kurus,magaza,il,ulusal_pay_yuzde,calisan,ulusal_sira,dunya_sira\n");
        for (const FRun& Trial : Runs)
            for (const FRow& Value : bWeekly ? Trial.Weekly : Trial.Daily)
                Output += FString::Printf(TEXT("%s,%d,%d,%lld,%lld,%lld,%lld,%d,%d,%.8f,%d,%d,%d\n"), *Trial.Profile, Trial.Seed, Value.Day, Value.Cash, Value.Debt, Value.Profit, Value.Revenue, Value.Stores, Value.Provinces, Value.Share, Value.Workers, Value.NationalRank,Value.WorldRank);
        return Output;
    }
    FString ReportMoney(const FReport& Report, int64 Amount)
    {
        const MarketCountry::FProfile* Country = MarketCountry::Find(Report.Options.Country);
        if (!Country) return FString::Printf(TEXT("%lld kurus"), Amount);
        const int64 Local = FMath::RoundToInt64(static_cast<double>(Amount) * Country->DisplayScale);
        const int64 Magnitude = FMath::Abs(Local);
        return FString::Printf(TEXT("%s%lld%c%02lld %s"), Local < 0 ? TEXT("-") : TEXT(""), Magnitude / 100, Country->DecimalMark, Magnitude % 100, *Country->CurrencySymbol);
    }
    bool WriteReport(const FReport& Report, const FString& Directory)
    {
        IFileManager::Get().MakeDirectory(*Directory, true);
        FString Text = TEXT("# Otomatik oyuncunun denge raporu\n\n");
        Text += FString::Printf(TEXT("%d kosu, her biri %d gun; toplam sure %.1f saniye.\n\n"), Report.Runs.Num(), Report.Options.Days, Report.Seconds);
        Text += TEXT("Tune: ") + (Report.Tuning.IsEmpty() ? TEXT("C10 defaults") : Report.Tuning) + TEXT("\n\n");
        Text += TEXT("| Tarz | Tohum | Son kasa | Borc | Magaza | Il | Ulusal pay | Kasa eksi gun | Sikinti gunu | Denetim hatasi |\n|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|\n");
        int64 LowestPositive = MAX_int64, Highest = 0;
        int32 Broke = 0, Expanded = 0, Failures = 0;
        for (const FRun& Trial : Report.Runs)
        {
            const FRow& Last = Trial.Daily.Last();
            Broke += Trial.NegativeDays > 0 ? 1 : 0; Expanded += Last.Stores > 1 ? 1 : 0; Failures += Trial.AuditFailures;
            Text += FString::Printf(TEXT("| %s | %d | %s | %s | %d | %d | %.4f%% | %d | %d | %d |\n"), *Trial.Profile, Trial.Seed, *ReportMoney(Report, Last.Cash), *ReportMoney(Report, Last.Debt), Last.Stores, Last.Provinces, Last.Share, Trial.NegativeDays, Trial.TroubleDays, Trial.AuditFailures);
            if (Last.Cash > 0) LowestPositive = FMath::Min(LowestPositive, Last.Cash);
            Highest = FMath::Max(Highest, Last.Cash);
        }
        Text += TEXT("\n## Bulgular ve neye bakmali\n\n");
        Text += FString::Printf(TEXT("- %d/%d kosuda kasa eksiye dustu.\n- %d/%d kosu sonunda birden cok magaza acik kaldi.\n- Satis, siparis, stok ve sayi denetimi: %d hata.\n\n"), Broke, Report.Runs.Num(), Expanded, Report.Runs.Num(), Failures);
        if (LowestPositive < MAX_int64 && Highest / FMath::Max<int64>(1, LowestPositive) >= 4)
            Text += TEXT("- Istismar suphesi: son kasalar arasinda en az 4 kat fark var; asagidaki giderleri, magaza sayisini ve arka plan netini karsilastir. Bu fark tek basina hile kaniti degildir.\n");
        for (const FRun& Trial : Report.Runs)
        {
            Text += FString::Printf(TEXT("\n### %s / tohum %d\n\n"), *Trial.Profile, Trial.Seed);
            for (const TCHAR* Name : { TEXT("Ilk sube"), TEXT("5 magaza"), TEXT("Ilk depo"), TEXT("Ilk il muduru"), TEXT("5 il"), TEXT("Ikinci ulke") })
            {
                const int32* Day = Trial.Milestones.Find(Name);
                Text += Day ? FString::Printf(TEXT("- %s: %d. gun.\n"), Name, *Day) : FString::Printf(TEXT("- %s: bu kosuda ulasilmadi.\n"), Name);
            }
            TArray<FString> Names; Trial.Expenses.GetKeys(Names);
            Names.Sort([&](const FString& Left, const FString& Right) { const int64 A = Trial.Expenses[Left], B = Trial.Expenses[Right]; return A == B ? Left < Right : A > B; });
            Text += TEXT("\nEn buyuk 5 yuk (nakit gideri, stok yatirimi ve fire ayri anlam tasir):\n\n");
            for (int32 Index = 0; Index < FMath::Min(5, Names.Num()); ++Index) Text += FString::Printf(TEXT("- %s: %s.\n"), *Names[Index], *ReportMoney(Report, Trial.Expenses[Names[Index]]));
            Text += FString::Printf(TEXT("\nArka planin kasaya toplam net etkisi: %s. Reddedilen komut: %d.\n"), *ReportMoney(Report, Trial.BackgroundNet), Trial.RejectedDecisions);
            Text += TEXT("\nNet sonucun kaynagi (kasaya girisle ayni degildir):\n\n");
            TArray<FString> ResultNames; Trial.Results.GetKeys(ResultNames); ResultNames.Sort();
            for (const FString& Name : ResultNames) Text += FString::Printf(TEXT("- %s: %s.\n"), *Name, *ReportMoney(Report, Trial.Results[Name]));
            Text += MarketAutoPlayC::Report(Trial.C);
            Text += MarketAutoPlayFinance::Report(Trial.Finance);
            Text += MarketAutoPlayOnline::Report(Trial.Online);
            Text += MarketAutoPlayCommand::Report(Trial.CommandStats);
            Text += MarketAutoPlayRescue::Report(Trial.RescueStats);
            for (const FString& Problem : Trial.Issues) Text += TEXT("- Kontrol: ") + Problem + TEXT("\n");
        }
        Text += TEXT("\n## Denetimin kapsami\n\nSatis fisi, siparis bedeli, gun kapanisi ve mal kabul aktarimi bagimsiz hesapla kontrol edilir. Negatif stok, gecersiz sayilar ve pay sinirlari her gun denetlenir. Bagli muhasebe defterinin kasa farki hem isaretli hem mutlak toplamla C bolumunde verilir. Kasa eksisi oyun sonu degildir. Ligler yillik, subeler ilk 180 gunun gercek defter satirlariyla olculur.\n\nFiyatlar normal oyuncunun kullandigi adimlarla degisir. Kredi, sube, depo, yonetici ve kararlar normal komutlardan gecer. Aile dukkani PlayDay ile oynar; test modu, bedava mal veya para kullanilmaz. CSV tutarlari kurustur.\n");
        for (const FString& Error : Report.Errors) Text += TEXT("- Hata: ") + Error + TEXT("\n");
        struct FTranslation { const TCHAR* From; const TCHAR* To; };
        const FTranslation Translations[] = {
            {TEXT("kosu"), TEXT("ko\u015fu")}, {TEXT("gun"), TEXT("g\u00fcn")},
            {TEXT("magaza"), TEXT("ma\u011faza")}, {TEXT("sube"), TEXT("\u015fube")},
            {TEXT("Ilk"), TEXT("\u0130lk")}, {TEXT("| Il |"), TEXT("| \u0130l |")},
            {TEXT("Borc"), TEXT("Bor\u00e7")}, {TEXT("Sikinti"), TEXT("S\u0131k\u0131nt\u0131")},
            {TEXT("dukkani"), TEXT("d\u00fckk\u00e2n\u0131")}, {TEXT("dukkaninin"), TEXT("d\u00fckk\u00e2n\u0131n\u0131n")},
            {TEXT("ucret"), TEXT("\u00fccret")}, {TEXT("alim"), TEXT("al\u0131m")},
            {TEXT("yatirim"), TEXT("yat\u0131r\u0131m")}, {TEXT("Isletme"), TEXT("\u0130\u015fletme")},
            {TEXT("haric"), TEXT("hari\u00e7")}, {TEXT("odem"), TEXT("\u00f6dem")},
            {TEXT("cikis"), TEXT("\u00e7\u0131k\u0131\u015f")}, {TEXT("Ust"), TEXT("\u00dcst")},
            {TEXT("yonetim"), TEXT("y\u00f6netim")}, {TEXT("mudur"), TEXT("m\u00fcd\u00fcr")},
            {TEXT("ulasilmadi"), TEXT("ula\u015f\u0131lmad\u0131")}, {TEXT("ulasti"), TEXT("ula\u015ft\u0131")},
            {TEXT("buyuk"), TEXT("b\u00fcy\u00fck")}, {TEXT("kaynak"), TEXT("kaynak")},
            {TEXT("satis"), TEXT("sat\u0131\u015f")}, {TEXT("Satis"), TEXT("Sat\u0131\u015f")},
            {TEXT("Diger"), TEXT("Di\u011fer")}, {TEXT("Istismar"), TEXT("\u0130stismar")},
            {TEXT("suphesi"), TEXT("\u015f\u00fcphesi")}, {TEXT("kapsami"), TEXT("kapsam\u0131")},
            {TEXT("neye bakmali"), TEXT("neye bakmal\u0131")}, {TEXT("dust u"), TEXT("d\u00fc\u015ft\u00fc")},
            {TEXT("dustu"), TEXT("d\u00fc\u015ft\u00fc")}, {TEXT("kasa farki"), TEXT("kasa fark\u0131")}
        };
        for (const FTranslation& Translation : Translations) Text.ReplaceInline(Translation.From, Translation.To, ESearchCase::CaseSensitive);
        Text.ReplaceInline(TEXT("\u015fube180.csv"),TEXT("sube180.csv"),ESearchCase::CaseSensitive);
        FString AdsCsv=TEXT("tarz,tohum,takvim_yili,ulke,gun,kanal,harcama_kurus,tahmini_magaza_ek_ciro_kurus\n");
        FString DiagnosisCsv=TEXT("tarz,tohum,")+MarketAutoPlayDiagnosis::Header();
        FString DiagnosisEvents=TEXT("tarz,tohum,gun,tur,deger,aciklama\n");
        FString DiagnosisProducts=TEXT("tarz,tohum,")+MarketAutoPlayDiagnosis::ProductHeader();
        for(const auto& Trial:Report.Runs)
        {
            const FString Prefix=FString::Printf(TEXT("%s,%d,"),*Trial.Profile,Trial.Seed);
            for(const auto& Line:Trial.Diagnosis.Days)DiagnosisCsv+=Prefix+Line;
            for(const auto& Line:Trial.Diagnosis.Events)DiagnosisEvents+=Prefix+Line;
            for(const auto& Line:Trial.Diagnosis.Products)DiagnosisProducts+=Prefix+Line;
        }
        if(!FFileHelper::SaveStringToFile(DiagnosisCsv,*(Directory/TEXT("aile_gunluk.csv")),FFileHelper::EEncodingOptions::ForceUTF8) ||
            !FFileHelper::SaveStringToFile(DiagnosisEvents,*(Directory/TEXT("aile_kararlar.csv")),FFileHelper::EEncodingOptions::ForceUTF8) ||
            !FFileHelper::SaveStringToFile(DiagnosisProducts,*(Directory/TEXT("aile_urunler.csv")),FFileHelper::EEncodingOptions::ForceUTF8))return false;
        FString CommandsCsv=TEXT("tarz,tohum,gun,tur,id,deger\n");
        FString RescueCsv=TEXT("tarz,tohum,gun,engelin_sonu,magaza,ilk_yeni_sube,odendi_gun,yeni_plan_gun,anapara_kurus,kalan_kurus,silinen_kurus\n");
        FString RescueYears=TEXT("tarz,tohum,yil,gun,toplam_borc_kurus,aile_ciro_kurus\n");
        for(const auto& Trial:Report.Runs){RescueCsv+=MarketAutoPlayRescue::PlansCsv(Trial.RescueStats,Trial.Profile,Trial.Seed);RescueYears+=MarketAutoPlayRescue::YearsCsv(Trial.RescueStats,Trial.Profile,Trial.Seed);}
        if(!FFileHelper::SaveStringToFile(RescueCsv,*(Directory/TEXT("kurtarma.csv")),FFileHelper::EEncodingOptions::ForceUTF8) || !FFileHelper::SaveStringToFile(RescueYears,*(Directory/TEXT("kurtarma_yillar.csv")),FFileHelper::EEncodingOptions::ForceUTF8))return false;
        FString WeatherCsv=TEXT("tarz,tohum,gun,hava,sicaklik_c,yasal_kapali,tatil\n");
        for(const auto& Trial:Report.Runs)
        {AdsCsv+=MarketAutoPlayCommand::YearsCsv(Trial.CommandStats,Trial.Profile,Trial.Seed);CommandsCsv+=MarketAutoPlayCommand::EventsCsv(Trial.CommandStats,Trial.Profile,Trial.Seed);WeatherCsv+=MarketAutoPlayCommand::WeatherCsv(Trial.CommandStats,Trial.Profile,Trial.Seed);}
        if(!FFileHelper::SaveStringToFile(AdsCsv,*(Directory/TEXT("reklam.csv")),FFileHelper::EEncodingOptions::ForceUTF8) || !FFileHelper::SaveStringToFile(CommandsCsv,*(Directory/TEXT("komuta_olaylar.csv")),FFileHelper::EEncodingOptions::ForceUTF8) || !FFileHelper::SaveStringToFile(WeatherCsv,*(Directory/TEXT("ilk_yil_hava.csv")),FFileHelper::EEncodingOptions::ForceUTF8))return false;
        FString OnlineCsv=TEXT("tarz,tohum,yil,gun,oynanan_gun,kanal,siparis,ciro_kurus,katki_kar_kurus,siparis_basi_katki_tl,ortak_gider_kurus,toplam_net_kurus,magaza_ciro_kurus,ulke_internet_payi,karanlik_depo,salgin_gun,salgin_siparis,salgin_musteri,trafik_carpani\n");
        FString OnlineEvents=TEXT("tarz,tohum,gun,tur,id,secim,arg\n");
        for(const auto& Trial:Report.Runs){OnlineCsv+=MarketAutoPlayOnline::YearsCsv(Trial.Online,Trial.Profile,Trial.Seed);OnlineEvents+=MarketAutoPlayOnline::EventsCsv(Trial.Online,Trial.Profile,Trial.Seed);}
        if(!FFileHelper::SaveStringToFile(OnlineCsv,*(Directory/TEXT("internet.csv")),FFileHelper::EEncodingOptions::ForceUTF8) || !FFileHelper::SaveStringToFile(OnlineEvents,*(Directory/TEXT("internet_olaylar.csv")),FFileHelper::EEncodingOptions::ForceUTF8))return false;
        FString DeptCsv=TEXT("tarz,tohum,gun,magaza_turu,reyon,sube,kar30_kurus,ciro30_kurus\n");
        for(const auto& Trial:Report.Runs)DeptCsv+=MarketAutoPlayC::DeptCsv(Trial.C,Trial.Profile,Trial.Seed);
        FString LeagueCsv=TEXT("tarz,tohum,yil,gun,ulusal_sira,dunya_sira,magaza,bizim_ortak_ciro,lider_ortak_ciro\n");
        FString SupplyCsv=TEXT("tarz,tohum,gun,hat,once,sonra\n");
        FString EraCsv=TEXT("tarz,tohum,donem,dalga,baslangic,bitis,oynanan_gun,ilk_kasa_kurus,son_kasa_kurus,en_az_kasa_kurus,net_kar_kurus\n");
        FString C3Csv=TEXT("tarz,tohum,fark_gun,fark_kurus,mutlak_fark_kurus,hedef,tamamlanan,kutlama,sakin_olay,ertelenen_kotu,sikici_donem,en_uzun_sessizlik,kapanma,rakip_alimi,bizim_alim\n");
        FString BranchCsv=TEXT("tarz,tohum,sube,tur,acilis,oynanan_gun,kapanis,ciro_kurus,brut_kar_kurus,kira_kurus,ucret_kurus,sgk_kurus,isletme_kurus,lojistik_kurus,fire_kurus,net_kar_kurus\n");
        FString MatureCsv=TEXT("tarz,tohum,gun,olgun_sube,kar30_kurus,liste_duzeyi\n");
        FString BankCsv=TEXT("tarz,tohum,yil,gun,not,sirket_borcu_kurus,favok_kurus,borc_favok,faiz_kurus,limit_kullanimi_kurus,limit_kurus,bagli_sirket,bagli_magaza\n");
        FString C4Csv=TEXT("tarz,tohum,kurtarma,kapanan_sube,ihlal_ay,teklif,kabul,ret,finansmanli_alim,cevrilen,dev_cikisi,kapi,faiz_kurus,en_cok_limit_kurus\n");
        for(const auto& Trial:Report.Runs)
        {
            const auto& Metrics=Trial.C;
            BranchCsv+=MarketAutoPlayFinance::BranchCsv(Trial.Finance,Trial.Profile,Trial.Seed);
            MatureCsv+=MarketAutoPlayFinance::MatureCsv(Trial.Finance,Trial.Profile,Trial.Seed);
            BankCsv+=MarketAutoPlayFinance::BankCsv(Trial.Finance,Trial.Profile,Trial.Seed);
            C4Csv+=MarketAutoPlayFinance::SummaryCsv(Trial.Finance,Trial.Profile,Trial.Seed);
            EraCsv+=MarketAutoPlayC::EraCsv(Metrics,Trial.Profile,Trial.Seed);
            C3Csv+=FString::Printf(TEXT("%s,%d,%d,%lld,%lld,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n"),*Trial.Profile,Trial.Seed,Metrics.GapDays,Metrics.GapTotal,Metrics.GapAbsolute,Metrics.GoalsSeen,Metrics.GoalsCompleted,Metrics.Celebrations,Metrics.QuietEvents,Metrics.HeldBadEvents,Metrics.RhythmBoring,Metrics.RhythmLongest,Metrics.Closures,Metrics.Takeovers,Metrics.OurBuys);
            for(const auto& Year:Trial.C.Years)LeagueCsv+=FString::Printf(TEXT("%s,%d,%d,%d,%d,%d,%d,%.0f,%.0f\n"),*Trial.Profile,Trial.Seed,Year.Year,Year.Day,Year.National,Year.World,Year.Stores,Year.OurWorld,Year.LeaderWorld);
            for(const auto& Change:Trial.C.Sourcing)SupplyCsv+=FString::Printf(TEXT("%s,%d,%d,%d,%d,%d\n"),*Trial.Profile,Trial.Seed,Change.Day,Change.Line,Change.From,Change.To);
        }
        if(!FFileHelper::SaveStringToFile(MatureCsv,*(Directory/TEXT("olgun_sube.csv")),FFileHelper::EEncodingOptions::ForceUTF8) ||
            !FFileHelper::SaveStringToFile(BranchCsv,*(Directory/TEXT("sube180.csv")),FFileHelper::EEncodingOptions::ForceUTF8) ||
            !FFileHelper::SaveStringToFile(BankCsv,*(Directory/TEXT("banka.csv")),FFileHelper::EEncodingOptions::ForceUTF8) ||
            !FFileHelper::SaveStringToFile(C4Csv,*(Directory/TEXT("c4.csv")),FFileHelper::EEncodingOptions::ForceUTF8) ||
            !FFileHelper::SaveStringToFile(EraCsv,*(Directory/TEXT("donemler.csv")),FFileHelper::EEncodingOptions::ForceUTF8) ||
            !FFileHelper::SaveStringToFile(C3Csv,*(Directory/TEXT("c3.csv")),FFileHelper::EEncodingOptions::ForceUTF8) ||
            !FFileHelper::SaveStringToFile(LeagueCsv,*(Directory/TEXT("lig.csv")),FFileHelper::EEncodingOptions::ForceUTF8) ||
            !FFileHelper::SaveStringToFile(SupplyCsv,*(Directory/TEXT("tedarik.csv")),FFileHelper::EEncodingOptions::ForceUTF8))return false;
        return FFileHelper::SaveStringToFile(DeptCsv,*(Directory/TEXT("reyonlar.csv")),FFileHelper::EEncodingOptions::ForceUTF8) &&
            FFileHelper::SaveStringToFile(Text, *(Directory / TEXT("rapor.md")), FFileHelper::EEncodingOptions::ForceUTF8) &&
            FFileHelper::SaveStringToFile(Csv(Report.Runs, false), *(Directory / TEXT("gunluk.csv")), FFileHelper::EEncodingOptions::ForceUTF8) &&
            FFileHelper::SaveStringToFile(Csv(Report.Runs, true), *(Directory / TEXT("haftalik.csv")), FFileHelper::EEncodingOptions::ForceUTF8);
    }
}

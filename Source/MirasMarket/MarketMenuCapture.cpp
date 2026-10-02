#include "MarketGame.h"
#include "MarketAutoPlay.h"
#include "MarketBranchVisit.h"
#include "MarketDirector.h"
#include "MarketStart.h"
#include "MarketCountry.h"
#include "MarketCalendar.h"
#include "MarketOnline.h"
#include "MarketBranches.h"
#include "MarketStoreViews.h"
#include "MarketManagers.h"
#include "MarketMenuWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SOverlay.h"
#include "Styling/CoreStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
namespace MarketMenuCapture
{
    struct FTarget { int32 Page; const TCHAR* Id; const TCHAR* Tab; bool bBottom=false; const TCHAR* Anchor=TEXT(""); float Offset=0.f; };
    const TArray<FTarget>& Targets()
    {
        if(FParse::Param(FCommandLine::Get(),TEXT("MirasMenuC9")))
        {
            static const TArray<FTarget> C9List={
                {9,TEXT("reports_week"),TEXT("Hafta")},
                {3,TEXT("promotions"),TEXT("")},
                {8,TEXT("management"),TEXT("Y\u00f6netim")},
                {8,TEXT("company_advertising"),TEXT("\u015eirket"),false,TEXT("REKLAM")}
            };return C9List;
        }
        if(FParse::Param(FCommandLine::Get(),TEXT("MirasMenuC6")))
        {
            static const TArray<FTarget> C6List={
                {8,TEXT("company_advertising"),TEXT("\u015eirket"),false,TEXT("REKLAM")},
                {8,TEXT("company_advertising_manager"),TEXT("\u015eirket"),false,TEXT("REKLAM"),300.f},
                {7,TEXT("channels_advertising"),TEXT(""),false,TEXT("POL\u0130T\u0130KA")},
                {9,TEXT("branch_clearance"),TEXT(""),true}
            };return C6List;
        }
        static const TArray<FTarget> List = {
            {0,TEXT("map_shops"),TEXT("")}, {0,TEXT("map_rivals"),TEXT("Rakipler")}, {0,TEXT("map_opportunities"),TEXT("F\u0131rsatlar")},
            {1,TEXT("orders"),TEXT("")}, {2,TEXT("products"),TEXT("")}, {3,TEXT("promotions"),TEXT("")},
            {4,TEXT("rivals_local"),TEXT("")}, {4,TEXT("rivals_national"),TEXT("Ulusal")}, {4,TEXT("rivals_world"),TEXT("Uluslararas\u0131")},
            {5,TEXT("staff"),TEXT("")}, {6,TEXT("finance"),TEXT("")},
            {6,TEXT("finance_banking"),TEXT(""),false,TEXT("\u015e\u0130RKET F\u0130NANSI")},
            {6,TEXT("finance_day"),TEXT("D\u00fcn"),true}, {6,TEXT("finance_week"),TEXT("Bu hafta"),true},
            {6,TEXT("finance_month"),TEXT("Bu ay"),true}, {6,TEXT("finance_year"),TEXT("Bu y\u0131l"),true}, {7,TEXT("channels"),TEXT("")},
            {7,TEXT("channels_areas"),TEXT(""),false,TEXT("\u0130LLER")},
            {7,TEXT("channels_policy"),TEXT(""),false,TEXT("POL\u0130T\u0130KA")},
            {7,TEXT("channels_stats"),TEXT(""),false,TEXT("KANALLARIN GE\u00c7EN AYI")},
            {8,TEXT("shops"),TEXT("")}, {8,TEXT("management"),TEXT("Y\u00f6netim")}, {8,TEXT("company"),TEXT("\u015eirket")},
            {9,TEXT("reports_day"),TEXT("")}, {9,TEXT("reports_week"),TEXT("Hafta")}, {9,TEXT("reports_records"),TEXT("Rekorlar")}
        }; return List;
    }
    FString TextOf(const TSharedRef<SWidget>& Widget)
    {
        if (Widget->GetType() == TEXT("STextBlock")) return StaticCastSharedRef<STextBlock>(Widget)->GetText().ToString();
        FString Text; FChildren* Children = Widget->GetChildren();
        for(int32 Index=0;Index<Children->Num();++Index) Text += TextOf(Children->GetChildAt(Index));
        return Text;
    }
    bool Click(const TSharedRef<SWidget>& Widget, const FString& Text)
    {
        if(!Widget->GetVisibility().IsVisible()) return false;
        if(Widget->GetType() == TEXT("SButton") && Widget->IsEnabled() && TextOf(Widget) == Text)
        { StaticCastSharedRef<SButton>(Widget)->SimulateClick(); return true; }
        FChildren* Children = Widget->GetChildren();
        for(int32 Index=0;Index<Children->Num();++Index) if(Click(Children->GetChildAt(Index),Text))return true;
        return false;
    }
    void ScrollBottom(const TSharedRef<SWidget>& Widget)
    {
        if(!Widget->GetVisibility().IsVisible())return;
        if(Widget->GetType()==TEXT("SScrollBox"))StaticCastSharedRef<SScrollBox>(Widget)->ScrollToEnd();
        FChildren* Children=Widget->GetChildren();
        for(int32 Index=0;Index<Children->Num();++Index)ScrollBottom(Children->GetChildAt(Index));
    }
    TSharedPtr<SWidget> FindHeading(const TSharedRef<SWidget>& Widget,const FString& Heading)
    {
        if(!Widget->GetVisibility().IsVisible())return nullptr;
        if(Widget->GetType()==TEXT("STextBlock") && TextOf(Widget)==Heading)return Widget;
        FChildren* Children=Widget->GetChildren();
        for(int32 Index=0;Index<Children->Num();++Index)
            if(auto Found=FindHeading(Children->GetChildAt(Index),Heading))return Found;
        return nullptr;
    }
    void ScrollHeading(const TSharedRef<SWidget>& Widget,const FString& Heading)
    {
        if(!Widget->GetVisibility().IsVisible())return;
        if(Widget->GetType()==TEXT("SScrollBox"))
            if(auto Found=FindHeading(Widget,Heading))
                StaticCastSharedRef<SScrollBox>(Widget)->ScrollDescendantIntoView(Found,false,EDescendantScrollDestination::TopOrLeft);
        FChildren* Children=Widget->GetChildren();
        for(int32 Index=0;Index<Children->Num();++Index)ScrollHeading(Children->GetChildAt(Index),Heading);
    }
    void ScrollExtra(const TSharedRef<SWidget>& Widget,const FString& Heading,float Offset)
    {
        if(!Widget->GetVisibility().IsVisible())return;
        if(Widget->GetType()==TEXT("SScrollBox") && FindHeading(Widget,Heading).IsValid())
        {auto Scroll=StaticCastSharedRef<SScrollBox>(Widget);Scroll->SetScrollOffset(Scroll->GetScrollOffset()+Offset);return;}
        FChildren* Children=Widget->GetChildren();for(int32 Index=0;Index<Children->Num();++Index)ScrollExtra(Children->GetChildAt(Index),Heading,Offset);
    }
    struct FCapture
    {
        int32 Step = 0, Index = 0;
        double At = 0;
        FString Directory, File, Phase, Description;
        TSharedPtr<SMarketMenu> Widget;
        TSharedPtr<SWidget> Caption;
        TArray<uint8> Before;
        bool bNetworkFixture = false;
    };
    FCapture Capture;
    // Review-only rows are visibly identified and never written to campaign slots. The genuine bot
    // history, cash and trading records stay intact. This fixture exposes otherwise locked network UI.
    void AddNetworkFixture(FMarketState& State, const TArray<FMarketProduct>& Products)
    {
        const FString Provinces[]={TEXT("kirklareli"),TEXT("tekirdag"),TEXT("edirne"),TEXT("istanbul")};
        for(int32 Index=0;Index<4;++Index)
        {
            FMarketBranch Branch; Branch.Country=State.CountryId; Branch.Province=Provinces[Index]; Branch.Format=Index%2?TEXT("kucuk"):TEXT("mahalle");
            Branch.Name=FString::Printf(TEXT("Ornek %s %d"),*Branch.Province,Index+1); Branch.Stage=static_cast<uint8>(MarketBranches::EStage::Open); Branch.OpenedDay=State.Day-120;
            Branch.Workers=3+Index; Branch.ManagerName=Index%2?TEXT("Cem"):TEXT("Ayse"); Branch.ManagerSkill=60; Branch.ManagerHonesty=70; Branch.ManagerWage=80000;
            Branch.ManagerMorale=60; Branch.ManagerPotential=75; Branch.Maturity=1.f; Branch.Satisfaction=Index==3?35:75; Branch.PriceIndex=1.f;
            Branch.Rent=120000; Branch.LastShoppers=80+Index*30; Branch.LastQueueLost=Index==3?15:0; Branch.LastRevenue=50000+Index*10000; Branch.LastProfit=Index==3?-6000:15000;
            Branch.Last30Revenue=Branch.LastRevenue*30; Branch.Last30Profit=Branch.LastProfit*30;
            for(const auto& Product:Products) { FMarketBranchItem Item; Item.ProductId=Product.Id; Item.Capacity=24; Item.Units=Index==3?6:18; Item.LastSold=2; Item.LastEmpty=Index==3?4:0; Branch.Items.Add(Item); }
            MarketStoreViews::AssignTo(State,Branch); State.Branches.Add(Branch);
        }
        FMarketDepot Depot; Depot.Country=State.CountryId; Depot.Province=TEXT("kirklareli"); Depot.OpenedDay=State.Day-90; Depot.Rent=600000; State.Company.DepotSites.Add(Depot);
        const auto AddManager=[&](MarketManagers::ELevel Level,const TCHAR* Area,const TCHAR* Name)
        { FMarketManager Manager; Manager.Level=static_cast<uint8>(Level); Manager.Country=State.CountryId; Manager.Area=Area; Manager.Name=Name; Manager.Skill=65; Manager.Potential=80; Manager.Honesty=70; Manager.Morale=65; Manager.BaseWage=5000; Manager.AppointedDay=State.Day-60; State.Management.Managers.Add(Manager); };
        AddManager(MarketManagers::ELevel::Province,TEXT("kirklareli"),TEXT("Selim")); AddManager(MarketManagers::ELevel::Depot,TEXT("kirklareli"),TEXT("Deniz"));
        State.Company.Trucks=1; State.bSecondStore=true;
    }
    bool WriteIndex(const FCapture& Run)
    {
        FString Html=TEXT("<!doctype html><meta charset='utf-8'><title>Miras Market menu incelemesi</title><style>body{font:16px system-ui;background:#eae8e2;margin:24px;color:#182b2a}section{margin:30px 0}div{display:grid;grid-template-columns:1fr 1fr;gap:12px}figure{margin:0;background:white;padding:10px}img{width:100%;display:block}h2{font-size:20px}</style><h1>Miras Market: menu goruntuleri</h1><p>3 yil otomatik oyuncu; kampanya kaydi yazilmaz. Her sayfa ve sekme, iki tema ve iki boyut.</p>");
        if(Run.bNetworkFixture)Html+=TEXT("<p><strong>Bot bu surede sube acamadi. Ag ekranlarini incelemek icin yalniz goruntu kopyasina 4 ornek sube, 2 mudur ve 1 depo eklendi; bunlar botun kazandigi gelisme degildir.</strong></p>");
        Html.ReplaceInline(TEXT("3 yil otomatik oyuncu"),*Run.Description);
        FString Manifest=TEXT("page,tab,theme,width,height,file\n");
        for(int32 Target=0;Target<Targets().Num();++Target)
        {
            Html+=FString::Printf(TEXT("<section><h2>%s / %s</h2><div>"),SMarketMenu::PageName(Targets()[Target].Page),Targets()[Target].Id);
            for(int32 Variant=0;Variant<4;++Variant)
            {
                const bool Light=Variant%2==0; const int32 Width=Variant<2?1920:1280, Height=Variant<2?1080:720;
                const FString File=FString::Printf(TEXT("%02d_%s_%s_%dx%d.png"),Target,Targets()[Target].Id,Light?TEXT("light"):TEXT("dark"),Width,Height);
                if(IFileManager::Get().FileSize(*(Run.Directory/File))<=0)return false;
                Html+=FString::Printf(TEXT("<figure><figcaption>%s %dx%d</figcaption><a href='%s'><img loading='lazy' src='%s'></a></figure>"),Light?TEXT("Acik"):TEXT("Koyu"),Width,Height,*File,*File);
                Manifest+=FString::Printf(TEXT("%d,%s,%s,%d,%d,%s\n"),Targets()[Target].Page,Targets()[Target].Id,Light?TEXT("light"):TEXT("dark"),Width,Height,*File);
            }
            Html+=TEXT("</div></section>");
        }
        return FFileHelper::SaveStringToFile(Html,*(Run.Directory/TEXT("index.html")),FFileHelper::EEncodingOptions::ForceUTF8) && FFileHelper::SaveStringToFile(Manifest,*(Run.Directory/TEXT("manifest.csv")),FFileHelper::EEncodingOptions::ForceUTF8);
    }
}
bool AMarketGameMode::TickMenuCapture()
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("MirasMenuCapture")))return true;
    auto& R=MarketMenuCapture::Capture; const double Now=FPlatformTime::Seconds();
    const auto Fail=[](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("MirasMenuCapture FAILED: %s"),Why);FPlatformMisc::RequestExitWithStatus(false,1);return false;};
    if(GetWorld()->GetTimeSeconds()<4)return false;
    auto* PC=GetWorld()->GetFirstPlayerController(); auto* Hud=PC?Cast<AMarketHUD>(PC->GetHUD()):nullptr;
    if(!Hud||!Hud->MenuWidget().IsValid()||!GEngine||!GEngine->GameViewport)return false;
    if(R.Step==0)
    {
        TArray<FMarketProduct> Base; TArray<int32> Capacities; TArray<FString> Errors;
        if(!MarketAutoPlay::LoadInputs(Base,Capacities,Errors))return Fail(TEXT("catalog/plan"));
        MarketAutoPlay::FOptions Options; Options.Days=MarketCalendar::GameDayOf(MarketCalendar::StartYear+3,MarketCalendar::StartMonth,MarketCalendar::StartDayOfMonth)-1; Options.Seeds=1; Options.bKeepFinalStates=true; Options.StyleIndex=2;
        R.Phase=TEXT("three_year");FParse::Value(FCommandLine::Get(),TEXT("MirasMenuPhase="),R.Phase);
        if(R.Phase!=TEXT("three_year") && R.Phase!=TEXT("start") && R.Phase!=TEXT("before") && R.Phase!=TEXT("after"))return Fail(TEXT("unknown menu phase"));
        Options.FirstSeed=22;FParse::Value(FCommandLine::Get(),TEXT("MirasMenuSeed="),Options.FirstSeed);
        FMarketState Timeline;Timeline.Initialize(Base);MarketStart::Setup(Timeline,Options.Country,Options.Province,Options.FirstSeed);
        const int32 Anchor=MarketOnline::PandemicStart(Timeline);
        if(R.Phase==TEXT("start"))Options.Days=1;
        if(R.Phase==TEXT("before"))Options.Days=FMath::Max(1,Anchor-365-1);
        if(R.Phase==TEXT("after"))Options.Days=FMath::Max(1,Anchor+1095-1);
        FParse::Value(FCommandLine::Get(),TEXT("MirasMenuDays="),Options.Days);
        R.Description=FString::Printf(TEXT("%s | gercek atak bot, tohum %d, hedef gun %d, salgin %d | oyuncu kaydina yazilmaz"),*R.Phase,Options.FirstSeed,R.Phase==TEXT("start")?1:Options.Days+1,Anchor);
        MarketAutoPlay::FReport Report;
        if(R.Phase==TEXT("start"))
        {Timeline.ApplyShelfCapacities(Capacities);MarketStart::StockShelvesPartly(Timeline,Options.FirstSeed);Report.FinalStates.Add(Timeline);}
        else Report=MarketAutoPlay::Run(Options,Base,Capacities,State.RivalSeed);
        if(Report.FinalStates.Num()!=1 || Report.Errors.Num()!=0 || (!Report.Runs.IsEmpty() && Report.Runs[0].AuditFailures!=0))return Fail(TEXT("campaign review"));
        State=MoveTemp(Report.FinalStates.Last()); CatalogBase=Base; Products=Base; MarketCountry::SetActive(State.CountryId,State.RivalSeed); MarketEras::Activate(State); MarketDirector::ApplyPrices(State,Base,Products);
        // C5 captures genuine progress only; no review-only shops or money are injected.
        Message.Empty(); MessageTime=0.f; ReportTime=0.f; SyncWorkers();
        R.Before=MarketBranchVisit::StateBytes(State);
        bNeedStart=false; bOpen=false; bTestMode=false; bMenuOpen=true; bPauseInMenu=false; bTimePaused=false;
        GEngine->GameViewport->RemoveViewportWidgetContent(Hud->MenuWidget().ToSharedRef());
        R.Directory=FPaths::ProjectSavedDir()/TEXT("Screenshots/Menu")/FDateTime::Now().ToString(TEXT("%Y%m%d-%H%M%S"))+TEXT("_")+R.Phase; IFileManager::Get().MakeDirectory(*R.Directory,true);
        R.Caption = SNew(SOverlay).Visibility(EVisibility::HitTestInvisible) + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top)
            [ SNew(SBorder).Padding(4).BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"))).BorderBackgroundColor(FLinearColor(.02f,.03f,.03f,.94f))
                [ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),10)).ColorAndOpacity(FLinearColor::White)
                    .Text(FText::FromString(R.Description)) ] ];
        GEngine->GameViewport->AddViewportWidgetContent(R.Caption.ToSharedRef(),100);
        if(!Report.Runs.IsEmpty())MarketAutoPlay::WriteReport(Report,R.Directory/TEXT("bot"));
        UE_LOG(LogTemp,Display,TEXT("MirasMenuCapture campaign: day %d, branches %d, managers %d, depots %d, review-only network %d"),State.Day,State.Branches.Num(),State.Management.Managers.Num(),State.Company.DepotSites.Num(),R.bNetworkFixture);
        R.Step=1; R.At=Now; return false;
    }
    if(R.Index>=MarketMenuCapture::Targets().Num()*4)
    {
        if(R.Before!=MarketBranchVisit::StateBytes(State))return Fail(TEXT("page/tab navigation changed campaign"));
        if(!MarketMenuCapture::WriteIndex(R))return Fail(TEXT("missing PNG or index write"));
        GEngine->GameViewport->RemoveViewportWidgetContent(R.Widget.ToSharedRef()); R.Widget.Reset();
        GEngine->GameViewport->RemoveViewportWidgetContent(R.Caption.ToSharedRef()); R.Caption.Reset();
        UE_LOG(LogTemp,Display,TEXT("MirasMenuCapture PASSED: %d PNGs, campaign unchanged; %s"),R.Index,*R.Directory); FPlatformMisc::RequestExitWithStatus(false,0); return false;
    }
    const auto& Target=MarketMenuCapture::Targets()[R.Index/4]; const int32 Variant=R.Index%4;
    const int32 Width=Variant<2?1920:1280, Height=Variant<2?1080:720;
    if(R.Step==1)
    {
        bLightTheme=Variant%2==0; MenuPage=Target.Page;
        if(R.Widget.IsValid())GEngine->GameViewport->RemoveViewportWidgetContent(R.Widget.ToSharedRef());
        R.Widget=SNew(SMarketMenu).Game(this); GEngine->GameViewport->AddViewportWidgetContent(R.Widget.ToSharedRef(),50);
        FInputModeUIOnly Mode; Mode.SetWidgetToFocus(R.Widget); PC->SetInputMode(Mode); PC->bShowMouseCursor=false;
        GEngine->GameViewport->ConsoleCommand(FString::Printf(TEXT("r.SetRes %dx%dw"),Width,Height));
        R.Step=2; R.At=Now; return false;
    }
    if(Now-R.At<1.5)return false;
    if(R.Step==2)
    {
        if(GEngine->GameViewport->Viewport->GetSizeXY()!=FIntPoint(Width,Height)) { if(Now-R.At>15)return Fail(TEXT("viewport resolution"));return false; }
        if(Target.Tab[0] && !MarketMenuCapture::Click(R.Widget.ToSharedRef(),Target.Tab))return Fail(TEXT("tab button not found"));
        if(MenuPage!=Target.Page)return Fail(TEXT("tab changed wrong page"));
        if(Target.bBottom)MarketMenuCapture::ScrollBottom(R.Widget.ToSharedRef());
        if(Target.Anchor[0])MarketMenuCapture::ScrollHeading(R.Widget.ToSharedRef(),Target.Anchor);
        R.Step=3; R.At=Now; return false;
    }
    if(R.Step==3 && Target.Offset>0.f)
    {MarketMenuCapture::ScrollExtra(R.Widget.ToSharedRef(),Target.Anchor,Target.Offset);R.Step=5;R.At=Now;return false;}
    if(R.Step==3 || R.Step==5)
    {
        R.File=FString::Printf(TEXT("%02d_%s_%s_%dx%d.png"),R.Index/4,Target.Id,bLightTheme?TEXT("light"):TEXT("dark"),Width,Height);
        FScreenshotRequest::RequestScreenshot(R.Directory/R.File,true,false);
        R.Step=4; R.At=Now; return false;
    }
    if(IFileManager::Get().FileSize(*(R.Directory/R.File))<=0){if(Now-R.At>15)return Fail(TEXT("PNG not written"));return false;}
    UE_LOG(LogTemp,Display,TEXT("MirasMenuCapture captured %s"),*R.File); ++R.Index; R.Step=1; R.At=Now; return false;
}

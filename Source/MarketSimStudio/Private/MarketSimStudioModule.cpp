#include "Containers/Ticker.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Modules/ModuleManager.h"
#include "SProductStudio.h"
#include "SPlanogramStudio.h"
#include "SStoreStudio.h"
#include "MarketStoreEditing.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Widgets/SWindow.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

namespace
{
    const FName StudioTab(TEXT("SimProductStudio"));
    const FName PlanogramTab(TEXT("SimPlanogramStudio"));
    const FName StoreTab(TEXT("SimStoreStudio"));
}

class FMarketSimStudioModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        FGlobalTabmanager::Get()->RegisterNomadTabSpawner(StudioTab, FOnSpawnTab::CreateRaw(this, &FMarketSimStudioModule::SpawnTab))
            .SetDisplayName(FText::FromString(TEXT("\u00dcr\u00fcn St\u00fcdyosu")))
            .SetTooltipText(FText::FromString(TEXT("Kutu, etiket ve \u00fcr\u00fcn bilgisini oyuna ekle")))
            .SetMenuType(ETabSpawnerMenuType::Hidden);
        FGlobalTabmanager::Get()->RegisterNomadTabSpawner(PlanogramTab, FOnSpawnTab::CreateRaw(this, &FMarketSimStudioModule::SpawnPlanogramTab))
            .SetDisplayName(FText::FromString(TEXT("Raf Plan\u0131 Edit\u00f6r\u00fc")))
            .SetTooltipText(FText::FromString(TEXT("Gondol, marka, facing ve raf derinli\u011fini d\u00fczenle")))
            .SetMenuType(ETabSpawnerMenuType::Hidden);
        UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FMarketSimStudioModule::RegisterMenus));
        FGlobalTabmanager::Get()->RegisterNomadTabSpawner(StoreTab,FOnSpawnTab::CreateRaw(this,&FMarketSimStudioModule::SpawnStoreTab))
            .SetDisplayName(FText::FromString(TEXT("Ma\u011faza Edit\u00f6r\u00fc"))).SetMenuType(ETabSpawnerMenuType::Hidden);
    }

    virtual void ShutdownModule() override
    {
        UToolMenus::UnRegisterStartupCallback(this);
        UToolMenus::UnregisterOwner(this);
        if (FSlateApplication::IsInitialized())
        {
            FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(StudioTab);
            FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PlanogramTab);
            FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(StoreTab);
        }
    }

private:
    TSharedRef<SDockTab> SpawnStoreTab(const FSpawnTabArgs& Args)
    {
        return SNew(SDockTab).TabRole(ETabRole::NomadTab)[SNew(SStoreStudio)];
    }
    void OpenStore()
    {
        if(auto Tab=FGlobalTabmanager::Get()->TryInvokeTab(StoreTab))
            if(auto Window=FSlateApplication::Get().FindWidgetWindow(Tab.ToSharedRef()))Window->Maximize();
    }
    void ReviewStoreEditor()
    {
        auto Editor=SNew(SStoreStudio);
        auto Window=SNew(SWindow).Title(FText::FromString(TEXT("Ma\u011faza Edit\u00f6r\u00fc"))).ClientSize(FVector2D(1600,1000))[Editor];
        FSlateApplication::Get().AddWindow(Window);
        auto Step=MakeShared<int32>(0);
        FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Editor,Window,Step](float)
        {
            if(*Step==1)
            {
                Editor->Store=*MarketStoreKit::Find(TEXT("buyuk_01"));Editor->Store.Fixtures.Reset();Editor->Store.Obstacles.Reset();Editor->Store.Sections.Reset();
                FString Error;MarketStoreEditing::Resize(Editor->Store,4000,3000,2200,400,650,Error);
                Editor->Action(TEXT("color:Krem"));Editor->Action(TEXT("finish:tile"));Editor->Action(TEXT("arm:open_chiller_2500"));Editor->Category=TEXT("s\u00fct");Editor->AddAt(FVector(-1500,900,0));Editor->Action(TEXT("select"));Editor->Action(TEXT("duplicate"));
                const int32 Count=Editor->Store.Fixtures.Num();Editor->Action(TEXT("undo"));Editor->Action(TEXT("redo"));
                if(Count!=2||Editor->Store.Fixtures.Num()!=Count){UE_LOG(LogTemp,Error,TEXT("StoreEditor REVIEW failed: add / duplicate / undo"));FPlatformMisc::RequestExitWithStatus(false,1);return false;}
                Editor->Action(TEXT("department:Kasap"));Editor->Action(TEXT("department:Teknoloji"));Editor->Action(TEXT("department:Manav"));
                const auto Path=FPaths::ProjectSavedDir()/TEXT("Tests/StoreEditorReview.json");
                if(!MarketStoreEditing::Save(Editor->Store,Path,Error)){UE_LOG(LogTemp,Error,TEXT("StoreEditor REVIEW save failed: %s"),*Error);FPlatformMisc::RequestExitWithStatus(false,1);return false;}
            }
            if(*Step==2){FString Error;if(!Editor->ReviewMap(Error)){UE_LOG(LogTemp,Error,TEXT("StoreEditor REVIEW map failed: %s"),*Error);FPlatformMisc::RequestExitWithStatus(false,1);return false;}}
            if(*Step==0||*Step>=2)
            {
                TArray<FColor> Pixels;FIntVector Size;
                if(!FSlateApplication::Get().TakeScreenshot(Window,Pixels,Size)){UE_LOG(LogTemp,Error,TEXT("StoreEditor REVIEW screenshot failed"));FPlatformMisc::RequestExitWithStatus(false,1);return false;}
                TArray64<uint8> PNG;FImageUtils::PNGCompressImageArray(Size.X,Size.Y,MakeArrayView(Pixels),PNG);
                const FString Dir=FPaths::ProjectSavedDir()/TEXT("Screenshots/StoreEditor");IFileManager::Get().MakeDirectory(*Dir,true);FFileHelper::SaveArrayToFile(PNG,*(Dir/FString::Printf(TEXT("editor_%d.png"),*Step)));
            }

            if((*Step)++>=3)
            {
                auto NewEditor=SNew(SStoreStudio);bool Ok=NewEditor->CreateStore(TEXT("mahalle"),TEXT("Review empty store"));const FString First=NewEditor->Store.Id;
                if(Ok){NewEditor->Action(TEXT("arm:gondola_double_1200"));NewEditor->AddAt(FVector::ZeroVector);Ok=NewEditor->Store.Fixtures.Num()==1&&NewEditor->CreateStore(TEXT("mahalle"),TEXT("Review copied store"),true);}
                const FString Second=NewEditor->Store.Id;FStoreTemplate Draft;FString Error;Ok=Ok&&First!=Second&&NewEditor->Store.Fixtures.Num()==1&&MarketStoreEditing::LoadDraft(FPaths::ProjectSavedDir()/TEXT("StoreDrafts")/(Second+TEXT(".json")),Draft,Error)&&Draft.Id==Second;
                for(const auto& Id:{First,Second})if(Id.StartsWith(TEXT("mahalle_"))&&Id!=TEXT("mahalle_01"))IFileManager::Get().Delete(*(FPaths::ProjectSavedDir()/TEXT("StoreDrafts")/(Id+TEXT(".json"))));
                if(!Ok){UE_LOG(LogTemp,Error,TEXT("StoreEditor REVIEW create/copy/reload failed"));FPlatformMisc::RequestExitWithStatus(false,1);return false;}
                UE_LOG(LogTemp,Display,TEXT("StoreEditor REVIEW PASSED: new/copy/unique ID/reload, placement, duplicate, undo/redo, departments, resize, floor, draft, 2D empty-space pan, marquee selection, group movement, zero-gap contact, zoom clipping and screenshots"));Window->RequestDestroyWindow();FPlatformMisc::RequestExitWithStatus(false,0);return false;
            }
            return true;
        }),6.f);
    }
    TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args)
    {
        return SNew(SDockTab).TabRole(ETabRole::NomadTab)[SNew(SProductStudio)];
    }

    TSharedRef<SDockTab> SpawnPlanogramTab(const FSpawnTabArgs& Args)
    {
        return SNew(SDockTab).TabRole(ETabRole::NomadTab)[SNew(SPlanogramStudio)];
    }

    void Open()
    {
        FGlobalTabmanager::Get()->TryInvokeTab(StudioTab);
    }

    void OpenPlanogram() { FGlobalTabmanager::Get()->TryInvokeTab(PlanogramTab); }

    void RegisterMenus()
    {
        FToolMenuOwnerScoped Owner(this);
        const FText Label = FText::FromString(TEXT("\u00dcr\u00fcn St\u00fcdyosu"));
        const FText Tooltip = FText::FromString(TEXT("MarketSim: kutu/etiket/\u00fcr\u00fcn bilgisini oyuna ekle"));
        const FSlateIcon Icon(FAppStyle::GetAppStyleSetName(), TEXT("ClassIcon.StaticMesh"));
        const FUIAction Action(FExecuteAction::CreateRaw(this, &FMarketSimStudioModule::Open));

        if (UToolMenu* Tools = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools")))
        {
            FToolMenuSection& Section = Tools->FindOrAddSection(TEXT("MarketSim"));
            Section.Label = FText::FromString(TEXT("MarketSim"));
            Section.AddMenuEntry(TEXT("SimStoreStudio"),FText::FromString(TEXT("Ma\u011faza Edit\u00f6r\u00fc")),
                FText::FromString(TEXT("Bina, depo, zemin ve ekipman yerle\u015fimi")),Icon,FUIAction(FExecuteAction::CreateRaw(this,&FMarketSimStudioModule::OpenStore)));
            Section.AddMenuEntry(TEXT("SimProductStudio"), Label, Tooltip, Icon, Action);
            Section.AddMenuEntry(TEXT("SimPlanogramStudio"), FText::FromString(TEXT("Raf Plan\u0131 Edit\u00f6r\u00fc")),
                FText::FromString(TEXT("Ayn\u0131 rafta birden \u00e7ok \u00fcr\u00fcn\u00fc ve arka derinli\u011fi d\u00fczenle")),
                FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("ClassIcon.StaticMeshActor")),
                FUIAction(FExecuteAction::CreateRaw(this, &FMarketSimStudioModule::OpenPlanogram)));
        }
        if (UToolMenu* Toolbar = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.LevelEditorToolBar.User")))
        {
            FToolMenuSection& Section = Toolbar->FindOrAddSection(TEXT("MarketSim"));
            FToolMenuEntry Entry = FToolMenuEntry::InitToolBarButton(TEXT("SimProductStudioButton"), Action, Label, Tooltip, Icon);
            Section.AddEntry(Entry);
        }
        // STUDYO.cmd starts the editor with -SimStudio: open the studio once the editor UI is up.
        if(FParse::Param(FCommandLine::Get(),TEXT("SimStoreEditor")))
        {
            FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float){OpenStore();return false;}),1.5f);
        }
        if(FParse::Param(FCommandLine::Get(),TEXT("SimStoreEditorReview")))
        {
            FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float){ReviewStoreEditor();return false;}),2.f);
        }
        if (FParse::Param(FCommandLine::Get(), TEXT("SimStudio")))
        {
            FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float)
            {
                Open();
                return false;
            }), 1.5f);
        }
        if (FParse::Param(FCommandLine::Get(), TEXT("SimPlanogram")))
        {
            FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float)
            {
                OpenPlanogram();
                return false;
            }), 1.5f);
        }
    }
};

IMPLEMENT_MODULE(FMarketSimStudioModule, MarketSimStudio)

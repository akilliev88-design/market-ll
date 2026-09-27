#include "Containers/Ticker.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Modules/ModuleManager.h"
#include "SProductStudio.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

namespace
{
    const FName StudioTab(TEXT("MirasProductStudio"));
}

class FMirasMarketStudioModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        FGlobalTabmanager::Get()->RegisterNomadTabSpawner(StudioTab, FOnSpawnTab::CreateRaw(this, &FMirasMarketStudioModule::SpawnTab))
            .SetDisplayName(FText::FromString(TEXT("\u00dcr\u00fcn St\u00fcdyosu")))
            .SetTooltipText(FText::FromString(TEXT("Kutu, etiket ve \u00fcr\u00fcn bilgisini oyuna ekle")))
            .SetMenuType(ETabSpawnerMenuType::Hidden);
        UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FMirasMarketStudioModule::RegisterMenus));
    }

    virtual void ShutdownModule() override
    {
        UToolMenus::UnRegisterStartupCallback(this);
        UToolMenus::UnregisterOwner(this);
        if (FSlateApplication::IsInitialized()) FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(StudioTab);
    }

private:
    TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args)
    {
        return SNew(SDockTab).TabRole(ETabRole::NomadTab)[SNew(SProductStudio)];
    }

    void Open()
    {
        FGlobalTabmanager::Get()->TryInvokeTab(StudioTab);
    }

    void RegisterMenus()
    {
        FToolMenuOwnerScoped Owner(this);
        const FText Label = FText::FromString(TEXT("\u00dcr\u00fcn St\u00fcdyosu"));
        const FText Tooltip = FText::FromString(TEXT("Miras Market: kutu/etiket/\u00fcr\u00fcn bilgisini oyuna ekle"));
        const FSlateIcon Icon(FAppStyle::GetAppStyleSetName(), TEXT("ClassIcon.StaticMesh"));
        const FUIAction Action(FExecuteAction::CreateRaw(this, &FMirasMarketStudioModule::Open));

        if (UToolMenu* Tools = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools")))
        {
            FToolMenuSection& Section = Tools->FindOrAddSection(TEXT("MirasMarket"));
            Section.Label = FText::FromString(TEXT("Miras Market"));
            Section.AddMenuEntry(TEXT("MirasProductStudio"), Label, Tooltip, Icon, Action);
        }
        if (UToolMenu* Toolbar = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.LevelEditorToolBar.User")))
        {
            FToolMenuSection& Section = Toolbar->FindOrAddSection(TEXT("MirasMarket"));
            FToolMenuEntry Entry = FToolMenuEntry::InitToolBarButton(TEXT("MirasProductStudioButton"), Action, Label, Tooltip, Icon);
            Section.AddEntry(Entry);
        }
        // STUDYO.cmd starts the editor with -MirasStudio: open the studio once the editor UI is up.
        if (FParse::Param(FCommandLine::Get(), TEXT("MirasStudio")))
        {
            FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float)
            {
                Open();
                return false;
            }), 1.5f);
        }
    }
};

IMPLEMENT_MODULE(FMirasMarketStudioModule, MirasMarketStudio)

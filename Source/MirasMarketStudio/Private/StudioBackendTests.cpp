#include "StudioBackend.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS
using namespace MirasStudio;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStudioUvTemplateTest, "MirasMarket.Studio.UvTemplateExport", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FStudioUvTemplateTest::RunTest(const FString& Parameters)
{
    FStudioPackage Package;
    Package.MeshPath = TEXT("/Game/Products/Packages/box_95x64x195/SM_box_95x64x195.SM_box_95x64x195");
    Package.SlotNames = { TEXT("Etiket") };
    Package.LabelSlot = 0;
    const FString Output = FPaths::ProjectSavedDir() / TEXT("TestArtifacts/uv_sablon_test.png");
    FString Error;
    TestTrue(TEXT("UV template is exported"), ExportUvTemplate(Package, Output, Error));
    TestTrue(TEXT("UV template PNG exists"), FPaths::FileExists(Output));
    TestTrue(TEXT("UV template PNG is not empty"), IFileManager::Get().FileSize(*Output) > 1024);
    if (!Error.IsEmpty()) AddError(Error);
    return true;
}
#endif

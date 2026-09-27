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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStudioReadyTemplateTest, "MirasMarket.Studio.ReadyPackageTemplates", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FStudioReadyTemplateTest::RunTest(const FString& Parameters)
{
    FStudioDraft Box;
    Box.Id = TEXT("template_test_box"); Box.PackageType = TEXT("kutu"); Box.Preset = TEXT("test_box");
    Box.WidthMm = TEXT("70"); Box.DepthMm = TEXT("50"); Box.HeightMm = TEXT("200");
    TArray<FString> Files; FString Error;
    TestTrue(TEXT("Ready box template exports"), ExportReadyPackageTemplates(Box, TEXT("2011"), Files, Error));
    TestEqual(TEXT("Box produces one guide"), Files.Num(), 1);
    FStudioImage Image;
    if (Files.Num() == 1)
    {
        TestTrue(TEXT("Box guide can be decoded"), LoadImageFile(Files[0], Image, Error));
        TestTrue(TEXT("Box guide has exact production canvas"), Image.Width == 1920 && Image.Height == 2400);
        IFileManager::Get().MakeDirectory(*(FPaths::ProjectSavedDir() / TEXT("TestArtifacts")), true);
        IFileManager::Get().Copy(*(FPaths::ProjectSavedDir() / TEXT("TestArtifacts/ready_box_template.png")), *Files[0], true);
    }
    FString Prompt;
    TestTrue(TEXT("Box prompt is generated"), BuildPrompt(TEXT("A1"), Box, TEXT("2011"), Prompt, Error));
    TestTrue(TEXT("Box prompt requires attached exact-size guide"), Prompt.Contains(TEXT("acilim_sablonu.png")) && Prompt.Contains(TEXT("TAM AYNI piksel")));
    TestFalse(TEXT("Box prompt has no unresolved placeholders"), Prompt.Contains(TEXT("{{")));

    FStudioDraft Bottle;
    Bottle.Id = TEXT("template_test_bottle"); Bottle.PackageType = TEXT("pet_sise"); Bottle.Preset = TEXT("test_bottle");
    Bottle.DiameterMm = TEXT("80"); Bottle.LabelHeightMm = TEXT("100"); Bottle.Parts = TEXT("Etiket,Cam,Kapak");
    Files.Reset();
    TestTrue(TEXT("Ready round templates export"), ExportReadyPackageTemplates(Bottle, TEXT("2011"), Files, Error));
    TestEqual(TEXT("Bottle produces label and cap guides"), Files.Num(), 2);
    if (Files.Num() == 2)
    {
        TestTrue(TEXT("Label guide can be decoded"), LoadImageFile(Files[0], Image, Error));
        TestTrue(TEXT("Label guide uses circumference dimensions"), Image.Width == FMath::FloorToInt32(UE_PI * 80.0 * 8.0) && Image.Height == 800);
        TestTrue(TEXT("Cap guide can be decoded"), LoadImageFile(Files[1], Image, Error));
        TestTrue(TEXT("Cap guide is 512 square"), Image.Width == 512 && Image.Height == 512);
        IFileManager::Get().Copy(*(FPaths::ProjectSavedDir() / TEXT("TestArtifacts/ready_label_template.png")), *Files[0], true);
        IFileManager::Get().Copy(*(FPaths::ProjectSavedDir() / TEXT("TestArtifacts/ready_cap_template.png")), *Files[1], true);
    }
    Prompt.Reset();
    TestTrue(TEXT("Round prompt is generated"), BuildPrompt(TEXT("B2"), Bottle, TEXT("2011"), Prompt, Error));
    TestTrue(TEXT("Round prompt names both attached guides"), Prompt.Contains(TEXT("label_sablonu.png")) && Prompt.Contains(TEXT("kapak_sablonu.png")));
    TestFalse(TEXT("Round prompt has no unresolved placeholders"), Prompt.Contains(TEXT("{{")));
    IFileManager::Get().DeleteDirectory(*(FPaths::ProjectDir() / TEXT("Uretim/template_test_box")), false, true);
    IFileManager::Get().DeleteDirectory(*(FPaths::ProjectDir() / TEXT("Uretim/template_test_bottle")), false, true);
    if (!Error.IsEmpty()) AddError(Error);
    return true;
}
#endif

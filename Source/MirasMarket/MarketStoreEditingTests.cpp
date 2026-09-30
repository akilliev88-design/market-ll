#include "MarketStoreEditing.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStoreEditPlacementTest,"MirasMarket.Stores.EditorPlacement",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FStoreEditPlacementTest::RunTest(const FString&)
{
    TArray<FString> Errors;TestTrue(TEXT("Load"),MarketStoreKit::Load(Errors));auto S=*MarketStoreKit::Find(TEXT("buyuk_01"));S.Fixtures.Reset();S.Obstacles.Reset();
    int32 Index;TestTrue(TEXT("Add in free floor"),MarketStoreEditing::Place(S,TEXT("gondola_double_1200"),TEXT("s\u00fct"),FVector(0,0,0),Index));
    auto F=S.Fixtures[0];F.Location=FVector(12,0,0);TestFalse(TEXT("Block overlap"),MarketStoreEditing::CanPlace(S,F));
    F.Location=FVector(0,S.Backroom.Min.Y+10,0);TestFalse(TEXT("Block depot"),MarketStoreEditing::CanPlace(S,F));
    F.Location=FVector(90000,0,0);TestFalse(TEXT("Block exterior"),MarketStoreEditing::CanPlace(S,F));
    F.Location=FVector(S.FootprintCm.X/2-65,0,0);auto P=MarketStoreEditing::Snap(S,F,true,false,10);TestEqual(TEXT("Wall snap uses actual footprint and 2cm clearance"),P.X,S.FootprintCm.X/2-62);
    S.Fixtures[0].Yaw=90;int32 New;TestTrue(TEXT("Duplicate rotated module"),MarketStoreEditing::Duplicate(S,0,2,New));TestEqual(TEXT("Keep angle"),S.Fixtures[New].Yaw,90.f);TestTrue(TEXT("Duplicate fits"),MarketStoreEditing::CanPlace(S,S.Fixtures[New],New));TestNotEqual(TEXT("Unique fixture IDs"),S.Fixtures[0].Id,S.Fixtures[New].Id);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStoreEditNewAndPickTest,"MirasMarket.Stores.EditorCreateAndPick",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FStoreEditNewAndPickTest::RunTest(const FString&)
{
    FStoreTemplate S;TestTrue(TEXT("Create blank supermarket"),MarketStoreEditing::Create(TEXT("buyuk"),TEXT("buyuk_99"),TEXT("New store"),S));
    TestTrue(TEXT("High supermarket ceiling"),S.CeilingCm>=600);TestEqual(TEXT("New store starts empty"),S.Fixtures.Num(),0);TestTrue(TEXT("Editable architecture"),S.bEditableShell);
    const auto Before=S;TestFalse(TEXT("Unknown format refused"),MarketStoreEditing::Create(TEXT("unknown"),TEXT("unknown_01"),TEXT("Bad"),S));TestEqual(TEXT("Failure retains previous store"),S.Id,Before.Id);
    int32 I;TestTrue(TEXT("Place without unwanted rounding"),MarketStoreEditing::Place(S,TEXT("gondola_double_1200"),TEXT(""),FVector(123,17,0),I,false,false,0,90));
    TestEqual(TEXT("Disabled grid respected"),S.Fixtures[I].Location.X,123.);TestEqual(TEXT("Placement rotation preserved"),S.Fixtures[I].Yaw,90.f);
    TestEqual(TEXT("Top-down ray selects rotated fixture"),MarketStoreEditing::Pick(S,FVector(123,17,500),FVector(0,0,-1)),I);
    TestEqual(TEXT("Walking eye ray selects fixture"),MarketStoreEditing::Pick(S,FVector(123,-500,100),FVector(0,1,0)),I);
    TestEqual(TEXT("Ray misses above equipment"),MarketStoreEditing::Pick(S,FVector(123,-500,1000),FVector(0,1,0)),INDEX_NONE);
    TestEqual(TEXT("Ray misses empty floor"),MarketStoreEditing::Pick(S,FVector(900,900,500),FVector(0,0,-1)),INDEX_NONE);
    FString Error;const auto Path=FPaths::ProjectSavedDir()/TEXT("Tests/EmptyStoreDraft.json");TestTrue(TEXT("Empty store draft saves"),MarketStoreEditing::Save(S,Path,Error));FStoreTemplate Reload;TestTrue(TEXT("User store draft reloads"),MarketStoreEditing::LoadDraft(Path,Reload,Error));TestEqual(TEXT("New store ID persists"),Reload.Id,S.Id);IFileManager::Get().Delete(*Path);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStoreEditResizeTest,"MirasMarket.Stores.EditorArchitecture",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FStoreEditResizeTest::RunTest(const FString&)
{
    TArray<FString> Errors;MarketStoreKit::Load(Errors);auto S=*MarketStoreKit::Find(TEXT("mahalle_01"));const auto Before=S;FString Error;
    TestTrue(TEXT("Resize preserves recessed floor"),MarketStoreEditing::Resize(S,1600,1400,1400,220,350,Error));TestTrue(TEXT("Parametric architecture selected"),S.bEditableShell);TestEqual(TEXT("Polygon points preserved"),S.Outline.Num(),Before.Outline.Num());TestEqual(TEXT("Depot square metres computed"),S.BackroomM2,30.8);TestEqual(TEXT("Column height follows ceiling"),S.Obstacles[0].Size.Z,350.);
    TestFalse(TEXT("Invalid depot rejected"),MarketStoreEditing::Resize(S,1600,1400,2000,220,350,Error));TestEqual(TEXT("Failure leaves store unchanged"),S.FootprintCm.X,1600.);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStoreEditSaveTest,"MirasMarket.Stores.EditorDraftAndPublish",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FStoreEditSaveTest::RunTest(const FString&)
{
    TArray<FString> Errors;MarketStoreKit::Load(Errors);auto S=*MarketStoreKit::Find(TEXT("mahalle_01"));FString Error;const auto Path=FPaths::ProjectSavedDir()/TEXT("Tests/StoreEditorDraft.json");
    S.FloorColor=FLinearColor(.24,.37,.49);S.FloorFinish=TEXT("concrete");S.bEditableShell=true;
    TestTrue(TEXT("Save valid draft"),MarketStoreEditing::Save(S,Path,Error));FStoreTemplate Reload;TestTrue(TEXT("Read draft"),MarketStoreEditing::LoadDraft(Path,Reload,Error));TestEqual(TEXT("Floor finish roundtrip"),Reload.FloorFinish,S.FloorFinish);TestEqual(TEXT("Colour roundtrip"),Reload.FloorColor,S.FloorColor);TestTrue(TEXT("Architecture flag roundtrip"),Reload.bEditableShell);TestEqual(TEXT("Fixture count preserved"),Reload.Fixtures.Num(),S.Fixtures.Num());
    auto Invalid=S;Invalid.SalesAreaM2=1;const auto Catalogue=FPaths::ProjectConfigDir()/TEXT("magazalar.json");FString Before,After;FFileHelper::LoadFileToString(Before,*Catalogue);
    TestFalse(TEXT("Invalid published store refused"),MarketStoreEditing::Save(Invalid,Catalogue,Error));FFileHelper::LoadFileToString(After,*Catalogue);TestEqual(TEXT("Catalogue unchanged on rejection"),After,Before);
    Invalid=S;Invalid.Fixtures[0].Location=Invalid.Fixtures[1].Location;TestFalse(TEXT("Overlapping draft refused"),MarketStoreEditing::Save(Invalid,Path,Error));
    FFileHelper::LoadFileToString(After,*Path);TestTrue(TEXT("Last good draft retained"),After.Contains(TEXT("concrete")));IFileManager::Get().Delete(*Path);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStoreEditGroupTest,"MirasMarket.Stores.EditorGroupAndContact",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FStoreEditGroupTest::RunTest(const FString&)
{
    FStoreTemplate S;MarketStoreEditing::Create(TEXT("buyuk"),TEXT("buyuk_99"),TEXT("Group test"),S);int32 A,B;
    TestTrue(TEXT("First module"),MarketStoreEditing::Place(S,TEXT("gondola_double_1200"),TEXT(""),FVector(0,0,0),A,false,false,0));
    TestTrue(TEXT("Second flush module"),MarketStoreEditing::Place(S,TEXT("gondola_double_1200"),TEXT(""),FVector(123,0,0),B,false,true,0,0,0));
    TestEqual(TEXT("No forced gap between cabinets"),S.Fixtures[B].Location.X,120.);
    const FVector Spacing=S.Fixtures[B].Location-S.Fixtures[A].Location;TSet<int32> Group={A,B};
    TestTrue(TEXT("Group moves freely without grid"),MarketStoreEditing::MoveGroup(S,Group,FVector(371,213,0),false,false,0));TestEqual(TEXT("Exact free destination"),S.Fixtures[A].Location.X,371.);TestEqual(TEXT("Relative spacing retained"),S.Fixtures[B].Location-S.Fixtures[A].Location,Spacing);
    const auto Before=S;TestFalse(TEXT("Whole group blocked at exterior"),MarketStoreEditing::MoveGroup(S,Group,FVector(90000,0,0),true,true,0));TestEqual(TEXT("No partial group movement"),S.Fixtures[A].Location,Before.Fixtures[A].Location);
    auto F=S.Fixtures[A];F.Location=FVector(S.FootprintCm.X/2-65,-400,0);TestEqual(TEXT("Zero-gap wall snap"),MarketStoreEditing::Snap(S,F,true,false,0,0).X,S.FootprintCm.X/2-60);
    return true;
}

#include "MarketStoreEditing.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStoreGeometryTest,"MirasMarket.Stores.EditorArchitectureAndTour",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FStoreGeometryTest::RunTest(const FString&)
{
    FStoreTemplate S;MarketStoreEditing::Create(TEXT("buyuk"),TEXT("buyuk_editor_test"),TEXT("Editable tour test"),S);FString E;int32 A,B;
    MarketStoreEditing::Place(S,TEXT("gondola_double_1200"),TEXT(""),FVector(100,0,0),A,false,false,0,90);MarketStoreEditing::Place(S,TEXT("gondola_double_1200"),TEXT(""),FVector(600,133,0),B,false,false,0,90);
    TestTrue(TEXT("Vertical separated rows align"),MarketStoreEditing::MoveGroup(S,{B},FVector(0,-120,0),false,true,0,35));TestEqual(TEXT("Screen vertical alignment"),S.Fixtures[B].Location.Y,S.Fixtures[A].Location.Y);TestEqual(TEXT("Horizontal aisle remains"),S.Fixtures[B].Location.X,600.);
    S.Fixtures[B].Location=FVector(113,600,0);TestTrue(TEXT("Alignment also across the other axis"),MarketStoreEditing::MoveGroup(S,{B},FVector::ZeroVector,false,true,0,35));TestEqual(TEXT("Other axis aligned"),S.Fixtures[B].Location.X,S.Fixtures[A].Location.X);
    const auto Fixtures=S.Fixtures;S.Obstacles.Add({TEXT("column_test"),TEXT("column"),FVector(-500,0,0),FVector(50,50,600)});
    TestTrue(TEXT("Column moves and reshapes"),MarketStoreEditing::EditObstacle(S,0,FVector(-600,-200,0),FVector(80,120,600),TEXT("round"),E));const auto Column=S.Obstacles[0];
    TestTrue(TEXT("Building front wall extends"),MarketStoreEditing::MoveWall(S,false,0,FVector(0,-1800,0),E));for(int32 I=0;I<Fixtures.Num();++I)TestEqual(TEXT("Wall preserves fixture position"),S.Fixtures[I].Location,Fixtures[I].Location);TestEqual(TEXT("Wall preserves column position"),S.Obstacles[0].At,Column.At);
    TestTrue(TEXT("Depot front wall extends inward"),MarketStoreEditing::MoveWall(S,true,0,FVector(0,900,0),E));TestEqual(TEXT("Depot front coordinate"),S.Backroom.Min.Y,900.);
    TestTrue(TEXT("Depot exterior wall extends building"),MarketStoreEditing::MoveWall(S,true,2,FVector(0,1800,0),E));TestEqual(TEXT("Depot rear coordinate"),S.Backroom.Max.Y,1800.);TestEqual(TEXT("Building rear follows"),S.Outline[2].Y,1800.);
    const auto Before=S;TestFalse(TEXT("Depot cannot swallow shelves"),MarketStoreEditing::MoveWall(S,true,0,FVector(0,-100,0),E));TestEqual(TEXT("Invalid wall move is atomic"),S.Backroom.Min,Before.Backroom.Min);
    TestTrue(TEXT("Place depot door"),MarketStoreEditing::MoveDepotDoor(S,FVector(700,900,0)));TestEqual(TEXT("Depot door location"),S.DepotDoor.At,FVector(700,900,0));
    const auto Marker=FPaths::ProjectSavedDir()/TEXT("StoreTours/last.txt"),Path=FPaths::ProjectSavedDir()/TEXT("StoreTours")/(S.Id+TEXT(".json"));FString OldMarker;const bool HadMarker=FFileHelper::LoadFileToString(OldMarker,*Marker);FString OldStore;const bool HadStore=FFileHelper::LoadFileToString(OldStore,*Path);
    TestTrue(TEXT("Incomplete store saves for tour without mandatory departments"),MarketStoreEditing::SaveTour(S,E));FStoreTemplate Reload;TestTrue(TEXT("Tour reloads exact layout"),MarketStoreEditing::LoadTour(S.Id,Reload,E));TestEqual(TEXT("Tour keeps fixtures"),Reload.Fixtures[0].Location,S.Fixtures[0].Location);TestEqual(TEXT("Tour keeps depot door"),Reload.DepotDoor.At,S.DepotDoor.At);TestEqual(TEXT("Tour keeps column shape"),Reload.Obstacles[0].Shape,TEXT("round"));TestTrue(TEXT("Tour list includes edited store"),MarketStoreEditing::TourIds().Contains(S.Id));
    if(HadMarker)FFileHelper::SaveStringToFile(OldMarker,*Marker,FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);else IFileManager::Get().Delete(*Marker);if(HadStore)FFileHelper::SaveStringToFile(OldStore,*Path,FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);else IFileManager::Get().Delete(*Path);
    return true;
}

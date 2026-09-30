#include "MarketStoreEditing.h"
#include "ProductCatalog.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"

namespace StoreGeometry
{
    bool Inside(const FStoreTemplate& S,FVector2D P)
    {
        bool Hit=false;for(int32 I=0;I<S.Outline.Num();++I){const auto A=S.Outline[I],B=S.Outline[(I+1)%S.Outline.Num()];if(FMath::Abs((P.X-A.X)*(B.Y-A.Y)-(P.Y-A.Y)*(B.X-A.X))<.01&&P.X>=FMath::Min(A.X,B.X)-.01&&P.X<=FMath::Max(A.X,B.X)+.01&&P.Y>=FMath::Min(A.Y,B.Y)-.01&&P.Y<=FMath::Max(A.Y,B.Y)+.01)return true;if((A.Y>P.Y)!=(B.Y>P.Y)&&P.X<(B.X-A.X)*(P.Y-A.Y)/(B.Y-A.Y)+A.X)Hit=!Hit;}return Hit;
    }
    void Recount(FStoreTemplate& S)
    {
        double X=0,Y=0,Area=0;for(int32 I=0;I<S.Outline.Num();++I){const auto A=S.Outline[I],B=S.Outline[(I+1)%S.Outline.Num()];X=FMath::Max(X,FMath::Abs(A.X));Y=FMath::Max(Y,FMath::Abs(A.Y));Area+=A.X*B.Y-B.X*A.Y;}
        // Keep the world origin and every fixture fixed when only one wall moves.
        S.FootprintCm=FVector2D(2*X,2*Y);S.BackroomM2=S.Backroom.GetSize().X*S.Backroom.GetSize().Y/10000;S.SalesAreaM2=FMath::Abs(Area)/20000-S.BackroomM2;S.bEditableShell=true;S.Stats=MarketStoreKit::CalculateStats(S);
    }
    TArray<FVector2D> Depot(const FStoreTemplate& S){const auto L=S.Backroom.Min,H=S.Backroom.Max;return {FVector2D(L.X,L.Y),FVector2D(H.X,L.Y),FVector2D(H.X,H.Y),FVector2D(L.X,H.Y)};}
}
FStorePoint MarketStoreEditing::DepotDoor(const FStoreTemplate& S)
{
    auto P=S.DepotDoor;const auto L=S.Backroom.Min,H=S.Backroom.Max;
    if(P.At.IsNearlyZero()){P.At=FVector(FMath::Clamp(0.,L.X+90,H.X-90),L.Y,0);P.Yaw=90;}return P;
}
bool MarketStoreEditing::MoveDepotDoor(FStoreTemplate& S,FVector Desired)
{
    if(Desired.ContainsNaN())return false;const auto V=StoreGeometry::Depot(S);double Best=1.e30;FStorePoint P;
    for(int32 I=0;I<4;++I){const auto A=V[I],B=V[(I+1)%4],D=B-A;const double Len=D.Size();if(Len<180)continue;const auto Mid=(A+B)/2;bool Outer=false;for(int32 J=0;J<S.Outline.Num();++J){const auto C=S.Outline[J],E=S.Outline[(J+1)%S.Outline.Num()];if((FMath::Abs(A.X-B.X)<.01&&FMath::Abs(C.X-E.X)<.01&&FMath::Abs(Mid.X-C.X)<.01&&Mid.Y>=FMath::Min(C.Y,E.Y)&&Mid.Y<=FMath::Max(C.Y,E.Y))||(FMath::Abs(A.Y-B.Y)<.01&&FMath::Abs(C.Y-E.Y)<.01&&FMath::Abs(Mid.Y-C.Y)<.01&&Mid.X>=FMath::Min(C.X,E.X)&&Mid.X<=FMath::Max(C.X,E.X)))Outer=true;}if(Outer)continue;
        const auto Q=A+D*FMath::Clamp(FVector2D::DotProduct(FVector2D(Desired.X,Desired.Y)-A,D)/(Len*Len),90/Len,1-90/Len);const double Dist=(Q-FVector2D(Desired.X,Desired.Y)).SizeSquared();if(Dist<Best){Best=Dist;P.At=FVector(Q.X,Q.Y,0);P.Yaw=FVector(-D.Y,D.X,0).Rotation().Yaw;}}
    if(Best==1.e30)return false;S.DepotDoor=P;S.bHasDepotDoor=true;S.bEditableShell=true;return true;
}
bool MarketStoreEditing::GeometryValid(const FStoreTemplate& S,FString& Error)
{
    if(!MarketCatalog::IsValidId(S.Id)||S.Outline.Num()<4||S.CeilingCm<250||S.SalesAreaM2<=0||S.Backroom.GetSize().X<100||S.Backroom.GetSize().Y<100){Error=TEXT("Bina veya depo cok kucuk.");return false;}
    for(int32 I=0;I<S.Outline.Num();++I){const auto A=S.Outline[I],B=S.Outline[(I+1)%S.Outline.Num()];if(A.ContainsNaN()||(!FMath::IsNearlyEqual(A.X,B.X)&&!FMath::IsNearlyEqual(A.Y,B.Y))||(A-B).Size()<20){Error=TEXT("Duvarlar dik ve en az 20 cm olmali.");return false;}}
    for(int32 I=0;I<S.Outline.Num();++I)for(int32 J=I+2;J<S.Outline.Num();++J){if(I==0&&J==S.Outline.Num()-1)continue;const auto A=S.Outline[I],B=S.Outline[(I+1)%S.Outline.Num()],C=S.Outline[J],D=S.Outline[(J+1)%S.Outline.Num()];const bool Touch=FMath::Max(FMath::Min(A.X,B.X),FMath::Min(C.X,D.X))<=FMath::Min(FMath::Max(A.X,B.X),FMath::Max(C.X,D.X))+.01&&FMath::Max(FMath::Min(A.Y,B.Y),FMath::Min(C.Y,D.Y))<=FMath::Min(FMath::Max(A.Y,B.Y),FMath::Max(C.Y,D.Y))+.01;if(Touch){Error=TEXT("Duvarlar birbirini kesemez.");return false;}}
    for(const auto P:StoreGeometry::Depot(S))if(!StoreGeometry::Inside(S,P)){Error=TEXT("Depo bina disina tasiyor.");return false;}
    for(int32 I=0;I<S.Obstacles.Num();++I){const auto& O=S.Obstacles[I];if(O.At.ContainsNaN()||O.Size.ContainsNaN()||O.Size.GetMin()<10||O.Size.Z>S.CeilingCm){Error=TEXT("Kolon olcusu gecersiz.");return false;}const auto H=FVector2D(O.Size.X/2,O.Size.Y/2),C=FVector2D(O.At.X,O.At.Y);for(int32 X:{-1,1})for(int32 Y:{-1,1})if(!StoreGeometry::Inside(S,C+FVector2D(X*H.X,Y*H.Y))){Error=TEXT("Kolon bina disina tasiyor.");return false;}for(int32 J=0;J<I;++J){const auto& B=S.Obstacles[J];if(FMath::Abs(O.At.X-B.At.X)<(O.Size.X+B.Size.X)/2-.5&&FMath::Abs(O.At.Y-B.At.Y)<(O.Size.Y+B.Size.Y)/2-.5){Error=TEXT("Kolonlar cakisti.");return false;}}}
    for(int32 I=0;I<S.Fixtures.Num();++I)if(!CanPlace(S,S.Fixtures[I],I)){Error=TEXT("Raf/kolon/depo cakismasi: ")+S.Fixtures[I].Id;return false;}return true;
}
bool MarketStoreEditing::EditObstacle(FStoreTemplate& S,int32 Index,FVector At,FVector Size,FString Shape,FString& Error)
{
    if(!S.Obstacles.IsValidIndex(Index))return false;auto C=S;C.Obstacles[Index].At=At;C.Obstacles[Index].Size=Size;C.Obstacles[Index].Shape=Shape;if(!GeometryValid(C,Error))return false;C.bEditableShell=true;S=MoveTemp(C);return true;
}
bool MarketStoreEditing::MoveWall(FStoreTemplate& S,bool Depot,int32 Edge,FVector Desired,FString& Error)
{
    if(Desired.ContainsNaN())return false;auto C=S;
    if(Depot){if(Edge<0||Edge>3)return false;if(Edge==0)C.Backroom.Min.Y=Desired.Y;if(Edge==1)C.Backroom.Max.X=Desired.X;if(Edge==2)C.Backroom.Max.Y=Desired.Y;if(Edge==3)C.Backroom.Min.X=Desired.X;
        // Moving a depot's exterior wall also moves the matching building wall.
        const auto Old=StoreGeometry::Depot(S),New=StoreGeometry::Depot(C);const auto A=Old[Edge],B=Old[(Edge+1)%4];for(int32 I=0;I<C.Outline.Num();++I){const auto P=S.Outline[I],Q=S.Outline[(I+1)%S.Outline.Num()];if(FMath::IsNearlyEqual(A.Y,B.Y)&&FMath::IsNearlyEqual(P.Y,Q.Y)&&FMath::IsNearlyEqual(P.Y,A.Y)&&FMath::Max(FMath::Min(P.X,Q.X),FMath::Min(A.X,B.X))<FMath::Min(FMath::Max(P.X,Q.X),FMath::Max(A.X,B.X))){C.Outline[I].Y=C.Outline[(I+1)%C.Outline.Num()].Y=New[Edge].Y;}else if(FMath::IsNearlyEqual(A.X,B.X)&&FMath::IsNearlyEqual(P.X,Q.X)&&FMath::IsNearlyEqual(P.X,A.X)&&FMath::Max(FMath::Min(P.Y,Q.Y),FMath::Min(A.Y,B.Y))<FMath::Min(FMath::Max(P.Y,Q.Y),FMath::Max(A.Y,B.Y))){C.Outline[I].X=C.Outline[(I+1)%C.Outline.Num()].X=New[Edge].X;}}}
    else{if(!C.Outline.IsValidIndex(Edge))return false;auto& A=C.Outline[Edge];auto& B=C.Outline[(Edge+1)%C.Outline.Num()];const auto OldA=A,OldB=B;if(FMath::IsNearlyEqual(A.Y,B.Y)){A.Y=B.Y=Desired.Y;if(FMath::IsNearlyEqual(S.Backroom.Max.Y,OldA.Y))C.Backroom.Max.Y=Desired.Y;if(FMath::IsNearlyEqual(S.Backroom.Min.Y,OldA.Y))C.Backroom.Min.Y=Desired.Y;}else{A.X=B.X=Desired.X;if(FMath::IsNearlyEqual(S.Backroom.Max.X,OldA.X))C.Backroom.Max.X=Desired.X;if(FMath::IsNearlyEqual(S.Backroom.Min.X,OldA.X))C.Backroom.Min.X=Desired.X;}}
    StoreGeometry::Recount(C);if(!GeometryValid(C,Error))return false;
    MoveDoor(C,false,S.Entrance.At);MoveDoor(C,true,S.Receiving.At);if(C.bHasDepotDoor)MoveDepotDoor(C,DepotDoor(S).At);S=MoveTemp(C);return true;
}
bool MarketStoreEditing::SaveTour(const FStoreTemplate& S,FString& Error)
{
    if(!GeometryValid(S,Error)||!Save(S,FPaths::ProjectSavedDir()/TEXT("StoreTours")/(S.Id+TEXT(".json")),Error))return false;
    return FFileHelper::SaveStringToFile(S.Id,*(FPaths::ProjectSavedDir()/TEXT("StoreTours/last.txt")),FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}
bool MarketStoreEditing::LoadTour(const FString& Id,FStoreTemplate& S,FString& Error)
{
    if(!MarketCatalog::IsValidId(Id))return false;return LoadDraft(FPaths::ProjectSavedDir()/TEXT("StoreTours")/(Id+TEXT(".json")),S,Error)&&GeometryValid(S,Error);
}
TArray<FString> MarketStoreEditing::TourIds()
{
    TArray<FString> Files,Result;IFileManager::Get().FindFiles(Files,*(FPaths::ProjectSavedDir()/TEXT("StoreTours/*.json")),true,false);Files.Sort();for(const auto& F:Files){FStoreTemplate S;FString E;if(LoadTour(FPaths::GetBaseFilename(F),S,E))Result.Add(S.Id);}return Result;
}

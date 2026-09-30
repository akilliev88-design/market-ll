#include "MarketStoreEditing.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"

void MarketStoreEditing::BuildArchitecture(UWorld* W,const FStoreTemplate& S)
{
    auto Make=[&](const FLinearColor& Color,bool Roof,float Roughness)
    {
        auto* A=W->SpawnActor<AActor>();A->Tags.Add(TEXT("MirasStoreKit"));if(Roof)A->Tags.Add(TEXT("MirasStoreRoof"));
        auto* Root=NewObject<USceneComponent>(A);A->SetRootComponent(Root);Root->RegisterComponent();
        auto* C=NewObject<UHierarchicalInstancedStaticMeshComponent>(A);C->SetupAttachment(Root);C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));C->SetCollisionProfileName(TEXT("BlockAll"));C->RegisterComponent();
        auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Stores/Materials/M_EditableSurface.M_EditableSurface"));if(!Base)Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Materials/M_Market.M_Market"));
        auto* M=UMaterialInstanceDynamic::Create(Base,A);M->SetVectorParameterValue(TEXT("Color"),Color);M->SetScalarParameterValue(TEXT("Roughness"),Roughness);C->SetMaterial(0,M);return C;
    };
    auto* Floor=Make(S.FloorColor,false,S.FloorFinish==TEXT("polished")?.18f:.65f);
    auto* Grout=Make(S.FloorColor*.55f,false,.8f);
    auto* Walls=Make(FLinearColor(.77,.77,.73),false,.72f);auto* Ceiling=Make(FLinearColor(.69,.70,.68),true,.8f);
    auto Cube=[](UHierarchicalInstancedStaticMeshComponent* C,FVector Center,FVector Size){C->AddInstance(FTransform(FRotator::ZeroRotator,Center,Size/100));};
    auto V=S.Outline;if(V.IsEmpty()){const auto H=S.FootprintCm/2;V={-H,FVector2D(H.X,-H.Y),H,FVector2D(-H.X,H.Y)};}
    // Orthogonal polygon decomposition preserves recesses without filling exterior voids.
    TArray<double> Xs,Ys;for(auto P:V){Xs.AddUnique(P.X);Ys.AddUnique(P.Y);}Xs.Sort();Ys.Sort();
    auto Inside=[&](double X,double Y){bool Hit=false;for(int32 I=0;I<V.Num();++I){auto A=V[I],B=V[(I+1)%V.Num()];if((A.Y>Y)!=(B.Y>Y)&&X<(B.X-A.X)*(Y-A.Y)/(B.Y-A.Y)+A.X)Hit=!Hit;}return Hit;};
    const bool Tile=S.FloorFinish==TEXT("tile");
    // The continuous grout substrate supplies collision; visual tiles need no per-instance bodies.
    if(Tile)Floor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    for(int32 I=0;I<Xs.Num()-1;++I)for(int32 J=0;J<Ys.Num()-1;++J)
    {
        const double X=Xs[I],XX=Xs[I+1],Y=Ys[J],YY=Ys[J+1];if(!Inside((X+XX)/2,(Y+YY)/2))continue;
        Cube(Tile?Grout:Floor,FVector((X+XX)/2,(Y+YY)/2,Tile?-6:-5),FVector(XX-X,YY-Y,10));
        Cube(Ceiling,FVector((X+XX)/2,(Y+YY)/2,S.CeilingCm+5),FVector(XX-X,YY-Y,10));
        if(Tile)for(double TX=X;TX<XX;TX+=60)for(double TY=Y;TY<YY;TY+=60){double TW=FMath::Min(60.,XX-TX),TD=FMath::Min(60.,YY-TY);Cube(Floor,FVector(TX+TW/2,TY+TD/2,-.5),FVector(TW-.25,TD-.25,1));}
    }
    auto Wall=[&](FVector2D A,FVector2D B,const TArray<FVector>& Doors)
    {
        const bool Horizontal=FMath::IsNearlyEqual(A.Y,B.Y);const double Lo=Horizontal?FMath::Min(A.X,B.X):FMath::Min(A.Y,B.Y),Hi=Horizontal?FMath::Max(A.X,B.X):FMath::Max(A.Y,B.Y);
        TArray<FVector2D> Holes;
        for(auto P:Doors)if(FMath::Abs((Horizontal?P.Y:P.X)-(Horizontal?A.Y:A.X))<20){const double C=Horizontal?P.X:P.Y;if(C>=Lo&&C<=Hi)Holes.Add(FVector2D(FMath::Max(Lo,C-80),FMath::Min(Hi,C+80)));}
        Holes.Sort([](auto L,auto R){return L.X<R.X;});double Cursor=Lo;
        auto Segment=[&](double Start,double End,double Z,double Height){if(End-Start<.1)return;Cube(Walls,Horizontal?FVector((Start+End)/2,A.Y,Z):FVector(A.X,(Start+End)/2,Z),Horizontal?FVector(End-Start,12,Height):FVector(12,End-Start,Height));};
        for(auto H:Holes){Segment(Cursor,H.X,S.CeilingCm/2,S.CeilingCm);Segment(H.X,H.Y,(S.CeilingCm+230)/2,S.CeilingCm-230);Cursor=H.Y;}Segment(Cursor,Hi,S.CeilingCm/2,S.CeilingCm);
    };
    for(int32 I=0;I<V.Num();++I)Wall(V[I],V[(I+1)%V.Num()],{S.Entrance.At,S.Receiving.At});
    const auto Lo=S.Backroom.Min,Hi=S.Backroom.Max;
    Wall(FVector2D(Lo.X,Lo.Y),FVector2D(Hi.X,Lo.Y),{FVector(0,Lo.Y,0)});
    if(Lo.X>-S.FootprintCm.X/2+.1)Wall(FVector2D(Lo.X,Lo.Y),FVector2D(Lo.X,Hi.Y),{});
    if(Hi.X<S.FootprintCm.X/2-.1)Wall(FVector2D(Hi.X,Lo.Y),FVector2D(Hi.X,Hi.Y),{});
    for(auto O:S.Obstacles)Cube(Walls,O.At+FVector(0,0,O.Size.Z/2),O.Size);
}

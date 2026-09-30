#include "MarketStoreEditing.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "HAL/FileManager.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"

namespace StoreEdit
{
    bool Inside(const FVector2D& P,const FStoreTemplate& S)
    {
        if(S.Outline.IsEmpty()) return FMath::Abs(P.X)<=S.FootprintCm.X/2 && FMath::Abs(P.Y)<=S.FootprintCm.Y/2;
        bool Hit=false;
        for(int32 I=0;I<S.Outline.Num();++I)
        {
            const auto A=S.Outline[I],B=S.Outline[(I+1)%S.Outline.Num()];
            if(FMath::Abs((P.X-A.X)*(B.Y-A.Y)-(P.Y-A.Y)*(B.X-A.X))<.01 && P.X>=FMath::Min(A.X,B.X)-.01 && P.X<=FMath::Max(A.X,B.X)+.01 && P.Y>=FMath::Min(A.Y,B.Y)-.01 && P.Y<=FMath::Max(A.Y,B.Y)+.01) return true;
            if((A.Y>P.Y)!=(B.Y>P.Y)&&P.X<(B.X-A.X)*(P.Y-A.Y)/(B.Y-A.Y)+A.X) Hit=!Hit;
        }
        return Hit;
    }
    FBox2D Box(FVector At,FVector2D Half) { FVector2D C(At.X,At.Y);return FBox2D(C-Half+FVector2D(.5,.5),C+Half-FVector2D(.5,.5)); }
    TArray<TSharedPtr<FJsonValue>> Vec(FVector P) { return {MakeShared<FJsonValueNumber>(P.X),MakeShared<FJsonValueNumber>(P.Y),MakeShared<FJsonValueNumber>(P.Z)}; }
    TSharedPtr<FJsonObject> Point(FStorePoint P) { auto O=MakeShared<FJsonObject>();O->SetArrayField(TEXT("at"),Vec(P.At));O->SetNumberField(TEXT("yaw"),P.Yaw);return O; }
    void Patch(const FStoreTemplate& S,TSharedPtr<FJsonObject> O)
    {
        O->SetStringField(TEXT("id"),S.Id);O->SetStringField(TEXT("format"),S.Format);O->SetStringField(TEXT("name"),S.Name);O->SetStringField(TEXT("theme"),S.Theme);
        O->SetStringField(TEXT("shell"),S.Shell);O->SetStringField(TEXT("roof"),S.Roof);O->SetBoolField(TEXT("editableShell"),S.bEditableShell);
        O->SetArrayField(TEXT("footprintCm"),{MakeShared<FJsonValueNumber>(S.FootprintCm.X),MakeShared<FJsonValueNumber>(S.FootprintCm.Y)});
        O->SetNumberField(TEXT("salesAreaM2"),S.SalesAreaM2);O->SetNumberField(TEXT("backroomM2"),S.BackroomM2);O->SetNumberField(TEXT("ceilingCm"),S.CeilingCm);
        O->SetStringField(TEXT("floorFinish"),S.FloorFinish);O->SetArrayField(TEXT("floorColor"),Vec(FVector(S.FloorColor.R,S.FloorColor.G,S.FloorColor.B)));
        auto P=MakeShared<FJsonObject>();P->SetObjectField(TEXT("entrance"),Point(S.Entrance));P->SetObjectField(TEXT("receiving"),Point(S.Receiving));P->SetObjectField(TEXT("playerStart"),Point(S.PlayerStart));
        auto B=MakeShared<FJsonObject>();B->SetArrayField(TEXT("min"),Vec(S.Backroom.Min));B->SetArrayField(TEXT("max"),Vec(S.Backroom.Max));P->SetObjectField(TEXT("backroom"),B);
        TArray<TSharedPtr<FJsonValue>> Spawns;for(auto V:S.CustomerSpawn) Spawns.Add(MakeShared<FJsonValueArray>(Vec(V)));P->SetArrayField(TEXT("customerSpawn"),Spawns);O->SetObjectField(TEXT("points"),P);
        const TSharedPtr<FJsonObject>* Existing=nullptr;auto A=O->TryGetObjectField(TEXT("architecture"),Existing)?*Existing:MakeShared<FJsonObject>();
        TArray<TSharedPtr<FJsonValue>> Outline,Obstacles,Sections,Fixtures,Props;
        for(auto V:S.Outline) Outline.Add(MakeShared<FJsonValueArray>(TArray<TSharedPtr<FJsonValue>>{MakeShared<FJsonValueNumber>(V.X),MakeShared<FJsonValueNumber>(V.Y)}));
        for(const auto& V:S.Obstacles) { auto J=MakeShared<FJsonObject>();J->SetStringField(TEXT("id"),V.Id);J->SetStringField(TEXT("kind"),V.Kind);J->SetArrayField(TEXT("at"),Vec(V.At));J->SetArrayField(TEXT("sizeCm"),Vec(V.Size));Obstacles.Add(MakeShared<FJsonValueObject>(J)); }
        for(const auto& V:S.Sections) {auto J=MakeShared<FJsonObject>();J->SetStringField(TEXT("label"),V.Label);J->SetArrayField(TEXT("at"),Vec(V.At));J->SetNumberField(TEXT("yaw"),V.Yaw);J->SetNumberField(TEXT("widthCm"),V.WidthCm);Sections.Add(MakeShared<FJsonValueObject>(J));}
        A->SetArrayField(TEXT("outlineCm"),Outline);A->SetArrayField(TEXT("obstacles"),Obstacles);A->SetArrayField(TEXT("sections"),Sections);O->SetObjectField(TEXT("architecture"),A);
        for(const auto& F:S.Fixtures) {auto J=MakeShared<FJsonObject>();J->SetStringField(TEXT("id"),F.Id);J->SetStringField(TEXT("equipment"),F.EquipmentId);J->SetStringField(TEXT("category"),F.Category);J->SetArrayField(TEXT("at"),Vec(F.Location));J->SetNumberField(TEXT("yaw"),F.Yaw);auto Faces=MakeShared<FJsonObject>();for(const auto& C:F.FaceCategories) Faces->SetStringField(C.Key,C.Value);J->SetObjectField(TEXT("faceCategories"),Faces);Fixtures.Add(MakeShared<FJsonValueObject>(J));}
        O->SetArrayField(TEXT("fixtures"),Fixtures);
        for(const auto& V:S.Props) {auto J=MakeShared<FJsonObject>();J->SetStringField(TEXT("mesh"),V.Mesh);J->SetArrayField(TEXT("at"),Vec(V.At));J->SetNumberField(TEXT("yaw"),V.Yaw);Props.Add(MakeShared<FJsonValueObject>(J));}O->SetArrayField(TEXT("props"),Props);
        const auto Stats=MarketStoreKit::CalculateStats(S);auto J=MakeShared<FJsonObject>();
        J->SetNumberField(TEXT("shelfFrontM"),Stats.ShelfFrontM);J->SetNumberField(TEXT("coolerM"),Stats.CoolerM);J->SetNumberField(TEXT("freezerM"),Stats.FreezerM);J->SetNumberField(TEXT("produceM2"),Stats.ProduceM2);J->SetNumberField(TEXT("checkouts"),Stats.Checkouts);J->SetNumberField(TEXT("selfCheckouts"),Stats.SelfCheckouts);J->SetNumberField(TEXT("backroomPallets"),Stats.BackroomPallets);
        TArray<TSharedPtr<FJsonValue>> Counters;for(auto C:Stats.Counters) Counters.Add(MakeShared<FJsonValueString>(C));J->SetArrayField(TEXT("counters"),Counters);O->SetObjectField(TEXT("stats"),J);
    }
}
FVector2D MarketStoreEditing::HalfSize(const FPlanogramFixture& F)
{
    const auto D=MarketPlanogram::Equipment(F.EquipmentId).DimensionsCm;const double A=FMath::DegreesToRadians(F.Yaw);
    return FVector2D((FMath::Abs(FMath::Cos(A))*D.X+FMath::Abs(FMath::Sin(A))*D.Y)/2,(FMath::Abs(FMath::Sin(A))*D.X+FMath::Abs(FMath::Cos(A))*D.Y)/2);
}
bool MarketStoreEditing::CanPlace(const FStoreTemplate& S,const FPlanogramFixture& F,int32 Ignore)
{
    if(!MarketPlanogram::IsKnownEquipment(F.EquipmentId)||F.Location.ContainsNaN()||!FMath::IsFinite(F.Yaw))return false;
    const auto H=HalfSize(F);const auto B=StoreEdit::Box(F.Location,H);
    for(int32 X:{-1,0,1})for(int32 Y:{-1,0,1})if(!StoreEdit::Inside(FVector2D(F.Location.X+X*H.X,F.Location.Y+Y*H.Y),S))return false;
    if(B.Intersect(FBox2D(FVector2D(S.Backroom.Min.X+.5,S.Backroom.Min.Y+.5),FVector2D(S.Backroom.Max.X-.5,S.Backroom.Max.Y-.5))))return false;
    for(const auto& O:S.Obstacles)if(B.Intersect(StoreEdit::Box(O.At,FVector2D(O.Size.X/2,O.Size.Y/2))))return false;
    for(int32 I=0;I<S.Fixtures.Num();++I)if(I!=Ignore&&B.Intersect(StoreEdit::Box(S.Fixtures[I].Location,HalfSize(S.Fixtures[I]))))return false;
    return true;
}
FVector MarketStoreEditing::Snap(const FStoreTemplate& S,const FPlanogramFixture& F,bool Walls,bool Neighbours,float Grid)
{
    FVector P=F.Location;const auto H=HalfSize(F);if(Grid>0){P.X=FMath::GridSnap(P.X,Grid);P.Y=FMath::GridSnap(P.Y,Grid);}
    auto Near=[](double& Value,double Target){if(FMath::Abs(Value-Target)<35)Value=Target;};
    if(Walls)
    {
        const auto& V=S.Outline;
        for(int32 I=0;I<V.Num();++I)
        {
            const auto A=V[I],B=V[(I+1)%V.Num()];
            if(FMath::IsNearlyEqual(A.X,B.X)&&P.Y-H.Y>=FMath::Min(A.Y,B.Y)&&P.Y+H.Y<=FMath::Max(A.Y,B.Y)){Near(P.X,A.X+H.X+2);Near(P.X,A.X-H.X-2);}
            if(FMath::IsNearlyEqual(A.Y,B.Y)&&P.X-H.X>=FMath::Min(A.X,B.X)&&P.X+H.X<=FMath::Max(A.X,B.X)){Near(P.Y,A.Y+H.Y+2);Near(P.Y,A.Y-H.Y-2);}
        }
        Near(P.Y,S.Backroom.Min.Y-H.Y-2);
    }
    if(Neighbours)for(const auto& O:S.Fixtures)if(O.Id!=F.Id)
    {
        const auto OH=HalfSize(O);
        if(FMath::Abs(P.Y-O.Location.Y)<H.Y+OH.Y+30){Near(P.X,O.Location.X+OH.X+H.X+2);Near(P.X,O.Location.X-OH.X-H.X-2);Near(P.Y,O.Location.Y);}
        if(FMath::Abs(P.X-O.Location.X)<H.X+OH.X+30){Near(P.Y,O.Location.Y+OH.Y+H.Y+2);Near(P.Y,O.Location.Y-OH.Y-H.Y-2);Near(P.X,O.Location.X);}
    }
    P.Z=0;return P;
}
bool MarketStoreEditing::Place(FStoreTemplate& S,const FString& E,const FString& C,FVector At,int32& Selected,bool Walls,bool Neighbours,float Grid,float Yaw)
{
    FPlanogramFixture F;F.EquipmentId=E;F.Category=C;F.Label=C;F.Location=At;F.Location.Z=0;F.Yaw=Yaw;
    int32 N=1;do {F.Id=FString::Printf(TEXT("editor_%04d"),N++);}while(S.Fixtures.ContainsByPredicate([&](const auto& O){return O.Id==F.Id;})||S.Obstacles.ContainsByPredicate([&](const auto& O){return O.Id==F.Id;}));
    F.Location=Snap(S,F,Walls,Neighbours,Grid);if(!CanPlace(S,F))return false;Selected=S.Fixtures.Add(F);return true;
}
int32 MarketStoreEditing::Pick(const FStoreTemplate& S,FVector Origin,FVector Direction)
{
    double Best=1.e30;int32 Result=INDEX_NONE;
    for(int32 I=0;I<S.Fixtures.Num();++I)
    {
        const auto& F=S.Fixtures[I];const auto D=MarketPlanogram::Equipment(F.EquipmentId).DimensionsCm;
        const auto Rotation=FRotator(0,F.Yaw,0);const FVector O=Rotation.UnrotateVector(Origin-F.Location),V=Rotation.UnrotateVector(Direction);
        const FVector Min(-D.X/2,-D.Y/2,0),Max(D.X/2,D.Y/2,D.Z);double Near=0,Far=1.e30;bool Hit=true;
        for(int32 Axis=0;Axis<3;++Axis){if(FMath::Abs(V[Axis])<1.e-9){if(O[Axis]<Min[Axis]||O[Axis]>Max[Axis])Hit=false;}else{double A=(Min[Axis]-O[Axis])/V[Axis],B=(Max[Axis]-O[Axis])/V[Axis];if(A>B)Swap(A,B);Near=FMath::Max(Near,A);Far=FMath::Min(Far,B);}}
        if(Hit&&Far>=Near&&Near<Best){Best=Near;Result=I;}
    }
    return Result;
}
bool MarketStoreEditing::Create(const FString& Format,const FString& Id,const FString& Name,FStoreTemplate& Out)
{
    double W=1400,D=1200,H=310,BD=220;
    if(Format==TEXT("kucuk")){W=2200;D=1900;H=360;BD=260;}
    else if(Format==TEXT("buyuk")){W=4000;D=3000;H=600;BD=400;}
    else if(Format==TEXT("hiper")){W=8000;D=5000;H=800;BD=500;}
    else if(Format!=TEXT("mahalle"))return false;
    if(!Id.StartsWith(Format+TEXT("_"))||Name.TrimStartAndEnd().IsEmpty())return false;
    FStoreTemplate S;S.Id=Id;S.Name=Name;S.Format=Format;S.Theme=TEXT("aydinlik");S.bEditableShell=true;S.FootprintCm=FVector2D(W,D);S.CeilingCm=H;
    S.Shell=TEXT("/Game/Stores/Shells/SM_Shell_")+Format+TEXT("_01");S.Roof=TEXT("/Game/Stores/Shells/SM_Roof_")+Format+TEXT("_01");
    S.Outline={FVector2D(-W/2,-D/2),FVector2D(W/2,-D/2),FVector2D(W/2,D/2),FVector2D(-W/2,D/2)};
    S.Backroom=FBox(FVector(-W/2,D/2-BD,0),FVector(W/2,D/2,H));S.BackroomM2=W*BD/10000;S.SalesAreaM2=W*D/10000-S.BackroomM2;
    S.Entrance.At=FVector(0,-D/2,0);S.Receiving.At=FVector(0,D/2,0);S.PlayerStart.At=FVector(0,-D/2+180,100);S.PlayerStart.Yaw=90;S.CustomerSpawn={FVector(0,-D/2-180,100)};
    Out=MoveTemp(S);return true;
}
bool MarketStoreEditing::Duplicate(FStoreTemplate& S,int32 Index,float Gap,int32& New)
{
    if(!S.Fixtures.IsValidIndex(Index))return false;auto F=S.Fixtures[Index];const auto H=HalfSize(F);
    for(const FVector Offset:{FVector(H.X*2+Gap,0,0),FVector(-H.X*2-Gap,0,0),FVector(0,H.Y*2+Gap,0),FVector(0,-H.Y*2-Gap,0)})
    {auto Candidate=F;Candidate.Location+=Offset;if(CanPlace(S,Candidate)){int32 N=1;do{Candidate.Id=FString::Printf(TEXT("editor_%04d"),N++);}while(S.Fixtures.ContainsByPredicate([&](const auto& O){return O.Id==Candidate.Id;})||S.Obstacles.ContainsByPredicate([&](const auto& O){return O.Id==Candidate.Id;}));New=S.Fixtures.Add(Candidate);return true;}}
    return false;
}
bool MarketStoreEditing::Resize(FStoreTemplate& S,double W,double D,double BW,double BD,double H,FString& Error)
{
    if(!FMath::IsFinite(W)||!FMath::IsFinite(D)||!FMath::IsFinite(BW)||!FMath::IsFinite(BD)||!FMath::IsFinite(H)||W<500||D<500||BW<100||BD<100||BW>W||BD>D-300||H<250||H>1500){Error=TEXT("Ge\u00e7ersiz \u00f6l\u00e7\u00fc. Depo ma\u011fazadan k\u00fc\u00e7\u00fck olmal\u0131.");return false;}
    const double SX=W/S.FootprintCm.X,SY=D/S.FootprintCm.Y;
    if(S.Outline.IsEmpty())S.Outline={FVector2D(-S.FootprintCm.X/2,-S.FootprintCm.Y/2),FVector2D(S.FootprintCm.X/2,-S.FootprintCm.Y/2),S.FootprintCm/2,FVector2D(-S.FootprintCm.X/2,S.FootprintCm.Y/2)};
    for(auto& V:S.Outline){V.X*=SX;V.Y*=SY;}
    for(auto& O:S.Obstacles){O.At.X*=SX;O.At.Y*=SY;O.Size.Z=H;}
    for(auto& F:S.Fixtures){F.Location.X*=SX;F.Location.Y*=SY;}
    for(auto& V:S.Sections){V.At.X*=SX;V.At.Y*=SY;V.At.Z=FMath::Min(double(V.At.Z),H-55);}
    for(auto& P:S.Props){P.At.X*=SX;P.At.Y*=SY;}
    for(auto& P:S.CustomerSpawn){P.X*=SX;P.Y*=SY;}
    S.Entrance.At.X*=SX;S.Entrance.At.Y*=SY;S.Receiving.At.X=0;S.Receiving.At.Y=D/2;S.PlayerStart.At.X*=SX;S.PlayerStart.At.Y*=SY;
    S.FootprintCm=FVector2D(W,D);S.Backroom=FBox(FVector(-BW/2,D/2-BD,0),FVector(BW/2,D/2,H));S.CeilingCm=H;S.BackroomM2=BW*BD/10000;
    double Area=0;for(int32 I=0;I<S.Outline.Num();++I){const auto A=S.Outline[I],B=S.Outline[(I+1)%S.Outline.Num()];Area+=A.X*B.Y-B.X*A.Y;}S.SalesAreaM2=FMath::Abs(Area)/20000-S.BackroomM2;
    S.bEditableShell=true;S.Stats=MarketStoreKit::CalculateStats(S);return true;
}
bool MarketStoreEditing::Save(const FStoreTemplate& S,const FString& Path,FString& Error)
{
    // Drafts may be incomplete while designing. The published catalogue retains the full store contract.
    const bool Draft=!FPaths::IsSamePath(Path,FPaths::ProjectConfigDir()/TEXT("magazalar.json"));
    TArray<FString> Errors;if(!Draft&&!MarketStoreKit::Validate(S,Errors)){Error=FString::Join(Errors,TEXT("\n"));return false;}
    for(int32 I=0;I<S.Fixtures.Num();++I)if(!CanPlace(S,S.Fixtures[I],I)){Error=TEXT("Yerle\u015fim \u00e7ak\u0131\u015f\u0131yor veya ma\u011faza d\u0131\u015f\u0131na ta\u015f\u0131yor: ")+S.Fixtures[I].Id;return false;}
    FString Text;TSharedPtr<FJsonObject> Root;
    if(FFileHelper::LoadFileToString(Text,*Path))FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text),Root);
    if(!Root.IsValid()){if(!Draft){Error=TEXT("Ma\u011faza katalo\u011fu okunamad\u0131; \u00fczerine yaz\u0131lmad\u0131.");return false;}Root=MakeShared<FJsonObject>();Root->SetNumberField(TEXT("schemaVersion"),1);}
    TArray<TSharedPtr<FJsonValue>> Stores;const TArray<TSharedPtr<FJsonValue>>* Existing=nullptr;if(Root->TryGetArrayField(TEXT("stores"),Existing))Stores=*Existing;
    TSharedPtr<FJsonObject> Target;for(auto V:Stores)if(V->AsObject()->GetStringField(TEXT("id"))==S.Id)Target=V->AsObject();
    if(!Target.IsValid()){Target=MakeShared<FJsonObject>();Stores.Add(MakeShared<FJsonValueObject>(Target));}
    StoreEdit::Patch(S,Target);Root->SetArrayField(TEXT("stores"),Stores);
    FString Output;FJsonSerializer::Serialize(Root.ToSharedRef(),TJsonWriterFactory<>::Create(&Output));
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path),true);
    const FString Temp=Path+TEXT(".tmp");if(!FFileHelper::SaveStringToFile(Output,*Temp,FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM)){Error=TEXT("Dosya yaz\u0131lamad\u0131.");return false;}
    if(IFileManager::Get().FileExists(*Path)){const FString Backup=FPaths::ProjectSavedDir()/TEXT("StoreBackups")/(FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))+TEXT("_")+FPaths::GetCleanFilename(Path));IFileManager::Get().MakeDirectory(*FPaths::GetPath(Backup),true);if(IFileManager::Get().Copy(*Backup,*Path)!=COPY_OK){Error=TEXT("Yedek al\u0131namad\u0131.");return false;}}
    if(!IFileManager::Get().Move(*Path,*Temp,true,true)){Error=TEXT("Dosya de\u011fi\u015ftirilemedi.");return false;}return true;
}
bool MarketStoreEditing::LoadDraft(const FString& Path,FStoreTemplate& S,FString& Error)
{
    FString Json;TArray<FStoreTemplate> Stores;TArray<FString> Errors;
    if(!FFileHelper::LoadFileToString(Json,*Path)){Error=TEXT("Taslak bulunamad\u0131.");return false;}
    MarketStoreKit::Parse(Json,Stores,Errors);if(Stores.Num()!=1){Error=TEXT("Taslak okunamad\u0131.");return false;}S=Stores[0];return true;
}

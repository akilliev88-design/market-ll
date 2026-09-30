#include "MarketStoreKit.h"
#include "MarketGame.h"
#include "MarketWorldText.h"
#include "MarketVisuals.h"
#include "ProductCatalog.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/RectLightComponent.h"
#include "Engine/RectLight.h"
#include "Engine/StaticMesh.h"
#include "PhysicsEngine/BodySetup.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace StoreBuild
{
    const FName Tag(TEXT("MirasStoreKit"));
    AActor* Holder(UWorld* World)
    {
        auto* A=World->SpawnActor<AActor>(); A->Tags.Add(Tag);
        auto* Root=NewObject<USceneComponent>(A); A->SetRootComponent(Root); Root->RegisterComponent(); return A;
    }
    UHierarchicalInstancedStaticMeshComponent* Instances(AActor* A,UStaticMesh* Mesh,bool Collision)
    {
        auto* C=NewObject<UHierarchicalInstancedStaticMeshComponent>(A); C->SetupAttachment(A->GetRootComponent()); C->SetMobility(EComponentMobility::Movable);
        C->SetStaticMesh(Mesh); C->SetCollisionEnabled(Collision?ECollisionEnabled::QueryAndPhysics:ECollisionEnabled::NoCollision);
        C->SetCollisionResponseToAllChannels(ECR_Block); C->RegisterComponent(); return C;
    }
    UTextRenderComponent* Text(UWorld* W,const FVector& At,float Yaw,const FString& Value,float Size=10)
    {
        auto* A=Holder(W); auto* C=NewObject<UTextRenderComponent>(A); C->SetupAttachment(A->GetRootComponent()); C->RegisterComponent();
        C->SetWorldLocationAndRotation(At,FRotator(0,Yaw,0)); MarketWorldText::Apply(C); C->SetText(FText::FromString(Value));
        C->SetWorldSize(Size); C->SetHorizontalAlignment(EHTA_Center); C->SetVerticalAlignment(EVRTA_TextCenter); C->SetCullDistance(3500);
        return C;
    }
}
void MarketStoreKit::Clear(UWorld* World)
{
    if(!World) return;
    for(TActorIterator<AActor> It(World);It;++It) if(It->ActorHasTag(StoreBuild::Tag)) It->Destroy();
    if(auto* Game=World->GetAuthGameMode<AMarketGameMode>()) { Game->CategorySigns.Reset(); Game->CategorySignKeys.Reset(); Game->ActiveStoreKitId.Empty(); Game->StoreCategoryOverrides.Reset(); }
}
bool MarketStoreKit::Build(UWorld* World,const FStoreTemplate& S,const FMarketPlanogram& Filled)
{
    if(!World) return false;
    TArray<FString> Errors; if(!Validate(S,Errors)) return false;
    UStaticMesh* Shell=LoadObject<UStaticMesh>(nullptr,*S.Shell); if(!Shell) { UE_LOG(LogTemp,Error,TEXT("Store shell missing: %s"),*S.Shell); return false; }
    if(auto* Body=Shell->GetBodySetup())
    {
        UE_LOG(LogTemp,Display,TEXT("Store shell collision: %d convex, %d boxes"),Body->AggGeom.ConvexElems.Num(),Body->AggGeom.BoxElems.Num());
        if(!Body->AggGeom.ConvexElems.IsEmpty()) { UE_LOG(LogTemp,Display,TEXT("Store first hull: %s"),*Body->AggGeom.ConvexElems[0].ElemBox.ToString()); }
    }
    TMap<FString,UStaticMesh*> EquipmentMeshes;
    for(const auto& F:S.Fixtures)
    {
        if(EquipmentMeshes.Contains(F.EquipmentId)) continue;
        const auto E=MarketPlanogram::Equipment(F.EquipmentId); UStaticMesh* Mesh=LoadObject<UStaticMesh>(nullptr,*E.MeshPath);
        if(!Mesh) { UE_LOG(LogTemp,Error,TEXT("Store equipment missing: %s"),*E.MeshPath); return false; }
        EquipmentMeshes.Add(F.EquipmentId,Mesh);
    }
    Clear(World); auto* A=StoreBuild::Holder(World);
    auto* ShellComponent=NewObject<UStaticMeshComponent>(A); ShellComponent->SetupAttachment(A->GetRootComponent()); ShellComponent->SetStaticMesh(Shell); ShellComponent->SetCollisionProfileName(TEXT("BlockAll")); ShellComponent->RegisterComponent(); ShellComponent->SetRelativeRotation(FRotator(0,180,0));
    FHitResult FloorHit;
    const bool Floor=World->LineTraceSingleByChannel(FloorHit,S.PlayerStart.At+FVector(0,0,50),S.PlayerStart.At-FVector(0,0,200),ECC_Visibility);
    UE_LOG(LogTemp,Display,TEXT("Store floor trace: %d at %s, actor %s"),Floor,*FloorHit.ImpactPoint.ToString(),*GetNameSafe(FloorHit.GetActor()));
    if(auto* Game=World->GetAuthGameMode<AMarketGameMode>())
    {
        Game->ActiveStoreKitId=S.Id; Game->Planogram=Filled;
        for(const auto& F:Filled.Fixtures)
        {
            if(const auto* Original=S.Fixtures.FindByPredicate([&](const auto& V){return V.Id==F.Id;}))
                if(Original->Category!=F.Category) Game->StoreCategoryOverrides.Add(F.Id,F.Category);
            for(const auto& Pair:F.FaceCategories) Game->StoreCategoryOverrides.Add(F.Id+TEXT("/")+Pair.Key,Pair.Value);
        }
    }
    TMap<FString,UHierarchicalInstancedStaticMeshComponent*> Groups;
    for(const auto& Pair:EquipmentMeshes) Groups.Add(Pair.Key,StoreBuild::Instances(A,Pair.Value,true));
    auto* SignBoards=StoreBuild::Instances(A,LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")),false);
    SignBoards->SetMaterial(0,MarketVisuals::CreateSurface(A,EMarketSurface::SignRed));
    for(const auto& F:S.Fixtures)
    {
        const auto E=MarketPlanogram::Equipment(F.EquipmentId);
        Groups[F.EquipmentId]->AddInstance(FTransform(FRotator(0,F.Yaw+E.MeshYaw,0),F.Location));
        const auto* Planned=Filled.FindFixture(F.Id);
        if(!Planned||E.Levels==0) continue;
        const FTransform Xf(FRotator(0,F.Yaw,0),F.Location);
        // Opaque board separates the two gondola faces; the text material itself is two-sided.
        SignBoards->AddInstance(FTransform(FRotator(0,F.Yaw,0),Xf.TransformPosition(FVector(0,E.bSignOnTop?0:E.SignY,E.SignZ)),FVector(E.SignWidthCm/100,.025f,.22f)));
        for(int32 Side=0;Side<(E.bDoubleSided?2:1);++Side)
        {
            const FString Face=Side==0?TEXT("front"):TEXT("back"); const auto C=Planned->CategoryForFace(Face);
            if(C.IsEmpty()) continue;
            auto* Sign=StoreBuild::Text(World,Xf.TransformPosition(FVector(0,(E.bSignOnTop?0:E.SignY)+(Side==0?-2.f:2.f),E.SignZ)),F.Yaw+(Side==0?-90:90),MarketCatalog::UpperTurkish(C),S.Format==TEXT("hiper")?15:10);
            if(auto* Game=World->GetAuthGameMode<AMarketGameMode>()) { Game->CategorySigns.Add(Sign); Game->CategorySignKeys.Add(F.Id+TEXT("/")+Face); }
        }
    }
    for(const auto& P:S.Props)
        if(auto* Mesh=LoadObject<UStaticMesh>(nullptr,*P.Mesh)) StoreBuild::Instances(A,Mesh,true)->AddInstance(FTransform(FRotator(0,P.Yaw,0),P.At));
    TArray<FMarketProduct> Products; MarketCatalog::LoadFile(MarketCatalog::DefaultPath(),Products,Errors);
    int32 Count=0;
    for(const auto& P:Products)
    {
        TArray<const FPlanogramPlacement*> Blocks; for(const auto& B:Filled.Placements) if(B.ProductId==P.Id) Blocks.Add(&B);
        if(Blocks.IsEmpty()) continue;
        UStaticMesh* Mesh=P.MeshPath.IsEmpty()?nullptr:LoadObject<UStaticMesh>(nullptr,*P.MeshPath);
        const bool Fallback=Mesh==nullptr;
        if(Fallback) Mesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));
        auto* C=StoreBuild::Instances(A,Mesh,false);
        FVector Scale=Fallback?FVector(MarketPlanogram::NominalDepthCm(P)/100,MarketPlanogram::NominalWidthCm(P)/100,MarketPlanogram::NominalHeightCm(P)/100):FVector(P.VisualScale);
        if(Fallback)
        {
            auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Materials/M_Market.M_Market"));
            auto* M=UMaterialInstanceDynamic::Create(Base,A); M->SetVectorParameterValue(TEXT("Color"),FLinearColor(P.Color)); C->SetMaterial(0,M);
        }
        else for(int32 Slot=0;Slot<Mesh->GetStaticMaterials().Num();++Slot)
        {
            UMaterialInterface* M=P.Materials.IsValidIndex(Slot)&&!P.Materials[Slot].IsEmpty()?LoadObject<UMaterialInterface>(nullptr,*P.Materials[Slot]):Mesh->GetMaterial(Slot);
            C->SetMaterial(Slot,MarketVisuals::CreatePackageSurface(A,M,Mesh->GetStaticMaterials()[Slot].MaterialSlotName.ToString()));
        }
        for(const auto* B:Blocks)
        {
            const auto* F=Filled.FindFixture(B->FixtureId); if(!F) continue;
            const auto E=MarketPlanogram::Equipment(F->EquipmentId); if(B->Level<0||B->Level>=E.Levels) continue;
            const FRotator Display=B->Orientation==1?FRotator(0,90,0):B->Orientation==2?FRotator(0,0,90):FRotator::ZeroRotator;
            const FQuat Model=Display.Quaternion()*(Fallback?FQuat::Identity:P.VisualRotation.Quaternion());
            const FBox Bounds=Mesh->GetBoundingBox().TransformBy(FTransform(Model,FVector::ZeroVector,Scale));
            const FVector Size=Bounds.GetSize(); const float FaceSign=B->Face==TEXT("back")?1:-1;
            const FRotator Facing(0,FaceSign*90,0);
            const FVector Offset=Facing.RotateVector(FVector(Bounds.GetCenter().X,Bounds.GetCenter().Y,0))+FVector(0,0,Bounds.Min.Z);
            const FTransform Xf(FRotator(0,F->Yaw,0),F->Location);
            const float Center=MarketPlanogram::PlacementCenterX(Filled,Products,*B);
            const float Pitch=FMath::Max(Size.Y,MarketPlanogram::OrientedWidthCm(P,B->Orientation))+MarketPlanogram::ItemGapCm;
            const int32 Rows=FMath::Min(B->Depth,FMath::Max(1,FMath::FloorToInt((E.UsableDepthCm+2)/(Size.X+2))));
            for(int32 Row=0;Row<Rows;++Row) for(int32 Col=0;Col<B->Facings;++Col) for(int32 Stack=0;Stack<MarketPlanogram::EffectiveStack(Filled,P,*B);++Stack)
            {
                const FVector Local(Center+(Col-(B->Facings-1)*.5f)*Pitch,-FaceSign*E.FrontY-FaceSign*(Size.X*.5f+Row*(Size.X+2)),E.LevelTopZ[B->Level]+Stack*(Size.Z+.6f));
                C->AddInstance(FTransform(Facing.Quaternion()*Model,Local-Offset+Facing.RotateVector(P.VisualOffsetCm),Scale)*Xf); ++Count;
            }
        }
    }
    // One theme preset, shadow-free area lights: emissive service strips supply Lumen's indirect light.
    const int32 Temperature=S.Theme==TEXT("sicak_ahsap")?3800:S.Theme==TEXT("dogal_yesil")?4400:S.Theme==TEXT("aydinlik")?5000:5700;
    const float Spacing=S.Format==TEXT("hiper")?1200:600;
    for(float X=-S.FootprintCm.X/2+Spacing/2;X<S.FootprintCm.X/2;X+=Spacing)
        for(float Y=-S.FootprintCm.Y/2+Spacing/2;Y<S.FootprintCm.Y/2;Y+=Spacing)
        {
            auto* L=World->SpawnActor<ARectLight>(FVector(X,Y,S.CeilingCm-40),FRotator(-90,0,0)); L->Tags.Add(StoreBuild::Tag);
            auto* R=Cast<URectLightComponent>(L->GetLightComponent()); R->SetMobility(EComponentMobility::Movable); R->SetUseTemperature(true); R->SetTemperature(Temperature);
            R->SetSourceWidth(Spacing*.7f); R->SetSourceHeight(Spacing*.7f); R->SetIntensity(Spacing==1200?30000:9000); R->SetAttenuationRadius(Spacing*1.6f); R->SetCastShadows(false);
        }
    StoreBuild::Text(World,S.Entrance.At+FVector(0,15,240),90,TEXT("\u00c7IKI\u015e"),22);
    StoreBuild::Text(World,FVector(0,S.Backroom.Min.Y-15,230),-90,TEXT("DEPO / MAL KABUL"),18);
    UE_LOG(LogTemp,Display,TEXT("MirasStoreKit built %s: %d fixtures, %d product instances"),*S.Id,S.Fixtures.Num(),Count);
    return true;
}

#include "MarketStoreKit.h"
#include "MarketStoreDressing.h"
#include "MarketTelevisionDisplay.h"
#include "MarketStoreEditing.h"
#include "MarketGame.h"
#include "MarketBranchVisit.h"
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
    const FName Tag(TEXT("SimStoreKit"));
    UMaterialInstanceDynamic* CeilingSurface(UObject* Outer,const FLinearColor& Color,float Metal=0)
    {
        auto* Base=LoadObject<UMaterialInterface>(nullptr,MarketVisuals::SurfaceMasterPath);
        if(!Base)return nullptr;
        auto* M=UMaterialInstanceDynamic::Create(Base,Outer);
        M->SetVectorParameterValue(TEXT("Color"),Color);
        M->SetScalarParameterValue(TEXT("UseTexture"),0);M->SetScalarParameterValue(TEXT("Wear"),0);
        M->SetScalarParameterValue(TEXT("Roughness"),.75f);M->SetScalarParameterValue(TEXT("Metallic"),Metal);
        return M;
    }
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
bool MarketStoreKit::Build(UWorld* World,const FStoreTemplate& S,const FMarketPlanogram& Filled,bool bDesignPreview)
{
    if(!World) return false;
    TArray<FString> Errors; if(!bDesignPreview&&!Validate(S,Errors)) return false;
    UStaticMesh* Shell=S.bEditableShell?nullptr:LoadObject<UStaticMesh>(nullptr,*S.Shell); if(!S.bEditableShell&&!Shell) { UE_LOG(LogTemp,Error,TEXT("Store shell missing: %s"),*S.Shell); return false; }
    UStaticMesh* Roof=S.bEditableShell||S.Roof.IsEmpty()?nullptr:LoadObject<UStaticMesh>(nullptr,*S.Roof);
    if(!S.bEditableShell&&!S.Roof.IsEmpty()&&!Roof) { UE_LOG(LogTemp,Error,TEXT("Store roof missing: %s"),*S.Roof); return false; }
    if(Shell) if(auto* Body=Shell->GetBodySetup())
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
    if(S.bEditableShell) MarketStoreEditing::BuildArchitecture(World,S);
    else {auto* ShellComponent=NewObject<UStaticMeshComponent>(A); ShellComponent->SetupAttachment(A->GetRootComponent()); ShellComponent->SetStaticMesh(Shell); ShellComponent->SetCollisionProfileName(TEXT("BlockAll")); ShellComponent->RegisterComponent(); ShellComponent->SetRelativeRotation(FRotator(0,180,0));}
    if(Roof)
    {
        auto* RoofActor=StoreBuild::Holder(World); RoofActor->Tags.Add(TEXT("SimStoreRoof"));
        auto* C=NewObject<UStaticMeshComponent>(RoofActor); C->SetupAttachment(RoofActor->GetRootComponent()); C->SetStaticMesh(Roof); C->SetCollisionProfileName(TEXT("BlockAll")); C->RegisterComponent(); C->SetRelativeRotation(FRotator(0,180,0));
        if(auto* Gray=StoreBuild::CeilingSurface(RoofActor,FLinearColor(.62f,.65f,.67f)))
            for(int32 Slot=0;Slot<C->GetNumMaterials();++Slot)C->SetMaterial(Slot,Gray);
    }
    FHitResult FloorHit;
    if(S.Format==TEXT("buyuk")||S.Format==TEXT("hiper"))
    {
        if(auto* Services=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Environment/StoreKit/CeilingBay_6000/SM_CeilingBay_6000.SM_CeilingBay_6000")))
        {
            auto* CeilingActor=StoreBuild::Holder(World); CeilingActor->Tags.Add(TEXT("SimStoreRoof"));
            auto* Bays=StoreBuild::Instances(CeilingActor,Services,false);
            const auto& Slots=Services->GetStaticMaterials();
            for(int32 Slot=0;Slot<Slots.Num();++Slot)
            {
                const FString Name=Slots[Slot].MaterialSlotName.ToString();
                if(Name.Contains(TEXT("BlackSteel"))||Name.Contains(TEXT("Galvanized")))
                    if(auto* Gray=StoreBuild::CeilingSurface(CeilingActor,Name.Contains(TEXT("BlackSteel"))?FLinearColor(.32f,.35f,.37f):FLinearColor(.45f,.48f,.50f),.35f))Bays->SetMaterial(Slot,Gray);
            }
            for(float X=-S.FootprintCm.X*.5f+300;X<=S.FootprintCm.X*.5f-300;X+=600)
                for(float Y=-S.FootprintCm.Y*.5f+300;Y<=S.FootprintCm.Y*.5f-300;Y+=600)
                    Bays->AddInstance(FTransform(FRotator(0,180,0),FVector(X,Y,S.CeilingCm-15)));
        }
    }
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
    const FLinearColor SignColor=S.Theme==TEXT("sicak_ahsap")?FLinearColor(.19,.105,.055):S.Theme==TEXT("aydinlik")?FLinearColor(.16,.20,.22):S.Theme==TEXT("dogal_yesil")?FLinearColor(.055,.15,.095):FLinearColor(.055,.09,.12);
    auto* SignBase=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Materials/M_Market.M_Market"));
    auto* SignMaterial=UMaterialInstanceDynamic::Create(SignBase,A); SignMaterial->SetVectorParameterValue(TEXT("Color"),SignColor); SignBoards->SetMaterial(0,SignMaterial);
    for(const auto& Section:S.Sections)
    {
        const FRotator Facing(0,Section.Yaw,0);
        SignBoards->AddInstance(FTransform(FRotator(0,Section.Yaw+90,0),Section.At-Facing.Vector()*3,FVector(Section.WidthCm/100,.05f,.48f)));
        const float Size=FMath::Clamp(Section.WidthCm/FMath::Max(1,Section.Label.Len())*1.35f,16.f,40.f);
        StoreBuild::Text(World,Section.At,Section.Yaw,MarketCatalog::UpperTurkish(Section.Label),Size)->SetCullDistance(16000);
    }
    for(const auto& F:S.Fixtures)
    {
        const auto E=MarketPlanogram::Equipment(F.EquipmentId);
        Groups[F.EquipmentId]->AddInstance(FTransform(FRotator(0,F.Yaw+E.MeshYaw,0),F.Location));
        const auto* Planned=Filled.FindFixture(F.Id);
        if(!Planned||E.Levels==0||!E.bShowCategorySign) continue;
        const FTransform Xf(FRotator(0,F.Yaw,0),F.Location);
        // Opaque board separates the two gondola faces; the text material itself is two-sided.
        if (E.bSignOnTop) SignBoards->AddInstance(FTransform(FRotator(0,F.Yaw,0),Xf.TransformPosition(FVector(0,0,E.SignZ)),FVector(E.SignWidthCm/100,.025f,.22f)));
        for(int32 Side=0;Side<(E.bDoubleSided?2:1);++Side)
        {
            const FString Face=Side==0?TEXT("front"):TEXT("back"); const auto C=Planned->CategoryForFace(Face);
            if(C.IsEmpty() && !MarketTelevisionDisplay::IsDisplay(F.EquipmentId)) continue;
            const float FaceY = E.bSignOnTop ? 0.f : (Side == 0 ? E.SignY : -E.SignY);
            if (!E.bSignOnTop) SignBoards->AddInstance(FTransform(FRotator(0,F.Yaw,0),Xf.TransformPosition(FVector(0,FaceY,E.SignZ)),FVector(E.SignWidthCm/100,.025f,.22f)));
            auto* Sign=StoreBuild::Text(World,Xf.TransformPosition(FVector(0,FaceY+(Side==0?-2.f:2.f),E.SignZ)),F.Yaw+(Side==0?-90:90),MarketCatalog::UpperTurkish(C.IsEmpty()?TEXT("Kategorisiz"):C),S.Format==TEXT("hiper")?15:10);
            if(auto* Game=World->GetAuthGameMode<AMarketGameMode>()) { Game->CategorySigns.Add(Sign); Game->CategorySignKeys.Add(F.Id+TEXT("/")+Face); }
        }
    }
    for(const auto& P:S.Props)
        if(auto* Mesh=LoadObject<UStaticMesh>(nullptr,*P.Mesh)) StoreBuild::Instances(A,Mesh,true)->AddInstance(FTransform(FRotator(0,P.Yaw,0),P.At));
    TArray<FMarketProduct> Products; MarketCatalog::LoadFile(MarketCatalog::DefaultPath(),Products,Errors);
    int32 Count=0;
    const auto TVProfiles = MarketTelevisionDisplay::Load();
    for(const auto& P:Products)
    {
        TArray<const FPlanogramPlacement*> Blocks; for(const auto& B:Filled.Placements) if(B.ProductId==P.Id) Blocks.Add(&B);
        if(Blocks.IsEmpty()) continue;
        UStaticMesh* Mesh=P.MeshPath.IsEmpty()?nullptr:LoadObject<UStaticMesh>(nullptr,*P.MeshPath);
        const bool Fallback=Mesh==nullptr;
        if(Fallback) Mesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));
        auto* C=StoreBuild::Instances(A,Mesh,false); C->ComponentTags.Add(FName(*(TEXT("SimProduct:")+P.Id)));
        FVector Scale=Fallback?FVector(MarketPlanogram::NominalDepthCm(P)/100,MarketPlanogram::NominalWidthCm(P)/100,MarketPlanogram::NominalHeightCm(P)/100):FVector(P.VisualScale);
        if(Fallback)
        {
            auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Materials/M_Market.M_Market"));
            auto* M=UMaterialInstanceDynamic::Create(Base,A);
            // Unfinished packages use restrained paper colours; the catalogue's colour still distinguishes them.
            M->SetVectorParameterValue(TEXT("Color"),FLinearColor(P.Color)*.22f+FLinearColor(.60f,.56f,.47f)*.78f); C->SetMaterial(0,M);
        }
        else for(int32 Slot=0;Slot<Mesh->GetStaticMaterials().Num();++Slot)
        {
            UMaterialInterface* M=P.Materials.IsValidIndex(Slot)&&!P.Materials[Slot].IsEmpty()?LoadObject<UMaterialInterface>(nullptr,*P.Materials[Slot]):Mesh->GetMaterial(Slot);
            C->SetMaterial(Slot,MarketVisuals::CreatePackageSurface(A,M,Mesh->GetStaticMaterials()[Slot].MaterialSlotName.ToString()));
        }
        int32 ProductTotal = 0, ProductDrawn = 0;
        const auto* VisitGame = World->GetAuthGameMode<AMarketGameMode>();
        const bool Visiting = VisitGame && VisitGame->IsBranchVisit() && VisitGame->State.Branches.IsValidIndex(VisitGame->BranchVisitIndex);
        if (Visiting) for (const auto* Block : Blocks)
        {
            const auto* Fixture = Filled.FindFixture(Block->FixtureId); if (!Fixture) continue;
            const auto Equipment = MarketPlanogram::Equipment(Fixture->EquipmentId);
            if (Block->Level < 0 || Block->Level >= Equipment.Levels) continue;
            const FRotator DisplayRotation = Block->Orientation == 1 ? FRotator(0,90,0) : Block->Orientation == 2 ? FRotator(0,0,90) : FRotator::ZeroRotator;
            const FQuat ModelRotation = DisplayRotation.Quaternion() * (Fallback ? FQuat::Identity : P.VisualRotation.Quaternion());
            const FVector ModelSize = Mesh->GetBoundingBox().TransformBy(FTransform(ModelRotation,FVector::ZeroVector,Scale)).GetSize();
            const int32 VisualDepth = FMath::Min(Block->Depth,FMath::Max(1,FMath::FloorToInt((Equipment.UsableDepthCm+2)/(ModelSize.X+2))));
            ProductTotal += VisualDepth * Block->Facings * MarketPlanogram::EffectiveStack(Filled,P,*Block);
        }
        const int32 ProductLimit = Visiting ? FMath::FloorToInt(ProductTotal * MarketBranchVisit::Fill(VisitGame->State.Branches[VisitGame->BranchVisitIndex],P.Id)) : MAX_int32;
        for(const auto* B:Blocks)
        {
            const auto* F=Filled.FindFixture(B->FixtureId); if(!F) continue;
            const auto E=MarketPlanogram::Equipment(F->EquipmentId); if(B->Level<0||B->Level>=E.Levels) continue;
            TArray<AActor*> TVActors;
            const auto* TVGame = World->GetAuthGameMode<AMarketGameMode>();
            const FString TVName = TVGame && TVGame->State.bRealBrands ? P.RealName : P.FictionalName;
            MarketTelevisionDisplay::Decorate(World, Filled, Products, *B, TVName, TVProfiles.Find(P.Id), TVActors);
            for (auto* TVActor : TVActors) TVActor->Tags.Add(StoreBuild::Tag);
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
                if (ProductDrawn++ >= ProductLimit) continue;
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
            // Upward fixture spill lights the gray ceiling above the luminaires.
            auto* Up=World->SpawnActor<ARectLight>(FVector(X,Y,S.CeilingCm-80),FRotator(90,0,0));Up->Tags.Add(StoreBuild::Tag);
            auto* U=Cast<URectLightComponent>(Up->GetLightComponent());U->SetMobility(EComponentMobility::Movable);U->SetIntensityUnits(ELightUnits::Lumens);
            U->SetIntensity(Spacing==1200?16000:6000);U->SetSourceWidth(Spacing*.6f);U->SetSourceHeight(Spacing*.6f);U->SetAttenuationRadius(Spacing);U->SetCastShadows(false);
        }
    StoreBuild::Text(World,S.Entrance.At+FVector(0,15,240),90,TEXT("\u00c7IKI\u015e"),22);
    StoreBuild::Text(World,FVector(0,S.Backroom.Min.Y-15,230),-90,TEXT("DEPO / MAL KABUL"),18);
    MarketStoreDressing::Build(World,S);
    UE_LOG(LogTemp,Display,TEXT("SimStoreKit built %s: %d fixtures, %d product instances"),*S.Id,S.Fixtures.Num(),Count);
    return true;
}

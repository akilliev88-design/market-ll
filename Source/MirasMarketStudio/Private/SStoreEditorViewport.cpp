#include "SStoreEditorViewport.h"
#include "SStoreStudio.h"
#include "MarketStoreEditing.h"
#include "ProductCatalog.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "SceneManagement.h"
#include "UnrealClient.h"
#include "SceneView.h"

class FStoreEditorViewportClient : public FStudioViewportClient
{
public:
    SStoreEditorViewport* Widget;
    bool Drag=false,bMoved=false;
    FVector Offset=FVector::ZeroVector;
    FStoreEditorViewportClient(TSharedRef<FPreviewScene> S,TSharedRef<SStoreEditorViewport> W):FStudioViewportClient(S,W),Widget(&W.Get()){}
    bool Floor(FVector& At)
    {
        const auto Ray=GetCursorWorldLocationFromMousePos();if(FMath::Abs(Ray.GetDirection().Z)<.001)return false;
        const double T=-Ray.GetOrigin().Z/Ray.GetDirection().Z;if(T<0||T>100000)return false;At=Ray.GetOrigin()+Ray.GetDirection()*T;At.Z=0;return true;
    }
    virtual bool InputKey(const FInputKeyEventArgs& A) override
    {
        auto* O=Widget->Owner;
        if(A.Key==EKeys::LeftMouseButton)
        {
            FVector At;
            if(A.Event==IE_Pressed)
            {
                if(!O->ArmedEquipment.IsEmpty()){if(Floor(At))O->AddAt(At);return true;}
                const auto Ray=GetCursorWorldLocationFromMousePos();O->Select(MarketStoreEditing::Pick(O->Store,Ray.GetOrigin(),Ray.GetDirection()));
                if(O->Store.Fixtures.IsValidIndex(O->Selected)&&Floor(At)){Offset=O->Store.Fixtures[O->Selected].Location-At;Drag=true;bMoved=false;}
            }
            else if(A.Event==IE_Released&&Drag){Drag=false;if(bMoved)O->EndDrag();Invalidate();}
            return true;
        }
        if(A.Event==IE_Pressed)
        {
            const bool Ctrl=A.Viewport->KeyState(EKeys::LeftControl)||A.Viewport->KeyState(EKeys::RightControl);
            if(A.Key==EKeys::Escape){Drag=false;O->Action(TEXT("select"));return true;}
            if(A.Key==EKeys::Delete){O->Action(TEXT("delete"));return true;}
            if(A.Key==EKeys::R){O->Action(TEXT("rotate"));return true;}
            if(A.Key==EKeys::F){Widget->Focus(true);return true;}
            if(A.Key==EKeys::Home){Widget->Focus();return true;}
            if(Ctrl&&A.Key==EKeys::Z){O->Action(TEXT("undo"));return true;}
            if(Ctrl&&A.Key==EKeys::Y){O->Action(TEXT("redo"));return true;}
            if(Ctrl&&A.Key==EKeys::D){O->Action(TEXT("duplicate"));return true;}
            if(Ctrl&&A.Key==EKeys::S){O->Action(TEXT("draft"));return true;}
            if(A.Key==EKeys::F11){O->Action(TEXT("workspace"));return true;}
            if(A.Key==EKeys::One){O->Action(TEXT("view:0"));return true;}
            if(A.Key==EKeys::Two){O->Action(TEXT("view:1"));return true;}
            if(A.Key==EKeys::Three){O->Action(TEXT("view:2"));return true;}
        }
        if(Widget->bWalk&&(A.Key==EKeys::W||A.Key==EKeys::A||A.Key==EKeys::S||A.Key==EKeys::D||A.Key==EKeys::Q||A.Key==EKeys::E))return true;
        return FStudioViewportClient::InputKey(A);
    }
    void Move()
    {
        if(!Drag)return;FVector At;if(Floor(At)){if(!bMoved){Widget->Owner->Remember();bMoved=true;}if(Widget->Owner->MoveSelected(At+Offset))Widget->SyncSelected();Invalidate();}
    }
    virtual void MouseMove(FViewport* V,int32 X,int32 Y)override{FStudioViewportClient::MouseMove(V,X,Y);Move();Invalidate();}
    virtual void CapturedMouseMove(FViewport* V,int32 X,int32 Y)override{if(Drag)Move();else FStudioViewportClient::CapturedMouseMove(V,X,Y);Invalidate();}
    virtual bool InputAxis(const FInputKeyEventArgs& A)override{if(Drag)return true;return FStudioViewportClient::InputAxis(A);}
    FVector WalkDestination(FVector Before,FVector Desired)
    {
        Desired.Z=165;FHitResult Hit;const FVector From(Before.X,Before.Y,90),To(Desired.X,Desired.Y,90);
        if(Widget->Scene->GetWorld()->SweepSingleByChannel(Hit,From,To,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(25)))return FVector(Before.X,Before.Y,165);
        return Desired;
    }
    virtual void Tick(float Dt)override
    {
        const FVector Before=GetViewLocation();FStudioViewportClient::Tick(Dt);
        if(!Widget->bWalk||!Viewport||!Viewport->HasFocus())return;
        FVector Forward=FRotator(0,GetViewRotation().Yaw,0).Vector(),Right=FRotator(0,GetViewRotation().Yaw+90,0).Vector();
        FVector Move=Forward*(int(Viewport->KeyState(EKeys::W))-int(Viewport->KeyState(EKeys::S)))+Right*(int(Viewport->KeyState(EKeys::D))-int(Viewport->KeyState(EKeys::A)));
        FVector At=Before+Move.GetSafeNormal()*Dt*(Viewport->KeyState(EKeys::LeftShift)?500:220);At.Z=165;
        SetViewLocation(WalkDestination(Before,At));
    }
    virtual void Draw(const FSceneView* View,FPrimitiveDrawInterface* PDI)override
    {
        FStudioViewportClient::Draw(View,PDI);auto* O=Widget->Owner;
        auto Outline=[&](const FPlanogramFixture& F,FLinearColor Color){const FVector D=MarketPlanogram::Equipment(F.EquipmentId).DimensionsCm;DrawWireBox(PDI,FTransform(FRotator(0,F.Yaw,0),F.Location).ToMatrixWithScale(),FBox(FVector(-D.X/2,-D.Y/2,1),FVector(D.X/2,D.Y/2,D.Z+2)),Color,SDPG_Foreground,2);};
        if(O->Store.Fixtures.IsValidIndex(O->Selected))Outline(O->Store.Fixtures[O->Selected],FLinearColor(1,.75,0));
        if(!O->ArmedEquipment.IsEmpty()){FVector At;if(Floor(At)){const auto F=O->Placement(At);Outline(F,MarketStoreEditing::CanPlace(O->Store,F)?FLinearColor(.2,1,.4):FLinearColor(1,.15,.1));}}
    }
};
void SStoreEditorViewport::Construct(const FArguments& A){Owner=A._Owner;Scene=MakeShared<FPreviewScene>(FPreviewScene::ConstructionValues());Scene->SetLightBrightness(3);Scene->SetSkyBrightness(1.5f);SEditorViewport::Construct(SEditorViewport::FArguments());}
SStoreEditorViewport::~SStoreEditorViewport(){if(Client){Client->Viewport=nullptr;Client->Widget=nullptr;}}
TSharedRef<FEditorViewportClient> SStoreEditorViewport::MakeEditorViewportClient(){Client=MakeShared<FStoreEditorViewportClient>(Scene.ToSharedRef(),SharedThis(this));Client->ViewFOV=65;Client->ExposureSettings.bFixed=true;Client->ExposureSettings.FixedEV100=1;Client->EngineShowFlags.SetBillboardSprites(false);Client->SetViewModes(VMI_Unlit,VMI_Unlit);return Client.ToSharedRef();}
void SStoreEditorViewport::SetLighting(bool Lit){Client->SetViewModes(Lit?VMI_Lit:VMI_Unlit,Lit?VMI_Lit:VMI_Unlit);Client->Invalidate();}
void SStoreEditorViewport::SyncSelected()
{
    if(!Owner->Store.Fixtures.IsValidIndex(Owner->Selected))return;const auto& F=Owner->Store.Fixtures[Owner->Selected];const auto E=MarketPlanogram::Equipment(F.EquipmentId);int32 Instance=0;
    for(int32 I=0;I<Owner->Selected;++I)if(Owner->Store.Fixtures[I].EquipmentId==F.EquipmentId)++Instance;
    auto* Mesh=LoadObject<UStaticMesh>(nullptr,*E.MeshPath);
    for(TActorIterator<AActor> A(Scene->GetWorld());A;++A)if(!A->IsActorBeingDestroyed())
    {TArray<UHierarchicalInstancedStaticMeshComponent*> Components;A->GetComponents(Components);for(auto* C:Components)if(C->GetStaticMesh()==Mesh){C->UpdateInstanceTransform(Instance,FTransform(FRotator(0,F.Yaw+E.MeshYaw,0),F.Location),false,true,true);return;}}
}
void SStoreEditorViewport::Show(const FStoreTemplate& S,bool Fill)
{
    if(LastId!=S.Id)bFilled=false;if(Fill){bFilled=true;FillSeed=FMath::Rand();}
    auto Plan=MarketStoreKit::ToPlanogram(S);if(bFilled){TArray<FMarketProduct> P;TArray<FString> Errors;MarketCatalog::LoadFile(MarketCatalog::DefaultPath(),P,Errors);P.RemoveAll([](auto V){return !V.bActive;});MarketStoreKit::FillRandom(Plan,P,FillSeed);}
    MarketStoreKit::Build(Scene->GetWorld(),S,Plan,true);
    for(TActorIterator<AActor> A(Scene->GetWorld());A;++A)if(A->ActorHasTag(TEXT("MirasStoreRoof"))){A->SetIsTemporarilyHiddenInEditor(!bWalk);TArray<UPrimitiveComponent*> Components;A->GetComponents(Components);for(auto* C:Components)C->SetVisibility(bWalk,true);}
    if(LastId!=S.Id){LastId=S.Id;Focus();}Client->Invalidate();
}
void SStoreEditorViewport::SetWalk(bool Walk){if(bWalk==Walk)return;bWalk=Walk;for(TActorIterator<AActor> A(Scene->GetWorld());A;++A)if(A->ActorHasTag(TEXT("MirasStoreRoof"))){A->SetIsTemporarilyHiddenInEditor(!Walk);TArray<UPrimitiveComponent*> Components;A->GetComponents(Components);for(auto* C:Components)C->SetVisibility(Walk,true);}Focus();}
void SStoreEditorViewport::Focus(bool Selection)
{
    const auto& S=Owner->Store;
    if(bWalk){FVector At=S.PlayerStart.At;At.Z=165;if(Selection&&S.Fixtures.IsValidIndex(Owner->Selected)){const auto F=S.Fixtures[Owner->Selected];At=F.Location-FRotator(0,F.Yaw+90,0).Vector()*250;At.Z=165;Client->SetViewRotation((F.Location+FVector(0,0,100)-At).Rotation());}else Client->SetViewRotation(FRotator(0,S.PlayerStart.Yaw,0));Client->SetViewLocation(At);}
    else
    {
        const auto Size=GetCachedGeometry().GetLocalSize();const float Aspect=Size.Y>1?Size.X/Size.Y:1.5f;
        const double VerticalHalf=FMath::Atan(FMath::Tan(FMath::DegreesToRadians(65.f/2))/FMath::Max(1.f,Aspect));
        FVector Focus(0,0,S.CeilingCm/3);double Radius=FVector(S.FootprintCm.X/2,S.FootprintCm.Y/2,S.CeilingCm/2).Size();
        if(Selection&&S.Fixtures.IsValidIndex(Owner->Selected)){const auto F=S.Fixtures[Owner->Selected];Focus=F.Location+FVector(0,0,100);Radius=MarketPlanogram::Equipment(F.EquipmentId).DimensionsCm.Size()*.65;}
        const FVector At=Focus+FVector(.55,-.65,.75).GetSafeNormal()*(Radius/FMath::Sin(VerticalHalf)*.8);Client->SetViewLocation(At);Client->SetViewRotation((Focus-At).Rotation());
    }
    Client->Invalidate();
}
bool SStoreEditorViewport::ReviewInteraction(FString& Error)
{
    if(!Client->Viewport||Owner->Store.Fixtures.IsEmpty()){Error=TEXT("Missing viewport or equipment");return false;}
    auto* V=Client->Viewport;const int32 Index=0;const auto Before=Owner->Store.Fixtures[Index];
    FSceneViewFamilyContext Family(FSceneViewFamily::ConstructionValues(V,Scene->GetScene(),Client->EngineShowFlags).SetRealtimeUpdate(true));auto* View=Client->CalcSceneView(&Family);
    auto Mouse=[&](FVector At){FVector2D Pixel;if(!View->WorldToPixel(At,Pixel))return false;V->SetMouse(FMath::RoundToInt(Pixel.X),FMath::RoundToInt(Pixel.Y));return true;};
    auto Key=[&](EInputEvent Event){FInputKeyEventArgs Args;Args.Viewport=V;Args.Key=EKeys::LeftMouseButton;Args.Event=Event;Client->InputKey(Args);};
    Owner->Action(TEXT("select"));Mouse(Before.Location+FVector(0,0,MarketPlanogram::Equipment(Before.EquipmentId).DimensionsCm.Z*.5));Key(IE_Pressed);
    if(Owner->Selected!=Index){Error=TEXT("3D click did not select fixture");return false;}
    FVector Ground;if(!Client->Floor(Ground)){Error=TEXT("Cannot project ground");return false;}
    FVector Delta(500,-500,0);auto Candidate=Before;Candidate.Location+=Delta;
    if(!MarketStoreEditing::CanPlace(Owner->Store,Candidate,Index)){Error=TEXT("Review destination obstructed");return false;}
    const auto Camera=Client->GetViewLocation();Mouse(Ground+Delta);Client->Move();Key(IE_Released);
    if(Owner->Store.Fixtures[Index].Location.Equals(Before.Location,1)||!Client->GetViewLocation().Equals(Camera,.01)){Error=TEXT("Drag failed or camera jumped");return false;}
    Owner->Action(TEXT("undo"));
    const int32 Count=Owner->Store.Fixtures.Num();Owner->Action(TEXT("arm:tech_table_1800"));Mouse(FVector(0,0,0));Key(IE_Pressed);Key(IE_Released);
    if(Owner->Store.Fixtures.Num()!=Count+1){Error=TEXT("3D click placement failed");return false;}
    Owner->Action(TEXT("select"));Owner->Action(TEXT("undo"));
    const FVector From=Before.Location+FVector(0,-300,165),To=Before.Location+FVector(0,0,165);
    if(!Client->WalkDestination(From,To).Equals(From,.01)){Error=TEXT("Walking passed through fixture");return false;}
    const FVector Free(0,-800,165);if(!Client->WalkDestination(Free,Free+FVector(80,0,0)).Equals(Free+FVector(80,0,0),.01)){Error=TEXT("Walking on empty floor blocked");return false;}
    return true;
}

#include "MarketStoreDressing.h"
#include "MarketStoreKit.h"
#include "ProductCatalog.h"
#include "Engine/StaticMesh.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void MarketStoreDressing::FillPreview(FMarketPlanogram& Plan, const TArray<FMarketProduct>& Products)
{
    Plan.Placements.Reset();
    for(const auto& F:Plan.Fixtures)
    {
        const auto E=MarketPlanogram::Equipment(F.EquipmentId);
        for(int32 Side=0;Side<(E.bDoubleSided?2:1);++Side)
        {
            const FString Face=Side==0?TEXT("front"):TEXT("back");
            TArray<const FMarketProduct*> Items;
            for(const auto& P:Products)if(P.bActive&&P.Category==F.CategoryForFace(Face)&&!P.MeshPath.IsEmpty()&&!P.Materials.IsEmpty())Items.Add(&P);
            if(Items.IsEmpty())continue;
            for(int32 Level=0;Level<E.Levels;++Level)
            {
                float Cursor=-E.UsableWidthCm*.5f+1;int32 Pick=Level+Side;
                for(int32 Attempt=0;Attempt<60&&Cursor<E.UsableWidthCm*.5f-2;++Attempt)
                {
                    const auto& P=*Items[Pick++%Items.Num()];const float Width=MarketPlanogram::NominalWidthCm(P);
                    if(E.LevelClearanceCm[Level]>0&&MarketPlanogram::NominalHeightCm(P)>E.LevelClearanceCm[Level])continue;
                    const int32 Facings=FMath::Min(3,FMath::FloorToInt((E.UsableWidthCm*.5f-Cursor-1)/(Width+MarketPlanogram::ItemGapCm)));
                    if(Facings<1)continue;
                    const float Span=Facings*Width+(Facings-1)*MarketPlanogram::ItemGapCm;
                    FPlanogramPlacement B;B.ProductId=P.Id;B.FixtureId=F.Id;B.Face=Face;B.Level=Level;B.Facings=Facings;
                    B.Depth=FMath::Clamp(FMath::FloorToInt(E.UsableDepthCm/(MarketPlanogram::NominalDepthCm(P)+2)),1,4);
                    B.bHasX=true;B.XCm=Cursor+Span*.5f;Plan.Placements.Add(B);Cursor+=Span+.7f;
                }
            }
        }
    }
}

void MarketStoreDressing::Build(UWorld* World, const FStoreTemplate& Store)
{
    const TCHAR* Produce[] = {TEXT("apple_red"),TEXT("apple_green"),TEXT("orange"),TEXT("lemon"),TEXT("tomato"),TEXT("potato"),TEXT("onion"),TEXT("pepper_red")};
    TMap<FString,UHierarchicalInstancedStaticMeshComponent*> Groups;
    AActor* Holder = World->SpawnActor<AActor>(); Holder->Tags.Add(TEXT("MirasStoreKit"));
    auto* Root = NewObject<USceneComponent>(Holder); Holder->SetRootComponent(Root); Root->RegisterComponent();
    int32 Bin = 0, Count = 0;
    for (const auto& F : Store.Fixtures)
    {
        if (MarketPlanogram::Equipment(F.EquipmentId).Family != TEXT("produce")) continue;
        FString Json;
        if (!FFileHelper::LoadFileToString(Json, *(FPaths::ProjectDir()/TEXT("AssetInbox/Environment/Stores/Desktop")/F.EquipmentId/TEXT("equipment.json")))) continue;
        TSharedPtr<FJsonObject> Info; FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Info);
        const TArray<TSharedPtr<FJsonValue>>* Zones = nullptr;
        if (!Info.IsValid() || !Info->TryGetArrayField(TEXT("fillZones"), Zones)) continue;
        const FTransform Fixture(FRotator(0,F.Yaw,0),F.Location);
        for (const auto& Entry : *Zones)
        {
            const auto Zone = Entry->AsObject(); const FString Id = Produce[Bin++ % UE_ARRAY_COUNT(Produce)];
            auto* C = Groups.FindRef(Id);
            if (!C)
            {
                const FString Path = TEXT("/Game/Stores/Desktop/")+Id+TEXT("/SM_")+Id+TEXT(".SM_")+Id;
                auto* Mesh = LoadObject<UStaticMesh>(nullptr,*Path); if (!Mesh) continue;
                C = NewObject<UHierarchicalInstancedStaticMeshComponent>(Holder); C->SetupAttachment(Root);
                C->SetMobility(EComponentMobility::Movable); C->SetStaticMesh(Mesh);
                C->SetCollisionEnabled(ECollisionEnabled::NoCollision); C->SetSimulatePhysics(false); C->RegisterComponent(); Groups.Add(Id,C);
            }
            const auto Center = Zone->GetArrayField(TEXT("centerMetres"));
            const auto Size = Zone->GetArrayField(TEXT("innerSizeMetres"));
            const auto Angles = Zone->GetArrayField(TEXT("rotationEulerRadians"));
            const FQuat Tilt(FVector::ForwardVector,Angles[0]->AsNumber());
            const FVector At(-Center[0]->AsNumber()*100,Center[1]->AsNumber()*100,Center[2]->AsNumber()*100);
            const double Floor = Zone->GetNumberField(TEXT("floorTopLocalZ"))*100;
            const FVector Bounds = C->GetStaticMesh()->GetBoundingBox().GetSize();
            const double SX = FMath::Max(7.,Bounds.X+1), SY = FMath::Max(7.,Bounds.Y+1);
            const int32 NX = FMath::Clamp(FMath::FloorToInt(Size[0]->AsNumber()*100/SX),1,7);
            const int32 NY = FMath::Clamp(FMath::FloorToInt(Size[1]->AsNumber()*100/SY),1,7);
            for (int32 X=0;X<NX;++X) for(int32 Y=0;Y<NY;++Y)
            {
                const FVector Local = At+Tilt.RotateVector(FVector((X-(NX-1)*.5)*SX,(Y-(NY-1)*.5)*SY,Floor+.12));
                const FQuat Facing=FRotator(0,180+(X*13+Y*7)%25,0).Quaternion();
                C->AddInstance(FTransform(Fixture.GetRotation()*Tilt*Facing,Fixture.TransformPosition(Local)));
                ++Count;
            }
        }
    }
    UE_LOG(LogTemp,Display,TEXT("STORE_PRODUCE_DRESSING: %s %d authored produce instances; visual only"),*Store.Id,Count);
}

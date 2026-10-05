#include "MarketLargeStoreTrial.h"
#include "MarketGame.h"
#include "MarketStoreDressing.h"
#include "MarketStoreWalkAudit.h"
#include "ProductCatalog.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/RectLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/RectLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/PostProcessVolume.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
#if WITH_EDITOR
#include "ShaderCompiler.h"
#endif
AMarketLargeStoreTrialGameMode::AMarketLargeStoreTrialGameMode()
{
    PrimaryActorTick.bCanEverTick=true;DefaultPawnClass=AMarketCharacter::StaticClass();HUDClass=nullptr;
}
void AMarketLargeStoreTrialGameMode::InitGame(const FString& Map,const FString& Options,FString& Error)
{
    Super::InitGame(Map,Options,Error);TArray<FString> Errors;FString Id=TEXT("buyuk_01");FParse::Value(FCommandLine::Get(),TEXT("LargeStore="),Id);
    if (!MarketStoreKit::Load(Errors) || !MarketStoreKit::Find(Id)) { Error=TEXT("Large store layout missing");return; }
    Store=*MarketStoreKit::Find(Id);
    GetWorld()->SpawnActor<APlayerStart>(Store.PlayerStart.At,FRotator(0,90,0));
}
void AMarketLargeStoreTrialGameMode::BeginPlay()
{
    Super::BeginPlay();TArray<FMarketProduct> Products;TArray<FString> Errors;
    if(!MarketCatalog::LoadFile(MarketCatalog::DefaultPath(),Products,Errors))return;
    auto Plan=MarketStoreKit::ToPlanogram(Store);MarketStoreDressing::FillPreview(Plan,Products);
    if(!MarketStoreKit::Build(GetWorld(),Store,Plan)) { UE_LOG(LogTemp,Error,TEXT("LARGE_STORE_FAILED: build"));FPlatformMisc::RequestExitWithStatus(false,1);return; }
    auto* Sun=GetWorld()->SpawnActor<ADirectionalLight>(FVector(0,0,1000),FRotator(-48,32,0));
    auto* D=Cast<UDirectionalLightComponent>(Sun->GetLightComponent());D->SetMobility(EComponentMobility::Movable);D->SetIntensity(1.6f);D->SetAtmosphereSunLight(true);
    auto* Atmosphere=GetWorld()->SpawnActor<ASkyAtmosphere>();Atmosphere->GetComponent()->SetSkyLuminanceFactor(FLinearColor(2000,2000,2000));
    auto* Sky=GetWorld()->SpawnActor<ASkyLight>();Sky->GetLightComponent()->SetMobility(EComponentMobility::Movable);Sky->GetLightComponent()->SetRealTimeCaptureEnabled(true);Sky->GetLightComponent()->SetIntensity(.4f);
    const float W=Store.FootprintCm.X, Depth=Store.FootprintCm.Y;
    for(TActorIterator<ARectLight> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("MirasStoreKit"))&&It->GetActorRotation().Pitch<0)It->Destroy();
    for(float X=-W/2+300;X<W/2;X+=600)for(float Y=-Depth/2+300;Y<Depth/2;Y+=600)
    {
        auto* A=GetWorld()->SpawnActor<ARectLight>(FVector(X,Y,Store.CeilingCm-55),FRotator(-90,0,0));
        auto* L=Cast<URectLightComponent>(A->GetLightComponent());L->SetMobility(EComponentMobility::Movable);L->SetIntensityUnits(ELightUnits::Lumens);
        L->SetIntensity(Store.CeilingCm>500?19000:8000);L->SetSourceWidth(250);L->SetSourceHeight(45);L->SetAttenuationRadius(1400);L->SetCastShadows(false);L->SetUseTemperature(true);L->SetTemperature(5000);
    }
    auto* Facade=GetWorld()->SpawnActor<ARectLight>(FVector(Store.Entrance.At.X,-Depth/2-600,300),FRotator(-15,90,0));
    auto* R=Cast<URectLightComponent>(Facade->GetLightComponent());R->SetMobility(EComponentMobility::Movable);R->SetIntensityUnits(ELightUnits::Lumens);R->SetIntensity(20000);R->SetSourceWidth(W*.6);R->SetSourceHeight(220);R->SetAttenuationRadius(W);
    auto* PP=GetWorld()->SpawnActor<APostProcessVolume>();PP->bUnbound=true;auto& P=PP->Settings;
    P.bOverride_AutoExposureMinBrightness=true;P.AutoExposureMinBrightness=7.5f;P.bOverride_AutoExposureMaxBrightness=true;P.AutoExposureMaxBrightness=7.5f;
    P.bOverride_BloomIntensity=true;P.BloomIntensity=.06f;P.bOverride_VignetteIntensity=true;P.VignetteIntensity=0;
    Capture=FParse::Param(FCommandLine::Get(),TEXT("LargeStoreCapture"));Camera=GetWorld()->SpawnActor<ACameraActor>();Camera->GetCameraComponent()->SetFieldOfView(75);
    if(Capture)View(0);
    for(TActorIterator<AActor> It(GetWorld());It;++It) if(It->ActorHasTag(TEXT("MirasStoreKit")))
    {
        TArray<UStaticMeshComponent*> Components;It->GetComponents(Components);
        for(auto* C:Components) Transforms.Add(C,C->GetComponentTransform());
    }
    Started=FPlatformTime::Seconds();Ready=true;
    if(GEngine&&GEngine->GameViewport)GEngine->GameViewport->ConsoleCommand(TEXT("r.SetRes 1600x900w"));
    EdgeCapture=FParse::Param(FCommandLine::Get(),TEXT("StoreEdgeCapture"));
    if(EdgeCapture)
    {
        Capture=true;
        EdgeLabel=TEXT("review");FParse::Value(FCommandLine::Get(),TEXT("EdgeLabel="),EdgeLabel);
        for(const auto& F:Store.Fixtures)if(F.EquipmentId==TEXT("cooler_wall"))
        {
            const FTransform Xf(FRotator(0,F.Yaw,0),F.Location);
            EdgeEye=Xf.TransformPosition(FVector(-155,-310,150));EdgeTarget=Xf.TransformPosition(FVector(0,0,135));break;
        }
        Camera->SetActorLocationAndRotation(EdgeEye,(EdgeTarget-EdgeEye).Rotation());
        GetWorld()->GetFirstPlayerController()->SetViewTarget(Camera);
    }
    UE_LOG(LogTemp,Display,TEXT("LARGE_STORE_READY: %s %d fixtures %d product blocks"),*Store.Id,Store.Fixtures.Num(),Plan.Placements.Num());
}
void AMarketLargeStoreTrialGameMode::View(int32 Index)
{
    const float W=Store.FootprintCm.X,D=Store.FootprintCm.Y;
    FVector Fresh(-W*.3,-D*.2,0),Checkout(-W*.25,-D*.4,0);
    for(const auto& F:Store.Fixtures) if(MarketPlanogram::Equipment(F.EquipmentId).Family==TEXT("produce")){Fresh=F.Location;break;}
    for(const auto& F:Store.Fixtures) if(MarketPlanogram::Equipment(F.EquipmentId).CheckoutCount>0){Checkout=F.Location;break;}
    for(const auto& F:Store.Fixtures) if(F.EquipmentId==TEXT("checkout_dark_compact")){Checkout=F.Location;break;}
    const FVector Eyes[]={FVector(Store.Entrance.At.X+W*.23,-D/2-750,270),Store.PlayerStart.At+FVector(0,0,70),Fresh+FVector(230,-220,160),Checkout+FVector(175,200,160)};
    const FVector Targets[]={FVector(Store.Entrance.At.X,-D/2,170),FVector(Store.Entrance.At.X,0,140),Fresh+FVector(0,0,100),Checkout+FVector(0,0,130)};
    const int32 V=FMath::Min(Index,3);FVector Eye=Eyes[V];
    if(V>=2)
    {
        Eye.X=FMath::Clamp(Eye.X,-W/2+65,W/2-65);
        Eye.Y=FMath::Clamp(Eye.Y,-D/2+65,Store.Backroom.Min.Y-65);
    }
    if(Index==6)Eye.X-=25;else if(Index==7)Eye.X+=25;
    Camera->SetActorLocationAndRotation(Eye,(Targets[V]-Eye).Rotation());
    GetWorld()->GetFirstPlayerController()->SetViewTarget(Camera);
}
void AMarketLargeStoreTrialGameMode::Tick(float Dt)
{
    Super::Tick(Dt);if(!Ready)return;auto* PC=GetWorld()->GetFirstPlayerController();if(!PC)return;
    if(PC->WasInputKeyJustPressed(EKeys::Escape))FPlatformMisc::RequestExit(false);
    if(!Capture)
    {
        if(PC->WasInputKeyJustPressed(EKeys::One))View(0);
        if(PC->WasInputKeyJustPressed(EKeys::Two))View(1);
        if(PC->WasInputKeyJustPressed(EKeys::Three))View(2);
        if(PC->WasInputKeyJustPressed(EKeys::Four))View(3);
        if(PC->WasInputKeyJustPressed(EKeys::Zero))PC->SetViewTarget(PC->GetPawn());
        return;
    }
#if WITH_EDITOR
    if(GShaderCompilingManager&&GShaderCompilingManager->IsCompiling()){Started=FPlatformTime::Seconds();return;}
#endif
    if(EdgeCapture)
    {
        if(FPlatformTime::Seconds()-Started<3)return;
        if(EdgeFrame>=80){UE_LOG(LogTemp,Display,TEXT("STORE_EDGE_CAPTURE_PASSED: %s 40 fixed and 40 continuous moving frames"),*EdgeLabel);FPlatformMisc::RequestExit(false);return;}
        FVector Eye=EdgeEye;
        if(EdgeFrame>=40)Eye.X+=(EdgeFrame-40)*.5f;
        Camera->SetActorLocationAndRotation(Eye,(EdgeTarget-Eye).Rotation());
        const FString Dir=FPaths::ProjectSavedDir()/TEXT("Screenshots/StoreEdges")/EdgeLabel;IFileManager::Get().MakeDirectory(*Dir,true);
        FScreenshotRequest::RequestScreenshot(Dir/FString::Printf(TEXT("%03d.png"),EdgeFrame),false,false);++EdgeFrame;
        return;
    }
    // Six seconds for initial grounding; subsequent settled camera stages need
    // two seconds. Shader compilation still resets the timer above.
    if(FPlatformTime::Seconds()-Started<(Stage==0?6.:2.))return;
    if(!Checked)
    {
        auto* Walker=Cast<ACharacter>(PC->GetPawn());FCollisionQueryParams Q;if(Walker)Q.AddIgnoredActor(Walker);FHitResult Hit;
        const float X=Store.Entrance.At.X,Y=Store.Entrance.At.Y;
        const bool Blocked=GetWorld()->SweepSingleByChannel(Hit,FVector(X,Y-120,91),FVector(X,Y+180,91),FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,88),Q);
        const bool Ground=Walker&&Walker->GetCharacterMovement()->IsMovingOnGround();
        if(!Ground||Blocked){UE_LOG(LogTemp,Error,TEXT("LARGE_STORE_FAILED: floor=%d entryBlocked=%d"),Ground,Blocked);FPlatformMisc::RequestExitWithStatus(false,1);return;}
        if(!AuditStoreSalesFaces(GetWorld(),Store,Walker)){FPlatformMisc::RequestExitWithStatus(false,1);return;}
        Checked=true;UE_LOG(LogTemp,Display,TEXT("LARGE_STORE_WALK_PASSED: %s"),*Store.Id);
    }
    for(const auto& Pair:Transforms)if(Pair.Key.IsValid())
    {
        auto* C=Cast<UStaticMeshComponent>(Pair.Key.Get());
        if(!C->GetComponentTransform().Equals(Pair.Value,.0001)||C->IsSimulatingPhysics()){UE_LOG(LogTemp,Error,TEXT("LARGE_STORE_FAILED: static fixture moved or simulates"));FPlatformMisc::RequestExitWithStatus(false,1);return;}
    }
    if(Stage>=16){UE_LOG(LogTemp,Display,TEXT("LARGE_STORE_PASSED: %s; walk, static transforms, eight views"),*Store.Id);FPlatformMisc::RequestExit(false);return;}
    const FString Dir=FPaths::ProjectSavedDir()/TEXT("Screenshots/LargeStores")/Store.Id;IFileManager::Get().MakeDirectory(*Dir,true);
    if(Stage%2==0)FScreenshotRequest::RequestScreenshot(Dir/FString::Printf(TEXT("%02d.png"),Stage/2+1),false,false);
    else View((Stage+1)/2);
    ++Stage;Started=FPlatformTime::Seconds();
}

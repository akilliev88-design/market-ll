// Automated runs started from the command line, kept out of the game loop:
// -SimSmoke   (SmokeTest.ps1): player, restock, order, hiring, customer sales, day close, save/load.
// -SimCapture (visual review): five 1280x720 screenshots once shaders and exposure have settled.

#include "MarketGame.h"
#include "MarketStaff.h"
#include "MarketStoreKit.h"
#include "ProductCatalog.h"
#include "HAL/FileManager.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"
#include "Components/TextRenderComponent.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "Engine/RectLight.h"
#include "Components/RectLightComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#if WITH_EDITOR
#include "ShaderCompiler.h"
#endif

bool AMarketGameMode::TickAutomation()
{
    if (!StorePreviewId.IsEmpty())
    {
        const bool Benchmark = FParse::Param(FCommandLine::Get(), TEXT("SimStoreBenchmark"));
        const float Now = GetWorld()->GetTimeSeconds();
        const FStoreTemplate* Store = MarketStoreKit::Find(StorePreviewId);
        if (CaptureStage == 0)
        {
            TArray<FString> Errors;
            if (!MarketStoreKit::Load(Errors) || !(Store = MarketStoreKit::Find(StorePreviewId)))
            {
                for (const auto& Error : Errors) { UE_LOG(LogTemp, Error, TEXT("Store preview: %s"), *Error); }
                FPlatformMisc::RequestExitWithStatus(false, 1); return false;
            }
            Planogram = MarketStoreKit::ToPlanogram(*Store);
            MarketStoreKit::Fill(Planogram, Products);
            if (!MarketStoreKit::Build(GetWorld(), *Store, Planogram)) { FPlatformMisc::RequestExitWithStatus(false, 1); return false; }
            if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0)) Pawn->SetActorLocation(Store->PlayerStart.At, false, nullptr, ETeleportType::TeleportPhysics);
            StorePreviewCamera = GetWorld()->SpawnActor<ACameraActor>();
            // A neutral studio fill lets the fifth view inspect the facade without a surrounding map.
            auto* FacadeLight = GetWorld()->SpawnActor<ARectLight>(FVector(0,-Store->FootprintCm.Y*.5f-350,Store->CeilingCm*.6f),FRotator(0,90,0));
            auto* FacadeFill = Cast<URectLightComponent>(FacadeLight->GetLightComponent());
            FacadeFill->SetMobility(EComponentMobility::Movable);
            FacadeFill->SetSourceWidth(Store->FootprintCm.X*.7f); FacadeFill->SetSourceHeight(Store->CeilingCm*.7f);
            FacadeFill->SetIntensity(60000); FacadeFill->SetAttenuationRadius(Store->FootprintCm.X); FacadeFill->SetCastShadows(false);
            if (auto* PC = UGameplayStatics::GetPlayerController(this, 0)) PC->SetViewTarget(StorePreviewCamera);
            if (GEngine && GEngine->GameViewport) GEngine->GameViewport->ConsoleCommand(Benchmark ? TEXT("r.SetRes 1920x1080w") : TEXT("r.SetRes 1280x720w"));
            IFileManager::Get().MakeDirectory(*(FPaths::ProjectSavedDir()/TEXT("Screenshots/Stores")/StorePreviewId), true);
            CaptureStage = 1; CaptureAt = Now; CaptureReadySince = -1;
        }
        if (!Store) return false;
        if (CaptureStage == 1 && Now > CaptureAt + 3)
            if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
                if (Pawn->GetActorLocation().Z < 50) { UE_LOG(LogTemp, Error, TEXT("Store preview floor collision failed: %s, pawn %s"), *StorePreviewId, *Pawn->GetActorLocation().ToString()); FPlatformMisc::RequestExitWithStatus(false, 1); return false; }
        if (CaptureStage >= 2 && CaptureStage % 2 == 0 && Now > CaptureAt + 1)
        { StoreFrameSeconds += GetWorld()->GetDeltaSeconds(); ++StoreFrameCount; }
        auto SetView = [&](int32 Index)
        {
            const float W = Store->FootprintCm.X, D = Store->FootprintCm.Y;
            FVector Checkout=FVector(-W*.25f,-D*.4f,0),Fresh=FVector(-W*.3f,-D*.2f,0),Service=FVector(-W*.3f,D*.3f,0);
            for(const auto& F:Store->Fixtures)
            {
                const auto E=MarketPlanogram::Equipment(F.EquipmentId);
                if(E.Family==TEXT("checkout")) { Checkout=F.Location; break; }
            }
            for(const auto& F:Store->Fixtures) if(MarketPlanogram::Equipment(F.EquipmentId).Family==TEXT("produce")) { Fresh=F.Location; break; }
            if(Store->Format==TEXT("kucuk")) for(const auto& F:Store->Fixtures) if(MarketPlanogram::Equipment(F.EquipmentId).Family==TEXT("pallet")) { Fresh=F.Location; break; }
            if(!Store->Obstacles.IsEmpty()) Service=Store->Obstacles[0].At;
            else for(const auto& F:Store->Fixtures) if(MarketPlanogram::Equipment(F.EquipmentId).Family==TEXT("deli")) { Service=F.Location; break; }
            const FVector FreshEye(FMath::Clamp(Fresh.X+300,-W/2+80,W/2-80),FMath::Max(Fresh.Y-240,-D/2+80),195);
            const FVector Views[] = { FVector(W*.55f,-D*.65f,FMath::Max(W*.58f,float(Store->CeilingCm+450))), Store->PlayerStart.At+FVector(0,50,95), FreshEye, Service+FVector(250,-300,190), FVector(0,0,FMath::Max3(W*.75f,D*1.2f,float(Store->CeilingCm+600))) };
            const FVector Targets[] = { FVector(0,0,40), Checkout+FVector(0,0,105), Fresh+FVector(0,0,95), Service+FVector(0,0,130), FVector(0,0,0) };
            for(TActorIterator<AActor> It(GetWorld());It;++It) if(It->ActorHasTag(TEXT("SimStoreRoof"))) It->SetActorHiddenInGame(Index==0||Index==4);
            StorePreviewCamera->SetActorLocationAndRotation(Views[Index], Index==4?FRotator(-90,-90,0):(Targets[Index]-Views[Index]).Rotation());
        };
        bool ShadersReady = true;
#if WITH_EDITOR
        if (GShaderCompilingManager) ShadersReady = GShaderCompilingManager->GetNumRemainingJobs() == 0;
#endif
        if (!ShadersReady) CaptureReadySince = -1;
        else if (CaptureReadySince < 0) CaptureReadySince = Now;
        if (CaptureStage == 1 && Now - CaptureReadySince > 5 && CaptureReadySince >= 0)
        { SetView(0); CaptureStage = 2; CaptureAt = Now; }
        else if (CaptureStage >= 2 && CaptureStage % 2 == 0 && Now > CaptureAt + 4)
        {
            const int32 Index = (CaptureStage-2)/2;
            if (!Benchmark) FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/Stores")/StorePreviewId/FString::Printf(TEXT("%02d.png"),Index+1),false,false);
            if (StoreFrameSeconds > 0) { UE_LOG(LogTemp,Display,TEXT("SimStorePreview FPS %s view %d (%s): %.1f"),*StorePreviewId,Index+1,Benchmark?TEXT("1920x1080"):TEXT("1280x720"),StoreFrameCount/StoreFrameSeconds); }
            StoreFrameSeconds=0; StoreFrameCount=0;
            ++CaptureStage; CaptureAt=Now;
        }
        else if (CaptureStage >= 3 && CaptureStage % 2 == 1 && Now > CaptureAt + 2)
        {
            const int32 Next = (CaptureStage-1)/2;
            if (Next < 5) { SetView(Next); ++CaptureStage; CaptureAt=Now; }
            else { UE_LOG(LogTemp,Display,TEXT("SimStorePreview PASSED: %s, five views"),*StorePreviewId); FPlatformMisc::RequestExitWithStatus(false,0); }
        }
        return false; // previews have no first-store customer/worker simulation
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("SimSmoke")))
    {
        const auto Require = [](bool Condition, const TCHAR* Step)
        {
            if (!Condition)
            {
                UE_LOG(LogTemp, Error, TEXT("MarketSim smoke FAILED: %s"), Step);
                FPlatformMisc::RequestExitWithStatus(false, 1);
            }
            return Condition;
        };
        if (SmokeStage == 0 && GetWorld()->GetTimeSeconds() > 1)
        {
            auto* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
            if (!Require(Pawn != nullptr, TEXT("player spawned"))) return false;
            Pawn->SetActorLocation(ProductFixtureLocation(0) + FVector(0, 0, 90));
            const FMarketStock Before = State.Stock[0];
            const int32 ExpectedShelf = Before.Shelf + FMath::Max(0, FMath::Min(Before.Capacity - Before.Shelf, Before.Warehouse));
            Command("Interact");
            if (!Require(State.Stock[0].Shelf == ExpectedShelf && State.Stock[0].Shelf + State.Stock[0].Warehouse == Before.Shelf + Before.Warehouse,
                TEXT("nearby shelf interaction"))) return false;
            Pawn->SetActorLocation(FVector(-430, -180, 90));
            // Cases until the wholesaler's minimum order (MarketOrderAdvice::MinimumOrder) is reached.
            const int64 CashBeforeOrder = State.Cash;
            for (int32 Press = 0; Press < MarketOrderAdvice::MaxCases && OrderDraftBill() < MarketOrderAdvice::MinimumOrderOn(State.Day); ++Press) Command("Order");
            const int32 OrderedUnits = (OrderDraftCases.IsValidIndex(0) ? OrderDraftCases[0] : 0) * FMath::Clamp(Products[0].CaseUnits, 1, 48);
            Command("ConfirmOrder");
            if (!Require(OrderedUnits > 0 && State.Stock[0].Incoming == OrderedUnits && State.Cash == CashBeforeOrder - Products[0].Cost * OrderedUnits, TEXT("multi-product office order"))) return false;
            const int64 CashBeforeHire = State.Cash;
            Command("Hire");
            if (!Require(MarketStaff::CashierOnDuty(State) && State.Cash == CashBeforeHire - 12000, TEXT("hire cashier"))) return false;
            // New campaigns deliberately start empty. Use normal warehouse transfers here so the
            // smoke scenario can still exercise customer sales without changing player startup.
            for (int32 ProductIndex = 0; ProductIndex < State.Stock.Num(); ++ProductIndex)
            {
                State.Restock(ProductIndex);
            }
            Command("ToggleShop");
            SmokeStage = 1;
        }
        // G-053 baskets (1-4 shelves per shopper, walking MetaHumans) make the first sale take longer: close the
        // day after the first paid basket (at least 25 s), or give up after 90 s. The frame rate varies per run.
        if (SmokeStage == 1 && ((State.Served > 0 && GetWorld()->GetTimeSeconds() > 25) || GetWorld()->GetTimeSeconds() > 90))
        {
            const int32 ServedToday = State.Served;
            const float OpenFor = GetWorld()->GetTimeSeconds();
            CloseShop();
            UE_LOG(LogTemp, Display, TEXT("MarketSim smoke day close at %.1f s: served %d, lost %d, day %d, product0 incoming %d, dock %d, missing %d, damaged %d."),
                OpenFor, ServedToday, State.LastLost, State.Day, State.Stock[0].Incoming, State.Stock[0].Dock, State.LastDeliveryMissing, State.LastDeliveryDamaged);
            if (!Require(State.Day == 2, TEXT("day close"))) return false;
            if (!Require(State.LastServed > 0, TEXT("customer sale (no shopper paid within 90 s)"))) return false;
            if (!Require(State.Stock[0].Incoming == 0 && State.Stock[0].Dock > 0, TEXT("next-day rear-door delivery"))) return false;
            const int32 DockBefore = State.Stock[0].Dock;
            const int32 WarehouseBefore = State.Stock[0].Warehouse;
            const int32 Received = State.ReceiveDelivery(0, FMath::Clamp(Products[0].CaseUnits, 1, 48));
            if (!Require(Received > 0 && State.Stock[0].Dock == DockBefore - Received && State.Stock[0].Warehouse == WarehouseBefore + Received,
                TEXT("delivery receiving into warehouse"))) return false;
            const int64 SavedCash = State.Cash;
            State.Cash = 0;
            LoadCampaign();
            if (!Require(State.Cash == SavedCash && MarketStaff::CashierOnDuty(State), TEXT("disk save and load"))) return false;
            UE_LOG(LogTemp, Display, TEXT("MarketSim smoke PASSED: player, restock, multi-order, rear-door receiving, hiring, %d customer sales, day close, disk save/load."), State.LastServed);
            SmokeStage = 2;
            FPlatformMisc::RequestExitWithStatus(false, 0);
        }
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("SimCapture")))
    {
        // Waits until shaders are compiled and the image has settled (exposure, Lumen), then saves
        // a clean scene shot and a shot with the HUD for comparison with the reference photo.
        const float Now = GetWorld()->GetTimeSeconds();
        bool bShadersReady = true;
#if WITH_EDITOR
        if (GShaderCompilingManager) bShadersReady = GShaderCompilingManager->GetNumRemainingJobs() == 0;
#endif
        if (!bShadersReady) CaptureReadySince = -1;
        else if (CaptureReadySince < 0) CaptureReadySince = Now;
        const bool bSettled = CaptureReadySince >= 0 && Now - CaptureReadySince > 5;
        // Views for review: entrance, left wall shelves, gondola aisle, bulk island close-up, right wall.
        struct FView { FVector Location; FRotator Rotation; const TCHAR* File; };
        static const FView Views[] =
        {
            { FVector(0, -180, 90), FRotator(-4, 90, 0), TEXT("MarketSim.png") },
            { FVector(-330, 300, 90), FRotator(-6, 165, 0), TEXT("MarketSim_1_duvar_sol.png") },
            { FVector(150, 330, 90), FRotator(-8, 120, 0), TEXT("MarketSim_2_gondol.png") },
            { FVector(110, 120, 90), FRotator(-12, 132, 0), TEXT("MarketSim_3_dokme.png") },
            { FVector(330, 300, 90), FRotator(-6, 15, 0), TEXT("MarketSim_4_duvar_sag.png") },
        };
        constexpr int32 ViewCount = UE_ARRAY_COUNT(Views);
        auto SetView = [this](const FView& View)
        {
            if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0)) Pawn->SetActorLocation(View.Location, false, nullptr, ETeleportType::TeleportPhysics);
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0)) PC->SetControlRotation(View.Rotation);
        };
        const int32 ViewIndex = (CaptureStage - 1) / 2;
        if (CaptureStage == 0 && Now > 8 && (bSettled || Now > 400))
        {
            SetView(Views[0]);
            CaptureStage = 1; CaptureAt = Now;
        }
        else if (CaptureStage > 0 && CaptureStage % 2 == 1 && Now > CaptureAt + 2.5f)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Screenshots") / Views[ViewIndex].File, true, false);
            CaptureStage++; CaptureAt = Now;
        }
        else if (CaptureStage > 0 && CaptureStage % 2 == 0 && Now > CaptureAt + 1.5f)
        {
            if (ViewIndex + 1 < ViewCount) { SetView(Views[ViewIndex + 1]); CaptureStage++; CaptureAt = Now; }
            else FPlatformMisc::RequestExit(false);
        }
    }
    return true;
}

// Automated runs started from the command line, kept out of the game loop:
// -MirasSmoke   (SmokeTest.ps1): player, restock, order, hiring, customer sales, day close, save/load.
// -MirasCapture (visual review): five 1280x720 screenshots once shaders and exposure have settled.

#include "MarketGame.h"
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
    if (FParse::Param(FCommandLine::Get(), TEXT("MirasSmoke")))
    {
        const auto Require = [](bool Condition, const TCHAR* Step)
        {
            if (!Condition)
            {
                UE_LOG(LogTemp, Error, TEXT("MirasMarket smoke FAILED: %s"), Step);
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
            const int32 OrderedUnits = FMath::Clamp(Products[0].CaseUnits, 1, 48);
            const int64 CashBeforeOrder = State.Cash;
            Command("Order");
            if (!Require(State.Stock[0].Incoming == OrderedUnits && State.Cash == CashBeforeOrder - Products[0].Cost * OrderedUnits, TEXT("office order"))) return false;
            const int64 CashBeforeHire = State.Cash;
            Command("Hire");
            if (!Require(State.bCashier && State.Cash == CashBeforeHire - 12000, TEXT("hire cashier"))) return false;
            // New campaigns deliberately start empty. Use normal warehouse transfers here so the
            // smoke scenario can still exercise customer sales without changing player startup.
            for (int32 ProductIndex = 0; ProductIndex < State.Stock.Num(); ++ProductIndex)
            {
                State.Restock(ProductIndex);
            }
            Command("ToggleShop");
            SmokeStage = 1;
        }
        if (SmokeStage == 1 && GetWorld()->GetTimeSeconds() > 25)
        {
            CloseShop();
            if (!Require(State.Day == 2 && State.LastServed > 0 && State.Stock[0].Incoming == 0, TEXT("customer sale and next-day delivery"))) return false;
            const int64 SavedCash = State.Cash;
            State.Cash = 0;
            LoadCampaign();
            if (!Require(State.Cash == SavedCash && State.bCashier, TEXT("disk save and load"))) return false;
            UE_LOG(LogTemp, Display, TEXT("MirasMarket smoke PASSED: player, restock, order, hiring, %d customer sales, day close, disk save/load."), State.LastServed);
            SmokeStage = 2;
            FPlatformMisc::RequestExitWithStatus(false, 0);
        }
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("MirasCapture")))
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
            { FVector(0, -180, 90), FRotator(-4, 90, 0), TEXT("MirasMarket.png") },
            { FVector(-330, 300, 90), FRotator(-6, 165, 0), TEXT("MirasMarket_1_duvar_sol.png") },
            { FVector(150, 330, 90), FRotator(-8, 120, 0), TEXT("MirasMarket_2_gondol.png") },
            { FVector(110, 120, 90), FRotator(-12, 132, 0), TEXT("MirasMarket_3_dokme.png") },
            { FVector(330, 300, 90), FRotator(-6, 15, 0), TEXT("MirasMarket_4_duvar_sag.png") },
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

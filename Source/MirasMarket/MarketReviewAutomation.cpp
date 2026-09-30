#include "MarketGame.h"
#include "MarketBranchVisit.h"
#include "MarketStoreKit.h"
#include "MarketStoreViews.h"
#include "MarketBranches.h"
#include "Camera/CameraActor.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
namespace MarketVisitReview
{
    struct FReview
    {
        int32 Step = 0;
        double At = 0;
        TArray<uint8> Before;
        FTransform Player;
        FRotator Control;
        FString Plan, Directory;
        float DayTime = 0;
        bool Test = false;
        TWeakObjectPtr<ACameraActor> Camera;
        TMap<FString,TArray<uint8>> Saves;
    };
    FReview Review;
    TMap<FString,TArray<uint8>> SaveFiles()
    {
        TMap<FString,TArray<uint8>> Files; TArray<FString> Names;
        IFileManager::Get().FindFilesRecursive(Names,*(FPaths::ProjectSavedDir()/TEXT("SaveGames")),TEXT("*.sav"),true,false);
        for(const FString& Name:Names) FFileHelper::LoadFileToArray(Files.FindOrAdd(Name),*Name);
        return Files;
    }
}
bool AMarketGameMode::TickBranchVisitReview()
{
    if (!FParse::Param(FCommandLine::Get(),TEXT("BranchVisitReview"))) return true;
    auto& R = MarketVisitReview::Review;
    const double Now = FPlatformTime::Seconds();
    auto Fail = [](const TCHAR* Why) { UE_LOG(LogTemp,Error,TEXT("BranchVisitReview FAILED: %s"),Why); FPlatformMisc::RequestExitWithStatus(false,1); return false; };
    if (GetWorld()->GetTimeSeconds() < 4) return false;
    auto* PC = GetWorld()->GetFirstPlayerController(); APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    if (!Pawn) return Fail(TEXT("no walking player"));
    if (R.Step == 0)
    {
        bOpen = true; bNeedStart = false; bTestMode = false; State.Day = 20;
        FMarketBranch Branch; Branch.Name = TEXT("Inceleme Subesi"); Branch.Format = TEXT("mahalle"); Branch.Stage = static_cast<uint8>(MarketBranches::EStage::Open);
        Branch.Country = TEXT("tr"); Branch.Province = TEXT("kirklareli"); Branch.Workers = 3; Branch.LastShoppers = 120; Branch.LastQueueLost = 15; Branch.ManagerName = TEXT("Ayse"); Branch.OpenedDay = 1; Branch.Satisfaction = 20; Branch.Last30Profit = -100000;
        TArray<FString> Errors; if (!MarketStoreKit::Load(Errors)) return Fail(TEXT("store catalog"));
        const auto Ids = MarketStoreViews::IdsFor(Branch.Format); if (Ids.IsEmpty()) return Fail(TEXT("no branch view")); Branch.StoreView = Ids[0];
        for(int32 Index=0;Index<Products.Num();++Index) { FMarketBranchItem Item; Item.ProductId=Products[Index].Id; Item.LastEmpty=10; Item.Capacity=24; Item.Units=Index%3==0?0:Index%3==1?12:24; Branch.Items.Add(Item); }
        State.Branches.Add(Branch);
        R.Before = MarketBranchVisit::StateBytes(State); R.Player = Pawn->GetActorTransform(); R.Control = PC->GetControlRotation();
        R.Plan = MarketPlanogram::Serialize(Planogram); R.DayTime = DayTime; R.Test = bTestMode; R.Saves = MarketVisitReview::SaveFiles();
        R.Directory = FPaths::ProjectSavedDir()/TEXT("Screenshots/BranchVisit")/FDateTime::Now().ToString(TEXT("%Y%m%d-%H%M%S"));
        IFileManager::Get().MakeDirectory(*R.Directory,true);
        if (!StartBranchVisit(State.Branches.Num()-1)) return Fail(TEXT("visit did not open"));
        R.Camera = GetWorld()->SpawnActor<ACameraActor>(); PC->SetViewTarget(R.Camera.Get());
        R.Step = 1; R.At = Now; return false;
    }
    if (Now - R.At < 5) return false;
    if (R.Step < 7 && R.Step % 2 == 1)
    {
        const auto* Store = MarketStoreKit::Find(ActiveStoreKitId); if(!Store || !R.Camera.IsValid()) return Fail(TEXT("visit scene"));
        FVector Till = Store->Entrance.At+FVector(150,180,0);
        for(const auto& Fixture:Store->Fixtures) if(MarketPlanogram::Equipment(Fixture.EquipmentId).Family==TEXT("checkout")){Till=Fixture.Location;break;}
        const FVector Eyes[]={Store->PlayerStart.At+FVector(0,50,80),FVector(Store->FootprintCm.X*.3f,0,190),Till+FVector(300,-30,190)};
        const FVector Targets[]={FVector(0,0,120),FVector(0,150,100),Till+FVector(0,70,100)};
        R.Camera->SetActorLocationAndRotation(Eyes[(R.Step-1)/2],(Targets[(R.Step-1)/2]-Eyes[(R.Step-1)/2]).Rotation());
        ++R.Step; R.At=Now;
        // Separate view change from its screenshot by several settled frames.
        return false;
    }
    if (R.Step < 7 && R.Step % 2 == 0)
    {
        const int32 View = R.Step/2-1;
        FScreenshotRequest::RequestScreenshot(R.Directory/FString::Printf(TEXT("%02d.png"),View+1),true,false);
        ++R.Step;
        R.At=Now; return false;
    }
    // All three captures have settled before restoring the family scene.
    EndBranchVisit(); if(R.Camera.IsValid())R.Camera->Destroy();
    bool SameSaves = R.Saves.Num() == MarketVisitReview::SaveFiles().Num();
    const auto AfterSaves=MarketVisitReview::SaveFiles();
    for(const auto& Pair:R.Saves) { const auto* Bytes=AfterSaves.Find(Pair.Key); SameSaves &= Bytes && *Bytes==Pair.Value; }
    if (R.Before != MarketBranchVisit::StateBytes(State) || R.Plan != MarketPlanogram::Serialize(Planogram) || DayTime != R.DayTime || bTestMode != R.Test || !Pawn->GetActorTransform().Equals(R.Player,.01) || !PC->GetControlRotation().Equals(R.Control,.01) || !SameSaves) return Fail(TEXT("campaign, clock, position or disk save changed"));
    UE_LOG(LogTemp,Display,TEXT("BranchVisitReview PASSED: 3 views, exact campaign/plan/clock/player/disk preservation; %s"),*R.Directory);
    FPlatformMisc::RequestExitWithStatus(false,0); return false;
}

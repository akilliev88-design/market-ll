#include "MarketGame.h"
#include "MarketStoreKit.h"
#include "MarketStoreEditing.h"
#include "ProductCatalog.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace StoreTour
{
    TMap<FString,FString> Names;
    TArray<FString> Ids()
    {
        TArray<FString> Result;
        for(const TCHAR* Format:{TEXT("mahalle"),TEXT("kucuk"),TEXT("buyuk"),TEXT("hiper")}) Result.Append(MarketStoreKit::TemplatesFor(Format));
        for(auto Id:MarketStoreEditing::TourIds())Result.AddUnique(Id);
        return Result;
    }
}
bool AMarketGameMode::StartStoreTour(const FString& Id,bool bRandom)
{
    TArray<FString> Errors;
    if(!MarketStoreKit::Load(Errors)) return false;
    FStoreTemplate Editable;
    FString EditError;const bool Custom=MarketStoreEditing::LoadTour(Id,Editable,EditError);
    const auto* Store=Custom?&Editable:MarketStoreKit::Find(Id); if(!Store) return false;
    if(FParse::Param(FCommandLine::Get(),TEXT("MirasStoreEditableTest"))) {Editable=*Store;Editable.bEditableShell=true;Store=&Editable;}
    const auto Overrides=Id==ActiveStoreKitId?StoreCategoryOverrides:TMap<FString,FString>();
    auto TourPlan=MarketStoreKit::ToPlanogram(*Store,Overrides);
    if(bRandom) MarketStoreKit::FillRandom(TourPlan,Products,StoreTourSeed++);
    if(!MarketStoreKit::Build(GetWorld(),*Store,TourPlan,Custom)) return false;
    StoreTour::Names.Add(Id,Store->Name);
    bStoreTour=true; bTestMode=true; bOpen=false;
    if(auto* PC=UGameplayStatics::GetPlayerController(this,0))
    {
        if(APawn* Pawn=PC->GetPawn())
        {
            Pawn->SetActorLocation(Store->PlayerStart.At,false,nullptr,ETeleportType::TeleportPhysics);
            if(auto* Character=Cast<ACharacter>(Pawn)) Character->GetCharacterMovement()->StopMovementImmediately();
            PC->SetViewTarget(Pawn);
        }
        PC->SetControlRotation(FRotator(0,Store->PlayerStart.Yaw,0));
        PC->ResetIgnoreMoveInput(); PC->ResetIgnoreLookInput(); PC->bShowMouseCursor=false; PC->SetInputMode(FInputModeGameOnly());
    }
    StoreTourNotice=bRandom?TEXT("Raflar rastgele dolduruldu."):TEXT("Raflar bo\u015f.");
    if(!StoreTourOverlay.IsValid()&&GEngine&&GEngine->GameViewport)
    {
        StoreTourOverlay=SNew(SOverlay).Visibility(EVisibility::HitTestInvisible)
            +SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(10)
            [SNew(SBorder).Padding(12).BorderBackgroundColor(FLinearColor(.025f,.035f,.04f,.96f))
                [SNew(STextBlock).ColorAndOpacity(FLinearColor::White).Text_Lambda([this]()
                {
                    const auto* Name=StoreTour::Names.Find(ActiveStoreKitId);
                    return FText::FromString(FString::Printf(TEXT("TEST MA\u011eAZA GEZ\u0130S\u0130 \u2014 %s (%s)\nWASD: y\u00fcr\u00fc \u00b7 Fare: bak \u00b7 F10: sonraki \u00b7 Shift+F10: \u00f6nceki \u00b7 F3 / F7: rastgele doldur \u00b7 T: raf kategorisi \u00b7 Esc: \u00e7\u0131k\n%s"),Name?**Name:*ActiveStoreKitId,*ActiveStoreKitId,*StoreTourNotice));
                })]]
            +SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)[SNew(STextBlock).Text(FText::FromString(TEXT("+"))).ColorAndOpacity(FLinearColor::White)];
        GEngine->GameViewport->AddViewportWidgetContent(StoreTourOverlay.ToSharedRef(),50);
    }
    UE_LOG(LogTemp,Display,TEXT("MirasStoreTour built: %s, seed %d, %d blocks"),*Id,StoreTourSeed-1,Planogram.Placements.Num());
    return true;
}
bool AMarketGameMode::StoreTourCommand(FName Action)
{
    if(!bStoreTour)
    {
        if(Action=="RandomFill")
        {
            if(!bTestMode) { Notify(TEXT("Rastgele doldurmak i\u00e7in F2 ile test modunu a\u00e7.")); return true; }
            MarketStoreKit::FillRandom(Planogram,Products,StoreTourSeed++);
            RebuildShelfContents(); FillAllShelves(); RefreshLabels(); RefreshCategorySigns();
            Notify(TEXT("TEST: raflar rastgele dolduruldu. F7 yeniden doldurur.")); return true;
        }
        if(Action=="TourNext"||Action=="TourPrevious") { Notify(TEXT("Ma\u011fazalar\u0131 gezmek i\u00e7in MAGAZA_GEZI.cmd dosyas\u0131n\u0131 a\u00e7.")); return true; }
        return false;
    }
    if(Action=="TourNext"||Action=="TourPrevious")
    {
        const auto Ids=StoreTour::Ids(); const int32 Index=Ids.IndexOfByKey(ActiveStoreKitId);
        if(!Ids.IsEmpty()) StartStoreTour(Ids[(FMath::Max(0,Index)+Ids.Num()+(Action=="TourNext"?1:-1))%Ids.Num()],true);
    }
    else if(Action=="RandomFill"||Action=="FillAll") StartStoreTour(ActiveStoreKitId,true);
    else if(Action=="Fullscreen") return false;
    else StoreTourNotice=TEXT("Ma\u011faza gezisinde test modu a\u00e7\u0131k. F10 ile ma\u011faza de\u011fi\u015ftir, F3 / F7 ile raflar\u0131 doldur.");
    return true;
}
void AMarketGameMode::TickStoreTour()
{
    if(FParse::Param(FCommandLine::Get(),TEXT("MirasStoreTourSavedTest")))
    {
        const float Now=GetWorld()->GetTimeSeconds();if(Now<6)return;
        if(CaptureStage==0){FStoreTemplate S;FString E;auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);if(!MarketStoreEditing::LoadTour(ActiveStoreKitId,S,E)||!Pawn||Pawn->GetActorLocation().Z<50||Planogram.Fixtures.Num()!=S.Fixtures.Num()){UE_LOG(LogTemp,Error,TEXT("SavedStoreTour FAILED: layout or walking floor"));FPlatformMisc::RequestExitWithStatus(false,1);return;}
            for(int32 I=0;I<S.Fixtures.Num();++I)if(!Planogram.Fixtures[I].Location.Equals(S.Fixtures[I].Location,.01)){UE_LOG(LogTemp,Error,TEXT("SavedStoreTour FAILED: fixture moved"));FPlatformMisc::RequestExitWithStatus(false,1);return;}
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/Stores")/(ActiveStoreKitId+TEXT("_SavedTour.png")),true,false);CaptureStage=1;CaptureAt=Now;return;}
        if(Now>CaptureAt+2){UE_LOG(LogTemp,Display,TEXT("SavedStoreTour PASSED: exact edited layout and walking floor, %s"),*ActiveStoreKitId);FPlatformMisc::RequestExitWithStatus(false,0);}return;
    }
    if(!FParse::Param(FCommandLine::Get(),TEXT("MirasStoreTourTest"))) return;
    const float Now=GetWorld()->GetTimeSeconds();
    if(CaptureStage%3==1)
    {
        if(Now<CaptureAt+4) return; // settle the newly rebuilt instances and temporal lighting
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/Stores")/(ActiveStoreKitId+TEXT("_Tour.png")),true,false);
        ++CaptureStage; CaptureAt=Now; return;
    }
    if(CaptureStage%3==2)
    {
        if(Now<CaptureAt+1) return; // allow the screenshot to finish before switching worlds
        if(CaptureStage>=StoreTour::Ids().Num()*3-1)
        { UE_LOG(LogTemp,Display,TEXT("MirasStoreTour PASSED: all stores, random fill, walking pawn")); FPlatformMisc::RequestExitWithStatus(false,0); return; }
        StoreTourCommand(TEXT("TourNext")); ++CaptureStage; CaptureAt=Now; return;
    }
    if(Now<CaptureAt+6) return;
    const int64 CashBefore=State.Cash;
    auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);
    if(!Pawn||Pawn->GetActorLocation().Z<50||Planogram.Placements.IsEmpty())
    { UE_LOG(LogTemp,Error,TEXT("MirasStoreTour test: invalid walking pawn, floor or dressing")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
    const FString Before=MarketPlanogram::Serialize(Planogram);
    StoreTourCommand(TEXT("RandomFill"));
    if(State.Cash!=CashBefore||MarketPlanogram::Serialize(Planogram)==Before)
    { UE_LOG(LogTemp,Error,TEXT("MirasStoreTour test: random fill did not change or cash changed")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
    UE_LOG(LogTemp,Display,TEXT("MirasStoreTour test: %s walking, random fill and cash isolation passed"),*ActiveStoreKitId);
    ++CaptureStage; CaptureAt=Now;
}

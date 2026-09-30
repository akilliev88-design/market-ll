#include "MarketGame.h"
#include "MarketStoreKit.h"
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
    TArray<FString> Ids()
    {
        TArray<FString> Result;
        for(const TCHAR* Format:{TEXT("mahalle"),TEXT("kucuk"),TEXT("buyuk"),TEXT("hiper")}) Result.Append(MarketStoreKit::TemplatesFor(Format));
        return Result;
    }
}
bool AMarketGameMode::StartStoreTour(const FString& Id,bool bRandom)
{
    TArray<FString> Errors;
    if(!MarketStoreKit::Load(Errors)) return false;
    const auto* Store=MarketStoreKit::Find(Id); if(!Store) return false;
    FStoreTemplate Editable;
    if(FParse::Param(FCommandLine::Get(),TEXT("MirasStoreEditableTest"))) {Editable=*Store;Editable.bEditableShell=true;Store=&Editable;}
    const auto Overrides=Id==ActiveStoreKitId?StoreCategoryOverrides:TMap<FString,FString>();
    auto TourPlan=MarketStoreKit::ToPlanogram(*Store,Overrides);
    if(bRandom) MarketStoreKit::FillRandom(TourPlan,Products,StoreTourSeed++);
    if(!MarketStoreKit::Build(GetWorld(),*Store,TourPlan)) return false;
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
                    const auto* S=MarketStoreKit::Find(ActiveStoreKitId);
                    return FText::FromString(FString::Printf(TEXT("TEST MA\u011eAZA GEZ\u0130S\u0130 \u2014 %s\nWASD: y\u00fcr\u00fc \u00b7 Fare: bak \u00b7 F10: sonraki \u00b7 Shift+F10: \u00f6nceki \u00b7 F3 / F7: rastgele doldur \u00b7 T: raf kategorisi \u00b7 Esc: \u00e7\u0131k\n%s"),S?*S->Name:*ActiveStoreKitId,*StoreTourNotice));
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

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MarketPeople.h"
#include "MarketArtTrial.generated.h"

class ACameraActor;

// Isolated, reproducible art trial. No campaign, save, money or inventory is created.
UCLASS()
class AMarketArtTrialGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AMarketArtTrialGameMode();
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
private:
    void SetReviewView(int32 Index);
    UPROPERTY() TObjectPtr<ACameraActor> ReviewCamera;
    UPROPERTY() TObjectPtr<AActor> ReviewPerson;
    UPROPERTY() TArray<TObjectPtr<UObject>> PeopleAssets;
    MarketPeople::FLibrary People;
    MarketPeople::FShopper Shopper;
    bool bCapture = false;
    bool bReady = false;
    int32 View = 0;
    int32 Stage = 0;
    double ReadyAt = -1;
    double StageAt = 0;
    double LastFrameAt = 0;
    double StartedAt = 0;
    TArray<float> FrameTimes;
    FString OutputDirectory;
};

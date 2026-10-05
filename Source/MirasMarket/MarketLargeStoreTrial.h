#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MarketStoreKit.h"
#include "MarketLargeStoreTrial.generated.h"
class ACameraActor;
UCLASS()
class AMarketLargeStoreTrialGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AMarketLargeStoreTrialGameMode();
    virtual void InitGame(const FString& Map, const FString& Options, FString& Error) override;
    virtual void BeginPlay() override;
    virtual void Tick(float Dt) override;
private:
    void View(int32 Index);
    UPROPERTY() TObjectPtr<ACameraActor> Camera;
    FStoreTemplate Store;
    bool Capture=false,Ready=false,Checked=false;
    int32 Stage=0;
    double Started=0;
    TMap<TWeakObjectPtr<USceneComponent>,FTransform> Transforms;
};

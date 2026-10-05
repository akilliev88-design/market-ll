#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MarketGeneratedStore.generated.h"
class ACameraActor;
UCLASS()
class AMarketGeneratedStoreGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AMarketGeneratedStoreGameMode();
    virtual void InitGame(const FString&, const FString&, FString&) override;
    virtual void BeginPlay() override;
    virtual void Tick(float) override;
private:
    void View(int32 Index);
    UPROPERTY() TObjectPtr<ACameraActor> Camera;
    FVector Origin = FVector::ZeroVector;
    double Started = 0;
    int32 Stage = 0;
    bool Capture = false;
    bool PhysicsOnly = false;
    bool Ready = false;
};

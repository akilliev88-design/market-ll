#pragma once
#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "MarketSimAutoPlayCommandlet.generated.h"

UCLASS()
class UMarketSimAutoPlayCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    UMarketSimAutoPlayCommandlet();
    virtual int32 Main(const FString& Params) override;
};

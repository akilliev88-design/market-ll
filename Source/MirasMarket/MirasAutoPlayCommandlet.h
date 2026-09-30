#pragma once
#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "MirasAutoPlayCommandlet.generated.h"

UCLASS()
class UMirasAutoPlayCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    UMirasAutoPlayCommandlet();
    virtual int32 Main(const FString& Params) override;
};

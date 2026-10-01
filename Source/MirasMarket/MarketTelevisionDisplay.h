#pragma once
#include "Planogram.h"

class UWorld;
class AActor;

namespace MarketTelevisionDisplay
{
    struct FProfile
    {
        FString Technology;
        FString Resolution;
        int32 Inches = 0;
        int32 RefreshHz = 0;
        bool bRearLight = false;
        FColor RearColor = FColor(60, 130, 255);
    };
    MIRASMARKET_API bool IsDisplay(const FString& EquipmentId);
    MIRASMARKET_API bool Parse(const FString& Json, TMap<FString, FProfile>& Out, FString& Error);
    MIRASMARKET_API TMap<FString, FProfile> Load();
    MIRASMARKET_API FString Features(const FProfile& Profile);
    // Product identity is explicit; missing specifications never imply a technology.
    MIRASMARKET_API void Decorate(UWorld* World, const FMarketPlanogram& Plan, const TArray<FMarketProduct>& Products,
        const FPlanogramPlacement& Placement, const FString& Name, const FProfile* Profile, TArray<AActor*>& Actors);
}

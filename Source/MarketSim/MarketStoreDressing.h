#pragma once
#include "CoreMinimal.h"
class UWorld;
struct FStoreTemplate;
struct FMarketPlanogram;
struct FMarketProduct;
namespace MarketStoreDressing
{
    // Visual-only authored produce. No inventory, price or catalogue mutation.
    MARKETSIM_API void Build(UWorld* World, const FStoreTemplate& Store);
    MARKETSIM_API void FillPreview(FMarketPlanogram& Plan, const TArray<FMarketProduct>& Products);
}

#pragma once
#include "CoreMinimal.h"
#include "MarketEconomy.h"
namespace MarketBranchVisit
{
    float Fill(const FMarketBranch& Branch, const FString& ProductId);
    int32 Shoppers(const FMarketBranch& Branch);
    int32 Queue(const FMarketBranch& Branch);
    int32 Workers(const FMarketBranch& Branch);
    TArray<uint8> StateBytes(const FMarketState& State);
}
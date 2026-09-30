#pragma once
#include "MarketStoreKit.h"
namespace MarketStoreEditing
{
    MIRASMARKET_API FVector2D HalfSize(const FPlanogramFixture& Fixture);
    // Moves are rejected when they intersect fixtures, structures, depot, or floor recesses.
    MIRASMARKET_API bool CanPlace(const FStoreTemplate& Store,const FPlanogramFixture& Fixture,int32 Ignore=INDEX_NONE);
    MIRASMARKET_API FVector Snap(const FStoreTemplate& Store,const FPlanogramFixture& Fixture,bool Walls,bool Neighbours,float GridCm);
    MIRASMARKET_API bool Place(FStoreTemplate& Store,const FString& Equipment,const FString& Category,FVector At,int32& Selected);
    MIRASMARKET_API bool Duplicate(FStoreTemplate& Store,int32 Selected,float GapCm,int32& NewSelection);
    MIRASMARKET_API bool Resize(FStoreTemplate& Store,double WidthCm,double DepthCm,double DepotWidthCm,double DepotDepthCm,double HeightCm,FString& Error);
    MIRASMARKET_API bool Save(const FStoreTemplate& Store,const FString& Path,FString& Error);
    MIRASMARKET_API bool LoadDraft(const FString& Path,FStoreTemplate& Store,FString& Error);
    MIRASMARKET_API void BuildArchitecture(UWorld* World,const FStoreTemplate& Store);
}
